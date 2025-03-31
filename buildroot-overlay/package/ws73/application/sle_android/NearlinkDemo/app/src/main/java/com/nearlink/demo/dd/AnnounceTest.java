package com.nearlink.demo.dd;

import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAnnounceCallback;
import android.nearlink.NearlinkAnnounceCallbackConstants;
import android.nearlink.NearlinkAnnounceSettings;
import android.nearlink.NearlinkAnnouncer;
import android.nearlink.NearlinkPublicData;

@NearlinkTestClass("announce")
public class AnnounceTest extends BaseDdTest {


    @Override
    public void beforeTestMethod() {

    }

    @Override
    public void afterTestMethod() {

    }

    @NearlinkTest("001")
    public void announce001() {
        enableNearlink();
        boolean res;
        for (int i = 0; i < 10; i++) {
            NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
            NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
            NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().build();
            NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
            NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
            AnnounceCallback announceCallback = new AnnounceCallback();
            res = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
            Assert.assertTure(res, "启动announce");
            DdUtils.delaySeconds(3);
            Assert.assertTure(announceCallback.startSuccessCallbackFlag, "测试announce启动成功回调");
            Assert.assertTure(announceCallback.startFailCallbackErrorCode == 0, "测试announce启动失败没有回调");
            nearlinkAnnouncer.stopAnnounce(announceCallback);
            DdUtils.delaySeconds(3);
            Assert.assertTure(announceCallback.stopSuccessCallbackFlag, "测试announce停止成功回调");
            Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        }
    }


