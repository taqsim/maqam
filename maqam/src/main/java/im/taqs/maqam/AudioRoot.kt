//
// Maqam - Mobile App Quick Audio & MIDI
//
// SPDX-FileCopyrightText: 2024 TAQS.IM <contact@taqs.im>
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam

import android.content.Context
import android.media.midi.MidiDevice
import android.os.Handler
import android.os.Looper
import android.util.Log
import im.taqs.maqam.impl.AudioNodeMetadata
import im.taqs.maqam.impl.AudioNodeProperty
import im.taqs.maqam.impl.Midi
import im.taqs.maqam.impl.MidiPortOpenState
import im.taqs.maqam.impl.NativeWrapper
import im.taqs.maqam.impl.Variant
import kotlinx.serialization.Serializable
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import java.io.File

// Backed by a oboe::AudioStream instance and AMidi ports
class AudioRoot(context: Context, private val options: Options = Options())
    : NativeWrapper(), AudioGraph.Listener, Midi.Callback, Midi.Listener, AudioNode.Listener,
        AudioNodeMetadata.Callback {

    interface Listener {
        fun onAudioRootStarted() {}
        fun onAudioRootStopped() {}
    }

    data class Options(
        val stateFile: File? = null,
        val autoOpenMidiPorts: Boolean = true
    )

    var isStarted: Boolean = false
        private set

    var loadedStateFileOnStart = false
        private set

    private var _graph: AudioGraph = AudioGraph()
    var graph: AudioGraph
        get() = _graph
        set(value) {
            _graph.nodes.values.forEach { onAudioGraphNodeRemoved(_graph, it) }
            _graph.removeListener(this)
            _graph = value
            applyGraph()
        }

    val midi = Midi(context, if (Library.hasJNI) this else object : Midi.Callback {})
    val metadata = AudioNodeMetadata(Library.PrivateMetadataKey, this)

    private val listeners = mutableListOf<Listener>()
    private val handler = Handler(Looper.getMainLooper())

    private var isLoadingState = false
    private var saveStateBlock: Runnable? = null

    init {
        applyGraph()
        midi.addListener(this)
    }

    override fun finalize() {
        stop()
        super.finalize()
    }

    fun addListener(listener: Listener) {
        listeners.add(listener)
    }

    fun removeListener(listener: Listener) {
        listeners.remove(listener)
    }

    fun start() {
        if (isStarted) {
            throw Library.Exception("Already started")
        }

        midi.start()

        if (options.stateFile != null) {
            loadedStateFileOnStart = loadState()
        }

        if (Library.hasJNI) {
            jniStartStream()
        }

        listeners.forEach { it.onAudioRootStarted() }

        isStarted = true

        Log.i(Library.LOG_TAG, "Started")
    }

    fun stop() {
        if (! isStarted) {
            throw Library.Exception("Already stopped")
        }

        cancelSaveState()

        midi.stop()

        if (Library.hasJNI) {
            jniStopStream()
        }

        listeners.forEach { it.onAudioRootStopped() }

        isStarted = false

        Log.i(Library.LOG_TAG, "Stopped")
    }

    fun loadState(): Boolean {
        if (options.stateFile == null) {
            throw StateFileNotSpecifiedException()
        }

        if (! options.stateFile.exists()) {
            return false
        }

        isLoadingState = true

        val state = Json.decodeFromString<State>(options.stateFile.readText())

        graph.nodePropertyValues = state.graphNodePropertyValues
        midi.portOpenState = state.midiPortOpenState
        metadata.store = state.rootMetadata

        isLoadingState = false

        Log.i(Library.LOG_TAG, "Loaded state file")

        return true
    }

    fun saveState() {
        if (options.stateFile == null) {
            throw StateFileNotSpecifiedException()
        }

        val state = State(
            graphNodePropertyValues = graph.nodePropertyValues,
            midiPortOpenState = midi.portOpenState,
            rootMetadata = metadata.store
        )

        options.stateFile.writeText(Json.encodeToString(state))

        Log.i(Library.LOG_TAG, "Saved state")
    }

    //
    // AudioGraph.Listener
    //

    override fun onAudioGraphNodeAdded(graph: AudioGraph, node: AudioNode) {
        node.callback = object : AudioNode.Callback {
            override fun getMidi() = midi
        }

        node.addListener(this)
    }

    override fun onAudioGraphNodeRemoved(graph: AudioGraph, node: AudioNode) {
        node.callback = null
        node.removeListener(this)
    }

    //
    // Midi.Callback
    //

    override fun midiOpenPort(id: Int, midiDevice: MidiDevice) {
        jniOpenMidi(id, midiDevice)
    }

    override fun midiClosePort(id: Int) {
        jniCloseMidi(id)
    }

    override fun midiQueueData(bytes: ByteArray) {
        jniQueueMidi(bytes)
    }

    //
    // Midi.Listener
    //

    override fun onMidiPortsChange(ports: List<Midi.Port>) {
        if (options.autoOpenMidiPorts) {
            midi.ports
                .filter { ! it.isOpen }
                .forEach { it.open() }
        }
    }

    override fun onMidiPortOpen(port: Midi.Port) {
        saveStateIfNeeded()
    }

    override fun onMidiPortClose(port: Midi.Port) {
        saveStateIfNeeded()
    }

    //
    // AudioNode.Listener
    //

    override fun onAudioNodePropertyValueChanged(node: AudioNode, property: AudioNodeProperty,
                                                 value: Variant) {
        saveStateIfNeeded(delayed = true)
    }

    //
    // AudioNodeMetadata.Callback
    //

    override fun onAudioNodeMetadataChanged(metadata: AudioNodeMetadata) {
        saveStateIfNeeded()
    }

    //
    // Private methods
    //

    private fun applyGraph() {
        _graph.addListener(this)
        _graph.nodes.values.forEach { onAudioGraphNodeAdded(_graph, it) }

        if (Library.hasJNI) {
            jniSetGraph(_graph)
        }
    }

    @Synchronized
    private fun saveStateIfNeeded(delayed: Boolean = false) {
        if (isLoadingState) {
            return
        }

        if (options.stateFile == null) {
            return
        }

        if (delayed) {
            cancelSaveState()

            val block = Runnable {
                try {
                    saveState()
                } catch (e: Exception) {
                    Log.e(Library.LOG_TAG, "Could not automatically save state - ${e.message}")
                }
            }

            handler.postDelayed(block, 500L)

            saveStateBlock = block
        } else {
            saveState()
        }
    }

    private fun cancelSaveState() {
        saveStateBlock?.let { handler.removeCallbacks(it) }
        saveStateBlock = null
    }

    external fun jniOpenMidi(id: Int, midiDevice: MidiDevice)
    external fun jniCloseMidi(id: Int)
    external fun jniQueueMidi(bytes: ByteArray)

    private external fun jniStartStream()
    private external fun jniStopStream()
    private external fun jniSetGraph(graph: AudioGraph)

    private class StateFileNotSpecifiedException : Library.Exception("State file not specified")

    @Serializable
    private data class State(
        val graphNodePropertyValues: AudioGraphNodePropertyValues,
        val midiPortOpenState: MidiPortOpenState,
        val rootMetadata: Map<String, Variant>
    )

}
