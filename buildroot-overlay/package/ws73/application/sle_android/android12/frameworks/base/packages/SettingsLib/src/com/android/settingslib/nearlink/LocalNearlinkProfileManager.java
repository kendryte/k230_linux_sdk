/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkHidHost;
import android.nearlink.NearlinkProfile;
import android.os.ParcelUuid;
import android.util.Log;


/**
 * LocalNearlinkProfileManager provides access to the LocalNearlinkProfile
 * objects for the available Nearlink profiles.
 *
 * @since 2023-12-04
 */
public class LocalNearlinkProfileManager {
    private static final String TAG = "LocalNlProfileManager";
    private static final boolean DEBUG = Utils.D;
    /**
     * Singleton instance.
     */
    private static LocalNearlinkProfileManager sInstance;

    /**
     * An interface for notifying NearlinkHeadset IPC clients when they have
     * been connected to the NearlinkHeadset service.
     * Only used by com.android.settings.nearlink.DockService.
     */
    public interface ServiceListener {
        /**
         * Called to notify the client when this proxy object has been
         * connected to the NearlinkHeadset service. Clients must wait for
         * this callback before making IPC calls on the NearlinkHeadset
         * service.
         */
        void onServiceConnected();

        /**
         * Called to notify the client that this proxy object has been
         * disconnected from the NearlinkHeadset service. Clients must not
         * make IPC calls on the NearlinkHeadset service after this callback.
         * This callback will currently only occur if the application hosting
         * the NearlinkHeadset service, but may be called more often in future.
         */
        void onServiceDisconnected();
    }

    private final Context mContext;
    private final LocalNearlinkAdapter mLocalAdapter;
    private final CachedNearlinkDeviceManager mDeviceManager;
    private final NearlinkEventManager mEventManager;

    private final HidProfile mHidProfile;

    /**
     * Mapping from profile name, e.g. "HEADSET" to profile object.
     */
    private final Map<String, LocalNearlinkProfile>
            mProfileNameMap = new HashMap<String, LocalNearlinkProfile>();

    LocalNearlinkProfileManager(Context context,
                                LocalNearlinkAdapter adapter,
                                CachedNearlinkDeviceManager deviceManager,
                                NearlinkEventManager eventManager) {
        mContext = context;

        mLocalAdapter = adapter;
        mDeviceManager = deviceManager;
        mEventManager = eventManager;
        // pass this reference to adapter and event manager (circular dependency)
        mLocalAdapter.setProfileManager(this);
        mEventManager.setProfileManager(this);

        ParcelUuid[] uuids = adapter.getUuids();

        // uuids may be null if Nearlink is turned off
        if (uuids != null) {
            updateLocalProfiles(uuids);
        }

        // Always add HID host, HID device, and PAN profiles
        mHidProfile = new HidProfile(context, mLocalAdapter, mDeviceManager, this);
        addProfile(mHidProfile, com.android.settingslib.nearlink.HidProfile.NAME,
                NearlinkHidHost.ACTION_CONNECTION_STATE_CHANGED);
        if (DEBUG) Log.d(TAG, "LocalNearlinkProfileManager construction complete");
    }

    /**
     * Initialize or update the local profile objects. If a UUID was previously
     * present but has been removed, we print a warning but don't remove the
     * profile object as it might be referenced elsewhere, or the UUID might
     * come back and we don't want multiple copies of the profile objects.
     *
     * @param uuids
     */
    void updateLocalProfiles(ParcelUuid[] uuids) {
        mEventManager.registerProfileIntentReceiver();
    }

    private void addHeadsetProfile(LocalNearlinkProfile profile, String profileName,
                                   String stateChangedAction, String audioStateChangedAction, int audioDisconnectedState) {
        NearlinkEventManager.Handler handler = new HeadsetStateChangeHandler(
                profile, audioStateChangedAction, audioDisconnectedState);
        mEventManager.addProfileHandler(stateChangedAction, handler);
        mEventManager.addProfileHandler(audioStateChangedAction, handler);
        mProfileNameMap.put(profileName, profile);
    }

    private final Collection<ServiceListener> mServiceListeners =
            new ArrayList<ServiceListener>();

    private void addProfile(LocalNearlinkProfile profile,
                            String profileName, String stateChangedAction) {
        mEventManager.addProfileHandler(stateChangedAction, new StateChangedHandler(profile));
        mProfileNameMap.put(profileName, profile);
    }

    public LocalNearlinkProfile getProfileByName(String name) {
        return mProfileNameMap.get(name);
    }

    // Called from LocalNearlinkAdapter when state changes to ON
    void setNearlinkStateOn() {
        ParcelUuid[] uuids = mLocalAdapter.getUuids();
        if (uuids != null) {
            updateLocalProfiles(uuids);
        }
        mEventManager.readPairedDevices();
    }

    /**
     * Generic handler for connection state change events for the specified profile.
     */
    private class StateChangedHandler implements NearlinkEventManager.Handler {
        final LocalNearlinkProfile mProfile;

