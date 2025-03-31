/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: mimo-siso切换特性相关结构定义和实现文件
 * Date: 2020-07-04
 */

#ifdef _PRE_WLAN_FEATURE_M2S

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"

#include "hal_chip.h"
#include "hal_device_fsm.h"

#include "hmac_alg_notify.h"
#include "hmac_vap.h"
#include "hmac_scan.h"
#include "hmac_tx_mgmt.h"
#include "hmac_smps.h"
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#include "hmac_fcs.h"
#include "hmac_power.h"
#include "hmac_m2s.h"
#include "hmac_feature_main.h"
#include "oal_netbuf_data.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_notify.h"
#include "hmac_btcoex_m2s.h"
#include "hal_pm.h"
#include "hmac_psm_sta.h"
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_M2S_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_FEATURE_M2S

/*****************************************************************************
  1 函数申明
*****************************************************************************/
osal_void hmac_m2s_state_idle_entry(osal_void *ctx);
osal_void hmac_m2s_state_idle_exit(osal_void *ctx);
osal_u32 hmac_m2s_state_idle_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hmac_m2s_state_mimo_entry(osal_void *ctx);
osal_void hmac_m2s_state_mimo_exit(osal_void *ctx);
osal_u32 hmac_m2s_state_mimo_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hmac_m2s_state_miso_entry(osal_void *ctx);
osal_void hmac_m2s_state_miso_exit(osal_void *ctx);
osal_u32 hmac_m2s_state_miso_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hmac_m2s_state_siso_entry(osal_void *ctx);
osal_void hmac_m2s_state_siso_exit(osal_void *ctx);
osal_u32 hmac_m2s_state_siso_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hmac_m2s_state_simo_entry(osal_void *ctx);
osal_void hmac_m2s_state_simo_exit(osal_void *ctx);
osal_u32 hmac_m2s_state_simo_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_u32 hmac_m2s_fsm_trans_to_state(hal_m2s_fsm_stru *m2s_fsm, osal_u8 state);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 各状态入口出口函数 */
oal_fsm_state_info  g_hal_m2s_fsm_info[] = {
    {
        HAL_M2S_STATE_IDLE,
        "IDLE",
        hmac_m2s_state_idle_entry,
        hmac_m2s_state_idle_exit,
        hmac_m2s_state_idle_event,
    },
    {
        HAL_M2S_STATE_SISO,
        "SISO",
        hmac_m2s_state_siso_entry,
        hmac_m2s_state_siso_exit,
        hmac_m2s_state_siso_event,
    },
    {
        HAL_M2S_STATE_MIMO,
        "MIMO",
        hmac_m2s_state_mimo_entry,
        hmac_m2s_state_mimo_exit,
        hmac_m2s_state_mimo_event,
    },
    {
        HAL_M2S_STATE_MISO,
        "MISO",
        hmac_m2s_state_miso_entry,
        hmac_m2s_state_miso_exit,
        hmac_m2s_state_miso_event,
    },
    {
        HAL_M2S_STATE_SIMO,
        "SIMO",
        hmac_m2s_state_simo_entry,
        hmac_m2s_state_simo_exit,
        hmac_m2s_state_simo_event,
    },
};

/*****************************************************************************
  3 函数定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_m2s_mgr_param_info
 功能描述  :  mimo-siso切换参数查询
*****************************************************************************/
static INLINE__ hmac_vap_m2s_stru *hmac_vap_get_vap_m2s(osal_void *hmac_vap)
{
    hmac_vap_m2s_stru *m2s_vap_info = hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_M2S);
    if (m2s_vap_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_vap_get_vap_m2s is null");
    }
    return m2s_vap_info;
}

static INLINE__ hmac_vap_m2s_rx_statistics_stru *hmac_vap_get_vap_m2s_rx_statistics(osal_void *hmac_vap)
{
    return &(hmac_vap_get_vap_m2s(hmac_vap)->hmac_vap_m2s_rx_statistics);
}

#ifdef _PRE_WIFI_DEBUG
osal_void hmac_m2s_mgr_param_info(const hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mgr_param_info::param null.}");
        return;
    }

    oam_warning_log_alter(0, OAM_SF_M2S,
        "{hmac_m2s_switch_mgr_param_info::m2s_type[%d],cur state[%d],mode_stru[%d],miso_hold[%d].}", 4, /* 4个打印值 */
        hal_device->hal_m2s_fsm.m2s_type, hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device),
        get_hal_device_m2s_del_swi_miso_hold(hal_device));

    oam_warning_log_alter(0, OAM_SF_M2S,
        "{hmac_m2s_switch_mgr_param_info::nss_num[%d]phy_chain[%d]single_txrx_chain[%d]rf[%d]phy2dscr_chain[%d].}",
        5, hal_device->cfg_cap_info->nss_num, hal_device->cfg_cap_info->phy_chain, /* 5个打印值 */
        hal_device->cfg_cap_info->single_tx_chain, hal_device->cfg_cap_info->rf_chain,
        hal_device->cfg_cap_info->phy2dscr_chain);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_m2s_event_state_classify
 功能描述  : 根据切换事件类型确认新状态
*****************************************************************************/
hal_m2s_state_uint8 hmac_m2s_event_state_classify(hal_m2s_event_tpye_uint16 m2s_event)
{
    hal_m2s_state_uint8 new_m2s_state = HAL_M2S_STATE_BUTT;

    /* 后续根据业务需求，进行对应case添加 */
    switch (m2s_event) {
        /* 并发扫描切换到SIMO,软件还是mimo,硬件切siso, 业务场景下不交叉，siso采用c0 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            new_m2s_state = HAL_M2S_STATE_SIMO;
            break;

        /* 并发扫描结束切换回MIMO,软件和硬件都是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MIMO:
        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_SPEC_SISO_TO_MIMO:
            new_m2s_state = HAL_M2S_STATE_MIMO;
            break;

        /* RSSI切换到MISO,软件切换到SISO,硬件还是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1:
            new_m2s_state = HAL_M2S_STATE_MISO;
            break;

        case HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1:
            new_m2s_state = HAL_M2S_STATE_SISO;
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_event_state_classify: m2s_event[%d] error!}", m2s_event);
    }

    return new_m2s_state;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_chain_state_classify
 功能描述  : 根据chain状态确认m2s新状态
*****************************************************************************/
hal_m2s_state_uint8 hmac_m2s_chain_state_classify(const hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_chain_state_classify::param null.}");
        return HAL_M2S_STATE_BUTT;
    }

    /* 先判断软件状态 */
    if (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_DOUBLE) {
        if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_DOUBLE) {
            return HAL_M2S_STATE_MIMO;
        } else {
            return HAL_M2S_STATE_SIMO;
        }
    } else { /* 软件处于siso */
        if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_DOUBLE) {
            return HAL_M2S_STATE_MISO;
        } else {
            return HAL_M2S_STATE_SISO;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_event_trigger_mode_classify
 功能描述  : 根据切换事件类型确认新业务模式
*****************************************************************************/
wlan_m2s_trigger_mode_enum_uint8 hmac_m2s_event_trigger_mode_classify(hal_m2s_event_tpye_uint16 m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;

    switch (m2s_event) {
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
            /* 快速扫描，需要单独判断快速扫描开启时候置标记，因为共用了SCAN_BEGIN标记 */
            trigger_mode = WLAN_M2S_TRIGGER_MODE_FAST_SCAN;
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_RSSI;
            break;

        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MISO_TO_SISO_C1:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_BTCOEX;
            break;

        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_COMMAND;
            break;

        case HAL_M2S_EVENT_TEST_SISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0: /* test会在idle模式切换 */
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1: /* 要保持test mode标志,其他业务不存在idle切换,command约束测试行为 */
        case HAL_M2S_EVENT_TEST_IDLE_TO_MIMO:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_TEST;
            break;

        case HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_CUSTOM;
            break;

        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_SPEC_SISO_TO_MIMO:
        case HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0:
            trigger_mode = WLAN_M2S_TRIGGER_MODE_SPEC;
            break;

        default:
            oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_event_trigger_mode_classify: m2s_event[%d] other service!}",
                m2s_event);
    }

    return trigger_mode;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_update_switch_mgr_param
 功能描述  : SISO修订mimo-siso切换参数
             (1)分成五类，mimo miso sisoc0或者c1 simo  (2)刷新hal device下能力
*****************************************************************************/
osal_void hmac_m2s_update_switch_mgr_param(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_update_switch_mgr_param::param null.}");
        return;
    }

    /* 后续根据业务需求，进行对应case添加 */
    switch (m2s_event) {
        /* 并发扫描切换到SIMO,软件还是mimo,硬件切siso, 业务场景下不交叉，siso采用c0 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            hal_device->cfg_cap_info->nss_num = WLAN_DOUBLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_DOUBLE;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_ZERO;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_ZERO;
            break;

        /* 并发扫描结束切换回MIMO,软件和硬件都是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_TEST_SISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MIMO:
        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_SPEC_SISO_TO_MIMO:
            hal_device->cfg_cap_info->nss_num = WLAN_DOUBLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_DOUBLE;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_DOUBLE;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* C0 MISO 例如RSSI切换到MISO,软件切换到SISO,硬件还是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MISO_SCAN_BEGIN:
            hal_device->cfg_cap_info->nss_num = WLAN_SINGLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_DOUBLE;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* C1 MISO */
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1:
            hal_device->cfg_cap_info->nss_num = WLAN_SINGLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_ONE;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ONE;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_DOUBLE;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* 主phy0的siso,软件和硬件都切siso */
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_SCAN_END:
        case HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0:
            hal_device->cfg_cap_info->nss_num = WLAN_SINGLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_ZERO;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_ZERO;
            break;

        /* 主phy1的siso,软件和硬件都切siso */
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1:
            hal_device->cfg_cap_info->nss_num = WLAN_SINGLE_NSS;
            hal_device->cfg_cap_info->phy2dscr_chain = WLAN_PHY_CHAIN_ONE;
            hal_device->cfg_cap_info->single_tx_chain = WLAN_PHY_CHAIN_ONE;
            hal_device->cfg_cap_info->rf_chain = WLAN_RF_CHAIN_ONE;
            hal_device->cfg_cap_info->phy_chain = WLAN_PHY_CHAIN_ONE;
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S,
                "{hmac_m2s_update_switch_mgr_param: m2s_event[%d] error!}", m2s_event);
    }

#ifdef _PRE_WIFI_DEBUG
    hmac_m2s_mgr_param_info(hal_device);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_priority_check
 功能描述  : m2s触发业务申请，返回是否可行，暂时放开业务层自己判断，m2s不处理申请接入
             (1)第一步是哪个业务优先级高，或者已经申请了，其他业务不允许，如果当前状态符合要求，在另一个接口判断时直接返回true
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_switch_priority_check(const hal_to_dmac_device_stru *hal_device,
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode, hal_m2s_state_uint8 new_m2s_state)
{
    oal_bool_enum_uint8 allow = OSAL_TRUE; /* 初始申请业务允许 */

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_priority_check::param null.}");
        return OSAL_FALSE;
    }

    /* 1.对于切换miso/siso 需要判断其他业务是否已经在切换 */
    if (new_m2s_state == HAL_M2S_STATE_BUTT) {
        oam_error_log2(0, OAM_SF_M2S,
            "{hmac_m2s_switch_priority_check::trigger_mode[%d] new_m2s_state[%d]error.}", trigger_mode,
            new_m2s_state);
    } else if (new_m2s_state == HAL_M2S_STATE_MIMO || new_m2s_state == HAL_M2S_STATE_MISO) {
        /* 要切换到miso 或者mimo时，如果有其他业务已经在执行了，状态机都不能执行(其他业务在执行，都是在siso状态，不能恢复)
         具体业务可以根据是恢复到mimo时，如果失败，仍可以释放自己业务侧功能资源 */
        switch (trigger_mode) {
            /* dbdc先来，再来并发扫描是允许的 */
            case WLAN_M2S_TRIGGER_MODE_FAST_SCAN:
                if ((*(osal_u8 *)(&hal_device->hal_m2s_fsm.m2s_mode) & (BIT2 | BIT3 | BIT4 | BIT5)) != 0) {
                    allow = OSAL_FALSE;
                }
                break;

            case WLAN_M2S_TRIGGER_MODE_RSSI:
            case WLAN_M2S_TRIGGER_MODE_BTCOEX:
            case WLAN_M2S_TRIGGER_MODE_COMMAND:
            case WLAN_M2S_TRIGGER_MODE_TEST:
                /* 处于本状态才允许 */
                if (((*(osal_u8 *)(&hal_device->hal_m2s_fsm.m2s_mode) & (~trigger_mode))) != 0) {
                    allow = OSAL_FALSE;
                }
                break;

            default:
                oam_warning_log1(0, OAM_SF_M2S,
                    "{hmac_m2s_switch_priority_check::trigger_mode[%d] error.}", trigger_mode);
                break;
        }
    } else if (new_m2s_state == HAL_M2S_STATE_SIMO || new_m2s_state == HAL_M2S_STATE_SISO) {
        /* 要切换到simo 或者siso时，如果有其他业务已经在执行了， 一般只要不是处于miso状态，都可以执行，这个miso会在其他逻辑判断 */
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_switch_priority_check::trigger_mode[%d] new_m2s_state[%d]error.}", trigger_mode,
            new_m2s_state);
    }

    return allow;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mgr_back_to_mimo_check
 功能描述  : 去关联时，判断系统是否需要回mimo
             (1)del bitmap调用，保证find up all不会找到错误设备
             (2)当前支持btcoex mss判断， 其他业务需要的话，按照优先级排序判断即可
             (3)当前去关联，都需要做回mimo判断，特点业务不存在时要回到mimo，方便后续入网做对应处理
                当前主要是spec  btcoex mss rssi， dbdc和并发扫描会通过另一个接口
*****************************************************************************/
osal_void hmac_m2s_mgr_back_to_mimo_check(hal_to_dmac_device_stru *hal_device)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO_CHECK);
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mgr_back_to_mimo_check::param null.}");
        return;
    }

    /* 1.如果mss_on(sta去关联)，需要恢复到mimo，防止后续业务在siso工作异常,这里主要是去关联之后没有重关联，idle状态要保持在mimo */
    if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
        /* MSS存在时，不会有其他m2s业务存在，此时直接回mimo即可; 标志和切换保护功能在此回mimo时清,此时一定是单sta模式 */
        hmac_m2s_command_mode_force_to_mimo(hal_device);
    /* 2.有vap去关联时，判断系统当前是否需要回mimo;切换到过siso  那肯定能回mimo */
    } else if (fhook != OSAL_NULL &&
        ((hmac_btcoex_m2s_back_to_mimo_check_cb)fhook)(hal_device) == OSAL_TRUE) {
        return;
    /* 3.如果是spec触发了siso，此时需要判断是否需要回到mimo; dbdc场景下去关联，由dbdc自己处理即可 */
    } else if (hal_m2s_check_spec_on(hal_device) == OSAL_TRUE) {
        hmac_m2s_spec_mode_force_to_mimo(hal_device);
        /* 4.如果test方式，主要是dbdc等常发常收测试，出现搬移的话，当前hal device需要保持 */
    } else if (hal_m2s_check_test_on(hal_device) == OSAL_TRUE) {
        /* dbac场景会反复跳转进入，test模式下会日志刷屏 */
        /* 5.如果是rssi触发了siso，此时需要判断是否需要回到mimo */
    } else if (hal_m2s_check_rssi_on(hal_device) == OSAL_TRUE) {
        if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) {
            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO, 0, OSAL_NULL);
        } else {
            oam_error_log2(0, OAM_SF_M2S,
                "{hmac_m2s_mgr_back_to_mimo_check: m2s_mode[%d]m2s_cur_state[%d] can't resume mimo.}",
                get_hal_m2s_mode_tpye(hal_device), hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        }
    }
}

osal_void hmac_m2s_siso_switch_back_to_mimo(hal_m2s_fsm_stru *m2s_fsm, hal_to_dmac_device_stru *hal_device,
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    osal_u8 m2s_mode_mask = 0;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(m2s_fsm == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_siso_switch_back_to_mimo::param null.}");
        return;
    }

    /* 2.3.1 如果是dbdc和并发扫描退出，spec约束还在，需要保持在c0 siso，不会是c1 siso */
    /* spec自己申请回，直接回 */
    if (hmac_m2s_spec_need_switch_keep_check(hal_device) == OSAL_TRUE) {
        /* 前景扫描直接回 */
        get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);

        oam_warning_log1(0, OAM_SF_M2S,
            "{hmac_m2s_siso_switch_back_to_mimo::spec mode need keep siso, mode_stru[%d].}",
            get_hal_m2s_mode_tpye(hal_device));

        /* 需要保持在siso，置上spec标志,清掉结束业务标志 */
        get_hal_m2s_mode_tpye(hal_device) |= WLAN_M2S_TRIGGER_MODE_SPEC;

        return;
    }

    /* btcoex业务在，直接切c1 siso即可 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO);
    if (fhook != OSAL_NULL && ((hmac_btcoex_m2s_back_to_mimo_cb)fhook)(hal_device, &m2s_mode_mask) == OSAL_TRUE) {
        return;
    } else {
        if (trigger_mode == WLAN_M2S_TRIGGER_MODE_FAST_SCAN) {
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_SPEC);
        }

        get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);
        hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MIMO);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_back_to_misx_check
 功能描述  : 特定业务结束，申请m2s做状态跳转和vap 硬件能力更新，期待回到mimo或者miso状态，这里做判断是否保持原状态，
             还是继续跳转到miso或者mimo
             (1)业务结束调用m2s_handle，对应状态的event里面增加调用本函数，检查到其他业务在，如果是跳mimo或者miso，
             就不执行
             (2)m2s自身保存业务调用的标志维护，防止其他业务模块无特定标记，公共模块也和其他业务模块解耦
*****************************************************************************/
osal_void hmac_m2s_switch_back_to_mixo_check(hal_m2s_fsm_stru *m2s_fsm, hal_to_dmac_device_stru *hal_device,
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    osal_u32 he_rom_nss;

    if (osal_unlikely(m2s_fsm == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_back_to_mixo_check::param null.}");
        return;
    }

    if ((hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MIMO) ||
        (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_BUTT)) {
        oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_back_to_mixo_check::cur_m2s_state[%d] error.}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return;
    }

    hal_get_reg_he_rom_nss(&he_rom_nss);
    if ((hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) ||
        (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO)) {
        if (he_rom_nss == 0) {
            oam_error_log1(0, OAM_SF_M2S, "{dmac_m2s_switch_back_to_mixo_check:not support double nss[%d]error.}",
                he_rom_nss);
            return;
        }
    }

    /* 1.清零本业务状态,需要在各自分支，还要使用该状态，放置到各个冲突业务中自己来清; mimo entry可以统一清零也可以 */
    /* 2.判断是否状态已经是清零，根据存在的业务，切换到新的状态，不一定是回到mimo */
    /* (0)优先级排序 RSSI<COMMAND<(btcoex,dbdc,fast_scan)   test进做测试用，尽量不考虑，为了ci，只处理和fast_scan冲突
       (1)尽量不允许并发执行
       (2)simo/miso退出，如果有btcoex，回到mimo后继续切换到c1 siso,否则切换到mimo
       (3)siso退出，如果有其他btcoex切到c1 siso，否则回mimo
    */
    oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_switch_back_to_mixo_check::cur_m2s_state[%d] mode_stru[%d].}",
        hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));

    /* 2.1
     * 并发扫描特殊状态，先回mimo，然后切换到对应需求状态;此时前景处于simo状态；背景扫描会先配置信道，
     * spec不切状态但是清了标志，并发扫描也在此直接回
     */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SIMO) {
        get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);

        /* 2g simo恢复时需要清spec标志 */
        get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_SPEC);

        hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MIMO);
    } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) { /* 2.2 MISO稳定态回mimo */
        get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);
        hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MIMO);
    } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO) { /* 2.3 SISO稳定态回mimo */
        hmac_m2s_siso_switch_back_to_mimo(m2s_fsm, hal_device, trigger_mode);
    } else {
        oam_error_log2(0, OAM_SF_M2S,
            "{hmac_m2s_switch_back_to_mixo_check:: m2s_state[%d]m2s_mode[%d]!}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_apply_and_confirm
 功能描述  : m2s切换申请,当前用于test和command测试，后续还是要业务自己根据状态和业务类型来做判断
             (1)其他模块的siso或者mimo事件进行业务申请，返回true或者false（首先新状态机满足当前状态机跳转要求， 其次要满足业务优先级管理），
             (2)其他模块才能继续执行抛切换m2s event函数
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_switch_apply_and_confirm(hal_to_dmac_device_stru *hal_device, osal_u16 m2s_type,
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    hal_m2s_state_uint8 new_m2s_state;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_apply_and_confirm::param null.}");
        return OSAL_FALSE;
    }

    /* 1.根据切换业务，确认新状态类型 */
    new_m2s_state = hmac_m2s_event_state_classify(m2s_type);
    if (new_m2s_state == HAL_M2S_STATE_BUTT) {
        return OSAL_FALSE;
    }

    /* 2.1. 为了不影响2.2 在c0 和 c1 siso切换，这里做custom进行约束 */
    if (hal_m2s_check_custom_on(hal_device) == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{hmac_m2s_switch_apply_and_confirm: cur state[%d] cnnnot switch because of CUSTOM!}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return OSAL_FALSE;
    }

    /* 当前spec下确认是否支持切换 */
    if (hmac_m2s_spec_support_siso_switch_check(hal_device, hmac_device, WLAN_M2S_TRIGGER_MODE_TEST) == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_apply_and_confirm: spec contrain to switch!}");
        return OSAL_FALSE;
    }

    /* 2.2. old和new状态一致，直接返回true给外层并提示,可以继续执行，例如虽然状态不跳转，但是可以在siso时切换通道 */
    if (new_m2s_state == hal_device->hal_m2s_fsm.oal_fsm.cur_state) {
        oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_switch_apply_and_confirm: cur state[%d] conform m2s_type[%d]!}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, m2s_type);
        return OSAL_TRUE;
    }

    /* 3.如果是test触发，增加保护，需要清MSS资源，防止测试结果误报host给上层 */
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_TEST) {
        if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
            /* 切换保护功能关闭 */
            get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;

            /* 清业务标志 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_COMMAND);
        } else if (hal_m2s_check_spec_on(hal_device) == OSAL_TRUE) { /* 当前已经切换到spec的话，直接不支持 */
            oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_apply_and_confirm::already spec siso!}");
            return OSAL_FALSE;
        }
    } else if (trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
        /* 测试模式下约束，3 0 0/2 0 0不要混合使用;一定要等其中一个业务回到mimo，再执行另一个业务切换模式 */
    }

    /* 5.test和command本身，需要约束下，并发扫描无可避免,直接scan abort,其他业务保证不先执行，btcoex约束不允许切siso再测试btcoex的切siso */
    if ((get_hal_m2s_mode_tpye(hal_device) & (~(WLAN_M2S_TRIGGER_MODE_TEST | WLAN_M2S_TRIGGER_MODE_COMMAND))) != 0) {
        /* 1.1 测试模式下，优先级高于并发扫描，scan abort掉，避免影响CI测试 */
        if (get_hal_m2s_mode_tpye(hal_device) == WLAN_M2S_TRIGGER_MODE_FAST_SCAN) {
            hmac_scan_abort(hmac_device);
            oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_apply_and_confirm::abort scan!}");
            return OSAL_TRUE;
        } else {
            oam_warning_log1(0, OAM_SF_M2S,
                "{hmac_m2s_switch_apply_and_confirm::cannot to switch, mode[%d]!}", get_hal_m2s_mode_tpye(hal_device));
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_get_bss_max_nss
 功能描述  : 获取某个BSS的最大支持空间流
*****************************************************************************/
wlan_nss_enum_uint8 hmac_m2s_get_bss_max_nss(const hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf,
    osal_u16 frame_len, oal_bool_enum_uint8 assoc_status)
{
    osal_u8 *frame_body = OSAL_NULL; /* 获取帧体起始指针 */
    osal_u8 *ht_mcs_bitmask = OSAL_NULL;
    wlan_nss_enum_uint8 nss = WLAN_SINGLE_NSS;
    osal_u16 vht_mcs_map, msg_idx;
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN; /* 设置Beacon帧的field偏移量 */
    osal_u8 *ie = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_get_bss_max_nss::param null.}");
        return WLAN_USER_HT_NSS_LIMIT;
    }

    frame_body = oal_netbuf_rx_data(netbuf); /* 获取帧体起始指针 */
    ie = mac_find_ie_etc(MAC_EID_HT_CAP, frame_body + offset, frame_len - offset); /* 查ht ie */
    if (ie != OSAL_NULL) {
        msg_idx = MAC_IE_HDR_LEN + MAC_HT_CAPINFO_LEN + MAC_HT_AMPDU_PARAMS_LEN;
        ht_mcs_bitmask = &ie[msg_idx];
        for (nss = WLAN_SINGLE_NSS; nss < WLAN_USER_HT_NSS_LIMIT; nss++) {
            if (ht_mcs_bitmask[nss] == 0) {
                break;
            }
        }
        nss = (nss != WLAN_SINGLE_NSS) ? (--nss) : nss;

        /* 如果是考虑up状态，并且是ht协议时，按照此值返回 */
        if (assoc_status == OSAL_TRUE && (hmac_vap->protocol == WLAN_HT_MODE ||
            hmac_vap->protocol == WLAN_HT_ONLY_MODE || WLAN_HT_11G_MODE == hmac_vap->protocol)) {
            return nss;
        }
    }

    /* 查vht ie */
    ie = mac_find_ie_etc(MAC_EID_VHT_CAP, frame_body + offset, frame_len - offset);
    /* 增加兼容性，还需从私有vendor ie里面查找vht ie */
    if (ie == OSAL_NULL) {
        ie = hmac_ie_find_vendor_vht_ie(frame_body + offset, frame_len - offset);
    }

    if (ie != OSAL_NULL) {
        msg_idx = MAC_IE_HDR_LEN + MAC_VHT_CAP_INFO_FIELD_LEN;

        vht_mcs_map = oal_make_word16(ie[msg_idx], ie[msg_idx + 1]);

        for (nss = WLAN_SINGLE_NSS; nss < WLAN_USER_VHT_NSS_LIMIT; nss++) {
            if (wlan_get_vht_max_support_mcs(vht_mcs_map & 0x3) == WLAN_INVALD_VHT_MCS) {
                break;
            }
            vht_mcs_map >>= 2;  /* 右移2位 */
        }
        nss = (nss != WLAN_SINGLE_NSS) ? (--nss) : nss;

        /* 如果是考虑up状态，并且是vht协议时，按照此值返回 */
        if (assoc_status == OSAL_TRUE && (hmac_vap->protocol == WLAN_VHT_MODE ||
            hmac_vap->protocol == WLAN_VHT_ONLY_MODE || hmac_vap->protocol == WLAN_HE_MODE)) {
            return nss;
        }
    }

    return nss;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_scan_get_num_sounding_dim
 功能描述  : 取对端的number of sounding dimension数
