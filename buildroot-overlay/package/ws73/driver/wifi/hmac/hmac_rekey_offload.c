/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_rekey_offload.c
 * 生成日期   : 2022年10月14日
 * 功能描述   : REKEY Offloading相关函数实现
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_rekey_offload.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "wlan_msg.h"
#include "oal_netbuf_data.h"
#include "msg_rekey_offload_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_REKEY_OFFLOAD_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD

mac_rekey_offload_stru g_rekey_offload = {0};
osal_u8 g_rekey_info_exist = OAL_FALSE; /* rekeyinfo是否已经被下发到DMAC */

osal_void hmac_rekey_offload_get_param(mac_rekey_offload_param_sync *rekey_param)
{
    memcpy_s(&rekey_param->rekey_offload, sizeof(mac_rekey_offload_stru), &g_rekey_offload,
        sizeof(mac_rekey_offload_stru));
    rekey_param->rekey_info_exist = g_rekey_info_exist;
}

osal_s32 hmac_config_set_rekey_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    if (msg->data_len != sizeof(mac_rekey_offload_stru)) {
        oam_error_log1(0, OAM_SF_WPA,
            "{hmac_config_set_rekey_info: msg len [%d] is not sizeof[mac_rekey_offload_stru].}", msg->data_len);
        return OAL_FAIL;
    }
    /* 全局变量下只支持单sta rekey, 多个vap rekey全局变量需移至dmac user结构体中 */
    g_rekey_info_exist = OAL_TRUE;
    if (msg->data != OSAL_NULL) {
        if (memcpy_s(&g_rekey_offload, sizeof(mac_rekey_offload_stru), msg->data, sizeof(mac_rekey_offload_stru))
            != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_rekey_info:: param memcpy_s fail.");
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

osal_u32 hmac_rekey_offload_init(osal_void)
{
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_SET_REKEY, hmac_config_set_rekey_info);

    return OAL_SUCC;
}

osal_void hmac_rekey_offload_deinit(osal_void)
{
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_SET_REKEY);
}

#endif
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

