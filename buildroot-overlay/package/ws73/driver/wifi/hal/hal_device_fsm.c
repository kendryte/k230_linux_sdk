/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: FSM for hal device.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_device_fsm.h"
#include "hal_device.h"
#include "hmac_device.h"
#include "hal_chip.h"
#include "hal_mac_reg_field.h"
#include "hal_common_ops.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hal_coex_reg.h"
#endif
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_DEVICE_FSM_ROM_C

osal_void hal_device_state_init_entry(osal_void *ctx);
osal_void hal_device_state_init_exit(osal_void *ctx);
osal_u32 hal_device_state_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);

osal_void hal_device_state_idle_entry(osal_void *ctx);
osal_void hal_device_state_idle_exit(osal_void *ctx);
osal_u32 hal_device_state_idle_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hal_device_state_work_entry(osal_void *ctx);
osal_void hal_device_state_work_exit(osal_void *ctx);
osal_u32 hal_device_state_work_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
osal_void hal_device_state_scan_entry(osal_void *ctx);
osal_void hal_device_state_scan_exit(osal_void *ctx);
osal_u32 hal_device_state_scan_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 全局状态机函数表 */
oal_fsm_state_info g_hal_device_fsm_info[] = {
    {
        HAL_DEVICE_INIT_STATE,
        "INIT",
        hal_device_state_init_entry,
        hal_device_state_init_exit,
        hal_device_state_init_event,
    },

    {
        HAL_DEVICE_IDLE_STATE,
        "IDLE",
        hal_device_state_idle_entry,
        hal_device_state_idle_exit,
        hal_device_state_idle_event,
    },

    {
        HAL_DEVICE_WORK_STATE,
        "WORK",
        hal_device_state_work_entry,
        hal_device_state_work_exit,
        hal_device_state_work_event,
    },

    {
        HAL_DEVICE_SCAN_STATE,
        "SCAN",
        hal_device_state_scan_entry,
        hal_device_state_scan_exit,
        hal_device_state_scan_event,
    },
};

osal_u8 g_mac_pa_switch = OSAL_TRUE;
/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hal_device_handle_scan_abort
 功能描述  : 扫描abort,任何状态下都有可能
*****************************************************************************/
static INLINE__ osal_void hal_device_handle_scan_abort(hal_to_dmac_device_stru *hal_device)
{
    hal_scan_params_stru *hal_scan_params = &(hal_device->hal_scan_params);

    hal_scan_params->working_in_home_chan = OSAL_FALSE; // 扫描完成不需要此标记
    hal_scan_params->need_switch_back_home_channel = OSAL_FALSE;
    hal_scan_params->curr_scan_state = MAC_SCAN_STATE_IDLE;

    oam_warning_log1(0, OAM_SF_SCAN, "{hal_device_scan_abort:state[%d]}",
        hal_device->hal_dev_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hal_device_handle_scan_pause
 功能描述  : 扫描pause
*****************************************************************************/
static INLINE__ osal_void hal_device_handle_scan_pause(hal_to_dmac_device_stru *hal_device,
    hal_scan_pause_type_enum_uint8 hal_san_pause_type)
{
    if (oal_bit_get_bit_one_byte(hal_device->hal_scan_params.scan_pause_bitmap, hal_san_pause_type)) {
        oam_error_log2(0, OAM_SF_DBDC,
            "{hal_device_handle_scan_pause::alreay paused,type[0x%x]scan idx[%d]}", hal_san_pause_type,
            hal_device->hal_scan_params.scan_chan_idx);
        return;
    }

    oal_bit_set_bit_one_byte(&(hal_device->hal_scan_params.scan_pause_bitmap), hal_san_pause_type);

    if (hal_device->hal_scan_params.scan_timer.is_registerd) {
        oam_warning_log2(0, OAM_SF_DBDC,
            "{hal_device_handle_scan_pause::pause type[0x%x]scan idx[%d]} timer is regsterd", hal_san_pause_type,
            hal_device->hal_scan_params.scan_chan_idx);
        frw_timer_stop_timer(&(hal_device->hal_scan_params.scan_timer));
    }

    oam_warning_log1(0, OAM_SF_SCAN, "{hal_device_handle_scan_pause::idx[%d]}",
        hal_device->hal_scan_params.scan_chan_idx);
}

/*****************************************************************************
 函 数 名  : hal_device_handle_scan_resume
 功能描述  : 扫描resume
*****************************************************************************/
static INLINE__ osal_void hal_device_handle_scan_resume(hal_to_dmac_device_stru *hal_device,
    hal_scan_pause_type_enum_uint8 hal_san_pause_type)
{
    if (!(hal_device->hal_scan_params.scan_pause_bitmap)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hal_device_handle_scan_resume:: not paused}");
        return;
    }

    oal_bit_clear_bit_one_byte(&(hal_device->hal_scan_params.scan_pause_bitmap), hal_san_pause_type);

    /* 信道切回homechannel触发的scan pause需要重新申请rf开前端 */
    if (hal_san_pause_type == HAL_SCAN_PASUE_TYPE_SWITCH_BACK) {
        hal_device_update_phy_chain(hal_device, OSAL_TRUE); /* 辅路申请rf资源 */
        hal_set_ana_dbb_ch_sel(hal_device);               /* 配置soc 天线和phy通道对应寄存器 */
        hal_pm_enable_front_end_to_dev(hal_device, OSAL_TRUE);
    }

    if (hal_device->hal_scan_params.scan_pause_bitmap != 0) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hal_device_handle_scan_resume:: paused[0x%x]}",
            hal_device->hal_scan_params.scan_pause_bitmap);
        return;
    }

    if (hal_device->hal_dev_fsm.hal_device_fsm_cb.hmac_scan_one_channel_start) {
        hal_device->hal_dev_fsm.hal_device_fsm_cb.hmac_scan_one_channel_start(hal_device, OSAL_TRUE);
    }
    oam_warning_log1(0, OAM_SF_SCAN,
        "{hal_device_handle_scan_resume:: idx[%d]}", hal_device->hal_scan_params.scan_chan_idx);
}

