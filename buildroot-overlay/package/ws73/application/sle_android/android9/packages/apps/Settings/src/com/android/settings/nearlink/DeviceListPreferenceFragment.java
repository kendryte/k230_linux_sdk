/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;
import java.util.WeakHashMap;

import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.Bundle;
import android.support.annotation.VisibleForTesting;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceGroup;
import android.text.BidiFormatter;
import android.util.Log;

import com.android.settings.R;
import com.android.settings.core.SubSettingLauncher;
import com.android.settings.dashboard.RestrictedDashboardFragment;
import com.android.settings.widget.GearPreference;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.nearlink.NearlinkCallback;
import com.android.settingslib.nearlink.NearlinkDeviceFilter;

/**
 * Parent class for settings fragments that contain a list of Nearlink
 * devices.
 *
 * @since 2023-12-04
 */
public abstract class DeviceListPreferenceFragment extends RestrictedDashboardFragment implements NearlinkCallback {
    private static final String TAG = "DeviceListPreferenceFragment";

    private static final String KEY_NL_SCAN = "nl_scan";

    private static final String NEARLINK_SHOW_DEVICES_WITHOUT_NAMES_PROPERTY
        = "persist.nearlink.showdeviceswithoutnames";

    private NearlinkDeviceFilter.Filter mFilter;

    @VisibleForTesting
    boolean mScanEnabled;

    NearlinkDevice mSelectedDevice;

    LocalNearlinkAdapter mLocalAdapter;

    LocalNearlinkManager mLocalManager;

    @VisibleForTesting
    PreferenceGroup mDeviceListGroup;

    final WeakHashMap<CachedNearlinkDevice, NearlinkDevicePreference> mDevicePreferenceMap
        = new WeakHashMap<CachedNearlinkDevice, NearlinkDevicePreference>();

    boolean mShowDevicesWithoutNames;

    DeviceListPreferenceFragment(String restrictedKey) {
        super(restrictedKey);
        mFilter = NearlinkDeviceFilter.ALL_FILTER;
    }

    final void setFilter(NearlinkDeviceFilter.Filter filter) {
        mFilter = filter;
    }

    final void setFilter(int filterType) {
        mFilter = NearlinkDeviceFilter.getFilter(filterType);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mLocalManager = Utils.getLocalNlManager(getActivity());
        if (mLocalManager == null) {
            Log.e(TAG, "Nearlink is not supported on this device");
            return;
        }
        mLocalAdapter = mLocalManager.getNearlinkAdapter();
        // todo: 该配置项为是否允许界面显示没有Name的设备，在Setting的开发者选项中进行配置，当前暂未实现
        mShowDevicesWithoutNames = true;

        initPreferencesFromPreferenceScreen();

        mDeviceListGroup = (PreferenceCategory) findPreference(getDeviceListKey());
    }

    /**
     * find and update preference that already existed in preference screen
     */
    abstract void initPreferencesFromPreferenceScreen();

    @Override
    public void onStart() {
        super.onStart();
        if (mLocalManager == null || isUiRestricted()) {
            return;
        }

        mLocalManager.setForegroundActivity(getActivity());
        mLocalManager.getEventManager().registerCallback(this);
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mLocalManager == null || isUiRestricted()) {
            return;
        }

