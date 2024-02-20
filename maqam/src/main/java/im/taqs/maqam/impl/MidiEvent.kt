//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

import kotlin.experimental.and
import kotlin.experimental.or

// https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message

open class MidiEvent {

    companion object {
        fun fromBytes(bytes: ByteArray): MidiEvent? = when (bytes[0].and(0xF0.toByte())) {
            NoteOff.STATUS -> NoteOff(bytes)
            NoteOn.STATUS -> NoteOn(bytes)
            ControlChange.STATUS -> ControlChange(bytes)
            ProgramChange.STATUS -> ProgramChange(bytes)
            PitchBend.STATUS -> PitchBend(bytes)
            else -> null
        }
    }

    val bytes: ByteArray

    val status: Byte
        get() = bytes[0].and(0xF0.toByte())

    val channel: Byte
        get() = bytes[0].and(0x0F.toByte())

    internal constructor(bytes: ByteArray) {
        this.bytes = bytes
    }

    internal constructor(status: Byte, channel: Byte, data1: Byte, data2: Byte? = null) {
        bytes = ByteArray(if (data2 != null) 3 else 2)
        bytes[0] = status.or(channel)
        bytes[1] = data1
        data2?.let { bytes[2] = it }
    }

    override fun toString(): String {
        return "MidiEvent(bytes=$bytes)"
    }

    class NoteOff : MidiEvent {

        companion object {
            const val STATUS = 0x80.toByte()
        }

        val note: Byte
            get() = bytes[1]

        val velocity: Byte
            get() = bytes[2]

        constructor(channel: Byte, note: Byte, velocity: Byte)
            : super(STATUS, channel, note, velocity)

        internal constructor(bytes: ByteArray) : super(bytes)

        override fun toString(): String {
            return "NoteOff(channel=$channel note=$note velocity=$velocity)"
        }

    }

    class NoteOn : MidiEvent {

        companion object {
            const val STATUS = 0x90.toByte()
        }

        val note: Byte
            get() = bytes[1]

        val velocity: Byte
            get() = bytes[2]

        constructor(channel: Byte, note: Byte, velocity: Byte)
                : super(STATUS, channel, note, velocity)

        internal constructor(bytes: ByteArray) : super(bytes)

        override fun toString(): String {
            return "NoteOn(channel=$channel note=$note velocity=$velocity)"
        }

    }

    class ControlChange : MidiEvent {

        companion object {
            const val STATUS = 0xB0.toByte()

            const val BANK_SELECT_MSB = 0x00.toByte()
            const val MODULATION_WHEEL = 0x01.toByte()
            const val BANK_SELECT_LSB = 0x20.toByte()
            const val ALL_SOUND_OFF = 0x78.toByte()
        }

        val controller: Byte
            get() = bytes[1]

        val value: Byte
            get() = bytes[2]

        constructor(channel: Byte, controller: Byte, value: Byte)
                : super(STATUS, channel, controller, value)

        internal constructor(bytes: ByteArray) : super(bytes)

        override fun toString(): String {
            return "ControlChange(channel=$channel controller=$controller value=$value)"
        }

    }

    class ProgramChange : MidiEvent {

        companion object {
            const val STATUS = 0xC0.toByte()
        }

        val value: Byte
            get() = bytes[1]

        constructor(channel: Byte, value: Byte) : super(
            STATUS,
            channel,
            value
        )

        internal constructor(bytes: ByteArray) : super(bytes)

        override fun toString(): String {
            return "ProgramChange(channel=$channel value=$value)"
        }

    }

    class PitchBend : MidiEvent {

        companion object {
            const val STATUS = 0xE0.toByte()
        }

        val value: UInt
            get() = bytes[2].toUByte().toUInt().shl(7)
                .or(bytes[1].toUByte().toUInt())

        constructor(channel: Byte, value: UInt) : super(
            STATUS,
            channel,
            value.and(0x7Fu).toByte(), // LSB (7)
            value.shr(7).toByte()   // MSB (7)
        )

        internal constructor(bytes: ByteArray) : super(bytes)

        override fun toString(): String {
            return "PitchBend(channel=$channel value=$value)"
        }

    }

}
