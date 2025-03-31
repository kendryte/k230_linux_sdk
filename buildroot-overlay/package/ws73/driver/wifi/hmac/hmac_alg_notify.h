/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: header file of hmac_alg_notify.c.
 * Create: 2020-7-5
 */

#ifndef __HMAC_ALG_NOTIFY_H__
#define __HMAC_ALG_NOTIFY_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_alg_if.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif
#include "hmac_thruput_test.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_alg_cfg_channel_notify(hmac_vap_stru *hmac_vap, mac_alg_channel_bw_chg_type_uint8 type);

osal_u32 hmac_alg_cfg_btcoex_state_notify(hal_to_dmac_device_stru *hal_device, hmac_alg_bt_aggr_time_uint8 type);

osal_u32 hmac_alg_add_assoc_user_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_alg_del_assoc_user_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_alg_rx_mgmt_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf);
osal_u32 hmac_alg_rx_cntl_notify(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const oal_netbuf_stru *buf);

osal_void *hmac_alg_get_algorithm_main(osal_void);
osal_u32 hmac_scan_param_sync(const mac_scan_req_stru *scan_req_params);
osal_u32 hmac_alg_scan_ch_complete_notify(osal_void *param);
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
osal_u32 hmac_alg_anti_intf_switch(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 alg_enable);
#endif
osal_u32 hmac_alg_anti_intf_tbtt_handler(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device);

#ifdef _PRE_WLAN_FEATURE_DBAC
osal_s32 hmac_alg_dbac_pause(hmac_vap_stru *hmac_vap);
osal_s32 hmac_alg_dbac_send_disassoc(hmac_vap_stru *hmac_vap, osal_u8 is_send);
osal_s32 hmac_alg_dbac_resume(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 is_resume_channel);
oal_bool_enum_uint8 hmac_alg_dbac_is_pause(hal_to_dmac_device_stru *hal_device);
osal_s32 hmac_alg_update_dbac_fcs_config(hmac_vap_stru *hmac_vap);
#endif
osal_void hmac_alg_vap_up_hmac_to_dmac(hmac_vap_stru *mac_vap);

/*****************************************************************************
 函 数 名  : hmac_alg_tx_schedule_notify
 功能描述  : 调用挂接到TX COMPLETE流程上的算法钩子
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_tx_schedule_notify(const hal_to_dmac_device_stru *hal_device, osal_u8 ac_num,
    mac_tid_schedule_output_stru *sch_output)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    if (osal_unlikely(hal_device == OSAL_NULL || sch_output == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();
    if (osal_unlikely(alg_stru->tx_schedule_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ac_num >= HAL_TX_QUEUE_NUM) {
        return OAL_FAIL;
    }

    return alg_stru->tx_schedule_func(hal_device, ac_num, sch_output);
}
/*****************************************************************************
 函 数 名  : hmac_alg_tx_schedule_timer_notify
 功能描述  : 通知调度算法更新调度灌包timer
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_tx_schedule_timer_notify(const hal_to_dmac_device_stru *hal_device)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();
    if (osal_unlikely(alg_stru->tx_schedule_timer_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return alg_stru->tx_schedule_timer_func(hal_device);
}
/*****************************************************************************
 函 数 名  : hmac_alg_user_info_update_notify
 功能描述  : 更新user info 时回调算法
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_user_info_update_notify(const hal_to_dmac_device_stru *hal_device,
    hmac_user_stru *hmac_user, mac_tid_schedule_output_stru schedule_ouput, osal_u8 device_mpdu_full)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    if (osal_unlikely(hal_device == OSAL_NULL || hmac_user == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();
    if (osal_unlikely(alg_stru->tid_update_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return alg_stru->user_info_update_func(hal_device, hmac_user, schedule_ouput, device_mpdu_full);
}
/*****************************************************************************
 函 数 名  : hmac_alg_tid_update_notify
 功能描述  : 更新TID队列时回调算法
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_tid_update_notify(hmac_tid_stru *tid, osal_u8 in_mpdu_num)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    if (osal_unlikely(tid == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();
    if (osal_unlikely(alg_stru->tid_update_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return alg_stru->tid_update_func(tid, in_mpdu_num);
}

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
 函 数 名  : hmac_tx_check_mimo_rate
 功能描述  : 更新发送描述符中的"phy tx mode 1"
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 hmac_tx_check_mimo_rate(osal_u8 protocol_mode, const hal_tx_txop_alg_stru *txop_alg,
    osal_u8 data_idx)
{
    oal_bool_enum_uint8 mimo_rate = OSAL_FALSE;
    osal_u8 daterate = WLAN_HT_MCS_BUTT;
    if (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        daterate = hal_txop_alg_get_tx_dscr(txop_alg)[data_idx].nss_rate.ht_rate.ht_mcs;
        if ((daterate >= WLAN_HT_MCS8) && (daterate < WLAN_HT_MCS_BUTT)) {
            mimo_rate = OSAL_TRUE;
        }
    } else if (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE) {
        daterate = hal_txop_alg_get_tx_dscr(txop_alg)[data_idx].nss_rate.vht_nss_mcs.vht_mcs;
        if (daterate > WLAN_SINGLE_NSS) {
            mimo_rate = OSAL_TRUE;
        }
    } else {
        /* 暂时不做处理 */
    }

    return mimo_rate;
}

