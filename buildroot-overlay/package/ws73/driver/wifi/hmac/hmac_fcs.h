/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_fcs_rom.c 的头文件。
 * Create: 2020-07-03
 */

#ifndef __HMAC_FCS_H__
#define __HMAC_FCS_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* OFDM 12M: 0x004a0113 6M: 0x004b0113  11b 1M:  0x08000113 */
#define WLAN_PROT_DATARATE_CHN0_1M (0x08000113)
#define WLAN_PROT_DATARATE_CHN1_1M (0x08000123)
#define WLAN_PROT_DATARATE_CHN0_6M (0x004b0113)
#define WLAN_PROT_DATARATE_CHN1_6M (0x004b0123)
#define WLAN_PROT_DATARATE_CHN0_12M (0x004a0113)
#define WLAN_PROT_DATARATE_CHN1_12M (0x004a0123)
#define WLAN_PROT_DATARATE_CHN0_24M (0x00490113)
#define WLAN_PROT_DATARATE_CHN1_24M (0x00490123)

/*****************************************************************************
  10 函数声明
*****************************************************************************/

osal_void hmac_fcs_start_enhanced(mac_fcs_cfg_stru *fcs_cfg);
osal_void hmac_fcs_start(mac_fcs_cfg_stru *fcs_cfg);
osal_void hmac_fcs_start_same_channel(mac_fcs_cfg_stru *fcs_cfg);
osal_void hmac_fcs_start_enhanced_same_channel(mac_fcs_cfg_stru *fcs_cfg);
osal_void hmac_fcs_start_send_one_packet(mac_fcs_cfg_stru *fcs_cfg);
osal_u32 hmac_fcs_set_prot_datarate(const hmac_vap_stru *src_vap);
osal_void hmac_fcs_prepare_one_packet_cfg(const hmac_vap_stru *hmac_vap, hal_one_packet_cfg_stru *one_packet_cfg,
    osal_u16 protect_time);
osal_u32 hmac_fcs_get_prot_mode(const hmac_vap_stru *hmac_vap, const hal_one_packet_cfg_stru *one_packet_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_fcs.h */
