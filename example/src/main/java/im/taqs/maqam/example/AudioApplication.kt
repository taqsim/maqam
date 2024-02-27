package im.taqs.maqam.example

import android.app.Application
import im.taqs.maqam.AudioGraph
import im.taqs.maqam.AudioRoot
import im.taqs.maqam.Library
import im.taqs.maqam.node.TestTone

class AudioApplication : Application() {

    override fun onCreate() {
        super.onCreate()

        Library.init(this)

        val audio = AudioRoot(this)

        audio.graph = AudioGraph.Builder()
            .add(tag = DemoConstants.TestToneNodeTag, node = TestTone())
            .build()

        audio.start()

        AudioApplication.audio = audio
    }

    companion object {
        var audio: AudioRoot? = null
    }

}
