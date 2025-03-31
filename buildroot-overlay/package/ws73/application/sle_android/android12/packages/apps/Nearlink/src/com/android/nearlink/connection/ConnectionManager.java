/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkProfile;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import com.android.nearlink.discovery.DiscoveryService;
import com.android.nearlink.hid.HidHostService;
import com.android.nearlink.nlservice.AdapterService;
import com.android.nearlink.nlservice.Config;
import com.android.nearlink.nlservice.ServiceFactory;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 连接管理
 *
 * @since 2023-12-11
 */
public class ConnectionManager {
    private static final String TAG = "[SLE_CONN_PACKAGE]--ConnectionManager";
    private static final int MESSAGE_ACB_DISCONNECTED_BY_REMOTE = 0x01; // 消息-对端断连
    private static final int MESSAGE_ADAPTER_STATE_TURNED_ON = 0x02; // 消息-星闪开启
    private static final int MESSAGE_ADAPTER_STATE_TURNED_OFF = 0x03; // 消息-星闪关闭
    private static final int MESSAGE_NOTE_CONNECT = 0x04; // 消息-通知连接
    private static final int MESSAGE_NOTE_DISCONNECT = 0x05; // 消息-通知断连
    private static final int MESSAGE_ACTION_DETECTED = 0x06; // 消息-扫描到设备
    private static final int MESSAGE_SAVE_DEVICE_INFO = 0x07; // 消息-保存缓存信息
    private static ConnectionManager instance;
    private static final long CONN_TIMEOUT = 1000 * 15;
    private static final int HAS_CONNECTED = 0x01;

    public static final Object obj = new Object();

