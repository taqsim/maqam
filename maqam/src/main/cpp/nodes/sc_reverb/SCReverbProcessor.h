//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef SC_REVERB_PROCESSOR_H
#define SC_REVERB_PROCESSOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

extern "C" {
#include "soundpipe.h"
}

namespace maqam {

class SCReverbProcessor : public juce::AudioProcessor
                        , private juce::AudioProcessorListener
{
public:
    static constexpr const char* kParameterBypass               = "bypass";
    static constexpr const char* kParameterMix                  = "mix";
    static constexpr const char* kParameterFeedback             = "feedback";
    static constexpr const char* kParameterLowPassFilterCutoff  = "lp_filter_cutoff";
    static constexpr const char* kParameterHighPassFilterCutoff = "hp_filter_cutoff";

    SCReverbProcessor() noexcept;
    virtual ~SCReverbProcessor() {};

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioProcessorParameter* getBypassParameter() const override
    {
        return mParameters.getParameter(kParameterBypass);
    }

    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    const juce::String getName() const override { return "SC Reverb"; }

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
    // juce::AudioProcessorListener
    void audioProcessorParameterChanged(AudioProcessor* processor, int parameterIndex, float newValue) override;
    void audioProcessorChanged(AudioProcessor* processor, const ChangeDetails& details) override {}

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout() noexcept;

    inline float getParameterValue(juce::StringRef parameterID) noexcept
    {
        return reinterpret_cast<juce::AudioParameterFloat*>(mParameters.getParameter(parameterID))
            ->get();
    }

    juce::AudioProcessorValueTreeState mParameters;

    sp_data*  mDspLibraryData;
    sp_revsc* mDsp;

    using FloatCoefficients = juce::dsp::IIR::Coefficients<float>;
    using MonoFilter        = juce::dsp::IIR::Filter<float>;
    using StereoFilter      = juce::dsp::ProcessorDuplicator<MonoFilter, FloatCoefficients>;

    StereoFilter mHiPassFilterDsp;

    juce::dsp::DryWetMixer<float> mDryWetMixer;

};

} // maqam

#endif //SC_REVERB_PROCESSOR_H
