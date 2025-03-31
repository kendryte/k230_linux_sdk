/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.hid;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.nearlink.INearlinkHidHost;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkHidHost;
import android.nearlink.NearlinkProfile;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.Settings;
import android.support.annotation.VisibleForTesting;
import android.util.Log;

import com.android.nearlink.Utils;
import com.android.nearlink.connection.PubTools;
import com.android.nearlink.nlservice.AdapterService;
import com.android.nearlink.nlservice.ProfileService;

/**
 * Provides Nearlink Hid Host profile, as a service in
 * the Nearlink application.
 *
 * @hide
 * @since 2023-11-24
 */
public class HidHostService extends ProfileService {
    private static final boolean DBG = true;
    private static final String TAG = "NearlinkHidHostService";

    private Map<NearlinkDevice, Integer> mInputDevices;
    private boolean mNativeAvailable;
    private static HidHostService sHidHostService;
    private NearlinkDevice mTargetDevice = null;
    private Map<String, Long> mConnectDevicesWithTime = new ConcurrentHashMap<>();

    private static final int MESSAGE_CONNECT = 1;
    private static final int MESSAGE_DISCONNECT = 2;
    private static final int MESSAGE_CONNECT_STATE_CHANGED = 3;
    private static final int MESSAGE_GET_PROTOCOL_MODE = 4;
    private static final int MESSAGE_VIRTUAL_UNPLUG = 5;
    private static final int MESSAGE_ON_GET_PROTOCOL_MODE = 6;
    private static final int MESSAGE_SET_PROTOCOL_MODE = 7;
    private static final int MESSAGE_GET_REPORT = 8;
    private static final int MESSAGE_ON_GET_REPORT = 9;
    private static final int MESSAGE_SET_REPORT = 10;
    private static final int MESSAGE_SEND_DATA = 11;
    private static final int MESSAGE_ON_VIRTUAL_UNPLUG = 12;
    private static final int MESSAGE_ON_HANDSHAKE = 13;
    private static final int MESSAGE_GET_IDLE_TIME = 14;
    private static final int MESSAGE_ON_GET_IDLE_TIME = 15;
    private static final int MESSAGE_SET_IDLE_TIME = 16;

    private static final int TIMEOUT_CONNECT_MS = 15 * 1000;

    /**
     * 连接失败的重试次数
     * 
     * 当前只允许重试1次
     */
    private static volatile int mReconnTime = 0;

    static {
        classInitNative();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (DBG) Log.d(TAG, "onCreate: ");
    }

    @Override
    public IProfileServiceBinder initBinder() {
        if (DBG) Log.d(TAG, "initBinder: ");
        return new NearlinkHidHostBinder(this);
    }

    @Override
    protected boolean start() {
        if (DBG) Log.d(TAG, "start: ");
        mInputDevices = Collections.synchronizedMap(new HashMap<NearlinkDevice, Integer>());
        initializeNative();
        mNativeAvailable = true;
        setHidHostService(this);
        return true;
    }

    @Override
    protected boolean stop() {
        if (DBG) {
            Log.d(TAG, "Stopping Nearlink HidHostService");
        }
        return true;
    }