    private final AdapterService mService;
    private final Map<String, ConnectionDevice> connMap = new HashMap<>();
    private final Map<String, Long> connQueue = new HashMap<>();
    private final ServiceFactory serviceFactory;
    private final Handler mHandler;
    private boolean isAutoConnect = true;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Log.d(TAG, "Received intent with null action");
                return;
            }
            if (NearlinkAdapter.ACTION_STATE_CHANGED.equals(action)) {
                int newState = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, -1);
                if (newState == NearlinkAdapter.STATE_ON) {
                    Log.d(TAG, "ACTION_STATE_CHANGED newState:STATE_ON");
                    mHandler.obtainMessage(MESSAGE_ADAPTER_STATE_TURNED_ON).sendToTarget();
                } else if (newState == NearlinkAdapter.STATE_OFF) {
                    Log.d(TAG, "ACTION_STATE_CHANGED newState:STATE_OFF");
                    mHandler.obtainMessage(MESSAGE_ADAPTER_STATE_TURNED_OFF).sendToTarget();
                }
            } else if (NearlinkDevice.ACTION_DETECTED.equals(action)) {
                if (isAutoConnect) {
                    NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                    if (device != null) {
                        mHandler.obtainMessage(MESSAGE_ACTION_DETECTED, device).sendToTarget();
                    }
                }
            } else {
                Log.d(TAG, "Received intent with filter action");
            }
        }
    };

    private ConnectionManager(AdapterService service) {
        this.mService = service;
        this.serviceFactory = new ServiceFactory();
        this.mHandler = new ConnHandler(this.mService.getMainLooper());
        connMap.clear();
        connQueue.clear();
        connMap.putAll(readFromDisk());
        registerReceiver();
    }

    public static synchronized ConnectionManager getInstance() {
        if (instance == null) {
            instance = new ConnectionManager(AdapterService.getAdapterService());
        }
        return instance;
    }

    private class ConnHandler extends Handler {
        ConnHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_ACB_DISCONNECTED_BY_REMOTE:
                case MESSAGE_ADAPTER_STATE_TURNED_ON: {
                    Log.d(TAG, "ConnHandler msg.what(" + msg.what + "):" + getMessName(msg.what));
                    if (hasReConnectionDevice()) {
                        startSeek();
                    }
                    break;
                }
                case MESSAGE_ADAPTER_STATE_TURNED_OFF: {
                    Log.d(TAG, "ConnHandler msg.what(" + msg.what + "):" + getMessName(msg.what));
                    cleanup();
                    break;
                }
                case MESSAGE_NOTE_CONNECT: {
                    ConnectionDevice connDevice = (ConnectionDevice) msg.obj;
                    Log.d(TAG, "ConnHandler msg.what(" + msg.what + "):" + getMessName(msg.what));
                    Log.d(TAG, "address=" + PubTools.macPrint(connDevice.getAddress()) + ", profileType="
                            + connDevice.getProfileType());
                    connectDevice(connDevice);
                    break;
                }
                case MESSAGE_NOTE_DISCONNECT: {
                    ConnectionDevice connDevice = (ConnectionDevice) msg.obj;
                    Log.d(TAG, "ConnHandler msg.what(" + msg.what + "):" + getMessName(msg.what));
                    Log.d(TAG, "address=" + PubTools.macPrint(connDevice.getAddress()) + ", profileType="
                            + connDevice.getProfileType());
                    disConnectDevice(connDevice);
                    break;
                }
                case MESSAGE_ACTION_DETECTED: {
                    NearlinkDevice device = (NearlinkDevice) msg.obj;
                    noticeProfileDevices(device);
                    break;
                }
                case MESSAGE_SAVE_DEVICE_INFO: {
                    ConnectionDevice connDevice = (ConnectionDevice) msg.obj;
                    Log.d(TAG, connDevice.toString());
                    updateConnDeviceInfo(connDevice);
                    break;
                }
                default:
                    Log.d(TAG, "ConnHandler msg.what(" + msg.what + "):" + getMessName(msg.what));
                    break;
            }
        }
    }

    /**
     * 简单连接队列
     *
     * @param address     设备地址信息
     * @param profileType profileType
     * @return 新加入队列是否成功，不成功说明已经有连接或断连在队列中
     */
    private boolean addConnQueue(NearlinkAddress address, int profileType) {
        String key = "KEY:" + address.getAddress() + ":" + profileType;
        if (connQueue.containsKey(key)) {
            if ((System.currentTimeMillis() - connQueue.get(key)) > CONN_TIMEOUT) {
                connQueue.put(key, System.currentTimeMillis());
                return true;
            } else {
                return false;
            }
        } else {
            connQueue.put(key, System.currentTimeMillis());
            return true;
        }
    }

    /**
     * 保存连接信息
     *
     * @param device 设备信息
     * @param intent 广播Intent
     */
    public void saveConnectDeviceInfo(NearlinkDevice device, Intent intent) {
        if (device != null) {
            int newState = intent.getIntExtra(NearlinkAdapter.EXTRA_CONNECTION_STATE, NearlinkConstant.SLE_ACB_STATE_NONE);
            int preState = intent.getIntExtra(NearlinkAdapter.EXTRA_PREVIOUS_CONNECTION_STATE, NearlinkConstant.SLE_ACB_STATE_NONE);
            int pairState = intent.getIntExtra(NearlinkAdapter.EXTRA_PAIR_STATE, NearlinkConstant.SLE_PAIR_NONE);
            int discReason = intent.getIntExtra(NearlinkAdapter.EXTRA_DISC_REASON, NearlinkConstant.SLE_DISCONNECT_BY_NONE);
            int profile = intent.getIntExtra(NearlinkAdapter.EXTRA_PROFILE_TYPE, -1);
            NearlinkAddress nearlinkAddress = device.getNearlinkAddress();
            ConnectionDevice connDevice = new ConnectionDevice(nearlinkAddress, newState, pairState,
                    profile, discReason);
            // 本地记录连接信息(只在连接和断联时保存)
            if (newState == NearlinkConstant.SLE_ACB_STATE_CONNECTED || newState == NearlinkConstant.SLE_ACB_STATE_DISCONNECTED) {
                mHandler.obtainMessage(MESSAGE_SAVE_DEVICE_INFO, connDevice).sendToTarget();
            }

            if (preState == NearlinkConstant.SLE_ACB_STATE_CONNECTED && newState == NearlinkConstant.SLE_ACB_STATE_DISCONNECTED) {
                mHandler.obtainMessage(MESSAGE_ACB_DISCONNECTED_BY_REMOTE).sendToTarget();
            }
        }
    }

    /**
     * 是否自动连接
     *
     * @param isAutoConnect 是否自动连接
     */
    public void autoConnect(boolean isAutoConnect) {
        this.isAutoConnect = isAutoConnect;
    }

    /**
     * 恢复断连设备
     */
    public void restoreConnectedDeviceProfiles() {
        autoConnect(true);
    }

    /**
     * 连接所有profile
     *
     * @param nearlinkAddress 设备地址
     * @return 是否执行成功
     */
    protected boolean connectAllProfile(NearlinkAddress nearlinkAddress) {
        Class<?>[] clsA = Config.getSupportedProfiles();
        if (clsA.length <= 0) {
            Log.d(TAG, "no profile support.");
            return false;
        }
        for (Class<?> cla : clsA) {
            ConnectionDevice connectionDevice = new ConnectionDevice();
            connectionDevice.setNearlinkAddress(nearlinkAddress);
            if (HidHostService.class.equals(cla)) {
                Log.d(TAG, "connectAllProfile() connect hidHostService");
                connectionDevice.setProfileType(NearlinkProfile.HID_HOST);
                mHandler.obtainMessage(MESSAGE_NOTE_CONNECT, connectionDevice).sendToTarget();
            } else {
                Log.d(TAG, "connectAllProfile() what class?");
            }
        }
        return true;
    }

    /**
     * 断连所有profile
     *
     * @param nearlinkAddress 设备地址
     * @return 是否执行成功
     */
    protected boolean disConnectAllProfile(NearlinkAddress nearlinkAddress) {
        Class<?>[] clsA = Config.getSupportedProfiles();
        if (clsA.length <= 0) {
            Log.d(TAG, "no profile support.");
            return false;
        }
        for (Class<?> cla : clsA) {
            ConnectionDevice connectionDevice = new ConnectionDevice();
            connectionDevice.setNearlinkAddress(nearlinkAddress);
            if (HidHostService.class.equals(cla)) {
                Log.d(TAG, "disConnectAllProfile() disConnect hidHostService");
                connectionDevice.setProfileType(NearlinkProfile.HID_HOST);
                mHandler.obtainMessage(MESSAGE_NOTE_DISCONNECT, connectionDevice).sendToTarget();
            } else {
                Log.d(TAG, "disConnectAllProfile() what class?");
            }
        }
        return true;
    }

    /**
     * 删除含address的缓存
     *
     * @param address 设备地址
     */
    protected void remove(NearlinkAddress address) {
        synchronized (obj) {
            connMap.values().removeIf(device -> device.getAddress().equals(address.getAddress()));
            writeToDisk(connMap);

            // sendBroadcast ACTION_DISAPPEARED
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent(NearlinkDevice.ACTION_DISAPPEARED);
                    intent.putExtra(NearlinkDevice.EXTRA_DEVICE, NearlinkAdapter.getDefaultAdapter().getRemoteDevice(address));
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                    mService.sendBroadcast(intent, AdapterService.NEARLINK_PERM);
                    Log.e(TAG, "remove sendBroadcast ACTION_DISAPPEARED  device:" + address.toString());
                }
            }, 2000);
        }
    }

    /**
     * 删除所有缓存
     */
    protected void removeAll() {
        synchronized (obj) {
            Collection<ConnectionDevice> connectionDevices = connMap.values();
            connMap.clear();
            connQueue.clear();
            writeToDisk(connMap);

            // sendBroadcast ACTION_DISAPPEARED
            Map<String, NearlinkDevice> deviceMap = new HashMap<>();
            for (ConnectionDevice device : connectionDevices) {
                deviceMap.put(device.getKey(), device.getNearlinkDevice());
            }
            for (NearlinkDevice device : deviceMap.values()) {
                Intent intent = new Intent(NearlinkDevice.ACTION_DISAPPEARED);
                intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
                intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                mService.sendBroadcast(intent, AdapterService.NEARLINK_PERM);
                Log.e(TAG, "remove sendBroadcast ACTION_DISAPPEARED  device:" + device.getNearlinkAddress().toString());
            }
        }
    }

    /**
     * 物理连接获取连接状态
     *
     * @param address 连接地址
     * @return 连接状态
     */
    protected int getConnectionState(String address) {
        synchronized (obj) {
            int connState = NearlinkConstant.SLE_ACB_STATE_NONE;
            for (ConnectionDevice device : connMap.values()) {
                if (device.getAddress().equals(address)) {
                    if (device.isConnected()) {
                        connState = NearlinkConstant.SLE_ACB_STATE_CONNECTED;
                        break;
                    } else {
                        connState = device.getConnState();
                    }
                }
            }

            return connState;
        }
    }

    /**
     * 获取连接过的设备状态
     *
     * @param nearlinkAddress 连接地址
     * @return 连接过的设备状态
     */
    public int getPairedState(NearlinkAddress nearlinkAddress) {
        synchronized (obj) {
            int pairedState = NearlinkConstant.SLE_PAIR_NONE;
            for (ConnectionDevice device : connMap.values()) {
                if (device.getHasConnected() == HAS_CONNECTED && device.getAddress().equals(nearlinkAddress.getAddress())) {
                    pairedState = NearlinkConstant.SLE_PAIR_PAIRED;
                    break;
                }
            }
            return pairedState;
        }
    }

    /**
     * 获取配对数量
     *
     * @return 配对数量
     */
    protected int getPairedDevicesNum() {
        synchronized (obj) {
            Map<String, String> map = new HashMap<>();
            for (ConnectionDevice device : connMap.values()) {
                if (device.getHasConnected() == HAS_CONNECTED) {
                    map.put(device.getAddress(), "");
                }
            }
            return map.keySet().size();
        }
    }

    /**
     * 获取配对设备列表
     *
     * @return 配对设备列表
     */
    protected List<NearlinkAddress> getPairedDevices() {
        synchronized (obj) {
            Map<String, NearlinkAddress> map = new HashMap<>();
            for (ConnectionDevice device : connMap.values()) {
                if (device.getHasConnected() == HAS_CONNECTED) {
                    map.put(device.getAddress(), device.getNearlinkAddress());
                }
            }
            List<NearlinkAddress> list = new ArrayList<>(map.values());
            Log.d(TAG, "getPairedDevices.size=" + list.size());
            return list;
        }
    }

    /**
     * 判断是否有设备需要回连
     * 回连条件：
     * 1、缓存列表不为空
     * 2、成功连接过
     * 3、当前非已连接状态
     * 4、断联原因非本端断联(0x11)(已废弃---2024.03.25)
     *
     * @return true:有需要回连的设备
     * false：没有需要回连的设备
     */
    private boolean hasReConnectionDevice() {
        synchronized (obj) {
            for (ConnectionDevice device : connMap.values()) {
                if (device.getHasConnected() == HAS_CONNECTED && !device.isConnected()) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * 获取主设备连接状态
     *
     * @return 设备连接状态
     */
    protected int getAdapterConnectionState() {
        synchronized (obj) {
            for (ConnectionDevice device : connMap.values()) {
                if (device.isConnected()) {
                    return NearlinkConstant.SLE_ACB_STATE_CONNECTED;
                }
            }
            return NearlinkConstant.SLE_ACB_STATE_NONE;
        }
    }

    private void removeConnQueue(NearlinkAddress nearlinkAddress, int profileType) {
        String key = "KEY:" + nearlinkAddress.getAddress() + ":" + profileType;
        connQueue.remove(key);
    }

    private void startSeek() {
        try {
            Log.i(TAG, "startSeek...");
            DiscoveryService.getDiscoveryService().startSeekForIntent();
        } catch (Exception e) {
            Log.i(TAG, "startSeek...:e=", e);
        }
    }

    private void stopSeek() {
        try {
            Log.i(TAG, "stopSeek...");
            DiscoveryService.getDiscoveryService().stopSeekForIntent();
        } catch (Exception e) {
            Log.i(TAG, "stopSeek...:e=", e);
        }
    }

    /**
     * 收到回连广播，判断设备是否可以触发连接
     * 1.当前设备在缓存列表中
     * 2.设备成功连接过
     * 3.设备当前为非连接状态
     * 4.断联原因非0x11(已废弃---2024.03.25)
     *
     * @param nearlinkDevice
     */
    private void noticeProfileDevices(NearlinkDevice nearlinkDevice) {
        synchronized (obj) {
            for (ConnectionDevice device : connMap.values()) {
                if (device.getAddress().equals(nearlinkDevice.getAddress())
                        && device.getHasConnected() == HAS_CONNECTED
                        && !device.isConnected()) {
                    mHandler.obtainMessage(MESSAGE_NOTE_CONNECT, device).sendToTarget();
                }
            }
        }
    }

    private void cleanup() {
        Log.i(TAG, "cleanup().");
        if (mService != null) {
            mService.unregisterReceiver(mReceiver);
        }
        connMap.clear();
        connQueue.clear();
        stopSeek();
        instance = null;
    }

    private void registerReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_STATE_CHANGED);
        filter.addAction(NearlinkDevice.ACTION_DETECTED);
        mService.registerReceiver(mReceiver, filter);
    }

    private void updateConnDeviceInfo(ConnectionDevice device) {
        synchronized (obj) {
            if (isConnStateChange(device)) {
                if (device.getConnState() == NearlinkConstant.SLE_ACB_STATE_CONNECTED) {
                    device.setHasConnected(HAS_CONNECTED);
                } else {
                    ConnectionDevice preConnDevice = connMap.get(device.getKey());
                    if (preConnDevice != null) {
                        device.setHasConnected(preConnDevice.getHasConnected());
                    }
                }
                connMap.put(device.getKey(), device);
                writeToDisk(connMap);
            }
            if (device.getConnState() == NearlinkConstant.SLE_ACB_STATE_CONNECTED
                    || device.getConnState() == NearlinkConstant.SLE_ACB_STATE_DISCONNECTED) {
                removeConnQueue(device.getNearlinkAddress(),
                        device.getProfileType());
            }
            if (device.getConnState() == NearlinkConstant.SLE_ACB_STATE_CONNECTED
                    && !hasReConnectionDevice()) {
                stopSeek();
            }
        }
    }

    private boolean isConnStateChange(ConnectionDevice device) {
        ConnectionDevice preConnDevice = connMap.get(device.getKey());
        if (preConnDevice == null) {
            return true;
        } else {
            return preConnDevice.getConnState() != device.getConnState();
        }
    }

    private void connectDevice(ConnectionDevice device) {
        if (device == null) {
            Log.d(TAG, "ConnectionDevice is null.");
            return;
        }
        switch (device.getProfileType()) {
            case NearlinkProfile.HID_HOST:
                Log.d(TAG, "connect HID_HOST.->" + PubTools.macPrint(device.getAddress()));
                // connect hid
                HidHostService hidHostService = serviceFactory.getHidHostService();
                if (hidHostService == null) {
                    Log.d(TAG, "HidHostService service not available");
                } else {
                    if (addConnQueue(device.getNearlinkAddress(), NearlinkProfile.HID_HOST)) {
                        hidHostService.connect(device.getNearlinkDevice());
                    }
                }
                break;
            // 其他profile 依次添加
            case NearlinkProfile.HID_DEVICE:
                Log.d(TAG, "connect HID_DEVICE.");
                // connect XXX
                break;
            default:
                break;
        }
    }

    private void disConnectDevice(ConnectionDevice device) {
        if (device == null) {
            Log.d(TAG, "ConnectionDevice is null.");
            return;
        }
        switch (device.getProfileType()) {
            case NearlinkProfile.HID_HOST:
                Log.d(TAG, "disConnect HID_HOST.->" + PubTools.macPrint(device.getAddress()));
                // connect hid
                HidHostService hidHostService = serviceFactory.getHidHostService();
                if (hidHostService == null) {
                    Log.d(TAG, "HidHostService service not available");
                } else {
                    if (addConnQueue(device.getNearlinkAddress(), NearlinkProfile.HID_HOST)) {
                        hidHostService.disconnect(device.getNearlinkDevice());
                    }
                }
                break;
            // 其他profile 依次添加
            case NearlinkProfile.HID_DEVICE:
                Log.d(TAG, "disConnect HID_DEVICE.");
                // connect XXX
                break;
            default:
                break;
        }
    }

    private Map<String, ConnectionDevice> readFromDisk() {
        List<String> fileDataList = PubTools.loadConnDataFromFile(PubTools.CONN_DATA_FILE_V2, false);
        Log.d(TAG, "readFromDisk(): get v2 file result size = " + fileDataList.size());
        if (fileDataList.isEmpty()) {
            fileDataList = PubTools.loadConnDataFromFile(PubTools.CONN_DATA_FILE, true);
            Log.d(TAG, "readFromDisk(): v2 file isEmpty, get old file result size = " + fileDataList.size());
        }
        Map<String, ConnectionDevice> connCacheMap = new HashMap<>();

        for (String deviceCacheInfo : fileDataList) {
            ConnectionDevice connDevice = ConnectionDevice.initFromCacheString(deviceCacheInfo);
            if (connDevice != null
                    && !connDevice.getAddress().isEmpty()
                    && connDevice.getHasConnected() == HAS_CONNECTED) {
                // 初始化状态重置为 断连
                connDevice.setConnState(NearlinkConstant.SLE_ACB_STATE_DISCONNECTED);
                connCacheMap.put(connDevice.getKey(), connDevice);
            }
        }
        Log.d(TAG, "readFromDisk: cache data = " + connCacheMap);
        return connCacheMap;
    }

    private void writeToDisk(Map<String, ConnectionDevice> connMap) {
        if (connMap != null) {
            List<String> inputList = new ArrayList<>();
            if (connMap.isEmpty()) {
                PubTools.saveConnDataToFile(inputList);
                return;
            }
            for (ConnectionDevice device : connMap.values()) {
                if (device.getAddress() != null && !device.getAddress().isEmpty()) {
                    String line = device.toCacheString();
                    inputList.add(line);
                }
            }
            PubTools.saveConnDataToFile(inputList);
            Log.d(TAG, "writeToDisk: data size = " + inputList.size());
        }
    }

    private String getMessName(int msgWhat) {
        switch (msgWhat) {
            case MESSAGE_ACB_DISCONNECTED_BY_REMOTE:
                return "MESSAGE_ACB_DISCONNECTED_BY_REMOTE";
            case MESSAGE_ADAPTER_STATE_TURNED_ON:
                return "MESSAGE_ADAPTER_STATE_TURNED_ON";
            case MESSAGE_ADAPTER_STATE_TURNED_OFF:
                return "MESSAGE_ADAPTER_STATE_TURNED_OFF";
            case MESSAGE_NOTE_CONNECT:
                return "MESSAGE_NOTE_CONNECT";
            case MESSAGE_NOTE_DISCONNECT:
                return "MESSAGE_NOTE_DISCONNECT";
            case MESSAGE_ACTION_DETECTED:
                return "MESSAGE_ACTION_DETECTED";
            case MESSAGE_SAVE_DEVICE_INFO:
                return "MESSAGE_SAVE_DEVICE_INFO";
            default:
                return "Unknown Message Name";
        }
    }
}
