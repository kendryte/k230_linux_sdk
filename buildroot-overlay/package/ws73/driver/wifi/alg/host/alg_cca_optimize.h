/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm cca optimaize
 */

#ifndef __ALG_CCA_OPTIMIZE_H__
#define __ALG_CCA_OPTIMIZE_H__

#ifdef _PRE_WLAN_FEATURE_CCA_OPT

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "oal_ext_if.h"
#include "hmac_vap.h"
#include "alg_common_rom.h"
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#include "alg_intf_det.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_CCA_OPT_ED_HIGH_20TH_LOW_TH (-82)
/******************************************************************************
  3 STRUCT定义
******************************************************************************/
/* device下CCA优化特性对应的信息结构体 */
typedef struct {
    oal_bool_enum_uint8 cca_opt_en;                         /* cca优化使能 */
    oal_bool_enum_uint8 cca_opt_debug_mode;                 /* cca优化DEBUG模式启动 */
    oal_bool_enum_uint8 log_flag;                           /* cca优化log使能 */
    osal_u8 high_intf_flag;                                 /* cca是否进入高底噪场景标记 */

    osal_u8 ed_high_20th_reg_adj_debug; /* CCA 20M门限调节增大量 */
    osal_s8 sd_cca_20th_dsss;           /* CCA DSSS协议门限 */
    osal_s8 sd_cca_20th_ofdm;           /* CCA OFDM协议门限 */
    osal_s8 ed_high_20th_reg;           /* CCA门限寄存器值 */
    osal_s8 ed_high_40th_reg;           /* CCA门限寄存器值 */
    osal_u8 is_cca_opt_registered;      /* CCA算法是否注册 */
    osal_u8 high_intf_enter_cnt;        /* 进入高底噪判断统计 */
    osal_u8 high_intf_exit_cnt;         /* 退出高底噪判断统计 */
    osal_s16 rx_data_rssi;              /* 保存收到的数据帧的rssi */
    osal_s16 rx_mgmt_rssi;              /* 保存收到的管理帧帧的rssi */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s8 ed_high_20th_reg_default; /* 默认CCA门限寄存器值，每次入网时根据定制化δ调整值与band更新此值 */
    osal_s8 ed_high_40th_reg_default; /* 默认CCA门限寄存器值，每次入网时根据定制化δ调整值与band更新此值 */
    osal_u8 resv2[2]; /* 2:数组大小 */
#endif
} alg_cca_opt_stru;
/******************************************************************************
  4 函数声明
******************************************************************************/
osal_s32 alg_cca_opt_init(osal_void);
osal_void alg_cca_opt_exit(osal_void);
osal_void alg_cca_opt_set_th_default(alg_cca_opt_stru *cca_opt, wlan_channel_band_enum_uint8 freq_band);
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
osal_void alg_cca_set_intrf_mode_switch(osal_u8 cca_switch);
osal_u32 alg_cca_intrf_mode_process(osal_void);
#endif
alg_cca_opt_stru *alg_cca_get_cca_opt(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_CCA_OPT */
#endif /* end of alg_cca_optimize.h */
