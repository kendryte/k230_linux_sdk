/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.writer;


/**
 * 解析后的公开数据对象
 *
 * @since 2023-12-04
 */
public class PublicData {

    private int length;
    private byte[] data;

    public PublicData(int length, byte[] data) {
        this.length = length;
        this.data = data;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }

    public int getLength() {
        return length;
    }

    public void setLength(int length) {
        this.length = length;
    }
}
