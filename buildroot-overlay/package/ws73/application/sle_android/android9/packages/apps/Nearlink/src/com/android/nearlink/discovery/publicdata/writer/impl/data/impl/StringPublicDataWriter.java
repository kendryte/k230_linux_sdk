/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.writer.impl.data.impl;

import com.android.nearlink.discovery.publicdata.writer.impl.data.AbstractStandPublicDataWriter;

import java.nio.charset.StandardCharsets;

/**
 * String类型数据写入器
 *
 * @since 2023-12-04
 */
public class StringPublicDataWriter extends AbstractStandPublicDataWriter<String> {

    @Override
    protected byte[] getValueBytes(String value) {
        return value.getBytes(StandardCharsets.UTF_8);
    }
}
