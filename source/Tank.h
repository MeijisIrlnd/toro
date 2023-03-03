//
// Created by Syl on 26/02/2023.
//
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <APF.h>
#include <SinglePoleLowpass.h>
namespace Toro {
    class Tank
    {
    public:
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        std::tuple<float, float> processSample(float in);
        void setDecay(float newDecay) noexcept {
            m_decay = newDecay;
        }

        void setDecayDiffusion1(float newDiffusion) noexcept {
            m_decayDiffusion1 = newDiffusion;
            m_leftDecayDiffuser1.setCoeff(newDiffusion);
            m_rightDecayDiffuser1.setCoeff(newDiffusion);
        }

        void setDecayDiffusion2(float newDiffusion) noexcept {
            m_decayDiffusion2 = newDiffusion;
            m_leftDecayDiffuser2.setCoeff(newDiffusion);
            m_rightDecayDiffuser2.setCoeff(newDiffusion);
        }

        void setDamping(float newDamping) noexcept {
            m_damping = newDamping;
            m_leftDampingFilter.setCoeff(newDamping);
            m_rightDampingFilter.setCoeff(newDamping);
        }

        void setExcursion(float excursionTimeMS){
            m_excursionTimeMS = excursionTimeMS;
            m_leftDecayDiffuser1.setExcursionSeconds(m_excursionTimeMS / 1000.0f);
            m_rightDecayDiffuser1.setExcursionSeconds(m_excursionTimeMS / 1000.0f);
        }
    private:
        std::tuple<float, float> tap();
        struct TapIndices {
            void prepare(double sampleRate) {
                // use original taps at 29761, and rescale..
                const int sourceSr = 29761;
                const int sr = static_cast<int>(sampleRate);
                tapIndicesL[0] = (266 / sourceSr) * sr;
                tapIndicesL[1] = (2974 / sourceSr) * sr;
                tapIndicesL[2] = (1913 / sourceSr) * sr;
                tapIndicesL[3] = (1996 / sourceSr) * sr;
                tapIndicesL[4] = (1990 / sourceSr) * sr;
                tapIndicesL[5] = (187 / sourceSr) * sr;
                tapIndicesL[6] = (1066 / sourceSr) * sr;

                tapIndicesR[0] = (353 / sourceSr) * sr;
                tapIndicesR[1] = (3627 / sourceSr) * sr;
                tapIndicesR[2] = (1228 / sourceSr) * sr;
                tapIndicesR[3] = (2673 / sourceSr) * sr;
                tapIndicesR[4] = (2111 / sourceSr) * sr;
                tapIndicesR[5] = (335 / sourceSr) * sr;
                tapIndicesR[6] = (121 / sourceSr) * sr;
            }

            std::array<int, 7> tapIndicesL, tapIndicesR;

        } m_tapIndices;
        float m_decayDiffusion1{ 0.7f }, m_decayDiffusion2{0.5f };
        //float m_damping{ 5e-4f };
        float m_damping{ 0.0005f };
        float m_decay{ 0.5f };
        float m_prevL{ 0.0f }, m_prevR{ 0.0f };
        float m_excursionTimeMS{ 0.53676f };
        SinglePoleLowpass m_leftDampingFilter;
        juce::dsp::DelayLine<float> m_leftDelay1, m_leftDelay2;
        TypeBAPF m_leftDecayDiffuser1;
        TypeAAPF m_leftDecayDiffuser2;

        SinglePoleLowpass m_rightDampingFilter;
        juce::dsp::DelayLine<float> m_rightDelay1, m_rightDelay2;
        TypeBAPF m_rightDecayDiffuser1;
        TypeAAPF m_rightDecayDiffuser2;
    };
}