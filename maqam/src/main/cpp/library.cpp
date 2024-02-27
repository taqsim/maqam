//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#include <jni.h>
#include <juce_core/juce_core.h>
#include <juce_core/native/juce_JNIHelpers_android.h>
#include <juce_events/juce_events.h>

#include "impl/AudioRoot.h"
#include "impl/AudioGraph.h"
#include "impl/NativeWrapper.h"
#include "nodes/nodes.h"
#include "client/maqam.h"

#define LIBRARY_JAVA_PACKAGE "im.taqs.maqam"

using namespace maqam;

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_LibraryKt_jniInit(JNIEnv *env, jclass /*clazz*/, jobject context)
{
    // Bind Java to native library classes
    NativeWrapper::bindClass<AudioRoot>(LIBRARY_JAVA_PACKAGE ".AudioRoot");
    NativeWrapper::bindClass<AudioGraph>(LIBRARY_JAVA_PACKAGE ".AudioGraph");
    NativeWrapper::bindClass<AudioNode>(LIBRARY_JAVA_PACKAGE ".AudioNode");

    // Bind Java to native node classes
    bindNodeClasses();

    // Init JUCE for a non-Projucer project
    // https://atsushieno.github.io/2021/01/16/juce-cmake-android-now-works.html
    // https://forum.juce.com/t/jmethodid-was-null-error-when-calling-devicemanager-initialisewithdefaultdevices/53506/9
    // JUCE/modules/juce_core/native/juce_android_Threads.cpp : juce_JavainitialiseJUCE()
    juce::JNIClassBase::initialiseAllClasses(env, context);
    juce::Thread::initialiseJUCE(env, context);

    // Avoid JUCE_ASSERT_MESSAGE_MANAGER_EXISTS when trying to instantiate an AudioProcessorGraph
    juce::MessageManager::getInstance();
}

extern "C"
void _maqam_bind_dsp_class(const char* name, maqam_impl_factory_func_t factory,
                           maqam_impl_deleter_func_t deleter)
{
    AudioNode::bindDSPClass(name, factory, deleter);
}
