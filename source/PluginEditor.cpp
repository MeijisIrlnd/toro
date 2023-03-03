/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p, APVTS& tree)
    : AudioProcessorEditor (&p), audioProcessor (p), m_tree(tree)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    m_layout.instantiateSlider(this, m_preDelaySlider, m_tree, "PreDelay");
    m_layout.instantiateSlider(this, m_earlyReflectionsSlider, m_tree, "EarlyReflections");
    m_layout.instantiateSlider(this, m_decaySlider, m_tree, "Decay");
    m_layout.instantiateSlider(this, m_excursionTimeSlider, m_tree, "ExcursionMS");
    m_layout.instantiateSlider(this, m_inputDiffusion1Slider, m_tree, "InputDiffusion1");
    m_layout.instantiateSlider(this, m_inputDiffusion2Slider, m_tree, "InputDiffusion2");
    m_layout.instantiateSlider(this, m_decayDiffusion1Slider, m_tree, "DecayDiffusion1");
    m_layout.instantiateSlider(this, m_decayDiffusion2Slider, m_tree, "DecayDiffusion2");
    m_layout.instantiateSlider(this, m_bandwidthSlider, m_tree, "Bandwidth");
    m_layout.instantiateSlider(this, m_dampingSlider, m_tree, "Damping");
    m_layout.instantiateSlider(this, m_dryWetSlider, m_tree, "DryWet");
    //resized();
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
    
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    if(m_layout.uiElements.empty()) return;
    auto h = getHeight() / m_layout.uiElements.size();
    for(auto i = 0; i < m_layout.uiElements.size(); ++i) {
        m_layout.uiElements[i]->label.setBounds(getLocalBounds().withY(h * i).withHeight(h).withWidth(getWidth() / 4));
        m_layout.uiElements[i]->slider.setBounds(getLocalBounds().withY(h * i).withHeight(h).withX(getWidth() / 4).withWidth(getWidth() - getWidth() / 4));
    }
}
