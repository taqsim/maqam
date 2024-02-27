//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef AUDIONODE_H
#define AUDIONODE_H

#include <map>
#include <thread>
#include <jni.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include "NativeWrapper.h"

namespace maqam {

class AudioNode : protected juce::AudioProcessorListener, protected juce::ValueTree::Listener
{
public:
    AudioNode();
    ~AudioNode() override;

    constexpr static const char* kDSPFieldName = "dsp";

    static AudioNode* fromJava(JNIEnv *env, jobject thiz) noexcept;

    static void bindDSPClass(const std::string& javaClassName,
                             NativeWrapper::ImplFactoryFunction factory,
                             NativeWrapper::ImplDeleterFunction deleter)
    {
        NativeWrapper::bindClass(javaClassName, factory, deleter, kDSPFieldName);
    }

    template<class T>
    static void bindDSPClass(const std::string& javaClassName)
    {
        NativeWrapper::bindClass<T>(javaClassName, kDSPFieldName);
    }

    static juce::AudioProcessor* getDSP(JNIEnv *env, jobject thiz) noexcept;

    void createDSP(JNIEnv *env, jobject thiz) noexcept;

    juce::AudioProcessorGraph::NodeID getAudioProcessorGraphNodeID()
    {
        return mAudioProcessorGraphNodeID;
    }

    void setAudioProcessorGraphNodeID(juce::AudioProcessorGraph::NodeID nodeID)
    {
        mAudioProcessorGraphNodeID = nodeID;
    }

    float getParameterValue(const juce::String& id) noexcept;
    void  setParameterValue(const juce::String& id, float value) noexcept;
    void  getParameterValueRange(const juce::String& id, float range[]) noexcept;
    juce::String getParameterValueAsText(const juce::String& id) noexcept;
    juce::String getParameterName(const juce::String& id) noexcept;
    juce::String getParameterLabel(const juce::String& id) noexcept;

    float        getValueTreePropertyFloatValue(const juce::String& id) noexcept;
    void         setValueTreePropertyFloatValue(const juce::String& id, const float value);
    juce::String getValueTreePropertyStringValue(const juce::String& id) noexcept;
    void         setValueTreePropertyStringValue(const juce::String& id, const juce::String& value);

protected:
    // juce::AudioProcessorListener
    void audioProcessorParameterChanged(juce::AudioProcessor *processor, int parameterIndex,
            float newValue) override;

    void audioProcessorChanged(juce::AudioProcessor *processor, const ChangeDetails &details) override
    {}

    // juce::ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                  const juce::Identifier& property) override;

private:
    JavaVM*         mJVM;
    JNIEnv*         mEnv;
    std::thread::id mEnvThreadId;
    jobject         mOwner;

    using AudioProcessorParameterMap = std::map<juce::String, juce::AudioParameterFloat *>;
    AudioProcessorParameterMap mAudioProcessorParameters;

    juce::AudioProcessorGraph::NodeID mAudioProcessorGraphNodeID;

};

} // maqam

#endif // AUDIONODE_H
