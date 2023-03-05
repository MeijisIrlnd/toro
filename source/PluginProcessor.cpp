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
 m_tree(*this, nullptr, juce::Identifier{"Params"}, createLayout())
{
    bindListeners();
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
    m_datorroSimple.prepareToPlay(samplesPerBlock, sampleRate);
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
    m_datorroSimple.getNextAudioBlock(buffer);
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
        m_datorroSimple.setPreDelaySeconds(value);
    }
    else if(id == "EarlyReflections") {
        m_datorroSimple.setEarlyReflectionsLevel(value);
    }
    else if(id == "Decay") {
        m_datorroSimple.setDecay(value);
    }
    else if(id == "ExcursionMS") {
        m_datorroSimple.setExcursion(value);
    }
    else if(id == "DecayDiffusion1") {
        m_datorroSimple.setDecayDiffusion1(value);
    }
    else if(id == "DecayDiffusion2") {
        m_datorroSimple.setDecayDiffusion2(value);
    }
    else if(id == "InputDiffusion1") {
        m_datorroSimple.setInputDiffusion1(value);
    }
    else if(id == "InputDiffusion2") {
        m_datorroSimple.setInputDiffusion2(value);
    }
    else if(id == "Bandwidth") {
        m_datorroSimple.setBandwidth(value);
    }
    else if(id == "Damping") {
        m_datorroSimple.setDamping(value);
    }
    else if(id == "DryWet") {
        m_datorroSimple.setDryWet(value);
    }
}

APVTS::ParameterLayout PluginProcessor::createLayout() {
    using FloatParam = juce::AudioParameterFloat;
    APVTS::ParameterLayout layout;
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("PreDelay", 1), "Pre Delay", juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f), 0.0f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("EarlyReflections", 1), "Early Reflections", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("Decay", 1), "Decay", juce::NormalisableRange<float>(0.01f, 0.9f, 0.01f), 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("ExcursionMS", 1), "Excursion Time MS", juce::NormalisableRange<float>(0.2f, 1.0416f, 0.0001f), 0.53676f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("DecayDiffusion1", 1), "Decay Diffusion 1", juce::NormalisableRange<float>(0.1f, 0.9f, 0.01f), 0.7f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("DecayDiffusion2", 1), "Decay Diffusion 2", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("InputDiffusion1", 1), "Input Diffusion 1", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.75f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("InputDiffusion2", 1), "Input Diffusion 2", juce::NormalisableRange<float>{0.1f, 0.9f, 0.01f}, 0.625f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("Bandwidth", 1), "Bandwidth", juce::NormalisableRange<float>(0.01f, 0.99999f, 0.01f), 0.7f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("Damping", 1), "Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0005f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID("DryWet", 1), "DryWet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
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
