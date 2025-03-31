/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.util.Log;

import com.android.internal.annotations.VisibleForTesting;

/**
 * CachedNearlinkDeviceManager manages the set of remote Nearlink devices.
 *
 * @since 2023-12-04
 */
public class CachedNearlinkDeviceManager {
    private static final String TAG = "CachedNearlinkDeviceManager";
    private static final boolean DEBUG = Utils.D;

    private Context mContext;
    private final LocalNearlinkManager mNlManager;

    @VisibleForTesting
    final List<CachedNearlinkDevice> mCachedDevices = new ArrayList<CachedNearlinkDevice>();
    @VisibleForTesting
    final Map<Long, CachedNearlinkDevice> mCachedDevicesMapForHearingAids = new HashMap<Long, CachedNearlinkDevice>();

    CachedNearlinkDeviceManager(Context context, LocalNearlinkManager localBtManager) {
        mContext = context;
        mNlManager = localBtManager;
    }

    public synchronized Collection<CachedNearlinkDevice> getCachedDevicesCopy() {
        Log.e(TAG, "[getCachedDevicesCopy] mCachedDevices.size:" + mCachedDevices.size());
        for (CachedNearlinkDevice d : mCachedDevices) {
            Log.e(TAG, "[getCachedDevicesCopy] mCachedDevices d:" + d);
        }

        Set<NearlinkDevice> bondedDevices = mNlManager.getNearlinkAdapter().getBondedDevices();
        Log.e(TAG, "[getCachedDevicesCopy] bondedDevices.size:" + bondedDevices.size());
        for (NearlinkDevice d : bondedDevices) {
            Log.e(TAG, "[getCachedDevicesCopy] bondedDevices d:" + d);
        }

        return new ArrayList<CachedNearlinkDevice>(mCachedDevices);
    }

    public static boolean onDeviceDisappeared(CachedNearlinkDevice cachedDevice) {
        cachedDevice.setJustDiscovered(false);
        int connectState = cachedDevice.getConnectState();
        return (connectState != NearlinkConstant.SLE_ACB_STATE_CONNECTED) || (connectState != NearlinkConstant.SLE_ACB_STATE_CONNECTING);
    }

    public void onDeviceNameUpdated(NearlinkDevice device) {
        CachedNearlinkDevice cachedDevice = findDevice(device);
        if (cachedDevice != null) {
            cachedDevice.refreshName();
        }
    }

    /**
     * Search for existing {@link CachedNearlinkDevice} or return null
     * if this device isn't in the cache. Use {@link #addDevice}
     * to create and return a new {@link CachedNearlinkDevice} for
     * a newly discovered {@link NearlinkDevice}.
     *
     * @param device the address of the Nearlink device
     * @return the cached device object for this device, or null if it has
     * not been previously seen
     */
    public synchronized CachedNearlinkDevice findDevice(NearlinkDevice device) {
        for (CachedNearlinkDevice cachedDevice : mCachedDevices) {
            if (cachedDevice.getDevice().equals(device)) {
                return cachedDevice;
            }
        }
        return null;
    }

    /**
     * Create and return a new {@link CachedNearlinkDevice}. This assumes
     * that {@link #findDevice} has already been called and returned null.
     *
     * @param device the address of the new Nearlink device
     * @return the newly created CachedNearlinkDevice object
     */
    public CachedNearlinkDevice addDevice(LocalNearlinkAdapter adapter, LocalNearlinkProfileManager profileManager, NearlinkDevice device) {
        CachedNearlinkDevice newDevice = new CachedNearlinkDevice(mContext, adapter, profileManager, device);
        synchronized (this) {
            mCachedDevices.add(newDevice);
            mNlManager.getEventManager().dispatchDeviceAdded(newDevice);
        }

        return newDevice;
    }

    /**
     * 从缓存中移出指定的cacheDevice对象。
     * 当设备从UI中移除时，同时删除内存缓存中的对象
     */
    public void removeDevice(CachedNearlinkDevice cacheDevice) {
        Log.i(TAG, "[removeDevice] device:" + cacheDevice);
        synchronized (this) {
            mCachedDevices.removeIf(device -> device == cacheDevice);
            mNlManager.getEventManager().dispatchDeviceRemoved(cacheDevice);
        }
    }

