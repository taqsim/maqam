//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam

import im.taqs.maqam.impl.AudioNodeMetadata
import im.taqs.maqam.impl.AudioNodeParameter
import im.taqs.maqam.impl.AudioNodeProperty
import im.taqs.maqam.impl.AudioNodeState
import im.taqs.maqam.impl.Midi
import im.taqs.maqam.impl.MidiEvent
import im.taqs.maqam.impl.NativeWrapper
import im.taqs.maqam.impl.Variant
import java.lang.ref.WeakReference

typealias AudioNodePropertyIdentifiers = List<String>
typealias AudioNodePropertyValues = Map<String, Variant>

// Backed by a juce::AudioProcessor instance
open class AudioNode(): NativeWrapper() {

    interface Listener {
        fun onAudioNodePropertyValueChanged(node: AudioNode, property: AudioNodeProperty,
                                            value: Variant) {}
    }

    internal interface Callback {
        fun getMidi(): Midi
    }

    val metadata: AudioNodeMetadata

    internal var callback: Callback? = null
    internal val midi get() = checkNotNull(callback?.getMidi()) {
        "Node is not connected to a graph"
    }

    private val listeners = mutableListOf<Listener>()
    private val properties = mutableMapOf<String, AudioNodeProperty>()
    private val defaultPropertyValues = mutableMapOf<String, Variant>()

    private var dsp: Long = 0

    init {
        if (Library.hasJNI) {
            jniCreateProcessor()
        }

        metadata = this.metadata(Library.PrivateMetadataKey)
    }

    internal var propertyValues: AudioNodePropertyValues
        get() {
            return properties
                .filter { it.value.isSerializable }
                .mapValues { it.value.serializableValue }
        }
        set(value) {
            mergePropertyValues(value)
        }

    internal fun resetPropertyValues(
        overrideDefaults: AudioNodePropertyValues,
        skip: AudioNodePropertyIdentifiers = listOf(),
        includeMetadata: Boolean = false
    ) {
        val other = (defaultPropertyValues + overrideDefaults).toMutableMap()

        if (! includeMetadata) {
            other.remove(Library.PrivateMetadataKey)
        }

        mergePropertyValues(other, skip)
    }

    internal fun mergePropertyValues(
        other: AudioNodePropertyValues,
        skip: AudioNodePropertyIdentifiers = listOf(),
    ) {
        other.forEach { (key, value) ->
            properties[key]?.let { prop ->
                if (! skip.contains(key) && prop.isSerializable) {
                    prop.serializableValue = value

                    synchronized(listeners) {
                        listeners.forEach {
                            it.onAudioNodePropertyValueChanged(this, prop, value)
                        }
                    }
                }
            }
        }
    }

    fun addListener(listener: Listener) {
        synchronized(listeners) {
            listeners.add(listener)
        }
    }

    fun removeListener(listener: Listener) {
        synchronized(listeners) {
            listeners.remove(listener)
        }
    }

    //
    // There are three types of node properties:
    //
    //   Parameter   Mapped to a juce::AudioProcessorParameter, holds a float value.
    //   State       Backed by properties in a juce::ValueTree, holds a string or float value.
    //   Metadata    Backed by a Kotlin map, holds variable types. This is a convenience property
    //               type intended for storing application data not visible to JUCE. Like with
    //               parameters and states, all changes are automatically persisted.
    //

    protected fun parameter(key: String, serializable: Boolean = true): AudioNodeParameter {
        if (properties[key] is AudioNodeParameter) {
            return properties[key] as AudioNodeParameter
        }

        val weakThis = WeakReference(this)

        val callback = if (Library.hasJNI) {
            object : AudioNodeParameter.Callback {
                override fun getAudioNode() = weakThis.get()

                override fun getAudioNodeParameterName(parameter: AudioNodeParameter) =
                    weakThis.get()?.jniGetParameterName(parameter.key) ?: ""

                override fun getAudioNodeParameterUnit(parameter: AudioNodeParameter) =
                    weakThis.get()?.jniGetParameterLabel(parameter.key) ?: ""

                override fun getAudioNodeParameterValue(parameter: AudioNodeParameter) =
                    weakThis.get()?.jniGetParameterValue(parameter.key) ?: 0f

                override fun setAudioNodeParameterValue(parameter: AudioNodeParameter, value: Float) {
                    weakThis.get()?.jniSetParameterValue(parameter.key, value)
                }

                override fun getAudioNodeParameterValueAsText(parameter: AudioNodeParameter) =
                    weakThis.get()?.jniGetParameterValueAsText(parameter.key) ?: ""

                override fun getAudioNodeParameterValueRange(parameter: AudioNodeParameter)
                        : ClosedFloatingPointRange<Float> =
                    weakThis.get()?.jniGetParameterValueRange(parameter.key)?.let {
                        it[0]..it[1]
                    } ?: 0f..0f
            }
        } else {
            object : AudioNodeParameter.Callback {}
        }

        return registerProperty(AudioNodeParameter(key, serializable, callback), false)
    }

