/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settings.R;
import com.android.settingslib.widget.ActionButtonsPreference;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;

/**
 * This class adds two buttons: one to connect/disconnect from a device (depending on the current
 * connected state), and one to "forget" (ie unpair) the device.
 *
 * @since 2023-12-04
 */
public class NearlinkDetailsButtonsController extends NearlinkDetailsController {
    private static final String KEY_ACTION_BUTTONS = "action_buttons";

    private boolean mIsConnected;

    private boolean mConnectButtonInitialized;

    private ActionButtonsPreference mActionButtons;

    public NearlinkDetailsButtonsController(Context context, PreferenceFragmentCompat fragment, CachedNearlinkDevice device, Lifecycle lifecycle) {
        super(context, fragment, device, lifecycle);
        mIsConnected = device.isConnected();
    }

    private void onForgetButtonPressed() {
        ForgetDeviceDialogFragment fragment = ForgetDeviceDialogFragment.newInstance(mCachedDevice.getAddress());
        fragment.show(mFragment.getFragmentManager(), ForgetDeviceDialogFragment.TAG);
    }

    @Override
    protected void init(PreferenceScreen screen) {
        mActionButtons =
                ((ActionButtonsPreference) screen.findPreference(getPreferenceKey()))
        .setButton1Text(R.string.unpair)
        .setButton1OnClickListener((view) -> onForgetButtonPressed())
        .setButton1Enabled(true);
    }

    @Override
    protected void refresh() {
        mActionButtons.setButton2Enabled(!mCachedDevice.isBusy());

        boolean previouslyConnected = mIsConnected;
        mIsConnected = mCachedDevice.isConnected();
        Log.e(TAG, "[refresh] mIsConnected:" + mIsConnected);
        if (mIsConnected) {
            if (!mConnectButtonInitialized || !previouslyConnected) {
                mActionButtons.setButton2Text(R.string.nearlink_device_context_disconnect)
                        .setButton2OnClickListener((view) -> mCachedDevice.disconnect());
                mConnectButtonInitialized = true;
            }
        } else {
            if (!mConnectButtonInitialized || previouslyConnected) {
                mActionButtons.setButton2Text(R.string.nearlink_device_context_connect)
                        .setButton2OnClickListener(view -> mCachedDevice.connect(true /* connectAllProfiles */));
                mConnectButtonInitialized = true;
            }
        }
    }

    @Override
    public String getPreferenceKey() {
        return KEY_ACTION_BUTTONS;
    }
}
