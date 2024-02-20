//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#include <cassert>

#include "NativeWrapper.h"

using namespace maqam;

NativeWrapper::ImplFactoryMap NativeWrapper::sImplFactory;
NativeWrapper::ImplDeleterMap NativeWrapper::sImplDeleter;

// https://stackoverflow.com/questions/61870844/how-to-obtain-the-name-of-a-java-class-from-its-corresponding-jclass
std::string NativeWrapper::getClassName(JNIEnv *env, /*Class<T>*/jclass clazz)
{
    jclass cls = env->GetObjectClass(clazz); // Class<Class>
    jmethodID mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
    auto jClsName = reinterpret_cast<jstring>(env->CallObjectMethod(clazz, mid));
    const char* cClsName = env->GetStringUTFChars(jClsName, nullptr);
    std::string clsName (cClsName);
    env->ReleaseStringUTFChars(jClsName, cClsName);
    return clsName;
}

void NativeWrapper::createImpl(JNIEnv *env, jobject thiz)
{
    ImplFactoryFunction factory = nullptr;
    jclass cls = env->GetObjectClass(thiz);

    while (factory == nullptr) {
        factory = sImplFactory[getClassName(env, cls)];

        if (factory == nullptr) {
            cls = env->GetSuperclass(cls);
            assert(cls != nullptr); // reached java.lang.Object -- forgot to register class?
        }
    }

    const auto impl = reinterpret_cast<jlong>(factory());
    env->SetLongField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "impl", "J"), impl);
}

void NativeWrapper::deleteImpl(JNIEnv *env, jobject thiz)
{
    ImplDeleterFunction deleter = nullptr;
    jclass cls = env->GetObjectClass(thiz);

    while (deleter == nullptr) {
        deleter = sImplDeleter[getClassName(env, cls)];

        if (deleter == nullptr) {
            cls = env->GetSuperclass(cls);
            assert(cls != nullptr);
        }
    }

    deleter(NativeWrapper::getImpl<void*>(env, thiz));
    env->SetLongField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "impl", "J"), 0LL);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_impl_NativeWrapper_jniCreateImpl(JNIEnv *env, jobject thiz)
{
    NativeWrapper::createImpl(env, thiz);
}

extern "C"
JNIEXPORT void JNICALL
Java_im_taqs_maqam_impl_NativeWrapper_jniDestroyImpl(JNIEnv *env, jobject thiz)
{
    NativeWrapper::deleteImpl(env, thiz);
}
