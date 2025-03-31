/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.ParcelUuid;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 星闪SSAP服务属性
 *
 * <p>SSAP属性是构成SSAP服务{@link NearlinkSsapService}的基础元素。SSAP属性内可包含属性值和
 * SSAP描述符列表{@link NearlinkSsapDescriptor}</p>
 *
 * @since 2023-12-01
 */
public final class NearlinkSsapProperty implements Parcelable {
    /**
     * 描述符类型：属性值
     *
     * <p>与{@link NearlinkSsapDescriptor}中定义的其他描述符类型结合起来看</p>
     */
    static final int TYPE_VALUE = 0x00;

    /**
     * 属性操作指示：value值可读
     */
    public static final int PROPERTY_READ = 0x01;

    /**
     * 属性操作指示：value值可写并且不需要服务端回复
     */
    public static final int PROPERTY_WRITE_NO_RSP = 0x02;

    /**
     * 属性操作指示：value值可读并且需要服务端回复
     */
    public static final int PROPERTY_WRITE = 0x04;

    /**
     * 属性操作指示：value值可被通知
     */
    public static final int PROPERTY_NOTIFY = 0x08;

    /**
     * 属性操作指示：value值可被指示
     */
    public static final int PROPERTY_INDICATE = 0x10;

    /**
     * 属性操作指示：value可在广播中携带
     */
    public static final int PROPERTY_BROADCAST = 0x20;

    /**
     * 属性操作指示：描述符的value可写
     */
    public static final int PROPERTY_DESCRIPTOR_WRITE = 0x100;

    /**
     * 属性操作指示：上限值
     */
    public static final int PROPERTY_MAX = 0x101;

    /**
     * 属性权限：读取
     */
    public static final int PERMISSION_READ = 0x01;

    /**
     * 属性权限：写入
     */
    public static final int PERMISSION_WRITE = 0x02;

    /**
     * 属性权限：需加密
     */
    public static final int PERMISSION_ENCRYPTION_NEED = 0x04;

    /**
     * 属性权限：需认证
     */
    public static final int PERMISSION_AUTHENTICATION_NEED = 0x08;

    /**
     * 属性权限：需授权
     */
    public static final int PERMISSION_AUTHORIZATION_NEED = 0x10;

    /**
     * 属性的UUID
     *
     * @hide
     */
    protected UUID mUuid;

    /**
     * 属性的handle值
     *
     * @hide
     */
    protected int mHandle;

    /**
     * 属性的权限
     *
     * @hide
     */
    protected int mPermissions;

    /**
     * 属性的操作指示
     *
     * @hide
     */
    protected int mOperateIndication;

    /**
     * 属性被谁持有
     *
     * @hide
     */
    protected NearlinkSsapService mService;

    /**
     * 属性下的描述符列表
     */
    protected List<NearlinkSsapDescriptor> mDescriptors;

    /**
     * 属性值
     *
     * @hide
     */
    protected byte[] mValue;

    /**
     * 新建一个属性
     *
     * @param uuid              属性的UUID
     * @param permissions       属性的权限
     * @param operateIndication 属性的操作指示
     */
    public NearlinkSsapProperty(UUID uuid, int permissions, int operateIndication) {
        mUuid = uuid;
        mHandle = 0;
        mPermissions = permissions;
        mOperateIndication = operateIndication;
        mDescriptors = new ArrayList<>();
    }

    protected NearlinkSsapProperty(Parcel in) {
        mUuid = ((ParcelUuid) in.readParcelable(null)).getUuid();
        mHandle = in.readInt();
        mPermissions = in.readInt();
        mOperateIndication = in.readInt();
        int len = in.readInt();
        if (len > 0) {
            mValue = new byte[len];
            in.readByteArray(mValue);
        }
        mDescriptors = new ArrayList<>();
        ArrayList<NearlinkSsapDescriptor> descriptors = in.createTypedArrayList(NearlinkSsapDescriptor.CREATOR);
        if (descriptors != null) {
            for (NearlinkSsapDescriptor desc : descriptors) {
                desc.setProperty(this);
                addDescriptor(desc);
            }
        }
    }

    @Override
    public void writeToParcel(Parcel out, int i) {
        out.writeParcelable(new ParcelUuid(mUuid), 0);
        out.writeInt(mHandle);
        out.writeInt(mPermissions);
        out.writeInt(mOperateIndication);
        int len = mValue == null ? 0 : mValue.length;
        out.writeInt(len);
        if (len > 0) {
            out.writeByteArray(mValue);
        }
        out.writeTypedList(mDescriptors);
    }

    public static final Creator<NearlinkSsapProperty> CREATOR = new Creator<NearlinkSsapProperty>() {
        @Override
        public NearlinkSsapProperty createFromParcel(Parcel in) {
            return new NearlinkSsapProperty(in);
        }

        @Override
        public NearlinkSsapProperty[] newArray(int size) {
            return new NearlinkSsapProperty[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * @hide
     */
    void setService(NearlinkSsapService service) {
        mService = service;
    }

    public UUID getUuid() {
        return mUuid;
    }

    public int getHandle() {
        return mHandle;
    }

    public void setHandle(int handle) {
        this.mHandle = handle;
    }

    public int getPermissions() {
        return mPermissions;
    }

    public int getOperateIndication() {
        return mOperateIndication;
    }

    public NearlinkSsapService getService() {
        return mService;
    }

    public List<NearlinkSsapDescriptor> getDescriptors() {
        return mDescriptors;
    }

    public void addDescriptor(NearlinkSsapDescriptor descriptor) {
        mDescriptors.add(descriptor);
        descriptor.setProperty(this);
    }

    /**
     * 获取属性的value值
     *
     * <p>该方法返回的是属性缓存的值，该值可被{@link NearlinkSsapClient#readProperty}
     * 的回调结果更新，也可以被服务端的属性变更通知更新</p>
     *
     * @return 属性缓存的值
     */
    public byte[] getValue() {
        return mValue;
    }

    /**
     * 设置本地属性value值
     *
     * <p>该方法仅设置本地缓存的属性值，要想将其同步到对端设备，需要调用
     * {@link NearlinkSsapClient#writeProperty}方法</p>
     *
     * @param value 属性的新值
     * @return true
     */
    public boolean setValue(byte[] value) {
        mValue = value;
        return true;
    }

    /**
     * 校验属性的操作指示设置是否合理
     *
     * <p>属性的"读"标志位与"无需回复读“标志位是互斥的，不能同时设置</p>
     *
     * <p>属性的"通知"标志位与"指示"标志位是互斥的，不能同时设置</p>
     *
     * @return 如果设置合理则返回true
     */
    public boolean valid() {
        return !((mOperateIndication & PROPERTY_WRITE_NO_RSP) == PROPERTY_WRITE_NO_RSP && (mOperateIndication & PROPERTY_WRITE) == PROPERTY_WRITE
                || (mOperateIndication & PROPERTY_NOTIFY) == PROPERTY_NOTIFY && (mOperateIndication & PROPERTY_INDICATE) == PROPERTY_INDICATE);
    }
}
