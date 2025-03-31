/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: intrf_mode algorithm rom.
 */

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include "hmac_alg_if.h"
#include "alg_edca_opt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_EDCA_INTRF_MODE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/*****************************************************************************
  1 全局变量定义
 *****************************************************************************/
osal_u8 g_edca_hist_enable = OSAL_FALSE; /* 配置命令下发开关edca算法时记录原有值 */
osal_u8 g_edca_intrf_mode_switch = OSAL_FALSE; /* 当前是否处于EDCA抗干扰模式 */

/*****************************************************************************
  2 函数实现
****************************************************************************/
/******************************************************************************
 功能描述  : 干扰场景优化配置 - 重关联后处理
******************************************************************************/
OSAL_STATIC osal_u32 alg_edca_intrf_mode_process_etc(osal_void)
{
    wlan_wme_ac_type_enum_uint8 ac_type;

    hal_vap_set_machw_aifsn_all_ac(0, 0, 0, 0);
    alg_edca_print_aifsn_all_gla_info(0, 0, 0, 0);

    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hal_vap_set_edca_machw_cw(0, 0, ac_type);
        alg_edca_print_cw_gla_info(ac_type, 0, 0);
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 干扰场景优化配置
******************************************************************************/
osal_void alg_edca_intrf_mode_process(hmac_vap_stru *hmac_vap, osal_u8 edca_switch)
{
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u32 ret;

    hal_device = hal_chip_get_hal_device();
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void **)&edca_opt);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_INTRF_MODE, "{alg_edca_intrf_mode_process::get edca switch fail, params not set.}");
        return;
    }

    g_edca_intrf_mode_switch = edca_switch;

    if (edca_switch == OSAL_TRUE) {
        g_edca_hist_enable = edca_opt->edca_opt_en_sta;
        /* 进入干扰场景，关闭edca算法，并将edca寄存器参数置0 */
        if (edca_opt->edca_opt_en_sta == OSAL_TRUE) {
            edca_opt->edca_opt_en_sta = OSAL_FALSE;
        }
        alg_edca_intrf_mode_process_etc();
        oam_info_log1(0, OAM_SF_INTRF_MODE, "{alg_edca_intrf_mode_process::edca disabled[%d], params set to zero.}",
            edca_opt->edca_opt_en_sta);
    } else {
        /* 退出时重新开启edca算法，并恢复edca寄存器参数 */
        if (g_edca_hist_enable == OSAL_TRUE) {
            edca_opt->edca_opt_en_sta = OSAL_TRUE;
        }
        alg_edca_param_reset(hmac_vap, edca_opt);
        oam_info_log1(0, OAM_SF_INTRF_MODE, "{alg_edca_intrf_mode_process::edca re-enabled[%d], params restored.}",
            edca_opt->edca_opt_en_sta);
    }
}

/******************************************************************************
 功能描述  : 干扰场景优化配置 - 重关联后处理
******************************************************************************/
osal_u32 alg_edca_intrf_mode_process_assoc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    unref_param(hmac_vap);
    unref_param(hmac_user);

    if (g_edca_intrf_mode_switch == OSAL_FALSE) {
        return OAL_SUCC;
    }

    alg_edca_intrf_mode_process_etc();

    oam_info_log0(0, OAM_SF_INTRF_MODE, "{alg_edca_intrf_mode_process_assoc::edca set to zero after assoc.}");
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_EDCA_OPT */
#endif /* #ifdef _PRE_WLAN_FEATURE_INTRF_MODE */