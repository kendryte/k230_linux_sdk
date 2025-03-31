/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize rssi comp 头文件
 */
#ifndef _FE_RF_CUSTOMIZE_RSSI_COMP_H_
#define _FE_RF_CUSTOMIZE_RSSI_COMP_H_

#include "osal_types.h"
#include "wlan_msg.h"
#include "dmac_ext_if_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define FE_CUST_RSSI_COMP_2G_NUM  3  /* RSSI数量 与band划分对应 */
typedef struct {
    osal_s8 rssi_comp[FE_CUST_RSSI_COMP_2G_NUM];
    osal_u8 select_subband; /* 设置或获取rssi_comp对应的subband标志位 */
} fe_custom_rssi_comp_2g_stru;

typedef struct {
    fe_custom_rssi_comp_2g_stru rssi_comp_2g;
} fe_custom_rssi_stru;

osal_s32 fe_custom_get_rssi_comp(osal_u8 band, osal_s8 *rssi_comp, osal_u8 subband);
osal_s32 fe_custom_process_rssi_comp(dmac_vap_stru *dmac_vap, frw_msg *msg);
#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // _FE_RF_CUSTOMIZE_RSSI_COMP_H_