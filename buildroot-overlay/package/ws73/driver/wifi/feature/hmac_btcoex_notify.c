/*
 * Copyright: Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 状态通知、去/关联流程处理
 * Date: 2023-02-01 15:33
 */
#ifndef __HMAC_BTCOEX_NOTIFY_C__
#define __HMAC_BTCOEX_NOTIFY_C__

#include "hmac_mgmt_sta.h"
#include "hmac_scan.h"
#include "oal_netbuf_data.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ba.h"
#include "hmac_btcoex_m2s.h"
#include "hmac_btcoex_ps.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex_notify.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_NOTIFY_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/* WIFI状态通知处理 */
/*****************************************************************************
 函 数 名  : hmac_btcoex_set_wifi_status_irq_notify
 功能描述  : 软件设定共天线通知位: bit8 主路c1 siso  bit9 DBDC(辅路c1 siso)
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_set_wifi_status_irq_notify(osal_u8 id, osal_u8 val)
{
    hal_set_btcoex_wifi_status_notify(id, val);

    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
}

OSAL_STATIC osal_u16 hmac_btcoex_chan_to_bitmap(osal_u8 chan_idx, wlan_channel_bandwidth_enum_uint8 band_width)
{
    osal_u8 left = 0;
    osal_u16 mask = 0, ch_bitmap = 0;

    if (chan_idx >= MAC_CHANNEL_FREQ_2_BUTT) {
        return ch_bitmap;
    }

    switch (band_width) {
        case WLAN_BAND_WIDTH_20M:
        case WLAN_BAND_WIDTH_5M: /* 5M,10M考虑到beacon 按照占用20M处理 */
        case WLAN_BAND_WIDTH_10M:
            left  = BTCOEX_20M_40PLUS_CHAN_OFFSET;
            mask  = BTCOEX_20M_CHAN_BITMASK;
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            left  = BTCOEX_20M_40PLUS_CHAN_OFFSET;
            mask  = BTCOEX_40M_CHAN_BITMASK;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
            left  = BTCOEX_40MINUS_CHAN_OFFSET;
            mask  = BTCOEX_40M_CHAN_BITMASK;
            break;

        default:
            return ch_bitmap;
    }

    left = (chan_idx >= left) ? (chan_idx - left) : (left - chan_idx);
    ch_bitmap = (mask << left) & BTCOEX_2G_ALL_CHAN_BITMASK;
    return ch_bitmap;
}

OSAL_STATIC osal_void hmac_btcoex_update_ps_capability(hal_to_dmac_device_stru *hal_device,
    hmac_btcoex_ps_stru *ps_param)
{
    oal_bool_enum_uint8 sco_status = OSAL_FALSE;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 5.3.电话场景 或 dbac场景 或 c1 siso场景 */
    hal_btcoex_get_bt_sco_status(hal_device, &sco_status);
    if ((sco_status == OSAL_TRUE) || (mac_is_dbac_running(hmac_device) == OSAL_TRUE) ||
        (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE)) {
        ps_param->ps_stop = OSAL_TRUE;
    }

    if (hmac_device->vap_num == 0) {
        hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_ON, OAL_FALSE);
    }

    if (hmac_device_calc_up_vap_num_etc(hmac_device) == 0) {
        /* 状态上报BT */
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_update_ps_capability::Notify BT cancel AFH.}");
    }

    /* 刷新ps能力 */
    hal_device->btcoex_sw_preempt.ps_stop = ps_param->ps_stop;
    hmac_btcoex_set_ps_flag(hal_device);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_BAND, ps_param->band);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_CONN, ps_param->legacy_connect_state);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_P2P_CONN, ps_param->p2p_connect_state);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_CHAN_NUM, ps_param->channel_num);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_BAND_WIDTH, ps_param->bandwidth);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_CHAN_BITMAP, ps_param->chan_bitmap);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_PS_STOP, ps_param->ps_stop);

    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_WORK_MODE, ps_param->work_mode);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_DIFF_CHANNEL, ps_param->diff_chan);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11AX, ps_param->protocol_11ax);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11BGN, ps_param->protocol_11bgn);

    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);

    oam_warning_log_alter(0, OAM_SF_COEX,
        "{hmac_btcoex_update_ps_capability::band[%d]legacy[%d]p2p[%d]chan_idx[%d]bandwidth[%d] !}",
        // 5表示5后面含5个待打印参数
        5, ps_param->band, ps_param->legacy_connect_state, ps_param->p2p_connect_state, ps_param->channel_num,
        ps_param->bandwidth);
}

