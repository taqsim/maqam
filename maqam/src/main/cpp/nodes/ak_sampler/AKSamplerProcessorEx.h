//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef AKSAMPLER_PROCESSOR_EX_H
#define AKSAMPLER_PROCESSOR_EX_H

#include <array>
#include <atomic>
#include <filesystem>

#include <parser/Parser.h>
#include <parser/ParserListener.h>

#include "dsp/Plugin/JuceHeader.h"

#include "AKSamplerProcessor.h"

/**
 * This class extends AudioKit's AKSamplerProcessor:
 *
 *   - Expose all sampler parameters as JUCE audio processor parameters
 *   - Replace basic SFZ parsing code with the standard-compliant parser from Sfizz
 *   - Allow to optionally listen on a single MIDI channel
 *   - Avoid a race condition while loading sounds
 *   - Support 12-ET microtonal scales
 *   - Parameter for disabling ADSR envelope
 *
 */

namespace maqam {

class AKSamplerProcessorEx : public AKSamplerProcessor
                           , private juce::AudioProcessorListener
                           , private sfz::ParserListener
{
public:
    static constexpr int kMidiChannelOmni = 16; // out of range [0-15]

    // See PatchParams.h
    static constexpr const char* kParameterMainMasterLevel               = "main_master_level";
    static constexpr const char* kParameterMainPitchBendUpSemitones      = "main_pitchbend_up_semitones";
    static constexpr const char* kParameterMainPitchBendDownSemitones    = "main_pitchbend_down_semitones";
    static constexpr const char* kParameterMainAmpVelocitySensitivity    = "main_amp_velocity_sensitivity";
    static constexpr const char* kParameterMainFilterVelocitySensitivity = "main_filter_velocity_sens"; // AAX limit 31 chars
    static constexpr const char* kParameterOsc1PitchOffsetSemitones      = "osc1_pitch_offset_semitones";
    static constexpr const char* kParameterOsc1DetuneOffsetCents         = "osc1_detune_offset_cents";
    static constexpr const char* kParameterFilterStages                  = "filter_stages";
    static constexpr const char* kParameterFilterCutoff                  = "filter_cutoff";
    static constexpr const char* kParameterFilterResonance               = "filter_resonance";
    static constexpr const char* kParameterFilterEnvAmount               = "filter_env_amount";
    static constexpr const char* kParameterAmpEGBypass                   = "ampeg_bypass";
    static constexpr const char* kParameterAmpEGAttackTimeSeconds        = "ampeg_attack_time_seconds";
    static constexpr const char* kParameterAmpEGDecayTimeSeconds         = "ampeg_decay_time_seconds";
    static constexpr const char* kParameterAmpEGSustainLevel             = "ampeg_sustain_level";
    static constexpr const char* kParameterAmpEGReleaseTimeSeconds       = "ampeg_release_time_seconds";
    static constexpr const char* kParameterFilterEGAttackTimeSeconds     = "filtereg_attack_time_seconds";
    static constexpr const char* kParameterFilterEGDecayTimeSeconds      = "filtereg_decay_time_seconds";
    static constexpr const char* kParameterFilterEGSustainLevel          = "filtereg_sustain_level";
    static constexpr const char* kParameterFilterEGReleaseTimeSeconds    = "filtereg_release_time_seconds";

    AKSamplerProcessorEx();
    virtual ~AKSamplerProcessorEx() {}

    void load(const String& path);
    void stopAllVoices() noexcept;

    int  getMidiChannel() const noexcept { return mMidiChannel; }
    void setMidiChannel(int midiChannel) noexcept { mMidiChannel = midiChannel; }

    void setA4Frequency(float frequency) noexcept;
    void setScaleCents(std::array<int,12> centsFromC) noexcept;

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

protected:
    void handleMidiEvent(const MidiMessage& message) noexcept override;

private:
    void setDefaultOpcodeValues() noexcept;
    int  parseOpcodeIntValue(const std::string& opcode, const std::string& value) noexcept;

    static void debugPrint(const AKSamplerParams& params) noexcept;

    // juce::AudioProcessorListener
    void audioProcessorParameterChanged(AudioProcessor* processor, int parameterIndex, float newValue) override;
    void audioProcessorChanged(AudioProcessor* processor, const ChangeDetails& details) override {}

    // sfz::ParserListener
    void onParseHeader(const sfz::SourceRange& /*range*/, const std::string& /*header*/) override;
    void onParseOpcode(const sfz::SourceRange& /*rangeOpcode*/, const sfz::SourceRange& /*rangeValue*/,
                       const std::string& /*name*/, const std::string& /*value*/) override;
    void onParseError(const sfz::SourceRange& /*range*/, const std::string& /*message*/) override;
    void onParseFullBlock(const std::string& /*header*/, const std::vector<sfz::Opcode>& /*opcodes*/) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout() noexcept;

    inline float getParameterValue(juce::StringRef parameterID) noexcept
    {
        return reinterpret_cast<juce::AudioParameterFloat*>(mParameters.getParameter(parameterID))
                ->get();
    }

    // AudioProcessorValueTreeState only handles RangedAudioParameter
    juce::AudioProcessorValueTreeState mParameters;
    AKSamplerParams mSamplerParams;
    std::atomic<float> mA4Frequency;
    std::array<std::atomic<int>,12> mCentsFromC;

    std::atomic_flag mSamplerBusy;
    std::filesystem::path mSfzPath;

    int mMidiChannel;
    int mPitchKeycenter;
    int mLoKey, mHiKey;
    int mLoVel, mHiVel;
    int mLoopStart, mLoopEnd;
    std::string mLoopMode;
    std::string mSample;
    std::string mErrorMessage;

};

} // maqam

#endif // AKSAMPLER_PROCESSOR_EX_H
