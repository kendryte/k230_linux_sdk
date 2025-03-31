/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkAuthInfoEvt;
import android.util.Log;

import java.nio.charset.Charset;

/**
 * 连接管理 Jni回调
 *
 * @since 2023-12-01
 */
final class JniConnectionCallbacks {
    private static final String TAG = "[SLE_CONN_PACKAGE]--JniConnectionCallbacks";
    private final ConnectionService mConnectionService;

    JniConnectionCallbacks(ConnectionService connectionService) {
        this.mConnectionService = connectionService;
    }

    /**
     * 连接状态改变的回调函数
     *
     * @param connId      连接id
     * @param address     连接地址
     * @param addressType 地址类型
     * @param connState   连接状态
     * @param pairState   配对状态
     * @param discReason  断连原因
     */
    public void onConnectStateChanged(int connId, byte[] address, int addressType,
                                      int connState, int pairState, int discReason) {
        Log.d(TAG, "onConnectStateChanged(connId/address/addressType/connState/pairState/discReason):"
                + connId + "/" + PubTools.bytesToHex(address) + "/" + addressType + "/"
                + String.format("0x%02x", connState) + "/"
                + String.format("0x%02x", pairState) + "/"
                + String.format("0x%02x", discReason));
    }

    /**
     * 弃用的回调
     *
     * @param connId             连接id
     * @param errCode            错误码
     * @param intervalMin        intervalMin
     * @param intervalMax        intervalMax
     * @param maxLatency         maxLatency
     * @param supervisionTimeout supervisionTimeout
     */
    public void onConnectParamUpdateReq(int connId, int errCode,
                                        int intervalMin, int intervalMax,
                                        int maxLatency, int supervisionTimeout) {
        Log.d(TAG, "onConnectParamUpdateReq(connId/errCode/intervalMin/intervalMax/maxLatency/supervisionTimeout):"
                + connId + "/" + String.format("0x%x", errCode) + "/"
                + intervalMin + "/" + intervalMax + "/" + maxLatency + "/" + supervisionTimeout);
    }

    /**
     * 连接参数更新的回调函数。NearlinkConnectionParamUpdateEvt:interval,latency,supervision
     *
     * @param connId      连接id
     * @param errCode     错误码
     * @param interval    interval
     * @param latency     latency
     * @param supervision supervision
     */
    public void onConnectParamUpdate(int connId, int errCode, int interval, int latency, int supervision) {
        Log.d(TAG, "onConnectParamUpdate(connId/errCode/interval/latency/supervision):"
                + connId + "/" + String.format("0x%x", errCode) + "/" + interval + "/" + latency + "/" + supervision);
        mConnectionService.onConnectParamUpdate(connId, errCode, interval, latency, supervision);
    }

    /**
     * 认证完成的回调函数 NearlinkAuthInfoEvt:linkKey,cryptoAlgo,keyDerivAlgo,integrChkInd
     *
     * @param connId       连接id
     * @param address      连接地址
     * @param addressType  连接类型
     * @param errCode      错误码
     * @param linkKey      linkKey
     * @param cryptoAlgo   cryptoAlgo
     * @param keyDerivAlgo keyDerivAlgo
     * @param integrChkInd integrChkInd
     */
    public void onAuthComplete(int connId, byte[] address, int addressType, int errCode,
                               byte[] linkKey, int cryptoAlgo, int keyDerivAlgo, int integrChkInd) {
        Log.d(TAG, "onAuthComplete(connId/address/addressType/errCode/linkKey/cryptoAlgo/keyDerivAlgo/integrChkInd):"
                + connId + "/" + String.format("0x%x", errCode) + "/" + addressType
                + "/" + errCode + "/"
                + new String(linkKey, Charset.defaultCharset())
                + "/" + cryptoAlgo + "/" + keyDerivAlgo + "/" + integrChkInd);
        NearlinkAddress nearlinkAddress = new NearlinkAddress(PubTools.bytesToHex(address), addressType);
        NearlinkAuthInfoEvt authInfo = new NearlinkAuthInfoEvt();
        authInfo.setLinkKey(linkKey);
        authInfo.setCryptoAlgo(cryptoAlgo);
        authInfo.setKeyDerivAlgo(keyDerivAlgo);
        authInfo.setIntegrChkInd(integrChkInd);
        mConnectionService.onAuthComplete(nearlinkAddress, connId, authInfo, errCode);
    }

    /**
     * 配对完成的回调函数
     *
     * @param connId      连接id
     * @param address     连接地址
     * @param addressType 地址类型
     * @param errCode     错误码
     */
    public void onPairComplete(int connId, byte[] address, int addressType, int errCode) {
        Log.d(TAG, "onPairComplete(connId/address/addressType/errCode):"
                + connId + "/"
                + PubTools.bytesToHex(address) + "/" + addressType + "/"
                + String.format("0x%x", errCode));
        NearlinkAddress nearlinkAddress = new NearlinkAddress(PubTools.bytesToHex(address), addressType);
        mConnectionService.onPairComplete(nearlinkAddress, connId, errCode);
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
        mConnectionService.onReadRssi(connId, rssi, errCode);
    }
}
