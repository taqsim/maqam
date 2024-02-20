//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#include <memory>
#include <sstream>

#include "AudioGraph.h"
#include "AudioRoot.h"
#include "NativeWrapper.h"
#include "log.h"

using namespace maqam;
using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;

AudioGraph::AudioGraph()
{
    mImpl.setProcessingPrecision(juce::AudioProcessor::singlePrecision);
    mImpl.setPlayConfigDetails(0, AudioRoot::kChannelCount, AudioRoot::kSampleRate,
                               AudioRoot::kMaxFramesPerBlock);
    mAudioInputNodeID = mImpl.addNode(std::make_unique<AudioGraphIOProcessor>(
            AudioGraphIOProcessor::audioInputNode))->nodeID;
    mAudioOutputNodeID = mImpl.addNode(std::make_unique<AudioGraphIOProcessor>(
            AudioGraphIOProcessor::audioOutputNode))->nodeID;
    mMidiInputNodeID = mImpl.addNode(std::make_unique<AudioGraphIOProcessor>(
            AudioGraphIOProcessor::midiInputNode))->nodeID;
    mMidiOutputNodeID = mImpl.addNode(std::make_unique<AudioGraphIOProcessor>(
            AudioGraphIOProcessor::midiOutputNode))->nodeID;
}

AudioGraph::~AudioGraph()
{
    std::vector<juce::AudioProcessorGraph::Connection> connections = mImpl.getConnections();

    for (auto it = connections.begin(); it != connections.end(); ++it) {
        mImpl.removeConnection(*it, juce::AudioProcessorGraph::UpdateKind::sync);
    }

    juce::ReferenceCountedArray<juce::AudioProcessorGraph::Node> nodes = mImpl.getNodes();

    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        mImpl.removeNode(*it, juce::AudioProcessorGraph::UpdateKind::sync);
    }
}

AudioGraph* AudioGraph::fromJava(JNIEnv *env, jobject thiz) noexcept
{
    if (thiz == nullptr) {
        return nullptr;
    }

    return NativeWrapper::getImpl<AudioGraph>(env, thiz);
}

void AudioGraph::addNode(AudioNode* node, juce::AudioProcessor* processor)
{
    if (node->getJUCEAudioProcessorGraphNodeID().uid != 0) {
        throw std::runtime_error("Node already owned by a graph");
    }

    juce::AudioProcessorGraph::NodeID nodeID =
            mImpl.addNode(std::unique_ptr<juce::AudioProcessor>(processor))->nodeID;
    node->setJUCEAudioProcessorGraphNodeID(nodeID);
}

void AudioGraph::connectNodes(AudioNode* source, AudioNode* sink, bool audio, bool midi)
{
    juce::AudioProcessorGraph::NodeID srcAudioNodeID, dstAudioNodeID, srcMidiNodeID, dstMidiNodeID;

    if (source != nullptr) {
        srcAudioNodeID = srcMidiNodeID = source->getJUCEAudioProcessorGraphNodeID();

        if (srcAudioNodeID.uid == 0) {
            throw std::runtime_error("Source node is not owned by graph");
        }
    } else {
        srcAudioNodeID = mAudioInputNodeID;
        srcMidiNodeID = mMidiInputNodeID;
    }

    if (sink != nullptr) {
        dstAudioNodeID = dstMidiNodeID = sink->getJUCEAudioProcessorGraphNodeID();

        if (dstAudioNodeID.uid == 0) {
            throw std::runtime_error("Sink node is not owned by graph");
        }
    } else {
        dstAudioNodeID = mAudioOutputNodeID;
        dstMidiNodeID = mMidiOutputNodeID;
    }

    if (audio) {
        for (int i = 0; i < AudioRoot::kChannelCount; ++i) {
            const bool success = mImpl.addConnection({
                { srcAudioNodeID, i }, { dstAudioNodeID, i }
            });

            if (! success) {
                throw std::runtime_error("Could not connect nodes audio");
            }
        }
    }

    if (midi) {
        const bool success = mImpl.addConnection({
            { srcMidiNodeID, juce::AudioProcessorGraph::midiChannelIndex },
            { dstMidiNodeID, juce::AudioProcessorGraph::midiChannelIndex }
        });

        if (! success) {
            throw std::runtime_error("Could not connect nodes MIDI");
        }
    }
}

void AudioGraph::debugPrintConnections() const noexcept
{
    std::stringstream ss;
    ss << "juce::AudioProcessorGraph connections :\n";

    int i = 0;

    for (auto conn : mImpl.getConnections()) {
        ss << "   #" << i << " :"
            << " uid=" << conn.source.nodeID.uid
                << (conn.source.isMIDI() ?
                    "" : " ch=" + std::to_string(conn.source.channelIndex))
                << (conn.source.isMIDI() ? "(M)" : "")
            << " ->"
            << " uid=" << conn.destination.nodeID.uid
                << (conn.destination.isMIDI() ?
                    "" : " ch=" + std::to_string(conn.destination.channelIndex))
                << (conn.destination.isMIDI() ? "(M)" : "")
            << "\n";
        i++;
    }

    LOG_I(LOG_TAG, "%s", ss.str().c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioGraph_jniAddNode(JNIEnv *env, jobject thiz, jobject node)
{
    try {
    AudioGraph::fromJava(env, thiz)->addNode(AudioNode::fromJava(env, node),
                                             AudioNode::getJUCEAudioProcessor(env, node));
    } catch (const std::exception& e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioGraph_jniConnectNodes(JNIEnv *env, jobject thiz,
                                                   jobject source, jobject sink,
                                                   jboolean audio, jboolean midi)
{
    try {
        AudioGraph::fromJava(env, thiz)->connectNodes(AudioNode::fromJava(env, source),
                                                      AudioNode::fromJava(env, sink),
                                                      audio, midi);
    } catch (const std::exception& e) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), e.what());
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_AudioGraph_jniDebugPrintConnections(JNIEnv *env, jobject thiz)
{
    AudioGraph::fromJava(env, thiz)->debugPrintConnections();
}
