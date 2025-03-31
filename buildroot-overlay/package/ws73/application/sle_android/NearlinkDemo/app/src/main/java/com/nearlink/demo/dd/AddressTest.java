package com.nearlink.demo.dd;

import android.nearlink.NearlinkAdapter;

@NearlinkTestClass("address")
public class AddressTest extends BaseDdTest{

    @Override
    public void beforeTestMethod() {

    }

    @Override
    public void afterTestMethod() {

    }

    @NearlinkTest("001")
    public void address001(){
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        String address = nearlinkAdapter.getAddress();
        Assert.assertTure(!DdUtils.isBlank(address), "测试获取地址不为空");
        Assert.showMessage("address: " + address);
    }


    /**
     * 没有enable的情况下，获取和修改本端名称
     */
    @NearlinkTest("002")
    public void address002(){
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        nearlinkAdapter.enable();
        DdUtils.delaySeconds(3);
        String newAddress = "11:22:33:44:55:" + DdUtils.getRandomHexString(2);
        Assert.showMessage("newAddress:" + newAddress);

        nearlinkAdapter.disable();
        DdUtils.delaySeconds(3);
        Assert.assertTure(nearlinkAdapter.getState() == NearlinkAdapter.STATE_OFF, "测试OFF状态");
        String readAddress = nearlinkAdapter.getAddress();
        Assert.showMessage("readAddress:" + readAddress);
        Assert.assertTure(newAddress.equals(readAddress), "测试读出的修改成功");
    }

}
