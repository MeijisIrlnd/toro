//
// Created by Syl on 05/03/2023.
//

#include "DatorroSimple.h"

namespace Toro
{

    void DatorroSimple::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        m_inputStage.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_tankStage.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_dryWetMixer.prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 2});
    }

    void DatorroSimple::getNextAudioBlock(juce::AudioBuffer<float> &buffer) {
        auto* read = buffer.getArrayOfReadPointers();
        auto* write = buffer.getArrayOfWritePointers();
        m_dryWetMixer.pushDrySamples(buffer);
        for(auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto avg = 0.0f;
            for(auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
                avg += read[channel][sample];
            }
            avg /= static_cast<float>(buffer.getNumChannels());
            auto[x, earlyReflections] = m_inputStage.processSample(avg);
            auto[l, r] = m_tankStage.processSample(x);
            write[0][sample] = (l + earlyReflections);
            write[1][sample] = (r + earlyReflections);
        }
        m_dryWetMixer.setWetMixProportion(m_dryWet);
        m_dryWetMixer.mixWetSamples(buffer);
    }
}