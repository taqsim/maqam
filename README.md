Maqam
-----
*Mobile App Quick Audio &amp; MIDI*

Maqam is a library that facilitates the creation of audio software on the
Android platform. It leverages the full power of [JUCE](https://juce.com) for the
DSP aspect while leaving the choice of an UI framework up to the programmer.
Since Android lacks an official audio plugin architecture like AudioUnit on iOS,
its primary use case are standalone applications that generate sound like for
example synthesizers.

It is designed from scratch to comply with the latest Android coding practices.
One important goal is to be Compose friendly.

Maqam is developed by [TAQS.IM](https://taqs.im) and is distributed under a
friendly MIT license.

Usage
-----

Stack initialization

```Kotlin
Library.init(this) // loads the dynamic link library

val audio = AudioRoot(this)
```

Creating an audio graph is done following the builder pattern

```Kotlin
val testTone = TestTone()

audio.graph = AudioGraph.Builder()
    .add(testTone)
    .build()
```

Finally audio is started

```Kotlin
audio.start()
```

Importing [this extension](https://github.com/taqsim/maqam/blob/master/maqam/src/main/java/im/taqs/maqam/ext/AudioNodeParameterExt.kt)
exposes node parameters as [flows](https://developer.android.com/kotlin/flow),
so creating a slider composable that controls the test tone frequency in the
above example is simple:

```Kotlin
val frequency by testTone.sineWaveFrequency.valueFlow().collectAsStateWithLifecycle(0f)

Column {
    Text(
        text = "${frequency.roundToInt()} Hz"
    )
    Slider(
        value = frequency,
        valueRange = testTone.sineWaveFrequency.valueRange,
        onValueChange = {
            testTone.sineWaveFrequency.value = it
        }
    )
}
```

Links
-----

A complete example is available [here](https://github.com/taqsim/maqam/tree/master/example/src/main/java/im/taqs/maqam/example)

For a real world app implementing Maqam check our flagship Android app [World
Piano](https://play.google.com/store/apps/details?id=com.taqsim.worldsynth) at the
Google Play Store

A similar project named [AudioKit](https://github.com/AudioKit/AudioKit) is
available for iOS
