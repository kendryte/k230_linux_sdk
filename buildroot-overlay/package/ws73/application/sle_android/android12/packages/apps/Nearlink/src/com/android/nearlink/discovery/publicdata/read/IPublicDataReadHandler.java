/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.read;

/**
 * 公开数据读取接口
 *
 * @since 2023-12-04
 */
public interface IPublicDataReadHandler {

    void doHandle(byte[] subByteData, IPublicDataReadCallback iWritePublicDataCallback);

}
