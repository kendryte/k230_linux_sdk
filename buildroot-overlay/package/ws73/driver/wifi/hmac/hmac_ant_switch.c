/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac ant switch
 * Create: 2022-5-10
 */

#include "hmac_ant_switch.h"
#include "dmac_ext_if_device.h"
#include "mac_resource_ext.h"
#include "hal_common_ops_device.h"
#include "hmac_user.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ANT_SWITCH_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
/*****************************************************************************
 函 数 名  : hmac_calc_rssi_th
 功能描述  : 更新初始rssi初始最小值门限，差值门限。当协议模式小于HT模式，则不进行数据帧天线切换。
*****************************************************************************/
osal_void hmac_calc_rssi_th(const hmac_user_stru *hmac_user, hal_rssi_stru *hal_rssi)
{
    if (hmac_user->avail_protocol_mode == WLAN_HT_MODE || hmac_user->avail_protocol_mode == WLAN_HT_ONLY_MODE ||
        hmac_user->avail_protocol_mode == WLAN_HT_11G_MODE) {
        hal_rssi->ori_min_th = HAL_ANT_SWITCH_RSSI_HT_MIN_TH;
        hal_rssi->diff_th = HAL_ANT_SWITCH_RSSI_HT_DIFF_TH;
    } else if (hmac_user->avail_protocol_mode == WLAN_VHT_MODE ||
               hmac_user->avail_protocol_mode == WLAN_VHT_ONLY_MODE) {
        hal_rssi->ori_min_th = HAL_ANT_SWITCH_RSSI_VHT_MIN_TH;
        hal_rssi->diff_th = HAL_ANT_SWITCH_RSSI_VHT_DIFF_TH;
    }
}

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : hmac_ant_ch_sel_by_rssi
 功能描述  : 切换至SISO时，选择通路
*****************************************************************************/
static osal_u8 hmac_ant_ch_sel_by_rssi(const hal_rx_ant_rssi_stru *rx_ant_rssi)
{
    osal_u8 chan = 0xFF;
    osal_char ant0_rssi;
    osal_char ant1_rssi;

    /* 某路RSSI未更新 */
    if (rx_ant_rssi->ant0_rssi_smth == OAL_RSSI_INIT_MARKER || rx_ant_rssi->ant1_rssi_smth == OAL_RSSI_INIT_MARKER) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_ant_ch_sel_by_rssi::ant0[%d],ant1[%d]",
                       (osal_s32)rx_ant_rssi->ant0_rssi_smth, (osal_s32)rx_ant_rssi->ant1_rssi_smth);
        return chan;
    }

    ant0_rssi = oal_get_real_rssi(rx_ant_rssi->ant0_rssi_smth);
    ant1_rssi = oal_get_real_rssi(rx_ant_rssi->ant1_rssi_smth);
    /* ANT0 上报的rssi正常,且大于ANT1, 则开启通道0 */
    if (ant0_rssi > ant1_rssi) {
        chan = WLAN_PHY_CHAIN_ZERO;
    } else {
        chan = WLAN_PHY_CHAIN_ONE;
    }

    return chan;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_ant_rssi_proc
 功能描述  : 双天线RSSI处理逻辑
