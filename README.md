Maqam
-----
*Mobile App Quick Audio &amp; MIDI*

Maqam is a graph-based audio processing library for the Android platform.
It leverages [JUCE](https://juce.com) for DSP development while leaving the
choice of UI frameworks up to the programmer.

It is developed by [TAQS.IM](https://taqs.im) and distributed under a friendly
MIT license.

Features
--------

- Prebuilt nodes: SFZ player, reverb, delay, low pass filter
- Support for custom nodes
- Compose friendly
- Automatic state persistence

Architecture
------------

The Kotlin `AudioGraph` and `AudioNode` classes are backed by `juce::AudioGraph`
and `juce::AudioProcessor` respectively. JUCE is only used for DSP,
by design JUCE is used as little as possible. All I/O is managed by Maqam through
the standard [Oboe](https://github.com/google/oboe) and [AMidi](https://developer.android.com/ndk/guides/audio/midi)
Android libraries and bridged to a JUCE AudioGraph instance, which in turns
manages the individual AudioProcessor instances. This way JUCE does not impose
any kind of restriction over the native Android app, it is just a part of it.

The library deliberately does not an attempt to map JUCE interfaces one to one.
It uses JUCE to implement some aspects of iOS' AVAudioEngine while not reinventing
the wheel.

Since Android lacks an official audio plugin architecture like AudioUnit on iOS,
Maqam's primary use case are standalone applications that generate sound, like for
example synthesizers.

Usage
-----

Initialization

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

Custom DSP
----------

Creating a custom node is done by extending the Kotlin `AudioNode` and
C++ `juce::AudioProcessor` classes. The former is a wrapper that handles
all JNI calls and the latter implements the actual DSP. Both must be linked
somehow, this is done by editing [nodes.h](https://github.com/taqsim/maqam/blob/master/maqam/src/main/cpp/nodes/nodes.h).
See [TestTone.kt](https://github.com/taqsim/maqam/blob/master/maqam/src/main/java/im/taqs/maqam/node/TestTone.kt)
for an example wrapper.

It is also possible to implement custom nodes outside the library namespace
by setting up NDK in the parent project and calling the functions defined in
[maqam.h](https://github.com/taqsim/maqam/blob/master/maqam/src/main/cpp/client/maqam.h)

Links
-----

A complete example is available [here](https://github.com/taqsim/maqam/tree/master/example/src/main/java/im/taqs/maqam/example)

For a real world app implementing Maqam check our flagship Android app [World
Piano](https://play.google.com/store/apps/details?id=com.taqsim.worldsynth) at the
Google Play Store

A similar project named [AudioKit](https://github.com/AudioKit/AudioKit) is
available for iOS
