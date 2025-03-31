/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.content.SharedPreferences;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkProfile;
import android.os.ParcelUuid;
import android.os.SystemClock;
import androidx.annotation.VisibleForTesting;
import android.text.TextUtils;
import android.util.Log;

import com.android.settingslib.R;

/**
 * CachedNearlinkDevice represents a remote Nearlink device. It contains
 * attributes of the device (such as the address, name, RSSI, etc.) and
 * functionality that can be performed on the device (connect, pair, disconnect,
 * etc.).
 *
 * @since 2023-12-04
 */
public class CachedNearlinkDevice implements Comparable<CachedNearlinkDevice> {
    private static final String TAG = "CachedNearlinkDevice";
    private static final boolean DEBUG = com.android.settingslib.nearlink.Utils.V;

    private final Context mContext;
    private final LocalNearlinkAdapter mLocalAdapter;
    private final LocalNearlinkProfileManager mProfileManager;
    private final NearlinkDevice mDevice;
    private String mName;
    // Need this since there is no method for getting RSSI
    private int mRssi;
    private int mNlClass = NearlinkAppearance.UNKNOWN;
    private HashMap<LocalNearlinkProfile, Integer> mProfileConnectionState;

    private final List<LocalNearlinkProfile> mProfiles = new ArrayList<LocalNearlinkProfile>();

    // List of profiles that were previously in mProfiles, but have been removed
    private final List<LocalNearlinkProfile> mRemovedProfiles = new ArrayList<LocalNearlinkProfile>();

    // Device supports PANU but not NAP: remove PanProfile after device disconnects from NAP
    private boolean mLocalNapRoleConnected;

    private boolean mJustDiscovered;

    private int mMessageRejectionCount;

    private final Collection<Callback> mCallbacks = new ArrayList<Callback>();

    // Following constants indicate the user's choices of Phone book/message access settings
    // User hasn't made any choice or settings app has wiped out the memory
    public final static int ACCESS_UNKNOWN = 0;
    // User has accepted the connection and let Settings app remember the decision
    public final static int ACCESS_ALLOWED = 1;
    // User has rejected the connection and let Settings app remember the decision
    public final static int ACCESS_REJECTED = 2;

    // How many times user should reject the connection to make the choice persist.
    private final static int MESSAGE_REJECTION_COUNT_LIMIT_TO_PERSIST = 2;

    private final static String MESSAGE_REJECTION_COUNT_PREFS_NAME = "nearlink_message_reject";

    /**
     * When we connect to multiple profiles, we only want to display a single
     * error even if they all fail. This tracks that state.
     */
    private boolean mIsConnectingErrorPossible;

    /**
     * Last time a bt profile auto-connect was attempted.
     * If an ACTION_UUID intent comes in within
     * MAX_UUID_DELAY_FOR_AUTO_CONNECT milliseconds, we will try auto-connect
     * again with the new UUIDs
     */
    private long mConnectAttempted;

    // See mConnectAttempted
    private static final long MAX_UUID_DELAY_FOR_AUTO_CONNECT = 5000;
    private static final long MAX_HOGP_DELAY_FOR_AUTO_CONNECT = 30000;

    // Active device state
    private boolean mIsActiveDeviceA2dp = false;
    private boolean mIsActiveDeviceHeadset = false;
    private boolean mIsActiveDeviceHearingAid = false;

    /**
     * Describes the current device and profile for logging.
     *
     * @param profile Profile to describe
     * @return Description of the device and profile
     */
    private String describe(LocalNearlinkProfile profile) {
        StringBuilder sb = new StringBuilder();
        sb.append("Address:").append(mDevice);
        if (profile != null) {
            sb.append(" Profile:").append(profile);
        }

        return sb.toString();
    }

