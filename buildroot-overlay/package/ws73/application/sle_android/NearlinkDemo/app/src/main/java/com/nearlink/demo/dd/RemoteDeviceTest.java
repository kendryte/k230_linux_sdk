package com.nearlink.demo.dd;

import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkSeekCallback;
import android.nearlink.NearlinkSeekResultInfo;
import android.nearlink.NearlinkSeeker;

import java.util.HashMap;
import java.util.Map;

@NearlinkTestClass("remote")
public class RemoteDeviceTest extends BaseDdTest{

    @Override
    public void beforeTestMethod() {

    }

    @Override
    public void afterTestMethod() {

    }

    @NearlinkTest("001")
    public void remote001(){
        enableNearlink();
        NearlinkSeeker nearlinkSeeker = NearlinkAdapter.getDefaultAdapter().getNearlinkSeeker();
        SeekCallback seekCallback = new SeekCallback();
        nearlinkSeeker.startSeek(null, seekCallback);
        Assert.showMessage("等待10s");
        DdUtils.delaySeconds(10);
        Map<String, NearlinkDevice> nearlinkDeviceMap = seekCallback.nearlinkDeviceMap;
        Assert.assertTure(!nearlinkDeviceMap.isEmpty(), "测试扫描到设备");
        for (Map.Entry<String, NearlinkDevice> entrySet :  nearlinkDeviceMap.entrySet()){
            NearlinkDevice nearlinkDevice = entrySet.getValue();
            String name = nearlinkDevice.getName();
            String alias = nearlinkDevice.getAlias();
            int appearance = nearlinkDevice.getNearlinkAppearance();
            Assert.showMessage("address:" + entrySet.getKey() + " name: " + name
                    + " alias: " + alias
                    + " appearance: " + appearance);

            String newAlias = DdUtils.getRandomString(10);
            nearlinkDevice.setAlias(newAlias);
            DdUtils.delaySeconds(1);
            String readAlias = nearlinkDevice.getAlias();
            Assert.assertTure(newAlias.equals(readAlias), "测试alias修改成功:" + newAlias);
            Assert.showMessage("address:" + entrySet.getKey() + " name: " + nearlinkDevice.getName()
                    + " alias: " + nearlinkDevice.getAlias() + " appearance: " + nearlinkDevice.getNearlinkAppearance());
        }
    }



    @NearlinkTest("002")
    public void remote002(){
        enableNearlink();
        String address = "C0:01:03:D2:00:BB";
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkDevice nearlinkDevice = nearlinkAdapter.getRemoteDevice(address);
        String newAlias = DdUtils.getRandomString(21);
        Assert.showMessage("newAlias = " + newAlias);
        boolean res  = nearlinkDevice.setAlias(newAlias);
        Assert.assertFalse(res, "测试超长");
    }


    class SeekCallback extends NearlinkSeekCallback {

        public Map<String , NearlinkDevice>  nearlinkDeviceMap = new HashMap<>();

        @Override
        public void onSeekStartedSuccess() {
            super.onSeekStartedSuccess();
        }

        @Override
        public void onSeekStoppedSuccess() {
            super.onSeekStoppedSuccess();
        }

        @Override
        public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
            super.onResult(nearlinkSeekResultInfo);
            nearlinkDeviceMap.put(nearlinkSeekResultInfo.getAddress(),
                    nearlinkSeekResultInfo.getNearlinkDevice());
        }

        @Override
        public void onSeekStartedFailure(int errorCode) {
            super.onSeekStartedFailure(errorCode);
        }

        @Override
        public void onSeekStoppedFailure(int errorCode) {
            super.onSeekStoppedFailure(errorCode);
        }
    }
}
