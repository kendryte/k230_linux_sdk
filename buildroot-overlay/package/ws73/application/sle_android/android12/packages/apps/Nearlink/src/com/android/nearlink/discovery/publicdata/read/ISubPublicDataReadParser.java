/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.read;

import java.io.ByteArrayInputStream;

/**
 * 子公开数据读取解析接口
 *
 * @since 2023-12-04
 */
public interface ISubPublicDataReadParser {

    ParsedSubPublicData parseOne(ByteArrayInputStream byteArrayInputStream);

}
