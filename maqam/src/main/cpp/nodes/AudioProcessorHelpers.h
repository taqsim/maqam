//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef AUDIO_PROCESSOR_HELPERS
#define AUDIO_PROCESSOR_HELPERS

#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

namespace maqam {

static std::unique_ptr<juce::AudioParameterFloat>
createFloatParameter(
        const juce::ParameterID& parameterID,
        const juce::String& parameterName,
        const juce::String& parameterLabel,
        float rangeStart,
        float rangeEnd,
        float defaultValue,
        juce::AudioParameterFloatAttributes::StringFromValue stringFromValue = nullptr)
{
    return std::make_unique<juce::AudioParameterFloat>(
            parameterID,
            parameterName,
            juce::NormalisableRange<float>(rangeStart, rangeEnd),
            defaultValue,
            juce::AudioParameterFloatAttributes()
                    .withLabel(parameterLabel)
                    .withStringFromValueFunction(std::move(stringFromValue))
    );
}

static std::unique_ptr<juce::AudioParameterFloat>
createParameterBypass(const juce::ParameterID& parameterID)
{
    return createFloatParameter(
            parameterID,
            "Bypass", "", /*min*/0, /*max*/1.f, /*def*/0,
            [](float v, int _) { return juce::String(v == 0 ? "off" : "on"); }
    );
}

static std::unique_ptr<juce::AudioParameterFloat>
createParameterMix(const juce::ParameterID& parameterID)
{
    return createFloatParameter(
            parameterID,
            "Mix", "%", /*min*/0, /*max*/1.f, /*def*/0.5f,
            [](float v, int _) { return juce::String(static_cast<int>(100.f * v)); }
    );
}

} // namespace

#endif // AUDIO_PROCESSOR_HELPERS
