/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.read;

import android.util.Log;

import com.android.nearlink.discovery.DiscoveryServiceConfig;
import com.android.nearlink.discovery.publicdata.PublicDataEnums;
import com.android.nearlink.discovery.publicdata.read.impl.handler.impl.RemoteDeviceNameReadHandler;
import com.android.nearlink.discovery.publicdata.read.impl.handler.impl.StandardServiceDataInfoReadHandler;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * 公开数据读取处理器
 *
 * @since 2023-12-04
 */
public class PublicDataReadProcessor {

    private static final boolean LOG_DEBUG = false;

    private volatile static PublicDataReadProcessor sPublicDataProcessor;

    Map<Integer, IPublicDataReadHandler> handlerMap;

    private static final String TAG = DiscoveryServiceConfig.TAG_PREFIX + "PublicDataProcessor";

    // 根据协议栈，至少由3个字节，第一个字节表示数据类型， 第二个字节表示数据部分的长度，第三个字节表示数据部分
    private final int MIN_DATA_LEN = 3;

    private ISubPublicDataReadParser subPublicDataParser;

    private PublicDataReadProcessor(ISubPublicDataReadParser subPublicDataParser) {
        this.subPublicDataParser = subPublicDataParser;
        this.handlerMap = new HashMap<>();
        initHandlerMap();
    }

    // 每一种数据类型的解析需要新增依次handler到HashMap中
    private void initHandlerMap() {
        Log.e(TAG, "initHandlerMap");
        handlerMap.put(PublicDataEnums.DEVICE_SHORT_LOCAL_NAME.getTypeVal(),
                new RemoteDeviceNameReadHandler());
        handlerMap.put(PublicDataEnums.STANDARD_SERVICE_DATA_INFORMATION.getTypeVal(),
                new StandardServiceDataInfoReadHandler());
    }

    public void cleanUp() {
        this.subPublicDataParser = null;
        if (handlerMap != null) {
            handlerMap.clear();
            handlerMap = null;
        }
    }

    public static PublicDataReadProcessor getPublicDataProcessor(ISubPublicDataReadParser subPublicDataParser) {
        if (sPublicDataProcessor == null) {
            synchronized (PublicDataReadProcessor.class) {
                if (sPublicDataProcessor == null) {
                    sPublicDataProcessor = new PublicDataReadProcessor(subPublicDataParser);
                }
            }
        }
        return sPublicDataProcessor;
    }


    public void doProcess(String addr, int dataLength, byte[] data, IPublicDataReadCallback iWritePublicDataCallback) {
        // data部分长度至少3个字节
        if (dataLength < MIN_DATA_LEN) {
            Log.e(TAG, "addr: " + addr + " dataLength <" + MIN_DATA_LEN + "only :" + dataLength);
            return;
        }
        if (data == null) {
            Log.e(TAG, "addr: " + addr + " data is null");
            return;
        }
        // data部分长度至少3个字节
        if (data.length < MIN_DATA_LEN) {
            Log.e(TAG, "addr：" + addr + "data.length <" + MIN_DATA_LEN + "only :" + data.length);
            return;
        }
        ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(data);
        while (isPublicDataAvailable(addr, byteArrayInputStream)) {
            ParsedSubPublicData parsedSubPublicData = subPublicDataParser.parseOne(byteArrayInputStream);
            if (parsedSubPublicData == null) {
                continue;
            }
            int subDataType = parsedSubPublicData.getType();
            byte[] subByteData = parsedSubPublicData.getData();
            IPublicDataReadHandler publicDataHandler = handlerMap.get(Integer.valueOf(subDataType));
            if (publicDataHandler == null) {
                logDebug("subDataType=" + subDataType + " no IPublicDataHandler can match!!!");
                continue;
            }
            try{
                publicDataHandler.doHandle(subByteData, iWritePublicDataCallback);
            }catch (Exception e){
                Log.e(TAG, "parse subByteData error " + e + ")");
            }

        }
    }

    private boolean isPublicDataAvailable(String addr, ByteArrayInputStream byteArrayInputStream) {
        int available = byteArrayInputStream.available();
        boolean flag = (available >= MIN_DATA_LEN);
        if (!flag && available != 0) {
            // 如果是小于MIN_DATA_LEN（实际值为可能为1,2），则表明数据解析有问题
            logDebug("available is " + available + " < " + MIN_DATA_LEN + " addr is " + addr);
        }
        return flag;
    }

    private void logDebug(String msg) {
        if (LOG_DEBUG) {
            Log.e(TAG, msg);
        }
    }

}