static osal_void hmac_btcoex_update_vap_work_status(hmac_vap_stru *hmac_vap, hmac_btcoex_ps_stru *ps_param)
{
    osal_u16 chan_bitmap;

    /* 1. legacy vap connect状态判断 */
    if ((is_legacy_sta(hmac_vap) || is_legacy_ap(hmac_vap)) && (hmac_vap->user_nums != 0)) {
        ps_param->legacy_connect_state = OSAL_TRUE;
    } else if ((is_p2p_cl(hmac_vap) || is_p2p_go(hmac_vap)) && (hmac_vap->user_nums != 0)) {
        /* 1.1. p2p vap connect状态判断 */
        ps_param->p2p_connect_state = OSAL_TRUE;
    }

    if (hmac_vap->user_nums != 0) {
        /* AP/STA/GO/GC mode */
        if (is_legacy_sta(hmac_vap)) {
            ps_param->work_mode |= BTCOEX_WORK_MODE_BIT_STA;
        } else if (is_legacy_ap(hmac_vap)) {
            ps_param->work_mode |= BTCOEX_WORK_MODE_BIT_AP;
        } else if (is_p2p_cl(hmac_vap)) {
            ps_param->work_mode |= BTCOEX_WORK_MODE_BIT_GC;
        } else if (is_p2p_go(hmac_vap)) {
            ps_param->work_mode |= BTCOEX_WORK_MODE_BIT_GO;
        } else {
            /* nothing */
        }

        /* protocol mode */
        if (hmac_vap->protocol <= WLAN_HT_11G_MODE) {
            ps_param->protocol_11bgn = OSAL_TRUE;
        } else {
            ps_param->protocol_11ax = OSAL_TRUE;
        }
    }

    /* 2.考虑2g下 */
    if (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) {
        ps_param->band = WLAN_BAND_2G; /* 存在2G设备，那就一直通知状态是2G频段，数传退让要生效 */

        /* 3.判断带宽 */
        ps_param->bandwidth = hmac_vap->channel.en_bandwidth;

        /* 3.信道 */
        ps_param->channel_num = hmac_vap->channel.chan_number;

        chan_bitmap = hmac_btcoex_chan_to_bitmap(hmac_vap->channel.chan_idx, hmac_vap->channel.en_bandwidth);
        ps_param->chan_bitmap |= chan_bitmap;
        if (chan_bitmap != ps_param->chan_bitmap) {
            ps_param->diff_chan = OSAL_TRUE;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_freq_and_work_state_hal_device
 功能描述  : 配置当前工作频段和connect状态，通知bt用于数传业务，
             频段和connect状态放到一起处理，在用户关联和去关联接口即可
*****************************************************************************/
osal_void hmac_btcoex_set_freq_and_work_state_hal_device(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    osal_u8 up_ap_num = 0;
    osal_u8 up_sta_num = 0;
    osal_u8 up_2g_num = 0;
    osal_u8 up_5g_num = 0;
    osal_u8 up_vap_num, vap_index;
    hmac_btcoex_ps_stru ps_param;

    (osal_void)memset_s(&ps_param, sizeof(hmac_btcoex_ps_stru), 0, sizeof(hmac_btcoex_ps_stru));
    ps_param.band = WLAN_BAND_5G; /* 默认是5G无影响状态 */
    ps_param.bandwidth = WLAN_BAND_WIDTH_20M; /* 默认带宽模式 */

    /* 2.频段直接所有up或者pause的vap，取小，保证有2G设备时，数传需要生效 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_set_freq_and_work_state_hal_device::the vap is null!}", mac_vap_id[vap_index]);
            continue;
        }

        /* 非有效状态，不统，去关联时候调用此接口时还是up状态 */
        if (hmac_vap->vap_state != MAC_VAP_STATE_UP && hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) {
            continue;
        }

        hmac_btcoex_update_vap_work_status(hmac_vap, &ps_param);

        /* 关联用户数不为0 */
        if (hmac_vap->user_nums != 0) {
            (is_ap(hmac_vap)) ? (up_ap_num++) : (up_sta_num++);

            (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) ? (up_2g_num++) : (up_5g_num++);
        }

        /* 5.单ap模式   或5g模式 */
        if ((up_ap_num != 0 && up_sta_num == 0) || (up_5g_num != 0 && up_2g_num == 0)) {
            ps_param.ps_stop = OSAL_TRUE;
        }
    }

    hmac_btcoex_update_ps_capability(hal_device, &ps_param);
}
/* 关联/去关联处理 */
osal_void hmac_config_btcoex_assoc_state_proc(hmac_vap_stru *hmac_vap, const ble_status_stru *ble_status,
    hmac_user_btcoex_delba_stru *btcoex_delba, const bt_status_stru *bt_status,
    const hal_btcoex_btble_status_stru *btcoex_btble_status)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hmac_vap_btcoex_stru *hmac_vap_btcoex = hmac_btcoex_get_vap_info(hmac_vap);
    oal_bool_enum_uint8 need_delba = OSAL_FALSE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AUTO_FREQ_BTCOEX);
    unref_param(ble_status);

    if (hmac_vap_btcoex == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_btcoex_assoc_state_proc::hmac_vap_btcoex is null.");
        return;
    }
    /* 看是否需要执行ap siso切换 */
    if (bt_status->bt_ba != 0) {
        hmac_btcoex_assoc_ap_check_process(hal_device);
    }

    /* 默认刚关联上时mimo */
    /* 电话场景下需要立即删聚合 */
    if (bt_status->bt_6slot != 0) {
        get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_6SLOT_MASK;

        /* 电话下，需要立即触发删减BA，刷新hmac侧的BA标记 */
        need_delba = OSAL_TRUE;

        /* Tplink5630不识别聚合2个，还是高聚合发送造成电话下容易掉底断流 */
        btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_1;
    } else if (bt_status->bt_a2dp != 0 || bt_status->bt_transfer != 0) { /* 有sco存在时，a2dp=0，会采用sco链路来播放音乐 */
        /* 如果是在ldac音乐下，需要关闭动态调频，并关闭拉occu */
        if (bt_status->bt_ldac != 0) {
            if (fhook != OSAL_NULL) {
                ((hmac_auto_freq_btcoex_handle_cb)fhook)((osal_u8)bt_status->bt_ldac);
            }

            /* 关闭优先级配置 */
            hal_device->btcoex_sw_preempt.coex_pri_forbit = OSAL_TRUE;

            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_LDAC_MASK;
        }

        get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_A2DP_MASK;

        frw_destroy_timer_entry(&(hal_device->btcoex_powersave_timer));

        /* ps机制启动时，需要根据当前状态，刷新超时定时器时间,因为对应业务存在时，不会再来ps中断，需要此处来刷时间 */
        hmac_btcoex_ps_timeout_update_time(hal_device);

        /* 创建ps定时器 */
        frw_create_timer_entry(&(hal_device->btcoex_powersave_timer), hmac_btcoex_pow_save_callback,
            hal_device->btcoex_sw_preempt.timeout_ms, (osal_void *)hal_device, OSAL_FALSE);

        btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_2;
        hmac_vap_btcoex->hmac_vap_btcoex_rx_statistics.rx_rate_statistics_flag = OSAL_TRUE;
        hmac_vap_btcoex->hmac_vap_btcoex_rx_statistics.rx_rate_statistics_timeout = OSAL_FALSE;

        if (hmac_vap_btcoex->hmac_vap_btcoex_rx_statistics.bt_coex_statistics_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&(hmac_vap_btcoex->hmac_vap_btcoex_rx_statistics.bt_coex_statistics_timer));
        }

        frw_create_timer_entry(&(hmac_vap_btcoex->hmac_vap_btcoex_rx_statistics.bt_coex_statistics_timer),
            hmac_btcoex_rx_rate_statistics_flag_callback, BTCOEX_RX_STATISTICS_TIME, (osal_void *)hmac_vap,
            OSAL_TRUE);
    }

    hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btcoex_btble_status);

    oam_warning_log3(0, OAM_SF_COEX, "{hmac_config_btcoex_assoc_state_proc::status 6slot:%d,a2dp:%d,data_transfer:%d.}",
        bt_status->bt_6slot, bt_status->bt_a2dp, bt_status->bt_transfer);
    oam_warning_log3(0, OAM_SF_COEX, "{experc_ba_size:%d, ba_size:%d, need_delba:%d.}",
        btcoex_delba->ba_size_expect_index, btcoex_delba->ba_size, need_delba);

    hmac_btcoex_delba_trigger(hmac_vap, need_delba, btcoex_delba);
}

