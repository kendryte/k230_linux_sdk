package com.nearlink.demo.dd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

@NearlinkTestClass("enable")
public class EnableTest extends BaseDdTest {

    private static final String TAG = "EnableTest";

    private final int DO_ENABLE_CMD = 0;
    private final int DO_DISABLE_CMD = 1;

    private Handler enableHandler;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Assert.showMessage("Received intent with null action");
                return;
            }
            switch (action) {
                case NearlinkAdapter.ACTION_STATE_CHANGED:
                    int preState = intent.getIntExtra(NearlinkAdapter.EXTRA_PREVIOUS_STATE, -1);
                    int newState = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, -1);
                    Log.e(TAG, NearlinkAdapter.nameForState(preState)
                            + " => " + NearlinkAdapter.nameForState(newState));
                    if (newState == NearlinkAdapter.STATE_OFF) {
                        enableHandler.sendEmptyMessage(DO_ENABLE_CMD);
                    }
                    if (newState == NearlinkAdapter.STATE_ON) {
                        enableHandler.sendEmptyMessage(DO_DISABLE_CMD);
                    }
                    break;
                default:
                    Assert.showMessage("Received unknown intent " + intent);
                    break;
            }
        }
    };

    @Override
    public void beforeTestMethod() {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                // 先调用 Looper.prepare() 方法创建 Looper 对象
                Looper.prepare();
                // 创建 Handler 对象，并重写 handleMessage 方法
                enableHandler = new Handler(Looper.myLooper()) {
                    @Override
                    public void handleMessage(Message msg) {
                        switch (msg.what) {
                            case DO_ENABLE_CMD:
                                NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
                                boolean res = nearlinkAdapter.enable();
                                Assert.assertTure(res, "测试启动");
                                break;
                            case DO_DISABLE_CMD:
                                nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
                                res = nearlinkAdapter.disable();
                                Assert.assertTure(res, "测试关闭");
                                break;
                            default:
                                Log.e(TAG, "unhandle msg");
                        }
                    }
                };
                // 调用 Looper.loop() 方法启动消息循环
                Looper.loop();
            }
        });
        thread.start();
    }

    @Override
    public void afterTestMethod() {
        if (enableHandler != null) {
            enableHandler.removeCallbacksAndMessages(null);
            Looper looper = enableHandler.getLooper();
            if (looper != null) {
                looper.quitSafely();
            }
        }
    }

    /**
     * 多次enable协议栈后，任然可以关闭协议栈
     */
    @NearlinkTest("001")
    public void enable001() {

        enableNearlink();

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;

        int state;
        for (int i = 0; i < 3; i++) {
            res = nearlinkAdapter.enable();
            Assert.assertTure(res, "enbale协议栈");
            state = nearlinkAdapter.getState();
            Assert.assertEquals(NearlinkAdapter.STATE_ON, state, "启动协议栈后检查状态");
            Assert.assertTure(nearlinkAdapter.isEnabled(), "启动协议栈后检查isEnabled()");
        }

        res = nearlinkAdapter.disable();
        state = nearlinkAdapter.getState();
        Assert.assertTure(res, "disable协议栈");
        DdUtils.delaySeconds(3);
        Assert.assertEquals(NearlinkAdapter.STATE_OFF, state, "关闭协议栈后检查状态");
        Assert.assertFalse(nearlinkAdapter.isEnabled(), "关闭协议栈后检查isEnabled()");


    }

    /**
     * 循环enable，disable
     */
    @NearlinkTest("002")
    public void enable002() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        int state = nearlinkAdapter.getState();
        Assert.assertEquals(NearlinkAdapter.STATE_ON, state, "启动android 默认STATE_ON");
        Assert.assertTure(nearlinkAdapter.isEnabled(), "启动android Apk 默认enable");

        for (int i = 0; i < 3; i++) {

            res = nearlinkAdapter.disable();
            Assert.assertTure(res, "关闭disable协议栈");

            DdUtils.delaySeconds(3);

            state = nearlinkAdapter.getState();
            Assert.assertEquals(NearlinkAdapter.STATE_OFF, state, "关闭协议栈");
            Assert.assertFalse(nearlinkAdapter.isEnabled(), "关闭协议栈");

            res = nearlinkAdapter.enable();
            Assert.assertTure(res, "disable协议栈");

            DdUtils.delaySeconds(3);

            state = nearlinkAdapter.getState();
            Assert.assertEquals(NearlinkAdapter.STATE_ON, state, "打开协议栈");
            Assert.assertFalse(nearlinkAdapter.isEnabled(), "打开协议栈");

        }
    }

    /**
     * 多次disable后，任然可以拉起
     */
    @NearlinkTest("003")
    public void enable003() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        int state = nearlinkAdapter.getState();
        Assert.assertEquals(NearlinkAdapter.STATE_ON, state, "启动android 默认STATE_ON");
        Assert.assertTure(nearlinkAdapter.isEnabled(), "启动android Apk 默认enable");

        for (int i = 0; i < 3; i++) {

            res = nearlinkAdapter.disable();
            Assert.assertTure(res, "关闭disable协议栈");

            DdUtils.delaySeconds(3);

            state = nearlinkAdapter.getState();
            Assert.assertEquals(NearlinkAdapter.STATE_OFF, state, "关闭协议栈");
            Assert.assertFalse(nearlinkAdapter.isEnabled(), "关闭协议栈");

        }

        res = nearlinkAdapter.enable();
        Assert.assertTure(res, "disable协议栈");

        DdUtils.delaySeconds(3);

        state = nearlinkAdapter.getState();
        Assert.assertEquals(NearlinkAdapter.STATE_ON, state, "打开协议栈");
        Assert.assertFalse(nearlinkAdapter.isEnabled(), "打开协议栈");

    }


    @NearlinkTest("004")
    public void enable004() {

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        for (int i = 0; i < 100; i++) {
            boolean res = nearlinkAdapter.enable();
            Assert.assertTure(res, "测试enable");
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            res = nearlinkAdapter.disable();
            Assert.assertTure(res, "测试disable");
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

    }

    @NearlinkTest("005")
    public void enable005() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res = nearlinkAdapter.disable();
        Assert.assertTure(res, "首次disable");
        int n = 30;
        while (n > 0) {
            n--;
            DdUtils.delaySeconds(3);
            Assert.showMessage("n = " + n);
        }

    }

}