*****************************************************************************/
osal_u8 hmac_m2s_scan_get_num_sounding_dim(const oal_netbuf_stru *netbuf, osal_u16 frame_len)
{
    osal_u8 *vht_cap_ie = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;
    osal_u16 offset;
    osal_u16 vht_cap_filed_low;
    osal_u16 vht_cap_filed_high;
    osal_u32 vht_cap_field;
    osal_u16 msg_idx;
    osal_u8 num_sounding_dim = 0;

    /* 获取帧体起始指针 */
    frame_body = oal_netbuf_rx_data(netbuf);

    /* 设置Beacon帧的field偏移量 */
    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    vht_cap_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, frame_body + offset, frame_len - offset);
    /* 增加兼容性，还需从私有vendor vht ie里面查找number of sounding dimension */
    if (vht_cap_ie == OSAL_NULL) {
        vht_cap_ie = hmac_ie_find_vendor_vht_ie(frame_body + offset, frame_len - offset);
        if (vht_cap_ie == OSAL_NULL) {
            return num_sounding_dim;
        }
    }

    msg_idx = MAC_IE_HDR_LEN;

    /* 解析VHT capablities info field */
    vht_cap_filed_low = oal_make_word16(vht_cap_ie[msg_idx], vht_cap_ie[msg_idx + 1]);
    vht_cap_filed_high = oal_make_word16(vht_cap_ie[msg_idx + 2], vht_cap_ie[msg_idx + 3]); /* 2,3:数据偏移 */
    vht_cap_field = oal_make_word32(vht_cap_filed_low, vht_cap_filed_high);
    /* 解析num_sounding_dim */
    num_sounding_dim = ((vht_cap_field & (BIT18 | BIT17 | BIT16)) >> 16);   /* 取高16bits */
    return num_sounding_dim;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_get_bss_support_opmode
 功能描述  : 获取对端extend capabilities中宣称是否支持OPMODE
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_get_bss_support_opmode(const hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8 *ie = OSAL_NULL;
    osal_u8 offset;

    unref_param(hmac_vap);
    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, frame_body + offset, frame_len - offset);
    if (ie == OSAL_NULL || ie[1] < MAC_HT_EXT_CAP_OPMODE_LEN) {
        return OSAL_FALSE;
    }
    return ((mac_ext_cap_ie_stru *)(ie + MAC_IE_HDR_LEN))->operating_mode_notification;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_update_switch_result
 功能描述  : 更新device切换结果信息
*****************************************************************************/
osal_void hmac_m2s_update_switch_result(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    hmac_m2s_complete_syn_stru *m2s_comp_param, osal_u8 *index)
{
    wlan_m2s_action_type_enum_uint8 ori_action_type;
    wlan_m2s_action_type_enum_uint8 cur_action_type;
    oal_bool_enum_uint8 vap_m2s_result;

    /* 1.添加到信息上报结构体 */
    ori_action_type = hmac_vap_get_vap_m2s(hmac_vap)->m2s_switch_ori_type;
    cur_action_type = ori_action_type;
    hmac_m2s_action_frame_type_query(hmac_vap, &cur_action_type);
    vap_m2s_result = ((cur_action_type == ori_action_type) ? OSAL_TRUE : OSAL_FALSE);

    /* 2. 更新device切换结果信息 */
    device_m2s_result |= vap_m2s_result;
    /* 上报切换vap的能力状态 */
    m2s_comp_param->m2s_comp_vap[vap_idx].m2s_result = vap_m2s_result;
    m2s_comp_param->m2s_comp_vap[vap_idx].action_type = cur_action_type;
    (osal_void)memcpy_s(m2s_comp_param->m2s_comp_vap[vap_idx].user_mac_addr, WLAN_MAC_ADDR_LEN,
        hmac_vap->bssid, WLAN_MAC_ADDR_LEN);

    /* 3.是否在黑名单, 为none模式的话hal device需要保持在miso状态 */
    if (hmac_m2s_check_blacklist_in_list(hmac_vap, index) == OSAL_TRUE) {
        /* 找到了一个黑名单关联ap */
        wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = &(get_hal_device_m2s_mgr(hal_device)->m2s_blacklist[*index]);
        /* 如果需要切换成none模式，只要存在一个vap需要保持在miso，后续hal device保持miso状态 */
        if (m2s_mgr_blacklist->action_type == WLAN_M2S_ACTION_TYPE_NONE) {
            get_hal_device_m2s_del_swi_miso_hold(hal_device) = OSAL_TRUE;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_protect_complete_apply
 功能描述  : 单vap切换保护完成，申请device做状态切换
             只有每个vap的状态都ok，hal device才能做m2s切换
*****************************************************************************/
osal_void hmac_m2s_switch_protect_complete_apply(hal_to_dmac_device_stru *hal_device,
    hal_m2s_event_tpye_uint16 event_type)
{
    hmac_vap_stru *mac_sta_vap = OSAL_NULL;
    osal_u8 index;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    osal_u8 up_vap_sta_num = 0;
    osal_u8 siso_allow = 0;
    hmac_m2s_complete_syn_stru m2s_comp_param;
    oal_bool_enum_uint8 device_m2s_result = OSAL_FALSE;
    osal_u8 up_vap_num, vap_idx;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_protect_complete_apply::param null.}");
        return;
    }

    memset_s(&m2s_comp_param, sizeof(m2s_comp_param), 0, sizeof(m2s_comp_param));

    /* 只看自己的hal device上找，正在关联或者去关联了的设备不需要考虑 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, 0, "{hmac_m2s_switch_protect_complete_apply::hmac_vap[%d] null.}", mac_vap_id[vap_idx]);
            continue;
        }
        /* 0.统计sta个数 */
        up_vap_sta_num = (is_sta(hmac_vap)) ? (up_vap_sta_num + 1) : up_vap_sta_num;
        /* 0.1是legacy sta,只会存在一个 */
        mac_sta_vap = (is_legacy_sta(hmac_vap)) ? hmac_vap : mac_sta_vap;

        hmac_m2s_update_switch_result(hal_device, hmac_vap, &m2s_comp_param, &index);

        /* 4.是否所有vap都已经切换完成，申请切换到siso,完成的vap++ */
        siso_allow = (hmac_vap_get_vap_m2s(hmac_vap)->m2s_wait_complete == OSAL_FALSE) ? siso_allow + 1 : siso_allow;
    }

    /* 5.所有vap都准备好，本身无设备时不可能，做一个保护 */
    if ((up_vap_num != 0) && (up_vap_sta_num == siso_allow)) {
        /* 5.1 (1)只要当前vap 不为空，就需要做切换信息上报，
        (2)多vap都完成了切换，(3)针对上层下发切换到siso时，填写剩余上报信息，上报切换结果 */
        /* 5.2 所有vap都准备好，并且是没有要求处于miso状态，才做siso切换，否则保持在miso */
        if (get_hal_device_m2s_del_swi_miso_hold(hal_device) == OSAL_FALSE) {
            hmac_m2s_handle_event(hal_device, event_type, 0, OSAL_NULL);
        }

        m2s_comp_param.m2s_result = device_m2s_result;
        m2s_comp_param.m2s_state = hal_device->hal_m2s_fsm.oal_fsm.cur_state;
        m2s_comp_param.m2s_mode = get_hal_m2s_mode_tpye(hal_device);
        m2s_comp_param.vap_num = up_vap_num;

        /* 切siso时，是MSS触发,并且上报功能使能，才做结果上报到host; mac_sta_vap为空是命令行2 0 0/3 0 0触发，需要保护 */
        if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE && get_hal_device_m2s_mss_on(hal_device) == OSAL_TRUE &&
            (mac_sta_vap != OSAL_NULL)) {
            hmac_m2s_switch_protect_comp_event_status(mac_sta_vap, &m2s_comp_param);
        }
    }
}

#ifdef _PRE_WIFI_DEBUG
/*****************************************************************************
 函 数 名  : hmac_m2s_show_blacklist_in_list
 功能描述  : dmac维测显示异常兼容性AP的地址
*****************************************************************************/
osal_void hmac_m2s_show_blacklist_in_list(const hal_to_dmac_device_stru *hal_device)
{
    hal_device_m2s_mgr_stru *device_m2s_mgr;
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = OSAL_NULL;
    osal_u8 index;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_show_blacklist_in_list::param null.}");
        return;
    }

    device_m2s_mgr = get_hal_device_m2s_mgr(hal_device);

    for (index = 0; index < device_m2s_mgr->blacklist_bss_cnt; index++) {
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[index]);

        oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_show_blacklist_in_list::Find in blacklist, index[%d],type[%d]}",
            index, m2s_mgr_blacklist->action_type);
        oam_warning_log4(0, OAM_SF_M2S,
            "{hmac_m2s_show_blacklist_in_list::Find in blacklist, addr->%02x:%02x:%02x:%02x:XX:XX.}",
            m2s_mgr_blacklist->user_mac_addr[0], m2s_mgr_blacklist->user_mac_addr[1],
            m2s_mgr_blacklist->user_mac_addr[2]), m2s_mgr_blacklist->user_mac_addr[3]);    /* 2/3:mac addr */
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_m2s_check_blacklist_in_list
 功能描述  : dmac记录异常兼容性AP的地址
*****************************************************************************/
osal_u32 hmac_m2s_check_blacklist_in_list(const hmac_vap_stru *hmac_vap, osal_u8 *index)
{
    osal_u8 index_blacklist;
    hal_to_dmac_device_stru *hal_device;
    hal_device_m2s_mgr_stru *device_m2s_mgr = OSAL_NULL;
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || index == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_check_blacklist_in_list::param null.}");
        return OAL_FAIL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_check_blacklist_in_list:: hal_device null}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    device_m2s_mgr = get_hal_device_m2s_mgr(hal_device);

    /* 初始化为无效index */
    *index = WLAN_M2S_BLACKLIST_MAX_NUM;

    for (index_blacklist = 0; index_blacklist < WLAN_M2S_BLACKLIST_MAX_NUM; index_blacklist++) {
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[index_blacklist]);

        if (oal_compare_mac_addr(m2s_mgr_blacklist->user_mac_addr, hmac_vap->bssid) == 0) {
            oam_warning_log4(0, OAM_SF_M2S,
                "{hmac_m2s_check_blacklist_in_list::Find in blacklist, addr->%02x:%02x:%02x:%02x:XX:XX.}",
                /* index 0，index 1, index 2，index 3 */
                hmac_vap->bssid[0], hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]);

            *index = index_blacklist;
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_add_and_update_blacklist_to_list
 功能描述  : 将异常ap添加到黑名单中，并刷新action帧发送类型
*****************************************************************************/
osal_void hmac_m2s_add_and_update_blacklist_to_list(hmac_vap_stru *hmac_vap)
{
    osal_u8 index;
    hal_to_dmac_device_stru *hal_device;
    hal_device_m2s_mgr_stru *device_m2s_mgr = OSAL_NULL;
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_add_and_update_blacklist_to_list::param null.}");
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_add_and_update_blacklist_to_list:: hal_device null}", hmac_vap->vap_id);
        return;
    }

    device_m2s_mgr = get_hal_device_m2s_mgr(hal_device);
    if (device_m2s_mgr->blacklist_bss_index >= WLAN_M2S_BLACKLIST_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_add_and_update_blacklist_to_list::already reach max num.}", hmac_vap->vap_id);
        device_m2s_mgr->blacklist_bss_index = 0;
    }

    /* 看该ap是否在列表中，在的话刷新切换模式即可，不在的话，需要重新添加 */
    if (hmac_m2s_check_blacklist_in_list(hmac_vap, &index) == OSAL_TRUE) {
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[index]);

        /* 如果已经是策略3，就不刷新 */
        if (m2s_mgr_blacklist->action_type == WLAN_M2S_ACTION_TYPE_NONE) {
            oam_warning_log1(0, OAM_SF_M2S,
                "vap_id[%d] {hmac_m2s_add_and_update_blacklist_to_list::already in MISO.}", hmac_vap->vap_id);
        } else {
            /* smps和opmode都不支持，需要保持在miso状态，后续该设备是none的话，都只能切换到miso状态 */
            /* 第二次添加采用none模式 */
            m2s_mgr_blacklist->action_type = WLAN_M2S_ACTION_TYPE_NONE;

            /* 本vap切换完成，置标志位 */
            hmac_vap_get_vap_m2s(hmac_vap)->m2s_wait_complete = OSAL_FALSE;

            /* 申请切换到siso仲裁，从而支持多vap切换，完成之后将切换结果上报上层，要保持miso传入butt非法事件即可 */
            hmac_m2s_switch_protect_complete_apply(hal_device, HAL_M2S_EVENT_BUTT);
        }
    } else {
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[device_m2s_mgr->blacklist_bss_index]);
        (osal_void)memcpy_s((osal_void *)(m2s_mgr_blacklist->user_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(hmac_vap->bssid), WLAN_MAC_ADDR_LEN);

        /* 第一次添加是采用OTHER方式,为了兼容小米3路由器，首次是smps话，此时直接到none,
           首次是opmode的话,才继续发smps 在none的话，就保持在miso状态了 配置命令需要把miso和siso对等来看，防止不执行 */
        (hmac_vap_get_vap_m2s(hmac_vap)->m2s_switch_ori_type == WLAN_M2S_ACTION_TYPE_SMPS) ?
            (m2s_mgr_blacklist->action_type = WLAN_M2S_ACTION_TYPE_OPMODE) :
            (m2s_mgr_blacklist->action_type = WLAN_M2S_ACTION_TYPE_SMPS);

        device_m2s_mgr->blacklist_bss_index++;

        /* 最多记录WLAN_BLACKLIST_MAX个用户，新增直接覆盖第一个，总数保持不变 */
        if (device_m2s_mgr->blacklist_bss_cnt < WLAN_M2S_BLACKLIST_MAX_NUM) {
            device_m2s_mgr->blacklist_bss_cnt++;
        }

        oam_warning_log4(0, OAM_SF_M2S,
            "{hmac_m2s_add_and_update_blacklist_to_list::new ap write in blacklist, addr->%02x:%02x:%02x:%02x:XX:XX.}",
            hmac_vap->bssid[0], hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]); /* 1,2,3表示数组下标 */

        /* 换一种action帧发送,看是否对端能退化到siso数据方式 */
        hmac_m2s_send_action_frame(hmac_vap);
        hmac_m2s_switch_protect_trigger(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_assoc_vap_find_in_device_blacklist
 功能描述  : command切换事件类型查询，
             (1)在黑名单中是OTHER直接切换到siso，为NONE就处于miso稳定态
             (2)多vap时，p2p主要跟随sta的方式来直接切换，存在一个黑名单用户就按照(1)来
             (3)不在黑名单里，直接处于miso过渡态
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_assoc_vap_find_in_device_blacklist(const hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx;
    osal_u8 index;
    osal_u8 up_vap_num;
    osal_u8  mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { 0 };
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_assoc_vap_find_in_device_blacklist::param null.}");
        return OSAL_FALSE;
    }

    /* 只看自己的hal device上找 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S,
                "vap_id[%d] {hmac_m2s_assoc_vap_find_in_device_blacklist::hmac_vap null.}", mac_vap_id[vap_idx]);
            continue;
        }

        /* 1.sta模式时候，此时去关联了但是vap状态还不是fake up，还是能find 找到，需要判断 */
        if (!is_sta(hmac_vap) || hmac_vap->assoc_vap_id == MAC_INVALID_USER_ID) {
            continue;
        }

        /* 2.还存在关联着的vap设备，处于黑名单之中 */
        if (hmac_m2s_check_blacklist_in_list(hmac_vap, &index) == OSAL_TRUE) {
            /* 找到了一个黑名单关联ap */
            m2s_mgr_blacklist = &(get_hal_device_m2s_mgr(hal_device)->m2s_blacklist[index]);
            /* 如果需要切换成none模式，后续保持miso状态 */
            if (m2s_mgr_blacklist->action_type == WLAN_M2S_ACTION_TYPE_NONE) {
                return OSAL_TRUE;
            }
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_check_blacklist_in_list
 功能描述  : 切换发送action帧模式查询
*****************************************************************************/
osal_void hmac_m2s_action_frame_type_query(const hmac_vap_stru *hmac_vap, wlan_m2s_action_type_enum_uint8 *action_type)
{
    osal_u8 index;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_device_m2s_mgr_stru *device_m2s_mgr = OSAL_NULL;
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || action_type == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_action_frame_type_query::param null.}");
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_action_frame_type_query:: hal_device null.}", hmac_vap->vap_id);
        return;
    }

    device_m2s_mgr = get_hal_device_m2s_mgr(hal_device);

    if (hmac_m2s_check_blacklist_in_list(hmac_vap, &index) == OSAL_TRUE) {
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[index]);

        /* 关联ap在黑名单之中，需要按照新的action帧发送类型来发送 */
        *action_type = m2s_mgr_blacklist->action_type;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_send_action_complete_check
 功能描述  : action帧发送成功，切换保护才使能
             (1)action帧要使能重传 (2)切换保护定时器在action帧发送成功之后才启动
