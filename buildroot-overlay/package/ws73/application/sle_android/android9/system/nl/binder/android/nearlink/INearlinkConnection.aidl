/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.nearlink.INearlinkConnectionCallback;
import android.nearlink.NearlinkAddress;

/**
 * 连接管理
 *
 * @since 2023-12-01
 */
interface INearlinkConnection {
    // 发送连接请求
    int connectRemoteDevice(in NearlinkAddress nearlinkAddress);

    // 发送断开连接请求
    int disconnectRemoteDevice(in NearlinkAddress nearlinkAddress);

    // 获取配对设备
    List<NearlinkAddress> getPairedDevices();

    // 获取配对设备数量
    int getPairedDevicesNum();

    // 获取连接状态
    int getConnectionState(in NearlinkAddress nearlinkAddress);

    // 获取配对状态
    int getPairState(in NearlinkAddress nearlinkAddress);

    // 删除配对
    int removePairedRemoteDevice(in NearlinkAddress nearlinkAddress);

    // 删除所有配对
    int removeAllPairs();

    // 是否在本地启动绑定
    boolean isPairingInitiatedLocally(in NearlinkAddress nearlinkAddress);

    // 主设备连接状态
    int getAdapterConnectionState();

    // 注册SLE连接管理回调函数
    void connectionRegisterCallbacks(in INearlinkConnectionCallback callback);

    // 注销SLE连接管理回调函数
    void connectionUnRegisterCallbacks(in INearlinkConnectionCallback callback);

    // 检测Sevices是否断开
    boolean check();
}