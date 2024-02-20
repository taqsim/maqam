//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.AudioNode
import im.taqs.maqam.impl.AudioNodeProperty
import im.taqs.maqam.impl.AudioNodeState
import im.taqs.maqam.impl.Variant
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow

fun AudioNodeState.valueFlow(): Flow<Variant> =
    callbackFlow {
        val listener = object : AudioNode.Listener {
            override fun onAudioNodePropertyValueChanged(node: AudioNode,
                                                         property: AudioNodeProperty,
                                                         value: Variant
            ) {
                if (property == this@valueFlow) {
                    trySend(value)
                }
            }
        }

        node?.addListener(listener)

        awaitClose {
            node?.removeListener(listener)
        }
    }
