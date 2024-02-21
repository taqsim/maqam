//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam

import im.taqs.maqam.impl.NativeWrapper
import kotlin.random.Random
import kotlin.random.nextUInt

typealias AudioGraphNodePropertyValues = Map<String,AudioNodePropertyValues>

fun randomNodeTag(): String = "node_" + Random.nextUInt(1000u, 9999u).toString()

// Backed by a juce::AudioProcessorGraph instance
class AudioGraph(
    private val overrideDefaultNodePropertyValues: AudioGraphNodePropertyValues = mapOf()
) : NativeWrapper() {

    interface Listener {
        fun onAudioGraphNodeAdded(graph: AudioGraph, node: AudioNode) {}
        fun onAudioGraphNodeRemoved(graph: AudioGraph, node: AudioNode) {}
    }

    // LinkedHashMap preserves insertion order
    internal val nodes = LinkedHashMap<String,AudioNode>()

    private val listeners = mutableListOf<Listener>()
    //private val defaultNodePropertyValues = mutableMapOf<String,AudioNodePropertyValues>()

    var nodePropertyValues: AudioGraphNodePropertyValues
        get() {
            return nodes.mapValues { it.value.propertyValues }
        }
        set(value) {
            value.forEach { (tag, values) -> nodes[tag]?.propertyValues = values }
        }

    fun addListener(listener: Listener) {
        listeners.add(listener)
    }

    fun removeListener(listener: Listener) {
        listeners.remove(listener)
    }

    fun get(tag: String): AudioNode? {
        return nodes[tag]
    }

    fun add(node: AudioNode, tag: String = randomNodeTag()): String {
        nodes[tag] = node

        if (Library.hasJNI) {
            jniAddNode(node)
        }

        listeners.forEach { it.onAudioGraphNodeAdded(this, node) }

        return tag
    }

    fun connect(source: AudioNode, sink: AudioNode, audio: Boolean = true, midi: Boolean = false) {
        if (Library.hasJNI) {
            jniConnectNodes(source, sink, audio, midi)
        }
    }

    fun connectInSeries(vararg nodes: AudioNode, audio: Boolean = true, midi: Boolean = false) {
        if (nodes.size < 2) {
            throw Library.Exception("Must specify two or more nodes")
        }

        for (i in 0..<nodes.size - 1) {
            connect(source = nodes[i], sink = nodes[i + 1], audio, midi)
        }
    }

    fun sendMidiInputTo(vararg sinks: AudioNode) {
        for (sink in sinks) {
            captureInputTo(sink, audio = false, midi = true)
        }
    }

    fun captureInputTo(sink: AudioNode, audio: Boolean = true, midi: Boolean = false) {
        if (Library.hasJNI) {
            jniConnectNodes(null, sink, audio, midi)
        }
    }

    fun playbackOutputFrom(vararg sources: AudioNode) {
        if (Library.hasJNI) {
            for (source in sources) {
                jniConnectNodes(source, null, audio = true, midi = false)
            }
        }
    }

    fun resetNodePropertyValues(
        skip: AudioNodePropertyIdentifiers = listOf(),
        includeMetadata: Boolean = false
    ) {
        nodes.forEach { (tag, node) ->
            node.resetPropertyValues(overrideDefaultNodePropertyValues[tag] ?: mapOf(),
                skip, includeMetadata)
        }
    }

    fun mergeNodePropertyValues(
        other: AudioGraphNodePropertyValues,
        skip: AudioNodePropertyIdentifiers = listOf()
    ) {
        nodes.forEach { (tag, node) ->
            other[tag]?.let {
                node.mergePropertyValues(it, skip)
            }
        }
    }

    fun debugPrintConnections() {
        jniDebugPrintConnections()
    }

    private external fun jniAddNode(node: AudioNode)
    private external fun jniConnectNodes(source: AudioNode?, sink: AudioNode?,
                                         audio: Boolean, midi: Boolean)
    private external fun jniDebugPrintConnections();

    // For simplicity, when creating a graph using Builder the first added node is automatically
    // connected to the graph audio input, and the last added node connected to the audio output.

    class Builder {

        private val graph = AudioGraph()

        fun add(node: AudioNode, tag: String = randomNodeTag()): Builder {
            with(graph) {
                val graphWasEmpty = nodes.isEmpty()

                add(node, tag)

                if (graphWasEmpty) {
                    try {
                        captureInputTo(node)
                    } catch (_: RuntimeException) {
                        // TODO - use library exception
                    }
                } else {
                    connect(nodes.values.last(), node)
                }
            }

            return this
        }

        fun build(): AudioGraph {
            with(graph) {
                if (nodes.isNotEmpty()) {
                    try {
                    playbackOutputFrom(nodes.values.last())
                    } catch (_: RuntimeException) {
                        // TODO - use library exception
                    }
                }

                return this
            }
        }

    }

}