*****************************************************************************/
static hal_m2s_state_uint8 hmac_ant_rssi_proc(hal_to_dmac_device_stru *hal_dev, const dmac_rx_ctl_stru *rx_cb)
{
    osal_char ant0_rssi, ant1_rssi, min_rssi;
    osal_u8 rssi_abs;
    hal_m2s_state_uint8 cur_m2s_state = hal_dev->hal_m2s_fsm.oal_fsm.cur_state;
    hal_rx_ant_rssi_stru *rx_ant_rssi = &hal_dev->rssi.rx_rssi;

    oal_rssi_smooth(&rx_ant_rssi->ant0_rssi_smth, rx_cb->rx_statistic.ant0_rssi);
    oal_rssi_smooth(&rx_ant_rssi->ant1_rssi_smth, rx_cb->rx_statistic.ant1_rssi);
    ant0_rssi = (osal_char)oal_get_real_rssi(rx_ant_rssi->ant0_rssi_smth);
    ant1_rssi = (osal_char)oal_get_real_rssi(rx_ant_rssi->ant1_rssi_smth);
    rssi_abs = (osal_u8)oal_absolute_sub(ant0_rssi, ant1_rssi);
    min_rssi = osal_min(ant0_rssi, ant1_rssi);

    if (rx_ant_rssi->log_print == OSAL_TRUE) {
        oam_error_log3(0, OAM_SF_CFG, "hmac_ant_rssi_proc::c_rssi_abs[%d],ant0[%d],ant1[%d]", rssi_abs,
                       (osal_s32)ant0_rssi, (osal_s32)ant1_rssi);
    }

    /* MIMO状态判断是否需要切换至MISO */
    if (cur_m2s_state == HAL_M2S_STATE_MIMO) {
        /* 两天线中较低的RSSI值没到最低切换门限，不切换，且保留MIMO差值计数uc_rssi_high_cnt */
        if (min_rssi > hal_dev->rssi.cur_min_th) {
            return cur_m2s_state;
        }

        /* 对于双天线RSSI差值小于最小差值门限，不切换，且清空MIMO差值计数uc_rssi_high_cnt */
        if (rssi_abs <= hal_dev->rssi.diff_th) {
            rx_ant_rssi->rssi_high_cnt = 0;
            return cur_m2s_state;
        }

        /* 连续多次rssi超过阈值则切换MISO */
        rx_ant_rssi->rssi_high_cnt++;
        if (rx_ant_rssi->rssi_high_cnt > rx_ant_rssi->rssi_high_cnt_th) {
            rx_ant_rssi->rssi_high_cnt = 0;
            return HAL_M2S_STATE_MISO;
        }
    } else { /* 仅由RSSI触发的MISO探测态,如果阈值达到恢复MIMO条件则切换至MIMO */
        /* 信号较差的那根天线的RSSI小于当前最小门限加3db，且RSSI差值大于差值门限减5，则不切换 */
        if ((min_rssi < hal_dev->rssi.cur_min_th + 3) && (rssi_abs >= hal_dev->rssi.diff_th - 5)) {
            rx_ant_rssi->rssi_low_cnt = 0;
            hal_dev->rssi.miso_hold = OSAL_TRUE;
            return cur_m2s_state;
        }

        /* 连续多次rssi差值小于阈值则恢复MIMO */
        rx_ant_rssi->rssi_low_cnt++;
        if (rx_ant_rssi->rssi_low_cnt > rx_ant_rssi->rssi_low_cnt_th) {
            rx_ant_rssi->rssi_low_cnt = 0;
            return HAL_M2S_STATE_MIMO;
        }
    }

    /* 维持原状继续探测 */
    return cur_m2s_state;
}

/*****************************************************************************
 函 数 名  : hmac_rssi_event
 功能描述  : 接收功率触发状态切换
*****************************************************************************/
static osal_void hmac_rssi_event(hal_to_dmac_device_stru *hal_dev, hal_m2s_state_uint8 new_state)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    osal_u8 chan;
    hal_m2s_event_tpye_uint16 event_type;