*****************************************************************************/
osal_void hmac_m2s_send_action_complete_check(hmac_vap_stru *hmac_vap, const mac_tx_ctl_stru *tx_ctl)
{
    /* 发送完成中断succ时候，判断cb帧类型符合smps或者opmode，m2s切换逻辑继续执行下去 */
    if (mac_get_cb_is_opmode_frame(tx_ctl) || mac_get_cb_is_smps_frame(tx_ctl)) {
        hmac_vap_get_vap_m2s(hmac_vap)->action_send_state = OSAL_TRUE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_send_action_frame
 功能描述  : 准备发送smps/opmode帧
*****************************************************************************/
osal_void hmac_m2s_send_action_frame(hmac_vap_stru *hmac_vap)
{
    wlan_m2s_action_type_enum_uint8 action_type;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_send_action_frame::param null.}");
        return;
    }

    /* 暂时只有STA模式才支持发送action帧 */
    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) {
        /* 1.初始切换方案 */
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        /* 部分三四空间流路由器，在HT协议下也不支持OPMODE帧, 根据opmode能力来选择发帧 */
        /* 存在对端beacon携带vht，最终关联在ht协议下，仍然需要发送smps帧提高兼容性 */
        if (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OSAL_TRUE &&
            (hmac_vap->cap_flag.opmode == OSAL_TRUE) &&
            (hmac_vap->protocol == WLAN_VHT_MODE || hmac_vap->protocol == WLAN_VHT_ONLY_MODE ||
            hmac_vap->protocol == WLAN_HE_MODE)) {
            action_type = WLAN_M2S_ACTION_TYPE_OPMODE;
        } else {
            action_type = WLAN_M2S_ACTION_TYPE_SMPS;
        }
#else
        action_type = WLAN_M2S_ACTION_TYPE_SMPS;
#endif

        /* 2.刷新sta初始切换action配置模式，用于后续兼容性路由器的特殊处理 */
        hmac_vap_get_vap_m2s(hmac_vap)->m2s_switch_ori_type = action_type;

        /* 3.根据黑名单进一步更新帧发送类型 */
        hmac_m2s_action_frame_type_query(hmac_vap, &action_type);

        /* 5.开启action帧发送开关 */
        hmac_vap_get_vap_m2s(hmac_vap)->action_send_state = OSAL_FALSE;

        /* 6.发送action帧 */
        if (action_type == WLAN_M2S_ACTION_TYPE_OPMODE) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
            hmac_mgmt_send_opmode_notify_action(hmac_vap, hmac_vap->vap_rx_nss, OSAL_FALSE);
#endif
        } else if (action_type == WLAN_M2S_ACTION_TYPE_SMPS) {
#ifdef _PRE_WLAN_FEATURE_SMPS
            hmac_smps_send_action(hmac_vap, mac_m2s_cali_smps_mode(hmac_vap->vap_rx_nss), OSAL_FALSE);
#endif
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_d2h_vap_info_syn
 功能描述  : hmac_offload架构下同步vap的m2s信息到hmac(user涉及的mimo-siso信息差异)
*****************************************************************************/
osal_u32 hmac_m2s_d2h_vap_info_syn(hmac_vap_stru *hmac_vap)
{
    wlan_mib_mimo_power_save_enum_uint8 smps_mode;
    osal_s32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_d2h_vap_info_syn::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_d2h_vap_info_syn::hal_to_dmac_device_stru null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 软切换之后，重新关联上，vap的nss能力要根据hal device的nss能力重新刷新 */
    hmac_vap_set_rx_nss_etc(hmac_vap, hal_device->cfg_cap_info->nss_num);

    /* 当前关联时，要根据spec再次刷新rx nss能力; add时是butt关系不大 */
    if (mac_vap_is_2g_spec_siso(hmac_vap) || mac_vap_is_5g_spec_siso(hmac_vap)) {
        hmac_vap->vap_rx_nss = WLAN_SINGLE_NSS;
    }

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* GO等重新vap init之后，切换到siso时候还是赋值的mimo的smps，此时需要根据vap nss取小 */
    smps_mode = mac_mib_get_smps(hmac_vap);
    smps_mode = osal_min(smps_mode, mac_m2s_cali_smps_mode(hmac_vap->vap_rx_nss));

    mac_mib_set_smps(hmac_vap, smps_mode);
#endif

    /***************************************************************************
        抛事件到HMAC层, 同步USER m2s能力到HMAC
    ***************************************************************************/
    ret = hmac_config_d2h_vap_m2s_info_syn(hmac_vap, hal_device->hal_m2s_fsm.m2s_type);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_M2S, "vap_id[%d] {hmac_m2s_d2h_vap_info_syn::dmac_send_msg_to_host failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :根据定制化文件切换m2s状态
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_custom_switch_check(const hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 up_vap_num;
    osal_u8                      mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 1.不是单vap，不支持 */
    if (up_vap_num != 1) {
        return OSAL_FALSE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[0]);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_M2S, "vap_id[%d] hmac_m2s_custom_switch_check::hmac_vap IS NULL.", mac_vap_id[0]);
        return OSAL_FALSE;
    }

    /* 2.是支持切换的ap设备，理论上5g不会进入此判断，后续可以优化 */
    if (is_legacy_ap(hmac_vap) &&
        (((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_2GAP) != 0 && hmac_vap->channel.band == WLAN_BAND_2G) ||
        ((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_5GAP) != 0 && hmac_vap->channel.band == WLAN_BAND_5G))) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_command_mode_force_to_mimo
 功能描述  : mss模式下强制回mimo，miso过渡态回mimo，siso稳定态回mimo
*****************************************************************************/
osal_void hmac_m2s_command_mode_force_to_mimo(hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_command_mode_force_to_mimo::param null.}");
        return;
    }

    /* MSS存在时，不会有其他m2s业务存在，此时直接回mimo即可; 标志和切换保护功能在此回mimo时清 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) {
        hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO, 0, OSAL_NULL);
    } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO) {
        hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO, 0, OSAL_NULL);
    } else {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_command_mode_force_to_mimo: m2s_mode[%d]m2s_cur_state[%d] can't resume mimo.}",
            get_hal_m2s_mode_tpye(hal_device), hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_spec_support_siso_switch_check
 功能描述  : spec模式下特点业务模式是否支持切换siso  后续考虑放开
             (1)5g 单天线模式下，异频dbac下，btcoex不支持切换siso
             (2)test模式不在idle下，支持spec就不允许切换siso，(5g启动下不支持切siso)其他约束测试行为
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_spec_support_siso_switch_check(const hal_to_dmac_device_stru *hal_device,
    const hmac_device_stru *hmac_device, wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    osal_u8 up_vap_num;
    osal_u8 up_2g_num = 0;
    osal_u8 up_5g_num = 0;
    osal_u8 vap_index;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *hmac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OSAL_NULL};

    if (osal_unlikely(hal_device == OSAL_NULL || hmac_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_support_siso_switch_check::param null.}");
        return OSAL_FALSE;
    }

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap[vap_index] = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap[vap_index] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S,
                "vap_id[%d] {hmac_m2s_spec_support_siso_switch_check::hmac_vap IS NULL.}", mac_vap_id[vap_index]);
            return OSAL_FALSE;
        }

        (hmac_vap[vap_index]->channel.band == OSAL_TRUE) ? (up_2g_num++) : (up_5g_num++);
    }

    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_BTCOEX) {
        if (!!mac_is_dbac_running(hmac_device) && up_2g_num != 0 && up_5g_num != 0 &&
            hal_get_chip_stru()->rf_custom_mgr.nss_num_5g == WLAN_SINGLE_NSS) {
            return OSAL_FALSE;
        }
    } else if (trigger_mode == WLAN_M2S_TRIGGER_MODE_TEST) {
        if (up_vap_num == 1) {
            if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap[0])) {
                return OSAL_FALSE;
            }
        } else if (up_vap_num == 2) {   /* 2个vap场景 */
            if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap[0]) || mac_vap_spec_is_sw_need_m2s_switch(hmac_vap[1])) {
                return OSAL_FALSE;
            }
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_spec_update_vap_sw_capbility
 功能描述  : 更新vap的m2s软件能力
             (1)基于当前关联band，根据spec cap info刷新siso/mimo能力
             (2)硬件侧能力，在配置band到硬件时来处理
             (3)这里和其他切siso业务冲突会比较大，后续注意测试解决
             (4)如果其他m2s业务做了切换，mac device会是siso的，也自然满足需求
*****************************************************************************/
osal_void hmac_m2s_spec_update_vap_sw_capbility(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    wlan_nss_enum_uint8 nss_num)
{
    oal_bool_enum_uint8 tx_stbc_is_supp;  /* 是否支持最少2x1 STBC发送 */
    oal_bool_enum_uint8 su_bfmer_is_supp; /* 是否支持单用户beamformer */

    if (osal_unlikely(hmac_device == OSAL_NULL || hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_update_vap_sw_capbility::param null.}");
        return;
    }

    if (nss_num == WLAN_SINGLE_NSS) {
        tx_stbc_is_supp = OSAL_FALSE;
        su_bfmer_is_supp = OSAL_FALSE;
    } else {
        tx_stbc_is_supp = OSAL_TRUE;
        su_bfmer_is_supp = OSAL_TRUE;
    }

    hmac_vap_set_rx_nss_etc(hmac_vap, nss_num);

    /* 2.修改HT能力 */
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 修改smps能力, 直接通知smps的来修改流程复杂，这里mimo/siso 相关mib能力统一管理 */
    mac_mib_set_smps(hmac_vap, mac_m2s_cali_smps_mode(nss_num));
#endif
    /* 修改tx STBC能力 */
    mac_mib_set_tx_stbc_option_implemented(hmac_vap, tx_stbc_is_supp & hmac_device->device_cap.tx_stbc_is_supp);
    mac_mib_set_vht_ctrl_field_cap(hmac_vap, nss_num); /* 代码目前都是写FALSE,后续再看优化整改需求 */

    /* 没有对应hal device的mib值，暂时按照nss来赋值， 后续看是否优化 */
    mac_mib_set_vht_num_sounding_dimensions(hmac_vap, osal_min(nss_num, hmac_device->device_cap.nss_num));
    mac_mib_set_vht_su_bfer_option_implemented(hmac_vap, su_bfmer_is_supp & hmac_device->device_cap.su_bfmer_is_supp);
    mac_mib_set_vht_tx_stbc_option_implemented(hmac_vap, tx_stbc_is_supp & hmac_device->device_cap.tx_stbc_is_supp);

    unref_param(su_bfmer_is_supp);
    /* 5.同步vap 的m2s能力到hmac */
    if (hmac_m2s_d2h_vap_info_syn(hmac_vap) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_spec_update_vap_sw_capbility::hmac_m2s_d2h_vap_info_syn failed.}", hmac_vap->vap_id);
    }

    oam_warning_log4(0, OAM_SF_M2S,
        "{hmac_m2s_spec_update_vap_sw_capbility::need to reflush vap id[%d]mode[%d]band[%d]nss[%d] sw m2s cap.}",
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->channel.band, nss_num);
}

