/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * 广播回调函数常量
 *
 * @since 2023-12-04
 */
public interface NearlinkAnnounceCallbackConstants {
    /**
     * 成功
     */
    int ANNOUNCE_SUCCESS = 0;

    /**
     * 数据过大
     */
    int ANNOUNCE_FAILED_DATA_TOO_LARGE = 1;

    /**
     * 广播者过多
     */
    int ANNOUNCE_FAILED_TOO_MANY_ADVERTISERS = 2;

    /**
     * 广播已经启动
     */
    int ANNOUNCE_FAILED_ALREADY_STARTED = 3;

    /**
     * 广播内部错误
     */
    int ANNOUNCE_FAILED_INTERNAL_ERROR = 4;

    /**
     * 广播特性不支持
     */
    int ANNOUNCE_FAILED_FEATURE_UNSUPPORTED = 5;

    /**
     * 广播路数超过最大值
     */
    int ANNOUNCE_EXCEED_MAX_USING_NUM = 6;

    /**
     * 重复广播
     */
    int ANNOUNCE_CALLBACK_DUPLICATE = 7;

    /**
     * 底层回调异常
     */
    int ANNOUNCE_HARDWARE_CALLBACK_ERROR = 8;
}