/*****************************************************************************
 函 数 名  : hal_device_get_state_from_bitmap
 功能描述  : 通过work bitmap反推现在hal device的状态,在scan end 和abort的时候调用
*****************************************************************************/
static INLINE__ hal_device_state_info hal_device_get_state_from_bitmap(hal_to_dmac_device_stru *hal_device)
{
    if (hal_device->work_vap_bitmap) {
        return HAL_DEVICE_WORK_STATE;
    } else {
        return HAL_DEVICE_IDLE_STATE;
    }
}

/*****************************************************************************
 函 数 名  : hal_every_channel_scan_state_clean
 功能描述  : 扫描时切换到每个信道都要去清的变量
*****************************************************************************/
static INLINE__ osal_void hal_every_channel_scan_state_clean(hal_scan_params_stru *hal_scan_params)
{
    if (hal_scan_params == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "hal_scan_every_channel_state_chean:: NULL PTR");
        return;
    }

    /* 本信道扫描到BSS 状态清除 */
    hal_scan_params->scan_curr_chan_find_bss_flag = OSAL_FALSE;
    hal_scan_params->working_in_home_chan = OSAL_FALSE;
    hal_scan_params->curr_channel_scan_count = 0; /* 本信道扫描次数清零 */
}

/*****************************************************************************
 函 数 名  : hal_scan_params_prepare
 功能描述  : 扫描开始时的公共参数准备
*****************************************************************************/
osal_void hal_scan_params_prepare(hal_to_dmac_device_stru *hal_device,
    const hal_scan_info_stru *hal_scan_info, hal_scan_params_stru *hal_scan_params)
{
    hal_dev_fsm_stru *hal_dev_fsm = &(hal_device->hal_dev_fsm);

    if ((hal_dev_fsm->is_fsm_attached == OSAL_FALSE) || (hal_scan_info == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hal_scan_params_prepare::fsm not attached.scan_channel_info[0x%p]}",
            (uintptr_t)hal_scan_info);
        return;
    }

    /* 扫描开始如果有定时器存在先删除扫描定时器cca */
    if (hal_scan_params->scan_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hal_device->hal_scan_params.scan_timer));
    }

    /* 扫描开始先清零 */
    (osal_void)memset_s(hal_scan_params, sizeof(hal_scan_params_stru), 0, sizeof(hal_scan_params_stru));

    /* 清空信道测量结果 */
    (osal_void)memset_s(&(hal_device->chan_result), sizeof(wlan_scan_chan_stats_stru),
        0, sizeof(wlan_scan_chan_stats_stru));

    /* 记录扫描开始时间,扫描结束后看整体扫描时间 */
    hal_scan_params->scan_timestamp = (osal_u32)osal_get_time_stamp_ms();

    hal_scan_params->scan_chan_idx = 0;
    hal_scan_params->curr_scan_state = MAC_SCAN_STATE_RUNNING;
    hal_scan_params->need_switch_back_home_channel = OSAL_FALSE;
    hal_scan_params->scan_ap_num_in_2p4 = 0;
    hal_scan_params->scan_ap_num_in_5p0 = 0;
    hal_scan_params->last_channel_band = WLAN_BAND_BUTT;

    hal_scan_params->scan_mode = hal_scan_info->scan_mode;
    hal_scan_params->scan_band = WLAN_BAND_BUTT;

    /* mac device下的每个信道扫描最大次数,以及每次扫描停留的时间都传给hal device,hal device可以改动其中的任何一个参数 */
    hal_scan_params->max_scan_count_per_channel = hal_scan_info->max_scan_count_per_channel;
    hal_scan_params->scan_time = hal_scan_info->scan_time;

    hal_scan_params->start_chan_idx = 0;
    hal_scan_params->channel_nums = hal_scan_info->num_channels_2g + hal_scan_info->num_channels_5g;
}

/*****************************************************************************
 函 数 名  : hal_scan_handle_scan_back
 功能描述  : 扫描切回home channel
*****************************************************************************/
osal_void hal_scan_handle_scan_back(hal_to_dmac_device_stru *hal_device, osal_u16 event)
{
    hal_device_state_info new_dev_state;
    hal_scan_params_stru *hal_scan_params = &(hal_device->hal_scan_params);

    if (hal_scan_params->need_switch_back_home_channel == OSAL_TRUE) {
        /* 扫描完成不需要再置need_switch_back_home_channel */
        if (event == HAL_DEVICE_EVENT_SCAN_END) {
            hal_scan_params->working_in_home_chan = OSAL_FALSE;
            hal_scan_params->need_switch_back_home_channel = OSAL_FALSE;
        }

        new_dev_state = HAL_DEVICE_WORK_STATE;
    } else {
        new_dev_state = hal_device_get_state_from_bitmap(hal_device);
        /* 如果是切回home chanenl事件却未置back home channel位一定是有问题 */
        if ((event == HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK) && (new_dev_state == HAL_DEVICE_WORK_STATE)) {
            oam_error_log2(0, OAM_SF_SCAN, "{hal_scan_handle_scan_back::event[%d], work bitmap[%d]}", event,
                hal_device->work_vap_bitmap);
        }
    }

    hal_device_fsm_trans_to_state(&(hal_device->hal_dev_fsm), new_dev_state, event);
}

