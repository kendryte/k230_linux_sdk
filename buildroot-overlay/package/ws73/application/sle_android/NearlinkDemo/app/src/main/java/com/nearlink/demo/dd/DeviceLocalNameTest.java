package com.nearlink.demo.dd;

import android.nearlink.NearlinkAdapter;

@NearlinkTestClass("name")
public class DeviceLocalNameTest extends BaseDdTest {

    @Override
    public void beforeTestMethod() {

    }

    @Override
    public void afterTestMethod() {

    }


    @NearlinkTest("001")
    public void name001() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res = false;
        String name = nearlinkAdapter.getName();
        Assert.assertTure(!DdUtils.isBlank(name), "读取名字非空");
        Assert.showMessage("name=" + name);
        for (int i = 0; i < 10; i++) {
            String newName = "nl" + i;
            res = nearlinkAdapter.setName(newName);
            Assert.assertTure(res, "测试修改名称");
            name = nearlinkAdapter.getName();
            Assert.assertTure(newName.equals(name), "测试名称修改是否成功");
            Assert.showMessage("expectedName=" + newName + " actualName=" + name);
        }

        String newName = DdUtils.getRandomString(20);

        res = nearlinkAdapter.setName(newName);
        Assert.assertTure(res, "测试修改名称");
        Assert.showMessage("修改name为" + newName + "重启后，请执行002，检查name是否固化");
    }

    @NearlinkTest("002")
    public void name002() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        String name = nearlinkAdapter.getName();
        Assert.showMessage("read device name =" + name);
    }

    @NearlinkTest("003")
    public void name003() {
        enableNearlink();
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        boolean res = nearlinkAdapter.setName("0123456789012345678901234567891");
        Assert.assertFalse(res, "检查名字超过30报错");
    }

    /**
     * 在enable的情况下setName
     * 在disable的情况下，获取和修改本端名称
     */
    @NearlinkTest("004")
    public void name004(){
        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        nearlinkAdapter.enable();
        DdUtils.delaySeconds(3);
        String newName = DdUtils.getRandomHexString(5);
        Assert.showMessage("newName:" + newName);
        boolean res = nearlinkAdapter.setName(newName);
        Assert.assertTure(res, "测试设置名称正常");
        nearlinkAdapter.disable();
        DdUtils.delaySeconds(3);
        String readName = nearlinkAdapter.getName();
        Assert.showMessage("readName:" + readName);
        Assert.assertTure(newName.equals(readName), "测试读出的修改成功");
    }
}
