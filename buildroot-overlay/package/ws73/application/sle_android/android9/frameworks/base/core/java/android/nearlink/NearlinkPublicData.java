/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 公开数据
 *
 * @since 2023-12-04
 */
public class NearlinkPublicData implements Parcelable {

    /**
     * 广播级别参数
     */
    public static final int ANNOUNCE_LEVEL_NONE = 0;
    // announce level normal
    public static final int ANNOUNCE_LEVEL_NORMAL = 1;
    // announce level priority, reserve
    public static final int ANNOUNCE_LEVEL_PRIORITY = 2;
    // announce level paired, reserve
    public static final int ANNOUNCE_LEVEL_PAIRED = 3;
    // announce level special
    public static final int ANNOUNCE_LEVEL_SPECIAL = 4;

    private int announceLevel;

    private byte[] data;

    public static Build builder() {
        return new Build();
    }

    private NearlinkPublicData(int announceLevel, byte[] data) {
        this.announceLevel = announceLevel;
        this.data = data;
    }

    public int getAnnounceLevel() {
        return announceLevel;
    }

    public void setAnnounceLevel(int announceLevel) {
        this.announceLevel = announceLevel;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }

    public static class Build {

        private int announceLevel = ANNOUNCE_LEVEL_NORMAL;

        private byte[] data;

        public Build announceLevel(int announceLevel) {
            if (checkAnnounceLevelInvalid(announceLevel)) {
                throw new IllegalArgumentException("announceLevel invalid");
            }
            this.announceLevel = announceLevel;
            return this;
        }

        private boolean checkAnnounceLevelInvalid(int announceLevel) {
            return announceLevel != ANNOUNCE_LEVEL_NONE
                    && announceLevel != ANNOUNCE_LEVEL_NORMAL
                    && announceLevel != ANNOUNCE_LEVEL_PRIORITY
                    && announceLevel != ANNOUNCE_LEVEL_PAIRED
                    && announceLevel != ANNOUNCE_LEVEL_SPECIAL;
        }

        public Build data(byte[] data) {
            this.data = data;
            return this;
        }

        public NearlinkPublicData build() {
            return new NearlinkPublicData(this.announceLevel,
                    this.data);
        }

    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.announceLevel);
        dest.writeByteArray(this.data);
    }

    public void readFromParcel(Parcel source) {
        this.announceLevel = source.readInt();
        this.data = source.createByteArray();
    }

    protected NearlinkPublicData(Parcel in) {
        this.announceLevel = in.readInt();
        this.data = in.createByteArray();
    }

    public static final Creator<NearlinkPublicData> CREATOR = new Creator<NearlinkPublicData>() {

        @Override
        public NearlinkPublicData createFromParcel(Parcel source) {
            return new NearlinkPublicData(source);
        }

        @Override
        public NearlinkPublicData[] newArray(int size) {
            return new NearlinkPublicData[size];
        }
    };
}
