package com.nearlink.demo;

import com.nearlink.demo.data.DeviceInfo;

public interface IScanCallback {
    void onScanResult(DeviceInfo deviceInfo);
}
