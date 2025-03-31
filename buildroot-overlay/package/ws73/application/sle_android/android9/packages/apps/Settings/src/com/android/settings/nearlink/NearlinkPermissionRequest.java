/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.nearlink.NearlinkDevice;
import android.os.PowerManager;
import android.os.UserManager;
import android.util.Log;

import com.android.settings.R;

/**
 * NearlinkPermissionRequest is a receiver to receive Nearlink connection
 * access request.
 *
 * @since 2023-12-04
 */
public final class NearlinkPermissionRequest extends BroadcastReceiver {

    private static final String TAG = "NearlinkPermissionRequest";
    private static final boolean DEBUG = com.android.settings.nearlink.Utils.V;
    private static final int NOTIFICATION_ID = R.drawable.stat_sys_data_nearlink;

    /* TODO: Consolidate this multiple defined but common channel ID with other
     * handlers that declare and use the same channel ID */
    private static final String NEARLINK_NOTIFICATION_CHANNEL = "nearlink_notification_channel";

    private NotificationChannel mNotificationChannel = null;

    Context mContext;
    int mRequestType;
    NearlinkDevice mDevice;
    String mReturnPackage = null;
    String mReturnClass = null;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();

        if (DEBUG) Log.d(TAG, "onReceive" + action);

        if (action.equals(NearlinkDevice.ACTION_CONNECTION_ACCESS_REQUEST)) {
            UserManager um = (UserManager) context.getSystemService(Context.USER_SERVICE);
            // skip the notification for managed profiles.
            if (um.isManagedProfile()) {
                if (DEBUG) Log.d(TAG, "Blocking notification for managed profile.");
                return;
            }
            // convert broadcast intent into activity intent (same action string)
            mDevice = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
            mRequestType = intent.getIntExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, NearlinkDevice.REQUEST_TYPE_PROFILE_CONNECTION);
            mReturnPackage = intent.getStringExtra(NearlinkDevice.EXTRA_PACKAGE_NAME);
            mReturnClass = intent.getStringExtra(NearlinkDevice.EXTRA_CLASS_NAME);

            if (DEBUG) Log.d(TAG, "onReceive request type: " + mRequestType + " return "
                    + mReturnPackage + "," + mReturnClass);

            // Even if the user has already made the choice, Nearlink still may not know that if
            // the user preference data have not been migrated from Settings app's shared
            // preferences to Nearlink app's. In that case, Nearlink app broadcasts an
            // ACTION_CONNECTION_ACCESS_REQUEST intent to ask to Settings app.
            //
            // If that happens, 'checkUserChoice()' here will do migration because it finds or
            // creates a 'CachedNearlinkDevice' object for the device.
            //
            // After migration is done, 'checkUserChoice()' replies to the request by sending an
            // ACTION_CONNECTION_ACCESS_REPLY intent. And we don't need to start permission activity
            // dialog or notification.
            if (checkUserChoice()) {
                return;
            }

