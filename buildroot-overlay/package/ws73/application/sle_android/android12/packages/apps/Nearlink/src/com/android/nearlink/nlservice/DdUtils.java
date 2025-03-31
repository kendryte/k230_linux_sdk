/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

public class DdUtils {

    /**
     * 检查字符串是否为空
     * @param val
     * @return
     */
    public static boolean isBlank(String val){

        if(val == null){
            return true;
        }

        if("".equals(val)){
            return true;
        }

        if(val.isEmpty()){
            return true;
        }

        if("".equals(val.trim())){
            return true;
        }

        return false;

    }
    
    public static byte[] changeByteOrder(byte[] input){
        int len = input.length;
        byte[] outArray = new byte[len];
        for (int i = 0 ; i < input.length ; i++){
            outArray[i] = input[len - 1 -i];
        }
        return outArray;
    }

    public static int byteArrayToIntValue(byte[] bytes){

        int val = 0;

        if(bytes == null){
            return 0;
        }

        if(bytes.length == 0){
            return 0;
        }

        for (byte b : bytes){
            val = (val << 8) + (b & 0xFF);
        }

        return val;
    }

    public static byte[] mergeBytes(byte[] data1, byte[] data2) {
        byte[] result = new byte[data1.length + data2.length];
        System.arraycopy(data1, 0, result, 0, data1.length);
        System.arraycopy(data2, 0, result, data1.length, data2.length);
        return result;
    }



}
