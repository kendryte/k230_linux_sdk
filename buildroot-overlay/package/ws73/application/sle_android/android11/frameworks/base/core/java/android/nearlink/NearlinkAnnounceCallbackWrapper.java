/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package android.nearlink;

/**
 * 广播回调Wrapper
 *
 * @since 2023-12-04
 */
public class NearlinkAnnounceCallbackWrapper {
    /**
     * 广播id
     */
    private Integer announceId;


    /**
     * 构造器
     */
    public NearlinkAnnounceCallbackWrapper() {

    }
    /**
     * 获取广播id
     *
     * @return 广播id
     */
    public Integer getAnnounceId() {
        return announceId;
    }

    public void setAnnounceId(Integer announceId) {
        this.announceId = announceId;
    }

    /**
     * 广播启动结果回调
     *
     * @param status 成功 0 失败 其他
     */
    public void onAnnounceEnabled(int status) {}

    /**
     * 广播关闭结果回调
     *
     * @param status 成功 0 失败 其他
     */
    public void onAnnounceDisabled(int status) {}

    /**
     * 广播底层断联回调
     */
    public void onAnnounceTerminaled() {}
}
