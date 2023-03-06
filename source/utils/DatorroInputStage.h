//
// Created by Syl on 05/03/2023.
//
#pragma once
#include <SDSP/Filters/APF.h>
#include <SDSP/Filters/UtilityFilters.h>
namespace Toro {

    class DatorroInputStage {
    public:
        struct OutputSample {
            float x{ 0.0f }, earlyReflections{ 0.0f };
        };
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        [[nodiscard]] OutputSample processSample(float in) noexcept;
        void setBandwidth(float newBandwidth) noexcept {
            m_bandwidth = newBandwidth;
            m_bandwidthFilter.setCoeff(1 - newBandwidth);
        }

        void setPreDelaySeconds(float newPreDelaySeconds) noexcept {
            m_preDelaySeconds = newPreDelaySeconds;
        }

        void setEarlyReflectionsLevel(float newEarlyReflectionsLevel) noexcept {
            m_earlyReflectionsLevel = newEarlyReflectionsLevel;
        }

        void setInputDiffusion1(float newCoeff) noexcept {
            m_inputDiffuserCoeffs[0] = m_inputDiffuserCoeffs[1] = newCoeff;
            m_inputDiffusers[0].setCoeff(newCoeff);
            m_inputDiffusers[1].setCoeff(newCoeff);
        }

        void setInputDiffusion2(float newCoeff) noexcept {
            m_inputDiffuserCoeffs[2] = m_inputDiffuserCoeffs[3] = newCoeff;
            m_inputDiffusers[2].setCoeff(newCoeff);
            m_inputDiffusers[3].setCoeff(newCoeff);
        }
    private:
        float m_sampleRate{ 44100 };
        float m_bandwidth{ 0.7f }, m_preDelaySeconds{ 0.0f }, m_earlyReflectionsLevel{ 0.5f };
        const std::array<float, 4> m_inputDiffuserTimes = {
                1e-5,
                3.5953e-3f,
                1.2735e-2f,
                9.3075e-3f
        };
        std::array<float, 4> m_inputDiffuserCoeffs = {
                0.75f, 0.75f, 0.625f, 0.625f
        };
        juce::dsp::DelayLine<float> m_preDelayLine;
        SDSP::Filters::SinglePoleLowpass m_bandwidthFilter;
        std::array<SDSP::Filters::APF, 4> m_inputDiffusers;
    };

} // Toro

