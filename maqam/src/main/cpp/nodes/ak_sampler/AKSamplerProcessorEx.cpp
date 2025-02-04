//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include "AKSamplerProcessorEx.h"
#include "nodes/AudioProcessorHelpers.h"

using namespace juce;
using namespace maqam;

static const char* kBuiltinTestWaveformPath = "builtin:test-waveform";

AKSamplerProcessorEx::AKSamplerProcessorEx()
    : mParameters (*this, nullptr, "AKSampler", createParameterLayout())
    , mA4Frequency(440.0)
    , mCentsFromC()
    , mSamplerBusy ATOMIC_FLAG_INIT
    , mMidiChannel(kMidiChannelOmni)
    , mPitchKeycenter(0)
    , mLoKey(0)
    , mHiKey(0)
    , mLoVel(0)
    , mHiVel(0)
    , mLoopStart(0)
    , mLoopEnd(0)
{
    AKSamplerProcessor::addListener(this);
}

void AKSamplerProcessorEx::load(const String& path)
{
    std::filesystem::path sfzPath(path.toStdString());

    if (mSfzPath == sfzPath) {
        return;
    }

    // AKSamplerProcessor::loadSfz() can make the program crash, just create a new AKSampler.

    int millis = 0;

    while (mSamplerBusy.test_and_set()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (++millis == 1000) {
            throw std::runtime_error("Could not lock the sampler");
        }
    }

    samplerPtr->deinit(); // free previously loaded samples memory, ~Sampler() is not doing it.

    samplerPtr = std::make_unique<AKSampler>();
    samplerPtr->init(getSampleRate());
    samplerPtr->deinit();

    mSfzPath = sfzPath;
    mErrorMessage.clear();
    setDefaultOpcodeValues();

    if (path == kBuiltinTestWaveformPath) {
        samplerPtr->loadTestWaveform();
    } else {
        sfz::Parser parser;
        parser.setListener(this);
        parser.parseFile(mSfzPath.string());

        if (! mErrorMessage.empty()) {
            mSamplerBusy.clear();
            throw std::runtime_error(mErrorMessage);
        }
    }

    samplerPtr->buildKeyMap();
    samplerPtr->setParams(mSamplerParams);

    mSamplerBusy.clear();
}

void AKSamplerProcessorEx::stopAllVoices() noexcept
{
    samplerPtr->stopAllVoices();
}

void AKSamplerProcessorEx::setA4Frequency(float frequency) noexcept
{
    mA4Frequency = frequency;
}

void AKSamplerProcessorEx::setScaleCents(std::array<int,12> centsFromC) noexcept
{
    for (int i = 0; i < 12; i++) {
        mCentsFromC[i] = centsFromC[i];
    }
}

void AKSamplerProcessorEx::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    if (! mSamplerBusy.test_and_set()) {
        AKSamplerProcessor::processBlock(buffer, midiMessages);
        mSamplerBusy.clear();
    }
}

/* realtime */
void AKSamplerProcessorEx::handleMidiEvent(const MidiMessage& message) noexcept
{
    // isForChannel() expects 1-based index [1-16]
    if ((mMidiChannel != kMidiChannelOmni) && ! message.isForChannel(mMidiChannel + 1)) {
        return;
    }

    if (message.isNoteOn()) {
        const int smpNn = message.getNoteNumber() + patchParams.sampler.osc1.pitchOffsetSemitones;
        const float cents = static_cast<float>(mCentsFromC[smpNn % 12])
                + patchParams.sampler.osc1.detuneOffsetCents;
        const float n = static_cast<float>(smpNn) + cents / 100.f;
        const auto fSmpHz = mA4Frequency * pow(2.f, (n - 69.f) / 12.f);
        const int vel = static_cast<int>(127.f * message.getFloatVelocity());
        if (vel == 0) {
            samplerPtr->stopNote(smpNn, false);
        } else {
            samplerPtr->playNote(smpNn, vel, fSmpHz);
        }
    } else if (message.isNoteOff()) {
        int smpNn = message.getNoteNumber() + patchParams.sampler.osc1.pitchOffsetSemitones;
        samplerPtr->stopNote(smpNn, false);
    } else if (message.isAllNotesOff() || message.isAllSoundOff()) {
        for (unsigned i = 0; i < 128; i++) {
            samplerPtr->stopNote(i, true);
        }
    } else if (message.isPitchWheel()) {
        samplerPtr->pitchBend(2.f * static_cast<float>(message.getPitchWheelValue() - 8192) / 8192.f);
    } else if (message.isController()) {
        samplerPtr->controller(message.getControllerNumber(), message.getControllerValue());
    }
}

