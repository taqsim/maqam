//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#include "AudioRoot.h"
#include "AudioGraph.h"
#include "NativeWrapper.h"
#include "log.h"

using namespace maqam;

AudioRoot::AudioRoot()
    : mMidiSource {}
    , mMidiQueue(kMidiEventQueueSize)
    , mAudioBuffer(kChannelCount, kMaxFramesPerBlock)
    , mAudioStreamStarted(false)
    , mGraph(nullptr)
{
    createStream();
}

AudioRoot* AudioRoot::fromJava(JNIEnv *env, jobject thiz) noexcept
{
    return NativeWrapper::getImpl<AudioRoot>(env, thiz);
}

void AudioRoot::setGraph(AudioGraph* graph) noexcept
{
    if (graph != nullptr) {
        juce::AudioProcessorGraph& audioProcessorGraph = graph->getAudioProcessorGraph();
        audioProcessorGraph.prepareToPlay(kSampleRate, kMaxFramesPerBlock);
        mGraph = &audioProcessorGraph;
    } else {
        mGraph = nullptr;
    }
}

void AudioRoot::connectMidiDevice(int id, AMidiDevice* midiDevice) noexcept
{
    mMidiSourceMutex.lock();

    if (AMidiDevice_getNumOutputPorts(midiDevice) > 0) {
        int idx = -1;

        for (int i = 0; i < kMaxMidiPorts; ++i) {
            if (mMidiSource[i].port == nullptr) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            LOG_E(LOG_TAG, "AudioRoot no more source MIDI ports available");
            mMidiSourceMutex.unlock();
            return;
        }

        AMidiOutputPort* port;
        const int32_t result = AMidiOutputPort_open(midiDevice, /*portNumber*/0, &port);

        if (result != AMEDIA_OK) {
            LOG_E(LOG_TAG, "AudioRoot could not open source MIDI port [%d]", id);
            mMidiSourceMutex.unlock();
            return;
        }

        mMidiSource[idx].isOpen = true;
        mMidiSource[idx].id = id;
        mMidiSource[idx].port = port;

    } else if (AMidiDevice_getNumInputPorts(midiDevice) > 0) {
        // MIDI output currently not supported
    }

    mMidiSourceMutex.unlock();
}

void AudioRoot::disconnectMidiDevice(int id) noexcept
{
    for (int i = 0; i < kMaxMidiPorts; ++i) {
        if (mMidiSource[i].id == id) {
            mMidiSource[i].isOpen = false;
        }
    }
}

void AudioRoot::queueMidiEvent(const MidiEvent& event) noexcept
{
    mMidiQueue.put(event);
}

void AudioRoot::startStream() noexcept
{
    mAudioStream->requestStart();
    mAudioStreamStarted = true;
}

void AudioRoot::stopStream() noexcept
{
    mAudioStreamStarted = false;
    mAudioStream->requestStop();
}

oboe::DataCallbackResult
AudioRoot::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames)
{
    mAudioBuffer.setSize(kChannelCount, numFrames, /*keepExistingContent=*/false,
        /*clearExtraSpace=*/false, /*avoidReallocating=*/true);
    juce::AudioProcessorGraph* graph = mGraph.load();

    juce::MidiBuffer midiBuffer;
    processMidi(midiBuffer);

    if (graph != nullptr) {
        graph->processBlock(mAudioBuffer, midiBuffer);
    } else {
        mAudioBuffer.clear();
    }

    // We requested AudioFormat::Float. So if the stream opens
    // we know we got the Float format.
    // If you do not specify a format then you should check what format
    // the stream has and cast to the appropriate type.
    const auto samples = static_cast<float *>(audioData);

    // Oboe expects interleaved channels sample data
    // JUCE/modules/juce_audio_devices/native/juce_android_Oboe.cpp
    const int numChannels = mAudioBuffer.getNumChannels();
    using Format = juce::AudioData::Format<juce::AudioData::Float32, juce::AudioData::NativeEndian>;

    juce::AudioData::interleaveSamples (
        juce::AudioData::NonInterleavedSource<Format> { mAudioBuffer.getArrayOfReadPointers(),
                                                        numChannels },
        juce::AudioData::InterleavedDest<Format> { samples, numChannels },
        numFrames
    );

    return oboe::DataCallbackResult::Continue;
}