    void onProfileStateChanged(LocalNearlinkProfile profile, int newProfileState) {
        if (com.android.settingslib.nearlink.Utils.D) {
            Log.d(TAG, "onProfileStateChanged: profile " + profile +
                    " newProfileState " + newProfileState);
        }
        if (mLocalAdapter.getNearlinkState() == NearlinkAdapter.STATE_TURNING_OFF) {
            if (com.android.settingslib.nearlink.Utils.D)
                Log.d(TAG, " NL Turninig Off...Profile conn state change ignored...");
            return;
        }
        mProfileConnectionState.put(profile, newProfileState);
        if (newProfileState == NearlinkProfile.STATE_CONNECTED) {
            if (!mProfiles.contains(profile)) {
                mRemovedProfiles.remove(profile);
                mProfiles.add(profile);
            }
        }
        fetchActiveDevices();
    }

    CachedNearlinkDevice(Context context,
                         LocalNearlinkAdapter adapter,
                         LocalNearlinkProfileManager profileManager,
                         NearlinkDevice device) {
        mContext = context;
        mLocalAdapter = adapter;
        mProfileManager = profileManager;
        mDevice = device;
        mProfileConnectionState = new HashMap<LocalNearlinkProfile, Integer>();
        fillData();
    }

    public void disconnect() {
        Log.e(TAG, "[disconnect]");
        for (LocalNearlinkProfile profile : mProfiles) {
            disconnect(profile);
        }
    }

    public void disconnect(LocalNearlinkProfile profile) {
        if (profile.disconnect(mDevice)) {
            if (com.android.settingslib.nearlink.Utils.D) {
                Log.d(TAG, "Command sent successfully:DISCONNECT " + describe(profile));
            }
        }
    }

    public void connect(boolean connectAllProfiles) {
        if (mLocalAdapter.isDiscovering()) {
            mLocalAdapter.cancelDiscovery();
        }

        mConnectAttempted = SystemClock.elapsedRealtime();
        connectWithoutResettingTimer(connectAllProfiles);
    }

    void onBondingDockConnect() {
        // Attempt to connect if UUIDs are available. Otherwise,
        // we will connect when the ACTION_UUID intent arrives.
        connect(false);
    }

    private void connectWithoutResettingTimer(boolean connectAllProfiles) {
        Log.e(TAG, "[connectWithoutResettingTimer] connectAllProfiles:" + connectAllProfiles);
        // Try to initialize the profiles if they were not.
        if (mProfiles.isEmpty()) {
            // if mProfiles is empty, then do not invoke updateProfiles. This causes a race
            // condition with carkits during pairing, wherein RemoteDevice.UUIDs have been updated
            // from nearlink stack but ACTION.uuid is not sent yet.
            // Eventually ACTION.uuid will be received which shall trigger the connection of the
            // various profiles
            // If UUIDs are not available yet, connect will be happen
            // upon arrival of the ACTION_UUID intent.
            Log.d(TAG, "No profiles. Maybe we will connect later");
            return;
        }

        // Reset the only-show-one-error-dialog tracking variable
        mIsConnectingErrorPossible = true;

        int preferredProfiles = 0;
        for (LocalNearlinkProfile profile : mProfiles) {
            Log.e(TAG, "[connectWithoutResettingTimer] profile.isConnectable:" + profile.isConnectable());
            Log.e(TAG, "[connectWithoutResettingTimer] profile.isAutoConnectable:" + profile.isAutoConnectable());
            if (connectAllProfiles ? profile.isConnectable() : profile.isAutoConnectable()) {
                Log.e(TAG, "[connectWithoutResettingTimer] profile.isPreferred:" + profile.isPreferred(mDevice));
                if (profile.isPreferred(mDevice)) {
                    ++preferredProfiles;
                    connectInt(profile);
                }
            }
        }
        if (DEBUG) Log.e(TAG, "Preferred profiles = " + preferredProfiles);

        if (preferredProfiles == 0) {
            connectAutoConnectableProfiles();
        }
    }

    private void connectAutoConnectableProfiles() {
        // Reset the only-show-one-error-dialog tracking variable
        mIsConnectingErrorPossible = true;

        for (LocalNearlinkProfile profile : mProfiles) {
            if (profile.isAutoConnectable()) {
                profile.setPreferred(mDevice, true);
                connectInt(profile);
            }
        }
    }

