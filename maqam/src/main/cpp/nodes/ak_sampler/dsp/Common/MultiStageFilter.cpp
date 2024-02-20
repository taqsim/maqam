//
//  MultiStageFilter.cpp
//  AudioKit Core
//
//  Created by Shane Dunne
//  Copyright © 2018 AudioKit and Apple.
//
// MultiStageFilter implements a simple digital low-pass filter with dynamically
// adjustable cutoff frequency and resonance.

#include "MultiStageFilter.hpp"
#include "FunctionTable.hpp"
#include <math.h>

namespace AudioKitCore
{
    MultiStageFilter::MultiStageFilter()
    {
        for (int i=0; i < maxStages; i++) stage[i].init(44100.0);
        stages = 0;
    }
    
    void MultiStageFilter::init(double sampleRateHz)
    {
        for (int i=0; i < maxStages; i++) stage[i].init(sampleRateHz);
        stages = 0;
    }

    void MultiStageFilter::updateSampleRate(double sampleRateHz)
    {
        for (int i = 0; i < maxStages; i++) stage[i].updateSampleRate(sampleRateHz);
    }

    void MultiStageFilter::setStages(int nStages)
    {
        if (nStages < 0) nStages = 0;
        if (nStages > maxStages) nStages = maxStages;
        stages = nStages;

        for (int i=1; i < stages; i++)
            stage[i].setParams(stage[0].mLastCutoffHz, stage[0].mLastResLinear);
    }
    
    void MultiStageFilter::setParams(double newCutoffHz, double newResLinear)
    {
        for (int i=0; i < stages; i++) stage[i].setParams(newCutoffHz, newResLinear);
    }
    
    float MultiStageFilter::process(float sample)
    {
        for (int i=0; i < stages; i++)
            sample = stage[i].process(sample);
        return sample;
    }

}
