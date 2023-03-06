//
// Created by Syl on 05/03/2023.
//
#pragma once
#include <algorithms/ReverbBase.h>
#include <SDSP/Macros.h>
#include <utils/LexiconInputStage.h>
#include <utils/LexiconTank.h>
namespace Toro {

    class Lexicon : public ReverbBase {
    public:
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(juce::AudioBuffer<float>& buffer) override;
        SDSP_INLINE void setPreDelaySeconds(float newPreDelaySeconds) noexcept override {
            m_inputStage.setPreDelaySeconds(newPreDelaySeconds);
        }
        SDSP_INLINE void setBandwidth(float newBandwidth) noexcept override {
            m_inputStage.setBandwidth(newBandwidth);
        }
        SDSP_INLINE void setEarlyReflectionsLevel(float newEarlyReflectionsLevel) noexcept override {
            m_inputStage.setEarlyReflectionsLevel(newEarlyReflectionsLevel);
        }
        SDSP_INLINE void setInputDiffusion1(float newCoeff) noexcept override {
            m_inputStage.setDiffusion1(newCoeff);
        }
        SDSP_INLINE void setInputDiffusion2(float newCoeff) noexcept override {
            m_inputStage.setDiffusion2(newCoeff);
        }
        SDSP_INLINE void setDecay(float newDecay) noexcept override {
            m_tankStage.setDecay(newDecay);
        }
        SDSP_INLINE void setDamping(float newDamping) noexcept override {
            m_tankStage.setDamping(newDamping);
        }
        SDSP_INLINE void setExcursion(float excursionTimeMS) noexcept override {
            m_tankStage.setExcursion(excursionTimeMS);
        }
        SDSP_INLINE void setDecayDiffusion1(float newCoeff) noexcept override {
            m_tankStage.setDecayDiffusion1(newCoeff);
        }
        SDSP_INLINE void setDecayDiffusion2(float newCoeff) noexcept override {
            m_tankStage.setDecayDiffusion2(newCoeff);
        }
        SDSP_INLINE void setDryWet(float newDryWet) noexcept override {
            m_dryWet = newDryWet;
        }
    private:
        LexiconInputStage m_inputStage;
        LexiconTank m_tankStage;
        float m_dryWet{ 0.5f };
        juce::dsp::DryWetMixer<float> m_dryWetMixer;
    };

} // Toro

