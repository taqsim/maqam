//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

import kotlinx.serialization.KSerializer
import kotlinx.serialization.Serializable
import kotlinx.serialization.builtins.MapSerializer
import kotlinx.serialization.builtins.serializer
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder
import kotlinx.serialization.json.JsonDecoder
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.booleanOrNull
import kotlinx.serialization.json.floatOrNull

@Serializable(with = VariantSerializer::class)
data class Variant constructor(
    val b: Boolean? = null,
    val f: Float? = null,
    val s: String? = null,
    val m: Map<String, Variant>? = null,
    internal val encodeAsInt: Boolean = false
) {
    constructor(b: Boolean) : this(b = b, encodeAsInt = false)
    constructor(f: Float) : this(f = f, encodeAsInt = false)
    constructor(s: String) : this(s = s, encodeAsInt = false)
    constructor(m: Map<String, Variant>) : this(m = m.toMap(), encodeAsInt = false)

    val isNull: Boolean get() = (b == null) && (f == null) && (s == null) && (m == null)
}

object VariantSerializer : KSerializer<Variant> {

    override val descriptor = PrimitiveSerialDescriptor("Variant",
        PrimitiveKind.STRING)

    private val objectSerializer = MapSerializer(String.serializer(), Variant.serializer())

    override fun serialize(encoder: Encoder, value: Variant) {
        value.b?.let { encoder.encodeBoolean(it) }
            ?: value.f?.let {
                if (value.encodeAsInt)
                    encoder.encodeInt(it.toInt())
                else
                    encoder.encodeFloat(it)
            }
            ?: value.s?.let { encoder.encodeString(it) }
            ?: value.m?.let {
                encoder.encodeSerializableValue(objectSerializer, it)
            }
    }

    override fun deserialize(decoder: Decoder): Variant =
        if (decoder is JsonDecoder) {
            deserialize(decoder.decodeJsonElement())
        } else {
            val value = decoder.decodeString()
            value.toFloatOrNull()?.let { Variant(it) }
                ?: value.toBooleanStrictOrNull()?.let { Variant(it) }
                ?: Variant(s = value)
        }

    private fun deserialize(obj: JsonObject): Variant =
        Variant(obj.mapValues {
            deserialize(it.value)
        })

    private fun deserialize(primitive: JsonPrimitive): Variant =
        if (primitive.isString) {
            Variant(primitive.content)
        } else {
            primitive.floatOrNull?.let { Variant(it) }
                ?: primitive.booleanOrNull?.let { Variant(it) }
                ?: Variant()
        }

    private fun deserialize(element: JsonElement): Variant =
        when (element) {
            is JsonPrimitive -> { deserialize(element) }
            is JsonObject -> { deserialize(element) }
            else -> { Variant() }
        }

}
