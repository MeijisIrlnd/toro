//
// Created by Syl on 05/03/2023.
//
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <SDSP/Macros.h>
#include <SDSP/Filters/APF.h>
#include <SDSP/Filters/UtilityFilters.h>
namespace Toro {
    class DatorroSimple {
    public:
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        void getNextAudioBlock(juce::AudioBuffer<float>& buffer);
        SDSP_INLINE void setPreDelaySeconds(float newPreDelaySeconds) noexcept {
            m_inputStage.setPreDelaySeconds(newPreDelaySeconds);
        }
        SDSP_INLINE void setBandwidth(float newBandwidth) noexcept {
            m_inputStage.setBandwidth(newBandwidth);
        }
        SDSP_INLINE void setEarlyReflectionsLevel(float newEarlyReflectionsLevel) noexcept {
            m_inputStage.setEarlyReflectionsLevel(newEarlyReflectionsLevel);
        }
        SDSP_INLINE void setInputDiffusion1(float newCoeff) noexcept {
            m_inputStage.setInputDiffusion1(newCoeff);
        }
        SDSP_INLINE void setInputDiffusion2(float newCoeff) noexcept {
            m_inputStage.setInputDiffusion2(newCoeff);
        }
        SDSP_INLINE void setDecay(float newDecay) noexcept {
            m_tankStage.setDecay(newDecay);
        }
        SDSP_INLINE void setDamping(float newDamping) noexcept {
            m_tankStage.setDamping(newDamping);
        }
        SDSP_INLINE void setExcursion(float excursionTimeMS) noexcept {
            m_tankStage.setExcursion(excursionTimeMS);
        }
        SDSP_INLINE void setDecayDiffusion1(float newCoeff) noexcept {
            m_tankStage.setDecayDiffusion1(newCoeff);
        }
        SDSP_INLINE void setDecayDiffusion2(float newCoeff) noexcept {
            m_tankStage.setDecayDiffusion2(newCoeff);
        }
        SDSP_INLINE void setDryWet(float newDryWet) noexcept {
            m_dryWet = newDryWet;
        }
    private:
        class Input {
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
        } m_inputStage;
        class Tank {
        public:
            void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
            [[nodiscard]] std::tuple<float, float> processSample(float in);
            void setDecay(float newDecay) noexcept {
                m_decay = newDecay;
            }

            void setDecayDiffusion1(float newDiffusion) noexcept {
                m_decayDiffusion1 = newDiffusion;
                m_leftDecayDiffuser1.setCoeff(-newDiffusion);
                m_rightDecayDiffuser1.setCoeff(-newDiffusion);
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
            SDSP::Filters::SinglePoleLowpass m_leftDampingFilter;
            juce::dsp::DelayLine<float> m_leftDelay1, m_leftDelay2;
            SDSP::Filters::ModulatedAPF m_leftDecayDiffuser1;
            SDSP::Filters::APF m_leftDecayDiffuser2;

            SDSP::Filters::SinglePoleLowpass m_rightDampingFilter;
            juce::dsp::DelayLine<float> m_rightDelay1, m_rightDelay2;
            SDSP::Filters::ModulatedAPF m_rightDecayDiffuser1;
            SDSP::Filters::APF m_rightDecayDiffuser2;
        } m_tankStage;
        float m_dryWet{ 0.5f };
        juce::dsp::DryWetMixer<float> m_dryWetMixer;
    };

}