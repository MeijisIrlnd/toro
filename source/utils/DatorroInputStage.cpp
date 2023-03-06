//
// Created by Syl on 05/03/2023.
//

#include "DatorroInputStage.h"

namespace Toro {
    void DatorroInputStage::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
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

    DatorroInputStage::OutputSample DatorroInputStage::processSample(float in) noexcept {
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
} // Toro