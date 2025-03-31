/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.support.v14.preference.PreferenceFragment;
import android.support.v7.preference.PreferenceScreen;

import com.android.settings.R;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.widget.FooterPreference;
import com.android.settingslib.widget.FooterPreferenceMixin;

/**
 * This class adds the device MAC address to a footer.
 *
 * @since 2023-12-04
 */
public class NearlinkDetailsMacAddressController extends NearlinkDetailsController {
    FooterPreferenceMixin mFooterPreferenceMixin;

    FooterPreference mFooterPreference;

    public NearlinkDetailsMacAddressController(Context context, PreferenceFragment fragment, CachedNearlinkDevice device, Lifecycle lifecycle) {
        super(context, fragment, device, lifecycle);
        mFooterPreferenceMixin = new FooterPreferenceMixin(fragment, lifecycle);
    }

    @Override
    protected void init(PreferenceScreen screen) {
        mFooterPreference = mFooterPreferenceMixin.createFooterPreference();
        mFooterPreference.setTitle(mContext.getString(R.string.nearlink_device_mac_address, mCachedDevice.getAddress()));
    }

    @Override
    protected void refresh() {
    }

    @Override
    public String getPreferenceKey() {
        if (mFooterPreference == null) {
            return null;
        }
        return mFooterPreference.getKey();
    }
}
