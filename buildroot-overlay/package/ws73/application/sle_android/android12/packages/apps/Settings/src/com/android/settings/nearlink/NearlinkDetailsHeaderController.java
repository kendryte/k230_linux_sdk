/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.graphics.drawable.Drawable;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.text.TextUtils;
import android.util.Log;
import android.util.Pair;

import com.android.settings.R;
import com.android.settings.widget.EntityHeaderController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.CachedNearlinkDeviceManager;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.widget.LayoutPreference;

/**
 * This class adds a header with device name and status (connected/disconnected, etc.).
 *
 * @since 2023-12-04
 */
public class NearlinkDetailsHeaderController extends NearlinkDetailsController {
    private static final String KEY_DEVICE_HEADER = "nearlink_device_header";

    private EntityHeaderController mHeaderController;

    private LocalNearlinkManager mLocalManager;

    private CachedNearlinkDeviceManager mDeviceManager;

    public NearlinkDetailsHeaderController(Context context, PreferenceFragmentCompat fragment, CachedNearlinkDevice device,
                                           Lifecycle lifecycle, LocalNearlinkManager nearlinkManager) {
        super(context, fragment, device, lifecycle);
        mLocalManager = nearlinkManager;
        mDeviceManager = mLocalManager.getCachedDeviceManager();
    }

    @Override
    protected void init(PreferenceScreen screen) {
        final LayoutPreference headerPreference = (LayoutPreference) screen.findPreference(KEY_DEVICE_HEADER);
        mHeaderController = EntityHeaderController.newInstance(mFragment.getActivity(), mFragment,
                headerPreference.findViewById(R.id.entity_header));
        screen.addPreference(headerPreference);
    }

    protected void setHeaderProperties() {
        final Pair<Drawable, String> pair = com.android.settingslib.nearlink.Utils.getNlClassDrawableWithDescription(mContext, mCachedDevice);
        String summaryText = mCachedDevice.getConnectionSummary();
        mHeaderController.setLabel(mCachedDevice.getName());
        mHeaderController.setIcon(pair.first);
        mHeaderController.setIconContentDescription(pair.second);
        mHeaderController.setSummary(summaryText);
        Log.e(TAG, "[setHeaderProperties] summaryText:" + summaryText);
    }

    @Override
    protected void refresh() {
        Log.e(TAG, "[refresh]");
        setHeaderProperties();
        mHeaderController.done(mFragment.getActivity(), true /* rebindActions */);
    }

    @Override
    public String getPreferenceKey() {
        return KEY_DEVICE_HEADER;
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (preference != null) {
            String key = preference.getKey();
            Log.e(TAG, "[handlePreferenceTreeClick] getKey:" + key);
            if (TextUtils.equals(getPreferenceKey(), key) && mFragment != null) {
                RemoteDeviceNameDialogFragment.newInstance(mCachedDevice)
                        .show(mFragment.getFragmentManager(), RemoteDeviceNameDialogFragment.TAG);
                return true;
            }
        }

        return super.handlePreferenceTreeClick(preference);
    }
}
