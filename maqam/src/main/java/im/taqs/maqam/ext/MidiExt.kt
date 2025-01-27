//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.impl.Midi
import im.taqs.maqam.impl.MidiEvent
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow

typealias MidiPortList = List<Midi.Port>

fun Midi.portsFlow(): Flow<MidiPortList> =
    callbackFlow {
        val listener = object : Midi.Listener {
            override fun onMidiPortsChange(ports: List<Midi.Port>) {
                trySend(ports)
            }
            override fun onMidiPortOpen(port: Midi.Port) {
                trySend(ports)
            }
            override fun onMidiPortClose(port: Midi.Port) {
                trySend(ports)
            }
        }

        addListener(listener)

        awaitClose {
            removeListener(listener)
        }
    }

fun Midi.eventsFlow(channel: Byte? = null): Flow<MidiEvent> =
    callbackFlow {
        val listener = object : Midi.Listener {
            override fun onMidiEvent(event: MidiEvent, port: Midi.Port) {
                if ((channel == event.channel) || (channel == null)) {
                    trySend(event)
                }
            }
        }

        addListener(listener)

        awaitClose {
            removeListener(listener)
        }
    }
