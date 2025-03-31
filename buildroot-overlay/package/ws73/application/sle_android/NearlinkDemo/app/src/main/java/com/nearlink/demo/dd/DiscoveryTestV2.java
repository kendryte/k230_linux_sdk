package com.nearlink.demo.dd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.util.Log;

@NearlinkTestClass("discoveryV2")
public class DiscoveryTestV2 extends BaseDdTest {
    private static final String TAG = "DiscoveryTestV2";

    @Override
    public void beforeTestMethod() {
        isDiscoveryStartFlag = false;
        isDiscoveryFinishFlag = false;
    }

    @Override
    public void afterTestMethod() {
        isDiscoveryStartFlag = false;
        isDiscoveryFinishFlag = false;
    }

    private boolean isDiscoveryStartFlag;

    private boolean isDiscoveryFinishFlag;

    private volatile long counter = 0;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Assert.showMessage("Received intent with null action");
                return;
            }
            switch (action) {
                case NearlinkAdapter.ACTION_DISCOVERY_STARTED:
                    isDiscoveryStartFlag = true;
                    break;

                case NearlinkAdapter.ACTION_DISCOVERY_FINISHED:
                    isDiscoveryFinishFlag = true;
                    break;

                default:
                    Assert.showMessage("Received unknown intent " + intent);
                    break;
            }
        }
    };

    @NearlinkTest("001")
    public void discoveryV2_001() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);

        registerReceiver(mReceiver, filter);

        while (true) {
            if (counter % 50L == 0) {
                Assert.showMessage("测试次数:" + counter);
            }
            Log.e(TAG, "第" + counter + "次测试");
            beforeTestMethod();
            res = nearlinkAdapter.startDiscovery();
            Assert.logToFile(res, TAG, "测试startDiscovery返回");
            DdUtils.delaySeconds(6);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.logToFile(isDiscovering, TAG, "启动startDiscovery，测试isDiscovering");
            Assert.logToFile(isDiscoveryStartFlag, TAG, "测试启动成功广播回调");
            res = nearlinkAdapter.cancelDiscovery();
            Assert.logToFile(res, TAG, "测试cancelDiscovery返回");
            DdUtils.delaySeconds(6);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.logToFile(isDiscovering, TAG, "取消discovery，测试isDiscovering");
            Assert.logToFile(isDiscoveryFinishFlag, TAG, "测试关闭成功广播回调");
            afterTestMethod();
            counter++;

            if (counter > 10 * 1000) {
                break;
            }
        }
    }
}
