package com.nearlink.demo.ssap;

import android.app.Activity;
import android.content.Intent;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkSeekCallback;
import android.nearlink.NearlinkSsapClient;
import android.nearlink.NearlinkSsapClientCallback;
import android.nearlink.NearlinkSsapDescriptor;
import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapService;

import java.util.List;

/**
 * SLE SSAP 用户接口示例
 * */
public class SsapClientUserIf extends Activity {

    private static final int REQ_CODE_ENABLE = 1;
    private static final int REQ_CODE_DISABLE = 2;

    void enableNearlink() {
        Intent intent = new Intent(NearlinkAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(intent, REQ_CODE_ENABLE);
    }

    void disableNearlink() {
        Intent intent = new Intent(NearlinkAdapter.ACTION_REQUEST_DISABLE);
        startActivityForResult(intent, REQ_CODE_DISABLE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQ_CODE_ENABLE) {
            if (resultCode == RESULT_OK) {
                // SLE enabled
            } else if (resultCode == RESULT_CANCELED) {
                // SLE enable canceled
            }
        } else if (requestCode == REQ_CODE_DISABLE) {
            if (resultCode == RESULT_OK) {
                // SLE disabled
            } else if (resultCode == RESULT_CANCELED) {
                // SLE disable canceled
            }
        }
    }

    NearlinkSeekCallback seekCallback = new NearlinkSeekCallback();

    void startSeek() {
        NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
        // 回调形式上报
        adapter.getNearlinkSeeker().startSeek(null, seekCallback);

        // 广播形式上报
        adapter.startDiscovery();
    }

    void stopSeek() {
        NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
        adapter.getNearlinkSeeker().stopSeek(seekCallback);
    }

    NearlinkSsapClient ssapClient = null;

    void connectSsap(NearlinkDevice device) {
        ssapClient = device.connectSsap(new NearlinkSsapClientCallback() {
            @Override
            public void onConnectionStateChange(NearlinkSsapClient client, int state) {
                super.onConnectionStateChange(client, state);
            }

            @Override
            public void onServicesDiscovered(NearlinkSsapClient client, int status) {
                super.onServicesDiscovered(client, status);
            }

            @Override
            public void onPropertyRead(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
                super.onPropertyRead(client, property, status);
            }

            @Override
            public void onDescriptorRead(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
                super.onDescriptorRead(client, descriptor, status);
            }

            @Override
            public void onPropertyWrite(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
                super.onPropertyWrite(client, property, status);
            }

            @Override
            public void onDescriptorWrite(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
                super.onDescriptorWrite(client, descriptor, status);
            }

            @Override
            public void onExchangeInfo(NearlinkSsapClient client, int mtu, int version, int status) {
                super.onExchangeInfo(client, mtu, version, status);
            }

            @Override
            public void onPropertyChanged(NearlinkSsapClient client, NearlinkSsapProperty property) {
                super.onPropertyChanged(client, property);
                // 收到服务端的通知时在这里处理
            }
        });
    }

    // 连接建立成功后
    void doSomething() {
        // 阻塞式调用，一般来说，读、写操作都要等到其回调上来之后才能进行下一个操作
        // 客户端与服务端双向交互各自独立，客户端读写的阻塞式调用不会影响服务端在任意时刻发送通知过来
        ssapClient.discoverServices();
        // 等onServicesDiscovered回调之后
        List<NearlinkSsapService> services = ssapClient.getServices();
        NearlinkSsapProperty property = services.get(0).getProperties().get(0);
        ssapClient.setPropertyNotification(property, true);
        ssapClient.readProperty(property);
        byte[] value = property.getValue();
        // 等onPropertyRead回调之后
        ssapClient.writeProperty(property);
        // 等onPropertyWrite回调之后
        NearlinkSsapDescriptor descriptor = property.getDescriptors().get(0);
        ssapClient.readDescriptor(descriptor);
        // 等onDescriptorRead回调之后
        ssapClient.writeDescriptor(descriptor);
        // 等onDescriptorWrite回调之后
        // 其他……
    }

    void disconnectSsap() {
        if (ssapClient != null) {
            ssapClient.disconnect();
        }
    }

    void closeSsap() {
        if (ssapClient != null) {
            ssapClient.close();
            ssapClient = null;
        }
    }

}
