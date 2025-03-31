/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.R;
import com.android.settings.core.instrumentation.InstrumentedDialogFragment;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * Implements an AlertDialog for confirming that a user wishes to unpair or "forget" a paired
 * device
 *
 * @since 2023-12-04
 */
public class ForgetDeviceDialogFragment extends InstrumentedDialogFragment {
    public static final String TAG = "ForgetNearlinkDevice";

    private static final String KEY_DEVICE_ADDRESS = "device_address";

    private CachedNearlinkDevice mDevice;

    public static ForgetDeviceDialogFragment newInstance(String deviceAddress) {
        Bundle args = new Bundle(1);
        args.putString(KEY_DEVICE_ADDRESS, deviceAddress);
        ForgetDeviceDialogFragment dialog = new ForgetDeviceDialogFragment();
        dialog.setArguments(args);
        return dialog;
    }

    @VisibleForTesting
    CachedNearlinkDevice getDevice(Context context) {
        String deviceAddress = getArguments().getString(KEY_DEVICE_ADDRESS);
        LocalNearlinkManager manager = Utils.getLocalNlManager(context);
        NearlinkDevice device = manager.getNearlinkAdapter().getRemoteDevice(deviceAddress);
        return manager.getCachedDeviceManager().findDevice(device);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsProto.MetricsEvent.DIALOG_NEARLINK_PAIRED_DEVICE_FORGET;
    }

    @Override
    public Dialog onCreateDialog(Bundle inState) {
        DialogInterface.OnClickListener onConfirm = (dialog, which) -> {
            mDevice.unpair();
            Activity activity = getActivity();
            if (activity != null) {
                activity.finish();
            }
        };
        Context context = getContext();
        mDevice = getDevice(context);
        AlertDialog dialog = new AlertDialog.Builder(context)
                .setPositiveButton(R.string.nearlink_unpair_dialog_forget_confirm_button, onConfirm)
                .setNegativeButton(android.R.string.cancel, null)
                .create();
        dialog.setTitle(R.string.nearlink_unpair_dialog_title);
        dialog.setMessage(context.getString(R.string.nearlink_unpair_dialog_body, mDevice.getName()));
        return dialog;
    }
}
