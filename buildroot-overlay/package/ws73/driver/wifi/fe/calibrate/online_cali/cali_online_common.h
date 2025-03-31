/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: cali_online_common.c的头文件
 */

#ifndef __CALI_ONLINE_COMMON_H__
#define __CALI_ONLINE_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define CALI_DC_OFFSET_NUM      2
#define CALI_POW_PRECISION_SHIFT       (10)     /* TPC算法中功率的精度 */
#define CALI_WAIT_PPA_LUT_TIMEOUT      (1000)  /* 1 ms */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* pwr tx notify发送到device侧的数据结构 */
typedef struct {
    osal_u8 enable_online_cali_mask;
    osal_u8 interval_available : 4;
    osal_u8 cali_band          : 4;
    osal_u16 aus_cali_en_interval;
} online_cali_pow_notify_para_stru;

typedef osal_u8 wlan_phy_protocol_mode_enum_uint8;
typedef struct {
    osal_s32                        pdet_val;
    oal_bool_enum_uint8             pdet_enable;
    oal_bool_enum_uint8             invalid;
    osal_u8                         chain_idx; /* 动态功率校准通道 */
    wlan_bw_cap_enum_uint8          band_width;

    wlan_channel_band_enum_uint8 freq;
    wlan_phy_protocol_mode_enum_uint8  cur_protocol;
    osal_u8 upc_idx;
    osal_u8 rate_idx;

    osal_u8 chain_selection;
    osal_u8 al_tx_flag;
    osal_u8 tpc_code;
    osal_u8 rsv;

    mac_channel_stru channel_info;
    osal_u32 pwr_dist;
    osal_s16 dc_offset[CALI_DC_OFFSET_NUM];
} online_cali_para_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 cali_wait_ppa_lut_free(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif