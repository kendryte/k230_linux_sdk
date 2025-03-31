/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

/**
 * NearlinkCallback provides a callback interface for the settings
 * UI to receive events from {@link NearlinkEventManager}.
 *
 * @since 2023-12-04
 */
public interface NearlinkCallback {
    void onNearlinkStateChanged(int nearlinkState);

    void onScanningStateChanged(boolean started);

    void onDeviceAdded(CachedNearlinkDevice cachedDevice);

    void onDeviceDeleted(CachedNearlinkDevice cachedDevice);

    void onDeviceBondStateChanged(CachedNearlinkDevice cachedDevice, int bondState);

    void onConnectionStateChanged(CachedNearlinkDevice cachedDevice, int state);

    void onPairStateChanged(CachedNearlinkDevice cachedDevice, int state);

    void onActiveDeviceChanged(CachedNearlinkDevice activeDevice, int nearlinkProfile);

    void onAudioModeChanged();

    default void onProfileConnectionStateChanged(CachedNearlinkDevice cachedDevice, int state, int nearlinkProfile) {
    }
}
