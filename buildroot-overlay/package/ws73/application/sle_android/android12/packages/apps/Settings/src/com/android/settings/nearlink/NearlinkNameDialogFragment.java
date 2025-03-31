/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.android.settings.R;
import com.android.settings.core.instrumentation.InstrumentedDialogFragment;

/**
 * Dialog fragment for renaming a Nearlink device.
 *
 * @since 2023-12-04
 */
abstract class NearlinkNameDialogFragment extends InstrumentedDialogFragment implements TextWatcher {
    private AlertDialog mAlertDialog;

    private Button mOkButton;

    EditText mDeviceNameView;

    // This flag is set when the name is updated by code, to distinguish from user changes
    private boolean mDeviceNameUpdated;

    // This flag is set when the user edits the name (preserved on rotation)
    private boolean mDeviceNameEdited;

    // Key to save the edited name and edit status for restoring after rotation
    private static final String KEY_NAME = "device_name";

    private static final String KEY_NAME_EDITED = "device_name_edited";

    /**
     * @return the title to use for the dialog.
     */
    abstract protected int getDialogTitle();

    /**
     * @return the current name used for this device.
     */
    abstract protected String getDeviceName();

    /**
     * Set the device to the given name.
     *
     * @param deviceName the name to use
     */
    abstract protected void setDeviceName(String deviceName);

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        String deviceName = getDeviceName();
        if (savedInstanceState != null) {
            deviceName = savedInstanceState.getString(KEY_NAME, deviceName);
            mDeviceNameEdited = savedInstanceState.getBoolean(KEY_NAME_EDITED, false);
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity()).setTitle(getDialogTitle()).setView(createDialogView(deviceName)).setPositiveButton(R.string.nearlink_rename_button, (dialog, which) -> {
            setDeviceName(mDeviceNameView.getText().toString());
        }).setNegativeButton(android.R.string.cancel, null);
        mAlertDialog = builder.create();
        mAlertDialog.setOnShowListener(d -> {
            if (mDeviceNameView != null && mDeviceNameView.requestFocus()) {
                InputMethodManager imm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.showSoftInput(mDeviceNameView, InputMethodManager.SHOW_IMPLICIT);
                }
            }
        });

        return mAlertDialog;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putString(KEY_NAME, mDeviceNameView.getText().toString());
        outState.putBoolean(KEY_NAME_EDITED, mDeviceNameEdited);
    }

    private View createDialogView(String deviceName) {
        final LayoutInflater layoutInflater = (LayoutInflater) getActivity().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = layoutInflater.inflate(R.layout.dialog_edittext, null);
        mDeviceNameView = (EditText) view.findViewById(R.id.edittext);
        mDeviceNameView.setFilters(new InputFilter[]{new NearlinkLengthDeviceNameFilter()});
        mDeviceNameView.setText(deviceName);    // set initial value before adding listener
        if (!TextUtils.isEmpty(deviceName)) {
            mDeviceNameView.setSelection(deviceName.length());
        }
        mDeviceNameView.addTextChangedListener(this);
        com.android.settings.Utils.setEditTextCursorPosition(mDeviceNameView);
        mDeviceNameView.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    setDeviceName(v.getText().toString());
                    mAlertDialog.dismiss();
                    return true;    // action handled
                } else {
                    return false;   // not handled
                }
            }
        });
        return view;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mAlertDialog = null;
        mDeviceNameView = null;
        mOkButton = null;
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mOkButton == null) {
            mOkButton = mAlertDialog.getButton(DialogInterface.BUTTON_POSITIVE);
            mOkButton.setEnabled(mDeviceNameEdited);    // Ok button enabled after user edits
        }
    }

    void updateDeviceName() {
        String name = getDeviceName();
        if (name != null) {
            mDeviceNameUpdated = true;
            mDeviceNameEdited = false;
            mDeviceNameView.setText(name);
        }
    }

    public void afterTextChanged(Editable s) {
        if (mDeviceNameUpdated) {
            // Device name changed by code; disable Ok button until edited by user
            mDeviceNameUpdated = false;
            mOkButton.setEnabled(false);
        } else {
            mDeviceNameEdited = true;
            if (mOkButton != null) {
                mOkButton.setEnabled(s.toString().trim().length() != 0);
            }
        }
    }

    public void onConfigurationChanged(Configuration newConfig, CharSequence s) {
        super.onConfigurationChanged(newConfig);
        if (mOkButton != null) {
            mOkButton.setEnabled(s.length() != 0 && !(s.toString().trim().isEmpty()));
        }
    }

    /* Not used */
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
    }

    /* Not used */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
    }
}
