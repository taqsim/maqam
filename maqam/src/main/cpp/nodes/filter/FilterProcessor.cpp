//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include "FilterProcessor.h"

#include "nodes/AudioProcessorHelpers.h"

using namespace juce;
using namespace maqam;

constexpr static float PI_2 = 2.f * M_PI;

FilterProcessor::FilterProcessor() noexcept
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                                      .withOutput("Output", AudioChannelSet::stereo(), true))
    , mParameters (*this, nullptr, "Filter", createParameterLayout())
    , mDspLibraryData(nullptr)
    , mDsp(nullptr)
    , mLfoPhase(0)
{}

void FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (mDsp != nullptr) {
        return;
    }

    sp_create(&mDspLibraryData);
    sp_moogladder_create(&mDsp);
    sp_moogladder_init(mDspLibraryData, mDsp);

    dsp::ProcessSpec spec = {
        .sampleRate = sampleRate,
        .maximumBlockSize = static_cast<uint32>(samplesPerBlock),
        .numChannels = 2
    };

    mDryWetMixer.prepare(spec);
    mDryWetMixer.setMixingRule(dsp::DryWetMixingRule::sin6dB);
}

void FilterProcessor::releaseResources()
{
    if (mDsp != nullptr) {
        sp_moogladder_destroy(&mDsp);
    }

    if (mDspLibraryData != nullptr) {
        sp_destroy(&mDspLibraryData);
    }
}

void FilterProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    if (getBypassParameter()->getValue() != 0) {
        processBlockBypassed(buffer, midi);
        return;
    }

    dsp::AudioBlock<float> block(buffer);
    mDryWetMixer.pushDrySamples(block);

    const float cutoff = getParameterValue(kParameterCutoff);
    const float modCutoff = getParameterValue(kParameterLFOAmplitude) * ::sin(mLfoPhase);

    mLfoPhase += PI_2 * getParameterValue(kParameterLFORate) / getSampleRate();
    if (mLfoPhase > PI_2) mLfoPhase -= PI_2;

    mDsp->freq = fmax(cutoff + modCutoff, 0);
    mDsp->res = getParameterValue(kParameterResonance);

    const int numSamples = buffer.getNumSamples();
    float ki0, ki1;

    for (int i = 0; i < numSamples; i++) {
        ki0 = buffer.getSample(0, i);
        sp_moogladder_compute(mDspLibraryData, mDsp, &ki0, buffer.getWritePointer(0, i));
        ki1 = buffer.getSample(1, i);
        sp_moogladder_compute(mDspLibraryData, mDsp, &ki1, buffer.getWritePointer(1, i));
    }

    mDryWetMixer.setWetMixProportion(getParameterValue(kParameterMix));
    mDryWetMixer.mixWetSamples(block);
}

AudioProcessorValueTreeState::ParameterLayout
FilterProcessor::createParameterLayout() noexcept
{
    return {
        createParameterBypass(kParameterBypass),
        createParameterMix(kParameterMix),
        createFloatParameter(
                kParameterCutoff,
                "Cutoff frequency", "Hz",
                /*min*/12.f, /*max*/20000.f, /*def*/1000.f
        ),
        createFloatParameter(
                kParameterResonance,
                "Resonance factor", "",
                /*min*/0.f, /*max*/1.f, /*def*/0.9f
        ),
        createFloatParameter(
                kParameterLFOAmplitude,
                "LFO Amplitude", "",
                /*min*/1.f, /*max*/10000.f, /*def*/1000.f
        ),
        createFloatParameter(
                kParameterLFORate,
                "LFO Rate", "Hz",
                /*min*/1.f, /*max*/1000.f, /*def*/10.f
        )
    };
}