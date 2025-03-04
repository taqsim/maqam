//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include <jni.h>

#include "impl/AudioNode.h"
#include "AKSamplerProcessorEx.h"

using namespace maqam;

#define GET_DSP(e,t) (*reinterpret_cast<AKSamplerProcessorEx*>(AudioNode::getDSP(e,t)))

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_node_AKSampler_load(JNIEnv *env, jobject thiz, jstring path)
{
    const char* cPath = env->GetStringUTFChars(path, nullptr);

    try {
        GET_DSP(env, thiz).load(cPath);
    } catch (const std::exception& e) {
        env->ThrowNew(env->FindClass("im/taqs/maqam/Library$Exception"), e.what());
    }

    env->ReleaseStringUTFChars(path, cPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_node_AKSampler_stopAllVoices(JNIEnv *env, jobject thiz)
{
    GET_DSP(env, thiz).stopAllVoices();
}

extern "C"
JNIEXPORT jint JNICALL
Java_im_taqs_maqam_node_AKSampler_jniGetMidiChannel(JNIEnv *env, jobject thiz)
{
    return GET_DSP(env, thiz).getMidiChannel();
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_node_AKSampler_jniSetMidiChannel(JNIEnv *env, jobject thiz,
                                                         jint midi_channel)
{
    GET_DSP(env, thiz).setMidiChannel(midi_channel);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_node_AKSampler_setA4Frequency(JNIEnv *env, jobject thiz, jfloat frequency)
{
    GET_DSP(env, thiz).setA4Frequency(frequency);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_node_AKSampler_setScaleTuning(JNIEnv *env, jobject thiz, jintArray cents_from_c)
{
    if (env->GetArrayLength(cents_from_c) != 12) {
        env->ThrowNew(env->FindClass("im/taqs/maqam/Library$Exception"), "array size must be 12");
        return;
    }

    jboolean isCopy;
    jint* jcents = env->GetIntArrayElements(cents_from_c, &isCopy);

    std::array<int,12> cppCents {};

    for (int i = 0; i < 12; i++) {
        cppCents[i] = jcents[i];
    }

    GET_DSP(env, thiz).setScaleCents(cppCents);

    env->ReleaseIntArrayElements(cents_from_c, jcents, 0);
}
