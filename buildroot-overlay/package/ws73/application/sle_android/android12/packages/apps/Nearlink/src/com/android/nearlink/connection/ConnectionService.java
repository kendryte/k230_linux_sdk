/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.content.Intent;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkAuthInfoEvt;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkErrorCode;
import android.os.IBinder;
import android.util.Log;

import com.android.nearlink.nlservice.ProfileService;

/**
 * 连接管理服务
 *
 * @since 2023-12-01
 */
public class ConnectionService extends ProfileService {
    private static final String TAG = "[SLE_CONN_PACKAGE]--ConnectionService";
    private static ConnectionService connectionService;

    static {
        classInitNative();
        Log.d(TAG, "static init finish!");
    }

    private JniConnectionCallbacks mJniCallbacks;
    private ConnectionServiceBinder mBinder;

    /**
     * 获取 ConnectionService
     *
     * @return ConnectionService
     */
    public static synchronized ConnectionService getConnectionService() {
        Log.d(TAG, "getAdapterService() - returning " + connectionService);
        return connectionService;
    }

    private static synchronized void setConnectionService(ConnectionService instance) {
        Log.d(TAG, "setAdapterService() - trying to set service to " + instance);
        if (instance == null) {
            return;
        }
        connectionService = instance;
    }

    private static synchronized void clearConnectionService(ConnectionService current) {
        Log.d(TAG, "clearAdapterService  ..." + current);
        if (connectionService == current) {
            connectionService = null;
        }
    }

    @Override
    protected IProfileServiceBinder initBinder() {
        Log.d(TAG, "initBinder  ...");
        this.mBinder = new ConnectionServiceBinder(this);
        return this.mBinder;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    @Override
    protected boolean stop() {
        return true;
    }

    @Override
    protected void cleanup() {
        clearConnectionService(this);
        cleanupNative();
    }

    @Override
    protected void create() {
        super.create();
        mJniCallbacks = new JniConnectionCallbacks(this);
        setConnectionService(this);
    }

    protected ConnectionServiceBinder getBinder() {
        return mBinder;
    }

    @Override
    protected boolean start() {
        return true;
    }

    /**
     * 连接状态改变的回调函数
     *
     * @param connId     连接id
     * @param nlAddress  连接地址
     * @param connState  连接状态
     * @param pairState  配对状态
     * @param discReason 断连原因
     * @deprecated
     */
    void onConnectStateChanged(NearlinkAddress nlAddress, int connId, int connState, int pairState, int discReason) {
        Log.d(TAG, "onConnectStateChanged(connId/address/addressType/connState/pairState/discReason):"
                + connId + "/" + PubTools.macPrint(nlAddress.getAddress()) + "/" + nlAddress.getType() + "/"
                + String.format("0x%02x", connState) + "/"
                + String.format("0x%02x", pairState) + "/"
                + String.format("0x%02x", discReason));
    }


    /**
     * 连接参数更新的回调函数
     *
     * @param connId      连接id
     * @param errCode     错误码
     * @param interval    链路调度间隔
     * @param latency     延迟周期
     * @param supervision 超时时间
     * @deprecated
     */
    public void onConnectParamUpdate(int connId, int errCode, int interval, int latency, int supervision) {
        Log.d(TAG, "onConnectParamUpdate(connId/errCode/interval/latency/supervision):"
                + connId + "/" + String.format("0x%x", errCode)
                + "/" + interval + "/" + latency + "/" + supervision);
    }

    /**
     * 认证完成的回调函数
     *
     * @param connId    连接id
     * @param nlAddress 连接地址
     * @param errCode   错误码
     * @param authInfo  认证参数
     */
    public void onAuthComplete(NearlinkAddress nlAddress, int connId, NearlinkAuthInfoEvt authInfo, int errCode) {
        Log.d(TAG, "onAuthComplete nearlinkAddress:" + nlAddress.toString());
        Log.d(TAG, "onAuthComplete connId:" + connId);
        Log.d(TAG, "onAuthComplete authInfo:" + authInfo.toString());
        Log.d(TAG, "onAuthComplete errCode:" + errCode);
    }

    /**
     * 配对完成的回调函数
     *
     * @param connId    连接id
     * @param nlAddress 连接地址
     * @param errCode   错误码
     */
    public void onPairComplete(NearlinkAddress nlAddress, int connId, int errCode) {
        Log.d(TAG, "onPairComplete(connId/address/addressType/errCode):" + connId + "/"
                + nlAddress.getAddress() + "/" + nlAddress.getType() + "/" + String.format("0x%x", errCode));
    }

    /**
     * 读取rssi的回调函数
     *
     * @param connId  连接id
     * @param rssi    rssi
     * @param errCode 错误码
     */
    public void onReadRssi(int connId, int rssi, int errCode) {
        Log.d(TAG, "onReadRssi(connId/rssi/errCode):" + connId + "/" + rssi + "/" + String.format("0x%x", errCode));
    }

    int pairRemoteDevice(NearlinkAddress nlAddress) {
        Log.d(TAG, "pairRemoteDevice.....:" + nlAddress.toString());
        return NearlinkDevice.ERROR;
    }
    int removePairedRemoteDeviceAndCleanCache(NearlinkAddress nlAddress) {
        int ret = removePairedRemoteDeviceNative(PubTools.hexToByteArray(nlAddress.getAddress()),
                (byte) nlAddress.getType());
        ConnectionManager.getInstance().remove(nlAddress);
        return ret;
    }

    int removeAllPairsAndCleanCache() {
        int ret = removeAllPairsNative();
        if (ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            ConnectionManager.getInstance().removeAll();
        }
        return ret;
    }

    int readRemoteDeviceRssi(int connId) {
        Log.d(TAG, "readRemoteDeviceRssi.....connId:" + connId);
        int ret = readRemoteDeviceRssiNative((short) connId);
        Log.d(TAG, "readRemoteDeviceRssi.....ret:" + ret);
        return ret;
    }

    // 初始化JNI
    static native void classInitNative();

    // 初始化JNI
    native boolean initNative();

    // 释放JNI
    native void cleanupNative();

    // 发送连接请求
    native int connectRemoteDeviceNative(byte[] addr, byte type);

    // 发送断开连接请求
    native int disconnectRemoteDeviceNative(byte[] addr, byte type);

    // 发送更新连接参数请求
    native int updateConnectParamNative(short connId, short intervalMin, short intervalMax,
                                        short maxLatency, short supervisionTimeout);

    // 发送配对请求
    native int pairRemoteDeviceNative(byte[] addr, byte type);

    // 删除配对
    native int removePairedRemoteDeviceNative(byte[] addr, byte type);

    // 删除所有配对
    native int removeAllPairsNative();

    // 获取配对设备数量
    native int getPairedDevicesNumNative();

    // 获取配对设备
    native int getPairedDevicesNative(NearlinkAddressList list);

    // 获取配对状态
    native int getPairStateNative(byte[] addr, byte type);

    // 读取设备rssi值
    native int readRemoteDeviceRssiNative(short connId);
}
