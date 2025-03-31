/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * 工具类
 *
 * @since 2023-12-01
 */
public class PubTools {
    private static final String TAG = "[SLE_CONN_PACKAGE]--PubTools";

    public static final String CONN_DATA_FILE = "/data/misc/nearlink/conn_data";
    public static final String CONN_DATA_FILE_V2 = "/data/misc/nearlink/conn_data_v2";

    /**
     * mac地址 字节转字符串
     *
     * @param bytes mac地址byte[]
     * @return mac地址字符串 格式00:00:00:00:00:00
     */
    public static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte aByte : bytes) {
            sb.append(":");
            String hex = Integer.toHexString(aByte & 0xFF).toUpperCase();
            if (hex.length() < 2) {
                sb.append(0);
            }
            sb.append(hex);
        }
        return sb.substring(1);
    }

    /**
     * mac地址 字符串转字节
     *
     * @param hex 地址字符串 格式00:00:00:00:00:00
     * @return mac地址byte[]
     */
    public static byte[] hexToByteArray(String hex) {
        String[] address = hex.split(":");
        if (address.length != 6) {
            throw new IllegalArgumentException("Invalid mac address");
        }
        byte[] byteAddress = new byte[6];
        for (int i = 0; i < byteAddress.length; i++) {
            byteAddress[i] = (byte) Short.parseShort(address[i], 0x10);
        }
        return byteAddress;
    }

    private static byte hexToByte(String inHex) {
        return (byte) Integer.parseInt(inHex, 16);
    }

    /**
     * Mac地址脱敏，打印用
     *
     * @param mac mac地址
     * @return Mac地址脱敏
     */
    public static String macPrint(String mac) {
        if (mac == null) {
            return "";
        }
        String[] temp = mac.split(":");

        return temp[0] + ":*****:" + temp[temp.length - 1];
    }

    /**
     * 加载磁盘文件内容
     *
     * @return 磁盘文件内容
     */
    protected static List<String> loadConnDataFromFile(String filePath, boolean needXcode) {
        FileInputStream fis = null;
        InputStreamReader isr = null;
        BufferedReader br = null;
        List<String> list = new ArrayList<>();
        try {
            File file = new File(filePath);
            if (!file.exists()) {
                // 文件不存在则从bak文件恢复
                File bakFile = new File(filePath + ".bak");
                if (!bakFile.exists()) {
                    Log.i(TAG, "oriFile and bakFile both not exist , return empty");
                    return list;
                } else {
                    Log.i(TAG, "read from bakFile(" + bakFile.getPath() + ")");
                    fis = new FileInputStream(bakFile);
                }
            } else {
                Log.i(TAG, "read from oriFile");
                fis = new FileInputStream(file);
            }
            isr = new InputStreamReader(fis, StandardCharsets.UTF_8);
            br = new BufferedReader(isr);
            StringBuilder sb = new StringBuilder();
            int length;
            char[] chars = new char[ConnectionDevice.DATA_LEN];
            while ((length = br.read(chars)) != -1) {
                sb.append(String.valueOf(chars, 0, length));
            }
            String fileReadResult;
            if (needXcode) { // 如果是旧文件，需要用xcode来解
                fileReadResult = xcode(sb.toString());
            } else {
                fileReadResult = sb.toString();
            }
            if (!fileReadResult.isEmpty() && fileReadResult.length() % ConnectionDevice.DATA_LEN == 0) {
                int count = fileReadResult.length() / ConnectionDevice.DATA_LEN;
                for (int i = 0; i < count; i++) {
                    int beginIndex = i * ConnectionDevice.DATA_LEN;
                    int endIndex = (i + 1) * ConnectionDevice.DATA_LEN;
                    String obj = fileReadResult.substring(beginIndex, endIndex);
                    list.add(obj);
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "IOException", e);
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
                if (isr != null) {
                    isr.close();
                }
                if (fis != null) {
                    fis.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "IOException", e);
            }
        }
        return list;
    }

    /**
     * 保存内容到磁盘文件
     *
     * @param inputList 需要保存内容
     */
    protected static void saveConnDataToFile(List<String> inputList) {
        FileOutputStream fos = null;
        BufferedOutputStream bos = null;
        try {
            File oriFile = new File(CONN_DATA_FILE_V2);
            File bakFile = new File(CONN_DATA_FILE_V2 + ".bak");
            if (oriFile.exists()) {
                if (bakFile.exists()) {
                    bakFile.delete();
                }
                // 先备份已有文件
                boolean renamed = oriFile.renameTo(bakFile);
                Log.i(TAG, "oriFile(" + oriFile.getPath() + ") rename result = " + renamed);
            }
            // 写新文件
            File tempFile = new File(CONN_DATA_FILE_V2 + ".new");
            if (!tempFile.exists()) {
                boolean isCreate = tempFile.createNewFile();
                if (!isCreate) {
                    return;
                }
                Log.i(TAG, "tempFile(" + tempFile.getPath() + ") create success");
            }
            fos = new FileOutputStream(tempFile);
            bos = new BufferedOutputStream(fos);
            StringBuilder builder = new StringBuilder(ConnectionDevice.DATA_LEN);
            for (String line : inputList) {
                builder.append(line);
            }
            bos.write(builder.toString().getBytes(StandardCharsets.UTF_8));
            bos.flush();
            fos.getFD().sync();
            // 临时文件重命名为新文件
            if (tempFile.renameTo(oriFile)) {
                if (bakFile.delete()) {
                    Log.i(TAG, "bakFile(" + bakFile.getPath() + ") delete success");
                }
            } else {
                Log.i(TAG, "final rename tempFile to oriFile error: " + tempFile);
            }
        } catch (IOException e) {
            Log.e(TAG, "IOException", e);
        } finally {
            try {
                if (bos != null) {
                    bos.close();
                }
                if (fos != null) {
                    fos.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "IOException", e);
            }
        }
    }

    protected static String xcode(String obj) {
        char[] chars = obj.toCharArray();
        int len = chars.length;
        for (int i = 0; i < len; i++) {
            chars[i] = (char) (chars[i] ^ len);
        }
        return new String(chars);
    }
}