void AKSamplerProcessorEx::setDefaultOpcodeValues() noexcept
{
    mPitchKeycenter = 69; // A4 - 440Hz
    mLoKey = 0;
    mHiKey = 127;
    mLoVel = 0;
    mHiVel = 127;
    mLoopStart = 0;
    mLoopEnd = 0;
    mLoopMode = "no_loop";
    mSample = "";
}

int AKSamplerProcessorEx::parseOpcodeIntValue(const std::string& opcode,
                                              const std::string& value) noexcept
{
    int ival;

    try {
        ival = std::stoi(value);
    } catch(std::invalid_argument const&) {
    } catch(std::out_of_range const&) {
        // Should never happen if the sfizz parser already validates values
        ival = 0;
        mErrorMessage = "Could not parse opcode integer value - " + opcode + "=" + value;
    }

    return ival;
}

void AKSamplerProcessorEx::debugPrint(const AKSamplerParams& params) noexcept
{
    static const char *format =
        "main.masterLevel = %.2f\n"
        "main.pitchBendUpSemitones = %d\n"
        "main.pitchBendDownSemitones = %d\n"
        "main.ampVelocitySensitivity = %.2f\n"
        "main.filterVelocitySensitivity = %.2f\n"
        "osc1.sfzName = %s\n"
        "osc1.pitchOffsetSemitones = %d\n"
        "osc1.detuneOffsetCents = %.2f\n"
        "filter.stages = %d\n"
        "filter.cutoff = %.2f\n"
        "filter.resonance = %.2f\n"
        "filter.envAmount = %.2f\n"
        "ampEG.attackTimeSeconds = %.2f\n"
        "ampEG.decayTimeSeconds = %.2f\n"
        "ampEG.sustainLevel = %.2f\n"
        "ampEG.releaseTimeSeconds = %.2f\n"
        "filterEG.attackTimeSeconds = %.2f\n"
        "filterEG.decayTimeSeconds = %.2f\n"
        "filterEG.sustainLevel = %.2f\n"
        "filterEG.releaseTimeSeconds = %.2f\n";

    printf(format,
           params.main.masterLevel,
           params.main.pitchBendUpSemitones,
           params.main.pitchBendDownSemitones,
           params.main.ampVelocitySensitivity,
           params.main.filterVelocitySensitivity,
           params.osc1.sfzName.toUTF8(),
           params.osc1.pitchOffsetSemitones,
           params.osc1.detuneOffsetCents,
           params.filter.stages,
           params.filter.cutoff,
           params.filter.resonance,
           params.filter.envAmount,
           params.ampEG.attackTimeSeconds,
           params.ampEG.decayTimeSeconds,
           params.ampEG.sustainLevel,
           params.ampEG.releaseTimeSeconds,
           params.filterEG.attackTimeSeconds,
           params.filterEG.decayTimeSeconds,
           params.filterEG.sustainLevel,
           params.filterEG.releaseTimeSeconds);
}