    /**
     * Connect this device to the specified profile.
     *
     * @param profile the profile to use with the remote device
     */
    public void connectProfile(LocalNearlinkProfile profile) {
        mConnectAttempted = SystemClock.elapsedRealtime();
        // Reset the only-show-one-error-dialog tracking variable
        mIsConnectingErrorPossible = true;
        connectInt(profile);
        // Refresh the UI based on profile.connect() call
        refresh();
    }

    synchronized void connectInt(LocalNearlinkProfile profile) {
        Log.e(TAG, "[connectInt] profile:" + profile);
        if (profile.connect(mDevice)) {
            if (com.android.settingslib.nearlink.Utils.D) {
                Log.d(TAG, "Command sent successfully:CONNECT " + describe(profile));
            }
            return;
        }
        Log.e(TAG, "[connectInt] Failed to connect " + profile.toString() + " to " + mName);
    }

    /**
     * Return true if user initiated pairing on this device. The message text is
     * slightly different for local vs. remote initiated pairing dialogs.
     */
    boolean isUserInitiatedPairing() {
        return mDevice.isPairingInitiatedLocally();
    }

    public void unpair() {
        int state = getBondState();

        if (state == NearlinkConstant.SLE_PAIR_PAIRING) {
            mDevice.cancelPairProcess();
        }

        if (state != NearlinkConstant.SLE_PAIR_NONE) {
            final NearlinkDevice dev = mDevice;
            if (dev != null) {
                final boolean successful = dev.removePair();
                if (successful) {
                    if (Utils.D) {
                        Log.d(TAG, "Command sent successfully:REMOVE_PAIR " + describe(null));
                    }
                } else if (Utils.V) {
                    Log.v(TAG, "Framework rejected command immediately:REMOVE_PAIR " + describe(null));
                }
            }
        }
    }

    public int getProfileConnectionState(LocalNearlinkProfile profile) {
        if (mProfileConnectionState.get(profile) == null) {
            // If cache is empty make the binder call to get the state
            int state = profile.getConnectionStatus(mDevice);
            mProfileConnectionState.put(profile, state);
        }
        return mProfileConnectionState.get(profile);
    }

    public void clearProfileConnectionState() {
        if (com.android.settingslib.nearlink.Utils.D) {
            Log.d(TAG, " Clearing all connection state for dev:" + mDevice.getName());
        }
        for (LocalNearlinkProfile profile : getProfiles()) {
            mProfileConnectionState.put(profile, NearlinkProfile.STATE_DISCONNECTED);
        }
    }

    // TODO: do any of these need to run async on a background thread?
    private void fillData() {
        fetchName();
        fetchNlClass();
        updateProfiles();
        fetchActiveDevices();
        migratePhonebookPermissionChoice();
        migrateMessagePermissionChoice();
        fetchMessageRejectionCount();

        dispatchAttributesChanged();
    }

    public NearlinkDevice getDevice() {
        return mDevice;
    }

    /**
     * Convenience method that can be mocked - it lets tests avoid having to call getDevice() which
     * causes problems in tests since NearlinkDevice is final and cannot be mocked.
     *
     * @return the address of this device
     */
    public String getAddress() {
        return mDevice.getAddress();
    }

    public String getName() {
        return mName;
    }

    /**
     * Populate name from NearlinkDevice.ACTION_FOUND intent
     */
    void setNewName(String name) {
        Log.e(TAG, "[setNewName] name:" + name + ", mName:" + mName);
        if (name != null && !TextUtils.equals(name, mName)) {
            mName = name;
            if (TextUtils.isEmpty(mName)) {
                mName = mDevice.getAddress();
            }
            dispatchAttributesChanged();
        }
    }

    /**
     * User changes the device name
     *
     * @param name new alias name to be set, should never be null
     */
    public void setName(String name) {
        // Prevent mName to be set to null if setName(null) is called
        if (name != null && !TextUtils.equals(name, mName)) {
            mName = name;
            mDevice.setAlias(name);
            dispatchAttributesChanged();
        }
    }

    /**
     * Set this device as active device
     *
     * @return true if at least one profile on this device is set to active, false otherwise
     */
    public boolean setActive() {
        boolean result = false;
        return result;
    }

    void refreshName() {
        fetchName();
        dispatchAttributesChanged();
    }

