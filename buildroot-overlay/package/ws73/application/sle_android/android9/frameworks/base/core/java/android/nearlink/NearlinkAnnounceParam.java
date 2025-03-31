/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 设备公开参数。
 *
 * @since 2023-12-04
 */

public class NearlinkAnnounceParam implements Parcelable {

    /**
     * G/T 参数
     */
    public static final int ANNOUNCE_ROLE_T_CAN_NEGO = 0;


    public static Build builder() {
        return new Build();
    }

    public static final int NEARLINK_ANNOUNCE_HANDLE_DEFAULT = 1;
    /* 连接调度间隔25ms，单位125us */
    public static final long NEARLINK_ANNOUNCE_INTERVAL_MIN_DEFAULT = 0xC8;
    /* 连接调度间隔25ms，单位125us */
    public static final long NEARLINK_ANNOUNCE_INTERVAL_MAX_DEFAULT = 0xC8;
    /* 连接调度间隔12.5ms，单位125us */
    public static final int NEARLINK_CONN_INTERVAL_MIN_DEFAULT = 0x64;
    /* 连接调度间隔12.5ms，单位125us */
    public static final int NEARLINK_CONN_INTERVAL_MAX_DEFAULT = 0x64;
    /* 最大延时时间4990ms，单位10ms */
    public static final int NEARLINK_CONN_MAX_LATENCY = 0x1F3;
    /* 超时时间5000ms，单位10ms */
    public static final int NEARLINK_CONN_SUPERVISION_TIMEOUT_DEFAULT = 0x1F4;

    public static final int NEARLINK_ANNOUNCE_CHANNEL_MAP_DEFAULT = 1; // TODO: 2023/10/18 need get default value

    // 设备公开句柄,多路广播编号
    int announceHandle;
    //  设备公开类型
    int announceMode;
    // 角色
    int announceGtRole;
    // 发现等级
    int announceLevel;
    // 最小设备公开周期
    long announceIntervalMin;
    // 最大设备公开周期
    long announceIntervalMax;
    //  设备公开信道
    int announceChannelMap;
    // 发送功率,can native
    byte announceTxPower;
    // 本端地址
    String ownAddress;
    int ownAddrType;
    // 对端地址
    String peerAddress;
    int peerAddrType;
    // 连接间隔最小取值
    int connIntervalMin;
    //  连接间隔最大取值
    int connIntervalMax;
    // 最大休眠连接间隔
    int connMaxLatency;
    //  最大超时时间，取值范围[0x000A,0x0C80]，
    int connSupervisionTimeout;


    private NearlinkAnnounceParam(int announceHandle, int announceMode, int announceGtRole, int announceLevel, long announceIntervalMin, long announceIntervalMax, int announceChannelMap, byte announceTxPower, String ownAddress, int ownAddrType, String peerAddress, int peerAddrType, int connIntervalMin, int connIntervalMax, int connMaxLatency, int connSupervisionTimeout) {
        this.announceHandle = announceHandle;
        this.announceMode = announceMode;
        this.announceGtRole = announceGtRole;
        this.announceLevel = announceLevel;
        this.announceIntervalMin = announceIntervalMin;
        this.announceIntervalMax = announceIntervalMax;
        this.announceChannelMap = announceChannelMap;
        this.announceTxPower = announceTxPower;
        this.ownAddress = ownAddress;
        this.ownAddrType = ownAddrType;
        this.peerAddress = peerAddress;
        this.peerAddrType = peerAddrType;
        this.connIntervalMin = connIntervalMin;
        this.connIntervalMax = connIntervalMax;
        this.connMaxLatency = connMaxLatency;
        this.connSupervisionTimeout = connSupervisionTimeout;
    }

    public int getAnnounceHandle() {
        return announceHandle;
    }

    public void setAnnounceHandle(int announceHandle) {
        this.announceHandle = announceHandle;
    }

    public int getAnnounceMode() {
        return announceMode;
    }

    public void setAnnounceMode(int announceMode) {
        this.announceMode = announceMode;
    }

    public int getAnnounceGtRole() {
        return announceGtRole;
    }

