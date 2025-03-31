/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;
import android.support.v7.preference.Preference;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.settings.R;

/**
 * NearlinkPermissionActivity shows a dialog for accepting incoming
 * profile connection request from untrusted devices.
 * It is also used to show a dialogue for accepting incoming phonebook
 * read request. The request could be initiated by PBAP PCE or by HF AT+CPBR.
 *
 * @since 2023-12-04
 */
public class NearlinkPermissionActivity extends AlertActivity implements
        DialogInterface.OnClickListener, Preference.OnPreferenceChangeListener {
    private static final String TAG = "NearlinkPermissionActivity";
    private static final boolean DEBUG = com.android.settings.nearlink.Utils.D;

    private View mView;
    private TextView messageView;
    private Button mOkButton;
    private NearlinkDevice mDevice;
    private String mReturnPackage = null;
    private String mReturnClass = null;

    private int mRequestType = 0;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(NearlinkDevice.ACTION_CONNECTION_ACCESS_CANCEL)) {
                int requestType = intent.getIntExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, NearlinkDevice.REQUEST_TYPE_PHONEBOOK_ACCESS);
                if (requestType != mRequestType) return;
                NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                if (mDevice.equals(device)) dismissDialog();
            }
        }
    };
    private boolean mReceiverRegistered = false;

    private void dismissDialog() {
        this.dismiss();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent i = getIntent();
        String action = i.getAction();
        if (!action.equals(NearlinkDevice.ACTION_CONNECTION_ACCESS_REQUEST)) {
            Log.e(TAG, "Error: this activity may be started only with intent "
                    + "ACTION_CONNECTION_ACCESS_REQUEST");
            finish();
            return;
        }

        mDevice = i.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
        mReturnPackage = i.getStringExtra(NearlinkDevice.EXTRA_PACKAGE_NAME);
        mReturnClass = i.getStringExtra(NearlinkDevice.EXTRA_CLASS_NAME);
        mRequestType = i.getIntExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, NearlinkDevice.REQUEST_TYPE_PHONEBOOK_ACCESS);

        if (DEBUG) Log.i(TAG, "onCreate() Request type: " + mRequestType);

        if (mRequestType == NearlinkDevice.REQUEST_TYPE_PROFILE_CONNECTION) {
            showDialog(getString(R.string.nearlink_connection_permission_request), mRequestType);
        } else {
            Log.e(TAG, "Error: bad request type: " + mRequestType);
            finish();
            return;
        }
        registerReceiver(mReceiver, new IntentFilter(NearlinkDevice.ACTION_CONNECTION_ACCESS_CANCEL));
        mReceiverRegistered = true;
    }


    private void showDialog(String title, int requestType) {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = title;
        if (DEBUG) Log.i(TAG, "showDialog() Request type: " + mRequestType + " this: " + this);
        switch (requestType) {
            case NearlinkDevice.REQUEST_TYPE_PROFILE_CONNECTION:
                p.mView = createConnectionDialogView();
                break;
        }
        p.mPositiveButtonText = getString(R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.no);
        p.mNegativeButtonListener = this;
        mOkButton = mAlert.getButton(DialogInterface.BUTTON_POSITIVE);
        setupAlert();

    }

    @Override
    public void onBackPressed() {
        /*we need an answer so ignore back button presses during auth */
        if (DEBUG) Log.i(TAG, "Back button pressed! ignoring");
        return;
    }

    // TODO(edjee): createConnectionDialogView, createPhonebookDialogView and createMapDialogView
    // are similar. Refactor them into one method.
    // Also, the string resources nearlink_remember_choice and nearlink_pb_remember_choice should
    // be removed.
    private View createConnectionDialogView() {
        String mRemoteName = com.android.settings.nearlink.Utils.createRemoteName(this, mDevice);
        mView = getLayoutInflater().inflate(R.layout.nearlink_access, null);
        messageView = (TextView) mView.findViewById(R.id.message);
        messageView.setText(getString(R.string.nearlink_connection_dialog_text,
                mRemoteName));
        return mView;
    }

    private void onPositive() {
        if (DEBUG) Log.d(TAG, "onPositive");
        sendReplyIntentToReceiver(true, true);
        finish();
    }

    private void onNegative() {
        if (DEBUG) Log.d(TAG, "onNegative");

        boolean always = true;
        sendReplyIntentToReceiver(false, always);
    }

    private void sendReplyIntentToReceiver(final boolean allowed, final boolean always) {
        Intent intent = new Intent(NearlinkDevice.ACTION_CONNECTION_ACCESS_REPLY);

        if (mReturnPackage != null && mReturnClass != null) {
            intent.setClassName(mReturnPackage, mReturnClass);
        }
        if (DEBUG) Log.i(TAG, "sendReplyIntentToReceiver() Request type: " + mRequestType +
                " mReturnPackage" + mReturnPackage + " mReturnClass" + mReturnClass);

        intent.putExtra(NearlinkDevice.EXTRA_CONNECTION_ACCESS_RESULT,
                allowed ? NearlinkDevice.CONNECTION_ACCESS_YES
                        : NearlinkDevice.CONNECTION_ACCESS_NO);
        intent.putExtra(NearlinkDevice.EXTRA_ALWAYS_ALLOWED, always);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, mDevice);
        intent.putExtra(NearlinkDevice.EXTRA_ACCESS_REQUEST_TYPE, mRequestType);
        sendBroadcast(intent, android.Manifest.permission.NEARLINK_ADMIN);
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onPositive();
                break;

            case DialogInterface.BUTTON_NEGATIVE:
                onNegative();
                break;
            default:
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mReceiverRegistered) {
            unregisterReceiver(mReceiver);
            mReceiverRegistered = false;
        }
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }
}
