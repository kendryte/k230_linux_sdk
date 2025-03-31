/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 广播设置类
 *
 * @since 2023-12-04
 */
public class NearlinkAnnounceSettings implements Parcelable {

    /**
     * non-connectable, non-scannable
     */
    public static final int ANNOUNCE_MODE_NONCONN_NONSCAN = 0;

    /**
     * connectable, non-scannable
     */
    public static final int ANNOUNCE_MODE_CONNECTABLE_NONSCAN = 1;

    /**
     * non-connectable, scannable
     */
    public static final int ANNOUNCE_MODE_NONCONN_SCANABLE = 2;

    /**
     * connectable, scannable
     */
    public static final int ANNOUNCE_MODE_CONNECTABLE_SCANABLE = 3;

    /**
     * connectable, scannable, directed
     */
    public static final int ANNOUNCE_MODE_CONNECTABLE_DIRECTED = 7;

    // 默认广播时间
    public static final int DEFAULT_TIMEOUT_MILLIS = 60 * 1000;

    // 最大广播时间
    public static final int MAX_TIMEOUT_MILLIS = 180 * 1000;

    private int announceMode;
    private int timeoutMillis;

    private NearlinkAnnounceSettings(int announceMode, int timeoutMillis) {
        this.announceMode = announceMode;
        this.timeoutMillis = timeoutMillis;
    }

    public int getAnnounceMode() {
        return announceMode;
    }

    public void setAnnounceMode(int announceMode) {
        this.announceMode = announceMode;
    }

    public int getTimeoutMillis() {
        return timeoutMillis;
    }

    public void setTimeoutMillis(int timeoutMillis) {
        this.timeoutMillis = timeoutMillis;
    }

    public static class Build {

        int announceMode = ANNOUNCE_MODE_CONNECTABLE_SCANABLE;
        int timeoutMillis = DEFAULT_TIMEOUT_MILLIS;

        public Build announceMode(int announceMode) {
            if (checkAnnounceModeInvalid(announceMode)) {
                throw new IllegalArgumentException("announceMode invalid");
            }
            this.announceMode = announceMode;
            return this;
        }

        private boolean checkAnnounceModeInvalid(int announceMode) {
            return announceMode != ANNOUNCE_MODE_NONCONN_NONSCAN
                    && announceMode != ANNOUNCE_MODE_CONNECTABLE_NONSCAN
                    && announceMode != ANNOUNCE_MODE_NONCONN_SCANABLE
                    && announceMode != ANNOUNCE_MODE_CONNECTABLE_SCANABLE
                    && announceMode != ANNOUNCE_MODE_CONNECTABLE_DIRECTED;
        }

        public Build timeoutMillis(int timeoutMillis) {
            if (timeoutMillis < 0 || timeoutMillis > MAX_TIMEOUT_MILLIS) {
                throw new IllegalArgumentException("timeoutMillis invalid (must be 0-"
                        + MAX_TIMEOUT_MILLIS + " milliseconds)");
            }
            this.timeoutMillis = timeoutMillis;
            return this;
        }

        public NearlinkAnnounceSettings build() {
            return new NearlinkAnnounceSettings(announceMode,
                    timeoutMillis);
        }

    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.announceMode);
        dest.writeInt(this.timeoutMillis);
    }

    public void readFromParcel(Parcel source) {
        this.announceMode = source.readInt();
        this.timeoutMillis = source.readInt();
    }

    protected NearlinkAnnounceSettings(Parcel in) {
        this.announceMode = in.readInt();
        this.timeoutMillis = in.readInt();
    }

    public static final Creator<NearlinkAnnounceSettings> CREATOR = new Creator<NearlinkAnnounceSettings>() {
        @Override
        public NearlinkAnnounceSettings createFromParcel(Parcel source) {
            return new NearlinkAnnounceSettings(source);
        }

        @Override
        public NearlinkAnnounceSettings[] newArray(int size) {
            return new NearlinkAnnounceSettings[size];
        }
    };
}
