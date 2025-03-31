/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.RemoteException;
import android.util.Log;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 连接回调
 *
 * @since 2023-12-01
 */
class NearlinkConnectionCallbackWrapper extends INearlinkConnectionCallback.Stub {
    private static final String TAG = "[SLE_CONN_FRAMEWORK]--NearlinkConnectionCallbackWrapper";
    private final Map<String, NearlinkCommCallback.PairStateChangedCallback> pairCbMap = new HashMap<>();
    private final Map<Integer, NearlinkCommCallback.ReadRssiCallback> readRssiCbMap = new HashMap<>();

    private final INearlinkConnection mConnectionServer;

    /**
     * 构造方法
     *
     * @param connectionServer INearlinkConnection
     */
    protected NearlinkConnectionCallbackWrapper(INearlinkConnection connectionServer) {
        Log.d(TAG, "NearlinkConnectionCallbackWrapper init...");
        this.mConnectionServer = connectionServer;
    }

    @Override
    public void onAuthCompleteCallback(NearlinkAddress nearlinkAddress, int connId,
                                       NearlinkAuthInfoEvt authInfo, int errCode) {
        Log.i(TAG, "onAuthCompleteCallback connId:" + connId);
        Log.i(TAG, "onAuthCompleteCallback nearlinkAddress:" + nearlinkAddress.toString());
        Log.i(TAG, "onAuthCompleteCallback authInfo:" + authInfo);
        Log.i(TAG, "onAuthCompleteCallback errCode:" + errCode);
    }

    /**
     * 删除所有配对
     *
     * @return 返回该方法是否执行成功
     * @throws RemoteException remote exception
     */
    protected boolean removeAllPairs() throws RemoteException {
        synchronized (pairCbMap) {
            int ret = mConnectionServer.removeAllPairs();
            Log.d(TAG, "removeAllPairs.....ret:" + ret);
            if (ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                pairCbMap.clear();
            }
            return ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
        }
    }

    /**
     * 删除配对
     *
     * @param nearlinkAddress 连接地址
     * @return 返回该方法是否执行成功
     * @throws RemoteException remote exception
     */
    protected boolean removePair(NearlinkAddress nearlinkAddress) throws RemoteException {
        synchronized (pairCbMap) {
            int ret = mConnectionServer.removePairedRemoteDevice(nearlinkAddress);
            Log.d(TAG, "removePair.....ret:" + ret
                    + ".....:" + nearlinkAddress.toString());
            if (ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                pairCbMap.remove(nearlinkAddress.getAddress());
            }
            return ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
        }
    }

    /**
     * 是否在本地启动绑定
     *
     * @param nearlinkAddress 连接地址
     * @return 是否在本地启动绑定
     * @throws RemoteException remote exception
     */
    protected boolean isPairingInitiatedLocally(NearlinkAddress nearlinkAddress) throws RemoteException {
        boolean isRet = mConnectionServer.isPairingInitiatedLocally(nearlinkAddress);
        Log.d(TAG, "isPairingInitiatedLocally.....ret:" + isRet
                + ".....:" + nearlinkAddress.toString());
        return isRet;
    }

    /**
     * 获取配对设备列表
     * NearlinkAdapter.getPairedDevices()调用
     *
     * @return 获取配对设备列表
     * @throws RemoteException remote exception
     */
    protected List<NearlinkAddress> getPairedDevices() throws RemoteException {
        return mConnectionServer.getPairedDevices();
    }

    /**
     * 主设备连接状态
     * NearlinkAdapter.getConnectionState()调用
     *
     * @return 主设备连接状态：
     * 若有至少一台设备连接返回 NearlinkConstant.SLE_ACB_STATE_CONNECTED;
     * 无连接返回 NearlinkConstant.SLE_ACB_STATE_NONE
     * @throws RemoteException remote exception
     */
    protected int getAdapterConnectionState() throws RemoteException {
        int ret = mConnectionServer.getAdapterConnectionState();
        Log.d(TAG, "getAdapterConnectionState.....ret:" + ret);
        return ret;
    }

    /**
     * 断开连接
     *
     * @param nearlinkAddress 连接地址
     * @return 返回该方法是否执行成功
     * @throws RemoteException remote exception
     */
    protected boolean disConnect(NearlinkAddress nearlinkAddress) throws RemoteException {
        int ret = mConnectionServer.disconnectRemoteDevice(nearlinkAddress);
        Log.d(TAG, "disConnect.....ret:" + ret
                + ".....:" + nearlinkAddress.toString());
        return ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    /**
     * 发起连接
     *
     * @param nearlinkAddress 连接地址
     * @return 返回该方法是否执行成功
     * @throws RemoteException remote exception
     */
    protected boolean createConnect(NearlinkAddress nearlinkAddress) throws RemoteException {
        int ret = mConnectionServer.connectRemoteDevice(nearlinkAddress);
        Log.d(TAG, "createConnect.....ret:" + ret
                + ".....:" + nearlinkAddress.toString());
        return ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    /**
     * 获取连接状态
     *
     * @param nearlinkAddress 连接地址
     * @return 返回连接状态
     * @throws RemoteException remote exception
     */
    protected int getConnectionState(NearlinkAddress nearlinkAddress) throws RemoteException {
        return mConnectionServer.getConnectionState(nearlinkAddress);
    }

    /**
     * 获取配对状态
     *
     * @param nearlinkAddress 连接地址
     * @return 返回配对状态
     * @throws RemoteException remote exception
     */
    protected int getPairState(NearlinkAddress nearlinkAddress) throws RemoteException {
        return mConnectionServer.getPairState(nearlinkAddress);
    }

    /**
     * 获取配对设备数量
     *
     * @return 返回配对设备数量
     * @throws RemoteException remote exception
     */
    protected int getPairedDevicesNum() throws RemoteException {
        int ret = mConnectionServer.getPairedDevicesNum();
        Log.d(TAG, "getPairedDevicesNum ret:" + ret);
        return ret;
    }
}
