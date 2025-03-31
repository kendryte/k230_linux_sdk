package com.nearlink.demo.dd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkSeekCallback;
import android.nearlink.NearlinkSeekResultInfo;
import android.nearlink.NearlinkSeeker;

import java.util.HashMap;
import java.util.Map;

@NearlinkTestClass("discovery")
public class DiscoveryTest extends BaseDdTest {

    private boolean isDiscoveryStartFlag;
    private boolean isDiscoveryFinishFlag;
    private Map<String, NearlinkDevice> deviceMap;
    private Map<String, String> nameMap;
    private Map<String, Integer> appearanceMap;

    private Map<String, NearlinkDevice> detectedMap;

    @Override
    public void beforeTestMethod() {
        isDiscoveryStartFlag = false;
        isDiscoveryFinishFlag = false;
        deviceMap = new HashMap<>();
        nameMap = new HashMap<>();
        appearanceMap = new HashMap<>();
        detectedMap = new HashMap<>();
    }

    @Override
    public void afterTestMethod() {
        isDiscoveryStartFlag = false;
        isDiscoveryFinishFlag = false;
        if (nameMap != null) {
            nameMap.clear();
            nameMap = null;
        }
        if (appearanceMap != null) {
            appearanceMap.clear();
            appearanceMap = null;
        }
        if (deviceMap == null) {
            deviceMap.clear();
            deviceMap = null;
        }
        if (detectedMap == null) {
            detectedMap.clear();
            deviceMap = null;
        }

    }

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

