/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
  文 件 名   : hmac_ampdu_config.c
  生成日期   : 2022年9月21日
  功能描述   : AMPDU聚合、BA处理接口定义源文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_ampdu_config.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_tx_data.h"
#include "hmac_blockack.h"
#include "hmac_config.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_AMPDU_CONFIG_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数声明
*****************************************************************************/
/*****************************************************************************
  4 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hmac_config_auto_ba_switch_etc
 功能描述  : 设置amsdu+ampdu联合聚合的开关
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_amsdu_ampdu_switch_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_set_ampdu_amsdu_stru       *amsdu_ampdu_cfg;
    amsdu_ampdu_cfg = (mac_cfg_set_ampdu_amsdu_stru *)msg->data;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    g_st_tx_large_amsdu.cur_amsdu_enable = (osal_u8)amsdu_ampdu_cfg->tx_switch;
#endif
    mac_mib_set_AmsduPlusAmpduActive(hmac_vap, (osal_u8)amsdu_ampdu_cfg->tx_switch);
    hmac_vap->rx_ampduplusamsdu_active = (osal_u8)amsdu_ampdu_cfg->rx_switch;
    hmac_set_host_rx_ampdu_amsdu(hmac_vap->rx_ampduplusamsdu_active);
    oam_warning_log3(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_amsdu_ampdu_switch_etc::tx[%d] rx[%d][0:disable,1:enable].}",
        hmac_vap->vap_id, amsdu_ampdu_cfg->tx_switch, amsdu_ampdu_cfg->rx_switch);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_ampdu_tx_on_etc
 功能描述  : hmac设置ampdu tx 开关
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_ampdu_tx_on_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_ampdu_tx_on_param_stru *ampdu_tx_on;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on_etc:: param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ampdu_tx_on = (mac_cfg_ampdu_tx_on_param_stru *)msg->data;
    if ((hmac_vap->user_nums != 0) && ((ampdu_tx_on->aggr_tx_on & BIT1) != 0)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on_etc:: need to be set before assoc!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on_etc:: tx_aggr_on[0x%x], snd type[%d]!}",
        ampdu_tx_on->aggr_tx_on, ampdu_tx_on->snd_type);

    /* ampdu_tx_on为0、1,删建聚合 */
    if ((osal_u8)(ampdu_tx_on->aggr_tx_on & (~(BIT1 | BIT0))) == 0) {
        mac_mib_set_CfgAmpduTxAtive(hmac_vap, ampdu_tx_on->aggr_tx_on & BIT0);

        /* ampdu_tx_on为0b11切换硬件聚合 */
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        /* 切换为硬件聚合时才需要下发事件 */
        if ((ampdu_tx_on->aggr_tx_on & BIT1) != 0) {
            ampdu_tx_on->aggr_tx_on &= BIT0; /* enable hw ampdu */
            hmac_config_set_ampdu_tx_hw_on(hmac_vap, ampdu_tx_on);
        }
#endif
        return OAL_SUCC;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
/*****************************************************************************
 函 数 名  : hmac_config_set_ampdu_tx_hw_on
 功能描述  : 软硬件聚合切换
*****************************************************************************/
osal_s32 hmac_config_set_ampdu_tx_hw_on(hmac_vap_stru *hmac_vap, mac_cfg_ampdu_tx_on_param_stru *ampdu_tx_on)
{
    osal_s32 ret;

    if (ampdu_tx_on == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_set_ampdu_tx_hw_on::param is NULL!}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 聚合方式切换 */
    ret = (osal_s32)hmac_ba_tx_ampdu_switch(hmac_vap, ampdu_tx_on);
    oam_warning_log4(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_set_ampdu_tx_hw_on::enable[%d],snd partial[%d], ret[%d].}",
        hmac_vap->vap_id, ampdu_tx_on->aggr_tx_on, ampdu_tx_on->snd_type, ret);

    return ret;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

