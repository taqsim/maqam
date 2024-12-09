//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

package im.taqs.maqam.impl

import android.content.Context
import android.media.midi.MidiDevice
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiDeviceStatus
import android.media.midi.MidiManager
import android.media.midi.MidiOutputPort
import android.media.midi.MidiReceiver
import android.util.Log
import im.taqs.maqam.Library
import java.lang.ref.WeakReference

typealias MidiPortOpenState = Map<Int, Boolean>

class Midi internal constructor(context: Context, private val callback: Callback) {

    interface Listener {
        fun onMidiEvent(event: MidiEvent, port: Port) {}
        fun onMidiPortsChange(ports: List<Port>) {}
        fun onMidiPortOpen(port: Port) {}
        fun onMidiPortClose(port: Port) {}
    }

    internal interface Callback {
        fun midiOpenPort(id: Int, midiDevice: MidiDevice) {}
        fun midiClosePort(id: Int) {}
        fun midiQueueData(bytes: ByteArray) {}
    }

    private val listeners = mutableListOf<Listener>()
    private val deviceCallback = getDeviceCallback()
    private var manager: MidiManager? = null

    var ports = listOf<Port>()
        private set

    var portOpenState: MidiPortOpenState
        get() {
            val state = HashMap<Int,Boolean>()
            for (port in ports) {
                state[port.id] = port.isOpen
            }
            return state
        }
        set(value) {
            for (port in ports) {
                if (value[port.id] == true) {
                    port.open()
                } else {
                    port.close()
                }
            }
        }

    init {
        try {
            manager = context.getSystemService(Context.MIDI_SERVICE) as? MidiManager

            if (manager == null) {
                throw Library.Exception("System service name does not exist")
            }
        } catch(e: Exception) {
            // java.lang.AssertionError: Unsupported Service: midi
            Log.e(Library.LOG_TAG, "MIDI not available")
        }
    }

    @Synchronized
    fun addListener(listener: Listener) {
        listeners.add(listener)
    }

    @Synchronized
    fun removeListener(listener: Listener) {
        listeners.remove(listener)
    }

    fun queue(event: MidiEvent) {
        callback.midiQueueData(event.bytes)
    }

    internal fun start() {
        manager?.registerDeviceCallback(deviceCallback, null)
    }

    internal fun stop() {
        manager?.unregisterDeviceCallback(deviceCallback)
    }

    private fun getDeviceCallback() = object : MidiManager.DeviceCallback() {
        override fun onDeviceAdded(device: MidiDeviceInfo?) {
            rebuildPortsList()
        }

        override fun onDeviceRemoved(device: MidiDeviceInfo?) {
            rebuildPortsList()
        }

        override fun onDeviceStatusChanged(status: MidiDeviceStatus?) {
            rebuildPortsList()
        }
    }

    private fun rebuildPortsList() {
        val devices = manager?.devices ?: return
        val portsCopy = ports.toMutableList()

        for (port in ports) {
            if (! devices.contains(port.deviceInfo)) {
                port.close()
                portsCopy.remove(port)
            }
        }

        for (deviceInfo in devices) {
            if (ports.find { it.deviceInfo == deviceInfo } == null) {
                portsCopy.add(Port(deviceInfo, this))
            }
        }

        ports = portsCopy

        synchronized(this) {
            listeners.forEach { it.onMidiPortsChange(ports) }
        }
    }

    class Port internal constructor(internal val deviceInfo: MidiDeviceInfo, owner: Midi)
        : MidiReceiver() {

        private val midi = WeakReference(owner)
        private var source: MidiOutputPort? = null

        var isOpen = false
            private set

        val isSink: Boolean
            get() = deviceInfo.inputPortCount > 0

        val isSource: Boolean
            get() = deviceInfo.outputPortCount > 0

        val id: Int
            get() = deviceInfo.id

        val name: String
            get() = deviceInfo.properties.getString(MidiDeviceInfo.PROPERTY_NAME) ?: "unknown"

        fun open() {
            if (isOpen) {
                return
            }

            Log.i(Library.LOG_TAG, "Open MIDI port [$name]")

            midi.get()?.let { midi ->
                midi.manager?.openDevice(deviceInfo, { midiDevice ->
                    if (midiDevice != null) {
                        isOpen = true

                        midi.callback.midiOpenPort(id, midiDevice)

                        if (isSource) {
                            source = midiDevice.openOutputPort(0)
                            source?.connect(this)
                        }

                        synchronized(midi) {
                            midi.listeners.forEach { it.onMidiPortOpen(this) }
                        }
                    }
                }, null)
            }
        }

        fun close() {
            if (! isOpen) {
                return
            }

            Log.i(Library.LOG_TAG, "Close MIDI port [$name]")

            source?.disconnect(this)

            midi.get()?.let { midi ->
                midi.callback.midiClosePort(id)

                synchronized(midi) {
                    midi.listeners.forEach { it.onMidiPortClose(this) }
                }
            }
        }

        override fun onSend(msg: ByteArray?, offset: Int, count: Int, timestamp: Long) {
            if (msg == null) {
                return
            }

            MidiEvent.fromBytes(msg.copyOfRange(offset, msg.size - 1))?.let { event ->
                midi.get()?.let { midi ->
                    synchronized(midi) {
                        midi.listeners.forEach { it.onMidiEvent(event, this) }
                    }
                }
            }
        }
    }

}
