/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.nearlink.INearlinkConnection;
import android.nearlink.INearlinkConnectionCallback;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.nlservice.ProfileService;

import java.util.List;

/**
 * 连接管理 ServiceBinder
 *
 * @since 2023-12-01
 */
public class ConnectionServiceBinder extends INearlinkConnection.Stub implements ProfileService.IProfileServiceBinder {
    private static final String TAG = "[SLE_CONN_PACKAGE]--ConnectionServiceBinder";
    private ConnectionService mService;
    private final RemoteCallbackList<INearlinkConnectionCallback> callbackList = new RemoteCallbackList<>();

    ConnectionServiceBinder(ConnectionService svc) {
        Log.d(TAG, "init.....svc:" + svc);
        mService = svc;
    }

    /**
     * 获取 ConnectionService
     *
     * @return ConnectionService
     */
    public ConnectionService getService() {
        Log.d(TAG, "getService.....:" + mService);
        return mService;
    }

    protected RemoteCallbackList<INearlinkConnectionCallback> getCallbackList() {
        return callbackList;
    }

    @Override
    public int connectRemoteDevice(NearlinkAddress nearlinkAddress) throws RemoteException {
        boolean isRet = ConnectionManager.getInstance().connectAllProfile(nearlinkAddress);
        Log.d(TAG, "connectRemoteDevice.....ret:" + isRet
                + ".....:" + nearlinkAddress.toString());
        return isRet ? 0 : NearlinkDevice.ERROR;
    }

    @Override
    public int disconnectRemoteDevice(NearlinkAddress nearlinkAddress) throws RemoteException {
        boolean isRet = ConnectionManager.getInstance().disConnectAllProfile(nearlinkAddress);
        Log.d(TAG, "disconnectRemoteDevice.....ret:" + isRet
                + ".....:" + nearlinkAddress.toString());
        return isRet ? 0 : NearlinkDevice.ERROR;
    }

    @Override
    public int removePairedRemoteDevice(NearlinkAddress nearlinkAddress) throws RemoteException {
        int ret = mService.removePairedRemoteDeviceAndCleanCache(nearlinkAddress);
        Log.d(TAG, "removePairedRemoteDevice.....ret:" + ret
                + ".....:" + nearlinkAddress.toString());
        return ret;
    }

    @Override
    public int removeAllPairs() throws RemoteException {
        int ret = mService.removeAllPairsAndCleanCache();
        Log.d(TAG, "removeAllPairs.....ret:" + ret);
        return ret;
    }

    @Override
    public int getPairedDevicesNum() throws RemoteException {
        int ret = ConnectionManager.getInstance().getPairedDevicesNum();
        Log.d(TAG, "getPairedDevicesNum.....ret:" + ret);
        return ret;
    }

    @Override
    public List<NearlinkAddress> getPairedDevices() throws RemoteException {
        return ConnectionManager.getInstance().getPairedDevices();
    }

    @Override
    public int getPairState(NearlinkAddress nearlinkAddress) throws RemoteException {
        return ConnectionManager.getInstance().getPairedState(nearlinkAddress);
    }

    @Override
    public void connectionRegisterCallbacks(INearlinkConnectionCallback callback) throws RemoteException {
        Log.d(TAG, "connectionRegisterCallbacks.....callback:" + callback);
        callbackList.register(callback);
    }

    @Override
    public void connectionUnRegisterCallbacks(INearlinkConnectionCallback callback) throws RemoteException {
        Log.d(TAG, "connectionUnRegisterCallbacks.....callback:" + callback);
        callbackList.unregister(callback);
    }

    @Override
    public int getConnectionState(NearlinkAddress nearlinkAddress) throws RemoteException {
        int ret = ConnectionManager.getInstance().getConnectionState(nearlinkAddress.getAddress());
        ret = (NearlinkConstant.SLE_ACB_STATE_NONE <= ret
                && ret <= NearlinkConstant.SLE_ACB_STATE_DISCONNECTING) ? ret : NearlinkDevice.ERROR;
        // --屏蔽日志-- Log.d(TAG, "getConnectionState.....ret:" + ret
        // --屏蔽日志--         + ".....:" + nearlinkAddress.toString());
        return ret;
    }

    @Override
    public boolean isPairingInitiatedLocally(NearlinkAddress nearlinkAddress) throws RemoteException {
        boolean isRet = true;
        Log.d(TAG, "isPairingInitiatedLocally.....ret:" + isRet
                + ".....:" + nearlinkAddress.toString());
        return isRet;
    }

    @Override
    public int getAdapterConnectionState() throws RemoteException {
        int ret = ConnectionManager.getInstance().getAdapterConnectionState();
        Log.d(TAG, "getAdapterConnectionState.....ret:" + ret);
        return ret;
    }

    @Override
    public boolean check() throws RemoteException {
        return true;
    }


    @Override
    public void cleanup() {
        Log.d(TAG, "cleanup.....");
        mService = null;
    }
}
