//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include "DelayProcessor.h"

#include "nodes/AudioProcessorHelpers.h"

using namespace juce;
using namespace maqam;

DelayProcessor::DelayProcessor() noexcept
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                                      .withOutput("Output", AudioChannelSet::stereo(), true))
    , mParameters (*this, nullptr, "Delay", createParameterLayout())
{}

void DelayProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec = {
        .sampleRate = sampleRate,
        .maximumBlockSize = static_cast<uint32>(samplesPerBlock),
        .numChannels = 2
    };

    const int maxDelay = static_cast<int>(static_cast<float>(sampleRate)
            * mParameters.getParameterRange(kParameterTime).getRange().getEnd());
    mDelayDsp.setMaximumDelayInSamples(maxDelay);
    mDelayDsp.prepare(spec);

    mDryWetMixer.setMixingRule(dsp::DryWetMixingRule::sin6dB);
    mDryWetMixer.prepare(spec);
}

void DelayProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    if (getBypassParameter()->getValue() != 0) {
        processBlockBypassed(buffer, midi);
        return;
    }

    dsp::AudioBlock<float> block(buffer);
    mDryWetMixer.pushDrySamples(block);

    const size_t numSamples = block.getNumSamples();
    const size_t numChannels = block.getNumChannels();

    const float feedback = getParameterValue(kParameterFeedback);
    const float delayInSamples = getParameterValue(kParameterTime)
            * static_cast<float>(getSampleRate());

    for (size_t ch = 0; ch < numChannels; ++ch) {
        float* samples = block.getChannelPointer(ch);

        for (size_t i = 0; i < numSamples; ++i) {
            samples[i] += feedback * mDelayDsp.popSample(static_cast<int>(ch), delayInSamples);
            mDelayDsp.pushSample(static_cast<int>(ch), samples[i]);
        }
    }

    mDryWetMixer.setWetMixProportion(getParameterValue(kParameterMix));
    mDryWetMixer.mixWetSamples(block);
}

AudioProcessorValueTreeState::ParameterLayout
DelayProcessor::createParameterLayout() noexcept
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
                kParameterTime,
                "Time", "sec",
                /*min*/0.01f, /*max*/1.f, /*def*/0.5f
        ),
    };
}
