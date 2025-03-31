package com.nearlink.demo;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

public abstract class BaseActivity extends AppCompatActivity {

    private Handler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                int msgId = msg.what;
                if (msgId == UILog.ID_LOG) {
                    handleLog((String) (msg.obj));
                }
            }
        };
    }

    protected abstract String getLogFlag();

    @Override
    protected void onStart() {
        super.onStart();
        UILog.addOuter(getLogFlag(), mHandler);
    }

    public void log2UI(String tag, String log) {
        StringBuilder sb = new StringBuilder();
        sb.append("[").append(tag).append("] ").append(log).append("\r\n");
        Message message = mHandler.obtainMessage(UILog.ID_LOG, sb.toString());
        mHandler.sendMessage(message);
    }

    protected void handleLog(String log) {
        getLogTxt().append(log);
    }

    protected abstract EditText getLogTxt();

    @Override
    protected void onDestroy() {
        super.onDestroy();
        UILog.rmOuter(getLogFlag());
    }
}