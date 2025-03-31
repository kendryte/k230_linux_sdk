/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.writer.impl.data;

import com.android.nearlink.discovery.publicdata.writer.impl.array.StandSubPublicDataByteArrayWriter;

/**
 * 标准协议公开数据写入
 *
 * @since 2023-12-04
 */
public abstract class AbstractStandPublicDataWriter<T> extends AbstractPublicDataWriter<T> {

    public AbstractStandPublicDataWriter() {
        super.iSubPublicDataByteArrayWriter = new StandSubPublicDataByteArrayWriter();
    }
}
