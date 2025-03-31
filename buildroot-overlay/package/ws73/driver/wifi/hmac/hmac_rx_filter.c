/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_rx_filter.c
 * 生成日期   : 2014年7月14日
 * 功能描述   : 帧过滤处理文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_rx_filter.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_RX_FILTER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_calc_up_ap_num_etc
 功能描述  : 计算不处于inti状态的VAP个数
*****************************************************************************/
osal_u32 hmac_calc_up_ap_num_etc(hmac_device_stru *hmac_device)
{
    hmac_vap_stru                  *hmac_vap;
    osal_u8                      vap_idx;
    osal_u8                      up_ap_num = 0;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_calc_up_ap_num_etc::hmac_vap null,vap_idx=%d.}",
                           hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((hmac_vap->vap_state != MAC_VAP_STATE_INIT) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            up_ap_num++;
        } else if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (hmac_vap->vap_state == MAC_VAP_STATE_UP)) {
            up_ap_num++;
        }
    }

    return up_ap_num;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

