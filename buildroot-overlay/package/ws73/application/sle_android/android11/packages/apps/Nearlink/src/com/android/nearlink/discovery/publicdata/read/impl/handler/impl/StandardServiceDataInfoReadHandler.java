package com.android.nearlink.discovery.publicdata.read.impl.handler.impl;

import android.nearlink.NearlinkAppearance;
import android.util.Log;

import com.android.nearlink.discovery.publicdata.read.IPublicDataReadCallback;
import com.android.nearlink.discovery.publicdata.read.IPublicDataReadHandler;
import com.android.nearlink.nlservice.DdUtils;

import java.io.ByteArrayInputStream;
import java.util.Optional;

/**
 * 标准服务数据信息处理器
 */
public class StandardServiceDataInfoReadHandler implements IPublicDataReadHandler {

    private static final String TAG = "StandardServiceDataInformationHandler";

    private boolean isLog = false;

    /**
     * 0x03,0x12
     * 类型, 长度
     * 0x09,0x06,  0x07,  0x03,    0x02,     0x05,     0x00
     * UUID,UUID,外观类型，外观长度， 外观值字节,外观值字节,外观值字节
     */
    private final int UUID_LEN = 2;
    private final int APPEARANCE_LEN = 3;

    private final int MIN_LEN = 3;

    private final int APPEARANCE_SUB_TYPE = 0x07;


    @Override
    public void doHandle(byte[] subByteData, IPublicDataReadCallback iWritePublicDataCallback) {

        if(subByteData == null){
            debug("subByteData == null");
            iWritePublicDataCallback.setAppearance(NearlinkAppearance.UNKNOWN);
            return;
        }

        if(isLog){
            Log.e(TAG, "subByteData.len = " + subByteData.length);
            iWritePublicDataCallback.setAppearance(NearlinkAppearance.UNKNOWN);
            showBytes(subByteData);
        }

        if(subByteData.length <= UUID_LEN){
            debug("subByteData.length <= UUID_LEN");
            iWritePublicDataCallback.setAppearance(NearlinkAppearance.UNKNOWN);
            return;
        }

        Optional<byte[]> optional = getExpectSubDataByType(APPEARANCE_SUB_TYPE, subByteData);
        if(!optional.isPresent()){
            debug("can not find appearance");
            iWritePublicDataCallback.setAppearance(NearlinkAppearance.UNKNOWN);
            return;
        }

        byte[] protoAppearanceByteData = optional.get();
        if(protoAppearanceByteData.length != APPEARANCE_LEN){
            debug("appearance len != 3");
            iWritePublicDataCallback.setAppearance(NearlinkAppearance.UNKNOWN);
            return;
        }

        byte[] appearanceByteData = DdUtils.changeByteOrder(protoAppearanceByteData);
        int appearance = DdUtils.byteArrayToIntValue(appearanceByteData);
        iWritePublicDataCallback.setAppearance(appearance);
    }


    private Optional<byte[]> getExpectSubDataByType(int expectType, byte[] subByteData){
        ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(subByteData);
        byteArrayInputStream.skip(UUID_LEN);
        while (byteArrayInputStream.available() >= MIN_LEN){
            int type = byteArrayInputStream.read();
            int len = byteArrayInputStream.read();
            if(len > byteArrayInputStream.available()){
                return Optional.empty();
            }
            if(expectType != type){
                byteArrayInputStream.skip(len);
                continue;
            }
            byte[] expectDataBytes = new byte[len];
            byteArrayInputStream.read(expectDataBytes, 0, len);
            return Optional.ofNullable(expectDataBytes);
        }
        return Optional.empty();
    }

    private void debug(String msg){
        if(isLog){
            Log.e(TAG, msg);
        }
    }

    private void showBytes(byte[] bytes){
        for (byte b : bytes){
            Log.e(TAG, "byte = " + b);
        }
    }
}