    protected fun state(key: String, serializable: Boolean = true, numeric: Boolean = false)
            : AudioNodeState {
        if (properties[key] is AudioNodeState) {
            return properties[key] as AudioNodeState
        }

        val weakThis = WeakReference(this)

        val callback = if (Library.hasJNI) {
            object : AudioNodeState.Callback {
                override fun getAudioNode() = weakThis.get()

                override fun getAudioNodeStateFloatValue(state: AudioNodeState) =
                    weakThis.get()?.jniGetValueTreePropertyFloatValue(state.key) ?: 0f

                override fun setAudioNodeStateFloatValue(state: AudioNodeState, value: Float) {
                    weakThis.get()?.jniSetValueTreePropertyFloatValue(state.key, value)
                }

                override fun getAudioNodeStateStringValue(state: AudioNodeState) =
                    weakThis.get()?.jniGetValueTreePropertyStringValue(state.key) ?: ""

                override fun setAudioNodeStateStringValue(state: AudioNodeState, value: String) {
                    weakThis.get()?.jniSetValueTreePropertyStringValue(state.key, value)
                }
            }
        } else {
            object : AudioNodeState.Callback {}
        }

        val type = if (numeric) AudioNodeState.Type.FLOAT else AudioNodeState.Type.STRING

        return registerProperty(AudioNodeState(key, serializable, callback, type), false)
    }

    // By design, MIDI events are sent to all nodes owned by the graph and not just this node.
    protected fun queueMidiEvent(event: MidiEvent) {
        midi.queue(event)
    }

    private fun metadata(key: String): AudioNodeMetadata {
        if (properties[key] is AudioNodeMetadata) {
            return properties[key] as AudioNodeMetadata
        }

        val weakThis = WeakReference(this)

        val callback = object : AudioNodeMetadata.Callback {
            override fun getAudioNode() = weakThis.get()

            override fun onAudioNodeMetadataChanged(metadata: AudioNodeMetadata) {
                weakThis.get()?.let { node ->
                    synchronized(node.listeners) {
                        node.listeners.forEach {
                            it.onAudioNodePropertyValueChanged(
                                node, metadata,
                                metadata.serializableValue
                            )
                        }
                    }
                }
            }
        }

        return registerProperty(AudioNodeMetadata(key, callback), true)
    }

    private fun <T: AudioNodeProperty> registerProperty(property: T, isPrivate: Boolean): T {
        val key = property.key

        if (!isPrivate && property.key.startsWith('_')) {
            throw Library.Exception("Key cannot start with underscore")
        }

        if (properties.containsKey(key)) {
            throw Library.Exception("Property already registered")
        }

        properties[key] = property

        // juce::AudioParameterFloat::getDefaultValue() is private, capture here.
        defaultPropertyValues[key] = property.serializableValue

        return property
    }

    // Callback invoked from JNI code
    protected fun jniOnParameterChanged(id: String, value: Float) {
        properties[id]?.let { property ->
            synchronized(listeners) {
                listeners.forEach {
                    it.onAudioNodePropertyValueChanged(this, property,
                        property.serializableValue)
                }
            }
        }
    }

    // Callback invoked from JNI code
    protected fun jniOnValueTreePropertyStringValueChanged(id: String, value: String) {
        properties[id]?.let { property ->
            synchronized(listeners) {
                listeners.forEach {
                    it.onAudioNodePropertyValueChanged(this, property,
                        property.serializableValue)
                }
            }
        }
    }

    // Callback invoked from JNI code
    protected fun jniOnValueTreePropertyFloatValueChanged(id: String, value: Float) {
        properties[id]?.let { property ->
            synchronized(listeners) {
                listeners.forEach {
                    it.onAudioNodePropertyValueChanged(this, property,
                        property.serializableValue)
                }
            }
        }
    }

    external fun jniGetParameterName(id: String): String
    external fun jniGetParameterLabel(id: String): String
    external fun jniGetParameterValue(id: String): Float
    external fun jniSetParameterValue(id: String, value: Float)
    external fun jniGetParameterValueAsText(id: String): String
    external fun jniGetParameterValueRange(id: String): FloatArray
    external fun jniGetValueTreePropertyFloatValue(id: String): Float
    external fun jniSetValueTreePropertyFloatValue(id: String, value: Float)
    external fun jniGetValueTreePropertyStringValue(id: String): String
    external fun jniSetValueTreePropertyStringValue(id: String, value: String)

    private external fun jniCreateProcessor()

}