osal_void hal_device_add_work_vap_bitmap_bcn_cfg(hal_to_dmac_device_stru *hal_device,
    hal_to_dmac_vap_stru *hal_vap)
{
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
    osal_u8 up_vap_num = hal_device_calc_up_vap_num(hal_device);
    if ((hal_device->cfg_cap_info->rf_chain == WLAN_RF_CHAIN_DOUBLE) && (up_vap_num > 1)) {
        hal_device_reset_bcn_rf_chain(hal_device, up_vap_num);
    }
#endif
    unref_param(hal_device);
    unref_param(hal_vap);
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    /* 单sta场景打开,其他场景关闭 */
    if ((oal_bit_get_num_four_byte(hal_device->work_vap_bitmap) == 1) &&
        (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (hal_vap->p2p_mode == WLAN_LEGACY_VAP_MODE)) {
        hal_device->mac_parse_tim = OSAL_TRUE;
        /* unmask tbtt中断,前一个1表示连续1个bit，后一个0表示val */
        hal_reg_write_bits_by_msg(HH503_MAC_CTRL0_BANK_BASE_0x54, HH503_MAC_RPT_INTR_BCN_NO_FRM_OFFSET, 1, 0, OSAL_TRUE);
    } else {
        hal_device->mac_parse_tim = OSAL_FALSE;
        hal_clear_mac_int_status(HH503_MAC_RPT_INTR_BCN_NO_FRM_MASK);
        /* mask tbtt中断,前一个1表示连续1个bit，后一个1表示val */
        hal_reg_write_bits_by_msg(HH503_MAC_CTRL0_BANK_BASE_0x54, HH503_MAC_RPT_INTR_BCN_NO_FRM_OFFSET, 1, 1, OSAL_TRUE);
    }
#endif
}

/*****************************************************************************
 函 数 名  : hal_device_add_work_vap_bitmap
 功能描述  : work vap add bitmap
*****************************************************************************/
osal_void hal_device_add_work_vap_bitmap(hal_to_dmac_device_stru *hal_device, osal_u16 event,
    osal_void *event_data)
{
    hal_to_dmac_vap_stru *hal_vap = OAL_PTR_NULL;
    osal_u8 hal_vap_id;
    oal_bool_enum_uint8 vap_is_registered;

    if (event_data == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::event[%d], ptr null}", event);
        return;
    }

    hal_vap = (hal_to_dmac_vap_stru *)event_data;
    hal_vap_id = hal_vap->vap_id;

    if (hal_vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log3(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::vap id error[%d],"
            "mac vap id[%d]vap mode[%d]}", hal_vap_id, hal_vap->mac_vap_id, hal_vap->vap_mode);
        return;
    }

    if ((event == HAL_DEVICE_EVENT_JOIN_COMP) && (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        oam_error_log3(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::event join complete,hal vap id[%d],"
            "vap mode[%d]!!!not sta}", hal_vap_id, hal_vap->vap_mode, hal_device->work_vap_bitmap);
        return;
    }

    vap_is_registered = hal_device->work_vap_bitmap & osal_bit(hal_vap_id);
    /* STAUT HAL_DEVICE_EVENT_JOIN_COMP时注册 */
    if ((hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (event == HAL_DEVICE_EVENT_VAP_UP) && (!vap_is_registered)) {
        oam_error_log4(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::hal vap id[%d],registerd[%d],"
            "event[%d],now bitmap[%d]!!!}", hal_vap_id, vap_is_registered, event, hal_device->work_vap_bitmap);
    }

    hal_vap->hal_vap_state = (event == HAL_DEVICE_EVENT_JOIN_COMP) ? HAL_VAP_STATE_CONNECT : HAL_VAP_STATE_UP;
    hal_device_sync_vap(hal_vap);
    if (vap_is_registered) {
        oam_warning_log4(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::event[%d],hal vap id[%d],vap mode[%d]"
            "already registered,now bitmap[%d]}", event, hal_vap_id, hal_vap->vap_mode, hal_device->work_vap_bitmap);
        return;
    }

    hal_device->work_vap_bitmap |= (osal_u32)osal_bit(hal_vap_id);
    /* 将work_vap_bitmap同步到device侧 */
    hal_dev_sync_work_vap_bitmap(hal_device->work_vap_bitmap);
    hal_device_add_work_vap_bitmap_bcn_cfg(hal_device, hal_vap);

    oam_warning_log4(0, OAM_SF_CFG, "{hal_device_add_work_vap_bitmap::event[%d]add vap hal id[%d],"
        "mode[%d]to work_vap_bitmap[%d]}", event, hal_vap_id, hal_vap->vap_mode, hal_device->work_vap_bitmap);

    // 扫描中有其他VAP UP，不设置filter
    if (hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_SCAN_STATE) {
        /* sta + p2p_go 避免广播帧被过滤 */
        hal_device_state_work_set_rx_filter_reg(hal_device);
    }

    /* 发消息到device侧同步pm_bitmap */
    hal_dev_req_sync_pm_bitmap(hal_vap_id, HAL_DEVICE_WORK_SUB_STATE_ACTIVE);
}

/*****************************************************************************
 函 数 名  : hal_device_del_work_vap_bitmap
 功能描述  : work vap del bitmap
*****************************************************************************/
osal_void hal_device_del_work_vap_bitmap(hal_to_dmac_device_stru *hal_device, osal_u16 event, osal_void *event_data)
{
    hal_to_dmac_vap_stru *hal_vap = OAL_PTR_NULL;
    osal_u8 hal_vap_id;
    oal_bool_enum_uint8 vap_is_registered;

    if (event_data == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hal_device_del_work_vap_bitmap::event[%d],ptr null}", event);
        return;
    }

    hal_vap = (hal_to_dmac_vap_stru *)event_data;
    hal_vap_id = hal_vap->vap_id;

    vap_is_registered = ((hal_device->work_vap_bitmap & osal_bit(hal_vap_id)) != 0) ? OSAL_TRUE : OSAL_FALSE;
    if (vap_is_registered) {
        hal_vap->hal_vap_state = HAL_VAP_STATE_INIT;
        hal_device_sync_vap(hal_vap);
        hal_device->work_vap_bitmap &= (~(osal_u32)(osal_bit(hal_vap_id)));
        /* 将work_vap_bitmap同步到device侧 */
        hal_dev_sync_work_vap_bitmap(hal_device->work_vap_bitmap);

        oam_warning_log4(0, OAM_SF_CFG,
            "{hal_device_del_work_vap_bitmap::vap id[%d]hal id[%d],vap is registerd[%d],now work_vap_bitmap[%d]}",
            hal_vap->mac_vap_id, hal_vap_id, vap_is_registered, hal_device->work_vap_bitmap);
#ifdef _PRE_WLAN_FEATURE_M2S
        /* ap down或者sta去关联时，如果处于c1 siso，需要判断此时是否需要回mimo */
        get_hal_device_m2s_mgr(hal_device)->hal_device_m2s_mgr_cb.m2s_back_to_mimo_check(hal_device);
#endif
    }
}

/* 同步mac_pa_switch到device侧 */
osal_void hal_dev_sync_mac_pa_switch_to_dev(osal_u8 mac_pa_switch)
{
    frw_msg msg = {0};
    msg.data = &mac_pa_switch;
    msg.data_len = sizeof(osal_u8);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SYNC_MAC_PA_SWITCH, &msg, OSAL_TRUE);
}
/* 消息到device侧使能前端 */
osal_void hal_pm_enable_front_end_to_dev(hal_to_dmac_device_stru *hal_dev, osal_bool enable_paldo)
{
    frw_msg msg = {0};
    msg.data = &enable_paldo;
    msg.data_len = sizeof(osal_bool);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_ENABLE_FRONT_END, &msg, OSAL_TRUE);
}

