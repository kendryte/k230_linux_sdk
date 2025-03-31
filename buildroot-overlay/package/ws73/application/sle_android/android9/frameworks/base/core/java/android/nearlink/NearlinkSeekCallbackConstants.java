/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * 扫描回调常量
 *
 * @since 2023-12-04
 */

public interface NearlinkSeekCallbackConstants {
    /**
     * 成功
     */
    int SEEK_SUCCESS = 0;

    /**
     * 启动中
     */
    int SEEK_STARTING_BUSY = 1;

    /**
     * 关闭中
     */
    int SEEK_STOPPING_BUSY = 2;

    /**
     * seek内部错误
     */
    int SEEK_FAILED_INTERNAL_ERROR = 3;

    /**
     * seek已启动
     */
    int SEEK_FAILED_ALREADY_STARTED = 4;


    /**
     * 扫描过于频繁
     */
    int SEEK_FAILED_SCANNING_TOO_FREQUENTLY = 6;


}
