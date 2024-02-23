//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef NATIVEWRAPPER_H
#define NATIVEWRAPPER_H

#include <functional>
#include <map>
#include <string>
#include <utility>

#include <jni.h>

namespace maqam {

class NativeWrapper
{
public:
    using ImplFactoryFunction = void*(*)();
    using ImplDeleterFunction = void(*)(void*);

    static void bindClass(const std::string& javaClassName, ImplFactoryFunction factory,
                          ImplDeleterFunction deleter)
    {
        sImplFactory[javaClassName] = factory;
        sImplDeleter[javaClassName] = deleter;
    }

    template<class T>
    static void bindClass(const std::string& javaClassName)
    {
        sImplFactory[javaClassName] = &createImpl<T>;
        sImplDeleter[javaClassName] = &deleteImpl<T>;
    }

    static void createImpl(JNIEnv *env, jobject thiz);
    static void deleteImpl(JNIEnv *env, jobject thiz);

    template<class T>
    static T* getImpl(JNIEnv *env, jobject thiz)
    {
        return getPointer<T>(env, thiz, "impl");
    }

    template<class T>
    static T* getPointer(JNIEnv *env, jobject thiz, const char* field)
    {
        if (thiz == nullptr) {
            return nullptr;
        }

        const jlong impl = env->GetLongField(thiz, env->GetFieldID(env->GetObjectClass(thiz),
                                                                   field, "J"));
        return reinterpret_cast<T*>(impl);
    }

private:
    static std::string getClassName(JNIEnv *env, jclass clazz);

    template<class T>
    static void* createImpl()
    {
        return new T;
    }

    template<class T>
    static void deleteImpl(void* impl)
    {
        delete reinterpret_cast<T*>(impl);
    }

    using ImplFactoryMap = std::map<std::string, ImplFactoryFunction>;
    static ImplFactoryMap sImplFactory;

    using ImplDeleterMap = std::map<std::string, ImplDeleterFunction>;
    static ImplDeleterMap sImplDeleter;

};

} // maqam

#endif // NATIVEWRAPPER_H