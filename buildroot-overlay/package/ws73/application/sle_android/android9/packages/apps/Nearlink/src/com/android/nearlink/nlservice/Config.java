/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.nearlink.NearlinkProfile;
import android.provider.Settings;
import android.util.Log;

import com.android.nearlink.R;
import com.android.nearlink.hid.HidHostService;

import java.util.ArrayList;

public class Config {
    private static final String TAG = "AdapterServiceConfig";

    private static class ProfileConfig {
        Class mClass;
        int mSupported;
        long mMask;

        ProfileConfig(Class theClass, int supportedFlag, long mask) {
            mClass = theClass;
            mSupported = supportedFlag;
            mMask = mask;
        }
    }

    /**
     * List of profile services with the profile-supported resource flag and bit mask.
     */
    private static final ProfileConfig[] PROFILE_SERVICES_AND_FLAGS = {
            new ProfileConfig(HidHostService.class, R.bool.profile_supported_hid_host,
                    (1 << NearlinkProfile.HID_HOST)),
    };

    private static Class[] sSupportedProfiles = new Class[0];

    static void init(Context ctx) {
        if (ctx == null) {
            return;
        }
        Resources resources = ctx.getResources();
        if (resources == null) {
            return;
        }

        ArrayList<Class> profiles = new ArrayList<>(PROFILE_SERVICES_AND_FLAGS.length);
        for (ProfileConfig config : PROFILE_SERVICES_AND_FLAGS) {
            boolean supported = resources.getBoolean(config.mSupported);
            if (supported && !isProfileDisabled(ctx, config.mMask)) {
                Log.v(TAG, "Adding " + config.mClass.getSimpleName());
                profiles.add(config.mClass);
            }
            sSupportedProfiles = profiles.toArray(new Class[profiles.size()]);
        }
    }

    public static Class[] getSupportedProfiles() {
        return sSupportedProfiles;
    }

    private static long getProfileMask(Class profile) {
        for (ProfileConfig config : PROFILE_SERVICES_AND_FLAGS) {
            if (config.mClass == profile) {
                return config.mMask;
            }
        }
        Log.w(TAG, "Could not find profile bit mask for " + profile.getSimpleName());
        return 0;
    }

    static long getSupportedProfilesBitMask() {
        long mask = 0;
        for (final Class profileClass : getSupportedProfiles()) {
            mask |= getProfileMask(profileClass);
        }
        return mask;
    }

    private static boolean isProfileDisabled(Context context, long profileMask) {
        final ContentResolver resolver = context.getContentResolver();
        final long disabledProfilesBitMask =
                Settings.Global.getLong(resolver, Settings.Global.NEARLINK_DISABLED_PROFILES, 0);
        Log.e(TAG, "isProfileDisabled: profileMask = " + profileMask + " , disabledProfilesBitMask = " + disabledProfilesBitMask);
        return (disabledProfilesBitMask & profileMask) != 0;
    }
}
