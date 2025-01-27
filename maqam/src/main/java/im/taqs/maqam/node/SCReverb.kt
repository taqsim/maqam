//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.AudioNode

class SCReverb: AudioNode(), BypassControl, MixControl {

    override val bypass      = parameter("bypass")
    override val mix         = parameter("mix")
    val feedback             = parameter("feedback")
    val lowPassFilterCutoff  = parameter("lp_filter_cutoff")
    val highPassFilterCutoff = parameter("hp_filter_cutoff")

}
