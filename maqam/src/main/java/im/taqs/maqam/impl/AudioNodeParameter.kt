//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

class AudioNodeParameter internal constructor(
    key: String,
    isSerializable: Boolean,
    private val callback: Callback
) : AudioNodeProperty(key, isSerializable, callback) {

    internal interface Callback : AudioNodeProperty.Callback {
        fun getAudioNodeParameterName(parameter: AudioNodeParameter): String = ""
        fun getAudioNodeParameterUnit(parameter: AudioNodeParameter): String = ""
        fun getAudioNodeParameterValue(parameter: AudioNodeParameter): Float = 0f
        fun setAudioNodeParameterValue(parameter: AudioNodeParameter, value: Float) {}
        fun getAudioNodeParameterValueAsText(parameter: AudioNodeParameter): String = ""
        fun getAudioNodeParameterValueRange(parameter: AudioNodeParameter)
            : ClosedFloatingPointRange<Float> = 0f..0f
    }

    val name get() = callback.getAudioNodeParameterName(this)
    val unit get() = callback.getAudioNodeParameterUnit(this)

    var value: Float
        get() = callback.getAudioNodeParameterValue(this)
        set(value) { callback.setAudioNodeParameterValue(this, value) }

    val valueAsText get() = callback.getAudioNodeParameterValueAsText(this)

    val valueRange: ClosedFloatingPointRange<Float>
        get() = callback.getAudioNodeParameterValueRange(this)

    override var serializableValue: Variant
        get() = Variant(value)
        set(newValue) { newValue.f?.let { value = it } }

}
