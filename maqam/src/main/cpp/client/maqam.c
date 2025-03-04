//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#include <jni.h>

#include <dlfcn.h>
#include <stdlib.h>

#include "maqam.h"

typedef void(*maqam_bind_class_func_t)(const char*, maqam_impl_factory_func_t,
                                        maqam_impl_deleter_func_t);

struct priv_maqam_t {
    void* so;
};

maqam_t* maqam_create()
{
    maqam_t* maqam = (maqam_t*)malloc(sizeof(maqam_t));
    maqam->so = dlopen("libmaqam.so", RTLD_LAZY);
    return maqam;
}

void maqam_destroy(maqam_t* maqam)
{
    dlclose(maqam->so);
    free(maqam);
}

void maqam_bind_dsp_class(maqam_t* maqam, const char* name, maqam_impl_factory_func_t factory,
                          maqam_impl_deleter_func_t deleter)
{
    maqam_bind_class_func_t fp = (maqam_bind_class_func_t)dlsym(maqam->so, "_maqam_bind_dsp_class");
    fp(name, factory, deleter);
}

void* maqam_get_dsp(JNIEnv* env, jobject thiz)
{
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID field = (*env)->GetFieldID(env, clazz, "dsp", "J");
    return (void*)(*env)->GetLongField(env, thiz, field);
}
