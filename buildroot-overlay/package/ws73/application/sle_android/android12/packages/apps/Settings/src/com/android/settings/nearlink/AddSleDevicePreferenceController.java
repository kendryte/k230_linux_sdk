/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.nearlink.NearlinkAdapter;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settings.R;
import com.android.settings.core.BasePreferenceController;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;

/**
 * Controller to maintain the {@link android.support.v7.preference.Preference} for add
 * device. It monitor Nearlink's status(on/off) and decide if need to show summary or not.
 *
 * @since 2023-12-04
 */
public class AddSleDevicePreferenceController extends BasePreferenceController
        implements LifecycleObserver, OnStart, OnStop {
    private static final String TAG = "AddSleDevicePreferenceController";

    private Preference mPreference;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            updateState();
        }
    };

    private IntentFilter mIntentFilter;

    private NearlinkAdapter mNearlinkAdapter;

    public AddSleDevicePreferenceController(Context context, String key) {
        super(context, key);
        mIntentFilter = new IntentFilter(NearlinkAdapter.ACTION_STATE_CHANGED);
        mNearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
    }

    @Override
    public void onStart() {
        mContext.registerReceiver(mReceiver, mIntentFilter);
    }

    @Override
    public void onStop() {
        mContext.unregisterReceiver(mReceiver);
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        Log.e(TAG, "[displayPreference] isAvailable:" + isAvailable());
        if (isAvailable()) {
            Log.e(TAG, "[displayPreference] getPreferenceKey:" + getPreferenceKey());
            mPreference = (Preference) screen.findPreference(getPreferenceKey());
        }
    }

    @Override
    public int getAvailabilityStatus() {
        return mContext.getPackageManager().hasSystemFeature(PackageManager.FEATURE_NEARLINK) ? AVAILABLE
                : UNSUPPORTED_ON_DEVICE;
    }

    @Override
    public CharSequence getSummary() {
        return mNearlinkAdapter != null && mNearlinkAdapter.isEnabled() ? ""
                : mContext.getString(R.string.connected_sledevice_add_device_summary);
    }

    void updateState() {
        updateState(mPreference);
    }
}
