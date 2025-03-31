/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.os.UserManager;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.annotations.VisibleForTesting;
import com.android.settings.R;
import com.android.settings.widget.SwitchWidgetController;
import com.android.settingslib.RestrictedLockUtils.EnforcedAdmin;
import com.android.settingslib.WirelessUtils;
import com.android.settingslib.core.instrumentation.MetricsFeatureProvider;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * NearlinkEnabler is a helper to manage the Nearlink on/off checkbox
 * preference. It turns on/off Nearlink and ensures the summary of the
 * preference reflects the current state.
 *
 * @since 2023-12-04
 */

public final class NearlinkEnabler implements SwitchWidgetController.OnSwitchChangeListener {
    private static final String TAG = "NearlinkEnabler";

    private final SwitchWidgetController mSwitchController;

    private final MetricsFeatureProvider mMetricsFeatureProvider;

    private Context mContext;

    private boolean mValidListener;

    private final LocalNearlinkAdapter mLocalAdapter;

    private final IntentFilter mIntentFilter;

    private final RestrictionUtils mRestrictionUtils;

    private SwitchWidgetController.OnSwitchChangeListener mCallback;

    private static final String EVENT_DATA_IS_BT_ON = "is_nearlink_on";

    private static final int EVENT_UPDATE_INDEX = 0;

    private final int mMetricsEvent;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Broadcast receiver is always running on the UI thread here,
            // so we don't need consider thread synchronization.
            int state = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, NearlinkAdapter.ERROR);
            handleStateChanged(state);
        }
    };

    public NearlinkEnabler(Context context, SwitchWidgetController switchController,
                           MetricsFeatureProvider metricsFeatureProvider, LocalNearlinkManager manager, int metricsEvent) {
        this(context, switchController, metricsFeatureProvider, manager, metricsEvent, new RestrictionUtils());
    }

    public NearlinkEnabler(Context context, SwitchWidgetController switchController,
                           MetricsFeatureProvider metricsFeatureProvider, LocalNearlinkManager manager, int metricsEvent,
                           RestrictionUtils restrictionUtils) {
        mContext = context;
        mMetricsFeatureProvider = metricsFeatureProvider;
        mSwitchController = switchController;
        mSwitchController.setListener(this);
        mValidListener = false;
        mMetricsEvent = metricsEvent;

        if (manager == null) {
            // Nearlink is not supported
            mLocalAdapter = null;
            mSwitchController.setEnabled(false);
        } else {
            mLocalAdapter = manager.getNearlinkAdapter();
        }
        mIntentFilter = new IntentFilter(NearlinkAdapter.ACTION_STATE_CHANGED);
        mRestrictionUtils = restrictionUtils;
    }

    public void setupSwitchController() {
        mSwitchController.setupView();
    }

    public void teardownSwitchController() {
        mSwitchController.teardownView();
    }

    public void resume(Context context) {
        if (mContext != context) {
            mContext = context;
        }

        final boolean restricted = maybeEnforceRestrictions();

        if (mLocalAdapter == null) {
            mSwitchController.setEnabled(false);
            return;
        }

        // Nearlink state is not sticky, so set it manually
        if (!restricted) {
            handleStateChanged(mLocalAdapter.getNearlinkState());
        }

        mSwitchController.startListening();
        mContext.registerReceiver(mReceiver, mIntentFilter);
        mValidListener = true;
    }

    public void pause() {
        if (mLocalAdapter == null) {
            return;
        }
        if (mValidListener) {
            mSwitchController.stopListening();
            mContext.unregisterReceiver(mReceiver);
            mValidListener = false;
        }
    }

    void handleStateChanged(int state) {
        switch (state) {
            case NearlinkAdapter.STATE_TURNING_ON:
                mSwitchController.setEnabled(false);
                break;
            case NearlinkAdapter.STATE_ON:
                setChecked(true);
                mSwitchController.setEnabled(true);
                break;
            case NearlinkAdapter.STATE_TURNING_OFF:
                mSwitchController.setEnabled(false);
                break;
            case NearlinkAdapter.STATE_OFF:
                setChecked(false);
                mSwitchController.setEnabled(true);
                break;
            default:
                setChecked(false);
                mSwitchController.setEnabled(true);
        }
    }

    private void setChecked(boolean isChecked) {
        if (isChecked != mSwitchController.isChecked()) {
            // set listener to null, so onCheckedChanged won't be called
            // if the checked status on Switch isn't changed by user click
            if (mValidListener) {
                mSwitchController.stopListening();
            }
            mSwitchController.setChecked(isChecked);
            if (mValidListener) {
                mSwitchController.startListening();
            }
        }
    }

    @Override
    public boolean onSwitchToggled(boolean isChecked) {
        if (maybeEnforceRestrictions()) {
            triggerParentPreferenceCallback(isChecked);
            return true;
        }

        // Show toast message if Nearlink is not allowed in airplane mode
        if (isChecked && !WirelessUtils.isRadioAllowed(mContext, Settings.Global.RADIO_NEARLINK)) {
            Toast.makeText(mContext, R.string.wifi_in_airplane_mode, Toast.LENGTH_SHORT).show();
            // Reset switch to off
            mSwitchController.setChecked(false);
            triggerParentPreferenceCallback(false);
            return false;
        }

        mMetricsFeatureProvider.action(mContext, mMetricsEvent, isChecked);

        if (mLocalAdapter != null) {
            Log.e(TAG, "[onSwitchToggled] =>mLocalAdapter.setNearlinkEnabled(" + isChecked + ")");
            boolean status = mLocalAdapter.setNearlinkEnabled(isChecked);
            // If we cannot toggle it ON then reset the UI assets:
            // a) The switch should be OFF but it should still be togglable (enabled = True)
            // b) The switch bar should have OFF text.
            if (isChecked && !status) {
                mSwitchController.setChecked(false);
                mSwitchController.setEnabled(true);
                triggerParentPreferenceCallback(false);
                return false;
            }
        }
        mSwitchController.setEnabled(false);
        triggerParentPreferenceCallback(isChecked);
        return true;
    }

    /**
     * Sets a callback back that this enabler will trigger in case the preference using the enabler
     * still needed the callback on the SwitchController (which we now use).
     *
     * @param listener The listener with a callback to trigger.
     */
    public void setToggleCallback(SwitchWidgetController.OnSwitchChangeListener listener) {
        mCallback = listener;
    }

    /**
     * Enforces user restrictions disallowing Nearlink (or its configuration) if there are any.
     *
     * @return if there was any user restriction to enforce.
     */
    @VisibleForTesting
    boolean maybeEnforceRestrictions() {
        EnforcedAdmin admin = getEnforcedAdmin(mRestrictionUtils, mContext);
        mSwitchController.setDisabledByAdmin(admin);
        if (admin != null) {
            mSwitchController.setChecked(false);
            mSwitchController.setEnabled(false);
        }
        return admin != null;
    }

    public static EnforcedAdmin getEnforcedAdmin(RestrictionUtils mRestrictionUtils, Context mContext) {
        EnforcedAdmin admin = mRestrictionUtils.checkIfRestrictionEnforced(mContext, UserManager.DISALLOW_NEARLINK);
        if (admin == null) {
            admin = mRestrictionUtils.checkIfRestrictionEnforced(mContext, UserManager.DISALLOW_CONFIG_NEARLINK);
        }
        return admin;
    }

    // This triggers the callback which was manually set for this enabler since the enabler will
    // take over the switch controller callback
    private void triggerParentPreferenceCallback(boolean isChecked) {
        if (mCallback != null) {
            mCallback.onSwitchToggled(isChecked);
        }
    }
}
