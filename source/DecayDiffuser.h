//
// Created by Syl on 26/02/2023.
//
#pragma once
#include <APF.h>
namespace Toro
{
    class DecayDiffuser
    {
    public:
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
        {
            m_apf.setDelayTimeSamples(static_cast<int>(2.2579e-2 * sampleRate));
            m_apf.prepareToPlay(samplesPerBlockExpected, sampleRate);
            m_apf.setLfoRate(1.0f);
            // 16 @ 2sr = 29761
            m_apf.setExcursionSeconds(5.3761e-4f);
        }

        float processSample(float in) noexcept {
            return m_apf.processSample(in);
        }

    private:
        TypeBAPF m_apf;
    };
}