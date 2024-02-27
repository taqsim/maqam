package im.taqs.maqam.example

import androidx.compose.foundation.layout.Column
import androidx.compose.material3.Divider
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import im.taqs.maqam.example.ui.theme.MaqamTheme
import im.taqs.maqam.ext.eventsFlow
import im.taqs.maqam.ext.valueFlow
import im.taqs.maqam.node.TestTone
import kotlinx.coroutines.flow.MutableStateFlow
import kotlin.math.roundToInt

@Composable
fun Demo(modifier: Modifier = Modifier) {
    Column(
        modifier = modifier
    ) {
        FrequencySlider()
        Divider()
        MidiEventLog()
    }
}

@Composable
fun FrequencySlider(modifier: Modifier = Modifier) {
    val testTone = AudioApplication.audio?.graph?.get(DemoConstants.TestToneNodeTag) as? TestTone
    val sineWaveFrequency = testTone?.sineWaveFrequency

    val frequency by (
            /*app*/ sineWaveFrequency?.let {
                        it.valueFlow().collectAsStateWithLifecycle(initialValue = it.value)
                    } ?:
        /*preview*/ MutableStateFlow(DemoConstants.PreviewFrequencyValue)
                        .collectAsStateWithLifecycle()
    )

    val range = sineWaveFrequency?.valueRange ?: DemoConstants.PreviewFrequencyRange

    Column(
        modifier = modifier
    ) {
        Text(
            text = "Test tone frequency ${frequency.roundToInt()} Hz"
        )
        Slider(
            value = frequency,
            valueRange = range,
            onValueChange = {
                sineWaveFrequency?.value = it
            }
        )
    }
}

@Composable
fun MidiEventLog(modifier: Modifier = Modifier) {
    val midiEvent by (
            /*app*/ AudioApplication.audio?.midi?.eventsFlow()
                    ?:
        /*preview*/ MutableStateFlow(DemoConstants.PreviewMidiEvent)
    ).collectAsStateWithLifecycle(initialValue = null)

    Column(
        modifier = modifier
    ) {
        Text(
            text = "Last received MIDI event"
        )
        Text(
            text = midiEvent?.toString() ?: "none"
        )
    }
}

@Preview(showBackground = true)
@Composable
fun DemoPreview() {
    MaqamTheme {
        Demo()
    }
}
