package im.taqs.maqam.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Divider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import im.taqs.maqam.AudioGraph
import im.taqs.maqam.AudioRoot
import im.taqs.maqam.Library
import im.taqs.maqam.example.ui.theme.MaqamTheme
import im.taqs.maqam.ext.eventsFlow
import im.taqs.maqam.ext.valueFlow
import im.taqs.maqam.impl.AudioNodeParameter
import im.taqs.maqam.impl.Midi
import im.taqs.maqam.impl.MidiEvent
import im.taqs.maqam.node.TestTone
import kotlinx.coroutines.flow.MutableStateFlow
import kotlin.math.roundToInt

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Library.init(this)

        val audio = AudioRoot(this)

        audio.graph = AudioGraph.Builder()
            .add(tag = DemoConstants.TestToneNodeTag, node = TestTone())
            .build()

        audio.start()

        setContent {
            MaqamTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Demo(
                        audio = audio
                    )
                }
            }
        }
    }
}

@Composable
fun Demo(modifier: Modifier = Modifier, audio: AudioRoot?) {
    Column(
        modifier = modifier
    ) {
        FrequencySlider(
            parameter = (audio?.graph?.get(DemoConstants.TestToneNodeTag) as? TestTone)
                ?.sineWaveFrequency
        )
        Divider()
        MidiEventLog(
            midi = audio?.midi
        )
    }
}

@Composable
fun FrequencySlider(modifier: Modifier = Modifier, parameter: AudioNodeParameter?) {
    val frequency by (
            /*app*/ parameter?.let {
                        it.valueFlow().collectAsStateWithLifecycle(initialValue = it.value)
                    } ?:
        /*preview*/ MutableStateFlow(DemoConstants.PreviewFrequencyValue)
                        .collectAsStateWithLifecycle()
    )

    val range = parameter?.valueRange ?: DemoConstants.PreviewFrequencyRange

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
                parameter?.value = it
            }
        )
    }
}

@Composable
fun MidiEventLog(modifier: Modifier = Modifier, midi: Midi?) {
    val midiEvent by (
            /*app*/ midi?.eventsFlow()
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

private object DemoConstants {

    const val TestToneNodeTag = "test_tone"

    const val PreviewFrequencyValue = 440f
    val PreviewFrequencyRange = 220f..880f

    val PreviewMidiEvent = MidiEvent.NoteOn(channel = 0, note = 69, velocity = 127)

}

@Preview(showBackground = true)
@Composable
fun DemoPreview() {
    MaqamTheme {
        Demo(
            audio = null
        )
    }
}