        removeAllDevices();
        mLocalManager.setForegroundActivity(null);
        mLocalManager.getEventManager().unregisterCallback(this);
    }

    void removeAllDevices() {
        Log.e(TAG, "[removeAllDevices]");
        if (mDeviceListGroup != null) {
            int count = mDeviceListGroup.getPreferenceCount();
            Log.e(TAG, "[removeAllDevices] count:" + count);
            if (count > 0) {
                for (int i = count - 1; i >= 0; i--) {
                    Preference preference = mDeviceListGroup.getPreference(i);
                    if (preference != null && preference instanceof NearlinkDevicePreference) {
                        CachedNearlinkDevice cachedDevice = ((NearlinkDevicePreference) preference).getCachedDevice();
                        if (cachedDevice != null) {
                            boolean connected = cachedDevice.isConnected();
                            int pairState = cachedDevice.getBondState();
                            if (pairState != NearlinkConstant.SLE_PAIR_PAIRED && !connected) {
                                Log.e(TAG, "[removeAllDevices] :" + cachedDevice.getAddress());
                                mDeviceListGroup.removePreference(preference);
                            }
                        }
                    } else {
                        mDeviceListGroup.removePreference(preference);
                    }
                }
            }
        }

        if (mDevicePreferenceMap != null) {
            Set<CachedNearlinkDevice> keySet = mDevicePreferenceMap.keySet();
            if (keySet != null && keySet.size() > 0) {
                Set<CachedNearlinkDevice> rmSet = new HashSet<>();
                for (CachedNearlinkDevice cachedDevice : keySet) {
                    boolean connected = cachedDevice.isConnected();
                    int pairState = cachedDevice.getBondState();
                    if (pairState != NearlinkConstant.SLE_PAIR_PAIRED && !connected) {
                        rmSet.add(cachedDevice);
                    }
                }
                if (rmSet.size() > 0) {
                    for (CachedNearlinkDevice device : rmSet) {
                        mDevicePreferenceMap.remove(device);
                    }
                }
            }
        }
    }

    void addCachedDevices() {
        Collection<CachedNearlinkDevice> cachedDevices = mLocalManager.getCachedDeviceManager().getCachedDevicesCopy();
        Log.e(TAG, "[addCachedDevices] cachedDevices size:" + cachedDevices.size());
        for (CachedNearlinkDevice cachedDevice : cachedDevices) {
            Log.e(TAG, "[addCachedDevices] cachedDevice name:" + cachedDevice.getName());
            Log.e(TAG, "[addCachedDevices] cachedDevice address:" + cachedDevice.getAddress());
            Log.e(TAG, "[addCachedDevices] cachedDevice getNlClass:" + cachedDevice.getNlClass());
            onDeviceAdded(cachedDevice);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        Log.e(TAG, "[onPreferenceTreeClick]");
        if (KEY_NL_SCAN.equals(preference.getKey())) {
            mLocalAdapter.startScanning(true);
            return true;
        }

        if (preference instanceof NearlinkDevicePreference) {
            NearlinkDevicePreference nlPreference = (NearlinkDevicePreference) preference;
            CachedNearlinkDevice device = nlPreference.getCachedDevice();
            mSelectedDevice = device.getDevice();
            onDevicePreferenceClick(nlPreference);
            return true;
        }

        return super.onPreferenceTreeClick(preference);
    }

    void onDevicePreferenceClick(NearlinkDevicePreference nlPreference) {
        nlPreference.onClicked();
    }

    @Override
    public void onDeviceAdded(CachedNearlinkDevice cachedDevice) {
        Log.e(TAG, "[onDeviceAdded] getAddress:" + cachedDevice.getAddress());
        Log.e(TAG, "[onDeviceAdded] cachedDevice getNlClass:" + cachedDevice.getNlClass());
        Log.e(TAG, "[onDeviceAdded] mDevicePreferenceMap.get(cachedDevice):" + mDevicePreferenceMap.get(cachedDevice));
        if (mDevicePreferenceMap.get(cachedDevice) != null) {
            return;
        }

        // Prevent updates while the list shows one of the state messages
        if (mLocalAdapter.getNearlinkState() != NearlinkAdapter.STATE_ON) {
            return;
        }

        if (mFilter.matches(cachedDevice.getDevice())) {
            createDevicePreference(cachedDevice);
        }
    }

    final GearPreference.OnGearClickListener mDeviceProfilesListener = pref -> {
        launchDeviceDetails(pref);
    };

    void launchDeviceDetails(Preference preference) {
        final CachedNearlinkDevice device = ((NearlinkDevicePreference) preference).getNearlinkDevice();
        if (device == null) {
            return;
        }
        final Bundle args = new Bundle();
        args.putString(NearlinkDeviceDetailsFragment.KEY_DEVICE_ADDRESS, device.getDevice().getAddress());

        new SubSettingLauncher(getContext()).setDestination(NearlinkDeviceDetailsFragment.class.getName())
            .setArguments(args)
            .setTitle(R.string.device_details_title)
            .setSourceMetricsCategory(getMetricsCategory())
            .launch();
    }

    void createDevicePreference(CachedNearlinkDevice cachedDevice) {
        if (mDeviceListGroup == null) {
            Log.w(TAG, "Trying to create a device preference before the list group/category " + "exists!");
            return;
        }

        String key = cachedDevice.getDevice().getAddress();
        NearlinkDevicePreference preference = (NearlinkDevicePreference) getCachedPreference(key);

        if (preference == null) {
            preference = new NearlinkDevicePreference(getPrefContext(), cachedDevice, mShowDevicesWithoutNames);
            preference.setKey(key);
            preference.setOnGearClickListener(mDeviceProfilesListener);
            mDeviceListGroup.addPreference(preference);
        } else {
            // Tell the preference it is being re-used in case there is new info in the
            // cached device.
            preference.rebind();
        }

        initDevicePreference(preference);
        mDevicePreferenceMap.put(cachedDevice, preference);
    }

    void initDevicePreference(NearlinkDevicePreference preference) {
        // Does nothing by default
    }

    @VisibleForTesting
    void updateFooterPreference(Preference myDevicePreference) {
        final BidiFormatter bidiFormatter = BidiFormatter.getInstance();

        myDevicePreference.setTitle(
            getString(R.string.nearlink_footer_mac_message, bidiFormatter.unicodeWrap(mLocalAdapter.getAddress())));
    }

    @Override
    public void onDeviceDeleted(CachedNearlinkDevice cachedDevice) {
        if (cachedDevice != null) {
            Log.e(TAG, "[onDeviceDeleted] cachedDevice.getAddress:" + cachedDevice.getAddress());
            NearlinkDevicePreference preference = mDevicePreferenceMap.remove(cachedDevice);
            if (preference != null) {
                mDeviceListGroup.removePreference(preference);
                Log.e(TAG, "[onDeviceDeleted] removePreference");
            }
        }
    }

    @VisibleForTesting
    void enableScanning() {
        // LocalNearlinkAdapter already handles repeated scan requests
        Log.e(TAG, "[enableScanning] => mLocalAdapter.startScanning(true)");
        mLocalAdapter.startScanning(true);
        mScanEnabled = true;
        Log.e(TAG, "[enableScanning] mScanEnabled = true:");
    }

    @VisibleForTesting
    void disableScanning() {
        Log.e(TAG, "[disableScanning] => mLocalAdapter.stopScanning()");
        mLocalAdapter.stopScanning();
        mScanEnabled = false;
        Log.e(TAG, "[disableScanning] mScanEnabled = false:");
    }

    @Override
    public void onScanningStateChanged(boolean started) {
        if (!started && mScanEnabled) {
            Log.e(TAG, "[onScanningStateChanged] => mLocalAdapter.startScanning(true)");
            mLocalAdapter.startScanning(true);
        }
    }

    @Override
    public void onNearlinkStateChanged(int nearlinkState) {
    }

    /**
     * Add nearlink device preferences to {@code preferenceGroup} which satisfy the {@code filter}
     * <p>
     * This method will also (1) set the title for {@code preferenceGroup} and (2) change the
     * default preferenceGroup and filter
     *
     * @param preferenceGroup
     * @param titleId
     * @param filter
     */
    public void addDeviceCategory(PreferenceGroup preferenceGroup, int titleId, NearlinkDeviceFilter.Filter filter) {
        cacheRemoveAllPrefs(preferenceGroup);
        preferenceGroup.setTitle(titleId);
        mDeviceListGroup = preferenceGroup;
        setFilter(filter);
        addCachedDevices();
        preferenceGroup.setEnabled(true);
        removeCachedPrefs(preferenceGroup);
    }

    public void onConnectionStateChanged(CachedNearlinkDevice cachedDevice, int state) {
        Log.e(TAG, "[onConnectionStateChanged] state:" + state);
        NearlinkDevicePreference nlPreference = mDevicePreferenceMap.get(cachedDevice);
        if (nlPreference != null) {
            nlPreference.onDeviceAttributesChanged();
        }
    }

    public void onPairStateChanged(CachedNearlinkDevice cachedDevice, int state) {
        Log.e(TAG, "[onPairStateChanged] state:" + state);
        NearlinkDevicePreference nlPreference = mDevicePreferenceMap.get(cachedDevice);
        if (nlPreference != null) {
            nlPreference.onDeviceAttributesChanged();
        }
    }

    @Override
    public void onActiveDeviceChanged(CachedNearlinkDevice activeDevice, int nearlinkProfile) {
    }

    @Override
    public void onAudioModeChanged() {
    }

    /**
     * Return the key of the {@link PreferenceGroup} that contains the nearlink devices
     */
    public abstract String getDeviceListKey();

    public boolean shouldShowDevicesWithoutNames() {
        return mShowDevicesWithoutNames;
    }
}
