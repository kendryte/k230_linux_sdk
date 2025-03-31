/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.nearlink.NearlinkAdapter;
import android.os.Handler;
import android.os.SystemProperties;
import androidx.preference.Preference;
import android.util.Log;

import com.android.settings.R;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.NearlinkDiscoverableTimeoutReceiver;

/**
 * NearlinkDiscoverableEnabler is a helper to manage the "Discoverable"
 * checkbox. It sets/unsets discoverability and keeps track of how much time
 * until the the discoverability is automatically turned off.
 */
final class NearlinkDiscoverableEnabler implements Preference.OnPreferenceClickListener {

    private static final String TAG = "NearlinkDiscoverableEnabler";

    private static final String SYSTEM_PROPERTY_DISCOVERABLE_TIMEOUT =
            "debug.bt.discoverable_time";

    private static final int DISCOVERABLE_TIMEOUT_TWO_MINUTES = 120;
    private static final int DISCOVERABLE_TIMEOUT_FIVE_MINUTES = 300;
    private static final int DISCOVERABLE_TIMEOUT_ONE_HOUR = 3600;
    static final int DISCOVERABLE_TIMEOUT_NEVER = 0;

    // Nearlink advanced settings screen was replaced with action bar items.
    // Use the same preference key for discoverable timeout as the old ListPreference.
    private static final String KEY_DISCOVERABLE_TIMEOUT = "bt_discoverable_timeout";

    private static final String VALUE_DISCOVERABLE_TIMEOUT_TWO_MINUTES = "twomin";
    private static final String VALUE_DISCOVERABLE_TIMEOUT_FIVE_MINUTES = "fivemin";
    private static final String VALUE_DISCOVERABLE_TIMEOUT_ONE_HOUR = "onehour";
    private static final String VALUE_DISCOVERABLE_TIMEOUT_NEVER = "never";

    static final int DEFAULT_DISCOVERABLE_TIMEOUT = DISCOVERABLE_TIMEOUT_TWO_MINUTES;

    private Context mContext;
    private final Handler mUiHandler;
    private final Preference mDiscoveryPreference;

    private final LocalNearlinkAdapter mLocalAdapter;

    private final SharedPreferences mSharedPreferences;

    private boolean mDiscoverable;
    private int mNumberOfPairedDevices;

