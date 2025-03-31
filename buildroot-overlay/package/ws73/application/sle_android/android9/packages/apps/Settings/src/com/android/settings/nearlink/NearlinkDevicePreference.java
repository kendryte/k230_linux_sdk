/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.nearlink.NearlinkConstant;
import android.os.UserManager;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceViewHolder;
import android.text.Html;
import android.text.TextUtils;
import android.util.Log;
import android.util.Pair;
import android.util.TypedValue;
import android.widget.ImageView;
import android.widget.Toast;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.overlay.FeatureFactory;
import com.android.settings.widget.GearPreference;
import com.android.settingslib.core.instrumentation.MetricsFeatureProvider;
import com.android.settingslib.nearlink.CachedNearlinkDevice;

/**
 * NearlinkDevicePreference is the preference type used to display each remote
 * Nearlink device in the Nearlink Settings screen.
 *
 * @since 2023-12-04
 */
public final class NearlinkDevicePreference extends GearPreference implements CachedNearlinkDevice.Callback {
    private static final String TAG = "NearlinkDevicePreference";

    private static int sDimAlpha = Integer.MIN_VALUE;

    private final CachedNearlinkDevice mCachedDevice;

    private final UserManager mUserManager;

    private final boolean mShowDevicesWithoutNames;

    private AlertDialog mDisconnectDialog;

    private String contentDescription = null;

    private boolean mHideSecondTarget = false;

    /* Talk-back descriptions for various BT icons */
    Resources mResources;

    public NearlinkDevicePreference(Context context, CachedNearlinkDevice cachedDevice, boolean showDeviceWithoutNames) {
        super(context, null);
        mResources = getContext().getResources();
        mUserManager = (UserManager) context.getSystemService(Context.USER_SERVICE);
        mShowDevicesWithoutNames = showDeviceWithoutNames;

        if (sDimAlpha == Integer.MIN_VALUE) {
            TypedValue outValue = new TypedValue();
            context.getTheme().resolveAttribute(android.R.attr.disabledAlpha, outValue, true);
            sDimAlpha = (int) (outValue.getFloat() * 255);
        }

        mCachedDevice = cachedDevice;
        mCachedDevice.registerCallback(this);

        onDeviceAttributesChanged();
    }

    void rebind() {
        notifyChanged();
    }

    @Override
    protected boolean shouldHideSecondTarget() {
        return mCachedDevice == null || mCachedDevice.getBondState() != NearlinkConstant.SLE_PAIR_PAIRED
                || mHideSecondTarget;
    }

    @Override
    protected int getSecondTargetResId() {
        return R.layout.preference_widget_gear;
    }

    CachedNearlinkDevice getCachedDevice() {
        return mCachedDevice;
    }

    @Override
    protected void onPrepareForRemoval() {
        super.onPrepareForRemoval();
        mCachedDevice.unregisterCallback(this);
        if (mDisconnectDialog != null) {
            mDisconnectDialog.dismiss();
            mDisconnectDialog = null;
        }
    }

    public CachedNearlinkDevice getNearlinkDevice() {
        return mCachedDevice;
    }

    public void hideSecondTarget(boolean hideSecondTarget) {
        mHideSecondTarget = hideSecondTarget;
    }

