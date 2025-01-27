//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "SineWave.h"

namespace maqam {

    class SineWaveAudioProcessor : public juce::AudioProcessor
    {
    public:
        static constexpr const char* kParameterIdSineWaveFrequency = "sine_wave_frequency";

        SineWaveAudioProcessor()
            : AudioProcessor (BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                             .withOutput("Output", juce::AudioChannelSet::stereo()))
            , mParameters (
                /*processorToConnectTo=*/*this,
                /*undoManagerToUse=*/nullptr,
                /*valueTreeType=*/juce::Identifier("SineWaveAudioProcessor"),
                /*parameterLayout=*/createParameterLayout()
            )
        {}

        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            juce::AudioProcessorValueTreeState::ParameterLayout params;

            params.add(std::make_unique<juce::AudioParameterFloat>(
                /*parameterID=*/kParameterIdSineWaveFrequency,
                /*parameterName=*/"Frequency",
                /*minValue=*/220.f,
                /*maxValue=*/880.f,
                /*defaultValue=*/440.f
            ));

            return params;
        }

        void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
        {
            const float f = mParameters.getParameterAsValue(kParameterIdSineWaveFrequency).getValue();
            mSineWave = std::make_unique<SineWave>(sampleRate, f);
        }

        void releaseResources() override {}

        void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages) override
        {
            const float f = reinterpret_cast<juce::AudioParameterFloat*>(
                    mParameters.getParameter(kParameterIdSineWaveFrequency))->get();
            mSineWave->setFrequency(f);

            const int numSamples = buffer.getNumSamples();
            const int numChannels = buffer.getNumChannels();

            float k;

            for (int i = 0; i < numSamples; ++i) {
                k = mSineWave->getSample();

                for (int j = 0; j < numChannels; ++j) {
                    buffer.setSample(j, i, k);
                }
            }
        }

        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }

        const juce::String getName() const override { return "Test"; }

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
        juce::AudioProcessorValueTreeState mParameters;
        std::unique_ptr<SineWave> mSineWave;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWaveAudioProcessor)
    };

} // maqam
