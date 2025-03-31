/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 * Date: 2022-10-19
 */

#ifndef __FE_POWER_HOST_H__
#define __FE_POWER_HOST_H__
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "fe_hal_phy_reg_if_host.h"
#if defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA)
#include "hal_device_rom.h"
#else
#include "hal_device.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_ONLINE_CALI
// 动态功率
osal_void fe_hal_phy_config_online_pow(const hal_device_stru *device);
osal_void fe_hal_phy_recover_online_pow(const hal_device_stru *device);
#endif
// 二级表
osal_u32 fe_hal_phy_get_single_ppa_lut(const hal_device_stru *device, osal_u16 index);
osal_void fe_hal_phy_set_single_ppa_lut(const hal_device_stru *device, osal_u16 upc_code, osal_u16 index);
osal_void fe_hal_phy_set_single_pa_lut(const hal_device_stru *device, osal_u16 upc_code, osal_u16 index);
osal_void fe_hal_phy_set_init_pa_code(hal_device_stru *device, wlan_channel_band_enum_uint8 band);
// 一级表
osal_void fe_hal_pow_write_single_tpc_mem(hal_device_stru *device, osal_u32 tpc_idx,
    osal_u32 pow_dist, oal_bool_enum_uint8 is_dsss);
osal_void fe_hal_pow_write_tpc_mem_prepare(void);
osal_void fe_hal_pow_write_tpc_mem_finish(void);

// dbb
osal_void fe_hal_cfg_tpc_boundary(const hal_device_stru *device, osal_u8 value, osal_u8 index);
osal_void fe_hal_init_dbb_scale_shadow_to_0x80(hal_device_stru *device);
osal_void fe_hal_cfg_dbb_scale_shadow(hal_device_stru *device, osal_u8 value, osal_u8 index);
osal_u32 fe_hal_get_dbb_scale_shadow(hal_device_stru *device, osal_u8 index);

osal_s16 fe_hal_rf_he_er_su_comp(osal_u8 protocol_mode);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_PHY_IF_H__