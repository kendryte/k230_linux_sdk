/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.annotation.NonNull;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;

import com.android.settings.R;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.nearlink.NearlinkDiscoverableTimeoutReceiver;

/**
 * RequestPermissionActivity asks the user whether to enable discovery. This is
 * usually started by an application wanted to start nearlink and or discovery
 *
 * @since 2023-12-04
 */
public class RequestPermissionActivity extends Activity implements DialogInterface.OnClickListener {
    // Command line to test this
    // adb shell am start -a android.nearlink.adapter.action.REQUEST_ENABLE
    // adb shell am start -a android.nearlink.adapter.action.REQUEST_DISCOVERABLE
    // adb shell am start -a android.nearlink.adapter.action.REQUEST_DISABLE

    private static final String TAG = "RequestPermissionActivity";

    private static final int MAX_DISCOVERABLE_TIMEOUT = 3600; // 1 hr

    static final int REQUEST_ENABLE = 1;
    static final int REQUEST_ENABLE_DISCOVERABLE = 2;
    static final int REQUEST_DISABLE = 3;

    private LocalNearlinkAdapter mLocalAdapter;

    private int mTimeout = NearlinkDiscoverableEnabler.DEFAULT_DISCOVERABLE_TIMEOUT;

    private int mRequest;

    private AlertDialog mDialog;

    private BroadcastReceiver mReceiver;

    private @NonNull
    CharSequence mAppLabel;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.e(TAG, "[onCreate]");
        setResult(Activity.RESULT_CANCELED);

        // Note: initializes mLocalAdapter and returns true on error
        if (parseIntent()) {
            finish();
            return;
        }

