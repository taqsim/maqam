//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.AudioNode
import im.taqs.maqam.ext.midiEventsFlow
import im.taqs.maqam.impl.AudioNodeParameter

abstract class SFZPlayer(open var midiChannel: Int = 0): AudioNode(), GainControl, EnvelopeControl {

    abstract val transposeSemitones: AudioNodeParameter

    abstract fun load(path: String)
    abstract fun stopAllVoices()

    abstract fun setA4Frequency(frequency: Float)
    abstract fun setScaleTuning(centsFromC: IntArray)

    abstract fun noteOn(note: Int, velocity: Float = 1f)
    abstract fun noteOff(note: Int, velocity: Float = 1f)
    abstract fun pitchBend(value: Float)
    abstract fun modulation(value: Float)

    fun midiEventsFlow() = midiEventsFlow(midiChannel.toByte())

}
