/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.nlservice;

/**
 * 原始地址工具类
 *
 * @since 2023-12-04
 */

public class ProtoDeviceAddrUtils {

    private static final String TAG = "ProtoDeviceAddrUtils";

    public static  String byte2Str(byte[] addr) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < addr.length; i++) {
            sb.append(":");
            byte b = addr[i];
            byte b0 = (byte) (b & 0xFF);
            String subStr = Integer.toHexString(b0 & 0xFF).toUpperCase();
            if (subStr.length() == 1) {
                sb.append('0');
            }
            sb.append(subStr);
        }
        return sb.substring(1);
    }

}