    private void fetchName() {
        mName = mDevice.getAliasName();

        if (TextUtils.isEmpty(mName)) {
            mName = mDevice.getAddress();
            if (DEBUG) Log.d(TAG, "Device has no name (yet), use address: " + mName);
        }
    }

    /**
     * Checks if device has a human readable name besides MAC address
     *
     * @return true if device's alias name is not null nor empty, false otherwise
     */
    public boolean hasHumanReadableName() {
        return !TextUtils.isEmpty(mDevice.getAliasName());
    }

    /**
     * Get battery level from remote device
     *
     * @return battery level in percentage [0-100], or {@link NearlinkDevice#BATTERY_LEVEL_UNKNOWN}
     */
    public int getBatteryLevel() {
        return mDevice.getBatteryLevel();
    }

    void refresh() {
        dispatchAttributesChanged();
    }

    public void setJustDiscovered(boolean justDiscovered) {
        if (mJustDiscovered != justDiscovered) {
            mJustDiscovered = justDiscovered;
            dispatchAttributesChanged();
        }
    }

    public int getBondState() {
        Log.i(TAG, "[getBondState] device:" + mDevice);
        Log.i(TAG, "[getBondState] getPairState:" + mDevice.getPairState());
        return mDevice.getPairState();
    }

    public int getConnectState() {
        return mDevice.getConnectionState();
    }

    /**
     * Update the device status as active or non-active per Nearlink profile.
     *
     * @param isActive        true if the device is active
     * @param nearlinkProfile the Nearlink profile
     */
    public void onActiveDeviceChanged(boolean isActive, int nearlinkProfile) {
        boolean changed = false;
        switch (nearlinkProfile) {
            default:
                Log.w(TAG, "onActiveDeviceChanged: unknown profile " + nearlinkProfile +
                        " isActive " + isActive);
                break;
        }
        if (changed) {
            dispatchAttributesChanged();
        }
    }

    /**
     * Update the profile audio state.
     */
    void onAudioModeChanged() {
        dispatchAttributesChanged();
    }

    /**
     * Get the device status as active or non-active per Nearlink profile.
     *
     * @param nearlinkProfile the Nearlink profile
     * @return true if the device is active
     */
    @VisibleForTesting(otherwise = VisibleForTesting.PACKAGE_PRIVATE)
    public boolean isActiveDevice(int nearlinkProfile) {
        switch (nearlinkProfile) {
            default:
                Log.w(TAG, "getActiveDevice: unknown profile " + nearlinkProfile);
                break;
        }
        return false;
    }

    void setRssi(int rssi) {
        if (mRssi != rssi) {
            mRssi = rssi;
            dispatchAttributesChanged();
        }
    }

    /**
     * Checks whether we are connected to this device (any profile counts).
     *
     * @return Whether it is connected.
     */
    public boolean isConnected() {
        if (mDevice != null) {
            return mDevice.isConnected();
        }

        return false;
    }

    public boolean isConnectedProfile(LocalNearlinkProfile profile) {
        int status = getProfileConnectionState(profile);
        return status == NearlinkProfile.STATE_CONNECTED;

    }

    public boolean isBusy() {
        for (LocalNearlinkProfile profile : mProfiles) {
            int status = getProfileConnectionState(profile);
            Log.e(TAG, "[isBusy] status:" + status);
            if (status == NearlinkProfile.STATE_CONNECTING
                    || status == NearlinkProfile.STATE_DISCONNECTING) {
                return true;
            }
        }
        Log.e(TAG, "[isBusy] getConnectState:" + getConnectState());
        return getConnectState() == NearlinkConstant.SLE_ACB_STATE_CONNECTING;
    }

    /**
     * Fetches a new value for the cached NL class.
     */
    private void fetchNlClass() {
        mNlClass = mDevice.getNearlinkAppearance();
    }

    private boolean updateProfiles() {
        Log.e(TAG, "[updateProfiles] mDevice.getAddress:" + mDevice.getAddress());
        mProfileManager.updateProfiles(mProfiles, mRemovedProfiles, mDevice);

        if (DEBUG) {
            Log.e(TAG, "updating profiles for " + mDevice.getAliasName());
            int nlClass = mDevice.getNearlinkAppearance();
            Log.e(TAG, "nlClass: " + nlClass);
        }
        return true;
    }

