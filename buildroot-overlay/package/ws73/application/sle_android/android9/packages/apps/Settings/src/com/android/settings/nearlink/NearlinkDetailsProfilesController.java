/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import java.util.List;

import android.content.Context;
import android.nearlink.NearlinkDevice;
import android.support.annotation.VisibleForTesting;
import android.support.v14.preference.PreferenceFragment;
import android.support.v14.preference.SwitchPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceScreen;

import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.nearlink.CachedNearlinkDevice;
import com.android.settingslib.nearlink.LocalNearlinkManager;
import com.android.settingslib.nearlink.LocalNearlinkProfile;
import com.android.settingslib.nearlink.LocalNearlinkProfileManager;

/**
 * 功能描述
 * This class adds switches for toggling the individual profiles that a Nearlink device
 * supports, such as "Phone audio", "Media audio", "Contact sharing", etc.
 *
 * @since 2023-12-04
 */
public class NearlinkDetailsProfilesController extends NearlinkDetailsController
        implements Preference.OnPreferenceClickListener {
    private static final String KEY_PROFILES_GROUP = "nearlink_profiles";

    @VisibleForTesting
    static final String HIGH_QUALITY_AUDIO_PREF_TAG = "A2dpProfileHighQualityAudio";

    private LocalNearlinkManager mManager;

    private LocalNearlinkProfileManager mProfileManager;

    private CachedNearlinkDevice mCachedDevice;

    private PreferenceCategory mProfilesContainer;

    public NearlinkDetailsProfilesController(Context context, PreferenceFragment fragment, LocalNearlinkManager manager,
                                             CachedNearlinkDevice device, Lifecycle lifecycle) {
        super(context, fragment, device, lifecycle);
        mManager = manager;
        mProfileManager = mManager.getProfileManager();
        mCachedDevice = device;
        lifecycle.addObserver(this);
    }

    @Override
    protected void init(PreferenceScreen screen) {
        mProfilesContainer = (PreferenceCategory) screen.findPreference(getPreferenceKey());
        // Call refresh here even though it will get called later in onResume, to avoid the
        // list of switches appearing to "pop" into the page.
        refresh();
    }

    /**
     * Creates a switch preference for the particular profile.
     *
     * @param context The context to use when creating the SwitchPreference
     * @param profile The profile for which the preference controls.
     * @return A preference that allows the user to choose whether this profile
     * will be connected to.
     */
    private SwitchPreference createProfilePreference(Context context, LocalNearlinkProfile profile) {
        SwitchPreference pref = new SwitchPreference(context);
        pref.setKey(profile.toString());
        pref.setTitle(profile.getNameResource(mCachedDevice.getDevice()));
        pref.setOnPreferenceClickListener(this);
        return pref;
    }

    /**
     * Refreshes the state for an existing SwitchPreference for a profile.
     */
    private void refreshProfilePreference(SwitchPreference profilePref, LocalNearlinkProfile profile) {
        NearlinkDevice device = mCachedDevice.getDevice();
        profilePref.setEnabled(!mCachedDevice.isBusy());
        profilePref.setChecked(profile.isPreferred(device));
    }

    /**
     * Helper method to enable a profile for a device.
     */
    private void enableProfile(LocalNearlinkProfile profile, NearlinkDevice device, SwitchPreference profilePref) {
        profile.setPreferred(device, true);
        mCachedDevice.connectProfile(profile);
    }

    /**
     * Helper method to disable a profile for a device
     */
    private void disableProfile(LocalNearlinkProfile profile, NearlinkDevice device, SwitchPreference profilePref) {
    }

    /**
     * When the pref for a nearlink profile is clicked on, we want to toggle the enabled/disabled
     * state for that profile.
     */
    @Override
    public boolean onPreferenceClick(Preference preference) {
        LocalNearlinkProfile profile = mProfileManager.getProfileByName(preference.getKey());
        SwitchPreference profilePref = (SwitchPreference) preference;
        NearlinkDevice device = mCachedDevice.getDevice();
        if (profilePref.isChecked()) {
            enableProfile(profile, device, profilePref);
        } else {
            disableProfile(profile, device, profilePref);
        }
        refreshProfilePreference(profilePref, profile);
        return true;
    }

    /**
     * Helper to get the list of connectable and special profiles.
     */
    private List<LocalNearlinkProfile> getProfiles() {
        List<LocalNearlinkProfile> result = mCachedDevice.getConnectableProfiles();
        return result;
    }

    /**
     * This is a helper method to be called after adding a Preference for a profile. If that
     * profile happened to be A2dp and the device supports high quality audio, it will add a
     * separate preference for controlling whether to actually use high quality audio.
     *
     * @param profile the profile just added
     */
    private void maybeAddHighQualityAudioPref(LocalNearlinkProfile profile) {
    }

    /**
     * Refreshes the state of the switches for all profiles, possibly adding or removing switches as
     * needed.
     */
    @Override
    protected void refresh() {
        for (LocalNearlinkProfile profile : getProfiles()) {
            SwitchPreference pref = (SwitchPreference) mProfilesContainer.findPreference(profile.toString());
            if (pref == null) {
                pref = createProfilePreference(mProfilesContainer.getContext(), profile);
                mProfilesContainer.addPreference(pref);
                maybeAddHighQualityAudioPref(profile);
            }
            refreshProfilePreference(pref, profile);
        }
        for (LocalNearlinkProfile removedProfile : mCachedDevice.getRemovedProfiles()) {
            SwitchPreference pref = (SwitchPreference) mProfilesContainer.findPreference(removedProfile.toString());
            if (pref != null) {
                mProfilesContainer.removePreference(pref);
            }
        }
    }

    @Override
    public String getPreferenceKey() {
        return KEY_PROFILES_GROUP;
    }
}