#endif

    /* 接收功率决策满足恢复至MIMO状态 */
    if (new_state == HAL_M2S_STATE_MIMO) {
        /* 保持,暂时不探测 */
        hal_dev->rssi.mimo_hold = OSAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_M2S
        hmac_m2s_handle_event(hal_dev, HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO, 0, OSAL_NULL);
#endif
        return;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 确认切换到C0还是C1 */
    chan = hmac_ant_ch_sel_by_rssi(&(hal_dev->rssi.rx_rssi));

    if (hal_dev->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MIMO) {
        if (chan == WLAN_PHY_CHAIN_ZERO) {
            event_type = HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0;
        } else {
            event_type = HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1;
        }
    } else {
        /* 校验是否需要切换到另一个MISO状态,若相同则不切换 */
        if (chan == hal_dev->cfg_cap_info->single_tx_chain) {
            return;
        }

        if (chan == WLAN_PHY_CHAIN_ZERO) {
            event_type = HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0;
        } else {
            event_type = HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1;
        }
    }
    hmac_m2s_handle_event(hal_dev, event_type, 0, OSAL_NULL);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_ant_rssi_notify
 功能描述  : 双天线场景下, MIMO/SISO切换判决入口函数
*****************************************************************************/
static osal_void hmac_ant_rssi_notify(hal_to_dmac_device_stru *hal_dev)
{
    hal_m2s_state_uint8 next_m2s_state;
    dmac_rx_ctl_stru *rx_cb;

    rx_cb = (dmac_rx_ctl_stru *)hal_dev->rssi.cb;
    next_m2s_state = hmac_ant_rssi_proc(hal_dev, rx_cb);
    /* 若处于MIMO且探测后状态仍为MIMO，则不切换。若当前是MISO状态，无论探测结果如何都需要校验一次是否切换到另一种MISO状态。 */
    if (hal_dev->hal_m2s_fsm.oal_fsm.cur_state == next_m2s_state && next_m2s_state == HAL_M2S_STATE_MIMO) {
        return;
    }

    hmac_rssi_event(hal_dev, next_m2s_state);
}

/*****************************************************************************
 函 数 名  : hmac_ant_rx_frame
 功能描述  : MIMO/MISO场景下, 接收数据帧入口
*****************************************************************************/
osal_void hmac_ant_rx_frame(hal_to_dmac_device_stru *hal_dev, const hmac_vap_stru *hmac_vap)
{
    hal_m2s_state_uint8 cur_m2s_state;
    hmac_user_stru *hmac_user;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        return;
    }

    /* 对方协议模式小于HT时，不进行数据帧切换 */
    if (hmac_user->avail_protocol_mode < WLAN_HT_MODE) {
        return;
    }

    /* 数据帧切换没有使能则退出 */
    if ((HAL_ANT_SWITCH_RSSI_DATA_ENABLE & hal_dev->rssi.rx_rssi.ant_rssi_sw) == 0) {
        return;
    }

    /* 在MIMO HOLD或MISO HOLD状态不进入探测 */
    if (hal_dev->rssi.mimo_hold == OSAL_TRUE || hal_dev->rssi.miso_hold == OSAL_TRUE) {
        return;
    }

    cur_m2s_state = hal_dev->hal_m2s_fsm.oal_fsm.cur_state;
    /* 只有MIMO和仅由RSSI触发的MISO才可能进入探测态 */
    if ((cur_m2s_state == HAL_M2S_STATE_MIMO) ||
        (cur_m2s_state == HAL_M2S_STATE_MISO && get_hal_m2s_mode_tpye(hal_dev) == WLAN_M2S_TRIGGER_MODE_RSSI)) {
        /* 更新当前最小值门限。20M带宽对应初始最小值门限，40,80,160逐级+3db */
        hal_dev->rssi.cur_min_th = (osal_char)(hal_dev->rssi.ori_min_th + hmac_user->avail_bandwidth * 3);

        hmac_ant_rssi_notify(hal_dev);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ant_tbtt_notify
 功能描述  : tbtt中断天线切换处理
*****************************************************************************/
static osal_void hmac_ant_tbtt_notify(hal_to_dmac_device_stru *hal_dev)
{
    hal_m2s_state_uint8 cur_m2s_state;

    cur_m2s_state = hal_dev->hal_m2s_fsm.oal_fsm.cur_state;
    if (cur_m2s_state == HAL_M2S_STATE_MIMO) {
        /* M2S优先级未定，其他业务结束时不会判断是否存在RSSI状态。故可能在MIMO状态时，RSSI事件仍然存在。待优先级确定后修改 */
        get_hal_m2s_mode_tpye(hal_dev) &= (~WLAN_M2S_TRIGGER_MODE_RSSI);

        hal_dev->rssi.mimo_tbtt_cnt++;

        if (hal_dev->rssi.mimo_tbtt_cnt > hal_dev->rssi.mimo_tbtt_open_th + hal_dev->rssi.mimo_tbtt_close_th) {
            hal_dev->rssi.mimo_tbtt_cnt = 0;

            /* 每隔一定周期MIMO状态停止探测 */
            hal_dev->rssi.mimo_hold = OSAL_TRUE;
        }

        if (hal_dev->rssi.mimo_tbtt_cnt > hal_dev->rssi.mimo_tbtt_open_th) {
            /* 每隔一定周期MIMO状态开启探测 */
            hal_dev->rssi.mimo_hold = OSAL_FALSE;
        }
    } else if ((cur_m2s_state == HAL_M2S_STATE_MISO) &&
               (get_hal_m2s_mode_tpye(hal_dev) == WLAN_M2S_TRIGGER_MODE_RSSI)) { /* 仅由RSSI自己触发的MISO状态 */
        hal_dev->rssi.tbtt_cnt++;
        if (hal_dev->rssi.tbtt_cnt > hal_dev->rssi.tbtt_cnt_th) {
            hal_dev->rssi.tbtt_cnt = 0;
            hal_dev->rssi.miso_hold = OSAL_FALSE;
        }
    }
}

/*****************************************************************************
 功能描述  : SISO场景下, TBTT事件触发进入MIMO状态探测
*****************************************************************************/
osal_void hmac_ant_tbtt_process(hal_to_dmac_device_stru *hal_dev)
{
    /* 没有使能则退出 */
    if ((HAL_ANT_SWITCH_RSSI_DATA_ENABLE & hal_dev->rssi.rx_rssi.ant_rssi_sw) == 0) {
        return;
    }

    hmac_ant_tbtt_notify(hal_dev);
}
#endif