    private void fetchActiveDevices() {
    }

    /**
     * Refreshes the UI for the NL class, including fetching the latest value
     * for the class.
     */
    void refreshBtClass() {
        fetchNlClass();
        dispatchAttributesChanged();
    }

    /**
     * Refreshes the UI when framework alerts us of a UUID change.
     */
    void onUuidChanged() {
        updateProfiles();
        ParcelUuid[] uuids = mDevice.getUuids();

        long timeout = MAX_UUID_DELAY_FOR_AUTO_CONNECT;
        if (DEBUG) {
            Log.d(TAG, "onUuidChanged: Time since last connect"
                    + (SystemClock.elapsedRealtime() - mConnectAttempted));
        }

        /*
         * If a connect was attempted earlier without any UUID, we will do the connect now.
         * Otherwise, allow the connect on UUID change.
         */
        if (!mProfiles.isEmpty()
                && ((mConnectAttempted + timeout) > SystemClock.elapsedRealtime())) {
            connectWithoutResettingTimer(false);
        }

        dispatchAttributesChanged();
    }

    void onBondingStateChanged(int bondState) {
        if (bondState == NearlinkConstant.SLE_PAIR_NONE) {
            mProfiles.clear();
            setPhonebookPermissionChoice(ACCESS_UNKNOWN);
            setMessagePermissionChoice(ACCESS_UNKNOWN);
            setSimPermissionChoice(ACCESS_UNKNOWN);
            mMessageRejectionCount = 0;
            saveMessageRejectionCount();
        }

        refresh();

        if (bondState == NearlinkConstant.SLE_PAIR_PAIRED) {
            if (mDevice.isNearlinkDock()) {
                onBondingDockConnect();
            } else if (mDevice.isPairingInitiatedLocally()) {
                connect(false);
            }
        }
    }

    void setNlClass(int nlClass) {
        if (mNlClass != nlClass) {
            mNlClass = nlClass;
            dispatchAttributesChanged();
        }
    }

    public int getNlClass() {
        return mNlClass;
    }

    public List<LocalNearlinkProfile> getProfiles() {
        return Collections.unmodifiableList(mProfiles);
    }

    public List<LocalNearlinkProfile> getConnectableProfiles() {
        List<LocalNearlinkProfile> connectableProfiles =
                new ArrayList<LocalNearlinkProfile>();
        for (LocalNearlinkProfile profile : mProfiles) {
            if (profile.isConnectable()) {
                connectableProfiles.add(profile);
            }
        }
        return connectableProfiles;
    }

    public List<LocalNearlinkProfile> getRemovedProfiles() {
        return mRemovedProfiles;
    }

    public void registerCallback(Callback callback) {
        synchronized (mCallbacks) {
            mCallbacks.add(callback);
        }
    }

    public void unregisterCallback(Callback callback) {
        synchronized (mCallbacks) {
            mCallbacks.remove(callback);
        }
    }

    private void dispatchAttributesChanged() {
        synchronized (mCallbacks) {
            for (Callback callback : mCallbacks) {
                callback.onDeviceAttributesChanged();
            }
        }
    }

    @Override
    public String toString() {
        return mDevice.toString();
    }

    @Override
    public boolean equals(Object o) {
        if ((o == null) || !(o instanceof CachedNearlinkDevice)) {
            return false;
        }
        return mDevice.equals(((CachedNearlinkDevice) o).mDevice);
    }

    @Override
    public int hashCode() {
        return mDevice.getAddress().hashCode();
    }

    // This comparison uses non-final fields so the sort order may change
    // when device attributes change (such as bonding state). Settings
    // will completely refresh the device list when this happens.
    public int compareTo(CachedNearlinkDevice another) {
        // Connected above not connected
        int comparison = (another.isConnected() ? 1 : 0) - (isConnected() ? 1 : 0);
        if (comparison != 0) return comparison;

        // Fallback on name
        return mName.compareTo(another.mName);
    }

