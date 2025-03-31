/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.Activity;
import android.app.admin.DevicePolicyManager;
import android.content.DialogInterface;
import android.content.Intent;
import android.nearlink.NearlinkAdapter;
import android.os.Bundle;
import android.os.UserManager;
import android.util.Log;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.settings.R;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;

/**
 * RequestPermissionHelperActivity asks the user whether to toggle Nearlink.
 *
 * @since 2023-12-04
 */
public class RequestPermissionHelperActivity extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "RequestPermissionHelperActivity";

    public static final String ACTION_INTERNAL_REQUEST_NL_ON = "com.android.settings.nearlink.ACTION_INTERNAL_REQUEST_NL_ON";

    public static final String ACTION_INTERNAL_REQUEST_NL_OFF = "com.android.settings.nearlink.ACTION_INTERNAL_REQUEST_NL_OFF";

    public static final String EXTRA_APP_LABEL = "com.android.settings.nearlink.extra.APP_LABEL";

    private LocalNearlinkAdapter mLocalAdapter;

    private CharSequence mAppLabel;

    private int mTimeout = -1;

    private int mRequest;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.e(TAG, "[onCreate] ");
        setResult(RESULT_CANCELED);

        // Note: initializes mLocalAdapter and returns true on error
        if (!parseIntent()) {
            finish();
            return;
        }

        if (getResources().getBoolean(R.bool.auto_confirm_nearlink_activation_dialog)) {
            // Don't even show the dialog if configured this way
            onClick(null, BUTTON_POSITIVE);
            dismiss();
        }

        createDialog();
    }

    void createDialog() {
        final AlertController.AlertParams p = mAlertParams;

        switch (mRequest) {
            case RequestPermissionActivity.REQUEST_ENABLE: {
                if (mTimeout < 0) {
                    p.mMessage = mAppLabel != null ? getString(R.string.nearlink_ask_enablement, mAppLabel) : getString(R.string.nearlink_ask_enablement_no_name);
                } else if (mTimeout == NearlinkDiscoverableEnabler.DISCOVERABLE_TIMEOUT_NEVER) {
                    p.mMessage = mAppLabel != null ? getString(R.string.nearlink_ask_enablement_and_lasting_discovery, mAppLabel) : getString(R.string.nearlink_ask_enablement_and_lasting_discovery_no_name);
                } else {
                    p.mMessage = mAppLabel != null ? getString(R.string.nearlink_ask_enablement_and_discovery, mAppLabel, mTimeout) : getString(R.string.nearlink_ask_enablement_and_discovery_no_name, mTimeout);
                }
            }
            break;

            case RequestPermissionActivity.REQUEST_DISABLE: {
                p.mMessage = mAppLabel != null ? getString(R.string.nearlink_ask_disablement, mAppLabel) : getString(R.string.nearlink_ask_disablement_no_name);
            }
            break;
        }

        p.mPositiveButtonText = getString(R.string.allow);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.deny);

        setupAlert();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (mRequest) {
            case RequestPermissionActivity.REQUEST_ENABLE:
            case RequestPermissionActivity.REQUEST_ENABLE_DISCOVERABLE: {
                UserManager userManager = getSystemService(UserManager.class);
                if (userManager.hasUserRestriction(UserManager.DISALLOW_NEARLINK)) {
                    // If Nearlink is disallowed, don't try to enable it, show policy transparency
                    // message instead.
                    DevicePolicyManager dpm = getSystemService(DevicePolicyManager.class);
                    Intent intent = dpm.createAdminSupportIntent(UserManager.DISALLOW_NEARLINK);
                    if (intent != null) {
                        startActivity(intent);
                    }
                } else {
                    mLocalAdapter.enable();
                    setResult(Activity.RESULT_OK);
                }
            }
            break;

            case RequestPermissionActivity.REQUEST_DISABLE: {
                mLocalAdapter.disable();
                setResult(Activity.RESULT_OK);
            }
            break;
        }
    }

    /**
     * Parse the received Intent and initialize mLocalNearlinkAdapter.
     *
     * @return true if an error occurred; false otherwise
     */
    private boolean parseIntent() {
        Intent intent = getIntent();
        if (intent == null) {
            return false;
        }

        String action = intent.getAction();
        Log.e(TAG, "[parseIntent] action:" + action);
        if (ACTION_INTERNAL_REQUEST_NL_ON.equals(action)) {
            mRequest = RequestPermissionActivity.REQUEST_ENABLE;
            if (intent.hasExtra(NearlinkAdapter.EXTRA_DISCOVERABLE_DURATION)) {
                // Value used for display purposes. Not range checking.
                mTimeout = intent.getIntExtra(NearlinkAdapter.EXTRA_DISCOVERABLE_DURATION, NearlinkDiscoverableEnabler.DEFAULT_DISCOVERABLE_TIMEOUT);
            }
        } else if (ACTION_INTERNAL_REQUEST_NL_OFF.equals(action)) {
            mRequest = RequestPermissionActivity.REQUEST_DISABLE;
        } else {
            return false;
        }

        LocalNearlinkManager manager = Utils.getLocalNlManager(this);
        if (manager == null) {
            Log.e(TAG, "Error: there's a problem starting Nearlink");
            return false;
        }

        mAppLabel = getIntent().getCharSequenceExtra(EXTRA_APP_LABEL);
        mLocalAdapter = manager.getNearlinkAdapter();
        return true;
    }
}
