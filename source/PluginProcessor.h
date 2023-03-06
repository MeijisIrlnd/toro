/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <algorithms/DatorroSimple.h>
#include <algorithms/Lexicon.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "algorithms/AlgorithmType.h"


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
    ~PluginProcessor() override = default;

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
    Toro::Lexicon m_datorroSimple;
    bool m_hasBeenPrepared{ false };
    double m_sampleRate{ 44100 };
    APVTS m_tree;
    std::atomic_int m_currentProcessorIndex{0};
    struct ReverbAlgorithm {
        std::unique_ptr<Toro::ReverbBase> processor{ nullptr };
        Toro::ALGORITHM_TYPE type;
    };
    std::vector<ReverbAlgorithm> m_reverbProcessors;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