osal_void hal_dev_sync_work_vap_bitmap(osal_u32 work_vap_bitmap)
{
    frw_msg msg = {0};
    msg.data = (osal_u8 *)&work_vap_bitmap;
    msg.data_len = sizeof(osal_u32);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SYNC_WORK_VAP_STATE, &msg, OSAL_TRUE);
}

/* 发消息通知device侧同步pm_bitmap */
osal_void hal_dev_req_sync_pm_bitmap(osal_u8 hal_vap_id, osal_u8 work_sub_state)
{
    frw_msg msg = {0};
    osal_u8 pm_bitmap[2] = {0};
    pm_bitmap[0] = hal_vap_id;
    pm_bitmap[1] = work_sub_state;

    msg.data = pm_bitmap;
    msg.data_len = sizeof(pm_bitmap);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REQ_SYNC_PMBIT, &msg, OSAL_TRUE);
}

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
功能描述  : 维测输出低功耗子状态机信息
*****************************************************************************/
osal_void hal_device_fsm_info(hal_to_dmac_device_stru *hal_device)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    osal_u8 cur_sub_state;

    hal_dev_fsm = &hal_device->hal_dev_fsm;
    cur_sub_state = hal_dev_fsm->sub_work_state;

    oam_warning_log4(0, OAM_SF_TX,
        "{hal_dev_fsm_info::mac_pa_switch_en[%d],wait_hw_txq_empty[0x%x],sub_fsm_stat[%d],fsm_stat[%d]}",
        hal_dev_fsm->mac_pa_switch_en, hal_dev_fsm->wait_hw_txq_empty,
        cur_sub_state, hal_dev_fsm->oal_fsm.cur_state);

    if (cur_sub_state <= HAL_DEVICE_WORK_SUB_STATE_DOZE) {
        oam_warning_log3(0, OAM_SF_TX,
            "{hal_dev_fsm_info::cur_sub_stat[%d],vap_pm_state_bitmap[0x%x],last_set_vap_id[0x%x]}", cur_sub_state,
            hal_dev_fsm->vap_pm_state_bitmap[cur_sub_state], hal_dev_fsm->pm_vap_id);
    }
}
#endif

osal_u8 hal_device_get_mac_pa_switch(osal_void)
{
    return g_mac_pa_switch;
}

osal_void hal_device_set_mac_pa_switch(osal_u8 val)
{
    g_mac_pa_switch = val;
}

/*****************************************************************************
 函 数 名  : hal_device_init_rx_dscr
 功能描述  : idle->work idle->scan 初始化接收描述符
*****************************************************************************/
osal_void hal_device_init_rx_dscr(hal_to_dmac_device_stru *hal_device)
{
    frw_msg msg = {0};

    /* 从idle状态跳转,idle->scan/work状态add接收描述符,pa already[on]->rx dscr not been destroyed */
    if (hal_device_state_is_busy(hal_device) == OSAL_TRUE) {
        return;
    }

    /* idle->scan/work状态add接收描述符 */
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_INIT_RX_DSCR, &msg, OSAL_TRUE);
}

osal_void hal_device_destroy_rx_dscr(hal_to_dmac_device_stru *hal_device)
{
    frw_msg msg = {0};

    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DESTROY_RX_DSCR, &msg, OSAL_TRUE);
    oam_warning_log1(0, OAM_SF_RX,
        "{hal_device_destroy_rx_dscr::dev state[%d],destroy rx dscr succ}", hal_device->hal_dev_fsm.oal_fsm.cur_state);
}