        StateChangedHandler(LocalNearlinkProfile profile) {
            mProfile = profile;
        }

        public void onReceive(Context context, Intent intent, NearlinkDevice device) {
            CachedNearlinkDevice cachedDevice = mDeviceManager.findDevice(device);
            if (cachedDevice == null) {
                Log.w(TAG, "StateChangedHandler found new device: " + device);
                cachedDevice = mDeviceManager.addDevice(mLocalAdapter,
                        LocalNearlinkProfileManager.this, device);
            }
            onReceiveInternal(intent, cachedDevice);
        }

        protected void onReceiveInternal(Intent intent, CachedNearlinkDevice cachedDevice) {
            int newState = intent.getIntExtra(NearlinkProfile.EXTRA_STATE, 0);
            int oldState = intent.getIntExtra(NearlinkProfile.EXTRA_PREVIOUS_STATE, 0);
            if (newState == NearlinkProfile.STATE_DISCONNECTED &&
                    oldState == NearlinkProfile.STATE_CONNECTING) {
                Log.i(TAG, "Failed to connect " + mProfile + " device");
            }

            mEventManager.dispatchProfileConnectionStateChanged(cachedDevice, newState,
                    mProfile.getProfileId());
            cachedDevice.onProfileStateChanged(mProfile, newState);
            cachedDevice.refresh();
        }
    }

    /**
     * Connectivity and audio state change handler for headset profiles.
     */
    private class HeadsetStateChangeHandler extends StateChangedHandler {
        private final String mAudioChangeAction;
        private final int mAudioDisconnectedState;

        HeadsetStateChangeHandler(LocalNearlinkProfile profile, String audioChangeAction,
                                  int audioDisconnectedState) {
            super(profile);
            mAudioChangeAction = audioChangeAction;
            mAudioDisconnectedState = audioDisconnectedState;
        }

        @Override
        public void onReceiveInternal(Intent intent, CachedNearlinkDevice cachedDevice) {
            if (mAudioChangeAction.equals(intent.getAction())) {
                int newState = intent.getIntExtra(NearlinkProfile.EXTRA_STATE, 0);
                if (newState != mAudioDisconnectedState) {
                    cachedDevice.onProfileStateChanged(mProfile, NearlinkProfile.STATE_CONNECTED);
                }
                cachedDevice.refresh();
            } else {
                super.onReceiveInternal(intent, cachedDevice);
            }
        }
    }

    // called from DockService
    public void addServiceListener(ServiceListener l) {
        mServiceListeners.add(l);
    }

    // called from DockService
    public void removeServiceListener(ServiceListener l) {
        mServiceListeners.remove(l);
    }

    // not synchronized: use only from UI thread! (TODO: verify)
    void callServiceConnectedListeners() {
        for (ServiceListener l : mServiceListeners) {
            l.onServiceConnected();
        }
    }

    // not synchronized: use only from UI thread! (TODO: verify)
    void callServiceDisconnectedListeners() {
        for (ServiceListener listener : mServiceListeners) {
            listener.onServiceDisconnected();
        }
    }

    /**
     * Fill in a list of LocalNearlinkProfile objects that are supported by
     * the local device and the remote device.
     *
     * @param uuids           of the remote device
     * @param localUuids      UUIDs of the local device
     * @param profiles        The list of profiles to fill
     * @param removedProfiles list of profiles that were removed
     */
    synchronized void updateProfiles(ParcelUuid[] uuids, ParcelUuid[] localUuids,
                                     Collection<LocalNearlinkProfile> profiles,
                                     Collection<LocalNearlinkProfile> removedProfiles,
                                     boolean isPanNapConnected, NearlinkDevice device) {
        // Copy previous profile list into removedProfiles
        removedProfiles.clear();
        removedProfiles.addAll(profiles);
        if (DEBUG) {
            Log.d(TAG, "Current Profiles" + profiles.toString());
        }
        profiles.clear();

        if (DEBUG) {
            Log.d(TAG, "New Profiles" + profiles.toString());
        }
    }

    /**
     * Fill in a list of LocalNearlinkProfile objects that are supported by
     * the local device and the remote device.
     *
     * @param profiles        The list of profiles to fill
     * @param removedProfiles list of profiles that were removed
     */
    synchronized void updateProfiles(Collection<LocalNearlinkProfile> profiles, Collection<LocalNearlinkProfile> removedProfiles, NearlinkDevice device) {
        // Copy previous profile list into removedProfiles
        removedProfiles.clear();
        removedProfiles.addAll(profiles);
        if (DEBUG) {
            Log.d(TAG, "Current Profiles" + profiles.toString());
        }
        profiles.clear();

        if (mHidProfile != null) {
            profiles.add(mHidProfile);
            removedProfiles.remove(mHidProfile);
        }
        if (DEBUG) {
            Log.d(TAG, "New Profiles" + profiles.toString());
        }
    }
}