        int nlState = mLocalAdapter.getState();
        Log.e(TAG, "[onCreate] nlState:" + nlState);
        if (mRequest == REQUEST_DISABLE) {
            switch (nlState) {
                case NearlinkAdapter.STATE_OFF:
                case NearlinkAdapter.STATE_TURNING_OFF: {
                    proceedAndFinish();
                }
                break;

                case NearlinkAdapter.STATE_ON:
                case NearlinkAdapter.STATE_TURNING_ON: {
                    Intent intent = new Intent(this, com.android.settings.nearlink.RequestPermissionHelperActivity.class);
                    intent.putExtra(com.android.settings.nearlink.RequestPermissionHelperActivity.EXTRA_APP_LABEL, mAppLabel);
                    intent.setAction(com.android.settings.nearlink.RequestPermissionHelperActivity.ACTION_INTERNAL_REQUEST_NL_OFF);

                    startActivityForResult(intent, 0);
                }
                break;

                default: {
                    Log.e(TAG, "Unknown adapter nlState: " + nlState);
                    cancelAndFinish();
                }
                break;
            }
        } else {
            switch (nlState) {
                case NearlinkAdapter.STATE_OFF:
                case NearlinkAdapter.STATE_TURNING_OFF:
                case NearlinkAdapter.STATE_TURNING_ON: {
                    /*
                     * Strictly speaking STATE_TURNING_ON belong with STATE_ON;
                     * however, BT may not be ready when the user clicks yes and we
                     * would fail to turn on discovery mode. By kicking this to the
                     * RequestPermissionHelperActivity, this class will handle that
                     * case via the broadcast receiver.
                     */

                    /*
                     * Start the helper activity to:
                     * 1) ask the user about enabling bt AND discovery
                     * 2) enable BT upon confirmation
                     */
                    Intent intent = new Intent(this, com.android.settings.nearlink.RequestPermissionHelperActivity.class);
                    intent.setAction(com.android.settings.nearlink.RequestPermissionHelperActivity.ACTION_INTERNAL_REQUEST_NL_ON);
                    intent.putExtra(RequestPermissionHelperActivity.EXTRA_APP_LABEL, mAppLabel);
                    if (mRequest == REQUEST_ENABLE_DISCOVERABLE) {
                        intent.putExtra(NearlinkAdapter.EXTRA_DISCOVERABLE_DURATION, mTimeout);
                    }
                    startActivityForResult(intent, 0);
                }
                break;

                case NearlinkAdapter.STATE_ON: {
                    if (mRequest == REQUEST_ENABLE) {
                        // Nothing to do. Already enabled.
                        proceedAndFinish();
                    } else {
                        // Ask the user about enabling discovery mode
                        createDialog();
                    }
                }
                break;

                default: {
                    Log.e(TAG, "Unknown adapter nlState: " + nlState);
                    cancelAndFinish();
                }
                break;
            }
        }
    }

    private void createDialog() {
        if (getResources().getBoolean(R.bool.auto_confirm_nearlink_activation_dialog)) {
            onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        // Non-null receiver means we are toggling
        if (mReceiver != null) {
            switch (mRequest) {
                case REQUEST_ENABLE:
                case REQUEST_ENABLE_DISCOVERABLE: {
                    builder.setMessage(getString(R.string.nearlink_turning_on));
                }
                break;

                default: {
                    builder.setMessage(getString(R.string.nearlink_turning_off));
                }
                break;
            }
            builder.setCancelable(false);
        } else {
            // Ask the user whether to turn on discovery mode or not
            // For lasting discoverable mode there is a different message
            if (mTimeout == NearlinkDiscoverableEnabler.DISCOVERABLE_TIMEOUT_NEVER) {
                CharSequence message = mAppLabel != null
                        ? getString(R.string.nearlink_ask_lasting_discovery, mAppLabel)
                        : getString(R.string.nearlink_ask_lasting_discovery_no_name);
                builder.setMessage(message);
            } else {
                CharSequence message = mAppLabel != null
                        ? getString(R.string.nearlink_ask_discovery, mAppLabel, mTimeout)
                        : getString(R.string.nearlink_ask_discovery_no_name, mTimeout);
                builder.setMessage(message);
            }
            builder.setPositiveButton(getString(R.string.allow), this);
            builder.setNegativeButton(getString(R.string.deny), this);
        }

        mDialog = builder.create();
        mDialog.show();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode != Activity.RESULT_OK) {
            cancelAndFinish();
            return;
        }

        Log.e(TAG, "[onActivityResult] mRequest:" + mRequest);
        switch (mRequest) {
            case REQUEST_ENABLE:
            case REQUEST_ENABLE_DISCOVERABLE: {
                if (mLocalAdapter.getNearlinkState() == NearlinkAdapter.STATE_ON) {
                    proceedAndFinish();
                } else {
                    // If BT is not up yet, show "Turning on Nearlink..."
                    mReceiver = new StateChangeReceiver();
                    registerReceiver(mReceiver, new IntentFilter(
                            NearlinkAdapter.ACTION_STATE_CHANGED));
                    createDialog();
                }
            }
            break;

            case REQUEST_DISABLE: {
                if (mLocalAdapter.getNearlinkState() == NearlinkAdapter.STATE_OFF) {
                    proceedAndFinish();
                } else {
                    // If BT is not up yet, show "Turning off Nearlink..."
                    mReceiver = new StateChangeReceiver();
                    registerReceiver(mReceiver, new IntentFilter(
                            NearlinkAdapter.ACTION_STATE_CHANGED));
                    createDialog();
                }
            }
            break;

            default: {
                cancelAndFinish();
            }
            break;
        }
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                proceedAndFinish();
                break;

            case DialogInterface.BUTTON_NEGATIVE:
                setResult(RESULT_CANCELED);
                finish();
                break;
        }
    }

    private void proceedAndFinish() {
        int returnCode;

        Log.e(TAG, "[proceedAndFinish] mRequest:" + mRequest);
        if (mRequest == REQUEST_ENABLE || mRequest == REQUEST_DISABLE) {
            // BT toggled. Done
            returnCode = RESULT_OK;
        } else if (mLocalAdapter.setScanMode(
                NearlinkAdapter.ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE, mTimeout)) {
            // If already in discoverable mode, this will extend the timeout.
            long endTime = System.currentTimeMillis() + (long) mTimeout * 1000;
            LocalNearlinkPreferences.persistDiscoverableEndTimestamp(
                    this, endTime);
            if (0 < mTimeout) {
                NearlinkDiscoverableTimeoutReceiver.setDiscoverableAlarm(this, endTime);
            }
            returnCode = mTimeout;
            // Activity.RESULT_FIRST_USER should be 1
            if (returnCode < RESULT_FIRST_USER) {
                returnCode = RESULT_FIRST_USER;
            }
        } else {
            returnCode = RESULT_CANCELED;
        }

        if (mDialog != null) {
            mDialog.dismiss();
        }

        Log.e(TAG, "[proceedAndFinish] returnCode:" + returnCode);
        setResult(returnCode);
        finish();
    }

    private void cancelAndFinish() {
        setResult(Activity.RESULT_CANCELED);
        finish();
    }

    /**
     * Parse the received Intent and initialize mLocalNearlinkAdapter.
     *
     * @return true if an error occurred; false otherwise
     */
    private boolean parseIntent() {
        Intent intent = getIntent();
        if (intent == null) {
            return true;
        }
        String action = intent.getAction();
        Log.e(TAG, "[parseIntent] action:" + action);
        if (action.equals(NearlinkAdapter.ACTION_REQUEST_ENABLE)) {
            mRequest = REQUEST_ENABLE;
        } else if (action.equals(NearlinkAdapter.ACTION_REQUEST_DISABLE)) {
            mRequest = REQUEST_DISABLE;
        } else if (action.equals(NearlinkAdapter.ACTION_REQUEST_DISCOVERABLE)) {
            mRequest = REQUEST_ENABLE_DISCOVERABLE;
            mTimeout = intent.getIntExtra(NearlinkAdapter.EXTRA_DISCOVERABLE_DURATION, NearlinkDiscoverableEnabler.DEFAULT_DISCOVERABLE_TIMEOUT);

            Log.e(TAG, "Setting Nearlink Discoverable Timeout = " + mTimeout);

            if (mTimeout < 1 || mTimeout > MAX_DISCOVERABLE_TIMEOUT) {
                mTimeout = NearlinkDiscoverableEnabler.DEFAULT_DISCOVERABLE_TIMEOUT;
            }
        } else {
            Log.e(TAG, "Error: this activity may be started only with intent "
                    + NearlinkAdapter.ACTION_REQUEST_ENABLE + " or "
                    + NearlinkAdapter.ACTION_REQUEST_DISCOVERABLE);
            setResult(RESULT_CANCELED);
            return true;
        }

        LocalNearlinkManager manager = Utils.getLocalNlManager(this);
        if (manager == null) {
            Log.e(TAG, "Error: there's a problem starting Nearlink");
            setResult(RESULT_CANCELED);
            return true;
        }

        String packageName = getCallingPackage();
        if (TextUtils.isEmpty(packageName)) {
            packageName = getIntent().getStringExtra(Intent.EXTRA_PACKAGE_NAME);
        }
        if (!TextUtils.isEmpty(packageName)) {
            try {
                ApplicationInfo applicationInfo = getPackageManager().getApplicationInfo(packageName, 0);
                mAppLabel = applicationInfo.loadSafeLabel(getPackageManager());
            } catch (PackageManager.NameNotFoundException e) {
                Log.e(TAG, "Couldn't find app with package name " + packageName);
                setResult(RESULT_CANCELED);
                return true;
            }
        }

        mLocalAdapter = manager.getNearlinkAdapter();

        return false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
            mReceiver = null;
        }
        Log.e(TAG, "[onDestroy] ");
    }

    @Override
    public void onBackPressed() {
        setResult(RESULT_CANCELED);
        super.onBackPressed();
    }

    private final class StateChangeReceiver extends BroadcastReceiver {
        private static final long TOGGLE_TIMEOUT_MILLIS = 10000; // 10 sec

        public StateChangeReceiver() {
            getWindow().getDecorView().postDelayed(() -> {
                if (!isFinishing() && !isDestroyed()) {
                    cancelAndFinish();
                }
            }, TOGGLE_TIMEOUT_MILLIS);
        }

        public void onReceive(Context context, Intent intent) {
            if (intent == null) {
                return;
            }
            final int currentState = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, NearlinkDevice.ERROR);
            Log.e(TAG, "[onReceive] currentState:" + currentState);
            switch (mRequest) {
                case REQUEST_ENABLE:
                case REQUEST_ENABLE_DISCOVERABLE: {
                    if (currentState == NearlinkAdapter.STATE_ON) {
                        proceedAndFinish();
                    }
                }
                break;

                case REQUEST_DISABLE: {
                    if (currentState == NearlinkAdapter.STATE_OFF) {
                        proceedAndFinish();
                    }
                }
                break;
            }
        }
    }
}
