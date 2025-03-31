/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hmac version and get device version
 * Create: 2021-9-28
 */

#include "hmac_version.h"
#include "wlan_msg.h"
#include "frw_msg_rom.h"
#include "frw_hmac.h"
#include "oal_util.h"
#include "gen_version.h"

#define DEVICE_SIZE_OF_VERSION 2

version_t g_all_version = {{0}, {0}, {0}};

/*
 * WSxx [  a  ].[  c  ].T[  c  ].[  d  ]
 * WSxx [1-255].[1-255].T[1-255].[1-255]
 */
static osal_u32 get_wifi_host_ver(osal_u8 a, osal_u8 b, osal_u8 c, osal_u8 d)
{
    // 24:a bit31-bit24 16:b bit23-bit16 8:c bit15-bit8; 0:d bit7-bit0
    return (((a) << 24) + ((b) << 16) + ((c) << 8) + (d));
}

osal_u32 hmac_get_version(osal_void)
{
    // 1:a   10:b   105:c   1:d
    return get_wifi_host_ver(1, 10, 105, 1);
}

/* 获取device的版本 */
osal_u32 hmac_get_device_version(osal_void)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    osal_u32 read_val[DEVICE_SIZE_OF_VERSION] = {0};
    if (g_all_version.device.ver != 0) {
        return g_all_version.device.ver;
    }
    cfg_msg_init(OSAL_NULL, 0, (osal_u8 *)&read_val, DEVICE_SIZE_OF_VERSION * sizeof(osal_u32), &msg_info);
    ret = send_cfg_to_device(0, WLAN_MSG_H2D_C_CFG_GET_VERSION, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "WLAN_MSG_H2D_C_CFG_GET_VERSION send to device failed!! ret=0x%x.", ret);
        return 0;
    }
    g_all_version.device.ver = read_val[0];
    g_all_version.device_rom.ver = read_val[1];
    return g_all_version.device.ver;
}

osal_u32 hmac_get_device_rom_version(osal_void)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    osal_u32 read_val[DEVICE_SIZE_OF_VERSION] = {0};
 
    if (g_all_version.device_rom.ver != 0) {
        return g_all_version.device_rom.ver;
    }
    cfg_msg_init(OSAL_NULL, 0, (osal_u8 *)&read_val, DEVICE_SIZE_OF_VERSION * sizeof(osal_u32), &msg_info);
    ret = send_cfg_to_device(0, WLAN_MSG_H2D_C_CFG_GET_VERSION, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "WLAN_MSG_H2D_C_CFG_GET_VERSION send to device failed!! ret=0x%x.", ret);
        return 0;
    }
    g_all_version.device.ver = read_val[0];
    g_all_version.device_rom.ver = read_val[1];
    return g_all_version.device_rom.ver;
}

