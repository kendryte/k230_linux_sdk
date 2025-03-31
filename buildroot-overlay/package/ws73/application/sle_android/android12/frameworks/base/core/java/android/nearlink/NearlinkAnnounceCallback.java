/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * 广播回调
 *
 * @since 2023-12-04
 */
public class NearlinkAnnounceCallback {
    /**
     * 启动广播成功回调
     */
    public void onStartedSuccess() {}

    /**
     * 启动广播失败回调
     *
     * @param errorCode 成功 0 失败 其他
     */
    public void onStartedFailure(int errorCode) {}

    /**
     * 停止广播成功回调
     */
    public void onStoppedSuccess() {}

    /**
     * 停止广播失败回调
     *
     * @param errorCode 成功 0 失败 其他
     */
    public void onStoppedFailure(int errorCode) {}

    /**
     * 底层协议栈链接后，广播停止回调
     */
    public void onTerminaled() {}
}
