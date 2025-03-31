/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.connection;

import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * 设备profile连接缓存类
 *
 * @since 2023-12-11
 */
class ConnectionDevice implements Parcelable {
    private static final String TAG = "[SLE_CONN_PACKAGE]--ConnectionDevice";
    /**
     * CREATOR
     */
    public static final Creator<ConnectionDevice> CREATOR = new Creator<ConnectionDevice>() {
        @Override
        public ConnectionDevice createFromParcel(Parcel in) {
            return new ConnectionDevice(in);
        }

        @Override
        public ConnectionDevice[] newArray(int size) {
            return new ConnectionDevice[size];
        }
    };

    /**
     * 数据结构长度
     */
    public static final int DATA_LEN = 24;

    // 连接成功过的状态：0，未成功过；1，连成功过
    private int hasConnected = 0;

    // 获取设备mac地址
    private String address = "";

    // 获取设备mac地址类型
    private int addressType;

    // 获取连接状态,默认未连接
    private int connState = NearlinkConstant.SLE_ACB_STATE_NONE;

    // 获取配对状态，默认未配对
    private int pairState = NearlinkConstant.SLE_PAIR_NONE;

    // profile类型
    private int profileType;

    // 星闪断链原因
    private int discReason = NearlinkConstant.SLE_DISCONNECT_BY_LOCAL;


    /**
     * 构造方法
     *
     * @param address     address
     * @param connState   connState
     * @param pairState   pairState
     * @param profileType profileType
     * @param discReason  discReason
     */
    public ConnectionDevice(NearlinkAddress address, int connState, int pairState, int profileType, int discReason) {
        this.address = address.getAddress();
        this.addressType = address.getType();
        this.profileType = profileType;
        this.discReason = discReason;
        updateState(connState, pairState);
    }

    /**
     * 构造函数
     */
    protected ConnectionDevice() {

    }

    protected ConnectionDevice(Parcel in) {
        address = in.readString();
        addressType = in.readInt();
        connState = in.readInt();
        pairState = in.readInt();
        profileType = in.readInt();
        discReason = in.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(address);
        dest.writeInt(addressType);
        dest.writeInt(connState);
        dest.writeInt(pairState);
        dest.writeInt(profileType);
        dest.writeInt(discReason);
    }

    /**
     * 获取设备地址
     *
     * @return 设备地址
     */
    protected NearlinkAddress getNearlinkAddress() {
        return new NearlinkAddress(address, addressType);
    }

    /**
     * 获取设备信息
     *
     * @return 设备信息
     */
    protected NearlinkDevice getNearlinkDevice() {
        return NearlinkAdapter.getDefaultAdapter().getRemoteDevice(getNearlinkAddress());
    }

    /**
     * 存储key
     *
     * @return key
     */
    protected String getKey() {
        return "KEY:" + address + ":" + profileType;
    }

    private void updateState(int connState, int pairState) {
        setConnState(connState);
        setPairState(pairState);
    }

    public int getHasConnected() {
        return hasConnected;
    }

    public void setHasConnected(int hasConnected) {
        this.hasConnected = hasConnected;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public void setNearlinkAddress(NearlinkAddress nearlinkAddress) {
        this.address = nearlinkAddress.getAddress();
        this.addressType = nearlinkAddress.getType();
    }

    public int getAddressType() {
        return addressType;
    }

    public void setAddressType(int addressType) {
        this.addressType = addressType;
    }

    public int getConnState() {
        return connState;
    }

    public void setConnState(int connState) {
        this.connState = connState;
    }

    /**
     * 是否连接
     *
     * @return 是否连接
     */
    public boolean isConnected() {
        return connState == NearlinkConstant.SLE_ACB_STATE_CONNECTED;
    }


    public int getPairState() {
        return pairState;
    }

    public void setPairState(int pairState) {
        this.pairState = pairState;
    }

    /**
     * 是否配对
     *
     * @return 是否配对
     */
    public boolean isPaired() {
        return pairState == NearlinkConstant.SLE_PAIR_PAIRED;
    }

    public int getProfileType() {
        return profileType;
    }

    public void setProfileType(int profileType) {
        this.profileType = profileType;
    }

    public int getDiscReason() {
        return discReason;
    }

    public void setDiscReason(int discReason) {
        this.discReason = discReason;
    }

    /**
     * 转换为字符串存储
     *
     * @return 缓存信息字符串
     */
    protected String toCacheString() {
        StringBuilder buffer = new StringBuilder(DATA_LEN);
        String[] mac = address.split(":");
        StringBuilder macData = new StringBuilder();
        for (String macItem : mac) {
            macData.append(macItem);
        }
        buffer.append(String.format("%02X", hasConnected))
                .append(macData.toString())
                .append(String.format("%02X", addressType))
                .append(String.format("%02X", connState))
                .append(String.format("%02X", pairState))
                .append(String.format("%02X", profileType))
                .append(String.format("%02X", discReason));
        return buffer.toString();
    }

    /**
     * 字符串转设备profile连接缓存对象
     *
     * @param obj 带斜杠分割的连接信息
     * @return 设备profile连接缓存对象
     */
    protected static ConnectionDevice initFromCacheString(String obj) {
        ConnectionDevice device = null;
        if (obj != null && !obj.isEmpty()) {
            char[] chars = obj.toCharArray();
            String[] o = new String[chars.length / 2];
            for (int j = 0; j < chars.length; j += 2) {
                o[j / 2] = "" + chars[j] + chars[j + 1];
            }
            if (!"".equals(o[0])) {
                try {
                    device = new ConnectionDevice();
                    device.setHasConnected(Integer.parseInt(o[0], 16));
                    device.setAddress(o[1] + ":" + o[2] + ":" + o[3] + ":" + o[4] + ":" + o[5] + ":" + o[6]);
                    device.setAddressType(Integer.parseInt(o[7], 16));
                    device.setConnState(Integer.parseInt(o[8], 16));
                    device.setPairState(Integer.parseInt(o[9], 16));
                    device.setProfileType(Integer.parseInt(o[10], 16));
                    device.setDiscReason(Integer.parseInt(o[11], 16));
                } catch (NumberFormatException e) {
                    Log.e(TAG, "NumberFormatException", e);
                }
            }
        }
        return device;
    }

    @Override
    public String toString() {
        return "ConnectionDevice{"
                + "address='" + PubTools.macPrint(address) + '\''
                + ", addressType=" + addressType
                + ", connState=" + connState
                + ", pairState=" + pairState
                + ", profileType=" + profileType
                + ", discReason=" + discReason + '}';
    }
}
