package com.nearlink.demo;

import android.os.Handler;
import android.os.Message;

import java.util.HashMap;
import java.util.Map;

public class UILog {
    public static final int ID_LOG = 1;
    public static final String FLAG_MAIN = "MainActivity";
    public static final String FLAG_OP = "OperationsActivity";

    private static Map<String, Handler> mHandlerSet = new HashMap<>();

    public static void addOuter(final String flag, Handler handler) {
        mHandlerSet.put(flag, handler);
    }

    public static void rmOuter(final String flag) {
        mHandlerSet.remove(flag);
    }

    public static void log(final String flag, String log) {
        log(flag, flag, log);
    }

    public static void log(final String flag, String tag, String log) {
        Handler handler = mHandlerSet.get(flag);
        if (handler != null) {
            StringBuilder sb = new StringBuilder();
            sb.append("[").append(tag).append("] ").append(log).append("\r\n");
            Message message = handler.obtainMessage(ID_LOG, sb.toString());
            handler.sendMessage(message);
        }
    }
}
