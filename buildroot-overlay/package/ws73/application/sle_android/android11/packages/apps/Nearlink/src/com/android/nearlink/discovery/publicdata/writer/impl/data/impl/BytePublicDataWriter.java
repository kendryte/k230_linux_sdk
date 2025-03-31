/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.writer.impl.data.impl;

import com.android.nearlink.discovery.publicdata.writer.impl.data.AbstractStandPublicDataWriter;

/**
 * Byte类型数据公开数据写入器
 *
 * @since 2023-12-04
 */
public class BytePublicDataWriter extends AbstractStandPublicDataWriter<Byte> {

    @Override
    protected byte[] getValueBytes(Byte value) {
        byte[] bytes = new byte[1];
        bytes[0] = value;
        return bytes;
    }
}
