/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.os.Bundle;

import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.R;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * Provides a dialog for changing the advertised name of the local nearlink adapter.
 *
 * @since 2023-12-04
 */
public class LocalDeviceNameDialogFragment extends NearlinkNameDialogFragment {
    public static final String TAG = "LocalDeviceNameDialogFragment";

    private LocalNearlinkAdapter mLocalAdapter;

    public static LocalDeviceNameDialogFragment newInstance() {
        return new LocalDeviceNameDialogFragment();
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED.equals(action)
                    || (NearlinkAdapter.ACTION_STATE_CHANGED.equals(action) && intent
                    .getIntExtra(NearlinkAdapter.EXTRA_STATE, NearlinkAdapter.ERROR) == NearlinkAdapter.STATE_ON)) {
                updateDeviceName();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LocalNearlinkManager localManager = Utils.getLocalNlManager(getActivity());
        mLocalAdapter = localManager.getNearlinkAdapter();
    }

    @Override
    public void onResume() {
        super.onResume();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_STATE_CHANGED);
        filter.addAction(NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED);
        getActivity().registerReceiver(mReceiver, filter);
    }

    @Override
    public void onPause() {
        super.onPause();
        getActivity().unregisterReceiver(mReceiver);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsProto.MetricsEvent.DIALOG_NEARLINK_RENAME;
    }

    @Override
    protected int getDialogTitle() {
        return R.string.nearlink_rename_device;
    }

    @Override
    protected String getDeviceName() {
        if (mLocalAdapter != null && mLocalAdapter.isEnabled()) {
            return mLocalAdapter.getName();
        }
        return null;
    }

    @Override
    protected void setDeviceName(String deviceName) {
        mLocalAdapter.setName(deviceName);
    }
}
