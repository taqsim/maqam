package im.taqs.maqam.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import im.taqs.maqam.AudioGraph
import im.taqs.maqam.AudioRoot
import im.taqs.maqam.Library
import im.taqs.maqam.example.ui.theme.MaqamTheme
import im.taqs.maqam.node.TestTone

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Library.init(this)

        val audio = AudioRoot(this)

        audio.graph = AudioGraph.Builder()
            .add("test_tone", TestTone())
            .build()

        audio.start()

        setContent {
            MaqamTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Greeting("Android")
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    MaqamTheme {
        Greeting("Android")
    }
}