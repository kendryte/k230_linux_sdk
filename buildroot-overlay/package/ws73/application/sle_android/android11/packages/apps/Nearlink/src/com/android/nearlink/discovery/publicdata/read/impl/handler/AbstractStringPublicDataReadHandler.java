/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.read.impl.handler;

import android.util.Log;

import com.android.nearlink.discovery.publicdata.read.IPublicDataReadCallback;
import com.android.nearlink.discovery.publicdata.read.IPublicDataReadHandler;

import java.nio.charset.StandardCharsets;

/**
 * 抽象String类的读处理器
 *
 * @since 2023-12-04
 */
public abstract class AbstractStringPublicDataReadHandler implements IPublicDataReadHandler {

    private static final String TAG = "AbstractStringPublicDataReadHandler";

    @Override
    public void doHandle(byte[] subByteData, IPublicDataReadCallback iWritePublicDataCallback) {

        if(subByteData == null || subByteData.length == 0){
            return;
        }

        String stringValue = null;
        try{
            stringValue = new String(subByteData, StandardCharsets.UTF_8);
        }catch (Exception e){
            Log.e(TAG, "parse subByteData error" + e);
            return;

        }

        setStringValue(stringValue, iWritePublicDataCallback);

    }


    protected abstract void setStringValue(String value, IPublicDataReadCallback iWritePublicDataCallback);
}
