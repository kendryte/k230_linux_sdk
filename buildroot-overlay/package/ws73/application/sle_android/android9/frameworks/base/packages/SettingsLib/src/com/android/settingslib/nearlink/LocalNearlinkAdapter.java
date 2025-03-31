/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.List;
import java.util.Set;

import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkProfile;
import android.nearlink.NearlinkSeeker;
import android.os.ParcelUuid;
import android.util.Log;

/**
 * LocalNearlinkAdapter provides an interface between the Settings app
 * and the functionality of the local {@link NearlinkAdapter}, specifically
 * those related to state transitions of the adapter itself.
 *
 * <p>Connection and bonding state changes affecting specific devices
 * are handled by {@link CachedNearlinkDeviceManager},
 * {@link NearlinkEventManager}, and {@link LocalNearlinkProfileManager}.
 *
 * @since 2023-12-04
 */
public class LocalNearlinkAdapter {
    private static final String TAG = "LocalNearlinkAdapter";

    /**
     * This class does not allow direct access to the NearlinkAdapter.
     */
    private final NearlinkAdapter mAdapter;

    private LocalNearlinkProfileManager mProfileManager;

    private static LocalNearlinkAdapter sInstance;

    private int mState = NearlinkAdapter.ERROR;

    private static final int SCAN_EXPIRATION_MS = 5 * 60 * 1000; // 5 mins

    private long mLastScan;

    private LocalNearlinkAdapter(NearlinkAdapter adapter) {
        mAdapter = adapter;
    }

    void setProfileManager(LocalNearlinkProfileManager manager) {
        mProfileManager = manager;
    }

    /**
     * Get the singleton instance of the LocalNearlinkAdapter. If this device
     * doesn't support Nearlink, then null will be returned. Callers must be
     * prepared to handle a null return value.
     *
     * @return the LocalNearlinkAdapter object, or null if not supported
     */
    static synchronized LocalNearlinkAdapter getInstance() {
        if (sInstance == null) {
            NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
            if (adapter != null) {
                sInstance = new LocalNearlinkAdapter(adapter);
            }
        }

        return sInstance;
    }

    // Pass-through NearlinkAdapter methods that we can intercept if necessary

    public void cancelDiscovery() {
        mAdapter.cancelDiscovery();
    }

    public boolean enable() {
        return mAdapter.enable();
    }

    public boolean disable() {
        return mAdapter.disable();
    }

    public String getAddress() {
        return mAdapter.getAddress();
    }

    void getProfileProxy(Context context,
                         NearlinkProfile.ServiceListener listener, int profile) {
        mAdapter.getProfileProxy(context, listener, profile);
    }

    public Set<NearlinkDevice> getBondedDevices() {
        return mAdapter.getPairedDevices();
    }

    public String getName() {
        return mAdapter.getName();
    }

    public int getScanMode() {
        return mAdapter.getAnnounceMode();
    }

    public NearlinkSeeker getNearlinkLeScanner() {
        return mAdapter.getNearlinkSeeker();
    }

    public int getState() {
        return mAdapter.getState();
    }

    public ParcelUuid[] getUuids() {
        return mAdapter.getUuids();
    }

    public boolean isDiscovering() {
        return mAdapter.isDiscovering();
    }

    public boolean isEnabled() {
        return mAdapter.isEnabled();
    }

    public int getConnectionState() {
        return mAdapter.getConnectionState();
    }

    public void setDiscoverableTimeout(int timeout) {
        mAdapter.setDiscoverableTimeout(timeout);
    }

    public long getDiscoveryEndMillis() {
        return mAdapter.getDiscoveryEndMillis();
    }

    public void setName(String name) {
        mAdapter.setName(name);
    }

    public void setScanMode(int mode) {
        mAdapter.setAnnounceMode(mode);
    }

    public boolean setScanMode(int mode, int duration) {
        return mAdapter.setAnnounceMode(mode, duration);
    }

    public void startScanning(boolean force) {
        // Only start if we're not already scanning
        Log.e(TAG, "[startScanning] => NearlinkAdapter.isDiscovering:" + mAdapter.isDiscovering());
        if (!mAdapter.isDiscovering()) {
            if (!force) {
                // Don't scan more than frequently than SCAN_EXPIRATION_MS,
                // unless forced
                if (mLastScan + SCAN_EXPIRATION_MS > System.currentTimeMillis()) {
                    return;
                }
            }

            Log.e(TAG, "[startScanning] => NearlinkAdapter.startDiscovery():");
            if (mAdapter.startDiscovery()) {
                mLastScan = System.currentTimeMillis();
            }
        }
    }

    public void stopScanning() {
        Log.e(TAG, "[stopScanning] => NearlinkAdapter.isDiscovering:" + mAdapter.isDiscovering());
        if (mAdapter.isDiscovering()) {
            Log.e(TAG, "[stopScanning] => NearlinkAdapter.cancelDiscovery():");
            mAdapter.cancelDiscovery();
        }
    }

    public synchronized int getNearlinkState() {
        // Always sync state, in case it changed while paused
        syncNearlinkState();
        return mState;
    }

    void setNearlinkStateInt(int state) {
        synchronized (this) {
            if (mState == state) {
                return;
            }
            mState = state;
        }

        if (state == NearlinkAdapter.STATE_ON) {
            // if mProfileManager hasn't been constructed yet, it will
            // get the adapter UUIDs in its constructor when it is.
            if (mProfileManager != null) {
                mProfileManager.setNearlinkStateOn();
            }
        }
    }

    // Returns true if the state changed; false otherwise.
    boolean syncNearlinkState() {
        int currentState = mAdapter.getState();
        if (currentState != mState) {
            setNearlinkStateInt(mAdapter.getState());
            return true;
        }
        return false;
    }

    public boolean setNearlinkEnabled(boolean enabled) {
        boolean success = enabled
                ? mAdapter.enable()
                : mAdapter.disable();

        if (success) {
            setNearlinkStateInt(enabled
                    ? NearlinkAdapter.STATE_TURNING_ON
                    : NearlinkAdapter.STATE_TURNING_OFF);
        } else {
            if (Utils.V) {
                Log.v(TAG, "setNearlinkEnabled call, manager didn't return " +
                        "success for enabled: " + enabled);
            }

            syncNearlinkState();
        }
        return success;
    }

    public NearlinkDevice getRemoteDevice(String address) {
        return mAdapter.getRemoteDevice(address);
    }

    public int getMaxConnectedAudioDevices() {
        return mAdapter.getMaxConnectedAudioDevices();
    }

    public List<Integer> getSupportedProfiles() {
        return mAdapter.getSupportedProfiles();
    }
}
