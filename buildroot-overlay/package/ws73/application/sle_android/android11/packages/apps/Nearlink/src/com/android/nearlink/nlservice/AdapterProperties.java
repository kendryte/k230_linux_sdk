/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkHidHost;
import android.nearlink.NearlinkProfile;
import android.os.UserHandle;
import android.util.Log;
import android.util.Pair;
import com.android.nearlink.connection.ConnectionManager;

import java.util.HashMap;

/**
 * profile连接状态收集广播类
 *
 * @since 2023-12-11
 */
public class AdapterProperties {
    private static final String TAG = "[SLE_CONN_PACKAGE]--AdapterProperties";

    private volatile int mState = NearlinkAdapter.STATE_OFF;
    private volatile int mConnectionState = NearlinkAdapter.STATE_DISCONNECTED;

    // Lock for all getters and setters.
    // If finer grained locking is needer, more locks
    // can be added here.
    private final Object mObject = new Object();

    private AdapterService mService;
    private final NearlinkAdapter mAdapter;
    private int mProfilesConnecting;
    private int mProfilesConnected;
    private int mProfilesDisconnecting;
    private final HashMap<Integer, Pair<Integer, Integer>> mProfileConnectionState =
            new HashMap<>();
    private RemoteDevices mRemoteDevices;
    private boolean mReceiverRegistered;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Log.w(TAG, "Received intent with null action");
                return;
            }
            Log.d(TAG, "BroadcastReceiver: action:" + action);
            switch (action) {
                case NearlinkHidHost.ACTION_CONNECTION_STATE_CHANGED:
                    sendConnectionStateChange(NearlinkProfile.HID_HOST, intent);
                    break;
                default:
                    Log.w(TAG, "Received unknown intent " + intent);
                    break;
            }
        }
    };

    AdapterProperties(AdapterService service) {
        mService = service;
        mAdapter = NearlinkAdapter.getDefaultAdapter();
    }

    /**
     * 初始化
     *
     * @param remoteDevices remoteDevices
     */
    public void init(RemoteDevices remoteDevices) {
        mProfileConnectionState.clear();
        mRemoteDevices = remoteDevices;
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkHidHost.ACTION_CONNECTION_STATE_CHANGED);
        mService.registerReceiver(mReceiver, filter);
        mReceiverRegistered = true;
        Log.d(TAG, "init: registerReceiver");
    }


    private void sendConnectionStateChange(int profile, Intent connIntent) {
        NearlinkDevice device = connIntent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
        int prevState = connIntent.getIntExtra(NearlinkProfile.EXTRA_PREVIOUS_STATE, -1);
        int state = connIntent.getIntExtra(NearlinkProfile.EXTRA_STATE, -1);
        int discReason = connIntent.getIntExtra(NearlinkProfile.EXTRA_DISC_REASON, -1);
        int pairState = connIntent.getIntExtra(NearlinkProfile.EXTRA_PAIR_STATE, -1);
        if (!isNormalStateTransition(prevState, state)) {
            Log.w(TAG,
                    "PROFILE_CONNECTION_STATE_CHANGE: unexpected transition for profile=" + profile
                            + ", device=" + device + ", " + prevState + " -> " + state);
        }
        sendConnectionStateChange(device, profile, state, prevState, pairState, discReason);
    }

    void sendConnectionStateChange(NearlinkDevice device, int profile, int state, int prevState,
                                   int pairState, int discReason) {
        Log.d(TAG, "sendConnectionStateChange: device:" + device.getAddress()
                + ",profile:" + profile + ",state transition " + prevState + " -> "
                + state);
        if (!validateProfileConnectionState(state) || !validateProfileConnectionState(prevState)) {
            // Previously, an invalid state was broadcast anyway,
            // with the invalid state converted to -1 in the intent.
            // Better to log an error and not send an intent with
            // invalid contents or set mAdapterConnectionState to -1.
            Log.d(TAG, "sendConnectionStateChange: invalid state transition " + prevState + " -> "
                    + state);
            return;
        }
        synchronized (mObject) {
            updateProfileConnectionState(profile, state, prevState);

            int newAdapterState = convertToAdapterState(state);
            int prevAdapterState = convertToAdapterState(prevState);
            setConnectionState(newAdapterState);

            Intent intent = new Intent(NearlinkAdapter.ACTION_CONNECTION_STATE_CHANGED);
            intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
            intent.putExtra(NearlinkAdapter.EXTRA_CONNECTION_STATE, newAdapterState);
            intent.putExtra(NearlinkAdapter.EXTRA_PREVIOUS_CONNECTION_STATE, prevAdapterState);
            intent.putExtra(NearlinkAdapter.EXTRA_DISC_REASON, discReason);
            intent.putExtra(NearlinkAdapter.EXTRA_PAIR_STATE, pairState);
            intent.putExtra(NearlinkAdapter.EXTRA_PROFILE_TYPE, profile);
            // --屏蔽（setting无法接收广播） intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            Log.d(TAG, "ADAPTER_CONNECTION_STATE_CHANGE: " + device + ": " + prevAdapterState
                    + " -> " + newAdapterState);
            if (!isNormalStateTransition(prevState, state)) {
                Log.w(TAG, "ADAPTER_CONNECTION_STATE_CHANGE: unexpected transition for profile="
                        + profile + ", device=" + device + ", " + prevState + " -> " + state);
            }
            mService.sendBroadcastAsUser(intent, UserHandle.ALL, AdapterService.NEARLINK_PERM);
            Log.d(TAG, "sendConnectionStateChange: sendBroadcastAsUser device:" + device.getAddress()
                    + ",profile:" + profile + ",state transition " + prevState + " -> "
                    + state);

            ConnectionManager.getInstance().saveConnectDeviceInfo(device, intent);
        }
    }

    private static boolean isNormalStateTransition(int prevState, int nextState) {
        switch (prevState) {
            case NearlinkProfile.STATE_DISCONNECTED:
                return nextState == NearlinkProfile.STATE_CONNECTING;
            case NearlinkProfile.STATE_CONNECTED:
                return nextState == NearlinkProfile.STATE_DISCONNECTING;
            case NearlinkProfile.STATE_DISCONNECTING:
            case NearlinkProfile.STATE_CONNECTING:
                return (nextState == NearlinkProfile.STATE_DISCONNECTED)
                        || (nextState == NearlinkProfile.STATE_CONNECTED);
            default:
                return false;
        }
    }

    private boolean updateCountersAndCheckForConnectionStateChange(int state, int prevState) {
        switch (prevState) {
            case NearlinkProfile.STATE_CONNECTING:
                if (mProfilesConnecting > 0) {
                    mProfilesConnecting--;
                } else {
                    Log.d(TAG, "mProfilesConnecting " + mProfilesConnecting);
                    throw new IllegalStateException(
                            "Invalid state transition, " + prevState + " -> " + state);
                }
                break;

            case NearlinkProfile.STATE_CONNECTED:
                if (mProfilesConnected > 0) {
                    mProfilesConnected--;
                } else {
                    Log.d(TAG, "mProfilesConnected " + mProfilesConnected);
                    throw new IllegalStateException(
                            "Invalid state transition, " + prevState + " -> " + state);
                }
                break;

            case NearlinkProfile.STATE_DISCONNECTING:
                if (mProfilesDisconnecting > 0) {
                    mProfilesDisconnecting--;
                } else {
                    Log.d(TAG, "mProfilesDisconnecting " + mProfilesDisconnecting);
                    throw new IllegalStateException(
                            "Invalid state transition, " + prevState + " -> " + state);
                }
                break;
        }

        switch (state) {
            case NearlinkProfile.STATE_CONNECTING:
                mProfilesConnecting++;
                return (mProfilesConnected == 0 && mProfilesConnecting == 1);

            case NearlinkProfile.STATE_CONNECTED:
                mProfilesConnected++;
                return (mProfilesConnected == 1);

            case NearlinkProfile.STATE_DISCONNECTING:
                mProfilesDisconnecting++;
                return (mProfilesConnected == 0 && mProfilesDisconnecting == 1);

            case NearlinkProfile.STATE_DISCONNECTED:
                return (mProfilesConnected == 0 && mProfilesConnecting == 0);

            default:
                return true;
        }
    }

    private void updateProfileConnectionState(int profile, int newState, int oldState) {
        // mProfileConnectionState is a hashmap -
        // <Integer, Pair<Integer, Integer>>
        // The key is the profile, the value is a pair. first element
        // is the state and the second element is the number of devices
        // in that state.
        int numDev = 1;
        int newHashState = newState;
        boolean isUpdate = true;

        // The following conditions are considered in this function:
        // 1. If there is no record of profile and state - update
        // 2. If a new device's state is current hash state - increment
        //    number of devices in the state.
        // 3. If a state change has happened to Connected or Connecting
        //    (if current state is not connected), update.
        // 4. If numDevices is 1 and that device state is being updated, update
        // 5. If numDevices is > 1 and one of the devices is changing state,
        //    decrement numDevices but maintain oldState if it is Connected or
        //    Connecting
        Pair<Integer, Integer> stateNumDev = mProfileConnectionState.get(profile);
        if (stateNumDev != null) {
            int currHashState = stateNumDev.first;
            numDev = stateNumDev.second;

            if (newState == currHashState) {
                numDev++;
            } else if (newState == NearlinkProfile.STATE_CONNECTED || (
                    newState == NearlinkProfile.STATE_CONNECTING
                            && currHashState != NearlinkProfile.STATE_CONNECTED)) {
                numDev = 1;
            } else if (numDev == 1 && oldState == currHashState) {
                isUpdate = true;
            } else if (numDev > 1 && oldState == currHashState) {
                numDev--;

                if (currHashState == NearlinkProfile.STATE_CONNECTED
                        || currHashState == NearlinkProfile.STATE_CONNECTING) {
                    newHashState = currHashState;
                }
            } else {
                isUpdate = false;
            }
        }

        if (isUpdate) {
            mProfileConnectionState.put(profile, new Pair<Integer, Integer>(newHashState, numDev));
        }
    }

    private boolean validateProfileConnectionState(int state) {
        return (state == NearlinkProfile.STATE_DISCONNECTED
                || state == NearlinkProfile.STATE_CONNECTING
                || state == NearlinkProfile.STATE_CONNECTED
                || state == NearlinkProfile.STATE_DISCONNECTING);
    }

    private static int convertToAdapterState(int state) {
        switch (state) {
            case NearlinkProfile.STATE_DISCONNECTED:
                return NearlinkAdapter.STATE_DISCONNECTED;
            case NearlinkProfile.STATE_DISCONNECTING:
                return NearlinkAdapter.STATE_DISCONNECTING;
            case NearlinkProfile.STATE_CONNECTED:
                return NearlinkAdapter.STATE_CONNECTED;
            case NearlinkProfile.STATE_CONNECTING:
                return NearlinkAdapter.STATE_CONNECTING;
        }
        Log.d(TAG, "convertToAdapterState, unknow state " + state);
        return -1;
    }

    void onNearlinkReady() {
        Log.d(TAG, "onNearlinkReady");
        synchronized (mObject) {
            setConnectionState(NearlinkAdapter.STATE_DISCONNECTED);
            mProfileConnectionState.clear();
            mProfilesConnected = 0;
            mProfilesConnecting = 0;
            mProfilesDisconnecting = 0;
        }
    }

    void onSleDisable() {
        Log.d(TAG, "onSleDisable");
    }

    /**
     * 设置连接状态
     *
     * @param connectionState the mConnectionState to set
     */
    void setConnectionState(int connectionState) {
        mConnectionState = connectionState;
    }

    /**
     * 获取连接状态
     *
     * @return the mConnectionState
     */
    int getConnectionState() {
        return mConnectionState;
    }

    /**
     * 设置状态
     *
     * @param state 状态
     */
    void setState(int state) {
        Log.d(TAG, "Setting state to " + NearlinkAdapter.nameForState(state));
        mState = state;
    }

    /**
     * 获取状态
     *
     * @return the mState
     */
    int getState() {
        return mState;
    }

    /**
     * cleanup
     */
    public void cleanup() {
        mState = NearlinkAdapter.STATE_OFF;

        mRemoteDevices = null;
        mProfileConnectionState.clear();
        if (mReceiverRegistered) {
            mService.unregisterReceiver(mReceiver);
            mReceiverRegistered = false;
        }
        mService = null;
    }
}
