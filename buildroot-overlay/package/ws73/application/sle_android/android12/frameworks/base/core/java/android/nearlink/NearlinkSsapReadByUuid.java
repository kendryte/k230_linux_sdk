/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 星闪SSAP，通过UUID服务属性值的封装结构体
 *
 * @since 2023-12-01
 */
public class NearlinkSsapReadByUuid implements Parcelable {
    int handle;
    int type;
    byte[] data;

    public NearlinkSsapReadByUuid(int handle, int type, byte[] data) {
        this.handle = handle;
        this.type = type;
        this.data = data;
    }

    public int getHandle() {
        return handle;
    }

    public int getType() {
        return type;
    }

    public byte[] getData() {
        return data;
    }

    protected NearlinkSsapReadByUuid(Parcel in) {
        handle = in.readInt();
        type = in.readInt();
        data = in.createByteArray();
    }

    public static final Creator<NearlinkSsapReadByUuid> CREATOR = new Creator<NearlinkSsapReadByUuid>() {
        @Override
        public NearlinkSsapReadByUuid createFromParcel(Parcel in) {
            return new NearlinkSsapReadByUuid(in);
        }

        @Override
        public NearlinkSsapReadByUuid[] newArray(int size) {
            return new NearlinkSsapReadByUuid[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(handle);
        dest.writeInt(type);
        dest.writeByteArray(data);
    }
}