osal_void hmac_m2s_spec_update_hal_device_proc(hal_to_dmac_device_stru *hal_device,
    osal_u8 spec_tx_chain, hal_m2s_event_tpye_uint16 *m2s_event)
{
    osal_u8 m2s_mode_mask = 0;
    osal_void *fhook = OSAL_NULL;

    /* mss rssi逻辑下会处于miso状态，需要考虑miso状态下事件处理 */
    /* 如果只有低优先级的MSS,默认是c0 siso, mss暂时不考虑，直接清mss标记，然后切c1 siso即可 */
    if (hal_m2s_check_rssi_on(hal_device) == OSAL_TRUE) {
        /* 如果处于miso探测态，需要停止切换保护，切换到siso，已经是siso的就默认c0 siso不动 */
        if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) {
            *m2s_event = HAL_M2S_EVENT_SPEC_MISO_TO_SISO_C0;
        }

        /* 切换保护功能关闭 */
        get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;

        /* 清业务标志 */
        get_hal_m2s_mode_tpye(hal_device) &= (~(WLAN_M2S_TRIGGER_MODE_COMMAND | WLAN_M2S_TRIGGER_MODE_RSSI));

        get_hal_m2s_mode_tpye(hal_device) |= WLAN_M2S_TRIGGER_MODE_SPEC;
        /* 如果2g ap custom, 暂时ap模式不会出现此情况 */
        return;
    }

    if (hal_m2s_check_custom_on(hal_device) == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{hmac_m2s_spec_update_hal_device_proc:: custom on keep siso, mode[%d].}",
            get_hal_m2s_mode_tpye(hal_device));
        /* 如果test, 要保持配置 */
        return;
    }

    if (hal_m2s_check_test_on(hal_device) == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_spec_update_hal_device_proc:: test on keep siso, mode[%d].}",
            get_hal_m2s_mode_tpye(hal_device));
        return;
    }
    if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
        /* btcoex切换siso是在2g siso，spec回到2g或者本身2g想回mimo时候需要保持c1siso
          切换到5g会在其他逻辑保证btcoex清掉 如果是dbac切换的话，btcoex本身不处理切siso  也不用考虑此场景 */
        return;
    }
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_M2S_UPDATE_HAL_DEVICE);
    if (fhook != OSAL_NULL &&
        ((hmac_btcoex_m2s_update_hal_device_cb)fhook)(hal_device, &m2s_mode_mask) == OSAL_TRUE) {
        return;
    }

    /* 根据期望状态，申请m2s切换 */
    if (spec_tx_chain == WLAN_RF_CHAIN_ZERO) {
        *m2s_event = HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0;
    } else if (spec_tx_chain == WLAN_RF_CHAIN_ONE) {
        *m2s_event = HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1;
    } else {
        *m2s_event = HAL_M2S_EVENT_SPEC_SISO_TO_MIMO;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_spec_update_hal_device_capbility
 功能描述  : 更新hal device 的m2s硬件能力
             (1)基于当前关联band，根据spec hal chip刷新device chain;resp chain;mac+phy寄存器
             (2)硬件侧能力，在配置band到硬件时来处理; m2s切换本身不配置band，不涉及，业务侧做好冲突处理即可
             (3)如何做到重复配置约束，
                scan下配置只刷resp chain即可(初始是默认开双通道)
                work状态下配置，需要整个刷新，为了重复，配置时和当前hal device能力做比较，不一致才刷
             (4)新增两个标志，band变化，chain变化，出现band变化时需要立即刷新
*****************************************************************************/
osal_void hmac_m2s_spec_update_hal_device_capbility(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band)
{
    hal_m2s_event_tpye_uint16 m2s_event = HAL_M2S_EVENT_BUTT;
    osal_u8 spec_tx_chain = WLAN_PHY_CHAIN_DOUBLE;
    hal_chip_stru *hal_chip = hal_get_chip_stru();

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_update_hal_device_capbility::param null.}");
        return;
    }

    /* 2.根据spec刷新标志，本身是mimo定制的，就不刷新，跟随业务处理即可
         如果已经在dbac或者并发扫描，虽然不需要处理，但是需要置spec标志，用于多业务管理 */
    if ((band == WLAN_BAND_2G) && (hal_chip->rf_custom_mgr.nss_num_2g == WLAN_SINGLE_NSS)) {
        spec_tx_chain = hal_chip->rf_custom_mgr.support_rf_chain_2g;
    } else if ((band == WLAN_BAND_5G) && (hal_chip->rf_custom_mgr.nss_num_5g == WLAN_SINGLE_NSS)) {
        spec_tx_chain = hal_chip->rf_custom_mgr.support_rf_chain_5g;
    }

    // 此时即使在dbac和并发扫描状态，也需要置spec标志，然后dbdc退出时候能保持
    /* 3.
     * dbdc和并发扫描存在时，会自动切换到siso模式，此时不需要单独做刷新，要么这两个标志会比spec提前置，要么spec先;其他m2s模式暂不需要考虑处理
     */
    if (hal_m2s_check_fast_scan_on(hal_device) == OSAL_TRUE) {
        /*  如果已经在dbac或者并发扫描，虽然不需要处理，但是需要置spec标志，用于多业务管理 */
        if (spec_tx_chain != WLAN_PHY_CHAIN_DOUBLE) {
            get_hal_m2s_mode_tpye(hal_device) |= WLAN_M2S_TRIGGER_MODE_SPEC;
        } else {
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_SPEC);
        }

        return;
    }

    /* 3.1 为了解决扫描结束没有办法切回mimo，如果普通扫描下，只切换硬件能力不做状态切换 */
    /* 如果rf只有一个的话，phy通道也是需要刷新，还是修改成scan_end在siso时收到并且spec在的话，回到mimo */
    /* spec和当前device的rf能力不一致时，进行刷新，当前band支持双天线，就默认刷成c0 后续rssi逻辑再重新刷;单天线直接刷 */
    /* 信道切换时，此时chain相等，可以不处理此处 */
    if (spec_tx_chain != hal_device->cfg_cap_info->rf_chain) {
        hmac_m2s_spec_update_hal_device_proc(hal_device, spec_tx_chain, &m2s_event);

        /* 执行默认的软切换,切换硬件能力即可 */
        if (m2s_event != HAL_M2S_EVENT_BUTT) {
            oam_warning_log4(0, OAM_SF_M2S,
                "{hmac_m2s_spec_update_hal_device_capbility:: state[%d] rf chain[%d]band[%d]m2s_event[%d] switch.}",
                hal_device->hal_dev_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->rf_chain, band, m2s_event);
            hmac_m2s_handle_event(hal_device, m2s_event, 0, OSAL_NULL);
        }
    } else {
        /* 如果正好相等，比如是c0 siso，就直接置spec标志即可 */
        if (spec_tx_chain != WLAN_RF_CHAIN_DOUBLE) {
            get_hal_m2s_mode_tpye(hal_device) |= WLAN_M2S_TRIGGER_MODE_SPEC;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_spec_mode_force_to_mimo
 功能描述  : spec模式下强制回mimo
             (1)当前siso规格band全部去关联，回到mimo
*****************************************************************************/
osal_void hmac_m2s_spec_mode_force_to_mimo(hal_to_dmac_device_stru *hal_device)
{
    /* 判断当前是否还有up的vap，确实是否需要回到mimo */
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 up_vap_num;
    osal_u8                    mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    oal_bool_enum_uint8 back_mimo = OSAL_FALSE;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_mode_force_to_mimo::param null.}");
        return;
    }

    /* 找到所有up的vap设备 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (up_vap_num == 0) {
        back_mimo = OSAL_TRUE;
    } else if (up_vap_num == 1) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[0]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S, "hmac_m2s_spec_mode_force_to_mimo::hmac_vap[%d] IS NULL.", mac_vap_id[0]);
            return;
        }

        /* 另一个vap 当前2g 5g都是mimo spec的话，需要回到mimo */
        if (!mac_vap_is_2g_spec_siso(hmac_vap) || !mac_vap_is_5g_spec_siso(hmac_vap)) {
            back_mimo = OSAL_TRUE;
        }
    } else {
        oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_spec_mode_force_to_mimo::not support 2 vaps.");
    }

    if (back_mimo == OSAL_TRUE) {
        /* 修订直接硬件能力刷新回mimo即可，需要判断不影响其他业务,其他业务在的话，要保持，优先执行其他业务判断逻辑 */
        if ((get_hal_m2s_mode_tpye(hal_device) & (~WLAN_M2S_TRIGGER_MODE_SPEC)) == 0) {
            oam_warning_log1(0, OAM_SF_M2S,
                "{hmac_m2s_spec_mode_force_to_mimo::vap up num[%d] spec need to resume mimo.}", up_vap_num);

            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_SPEC_SISO_TO_MIMO, 0, OSAL_NULL);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_spec_need_switch_keep_check
 功能描述  : 其他业务结束之后，在判断是否需要回到mimo时，是否需要继续基于spec切到siso，保证业务正常
             (1)当前只是dbdc退出才需要，根据当前业务优先级
             (2)dbac不在此处理范围
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_spec_need_switch_keep_check(const hal_to_dmac_device_stru *hal_device)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_chip_stru *hal_chip = OSAL_NULL;
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 up_2g_vap_num = 0;
    osal_u8 up_5g_vap_num = 0;
    osal_u8                      mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    wlan_channel_band_enum_uint8 band;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_need_switch_keep_check::param null.}");
        return OSAL_FALSE;
    }

    if (mac_is_dbac_running(hmac_device)) {
        oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_spec_need_switch_keep_check: dbac is running.}");
        return OSAL_FALSE;
    }

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        mac_vap_temp = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (mac_vap_temp == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] hmac_m2s_spec_need_switch_keep_check::hmac_vap IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        /* 记录2g 5g vap个数 */
        (mac_vap_temp->channel.band == WLAN_BAND_2G) ? (up_2g_vap_num++) : (up_5g_vap_num++);
    }

    /* 1.前景扫描，直接回mimo */
    if (up_vap_num == 0) {
        return OSAL_FALSE;
    }

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 2.还有2G 5G spec时需要保持在当前c0 siso */
    /* 保证该函数是在配置了硬件band之后,判断当前工作spec siso band，是否存在工作vap */
    band = hal_device->wifi_channel_status.band;
    if (((band == WLAN_BAND_5G) && (hal_chip->rf_custom_mgr.nss_num_5g == WLAN_SINGLE_NSS)) ||
        ((band == WLAN_BAND_2G) && (hal_chip->rf_custom_mgr.nss_num_2g == WLAN_SINGLE_NSS))) {
        if ((up_2g_vap_num != 0 && up_5g_vap_num == 0 &&
            hal_chip->rf_custom_mgr.nss_num_2g == WLAN_SINGLE_NSS) ||
            (up_5g_vap_num != 0 && up_2g_vap_num == 0 &&
            hal_chip->rf_custom_mgr.nss_num_5g == WLAN_SINGLE_NSS)) {
            {
                oam_warning_log0(0, OAM_SF_COEX,
                    "hmac_m2s_spec_need_switch_keep_check::need to keep at siso.");
                return OSAL_TRUE;
            }
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_rx_ucast_nss_count_handle
 功能描述  : nss数据统计处理，根据不同门限确认切换方案是否调整
*****************************************************************************/
osal_u32 hmac_m2s_rx_ucast_nss_count_handle(hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device;
    hmac_vap_m2s_rx_statistics_stru *m2s_rx_statistics;
    hal_m2s_event_tpye_uint16 event_type = HAL_M2S_EVENT_BUTT;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_rx_ucast_nss_count_handle::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2s_rx_statistics = hmac_vap_get_vap_m2s_rx_statistics(hmac_vap);

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_rx_ucast_nss_count_handle: hal_device is null ptr}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.可能此时已经切回了mimo，业务结束，直接返回不处理即可 */
    if (get_hal_device_m2s_switch_prot(hal_device) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    oam_warning_log3(0, OAM_SF_M2S,
        "vap_id[%d] {hmac_m2s_rx_ucast_nss_count_handle:: rx_nss_mimo_count:%d, rx_nss_siso_counte:%d.}",
        hmac_vap->vap_id, m2s_rx_statistics->rx_nss_mimo_count, m2s_rx_statistics->rx_nss_siso_count);

    /*
       在切换后2s之后，3s统计时间内，需要对端不发mimo包(至少有siso包)
       1.该功能需要上层配合，连续发送10个ping包， 测试时候需要跑流或者ping包
       2. 没有mimo包，有很多siso包正常，目前门限都是5个
       3.针对环境可能比较差，对端只有siso包这个场景，暂时不考虑，理论上也不太可能
    */
    if (m2s_rx_statistics->rx_nss_mimo_count < M2S_MIMO_RX_CNT_THRESHOLD) {
        /* 满足切换方案要求，直接miso切换到siso即可 */
        if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
            (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO) ?
                (event_type = HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0) :
                (event_type = HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1);
        }

        /* 本vap切换完成，置标志位 */
        hmac_vap_get_vap_m2s(hmac_vap)->m2s_wait_complete = OSAL_FALSE;

        /* 申请切换到siso仲裁，从而支持多vap切换，该接口完成之后将切换结果上报上层 */
        hmac_m2s_switch_protect_complete_apply(hal_device, event_type);
    } else {
        /* 对端还是发mimo数据为主，说明对端对此action帧处理不好，需要换切换方案,加入黑名单 */
        hmac_m2s_add_and_update_blacklist_to_list(hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_rx_rate_nss_process
 功能描述  : rx方向做nss数据统计
*****************************************************************************/
osal_void hmac_m2s_rx_rate_nss_process(hmac_vap_stru *hmac_vap, const dmac_rx_ctl_stru *cb_ctrl,
    const mac_ieee80211_frame_stru *frame_hdr)
{
    hmac_vap_m2s_rx_statistics_stru *m2s_rx_statistics;
    wlan_phy_protocol_enum_uint8 phy_protocol;

    if (osal_unlikely(hmac_vap == OSAL_NULL || cb_ctrl == OSAL_NULL || frame_hdr == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_rx_rate_nss_process::param null.}");
        return;
    }

    m2s_rx_statistics = hmac_vap_get_vap_m2s_rx_statistics(hmac_vap);
    /* 1.统计开启时做统计，有数据收发之后才开始统计 */
    if (m2s_rx_statistics->rx_nss_statistics_start_flag == OSAL_FALSE) {
        return;
    }

    /* 2.vip帧或者广播帧不统计，可能属于siso帧 */
    if (cb_ctrl->rx_info.is_key_frame == OSAL_TRUE || ether_is_multicast(frame_hdr->address1)) {
        return;
    }

    /* 3.获取数据帧的nss类型，主要是判断mimo帧情况，以及siso是不是通
         (1)不需要启动定时器，因为可能本身处于低功耗，没有数据收发，那就在miso下保持足够久
         (2)有数据收发了，统计连续10个数据帧，判断这10个包的mimo和siso情况 */
    m2s_rx_statistics->rx_nss_ucast_count++;

    phy_protocol = cb_ctrl->rx_statistic.nss_rate.legacy_rate.protocol_mode;
    if (phy_protocol == WLAN_HT_PHY_PROTOCOL_MODE) {
        if (cb_ctrl->rx_statistic.nss_rate.ht_rate.ht_mcs >= WLAN_HT_MCS8 &&
            cb_ctrl->rx_statistic.nss_rate.ht_rate.ht_mcs != WLAN_HT_MCS32) {
            m2s_rx_statistics->rx_nss_mimo_count++;
        } else {
            m2s_rx_statistics->rx_nss_siso_count++;
        }
    } else if (phy_protocol == WLAN_VHT_PHY_PROTOCOL_MODE) {
        if (cb_ctrl->rx_statistic.nss_rate.vht_nss_mcs.nss_mode == 1) {
            m2s_rx_statistics->rx_nss_mimo_count++;
        } else {
            m2s_rx_statistics->rx_nss_siso_count++;
        }
    } else {
        m2s_rx_statistics->rx_nss_siso_count++;
    }

    /* 4. 达到数据统计门限，开始计算 */
    if (m2s_rx_statistics->rx_nss_ucast_count == M2S_RX_UCAST_CNT_THRESHOLD) {
        m2s_rx_statistics->rx_nss_statistics_start_flag = OSAL_FALSE;
        hmac_m2s_rx_ucast_nss_count_handle(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_delay_switch_nss_statistics_callback
 功能描述  : nss统计超时定时器，开启统计功能
*****************************************************************************/
osal_u32 hmac_m2s_delay_switch_nss_statistics_callback(osal_void *arg)
{
    hal_to_dmac_device_stru *hal_device;
    hmac_vap_m2s_rx_statistics_stru *m2s_rx_statistics;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_delay_switch_nss_statistics_callback::param null.}");
        return OAL_FAIL;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_delay_switch_nss_statistics_callback::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    m2s_rx_statistics = hmac_vap_get_vap_m2s_rx_statistics(hmac_vap);

    /* 1.业务结束 */
    if (get_hal_device_m2s_switch_prot(hal_device) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    /* 2.action帧发送成功，才继续执行下去，否则继续发action帧，并再次打开统计 */
    if (hmac_vap_get_vap_m2s(hmac_vap)->action_send_state == OSAL_FALSE &&
        M2S_ACTION_SENT_CNT_THRESHOLD > hmac_vap_get_vap_m2s(hmac_vap)->action_send_cnt) {
        hmac_m2s_send_action_frame(hmac_vap);

        /* 初始是0，这里进行累加 */
        hmac_vap_get_vap_m2s(hmac_vap)->action_send_cnt++;
        frw_create_timer_entry(&(m2s_rx_statistics->m2s_delay_switch_statistics_timer),
            hmac_m2s_delay_switch_nss_statistics_callback, M2S_RX_STATISTICS_START_TIME, hmac_vap, OAL_FALSE);
    } else {
        /* 3. action帧发送成功，清空统计值，开始统计nss */
        m2s_rx_statistics->rx_nss_mimo_count = 0;
        m2s_rx_statistics->rx_nss_siso_count = 0;
        m2s_rx_statistics->rx_nss_ucast_count = 0;
        m2s_rx_statistics->rx_nss_statistics_start_flag = OSAL_TRUE;
        hmac_vap_get_vap_m2s(hmac_vap)->action_send_cnt = 0;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_protect_trigger
 功能描述  : 切换保护触发到host进行统计
*****************************************************************************/
osal_void hmac_m2s_switch_protect_trigger(hmac_vap_stru *hmac_vap)
{
    hmac_vap_m2s_rx_statistics_stru *m2s_rx_statistics = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_protect_trigger::param null.}");
        return;
    }

    if (!is_sta(hmac_vap)) {
        return;
    }

    m2s_rx_statistics = hmac_vap_get_vap_m2s_rx_statistics(hmac_vap);

    /* 1.切换标志置true,开始切换 */
    hmac_vap_get_vap_m2s(hmac_vap)->m2s_wait_complete = OSAL_TRUE;

    /* 2.nss统计暂时功能打开,等超时定时到期才开始统计 */
    m2s_rx_statistics->rx_nss_statistics_start_flag = OSAL_FALSE;

    /* 3.创建统计定时器,1s之后开始统计双流数据包个数,会有部分设备，硬件队列还有部分mimo包要先发完才切换到siso发送 */
    if (m2s_rx_statistics->m2s_delay_switch_statistics_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(m2s_rx_statistics->m2s_delay_switch_statistics_timer));
    }
    frw_create_timer_entry(&(m2s_rx_statistics->m2s_delay_switch_statistics_timer),
        hmac_m2s_delay_switch_nss_statistics_callback, M2S_RX_STATISTICS_START_TIME, hmac_vap, OAL_FALSE);
}

/*****************************************************************************
 函 数 名  : hmac_config_m2s_assoc_state_syn
 功能描述  : sta模式关联上时，根据黑名单里面情况，做device miso能力置位处理
             (1)兼容dbdc，需要提前到join之前
*****************************************************************************/
osal_void hmac_config_m2s_assoc_state_syn(const hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u8 index;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_config_m2s_assoc_state_syn::param null.}");
        return;
    }

    /* 1.只处理sta模式 */
    if (!is_sta(hmac_vap)) {
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_m2s_assoc_state_syn: hal_device is null.}", hmac_vap->vap_id);
        return;
    }

    /* 2.该ap是否在黑名单中，是的话，device能力提前置位miso，后续切换不启动探测逻辑 */
    if (hmac_m2s_check_blacklist_in_list(hmac_vap, &index) == OSAL_TRUE) {
        /* 找到了一个黑名单关联ap 如果需要切换成none模式，后续保持miso状态 */
        if (get_hal_device_m2s_mgr(hal_device)->m2s_blacklist[index].action_type == WLAN_M2S_ACTION_TYPE_NONE) {
            get_hal_device_m2s_del_swi_miso_hold(hal_device) = OSAL_TRUE;

            oam_warning_log1(0, OAM_SF_M2S,
                "vap_id[%d] {hmac_config_m2s_assoc_state_syn: ap in blacklist, need keep at miso.}", hmac_vap->vap_id);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_m2s_disassoc_state_syn
 功能描述  : sta模式去关联，需要将对应m2s统计结构清零处理
*****************************************************************************/
osal_void hmac_config_m2s_disassoc_state_syn(hmac_vap_stru *hmac_vap)
{
    hmac_vap_m2s_stru *hmac_vap_m2s = OSAL_NULL;
    hmac_vap_m2s_rx_statistics_stru *m2s_rx_statistics = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_config_m2s_disassoc_state_syn::param null.}");
        return;
    }

    if (!is_sta(hmac_vap)) {
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_config_m2s_disassoc_state_syn: hal_device is null ptr.}", hmac_vap->vap_id);
        return;
    }

    hmac_vap_m2s = hmac_vap_get_vap_m2s(hmac_vap);
    m2s_rx_statistics = &(hmac_vap_m2s->hmac_vap_m2s_rx_statistics);
    frw_destroy_timer_entry(&(m2s_rx_statistics->m2s_delay_switch_statistics_timer));

    /* 1.统计信息统一清零 */
    (osal_void)memset_s(hmac_vap_m2s, sizeof(hmac_vap_m2s_stru), 0, sizeof(hmac_vap_m2s_stru));

    /* 2.device下不存在黑名单的关联ap时，device miso保持标记要清零，恢复可探测 */
    if (hmac_m2s_assoc_vap_find_in_device_blacklist(hal_device) == OSAL_FALSE) {
        get_hal_device_m2s_del_swi_miso_hold(hal_device) = OSAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_update_user_capbility
 功能描述  : 更新vap下用户m2s能力
*****************************************************************************/
osal_void hmac_m2s_update_user_capbility(hmac_user_stru *hmac_user, const hmac_vap_stru *hmac_vap)
{
    wlan_nss_enum_uint8 old_avail_nss;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(hmac_user == OSAL_NULL || hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_update_user_capbility::param null.}");
        return;
    }

    /* 用户空间流 */
    old_avail_nss = hmac_user->avail_num_spatial_stream;

    /* 更新之后的user空间流能力 */
    hmac_user_set_avail_num_spatial_stream_etc(hmac_user,
        osal_min(hmac_vap->vap_rx_nss, hmac_user->user_num_spatial_stream));

    /* 用户空间流能力没有发生变化也需要同步，可能此时hal phy chain能力变化，特定算法模块需要对应处理 */
    /* user空间流变化通知btcoex刷新速率门限 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_USER_SPATIAL_STREAM_CHANGE);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_user_spatial_stream_change_notify_cb)fhook)(hmac_vap, hmac_user);
    }

    /* 用户空间流能力发生变化，需要通知算法 */
    if (old_avail_nss != hmac_user->avail_num_spatial_stream) {
        /* 空间流更新 则同步至device */
        hmac_user_sync(hmac_user);
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_init_tx_frame_txchain_param
 功能描述  : 主要是发送m2s切换后，初始化所有数据帧和管理帧的发送通道
             (0)要兼容前景扫描，本hal device上所有vap都需要刷新
             (1)单播数据帧
             (2)单播和广播管理帧
             (3)组播广播数据帧
*****************************************************************************/
osal_u32  hmac_vap_init_tx_frame_txchain_params(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8  mgmt_rate_init_flag)
{
    osal_u8       protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    osal_u8       legacy_rate   = WLAN_PHY_RATE_6M;
    osal_u8       index;

    if (osal_unlikely(hmac_vap->hal_device == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 1.初始化单播数据帧发送通道 */
    /* 2.初始化管理帧发送通道 */
    if (mgmt_rate_init_flag == OSAL_TRUE) {
        /* 初始化单播管理帧参数 */
        /* 初始化组播、广播管理帧参数 */
        /* 获取管理帧速率和协议 */
        hmac_vap_get_legacy_rate_and_protocol(hmac_vap, WLAN_BAND_2G, &protocol_mode, &legacy_rate);
        /* 4表示每个用户支持的最大速率集个数 */
        for (index = 0; index < 4; index++) {
            hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[WLAN_BAND_2G]))[index].tx_chain_sel =
                hmac_vap->hal_device->cfg_cap_info->single_tx_chain;

            hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_bmcast[WLAN_BAND_2G]))[index].tx_chain_sel =
                hmac_vap->hal_device->cfg_cap_info->single_tx_chain;
        }

        hmac_vap_get_legacy_rate_and_protocol(hmac_vap, WLAN_BAND_5G, &protocol_mode, &legacy_rate);
        /* 4表示每个用户支持的最大速率集个数 */
        for (index = 0; index < 4; index++) {
            hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[WLAN_BAND_5G]))[index].tx_chain_sel =
                hmac_vap->hal_device->cfg_cap_info->single_tx_chain;

            hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_bmcast[WLAN_BAND_5G]))[index].tx_chain_sel =
                hmac_vap->hal_device->cfg_cap_info->single_tx_chain;
        }
    }

    /* 3.初始化组播数据帧发送参数 */
    /* 初始化广播数据帧发送参数 */
    if (hmac_vap_get_non_ucast_data_rate_and_protocol(hmac_vap, &protocol_mode, &legacy_rate) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_vap_init_tx_frame_txchain_params::hmac_vap_get_non_ucast_data_rate_and_protocol err!}");
    }
    /* 4表示每个用户支持的最大速率集个数 */

    return OAL_SUCC;
}

osal_void hmac_m2s_update_tx_param(const hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    oal_bool_enum send_action)
{
    struct osal_list_head *entry = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;

    /* 硬切换 */
    if (hal_device->hal_m2s_fsm.m2s_type == WLAN_M2S_TYPE_HW) {
        /* 4. 更新速率集，vap的空间流，暂时未看到其他需要更新的地方 */
        hmac_vap_init_rates_etc(hmac_vap);
    }

    /* 5. 重新初始化数据帧/管理帧发送参数 */
    hmac_vap_init_tx_frame_txchain_params(hmac_vap, OSAL_TRUE);

    /* 6.同步vap 的m2s能力到hmac */
    if (hmac_m2s_d2h_vap_info_syn(hmac_vap) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_update_vap_capbility::hmac_m2s_d2h_vap_info_syn failed.}", hmac_vap->vap_id);
    }

    if (hal_device->hal_m2s_fsm.m2s_type == WLAN_M2S_TYPE_SW) {
        /* 7.刷新用户信息，并通知算法更改tx参数 */
        osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
            hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
            if (osal_unlikely(hmac_user == OSAL_NULL)) {
                oam_warning_log1(0, OAM_SF_M2S,
                    "vap_id[%d] {hmac_m2s_update_vap_capbility::mac_user null pointer.}", hmac_vap->vap_id);
                continue;
            }
            /* 自身是大于HT支持切换，但是user关联状态为非ht和vht，或者对端不支持mimo，也不能支持切换，并且提示异常 */
            if ((hmac_user->user_num_spatial_stream == WLAN_SINGLE_NSS) ||
                (hmac_user->cur_protocol_mode < WLAN_HT_MODE)) {
                oam_warning_log4(0, OAM_SF_M2S,
                    "{hmac_m2s_update_vap_capbility::not support m2s switch id[%d], vap[%d], nss[%d], protocol[%d]!}",
                    hmac_user->assoc_id, hmac_vap->vap_mode, hmac_user->user_num_spatial_stream,
                    hmac_user->cur_protocol_mode);
                continue;
            }

            /* c0切换到c1 也只需要更新vap的非单播数据帧发送参数即可，后续也可以优化 */
            hmac_m2s_update_user_capbility(hmac_user, hmac_vap);

            /* DBDC更新vap以及user能力不需要发action帧,切换到siso模式搬移不需要发帧，回到mimo状态，需要发action帧 */
            if ((hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) || (send_action != OAL_TRUE)) {
                continue;
            }
            /* 8. 发送action帧，通知对端修改m2s能力 */
            hmac_m2s_send_action_frame(hmac_vap);

            /* 9.(1)处于siso状态，并且切换保护使能，启动切换保护触发
                 (2)如果已经是标记需要hold用户的device(该vap是blacklist的none)，也不需要开启切换保护 */
            if (get_hal_device_m2s_switch_prot(hal_device) == OSAL_TRUE &&
                WLAN_SINGLE_NSS == hal_device->cfg_cap_info->nss_num &&
                OSAL_FALSE == get_hal_device_m2s_del_swi_miso_hold(hal_device)) {
                hmac_m2s_switch_protect_trigger(hmac_vap);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_update_vap_capbility
 功能描述  : 更新vap的m2s能力(主路上hal device才支持mimo)
*****************************************************************************/
osal_void hmac_m2s_update_vap_capbility(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    oal_bool_enum send_action = OSAL_TRUE;

    /* vap提前删除，指针已经置为空 */
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_m2s_update_vap_capbility::hal devcie is null!}", hmac_vap->vap_id);
        return;
    }

    /* mimo/siso切换时，更新已关联sta唤醒收bcn的通道 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && (hmac_vap->vap_state == MAC_VAP_STATE_UP ||
        hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) && hmac_vap->hal_vap != OSAL_NULL) {
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
        hmac_psm_update_bcn_rf_chain(hmac_vap, oal_get_real_rssi(hmac_vap->query_stats.signal));
        oam_warning_log3(0, 0, "vap_id[%d] {hmac_m2s_update_vap_capbility:bcn_rx_chain[%d],rssi[%d]}", hmac_vap->vap_id,
            hmac_vap->hal_vap->pm_info.bcn_rf_chain, oal_get_real_rssi(hmac_vap->query_stats.signal));
#else
        hal_pm_set_bcn_rf_chain(hmac_vap->hal_vap, hal_device->cfg_cap_info->rf_chain);
#endif
    }

    // 如果切换到通道2的siso，此时也需要初始化发送参数通道，需要继续走到init_rate 继续走下去; siso支持继续走，本身不支持mimo，不允许切到mimo
    if ((hmac_vap->protocol < WLAN_HT_MODE) && (hal_device->cfg_cap_info->nss_num == WLAN_DOUBLE_NSS)) {
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_update_vap_capbility::the vap is not support HT/VHT!}", hmac_vap->vap_id);
        return;
    }

    // 启动时候需要更新同步到host侧，这里会相等，不做约束，保证其他业务正常
    /* 1. 改变vap下空间流个数能力,用于后续用户接入时与user能力做交集通知算法,工作模式通知ie字段填写 */
    /* 空间流能力不变，可能是c0 siso切换到c1 siso，需要支持 */
    if (hal_device->cfg_cap_info->nss_num == hmac_vap->vap_rx_nss) {
        oam_warning_log2(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_m2s_update_vap_capbility::nss[%d] = mac_vap's.}", hmac_vap->vap_id, hmac_vap->vap_rx_nss);
        send_action = OSAL_FALSE;
    }

    hmac_vap_set_rx_nss_etc(hmac_vap, hal_device->cfg_cap_info->nss_num);

    /* 2.修改HT能力 */
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 修改smps能力, 直接通知smps的来修改流程复杂，这里mimo/siso 相关mib能力统一管理 */
    mac_mib_set_smps(hmac_vap, mac_m2s_cali_smps_mode(hal_device->cfg_cap_info->nss_num));
#endif
    /* 修改tx STBC能力 */
    mac_mib_set_tx_stbc_option_implemented(hmac_vap,
        hal_device->cfg_cap_info->tx_stbc_is_supp & hmac_device->device_cap.tx_stbc_is_supp);
    mac_mib_set_vht_ctrl_field_cap(hmac_vap,
        hal_device->cfg_cap_info->nss_num); /* 代码目前都是写FALSE,后续再看优化整改需求 */

    /* 没有对应hal device的mib值，暂时按照nss来赋值， 后续看是否优化 */
    mac_mib_set_vht_num_sounding_dimensions(hmac_vap,
        osal_min(hal_device->cfg_cap_info->nss_num, hmac_device->device_cap.nss_num));
    mac_mib_set_vht_su_bfer_option_implemented(hmac_vap,
        hal_device->cfg_cap_info->su_bfmer_is_supp & hmac_device->device_cap.su_bfmer_is_supp);
    mac_mib_set_vht_tx_stbc_option_implemented(hmac_vap,
        hal_device->cfg_cap_info->tx_stbc_is_supp & hmac_device->device_cap.tx_stbc_is_supp);

    hmac_m2s_update_tx_param(hal_device, hmac_vap, send_action);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_update_vap_capbility
 功能描述  : mimo-siso切换更新device上vap能力
*****************************************************************************/
osal_void hmac_m2s_switch_update_vap_capbility(const hal_to_dmac_device_stru *hal_device)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx;

    /* 遍历mac device下所有业务vap，刷新所有属于本hal device的vap的能力 */
    /* 注意:硬切换会重新去关联，不能采用hal_device_find_all_up_vap来获取本hal device上的vap，不能覆盖扫描/常发常收模式
     */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S,
                "vap_id[%d] {hmac_m2s_switch_update_vap_capbility::the vap is null!}", hmac_device->vap_id[vap_idx]);
            continue;
        }

        /* 不属于本hal device的vap不进行mimo-siso操作(辅hal device本身不支持mimo) */
        if (hal_device != hmac_vap->hal_device) {
            oam_warning_log1(0, OAM_SF_M2S,
                "{hmac_m2s_switch_update_vap_capbility::vap[%d] not belong to this hal device!}", hmac_vap->vap_id);
            continue;
        }

        /* 更新vap能力 */
        hmac_m2s_update_vap_capbility(hmac_device, hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_update_vap_tx_chain_capbility
 功能描述  : hal chain device能力更新后刷新对应up vap的发送能力
             (1)要考虑前景扫描时候，不能只处理up的
*****************************************************************************/
osal_void hmac_m2s_switch_update_vap_tx_chain_capbility(const hal_to_dmac_device_stru *hal_device)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx;

    /* 遍历mac device下所有业务vap，刷新所有属于本hal device的vap的能力 */
    /* 注意:硬切换会重新去关联，不能采用hal_device_find_all_up_vap来获取本hal device上的vap，不能覆盖扫描/常发常收模式
     */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S,
                "{hmac_m2s_switch_update_vap_tx_chain_capbility::vap[%d] is null!}", hmac_device->vap_id[vap_idx]);
            continue;
        }

        /* 不属于本hal device的vap不进行mimo-siso操作(辅hal device本身不支持mimo) */
        if (hal_device != hmac_vap->hal_device) {
            oam_warning_log1(0, OAM_SF_M2S,
                "{hmac_m2s_switch_update_vap_tx_chain_capbility::vap[%d] not belong to hal device!}", hmac_vap->vap_id);
            continue;
        }

        /* mimo/siso切换时，更新已关联sta唤醒收bcn的通道 */
        if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
            ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) &&
            (hmac_vap->hal_vap != OSAL_NULL)) {
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
            hmac_psm_update_bcn_rf_chain(hmac_vap, oal_get_real_rssi(hmac_vap->query_stats.signal));
            oam_warning_log3(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_m2s_switch_update_vap_tx_chain_capbility:bcn_rx_chain[%d],rssi[%d]}",
                hmac_vap->vap_id,
                hmac_vap->hal_vap->pm_info.bcn_rf_chain, oal_get_real_rssi(hmac_vap->query_stats.signal));
