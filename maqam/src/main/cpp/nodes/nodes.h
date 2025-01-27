//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

#ifndef NODES_H
#define NODES_H

#include "impl/AudioNode.h"

#include "test_tone/SineWaveAudioProcessor.h"
#include "ak_sampler/AKSamplerProcessorEx.h"
#include "sc_reverb/SCReverbProcessor.h"
#include "filter/FilterProcessor.h"
#include "delay/DelayProcessor.h"

#define NODE_JAVA_PACKAGE "im.taqs.maqam.node"

namespace maqam {

template<class T>
constexpr auto bind = &AudioNode::bindDSPClass<T>;

void bindNodeClasses()
{
    bind<SineWaveAudioProcessor>(NODE_JAVA_PACKAGE ".TestTone");
    bind<AKSamplerProcessorEx>(NODE_JAVA_PACKAGE ".AKSampler");
    bind<SCReverbProcessor>(NODE_JAVA_PACKAGE ".SCReverb");
    bind<FilterProcessor>(NODE_JAVA_PACKAGE ".Filter");
    bind<DelayProcessor>(NODE_JAVA_PACKAGE ".Delay");
}

} // maqam

#endif // NODES_H
