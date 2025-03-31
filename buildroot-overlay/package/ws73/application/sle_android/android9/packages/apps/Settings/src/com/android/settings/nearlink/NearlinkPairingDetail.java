/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;
import android.support.annotation.VisibleForTesting;
import android.util.Log;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.search.Indexable;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.NearlinkDeviceFilter;
import com.android.settingslib.widget.FooterPreference;

import static android.os.UserManager.DISALLOW_CONFIG_NEARLINK;

/**
 * NearlinkPairingDetail is a page to scan nearlink devices and pair them.
 *
 * @since 2023-12-04
 */
public class NearlinkPairingDetail extends DeviceListPreferenceFragment implements Indexable {
    private static final String TAG = "NearlinkPairingDetail";

    @VisibleForTesting
    static final String KEY_AVAIL_DEVICES = "available_devices";

    @VisibleForTesting
    static final String KEY_FOOTER_PREF = "footer_preference";

    @VisibleForTesting
    NearlinkProgressCategory mAvailableDevicesCategory;

    @VisibleForTesting
    FooterPreference mFooterPreference;

    @VisibleForTesting
    AlwaysDiscoverable mAlwaysDiscoverable;

    private boolean mInitialScanStarted;

    public NearlinkPairingDetail() {
        super(DISALLOW_CONFIG_NEARLINK);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mInitialScanStarted = false;
        mAlwaysDiscoverable = new AlwaysDiscoverable(getContext(), mLocalAdapter);
    }

    @Override
    public void onStart() {
        super.onStart();
        if (mLocalManager == null) {
            Log.e(TAG, "Nearlink is not supported on this device");
            return;
        }
        Log.e(TAG, "[onStart] =>updateNearlink()");
        updateNearlink();
        mAvailableDevicesCategory.setProgress(mLocalAdapter.isDiscovering());
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        use(NearlinkDeviceRenamePreferenceController.class).setFragment(this);
    }

    @VisibleForTesting
    void updateNearlink() {
        if (mLocalAdapter.isEnabled()) {
            updateContent(mLocalAdapter.getNearlinkState());
        } else {
            // Turn on nearlink if it is disabled
            mLocalAdapter.enable();
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mLocalManager == null) {
            Log.e(TAG, "Nearlink is not supported on this device");
            return;
        }
        // Make the device only visible to connected devices.
        mAlwaysDiscoverable.stop();
        disableScanning();
    }

    @Override
    void initPreferencesFromPreferenceScreen() {
        mAvailableDevicesCategory = (NearlinkProgressCategory) findPreference(KEY_AVAIL_DEVICES);
        mFooterPreference = (FooterPreference) findPreference(KEY_FOOTER_PREF);
        mFooterPreference.setSelectable(false);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.NEARLINK_PAIRING;
    }

    @Override
    void enableScanning() {
        // Clear all device states before first scan
        if (!mInitialScanStarted) {
            if (mAvailableDevicesCategory != null) {
                mLocalManager.getCachedDeviceManager().clearNonBondedDevices();
                removeAllDevices();
            }
            mInitialScanStarted = true;
        }
        super.enableScanning();
    }

    @Override
    public void onScanningStateChanged(boolean started) {
        super.onScanningStateChanged(started);
        Log.e(TAG, "[onScanningStateChanged] 0 started:" + started + ", mScanEnabled:" + mScanEnabled);
        started |= mScanEnabled;
        Log.e(TAG, "[onScanningStateChanged] 1 started:" + started);
        mAvailableDevicesCategory.setProgress(started);
    }

    @VisibleForTesting
    void updateContent(int nearlinkState) {
        Log.e(TAG, "[updateContent] nearlinkState:" + nearlinkState);
        switch (nearlinkState) {
            case NearlinkAdapter.STATE_ON:
                mDevicePreferenceMap.clear();
                mLocalAdapter.setNearlinkEnabled(true);

                addDeviceCategory(mAvailableDevicesCategory, R.string.nearlink_preference_found_media_devices, NearlinkDeviceFilter.ALL_FILTER);
                updateFooterPreference(mFooterPreference);
                mAlwaysDiscoverable.start();
                enableScanning();
                break;

            case NearlinkAdapter.STATE_OFF:
                finish();
                break;
        }
    }

    @Override
    public void onNearlinkStateChanged(int nearlinkState) {
        super.onNearlinkStateChanged(nearlinkState);
        Log.e(TAG, "[onNearlinkStateChanged] nearlinkState:" + nearlinkState);
        updateContent(nearlinkState);
    }

    @Override
    public void onDeviceBondStateChanged(CachedNearlinkDevice cachedDevice, int bondState) {
        if (bondState == NearlinkConstant.SLE_PAIR_PAIRED) {
            // If one device is connected(bonded), then close this fragment.
            finish();
            return;
        }
        if (mSelectedDevice != null && cachedDevice != null) {
            NearlinkDevice device = cachedDevice.getDevice();
            if (device != null && mSelectedDevice.equals(device) && bondState == NearlinkConstant.SLE_PAIR_NONE) {
                // If currently selected device failed to bond, restart scanning
                enableScanning();
            }
        }
    }

    @Override
    public int getHelpResource() {
        return R.string.help_url_nearlink;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }

    @Override
    protected int getPreferenceScreenResId() {
        return R.xml.nearlink_pairing_detail;
    }

    @Override
    public String getDeviceListKey() {
        return KEY_AVAIL_DEVICES;
    }

}
