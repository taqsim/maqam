//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.AudioNode

class Delay: AudioNode(), BypassControl, MixControl {

    override val bypass = parameter("bypass")
    override val mix    = parameter("mix")
    val feedback        = parameter("feedback")
    val time            = parameter("time") // seconds

}
