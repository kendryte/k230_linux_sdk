/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: sta ps_poll低功耗功能所在函数
 * Date: 2020-07-14
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_psm_sta.h"
#include "hmac_sta_pm.h"
#include "hmac_beacon.h"
#include "hmac_uapsd.h"
#include "hal_ext_if.h"
#include "msg_psm_rom.h"
#include "hal_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_PSM_STA_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
/****************************************************************************
 函 数 名  : hmac_bcn_no_frm_event_hander
 功能描述  : mac解析无缓存帧中断处理事件
*****************************************************************************/
osal_s32 hmac_bcn_no_frm_event_hander(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);
    /* linkloss计数清0,门限+1 */
    hmac_vap_linkloss_clean(hmac_vap);

    return OAL_SUCC;
}

osal_u32 hmac_psm_mask_mac_parse_tim(hmac_vap_stru *hmac_vap, osal_u8 mac_parse_tim_switch)
{
    frw_msg msg2device = {0};
    osal_u32 ret;
    osal_u8 parse_tim_switch;

    parse_tim_switch = mac_parse_tim_switch;
    frw_msg_init((osal_u8 *)&parse_tim_switch, sizeof(parse_tim_switch), OSAL_NULL, 0, &msg2device);

    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_MASK_MAC_PARSE_TIM, &msg2device,
        OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_PWR, "{hmac_mask_mac_parse_tim:: send mac parse tim to device fail [%d]!}", ret);
    }
    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
/*****************************************************************************
 函 数 名  : hmac_psm_update_bcn_rf_chain
 功能描述  : 根据rssi更新唤醒后收beacon的通道
*****************************************************************************/
osal_void hmac_psm_update_bcn_rf_chain(const hmac_vap_stru *hmac_vap, osal_char rssi)
{
    hal_to_dmac_vap_stru *hal_vap = hmac_vap->hal_vap;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u8 up_vap_num;
    osal_u8 bcn_rf_chain;

    if ((hal_vap == OSAL_NULL) || (hal_device == OSAL_NULL)) {
        return;
    }

    /* 单通道收bcn开关关闭时不更新bcn rx chain */
    if (hal_device->srb_switch == OSAL_FALSE) {
        return;
    }

    up_vap_num = hal_device_calc_up_vap_num(hal_device);
    if (up_vap_num == 0) {
        oam_error_log1(0, 0, "vap_id[%d] hmac_psm_update_bcn_rx_chain:no vap up", hmac_vap->vap_id);
        return;
    }

    if (up_vap_num == 1) {
        /* rssi大于门限时使用用主通道接收beacon，否则使用双通道接收 */
        bcn_rf_chain = (rssi > HMAC_PSM_SINGLE_BCN_RX_CHAIN_TH) ? hal_device->cfg_cap_info->single_tx_chain :
            hal_device->cfg_cap_info->rf_chain;
    } else {
        /* 非单sta场景，使用phy chain作为bcn rx chain */
        bcn_rf_chain = hal_device->cfg_cap_info->rf_chain;
    }

    if (bcn_rf_chain != hal_vap->pm_info.bcn_rf_chain) {
        hal_pm_set_bcn_rf_chain(hal_vap, bcn_rf_chain);
    }
}
#endif

osal_u32 hmac_set_psm_param_to_device(const hmac_vap_stru *hmac_vap,
    osal_u8 is_dtim3_on, osal_u8 dtim3_on, osal_u8 is_keep_alive, osal_u8 is_all_vap)
{
    frw_msg msg2device = {0};
    mac_psm_sta_pm_param_dev_stru pm_cfg = {0};
    osal_u32 ret;

    pm_cfg.is_max_power_set = (is_dtim3_on == OSAL_TRUE ? 1 : 0);
    pm_cfg.max_power_set = dtim3_on;
    pm_cfg.is_keep_alive_set = (is_keep_alive == OSAL_TRUE ? 1 : 0);
    pm_cfg.is_all_vap = (is_all_vap == OSAL_TRUE ? 1 : 0);
    pm_cfg.in_tbtt_offset = hmac_vap->in_tbtt_offset;
    frw_msg_init((osal_u8 *)&pm_cfg, sizeof(pm_cfg), OSAL_NULL, 0, &msg2device);

    /* snyc ps config to device */
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SET_PSM_PARAM, &msg2device,
        OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_PWR, "{hmac_set_psm_param_to_device:: send pm cfg to device fail [%d]!}", ret);
    }
    return ret;
}