    @NearlinkTest("002")
    public void announce002() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
        for (int i = 0; i < 10; i++) {
            NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().build();
            NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
            NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
            AnnounceCallback announceCallback1 = new AnnounceCallback();
            AnnounceCallback announceCallback2 = new AnnounceCallback();
            AnnounceCallback announceCallback3 = new AnnounceCallback();
            boolean res1 = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback1);
            boolean res2 = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback2);
            boolean res3 = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback3);
            Assert.assertTure(res1, "测试第一次启动announce返回成功");
            Assert.assertTure(res2, "快速启动announce返回成功");
            Assert.assertTure(res3, "快速启动announce返回成功");
            DdUtils.delaySeconds(3);
            Assert.assertTure(announceCallback1.startSuccessCallbackFlag, "测试announce成功启动回调执行");
            Assert.assertFalse(announceCallback2.startSuccessCallbackFlag, "测试announce成功回调没有执行");
            Assert.assertEquals(NearlinkAnnounceCallbackConstants.ANNOUNCE_EXCEED_MAX_USING_NUM,
                    announceCallback2.startFailCallbackErrorCode, "测试触发了announce超过路数限制错误回调");
            Assert.assertFalse(announceCallback3.startSuccessCallbackFlag, "测试announce成功回调没有执行");
            Assert.assertEquals(NearlinkAnnounceCallbackConstants.ANNOUNCE_EXCEED_MAX_USING_NUM,
                    announceCallback3.startFailCallbackErrorCode, "测试触发了announce超过路数限制错误回调");
            nearlinkAnnouncer.stopAnnounce(announceCallback1);
            nearlinkAnnouncer.stopAnnounce(announceCallback2);
            nearlinkAnnouncer.stopAnnounce(announceCallback3);
            DdUtils.delaySeconds(3);
            Assert.assertTure(announceCallback1.stopSuccessCallbackFlag, "测试announce停止成功回调");
            Assert.assertTure(announceCallback1.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        }
    }

    @NearlinkTest("003")
    public void announce003() {
        enableNearlink();
        byte[] announceDataBytes = new byte[]{5, 4,
                11, 6, 9, 6,
                3, 18,
                9, 6,
                7, 3,
                2, 5, 0,
                6, 17,
                72, 85, 65, 87, 69, 73, 32, 77, 111, 117, 115, 101, 32, 67, 68, 50, 5};
        byte[] seekRespDataBytes = new byte[]{11, 17,
                72, 85, 65, 87, 69, 73, 32, 77, 111, 117, 115, 101, 32, 67, 68, 48, 48};
        boolean res;
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
        NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().timeoutMillis(65535).build();
        NearlinkPublicData announceData = new NearlinkPublicData.Build().data(announceDataBytes).build();
        NearlinkPublicData seekRespData = new NearlinkPublicData.Build().data(seekRespDataBytes).build();
        AnnounceCallback announceCallback = new AnnounceCallback();
        res = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
        Assert.assertTure(res, "启动announce");
        DdUtils.delaySeconds(3);
        Assert.assertTure(announceCallback.startSuccessCallbackFlag, "测试announce启动成功回调");
        Assert.assertTure(announceCallback.startFailCallbackErrorCode == 0, "测试announce启动失败没有回调");
        Assert.showMessage("等待66s");
        DdUtils.delaySeconds(66);
        Assert.assertTure(announceCallback.stopSuccessCallbackFlag, "测试announce停止成功回调");
        Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
    }

    @NearlinkTest("004")
    public void announce004() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
        for (int i = 0; i < 10; i++) {
            NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().build();
            NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
            NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
            AnnounceCallback announceCallback = new AnnounceCallback();
            boolean res1 = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
            Assert.assertTure(res1, "测试第一次启动announce返回成功");
            Assert.showMessage("等待3秒");
            DdUtils.delaySeconds(3);
            Assert.assertTure(announceCallback.startSuccessCallbackFlag, "测试announce成功启动回调执行");
            Assert.showMessage("等待55秒");
            DdUtils.delaySeconds(55);
            Assert.assertFalse(announceCallback.stopSuccessCallbackFlag, "测试announce停止回调没有执行");
            Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
            Assert.showMessage("等待5秒");
            DdUtils.delaySeconds(5);
            Assert.assertTure(announceCallback.stopSuccessCallbackFlag, "测试announce停止成功回调");
            Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        }
    }

    @NearlinkTest("005")
    public void announce005() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
        for (int i = 0; i < 10; i++) {
            NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().timeoutMillis(8 * 1000).build();
            NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
            NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
            AnnounceCallback announceCallback = new AnnounceCallback();
            boolean res1 = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
            Assert.assertTure(res1, "测试第一次启动announce返回成功");
            Assert.showMessage("等待2秒");
            DdUtils.delaySeconds(2);
            Assert.assertTure(announceCallback.startSuccessCallbackFlag, "测试announce成功启动回调执行");
            Assert.showMessage("等待2秒");
            DdUtils.delaySeconds(2);
            Assert.assertFalse(announceCallback.stopSuccessCallbackFlag, "测试announce停止回调没有执行");
            Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
            Assert.showMessage("等待5秒");
            DdUtils.delaySeconds(5);
            Assert.assertTure(announceCallback.stopSuccessCallbackFlag, "测试announce停止成功回调");
            Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        }
    }

    @NearlinkTest("006")
    public void announce006() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        NearlinkAnnouncer nearlinkAnnouncer = nearlinkAdapter.getNearlinkAnnouncer();
        NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().timeoutMillis(10 * 1000).build();
        NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
        NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
        AnnounceCallback announceCallback = new AnnounceCallback();
        boolean res = nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
        Assert.assertTure(res, "测试第一次启动announce返回成功");
        Assert.showMessage("等待2秒");
        DdUtils.delaySeconds(2);
        Assert.assertTure(announceCallback.startSuccessCallbackFlag, "测试announce成功启动回调执行");
        Assert.showMessage("等待2秒");
        DdUtils.delaySeconds(2);
        Assert.assertFalse(announceCallback.stopSuccessCallbackFlag, "测试announce停止回调没有执行");
        Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        nearlinkAnnouncer.stopAnnounce(announceCallback);
        DdUtils.delaySeconds(2);
        Assert.assertTure(announceCallback.stopSuccessCallbackFlag, "测试announce停止成功回调");
        Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");
        announceCallback.stopSuccessCallbackFlag = false;
        Assert.showMessage("等待5秒");
        DdUtils.delaySeconds(5);
        Assert.assertFalse(announceCallback.stopSuccessCallbackFlag, "测试不会触发自动停止announce回调");
        Assert.assertTure(announceCallback.stopFailCallbackErrorCode == 0, "测试announce失败回调没有执行");

    }

    class AnnounceCallback extends NearlinkAnnounceCallback {

        public Boolean startSuccessCallbackFlag = false;
        public int startFailCallbackErrorCode = 0;
        public Boolean stopSuccessCallbackFlag = false;
        public int stopFailCallbackErrorCode = 0;
        public Boolean terminaledCallbackFlag = false;

        public AnnounceCallback() {
            super();
        }

        @Override
        public void onStartedSuccess() {
            super.onStartedSuccess();
            startSuccessCallbackFlag = true;
        }

        @Override
        public void onStartedFailure(int errorCode) {
            super.onStartedFailure(errorCode);
            startFailCallbackErrorCode = errorCode;
        }

        @Override
        public void onStoppedSuccess() {
            super.onStoppedSuccess();
            stopSuccessCallbackFlag = true;
        }

        @Override
        public void onStoppedFailure(int errorCode) {
            super.onStoppedFailure(errorCode);
            stopFailCallbackErrorCode = errorCode;
        }

        @Override
        public void onTerminaled() {
            super.onTerminaled();
            terminaledCallbackFlag = true;
        }
    }

}
