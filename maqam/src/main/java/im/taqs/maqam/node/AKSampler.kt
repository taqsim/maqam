//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.node

import im.taqs.maqam.Library
import im.taqs.maqam.ext.fromNormalizedValue
import im.taqs.maqam.impl.MidiEvent
import im.taqs.maqam.impl.MidiEvent.ControlChange.Companion.MODULATION_WHEEL

// Wraps an AKSampler instance with custom TAQS.IM patches (AKSamplerProcessorEx)
open class AKSampler(midiChannel: Int = 0) : SFZPlayer(midiChannel) {

    val mainMasterLevel               = parameter("main_master_level")
    val mainPitchBendUpSemitones      = parameter("main_pitchbend_up_semitones")
    val mainPitchBendDownSemitones    = parameter("main_pitchbend_down_semitones")
    val mainAmpVelocitySensitivity    = parameter("main_amp_velocity_sensitivity")
    val mainFilterVelocitySensitivity = parameter("main_filter_velocity_sens")
    val osc1PitchOffsetSemitones      = parameter("osc1_pitch_offset_semitones")
    val osc1DetuneOffsetCents         = parameter("osc1_detune_offset_cents")
    val filterStages                  = parameter("filter_stages")
    val filterCutoff                  = parameter("filter_cutoff")
    val filterResonance               = parameter("filter_resonance")
    val filterEnvAmount               = parameter("filter_env_amount")
    val ampEGBypass                   = parameter("ampeg_bypass")
    val ampEGAttackTimeSeconds        = parameter("ampeg_attack_time_seconds")
    val ampEGDecayTimeSeconds         = parameter("ampeg_decay_time_seconds")
    val ampEGSustainLevel             = parameter("ampeg_sustain_level")
    val ampEGReleaseTimeSeconds       = parameter("ampeg_release_time_seconds")
    val filterEGAttackTimeSeconds     = parameter("filtereg_attack_time_seconds")
    val filterEGDecayTimeSeconds      = parameter("filtereg_decay_time_seconds")
    val filterEGSustainLevel          = parameter("filtereg_sustain_level")
    val filterEGReleaseTimeSeconds    = parameter("filtereg_release_time_seconds")

    override val gain get() = mainMasterLevel
    override val envelopeBypass get() = ampEGBypass
    override val envelopeAttack get() = ampEGAttackTimeSeconds
    override val envelopeDecay get() = ampEGDecayTimeSeconds
    override val envelopeSustain get() = ampEGSustainLevel
    override val envelopeRelease get() = ampEGReleaseTimeSeconds

    override var midiChannel: Int
        get() = jniGetMidiChannel()
        set(value) = jniSetMidiChannel(value)

    init {
        if (Library.hasJNI) {
            jniSetMidiChannel(midiChannel)
        }
    }

    override fun noteOn(note: Int, velocity: Float) {
       queueMidiEvent(MidiEvent.NoteOn(
            channel = midiChannel.toByte(),
            note = note.toByte(),
            velocity = (velocity * 0x7f).toInt().toByte()
        ))
    }

    override fun noteOff(note: Int, velocity: Float) {
        queueMidiEvent(MidiEvent.NoteOff(
            channel = midiChannel.toByte(),
            note = note.toByte(),
            velocity = (velocity * 0x7f).toInt().toByte()
        ))
    }

    override fun pitchBend(value: Float) {
        queueMidiEvent(MidiEvent.PitchBend.fromNormalizedValue(
            channel = midiChannel.toByte(),
            value = value
        ))
    }

    override fun modulation(value: Float) {
        queueMidiEvent(MidiEvent.ControlChange(
            channel = midiChannel.toByte(),
            controller = MODULATION_WHEEL,
            value = (value * 0x7f).toInt().toByte()
        ))
    }

    external override fun load(path: String)
    external override fun stopAllVoices()
    external override fun setA4Frequency(frequency: Float)
    external override fun setScaleTuning(centsFromC: IntArray)

    private external fun jniGetMidiChannel(): Int
    private external fun jniSetMidiChannel(midiChannel: Int)

}
