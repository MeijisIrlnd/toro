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

    void DatorroSimple::Input::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        m_sampleRate = static_cast<float>(sampleRate);
        m_bandwidthFilter.setCoeff(m_bandwidth);
        for(size_t i = 0; i < m_inputDiffusers.size(); ++i) {
            m_inputDiffusers[i].setDelayTimeSamples(static_cast<int>(m_inputDiffuserTimes[i] * sampleRate));
            m_inputDiffusers[i].setCoeff(m_inputDiffuserCoeffs[i]);
            m_inputDiffusers[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
        };

        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1};
        m_preDelayLine.prepare(spec);
        m_preDelayLine.setMaximumDelayInSamples(static_cast<int>(0.5 * sampleRate));
    }

    DatorroSimple::Input::OutputSample DatorroSimple::Input::processSample(float in) noexcept {
        auto x = m_preDelayLine.popSample(0, static_cast<float>(m_preDelaySeconds * m_sampleRate));
        m_preDelayLine.pushSample(0, in);
        auto earlyReflections = 0.0f;
        x = m_bandwidthFilter.processSample(x);
        for(auto i = 0; i < m_inputDiffusers.size(); ++i) {
            x = m_inputDiffusers[i].processSample(x);
            earlyReflections += (x / std::powf(2, static_cast<float>(m_inputDiffusers.size() - i)));
        }
        return {x, earlyReflections * m_earlyReflectionsLevel};
    }

    void DatorroSimple::Tank::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1};
        m_leftDelay1.prepare(spec);
        m_leftDelay2.prepare(spec);
        m_tapIndices.prepare(sampleRate);
        // times..
        //4453
        auto leftDelay1 = static_cast<float>(0.14962f * sampleRate);
        auto leftDelay2 = static_cast<float>(0.1249f * sampleRate);
        auto rightDelay1 = static_cast<float>(0.1417f * sampleRate);
        auto rightDelay2 = static_cast<float>(0.1063f * sampleRate);


        m_leftDelay1.setMaximumDelayInSamples(static_cast<int>(leftDelay1) + 1);
        m_leftDelay1.setDelay(leftDelay1);
        m_leftDelay2.setMaximumDelayInSamples(static_cast<int>(leftDelay2) + 1);
        m_leftDelay2.setDelay(leftDelay2);
        // Inverted coeff here..
        m_leftDecayDiffuser1.setCoeff(-m_decayDiffusion1);
        m_leftDecayDiffuser1.setDelayTimeSamples(static_cast<int>(2.2579e-2 * sampleRate));
        m_leftDecayDiffuser1.setLfoRate(0.01f);
        m_leftDecayDiffuser1.setExcursionSeconds(m_excursionTimeMS / 1000.0f);
        m_leftDecayDiffuser2.setDelayTimeSamples(static_cast<int>(6.0481e-2 * sampleRate));
        m_leftDecayDiffuser2.setCoeff(m_decayDiffusion2);
        m_leftDecayDiffuser1.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_leftDecayDiffuser2.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_leftDampingFilter.setCoeff(-m_damping);

        m_rightDelay1.prepare(spec);
        m_rightDelay2.prepare(spec);
        m_rightDelay1.setMaximumDelayInSamples(static_cast<int>(rightDelay1) + 1);
        m_rightDelay1.setDelay(rightDelay1);
        m_rightDelay2.setMaximumDelayInSamples(static_cast<int>(rightDelay2) + 1);
        m_rightDelay2.setDelay(rightDelay2);
        m_rightDecayDiffuser1.setDelayTimeSamples(static_cast<int>(3.0509e-2 * sampleRate));
        m_rightDecayDiffuser1.setLfoRate(0.03f);
        m_rightDecayDiffuser1.setExcursionSeconds(m_excursionTimeMS / 1000.0f);
        // Inverted coeff
        m_rightDecayDiffuser1.setCoeff(-m_decayDiffusion1);
        m_rightDecayDiffuser2.setDelayTimeSamples(static_cast<int>(8.9244e-2 * sampleRate));
        m_rightDecayDiffuser2.setCoeff(m_decayDiffusion2);
        m_rightDecayDiffuser1.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_rightDecayDiffuser2.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_rightDampingFilter.setCoeff(-m_damping);


    }

    std::tuple<float, float> DatorroSimple::Tank::processSample(float in) {

        float left = in + m_prevR;
        left = m_leftDecayDiffuser1.processSample(left);
        auto leftDelayOut = m_leftDelay1.popSample(0);
        m_leftDelay1.pushSample(0, left);
        leftDelayOut = m_leftDampingFilter.processSample(leftDelayOut);
        leftDelayOut *= m_decay;
        leftDelayOut = m_leftDecayDiffuser2.processSample(leftDelayOut);

        m_leftDelay2.pushSample(0, leftDelayOut);

        float right = in + m_prevL;
        right = m_rightDecayDiffuser1.processSample(right);
        auto rightDelayOut = m_rightDelay1.popSample(0);
        m_rightDelay1.pushSample(0, right);
        rightDelayOut = m_rightDampingFilter.processSample(rightDelayOut);
        rightDelayOut *= m_decay;
        rightDelayOut = m_rightDecayDiffuser2.processSample(rightDelayOut);
        m_rightDelay2.pushSample(0, rightDelayOut);

        m_prevL = m_leftDelay2.popSample(0) * m_decay;
        m_prevR = m_rightDelay2.popSample(0) * m_decay;
        return tap();
    }

    std::tuple<float, float> DatorroSimple::Tank::tap() {
        auto leftAccumulator = 0.6f * m_rightDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesL[0]), false);
        leftAccumulator += 0.6f * m_rightDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesL[1]), false);
        leftAccumulator -= 0.6f * m_rightDecayDiffuser2.tap(m_tapIndices.tapIndicesL[2]);
        leftAccumulator += 0.6f * m_rightDelay2.popSample(0, static_cast<float>(m_tapIndices.tapIndicesL[3]), false);
        leftAccumulator -= 0.6f * m_leftDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesL[4]), false);
        leftAccumulator -= 0.6f * m_leftDecayDiffuser2.tap(m_tapIndices.tapIndicesL[5]);
        leftAccumulator -= 0.6f * m_leftDelay2.popSample(0, static_cast<float>(m_tapIndices.tapIndicesL[6]), false);

        auto rightAccumulator = 0.6f * m_leftDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesR[0]), false);
        rightAccumulator += 0.6f * m_leftDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesR[1]), false);
        rightAccumulator -= 0.6f * m_leftDecayDiffuser2.tap(m_tapIndices.tapIndicesR[2]);
        rightAccumulator += 0.6f * m_leftDelay2.popSample(0, static_cast<float>(m_tapIndices.tapIndicesR[3]), false);
        rightAccumulator -= 0.6f * m_rightDelay1.popSample(0, static_cast<float>(m_tapIndices.tapIndicesR[4]), false);
        rightAccumulator -= 0.6f * m_rightDecayDiffuser2.tap(m_tapIndices.tapIndicesR[5]);
        rightAccumulator -= 0.6f * m_rightDelay2.popSample(0, static_cast<float>(m_tapIndices.tapIndicesR[6]), false);

        return {leftAccumulator, rightAccumulator};
    }


}