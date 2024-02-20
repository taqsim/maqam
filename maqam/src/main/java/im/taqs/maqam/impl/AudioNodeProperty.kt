//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

import im.taqs.maqam.AudioNode
import im.taqs.maqam.Library

open class AudioNodeProperty internal constructor(
    internal val key: String,
    internal val isSerializable: Boolean,
    private val callback: Callback
) {
    internal interface Callback {
        fun getAudioNode(): AudioNode? = null
    }

    internal val node get() = callback.getAudioNode()

    internal open var serializableValue: Variant
        get()  = throw Library.Exception("Getter for serializableValue not implemented")
        set(_) = throw Library.Exception("Setter for serializableValue not implemented")

    override fun equals(other: Any?): Boolean = (other as AudioNodeProperty).key == key
    override fun toString(): String = key
    override fun hashCode(): Int = key.hashCode()

}
