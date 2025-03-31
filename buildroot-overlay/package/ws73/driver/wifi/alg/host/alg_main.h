/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description:  Entry, initialization, and external interface definition of the algorithm module, depending on the DMAC
 */

#ifndef __ALG_MAIN_H__
#define __ALG_MAIN_H__

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_alg_if.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "alg_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_RSSI_MARGIN_DB 4                   /* RSSI余量，使用rssi计算时使用 */

/******************************************************************************
  3 枚举定义
******************************************************************************/
/******************************************************************************
  4 STRUCT定义
******************************************************************************/
/* 钩子函数指针数组结构体 */
typedef struct {
    p_alg_distance_notify_func pa_distance_notify_func[ALG_DISTANCE_NOTIFY_BUTT];
    p_alg_intf_det_pk_mode_notify_func cfg_intf_det_pk_mode_notify_func;
    p_alg_intf_det_edca_notify_func cfg_intf_det_edca_notify_func;
    p_alg_intf_det_cca_notify_func cfg_intf_det_cca_notify_func;
    p_alg_intf_det_cca_chk_noise_notify_func cfg_intf_det_cca_chk_noise_notify_func;
} alg_internal_hook_stru;
/******************************************************************************
  5 函数声明
******************************************************************************/
alg_internal_hook_stru* hmac_alg_get_notify_if(osal_void);
osal_void *alg_mem_alloc(alg_mem_enum_uint8 alg_id, osal_u16 len);
osal_void alg_hmac_main_init(osal_void);
osal_void alg_hmac_main_exit(osal_void);
osal_u32 hmac_alg_cfg_intf_det_pk_mode_notify(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 is_pk_mode);
osal_u32 hmac_alg_cfg_intf_det_edca_notify(osal_void);
osal_u32 hmac_alg_cfg_intf_det_cca_notify(hal_to_dmac_device_stru *hal_device, osal_u8 old_intf_mode,
    osal_u8 cur_intf_mode);
osal_u32 hmac_alg_cfg_intf_det_cca_chk_noise_notify(osal_s8 avg_rssi_20, oal_bool_enum_uint8 coch_intf_state_sta,
    osal_u8 cur_intf);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_main.h */