    public interface Callback {
        void onDeviceAttributesChanged();
    }

    public int getPhonebookPermissionChoice() {
        int permission = mDevice.getPhonebookAccessPermission();
        if (permission == NearlinkDevice.ACCESS_ALLOWED) {
            return ACCESS_ALLOWED;
        } else if (permission == NearlinkDevice.ACCESS_REJECTED) {
            return ACCESS_REJECTED;
        }
        return ACCESS_UNKNOWN;
    }

    public void setPhonebookPermissionChoice(int permissionChoice) {
        int permission = NearlinkDevice.ACCESS_UNKNOWN;
        if (permissionChoice == ACCESS_ALLOWED) {
            permission = NearlinkDevice.ACCESS_ALLOWED;
        } else if (permissionChoice == ACCESS_REJECTED) {
            permission = NearlinkDevice.ACCESS_REJECTED;
        }
        mDevice.setPhonebookAccessPermission(permission);
    }

    // Migrates data from old data store (in Settings app's shared preferences) to new (in Nearlink
    // app's shared preferences).
    private void migratePhonebookPermissionChoice() {
        SharedPreferences preferences = mContext.getSharedPreferences(
                "nearlink_phonebook_permission", Context.MODE_PRIVATE);
        if (!preferences.contains(mDevice.getAddress())) {
            return;
        }

        if (mDevice.getPhonebookAccessPermission() == NearlinkDevice.ACCESS_UNKNOWN) {
            int oldPermission = preferences.getInt(mDevice.getAddress(), ACCESS_UNKNOWN);
            if (oldPermission == ACCESS_ALLOWED) {
                mDevice.setPhonebookAccessPermission(NearlinkDevice.ACCESS_ALLOWED);
            } else if (oldPermission == ACCESS_REJECTED) {
                mDevice.setPhonebookAccessPermission(NearlinkDevice.ACCESS_REJECTED);
            }
        }

        SharedPreferences.Editor editor = preferences.edit();
        editor.remove(mDevice.getAddress());
        editor.commit();
    }

    public int getMessagePermissionChoice() {
        int permission = mDevice.getMessageAccessPermission();
        if (permission == NearlinkDevice.ACCESS_ALLOWED) {
            return ACCESS_ALLOWED;
        } else if (permission == NearlinkDevice.ACCESS_REJECTED) {
            return ACCESS_REJECTED;
        }
        return ACCESS_UNKNOWN;
    }

    public void setMessagePermissionChoice(int permissionChoice) {
        int permission = NearlinkDevice.ACCESS_UNKNOWN;
        if (permissionChoice == ACCESS_ALLOWED) {
            permission = NearlinkDevice.ACCESS_ALLOWED;
        } else if (permissionChoice == ACCESS_REJECTED) {
            permission = NearlinkDevice.ACCESS_REJECTED;
        }
        mDevice.setMessageAccessPermission(permission);
    }

    public int getSimPermissionChoice() {
        int permission = mDevice.getSimAccessPermission();
        if (permission == NearlinkDevice.ACCESS_ALLOWED) {
            return ACCESS_ALLOWED;
        } else if (permission == NearlinkDevice.ACCESS_REJECTED) {
            return ACCESS_REJECTED;
        }
        return ACCESS_UNKNOWN;
    }

    void setSimPermissionChoice(int permissionChoice) {
        int permission = NearlinkDevice.ACCESS_UNKNOWN;
        if (permissionChoice == ACCESS_ALLOWED) {
            permission = NearlinkDevice.ACCESS_ALLOWED;
        } else if (permissionChoice == ACCESS_REJECTED) {
            permission = NearlinkDevice.ACCESS_REJECTED;
        }
        mDevice.setSimAccessPermission(permission);
    }

