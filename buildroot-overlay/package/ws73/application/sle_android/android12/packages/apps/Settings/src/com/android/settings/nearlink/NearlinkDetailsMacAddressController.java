/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import com.android.settings.R;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.widget.FooterPreference;

/**
 * This class adds the device MAC address to a footer.
 *
 * @since 2023-12-04
 */
public class NearlinkDetailsMacAddressController extends NearlinkDetailsController {
    public static final String KEY_DEVICE_DETAILS_FOOTER = "device_details_footer";

    FooterPreference mFooterPreference;

    public NearlinkDetailsMacAddressController(Context context, PreferenceFragmentCompat fragment, CachedNearlinkDevice device, Lifecycle lifecycle) {
        super(context, fragment, device, lifecycle);
    }

    @Override
    protected void init(PreferenceScreen screen) {        
        mFooterPreference = screen.findPreference(KEY_DEVICE_DETAILS_FOOTER);
        mFooterPreference.setTitle(mContext.getString(R.string.nearlink_device_mac_address, mCachedDevice.getAddress()));
    }

    @Override
    protected void refresh() {
        mFooterPreference.setTitle(mContext.getString(
                R.string.nearlink_device_mac_address, mCachedDevice.getAddress()));
    }

    @Override
    public String getPreferenceKey() {
        return KEY_DEVICE_DETAILS_FOOTER;
    }
}