            Intent connectionAccessIntent = new Intent(action);
            connectionAccessIntent.setClass(context, NearlinkPermissionActivity.class);
            // We use the FLAG_ACTIVITY_MULTIPLE_TASK since we can have multiple concurrent access
            // requests.
            connectionAccessIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            // This is needed to create two pending intents to the same activity. The value is not
            // used in the activity.
            connectionAccessIntent.setType(Integer.toString(mRequestType));
            connectionAccessIntent.putExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, mRequestType);
            connectionAccessIntent.putExtra(NearlinkDevice.EXTRA_DEVICE, mDevice);
            connectionAccessIntent.putExtra(NearlinkDevice.EXTRA_PACKAGE_NAME, mReturnPackage);
            connectionAccessIntent.putExtra(NearlinkDevice.EXTRA_CLASS_NAME, mReturnClass);

            String deviceAddress = mDevice != null ? mDevice.getAddress() : null;
            String deviceName = mDevice != null ? mDevice.getName() : null;
            String title = null;
            String message = null;
            PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);

            if (powerManager.isScreenOn() && LocalNearlinkPreferences.shouldShowDialogInForeground(context, deviceAddress, deviceName)) {
                context.startActivity(connectionAccessIntent);
            } else {
                // Put up a notification that leads to the dialog

                // Create an intent triggered by clicking on the
                // "Clear All Notifications" button

                Intent deleteIntent = new Intent(NearlinkDevice.ACTION_CONNECTION_ACCESS_REPLY);
                deleteIntent.putExtra(NearlinkDevice.EXTRA_DEVICE, mDevice);
                deleteIntent.putExtra(NearlinkDevice.EXTRA_CONNECTION_ACCESS_RESULT, NearlinkDevice.CONNECTION_ACCESS_NO);
                deleteIntent.putExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, mRequestType);
                String deviceAlias = com.android.settings.nearlink.Utils.createRemoteName(context, mDevice);
                switch (mRequestType) {
                    default:
                        title = context.getString(R.string.nearlink_connection_permission_request);
                        message = context.getString(R.string.nearlink_connection_dialog_text, deviceAlias);
                        break;
                }
                NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
                if (mNotificationChannel == null) {
                    mNotificationChannel = new NotificationChannel(NEARLINK_NOTIFICATION_CHANNEL, context.getString(R.string.nearlink), NotificationManager.IMPORTANCE_HIGH);
                    notificationManager.createNotificationChannel(mNotificationChannel);
                }
                Notification notification = new Notification.Builder(context,
                        NEARLINK_NOTIFICATION_CHANNEL)
                        .setContentTitle(title)
                        .setTicker(message)
                        .setContentText(message)
                        .setSmallIcon(R.drawable.stat_sys_data_nearlink)
                        .setAutoCancel(true)
                        .setPriority(Notification.PRIORITY_MAX)
                        .setOnlyAlertOnce(false)
                        .setDefaults(Notification.DEFAULT_ALL)
                        .setContentIntent(PendingIntent.getActivity(context, 0, connectionAccessIntent, 0))
                        .setDeleteIntent(PendingIntent.getBroadcast(context, 0, deleteIntent, 0))
                        .setColor(context.getColor(com.android.internal.R.color.system_notification_accent_color))
                        .setLocalOnly(true)
                        .build();

                notification.flags |= Notification.FLAG_NO_CLEAR; // Cannot be set with the builder.

                notificationManager.notify(getNotificationTag(mRequestType), NOTIFICATION_ID, notification);
            }
        } else if (action.equals(NearlinkDevice.ACTION_CONNECTION_ACCESS_CANCEL)) {
            // Remove the notification
            NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
            mRequestType = intent.getIntExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, NearlinkDevice.REQUEST_TYPE_PHONEBOOK_ACCESS);
            manager.cancel(getNotificationTag(mRequestType), NOTIFICATION_ID);
        }
    }

    private String getNotificationTag(int requestType) {
        return null;
    }

    /**
     * @return true user had made a choice, this method replies to the request according
     * to user's previous decision
     * false user hadnot made any choice on this device
     */
    private boolean checkUserChoice() {
        boolean processed = false;
        return processed;
    }

    private void sendReplyIntentToReceiver(final boolean allowed) {
        Intent intent = new Intent(NearlinkDevice.ACTION_CONNECTION_ACCESS_REPLY);

        if (mReturnPackage != null && mReturnClass != null) {
            intent.setClassName(mReturnPackage, mReturnClass);
        }

        intent.putExtra(NearlinkDevice.EXTRA_CONNECTION_ACCESS_RESULT, allowed ? NearlinkDevice.CONNECTION_ACCESS_YES : NearlinkDevice.CONNECTION_ACCESS_NO);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, mDevice);
        intent.putExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, mRequestType);
        mContext.sendBroadcast(intent, Manifest.permission.NEARLINK_ADMIN);
    }
}
