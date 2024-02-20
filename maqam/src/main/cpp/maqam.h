//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef MAQAM_H
#define MAQAM_H

typedef void* (*ImplFactoryFunction)();
typedef void  (*ImplDeleterFunction)(void*);

extern "C" {

void maqam_bind_class(const char* name, ImplFactoryFunction factory, ImplDeleterFunction deleter);

}

#endif // MAQAM_H