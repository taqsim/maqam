//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#ifndef MAQAM_H
#define MAQAM_H

#include <jni.h>

typedef struct priv_maqam_t maqam_t;

typedef void* (*maqam_impl_factory_func_t)();
typedef void  (*maqam_impl_deleter_func_t)(void*);

maqam_t* maqam_create();
void     maqam_destroy(maqam_t*);
void     maqam_bind_dsp_class(maqam_t*, const char*, maqam_impl_factory_func_t,
                              maqam_impl_deleter_func_t);
void*    maqam_get_dsp(JNIEnv*, jobject);

#endif // MAQAM_H