/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.read;

/**
 * 公开数据解析完成数据
 *
 * @since 2023-12-04
 */
public class ParsedSubPublicData {

    int type;

    byte[] data;

    public ParsedSubPublicData(int type, byte[] data) {
        this.type = type;
        this.data = data;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }
}
