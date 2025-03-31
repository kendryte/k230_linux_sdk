/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

/**
 * NearlinkEventManager receives broadcasts and callbacks from the Nearlink
 * API and dispatches the event on the UI thread to the right class in the
 * Settings.
 *
 * @since 2023-12-04
 */
public class NearlinkEventManager {
    private static final String TAG = "NearlinkEventManager";

    private final LocalNearlinkAdapter mLocalAdapter;

    private final CachedNearlinkDeviceManager mDeviceManager;

    private LocalNearlinkProfileManager mProfileManager;

    private final IntentFilter mAdapterIntentFilter;

    private final IntentFilter mProfileIntentFilter;

    private final Map<String, Handler> mHandlerMap;

    private Context mContext;

    private final Collection<NearlinkCallback> mCallbacks = new ArrayList<NearlinkCallback>();

    private android.os.Handler mReceiverHandler;

    interface Handler {
        void onReceive(Context context, Intent intent, NearlinkDevice device);
    }

    private void addHandler(String action, Handler handler) {
        mHandlerMap.put(action, handler);
        mAdapterIntentFilter.addAction(action);
    }

    void addProfileHandler(String action, Handler handler) {
        mHandlerMap.put(action, handler);
        mProfileIntentFilter.addAction(action);
    }

    // Set profile manager after construction due to circular dependency
    void setProfileManager(LocalNearlinkProfileManager manager) {
        mProfileManager = manager;
    }

    NearlinkEventManager(LocalNearlinkAdapter adapter, CachedNearlinkDeviceManager deviceManager, Context context) {
        mLocalAdapter = adapter;
        mDeviceManager = deviceManager;
        mAdapterIntentFilter = new IntentFilter();
        mProfileIntentFilter = new IntentFilter();
        mHandlerMap = new HashMap<String, Handler>();
        mContext = context;

        // Nearlink on/off broadcasts
        addHandler(NearlinkAdapter.ACTION_STATE_CHANGED, new AdapterStateChangedHandler());
        // Generic connected/not broadcast
        addHandler(NearlinkAdapter.ACTION_CONNECTION_STATE_CHANGED, new ConnectionStateChangedHandler());

        addHandler(NearlinkAdapter.ACTION_PAIR_STATE_CHANGED, new PairStateChangedHandler());

        // Discovery broadcasts
        addHandler(NearlinkAdapter.ACTION_DISCOVERY_STARTED, new ScanningStateChangedHandler(true));
        addHandler(NearlinkAdapter.ACTION_DISCOVERY_FINISHED, new ScanningStateChangedHandler(false));
        addHandler(NearlinkDevice.ACTION_FOUND, new DeviceFoundHandler());
        addHandler(NearlinkDevice.ACTION_DISAPPEARED, new DeviceDisappearedHandler());
        addHandler(NearlinkDevice.ACTION_NAME_CHANGED, new NameChangedHandler());
        addHandler(NearlinkDevice.ACTION_ALIAS_CHANGED, new NameChangedHandler());

        // Fine-grained state broadcasts
        addHandler(NearlinkDevice.ACTION_CLASS_CHANGED, new ClassChangedHandler());
        addHandler(NearlinkDevice.ACTION_UUID, new UuidChangedHandler());
        addHandler(NearlinkDevice.ACTION_BATTERY_LEVEL_CHANGED, new BatteryLevelChangedHandler());

        // Dock event broadcasts
        addHandler(Intent.ACTION_DOCK_EVENT, new DockEventHandler());

        addHandler(TelephonyManager.ACTION_PHONE_STATE_CHANGED, new AudioModeChangedHandler());

        mContext.registerReceiver(mBroadcastReceiver, mAdapterIntentFilter, null, mReceiverHandler);
        mContext.registerReceiver(mProfileBroadcastReceiver, mProfileIntentFilter, null, mReceiverHandler);
    }

    void registerProfileIntentReceiver() {
        mContext.registerReceiver(mProfileBroadcastReceiver, mProfileIntentFilter, null, mReceiverHandler);
    }

    public void setReceiverHandler(android.os.Handler handler) {
        mContext.unregisterReceiver(mBroadcastReceiver);
        mContext.unregisterReceiver(mProfileBroadcastReceiver);
        mReceiverHandler = handler;
        mContext.registerReceiver(mBroadcastReceiver, mAdapterIntentFilter, null, mReceiverHandler);
        registerProfileIntentReceiver();
    }

