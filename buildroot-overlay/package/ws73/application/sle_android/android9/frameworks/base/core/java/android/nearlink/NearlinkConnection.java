/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.DeadObjectException;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * 连接管理
 *
 * @since 2023-12-01
 */
public class NearlinkConnection {
    private static final String TAG = "[SLE_CONN_FRAMEWORK]--NearlinkConnection";

    private final INearlinkManager iNearlinkManager;
    private INearlinkConnection connectionServer;
    private NearlinkConnectionCallbackWrapper wrapper;

    NearlinkConnection(INearlinkManager iNearlinkManager) {
        Log.d(TAG, "NearlinkConnection----create!");
        this.iNearlinkManager = iNearlinkManager;
        try {
            connectionServer = iNearlinkManager.getNearlinkConnection();
            wrapper = new NearlinkConnectionCallbackWrapper(connectionServer);
            connectionServer.connectionRegisterCallbacks(wrapper);
        } catch (RemoteException e) {
            Log.d(TAG, "Failed to get getNearlinkConnection - ", e);
        }
    }

    private NearlinkConnectionCallbackWrapper getWrapper() throws RemoteException {
        try {
            connectionServer.check();
        } catch (RemoteException e) {
            if (e instanceof DeadObjectException) {
                Log.d(TAG, "getWrapper() init NearlinkConnectionCallbackWrapper----:");
                connectionServer = iNearlinkManager.getNearlinkConnection();
                if (connectionServer != null) {
                    wrapper = new NearlinkConnectionCallbackWrapper(connectionServer);
                    connectionServer.connectionRegisterCallbacks(wrapper);
                } else {
                    Log.d(TAG, "getWrapper() connectionServer is null!");
                }
            }
            Log.d(TAG, "getWrapper() RemoteException:" + e.getMessage());
        }
        return wrapper;
    }

    private boolean hasInitParamSuccess() {
        if (connectionServer == null) {
            Log.d(TAG, "connectionServer service not available");
            try {
                connectionServer = iNearlinkManager.getNearlinkConnection();
                if (connectionServer != null) {
                    wrapper = new NearlinkConnectionCallbackWrapper(connectionServer);
                    connectionServer.connectionRegisterCallbacks(wrapper);
                } else {
                    Log.d(TAG, "hasInitParamSuccess:false");
                    return false;
                }
            } catch (Exception e) {
                Log.d(TAG, "hasInitParamSuccess:false", e);
                return false;
            }
        }
        return true;
    }

    /**
     * 主设备连接状态
     * <p>
     * NearlinkAdapter.getConnectionState()调用
     *
     * @return 主设备连接状态：
     * 若有至少一台设备连接返回 NearlinkConstant.SLE_ACB_STATE_CONNECTED;
     * 无连接返回 NearlinkConstant.SLE_ACB_STATE_NONE
     * @throws RemoteException remote exception
     */
    public int getAdapterConnectionState() throws RemoteException {
        if (!hasInitParamSuccess()) {
            return NearlinkConstant.SLE_ACB_STATE_NONE;
        }
        return getWrapper().getAdapterConnectionState();
    }

    /**
     * 获取配对设备数量
     *
     * @return 返回配对设备数量
     * @throws RemoteException remote exception
     */
    public int getPairedDevicesNum() throws RemoteException {
        if (!hasInitParamSuccess()) {
            return 0;
        }
        return getWrapper().getPairedDevicesNum();
    }

    /**
     * 获取配对设备列表
     * <p>
     * NearlinkAdapter.getPairedDevices()调用
     *
     * @return 获取配对设备列表
     * @throws RemoteException remote exception
     */
    public List<NearlinkDevice> getPairedDevices() throws RemoteException {
        if (!hasInitParamSuccess()) {
            return new ArrayList<>();
        }
        List<NearlinkAddress> list = getWrapper().getPairedDevices();
        List<NearlinkDevice> retList = new ArrayList<>();
        for (NearlinkAddress nearlinkAddress : list) {
            retList.add(new NearlinkDevice(nearlinkAddress.getAddress(), nearlinkAddress.getType()));
        }
        return retList;
    }

