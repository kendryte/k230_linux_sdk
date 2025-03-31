package com.nearlink.demo.dd;

import com.nearlink.demo.UILog;

import java.util.Random;

public class DdUtils {

    private final static String TAG = "DdUtils";

    public static void delaySeconds(int seconds) {

        long millis = 1000 * seconds;
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            UILog.log(TAG, e.getMessage());
        }
    }


    public static boolean isBlank(String value) {
        if (value == null) {
            return true;
        }

        if (value.isEmpty()) {
            return true;
        }

        if (value.trim().isEmpty()) {
            return true;
        }

        return false;
    }

    public static String getRandomString(int length) { //length表示生成字符串的长度
        String base = "abcdefghijklmnopqrstuvwxyz0123456789";
        Random random = new Random();
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < length; i++) {
            int number = random.nextInt(base.length());
            sb.append(base.charAt(number));
        }
        return sb.toString();
    }

    public static String getRandomHexString(int length) { //length表示生成字符串的长度
        String base = "0123456789ABCDE";
        Random random = new Random();
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < length; i++) {
            int number = random.nextInt(base.length());
            sb.append(base.charAt(number));
        }
        return sb.toString();
    }
}
