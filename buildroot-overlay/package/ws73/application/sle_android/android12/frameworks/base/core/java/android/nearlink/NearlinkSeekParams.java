/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 扫描参数
 *
 * @since 2023-12-04
 */
public class NearlinkSeekParams implements Parcelable {
    /**
     * 默认地址类型
     */
    static final int OWN_ADDR_TYPE_DEFAULT = 0;

    /**
     * 重复过滤开关。默认关闭
     */
    static final int FILTER_DUPLICATES_DEFAULT = 0;

    /**
     * 默认任何设备的数据包
     */
    static final int SEEK_FILTER_POLICY_DEFAULT = 0;

    /**
     * 默认1M
     */
    static final int SEEK_PHYS_DEFAULT = 1;

    /**
     * 被动扫描
     */
    static final int SEEK_TYPE_PASSIVE = 0;

    /**
     * 主动扫描
     */
    static final int SLE_SEEK_ACTIVE = 1;

    /**
     * 扫描间隔，默认30001
     */
    static final int SEEK_INTERVAL_DEFAULT = 30001;

    /**
     * 扫描窗口，默认30000
     */
    static final int SEEK_WINDOW_DEFAULT = 30000;

    /**
     * seek默认超时时间
     */
    static final int SEEK_TIMEOUT_MS_DEFAULT = 10000;

    int ownAddrType;   // 本端地址类型

    int filterDuplicates;  // 重复过滤开关

    int seekFilterPolicy;  // 扫描设备使用的过滤类型

    int seekPhys;  // 扫描设备所使用的PHY

    int seekType; // 扫描类型

    int seekInterval; // 扫描间隔

    int seekWindow;   //  扫描窗口

    int seekTimeoutMs;  // seek超时时间

    public NearlinkSeekParams(int ownAddrType, int filterDuplicates, int seekFilterPolicy, int seekPhys, int seekType,
        int seekInterval, int seekWindow, int seekTimeoutMs) {
        this.ownAddrType = ownAddrType;
        this.filterDuplicates = filterDuplicates;
        this.seekFilterPolicy = seekFilterPolicy;
        this.seekPhys = seekPhys;
        this.seekType = seekType;
        this.seekInterval = seekInterval;
        this.seekWindow = seekWindow;
        this.seekTimeoutMs = seekTimeoutMs;
    }

    /**
     * 构建者
     *
     * @return 构建者对象
     */
    public static Build builder() {
        return new Build();
    }

    public int getOwnAddrType() {
        return ownAddrType;
    }

    public void setOwnAddrType(int ownAddrType) {
        this.ownAddrType = ownAddrType;
    }

    public int getFilterDuplicates() {
        return filterDuplicates;
    }

    public void setFilterDuplicates(int filterDuplicates) {
        this.filterDuplicates = filterDuplicates;
    }

    public int getSeekFilterPolicy() {
        return seekFilterPolicy;
    }

    public void setSeekFilterPolicy(int seekFilterPolicy) {
        this.seekFilterPolicy = seekFilterPolicy;
    }

    public int getSeekPhys() {
        return seekPhys;
    }

    public void setSeekPhys(int seekPhys) {
        this.seekPhys = seekPhys;
    }

    public int getSeekType() {
        return seekType;
    }

    public void setSeekType(int seekType) {
        this.seekType = seekType;
    }

    public int getSeekInterval() {
        return seekInterval;
    }

    public void setSeekInterval(int seekInterval) {
        this.seekInterval = seekInterval;
    }

    public int getSeekWindow() {
        return seekWindow;
    }

    public void setSeekWindow(int seekWindow) {
        this.seekWindow = seekWindow;
    }

    public int getSeekTimeoutMs() {
        return seekTimeoutMs;
    }

    public void setSeekTimeoutMs(int seekTimeoutMs) {
        this.seekTimeoutMs = seekTimeoutMs;
    }

    public static class Build {
        int ownAddrType = OWN_ADDR_TYPE_DEFAULT;   // 本端地址类型

        int filterDuplicates = FILTER_DUPLICATES_DEFAULT;  // 重复过滤开关

        int seekFilterPolicy = SEEK_FILTER_POLICY_DEFAULT;  // 扫描设备使用的过滤类型

        int seekPhys = SEEK_PHYS_DEFAULT;  // 扫描设备所使用的PHY

        int seekType = SLE_SEEK_ACTIVE; // 扫描类型

        int seekInterval = SEEK_INTERVAL_DEFAULT; // 扫描间隔

        int seekWindow = SEEK_WINDOW_DEFAULT;   //  扫描窗口

        int seekTimeoutMs = SEEK_TIMEOUT_MS_DEFAULT;

        public Build ownAddrType(int ownAddrType) {
            this.ownAddrType = ownAddrType;
            return this;
        }

        public Build filterDuplicates(int filterDuplicates) {
            this.filterDuplicates = filterDuplicates;
            return this;
        }

        public Build seekFilterPolicy(int seekFilterPolicy) {
            this.seekFilterPolicy = seekFilterPolicy;
            return this;
        }

        public Build seekPhys(int seekPhys) {
            this.seekPhys = seekPhys;
            return this;
        }

        public Build seekType(int seekType) {
            this.seekType = seekType;
            return this;
        }

        public Build seekInterval(int seekInterval) {
            this.seekInterval = seekInterval;
            return this;
        }

        public Build seekWindow(int seekWindow) {
            this.seekWindow = seekWindow;
            return this;
        }

        public Build seekTimeoutMs(int seekTimeoutMs) {
            this.seekTimeoutMs = seekTimeoutMs;
            return this;
        }

        public NearlinkSeekParams build() {
            return new NearlinkSeekParams(ownAddrType, filterDuplicates, seekFilterPolicy, seekPhys, seekType,
                seekInterval, seekWindow, seekTimeoutMs);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.ownAddrType);
        dest.writeInt(this.filterDuplicates);
        dest.writeInt(this.seekFilterPolicy);
        dest.writeInt(this.seekPhys);
        dest.writeInt(this.seekType);
        dest.writeInt(this.seekInterval);
        dest.writeInt(this.seekWindow);
        dest.writeInt(this.seekTimeoutMs);
    }

    public void readFromParcel(Parcel source) {
        this.ownAddrType = source.readInt();
        this.filterDuplicates = source.readInt();
        this.seekFilterPolicy = source.readInt();
        this.seekPhys = source.readInt();
        this.seekType = source.readInt();
        this.seekInterval = source.readInt();
        this.seekWindow = source.readInt();
        this.seekTimeoutMs = source.readInt();
    }

    public NearlinkSeekParams() {
    }

    protected NearlinkSeekParams(Parcel in) {
        this.ownAddrType = in.readInt();
        this.filterDuplicates = in.readInt();
        this.seekFilterPolicy = in.readInt();
        this.seekPhys = in.readInt();
        this.seekType = in.readInt();
        this.seekInterval = in.readInt();
        this.seekWindow = in.readInt();
        this.seekTimeoutMs = in.readInt();
    }

    public static final Creator<NearlinkSeekParams> CREATOR = new Creator<NearlinkSeekParams>() {
        @Override
        public NearlinkSeekParams createFromParcel(Parcel source) {
            return new NearlinkSeekParams(source);
        }

        @Override
        public NearlinkSeekParams[] newArray(int size) {
            return new NearlinkSeekParams[size];
        }
    };
}
