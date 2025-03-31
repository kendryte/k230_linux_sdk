/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: intrf_mode algorithm rom.
 */

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include "hal_phy.h"
#include "alg_cca_optimize.h"
#include "alg_edca_opt.h"
#include "alg_intf_det.h"
#include "hmac_alg_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_INTRF_MODE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  1 全局变量定义
 *****************************************************************************/
osal_u8 g_intrf_mode_notch_ch = 0; /* 底噪规避信道 */

/*****************************************************************************
  2 函数声明
****************************************************************************/
__attribute__((weak)) osal_void alg_edca_intrf_mode_process(hmac_vap_stru *hmac_vap, osal_u8 edca_switch);
__attribute__((weak)) osal_void alg_cca_set_intrf_mode_switch(osal_u8 cca_switch);

OAL_STATIC osal_s32 alg_intrf_mode_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 alg_intrf_mode_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 alg_intrf_mode_todmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
OAL_STATIC osal_void alg_intrf_mode_chk_to_do_sgl_tone_notch(hmac_vap_stru *hmac_vap, osal_u8 switch_val);

/*****************************************************************************
  3 函数实现
****************************************************************************/
/******************************************************************************
 功能描述  : 干扰场景优化算法参数配置
******************************************************************************/
osal_s32 alg_intrf_mode_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_intrf_mode_param_stru *intrf_mode;
    intrf_mode = (mac_ioctl_alg_intrf_mode_param_stru *)msg->data;

    oam_warning_log2(0, OAM_SF_ANY, "{alg_intrf_mode_param::[host]intrf_mode_cfg:%d, new param:%d.\r\n",
        intrf_mode->alg_intrf_mode_cfg, intrf_mode->value);

    if ((intrf_mode->value != OSAL_TRUE) && (intrf_mode->value != OSAL_FALSE)) {
        oam_warning_log1(0, OAM_SF_ANY, "{alg_intrf_mode_param::invalid switch value [%d]!}", intrf_mode->value);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
    /* edca_switch配置处理 */
    if (intrf_mode->alg_intrf_mode_cfg == MAC_ALG_INTRF_MODE_EDCA_SWITCH) {
        alg_edca_intrf_mode_process(hmac_vap, (osal_u8)intrf_mode->value);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_CCA_OPT
    /* cca_switch配置处理 */
    if (intrf_mode->alg_intrf_mode_cfg == MAC_ALG_INTRF_MODE_CCA_SWITCH) {
        alg_cca_set_intrf_mode_switch((osal_u8)intrf_mode->value);
    }
#endif

    /* long_range_intrf_switch配置处理 */
    if (intrf_mode->alg_intrf_mode_cfg == MAC_ALG_INTRF_MODE_LONG_RANGE_INTRF_SWITCH) {
        alg_intrf_mode_chk_to_do_sgl_tone_notch(hmac_vap, (osal_u8)intrf_mode->value);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 干扰场景优化配置
*****************************************************************************/
osal_s32 alg_intrf_mode_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    ret = alg_intrf_mode_param(hmac_vap, msg);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intrf_mode_config_param::alg_intrf_mode_param failed!}");
        return ret;
    }

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_CFG_ALG_INTRF_MODE_CFG, msg, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intrf_mode_config_param::frw_send_msg_to_device failed!}");
        return ret;
    }
    return OAL_SUCC;
}
osal_s32 alg_intrf_mode_todmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_CFG_ALG_INTRF_MODE_TOTAL_CFG, msg, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intrf_mode_todmac_config_param::frw_send_msg_to_device failed!}");
        return ret;
    }
    return OAL_SUCC;
}
/*****************************************************************************
功能描述  : 特殊干扰场景底噪规避
*****************************************************************************/
osal_void alg_intrf_mode_chk_to_do_sgl_tone_notch(hmac_vap_stru *hmac_vap, osal_u8 switch_val)
{
    osal_u8 ch;
    osal_bool target_ch = OSAL_FALSE;
    osal_bool need_write = OSAL_FALSE;
    osal_u32 notch_filter_coef1[] = {0xd603fa99, 0, 0xdf93cf24, 0xdf93f221, 0xdf93ea42, 0xdf93c76b, 0,
        0xdf93d2e1, 0xdf93de18, 0xdf93e26e, 0xdf93bfb5};
    osal_u32 notch_filter_coef2[] = {0x00059f05, 0, 0x3731b, 0x77f08, 0x6fb0c, 0x2571e, 0,
        0x47f17, 0x7a312, 0x64b10, 0x12f22};
    osal_u32 sgl_tone_0_2_car_and_en[] = {0x80808000, 0, 0x16171801, 0x6070801, 0x8a898801, 0x9a999801, 0,
        0x13141501, 0x3040501, 0x8d8c8b01, 0x9d9c9b01};
    osal_u32 sgl_tone_3_car[] = {0x808080, 0, 0x808015, 0x808005, 0x80808b, 0x80809b, 0,
        0x808012, 0x808020, 0x80808e, 0x80809e};
    osal_u32 sgl_tone_0_7_weight[] = {0, 0, 0x804, 0x804, 0x804, 0x804, 0, 0x804, 0x38, 0x804, 0x804};

    ch = hmac_vap->channel.chan_number;
    target_ch = !((ch == 1) || (ch == 6) || (ch > 10)); /* 1 6 10:ch */
    /* update status */
    if ((switch_val == OSAL_TRUE) && (target_ch == OSAL_TRUE) && (g_intrf_mode_notch_ch != ch)) {
        g_intrf_mode_notch_ch = ch;
        need_write = OSAL_TRUE;
    } else if (((switch_val == OSAL_FALSE) || (target_ch == OSAL_FALSE)) && (g_intrf_mode_notch_ch != 0)) {
        /* restore */
        g_intrf_mode_notch_ch = 0;
        need_write = OSAL_TRUE;
    }

    /* write regs */
    if (need_write == OSAL_TRUE) {
        hal_phy_do_sgl_tone_notch_coef(notch_filter_coef1[g_intrf_mode_notch_ch],
            notch_filter_coef2[g_intrf_mode_notch_ch], sgl_tone_0_2_car_and_en[g_intrf_mode_notch_ch],
            sgl_tone_3_car[g_intrf_mode_notch_ch]);
        hal_phy_do_sgl_tone_notch_weight(sgl_tone_0_7_weight[g_intrf_mode_notch_ch], g_intrf_mode_notch_ch);
    }

    oam_warning_log3(0, OAM_SF_ANY,
        "{alg_intrf_mode_chk_to_do_sgl_tone_notch::switch[%d] need_write[%d] g_intrf_mode_notch_ch[%d]\r\n",
        switch_val, need_write, g_intrf_mode_notch_ch);
}

__attribute__((weak)) osal_void alg_edca_intrf_mode_process(hmac_vap_stru *hmac_vap, osal_u8 edca_switch)
{
    unref_param(hmac_vap);
    unref_param(edca_switch);
    return;
}
__attribute__((weak)) osal_void alg_cca_set_intrf_mode_switch(osal_u8 cca_switch)
{
    unref_param(cca_switch);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_INTRF_MODE */