//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.AudioNode

class TestTone: AudioNode() {

    val sineWaveFrequency = parameter("sine_wave_frequency")

}
