/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 扫描结果
 *
 * @since 2023-12-04
 */
public class NearlinkSeekResultInfo implements Parcelable {

    int eventType;   // 上报事件类型
    String address;   // 地址
    int addrType;    // 地址类型
    String directAddress; //  定向发现地址
    int directAddrType;  // 定向发现地址类型
    int rssi; //  信号强度
    int dataStatus; // 数据状态
    int dataLength;  // 数据长度

    byte[] data;    //  数据

    String deviceLocalName; // 设备完整本地名称

    NearlinkDevice nearlinkDevice;

    public NearlinkSeekResultInfo(int eventType, String address, int addrType, String directAddress,
                                  int directAddrType, int rssi, int dataStatus, int dataLength,
                                  byte[] data, String deviceLocalName, NearlinkDevice nearlinkDevice) {
        this.eventType = eventType;
        this.address = address;
        this.addrType = addrType;
        this.directAddress = directAddress;
        this.directAddrType = directAddrType;
        this.rssi = rssi;
        this.dataStatus = dataStatus;
        this.dataLength = dataLength;
        this.data = data;
        this.deviceLocalName = deviceLocalName;
        this.nearlinkDevice = nearlinkDevice;
    }

    public int getEventType() {
        return eventType;
    }

    public void setEventType(int eventType) {
        this.eventType = eventType;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public int getAddrType() {
        return addrType;
    }

    public void setAddrType(int addrType) {
        this.addrType = addrType;
    }

    public String getDirectAddress() {
        return directAddress;
    }

    public void setDirectAddress(String directAddress) {
        this.directAddress = directAddress;
    }

    public int getDirectAddrType() {
        return directAddrType;
    }

    public void setDirectAddrType(int directAddrType) {
        this.directAddrType = directAddrType;
    }

    public int getRssi() {
        return rssi;
    }

    public void setRssi(int rssi) {
        this.rssi = rssi;
    }

    public int getDataStatus() {
        return dataStatus;
    }

    public void setDataStatus(int dataStatus) {
        this.dataStatus = dataStatus;
    }

    public int getDataLength() {
        return dataLength;
    }

    public void setDataLength(int dataLength) {
        this.dataLength = dataLength;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }

    public String getDeviceLocalName() {
        return deviceLocalName;
    }

    public void setDeviceLocalName(String deviceLocalName) {
        this.deviceLocalName = deviceLocalName;
    }

    public NearlinkDevice getNearlinkDevice() {
        return nearlinkDevice;
    }

    public void setNearlinkDevice(NearlinkDevice nearlinkDevice) {
        this.nearlinkDevice = nearlinkDevice;
    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.eventType);
        dest.writeString(this.address);
        dest.writeInt(this.addrType);
        dest.writeString(this.directAddress);
        dest.writeInt(this.directAddrType);
        dest.writeInt(this.rssi);
        dest.writeInt(this.dataStatus);
        dest.writeInt(this.dataLength);
        dest.writeByteArray(this.data);
        dest.writeString(this.deviceLocalName);
        dest.writeParcelable(this.nearlinkDevice, flags);
    }

    public void readFromParcel(Parcel source) {
        this.eventType = source.readInt();
        this.address = source.readString();
        this.addrType = source.readInt();
        this.directAddress = source.readString();
        this.directAddrType = source.readInt();
        this.rssi = source.readInt();
        this.dataStatus = source.readInt();
        this.dataLength = source.readInt();
        this.data = source.createByteArray();
        this.deviceLocalName = source.readString();
        this.nearlinkDevice = source.readParcelable(NearlinkDevice.class.getClassLoader());
    }

    protected NearlinkSeekResultInfo(Parcel in) {
        this.eventType = in.readInt();
        this.address = in.readString();
        this.addrType = in.readInt();
        this.directAddress = in.readString();
        this.directAddrType = in.readInt();
        this.rssi = in.readInt();
        this.dataStatus = in.readInt();
        this.dataLength = in.readInt();
        this.data = in.createByteArray();
        this.deviceLocalName = in.readString();
        this.nearlinkDevice = in.readParcelable(NearlinkDevice.class.getClassLoader());
    }

    public static final Creator<NearlinkSeekResultInfo> CREATOR = new Creator<NearlinkSeekResultInfo>() {
        @Override
        public NearlinkSeekResultInfo createFromParcel(Parcel source) {
            return new NearlinkSeekResultInfo(source);
        }

        @Override
        public NearlinkSeekResultInfo[] newArray(int size) {
            return new NearlinkSeekResultInfo[size];
        }
    };
}