/*****************************************************************************
 功能描述  : 同步共存下mac的状态
*****************************************************************************/
osal_bool hmac_config_btcoex_disassoc_state_syn(osal_void *notify_data)
{
    hmac_vap_btcoex_rx_statistics_stru *btcoex_rx_stat = OSAL_NULL;
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (is_sta(hmac_vap) || (is_ap(hmac_vap) && (hmac_vap->user_nums == 0))) {
        /* sta down时,刷新频段，legacy vap和p2p 关联状态,带宽和信道 */
        hmac_btcoex_set_freq_and_work_state_hal_device(hmac_vap->hal_device);
    }

    btcoex_rx_stat = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics);
    btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);

    frw_destroy_timer_entry(&(btcoex_rx_stat->bt_coex_statistics_timer));
    frw_destroy_timer_entry(&(btcoex_rx_stat->bt_coex_low_rate_timer));
    // 功能暂时关闭
    frw_destroy_timer_entry(&(btcoex_occupied->bt_coex_occupied_timer));
    frw_destroy_timer_entry(&(btcoex_occupied->bt_coex_priority_timer));
    frw_destroy_timer_entry(&(hmac_btcoex_get_vap_info(hmac_vap)->bt_coex_double_chain_timer));

    hmac_btcoex_set_vap_ps_frame(hmac_vap, OSAL_FALSE);
    return OSAL_TRUE;
}

