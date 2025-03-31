/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: OAL模块初始化.
 * Author: Huanghe
 * Create: 2020-10-13
 */

#include "oal_main.h"
#include "oal_ext_if.h"
#include "oal_schedule.h"
#include "oal_net.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 动态/静态DBDC，其中一个默认使能 */
oal_uint8  g_auc_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {WLAN_INIT_DEVICE_RADIO_CAP};

DEFINE_GET_BUILD_VERSION_FUNC(oal);

oal_uint32  oal_chip_get_version_etc(oal_void)
{
    return WLAN_CHIP_VERSION_WF51;
}

/* 检查单chip中mac device使能个数 */
OAL_STATIC oal_uint8  oal_device_check_enable_num(oal_void)
{
    oal_uint8 uc_device_num = 0;
    oal_uint8 uc_device_id;

    for (uc_device_id = 0; uc_device_id < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_id++) {
        if (g_auc_wlan_service_device_per_chip[uc_device_id]) {
            uc_device_num++;
        }
    }
    return uc_device_num;
}

/* 获取device num */
oal_uint8 oal_chip_get_device_num_etc(oal_uint32   ul_chip_ver)
{
    (void)ul_chip_ver;
    return oal_device_check_enable_num();
}
/* 获取board上的业务vap其实idx */
oal_uint8 oal_board_get_service_vap_start_id(oal_void)
{
    oal_uint8   uc_device_num_per_chip = oal_device_check_enable_num();

    /* 配置vap个数 = mac device个数,vap idx分配先配置vap,后业务vap */
    return (oal_uint8)(WLAN_CHIP_MAX_NUM_PER_BOARD * uc_device_num_per_chip);
}

EXPORT_SYMBOL(oal_chip_get_version_etc);
EXPORT_SYMBOL(oal_chip_get_device_num_etc);
EXPORT_SYMBOL(oal_board_get_service_vap_start_id);
EXPORT_SYMBOL(g_auc_wlan_service_device_per_chip);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

