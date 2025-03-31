/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.read;

/**
 * 广播结果回写回调
 *
 * @since 2023-12-04
 */
public interface IPublicDataReadCallback {

    void setDeviceLocalName(String name);

    void setAppearance(int appearance);

}
