/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.text.BidiFormatter;
import android.text.TextUtils;
import android.util.Log;

import com.android.settings.R;
import com.android.settings.core.BasePreferenceController;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * Controller that shows and updates the nearlink device name
 *
 * @since 2023-12-04
 */
public class NearlinkDeviceNamePreferenceController extends BasePreferenceController
        implements LifecycleObserver, OnStart, OnStop {
    private static final String TAG = "NearlinkDeviceNamePreferenceController";

    @VisibleForTesting
    Preference mPreference;

    private LocalNearlinkManager mLocalManager;

    protected LocalNearlinkAdapter mLocalAdapter;

    /**
     * Constructor exclusively used for Slice.
     */
    public NearlinkDeviceNamePreferenceController(Context context, String preferenceKey) {
        super(context, preferenceKey);

        mLocalManager = Utils.getLocalNlManager(context);
        if (mLocalManager == null) {
            Log.e(TAG, "Nearlink is not supported on this device");
            return;
        }
        mLocalAdapter = mLocalManager.getNearlinkAdapter();
    }

    @VisibleForTesting
    NearlinkDeviceNamePreferenceController(Context context, LocalNearlinkAdapter localAdapter, String preferenceKey) {
        super(context, preferenceKey);
        mLocalAdapter = localAdapter;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        mPreference = screen.findPreference(getPreferenceKey());
        super.displayPreference(screen);
    }

    @Override
    public void onStart() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction( NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED);
        intentFilter.addAction(NearlinkAdapter.ACTION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onStop() {
        mContext.unregisterReceiver(mReceiver);
    }

    @Override
    public int getAvailabilityStatus() {
        return mLocalAdapter != null ? AVAILABLE : UNSUPPORTED_ON_DEVICE;
    }

    @Override
    public void updateState(Preference preference) {
        updatePreferenceState(preference);
    }

    @Override
    public CharSequence getSummary() {
        String deviceName = getDeviceName();
        if (TextUtils.isEmpty(deviceName)) {
            return super.getSummary();
        }

        return TextUtils
                .expandTemplate(mContext.getText(R.string.nearlink_device_name_summary),
                        BidiFormatter.getInstance().unicodeWrap(deviceName))
                .toString();
    }

    /**
     * Update device summary with {@code deviceName}, where {@code deviceName} has accent color
     *
     * @param preference to set the summary for
     */
    protected void updatePreferenceState(final Preference preference) {
        preference.setSelectable(false);
        preference.setSummary(getSummary());
    }

    protected String getDeviceName() {
        return mLocalAdapter.getName();
    }

    /**
     * Receiver that listens to {@link NearlinkDevice#ACTION_NAME_CHANGED} and updates the
     * device name if possible
     */
    @VisibleForTesting
    final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (TextUtils.equals(action, NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED)) {
                if (mPreference != null && mLocalAdapter != null && mLocalAdapter.isEnabled()) {
                    updatePreferenceState(mPreference);
                }
            } else if (TextUtils.equals(action, NearlinkAdapter.ACTION_STATE_CHANGED)) {
                int newState = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, -1);
                if (newState == NearlinkAdapter.STATE_ON) {
                    updatePreferenceState(mPreference);
                }
            }
        }
    };
}