    /**
     * Register to start receiving callbacks for Nearlink events.
     */
    public void registerCallback(NearlinkCallback callback) {
        synchronized (mCallbacks) {
            mCallbacks.add(callback);
        }
    }

    /**
     * Unregister to stop receiving callbacks for Nearlink events.
     */
    public void unregisterCallback(NearlinkCallback callback) {
        synchronized (mCallbacks) {
            mCallbacks.remove(callback);
        }
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.e(TAG, "[onReceive]  action:" + action);
            NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
            Log.e(TAG, "[onReceive]  device:" + device);
            Handler handler = mHandlerMap.get(action);
            if (handler != null) {
                handler.onReceive(context, intent, device);
            }
        }
    };

    private final BroadcastReceiver mProfileBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);

            Handler handler = mHandlerMap.get(action);
            if (handler != null) {
                handler.onReceive(context, intent, device);
            }
        }
    };

    private class AdapterStateChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            int state = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, NearlinkAdapter.ERROR);

            Log.e(TAG, "AdapterStateChangedHandler [onReceive] state:" + state);
            // Reregister Profile Broadcast Receiver as part of TURN OFF
            if (state == NearlinkAdapter.STATE_OFF) {
                context.unregisterReceiver(mProfileBroadcastReceiver);
                registerProfileIntentReceiver();
            }
            // update local profiles and get paired devices
            mLocalAdapter.setNearlinkStateInt(state);
            // send callback to update UI and possibly start scanning
            synchronized (mCallbacks) {
                for (NearlinkCallback callback : mCallbacks) {
                    callback.onNearlinkStateChanged(state);
                }
            }
            // Inform CachedDeviceManager that the adapter state has changed
            mDeviceManager.onNearlinkStateChanged(state);
        }
    }

    private class ScanningStateChangedHandler implements Handler {
        private final boolean mStarted;

        ScanningStateChangedHandler(boolean started) {
            mStarted = started;
        }

        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            synchronized (mCallbacks) {
                for (NearlinkCallback callback : mCallbacks) {
                    callback.onScanningStateChanged(mStarted);
                }
            }
            mDeviceManager.onScanningStateChanged(mStarted);
        }
    }

    private class DeviceFoundHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            int rssi = intent.getIntExtra(NearlinkDevice.EXTRA_RSSI, Short.MIN_VALUE);
            int nlClass = intent.getIntExtra(NearlinkDevice.EXTRA_APPEARANCE, NearlinkAppearance.UNKNOWN);
            String name = intent.getStringExtra(NearlinkDevice.EXTRA_NAME);

            // TODO Pick up UUID. They should be available for 2.1 devices.
            // Skip for now, there's a bluez problem and we are not getting uuids even for 2.1.
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                cachedDevice = mDeviceManager.addDevice(mLocalAdapter, mProfileManager, device);
                Log.d(TAG, "DeviceFoundHandler [onReceive] new cachedDevice: " + cachedDevice);
            }
            cachedDevice.setRssi(rssi);
            cachedDevice.setNlClass(nlClass);
            if (!TextUtils.isEmpty(name)) {
                cachedDevice.setNewName(name);
            }
            cachedDevice.setJustDiscovered(true);
            Log.e(TAG, "DeviceFoundHandler [onReceive] name:" + name + ", device:" + device + ", nlClass:" + nlClass);
        }
    }

    private class ConnectionStateChangedHandler implements Handler {
        @Override
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            int state = intent.getIntExtra(NearlinkAdapter.EXTRA_CONNECTION_STATE, NearlinkAdapter.ERROR);
            Log.e(TAG, "ConnectionStateChangedHandler [onReceive] device:" + device + ", state: " + state);
            dispatchConnectionStateChanged(cachedDevice, state);
        }
    }

    private void dispatchPairStateChanged(CachedNearlinkDevice cachedDevice, int state) {
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onPairStateChanged(cachedDevice, state);
            }
        }
        Log.e(TAG, "PairStateChangedHandler [onReceive] state: " + state);
        if (cachedDevice != null) {
            Log.e(TAG, "[dispatchPairStateChanged] cachedDevice.refresh()");
            cachedDevice.refresh();
        }
    }

    private class PairStateChangedHandler implements Handler {
        @Override
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            int state = intent.getIntExtra(NearlinkDevice.EXTRA_PAIR_STATE, NearlinkAdapter.ERROR);
            Log.e(TAG, "PairStateChangedHandler [onReceive] device:" + device + ", state: " + state);
            dispatchPairStateChanged(cachedDevice, state);
        }
    }

    private void dispatchConnectionStateChanged(CachedNearlinkDevice cachedDevice, int state) {
        Log.e(TAG, "[dispatchConnectionStateChanged] cachedDevice:" + cachedDevice + ", state:" + state);
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onConnectionStateChanged(cachedDevice, state);
            }
        }

        if (cachedDevice != null) {
            Log.e(TAG, "[dispatchConnectionStateChanged] cachedDevice.refresh()");
            cachedDevice.refresh();
        }
    }

    void dispatchDeviceAdded(CachedNearlinkDevice cachedDevice) {
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onDeviceAdded(cachedDevice);
            }
        }
    }

    void dispatchDeviceRemoved(CachedNearlinkDevice cachedDevice) {
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onDeviceDeleted(cachedDevice);
            }
        }
    }

    private class DeviceDisappearedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.w(TAG, "received ACTION_DISAPPEARED for an unknown device: " + device);
                return;
            }
            Log.e(TAG, "[DeviceDisappearedHandler] device:" + device);
            if (CachedNearlinkDeviceManager.onDeviceDisappeared(cachedDevice)) {
                mDeviceManager.removeDevice(cachedDevice);
            }
        }
    }

    private class NameChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            mDeviceManager.onDeviceNameUpdated(device);
        }
    }

    private class ClassChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            mDeviceManager.onBtClassChanged(device);
        }
    }

    private class UuidChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            mDeviceManager.onUuidChanged(device);
        }
    }

    private class DockEventHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            // Remove if unpair device upon undocking
            int anythingButUnDocked = Intent.EXTRA_DOCK_STATE_UNDOCKED + 1;
            int state = intent.getIntExtra(Intent.EXTRA_DOCK_STATE, anythingButUnDocked);
            if (state == Intent.EXTRA_DOCK_STATE_UNDOCKED) {
                if (device != null && device.getPairState() == NearlinkConstant.SLE_PAIR_NONE) {
                    CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
                    if (cachedDevice != null) {
                        cachedDevice.setJustDiscovered(false);
                    }
                }
            }
        }
    }

    private class BatteryLevelChangedHandler implements Handler {
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice != null) {
                cachedDevice.refresh();
            }
        }
    }

    boolean readPairedDevices() {
        Set<NearlinkDevice> bondedDevices = mLocalAdapter.getBondedDevices();
        if (bondedDevices == null) {
            return false;
        }

        boolean deviceAdded = false;
        for (NearlinkDevice device : bondedDevices) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                cachedDevice = mDeviceManager.addDevice(mLocalAdapter, mProfileManager, device);
                dispatchDeviceAdded(cachedDevice);
                deviceAdded = true;
            }
        }

        return deviceAdded;
    }

    private void dispatchActiveDeviceChanged(CachedNearlinkDevice activeDevice, int nearlinkProfile) {
        mDeviceManager.onActiveDeviceChanged(activeDevice, nearlinkProfile);
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onActiveDeviceChanged(activeDevice, nearlinkProfile);
            }
        }
    }

    private class AudioModeChangedHandler implements Handler {
        @Override
        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            final String action = intent.getAction();
            if (action == null) {
                Log.w(TAG, "AudioModeChangedHandler() action is null");
                return;
            }
            dispatchAudioModeChanged();
        }
    }

    private void dispatchAudioModeChanged() {
        mDeviceManager.dispatchAudioModeChanged();
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onAudioModeChanged();
            }
        }
    }

    void dispatchProfileConnectionStateChanged(CachedNearlinkDevice device, int state, int nearlinkProfile) {
        synchronized (mCallbacks) {
            for (NearlinkCallback callback : mCallbacks) {
                callback.onProfileConnectionStateChanged(device, state, nearlinkProfile);
            }
        }
        mDeviceManager.onProfileConnectionStateChanged(device, state, nearlinkProfile);
    }
}
