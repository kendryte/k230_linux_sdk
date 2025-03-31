/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.writer.impl.data;

import com.android.nearlink.discovery.publicdata.PublicDataEnums;
import com.android.nearlink.discovery.publicdata.writer.ISubPublicDataByteArrayWriter;
import com.android.nearlink.discovery.publicdata.writer.ISubPublicDataWriter;

/**
 * 公开数据写入器
 *
 * @since 2023-12-04
 */
public abstract class AbstractPublicDataWriter<T> implements ISubPublicDataWriter<T> {

    protected ISubPublicDataByteArrayWriter iSubPublicDataByteArrayWriter;

    @Override
    public byte[] writeValue(T value, PublicDataEnums publicDataEnums) {

        byte[] valueBytes =  getValueBytes(value);

        return iSubPublicDataByteArrayWriter.writeValue(publicDataEnums.getTypeVal(),
                valueBytes);

    }


    protected abstract byte[] getValueBytes(T value);
}
