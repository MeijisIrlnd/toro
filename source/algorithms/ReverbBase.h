//
// Created by Syl on 06/03/2023.
//

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
namespace Toro
{
    class ReverbBase
    {
    public:
        virtual void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/) = 0;
        virtual void getNextAudioBlock(juce::AudioBuffer<float>& /*buffer*/) = 0;
        virtual void setPreDelaySeconds(float newPreDelaySeconds) noexcept = 0;
    
        virtual void setBandwidth(float newBandwidth) noexcept = 0;
        virtual void setEarlyReflectionsLevel(float newEarlyReflectionsLevel) noexcept = 0;
        virtual void setInputDiffusion1(float newCoeff) noexcept = 0;
        virtual void setInputDiffusion2(float newCoeff) noexcept = 0;
        virtual void setDecay(float newDecay) noexcept = 0;
        virtual void setDamping(float newDamping) noexcept = 0;
        virtual void setExcursion(float excursionTimeMS) noexcept = 0;
        virtual void setDecayDiffusion1(float newCoeff) noexcept = 0;
        virtual void setDecayDiffusion2(float newCoeff) noexcept = 0;
        virtual void setDryWet(float newDryWet) noexcept = 0;
    };
}
