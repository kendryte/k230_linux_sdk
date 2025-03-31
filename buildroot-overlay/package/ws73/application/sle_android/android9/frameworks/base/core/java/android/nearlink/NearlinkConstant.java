/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * SLE常量定义
 *
 * @hide
 * @since 2023-12-01
 */
public class NearlinkConstant {
    /**
     * SLE版本1.0
     */
    public static final int SLE_VERSION = 0x10;

    /**
     * SLE地址长度
     */
    public static final int SLE_ADDR_LEN = 6;

    /**
     * SLE设备名称最大长度
     */
    public static final int SLE_NAME_MAX_LEN = 31;

    /**
     * SLE链路密钥长度
     */
    public static final int SLE_LINK_KEY_LEN = 16;

    // SLE地址类型
    /**
     * 公有地址
     */
    public static final int SLE_ADDRESS_TYPE_PUBLIC = 0;

    /**
     * 随机地址
     */
    public static final int SLE_ADDRESS_TYPE_RANDOM = 0;

    // SLE ACB连接状态sle_acb_state_t
    /**
     * SLE ACB 未连接状态
     */
    public static final int SLE_ACB_STATE_NONE = 0x00;

    /**
     * SLE ACB 已连接
     */
    public static final int SLE_ACB_STATE_CONNECTED = 0x01;

    /**
     * SLE ACB 已断接
     */
    public static final int SLE_ACB_STATE_DISCONNECTED = 0x02;

    /**
     * SLE ACB 连接中-自定义
     */
    public static final int SLE_ACB_STATE_CONNECTING = 0x03;

    /**
     * SLE ACB 断开连接中-自定义
     */
    public static final int SLE_ACB_STATE_DISCONNECTING = 0x04;

    // 星闪配对状态sle_pair_state_t
    /**
     * SLE ACB 未配对状态
     */
    public static final int SLE_PAIR_NONE = 0x01;

    /**
     * SLE ACB 正在配对
     */
    public static final int SLE_PAIR_PAIRING = 0x02;

    /**
     * SLE ACB 已完成配对
     */
    public static final int SLE_PAIR_PAIRED = 0x03;

    /**
     * SLE ACB 已完成配对-自定义
     */
    public static final int SLE_PAIR_REMOVING = 0x04;

    // 星闪断链原因sle_disc_reason_t
    /**
     * SLE ACB 未断链-结构体没有，Service层增加的，方便处理状态
     */
    public static final int SLE_DISCONNECT_BY_NONE = 0x00;

    /**
     * SLE ACB 超时断链
     */
    public static final int SLE_DISCONNECT_BY_TIMEOUT = 0x07;

    /**
     * SLE ACB 远端断链
     */
    public static final int SLE_DISCONNECT_BY_REMOTE = 0x10;

    /**
     * SLE ACB 本端断链
     */
    public static final int SLE_DISCONNECT_BY_LOCAL = 0x11;

    /**
     * SharedPreferences name
     */
    public static final String SHARE_NAME_TAG = "ANDROID_SHARE_TAG";

    /**
     * A bond attempt succeeded
     *
     * @hide
     */
    public static final int PAIR_SUCCESS = 0;

    /**
     * A bond attempt failed because pins did not match, or remote device did
     * not respond to pin request in time
     *
     * @hide
     */
    public static final int UNPAIR_REASON_AUTH_FAILED = 1;

    /**
     * A bond attempt failed because the other side explicitly rejected
     * bonding
     *
     * @hide
     */
    public static final int UNPAIR_REASON_AUTH_REJECTED = 2;

    /**
     * A bond attempt failed because we canceled the bonding process
     *
     * @hide
     */
    public static final int UNPAIR_REASON_AUTH_CANCELED = 3;

    /**
     * A bond attempt failed because we could not contact the remote device
     *
     * @hide
     */
    public static final int UNPAIR_REASON_REMOTE_DEVICE_DOWN = 4;

    /**
     * A bond attempt failed because a discovery is in progress
     *
     * @hide
     */
    public static final int UNPAIR_REASON_DISCOVERY_IN_PROGRESS = 5;

    /**
     * A bond attempt failed because of authentication timeout
     *
     * @hide
     */
    public static final int UNPAIR_REASON_AUTH_TIMEOUT = 6;

    /**
     * A bond attempt failed because of repeated attempts
     *
     * @hide
     */
    public static final int UNPAIR_REASON_REPEATED_ATTEMPTS = 7;

    /**
     * A bond attempt failed because we received an Authentication Cancel
     * by remote end
     *
     * @hide
     */
    public static final int UNPAIR_REASON_REMOTE_AUTH_CANCELED = 8;

    /**
     * An existing bond was explicitly revoked
     *
     * @hide
     */
    public static final int UNPAIR_REASON_REMOVED = 9;
}
