/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.R;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * Provides a dialog for changing the display name of a remote nearlink device.
 *
 * @since 2023-12-04
 */
public class RemoteDeviceNameDialogFragment extends NearlinkNameDialogFragment {
    public static final String TAG = "RemoteDeviceName";

    private static final String KEY_CACHED_DEVICE_ADDRESS = "cached_device";

    private CachedNearlinkDevice mDevice;

    public static RemoteDeviceNameDialogFragment newInstance(CachedNearlinkDevice device) {
        Bundle args = new Bundle(1);
        args.putString(KEY_CACHED_DEVICE_ADDRESS, device.getDevice().getAddress());
        RemoteDeviceNameDialogFragment fragment = new RemoteDeviceNameDialogFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @VisibleForTesting
    CachedNearlinkDevice getDevice(Context context) {
        String deviceAddress = getArguments().getString(KEY_CACHED_DEVICE_ADDRESS);
        LocalNearlinkManager manager = Utils.getLocalNlManager(context);
        NearlinkDevice device = manager.getNearlinkAdapter().getRemoteDevice(deviceAddress);
        return manager.getCachedDeviceManager().findDevice(device);
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mDevice = getDevice(context);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsProto.MetricsEvent.DIALOG_NEARLINK_PAIRED_DEVICE_RENAME;
    }

    @Override
    protected int getDialogTitle() {
        return R.string.nearlink_device_name;
    }

    @Override
    protected String getDeviceName() {
        if (mDevice != null) {
            return mDevice.getName();
        }
        return null;
    }

    @Override
    protected void setDeviceName(String deviceName) {
        if (mDevice != null) {
            mDevice.setName(deviceName);
        }
    }
}