osal_void hmac_psm_max_ps_en_sync(const hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    hmac_vap_stru *up_vap = OSAL_NULL;
    osal_u8 max_powersave = OSAL_FALSE;

    if (hmac_vap == OSAL_NULL || hmac_device == OSAL_NULL) {
        return;
    }

    oam_warning_log1(0, OAM_SF_PWR, "{hmac_psm_max_ps_en_sync : enable[%d]}", hmac_device->in_suspend);
    max_powersave = (hmac_device->in_suspend == 0) ? 0 : 1;

    if (mac_device_find_up_vap_etc(hmac_device, &up_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_psm_max_ps_en_sync : no up vap found}");
        return;
    }
    /* sync pm cfg to device */
    (osal_void)hmac_set_psm_param_to_device(up_vap, OAL_TRUE, max_powersave, OAL_TRUE, OAL_TRUE);
    return;
}

osal_s32 hmac_config_sta_psm_status_sync(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_ps_open_stru *ps_open = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_sta_psm_status_sync:: hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ps_open = (mac_cfg_ps_open_stru *)msg->data;
    hmac_set_sta_pm_on_cb(hmac_vap, ps_open);
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 低功耗参数配置
*****************************************************************************/
osal_s32 hmac_set_psm_param(hmac_vap_stru *hmac_vap, mac_cfg_ps_param_stru *ps_param)
{
    frw_msg msg2device = {0};
    osal_u16 beacon_timeout;
    osal_s32 ret;

    beacon_timeout = ps_param->beacon_timeout;
    /* 若 BCN_TIMEOUT 配置参数在允许范围内     ，才可配置 */
    if (beacon_timeout <= HMAC_DEF_BCN_TIMEOUT_MAX && beacon_timeout >= HMAC_DEF_BCN_TIMEOUT_MIN) {
        hmac_vap->beacon_timeout = beacon_timeout;
        hal_set_beacon_timeout(beacon_timeout);
    } else {
        ps_param->beacon_timeout = 0;
    }

    /* INTER TBTT OFFSET */
    if (ps_param->tbtt_offset != 0) {
        hmac_vap->in_tbtt_offset = ps_param->tbtt_offset;
        hal_set_tbtt_offset(hmac_vap->hal_vap, ps_param->tbtt_offset);

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
        if (hmac_vap->hal_vap == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_PWR, "{hmac_set_psm_param:: hal_vap is null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        if (hmac_vap->hal_vap->pm_info.bcn_rf_chain == WLAN_PHY_CHAIN_DOUBLE) {
            hmac_vap->hal_vap->pm_info.inner_tbtt_offset_mimo = ps_param->tbtt_offset;
        } else {
            hmac_vap->hal_vap->pm_info.inner_tbtt_offset_siso = ps_param->tbtt_offset;
        }

        hal_init_pm_info_sync(hmac_vap->hal_vap);
#endif
    }
    oam_warning_log4(0, OAM_SF_PWR,
        "{hmac_set_psm_param: pm_timer [%d]ms, pm_timeout_cnt [%d], mcast_timeout [%d], max_sleep_time[%d].}",
        ps_param->pm_timer, ps_param->pm_timer_cnt, ps_param->mcast_timeout, ps_param->sleep_time);

    frw_msg_init((osal_u8 *)ps_param, sizeof(mac_cfg_ps_param_stru), OSAL_NULL, 0, &msg2device);

    /* snyc ps_param config to device */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SET_STA_PM_PARAM, &msg2device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_PWR, "{hmac_set_psm_param:: send pm cfg to device fail [%d]!}", ret);
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

osal_s32 hmac_config_set_sta_pm_param_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_ps_param_stru *sta_pm_param = OSAL_NULL;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL))) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_set_sta_pm_param_cb::hmac_vap / param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    sta_pm_param = (mac_cfg_ps_param_stru *)msg->data;

    return hmac_set_psm_param(hmac_vap, sta_pm_param);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
