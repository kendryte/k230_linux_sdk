/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.writer.impl.array;

import com.android.nearlink.discovery.publicdata.writer.ISubPublicDataByteArrayWriter;

/**
 * 标准模式数据写入器
 *
 * @since 2023-12-04
 */
public class StandSubPublicDataByteArrayWriter implements ISubPublicDataByteArrayWriter {

    static final int OFF_SET = 2;

    @Override
    public byte[] writeValue(int type, byte[] value) {

        byte[] res = new byte[value.length + OFF_SET];
        // 第一个字节是类型
        res[0] = (byte)type;
        // 第二个字节是纯内容部分的长度
        res[1] = (byte)value.length;
        System.arraycopy(value, 0, res, OFF_SET, value.length);

        return res;
    }
}