void AKSamplerProcessorEx::audioProcessorParameterChanged(AudioProcessor* processor,
                                                          int parameterIndex, float newValue)
{
    mSamplerParams.main.masterLevel =
            getParameterValue(kParameterMainMasterLevel);
    mSamplerParams.main.pitchBendUpSemitones =
            static_cast<int>(getParameterValue(kParameterMainPitchBendUpSemitones));
    mSamplerParams.main.pitchBendDownSemitones =
            static_cast<int>(getParameterValue(kParameterMainPitchBendDownSemitones));
    mSamplerParams.main.ampVelocitySensitivity =
            getParameterValue(kParameterMainAmpVelocitySensitivity);
    mSamplerParams.main.filterVelocitySensitivity =
            getParameterValue(kParameterMainFilterVelocitySensitivity);

    mSamplerParams.osc1.pitchOffsetSemitones =
            static_cast<int>(getParameterValue(kParameterOsc1PitchOffsetSemitones));
    mSamplerParams.osc1.detuneOffsetCents =
            getParameterValue(kParameterOsc1DetuneOffsetCents);

    mSamplerParams.filter.stages = static_cast<int>(getParameterValue(kParameterFilterStages));
    mSamplerParams.filter.cutoff = getParameterValue(kParameterFilterCutoff);
    mSamplerParams.filter.resonance = getParameterValue(kParameterFilterResonance);
    mSamplerParams.filter.envAmount = getParameterValue(kParameterFilterEnvAmount);

    if (getParameterValue(kParameterAmpEGBypass) == 0) {
        mSamplerParams.ampEG.attackTimeSeconds =
                getParameterValue(kParameterAmpEGAttackTimeSeconds);
        mSamplerParams.ampEG.decayTimeSeconds =
                getParameterValue(kParameterAmpEGDecayTimeSeconds);
        mSamplerParams.ampEG.sustainLevel =
                getParameterValue(kParameterAmpEGSustainLevel);
        mSamplerParams.ampEG.releaseTimeSeconds =
                getParameterValue(kParameterAmpEGReleaseTimeSeconds);
    } else {
        mSamplerParams.ampEG.attackTimeSeconds = 0;
        mSamplerParams.ampEG.decayTimeSeconds = 0;
        mSamplerParams.ampEG.sustainLevel = 1.f;
        mSamplerParams.ampEG.releaseTimeSeconds = 0;
    }

    mSamplerParams.filterEG.attackTimeSeconds =
            getParameterValue(kParameterFilterEGAttackTimeSeconds);
    mSamplerParams.filterEG.decayTimeSeconds =
            getParameterValue(kParameterFilterEGDecayTimeSeconds);
    mSamplerParams.filterEG.sustainLevel =
            getParameterValue(kParameterFilterEGSustainLevel);
    mSamplerParams.filterEG.releaseTimeSeconds =
            getParameterValue(kParameterFilterEGReleaseTimeSeconds);

    samplerPtr->setParams(mSamplerParams);

    //debugPrint(params);
}

void AKSamplerProcessorEx::onParseHeader(const sfz::SourceRange& /*range*/,
                                         const std::string& header)
{
    if (header == "group") {
        setDefaultOpcodeValues();
    }
}

void AKSamplerProcessorEx::onParseOpcode(const sfz::SourceRange& /*rangeOpcode*/,
                                         const sfz::SourceRange& /*rangeValue*/,
                                         const std::string& name, const std::string& value)
{
    if (name == "pitch_keycenter") {
        mPitchKeycenter = parseOpcodeIntValue(name, value);
    } else if (name == "lokey") {
        mLoKey = parseOpcodeIntValue(name, value);
    } else if (name == "hikey") {
        mHiKey = parseOpcodeIntValue(name, value);
    } else if (name == "lovel") {
        mLoVel = parseOpcodeIntValue(name, value);
    } else if (name == "hivel") {
        mHiVel = parseOpcodeIntValue(name, value);
    } else if (name == "loop_start") {
        mLoopStart = parseOpcodeIntValue(name, value);
    } else if (name == "loop_end") {
        mLoopEnd = parseOpcodeIntValue(name, value);
    } else if (name == "loop_mode") {
        mLoopMode = value;
    } else if (name == "sample") {
        mSample = value;
    }
}

void AKSamplerProcessorEx::onParseError(const sfz::SourceRange& /*range*/,
                                        const std::string& message)
{
    mErrorMessage = message;
}

void AKSamplerProcessorEx::onParseFullBlock(const std::string& header,
                                            const std::vector<sfz::Opcode>& /*opcodes*/)
{
    if (header != "region") {
        return;
    }

    if (mSample.empty()) {
        mErrorMessage = "Empty filename in sample opcode";
        return;
    }

    std::replace(mSample.begin(), mSample.end(), '\\', '/');
    const std::filesystem::path samplePath = mSfzPath.parent_path().append(mSample).make_preferred();

    AKSampleFileDescriptor sfd;
    sfd.path = samplePath.c_str();
    sfd.sd.bLoop = mLoopMode != "no_loop";
    sfd.sd.fStart = 0.0;
    sfd.sd.fLoopStart = static_cast<float>(mLoopStart);
    sfd.sd.fLoopEnd = static_cast<float>(mLoopEnd);
    sfd.sd.fEnd = 0.0f;
    sfd.sd.noteNumber = mPitchKeycenter;
    sfd.sd.noteHz = 440.f * powf(2.f, (static_cast<float>(sfd.sd.noteNumber) - 69.f) / 12.f);
    sfd.sd.min_note = mLoKey;
    sfd.sd.max_note = mHiKey;
    sfd.sd.min_vel = mLoVel;
    sfd.sd.max_vel = mHiVel;

    if (samplePath.extension() == ".wv") {
        if (! loadCompressedSampleFile(sfd)) {
            mErrorMessage = "Error loading compressed sample file";
        }
    } else {
        if (! loadSampleFile(sfd)) {
            mErrorMessage = "Error loading sample file";
        }
    }
}

