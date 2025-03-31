/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.nearlink.NearlinkAddress;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * 地址列表
 *
 * @since 2023-12-01
 */
public class NearlinkAddressList {
    private static final String TAG = "[SLE_CONN_PACKAGE]--NearlinkAddressList";

    private final List<NearlinkAddress> list = new ArrayList<>();

    public List<NearlinkAddress> getList() {
        return list;
    }

    /**
     * 添加连接地址到List中
     *
     * @param address 连接地址
     * @param type    地址类型
     */
    public void add(byte[] address, byte type) {
        NearlinkAddress nearlinkAddress = new NearlinkAddress(PubTools.bytesToHex(address), type);
        Log.d(TAG, nearlinkAddress.toString());
        list.add(nearlinkAddress);
    }
}
