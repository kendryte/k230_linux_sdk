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
 * 星闪SSAP服务
 *
 * <p>SSAP服务内包含属性列表{@link NearlinkSsapProperty}</p>
 *
 * @since 2023-12-01
 */
public class NearlinkSsapService implements Parcelable {
    /**
     * 服务的UUID
     *
     * @hide
     */
    protected UUID mUuid;

    /**
     * 服务的handle值
     *
     * @hide
     */
    protected int mHandle;

    /**
     * 是否是首要服务
     *
     * @hide
     */
    protected boolean mIsPrimary;

    protected NearlinkDevice mDevice;

    /**
     * 服务下的属性列表
     */
    protected List<NearlinkSsapProperty> mProperties;

    /**
     * 新建一个SSAP首要服务
     *
     * @param uuid 服务的UUID
     * */
    public NearlinkSsapService(UUID uuid) {
        this(uuid, true);
    }

    /**
     * 新建一个SSAP服务
     *
     * @param uuid      服务的UUID
     * @param isPrimary 是否是首要服务
     */
    public NearlinkSsapService(UUID uuid, boolean isPrimary) {
        mUuid = uuid;
        mHandle = 0;
        mIsPrimary = isPrimary;
        mProperties = new ArrayList<>();
    }

    protected NearlinkSsapService(Parcel in) {
        mUuid = ((ParcelUuid) in.readParcelable(null)).getUuid();
        mHandle = in.readInt();
        mIsPrimary = in.readByte() == 1;
        mProperties = new ArrayList<>();

        ArrayList<NearlinkSsapProperty> props = in.createTypedArrayList(NearlinkSsapProperty.CREATOR);
        if (props != null) {
            for (NearlinkSsapProperty prop : props) {
                prop.setService(this);
                addProperty(prop);
            }
        }
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeParcelable(new ParcelUuid(mUuid), 0);
        out.writeInt(mHandle);
        out.writeByte(mIsPrimary ? (byte) 1 : (byte) 0);
        out.writeTypedList(mProperties);
    }

    /**
     * @hide
     */
    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<NearlinkSsapService> CREATOR = new Creator<NearlinkSsapService>() {
        @Override
        public NearlinkSsapService createFromParcel(Parcel in) {
            return new NearlinkSsapService(in);
        }

        @Override
        public NearlinkSsapService[] newArray(int size) {
            return new NearlinkSsapService[size];
        }
    };

    public UUID getUuid() {
        return mUuid;
    }

    public int getHandle() {
        return mHandle;
    }

    public void setHandle(int handle) {
        this.mHandle = handle;
    }

    public boolean isPrimary() {
        return mIsPrimary;
    }

    public NearlinkDevice getDevice() {
        return mDevice;
    }

    public void setDevice(NearlinkDevice mDevice) {
        this.mDevice = mDevice;
    }

    public List<NearlinkSsapProperty> getProperties() {
        return mProperties;
    }

    final public void addProperty(NearlinkSsapProperty property) {
        mProperties.add(property);
        property.setService(this);
    }
}
