/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.List;

import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkHidHost;
import android.nearlink.NearlinkProfile;
import android.util.Log;

import com.android.settingslib.R;

/**
 * HidProfile handles Nearlink HID profile.
 *
 * @since 2023-12-04
 */
public class HidProfile implements LocalNearlinkProfile {
    private static final String TAG = "HidProfile";
    private static boolean V = true;

    private NearlinkHidHost mService;
    private boolean mIsProfileReady;

    private final LocalNearlinkAdapter mLocalAdapter;
    private final CachedNearlinkDeviceManager mDeviceManager;
    private final LocalNearlinkProfileManager mProfileManager;

    static final String NAME = "HID";

    // Order of this profile in device profiles list
    // 该值同 NearlinkProfile.HID_HOST = 3
    private static final int ORDINAL = 3;

    // These callbacks run on the main thread.
    private final class HidHostServiceListener implements NearlinkProfile.ServiceListener {

        public void onServiceConnected(int profile, NearlinkProfile proxy) {
            if (V) Log.d(TAG, "Nearlink service connected");
            mService = (NearlinkHidHost) proxy;
            // We just bound to the service, so refresh the UI for any connected HID devices.
            List<NearlinkDevice> deviceList = mService.getConnectedDevices();
            while (!deviceList.isEmpty()) {
                NearlinkDevice nextDevice = deviceList.remove(0);
                CachedNearlinkDevice device = mDeviceManager.findDevice(nextDevice);
                // we may add a new device here, but generally this should not happen
                if (device == null) {
                    Log.w(TAG, "HidProfile found new device: " + nextDevice);
                    device = mDeviceManager.addDevice(mLocalAdapter, mProfileManager, nextDevice);
                }
                device.onProfileStateChanged(HidProfile.this, NearlinkProfile.STATE_CONNECTED);
                device.refresh();
            }
            mIsProfileReady = true;
        }

        public void onServiceDisconnected(int profile) {
            if (V) Log.d(TAG, "Nearlink service disconnected");
            mIsProfileReady = false;
        }
    }

    public boolean isProfileReady() {
        return mIsProfileReady;
    }

    @Override
    public int getProfileId() {
        return NearlinkProfile.HID_HOST;
    }

    HidProfile(Context context, LocalNearlinkAdapter adapter, CachedNearlinkDeviceManager deviceManager, LocalNearlinkProfileManager profileManager) {
        mLocalAdapter = adapter;
        mDeviceManager = deviceManager;
        mProfileManager = profileManager;
        adapter.getProfileProxy(context, new HidHostServiceListener(), NearlinkProfile.HID_HOST);
    }

    public boolean isConnectable() {
        return true;
    }

    public boolean isAutoConnectable() {
        return true;
    }

    public boolean connect(NearlinkDevice device) {
        if (mService == null) return false;
        Log.e(TAG, "[connect] device:" + device);
        Log.e(TAG, "[connect] =>NearlinkHidHost.connect(device):");
        return mService.connect(device);
    }

    public boolean disconnect(NearlinkDevice device) {
        if (mService == null) return false;
        Log.e(TAG, "[disconnect] device:" + device);
        Log.e(TAG, "[disconnect] =>NearlinkHidHost.disconnect(device):");
        return mService.disconnect(device);
    }

    public int getConnectionStatus(NearlinkDevice device) {
        if (mService == null) {
            return NearlinkProfile.STATE_DISCONNECTED;
        }
        List<NearlinkDevice> deviceList = mService.getConnectedDevices();
        return !deviceList.isEmpty() && deviceList.get(0).equals(device) ? mService.getConnectionState(device) : NearlinkProfile.STATE_DISCONNECTED;
    }

    public boolean isPreferred(NearlinkDevice device) {
        if (mService == null) return false;
        Log.e(TAG, "[isPreferred] device:" + device);
        Log.e(TAG, "[isPreferred] getPriority(device):" + mService.getPriority(device));
        return mService.getPriority(device) != NearlinkProfile.PRIORITY_OFF;
    }

    public int getPreferred(NearlinkDevice device) {
        if (mService == null) return NearlinkProfile.PRIORITY_OFF;
        return mService.getPriority(device);
    }

    public void setPreferred(NearlinkDevice device, boolean preferred) {
        if (mService == null) return;
        if (preferred) {
            if (mService.getPriority(device) < NearlinkProfile.PRIORITY_ON) {
                mService.setPriority(device, NearlinkProfile.PRIORITY_ON);
            }
        } else {
            mService.setPriority(device, NearlinkProfile.PRIORITY_OFF);
        }
    }

    public String toString() {
        return NAME;
    }

    public int getOrdinal() {
        return ORDINAL;
    }

    public int getNameResource(NearlinkDevice device) {
        // TODO: distinguish between keyboard and mouse?
        return R.string.nearlink_profile_hid;
    }

    public int getSummaryResourceForDevice(NearlinkDevice device) {
        int state = getConnectionStatus(device);
        switch (state) {
            case NearlinkProfile.STATE_DISCONNECTED:
                return R.string.nearlink_hid_profile_summary_use_for;

            case NearlinkProfile.STATE_CONNECTED:
                return R.string.nearlink_hid_profile_summary_connected;

            default:
                return Utils.getConnectionStateSummary(state);
        }
    }

    public int getDrawableResource(int nlClass) {
        if (nlClass == NearlinkAppearance.UNKNOWN) {
            return R.drawable.ic_lockscreen_ime;
        }
        return getHidClassDrawable(nlClass);
    }

    public static int getHidClassDrawable(int nlClass) {
        switch (nlClass) {
            case NearlinkAppearance.MOUSE:
                return R.drawable.ic_bt_pointing_hid;
            default:
                return R.drawable.ic_bt_misc_hid;
        }
    }

    protected void finalize() {
        if (V) Log.d(TAG, "finalize()");
        if (mService != null) {
            try {
                NearlinkAdapter.getDefaultAdapter().closeProfileProxy(NearlinkProfile.HID_HOST, mService);
                mService = null;
            } catch (Throwable t) {
                Log.w(TAG, "Error cleaning up HID proxy", t);
            }
        }
    }
}
