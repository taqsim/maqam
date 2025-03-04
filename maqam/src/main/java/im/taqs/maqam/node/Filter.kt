//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.AudioNode

class Filter: AudioNode(), BypassControl, MixControl {

    override val bypass = parameter("bypass")
    override val mix    = parameter("mix")
    val cutoff          = parameter("cutoff")
    val resonance       = parameter("resonance")
    val lfoAmplitude    = parameter("lfo_amplitude")
    val lfoRate         = parameter("lfo_rate")

}
