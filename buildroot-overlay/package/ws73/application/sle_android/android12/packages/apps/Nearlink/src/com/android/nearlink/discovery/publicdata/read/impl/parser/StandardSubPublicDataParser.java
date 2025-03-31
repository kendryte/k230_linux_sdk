/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.read.impl.parser;

import android.util.Log;

import com.android.nearlink.discovery.publicdata.read.ParsedSubPublicData;
import com.android.nearlink.discovery.publicdata.read.ISubPublicDataReadParser;

import java.io.ByteArrayInputStream;

/**
 * 标准协议模式解析
 *
 * @since 2023-12-04
 */
public class StandardSubPublicDataParser implements ISubPublicDataReadParser {

    private static final String TAG = "StandardSubPublicDataParser";

    private int SUB_PUBLIC_DATA_MIN_LEN = 1;

    @Override
    public ParsedSubPublicData parseOne(ByteArrayInputStream byteArrayInputStream) {

        /**
         * 标准协议，第一个byte是type,第二个byte是len，len仅仅包含数据部分
         */

        int subDataType = byteArrayInputStream.read();
        int subDataLength = byteArrayInputStream.read();

        if(subDataLength < SUB_PUBLIC_DATA_MIN_LEN){
            Log.e(TAG, "subDataLength is " + subDataLength + " < " + SUB_PUBLIC_DATA_MIN_LEN +" error!!!");
            return null;
        }

        byte[] subByteData = new byte[subDataLength];
        byteArrayInputStream.read(subByteData,0, subDataLength);
        return new ParsedSubPublicData(subDataType, subByteData);
    }
}
