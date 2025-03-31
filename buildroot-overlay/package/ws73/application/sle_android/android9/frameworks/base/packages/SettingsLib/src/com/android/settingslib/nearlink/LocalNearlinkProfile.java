/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import android.nearlink.NearlinkDevice;

/**
 * LocalNearlinkProfile is an interface defining the basic
 * functionality related to a Nearlink profile.
 *
 * @since 2023-12-04
 */
public interface LocalNearlinkProfile {
    /**
     * Returns true if the user can initiate a connection, false otherwise.
     */
    boolean isConnectable();

    /**
     * Returns true if the user can enable auto connection for this profile.
     */
    boolean isAutoConnectable();

    boolean connect(NearlinkDevice device);

    boolean disconnect(NearlinkDevice device);

    int getConnectionStatus(NearlinkDevice device);

    boolean isPreferred(NearlinkDevice device);

    int getPreferred(NearlinkDevice device);

    void setPreferred(NearlinkDevice device, boolean preferred);

    boolean isProfileReady();

    int getProfileId();

    /**
     * Display order for device profile settings.
     */
    int getOrdinal();

    /**
     * Returns the string resource ID for the localized name for this profile.
     *
     * @param device the Nearlink device (to distinguish between PAN roles)
     */
    int getNameResource(NearlinkDevice device);

    /**
     * Returns the string resource ID for the summary text for this profile
     * for the specified device, e.g. "Use for media audio" or
     * "Connected to media audio".
     *
     * @param device the device to query for profile connection status
     * @return a string resource ID for the profile summary text
     */
    int getSummaryResourceForDevice(NearlinkDevice device);

    int getDrawableResource(int nlClass);
}