/*****************************************************************************
 函 数 名  : hal_device_idle_to_sleep_prepare
 功能描述  : idle状态入口检查是否满足条件关闭mac pa并投票睡眠
*****************************************************************************/
osal_void hal_device_idle_to_sleep_prepare(hal_to_dmac_device_stru *hal_device, hal_dev_fsm_stru *hal_dev_fsm)
{
    frw_msg msg = {0};
    unref_param(hal_dev_fsm);

    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_IDEL_SLEEP_PREPARE, &msg, OSAL_TRUE);
}

osal_u32 hal_device_idle_check_timeout_fn(osal_void *arg)
{
    hal_to_dmac_device_stru *hal_device = (hal_to_dmac_device_stru *)arg;
    hal_dev_fsm_stru hal_dev_fsm;

    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hal_device_idle_check_timeout_fn::hal_device is null}");
        return OSAL_FALSE;
    }

    hal_dev_fsm = hal_device->hal_dev_fsm;
    if (hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_IDLE_STATE) {
        frw_destroy_timer_entry(&(hal_dev_fsm.check_timer));
        oam_warning_log1(0, OAM_SF_PWR, "{hal_device_idle_check_timeout_fn :: hal_dev_fsm_current[%d]}",
            hal_dev_fsm.oal_fsm.cur_state);
        return OSAL_FALSE;
    }
    if (hal_device->work_vap_bitmap == 0) {
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_VAP_DOWN, 0, OSAL_NULL);
    }
    return OSAL_TRUE;
}

osal_void hal_device_handle_idle_vap_down(hal_to_dmac_device_stru *hal_device_base)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    hal_dev_fsm = &hal_device_base->hal_dev_fsm;

    if (hal_dev_fsm->check_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hal_dev_fsm->check_timer));
    }

    hal_device_idle_to_sleep_prepare(hal_device_base, hal_dev_fsm);
}

/*****************************************************************************
 函 数 名  : hal_device_idle_entry_ps_process
 功能描述  : idle状态入口检查是否满足条件关闭mac pa并投票睡眠。
*****************************************************************************/
osal_void hal_device_idle_entry_ps_process(hal_to_dmac_device_stru *hal_device)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    hal_dev_fsm = &hal_device->hal_dev_fsm;
    /* hal device 状态机 attach, 先不投票，大循环里投票 */
    if ((hal_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_IDLE_STATE) ||
        (hal_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_INIT_STATE)) {
        hal_dev_fsm->wait_hw_txq_empty = OSAL_TRUE;
        return;
    }
    if (hal_dev_fsm->check_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hal_dev_fsm->check_timer));
    }

    hal_device_idle_to_sleep_prepare(hal_device, hal_dev_fsm);
}

/*****************************************************************************
 函 数 名  : hal_device_state_init_entry
 功能描述  : init状态的entry接口
*****************************************************************************/
osal_void hal_device_state_init_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_state_init_entry::hal_device null.}");
        return;
    }
    hal_save_machw_phy_pa_status(hal_device);
    oam_warning_log2(0, OAM_SF_ANY,
    /* prev state[%d](0:Init,1:Idle,2:Work,3:Scan) */
        "{hal_device_state_init_entry::hal dev fsm in INIT, mac pa[%d](1:on,0:off), prev state[%d].}",
        hal_device->is_mac_pa_enabled, hal_dev_fsm->oal_fsm.cur_state);
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_state_init_exit
 功能描述  : init状态的exit接口
*****************************************************************************/
osal_void hal_device_state_init_exit(osal_void *ctx)
{
    unref_param(ctx);
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_event
 功能描述  : idle状态的event接口
*****************************************************************************/
osal_u32 hal_device_state_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    unref_param(event_data_len);
    unref_param(event_data);

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hal_device_state_init_event::hal_device null.}");
        return OAL_FAIL;
    }

    switch (event) {
        case HAL_DEVICE_EVENT_SYSTEM_INIT:
        case HAL_DEVICE_EVENT_SCAN_RESUME:
            oam_warning_log1(0, OAM_SF_ANY,
                "{hal_device_state_init_event::HAL_DEVICE_EVENT_SYSTEM_INIT. event[%d]}", event);
            hal_device_hw_init(hal_device);
            wifi_printf("====%s=====%d===%d=\r\n", __FUNCTION__, __LINE__, event);
            hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_IDLE_STATE, event);
            break;
        case HAL_DEVICE_EVENT_INIT_RX_DSCR:
        case HAL_DEVICE_EVENT_DESTROY_RX_DSCR:
            /*  辅路初始状态是init，唤醒时触发属于正常，不处理即可；主路增加报错维测，此处主路维测后续也可以删除 */
            oam_error_log1(0, OAM_SF_ANY,
                "{hal_device_state_init_event::wrong event1[%d]}", event);
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hal_device_state_init_event::wrong event2[%d]}", event);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_entry
 功能描述  : idle状态的entry接口
