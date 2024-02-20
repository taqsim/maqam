//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef AUDIOROOT_H
#define AUDIOROOT_H

#include <atomic>
#include <memory>
#include <mutex>

#include <jni.h>
#include <AMidi/AMidi.h>
#include <oboe/Oboe.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <ring_buffer/ring_buffer.h>

#include "AudioGraph.h"

namespace maqam {

struct MidiSource
{
    int id;
    std::atomic_bool isOpen;
    std::atomic<AMidiOutputPort*> port;
};

// juce::MidiMessage not suitable for Ring_Buffer
struct MidiEvent
{
    static constexpr int kMaxSizeBytes = 3;

    int size;
    uint8_t bytes[kMaxSizeBytes];
};

class AudioRoot : public oboe::AudioStreamDataCallback, public oboe::AudioStreamErrorCallback
{
public:
    static constexpr int kMaxMidiPorts           = 16;
    static constexpr int kMaxMidiReadBufferBytes = 3;
    static constexpr int kMidiEventQueueSize     = 128 * sizeof(MidiEvent);

    static constexpr int32_t kMaxFramesPerBlock = 8192;
    static constexpr int32_t kSampleRate        = 48000;
    static constexpr int32_t kChannelCount      = oboe::ChannelCount::Stereo;

    AudioRoot();

    static AudioRoot* fromJava(JNIEnv *env, jobject thiz) noexcept;

    void setGraph(AudioGraph* graph) noexcept;

    void connectMidiDevice(int id, AMidiDevice* midiDevice) noexcept;
    void disconnectMidiDevice(int id) noexcept;
    void queueMidiEvent(const MidiEvent& event) noexcept;

    void startStream() noexcept;
    void stopStream() noexcept;

protected:
    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

    void onErrorAfterClose(oboe::AudioStream* audioStream, oboe::Result error);

private:
    void createStream() noexcept;
    void processMidi(juce::MidiBuffer& inBuffer) noexcept;

    MidiSource  mMidiSource[kMaxMidiPorts];
    std::mutex  mMidiSourceMutex;
    Ring_Buffer mMidiQueue;

    juce::AudioBuffer<float> mAudioBuffer;

    bool mAudioStreamStarted;

    std::shared_ptr<oboe::AudioStream>      mAudioStream;
    std::atomic<juce::AudioProcessorGraph*> mGraph;

};

} // maqam

#endif // AUDIOROOT_H
