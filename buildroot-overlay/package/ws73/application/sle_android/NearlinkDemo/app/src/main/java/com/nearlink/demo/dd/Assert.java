package com.nearlink.demo.dd;

import android.util.Log;

import com.nearlink.demo.UILog;

public class Assert {

    private static final int STACK_LEVEL = 4;

    private static final String SUCCESS_TAG = "[SUCCESS!!!] ";
    private static final String FAIL_TAG = "[!!!FAIL] ";

    public static void assertTure(Boolean condition, String msg) {
        String info = buildInvokingLocationInfo();
        if (condition) {
            showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
        } else {
            showAssertLog(FAIL_TAG, msg + " expected ture, but is actual is false," + info);
        }
    }

    public static void assertFalse(Boolean condition, String msg) {
        String info = buildInvokingLocationInfo();
        if (!condition) {
            showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
        } else {
            showAssertLog(FAIL_TAG, msg + " expected false, but is actual is true," + info);
        }
    }

    public static void assertEquals(Object expected, Object actual, String msg) {
        String info = buildInvokingLocationInfo();
        if (expected != null) {
            if (expected.equals(actual)) {
                showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
            } else {
                showAssertLog(FAIL_TAG, msg + " expected=" + expected + " actual=" + actual + " is not equal," + info);
            }
        } else {
            if (actual == null) {
                showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
            } else {
                showAssertLog(FAIL_TAG, msg + " expected=" + expected + " actual=" + actual + " is not equal," + info);
            }
        }
    }


    public static void assertNotEquals(Object expected, Object actual, String msg) {
        String info = buildInvokingLocationInfo();
        if (expected != null) {
            if (expected.equals(actual)) {
                showAssertLog(FAIL_TAG, msg + " expected=" + expected + " actual=" + actual + " is equal," + info);
            } else {
                showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
            }
        } else {
            if (actual == null) {
                showAssertLog(SUCCESS_TAG, msg + " is pass," + info);
            } else {
                showAssertLog(FAIL_TAG, msg + " expected=" + expected + " actual=" + actual + " is equal," + info);
            }
        }
    }

    private static void showAssertLog(String assertResultTag, String message) {
        StringBuilder sb = new StringBuilder();
        sb.append(assertResultTag);
        sb.append(message);
        UILog.log(UILog.FLAG_MAIN, sb.toString());
    }

    public static void showMessage(String message) {
        UILog.log(UILog.FLAG_MAIN, message);
    }


    public static void logToFile(boolean isSuccess, String TAG, String msg){
        StringBuilder sb = new StringBuilder();
        sb.append(isSuccess?"passed: ":"failed: ");
        sb.append(msg);
        Log.e(TAG, sb.toString());
    }

    private static String buildInvokingLocationInfo(){
        StackTraceElement[] stacktrace = Thread.currentThread().getStackTrace();
        StringBuilder sb = new StringBuilder();
        // 0 是Thread, 1 是buildInvokingLocation， 2 是 AssertXXX
        StackTraceElement stackTraceElement = stacktrace[STACK_LEVEL];
        sb.append("方法名:").append(stackTraceElement.getMethodName()).append(",");
        sb.append("行数:").append(stackTraceElement.getLineNumber()).append(",");
        sb.append("类名:").append(stackTraceElement.getClassName());
        return sb.toString();
    }

}
