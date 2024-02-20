//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef FILTER_PROCESSOR_H
#define FILTER_PROCESSOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

extern "C" {
#include "soundpipe.h"
}

namespace maqam {

class FilterProcessor : public juce::AudioProcessor
{
public:
    static constexpr const char* kParameterBypass       = "bypass";
    static constexpr const char* kParameterMix          = "mix";
    static constexpr const char* kParameterCutoff       = "cutoff";
    static constexpr const char* kParameterResonance    = "resonance";
    static constexpr const char* kParameterLFOAmplitude = "lfo_amplitude";
    static constexpr const char* kParameterLFORate      = "lfo_rate";


    FilterProcessor() noexcept;
    virtual ~FilterProcessor() {};

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioProcessorParameter* getBypassParameter() const override
    {
        return mParameters.getParameter(kParameterBypass);
    }

    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    const juce::String getName() const override { return "Filter"; }

    bool   acceptsMidi() const override { return false; }
    bool   producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override { return 0; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return ""; }
    void changeProgramName(int index, const juce::String& newName) override {}

    void getStateInformation(juce::MemoryBlock& destData) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout() noexcept;

    inline float getParameterValue(juce::StringRef parameterID) noexcept
    {
        return reinterpret_cast<juce::AudioParameterFloat*>(mParameters.getParameter(parameterID))
            ->get();
    }

    juce::AudioProcessorValueTreeState mParameters;

    sp_data*       mDspLibraryData;
    sp_moogladder* mDsp;

    float mLfoPhase;

    juce::dsp::DryWetMixer<float> mDryWetMixer;

};

} // maqam

#endif //FILTER_PROCESSOR_H
