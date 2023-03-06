#pragma once
#include <SDSP/Macros.h>
#include <SDSP/Filters/APF.h>
#include <SDSP/Filters/UtilityFilters.h>
// The room reverb looks just like Griesinger 'plate' reverb covered by the AES paper 'Effect Design - Part 1, Reverberator and other filters'.
// Except that there is one additional stage of diffusion and one additional stage of delay through each leg of the tank.
// Also, the placement of the damping low-pass filters are somewhat different.
// The input diffusion is done differently. Rather than having four cascaded input diffusors,
// there are two pairs of cascaded diffusors, each feeding one leg of the tank.
// Both are fed from the predelay line. The output tap summation uses a lot more taps,
// including some in the predelay.

namespace Toro {
    class LexiconInputStage {
    public:

        SDSP_INLINE void setPreDelaySeconds(float newPreDelaySeconds) noexcept {
            m_preDelaySeconds = newPreDelaySeconds;
        }

        SDSP_INLINE void setBandwidth(float newBandwidth) noexcept {
            m_bandwidth = newBandwidth;
            m_bandwidthFilter.setCoeff(1 - newBandwidth);
        }

        SDSP_INLINE void setEarlyReflectionsLevel(float newEarlyReflectionsLevel) noexcept {
            m_earlyReflectionLevel = newEarlyReflectionsLevel;
        }

        SDSP_INLINE void setDiffusion1(float newCoeff) noexcept {
            m_leftCoeff = newCoeff;
            m_leftLegDiffusers[0].setCoeff(newCoeff);
            m_leftLegDiffusers[1].setCoeff(newCoeff);
        }

        SDSP_INLINE void setDiffusion2(float newCoeff) noexcept {
            m_rightCoeff = newCoeff;
            m_rightLegDiffusers[0].setCoeff(newCoeff);
            m_rightLegDiffusers[1].setCoeff(newCoeff);
        }

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
            m_sampleRate = static_cast<float>(sampleRate);
            m_bandwidthFilter.setCoeff(m_bandwidth);
            m_preDelayLine.prepare({sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 2});
            m_preDelayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.5));
            for(auto i = 0; i < 2; ++i) {
                m_leftLegDiffusers[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
                m_leftLegDiffusers[i].setCoeff(m_leftCoeff);
                m_rightLegDiffusers[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
                m_rightLegDiffusers[i].setCoeff(m_rightCoeff);
            }
        }
        SDSP_INLINE std::tuple<float, float, float, float> processSample(float x) noexcept {
            auto out = m_preDelayLine.popSample(0, m_preDelaySeconds * m_sampleRate);
            m_preDelayLine.pushSample(0, x);
            out = m_bandwidthFilter.processSample(out);
            auto xL = out;
            auto xR = out;
            auto erLeft = 0.0f, erRight = 0.0f;
            for(auto i = 0; i < 2; ++i) {
                xL = m_leftLegDiffusers[static_cast<size_t>(i)].processSample(xL);
                xR = m_rightLegDiffusers[static_cast<size_t>(i)].processSample(xR);
                erLeft += (xL / std::powf(2, static_cast<float>(m_leftLegDiffusers.size() - i)));
                erRight += (xR / std::powf(2, static_cast<float>(m_leftLegDiffusers.size() - i)));
            }
            return {xL, xR, erLeft * m_earlyReflectionLevel, erRight * m_earlyReflectionLevel};
        }
    private:
        float m_sampleRate{ 44100 };
        float m_preDelaySeconds{ 0.0f };
        float m_earlyReflectionLevel{ 0.5f };
        juce::dsp::DelayLine<float> m_preDelayLine;
        float m_bandwidth{ 0.5f };
        SDSP::Filters::SinglePoleLowpass m_bandwidthFilter;
        float m_leftCoeff{ 0.75f }, m_rightCoeff{ 0.625f };
        std::array<SDSP::Filters::APF, 2> m_leftLegDiffusers, m_rightLegDiffusers;
    };


} // Toro

