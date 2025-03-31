/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.app.Fragment;
import android.content.Context;
import android.support.annotation.VisibleForTesting;
import android.support.v7.preference.Preference;
import android.text.TextUtils;

import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.overlay.FeatureFactory;
import com.android.settingslib.core.instrumentation.MetricsFeatureProvider;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;

/**
 * 功能描述
 *
 * @since 2023-12-04
 */
public class NearlinkDeviceRenamePreferenceController extends NearlinkDeviceNamePreferenceController {
    private Fragment mFragment;

    private MetricsFeatureProvider mMetricsFeatureProvider;

    /**
     * Constructor exclusively used for Slice.
     */
    public NearlinkDeviceRenamePreferenceController(Context context, String preferenceKey) {
        super(context, preferenceKey);
        mMetricsFeatureProvider = FeatureFactory.getFactory(context).getMetricsFeatureProvider();
    }

    @VisibleForTesting
    NearlinkDeviceRenamePreferenceController(Context context, LocalNearlinkAdapter localAdapter, String preferenceKey) {
        super(context, localAdapter, preferenceKey);
        mMetricsFeatureProvider = FeatureFactory.getFactory(context).getMetricsFeatureProvider();
    }

    /**
     * Set the {@link Fragment} that used to show {@link LocalDeviceNameDialogFragment}
     * in {@code handlePreferenceTreeClick}
     */
    @VisibleForTesting
    public void setFragment(Fragment fragment) {
        mFragment = fragment;
    }

    @Override
    protected void updatePreferenceState(final Preference preference) {
        preference.setSummary(getSummary());
        preference.setVisible(mLocalAdapter != null && mLocalAdapter.isEnabled());
    }

    @Override
    public CharSequence getSummary() {
        return getDeviceName();
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(getPreferenceKey(), preference.getKey()) && mFragment != null) {
            mMetricsFeatureProvider.action(mContext, MetricsProto.MetricsEvent.ACTION_NEARLINK_RENAME);
            LocalDeviceNameDialogFragment.newInstance()
                    .show(mFragment.getFragmentManager(), LocalDeviceNameDialogFragment.TAG);
            return true;
        }

        return false;
    }
}
