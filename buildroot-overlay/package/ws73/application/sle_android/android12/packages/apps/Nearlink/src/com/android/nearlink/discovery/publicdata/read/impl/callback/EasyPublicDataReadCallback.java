/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata.read.impl.callback;

import com.android.nearlink.discovery.publicdata.read.EasyPublicData;
import com.android.nearlink.discovery.publicdata.read.IPublicDataReadCallback;

/**
 * 写入EasyPublicData类
 *
 * @since 2023-12-04
 */
public class EasyPublicDataReadCallback implements IPublicDataReadCallback {

    private EasyPublicData easyPublicData;

    public EasyPublicDataReadCallback(EasyPublicData easyPublicData) {
        this.easyPublicData = easyPublicData;
    }

    @Override
    public void setDeviceLocalName(String name) {
        easyPublicData.deviceLocalName = name;
    }

    @Override
    public void setAppearance(int appearance) {
        easyPublicData.appearance = appearance;
    }
}
