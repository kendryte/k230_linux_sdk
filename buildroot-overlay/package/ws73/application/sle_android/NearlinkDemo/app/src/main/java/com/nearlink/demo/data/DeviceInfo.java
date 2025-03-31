package com.nearlink.demo.data;

import android.nearlink.NearlinkConstant;

import java.io.Serializable;

public class DeviceInfo implements Serializable {
    public String name;
    public String address;
    public int rssi;
    public int addrType;
    public int pairState = NearlinkConstant.SLE_PAIR_NONE;
    public int connState =NearlinkConstant.SLE_ACB_STATE_NONE;

    @Override
    public String toString() {
        return "DeviceInfo{" +
                "name='" + name + '\'' +
                ", address='" + address + '\'' +
                ", rssi=" + rssi +
                ", addrType=" + addrType +
                ", pairState=" + pairState +
                ", connState=" + connState +
                '}';
    }
}
