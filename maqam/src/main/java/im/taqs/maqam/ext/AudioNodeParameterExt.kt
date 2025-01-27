//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.AudioNode
import im.taqs.maqam.impl.AudioNodeParameter
import im.taqs.maqam.impl.AudioNodeProperty
import im.taqs.maqam.impl.Variant
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.map

var AudioNodeParameter.booleanValue: Boolean
    get() = this.value != 0f
    set(value) { this.value = if (value) 1f else 0f }

fun AudioNodeParameter.booleanValueFlow(): Flow<Boolean> = valueFlow().map { it != 0f }

fun AudioNodeParameter.valueFlow(): Flow<Float> =
    callbackFlow {
        val listener = object : AudioNode.Listener {
            override fun onAudioNodePropertyValueChanged(node: AudioNode,
                                                         property: AudioNodeProperty,
                                                         value: Variant
            ) {
                value.f?.let {
                    if (property == this@valueFlow) {
                        trySend(it)
                    }
                }
            }
        }

        node?.addListener(listener)

        awaitClose {
            node?.removeListener(listener)
        }
    }
