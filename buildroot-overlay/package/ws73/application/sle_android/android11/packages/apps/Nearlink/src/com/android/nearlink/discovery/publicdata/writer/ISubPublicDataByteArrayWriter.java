/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.writer;

/**
 * 公开数子结构数据写入
 *
 * @since 2023-12-04
 */
public interface ISubPublicDataByteArrayWriter {

    byte[] writeValue(int type, byte[] value);

}
