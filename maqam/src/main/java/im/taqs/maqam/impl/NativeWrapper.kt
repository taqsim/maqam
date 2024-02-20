//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

import im.taqs.maqam.Library

open class NativeWrapper internal constructor() {

    private var impl: Long = 0

    init {
        if (Library.hasJNI) {
            jniCreateImpl()
        }
    }

    protected open fun finalize() {
        if (Library.hasJNI) {
            jniDestroyImpl()
        }
    }

    private external fun jniCreateImpl()
    private external fun jniDestroyImpl()

}