    public void setAnnounceGtRole(int announceGtRole) {
        this.announceGtRole = announceGtRole;
    }

    public int getAnnounceLevel() {
        return announceLevel;
    }

    public void setAnnounceLevel(int announceLevel) {
        this.announceLevel = announceLevel;
    }

    public long getAnnounceIntervalMin() {
        return announceIntervalMin;
    }

    public void setAnnounceIntervalMin(long announceIntervalMin) {
        this.announceIntervalMin = announceIntervalMin;
    }

    public long getAnnounceIntervalMax() {
        return announceIntervalMax;
    }

    public void setAnnounceIntervalMax(long announceIntervalMax) {
        this.announceIntervalMax = announceIntervalMax;
    }

    public int getAnnounceChannelMap() {
        return announceChannelMap;
    }

    public void setAnnounceChannelMap(int announceChannelMap) {
        this.announceChannelMap = announceChannelMap;
    }

    public byte getAnnounceTxPower() {
        return announceTxPower;
    }

    public void setAnnounceTxPower(byte announceTxPower) {
        this.announceTxPower = announceTxPower;
    }

    public String getOwnAddress() {
        return ownAddress;
    }

    public void setOwnAddress(String ownAddress) {
        this.ownAddress = ownAddress;
    }

    public int getOwnAddrType() {
        return ownAddrType;
    }

    public void setOwnAddrType(int ownAddrType) {
        this.ownAddrType = ownAddrType;
    }

    public String getPeerAddress() {
        return peerAddress;
    }

    public void setPeerAddress(String peerAddress) {
        this.peerAddress = peerAddress;
    }

    public int getPeerAddrType() {
        return peerAddrType;
    }

    public void setPeerAddrType(int peerAddrType) {
        this.peerAddrType = peerAddrType;
    }

    public int getConnIntervalMin() {
        return connIntervalMin;
    }

    public void setConnIntervalMin(int connIntervalMin) {
        this.connIntervalMin = connIntervalMin;
    }

    public int getConnIntervalMax() {
        return connIntervalMax;
    }

    public void setConnIntervalMax(int connIntervalMax) {
        this.connIntervalMax = connIntervalMax;
    }

    public int getConnMaxLatency() {
        return connMaxLatency;
    }

    public void setConnMaxLatency(int connMaxLatency) {
        this.connMaxLatency = connMaxLatency;
    }

    public int getConnSupervisionTimeout() {
        return connSupervisionTimeout;
    }

    public void setConnSupervisionTimeout(int connSupervisionTimeout) {
        this.connSupervisionTimeout = connSupervisionTimeout;
    }

    public static class Build {

        // 设备公开句柄,多路广播编号
        int announceHandle = NEARLINK_ANNOUNCE_HANDLE_DEFAULT;
        //  设备公开类型
        int announceMode = NearlinkAnnounceSettings.ANNOUNCE_MODE_CONNECTABLE_SCANABLE;
        // 角色
        int announceGtRole = ANNOUNCE_ROLE_T_CAN_NEGO;
        // 发现等级
        int announceLevel;
        // 最小设备公开周期
        long announceIntervalMin = NEARLINK_ANNOUNCE_INTERVAL_MIN_DEFAULT;
        // 最大设备公开周期
        long announceIntervalMax = NEARLINK_ANNOUNCE_INTERVAL_MAX_DEFAULT;
        //  设备公开信道
        int announceChannelMap = NEARLINK_ANNOUNCE_CHANNEL_MAP_DEFAULT;
        // 发送功率
        byte announceTxPower;
        // 本端地址
        String ownAddress;
        int ownAddrType;
        // 对端地址
        String peerAddress;
        int peerAddrType;
        // 连接间隔最小取值
        int connIntervalMin = NEARLINK_CONN_INTERVAL_MIN_DEFAULT;
        //  连接间隔最大取值
        int connIntervalMax = NEARLINK_CONN_INTERVAL_MAX_DEFAULT;
        // 最大休眠连接间隔
        int connMaxLatency = NEARLINK_CONN_MAX_LATENCY;
        //  最大超时时间，取值范围[0x000A,0x0C80]，
        int connSupervisionTimeout = NEARLINK_CONN_SUPERVISION_TIMEOUT_DEFAULT;

