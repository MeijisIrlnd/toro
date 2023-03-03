/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <SinglePoleLowpass.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DecayDiffuser.h"
#include "Tank.h"

//==============================================================================
/**
*/
using APVTS = juce::AudioProcessorValueTreeState;
class PluginProcessor  : public juce::AudioProcessor, public APVTS::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PluginProcessor();
    ~PluginProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& id, float value) override;
private:
    static APVTS::ParameterLayout createLayout();
    void bindListeners();
    bool m_hasBeenPrepared{ false };
    double m_sampleRate{ 44100 };
    APVTS m_tree;
    float m_bandwidth{ 0.7f };
    float m_preDelaySeconds{ 0.0f };
    float m_earlyReflectionsLevel{ 0.5f };
    juce::dsp::DelayLine<float> m_preDelay;
    juce::dsp::DryWetMixer<float> m_mixer;
    Toro::SinglePoleLowpass m_lowpass;
    std::array<Toro::TypeAAPF, 4> m_inputDiffusers;
    const std::array<float, 4> m_inputDiffuserTimes = {
            //4.7713e-3f,
            1e-5,
            3.5953e-3f,
            1.2735e-2f,
            9.3075e-3f
    };
    std::array<float, 4> m_inputDiffuserCoeffs = {
            0.75f, 0.75f,
            0.625f, 0.625f
    };
    Toro::Tank m_tank;
    float m_dryWet{ 0.5f };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
