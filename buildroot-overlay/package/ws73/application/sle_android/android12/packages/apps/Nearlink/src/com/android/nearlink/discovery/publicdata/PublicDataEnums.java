/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery.publicdata;

/**
 * 公开数据子数据类型枚举
 *
 * @since 2023-12-04
 */
public enum PublicDataEnums {

    // 发现等级
    SEEK_LEVEL(0x01),
    // 接入层能力
    ACCESS_LAYER_CAPABILITY(0x02),
    // 标准服务数据信息
    STANDARD_SERVICE_DATA_INFORMATION(0x03),
    // 设备缩写本地名称 huawei pencil 0x09, 标准0x0A, remoteControl 0x0B
    DEVICE_SHORT_LOCAL_NAME(0x0B),
    // 广播功率大小
    ANNOUNCE_TX_POWER(0x0C);


    PublicDataEnums(Integer typeVal) {
        this.typeVal = typeVal;
    }

    private final Integer typeVal;

    public Integer getTypeVal() {
        return typeVal;
    }
}
