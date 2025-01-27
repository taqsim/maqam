//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.impl.AudioNodeParameter

interface BypassControl {
    val bypass: AudioNodeParameter
}

interface GainControl {
    val gain: AudioNodeParameter
}

interface MixControl {
    val mix: AudioNodeParameter
}

interface EnvelopeControl {
    val envelopeBypass: AudioNodeParameter
    val envelopeAttack: AudioNodeParameter  // sec
    val envelopeDecay: AudioNodeParameter   // sec
    val envelopeSustain: AudioNodeParameter
    val envelopeRelease: AudioNodeParameter // sec
}