        public Build announceTxPower(byte announceTxPower) {
            this.announceTxPower = announceTxPower;
            return this;
        }

        public Build ownAddress(String ownAddress) {
            this.ownAddress = ownAddress;
            return this;
        }

        public Build ownAddrType(int ownAddrType) {
            this.ownAddrType = ownAddrType;
            return this;
        }

        public Build peerAddress(String peerAddress) {
            this.peerAddress = peerAddress;
            return this;
        }

        public Build peerAddrType(int peerAddrType) {
            this.peerAddrType = peerAddrType;
            return this;
        }

        public Build announceMode(int announceMode){
            this.announceMode = announceMode;
            return this;
        }

        public Build announceLevel(int announceLevel){
            this.announceLevel = announceLevel;
            return this;
        }

        public NearlinkAnnounceParam build() {
            return new NearlinkAnnounceParam(announceHandle, announceMode, announceGtRole, announceLevel, announceIntervalMin,
                    announceIntervalMax, announceChannelMap, announceTxPower, ownAddress, ownAddrType, peerAddress,
                    peerAddrType, connIntervalMin, connIntervalMax, connMaxLatency, connSupervisionTimeout);
        }

    }


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.announceHandle);
        dest.writeInt(this.announceMode);
        dest.writeInt(this.announceGtRole);
        dest.writeInt(this.announceLevel);
        dest.writeLong(this.announceIntervalMin);
        dest.writeLong(this.announceIntervalMax);
        dest.writeInt(this.announceChannelMap);
        dest.writeByte(this.announceTxPower);
        dest.writeString(this.ownAddress);
        dest.writeInt(this.ownAddrType);
        dest.writeString(this.peerAddress);
        dest.writeInt(this.peerAddrType);
        dest.writeInt(this.connIntervalMin);
        dest.writeInt(this.connIntervalMax);
        dest.writeInt(this.connMaxLatency);
        dest.writeInt(this.connSupervisionTimeout);
    }

    public void readFromParcel(Parcel source) {
        this.announceHandle = source.readInt();
        this.announceMode = source.readInt();
        this.announceGtRole = source.readInt();
        this.announceLevel = source.readInt();
        this.announceIntervalMin = source.readLong();
        this.announceIntervalMax = source.readLong();
        this.announceChannelMap = source.readInt();
        this.announceTxPower = source.readByte();
        this.ownAddress = source.readString();
        this.ownAddrType = source.readInt();
        this.peerAddress = source.readString();
        this.peerAddrType = source.readInt();
        this.connIntervalMin = source.readInt();
        this.connIntervalMax = source.readInt();
        this.connMaxLatency = source.readInt();
        this.connSupervisionTimeout = source.readInt();
    }

    protected NearlinkAnnounceParam(Parcel in) {
        this.announceHandle = in.readInt();
        this.announceMode = in.readInt();
        this.announceGtRole = in.readInt();
        this.announceLevel = in.readInt();
        this.announceIntervalMin = in.readLong();
        this.announceIntervalMax = in.readLong();
        this.announceChannelMap = in.readInt();
        this.announceTxPower = in.readByte();
        this.ownAddress = in.readString();
        this.ownAddrType = in.readInt();
        this.peerAddress = in.readString();
        this.peerAddrType = in.readInt();
        this.connIntervalMin = in.readInt();
        this.connIntervalMax = in.readInt();
        this.connMaxLatency = in.readInt();
        this.connSupervisionTimeout = in.readInt();
    }

    public static final Creator<NearlinkAnnounceParam> CREATOR = new Creator<NearlinkAnnounceParam>() {
        @Override
        public NearlinkAnnounceParam createFromParcel(Parcel source) {
            return new NearlinkAnnounceParam(source);
        }

        @Override
        public NearlinkAnnounceParam[] newArray(int size) {
            return new NearlinkAnnounceParam[size];
        }
    };
}
