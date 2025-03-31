/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.writer;

import com.android.nearlink.discovery.publicdata.PublicDataEnums;

/**
 * 公开数据写入
 *
 * @since 2023-12-04
 */
public interface ISubPublicDataWriter<T> {

    byte[] writeValue(T value, PublicDataEnums publicDataEnums);

}