void AudioRoot::createStream() noexcept
{
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::Float)
            ->setChannelCount(kChannelCount)
            ->setSampleRate(kSampleRate)
            ->setDataCallback(this)
            ->setErrorCallback(this);

    const oboe::Result result = builder.openStream(mAudioStream);

    if (result != oboe::Result::OK) {
        LOG_E(LOG_TAG, "AudioRoot failed to create stream. Error: %s", oboe::convertToText(result));
    }
}

// MIDI events are currently not sample accurate
void AudioRoot::processMidi(juce::MidiBuffer& inBuffer) noexcept
{
    uint8_t midiBytes[kMaxMidiReadBufferBytes];
    int32_t opcode;
    size_t numBytesReceived;
    int64_t timestamp;
    int32_t numMessages;

    for (int i = 0; i < kMaxMidiPorts; ++i) {
        bool receiveMidi = true;

        if (mMidiSource[i].port != nullptr) {
            if (mMidiSource[i].isOpen) {
                while (receiveMidi) {
                    numMessages = AMidiOutputPort_receive(mMidiSource[i].port, &opcode, midiBytes,
                                                          sizeof(midiBytes), &numBytesReceived,
                                                          &timestamp);
                    if (numMessages == 1) {
                        inBuffer.addEvent(midiBytes,
                                          static_cast<int>(numBytesReceived), /*sampleNumber*/0);
                    } else {
                        if (numMessages < 0) {
                            LOG_E(LOG_TAG, "AudioRoot error receiving data from MIDI port");
                        }
                        receiveMidi = false;
                    }
                }
            } else {
                AMidiOutputPort_close(mMidiSource[i].port);
                mMidiSource[i].port = nullptr;
            }
        }
    }

    MidiEvent localEvent {};

    while (mMidiQueue.get(localEvent)) {
        inBuffer.addEvent(localEvent.bytes, localEvent.size, /*sampleNumber*/0);
    }
}

void AudioRoot::onErrorAfterClose(oboe::AudioStream* /* audioStream */, oboe::Result /* error */)
{
    createStream();

    if (mAudioStreamStarted) {
        mAudioStream->requestStart();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniOpenMidi(JNIEnv *env, jobject thiz, jint id, jobject midi_device)
{
    AMidiDevice* midiDevice = nullptr;
    AMidiDevice_fromJava(env, midi_device, &midiDevice);
    AudioRoot::fromJava(env, thiz)->connectMidiDevice(id, midiDevice);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniCloseMidi(JNIEnv *env, jobject thiz, jint id)
{
    AudioRoot::fromJava(env, thiz)->disconnectMidiDevice(id);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniQueueMidi(JNIEnv *env, jobject thiz, jbyteArray bytez)
{
    jboolean isCopy;
    jbyte* bytes = env->GetByteArrayElements(bytez, &isCopy);
    const jsize size = env->GetArrayLength(bytez);
    MidiEvent event { size < MidiEvent::kMaxSizeBytes ? size : MidiEvent::kMaxSizeBytes };
    memcpy(event.bytes, bytes, event.size);
    env->ReleaseByteArrayElements(bytez, bytes, 0);

    AudioRoot::fromJava(env, thiz)->queueMidiEvent(event);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniStartStream(JNIEnv *env, jobject thiz)
{
    AudioRoot::fromJava(env, thiz)->startStream();
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniStopStream(JNIEnv *env, jobject thiz)
{
    AudioRoot::fromJava(env, thiz)->stopStream();
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioRoot_jniSetGraph(JNIEnv *env, jobject thiz, jobject graph)
{
    AudioRoot::fromJava(env, thiz)->setGraph(AudioGraph::fromJava(env, graph));
}
