/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.util.Log;

/**
 * include All callback of Nearlink connection
 *
 * @since 2023-12-01
 */
public class NearlinkCommCallback {
    private static final String TAG = "[SLE_CONN_PACKAGE]--NearlinkCommCallback";

    /**
     * 配对状态改变回调 interface
     */
    public interface PairStateChangedCallback {
        /**
         * 配对过程中状态
         *
         * @param connId          连接id
         * @param nearlinkAddress 连接地址
         * @param pairState       配对状态
         * @param errCode         错误码
         */
        default void onProcessing(NearlinkAddress nearlinkAddress, int connId, int pairState, int errCode) {
        }

        /**
         * 配对成功
         *
         * @param nearlinkAddress 连接地址
         * @param connId          连接id
         */
        void onSuccess(NearlinkAddress nearlinkAddress, int connId);

        /**
         * 配对失败
         *
         * @param errCode 错误码
         */
        void onFail(int errCode);
    }

    /**
     * 获取RSSI回调 interface
     */
    public interface ReadRssiCallback {
        /**
         * 获取rssi过程中
         *
         * @param connId  连接id
         * @param rssi    rssi
         * @param errCode 错误码
         */
        default void onProcessing(int connId, int rssi, int errCode) {
            Log.d(TAG, "ReadRssiCallback onProcessing(connId/rssi/errCode):"
                    + connId + "/" + rssi + "/" + String.format("0x%x", errCode));
        }

        /**
         * 获取rssi成功
         *
         * @param connId 连接id
         * @param rssi   rssi
         */
        void onSuccess(int connId, int rssi);

        /**
         * 获取rssi失败
         *
         * @param errCode 错误码
         */
        void onFail(int errCode);
    }
}
