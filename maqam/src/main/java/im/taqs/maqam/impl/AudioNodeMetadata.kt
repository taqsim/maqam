//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

class AudioNodeMetadata internal constructor(
    key: String,
    private val callback: Callback
) : AudioNodeProperty(key, true, callback) {

    internal interface Callback : AudioNodeProperty.Callback {
        fun onAudioNodeMetadataChanged(metadata: AudioNodeMetadata)
    }

    private val _store = mutableMapOf<String, Variant>()

    internal var store: Map<String, Variant>
        get() = _store
        set(value) {
            _store.clear()
            _store.putAll(value)
            callback.onAudioNodeMetadataChanged(this)
        }

    fun get(key: String): Variant? = _store[key]

    fun put(key: String, value: Variant) {
        _store[key] = value
        callback.onAudioNodeMetadataChanged(this)
    }

    fun getOptionalBoolean(key: String): Boolean? = get(key)?.b
    fun getBoolean(key: String, default: Boolean = false): Boolean = get(key)?.b ?: default
    fun putBoolean(key: String, value: Boolean) { put(key, Variant(value)) }

    fun getOptionalFloat(key: String): Float? = get(key)?.f
    fun getFloat(key: String, default: Float = 0f): Float = getOptionalFloat(key) ?: default
    fun putFloat(key: String, value: Float) { put(key, Variant(value)) }

    fun getOptionalString(key: String): String? = get(key)?.s
    fun getString(key: String, default: String = ""): String = getOptionalString(key) ?: default
    fun putString(key: String, value: String) { put(key, Variant(value)) }

    fun getOptionalMap(key: String): Map<String, Variant>? = get(key)?.m
    fun getMap(key: String, default: Map<String, Variant> = mapOf()): Map<String, Variant> =
        getOptionalMap(key) ?: default
    fun putMap(key: String, value: Map<String, Variant>) { put(key, Variant(value)) }

    fun remove(key: String) {
        _store.remove(key)
        callback.onAudioNodeMetadataChanged(this)
    }

    override var serializableValue: Variant
        get() = Variant(store)
        set(value) { store = value.m ?: mapOf() }

}