    @Override
    protected void cleanup() {
        if (DBG) Log.d(TAG, "Stopping Nearlink HidHostService");
        if (mNativeAvailable) {
            cleanupNative();
            mNativeAvailable = false;
        }

        if (mInputDevices != null) {
            for (NearlinkDevice device : mInputDevices.keySet()) {
                int inputDeviceState = getConnectionState(device);
                if (inputDeviceState != NearlinkProfile.STATE_DISCONNECTED) {
                    broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTED);
                }
            }
            mInputDevices.clear();
        }
        setHidHostService(null);
    }

    public static synchronized HidHostService getHidHostService() {
        if (sHidHostService == null) {
            Log.w(TAG, "getHidHostService(): service is null");
            return null;
        }
        if (!sHidHostService.isAvailable()) {
            Log.w(TAG, "getHidHostService(): service is not available ");
            return null;
        }
        return sHidHostService;
    }

    private static synchronized void setHidHostService(HidHostService instance) {
        if (DBG) {
            Log.d(TAG, "setHidHostService(): set to: " + instance);
        }
        sHidHostService = instance;
    }

    @SuppressLint("HandlerLeak")
    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (DBG) Log.v(TAG, "handleMessage(): msg.what=" + msg.what);

            switch (msg.what) {
                case MESSAGE_CONNECT: {
                    NearlinkDevice device = (NearlinkDevice) msg.obj;
                    byte[] addr = PubTools.hexToByteArray(device.getAddress());
                    broadcastConnectionState(device, NearlinkProfile.STATE_CONNECTING);
                    mConnectDevicesWithTime.put(device.getAddress().toLowerCase(), SystemClock.elapsedRealtime());
                    boolean connectHidNativeResult = connectHidNative(addr);
                    Log.e(TAG, "connectHidNativeResult =" + connectHidNativeResult);
                    if (!connectHidNativeResult) {
                        broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTING);
                        broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTED);
                        break;
                    }
                    mTargetDevice = device;
                }
                break;
                case MESSAGE_DISCONNECT: {
                    NearlinkDevice device = (NearlinkDevice) msg.obj;
                    byte[] addr = PubTools.hexToByteArray(device.getAddress());
                    broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTING);
                    boolean disconnectHidNativeResult = disconnectHidNative(addr);
                    Log.e(TAG, "disconnectHidNativeResult = " + disconnectHidNativeResult);
                    if (!disconnectHidNativeResult) {
                        broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTING);
                        broadcastConnectionState(device, NearlinkProfile.STATE_DISCONNECTED);
                        break;
                    }
                }
                break;
                case MESSAGE_CONNECT_STATE_CHANGED: {
                    Bundle data = msg.getData();
                    byte[] address = data.getByteArray("address");
                    int halState = data.getInt("connState");
                    int pairState = data.getInt("pairState");
                    int discState = data.getInt("discState");
                    NearlinkDevice device = getDevice(address);
                    Integer prevStateInteger = mInputDevices.get(device);
                    int prevState =
                            (prevStateInteger == null) ? NearlinkHidHost.STATE_DISCONNECTED
                                    : prevStateInteger;
                    if (DBG) {
                        Log.d(TAG, "MESSAGE_CONNECT_STATE_CHANGED newState:"
                            + convertHalState(halState) + ", prevState:" + convertHalState(prevState));
                    }
                    if (halState == CONN_STATE_CONNECTED
                            && prevState == NearlinkHidHost.STATE_DISCONNECTED
                            && (!okToConnect(device))) {
                        if (DBG) {
                            Log.e(TAG, "Incoming HID connection rejected");
                        }
                        disconnectHidNative(Utils.getByteAddress(device));
                        if (mReconnTime == 1) {
                            mReconnTime = 0;
                        }
                    } else if (halState == CONN_STATE_DISCONNECTED
                                && prevState == NearlinkProfile.STATE_CONNECTING
                                && mReconnTime == 0) {
                        Log.w(TAG, "reconnect remote device when connecting failed");
                        connectHidNative(address);
                        mReconnTime = 1;
                    } else {
                        broadcastConnectionState(device, convertHalState(halState), pairState, discState);
                        if (mReconnTime == 1) {
                            mReconnTime = 0;
                        }
                    }
                    if (halState == CONN_STATE_CONNECTED && (mTargetDevice != null
                            && mTargetDevice.equals(device))) {
                        mTargetDevice = null;
                        // local device originated connection to hid device, move out
                        // of quiet mode
                        AdapterService adapterService = AdapterService.getAdapterService();
                        adapterService.enable(false);
                    }
                }
                break;
                case MESSAGE_ON_HANDSHAKE: {
                    NearlinkDevice device = getDevice((byte[]) msg.obj);
                    int status = msg.arg1;
                    broadcastHandshake(device, status);
                }
                break;
                case MESSAGE_SET_REPORT: {
                    NearlinkDevice device = (NearlinkDevice) msg.obj;
                    Bundle data = msg.getData();
                    byte reportType = data.getByte(NearlinkHidHost.EXTRA_REPORT_TYPE);
                    String report = data.getString(NearlinkHidHost.EXTRA_REPORT);
                    if (!setReportNative(Utils.getByteAddress(device), reportType, report)) {
                        Log.e(TAG, "Error: set report native returns false");
                    }
                }
                break;
            }
        }
    };

    /**
     * Handlers for incoming service calls
     */
    private static class NearlinkHidHostBinder extends INearlinkHidHost.Stub
            implements IProfileServiceBinder {
        private HidHostService mService;

        NearlinkHidHostBinder(HidHostService svc) {
            Log.e(TAG, "create NearlinkHidHostBinder: ");
            mService = svc;
        }

        @Override
        public void cleanup() {
            mService = null;
        }

        private HidHostService getService() {
//            AdapterService里没有初始设置用户id以及没有监听用户切换事件之前，暂时不调用!Utils.checkCaller()检查
            if (mService != null && mService.isAvailable()) {
                return mService;
            }
            Log.w(TAG, "Service is null");
            return null;
        }

        @Override
        public boolean connect(NearlinkDevice device) throws RemoteException {
            if (DBG) Log.d(TAG, "NearlinkHidHostBinder connect device = " + device);
            HidHostService service = getService();
            if (service == null) {
                if (DBG) Log.d(TAG, "service == null");
                return false;
            }
            return service.connect(device);
        }

        @Override
        public boolean disconnect(NearlinkDevice device) throws RemoteException {
            if (DBG) Log.d(TAG, "NearlinkHidHostBinder: disconnect()");
            HidHostService service = getService();
            if (service == null) {
                if (DBG) Log.d(TAG, "service == null");
                return false;
            }
            return service.disconnect(device);
        }

        @Override
        public int getConnectionState(NearlinkDevice device) throws RemoteException {
            if (DBG) Log.d(TAG, "NearlinkHidHostBinder: getConnectionState()");
            HidHostService service = getService();
            if (service == null) {
                return NearlinkHidHost.STATE_DISCONNECTED;
            }
            return service.getConnectionState(device);
        }

        @Override
        public List<NearlinkDevice> getConnectedDevices() throws RemoteException {
            if (DBG) Log.d(TAG, "NearlinkHidHostBinder: getConnectedDevices()");
            return getDevicesMatchingConnectionStates(new int[]{NearlinkProfile.STATE_CONNECTED});
        }

        @Override
        public List<NearlinkDevice> getDevicesMatchingConnectionStates(int[] states) throws RemoteException {
            if (DBG) Log.d(TAG, "NearlinkHidHostBinder: getDevicesMatchingConnectionStates()");
            HidHostService service = getService();
            if (service == null) {
                return new ArrayList<NearlinkDevice>(0);
            }
            return service.getDevicesMatchingConnectionStates(states);
        }

        @Override
        public boolean setPriority(NearlinkDevice device, int priority) throws RemoteException {
            HidHostService service = getService();
            if (service == null) {
                return false;
            }
            return service.setPriority(device, priority);
        }

        @Override
        public int getPriority(NearlinkDevice device) throws RemoteException {
            HidHostService service = getService();
            if (service == null) {
                return NearlinkProfile.PRIORITY_UNDEFINED;
            }
            return service.getPriority(device);
        }

        @Override
        public boolean setReport(NearlinkDevice device, byte reportType, String report) throws RemoteException {
            Log.e(TAG, "NearlinkHidHostBinder: setReport()");
            HidHostService service = getService();
            if (service == null) {
                return false;
            }
            return service.setReport(device, reportType, report);
        }
    }

    //APIs
    public boolean connect(NearlinkDevice device) {
        if (DBG) Log.d(TAG, "connect: " + device.getAddress());
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (getConnectionState(device) != NearlinkHidHost.STATE_DISCONNECTED) {
            String deviceAddr = device.getAddress().toLowerCase();
            long lastConnectTime = mConnectDevicesWithTime.getOrDefault(deviceAddr, 0L);
            long dConnectTime = SystemClock.elapsedRealtime() - lastConnectTime;
            Log.d(TAG, "Hid Device connect dtime =  " + dConnectTime);
            if (dConnectTime < TIMEOUT_CONNECT_MS) {
                Log.e(TAG, "Hid Device not disconnected: " + device);
                return false;
            }
        }
        if (getPriority(device) == NearlinkHidHost.PRIORITY_OFF) {
            Log.e(TAG, "Hid Device PRIORITY_OFF: " + device);
            return false;
        }

        Message msg = mHandler.obtainMessage(MESSAGE_CONNECT, device);
        mHandler.sendMessage(msg);
        return true;
    }

    public boolean disconnect(NearlinkDevice device) {
        if (DBG) Log.d(TAG, "disconnect: " + device.getAddress());
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        Message msg = mHandler.obtainMessage(MESSAGE_DISCONNECT, device);
        mHandler.sendMessage(msg);
        return true;
    }

    int getConnectionState(NearlinkDevice device) {
        if (DBG) Log.d(TAG, "getConnectionState: " + device.getAddress());
        if (mInputDevices.get(device) == null) {
            return NearlinkHidHost.STATE_DISCONNECTED;
        }
        return mInputDevices.get(device);
    }

    List<NearlinkDevice> getDevicesMatchingConnectionStates(int[] states) {
        if (DBG) Log.d(TAG, "getDevicesMatchingConnectionStates()");
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        List<NearlinkDevice> inputDevices = new ArrayList<NearlinkDevice>();

        for (NearlinkDevice device : mInputDevices.keySet()) {
            int inputDeviceState = getConnectionState(device);
            for (int state : states) {
                if (state == inputDeviceState) {
                    inputDevices.add(device);
                    break;
                }
            }
        }
        return inputDevices;
    }

    public boolean setPriority(NearlinkDevice device, int priority) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        if (DBG) {
            Log.d(TAG, "setPriority: " + device.getAddress());
        }
        Settings.Global.putInt(getContentResolver(),
                Settings.Global.getNearlinkHidHostPriorityKey(device.getAddress()), priority);
        if (DBG) {
            Log.d(TAG, "Saved priority " + device + " = " + priority);
        }
        return true;
    }

    public int getPriority(NearlinkDevice device) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        if (DBG) {
            Log.d(TAG, "getPriority: " + device.getAddress());
        }
        int priority = Settings.Global.getInt(getContentResolver(),
                Settings.Global.getNearlinkHidHostPriorityKey(device.getAddress()),
                NearlinkProfile.PRIORITY_UNDEFINED);
        if (DBG) {
            Log.d(TAG, "getPriority: priority = " + priority);
        }
        return priority;
    }

    boolean setReport(NearlinkDevice device, byte reportType, String report) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        if (DBG) {
            Log.d(TAG, "setReport: " + device.getAddress());
        }
        int state = this.getConnectionState(device);
        if (state != NearlinkHidHost.STATE_CONNECTED) {
            return false;
        }
        Message msg = mHandler.obtainMessage(MESSAGE_SET_REPORT);
        msg.obj = device;
        Bundle data = new Bundle();
        data.putByte(NearlinkHidHost.EXTRA_REPORT_TYPE, reportType);
        data.putString(NearlinkHidHost.EXTRA_REPORT, report);
        msg.setData(data);
        mHandler.sendMessage(msg);
        return true;

    }

    private void onHandshake(byte[] address, int status) {
        if (DBG)
            Log.d(TAG, "onHandshake: address = " + Arrays.toString(address) + " , status=" + status);
        Message msg = mHandler.obtainMessage(MESSAGE_ON_HANDSHAKE);
        msg.obj = address;
        msg.arg1 = status;
        mHandler.sendMessage(msg);
    }

    private void onConnectStateChanged(byte[] address, int connState, int pairState, int discState) {
        if (DBG)
            Log.d(TAG, "onConnectStateChanged:" +
                    " address = " + PubTools.macPrint(Utils.getAddressStringFromByte(address)) + " ," +
                    " state = " + connState);
        Message msg = mHandler.obtainMessage(MESSAGE_CONNECT_STATE_CHANGED);
        Bundle bundle = new Bundle();
        bundle.putByteArray("address", address);
        bundle.putInt("connState", connState);
        bundle.putInt("pairState", pairState);
        bundle.putInt("discState", discState);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    private void broadcastConnectionState(NearlinkDevice device, int newState) {
        broadcastConnectionState(device, newState, NearlinkConstant.SLE_PAIR_NONE, NearlinkConstant.SLE_DISCONNECT_BY_NONE);
    }

    // This method does not check for error condition (newState == prevState)
    private void broadcastConnectionState(NearlinkDevice device, int newState, int pairState, int discState) {
        Integer prevStateInteger = mInputDevices.get(device);
        int prevState = (prevStateInteger == null) ? NearlinkHidHost.STATE_DISCONNECTED
                : prevStateInteger;
        if (prevState == newState) {
            Log.w(TAG, "no state change: " + newState);
            return;
        }
        mInputDevices.put(device, newState);

        /* Notifying the connection state change of the profile before sending the intent for
           connection state change, as it was causing a race condition, with the UI not being
           updated with the correct connection state. */
        Log.e(TAG, "Connection state " + device + ": " + prevState + "->" + newState);
        Intent intent = new Intent(NearlinkHidHost.ACTION_CONNECTION_STATE_CHANGED);
        intent.putExtra(NearlinkProfile.EXTRA_PREVIOUS_STATE, prevState);
        intent.putExtra(NearlinkProfile.EXTRA_STATE, newState);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
        intent.putExtra(NearlinkProfile.EXTRA_PAIR_STATE, pairState);
        intent.putExtra(NearlinkProfile.EXTRA_DISC_REASON, discState);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sendBroadcastAsUser(intent, UserHandle.ALL, NEARLINK_PERM);
    }

    private void broadcastHandshake(NearlinkDevice device, int status) {
        Intent intent = new Intent(NearlinkHidHost.ACTION_HANDSHAKE);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
        intent.putExtra(NearlinkHidHost.EXTRA_STATUS, status);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sendBroadcast(intent, NEARLINK_PERM);
    }

    private void broadcastReport(NearlinkDevice device, byte[] report, int rptSize) {
        Intent intent = new Intent(NearlinkHidHost.ACTION_REPORT);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
        intent.putExtra(NearlinkHidHost.EXTRA_REPORT, report);
        intent.putExtra(NearlinkHidHost.EXTRA_REPORT_BUFFER_SIZE, rptSize);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sendBroadcast(intent, NEARLINK_PERM);
    }

    /**
     * Check whether can connect to a peer device.
     * The check considers a number of factors during the evaluation.
     *
     * @param device the peer device to connect to
     * @return true if connection is allowed, otherwise false
     */
    @VisibleForTesting(otherwise = VisibleForTesting.PACKAGE_PRIVATE)
    public boolean okToConnect(NearlinkDevice device) {
        AdapterService adapterService = AdapterService.getAdapterService();
        // Check if adapter service is null.
        if (adapterService == null) {
            Log.w(TAG, "okToConnect: adapter service is null");
            return false;
        }
        // Check if this is an incoming connection in Quiet mode.
        if (adapterService.isQuietModeEnabled() && mTargetDevice == null) {
            Log.w(TAG, "okToConnect: return false as quiet mode enabled");
            return false;
        }
        return true;
    }

    private static int convertHalState(int halState) {
        switch (halState) {
            case CONN_STATE_CONNECTED:
                return NearlinkProfile.STATE_CONNECTED;
            case CONN_STATE_CONNECTING:
                return NearlinkProfile.STATE_CONNECTING;
            case CONN_STATE_DISCONNECTED:
                return NearlinkProfile.STATE_DISCONNECTED;
            case CONN_STATE_DISCONNECTING:
                return NearlinkProfile.STATE_DISCONNECTING;
            default:
                Log.e(TAG, "bad hid connection state: " + halState);
                return NearlinkProfile.STATE_DISCONNECTED;
        }
    }

    @Override
    public void dump(StringBuilder sb) {
        super.dump(sb);
        println(sb, "mTargetDevice: " + mTargetDevice);
        println(sb, "mInputDevices:");
        for (NearlinkDevice device : mInputDevices.keySet()) {
            println(sb, "  " + device + " : " + mInputDevices.get(device));
        }
    }

    private static final int CONN_STATE_CONNECTED = NearlinkConstant.SLE_ACB_STATE_CONNECTED;
    private static final int CONN_STATE_CONNECTING = NearlinkConstant.SLE_ACB_STATE_CONNECTING;
    private static final int CONN_STATE_DISCONNECTED = NearlinkConstant.SLE_ACB_STATE_DISCONNECTED;
    private static final int CONN_STATE_DISCONNECTING = NearlinkConstant.SLE_ACB_STATE_DISCONNECTING;

    private static native void classInitNative();

    private native void initializeNative();

    private native void cleanupNative();

    private native boolean connectHidNative(byte[] address);

    private native boolean disconnectHidNative(byte[] address);

    private native boolean setReportNative(byte[] address, byte reportType, String report);

}
