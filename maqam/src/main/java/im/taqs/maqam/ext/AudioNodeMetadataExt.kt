//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.ext

import im.taqs.maqam.impl.AudioNodeMetadata
import im.taqs.maqam.impl.Variant

fun AudioNodeMetadata.getOptionalInt(key: String): Int? = getOptionalFloat(key)?.toInt()

fun AudioNodeMetadata.getInt(key: String, default: Int = 0): Int = getOptionalInt(key) ?: default

fun AudioNodeMetadata.putInt(key: String, value: Int) {
    put(key, Variant(f = value.toFloat(), encodeAsInt = true))
}

fun AudioNodeMetadata.getOptionalLong(key: String): Long? = getOptionalFloat(key)?.toLong()

fun AudioNodeMetadata.getLong(key: String, default: Long = 0L): Long = getOptionalLong(key) ?: default

fun AudioNodeMetadata.putLong(key: String, value: Long) {
    put(key, Variant(f = value.toFloat(), encodeAsInt = true))
}
