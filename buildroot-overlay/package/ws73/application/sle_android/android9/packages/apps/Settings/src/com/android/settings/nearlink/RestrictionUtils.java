/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.os.UserHandle;

import com.android.settingslib.RestrictedLockUtils;
import com.android.settingslib.RestrictedLockUtils.EnforcedAdmin;

/**
 * A utility class to aid testing.
 *
 * @since 2023-12-04
 */
public class RestrictionUtils {

    public RestrictionUtils() {
    }

    /**
     * Utility method to check if user restriction is enforced on the current user.
     * <p>
     * It helps with testing - override it to avoid calling static method which calls system
     * API.
     */
    public EnforcedAdmin checkIfRestrictionEnforced(Context context, String restriction) {
        return RestrictedLockUtils.checkIfRestrictionEnforced(context, restriction, UserHandle.myUserId());
    }

}
