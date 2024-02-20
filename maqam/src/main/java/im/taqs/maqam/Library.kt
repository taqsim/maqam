//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam

import android.content.Context

object Library {

    open class Exception(message: String) : kotlin.Exception(message)

    const val LOG_TAG = "Maqam"

    const val PrivateMetadataKey = "_metadata"

    internal var hasJNI = false
        private set

    // Must call before instantiating any Maqam class

    fun init(context: Context) {
        if (hasJNI) {
            return
        }

        System.loadLibrary("maqam")
        jniInit(context)

        hasJNI = true
    }

}

private external fun jniInit(context: Context)
