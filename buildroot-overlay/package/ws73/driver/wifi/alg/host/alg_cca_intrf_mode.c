/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: intrf_mode algorithm rom.
 */

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
#ifdef _PRE_WLAN_FEATURE_CCA_OPT

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include "hmac_alg_if.h"
#include "alg_cca_optimize.h"
#include "alg_intf_det.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_CCA_INTRF_MODE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  1 全局变量定义
 *****************************************************************************/
osal_u8 g_cca_intrf_mode_switch = OSAL_FALSE;      /* 特殊场景CCA配置开关 */
osal_u8 g_cca_intrf_mode_hist_enable = OSAL_FALSE; /* 特殊场景CCA配置是否开启的历史状态 */

/*****************************************************************************
  2 函数实现
****************************************************************************/
#define ALG_CCA_OPT_ED_HIGH_20TH_LOW_INTRF_TH (-10)    /* CCA 20M检测门限在特殊干扰场景下的门限值 */

/*****************************************************************************
  3 宏定义
****************************************************************************/
/******************************************************************************
 功能描述  : 干扰场景优化配置
******************************************************************************/
osal_void alg_cca_set_intrf_mode_switch(osal_u8 cca_switch)
{
    g_cca_intrf_mode_switch = cca_switch;
    alg_cca_intrf_mode_process();
}

/******************************************************************************
 功能描述  : 干扰场景优化配置
******************************************************************************/
osal_u32 alg_cca_intrf_mode_process(osal_void)
{
    alg_cca_opt_stru *cca_opt = OSAL_NULL;

    cca_opt = alg_cca_get_cca_opt();

    /* 优化配置关闭时不做处理 */
    if (g_cca_intrf_mode_switch == OSAL_FALSE) {
        if (g_cca_intrf_mode_hist_enable == OSAL_TRUE) {
            /* 恢复CCA参数 */
            g_cca_intrf_mode_hist_enable = OSAL_FALSE;
            cca_opt->cca_opt_en = WLAN_CCA_OPT_ENABLE;
            alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);
            hal_set_ed_high_th(cca_opt->ed_high_20th_reg, cca_opt->ed_high_40th_reg, OSAL_TRUE);
            hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);
        }
        return OAL_SUCC;
    }

    if (g_cca_intrf_mode_hist_enable == OSAL_TRUE) {
        return OAL_SUCC;
    }

    g_cca_intrf_mode_hist_enable = OSAL_TRUE;
    cca_opt->cca_opt_en = WLAN_CCA_OPT_DISABLE;
    alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);
    hal_set_ed_high_th(ALG_CCA_OPT_ED_HIGH_20TH_LOW_INTRF_TH, ALG_CCA_OPT_ED_HIGH_20TH_LOW_INTRF_TH, OSAL_TRUE);
    hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_CCA_OPT */
#endif /* #ifdef _PRE_WLAN_FEATURE_INTRF_MODE */