OSAL_STATIC osal_void hmac_btcoex_notify_inout_siso(hal_to_dmac_device_stru *hal_device)
{
    if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) {
        /* 进入c1 siso, wifi使用c1时，需要通知蓝牙c1 siso状态，可以全速发送 */
        hmac_btcoex_set_wifi_status_irq_notify(HAL_BTCOEX_WIFI_STATE_C1_SISO, (osal_u8)OSAL_TRUE);
    } else if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ZERO) {
        /* 出c1 siso, wifi使用c0，需要通知蓝牙c1 siso状态，不要全速发送 */
        hmac_btcoex_set_wifi_status_irq_notify(HAL_BTCOEX_WIFI_STATE_C1_SISO, (osal_u8)OSAL_FALSE);
    } else {
        oam_error_log2(0, OAM_SF_M2S, "{hmac_m2s_siso_to_siso:: phy_chain[%d], cur state[%d].}",
            hal_device->cfg_cap_info->phy_chain, hal_device->hal_m2s_fsm.oal_fsm.cur_state);
    }
}
osal_u32 hmac_btcoex_notify_init(osal_void)
{
    /* 消息接口注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_RESET, hmac_config_btcoex_disassoc_state_syn);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_NOTIFY_SET_WIFI_STATUS, hmac_btcoex_set_wifi_status_irq_notify);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_NOTIFY_INOUT_SISO, hmac_btcoex_notify_inout_siso);
    return OAL_SUCC;
}

osal_void hmac_btcoex_notify_deinit(osal_void)
{
    /* 消息接口去注册 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_RESET, hmac_config_btcoex_disassoc_state_syn);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_NOTIFY_SET_WIFI_STATUS);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_NOTIFY_INOUT_SISO);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