*****************************************************************************/
osal_void hal_device_state_idle_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_state_idle_entry::hal_device null.}");
        return;
    }
    hal_device_state_idle_set_rx_filter_reg(hal_device);

    hal_device_idle_entry_ps_process(hal_device);
    /* work/scan->idle才需要下面的操作,idle->idle可能存在rx dscr还没初始化(attach那次) */
    if (hal_device_state_is_busy(hal_device) == OSAL_TRUE) {
        hal_device_destroy_rx_dscr(hal_device);
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* wifi进入idle状态，通知bt此时wifi无业务 */
    hal_device_set_state_btcoex_notify(OSAL_FALSE);
#endif
    hal_save_machw_phy_pa_status(hal_device);
    oam_warning_log2(0, OAM_SF_ANY,
    /* prev state[%d](0:Init,1:Idle,2:Work,3:Scan) */
        "{hal_device_state_idle_entry::hal dev fsm in IDLE, mac pa[%d](1:on,0:off), prev state[%d].}",
        hal_device->is_mac_pa_enabled, hal_dev_fsm->oal_fsm.cur_state);
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_exit
 功能描述  : idle状态的exit接口
*****************************************************************************/
osal_void hal_device_state_idle_exit(osal_void *ctx)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_dev_fsm->wait_hw_txq_empty = OSAL_FALSE;
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_idle_to_scan_prepare
 功能描述  : idle下准备扫描
*****************************************************************************/
osal_void hal_device_idle_to_scan_prepare(hal_to_dmac_device_stru *hal_device, osal_void *event_data)
{
    hal_scan_params_stru *hal_scan_params = OAL_PTR_NULL;
    hal_scan_info_stru *hal_scan_info = OAL_PTR_NULL;

    if (event_data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hal_device_idle_to_scan_prepare::ptr NULL");
        return;
    }

    hal_scan_params = &(hal_device->hal_scan_params);
    hal_scan_info = (hal_scan_info_stru *)event_data;

    hal_scan_params_prepare(hal_device, hal_scan_info, hal_scan_params);

    oam_warning_log4(0, OAM_SF_SCAN,
        "{hal_device_idle_to_scan_prepare::scan start idx[%d],scan channel nums[%d],scan time each chan[%d],mode[%d]}",
        hal_scan_params->start_chan_idx, hal_scan_params->channel_nums, hal_scan_params->scan_time,
        hal_scan_params->scan_mode);
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_event
 功能描述  : idle状态的event接口
*****************************************************************************/
osal_u32 hal_device_state_idle_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    unref_param(event_data_len);

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hal_device_state_idle_event::hal_device null.}");
        return OAL_FAIL;
    }

    switch (event) {
        case HAL_DEVICE_EVENT_SCAN_BEGIN:
            hal_device_idle_to_scan_prepare(hal_device, event_data);
            break;

        case HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF:
            hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_SCAN_STATE, event);
            break;

        /* 存在切回home channel,work后, scan abort */
        case HAL_DEVICE_EVENT_SCAN_ABORT:
            hal_device_handle_scan_abort(hal_device);
            break;

        case HAL_DEVICE_EVENT_VAP_UP:
        case HAL_DEVICE_EVENT_JOIN_COMP:
            hal_device_add_work_vap_bitmap(hal_device, event, event_data);
            hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_WORK_STATE, event);
            break;

        /* idle下也会有(init->wait_start)的事件此时不处理 */
        case HAL_DEVICE_EVENT_VAP_DOWN:
            hal_device_handle_idle_vap_down(hal_device);
            break;
        case HAL_DEVICE_EVENT_INIT_RX_DSCR:
        case HAL_DEVICE_EVENT_DESTROY_RX_DSCR:
        case HAL_DEVICE_EVENT_SCAN_RESUME:
            break;

        case HAL_DEVICE_EVENT_SCAN_PAUSE_FROM_CHAN_CONFLICT:
            hal_device_handle_scan_pause(hal_device, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT);
            break;

        case HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT:
            hal_device_handle_scan_resume(hal_device, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT);
            break;
#ifdef BOARD_ASIC_WIFI
        /* 辅路device下电操作 */
        case HAL_DEVICE_EVENT_DBDC_STOP:
        case HAL_DEVICE_EVENT_SCAN_END:
            break;
#endif
        case HAL_DEVICE_EVENT_SYSTEM_INIT:
            oam_warning_log1(0, OAM_SF_CFG, "{hal_device_state_idle_event::slave init event[%d].}", event);
            break;

        default:
            oam_error_log1(0, OAM_SF_CFG, "{hal_device_state_idle_event::idle state wrong event[%d]}", event);
            break;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_entry
 功能描述  : idle状态的entry接口
*****************************************************************************/
osal_void hal_device_state_work_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_state_work_entry::hal_device null.}");
        return;
    }

    /* work entry必须重新设置帧过滤!!!否则从扫描状态调回work的时候就未更新帧过滤的值 */
    hal_device_state_work_set_rx_filter_reg(hal_device);

    hal_device_init_rx_dscr(hal_device);
    hal_save_machw_phy_pa_status(hal_device);
    oam_warning_log2(0, OAM_SF_ANY,
        "{hal_device_state_work_entry:HalDev fsm inWORK,macPa=%d(1:on,0:off),prevState=%d(0:Init,1:Idle,2:Work,3:Scan}",
        hal_device->is_mac_pa_enabled, hal_dev_fsm->oal_fsm.cur_state);
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_exit
 功能描述  : idle状态的exit接口
*****************************************************************************/
osal_void hal_device_state_work_exit(osal_void *ctx)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_dev_fsm->wait_hw_txq_empty = OSAL_FALSE;
    return;
}

/*****************************************************************************
 函 数 名  : hal_device_work_to_scan_prepare
 功能描述  : work下准备扫描
*****************************************************************************/
osal_void hal_device_work_to_scan_prepare(hal_to_dmac_device_stru *hal_device, osal_void *event_data)
{
    hal_scan_params_stru *hal_scan_params = OSAL_NULL;
    hal_scan_info_stru *hal_scan_info = OSAL_NULL;

    hal_scan_params = &(hal_device->hal_scan_params);
    hal_scan_info = (hal_scan_info_stru *)event_data;

    hal_scan_params_prepare(hal_device, hal_scan_info, hal_scan_params);

    /* 这两种扫描在host判定为不需要回home channel,如果hal device状态机是work状态,一定有异常 */
    if ((hal_scan_info->scan_mode == WLAN_SCAN_MODE_FOREGROUND) ||
        (hal_scan_info->scan_mode == WLAN_SCAN_MODE_BACKGROUND_PNO)) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "{hal_device_work_to_scan_prepare::hal device workbitmap[%d]is work state wrong!!!scan mode[%d]}",
            hal_device->work_vap_bitmap, hal_scan_info->scan_mode);
        return;
    }

    /* 只要是work下发起的扫描一定要回home channel!!! */
    hal_scan_params->need_switch_back_home_channel = OSAL_TRUE;

    /* work下扫描才需要下面两个参数 */
    hal_scan_params->work_time_on_home_channel = hal_scan_info->work_time_on_home_channel;
    hal_scan_params->scan_channel_interval = hal_scan_info->scan_channel_interval;

    oam_warning_log4(0, OAM_SF_SCAN,
        "{hal_device_work_to_scan_prepare::scan start idx[%d],scan channel nums[%d],scan time each chan[%d], mode[%d]}",
        hal_scan_params->start_chan_idx, hal_scan_params->channel_nums, hal_scan_params->scan_time,
        hal_scan_params->scan_mode);
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_event
 功能描述  : idle状态的event接口
