/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import androidx.annotation.VisibleForTesting;

import com.android.settingslib.nearlink.LocalNearlinkAdapter;

/**
 * Helper class, intended to be used by an Activity, to keep the local Nearlink adapter in
 * discoverable mode indefinitely. By default setting the scan mode to
 * NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE will time out after some time, but some
 * Nearlink settings pages would like to keep the device discoverable as long as the page is
 * visible.
 *
 * @since 2023-12-04
 */
public class AlwaysDiscoverable extends BroadcastReceiver {
    private static final String TAG = "AlwaysDiscoverable";

    private Context mContext;

    private LocalNearlinkAdapter mLocalAdapter;

    private IntentFilter mIntentFilter;

    @VisibleForTesting
    boolean mStarted;

    public AlwaysDiscoverable(Context context, LocalNearlinkAdapter localAdapter) {
        mContext = context;
        mLocalAdapter = localAdapter;
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(NearlinkAdapter.ACTION_ANNOUNCE_MODE_CHANGED);
    }

    /**
     * After calling start(), consumers should make a matching call to stop() when they no longer
     * wish to enforce discoverable mode.
     */
    public void start() {
        if (mStarted) {
            return;
        }
        mContext.registerReceiver(this, mIntentFilter);
        mStarted = true;
        if (mLocalAdapter.getScanMode() != NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE) {
            mLocalAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE);
        }
    }

    public void stop() {
        if (!mStarted) {
            return;
        }
        mContext.unregisterReceiver(this);
        mStarted = false;
        mLocalAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action != NearlinkAdapter.ACTION_ANNOUNCE_MODE_CHANGED) {
            return;
        }
        if (mLocalAdapter.getScanMode() != NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE) {
            mLocalAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE);
        }
    }
}