#else
            hal_pm_set_bcn_rf_chain(hmac_vap->hal_vap, hal_device->cfg_cap_info->rf_chain);
#endif
        }

        /* 重新初始化数据帧/管理帧发送参数 */
        hmac_vap_init_tx_frame_txchain_params(hmac_vap, OSAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_update_hal_chain_capbility
 功能描述  : mimo-siso切换更新hal chain device能力
*****************************************************************************/
osal_void hmac_m2s_switch_update_hal_chain_capbility(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 set_channel)
{
    /* 设置对应phy和ana dbb通道映射关系 */
    hal_set_phy_rf_chain_relation(hal_device, set_channel);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_update_device_capbility
 功能描述  : mimo-siso切换更新device能力
*****************************************************************************/
osal_void hmac_m2s_switch_update_device_capbility(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL || hmac_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_update_device_capbility::param null.}");
        return;
    }

    /* 1.更新算法类能力 */
    switch (hal_device->cfg_cap_info->nss_num) {
        case WLAN_SINGLE_NSS:
            hal_device->cfg_cap_info->tx_stbc_is_supp = OSAL_FALSE;
            hal_device->cfg_cap_info->su_bfmer_is_supp = OSAL_FALSE;
            break;

        case WLAN_DOUBLE_NSS:
            hal_device->cfg_cap_info->tx_stbc_is_supp = OSAL_TRUE;
            hal_device->cfg_cap_info->su_bfmer_is_supp = OSAL_TRUE;
            break;

        default:
            oam_warning_log1(0, OAM_SF_M2S,
                "{hmac_m2s_switch_update_device_capbility: nss[%d] error!}", hal_device->cfg_cap_info->nss_num);
            return;
    }

    if (hal_device->hal_m2s_fsm.m2s_type == WLAN_M2S_TYPE_HW) {
        /* 2.硬切换准备，更换mac device的nss能力 */
        hmac_device->device_cap.nss_num = hal_device->cfg_cap_info->nss_num;
    }
    /* 3.同步mac device 的smps能力到hmac */
    hmac_device->mac_smps_mode = mac_m2s_cali_smps_mode(hal_device->cfg_cap_info->nss_num);

    oam_warning_log3(0, OAM_SF_M2S,
        "{hmac_m2s_switch_update_device_capbility::nss_num[%d],mac_dev_nss[%d],mac_dev_smps_mode[%d].}",
        hal_device->cfg_cap_info->nss_num, hmac_device->device_cap.nss_num, hmac_device->mac_smps_mode);
}

#endif /* _PRE_WLAN_FEATURE_M2S */

OSAL_STATIC osal_void hmac_m2s_set_one_packet_params(mac_fcs_cfg_stru *fcs_cfg,  hmac_vap_stru *mac_vap,
    hal_to_dmac_device_stru *hal_device)
{
    fcs_cfg->src_chl = mac_vap->channel;
    fcs_cfg->dst_chl = mac_vap->channel;
    fcs_cfg->vap_id  = mac_vap->vap_id;
    hmac_fcs_prepare_one_packet_cfg(mac_vap, &(fcs_cfg->one_packet_cfg), hal_device->hal_scan_params.scan_time);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_same_channel_vaps_begin
 功能描述  : 两同信道up vap切换
*****************************************************************************/
osal_void hmac_m2s_switch_same_channel_vaps_begin(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2)
{
    mac_fcs_mgr_stru *fcs_mgr = OSAL_NULL;
    mac_fcs_cfg_stru *fcs_cfg = OSAL_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_ONE_PKT_TYPE_UPDATE);
    if (osal_unlikely(hal_device == OSAL_NULL || hmac_device == OSAL_NULL || mac_vap1 == OSAL_NULL ||
        mac_vap2 == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_same_channel_vaps_begin::param null.}");
        return;
    }

    if (mac_vap1->vap_state == MAC_VAP_STATE_UP) {
        /* 暂停vap业务 */
        hmac_vap_pause_tx(mac_vap1);
    }

    if (mac_vap2->vap_state == MAC_VAP_STATE_UP) {
        /* 暂停vap业务 */
        hmac_vap_pause_tx(mac_vap2);
    }

    /* 处于低功耗状态，已经发了null帧，直接返回;并发扫描调用时，还是work状态，sub state置为了init，也需要走这里 */
    if (hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_WORK_STATE) {
        oam_info_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_same_channel_vaps_begin::powersave state[%d].}",
            hal_device->hal_dev_fsm.sub_work_state);
        return;
    }

    fcs_mgr = hmac_fcs_get_mgr_stru(hmac_device);
    fcs_cfg = &hmac_device->fcs_cfg;
    (osal_void)memset_s(fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));

    fcs_cfg->hal_device = hal_device;
    fcs_cfg->src_fake_queue = ((hmac_vap_stru *)mac_vap1)->tx_dscr_queue_fake;

    /* 同频双STA模式，需要起两次one packet */
    if (mac_vap1->vap_mode == WLAN_VAP_MODE_BSS_STA && mac_vap2->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 准备VAP1的fcs参数 */
        hmac_m2s_set_one_packet_params(fcs_cfg, mac_vap1, hal_device);

        /* 刷新vap 发送one pkt帧类型 */
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_one_pkt_type_and_duration_update_cb)fhook)(fcs_mgr, &(fcs_cfg->one_packet_cfg), mac_vap1)
        }

        /* 准备VAP2的fcs参数 */
        fcs_cfg->src_chl2 = mac_vap2->channel;
        hmac_fcs_prepare_one_packet_cfg(mac_vap2, &(fcs_cfg->one_packet_cfg2), hal_device->hal_scan_params.scan_time);

        /* 刷新vap 发送one pkt帧类型 */
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_one_pkt_type_and_duration_update_cb)fhook)(fcs_mgr, &(fcs_cfg->one_packet_cfg2), mac_vap2)
        }
        /* 减小第二次one packet的保护时长，从而减少总时长 */
        fcs_cfg->one_packet_cfg2.timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT2;
        fcs_cfg->hal_device = hal_device;

        hmac_fcs_start_enhanced_same_channel(fcs_cfg);
        /* 同频STA+GO模式，同频STA+AP模式，只需要STA起一次one packet */
    } else {
        hmac_vap_stru *vap_sta = (mac_vap1->vap_mode == WLAN_VAP_MODE_BSS_STA) ? mac_vap1 : mac_vap2;

        hmac_m2s_set_one_packet_params(fcs_cfg, vap_sta, hal_device);

        /* 刷新vap 发送one pkt帧类型 */
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_one_pkt_type_and_duration_update_cb)fhook)(fcs_mgr, &(fcs_cfg->one_packet_cfg), vap_sta)
        }

        /* 调用FCS切信道接口 保存当前硬件队列的帧到扫描虚假队列 */
        hmac_fcs_start_same_channel(fcs_cfg);
    }
}

#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 函 数 名  : hmac_m2s_switch_dbac_vaps_begin
 功能描述  : DBAC模式下切换
*****************************************************************************/
osal_void hmac_m2s_switch_dbac_vaps_begin(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2)
{
    mac_fcs_mgr_stru *fcs_mgr = OSAL_NULL;
    mac_fcs_cfg_stru *fcs_cfg = OSAL_NULL;
    hmac_vap_stru *current_chan_vap = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL || hmac_device == OSAL_NULL || mac_vap1 == OSAL_NULL ||
        mac_vap2 == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_dbac_vaps_begin::param null.}");
        return;
    }

    if (hal_device->current_chan_number == mac_vap1->channel.chan_number) {
        current_chan_vap = mac_vap1;
    } else {
        current_chan_vap = mac_vap2;
    }

    /* 暂停DBAC切信道 */
    if (mac_is_dbac_running(hmac_device)) {
        hmac_alg_dbac_pause(mac_vap1);
    }

    hmac_vap_pause_tx(mac_vap1);
    hmac_vap_pause_tx(mac_vap2);

    /* 处于低功耗状态，已经发了null帧，直接返回;并发扫描调用时，还是work状态，sub state置为了init，也需要走这里 */
    if (hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_WORK_STATE) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_dbac_vaps_begin::powersave state[%d].}",
            hal_device->hal_dev_fsm.sub_work_state);
        return;
    }

    fcs_mgr = hmac_fcs_get_mgr_stru(hmac_device);
    fcs_cfg = &hmac_device->fcs_cfg;
    (osal_void)memset_s(fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));

    fcs_cfg->dst_chl = hal_device->wifi_channel_status;
    fcs_cfg->hal_device = hal_device;

    fcs_cfg->src_fake_queue = ((hmac_vap_stru *)current_chan_vap)->tx_dscr_queue_fake;
    fcs_cfg->vap_id = current_chan_vap->vap_id;

    fcs_cfg->src_chl = hal_device->wifi_channel_status;
    hmac_fcs_prepare_one_packet_cfg(current_chan_vap, &(fcs_cfg->one_packet_cfg),
        hal_device->hal_scan_params.scan_time);

    if (hal_device->current_chan_number != current_chan_vap->channel.chan_number) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_switch_dbac_vap_begin::switch dbac. hal:%d, curr vap:%d. not same,do not send protect frame}",
            hal_device->current_chan_number, current_chan_vap->channel.chan_number);
        fcs_cfg->one_packet_cfg.protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

    hmac_fcs_start_same_channel(fcs_cfg);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_vap_off
 功能描述  : 单VAP发保护帧的方式为切换做准备
*****************************************************************************/
osal_void hmac_m2s_switch_vap_off(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap)
{
    mac_fcs_mgr_stru *fcs_mgr = OSAL_NULL;
    mac_fcs_cfg_stru *fcs_cfg = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL || hmac_device == OSAL_NULL || hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_vap_off::param null.}");
        return;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
        /* 暂停vap业务 */
        hmac_vap_pause_tx(hmac_vap);
    }

    /* 外部保证入参不为空 */
    fcs_mgr = hmac_fcs_get_mgr_stru(hmac_device);
    fcs_cfg = &hmac_device->fcs_cfg;

    (osal_void)memset_s(fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));

    fcs_cfg->src_chl = hmac_vap->channel;
    fcs_cfg->dst_chl = hmac_vap->channel;
    fcs_cfg->hal_device = hal_device;

    fcs_cfg->src_fake_queue = hmac_vap->tx_dscr_queue_fake;
    fcs_cfg->vap_id = hmac_vap->vap_id;

    /* 处于低功耗状态，已经发了null帧，直接返回;并发扫描调用时，还是work状态，sub state置为了init，也需要走这里 */
    if (hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_WORK_STATE) {
        oam_info_log2(0, OAM_SF_M2S, "vap_id[%d] {hmac_m2s_switch_vap_off::powersave state[%d].}", hmac_vap->vap_id,
            hal_device->hal_dev_fsm.sub_work_state);
        return;
    }

    hmac_fcs_prepare_one_packet_cfg(hmac_vap, &(fcs_cfg->one_packet_cfg), hal_device->hal_scan_params.scan_time);

    /* 刷新vap 发送one pkt帧类型 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_ONE_PKT_TYPE_UPDATE);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_one_pkt_type_and_duration_update_cb)fhook)(fcs_mgr, &(fcs_cfg->one_packet_cfg), hmac_vap)
    }

    /* 调用FCS切信道接口,保存当前硬件队列的帧到扫描虚假队列 */
    hmac_fcs_start_same_channel(fcs_cfg);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_do_switch_device_begin
 功能描述  : 开始执行device切换准备，需要执行one packet等流程
*****************************************************************************/
osal_u32 hmac_m2s_switch_device_begin(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device)
{
    osal_u8 vap_idx;
    osal_u8 up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    hmac_vap_stru *hmac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OSAL_NULL};

    if (osal_unlikely(hmac_device == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_device_begin::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Hal Device处于work状态才需要去检查up vap个数 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap[vap_idx] = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap[vap_idx] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_switch_device_begin::hmac_vap[%d] IS NULL.}", mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    if (up_vap_num == 0) {
        /* 没有work的vap，表示现在处于扫描状态，前面已经被扫描abort了，都处于idle状态，可以直接做切换，不需要vap pause操作 */
    } else if (up_vap_num == 1) {
        hmac_m2s_switch_vap_off(hal_device, hmac_device, hmac_vap[0]);
    } else if (up_vap_num == 2) {   /* 存在2个vap时判断是否为dbac */
        if (hmac_vap[0]->channel.chan_number != hmac_vap[1]->channel.chan_number) {
#ifdef _PRE_WLAN_FEATURE_DBAC
            if (mac_is_dbac_running(hmac_device) == OSAL_TRUE) {
                hmac_m2s_switch_dbac_vaps_begin(hal_device, hmac_device, hmac_vap[0], hmac_vap[1]);
            }
#endif
        } else {
            hmac_m2s_switch_same_channel_vaps_begin(hal_device, hmac_device, hmac_vap[0], hmac_vap[1]);
        }
    } else {
        /* m2s不存在其他情况，51静态MIMO，暂时不考虑此接口 */
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_device_begin: cannot support 3 and more vaps!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_do_switch_device_end
 功能描述  : 背景扫描切回home信道
*****************************************************************************/
osal_void hmac_m2s_switch_device_end(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 up_vap_num;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_device_end::param null.}");
        return;
    }

    /* Hal Device处于work状态才需要去检查up vap个数 */
    up_vap_num = hal_device_calc_up_vap_num(hal_device);
    if (up_vap_num == 0) {
        /* 处于扫描状态执行切换，没有执行pause，此处也不需要恢复发送，直接返回即可 */
        return;
    }
#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_running(hmac_device)) {
        /* dbac场景只需恢复dbac，由dbac自行切到工作信道 */
        hmac_alg_dbac_resume(hal_device, OSAL_TRUE);
        return;
    }
#endif
    /* 恢复home信道上被暂停的发送 */
    hmac_vap_resume_tx_by_chl(hmac_device, hal_device, &(hal_device->wifi_channel_status));
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_rssi_inaccurate_fix
 功能描述  : 修订常收测试中出现rssi不准问题
*****************************************************************************/
osal_void hmac_m2s_switch_rssi_inaccurate_fix(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 int_save;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch_rssi_inaccurate_fix::param null.}");
        return;
    }

    if ((hal_device->al_rx_flag == HAL_ALWAYS_RX_AMPDU_ENABLE) || (hal_device->al_rx_flag == HAL_ALWAYS_RX_ENABLE) ||
        (hal_device->al_rx_flag == HAL_ALWAYS_RX_RESERVED)) {
        /* 挂起硬件发送 */
        hal_set_machw_tx_suspend();

        /* 关闭pa */
        hal_save_machw_phy_pa_status(hal_device);
        hal_disable_machw_phy_and_pa();

        /* 关中断，在挂起硬件发送之后 */
        int_save = frw_osal_irq_lock();

        /* 处理完接收事件队列 */
        frw_event_flush_event_queue(WLAN_MSG_D2D_CRX_RX);
        frw_event_flush_event_queue(WLAN_MSG_D2D_RX);

        /* 清fifo和删除tx队列中所有帧 */
        hmac_clear_hw_fifo_tx_queue(hal_device);

        hal_device->rx_normal_mdpu_succ_num = 0;
        hal_device->always_rx_rssi = OAL_RSSI_INIT_MARKER;
        hal_device->rx_last_rssi = OAL_RSSI_INIT_VALUE;

        /* 开中断 */
        frw_osal_irq_restore(int_save);

        /* 使能pa */
        hal_recover_machw_phy_and_pa(hal_device);

        /* 恢复硬件发送 */
        hal_set_machw_tx_resume();
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch
 功能描述  : 执行mimo-siso切换, hal device级别切换，支持主辅路
*****************************************************************************/
osal_u32 hmac_m2s_switch(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event,
    oal_bool_enum_uint8 hw_switch)
{
    osal_u32 ret;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_void *fhook = OAL_PTR_NULL;
    unref_param(m2s_event);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch::param null.}");
        return OAL_FAIL;
    }

    /* wifi可能处于扫描状态，暂时直接abort，如果后续扫描优先级高不允许切换，再考虑直接返回
       pilot修改之后，不需要暂停扫描hmac_scan_abort(ddmac_device)，扫描时候不做保护切离即可 */
    /* 1.保护切离 */
    if (hal_device->hal_m2s_fsm.m2s_type == WLAN_M2S_TYPE_SW &&
        HAL_DEVICE_WORK_STATE == hal_device->hal_dev_fsm.oal_fsm.cur_state) {
        /* 刷新fcs业务类型 */
        hmac_device->fcs_mgr.fcs_service_type = HAL_FCS_SERVICE_TYPE_M2S;

        /* 做切换准备，硬件队列包缓存到虚假队列中 */
        ret = hmac_m2s_switch_device_begin(hmac_device, hal_device);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_switch::hmac_m2s_switch_device_begin return fail.}");
            return OAL_FAIL;
        }
    }

    /* 设置标志，正在做切换的准备工作 */
    hal_device->m2s_excute_flag = OSAL_TRUE;

    /* 2.更新device 能力 */
    hmac_m2s_switch_update_device_capbility(hal_device, hmac_device);

    /* 需要硬件切换才做 */
    if (hw_switch == OSAL_TRUE) {
        /* 2.1更新hal device chain能力 */
        hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_TRUE);
    }

    /* 3.更新原来hal device下vap和user能力,vap能力基于hal device能力来, user能力基于vap能力来 */
    hmac_m2s_switch_update_vap_capbility(hal_device);

    hal_device->m2s_excute_flag = OSAL_FALSE;

    if (hal_device->hal_m2s_fsm.m2s_type == WLAN_M2S_TYPE_SW &&
        HAL_DEVICE_WORK_STATE == hal_device->hal_dev_fsm.oal_fsm.cur_state) {
        /* 如果已经处于btcoex ps状态下, 不需要再执行restore动作 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_SW_PREEMPT_TYPE_CHECK);
        if (fhook != OSAL_NULL && ((hmac_btcoex_sw_preempt_type_check_cb)fhook)(hal_device) == OSAL_TRUE) {
        } else {
            hmac_m2s_switch_device_end(hal_device);
        }
    }
    hmac_m2s_switch_rssi_inaccurate_fix(hal_device);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_idle_to_xixo
 功能描述  : IDLE切换至XIXO, 重新申请rf即可
*****************************************************************************/
osal_void hmac_m2s_idle_to_xixo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    /* 软件提前切换了，这里做好硬件恢复工作即可,因为不涉及配置信道，scan或者work关联会配置信道 */
    /* 因为是idle状态，不存在数据发送，可以直接操作mac和phy寄存器 */
    unref_param(m2s_event);
    hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_FALSE);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_idle_to_idle
 功能描述  : IDLE切换至MIMO, 重新申请rf即可
*****************************************************************************/
osal_void hmac_m2s_idle_to_idle(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event,
    hal_m2s_state_uint8 m2s_state)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_idle_to_idle::param null.}");
        return;
    }

    /* 刷新配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 未申请rf，需要提前置标志 */
    rf_res_set_is_mimo(hal_device->cfg_cap_info->rf_chain);

    /* 当前只有test模式进来，其他状态直接报错 */
    if (m2s_state != HAL_M2S_STATE_MIMO && m2s_state != HAL_M2S_STATE_SISO) {
        oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_idle_to_idle:: FAIL m2s_state[%d].}",
            m2s_state);
        return;
    }

    /* 软件切换, 硬件保持不申请 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_idle_to_idle:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mimo_to_miso
 功能描述  : MIMO切换至MISO, 协议模式切换为SISO, 双通道
*****************************************************************************/
osal_void hmac_m2s_mimo_to_miso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mimo_to_miso::param null.}");
        return;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(m2s_event);
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_m2s_mimo_to_miso:: trigger_mode[%d] error!}",
            trigger_mode);
        return;
    }

    /* command方式切换到miso，才触发延迟切换保护，test方式不需要
       需要device开启切离保护, 继续切换到siso就恢复，后续是否执行切换保护，增加device级开关 */
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
        get_hal_device_m2s_switch_prot(hal_device) = OSAL_TRUE;
    }

    /* 刷新配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 软件切换到siso模式，硬件保持在mimo */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_mimo_to_miso:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_miso_to_mimo
 功能描述  : MISO切换至MIMO入口, 协议MIMO, 开启双通道
