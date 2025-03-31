/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.text.TextUtils;
import android.util.Log;

import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * LocalNearlinkPreferences provides an interface to the preferences
 * related to Nearlink.
 *
 * @since 2023-12-04
 */
final class LocalNearlinkPreferences {
    private static final String TAG = "LocalNearlinkPreferences";
    private static final boolean DEBUG = com.android.settings.nearlink.Utils.D;
    private static final String SHARED_PREFERENCES_NAME = "nearlink_settings";

    // If a device was picked from the device picker or was in discoverable mode
    // in the last 60 seconds, show the pairing dialogs in foreground instead
    // of raising notifications
    private static final int GRACE_PERIOD_TO_SHOW_DIALOGS_IN_FOREGROUND = 60 * 1000;

    private static final String KEY_LAST_SELECTED_DEVICE = "last_selected_device";

    private static final String KEY_LAST_SELECTED_DEVICE_TIME = "last_selected_device_time";

    private static final String KEY_DISCOVERABLE_END_TIMESTAMP = "discoverable_end_timestamp";

    private LocalNearlinkPreferences() {
    }

    private static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }

    static long getDiscoverableEndTimestamp(Context context) {
        return getSharedPreferences(context).getLong(KEY_DISCOVERABLE_END_TIMESTAMP, 0);
    }

    static boolean shouldShowDialogInForeground(Context context, String deviceAddress, String deviceName) {
        LocalNearlinkManager manager = Utils.getLocalNlManager(context);
        if (manager == null) {
            if (DEBUG) Log.v(TAG, "manager == null - do not show dialog.");
            return false;
        }

        // If Nearlink Settings is visible
        if (manager.isForegroundActivity()) {
            return true;
        }

        // If in appliance mode, do not show dialog in foreground.
        if ((context.getResources().getConfiguration().uiMode & Configuration.UI_MODE_TYPE_APPLIANCE) == Configuration.UI_MODE_TYPE_APPLIANCE) {
            if (DEBUG) Log.v(TAG, "in appliance mode - do not show dialog.");
            return false;
        }

        long currentTimeMillis = System.currentTimeMillis();
        SharedPreferences sharedPreferences = getSharedPreferences(context);

        // If the device was in discoverABLE mode recently
        long lastDiscoverableEndTime = sharedPreferences.getLong(KEY_DISCOVERABLE_END_TIMESTAMP, 0);
        if ((lastDiscoverableEndTime + GRACE_PERIOD_TO_SHOW_DIALOGS_IN_FOREGROUND) > currentTimeMillis) {
            return true;
        }

        // If the device was discoverING recently
        LocalNearlinkAdapter adapter = manager.getNearlinkAdapter();
        if (adapter != null) {
            if (adapter.isDiscovering()) {
                return true;
            }
            if ((adapter.getDiscoveryEndMillis() + GRACE_PERIOD_TO_SHOW_DIALOGS_IN_FOREGROUND) > currentTimeMillis) {
                return true;
            }
        }

        // If the device was picked in the device picker recently
        if (deviceAddress != null) {
            String lastSelectedDevice = sharedPreferences.getString(KEY_LAST_SELECTED_DEVICE, null);

            if (deviceAddress.equals(lastSelectedDevice)) {
                long lastDeviceSelectedTime = sharedPreferences.getLong(KEY_LAST_SELECTED_DEVICE_TIME, 0);
                if ((lastDeviceSelectedTime + GRACE_PERIOD_TO_SHOW_DIALOGS_IN_FOREGROUND) > currentTimeMillis) {
                    return true;
                }
            }
        }


        if (!TextUtils.isEmpty(deviceName)) {
            // If the device is a custom BT keyboard specifically for this device
            String packagedKeyboardName = context.getString(com.android.internal.R.string.config_packagedKeyboardName);
            if (deviceName.equals(packagedKeyboardName)) {
                if (DEBUG) Log.v(TAG, "showing dialog for packaged keyboard");
                return true;
            }
        }

        if (DEBUG) Log.v(TAG, "Found no reason to show the dialog - do not show dialog.");
        return false;
    }

    static void persistSelectedDeviceInPicker(Context context, String deviceAddress) {
        SharedPreferences.Editor editor = getSharedPreferences(context).edit();
        editor.putString(KEY_LAST_SELECTED_DEVICE, deviceAddress);
        editor.putLong(KEY_LAST_SELECTED_DEVICE_TIME, System.currentTimeMillis());
        editor.apply();
    }

    static void persistDiscoverableEndTimestamp(Context context, long endTimestamp) {
        SharedPreferences.Editor editor = getSharedPreferences(context).edit();
        editor.putLong(KEY_DISCOVERABLE_END_TIMESTAMP, endTimestamp);
        editor.apply();
    }
}
