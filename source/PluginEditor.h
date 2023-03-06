/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include <SUX/SUX.h>

//==============================================================================
/**
*/
using APVTS = juce::AudioProcessorValueTreeState;
class PluginEditor  : public juce::AudioProcessorEditor
{
public:
    PluginEditor (PluginProcessor&, APVTS&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& audioProcessor;
    APVTS& m_tree;
    juce::ComboBox m_typeSelector;
    std::unique_ptr<juce::ComboBoxParameterAttachment> m_typeAttachment;
    SUX::Quickstart::LayoutQuickstart m_layout;
    SUX::Quickstart::QuickSlider m_preDelaySlider, m_earlyReflectionsSlider, m_decaySlider, m_excursionTimeSlider, m_decayDiffusion1Slider, m_decayDiffusion2Slider, m_inputDiffusion1Slider, m_inputDiffusion2Slider, m_bandwidthSlider, m_dampingSlider, m_dryWetSlider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
