/*
 * Copyright (c) . 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 连接地址
 *
 * @since 2023-12-01
 */
public class NearlinkAddress implements Parcelable {
    private static final String TAG = "[SLE_CONN_FRAMEWORK]--NearlinkAddress";
    /**
     * CREATOR
     */
    public static final Creator<NearlinkAddress> CREATOR = new Creator<NearlinkAddress>() {
        @Override
        public NearlinkAddress createFromParcel(Parcel in) {
            return new NearlinkAddress(in);
        }

        @Override
        public NearlinkAddress[] newArray(int size) {
            return new NearlinkAddress[size];
        }
    };

    // 获取设备mac地址
    private String address;

    // 获取设备mac地址类型
    private int type;

    public NearlinkAddress(String address, int type) {
        this.address = address;
        this.type = type;
    }

    protected NearlinkAddress(Parcel in) {
        address = in.readString();
        type = in.readInt();
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }


    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(address);
        dest.writeInt(type);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public String toString() {
        return "NearlinkAddress{"
                + "address='" + address + '\''
                + ", type=" + type + '}';
    }
}
