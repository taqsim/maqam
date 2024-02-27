package im.taqs.maqam.example

import im.taqs.maqam.impl.MidiEvent

object DemoConstants {

    const val TestToneNodeTag = "test_tone"

    const val PreviewFrequencyValue = 440f

    val PreviewFrequencyRange = 220f..880f
    val PreviewMidiEvent = MidiEvent.NoteOn(channel = 0, note = 69, velocity = 127)

}
