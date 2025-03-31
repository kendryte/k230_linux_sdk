/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkManager;
import android.os.Bundle;
import androidx.preference.Preference;

import com.android.internal.logging.nano.MetricsProto;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.core.TogglePreferenceController;
import com.android.settings.dashboard.DashboardFragment;
import com.android.settings.search.BaseSearchIndexProvider;
import com.android.settingslib.search.SearchIndexableRaw;
import com.android.settings.widget.SettingsMainSwitchBar;
import com.android.settings.widget.MainSwitchBarController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.widget.FooterPreference;

/**
 * Dedicated screen for allowing the user to toggle nearlink which displays relevant information to
 * the user based on related settings such as nearlink scanning.
 *
 * @since 2023-12-04
 */
public class NearlinkDashboardFragment extends DashboardFragment {

    private static final String TAG = "NearlinkDashboardFragment";
    public static final String KEY_NEARLINK_SCREEN = "nearlink_switchbar_screen";
    public static final String KEY_NEARLINK_ADD = "nearlink_screen_add_sle_devices";
    private static final String KEY_NEARLINK_SCREEN_FOOTER = "nearlink_screen_footer";

    private FooterPreference mFooterPreference;

    private Preference mAddPreference;

    private SettingsMainSwitchBar mSwitchBar;

    private NearlinkSwitchPreferenceController mController;

    @Override
    public int getMetricsCategory() {
        return MetricsProto.MetricsEvent.NEARLINK_FRAGMENT;
    }

    @Override
    protected String getLogTag() {
        return TAG;
    }

    @Override
    public int getHelpResource() {
        return R.string.help_uri_nearlink_screen;
    }

    @Override
    protected int getPreferenceScreenResId() {
        return R.xml.nearlink_screen;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mFooterPreference = findPreference(KEY_NEARLINK_SCREEN_FOOTER);
        mAddPreference = findPreference(KEY_NEARLINK_ADD);
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        use(NearlinkDeviceRenamePreferenceController.class).setFragment(this);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        SettingsActivity activity = (SettingsActivity) getActivity();
        mSwitchBar = activity.getSwitchBar();
        mController = new NearlinkSwitchPreferenceController(activity, new MainSwitchBarController(mSwitchBar) {
            @Override
            public void setChecked(boolean checked) {
                super.setChecked(checked);
                if (mAddPreference != null) {
                    mAddPreference.setVisible(checked);
                }
            }
        }, mFooterPreference, mAddPreference);
        Lifecycle lifecycle = getSettingsLifecycle();
        if (lifecycle != null) {
            lifecycle.addObserver(mController);
        }
    }

    /**
     * For Search.
     */
    public static final SearchIndexProvider SEARCH_INDEX_DATA_PROVIDER = new BaseSearchIndexProvider() {
        @Override
        public List<SearchIndexableRaw> getRawDataToIndex(Context context, boolean enabled) {
            final List<SearchIndexableRaw> result = new ArrayList<>();

            // Add the activity title
            final SearchIndexableRaw data = new SearchIndexableRaw(context);
            data.title = context.getString(R.string.nearlink_settings_title);
            data.screenTitle = context.getString(R.string.nearlink_settings_title);
            data.keywords = context.getString(R.string.keywords_nearlink_settings);
            data.key = KEY_NEARLINK_SCREEN;
            result.add(data);

            return result;
        }

        @Override
        public List<String> getNonIndexableKeys(Context context) {
            final List<String> keys = super.getNonIndexableKeys(context);
            NearlinkManager manager = (NearlinkManager) context.getSystemService(Context.NEARLINK_SERVICE);
            if (manager != null) {
                NearlinkAdapter adapter = manager.getAdapter();
                final int status = adapter != null ? TogglePreferenceController.AVAILABLE
                        : TogglePreferenceController.UNSUPPORTED_ON_DEVICE;
                if (status != TogglePreferenceController.AVAILABLE) {
                    keys.add(KEY_NEARLINK_SCREEN);
                }
            }

            return keys;
        }
    };
}