    /**
     * Returns true if the one of the two hearing aid devices is already cached for UI.
     *
     * @param long hiSyncId
     * @return {@code True} if one of the two hearing aid devices is is already cached for UI.
     */
    private synchronized boolean isPairAddedInCache(long hiSyncId) {
        if (mCachedDevicesMapForHearingAids.containsKey(hiSyncId)) {
            return true;
        }
        return false;
    }


    /**
     * Attempts to get the name of a remote device, otherwise returns the address.
     *
     * @param device The remote device.
     * @return The name, or if unavailable, the address.
     */
    public String getName(NearlinkDevice device) {
        CachedNearlinkDevice cachedDevice = findDevice(device);
        if (cachedDevice != null && cachedDevice.getName() != null) {
            return cachedDevice.getName();
        }

        String name = device.getAliasName();
        if (name != null) {
            return name;
        }

        return device.getAddress();
    }

    public synchronized void clearNonBondedDevices() {
        mCachedDevicesMapForHearingAids.entrySet().removeIf(entries -> entries.getValue().getConnectState() == NearlinkConstant.SLE_ACB_STATE_CONNECTED);
        if (mCachedDevices.size() > 0) {
            List<CachedNearlinkDevice> rm = new ArrayList();
            for (CachedNearlinkDevice device : mCachedDevices) {
                int bondState = device.getBondState();
                int connectState = device.getConnectState();
                if (bondState == NearlinkConstant.SLE_PAIR_PAIRED || connectState == NearlinkConstant.SLE_ACB_STATE_CONNECTED || connectState == NearlinkConstant.SLE_ACB_STATE_CONNECTING) {
                    continue;
                }
                rm.add(device);
            }
            if (rm.size() > 0) {
                mCachedDevices.removeAll(rm);
            }
        }
    }

    public synchronized void onScanningStateChanged(boolean started) {
        if (!started) return;
        // If starting a new scan, clear old visibility
        // Iterate in reverse order since devices may be removed.
        for (int i = mCachedDevices.size() - 1; i >= 0; i--) {
            CachedNearlinkDevice cachedDevice = mCachedDevices.get(i);
            cachedDevice.setJustDiscovered(false);
        }
    }

    public synchronized void onBtClassChanged(NearlinkDevice device) {
        CachedNearlinkDevice cachedDevice = findDevice(device);
        if (cachedDevice != null) {
            cachedDevice.refreshBtClass();
        }
    }

    public synchronized void onUuidChanged(NearlinkDevice device) {
        CachedNearlinkDevice cachedDevice = findDevice(device);
        if (cachedDevice != null) {
            cachedDevice.onUuidChanged();
        }
    }

    public synchronized void onNearlinkStateChanged(int nearlinkState) {
        // When Nearlink is turning off, we need to clear the non-bonded devices
        // Otherwise, they end up showing up on the next BT enable
        if (nearlinkState == NearlinkAdapter.STATE_TURNING_OFF) {
            for (int i = mCachedDevices.size() - 1; i >= 0; i--) {
                CachedNearlinkDevice cachedDevice = mCachedDevices.get(i);
                if (cachedDevice.getConnectState() != NearlinkConstant.SLE_ACB_STATE_CONNECTED) {
                    cachedDevice.setJustDiscovered(false);
                    mCachedDevices.remove(i);
                } else {
                    // For bonded devices, we need to clear the connection status so that
                    // when BT is enabled next time, device connection status shall be retrieved
                    // by making a binder call.
                    cachedDevice.clearProfileConnectionState();
                }
            }
        }
    }

    public synchronized void onActiveDeviceChanged(CachedNearlinkDevice activeDevice,
                                                   int nearlinkProfile) {
        for (CachedNearlinkDevice cachedDevice : mCachedDevices) {
            boolean isActive = Objects.equals(cachedDevice, activeDevice);
            cachedDevice.onActiveDeviceChanged(isActive, nearlinkProfile);
        }
    }

    public synchronized void onProfileConnectionStateChanged(CachedNearlinkDevice cachedDevice, int state, int nearlinkProfile) {
    }

    public synchronized void onDeviceUnpaired(CachedNearlinkDevice device) {
    }

    public synchronized void dispatchAudioModeChanged() {
        for (CachedNearlinkDevice cachedDevice : mCachedDevices) {
            cachedDevice.onAudioModeChanged();
        }
    }

    private void log(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
