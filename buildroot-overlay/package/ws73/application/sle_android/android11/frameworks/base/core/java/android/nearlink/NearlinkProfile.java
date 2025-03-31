/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.Manifest;
import android.annotation.RequiresPermission;
import android.annotation.SystemApi;

import java.util.List;

/**
 * SLE Profile的公共接口
 *
 * <p> 每一个Profile都需要实现该接口
 *
 * @since 2023-12-04
 */
public interface NearlinkProfile {
    /**
     * Extra for the connection state intents of the individual profiles.
     * <p>
     * This extra represents the current connection state of the profile of the
     * Nearlink device.
     */
    String EXTRA_STATE = "android.nearlink.profile.extra.STATE";

    /**
     * Extra for the connection state intents of the individual profiles.
     * <p>
     * This extra represents the previous connection state of the profile of the
     * Nearlink device.
     */
    String EXTRA_PREVIOUS_STATE =
            "android.nearlink.profile.extra.PREVIOUS_STATE";

    /**
     * Extra for the connection pair state intents of the individual profiles.
     * <p>
     * This extra represents the current connection pair state of the profile of the
     * Nearlink device.
     */
    String EXTRA_PAIR_STATE = "android.nearlink.profile.extra.PAIR_STATE";

    /**
     * Extra for the disconnection reason of the profiles.
     */
    String EXTRA_DISC_REASON = "android.nearlink.profile.extra.DISC_REASON";

    /**
     * The profile is in disconnected state
     */
    int STATE_DISCONNECTED = NearlinkAdapter.STATE_DISCONNECTED;

    /**
     * The profile is in connecting state
     */
    int STATE_CONNECTING = NearlinkAdapter.STATE_CONNECTING;

    /**
     * The profile is in connected state
     */
    int STATE_CONNECTED = NearlinkAdapter.STATE_CONNECTED;

    /**
     * The profile is in disconnecting state
     */
    int STATE_DISCONNECTING = NearlinkAdapter.STATE_DISCONNECTING;

    /**
     * SSAP client profile
     */
    int SSAP_CLIENT = 1;

    /**
     * SSAP server profile
     */
    int SSAP_SERVER = 2;

    /**
     * HID Host
     *
     * @hide
     */
    int HID_HOST = 3;

    /**
     * HID Device
     */
    int HID_DEVICE = 4;

    /**
     * Max profile ID. This value should be updated whenever a new profile is added to match
     * the largest value assigned to a profile.
     *
     * @hide
     */
    int MAX_PROFILE_ID = 5;

    /**
     * Default priority for devices that we try to auto-connect to and
     * and allow incoming connections for the profile
     *
     * @hide
     **/
    int PRIORITY_AUTO_CONNECT = 1000;

    /**
     * Default priority for devices that allow incoming
     * and outgoing connections for the profile
     *
     * @hide
     **/
    @SystemApi
    int PRIORITY_ON = 100;

    /**
     * Default priority for devices that does not allow incoming
     * connections and outgoing connections for the profile.
     *
     * @hide
     **/
    @SystemApi
    int PRIORITY_OFF = 0;

    /**
     * Default priority when not set or when the device is unpaired
     *
     * @hide
     */
    int PRIORITY_UNDEFINED = -1;

    /**
     * 获取当前Profile已连接的对端设备
     *
     * @return 对端设备列表，如果出错的话将返回空列表
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    List<NearlinkDevice> getConnectedDevices();

    /**
     * 获取当前Profile具备指定连接状态的对端设备
     *
     * @param states 状态数组，状态必须是 {@link #STATE_CONNECTED}, {@link #STATE_CONNECTING},
     *               {@link #STATE_DISCONNECTED}, {@link #STATE_DISCONNECTING}中的一个
     * @return 对端设备列表，如果出错的话将返回空列表
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    List<NearlinkDevice> getDevicesMatchingConnectionStates(int[] states);

    /**
     * 获取指定对端设备在当前Profile的连接状态
     *
     * @param device 对端星闪设备
     * @return profile的连接状态， 结果是{@link #STATE_CONNECTED}, {@link #STATE_CONNECTING},
     * {@link #STATE_DISCONNECTED}, {@link #STATE_DISCONNECTING}其中之一
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    int getConnectionState(NearlinkDevice device);

    /**
     * 服务连接监听
     */
    interface ServiceListener {
        /**
         * 通知client已连接上server
         *
         * @param profile {@link #HID_HOST}, {@link #HID_DEVICE} 等profile的定义值
         * @param proxy   具体profile的实现对象
         */
        void onServiceConnected(int profile, NearlinkProfile proxy);

        /**
         * 通知client已断开与server的连接
         *
         * @param profile {@link #HID_HOST}, {@link #HID_DEVICE} 等profile的定义值
         */
        void onServiceDisconnected(int profile);
    }

    /**
     * 状态值转字符串显示
     *
     * @hide
     */
    static String getConnectionStateName(int connectionState) {
        switch (connectionState) {
            case STATE_DISCONNECTED:
                return "STATE_DISCONNECTED";
            case STATE_CONNECTING:
                return "STATE_CONNECTING";
            case STATE_CONNECTED:
                return "STATE_CONNECTED";
            case STATE_DISCONNECTING:
                return "STATE_DISCONNECTING";
            default:
                return "STATE_UNKNOWN";
        }
    }
}
