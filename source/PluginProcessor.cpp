/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
:
#endif
 m_tree(*this, nullptr, juce::Identifier{"Params"}, createLayout()),
 m_mixer(44100)
{
    bindListeners();
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    //setLatencySamples(static_cast<int>(0.1 * sampleRate));
    m_lowpass.setCoeff(1 - m_bandwidth);
    for(size_t i = 0; i < m_inputDiffusers.size(); ++i) {
        m_inputDiffusers[i].setDelayTimeSamples(static_cast<int>(m_inputDiffuserTimes[i] * sampleRate));
        m_inputDiffusers[i].setCoeff(m_inputDiffuserCoeffs[i]);
        m_inputDiffusers[i].prepareToPlay(samplesPerBlock, sampleRate);
    }
    m_tank.prepareToPlay(samplesPerBlock, sampleRate);
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1};
    m_preDelay.prepare(spec);
    m_preDelay.setMaximumDelayInSamples(static_cast<int>(0.5 * sampleRate));
    m_preDelay.setDelay(static_cast<float>(m_preDelaySeconds * sampleRate));
    m_mixer.prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2});
    m_hasBeenPrepared = true;
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //m_mixer.setWetLatency(static_cast<float>(0.151 * m_sampleRate));
    //m_mixer.setWetLatency(static_cast<float>(0.3 * m_sampleRate));
    m_mixer.pushDrySamples(juce::dsp::AudioBlock<float>{buffer});
    auto* read = buffer.getArrayOfReadPointers();
    auto* write = buffer.getArrayOfWritePointers();
    for(auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
        auto avg = 0.0f;
        for(auto channel = 0; channel < 2; ++channel) {
            avg += read[channel][sample];
        }
        float earlyReflections{ 0.0f };
        float delayInput  = avg / 2.0f;
        auto x = m_preDelay.popSample(0);
        m_preDelay.pushSample(0, delayInput);
        x = m_lowpass.processSample(x);

        for(auto i = 0; i < m_inputDiffusers.size(); ++i) {
            x = m_inputDiffusers[i].processSample(x);
            earlyReflections += (x / std::powf(2, static_cast<float>((m_inputDiffusers.size() - i))));
        }
        auto [l, r] = m_tank.processSample(x);
        write[0][sample] = l + (earlyReflections * m_earlyReflectionsLevel);
        write[1][sample] = r + (earlyReflections * m_earlyReflectionsLevel);
    }
    m_mixer.setWetMixProportion(m_dryWet);
    m_mixer.mixWetSamples(juce::dsp::AudioBlock<float>{buffer});
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this, m_tree);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = m_tree.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if(xmlState == nullptr) return;
    if(!xmlState->hasTagName(m_tree.state.getType())) return;
    auto newState = juce::ValueTree::fromXml(*xmlState);
    m_tree.replaceState(newState);
}

void PluginProcessor::parameterChanged(const juce::String &id, float value) {
    if(id == "PreDelay")  {
        m_preDelaySeconds = value;
        if(m_hasBeenPrepared) {
            m_preDelay.setDelay(static_cast<float>(m_preDelaySeconds * m_sampleRate));
        }
    }
    else if(id == "EarlyReflections") {
        m_earlyReflectionsLevel = value;
    }
    else if(id == "Decay") {
        m_tank.setDecay(value);
    }
    else if(id == "ExcursionMS") {
        m_tank.setExcursion(value);
    }
    else if(id == "DecayDiffusion1") {
        m_tank.setDecayDiffusion1(value);
    }
    else if(id == "DecayDiffusion2") {
        m_tank.setDecayDiffusion2(value);
    }
    else if(id == "InputDiffusion1") {
        m_inputDiffuserCoeffs[0] = m_inputDiffuserCoeffs[1] = value;
        m_inputDiffusers[0].setCoeff(m_inputDiffuserCoeffs[0]);
        m_inputDiffusers[1].setCoeff(m_inputDiffuserCoeffs[1]);
    }
    else if(id == "InputDiffusion2") {
        m_inputDiffuserCoeffs[2] = m_inputDiffuserCoeffs[3] = value;
        m_inputDiffusers[2].setCoeff(m_inputDiffuserCoeffs[2]);
        m_inputDiffusers[3].setCoeff(m_inputDiffuserCoeffs[3]);
    }
    else if(id == "Bandwidth") {
        m_lowpass.setCoeff(1 - value);
    }
    else if(id == "Damping") {
        m_tank.setDamping(value);
    }
    else if(id == "DryWet") {
        m_dryWet = value;
    }
}

APVTS::ParameterLayout PluginProcessor::createLayout() {
    using FloatParam = juce::AudioParameterFloat;
    APVTS::ParameterLayout layout;
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"PreDelay", 1}, "Pre Delay", juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f), 0.0f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"EarlyReflections", 1}, "Early Reflections", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Decay", 1}, "Decay", juce::NormalisableRange<float>(0.01f, 0.9f, 0.01f), 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"ExcursionMS", 1}, "Excursion Time MS", juce::NormalisableRange<float>(0.2f, 1.0416f, 0.0001f), 0.53676f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"DecayDiffusion1", 1}, "Decay Diffusion 1", juce::NormalisableRange<float>(0.1f, 0.9f, 0.01f), 0.7f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"DecayDiffusion2", 1}, "Decay Diffusion 2", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"InputDiffusion1", 1}, "Input Diffusion 1", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.75f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"InputDiffusion2", 1}, "Input Diffusion 2", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.625f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Bandwidth", 1}, "Bandwidth", juce::NormalisableRange<float>(0.01f, 0.99999f, 0.01f), 0.7f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Damping", 1}, "Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0005f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"DryWet", 1}, "DryWet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    return layout;

}

void PluginProcessor::bindListeners() {
    m_tree.addParameterListener("PreDelay", this);
    m_tree.addParameterListener("EarlyReflections", this);
    m_tree.addParameterListener("Decay", this);
    m_tree.addParameterListener("ExcursionMS", this);
    m_tree.addParameterListener("DecayDiffusion1", this);
    m_tree.addParameterListener("DecayDiffusion2", this);
    m_tree.addParameterListener("InputDiffusion1", this);
    m_tree.addParameterListener("InputDiffusion2", this);
    m_tree.addParameterListener("Bandwidth", this);
    m_tree.addParameterListener("Damping", this);
    m_tree.addParameterListener("DryWet", this);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
