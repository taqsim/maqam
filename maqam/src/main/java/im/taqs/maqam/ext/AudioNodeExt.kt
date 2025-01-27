//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.AudioNode

fun AudioNode.midiEventsFlow(channel: Byte? = null) = midi.eventsFlow(channel)