    /**
     * 取消配对
     * <p>
     * Nearlink中底层协议栈无取消配对动作，该方法同removePair()
     * NearlinkDevice.cancelPairProcess()调用
     *
     * @param nearlinkAddress 连接地址
     * @return true 发送成功；false 发送失败
     * @throws RemoteException 异常
     */
    public boolean cancelPairProcess(NearlinkAddress nearlinkAddress) throws RemoteException {
        return removePair(nearlinkAddress);
    }

    /**
     * 取消配对
     *
     * @param nearlinkAddress 连接地址
     * @return true 成功；false 失败
     * @throws RemoteException 异常
     */
    public boolean removePair(NearlinkAddress nearlinkAddress) throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return false;
        }
        return getWrapper().removePair(nearlinkAddress);
    }

    /**
     * 删除所有配对
     *
     * @return 返回该方法是否执行成功
     * @throws RemoteException remote exception
     */
    public boolean removeAllPairs() throws RemoteException {
        if (!hasInitParamSuccess()) {
            return false;
        }
        return getWrapper().removeAllPairs();
    }

    /**
     * 获取配对状态
     * <p>
     * NearlinkDevice.getPairState()调用
     *
     * @param nearlinkAddress 连接地址
     * @return 配对状态 NearlinkConstant中SLE_PAIR_NONE、SLE_PAIR_PAIRING、SLE_PAIR_PAIRED
     * @throws RemoteException 异常
     */
    public int getPairState(NearlinkAddress nearlinkAddress) throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return NearlinkConstant.SLE_PAIR_NONE;
        }
        return getWrapper().getPairState(nearlinkAddress);
    }

    /**
     * 获取连接状态
     * <p>
     * NearlinkDevice.getConnectionState()调用
     *
     * @param nearlinkAddress 连接地址
     * @return 连接状态
     * @throws RemoteException remote exception
     */
    public int getConnectionState(NearlinkAddress nearlinkAddress) throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return NearlinkConstant.SLE_ACB_STATE_NONE;
        }
        return getWrapper().getConnectionState(nearlinkAddress);
    }

    /**
     * 是否在本地启动绑定
     * <p>
     * NearlinkDevice.isPairingInitiatedLocally()调用
     *
     * @param nearlinkAddress 连接地址
     * @return true 是；false 否
     * @throws RemoteException 异常
     */
    public boolean isPairingInitiatedLocally(NearlinkAddress nearlinkAddress)
            throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return false;
        }
        return getWrapper().isPairingInitiatedLocally(nearlinkAddress);
    }

    /**
     * 发起连接，该方法无回调，连接状态由广播发送
     * NearlinkDevice.connection()调用
     *
     * @param nearlinkAddress 连接地址
     * @return true 是；false 否
     * @throws RemoteException 异常
     */
    public boolean createConnect(NearlinkAddress nearlinkAddress) throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return false;
        }
        return getWrapper().createConnect(nearlinkAddress);
    }

    /**
     * 断开连接，该方法无回调，连接状态由有广播发送
     * <p>
     * NearlinkDevice.disConnection()调用
     *
     * @param nearlinkAddress 连接地址
     * @return true 是；false 否
     * @throws RemoteException 异常
     */
    public boolean disConnect(NearlinkAddress nearlinkAddress) throws RemoteException {
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }
        if (!hasInitParamSuccess()) {
            return false;
        }
        return getWrapper().disConnect(nearlinkAddress);
    }

    /**
     * 释放回调注册
     *
     * @return 返回是否执行成功
     */
    public boolean onDestroy() {
        Log.d(TAG, "onDestroy()");
        try {
            connectionServer.check();
            connectionServer.connectionUnRegisterCallbacks(this.wrapper);
            return true;
        } catch (RemoteException e) {
            Log.d(TAG, "onDestroy()  RemoteException:" + e.getMessage());
            return false;
        }
    }
}