*****************************************************************************/
osal_void hmac_m2s_miso_to_mimo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_miso_to_mimo::param null.}");
        return;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(m2s_event);
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{hmac_m2s_miso_to_mimo:: device[%d] trigger_mode[%d] error!}", trigger_mode);
        return;
    }

    /* 只要是切换到mimo，延迟切换保护关闭 */
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
        get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;
    }

    /* 刷新mimo配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* miso切换回mimo，硬件能力还是mimo的，不要动，直接切换软件能力 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_miso_to_mimo:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_miso_to_siso
 功能描述  : MISO切换至SISO入口，协议本身是siso的，此时直接切换硬件能力即可
*****************************************************************************/
osal_void hmac_m2s_miso_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_miso_to_siso::param null.}");
        return;
    }

    /* 刷新miso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 如果软件能力从c0变化到c1，也需要刷新非单播数据帧的发送通道 */
    /* 软件能力仍然是siso，硬件能力从mimo切换到siso */
    /* 配置mac和phy寄存器时，需要调用pause和resume逻辑，不能只调用hmac_m2s_switch_update_hal_chain_capbility()
       防止此时硬件还在有数据在发送，造成mac异常上报中断，造成软件异常 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_miso_to_siso:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return;
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_miso_to_siso:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_siso_to_miso
 功能描述  : SISO切换至MISO, 协议模式SISO, 切换为双通道
*****************************************************************************/
osal_void hmac_m2s_siso_to_miso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_siso_to_miso::param null.}");
        return;
    }

    /* 刷新miso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 注意:如果软件能力从c0变化到c1，也需要刷新非单播数据帧的发送通道 */
    /* 软件仍然是siso状态，硬件切回mimo */
    /* 配置mac和phy寄存器时，需要调用pause和resume逻辑，不能只调用hmac_m2s_switch_update_hal_chain_capbility()
       防止此时硬件还在有数据在发送，造成mac异常上报中断，造成软件异常 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_siso_to_miso:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return;
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_siso_to_miso:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mimo_to_siso
 功能描述  : MIMO切换至SISO, 协议模式切换到SISO, 通道能力切换为双通道
*****************************************************************************/
osal_void hmac_m2s_mimo_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mimo_to_siso::param null.}");
        return;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 软件和硬件能力都刷到siso */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_mimo_to_siso:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_siso_to_mimo
 功能描述  : SISO切换至MIMO, 协议模式MIMO, 切换为双通道
*****************************************************************************/
osal_void hmac_m2s_siso_to_mimo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_siso_to_mimo::param null.}");
        return;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(m2s_event);
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{hmac_m2s_siso_to_mimo:: device[%d] trigger_mode[%d] error!}", trigger_mode);
        return;
    }

    /* 只要是切换到mimo，延迟切换保护关闭 */
    if (trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
        get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 软件和硬件能力都刷回mimo */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_siso_to_mimo:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_siso_to_mimo:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_miso_to_miso
 功能描述  : MISO切换至MISO, 协议模式SISO, 软件发送描述符通道变化，硬件发送通道不变
*****************************************************************************/
osal_void hmac_m2s_miso_to_miso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_miso_to_miso::param null.}");
        return;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 1.硬件能力不变，软件能力从c0 siso和c1 siso切换 */
    /* 更新原来hal device下vap和user能力即可,vap能力基于hal device能力来, user能力基于vap能力来 */
    hmac_m2s_switch_update_vap_capbility(hal_device);

    /* 2.修订寄存器控响应帧发送通道，尽量保持和管理帧广播数据帧等保持一致 */
    hal_update_datarate_by_chain(hal_device, hal_device->cfg_cap_info->phy_chain);

    oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_miso_to_miso:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_siso_to_siso
 功能描述  : SISO切换至SISO, 协议模式SISO, 从一个通道切换到另一个通道
*****************************************************************************/
osal_void hmac_m2s_siso_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_siso_to_siso::param null.}");
        return;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* 软件和硬件能力都从siso刷新到另一侧siso */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S, "{hmac_m2s_siso_to_siso:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_NOTIFY_INOUT_SISO);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_notify_inout_siso_cb)fhook)(hal_device);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mimo_to_simo
 功能描述  : mimo切换至simo, 协议模式mimo, 硬件能力是siso
*****************************************************************************/
osal_void hmac_m2s_mimo_to_simo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mimo_to_simo::param null.}");
        return;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* rssi逻辑可能已经刷新了非单播数据帧的通道，此时需要根据当前能力重新刷回来，rssi逻辑重新自主更新，防止冲突 */
    /* 软件仍然是mimo状态，硬件切到simo */
    /* 配置mac和phy寄存器时，需要调用pause和resume逻辑，不能只调用
     * 函数hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_FALSE)
     * 防止此时硬件还在有数据在发送，造成mac异常上报中断，造成软件异常 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_mimo_to_simo:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return;
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_mimo_to_simo:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_simo_to_mimo
 功能描述  : simo切换至mimo, 软件能力保持mimo，硬件能力切换回mimo
*****************************************************************************/
osal_void hmac_m2s_simo_to_mimo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    osal_u32 ret;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_simo_to_mimo::param null.}");
        return;
    }

    /* 刷新siso配置 */
    hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

    /* rssi逻辑可能已经刷新了非单播数据帧的通道，此时需要根据当前能力重新刷回来，rssi逻辑重新自主更新，防止冲突 */
    /* 软件仍然是mimo状态，硬件切回mimo */
    /* 配置mac和phy寄存器时，需要调用pause和resume逻辑，不能只调用
     * 函数hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_TRUE)
     * 防止此时硬件还在有数据在发送，造成mac异常上报中断，造成软件异常 */
    ret = hmac_m2s_switch(hal_device, m2s_event, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_m2s_simo_to_mimo:: FAIL m2s_event[%d], cur state[%d].}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);
        return;
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_simo_to_mimo:: FINISH m2s_event[%d], cur state[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_fsm_trans_to_state
 功能描述  : m2s状态机状态切换函数
*****************************************************************************/
osal_u32 hmac_m2s_fsm_trans_to_state(hal_m2s_fsm_stru *m2s_fsm, osal_u8 state)
{
    hal_to_dmac_device_stru *hal_device;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_trans_to_state::param null.}");
        return OAL_FAIL;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_trans_to_state::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    oal_fsm_trans_to_state(&(m2s_fsm->oal_fsm), state);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_NOTIFY_SET_WIFI_STATUS);
    if (fhook != OSAL_NULL &&
        hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE)) {
        /* 进入c1 siso时，需要通知蓝牙c1 siso状态，可以全速发送 */
        ((hmac_btcoex_set_wifi_status_irq_notify_cb)fhook)(HAL_BTCOEX_WIFI_STATE_C1_SISO, (osal_u8)OSAL_TRUE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_device_state_idle_entry
 功能描述  : idle状态的entry接口
*****************************************************************************/
osal_void hmac_m2s_state_idle_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_m2s_event_tpye_uint16 m2s_event;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_idle_entry::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_idle_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    m2s_event = m2s_fsm->oal_fsm.last_event;

    switch (m2s_event) {
        /* 进入idle状态，需要释放rf资源即可 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            break;

        /* finish init */
        case HAL_M2S_EVENT_FSM_INIT:
            oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_state_idle_entry: m2s fsm finish init!}");
            return;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_state_idle_entry: m2s_event[%d] error!}",
                m2s_event);
            return;
    }

    oam_warning_log2(0, OAM_SF_M2S,
        "{hmac_m2s_state_idle_entry::last event[%d],change to idle,cur state[%d]}", m2s_fsm->oal_fsm.last_event,
        m2s_fsm->oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hmac_device_state_idle_exit
 功能描述  : idle状态的exit接口
*****************************************************************************/
osal_void hmac_m2s_state_idle_exit(osal_void *ctx)
{
    if (ctx == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_m2s_state_idle_exit::ctx == NULL but no use!}");
    }
}

/*****************************************************************************
 函 数 名  : hmac_device_state_idle_event
 功能描述  : idle状态的event接口
*****************************************************************************/
osal_u32 hmac_m2s_state_idle_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_m2s_state_uint8 m2s_state;
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_idle_event::param null.}");
        return OAL_FAIL;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_idle_event::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(event);
    unref_param(event_data_len);
    unref_param(event_data);

    /* 1. 刷新对应业务标记 */
    if (trigger_mode != WLAN_M2S_TRIGGER_MODE_BUTT) {
        get_hal_m2s_mode_tpye(hal_device) |= trigger_mode;
    }

    /* 如果不是并发扫描，置了并发扫描的话，需要清标记 */
    get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);

    m2s_state = hmac_m2s_chain_state_classify(hal_device);

    /* 除了hal idle end，其他业务触发事件都是修改对应的chain能力 */
    switch (event) {
        case HAL_M2S_EVENT_WORK_BEGIN:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, m2s_state);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
            /* 如果初始是c1 siso的话，并发扫描会在前面拦截不执行 */
            hmac_m2s_fsm_trans_to_state(m2s_fsm, m2s_state);
            break;

        default:
            m2s_state = hmac_m2s_event_state_classify(event);
            /* 是恢复到mimo，那就是业务结束，需要清标记 */
            if (m2s_state == HAL_M2S_STATE_MIMO) {
                get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);
            }

            /* 其他事件来了需要刷新软硬件能力，不申请释放rf，还是处于idle状态，等状态跳变时候再做切换 rf能力申请 */
            /* 当前约束测试行为，只有test模式才会在idle触发，mss对应也要求如此，其他切换都是非idle状态 */
            hmac_m2s_idle_to_idle(hal_device, event, m2s_state);

            oam_warning_log2(0, OAM_SF_M2S,
                "{hmac_m2s_state_idle_event::event[%d]m2s_state[%d] no handle!!!}", event, m2s_state);
            break;
    }

    return OAL_SUCC;
}

osal_u32 hmac_m2s_event_process(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_event_process::param null.}");
        return OAL_FAIL;
    }

    switch (m2s_event) {
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_TO_MIMO:
        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
            hmac_m2s_siso_to_mimo(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_SPEC_SISO_TO_MIMO:
            /* 前面vap软件能力已经正常刷新，mimo时single_tx_chain能力不同关系不大，只刷新硬件能力 */
            /* 刷新siso配置 */
            hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

            /* 回到mimo时，根据2G rssi mgmt学习到的刷新single_tx_chain值 */
            if (get_hal_device_m2s_rssi_mgmt_single_txchain(hal_device) != 0) {
                oam_warning_log1(0, OAM_SF_M2S,
                    "{hmac_m2s_event_process: rssi mgmt need to reflush tx_single_chain to [%d]!}",
                    get_hal_device_m2s_rssi_mgmt_single_txchain(hal_device));

                hal_device->cfg_cap_info->single_tx_chain = get_hal_device_m2s_rssi_mgmt_single_txchain(hal_device);
            }

            /* 刷新vap的tx chain能力 */
            hmac_m2s_switch_update_vap_tx_chain_capbility(hal_device);

            /* 刷新硬件的tx chain mac+phy能力 */
            hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_TRUE);
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MISO_TO_MIMO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
            hmac_m2s_miso_to_mimo(hal_device, m2s_event);
            break;

        /* scan结束需要恢复硬件能力 */
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
            /* m2s状态机清除并发扫描 */
            if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SIMO) {
                hmac_m2s_simo_to_mimo(hal_device, m2s_event);
            } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO) {
                hmac_m2s_siso_to_mimo(hal_device, m2s_event);
            }
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            hmac_m2s_idle_to_xixo(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_FSM_INIT:
            oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_event_process: m2s fsm finish init!}");
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_event_process: m2s_event[%d] error!}",
                m2s_event);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_mimo_entry
 功能描述  : M2S状态机MIMO ENTRY
*****************************************************************************/
osal_void hmac_m2s_state_mimo_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_m2s_event_tpye_uint16 m2s_event;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_entry::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    m2s_event = m2s_fsm->oal_fsm.last_event;
    if (hmac_m2s_event_process(hal_device, m2s_event) != OAL_SUCC) {
        return;
    }

    /* 进入mimo需要mode都清掉，增加维测 */
    oam_warning_log3(0, OAM_SF_M2S,
        "{hmac_m2s_state_mimo_entry::event[%d],change to mimo,last state[%d] m2s_mode[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));

    if (get_hal_m2s_mode_tpye(hal_device) != 0) {
        get_hal_m2s_mode_tpye(hal_device) &= 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_mimo_exit
 功能描述  : M2S状态机MIMO EXIT
*****************************************************************************/
osal_void hmac_m2s_state_mimo_exit(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_exit::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_entry::hal_to_dmac_device_stru null.}");
        return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_fsm_trans_state
 功能描述  : M2S状态机转换状态
*****************************************************************************/
osal_void hmac_m2s_fsm_trans_state(hal_m2s_fsm_stru *m2s_fsm, osal_u16 event)
{
    switch (event) {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_IDLE);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MISO);
            break;

        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_SISO);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
        case HAL_M2S_EVENT_SCAN_END: /* mimo状态也会收到此事件，非并发扫描时 */
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
            /* 普通扫描在mimo状态下结束，前景扫描的话不处理即可，背景扫描会在set home channel时候做正确切换spec siso */
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "hmac_m2s_state_mimo_event: event[%d] INVALID!", event);
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_mimo_event
 功能描述  : M2S状态机MIMO 事件入口
*****************************************************************************/
osal_u32 hmac_m2s_state_mimo_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_m2s_fsm_stru *m2s_fsm = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_event::param null.}");
        return OAL_FAIL;
    }

    m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_mimo_event::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(event);
    unref_param(event_data_len);
    unref_param(event_data);

    /* 1. 刷新对应业务标记 */
    if (trigger_mode != WLAN_M2S_TRIGGER_MODE_BUTT) {
        get_hal_m2s_mode_tpye(hal_device) |= trigger_mode;
    }

    /* 如果不是并发扫描，置了并发扫描的话，需要清标记 */
    get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);

    hmac_m2s_fsm_trans_state(m2s_fsm, event);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_siso_entry
 功能描述  : M2S状态机SISO ENTRY
*****************************************************************************/
osal_void hmac_m2s_state_siso_entry(osal_void *ctx)
{
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_m2s_event_tpye_uint16 m2s_event;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_entry::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    m2s_event = m2s_fsm->oal_fsm.last_event;

    switch (m2s_event) {
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0:
            hmac_m2s_mimo_to_siso(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1:
            /* 前面vap软件能力已经正常刷新，mimo时single_tx_chain能力不同关系不大，只刷新硬件能力 */
            /* 刷新siso配置 */
            hmac_m2s_update_switch_mgr_param(hal_device, m2s_event);

            /* 刷新vap的tx chain能力 */
            hmac_m2s_switch_update_vap_tx_chain_capbility(hal_device);

            /* 刷新硬件的tx chain mac+phy能力 */
            hmac_m2s_switch_update_hal_chain_capbility(hal_device, OSAL_TRUE);
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MISO_TO_SISO_C1:
            hmac_m2s_miso_to_siso(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            hmac_m2s_idle_to_xixo(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_SCAN_END:
            /* 如果此时mss使能，并且处于siso稳定态，需要切换到miso状态，等扫描结束时恢复到siso状态 */
            hmac_m2s_miso_to_siso(hal_device, m2s_event);
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_state_siso_entry: m2s_event[%d] error!}", m2s_event);
            return;
    }

    oam_warning_log3(0, OAM_SF_M2S, "{hmac_m2s_state_siso_entry::event[%d],change to siso,last state[%d] m2s_mode[%d]}",
        m2s_event, hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_siso_exit
 功能描述  : M2S状态机SISO EXIT
*****************************************************************************/
osal_void hmac_m2s_state_siso_exit(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_exit::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_exit::hal_to_dmac_device_stru null.}");
        return;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_NOTIFY_SET_WIFI_STATUS);
    if (fhook != OSAL_NULL &&
        hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE)) {
        /* 退出c1 siso时，需要通知蓝牙c1 siso状态，不可以全速发送  */
        ((hmac_btcoex_set_wifi_status_irq_notify_cb)fhook)(HAL_BTCOEX_WIFI_STATE_C1_SISO, (osal_u8)OSAL_FALSE);
    }
}

osal_void hmac_m2s_state_siso_event_proc(osal_u16 event, hal_m2s_fsm_stru *m2s_fsm,
    hal_to_dmac_device_stru *hal_device, wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    /* 2.计划回到mixo, 对应业务结束置回标记 */
    switch (event) {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_IDLE);
            break;

        /* 接收功率周期性触发探测 */
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1:
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MISO);
            break;

        /* NONE方式下仍打开了切换保护，此时不处理即可 */
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
            oam_warning_log0(0, OAM_SF_M2S,
                "{hmac_m2s_state_siso_event_proc::delay switch already succ to siso.}");
            break;

        case HAL_M2S_EVENT_TEST_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_SPEC_SISO_TO_MIMO:
            /* 业务结束申请状态切换管理 */
            hmac_m2s_switch_back_to_mixo_check(m2s_fsm, hal_device, trigger_mode);
            break;

        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0:
        /* c0到c1之间的切换，状态机不变，只是抛事件做通道重新配置 */
        /* btcoex切换到c1 siso，扫描到5g时需要主路切换到c0 siso */
        case HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0:
            hmac_m2s_siso_to_siso(hal_device, event);
            break;

        /* siso状态收到扫描事件需要处理切换到miso扫描 scan结束时需要恢复siso */
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MISO_SCAN_BEGIN:
            oam_warning_log0(0, OAM_SF_M2S,
                "{hmac_m2s_state_siso_event_proc:: mss is on, change to miso!}");
            hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MISO);
            break;

        /* siso状态收到扫描事件不需要处理, scan恢复到work的话，也不需要处理 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            break;

        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
            /* 如果spec使能，主要是普通扫描下,需要回到mimo */
            /* 普通扫描在siso状态下结束，当前处于此状态的话，一定是并发扫描或者spec，test不考虑;
               单独的spec下时，应该是5g前景扫描才需要考虑回mimo，背景扫描由switch back home做处理即可 */
            /* 当前先配置信道，再状态机跳，先配置信道就决定了有没有spec，此时可能存在spec和fast_scan并发 */
            if (hal_m2s_check_fast_scan_on(hal_device) == OSAL_TRUE || hal_m2s_check_spec_on(hal_device) == OSAL_TRUE) {
                /* 业务结束申请状态切换管理 */
                hmac_m2s_switch_back_to_mixo_check(m2s_fsm, hal_device, trigger_mode);
            }
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_state_siso_event_proc:: event[%d] INVALID!}",
                event);
            break;
    }
}


/*****************************************************************************
 函 数 名  : hmac_m2s_state_siso_event
 功能描述  : M2S状态机SISO 事件入口
*****************************************************************************/
osal_u32 hmac_m2s_state_siso_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_m2s_fsm_stru *m2s_fsm = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_event::param null.}");
        return OAL_FAIL;
    }

    m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_siso_event::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    unref_param(event_data_len);
    unref_param(event_data);

    trigger_mode = hmac_m2s_event_trigger_mode_classify(event);
    /* 1. 刷新对应业务标记 */
    if (trigger_mode != WLAN_M2S_TRIGGER_MODE_BUTT) {
        get_hal_m2s_mode_tpye(hal_device) |= trigger_mode;
    }

    /* 如果不是并发扫描，置了并发扫描的话，需要清标记 */
    get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);

    hmac_m2s_state_siso_event_proc(event, m2s_fsm, hal_device, trigger_mode);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_miso_entry
 功能描述  : M2S状态机MISO ENTRY
*****************************************************************************/
osal_void hmac_m2s_state_miso_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_m2s_event_tpye_uint16 m2s_event;

    if (osal_unlikely(m2s_fsm == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_miso_entry::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_miso_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    m2s_event = m2s_fsm->oal_fsm.last_event;

    switch (m2s_event) {
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1:
            hmac_m2s_siso_to_miso(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1:
            hmac_m2s_mimo_to_miso(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_COMMAND_SISO_TO_MISO_SCAN_BEGIN:
            hmac_m2s_siso_to_miso(hal_device, m2s_event);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            hmac_m2s_idle_to_xixo(hal_device, m2s_event);
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_state_miso_entry: m2s_event[%d] error!}",
                m2s_event);
            return;
    }

    oam_warning_log3(0, OAM_SF_M2S,
        "{hmac_m2s_state_miso_entry::event[%d],change to miso,last state[%d] m2s_mode[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_miso_exit
 功能描述  : M2S状态机MISO EXIT
*****************************************************************************/
osal_void hmac_m2s_state_miso_exit(osal_void *ctx)
{
    /* 打开双通道, 协议依然为SISO */
    unref_param(ctx);
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_miso_event_process
 功能描述  : M2S状态机MISO 事件依据event类型处理
*****************************************************************************/
osal_u32 hmac_m2s_state_miso_event_process(osal_u16 event, hal_m2s_fsm_stru *m2s_fsm,
    hal_to_dmac_device_stru *hal_device, wlan_m2s_trigger_mode_enum_uint8 trigger_mode)
{
    osal_u32 ret = OAL_SUCC;

    if (osal_unlikely(m2s_fsm == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_miso_event_process::param null.}");
        return OAL_FAIL;
    }

    switch (event) {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            ret = hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_IDLE);
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_BT_MISO_TO_SISO_C1:
            ret = hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_SISO);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MISO_TO_MIMO:
            /* 允许切回misx，就切回，否则保持在siso状态 */
            hmac_m2s_switch_back_to_mixo_check(m2s_fsm, hal_device, trigger_mode);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1:
            /* c0到c1之间的切换，状态机不变，只是抛事件做通道重新配置 */
            hmac_m2s_miso_to_miso(hal_device, event);
            break;

        /* mimo状态也会收到此事件，非并发扫描时 */
        case HAL_M2S_EVENT_WORK_BEGIN:
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_SCAN_END:
            oam_warning_log0(0, OAM_SF_M2S,
                "{hmac_m2s_state_miso_event:mss is on,need resume siso}");
            ret = hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_SISO);
            break;

        default:
            oam_error_log2(0, OAM_SF_M2S, "{hmac_m2s_state_miso_event:cur state[%d] wrong event[%d].}",
                hal_device->hal_m2s_fsm.oal_fsm.cur_state, event);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_miso_event
 功能描述  : M2S状态机MISO 事件入口
*****************************************************************************/
osal_u32 hmac_m2s_state_miso_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_m2s_fsm_stru *m2s_fsm = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u32 ret;
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_miso_event::param null.}");
        return OAL_FAIL;
    }

    m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_miso_event::hal_to_dmac_device_stru null.}");
        return OAL_FAIL;
    }

    trigger_mode = hmac_m2s_event_trigger_mode_classify(event);
    unref_param(event_data_len);
    unref_param(event_data);

    /* 1. 刷新对应业务标记 */
    if (trigger_mode != WLAN_M2S_TRIGGER_MODE_BUTT) {
        get_hal_m2s_mode_tpye(hal_device) |= trigger_mode;
    }

    /* 如果不是并发扫描，置了并发扫描的话，需要清标记 */
    get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);

    /* 根据event类型做处理 */
    ret = hmac_m2s_state_miso_event_process(event, m2s_fsm, hal_device, trigger_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_M2S,
            "{hmac_m2s_state_miso_event::err in hmac_m2s_state_miso_event_process}");
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_m2s_state_simo_entry
 功能描述  : M2S状态机SIMO ENTRY
