//
// Created by Syl on 26/02/2023.
//

#include "Tank.h"

namespace Toro
{
    void Tank::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
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
        m_leftDecayDiffuser1.setCoeff(m_decayDiffusion1);
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
        m_rightDecayDiffuser1.setCoeff(m_decayDiffusion1);
        m_rightDecayDiffuser2.setDelayTimeSamples(static_cast<int>(8.9244e-2 * sampleRate));
        m_rightDecayDiffuser2.setCoeff(m_decayDiffusion2);
        m_rightDecayDiffuser1.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_rightDecayDiffuser2.prepareToPlay(samplesPerBlockExpected, sampleRate);
        m_rightDampingFilter.setCoeff(-m_damping);


    }

    std::tuple<float, float> Tank::processSample(float in) {

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

    std::tuple<float, float> Tank::tap() {
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