    public void onDeviceAttributesChanged() {
        /*
         * The preference framework takes care of making sure the value has
         * changed before proceeding. It will also call notifyChanged() if
         * any preference info has changed from the previous value.
         */
        String tName = mCachedDevice.getName();
        if (tName == null || TextUtils.isEmpty(tName) || tName.equals(mCachedDevice.getAddress())) {
            setTitle(mCachedDevice.getAddress());
        } else {
            setTitle(tName + " (" + mCachedDevice.getAddress() + ")");
        }
        // Null check is done at the framework
        String summary = mCachedDevice.getConnectionSummary();
        Log.e(TAG, "[onDeviceAttributesChanged] summary:" + summary);
        setSummary(summary);

        final Pair<Drawable, String> pair = com.android.settingslib.nearlink.Utils.getNlClassDrawableWithDescription(getContext(), mCachedDevice);
        if (pair.first != null) {
            setIcon(pair.first);
            contentDescription = pair.second;
        }

        // Used to gray out the item
        Log.e(TAG, "[onDeviceAttributesChanged] mCachedDevice.isBusy:" + mCachedDevice.isBusy());
        setEnabled(!mCachedDevice.isBusy());

        // Device is only visible in the UI if it has a valid name besides MAC address or when user
        // allows showing devices without user-friendly name in developer settings
        setVisible(mShowDevicesWithoutNames || mCachedDevice.hasHumanReadableName());

        // This could affect ordering, so notify that
        notifyHierarchyChanged();
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder view) {
        // Disable this view if the nearlink enable/disable preference view is off
        if (null != findPreferenceInHierarchy("nl_checkbox")) {
            setDependency("nl_checkbox");
        }

        if (mCachedDevice.getBondState() == NearlinkConstant.SLE_PAIR_PAIRED) {
            ImageView deviceDetails = (ImageView) view.findViewById(R.id.settings_button);

            if (deviceDetails != null) {
                deviceDetails.setOnClickListener(this);
            }
        }
        final ImageView imageView = (ImageView) view.findViewById(android.R.id.icon);
        if (imageView != null) {
            imageView.setContentDescription(contentDescription);
        }
        super.onBindViewHolder(view);
    }

    @Override
    public boolean equals(Object o) {
        if ((o == null) || !(o instanceof NearlinkDevicePreference)) {
            return false;
        }
        return mCachedDevice.equals(((NearlinkDevicePreference) o).mCachedDevice);
    }

    @Override
    public int hashCode() {
        return mCachedDevice.hashCode();
    }

    @Override
    public int compareTo(Preference another) {
        if (!(another instanceof NearlinkDevicePreference)) {
            // Rely on default sort
            return super.compareTo(another);
        }

        return mCachedDevice.compareTo(((NearlinkDevicePreference) another).mCachedDevice);
    }

    void onClicked() {
        Context context = getContext();
        boolean connected = mCachedDevice.isConnected();
        int connState = mCachedDevice.getConnectState();
        Log.e(TAG, "[onClicked] connected:" + connected + ", connState:" + connState);

        final MetricsFeatureProvider metricsFeatureProvider = FeatureFactory.getFactory(context).getMetricsFeatureProvider();

        if (connected) {
            metricsFeatureProvider.action(context, MetricsEvent.ACTION_SETTINGS_NEARLINK_DISCONNECT);
            askDisconnect();
        } else {
            if (connState == NearlinkConstant.SLE_ACB_STATE_NONE
                    || connState == NearlinkConstant.SLE_ACB_STATE_DISCONNECTED) {
                metricsFeatureProvider.action(context, MetricsEvent.ACTION_SETTINGS_NEARLINK_CONNECT);
                Log.e(TAG, "[onClicked] => mCachedDevice.connect(true)");
                mCachedDevice.connect(true);
            } else if (connState != NearlinkConstant.SLE_ACB_STATE_DISCONNECTING
                    && connState != NearlinkConstant.SLE_ACB_STATE_CONNECTING) {
                Toast.makeText(context, "连接状态异常 connState:" + connState, Toast.LENGTH_SHORT).show();
            }
        }
    }

    // Show disconnect confirmation dialog for a device.
    private void askDisconnect() {
        Context context = getContext();
        String name = mCachedDevice.getName();
        if (TextUtils.isEmpty(name)) {
            name = context.getString(R.string.nearlink_device);
        }
        String message = context.getString(R.string.nearlink_disconnect_all_profiles, name);
        String title = context.getString(R.string.nearlink_disconnect_title);

        DialogInterface.OnClickListener disconnectListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                Log.e(TAG, "[askUnpair] => mCachedDevice.disconnect()");
                mCachedDevice.disconnect();
            }
        };

        mDisconnectDialog =
                Utils.showDisconnectDialog(context, mDisconnectDialog, disconnectListener, title, Html.fromHtml(message));
    }
}
