//
// Created by Syl on 22/02/2023.
//
#pragma once
#include <juce_dsp/juce_dsp.h>
namespace Toro
{
    class APF {
    public:
        [[maybe_unused]] float tap(const int index) {
            return m_delayLine.popSample(0, static_cast<float>(index), false);
        }
        virtual void setDelayTimeSamples(int newDelayTimeSamples) {
            m_delayTimeSamples = newDelayTimeSamples;
            if(m_hasBeenPrepared) {
                m_delayLine.setDelay(static_cast<float>(m_delayTimeSamples));
            }
        }

        void setCoeff(float newCoeff) {
            m_coeff = newCoeff;
            if(m_hasBeenPrepared) {
                m_smoothedCoeff.setTargetValue(m_coeff);
            }
        }

        virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
            m_sampleRate = sampleRate;
            m_hasBeenPrepared = true;
            juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1};
            m_delayLine.prepare(spec);
            // Allow pad for delay modulation
            m_delayLine.setMaximumDelayInSamples(m_delayTimeSamples + 32);
            m_delayLine.setDelay(static_cast<float>(m_delayTimeSamples));
            m_smoothedCoeff.reset(sampleRate, 0.1);
            m_smoothedCoeff.setCurrentAndTargetValue(m_coeff);
        }

        virtual float processSample(float in) = 0;

        void releaseResources()
        {

        }

    protected:
        double m_sampleRate{ 44100 };
        float m_prev{ 0.0f };
        bool m_hasBeenPrepared{ false };
        int m_delayTimeSamples{ 0 };
        juce::dsp::DelayLine<float> m_delayLine;
        float m_coeff{ 0.5f };
        juce::SmoothedValue<float> m_smoothedCoeff;
    };

    class TypeAAPF : public APF {
    public:
        float processSample(float in) override
        {
            auto delayed = m_delayLine.popSample(0);
            in += (delayed * -m_coeff);
            m_delayLine.pushSample(0, in);
            auto feedforward = in * m_coeff;
            return (delayed + feedforward);
        }
    };

    class TypeBAPF : public APF {
    public:

        void setLfoRate(const float newRate) noexcept {
            m_lfo.setFrequency(newRate);
        }

        void setExcursionSeconds(float newExcursionSeconds) {
            m_excursionSeconds = newExcursionSeconds;
        }

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
        {
            APF::prepareToPlay(samplesPerBlockExpected, sampleRate);
            juce::dsp::ProcessSpec spec{sampleRate / static_cast<double>(m_updateRate), static_cast<juce::uint32>(samplesPerBlockExpected), 1};
            m_lfo.prepare(spec);
            m_lfo.initialise([](float x) { return std::sinf(x); });
            m_smoothedDelayTimeSamples.reset(sampleRate, 0.1);
            m_smoothedDelayTimeSamples.setCurrentAndTargetValue(static_cast<float>(m_delayTimeSamples));
        }

        float processSample(float in) override {
            if(m_samplesUntilUpdate == 0) {
                auto lfoSample = m_lfo.processSample(0);
                auto excursionSamples = static_cast<float>(m_excursionSeconds * m_sampleRate);
                lfoSample = juce::jmap<float>(lfoSample, -1, 1, -excursionSamples, excursionSamples);
                auto newDelayTime = (static_cast<float>(m_delayTimeSamples) + lfoSample);
                m_smoothedDelayTimeSamples.setTargetValue(newDelayTime);
                m_samplesUntilUpdate = m_updateRate;
            }
            --m_samplesUntilUpdate;
            auto currentDelay = m_smoothedDelayTimeSamples.getNextValue();
            auto delayed = m_delayLine.popSample(0, currentDelay);
            in += (delayed * m_coeff);
            m_delayLine.pushSample(0, in);
            auto feedforward = in * -m_coeff;
            return (delayed + feedforward);
        }
    private:
        const int m_updateRate{ 100 };
        int m_samplesUntilUpdate{ 0 };
        float m_excursionSeconds{ 0.0f };
        juce::dsp::Oscillator<float> m_lfo;
        juce::SmoothedValue<float> m_smoothedDelayTimeSamples;
    };
}