                case NearlinkDevice.ACTION_FOUND:
                    NearlinkDevice device = intent
                            .getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                    String name = intent
                            .getStringExtra(NearlinkDevice.EXTRA_NAME);
                    int appearance = intent
                            .getIntExtra(NearlinkDevice.EXTRA_APPEARANCE, NearlinkAppearance.UNKNOWN);
                    String address = device.getAddress();
                    if (detectedMap != null) {
                        deviceMap.put(address, device);
                    }
                    if (nameMap != null) {
                        nameMap.put(address, DdUtils.isBlank(name) ? "" : name);
                    }
                    if (appearanceMap != null) {
                        appearanceMap.put(address, appearance);
                    }
                    break;
                case NearlinkDevice.ACTION_DETECTED:
                    NearlinkDevice detectedDevice = intent
                            .getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                    String address1 = detectedDevice.getAddress();
                    if (detectedMap != null) {
                        detectedMap.put(address1, detectedDevice);
                    }
                    break;
                default:
                    Assert.showMessage("Received unknown intent " + intent);
                    break;
            }
        }
    };

    @NearlinkTest("001")
    public void discovery001() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;
        for (int i = 0; i < 3; i++) {

            res = nearlinkAdapter.startDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(3);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertTure(isDiscovering, "");

            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(3);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertFalse(isDiscovering, "");
        }
    }

    @NearlinkTest("002")
    public void discovery002() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        enableNearlink();

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "");
        Assert.assertTure(isDiscoveryStartFlag, "收到启动discovery广播");

        Assert.showMessage("等待50s");
        DdUtils.delaySeconds(50);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "");

        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertFalse(isDiscovering, "");
        Assert.assertTure(isDiscoveryFinishFlag, "收到启动discovery停止广播");

        Assert.assertTure(!nameMap.isEmpty(), "测试收到设备发现广播");
        for (Map.Entry<String, String> entry : nameMap.entrySet()) {
            Assert.showMessage("address= " + entry.getKey() + " name= " + entry.getValue());
        }
    }

    @NearlinkTest("003")
    public void discovery003() {

        enableNearlink();

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "");

        Assert.showMessage("等待20s");
        DdUtils.delaySeconds(20);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "等待20s希望任然在isDiscovering=true");

        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(2);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertFalse(isDiscovering, "");

        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(2);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "关闭discovery后，重启启动startDiscovery");

        Assert.showMessage("等待50s");
        DdUtils.delaySeconds(50);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "等待50s,discovery状态没有被上一个延迟消息stop");

        Assert.showMessage("等待11s");
        DdUtils.delaySeconds(15);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertFalse(isDiscovering, "discovery被本次延迟消息stop");
    }

    @NearlinkTest("004")
    public void discovery004() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);

        for (int i = 0; i < 10; i++) {

            beforeTestMethod();
            registerReceiver(mReceiver, filter);

            res = nearlinkAdapter.startDiscovery();
            Assert.assertTure(res, "测试startDiscovery返回");

            Assert.showMessage("等待10s");
            DdUtils.delaySeconds(10);

            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertTure(isDiscovering, "启动startDiscovery，测试isDiscovering");

            Assert.assertTure(isDiscoveryStartFlag, "测试启动成功广播回调");

            Assert.assertTure(!nameMap.isEmpty(), "测试收到设备发现广播nameMap非空");
            for (Map.Entry<String, String> entry : nameMap.entrySet()) {
                Assert.showMessage("address= " + entry.getKey() + " name= " + entry.getValue());
            }
            Assert.assertTure(!appearanceMap.isEmpty(), "测试收到设备发现广播appearanceMap非空");
            for (Map.Entry<String, Integer> entry : appearanceMap.entrySet()) {
                Assert.showMessage("address= " + entry.getKey() + " appearance= " + entry.getValue());
            }


            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "测试cancelDiscovery返回");

            DdUtils.delaySeconds(1);

            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertFalse(isDiscovering, "取消discovery，测试isDiscovering");
            Assert.assertTure(isDiscoveryFinishFlag, "测试关闭成功广播回调");

            unregisterAllReceiver();
            afterTestMethod();
        }
    }

    @NearlinkTest("005")
    public void discovery005() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        for (int i = 0; i < 10; i++) {
            res = nearlinkAdapter.startDiscovery();
            Assert.assertTure(res, "测试startDiscovery返回");
            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "测试cancelDiscovery返回");
        }

        Assert.showMessage("等待3s");
        DdUtils.delaySeconds(3);
        beforeTestMethod();

        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "测试startDiscovery返回");
        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "检查正在扫描中");
        Assert.assertTure(isDiscoveryStartFlag, "测试启动广播发送");
        Assert.assertTure(!nameMap.isEmpty(), "测试收到设备发现广播nameMap非空");
        for (Map.Entry<String, String> entry : nameMap.entrySet()) {
            Assert.showMessage("address= " + entry.getKey() + " name= " + entry.getValue());
        }
        Assert.assertTure(!appearanceMap.isEmpty(), "测试收到设备发现广播appearanceMap非空");
        for (Map.Entry<String, Integer> entry : appearanceMap.entrySet()) {
            Assert.showMessage("address= " + entry.getKey() + " appearance= " + entry.getValue());
        }

        DdUtils.delaySeconds(1);
        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "测试cancelDiscovery返回");
        DdUtils.delaySeconds(1);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertFalse(isDiscovering, "取消discovery，测试isDiscovering");
        Assert.assertTure(isDiscoveryFinishFlag, "测试关闭成功广播回调");
        unregisterAllReceiver();
    }

    /**
     * 把所有callback关闭不会影响discovery
     */
    @NearlinkTest("006")
    public void discovery006() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        SeekCallback seekCallback1 = new SeekCallback();
        SeekCallback seekCallback2 = new SeekCallback();
        SeekCallback seekCallback3 = new SeekCallback();
        SeekCallback seekCallback4 = new SeekCallback();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();

        Assert.showMessage("确保关闭discovery");
        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        while (nearlinkAdapter.isDiscovering()) {
            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(3);
        }

        Assert.showMessage("开始测试");

        isDiscoveryFinishFlag = false;
        // 先启动discovery
        res = nearlinkAdapter.setDiscoverableTimeout(5 * 60 * 1000);
        Assert.assertTure(res, "设置discovery超时时间为5分钟");
        res = nearlinkAdapter.startDiscovery();
        nearlinkSeeker.startSeek(seekCallback1);
        nearlinkSeeker.startSeek(seekCallback2);
        nearlinkSeeker.startSeek(seekCallback3);
        nearlinkSeeker.startSeek(seekCallback4);

        Assert.assertTure(res, "测试startDiscovery返回");
        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "检查正在扫描中");
        Assert.assertTure(isDiscoveryStartFlag, "测试启动广播发送");
        Assert.assertTure(!deviceMap.isEmpty(), "discovery到设备非空");

        Assert.assertTure(seekCallback1.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback2.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback3.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback4.isStartSuccessFlag, "");

        Assert.assertTure(seekCallback1.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback2.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback3.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback4.nearlinkDevice != null, "");

        nearlinkSeeker.stopSeek(seekCallback1);
        nearlinkSeeker.stopSeek(seekCallback2);
        nearlinkSeeker.stopSeek(seekCallback3);
        nearlinkSeeker.stopSeek(seekCallback4);
        DdUtils.delaySeconds(3);

        seekCallback4.nearlinkDevice = null;
        seekCallback3.nearlinkDevice = null;
        seekCallback2.nearlinkDevice = null;
        seekCallback1.nearlinkDevice = null;
        deviceMap.clear();
        isDiscoveryStartFlag = false;

        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);

        Assert.assertTure(seekCallback1.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback2.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback3.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback4.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(!deviceMap.isEmpty(), "discovery到设备非空");
        Assert.assertTure(!isDiscoveryFinishFlag, "测试没有收到停止广播");
    }

    /**
     * 把discovery广播不会影响callback，stop callback不能收到
     */
    @NearlinkTest("007")
    public void discovery007() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        SeekCallback seekCallback1 = new SeekCallback();
        SeekCallback seekCallback2 = new SeekCallback();
        SeekCallback seekCallback3 = new SeekCallback();
        SeekCallback seekCallback4 = new SeekCallback();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();

        Assert.showMessage("确保关闭discovery");
        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        while (nearlinkAdapter.isDiscovering()) {
            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(3);
        }

        Assert.showMessage("开始测试");

        isDiscoveryFinishFlag = false;
        // 先启动discovery
        res = nearlinkAdapter.setDiscoverableTimeout(5 * 60 * 1000);
        Assert.assertTure(res, "设置discovery超时时间为5分钟");
        res = nearlinkAdapter.startDiscovery();
        nearlinkSeeker.startSeek(seekCallback1);
        nearlinkSeeker.startSeek(seekCallback2);
        nearlinkSeeker.startSeek(seekCallback3);
        nearlinkSeeker.startSeek(seekCallback4);

        Assert.assertTure(res, "测试startDiscovery返回");
        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "检查正在扫描中");
        Assert.assertTure(isDiscoveryStartFlag, "测试启动广播发送");
        Assert.assertTure(!deviceMap.isEmpty(), "discovery到设备非空");

        Assert.assertTure(seekCallback1.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback2.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback3.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback4.isStartSuccessFlag, "");

        Assert.assertTure(seekCallback1.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback2.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback3.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback4.nearlinkDevice != null, "");

        isDiscoveryFinishFlag = false;
        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "关闭discovery");
        DdUtils.delaySeconds(3);

        seekCallback4.nearlinkDevice = null;
        seekCallback3.nearlinkDevice = null;
        seekCallback2.nearlinkDevice = null;
        seekCallback1.nearlinkDevice = null;
        deviceMap.clear();

        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);

        Assert.assertTure(seekCallback1.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback2.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback3.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback4.nearlinkDevice != null, "");
        Assert.assertTure(deviceMap.isEmpty(), "discovery到设备非空");
        Assert.assertTure(isDiscoveryFinishFlag, "测试没有收到停止广播");


        nearlinkSeeker.stopSeek(seekCallback1);
        nearlinkSeeker.stopSeek(seekCallback2);
        nearlinkSeeker.stopSeek(seekCallback3);
        nearlinkSeeker.stopSeek(seekCallback4);
        DdUtils.delaySeconds(3);

        seekCallback4.nearlinkDevice = null;
        seekCallback3.nearlinkDevice = null;
        seekCallback2.nearlinkDevice = null;
        seekCallback1.nearlinkDevice = null;

        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);

        Assert.assertTure(seekCallback1.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback2.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback3.nearlinkDevice == null, "测试stop后没有设备回调");
        Assert.assertTure(seekCallback4.nearlinkDevice == null, "测试stop后没有设备回调");
    }

    /**
     * 停止discovery、seek后，intentSeek不会停止
     */
    @NearlinkTest("008")
    public void discovery008() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        filter.addAction(NearlinkDevice.ACTION_DETECTED);
        registerReceiver(mReceiver, filter);

        SeekCallback seekCallback1 = new SeekCallback();
        SeekCallback seekCallback2 = new SeekCallback();
        SeekCallback seekCallback3 = new SeekCallback();
        SeekCallback seekCallback4 = new SeekCallback();
        NearlinkSeeker nearlinkSeeker = nearlinkAdapter.getNearlinkSeeker();

        Assert.showMessage("确保关闭discovery");
        res = nearlinkAdapter.cancelDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        while (nearlinkAdapter.isDiscovering()) {
            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(3);
        }

        Assert.showMessage("开始测试");

        isDiscoveryFinishFlag = false;
        // 先启动discovery
        res = nearlinkAdapter.setDiscoverableTimeout(5 * 60 * 1000);
        Assert.assertTure(res, "设置discovery超时时间为5分钟");
        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "测试startDiscovery返回");

        nearlinkSeeker.startSeek(seekCallback1);
        nearlinkSeeker.startSeek(seekCallback2);
        nearlinkSeeker.startSeek(seekCallback3);
        nearlinkSeeker.startSeek(seekCallback4);

        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);

        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "检查正在扫描中");
        Assert.assertTure(isDiscoveryStartFlag, "测试启动广播发送");
        Assert.assertTure(!deviceMap.isEmpty(), "discovery到设备非空");

        Assert.assertTure(seekCallback1.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback2.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback3.isStartSuccessFlag, "");
        Assert.assertTure(seekCallback4.isStartSuccessFlag, "");

        Assert.assertTure(seekCallback1.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback2.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback3.nearlinkDevice != null, "");
        Assert.assertTure(seekCallback4.nearlinkDevice != null, "");
        Assert.assertTure(!detectedMap.isEmpty(), "detected到设备非空");

        isDiscoveryFinishFlag = false;
        res = nearlinkAdapter.cancelDiscovery();
        nearlinkSeeker.stopSeek(seekCallback1);
        nearlinkSeeker.stopSeek(seekCallback2);
        nearlinkSeeker.stopSeek(seekCallback3);
        nearlinkSeeker.stopSeek(seekCallback4);

        Assert.assertTure(res, "关闭discovery");
        DdUtils.delaySeconds(3);

        seekCallback4.nearlinkDevice = null;
        seekCallback3.nearlinkDevice = null;
        seekCallback2.nearlinkDevice = null;
        seekCallback1.nearlinkDevice = null;
        deviceMap.clear();
        detectedMap.clear();

        Assert.showMessage("等待30s");
        DdUtils.delaySeconds(30);

        Assert.assertTure(seekCallback1.nearlinkDevice == null, "");
        Assert.assertTure(seekCallback2.nearlinkDevice == null, "");
        Assert.assertTure(seekCallback3.nearlinkDevice == null, "");
        Assert.assertTure(seekCallback4.nearlinkDevice == null, "");
        Assert.assertTure(deviceMap.isEmpty(), "discovery到设备非空");
        Assert.assertTure(isDiscoveryFinishFlag, "测试没有收到停止广播");
        Assert.assertTure(!detectedMap.isEmpty(), "detected设备非空");
    }

    /**
     * 停止discovery，不影响discoveryIntent扫描
     */
    @NearlinkTest("009")
    public void discovery009() {

        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        filter.addAction(NearlinkDevice.ACTION_DETECTED);
        registerReceiver(mReceiver, filter);

        for (int i = 0; i < 10; i++) {

            isDiscoveryStartFlag = false;
            isDiscoveryFinishFlag = false;
            deviceMap.clear();
            detectedMap.clear();
            Assert.showMessage("第 " + i + " 测试");

            DdUtils.delaySeconds(3);
            res = nearlinkAdapter.startDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(1);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertTure(isDiscovering, "");
            DdUtils.delaySeconds(5);
            Assert.assertTure(isDiscoveryStartFlag, "测试收到discovery start广播");
            Assert.assertTure(!deviceMap.isEmpty(), "测试收到discovery广播的设备");
            Assert.assertTure(!detectedMap.isEmpty(), "测试收到了detected的设备");

            res = nearlinkAdapter.cancelDiscovery();
            Assert.assertTure(res, "");
            DdUtils.delaySeconds(1);
            isDiscovering = nearlinkAdapter.isDiscovering();
            Assert.assertFalse(isDiscovering, "");
            DdUtils.delaySeconds(3);
            Assert.assertTure(isDiscoveryFinishFlag, "测试收到discovery start广播");
            isDiscoveryStartFlag = false;
            isDiscoveryFinishFlag = false;
            deviceMap.clear();
            detectedMap.clear();

            DdUtils.delaySeconds(10);
            Assert.assertTure(deviceMap.isEmpty(), "测试没有收到discovery广播的设备");
            Assert.assertTure(!detectedMap.isEmpty(), "测试收到了detected的设备");
        }
    }

    @NearlinkTest("010")
    public void discovery010() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(NearlinkDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        enableNearlink();

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res;
        boolean isDiscovering;

        res = nearlinkAdapter.setDiscoverableTimeout(2 * 60 * 1000);
        Assert.assertTure(res, "设置超时时间2min");
        res = nearlinkAdapter.startDiscovery();
        Assert.assertTure(res, "");
        DdUtils.delaySeconds(3);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "");
        Assert.assertTure(isDiscoveryStartFlag, "收到启动discovery广播");

        Assert.showMessage("等待115s");
        DdUtils.delaySeconds(115);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertTure(isDiscovering, "检查等待115s后，没有停止扫描");

        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        isDiscovering = nearlinkAdapter.isDiscovering();
        Assert.assertFalse(isDiscovering, "");
        Assert.assertTure(isDiscoveryFinishFlag, "收到启动discovery停止广播");

        Assert.assertTure(!nameMap.isEmpty(), "测试收到设备发现广播");
        for (Map.Entry<String, String> entry : nameMap.entrySet()) {
            Assert.showMessage("address= " + entry.getKey() + " name= " + entry.getValue());
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
