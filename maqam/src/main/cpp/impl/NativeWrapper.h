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
                          ImplDeleterFunction deleter, const std::string& field = kDefaultImpl)
    {
        std::string key = javaClassName + "_" + field;
        sImplFactory[key] = factory;
        sImplDeleter[key] = deleter;
    }

    template<class T>
    static void bindClass(const std::string& javaClassName, const std::string& field = kDefaultImpl)
    {
        std::string key = javaClassName + "_" + field;
        sImplFactory[key] = []() -> void* { return new T(); };
        sImplDeleter[key] = [](void* ptr) { delete reinterpret_cast<T*>(ptr); };
    }

    static void createImpl(JNIEnv *env, jobject thiz, const char* field = kDefaultImpl);
    static void deleteImpl(JNIEnv *env, jobject thiz, const char* field = kDefaultImpl);

    template<class T>
    static T* getImpl(JNIEnv *env, jobject thiz, const char* field = kDefaultImpl)
    {
        if (thiz == nullptr) {
            return nullptr;
        }

        const jlong impl = env->GetLongField(thiz, env->GetFieldID(env->GetObjectClass(thiz),
                                                                   field, "J"));
        return reinterpret_cast<T*>(impl);
    }

private:
    static constexpr const char* kDefaultImpl = "impl";

    static std::string getClassName(JNIEnv *env, jclass clazz);

    using ImplFactoryMap = std::map<std::string, ImplFactoryFunction>;
    static ImplFactoryMap sImplFactory;

    using ImplDeleterMap = std::map<std::string, ImplDeleterFunction>;
    static ImplDeleterMap sImplDeleter;

};

} // maqam

#endif // NATIVEWRAPPER_H