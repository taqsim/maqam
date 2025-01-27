//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#pragma once

#include <cmath>

class SineWave
{
public:
    constexpr static float PI_2 = 2.f * M_PI;
    
    SineWave(double samplerate, float frequency) noexcept
        : mSamplerate(static_cast<float>(samplerate))
        , mFrequency(frequency)
        , mPhase(0)
    {}
    
    float getSample() noexcept
    {
        const float k = ::sin(mPhase);
        const float radiansPerSample = PI_2 * mFrequency / mSamplerate;
        
        mPhase += radiansPerSample;
        
        if (mPhase > PI_2) {
            mPhase -= PI_2;
        }
        
        return k;
    }
    
    void setSamplerate(double samplerate)
    {
        mSamplerate = samplerate;
    }
    
    void setFrequency(float frequency)
    {
        mFrequency = frequency;
    }
    
private:
    float mSamplerate;
    float mFrequency;
    float mPhase;

};
