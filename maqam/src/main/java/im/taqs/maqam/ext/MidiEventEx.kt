//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.impl.MidiEvent

fun MidiEvent.PitchBend.Companion.fromNormalizedValue(channel: Byte, value: Float) =
    MidiEvent.PitchBend(
        channel = channel,
        value = (value * 0x4000.toFloat()).toUInt()
    )