*****************************************************************************/
osal_u32 hal_device_state_work_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    unref_param(event_data_len);

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hal_device_state_work_event::hal_device null.}");
        return OAL_FAIL;
    }

    switch (event) {
        case HAL_DEVICE_EVENT_SCAN_BEGIN:
            hal_device_work_to_scan_prepare(hal_device, event_data);
            break;

        /* 存在切回home channel,work后, scan abort */
        case HAL_DEVICE_EVENT_SCAN_ABORT:
            hal_device_handle_scan_abort(hal_device);
            break;

        case HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF:
            hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_SCAN_STATE, event);
            break;

        case HAL_DEVICE_EVENT_VAP_UP:
        case HAL_DEVICE_EVENT_JOIN_COMP:
            hal_device_add_work_vap_bitmap(hal_device, event, event_data);
            break;
        case HAL_DEVICE_EVENT_VAP_DOWN:
            hal_device_del_work_vap_bitmap(hal_device, event, event_data);
            if (hal_device->work_vap_bitmap == 0) {
                hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_IDLE_STATE, event);
            }
            break;
        case HAL_DEVICE_EVENT_INIT_RX_DSCR:
            break;

        case HAL_DEVICE_EVENT_DESTROY_RX_DSCR:
            break;

        default:
            oam_error_log1(0, OAM_SF_CFG, "{hal_device_state_work_event::work state wrong event[%d]}", event);
#ifdef _PRE_WLAN_DFT_STAT
            hal_device_fsm_info(hal_device);
#endif
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_entry
 功能描述  : idle状态的entry接口
*****************************************************************************/
osal_void hal_device_state_scan_entry(osal_void *ctx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_state_scan_entry::hal_device null.}");
        return;
    }

    hal_every_channel_scan_state_clean(&(hal_device->hal_scan_params));
    hal_pm_vote2platform(HAL_PM_WORK);
    hal_device_init_rx_dscr(hal_device);
    hal_pm_enable_front_end_to_dev(hal_device, OSAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 通知bt此时wifi有业务 */
    hal_device_set_state_btcoex_notify(OSAL_TRUE);
#endif

    hal_device_state_scan_set_rx_filter_reg(hal_device);
    hal_save_machw_phy_pa_status(hal_device);
    oam_warning_log2(0, OAM_SF_ANY,
        "{hal_device_state_scan_entry::hal dev fsm in SCAN, mac pa[%d](1:on,0:off), prev state[%d]"
        "(0:Init,1:Idle,2:Work,3:Scan).}", hal_device->is_mac_pa_enabled, hal_dev_fsm->oal_fsm.cur_state);

    return;
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_exit
 功能描述  : idle状态的exit接口
*****************************************************************************/
osal_void hal_device_state_scan_exit(osal_void *ctx)
{
    osal_u16 last_hal_fsm_event;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)(hal_dev_fsm->oal_fsm.p_oshandler);
    if (hal_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_state_scan_exit::hal_device null.}");
        return;
    }

    last_hal_fsm_event = hal_dev_fsm->oal_fsm.last_event;
    if ((last_hal_fsm_event == HAL_DEVICE_EVENT_SCAN_END) || (last_hal_fsm_event == HAL_DEVICE_EVENT_SCAN_ABORT)) {
        hal_device->hal_scan_params.curr_scan_state = MAC_SCAN_STATE_IDLE;
    } else if (last_hal_fsm_event != HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hal_device_state_scan_exit::wrong event[%d],exit scan state}", last_hal_fsm_event);
    }
}

