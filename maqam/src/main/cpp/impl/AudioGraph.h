//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#ifndef AUDIOGRAPH_H
#define AUDIOGRAPH_H

#include <jni.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "AudioNode.h"

namespace maqam {

class AudioGraph {
public:
    AudioGraph();
    ~AudioGraph();

    static AudioGraph* fromJava(JNIEnv *env, jobject thiz) noexcept;

    juce::AudioProcessorGraph& getAudioProcessorGraph() noexcept { return mImpl; }

    void addNode(AudioNode* node, juce::AudioProcessor* processor);
    void connectNodes(AudioNode* source, AudioNode* sink, bool audio, bool midi);

    void debugPrintConnections() const noexcept;

private:
    juce::AudioProcessorGraph         mImpl;
    juce::AudioProcessorGraph::NodeID mAudioInputNodeID;
    juce::AudioProcessorGraph::NodeID mAudioOutputNodeID;
    juce::AudioProcessorGraph::NodeID mMidiInputNodeID;
    juce::AudioProcessorGraph::NodeID mMidiOutputNodeID;

};

} // maqam

#endif // AUDIOGRAPH_H