/*****************************************************************************
 函 数 名  : hmac_tx_update_smps_txop_alg
 功能描述  : 更新发送描述符中的"phy tx mode 1"
*****************************************************************************/
static INLINE__ osal_void hmac_tx_update_smps_txop_alg(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    hal_tx_txop_alg_stru *txop_alg)
{
    osal_u8 smps = 0;
    osal_u8 protocol_mode = 0;
    osal_u8 mimo_rate = 0;
    osal_u8 index = 0;
    hal_cfg_rts_tx_param_stru hal_rts_tx_param;

    smps = hmac_user_get_smps_mode(hmac_vap, hmac_user);

    if (smps >= WLAN_MIB_MIMO_POWER_SAVE_MIMO) {
        return;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return;
    }

    for (index = 0; index < HAL_TX_RATE_MAX_NUM; index++) {
        protocol_mode = hal_txop_alg_get_tx_dscr(txop_alg)[index].protocol_mode;
        mimo_rate = hmac_tx_check_mimo_rate(protocol_mode, txop_alg, index);

        if (mimo_rate == OSAL_FALSE) {
            continue;
        }

        if (smps == WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC) {
            hal_txop_alg_get_tx_dscr(txop_alg)[index].rts_cts_protect_mode = OSAL_TRUE;

            /* 设置RTS速率对应的频带 */
            hal_rts_tx_param.band = hmac_vap->channel.band;

            /* RTS[0~2]设为24Mbps */
            hal_rts_tx_param.protocol_mode[0] = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            hal_rts_tx_param.rate[0] = WLAN_LEGACY_OFDM_24M_BPS;
            hal_rts_tx_param.protocol_mode[1] = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            hal_rts_tx_param.rate[1] = WLAN_LEGACY_OFDM_24M_BPS;
            hal_rts_tx_param.protocol_mode[2] = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            hal_rts_tx_param.rate[2] = WLAN_LEGACY_OFDM_24M_BPS;

            /* 2G的RTS[3]设为1Mbps */
            if (hal_rts_tx_param.band == WLAN_BAND_2G) {
                hal_rts_tx_param.protocol_mode[3] = WLAN_11B_PHY_PROTOCOL_MODE;
                hal_rts_tx_param.rate[3] = WLAN_LONG_11B_1M_BPS;
            }
            /* 5G的RTS[3]设为24Mbps */
            else {
                hal_rts_tx_param.protocol_mode[3] = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                hal_rts_tx_param.rate[3] = WLAN_LEGACY_OFDM_24M_BPS;
            }

        } else if (smps == WLAN_MIB_MIMO_POWER_SAVE_STATIC) {
            hal_txop_alg_get_tx_dscr(txop_alg)[index].nss_rate.ht_rate.ht_mcs = WLAN_HT_MCS0;
        }
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_alg_rx_notify
 功能描述  : 调用挂接到RX流程上的算法钩子
*****************************************************************************/
static INLINE__ osal_void hmac_alg_rx_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf,
    hal_rx_statistic_stru *rx_stats)
{
    hmac_alg_stru *alg_stru;
    osal_u32 index;
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL) ||
        (buf == OSAL_NULL) || (rx_stats == OSAL_NULL))) {
        return;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (osal_unlikely(alg_stru->alg_bitmap == 0)) {
        return;
    }

    if (hmac_is_thruput_enable(THRUPUT_ALG_BYPASS)) {
        return;
    }

    /* 调用钩子函数 */
    for (index = HMAC_ALG_RX_START; index < HMAC_ALG_RX_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_rx_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_rx_notify_func[index](hmac_vap, hmac_user, buf, rx_stats);
        }
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_alg_vap_up_notify
 功能描述  : vap up时调用算法钩子
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_vap_up_notify(hmac_vap_stru *hmac_vap)
{
    hmac_alg_stru *alg_stru;
    osal_u32 index;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_alg_vap_up_hmac_to_dmac(hmac_vap);
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }

    /* 调用钩子函数 */
    for (index = HMAC_ALG_VAP_UP_START; index < HMAC_ALG_VAP_UP_BUTT; index++) {
        if (alg_stru->pa_alg_vap_up_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_alg_vap_up_notify_func[index](hmac_vap);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_vap_down_notify
 功能描述  : vap down时调用算法钩子
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_vap_down_notify(hmac_vap_stru *hmac_vap)
{
    hmac_alg_stru *alg_stru;
    osal_u32 index;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }

    /* 调用钩子函数 */
    for (index = HMAC_ALG_VAP_DOWN_START; index < HMAC_ALG_VAP_DOWN_BUTT; index++) {
        if (alg_stru->pa_alg_vap_down_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_alg_vap_down_notify_func[index](hmac_vap);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :  算法参数同步通知,入参由调用者保证有效性
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_para_sync_notify(alg_param_sync_stru *para)
{
    osal_u32 index;
    hmac_alg_stru *alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }
    if (osal_unlikely(para == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 调用钩子函数 */
    for (index = 0; index < ALG_PARAM_SYNC_NOTIFY_BUTT; index++) {
        if (alg_stru->alg_para_sync_notify_func[index] != OSAL_NULL) {
            alg_stru->alg_para_sync_notify_func[index](para);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :  算法参数同步通知,入参由调用者保证有效性
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_para_cfg_notify(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 index;
    frw_msg alg_msg = {0};
    hmac_alg_stru *alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (msg == OSAL_NULL))) {
        return OAL_FAIL;
    }
    cfg_msg_init(msg->data, msg->data_len, msg->rsp, msg->rsp_buf_len, &alg_msg);

    /* 调用钩子函数 */
    for (index = 0; index < ALG_PARAM_CFG_NOTIFY_BUTT; index++) {
        if (alg_stru->alg_para_cfg_notify_func[index] != OSAL_NULL) {
            alg_stru->alg_para_cfg_notify_func[index](hmac_vap, &alg_msg);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :  bfee report帧速率或用户距离信息改变通知tpc算法更新功率,入参由调用者保证有效性
*****************************************************************************/
static INLINE__ osal_u32 hmac_bfee_report_pow_adjust_notify(hal_to_dmac_device_stru *hal_device)
{
    hmac_alg_stru *alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 检查钩子函数是否注册 */
    if (alg_stru->bfee_report_pow_adjust_notify != OSAL_NULL) {
        alg_stru->bfee_report_pow_adjust_notify(hal_device);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 功率表更新通知TPC更新参数,入参由调用者保证有效性
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_pow_table_refresh_notify(hmac_vap_stru *hmac_vap)
{
    hmac_alg_stru *alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 检查钩子函数是否注册 */
    if (alg_stru->alg_pow_table_refresh_notify_func != OSAL_NULL) {
        alg_stru->alg_pow_table_refresh_notify_func(hmac_vap);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : cb更新 通知tpc算法更新cb->rssi_level,入参由调用者保证有效性
*****************************************************************************/
static INLINE__ osal_u32 hmac_alg_update_rssi_level_notify(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *cb,
    hmac_user_stru *hmac_user)
{
    hmac_alg_stru *alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();

    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (cb == OSAL_NULL))) {
        return OAL_FAIL;
    }

    /* 检查钩子函数是否注册 */
    if (alg_stru->alg_update_cb_rssi_level_notify_func != OSAL_NULL) {
        alg_stru->alg_update_cb_rssi_level_notify_func(hmac_vap, cb, hmac_user);
        return OAL_SUCC;
    }

    return OAL_FAIL;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_alg_notify.h */
