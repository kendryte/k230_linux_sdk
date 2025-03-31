/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

/* Required to handle timeout notification when phone is suspended */

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.nearlink.NearlinkAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.android.settingslib.nearlink.LocalNearlinkAdapter;

/**
 * @since 2023-12-04
 */
public class NearlinkDiscoverableTimeoutReceiver extends BroadcastReceiver {
    private static final String TAG = "NLDTimeoutReceiver";

    private static final String INTENT_DISCOVERABLE_TIMEOUT = "android.nearlink.intent.DISCOVERABLE_TIMEOUT";

    public static void setDiscoverableAlarm(Context context, long alarmTime) {
        Log.d(TAG, "setDiscoverableAlarm(): alarmTime = " + alarmTime);

        Intent intent = new Intent(INTENT_DISCOVERABLE_TIMEOUT);
        intent.setClass(context, NearlinkDiscoverableTimeoutReceiver.class);
        PendingIntent pending = PendingIntent.getBroadcast(context, 0, intent, 0);
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        if (pending != null) {
            // Cancel any previous alarms that do the same thing.
            alarmManager.cancel(pending);
            Log.d(TAG, "setDiscoverableAlarm(): cancel prev alarm");
        }
        pending = PendingIntent.getBroadcast(context, 0, intent, 0);

        alarmManager.set(AlarmManager.RTC_WAKEUP, alarmTime, pending);
    }

    public static void cancelDiscoverableAlarm(Context context) {
        Log.d(TAG, "cancelDiscoverableAlarm(): Enter");

        Intent intent = new Intent(INTENT_DISCOVERABLE_TIMEOUT);
        intent.setClass(context, NearlinkDiscoverableTimeoutReceiver.class);
        PendingIntent pending = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_NO_CREATE);
        if (pending != null) {
            // Cancel any previous alarms that do the same thing.
            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            alarmManager.cancel(pending);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction() == null || !intent.getAction().equals(INTENT_DISCOVERABLE_TIMEOUT)) {
            return;
        }
        LocalNearlinkAdapter localNearlinkAdapter = LocalNearlinkAdapter.getInstance();
        if (localNearlinkAdapter != null && localNearlinkAdapter.getState() == NearlinkAdapter.STATE_ON) {
            Log.d(TAG, "Disable discoverable...");
            localNearlinkAdapter.setScanMode(NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE);
        } else {
            Log.e(TAG, "localNearlinkAdapter is NULL!!");
        }
    }
};
