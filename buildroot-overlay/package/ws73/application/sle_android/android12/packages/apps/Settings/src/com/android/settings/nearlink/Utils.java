/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkProfile;
import android.provider.Settings;
import androidx.annotation.VisibleForTesting;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.overlay.FeatureFactory;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.nearlink.LocalNearlinkManager.NearlinkManagerCallback;
import com.android.settingslib.nearlink.Utils.ErrorListener;

/**
 * Utils is a helper class that contains constants for various
 * Android resource IDs, debug logging flags, and static methods
 * for creating dialogs.
 * <p>
 * *
 * * @since 2023-12-04
 */

public final class Utils {
    static final boolean V = com.android.settingslib.nearlink.Utils.V; // verbose logging

    static final boolean D = com.android.settingslib.nearlink.Utils.D;  // regular logging

    private Utils() {
    }

    public static int getConnectionStateSummary(int connectionState) {
        switch (connectionState) {
            case NearlinkProfile.STATE_CONNECTED:
                return R.string.nearlink_connected;
            case NearlinkProfile.STATE_CONNECTING:
                return R.string.nearlink_connecting;
            case NearlinkProfile.STATE_DISCONNECTED:
                return R.string.nearlink_disconnected;
            case NearlinkProfile.STATE_DISCONNECTING:
                return R.string.nearlink_disconnecting;
            default:
                return 0;
        }
    }

    // Create (or recycle existing) and show disconnect dialog.
    static AlertDialog showDisconnectDialog(Context context, AlertDialog dialog,
                                            DialogInterface.OnClickListener disconnectListener, CharSequence title, CharSequence message) {
        if (dialog == null) {
            dialog = new AlertDialog.Builder(context).setPositiveButton(android.R.string.ok, disconnectListener)
                    .setNegativeButton(android.R.string.cancel, null)
                    .create();
        } else {
            if (dialog.isShowing()) {
                dialog.dismiss();
            }
            // use disconnectListener for the correct profile(s)
            CharSequence okText = context.getText(android.R.string.ok);
            dialog.setButton(DialogInterface.BUTTON_POSITIVE, okText, disconnectListener);
        }
        dialog.setTitle(title);
        dialog.setMessage(message);
        dialog.show();
        return dialog;
    }

    // TODO: wire this up to show connection errors...
    static void showConnectingError(Context context, String name) {
        showConnectingError(context, name, getLocalNlManager(context));
    }

    @VisibleForTesting
    static void showConnectingError(Context context, String name, LocalNearlinkManager manager) {
        FeatureFactory.getFactory(context)
                .getMetricsFeatureProvider()
                .visible(context, MetricsEvent.VIEW_UNKNOWN, MetricsEvent.ACTION_SETTINGS_NEARLINK_CONNECT_ERROR, 0);
        showError(context, name, R.string.nearlink_connecting_error_message, manager);
    }

    static void showError(Context context, String name, int messageResId) {
        showError(context, name, messageResId, getLocalNlManager(context));
    }

    private static void showError(Context context, String name, int messageResId, LocalNearlinkManager manager) {
        String message = context.getString(messageResId, name);
        Context activity = manager.getForegroundActivity();
        if (manager.isForegroundActivity()) {
            new AlertDialog.Builder(activity).setTitle(R.string.nearlink_error_title)
                    .setMessage(message)
                    .setPositiveButton(android.R.string.ok, null)
                    .show();
        } else {
            Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
        }
    }

    public static LocalNearlinkManager getLocalNlManager(Context context) {
        return LocalNearlinkManager.getInstance(context, mOnInitCallback);
    }

    public static String createRemoteName(Context context, NearlinkDevice device) {
        String mRemoteName = device != null ? device.getAliasName() : null;

        if (mRemoteName == null) {
            mRemoteName = context.getString(R.string.unknown);
        }
        return mRemoteName;
    }

    private static final ErrorListener mErrorListener = new ErrorListener() {
        @Override
        public void onShowError(Context context, String name, int messageResId) {
            showError(context, name, messageResId);
        }
    };

    private static final NearlinkManagerCallback mOnInitCallback = new NearlinkManagerCallback() {
        @Override
        public void onNearlinkManagerInitialized(Context appContext, LocalNearlinkManager nlManager) {
            com.android.settingslib.nearlink.Utils.setErrorListener(mErrorListener);
        }
    };

    public static boolean isNearlinkScanningEnabled(Context context) {
        return Settings.Global.getInt(context.getContentResolver(), Settings.Global.SLE_SCAN_ALWAYS_AVAILABLE, 0) == 1;
    }

    static void printStackTrace(String tag) {
        StackTraceElement[] stackElements = new Throwable().getStackTrace();
        if (stackElements != null) {
            for (int i = 0; i < stackElements.length; i++) {
                Log.e(tag, "" + stackElements[i]);
            }
        }
    }
}