AudioProcessorValueTreeState::ParameterLayout
AKSamplerProcessorEx::createParameterLayout() noexcept
{
    return {
        createFloatParameter(
                kParameterMainMasterLevel,
                "Master level", "%",
                /*min*/0, /*max*/1.f, /*def*/1.f,
                [](float v, int _) { return String(static_cast<int>(100.f * v)); }
        ),
        createFloatParameter(
                kParameterMainPitchBendUpSemitones,
                "Pitch bend up", "semitones",
                /*min*/-12.f, /*max*/12.f, /*def*/0
        ),
        createFloatParameter(
                kParameterMainPitchBendDownSemitones,
                "Pitch bend down", "semitones",
                /*min*/-12.f, /*max*/12.f, /*def*/0
        ),
        createFloatParameter(
                kParameterMainAmpVelocitySensitivity,
                "Amplifier velocity sensitivity", "",
                /*min*/0, /*max*/1.f, /*def*/1.f),
        createFloatParameter(
                kParameterMainFilterVelocitySensitivity,
                "Filter velocity sensitivity", "",
                /*min*/0, /*max*/1.f, /*def*/1.f
        ),
        createFloatParameter(
                kParameterOsc1PitchOffsetSemitones,
                "Oscillator 1 pitch offset", "semitones",
                /*min*/-12.f, /*max*/12.f, /*def*/0
        ),
        createFloatParameter(
                kParameterOsc1DetuneOffsetCents,
                "Oscillator 1 detune offset", "cents",
                /*min*/-100.f, /*max*/100.f, /*def*/0
        ),
        createFloatParameter(
                kParameterFilterStages,
                "Filter stages", "",
                /*min*/0, /*max*/4.f, /*def*/1.f
        ),
        createFloatParameter(
                kParameterFilterCutoff,
                "Filter cutoff", "Hz",
                /*min*/0, /*max*/1000.f, /*def*/1000.f
        ),
        createFloatParameter(
                kParameterFilterResonance,
                "Filter resonance", "",
                /*min*/-12.f, /*max*/12.f, /*def*/0
        ),
        createFloatParameter(
                kParameterFilterEnvAmount,
                "Filter envelope amount", "",
                /*min*/0, /*max*/1000.f, /*def*/0
        ),
        createFloatParameter(
                kParameterAmpEGBypass,
                "Amplifier envelope generator bypass", "",
                /*min*/0, /*max*/1.f, /*def*/0,
                [](float v, int _) { return String(v == 0 ? "off" : "on"); }
        ),
        createFloatParameter(
                kParameterAmpEGAttackTimeSeconds,
                "Amplifier envelope generator attack time", "sec",/*min*/0, /*max*/3.f, /*def*/0
        ),
        createFloatParameter(
                kParameterAmpEGDecayTimeSeconds,
                "Amplifier envelope generator decay time", "sec",
                /*min*/0, /*max*/3.f, /*def*/0
        ),
        createFloatParameter(
                kParameterAmpEGSustainLevel,
                "Amplifier envelope generator sustain level", "",
                /*min*/0, /*max*/1.f, /*def*/1.f
        ),
        createFloatParameter(
                kParameterAmpEGReleaseTimeSeconds,
                "Amplifier envelope generator release time", "sec",
                /*min*/0, /*max*/3.f, /*def*/0
        ),
        createFloatParameter(
                kParameterFilterEGAttackTimeSeconds,
                "Filter envelope generator attack time", "sec",
                /*min*/0, /*max*/3.f, /*def*/0
        ),
        createFloatParameter(
                kParameterFilterEGDecayTimeSeconds,
                "Filter envelope generator decay time", "sec",
                /*min*/0, /*max*/3.f, /*def*/0
        ),
        createFloatParameter(
                kParameterFilterEGSustainLevel,
                "Filter envelope generator sustain level", "",
                /*min*/0, /*max*/1.f, /*def*/1.f
        ),
        createFloatParameter(
                kParameterFilterEGReleaseTimeSeconds,
                "Filter envelope generator release time", "sec",
                /*min*/0, /*max*/3.f, /*def*/0
        )
    };
}
