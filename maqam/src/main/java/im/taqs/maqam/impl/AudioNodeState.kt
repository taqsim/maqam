//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

class AudioNodeState internal constructor(
    key: String,
    isSerializable: Boolean,
    private val callback: Callback,
    private val type: Type
) : AudioNodeProperty(key, isSerializable, callback) {

    internal interface Callback : AudioNodeProperty.Callback {
        fun getAudioNodeStateFloatValue(state: AudioNodeState): Float = 0f
        fun setAudioNodeStateFloatValue(state: AudioNodeState, value: Float) {}
        fun getAudioNodeStateStringValue(state: AudioNodeState): String = ""
        fun setAudioNodeStateStringValue(state: AudioNodeState, value: String) {}
    }

    enum class Type {
        FLOAT, STRING
    }

    var floatValue: Float
        get() = callback.getAudioNodeStateFloatValue(this)
        set(value) { callback.setAudioNodeStateFloatValue(this, value) }

    var stringValue: String
        get() = callback.getAudioNodeStateStringValue(this)
        set(value) { callback.setAudioNodeStateStringValue(this, value) }

    var value: Variant
        get() = when (type) {
            Type.FLOAT -> Variant(floatValue)
            Type.STRING -> Variant(stringValue)
        }
        set(newValue) {
            when (type) {
                Type.FLOAT -> newValue.f?.let { floatValue = it }
                Type.STRING -> newValue.s?.let { stringValue = it }
            }
        }

    override var serializableValue: Variant
        get() = value
        set(newValue) { value = newValue }

}
