/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.read.impl.handler.impl;

import com.android.nearlink.discovery.publicdata.read.IPublicDataReadCallback;
import com.android.nearlink.discovery.publicdata.read.impl.handler.AbstractStringPublicDataReadHandler;

/**
 * 远端设备名称解析处理器
 *
 * @since 2023-12-04
 */

public class RemoteDeviceNameReadHandler extends AbstractStringPublicDataReadHandler {

    @Override
    protected void setStringValue(String value, IPublicDataReadCallback iWritePublicDataCallback) {

        iWritePublicDataCallback.setDeviceLocalName(value);

    }
}
