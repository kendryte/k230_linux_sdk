/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

/**
 * 连接认证回调参数
 *
 * @since 2023-12-01
 */
public class NearlinkAuthInfoEvt implements Parcelable {
    private static final String TAG = "[SLE_CONN_FRAMEWORK]--NearlinkAuthInfoEvt";
    /**
     * CREATOR
     */
    public static final Creator<NearlinkAuthInfoEvt> CREATOR = new Creator<NearlinkAuthInfoEvt>() {
        @Override
        public NearlinkAuthInfoEvt createFromParcel(Parcel in) {
            return new NearlinkAuthInfoEvt(in);
        }

        @Override
        public NearlinkAuthInfoEvt[] newArray(int size) {
            return new NearlinkAuthInfoEvt[size];
        }
    };

    // 链路密钥
    byte[] linkKey;

    // 加密算法类型
    int cryptoAlgo;

    // 秘钥分发算法类型
    int keyDerivAlgo;

    // 完整性校验指示
    int integrChkInd;

    /**
     * 构造方法
     */
    public NearlinkAuthInfoEvt() {

    }

    protected NearlinkAuthInfoEvt(Parcel in) {
        linkKey = in.createByteArray();
        cryptoAlgo = in.readInt();
        keyDerivAlgo = in.readInt();
        integrChkInd = in.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeByteArray(linkKey);
        dest.writeInt(cryptoAlgo);
        dest.writeInt(keyDerivAlgo);
        dest.writeInt(integrChkInd);
    }

    public byte[] getLinkKey() {
        return linkKey;
    }

    public void setLinkKey(byte[] linkKey) {
        this.linkKey = linkKey;
    }

    public int getCryptoAlgo() {
        return cryptoAlgo;
    }

    public void setCryptoAlgo(int cryptoAlgo) {
        this.cryptoAlgo = cryptoAlgo;
    }

    public int getKeyDerivAlgo() {
        return keyDerivAlgo;
    }

    public void setKeyDerivAlgo(int keyDerivAlgo) {
        this.keyDerivAlgo = keyDerivAlgo;
    }

    public int getIntegrChkInd() {
        return integrChkInd;
    }

    public void setIntegrChkInd(int integrChkInd) {
        this.integrChkInd = integrChkInd;
    }

    @Override
    public String toString() {
        return "NearlinkAuthInfoEvt{"
                + "linkKey=" + Arrays.toString(linkKey)
                + ", cryptoAlgo=" + cryptoAlgo
                + ", keyDerivAlgo=" + keyDerivAlgo
                + ", integrChkInd=" + integrChkInd
                + '}';
    }
}
