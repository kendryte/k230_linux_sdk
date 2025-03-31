/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;
import android.support.annotation.VisibleForTesting;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.R;
import com.android.settings.dashboard.RestrictedDashboardFragment;
import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.LocalNearlinkManager;

import static android.os.UserManager.DISALLOW_CONFIG_NEARLINK;

/**
 * 功能描述
 *
 * @since 2023-12-04
 */
public class NearlinkDeviceDetailsFragment extends RestrictedDashboardFragment {
    public static final String KEY_DEVICE_ADDRESS = "device_address";

    private static final String TAG = "NearlinkDeviceDetailsFragment";

    @VisibleForTesting
    static int EDIT_DEVICE_NAME_ITEM_ID = Menu.FIRST;

    /**
     * An interface to let tests override the normal mechanism for looking up the
     * CachedNearlinkDevice and LocalNearlinkManager, and substitute their own mocks instead.
     * This is only needed in situations where you instantiate the fragment indirectly (eg via an
     * intent) and can't use something like spying on an instance you construct directly via
     * newInstance.
     */
    @VisibleForTesting
    interface TestDataFactory {
        CachedNearlinkDevice getDevice(String deviceAddress);

        LocalNearlinkManager getManager(Context context);
    }

    @VisibleForTesting
    static TestDataFactory sTestDataFactory;

    private String mDeviceAddress;

    private LocalNearlinkManager mManager;

    private CachedNearlinkDevice mCachedDevice;

    public NearlinkDeviceDetailsFragment() {
        super(DISALLOW_CONFIG_NEARLINK);
    }

    @VisibleForTesting
    LocalNearlinkManager getLocalNearlinkManager(Context context) {
        if (sTestDataFactory != null) {
            return sTestDataFactory.getManager(context);
        }
        return Utils.getLocalNlManager(context);
    }

    @VisibleForTesting
    CachedNearlinkDevice getCachedDevice(String deviceAddress) {
        if (sTestDataFactory != null) {
            return sTestDataFactory.getDevice(deviceAddress);
        }
        NearlinkDevice remoteDevice = mManager.getNearlinkAdapter().getRemoteDevice(deviceAddress);
        return mManager.getCachedDeviceManager().findDevice(remoteDevice);
    }

    public static NearlinkDeviceDetailsFragment newInstance(String deviceAddress) {
        Bundle args = new Bundle(1);
        args.putString(KEY_DEVICE_ADDRESS, deviceAddress);
        NearlinkDeviceDetailsFragment fragment = new NearlinkDeviceDetailsFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onAttach(Context context) {
        mDeviceAddress = getArguments().getString(KEY_DEVICE_ADDRESS);
        mManager = getLocalNearlinkManager(context);
        mCachedDevice = getCachedDevice(mDeviceAddress);
        super.onAttach(context);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsProto.MetricsEvent.NEARLINK_DEVICE_DETAILS;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }

    @Override
    protected int getPreferenceScreenResId() {
        return R.xml.nearlink_device_details_fragment;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        MenuItem item = menu.add(0, EDIT_DEVICE_NAME_ITEM_ID, 0, R.string.nearlink_rename_button);
        item.setIcon(R.drawable.ic_mode_edit);
        item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        if (menuItem.getItemId() == EDIT_DEVICE_NAME_ITEM_ID) {
            RemoteDeviceNameDialogFragment.newInstance(mCachedDevice)
                    .show(getFragmentManager(), RemoteDeviceNameDialogFragment.TAG);
            return true;
        }
        return super.onOptionsItemSelected(menuItem);
    }

    @Override
    protected List<AbstractPreferenceController> createPreferenceControllers(Context context) {
        ArrayList<AbstractPreferenceController> controllers = new ArrayList<>();

        if (mCachedDevice != null) {
            Lifecycle lifecycle = getLifecycle();
            controllers.add(new NearlinkDetailsHeaderController(context, this, mCachedDevice, lifecycle, mManager));
            controllers.add(new NearlinkDetailsButtonsController(context, this, mCachedDevice, lifecycle));
            controllers.add(new NearlinkDetailsProfilesController(context, this, mManager, mCachedDevice, lifecycle));
            controllers.add(new NearlinkDetailsMacAddressController(context, this, mCachedDevice, lifecycle));
        }
        return controllers;
    }
}
