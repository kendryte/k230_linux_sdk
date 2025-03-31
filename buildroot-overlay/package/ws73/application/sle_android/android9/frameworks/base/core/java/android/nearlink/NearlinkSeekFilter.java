/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.annotation.Nullable;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * 扫描结果过滤器
 *
 * @since 2023-12-04
 */
public class NearlinkSeekFilter implements Parcelable {

    @Nullable
    private String deviceName;

    @Nullable
    private String deviceAddress;

    private NearlinkSeekFilter(String mDeviceName, String mDeviceAddress) {
        this.deviceName = mDeviceName;
        this.deviceAddress = mDeviceAddress;
    }

    /**
     * 构建者对象
     * @return 构建者
     */
    public static Build builder(){
        return new Build();
    }

    /**
     * 根据NearlinkSeekResultInfo匹配过滤器
     * @param resultInfo 需要过滤的信息seek结果参数
     * @return 命中 true 不命中 false
     */
    public boolean matches(NearlinkSeekResultInfo resultInfo){
        if(resultInfo == null){
            return false;
        }

        NearlinkDevice device = resultInfo.getNearlinkDevice();

        // 地址过滤
        if (deviceAddress != null
                && (device == null || !deviceAddress.equals(device.getAddress()))) {
            return false;
        }

        // 名称过滤
        if (deviceName != null
                && !deviceName.equals(resultInfo.getDeviceLocalName())) {
            return false;
        }

        return true;

    }

    /**
     * 构建者
     */
    public static class Build {

        private String deviceName;

        private String deviceAddress;

        public Build deviceName(String deviceName){
            this.deviceName = deviceName;
            return this;
        }

        public Build deviceAddress(String deviceAddress){
            this.deviceAddress = deviceAddress;
            return this;
        }

        public NearlinkSeekFilter build(){
            return new NearlinkSeekFilter(deviceName, deviceAddress);
        }

    }

    public String getDeviceName() {
        return deviceName;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public String getDeviceAddress() {
        return deviceAddress;
    }

    public void setDeviceAddress(String deviceAddress) {
        this.deviceAddress = deviceAddress;
    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.deviceName);
        dest.writeString(this.deviceAddress);
    }

    public void readFromParcel(Parcel source) {
        this.deviceName = source.readString();
        this.deviceAddress = source.readString();
    }

    protected NearlinkSeekFilter(Parcel in) {
        this.deviceName = in.readString();
        this.deviceAddress = in.readString();
    }

    public static final Creator<NearlinkSeekFilter> CREATOR = new Creator<NearlinkSeekFilter>() {
        @Override
        public NearlinkSeekFilter createFromParcel(Parcel source) {
            return new NearlinkSeekFilter(source);
        }

        @Override
        public NearlinkSeekFilter[] newArray(int size) {
            return new NearlinkSeekFilter[size];
        }
    };
}
