package com.nearlink.demo.dd;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;

import com.nearlink.demo.MainActivity;

public abstract class BaseDdTest {

    final private MainActivity mainActivity = MainActivity.getMainActivity();

    List<BroadcastReceiver> registerReceiver = new ArrayList<>();

    protected synchronized void registerReceiver(BroadcastReceiver mReceiver,
                                                 IntentFilter filter) {
        if (!registerReceiver.isEmpty()) {
            mainActivity.unregisterReceiver(mReceiver);
        }
        registerReceiver.add(mReceiver);
        mainActivity.registerReceiver(mReceiver, filter, "android.permission.NEARLINK", null);
        mainActivity.registerReceiver(mReceiver, filter, "android.permission.ACCESS_COARSE_LOCATION", null);
    }

    protected synchronized void unregisterAllReceiver() {
        if (registerReceiver.isEmpty()) {
            return;
        }
        Iterator<BroadcastReceiver> it = registerReceiver.iterator();
        while (it.hasNext()) {
            BroadcastReceiver receiver = it.next();
            mainActivity.unregisterReceiver(receiver);
            it.remove();
        }
    }

    protected void enableNearlink() {

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        boolean res;
        int state = nearlinkAdapter.getState();
        if (state == NearlinkAdapter.STATE_ON) {
            return;
        }

        res = nearlinkAdapter.enable();
        Assert.assertTure(res, "enable 结果");
        DdUtils.delaySeconds(3);
        Assert.assertTure(nearlinkAdapter.isEnabled(), "启动enable");

    }


    protected void restartNearlink() {

        NearlinkAdapter nearlinkAdapter = NearlinkAdapter.getDefaultAdapter();

        boolean res;
        res = nearlinkAdapter.disable();
        Assert.assertTure(res, "执行disable");
        DdUtils.delaySeconds(3);

        res = nearlinkAdapter.enable();
        Assert.assertTure(res, "执行enable");
        DdUtils.delaySeconds(3);
        Assert.assertTure(nearlinkAdapter.isEnabled(), "启动enable");

    }

    abstract public void beforeTestMethod();

    abstract public void afterTestMethod();

}
