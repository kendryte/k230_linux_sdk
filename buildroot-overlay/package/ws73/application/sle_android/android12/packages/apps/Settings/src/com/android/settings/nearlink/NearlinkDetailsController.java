/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnPause;
import com.android.settingslib.core.lifecycle.events.OnResume;
import com.android.settingslib.nearlink.CachedNearlinkDevice;

/**
 * This class provides common lifecycle and nearlink device event registration for Nearlink device
 * details controllers.
 *
 * @since 2023-12-04
 */
public abstract class NearlinkDetailsController extends AbstractPreferenceController
        implements PreferenceControllerMixin, CachedNearlinkDevice.Callback, LifecycleObserver, OnPause, OnResume {
    protected final Context mContext;

    protected final PreferenceFragmentCompat mFragment;

    protected final CachedNearlinkDevice mCachedDevice;

    public NearlinkDetailsController(Context context, PreferenceFragmentCompat fragment, CachedNearlinkDevice device,
                                     Lifecycle lifecycle) {
        super(context);
        mContext = context;
        mFragment = fragment;
        mCachedDevice = device;
        lifecycle.addObserver(this);
    }

    @Override
    public void onPause() {
        mCachedDevice.unregisterCallback(this);
    }

    @Override
    public void onResume() {
        mCachedDevice.registerCallback(this);
        refresh();
    }

    @Override
    public boolean isAvailable() {
        return true;
    }

    @Override
    public void onDeviceAttributesChanged() {
        refresh();
    }

    @Override
    public final void displayPreference(PreferenceScreen screen) {
        init(screen);
        super.displayPreference(screen);
    }

    /**
     * This is a method to do one-time initialization when the screen is first created, such as
     * adding preferences.
     *
     * @param screen the screen where this controller's preferences should be added
     */
    protected abstract void init(PreferenceScreen screen);

    /**
     * This method is called when something about the nearlink device has changed, and this object
     * should update the preferences it manages based on the new state.
     */
    protected abstract void refresh();
}
