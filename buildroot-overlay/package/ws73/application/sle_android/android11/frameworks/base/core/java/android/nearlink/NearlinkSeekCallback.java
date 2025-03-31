/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * 扫描回调
 *
 * @since 2023-12-04
 */

public class NearlinkSeekCallback {
    /**
     * seek已启动
     */
    public static final int SEEK_FAILED_ALREADY_STARTED = 1;

    /**
     * seek内部错误
     */
    public static final int SEEK_FAILED_INTERNAL_ERROR = 3;

    /**
     * 扫描过于频繁
     */
    public static final int SEEK_FAILED_SCANNING_TOO_FREQUENTLY = 6;

    /**
     * 没有错误
     */
    public static final int NO_ERROR = 0;

    /**
     * seek启动成功回调
     */
    public void onSeekStartedSuccess() {
    }

    /**
     * seek启动失败回调
     *
     * @param errorCode 错误码
     */
    public void onSeekStartedFailure(int errorCode) {
    }

    /**
     * 停止seek成功回调
     */
    public void onSeekStoppedSuccess() {
    }

    /**
     * 停止seek失败回调
     *
     * @param errorCode 错误码
     */
    public void onSeekStoppedFailure(int errorCode) {
    }

    /**
     * seek上报结果回调
     *
     * @param nearlinkSeekResultInfo 回调信息
     */
    public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
    }
}