*****************************************************************************/
osal_void hmac_m2s_state_simo_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_m2s_event_tpye_uint16 m2s_event;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_simo_entry::param null.}");
        return;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_simo_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    m2s_event = m2s_fsm->oal_fsm.last_event;

    switch (m2s_event) {
        /* 并发扫描和普通前景背景扫描需要区分开类型，前景扫描，本质也是在mimo下，复用即可 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            break;

        case HAL_M2S_EVENT_WORK_BEGIN:
            hmac_m2s_idle_to_xixo(hal_device, m2s_event);
            break;

        default:
            oam_error_log1(0, OAM_SF_M2S, "{hmac_m2s_state_simo_entry: m2s_event[%d] error!}",
                m2s_event);
            return;
    }

    oam_warning_log3(0, OAM_SF_M2S,
        "{hmac_m2s_state_simo_entry::event[%d],change to simo,last state[%d] m2s_mode[%d].}", m2s_event,
        hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_simo_exit
 功能描述  : M2S状态机simo EXIT
*****************************************************************************/
osal_void hmac_m2s_state_simo_exit(osal_void *ctx)
{
    if (ctx == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_m2s_state_simo_exit::ctx == NULL but no use!}");
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_state_simo_event
 功能描述  : M2S状态机SIMO 事件入口
*****************************************************************************/
osal_u32 hmac_m2s_state_simo_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_to_dmac_device_stru *another_hal_device = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_m2s_fsm_stru *m2s_fsm = (hal_m2s_fsm_stru *)ctx;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode;

    if (osal_unlikely(ctx == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_state_simo_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = (hal_to_dmac_device_stru *)(m2s_fsm->oal_fsm.p_oshandler);
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_state_simo_event:hal_device is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }
    unref_param(event_data_len);
    unref_param(event_data);

    trigger_mode = hmac_m2s_event_trigger_mode_classify(event);
    /* 1. 刷新对应业务标记 */
    if (trigger_mode != WLAN_M2S_TRIGGER_MODE_BUTT) {
        get_hal_m2s_mode_tpye(hal_device) |= trigger_mode;
    }

    /* 如果不是并发扫描，置了并发扫描的话，需要清标记 */
    get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);

    if (event == HAL_M2S_EVENT_IDLE_BEGIN) {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_IDLE);
    } else if (event == HAL_M2S_EVENT_SCAN_CHANNEL_BACK) {
        /* 抛事件给另一个hal device状态机,暂停此hal device的扫描,让出rf资源供主路回home channel工作 */
        another_hal_device = hmac_device_get_another_h2d_dev(hmac_device, hal_device);
        if (another_hal_device == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_state_simo_event:another_hal_device is NULL");
            return OAL_ERR_CODE_PTR_NULL;
        }
        hal_device_handle_event(another_hal_device, HAL_DEVICE_EVENT_SCAN_PAUSE, 0, OSAL_NULL);

        /* 允许切回misx，就切回，否则保持在siso状态 */
        hmac_m2s_switch_back_to_mixo_check(m2s_fsm, hal_device, trigger_mode);
    } else if (event == HAL_M2S_EVENT_SCAN_END) {
        /* 允许切回misx，就切回，否则保持在siso状态 */
        hmac_m2s_switch_back_to_mixo_check(m2s_fsm, hal_device, trigger_mode);
    } else if (event != HAL_M2S_EVENT_WORK_BEGIN) {
        oam_error_log2(0, OAM_SF_M2S, "{hmac_m2s_state_simo_event:cur state[%d] wrong event[%d].}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, event);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_handle_event
 功能描述  : m2s状态机的事件处理接口
*****************************************************************************/
osal_u32 hmac_m2s_handle_event(hal_to_dmac_device_stru *hal_device, osal_u16 type, osal_u16 datalen, osal_u8 *data)
{
    osal_u32 ret;
    hal_m2s_fsm_stru *m2s_fsm;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_handle_event::param null.}");
        return OAL_FAIL;
    }

    m2s_fsm = &(hal_device->hal_m2s_fsm);
    if (m2s_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_handle_event::fsm not attached!}");
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(m2s_fsm->oal_fsm), type, datalen, data);
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_M2S, "{hmac_m2s_handle_event::state[%d]dispatch event[%d]not succ[%d]!}",
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, type, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_fsm_attach
 功能描述  : m2s创建状态机
*****************************************************************************/
osal_void hmac_m2s_fsm_attach(hal_to_dmac_device_stru *hal_device)
{
    osal_u8                fsm_name[6] = {0};
    osal_u32 ret;
    hal_m2s_fsm_stru *m2s_fsm = OSAL_NULL;
    hal_m2s_state_uint8 state = HAL_M2S_STATE_MIMO; /* 初始工作状态 */
    oal_fsm_create_stru fsm_create_stru;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_attach::param null.}");
        return;
    }

    if (hal_device->hal_m2s_fsm.is_fsm_attached == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_attach::fsm have attached.}");
        return;
    }

    m2s_fsm = &(hal_device->hal_m2s_fsm);

    /* 因为hal device初始是idle状态状态，m2s也需要初始为此状态 */
    (osal_void)memset_s(m2s_fsm, sizeof(hal_m2s_fsm_stru), 0, sizeof(hal_m2s_fsm_stru));
    fsm_create_stru.oshandle = (osal_void *)hal_device;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = m2s_fsm;

    ret = oal_fsm_create(&fsm_create_stru, &(m2s_fsm->oal_fsm), state, g_hal_m2s_fsm_info,
        sizeof(g_hal_m2s_fsm_info) / sizeof(oal_fsm_state_info));
    if (ret == OAL_SUCC) {
        /* oal fsm create succ */
        m2s_fsm->is_fsm_attached = OSAL_TRUE;
        m2s_fsm->m2s_type = WLAN_M2S_TYPE_SW;
        /* 暂时还没有业务申请切换 */
        (osal_void)memset_s(&m2s_fsm->m2s_mode, sizeof(wlan_m2s_mode_stru), 0, sizeof(wlan_m2s_mode_stru));
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2s_fsm_detach
 功能描述  : m2s注销状态机
*****************************************************************************/
osal_void hmac_m2s_fsm_detach(hal_to_dmac_device_stru *hal_device)
{
    hal_m2s_fsm_stru *m2s_fsm = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_detach::param null.}");
        return;
    }

    m2s_fsm = &(hal_device->hal_m2s_fsm);

    if (m2s_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_fsm_detach::fsm not attatched}");
        return;
    }

    /* 不是IDLE状态切换到IDLE状态 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state != HAL_M2S_STATE_MIMO) {
        hmac_m2s_fsm_trans_to_state(m2s_fsm, HAL_M2S_STATE_MIMO);
    }

    /* 清除m2s的业务状态 */
    get_hal_m2s_mode_tpye(hal_device) &= 0;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mgmt_switch
 功能描述  : 把管理帧发送天线切到单天线
*****************************************************************************/
osal_void hmac_m2s_mgmt_switch(hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap,
    osal_u8 single_tx_chain)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mgmt_switch::param null.}");
        return;
    }

    hal_device->cfg_cap_info->single_tx_chain = single_tx_chain;

    unref_param(hmac_vap);
    /* spec下需要记录该chain能力，用于band切换回2G mimo时候使用 */
    get_hal_device_m2s_rssi_mgmt_single_txchain(hal_device) = single_tx_chain;

    /* 1.修订寄存器控响应帧发送通道 */
    hal_update_datarate_by_chain(hal_device, hal_device->cfg_cap_info->phy_chain);

    /* 2.修订描述符控管理帧发送通道, 修订当前hal device上所有vap，同时也需要广播组播数据帧 */
    hmac_m2s_switch_update_vap_capbility(hal_device);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_mgmt_switch_judge
 功能描述  : 把管理帧发送天线切到单天线判断
*****************************************************************************/
osal_void hmac_m2s_mgmt_switch_judge(hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap,
    osal_char ant0_rssi_in, osal_char ant1_rssi_in)
{
    osal_u8 switch_chain;
    osal_u8 rssi_abs;
    osal_char ant0_rssi;
    osal_char ant1_rssi;
    hal_rx_ant_rssi_mgmt_stru *hal_rx_ant_rssi_mgmt;
    osal_u8 double_chain_flag = OSAL_FALSE;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_m2s_mgmt_switch_judge::param null.}");
        return;
    }

    hal_rx_ant_rssi_mgmt = get_hal_device_rx_ant_rssi_mgmt(hal_device);
    if (hal_rx_ant_rssi_mgmt->ant_rssi_sw != OSAL_TRUE) {
        return;
    }

    oal_rssi_smooth(&hal_rx_ant_rssi_mgmt->ant0_rssi_smth, ant0_rssi_in);
    oal_rssi_smooth(&hal_rx_ant_rssi_mgmt->ant1_rssi_smth, ant1_rssi_in);

    ant0_rssi = oal_get_real_rssi(hal_rx_ant_rssi_mgmt->ant0_rssi_smth);
    ant1_rssi = oal_get_real_rssi(hal_rx_ant_rssi_mgmt->ant1_rssi_smth);

    rssi_abs = (osal_u8)oal_absolute_sub(ant0_rssi, ant1_rssi);

    /* 若两根天线的RSSI都大于该门限，则认为处于强信号。强弱信号设置不同的切换门限 */
    if ((ant0_rssi > WLAN_FAR_DISTANCE_RSSI) && (ant1_rssi > WLAN_FAR_DISTANCE_RSSI)) {
        hal_rx_ant_rssi_mgmt->rssi_th = HAL_ANT_SWITCH_RSSI_MGMT_STRONG_TH;
    } else {
        hal_rx_ant_rssi_mgmt->rssi_th = HAL_ANT_SWITCH_RSSI_MGMT_WEAK_TH;
    }

    if (rssi_abs < hal_rx_ant_rssi_mgmt->rssi_th) {
        return;
    }

    if (ant0_rssi > ant1_rssi) {
        switch_chain = WLAN_TX_CHAIN_ZERO;
    } else {
        switch_chain = WLAN_TX_CHAIN_ONE;
    }
    /* 弱信号场景(rssi <-73dbm)时，响应帧控制帧采用双通道发送,避免收发不平衡导致的单天线发送失败 */
    if (((ant0_rssi != OAL_RSSI_INIT_VALUE) && (ant0_rssi < WLAN_FAR_DISTANCE_RSSI)) &&
        ((ant1_rssi != OAL_RSSI_INIT_VALUE) && (ant1_rssi < WLAN_FAR_DISTANCE_RSSI))) {
        double_chain_flag = OSAL_TRUE;
    }

    if ((switch_chain == hal_device->cfg_cap_info->single_tx_chain) || ((double_chain_flag == OSAL_TRUE) &&
        (hal_device->cfg_cap_info->ctrl_frm_tx_double_chain_flag == OSAL_TRUE))) {
        return;
    }

    hal_device->cfg_cap_info->ctrl_frm_tx_double_chain_flag = double_chain_flag;
    oam_warning_log4(0, OAM_SF_M2S,
        "hmac_m2s_mgmt_switch_judge::single tx cur_chain[%d],last_tx_chain[%d],ant0[%d],ant1[%d],",
        switch_chain, hal_device->cfg_cap_info->single_tx_chain, (osal_s32)ant0_rssi, (osal_s32)ant1_rssi);
    oam_warning_log1(0, OAM_SF_M2S, "double chain flag[%d].", double_chain_flag);
    hmac_m2s_mgmt_switch(hal_device, hmac_vap, switch_chain);
}