/*****************************************************************************
 函 数 名  : hal_device_state_idle_event
 功能描述  : idle状态的event接口
*****************************************************************************/
osal_u32 hal_device_state_scan_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    hal_scan_params_stru *hal_scan_params = OAL_PTR_NULL;
    hal_device_state_info new_dev_state;
    unref_param(event_data_len);

    hal_dev_fsm = (hal_dev_fsm_stru *)ctx;
    hal_device = (hal_to_dmac_device_stru *)hal_dev_fsm->oal_fsm.p_oshandler;
    if (hal_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hal_device_state_work_event::hal_device null.}");
        return OAL_FAIL;
    }

    hal_scan_params = &(hal_device->hal_scan_params);

    switch (event) {
        case HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF:
            hal_every_channel_scan_state_clean(hal_scan_params);
            break;

        case HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK:
        case HAL_DEVICE_EVENT_SCAN_END:
            hal_scan_handle_scan_back(hal_device, event);
            break;

        case HAL_DEVICE_EVENT_SCAN_ABORT:
            hal_device_handle_scan_abort(hal_device);
            new_dev_state = hal_device_get_state_from_bitmap(hal_device);
            hal_device_fsm_trans_to_state(hal_dev_fsm, new_dev_state, event);
            break;

        case HAL_DEVICE_EVENT_SCAN_PAUSE:
            hal_device_handle_scan_pause(hal_device, HAL_SCAN_PASUE_TYPE_SWITCH_BACK);
            break;

        case HAL_DEVICE_EVENT_SCAN_RESUME:
            hal_device_handle_scan_resume(hal_device, HAL_SCAN_PASUE_TYPE_SWITCH_BACK);
            break;

        case HAL_DEVICE_EVENT_SCAN_PAUSE_FROM_CHAN_CONFLICT:
            hal_device_handle_scan_pause(hal_device, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT);
            break;

        case HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT:
            hal_device_handle_scan_resume(hal_device, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT);
            break;
        /* 存在一个vap在扫描,另一个vap down的场景,此vap扫描结束后,看bitmap是否需要再回home channel */
        case HAL_DEVICE_EVENT_VAP_DOWN:
            oam_warning_log0(0, OAM_SF_CFG,
                "{hal_device_state_scan_event::in scan state vap down}");
            hal_device_del_work_vap_bitmap(hal_device, event, event_data);
            if (hal_device->work_vap_bitmap == 0) {
                hal_scan_params->need_switch_back_home_channel = OSAL_FALSE;
            }
            break;
        case HAL_DEVICE_EVENT_VAP_UP:
            hal_device_add_work_vap_bitmap(hal_device, event, event_data);
            break;

        default:
            oam_error_log1(0, OAM_SF_CFG,
                "{hal_device_state_scan_event::scan state wrong event[%d]}", event);
            break;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_device_fsm_trans_to_state
 功能描述  : hal devive状态机状态切换函数
*****************************************************************************/
osal_u32 hal_device_fsm_trans_to_state(hal_dev_fsm_stru *hal_dev_fsm, osal_u8 state, osal_u16 event)
{
    osal_u32 ret;
    oal_fsm_stru *oal_fsm = &(hal_dev_fsm->oal_fsm);
    unref_param(event);

    ret = oal_fsm_trans_to_state(oal_fsm, state);
    if (ret == OAL_SUCC) {
        frw_msg msg = {0};
        osal_u8 fsm_state[2] = {0}; /* 2个状态，0:curr state; 1:sub work state */
        osal_s32 msg_ret;
        fsm_state[0] = oal_fsm->cur_state;
        msg.data = fsm_state;
        msg.data_len = sizeof(fsm_state);
        msg_ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SYNC_HAL_DEV_FSM_STATE, &msg, OSAL_TRUE);
        if (msg_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "hal_device_fsm_trans_to_state:send fsm sta to device fail[%d]", msg_ret);
        }
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_pm_sta_attach
 功能描述  : sta类型VAP创建pm handler
*****************************************************************************/
osal_void hal_device_fsm_attach(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 fsm_name[5] = {0}; /* 数组大小5 */
    osal_u32 ret;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;
    oal_fsm_create_stru fsm_create_stru;

    if (hal_device->hal_dev_fsm.is_fsm_attached == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "hal_device_fsm_attach::hal device fsm have attached");
        return;
    }

    hal_dev_fsm = &(hal_device->hal_dev_fsm);

    (osal_void)memset_s(hal_dev_fsm, sizeof(hal_dev_fsm_stru), 0, sizeof(hal_dev_fsm_stru));
    hal_dev_fsm->wait_hw_txq_empty = OSAL_FALSE;
    hal_dev_fsm->mac_pa_switch_en = hal_device_get_mac_pa_switch();

    hal_dev_sync_mac_pa_switch_to_dev(hal_dev_fsm->mac_pa_switch_en);

    fsm_create_stru.oshandle = (osal_void *)hal_device;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = hal_dev_fsm;

    ret = oal_fsm_create(&fsm_create_stru, &(hal_dev_fsm->oal_fsm), HAL_DEVICE_INIT_STATE,
        g_hal_device_fsm_info, sizeof(g_hal_device_fsm_info) / sizeof(oal_fsm_state_info));
    /* oal fsm create succ */
    if (ret == OAL_SUCC) {
        hal_dev_fsm->is_fsm_attached = OSAL_TRUE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_pm_sta_deattach
 功能描述  : sta类型VAP销毁pm handler
*****************************************************************************/
osal_void hal_device_fsm_detach(hal_to_dmac_device_stru *hal_device)
{
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    hal_dev_fsm = &(hal_device->hal_dev_fsm);

    if (hal_dev_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_CFG, "hal_device_fsm_attach::hal device fsm not attatched");
        return;
    }

    /* 不是IDLE状态切换到IDLE状态 */
    if (hal_device_state_is_busy(hal_device) == OSAL_TRUE) {
        hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_IDLE_STATE, HAL_DEVICE_EVENT_DETATCH);
    }

    hal_device->hal_dev_fsm.is_fsm_attached = OSAL_FALSE;

    return;
}

/*****************************************************************************
 函 数 名  : hal_device_post_event
 功能描述  : hal devive状态机的事件处理接口
*****************************************************************************/
osal_void hal_device_handle_event(hal_to_dmac_device_stru *hal_device, osal_u16 type, osal_u16 datalen, osal_u8 *data)
{
    osal_u32 ret;
    hal_dev_fsm_stru *hal_dev_fsm = OSAL_NULL;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hal_device_handle_event::ptr NULL");
        return;
    }

    hal_dev_fsm = &(hal_device->hal_dev_fsm);
    if (hal_dev_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_ANY, "hal_device_handle_event::hal device fsm not attached");
        return;
    }

    ret = oal_fsm_event_dispatch(&(hal_dev_fsm->oal_fsm), type, datalen, data);
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_ANY, "hal_device_handle_event::state[%d]dispatch event[%d]not succ[%d]",
            hal_device->hal_dev_fsm.oal_fsm.cur_state, type, ret);
    }
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
