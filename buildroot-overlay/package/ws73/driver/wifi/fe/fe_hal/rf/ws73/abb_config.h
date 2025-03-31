/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: header file for initialization for CMU RF CLK.
 */

#ifndef __ABB_CONFIG_H__
#define __ABB_CONFIG_H__
/*****************************************************************************
  头文件包含
*****************************************************************************/
#include "fe_rf_dev_attach.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef enum {
    HAL_CMU_WF_TRX_CLK_320M,
    HAL_CMU_WF_TRX_CLK_160M,
    HAL_CMU_WF_TRX_CLK_80M,
    HAL_CMU_WF_TRX_CLK_32M,
    HAL_CMU_WF_TRX_CLK_BUTT
} hal_cmu_wf_tx_clk_enum;
/*****************************************************************************
  函数声明
*****************************************************************************/
osal_void wlan_open_wifi_abb_rf_clk(osal_u8 tx_clk, osal_u8 rx_clk);
osal_void hal_update_wifi_abb_rf_clk(wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void wlan_close_wifi_abb_rf_clk(osal_void);
osal_void wlan_config_coex_ctl_sel(osal_u8 is_manual);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