/*****************************************************************************
 函 数 名  : hmac_m2s_proc_vap_init
 功能描述  : vap关联的m2s数据结构初始化
*****************************************************************************/
osal_u32 hmac_m2s_proc_vap_init(hmac_vap_stru *hmac_vap)
{
    hmac_vap_m2s_stru *m2s_vap_info = OSAL_NULL;
    osal_u32 ret;

    m2s_vap_info = (hmac_vap_m2s_stru *)osal_kmalloc(sizeof(hmac_vap_m2s_stru), OSAL_GFP_KERNEL);
    if (m2s_vap_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_proc_vap_init mem alloc fail");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(m2s_vap_info, sizeof(hmac_vap_m2s_stru), 0, sizeof(hmac_vap_m2s_stru));

    /* 注册特性数据结构 */
    ret = hmac_vap_feature_registered(hmac_vap, WLAN_FEATURE_INDEX_M2S, m2s_vap_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_M2S, "hmac_m2s_proc_vap_init register feature fail(%u)", ret);
        osal_kfree(m2s_vap_info);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_proc_vap_deinit
 功能描述  : vap关联的m2s数据结构去初始化
*****************************************************************************/
osal_void hmac_m2s_proc_vap_deinit(hmac_vap_stru *hmac_vap)
{
    hmac_vap_m2s_stru *m2s_vap_info = OSAL_NULL;

    m2s_vap_info = (hmac_vap_m2s_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_M2S);
    if (m2s_vap_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_m2s_proc_vap_deinit get feature fail");
        return;
    }

    hmac_vap_feature_unregister(hmac_vap, WLAN_FEATURE_INDEX_M2S);
    osal_kfree(m2s_vap_info);
}

/*****************************************************************************
 函 数 名  : hmac_config_vap_m2s_info_syn
 功能描述  : hmac_offload架构下同步sta vap的状态到dmac
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_vap_m2s_info_syn(hmac_vap_stru *hmac_vap)
{
    osal_u32                  ul_ret;
    mac_vap_m2s_stru            m2s_vap_info;
    hmac_device_stru            *hmac_device;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_M2S, "vap_id[%d] {hmac_config_vap_m2s_info_syn::mac_dev[%d] null.}",
                       hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        /* 根据当前带宽刷新mib，5g默认160M支持 */
        mac_mib_set_vht_channel_width_option_implemented(hmac_vap, WLAN_MIB_VHT_SUPP_WIDTH_80);
    } else { /* 5G恢复成最大默认能力 */
        mac_mib_set_vht_channel_width_option_implemented(hmac_vap,
            hmac_device_trans_bandwith_to_vht_capinfo(mac_device_get_cap_bw(hmac_device)));
    }

    /* host暂时只是opmode的cap flag需要同步 */
    m2s_vap_info.support_opmode = hmac_vap->cap_flag.opmode;

    /***************************************************************************
        直调, 同步VAP最新状态
    ***************************************************************************/
    ul_ret = hmac_config_vap_m2s_info(hmac_vap, &m2s_vap_info);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_config_vap_m2s_info_syn::hmac_config_send_event_etc failed[%d],user_id[%d].}",
            hmac_vap->vap_id, ul_ret);
    }

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_d2h_vap_m2s_info_syn
 功能描述  : hmac_offload架构下同步vap的m2s信息到hmac(user涉及的mimo-siso信息差异)
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_d2h_vap_m2s_info_syn(hmac_vap_stru *hmac_vap, wlan_m2s_type_enum_uint8 m2s_type)
{
    mac_cfg_kick_user_param_stru kick_user_param;
    osal_u32 ul_ret;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    /* 开关机压力测试，对于device同步host事件，vap可能已经删除，需要增加此类保护，
    后续d2h同步接口都需要增加判空保护 */
    if ((hmac_vap->init_flag == MAC_VAP_INVAILD) || (hmac_vap->mib_info == OAL_PTR_NULL)) {
        oam_warning_log3(0, OAM_SF_M2S,
            "{hmac_config_d2h_vap_m2s_info_syn::init_flag[%d], mib_info[%p]!}",
            hmac_vap->init_flag, hmac_vap->mib_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (m2s_type == WLAN_M2S_TYPE_HW) {
        /* 硬切换需要踢除所有主路上的用户 */
        kick_user_param.reason_code = MAC_UNSPEC_REASON;
        oal_set_mac_addr(kick_user_param.mac_addr, mac_addr);

        /* 1. 踢掉该vap的用户 */
        msg_info.data = (osal_u8 *)&kick_user_param;
        msg_info.data_len = OAL_SIZEOF(osal_u32);

        oam_warning_log2(0, OAM_SF_DFT,
            "hmac_config_d2h_vap_m2s_info_syn:m2s Hard Switch,kick vap[%d] user, send DISASSOC, err code [%d]",
            hmac_vap->vap_id, kick_user_param.reason_code);
        ul_ret = hmac_config_kick_user_etc(hmac_vap, &msg_info);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_M2S,
                             "vap_id[%d] {hmac_config_d2h_vap_m2s_info_syn::hmac_config_kick_user_etc fail!}",
                             hmac_vap->vap_id);
        }

        /* 2. 更新速率集，vap的空间流，暂时未看到其他需要更新的地方 */
        hmac_vap_init_rates_etc(hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2s_switch_protect_comp_event_status
 功能描述  : m2s切换保护完成状态通知host
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_m2s_switch_protect_comp_event_status(hmac_vap_stru *hmac_vap,
    hmac_m2s_complete_syn_stru *m2s_comp_param)
{
    osal_s32 ret;
    frw_msg msg = {0};

    if ((hmac_vap->init_flag == MAC_VAP_INVAILD) || (m2s_comp_param == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_M2S,
                         "{hmac_m2s_switch_protect_comp_event_status::hmac_vap->init_flag[%d], param[%p]!}",
                         hmac_vap->init_flag, m2s_comp_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    msg.data = (osal_u8 *)m2s_comp_param;
    msg.data_len = sizeof(hmac_m2s_complete_syn_stru);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_M2S_STATUS, FRW_POST_PRI_LOW, hmac_vap->vap_id, msg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : host同步vap m2s信息
输入参数  : 无
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_vap_m2s_info(hmac_vap_stru *hmac_vap, mac_vap_m2s_stru *m2s_info)
{
    mac_vap_m2s_stru *vap_m2s_info;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if ((hmac_vap == OSAL_NULL) || (m2s_info == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_vap_m2s_info::vap=%p m2s_info=%p}",
            (uintptr_t)hmac_vap, (uintptr_t)m2s_info);
        return OAL_FAIL;
    }

    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        /* 根据当前带宽刷新mib，5g默认160M支持 */
        mac_mib_set_vht_channel_width_option_implemented(hmac_vap, WLAN_MIB_VHT_SUPP_WIDTH_80);
    } else { /* 5G恢复成最大默认能力 */
        mac_mib_set_vht_channel_width_option_implemented(hmac_vap,
            hmac_device_trans_bandwith_to_vht_capinfo(hmac_device->device_cap.channel_width));
    }

    vap_m2s_info = (mac_vap_m2s_stru *)m2s_info;
    /* 同步opmode mib能力 */
    hmac_vap->cap_flag.opmode = vap_m2s_info->support_opmode;

    oam_warning_log3(0, OAM_SF_M2S,
        "vap_id[%d] {hmac_config_vap_m2s_info_syn::cap_flag.opmode:%d,VHTChannelWidth:%d.}", hmac_vap->vap_id,
        hmac_vap->cap_flag.opmode, mac_mib_get_vht_channel_width_option_implemented(hmac_vap));

    return OAL_SUCC;
}

osal_u32 refresh_switch_event(osal_u8 m2s_mode, hal_m2s_state_uint8 m2s_cur_state,
    hal_m2s_event_tpye_uint16 *m2s_event, const hal_to_dmac_device_stru *master_hal_device)
{
    if (m2s_mode == MAC_M2S_COMMAND_MODE_SET_AUTO || m2s_mode == MAC_M2S_COMMAND_MODE_SET_SISO_C0) {
        if (m2s_cur_state == HAL_M2S_STATE_MIMO) {
            *m2s_event = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0;
        } else {
            oam_warning_log3(0, OAM_SF_M2S,
                "{refresh_switch_event:m2s_type[%d]m2s_mode[%d]m2s_cur_state[%d] cur not support mss siso c0.}",
                get_hal_m2s_mode_tpye(master_hal_device), m2s_mode, m2s_cur_state);
            return OAL_SUCC;
        }
    } else if (m2s_mode == MAC_M2S_COMMAND_MODE_SET_SISO_C1) {
        if (m2s_cur_state == HAL_M2S_STATE_MIMO) {
            *m2s_event = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1;
        } else {
            oam_warning_log3(0, OAM_SF_M2S,
                "{refresh_switch_event:m2s_type[%d]m2s_mode[%d]m2s_cur_state[%d] cur not support mss siso c1.}",
                get_hal_m2s_mode_tpye(master_hal_device), m2s_mode, m2s_cur_state);
            return OAL_SUCC;
        }
    } else {
        /* 此时期望回mimo，有三种状态可能性 (1)SISO  (2)MISO稳定态 (3)MISO探测态 后两种可以一起处理 */
        if (m2s_cur_state == HAL_M2S_STATE_SISO) {
            *m2s_event = HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO;
        } else if (m2s_cur_state == HAL_M2S_STATE_MISO) {
            *m2s_event = HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO;
        } else {
            oam_warning_log3(0, OAM_SF_M2S,
                "{refresh_switch_event:m2s_type[%d]m2s_mode[%d]m2s_cur_state[%d] cur not support mss mimo.}",
                get_hal_m2s_mode_tpye(master_hal_device), m2s_mode, m2s_cur_state);
            return OAL_SUCC;
        }
    }

    return OAL_CONTINUE;
}

osal_u32 hmac_config_set_m2s_switch_c0(const mac_m2s_mgr_stru *m2s_mgr,
    const hal_to_dmac_device_stru *hal_device, hal_m2s_state_uint8 m2s_state, osal_u16 *switch_type)
{
    if ((m2s_state == HAL_M2S_STATE_SISO) &&
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ONE)) {
        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0;
        }
    } else if (m2s_state == HAL_M2S_STATE_MIMO) { /* 当前是mimo 切换c0 siso */
        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0;
        }
    } else if ((m2s_state == HAL_M2S_STATE_MISO) &&
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ONE)) {
        /* 当前是c1 miso 切换c0 siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            /* 1.说明此时miso是稳态，切换也只能稳定切换，关闭了切换保护
               2.miso过渡态，还没完成10帧探测，又来了切换操作，此时切换保护继续执行即可 */
            *switch_type = HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0;
        }
    } else if ((m2s_state == HAL_M2S_STATE_MISO) &&
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO)) {
        /* 当前是c0 miso 切换c0 siso */
        *switch_type = HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0;
    } else if (m2s_state == HAL_M2S_STATE_IDLE) { /* idle状态下切换，只是根据事件做软件能力变更即可 */
        /* 当前是idle 切换c0 siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C0;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0;
        }
    } else {
        oam_warning_log4(0, OAM_SF_M2S,
            "{hmac_config_set_m2s_switch_c0: expert state[%d] mode[%d],real state[%d]phy2dscr_chain[%d]!}",
            m2s_mgr->pri_data.test_mode.m2s_state, m2s_mgr->pri_data.test_mode.trigger_mode,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy2dscr_chain);
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

osal_u32 hmac_config_set_m2s_switch_c1(const mac_m2s_mgr_stru *m2s_mgr,
    const hal_to_dmac_device_stru *hal_device, hal_m2s_state_uint8 m2s_state, osal_u16 *switch_type)
{
    if ((m2s_state == HAL_M2S_STATE_SISO) &&
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO)) {
        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1;
        }
    } else if (m2s_state == HAL_M2S_STATE_MIMO) { /* 当前是mimo 切换到c1 siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1;
        }
    } else if ((m2s_state == HAL_M2S_STATE_MISO) && /* 当前是c0 miso 切换c1 siso 主要是指延迟切换 */
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO)) {
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            /* 1.说明此时miso是稳态，切换也只能稳定切换，关闭了切换保护
               2.miso过渡态，还没完成10帧探测，又来了切换操作，此时切换保护继续执行即可 */
            *switch_type = HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1;
        }
    } else if ((m2s_state == HAL_M2S_STATE_MISO) && /* 当前是c1 miso 切换c1 siso */
        (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ONE)) {
        *switch_type = HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1;
    } else if (m2s_state == HAL_M2S_STATE_IDLE) { /* 当前是idle 切换c1 siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C1;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1;
        }
    } else {
        oam_warning_log4(0, OAM_SF_M2S,
            "{hmac_config_set_m2s_switch_c1: expert state[%d] mode[%d],real state[%d]phy2dscr_chain[%d]!}",
            m2s_mgr->pri_data.test_mode.m2s_state, m2s_mgr->pri_data.test_mode.trigger_mode,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy2dscr_chain);
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

osal_u32 hmac_config_set_m2s_switch_mimo(const mac_m2s_mgr_stru *m2s_mgr,
    const hal_to_dmac_device_stru *hal_device, hal_m2s_state_uint8 m2s_state, osal_u16 *switch_type)
{
    if (m2s_state == HAL_M2S_STATE_SISO) {
        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_SISO_TO_MIMO;
        }
    } else if (m2s_state == HAL_M2S_STATE_MISO) { /* 当前是c0/c1 miso 切换到mimo */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_MISO_TO_MIMO;
        }
    } else if (m2s_state == HAL_M2S_STATE_IDLE) { /* 当前是ilde 切换到mimo */
        if (m2s_mgr->pri_data.test_mode.trigger_mode == WLAN_M2S_TRIGGER_MODE_COMMAND) {
            *switch_type = HAL_M2S_EVENT_COMMAND_IDLE_TO_MIMO;
        } else {
            *switch_type = HAL_M2S_EVENT_TEST_IDLE_TO_MIMO;
        }
    } else {
        oam_warning_log4(0, OAM_SF_M2S,
            "{hmac_config_set_m2s_switch_mimo:expert state[%d] mode[%d], real state[%d]phy2dscr_chain[%d]!}",
            m2s_mgr->pri_data.test_mode.m2s_state, m2s_mgr->pri_data.test_mode.trigger_mode,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy2dscr_chain);
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

osal_u32 hmac_config_set_m2s_switch_delay(const mac_m2s_mgr_stru *m2s_mgr,
    const hal_to_dmac_device_stru *hal_device, hal_m2s_state_uint8 m2s_state, osal_u16 *switch_type)
{
    switch (m2s_mgr->pri_data.test_mode.m2s_state) {
        /* 1.期望软件切换到c0 siso */
        case MAC_M2S_COMMAND_STATE_SISO_C0:
            /* 当前是c1 siso  切换到c0 siso */
            return hmac_config_set_m2s_switch_c0(m2s_mgr, hal_device, m2s_state, switch_type);

        /* 2.期望软件切换到c1 siso */
        case MAC_M2S_COMMAND_STATE_SISO_C1:
            /* 当前是c0 siso  切换到c1 siso */
            return hmac_config_set_m2s_switch_c1(m2s_mgr, hal_device, m2s_state, switch_type);

        /* 3.期望软件切换到mimo */
        case MAC_M2S_COMMAND_STATE_MIMO:
            /* 当前是c0/c1 siso 切换到mimo */
            return hmac_config_set_m2s_switch_mimo(m2s_mgr, hal_device, m2s_state, switch_type);

        /* 4.期望软件切换到c0 miso 默认只有3和4支持 */
        case MAC_M2S_COMMAND_STATE_MISO_C0:
            /* 当前是c0/c1 siso  切换到c0 miso */
            if (m2s_state == HAL_M2S_STATE_SISO) {
                *switch_type = HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0;
            } else if (m2s_state == HAL_M2S_STATE_MIMO) { /* 当前是mimo 切换到c0 miso */
                *switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0;
            } else if ((m2s_state == HAL_M2S_STATE_MISO) && /* 当前是c1 miso 切换c0 miso */
                (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ONE)) {
                *switch_type = HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0;
            } else if (m2s_state == HAL_M2S_STATE_IDLE) {  /* 当前是idle 切换c0 miso */
                *switch_type = HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0;
            } else {
                oam_warning_log4(0, OAM_SF_M2S,
                    "{hmac_config_set_m2s_switch_delay:expert state[%d] mode[%d], real state[%d]phy2dscr_chain[%d]!}",
                    m2s_mgr->pri_data.test_mode.m2s_state, m2s_mgr->pri_data.test_mode.trigger_mode,
                    hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy2dscr_chain);
                return OAL_FAIL;
            }
            break;

        /* 5.期望软件切换到c1 miso 默认只有3和4支持 */
        case MAC_M2S_COMMAND_STATE_MISO_C1:
            /* 当前是c0/c1 siso  切换到c1 miso */
            if (m2s_state == HAL_M2S_STATE_SISO) {
                *switch_type = HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1;
            } else if (m2s_state == HAL_M2S_STATE_MIMO) { /* 当前是mimo 切换到c1 miso */
                *switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1;
            } else if ((m2s_state == HAL_M2S_STATE_MISO) && /* 当前是c0 miso 切换c1 miso */
                (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO)) {
                *switch_type = HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1;
            } else if ((m2s_state == HAL_M2S_STATE_IDLE) && /* 当前是ilde 切换c1 miso */
                (hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_ZERO)) {
                *switch_type = HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1;
            } else {
                oam_warning_log4(0, OAM_SF_M2S,
                    "{hmac_config_set_m2s_switch_delay:expert state[%d] mode[%d], real state[%d]phy2dscr_chain[%d]!}",
                    m2s_mgr->pri_data.test_mode.m2s_state, m2s_mgr->pri_data.test_mode.trigger_mode,
                    hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy2dscr_chain);
                return OAL_FAIL;
            }
            break;
        default:
            oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_set_m2s_switch_delay: command_m2s_state[%d] error!}",
                m2s_mgr->pri_data.test_mode.m2s_state);
            return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hmac_config_set_m2s_switch
 功能描述  : 设置mimo或者siso工作模式
*****************************************************************************/
osal_s32 hmac_config_set_m2s_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_m2s_mgr_stru *m2s_mgr = (mac_m2s_mgr_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u32 ret;
    osal_u16 switch_type = HAL_M2S_EVENT_BUTT;
    hal_m2s_state_uint8 m2s_state;

    if (hal_device == OSAL_NULL) {
        return OAL_FAIL;
    }
    switch (m2s_mgr->cfg_m2s_mode) {
        /* mimo-siso切换参数查询 */
        case MAC_M2S_MODE_QUERY:
#ifdef _PRE_WIFI_DEBUG
            hmac_m2s_mgr_param_info(hal_device);
            hmac_m2s_show_blacklist_in_list(hal_device);
#endif
            break;

        /* mss开关使能和关闭 */
        case MAC_M2S_MODE_MSS:
            get_hal_device_m2s_mss_on(hal_device) = m2s_mgr->pri_data.mss_mode.mss_on;
            oam_warning_log2(0, OAM_SF_M2S, "vap_id[%d] {hmac_config_set_m2s_switch::mss_on[%d].}", hmac_vap->vap_id,
                get_hal_device_m2s_mss_on(hal_device));
            break;

        /* 软切换和硬切换测试模式(前面已经提前抛事件做了参数更新)，这里开始运用切换管理模块统一来做 */
        case MAC_M2S_MODE_SW_TEST:
        case MAC_M2S_MODE_HW_TEST:
        case MAC_M2S_MODE_DELAY_SWITCH:
            m2s_state = hal_device->hal_m2s_fsm.oal_fsm.cur_state;

            /* 0.刷新软硬件切换能力 */
            hal_device->hal_m2s_fsm.m2s_type = m2s_mgr->pri_data.test_mode.m2s_type;
            ret = hmac_config_set_m2s_switch_delay(m2s_mgr, hal_device, m2s_state, &switch_type);
            if (ret != OAL_CONTINUE) {
                return ret;
            }
            oam_warning_log4(0, 0, "{hmac_config_set_m2s_switch: m2s_mode[%d] ex_state[%d] cur_state[%d] type[%d]!}",
                m2s_mgr->cfg_m2s_mode, m2s_mgr->pri_data.test_mode.m2s_state, m2s_state, switch_type);

            /* 满足切换条件，进行切换跳转 */
            if (hmac_m2s_switch_apply_and_confirm(hal_device, switch_type, m2s_mgr->pri_data.test_mode.trigger_mode) ==
                OSAL_TRUE) {
                hmac_m2s_handle_event(hal_device, switch_type, 0, OSAL_NULL);
            }
            break;

        case MAC_M2S_MODE_RSSI:
            hmac_config_set_m2s_mode_rssi(m2s_mgr, hal_device);
            break;
        default:
            oam_warning_log1(0, 0, "{hmac_config_set_m2s_switch: m2s_mode[%d] invalid!}", m2s_mgr->cfg_m2s_mode);
            return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : host同步黑名单用户到device侧
输入参数  : 无
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_m2s_switch_blacklist(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_m2s_ie_stru *m2s_ie;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    wlan_m2s_mgr_vap_stru *m2s_mgr_blacklist;
    osal_u8 idx;
    osal_u32 blacklist_max_num;
    hal_device_m2s_mgr_stru *device_m2s_mgr = OSAL_NULL;

    m2s_ie = (mac_m2s_ie_stru *)msg->data;
    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_set_m2s_switch_blacklist::vap id [%d],hal_device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    device_m2s_mgr = get_hal_device_m2s_mgr(hal_device);
    /* 开关wifi时，刷新device侧的m2s黑名单，此时黑名单用户数要为0，否则存在问题 */
    if (device_m2s_mgr->blacklist_bss_cnt != 0) {
        /* 上层状态机可能出现不匹配，在wifi不在开关状态也下发黑名单，此处直接返回即可 */
        oam_warning_log1(0, OAM_SF_M2S,
            "vap_id[%d] {hmac_config_set_m2s_switch_blacklist::device blacklist is not zero.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    device_m2s_mgr->blacklist_bss_cnt = m2s_ie->blacklist_cnt;
    blacklist_max_num = osal_array_size(m2s_ie->m2s_blacklist);
    for (idx = 0; idx < m2s_ie->blacklist_cnt; idx++) {
        if (device_m2s_mgr->blacklist_bss_index >= blacklist_max_num) {
            oam_warning_log2(0, OAM_SF_CFG,
                "{hmac_config_set_m2s_switch_blacklist::vap id [%d],blacklist_bss_index[%d]}", hmac_vap->vap_id,
                device_m2s_mgr->blacklist_bss_index);
            break;
        }
        m2s_mgr_blacklist = &(device_m2s_mgr->m2s_blacklist[device_m2s_mgr->blacklist_bss_index]);
        (osal_void)memcpy_s(m2s_mgr_blacklist, sizeof(wlan_m2s_mgr_vap_stru),
            &(m2s_ie->m2s_blacklist[device_m2s_mgr->blacklist_bss_index]), sizeof(wlan_m2s_mgr_vap_stru));

        device_m2s_mgr->blacklist_bss_index++;
    }

    oam_warning_log3(0, OAM_SF_M2S,
        "vap_id[%d] {hmac_config_set_m2s_switch_blacklist::blacklist_cnt[%d] blacklist_index[%d] SUCC.}",
        hmac_vap->vap_id, device_m2s_mgr->blacklist_bss_cnt, device_m2s_mgr->blacklist_bss_index);

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : host同步mss到device
          (1)MSS触发的切换结果，才上报到host侧，减少device自身切换造成频繁唤醒host触发的功耗
          (2)MSS为了保证和驱动状态一致，自己get当前状态
输入参数  : 无
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_m2s_switch_mss(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_to_dmac_device_stru *master_hal_device = OSAL_NULL;
    osal_u8 m2s_mode = *((osal_u8 *)msg->data);
    hal_m2s_event_tpye_uint16 m2s_event = HAL_M2S_EVENT_BUTT;
    hal_m2s_state_uint8 m2s_cur_state;
    hmac_m2s_complete_syn_stru m2s_comp_param;
    /* dbdc下vap在辅路了，此时需要保证查询主路的状态 */
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /*  默认都是legacy sta下发的切换，驱动支持+p2p gc */
    (osal_void)memset_s(&m2s_comp_param, sizeof(hmac_m2s_complete_syn_stru), 0, sizeof(hmac_m2s_complete_syn_stru));

    master_hal_device = hmac_device->hal_device[HAL_DEVICE_ID_MASTER];
    if (master_hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_m2s_switch_mss::master_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 0.功能使能才继续响应下发 */
    if (get_hal_device_m2s_mss_on(master_hal_device) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_set_m2s_switch_mss::[%d].}", m2s_mode);

    /* 当前状态 */
    m2s_cur_state = master_hal_device->hal_m2s_fsm.oal_fsm.cur_state;

    /* 1.获取state为get，直接上报当前结果即可，查询结果永远为1 OSAL_TRUE */
    if (m2s_mode == MAC_M2S_COMMAND_MODE_GET_STATE) {
        m2s_comp_param.m2s_state = master_hal_device->hal_m2s_fsm.oal_fsm.cur_state;
        m2s_comp_param.m2s_mode = get_hal_m2s_mode_tpye(master_hal_device);
        m2s_comp_param.m2s_result = OSAL_TRUE;
        return hmac_m2s_switch_protect_comp_event_status(hmac_vap, &m2s_comp_param);
    }

    /* 3.mss优先级较低，需要切换到mimo、siso时，有其他业务存在需要保持在特定siso状态，按照驱动为主，直接返回 */
    if ((get_hal_m2s_mode_tpye(master_hal_device) & (BIT0 | BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7)) != 0) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{hmac_config_set_m2s_switch_mss:m2s_type[%d] cur not support mss.}",
                         get_hal_m2s_mode_tpye(master_hal_device));
        return OAL_SUCC;
    }

    /* 4.刷新切换事件 */
    if (refresh_switch_event(m2s_mode, m2s_cur_state, &m2s_event, master_hal_device) == OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 5.如果是处于mimo需要切换到siso，直接调用切换即可,结果上报在siso complete接口中完成 */
    /* 如果是处于SISO需要切换到siso/mimo，直接调用切换即可，此时需要单独上报切换结果 */
    hmac_m2s_handle_event(master_hal_device, m2s_event, 0, OSAL_NULL);

    /* 6.刷新当前状态 */
    m2s_cur_state = master_hal_device->hal_m2s_fsm.oal_fsm.cur_state;

    /* 7.结果需要单独上报,切回mimo需要单独在这里做结果上报 */
    if (m2s_event == HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO || m2s_event == HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO) {
        (m2s_cur_state == HAL_M2S_STATE_MIMO) ? (m2s_comp_param.m2s_result = OSAL_TRUE) :
        (m2s_comp_param.m2s_result = OSAL_FALSE);
    } else {
        return OAL_SUCC;
    }

    /* 8.切换到mimo需要在此做结果上报，切siso，是在complete中做上报 */
    m2s_comp_param.m2s_state = master_hal_device->hal_m2s_fsm.oal_fsm.cur_state;
    m2s_comp_param.m2s_mode = get_hal_m2s_mode_tpye(master_hal_device);

    return hmac_m2s_switch_protect_comp_event_status(hmac_vap, &m2s_comp_param);
}
#endif

osal_void hmac_config_set_m2s_mode_rssi(const mac_m2s_mgr_stru *m2s_mgr, hal_to_dmac_device_stru *hal_device)
{
    hal_rx_ant_rssi_mgmt_stru *hal_rx_ant_rssi_mgmt;
    switch (m2s_mgr->pri_data.rssi_mode.opt) {
        case HAL_M2S_RSSI_SHOW_TH:
            oam_warning_log2(0, OAM_SF_M2S, "{hmac_config_set_m2s_mode_rssi::ori_min_th[%d] diff_th[%d]}",
                hal_device->rssi.ori_min_th, hal_device->rssi.diff_th);
            break;
        case HAL_M2S_RSSI_SHOW_MGMT:
            hal_rx_ant_rssi_mgmt = get_hal_device_rx_ant_rssi_mgmt(hal_device);
            oam_warning_log2(0, OAM_SF_M2S, "{hmac_config_set_m2s_mode_rssi::mgmt ant0[%d] ant1[%d]}",
                oal_get_real_rssi(hal_rx_ant_rssi_mgmt->ant0_rssi_smth),
                oal_get_real_rssi(hal_rx_ant_rssi_mgmt->ant1_rssi_smth));
            break;
        case HAL_M2S_RSSI_SHOW_DATA:
            oam_warning_log3(0, OAM_SF_M2S, "{hmac_config_set_m2s_mode_rssi::data ant0[%d] ant1[%d] cnt[%d]}",
                oal_get_real_rssi(hal_device->rssi.rx_rssi.ant0_rssi_smth),
                oal_get_real_rssi(hal_device->rssi.rx_rssi.ant1_rssi_smth),
                hal_device->rssi.rx_rssi.rssi_high_cnt);
            break;
        case HAL_M2S_RSSI_SET_MIN_TH:
            hal_device->rssi.ori_min_th = m2s_mgr->pri_data.rssi_mode.value;
            break;
        case HAL_M2S_RSSI_SET_DIFF_TH:
            hal_device->rssi.diff_th = (osal_u8)m2s_mgr->pri_data.rssi_mode.value;
            break;
        default:
            oam_warning_log0(0, OAM_SF_M2S, "{hmac_config_set_m2s_mode_rssi::m2s rssi invalid input}");
            break;
    }
}

osal_void hmac_config_start_vap_ext_m2s(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device)
{
    osal_void *fhook = OSAL_NULL;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if (hmac_m2s_d2h_vap_info_syn(hmac_vap) != OAL_SUCC) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_start_vap_ext:m2s_d2h_vap_info fail}", hmac_vap->vap_id);
        }

        /* 如果2g ap开启静态siso模式，切换到c1 siso;如果5g ap开启静态siso模式，切换到c1 siso */
        /* 解决方案上，暂时没有ap+sta共存，ap切换会自动恢复成mimo，单独恢复时不做处理即可 */
        if (is_legacy_ap(hmac_vap) && ((((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_2GAP) != 0) &&
            hmac_vap->channel.band == WLAN_BAND_2G) ||
            (((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_5GAP) != 0) &&
            hmac_vap->channel.band == WLAN_BAND_5G)) && hal_device != OSAL_NULL) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_start_vap_ext:ap 2G/5G open static SISO}",
                hmac_vap->vap_id);

            /* 定制化执行硬切换方式 */
            hal_device->hal_m2s_fsm.m2s_type = WLAN_M2S_TYPE_HW;

            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0, 0, OSAL_NULL);

            /* 定制化执行完成，需要恢复默认配置 */
            hal_device->hal_m2s_fsm.m2s_type = WLAN_M2S_TYPE_SW;
        }

        /* 前者静态siso优先级高于后两者动态siso，需要按照顺序 */
        /* 1.单vap时，此时要么是mimo，要么是dbdc,;2.多vap时，此时要么dbdc，要么本身c0 siso也申请 此时不可能回mimo */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_M2S_M2S_ALLOW_CHECK);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_m2s_allow_check_cb)fhook)(hal_device, hmac_vap);
        }
        /* ap模式需要在启动时候判断是否启动ps */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_STOP_CHECK);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_ps_stop_check_and_notify_cb)fhook)(hmac_vap->hal_device);
        }

        /* 2.如果不是启动在dbdc，如果mss_on,需要考虑会mimo，mss是legacy sta生效 */
        if (hal_device != OSAL_NULL && hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
            /* MSS存在时，不会有其他m2s业务存在，此时直接回mimo即可; 标志和切换保护功能在此回mimo时清 */
            hmac_m2s_command_mode_force_to_mimo(hal_device);
        }
    }
}
#endif /* _PRE_WLAN_FEATURE_M2S */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