    // Migrates data from old data store (in Settings app's shared preferences) to new (in Nearlink
    // app's shared preferences).
    private void migrateMessagePermissionChoice() {
        SharedPreferences preferences = mContext.getSharedPreferences("nearlink_message_permission", Context.MODE_PRIVATE);
        if (!preferences.contains(mDevice.getAddress())) {
            return;
        }

        if (mDevice.getMessageAccessPermission() == NearlinkDevice.ACCESS_UNKNOWN) {
            int oldPermission = preferences.getInt(mDevice.getAddress(), ACCESS_UNKNOWN);
            if (oldPermission == ACCESS_ALLOWED) {
                mDevice.setMessageAccessPermission(NearlinkDevice.ACCESS_ALLOWED);
            } else if (oldPermission == ACCESS_REJECTED) {
                mDevice.setMessageAccessPermission(NearlinkDevice.ACCESS_REJECTED);
            }
        }

        SharedPreferences.Editor editor = preferences.edit();
        editor.remove(mDevice.getAddress());
        editor.commit();
    }

    /**
     * @return Whether this rejection should persist.
     */
    public boolean checkAndIncreaseMessageRejectionCount() {
        if (mMessageRejectionCount < MESSAGE_REJECTION_COUNT_LIMIT_TO_PERSIST) {
            mMessageRejectionCount++;
            saveMessageRejectionCount();
        }
        return mMessageRejectionCount >= MESSAGE_REJECTION_COUNT_LIMIT_TO_PERSIST;
    }

    private void fetchMessageRejectionCount() {
        SharedPreferences preference = mContext.getSharedPreferences(MESSAGE_REJECTION_COUNT_PREFS_NAME, Context.MODE_PRIVATE);
        mMessageRejectionCount = preference.getInt(mDevice.getAddress(), 0);
    }

    private void saveMessageRejectionCount() {
        SharedPreferences.Editor editor = mContext.getSharedPreferences(
                MESSAGE_REJECTION_COUNT_PREFS_NAME, Context.MODE_PRIVATE).edit();
        if (mMessageRejectionCount == 0) {
            editor.remove(mDevice.getAddress());
        } else {
            editor.putInt(mDevice.getAddress(), mMessageRejectionCount);
        }
        editor.commit();
    }

    private void processPhonebookAccess() {
        if (mDevice.getPairState() != NearlinkConstant.SLE_PAIR_PAIRED) return;
    }

    public int getMaxConnectionState() {
        int maxState = NearlinkProfile.STATE_DISCONNECTED;
        for (LocalNearlinkProfile profile : getProfiles()) {
            int connectionStatus = getProfileConnectionState(profile);
            if (connectionStatus > maxState) {
                maxState = connectionStatus;
            }
        }
        return maxState;
    }

    /**
     * @return resource for string that discribes the connection state of this device.
     * case 1: idle or playing media, show "Active" on the only one A2DP active device.
     * case 2: in phone call, show "Active" on the only one HFP active device
     */
    public String getConnectionSummary() {
        StringBuilder summary = new StringBuilder();
        if (mDevice != null) {
            boolean connected = mDevice.isConnected();
            Log.i(TAG, "[getConnectionSummary] device:" + mDevice);
            Log.i(TAG, "[getConnectionSummary] connected:" + connected);

            int bondState = getBondState();
            Log.i(TAG, "[getConnectionSummary] bondState:" + bondState);
            if (bondState == NearlinkConstant.SLE_PAIR_PAIRED) {
                summary.append(mContext.getString(R.string.nearlink_paired)).append(", ");
            } else {
                summary.append(mContext.getString(R.string.nearlink_unpaired)).append(", ");
            }
            if (!connected) {
                int connState = mDevice.getConnectionState();
                Log.i(TAG, "[getConnectionSummary] connState:" + connState);
                if (connState == NearlinkConstant.SLE_ACB_STATE_CONNECTING) {
                    summary.append(mContext.getString(R.string.nearlink_connecting));
                } else if (connState == NearlinkConstant.SLE_ACB_STATE_CONNECTED) {
                    summary.append(mContext.getString(R.string.nearlink_state_connected));
                } else if (connState == NearlinkConstant.SLE_ACB_STATE_NONE || connState == NearlinkConstant.SLE_ACB_STATE_DISCONNECTED) {
                    summary.append(mContext.getString(R.string.nearlink_unconnected));
                }
            } else {
                summary.append(mContext.getString(R.string.nearlink_state_connected));
            }
        }
        return summary.toString();
    }
}