    private int mTimeoutSecs = -1;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (NearlinkAdapter.ACTION_ANNOUNCE_MODE_CHANGED.equals(intent.getAction())) {
                int mode = intent.getIntExtra(NearlinkAdapter.EXTRA_ANNOUNCE_MODE,
                        NearlinkAdapter.ERROR);
                if (mode != NearlinkAdapter.ERROR) {
                    handleModeChanged(mode);
                }
            }
        }
    };

    private final Runnable mUpdateCountdownSummaryRunnable = new Runnable() {
        public void run() {
            updateCountdownSummary();
        }
    };

    NearlinkDiscoverableEnabler(LocalNearlinkAdapter adapter,
                                Preference discoveryPreference) {
        mUiHandler = new Handler();
        mLocalAdapter = adapter;
        mDiscoveryPreference = discoveryPreference;
        mSharedPreferences = discoveryPreference.getSharedPreferences();
        discoveryPreference.setPersistent(false);
    }

    public void resume(Context context) {
        if (mLocalAdapter == null) {
            return;
        }

        if (mContext != context) {
            mContext = context;
        }

        IntentFilter filter = new IntentFilter(NearlinkAdapter.ACTION_ANNOUNCE_MODE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
        mDiscoveryPreference.setOnPreferenceClickListener(this);
        handleModeChanged(mLocalAdapter.getScanMode());
    }

    public void pause() {
        if (mLocalAdapter == null) {
            return;
        }

        mUiHandler.removeCallbacks(mUpdateCountdownSummaryRunnable);
        mContext.unregisterReceiver(mReceiver);
        mDiscoveryPreference.setOnPreferenceClickListener(null);
    }

    public boolean onPreferenceClick(Preference preference) {
        // toggle discoverability
        mDiscoverable = !mDiscoverable;
        setEnabled(mDiscoverable);
        return true;
    }

    private void setEnabled(boolean enable) {
        if (enable) {
            int timeout = getDiscoverableTimeout();
            long endTimestamp = System.currentTimeMillis() + timeout * 1000L;
            LocalNearlinkPreferences.persistDiscoverableEndTimestamp(mContext, endTimestamp);

            mLocalAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE, timeout);
            updateCountdownSummary();

            Log.d(TAG, "setEnabled(): enabled = " + enable + "timeout = " + timeout);

            if (timeout > 0) {
                NearlinkDiscoverableTimeoutReceiver.setDiscoverableAlarm(mContext, endTimestamp);
            } else {
                NearlinkDiscoverableTimeoutReceiver.cancelDiscoverableAlarm(mContext);
            }

        } else {
            mLocalAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE);
            NearlinkDiscoverableTimeoutReceiver.cancelDiscoverableAlarm(mContext);
        }
    }

    private void updateTimerDisplay(int timeout) {
        if (getDiscoverableTimeout() == DISCOVERABLE_TIMEOUT_NEVER) {
            mDiscoveryPreference.setSummary(R.string.nearlink_is_discoverable_always);
        } else {
            String textTimeout = formatTimeRemaining(timeout);
            mDiscoveryPreference.setSummary(mContext.getString(R.string.nearlink_is_discoverable,
                    textTimeout));
        }
    }

    private static String formatTimeRemaining(int timeout) {
        StringBuilder sb = new StringBuilder(6);    // "mmm:ss"
        int min = timeout / 60;
        sb.append(min).append(':');
        int sec = timeout - (min * 60);
        if (sec < 10) {
            sb.append('0');
        }
        sb.append(sec);
        return sb.toString();
    }

    void setDiscoverableTimeout(int index) {
        String timeoutValue;
        switch (index) {
            case 0:
            default:
                mTimeoutSecs = DISCOVERABLE_TIMEOUT_TWO_MINUTES;
                timeoutValue = VALUE_DISCOVERABLE_TIMEOUT_TWO_MINUTES;
                break;

            case 1:
                mTimeoutSecs = DISCOVERABLE_TIMEOUT_FIVE_MINUTES;
                timeoutValue = VALUE_DISCOVERABLE_TIMEOUT_FIVE_MINUTES;
                break;

            case 2:
                mTimeoutSecs = DISCOVERABLE_TIMEOUT_ONE_HOUR;
                timeoutValue = VALUE_DISCOVERABLE_TIMEOUT_ONE_HOUR;
                break;

            case 3:
                mTimeoutSecs = DISCOVERABLE_TIMEOUT_NEVER;
                timeoutValue = VALUE_DISCOVERABLE_TIMEOUT_NEVER;
                break;
        }
        mSharedPreferences.edit().putString(KEY_DISCOVERABLE_TIMEOUT, timeoutValue).apply();
        setEnabled(true);   // enable discovery and reset timer
    }

    private int getDiscoverableTimeout() {
        if (mTimeoutSecs != -1) {
            return mTimeoutSecs;
        }

        int timeout = SystemProperties.getInt(SYSTEM_PROPERTY_DISCOVERABLE_TIMEOUT, -1);
        if (timeout < 0) {
            String timeoutValue = mSharedPreferences.getString(KEY_DISCOVERABLE_TIMEOUT,
                    VALUE_DISCOVERABLE_TIMEOUT_TWO_MINUTES);

            if (timeoutValue.equals(VALUE_DISCOVERABLE_TIMEOUT_NEVER)) {
                timeout = DISCOVERABLE_TIMEOUT_NEVER;
            } else if (timeoutValue.equals(VALUE_DISCOVERABLE_TIMEOUT_ONE_HOUR)) {
                timeout = DISCOVERABLE_TIMEOUT_ONE_HOUR;
            } else if (timeoutValue.equals(VALUE_DISCOVERABLE_TIMEOUT_FIVE_MINUTES)) {
                timeout = DISCOVERABLE_TIMEOUT_FIVE_MINUTES;
            } else {
                timeout = DISCOVERABLE_TIMEOUT_TWO_MINUTES;
            }
        }
        mTimeoutSecs = timeout;
        return timeout;
    }

    int getDiscoverableTimeoutIndex() {
        int timeout = getDiscoverableTimeout();
        switch (timeout) {
            case DISCOVERABLE_TIMEOUT_TWO_MINUTES:
            default:
                return 0;

            case DISCOVERABLE_TIMEOUT_FIVE_MINUTES:
                return 1;

            case DISCOVERABLE_TIMEOUT_ONE_HOUR:
                return 2;

            case DISCOVERABLE_TIMEOUT_NEVER:
                return 3;
        }
    }

    void setNumberOfPairedDevices(int pairedDevices) {
        mNumberOfPairedDevices = pairedDevices;
        handleModeChanged(mLocalAdapter.getScanMode());
    }

    void handleModeChanged(int mode) {
        Log.d(TAG, "handleModeChanged(): mode = " + mode);
        if (mode == NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE) {
            mDiscoverable = true;
            updateCountdownSummary();
        } else {
            mDiscoverable = false;
            setSummaryNotDiscoverable();
        }
    }

    private void setSummaryNotDiscoverable() {
        if (mNumberOfPairedDevices != 0) {
            mDiscoveryPreference.setSummary(R.string.nearlink_only_visible_to_paired_devices);
        } else {
            mDiscoveryPreference.setSummary(R.string.nearlink_not_visible_to_other_devices);
        }
    }

    private void updateCountdownSummary() {
        int mode = mLocalAdapter.getScanMode();
        if (mode != NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE) {
            return;
        }

        long currentTimestamp = System.currentTimeMillis();
        long endTimestamp = LocalNearlinkPreferences.getDiscoverableEndTimestamp(mContext);

        if (currentTimestamp > endTimestamp) {
            // We're still in discoverable mode, but maybe there isn't a timeout.
            updateTimerDisplay(0);
            return;
        }

        int timeLeft = (int) ((endTimestamp - currentTimestamp) / 1000L);
        updateTimerDisplay(timeLeft);

        synchronized (this) {
            mUiHandler.removeCallbacks(mUpdateCountdownSummaryRunnable);
            mUiHandler.postDelayed(mUpdateCountdownSummaryRunnable, 1000);
        }
    }
}
