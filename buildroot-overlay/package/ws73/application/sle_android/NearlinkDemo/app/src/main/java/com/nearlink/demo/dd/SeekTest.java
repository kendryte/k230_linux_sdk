package com.nearlink.demo.dd;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkSeekCallback;
import android.nearlink.NearlinkSeekCallbackConstants;
import android.nearlink.NearlinkSeekFilter;
import android.nearlink.NearlinkSeekResultInfo;
import android.nearlink.NearlinkSeeker;

@NearlinkTestClass("seek")
public class SeekTest extends BaseDdTest {

    @Override
    public void beforeTestMethod() {
        deviceMap = new HashMap<>();
    }

    @Override
    public void afterTestMethod() {
        if (deviceMap != null) {
            deviceMap.clear();
            deviceMap = null;
        }
    }

    private Map<String, NearlinkDevice> deviceMap;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Assert.showMessage("Received intent with null action");
                return;
            }
            switch (action) {
                case NearlinkDevice.ACTION_DETECTED:
                    NearlinkDevice device = intent
                            .getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                    String address = device.getAddress();
                    deviceMap.put(address, device);
                    break;
                default:
                    Assert.showMessage("Received unknown intent " + intent);
                    break;
            }
        }
    };

    @NearlinkTest("001")
    public void seek001() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        SeekCallback seekCallback = new SeekCallback();
        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();
        seeker.startSeek(seekCallback);

        DdUtils.delaySeconds(3);
        Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
        Assert.assertFalse(seekCallback.isStopSuccessFlag, "");

        Assert.showMessage("等待70s");
        DdUtils.delaySeconds(70);

        Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止回调成功");
        Assert.assertTure(!DdUtils.isBlank(seekCallback.address), "测试seek结果回调");
    }


    @NearlinkTest("002")
    public void seek002() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        SeekCallback seekCallback = new SeekCallback();
        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();
        seeker.startSeek(seekCallback);

        DdUtils.delaySeconds(3);
        Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
        Assert.assertFalse(seekCallback.isStopSuccessFlag, "");

        seeker.stopSeek(seekCallback);

        DdUtils.delaySeconds(3);
        Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止成功回调");
        Assert.assertTure(!DdUtils.isBlank(seekCallback.address), "测试seek结果回调");

    }


    @NearlinkTest("003")
    public void seek003() {

        enableNearlink();


        List<SeekCallback> seekCallbacks = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            seekCallbacks.add(new SeekCallback());
        }

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();

        for (SeekCallback seekCallback : seekCallbacks) {
            seeker.startSeek(seekCallback);
        }

        DdUtils.delaySeconds(6);

        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
            Assert.assertFalse(seekCallback.isStopSuccessFlag, "");
        }

        Assert.showMessage("等待70s");
        DdUtils.delaySeconds(70);

        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(!DdUtils.isBlank(seekCallback.address), "测试seek结果回调");
            Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止回调成功");
        }

    }

    @NearlinkTest("004")
    public void seek004() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();


        List<SeekCallback> seekCallbacks = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            seekCallbacks.add(new SeekCallback());
        }

        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();

        for (SeekCallback seekCallback : seekCallbacks) {
            seeker.startSeek(seekCallback);
            seeker.startSeek(seekCallback);
        }

        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);

        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
            Assert.assertFalse(seekCallback.isStopSuccessFlag, "");
            Assert.assertEquals(NearlinkSeekCallbackConstants.SEEK_FAILED_ALREADY_STARTED, seekCallback.startErrorCode, "测试触发重复callback回调");
        }


        for (SeekCallback seekCallback : seekCallbacks) {
            seeker.stopSeek(seekCallback);
            seeker.stopSeek(seekCallback);
            seekCallback.address = null;
        }

        Assert.showMessage("stopSeek后，等待10s");
        DdUtils.delaySeconds(10);
        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止成功回调");
            Assert.assertTure(DdUtils.isBlank(seekCallback.address), "测试seek结果回调");
        }
    }

    @NearlinkTest("005")
    public void seek005() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();


        List<SeekCallback> seekCallbacks = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            seekCallbacks.add(new SeekCallback());
        }

        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();

        for (SeekCallback seekCallback : seekCallbacks) {
            seeker.startSeek(seekCallback);
        }

        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);

        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
            Assert.assertFalse(seekCallback.isStopSuccessFlag, "");
            Assert.assertFalse(DdUtils.isBlank(seekCallback.address), "测试地址信息回调上来");
            Assert.showMessage("address: " + seekCallback.address);
        }


        for (SeekCallback seekCallback : seekCallbacks) {
            seeker.stopSeek(seekCallback);
            seekCallback.address = null;
        }

        Assert.showMessage("stopSeek后，等待10s");
        DdUtils.delaySeconds(10);
        for (SeekCallback seekCallback : seekCallbacks) {
            Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止成功回调");
            Assert.assertTure(DdUtils.isBlank(seekCallback.address), "测试seek结果回调");
        }
    }


    @NearlinkTest("006")
    public void seek006() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        SeekCallback seekCallback = new SeekCallback();
        NearlinkSeeker seeker = nearlinkAdapter.getNearlinkSeeker();
        seeker.startSeek(seekCallback);

        DdUtils.delaySeconds(3);
        Assert.assertTure(seekCallback.isStartSuccessFlag, "测试seek启动成功回调");
        Assert.assertFalse(seekCallback.isStopSuccessFlag, "");

        int i = 0;
        while (i <= 10) {
            i++;
            DdUtils.delaySeconds(3);
            Assert.assertTure(seekCallback.nearlinkDevice != null, "");
            if (seekCallback.nearlinkDevice != null) {
                int appearance = seekCallback.nearlinkDevice.getNearlinkAppearance();
                Assert.showMessage("addr: " + seekCallback.address + " appearance: " + Integer.toHexString(appearance));
            }
        }

        seeker.stopSeek(seekCallback);
        DdUtils.delaySeconds(3);
        Assert.assertTure(seekCallback.isStopSuccessFlag, "测试seek停止成功回调");
        Assert.assertTure(!DdUtils.isBlank(seekCallback.address), "测试seek结果回调");

    }


    @NearlinkTest("007")
    public void seek007() {
        enableNearlink();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkDevice.ACTION_DETECTED);
        registerReceiver(mReceiver, filter);

        Assert.showMessage("等待20s");
        DdUtils.delaySeconds(20);
        Assert.assertTure(!deviceMap.isEmpty(), "测试deviceMap非空");
        for (Map.Entry<String, NearlinkDevice> entry : deviceMap.entrySet()) {
            Assert.showMessage("address: " + entry.getKey() + "name: " + entry.getValue().getName());
        }

        beforeTestMethod();

        Assert.showMessage("等待20s");
        DdUtils.delaySeconds(20);
        Assert.assertTure(deviceMap.isEmpty(), "测试deviceMap为空");

        unregisterAllReceiver();
    }


    @NearlinkTest("008")
    public void seek008() {
        enableNearlink();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkDevice.ACTION_DETECTED);
        registerReceiver(mReceiver, filter);

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        Assert.showMessage("等待20s");
        DdUtils.delaySeconds(20);
        Assert.assertTure(!deviceMap.isEmpty(), "测试deviceMap非空");
        for (Map.Entry<String, NearlinkDevice> entry : deviceMap.entrySet()) {
            Assert.showMessage("address: " + entry.getKey() + "name: " + entry.getValue().getName());
        }
        beforeTestMethod();
        Assert.showMessage("等待20s");
        DdUtils.delaySeconds(20);
        Assert.assertTure(deviceMap.isEmpty(), "测试deviceMap为空");
        unregisterAllReceiver();
    }


    @NearlinkTest("010")
    public void startSeekForIntent() {
        enableNearlink();
    }

    @NearlinkTest("011")
    public void stopSeekForIntent() {
        enableNearlink();
    }

    @NearlinkTest("012")
    public void seek012() {
        String address = "C0:01:03:4A:00:BB";
        Set<String> addressSet = new HashSet<>();
        SeekCallback seekCallback = new SeekCallback() {
            @Override
            public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
                super.onResult(nearlinkSeekResultInfo);
                String addr = nearlinkSeekResultInfo.getAddress();
                addressSet.add(addr);
            }
        };
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();
        NearlinkSeekFilter nearlinkSeekFilter = new NearlinkSeekFilter.Build().deviceAddress(address).build();
        List<NearlinkSeekFilter> filters = new ArrayList<>();
        filters.add(nearlinkSeekFilter);
        nearlinkSeeker.startSeek(filters, seekCallback);
        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        Assert.assertTure(addressSet.size() == 1, "测试仅仅收到" + address);
        Iterator<String> it = addressSet.iterator();
        while (it.hasNext()) {
            Assert.showMessage(it.next());
        }
    }

    @NearlinkTest("013")
    public void seek013() {
        String name = "HUAWEI Mouse CD23-R";
        Set<String> addressSet = new HashSet<>();
        SeekCallback seekCallback = new SeekCallback() {
            @Override
            public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
                super.onResult(nearlinkSeekResultInfo);
                String addr = nearlinkSeekResultInfo.getAddress();
                addressSet.add(addr);
            }
        };
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();
        NearlinkSeekFilter nearlinkSeekFilter = new NearlinkSeekFilter.Build().deviceName(name).build();
        List<NearlinkSeekFilter> filters = new ArrayList<>();
        filters.add(nearlinkSeekFilter);
        nearlinkSeeker.startSeek(filters, seekCallback);
        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        Assert.assertTure(addressSet.size() == 2, "测试按照名字" + name);
        Iterator<String> it = addressSet.iterator();
        while (it.hasNext()) {
            Assert.showMessage(it.next());
        }
    }

    @NearlinkTest("014")
    public void seek014() {
        String name = "HUAWEI Mouse CD23-R";
        String address = "C0:01:03:4A:00:BB";
        Set<String> addressSet = new HashSet<>();
        SeekCallback seekCallback = new SeekCallback() {
            @Override
            public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
                super.onResult(nearlinkSeekResultInfo);
                String addr = nearlinkSeekResultInfo.getAddress();
                addressSet.add(addr);
            }
        };
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();
        NearlinkSeekFilter nearlinkSeekFilter = new NearlinkSeekFilter.Build().deviceName(name)
                .deviceAddress(address).build();
        List<NearlinkSeekFilter> filters = new ArrayList<>();
        filters.add(nearlinkSeekFilter);
        nearlinkSeeker.startSeek(filters, seekCallback);
        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        Assert.assertTure(addressSet.size() == 1, "测试按照名字+地址" + name);
        Iterator<String> it = addressSet.iterator();
        while (it.hasNext()) {
            Assert.showMessage(it.next());
        }
    }


    class SeekCallback extends NearlinkSeekCallback {

        public boolean isStartSuccessFlag = false;
        public boolean isStopSuccessFlag = false;
        public String address = null;
        private NearlinkDevice nearlinkDevice = null;
        public int startErrorCode = 0;
        public int stopErrorCode = 0;

        @Override
        public void onSeekStartedSuccess() {
            super.onSeekStartedSuccess();
            isStartSuccessFlag = true;
        }

        @Override
        public void onSeekStoppedSuccess() {
            super.onSeekStoppedSuccess();
            isStopSuccessFlag = true;
        }

        @Override
        public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
            super.onResult(nearlinkSeekResultInfo);
            address = nearlinkSeekResultInfo.getAddress();
            nearlinkDevice = nearlinkSeekResultInfo.getNearlinkDevice();
        }

        @Override
        public void onSeekStartedFailure(int errorCode) {
            super.onSeekStartedFailure(errorCode);
            startErrorCode = errorCode;
        }

        @Override
        public void onSeekStoppedFailure(int errorCode) {
            super.onSeekStoppedFailure(errorCode);
            stopErrorCode = errorCode;
        }
    }

}
