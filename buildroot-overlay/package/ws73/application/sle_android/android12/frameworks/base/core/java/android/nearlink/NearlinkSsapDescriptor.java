/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 星闪SSAP服务属性的描述符
 *
 * @since 2023-12-01
 */
public class NearlinkSsapDescriptor implements Parcelable {
    /**
     * 描述符类型：属性描述符
     */
    public static final int TYPE_USER_DESCRIPTION = 0x01;

    /**
     * 描述符类型：客户端配置描述符
     */
    public static final int TYPE_CLIENT_CONFIGURATION = 0x02;

    /**
     * 描述符类型：服务端配置描述性
     */
    public static final int TYPE_SERVER_CONFIGURATION = 0x03;

    /**
     * 描述符类型：格式描述符
     */
    public static final int TYPE_PRESENTATION_FORMAT = 0x04;

    /**
     * 描述符类型：服务管理保留描述符
     */
    public static final int TYPE_RFU = 0x05;

    /**
     * 描述符类型：厂商自定义描述符
     */
    public static final int TYPE_CUSTOM = 0xFF;

    /**
     * 描述符类型
     *
     * @hide
     */
    protected int mType;

    /**
     * 描述符被谁持有
     *
     * @hide
     */
    protected NearlinkSsapProperty mProperty;

    /**
     * 描述符值
     *
     * @hide
     */
    protected byte[] mValue;


    /**
     * 新建一个描述符
     *
     * @param type 描述符类型
     */
    public NearlinkSsapDescriptor(int type) {
        mType = type;
    }

    protected NearlinkSsapDescriptor(Parcel in) {
        mType = in.readInt();
        int len = in.readInt();
        if (len > 0) {
            mValue = new byte[len];
            in.readByteArray(mValue);
        }
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mType);
        int len = mValue == null ? 0 : mValue.length;
        out.writeInt(len);
        if (len > 0) {
            out.writeByteArray(mValue);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<NearlinkSsapDescriptor> CREATOR = new Creator<NearlinkSsapDescriptor>() {
        @Override
        public NearlinkSsapDescriptor createFromParcel(Parcel in) {
            return new NearlinkSsapDescriptor(in);
        }

        @Override
        public NearlinkSsapDescriptor[] newArray(int size) {
            return new NearlinkSsapDescriptor[size];
        }
    };

    public int getType() {
        return mType;
    }

    public NearlinkSsapProperty getProperty() {
        return mProperty;
    }

    public byte[] getValue() {
        return mValue;
    }

    void setProperty(NearlinkSsapProperty mProperty) {
        this.mProperty = mProperty;
    }

    public void setValue(byte[] mValue) {
        this.mValue = mValue;
    }
}
