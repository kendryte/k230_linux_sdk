/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.support.v7.preference.Preference;
import android.util.Log;
import android.view.View;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.logging.nano.MetricsProto;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.core.SubSettingLauncher;
import com.android.settings.location.ScanningSettings;
import com.android.settings.overlay.FeatureFactory;
import com.android.settings.utils.AnnotationSpan;
import com.android.settings.widget.SwitchWidgetController;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnStart;
import com.android.settingslib.core.lifecycle.events.OnStop;
import com.android.settingslib.nearlink.LocalNearlinkAdapter;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.widget.FooterPreference;

/**
 * PreferenceController to update of nearlink state. All behavior except managing the footer text
 * is delegated to the SwitchWidgetController it uses.
 *
 * @since 2023-12-04
 */
public class NearlinkSwitchPreferenceController
        implements LifecycleObserver, OnStart, OnStop, SwitchWidgetController.OnSwitchChangeListener, View.OnClickListener {
    private static final String TAG = "NearlinkSwitchPreferenceController";

    private static final boolean DBG = true;

    @VisibleForTesting
    LocalNearlinkAdapter mNearlinkAdapter;

    private LocalNearlinkManager mNearlinkManager;

    private NearlinkEnabler mNearlinkEnabler;

    private RestrictionUtils mRestrictionUtils;

    private SwitchWidgetController mSwitch;

    private Context mContext;

    private FooterPreference mFooterPreference;

    private Preference mAddPreference;

    public NearlinkSwitchPreferenceController(Context context, SwitchWidgetController switchController,
                                              FooterPreference footerPreference, Preference add) {
        this(context, Utils.getLocalNlManager(context), new RestrictionUtils(), switchController, footerPreference,
                add);
    }

    @VisibleForTesting
    public NearlinkSwitchPreferenceController(Context context, LocalNearlinkManager nearlinkManager,
                                              RestrictionUtils restrictionUtils, SwitchWidgetController switchController, FooterPreference footerPreference,
                                              Preference add) {
        mNearlinkManager = nearlinkManager;
        mRestrictionUtils = restrictionUtils;
        mSwitch = switchController;
        mContext = context;

        mFooterPreference = footerPreference;
        mAddPreference = add;

        mSwitch.setupView();
        updateText(mSwitch.isChecked());

        if (mNearlinkManager != null) {
            mNearlinkAdapter = mNearlinkManager.getNearlinkAdapter();
        }
        mNearlinkEnabler = new NearlinkEnabler(context, switchController,
                FeatureFactory.getFactory(context).getMetricsFeatureProvider(), mNearlinkManager,
                MetricsEvent.ACTION_SETTINGS_MASTER_SWITCH_NEARLINK_TOGGLE, mRestrictionUtils);
        mNearlinkEnabler.setToggleCallback(this);
    }

    @Override
    public void onStart() {
        mNearlinkEnabler.resume(mContext);
        if (mSwitch != null) {
            updateText(mSwitch.isChecked());
        }
    }

    @Override
    public void onStop() {
        mNearlinkEnabler.pause();
    }

    @Override
    public boolean onSwitchToggled(boolean isChecked) {
        Log.e(TAG, "[onSwitchToggled] isChecked:" + isChecked);
        updateText(isChecked);

        if (mAddPreference != null) {
            mAddPreference.setVisible(isChecked);
        }

        return true;
    }

    @Override
    public void onClick(View v) {
        // send users to scanning settings if they click on the link in the summary text
        new SubSettingLauncher(mContext).setDestination(ScanningSettings.class.getName())
                .setSourceMetricsCategory(MetricsProto.MetricsEvent.NEARLINK_FRAGMENT)
                .launch();
    }

    @VisibleForTesting
    void updateText(boolean isChecked) {
        if (!isChecked && Utils.isNearlinkScanningEnabled(mContext)) {
            AnnotationSpan.LinkInfo info =
                    new AnnotationSpan.LinkInfo(AnnotationSpan.LinkInfo.DEFAULT_ANNOTATION, this);
            CharSequence text =
                    AnnotationSpan.linkify(mContext.getText(R.string.nearlink_scanning_on_info_message), info);
            mFooterPreference.setTitle(text);
        } else {
            mFooterPreference.setTitle(R.string.nearlink_empty_list_nearlink_off);
        }
    }
}