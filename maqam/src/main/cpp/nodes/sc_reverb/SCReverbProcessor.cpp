//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include "SCReverbProcessor.h"

#include "nodes/AudioProcessorHelpers.h"

using namespace juce;
using namespace maqam;

SCReverbProcessor::SCReverbProcessor() noexcept
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                                      .withOutput("Output", AudioChannelSet::stereo(), true))
    , mParameters (*this, nullptr, "SC Reverb", createParameterLayout())
    , mDspLibraryData(nullptr)
    , mDsp(nullptr)
{
    AudioProcessor::addListener(this);
}

void SCReverbProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (mDsp != nullptr) {
        return;
    }

    sp_create(&mDspLibraryData);
    sp_revsc_create(&mDsp);
    sp_revsc_init(mDspLibraryData, mDsp);

    dsp::ProcessSpec spec = {
        .sampleRate = sampleRate,
        .maximumBlockSize = static_cast<uint32>(samplesPerBlock),
        .numChannels = 2
    };

    mHiPassFilterDsp.prepare(spec);

    mDryWetMixer.prepare(spec);
    mDryWetMixer.setMixingRule(dsp::DryWetMixingRule::sin6dB);
}

void SCReverbProcessor::releaseResources()
{
    if (mDsp != nullptr) {
        sp_revsc_destroy(&mDsp);
    }

    if (mDspLibraryData != nullptr) {
        sp_destroy(&mDspLibraryData);
    }
}

void SCReverbProcessor::processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
    // According to JUCE docs, the default implementation of AudioProcessor::processBlockBypassed()
    // will pass-through any incoming audio, but code is clearing the audio buffer instead.
    // See juce_AudioProcessor.cpp : buffer.clear (ch, 0, buffer.getNumSamples());
}

void SCReverbProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    // juce::AudioProcessorGraph logic never calls AudioProcessor::processBlockBypassed() when
    // AudioProcessor::getBypassParameter() returns a non-null parameter. See static void process()
    if (getBypassParameter()->getValue() != 0) {
        processBlockBypassed(buffer, midi);
        return;
    }

    // Copies the dry path samples into an internal delay line
    dsp::AudioBlock<float> block(buffer);
    mDryWetMixer.pushDrySamples(block);

    mDsp->feedback = getParameterValue(kParameterFeedback);
    mDsp->lpfreq = getParameterValue(kParameterLowPassFilterCutoff);

    // Not clear why Soundpipe expects writable input parameters
    const int numSamples = buffer.getNumSamples();
    float ki0, ki1;

    for (int i = 0; i < numSamples; i++) {
        ki0 = buffer.getSample(0, i);
        ki1 = buffer.getSample(1, i);

        sp_revsc_compute(mDspLibraryData, mDsp, &ki0, &ki1,
                         buffer.getWritePointer(0, i), buffer.getWritePointer(1, i));
    }

    mHiPassFilterDsp.process(dsp::ProcessContextReplacing<float>(block));

    mDryWetMixer.setWetMixProportion(getParameterValue(kParameterMix));
    mDryWetMixer.mixWetSamples(block);
}

void SCReverbProcessor::audioProcessorParameterChanged(AudioProcessor* processor,
                                                       int parameterIndex, float newValue)
{
    // https://forum.juce.com/t/processorduplicator-how-to/24361/3
    *mHiPassFilterDsp.state = dsp::IIR::ArrayCoefficients<float>::makeHighPass(
            getSampleRate(), getParameterValue(kParameterHighPassFilterCutoff));
}

AudioProcessorValueTreeState::ParameterLayout
SCReverbProcessor::createParameterLayout() noexcept
{
    return {
        createParameterBypass(kParameterBypass),
        createParameterMix(kParameterMix),
        createFloatParameter(
                kParameterFeedback,
                "Feedback", "%",
                /*min*/0, /*max*/1.f, /*def*/0.5f,
                [](float v, int _) { return String(static_cast<int>(100.f * v)); }
        ),
        createFloatParameter(
                kParameterLowPassFilterCutoff,
                "Low pass filter cutoff frequency", "Hz",
                /*min*/12.f, /*max*/20000.f, /*def*/4000.f
        ),
        createFloatParameter(
                kParameterHighPassFilterCutoff,
                "High pass filter cutoff frequency", "Hz",
                /*min*/12.f, /*max*/20000.f, /*def*/4000.f
        )
    };
}