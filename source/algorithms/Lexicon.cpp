//
// Created by Syl on 05/03/2023.
//

#include "Lexicon.h"

namespace Toro {
    void Lexicon::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        m_inputStage.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_tankStage.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_dryWetMixer.prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 2});
    }

    void Lexicon::getNextAudioBlock(juce::AudioBuffer<float> &buffer) {
        m_dryWetMixer.pushDrySamples(buffer);
        auto* read = buffer.getArrayOfReadPointers();
        auto* write = buffer.getArrayOfWritePointers();
        for(auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
            auto avg = 0.0f;
            for(auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
                avg += read[channel][sample];
            }
            avg /= static_cast<float>(buffer.getNumChannels());
            auto[leftLeg, rightLeg, erLeft, erRight] = m_inputStage.processSample(avg);
            auto[l, r] = m_tankStage.processSample(leftLeg, rightLeg);
            write[0][sample] = l + erLeft;
            write[1][sample] = r + erRight;
        }
        m_dryWetMixer.setWetMixProportion(m_dryWet);
        m_dryWetMixer.mixWetSamples(buffer);
    }
} // Toro