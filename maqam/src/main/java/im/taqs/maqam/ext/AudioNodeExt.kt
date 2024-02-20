//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.AudioNode

fun AudioNode.midiEventsFlow(channel: Byte? = null) = midi.eventsFlow(channel)
