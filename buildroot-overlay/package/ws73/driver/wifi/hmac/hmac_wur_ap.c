/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: wur功能处理
 * Date: 2020-07-07
 */

#ifdef _PRE_WLAN_FEATURE_WUR_TX

/* 1 头文件包含 */
#include "hmac_wur_ap.h"
#include "hmac_tx_mgmt.h"
#include "hmac_psm_ap.h"
#include "mac_frame.h"

#include "hal_device.h"

#include "mac_frame.h"
#include "oal_netbuf_data.h"

#include "hal_ext_if.h"
#include "hmac_feature_dft.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_mgmt_bss_comm.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_WUR_AP_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/* 2 全局变量定义 */
osal_u8 g_wur_twbtt_bcn_en = 0; /* TWBTT发送WUR BEACON帧开关 */
osal_u8 g_wur_period_en = 0; /* WUR周期配置开关 */
osal_u8 g_wur_frame_type = 0; /* 调试单播唤醒帧类型开关 */
/* 3 函数实现 */
/*****************************************************************************
 函 数 名  : hmac_wur_get_user_status
 功能描述  : 获取当前用户WUR的状态
*****************************************************************************/
osal_u8 hmac_wur_get_user_status(const hmac_user_stru *hmac_user)
{
    return hmac_user->wur_info.wur_status;
}

/*****************************************************************************
 函 数 名  : hmac_wur_is_ps_queue
 功能描述  : wur用户是否下发数据进节能队列
*****************************************************************************/
osal_u8 hmac_wur_is_ps_enqueue(const hmac_user_stru *hmac_user)
{
    return hmac_user->wur_info.wur_ps_enqueue;
}

/*****************************************************************************
 函 数 名  : hmac_wur_init_user_info
 功能描述  : wur用户信息初始化
*****************************************************************************/
osal_void hmac_wur_init_user_info(hmac_user_stru *hmac_user)
{
    hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_OFF;
    hmac_user->wur_info.wur_ps_enqueue = OSAL_FALSE;
    hmac_user->wur_info.unicast_buffered = OSAL_FALSE;
    hmac_user->wur_info.multi_buffered = OSAL_FALSE;
    hmac_user->wur_info.multi_time_cnt = 0;
    hmac_user->wur_info.send_multi_wakeup = OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_wur_user_del_handle
 功能描述  : 去关联时清除WUR寄存器，清除标志位
*****************************************************************************/
osal_void hmac_wur_user_del_handle(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_wur_mode_teardown_success(hmac_vap, hmac_user); /* 与取消WUR一致 */
}

/*****************************************************************************
 函 数 名  : hmac_wur_assoc_update
 功能描述  : 处理接收关联请求帧,同步参数从hmac到dmac
*****************************************************************************/
osal_void hmac_wur_assoc_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const mac_user_wur_cap_ie_stru *wur_cap_ie)
{
    /* 记录关联用户的WUR能力信息 */
    hmac_user->wur_cap_ie = *wur_cap_ie;

    /* 记录当前挂载WUR STA的最大转换时延 */
    if (hmac_user->wur_cap_ie.transition_delay >= hmac_vap->max_trans_delay) {
        hmac_vap->max_trans_delay = hmac_user->wur_cap_ie.transition_delay;
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_update
 功能描述  : 处理接收WUR相关帧,同步参数从hmac到dmac
*****************************************************************************/
osal_s32 hmac_wur_update(hmac_vap_stru *hmac_vap, hmac_ctx_update_wur_stru *crx_action_sync)
{
    osal_u32 ret;
    hmac_vap_stru *mac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user;

    if (crx_action_sync == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_update::crx_action_sync null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取vap结构信息 */
    mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_vap->vap_id);
    if (mac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_update::mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(crx_action_sync->user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_twt_update_regs::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap->wur_param_htd = crx_action_sync->wur_param_htd; /* hmac同步到dmac的参数 */

    /* WUR更新事件：收wur teardown帧，收wur setup帧 关联wur sta */
    if (crx_action_sync->update_source == MAC_WUR_UPDATE_SOURCE_TEARDOWN) {
        ret = hmac_wur_mode_teardown_success(hmac_vap, hmac_user);
        if (ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_ANY, "hmac_wur_update:wur_mode_teardown_success return [%d]", ret);
            return ret;
        }
    } else if (crx_action_sync->update_source == MAC_WUR_UPDATE_SOURCE_SETUP) {
        ret = hmac_wur_mode_setup_update(hmac_vap, hmac_user, crx_action_sync->wur_param_htd.action_type);
        if (ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_ANY, "hmac_wur_update:wur_mode_setup_update return [%d]", ret);
            return ret;
        }
    } else if (crx_action_sync->update_source == MAC_WUR_UPDATE_SOURCE_ASSOC) {
        hmac_wur_assoc_update(hmac_vap, hmac_user, &crx_action_sync->wur_cap_ie);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : dmac_mgmt_tx_wur_mode_setup
 功能描述  : send wur_mode_setup frame
*****************************************************************************/
osal_u32 dmac_mgmt_tx_wur_mode_setup(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;

    /* 当前只支持WUR STA一次请求一次响应 */
    if (hmac_wur_get_user_status(hmac_user) != HMAC_USER_WUR_MODE_OFF) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        oam_warning_log0(0, OAM_SF_11AX, "{dmac_mgmt_tx_wur_mode_setup:user is wur sta.}");
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    /* 填写WUR SETUP帧里的开始时间 */
    ret = hmac_wur_fill_start_time(hmac_vap, hmac_user, ctx_action_event, netbuf);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "dmac_mgmt_tx_wur_mode_setup:wur_fill_start_time return [%d]", ret);
        if (ret != OAL_SUCC) {
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        }
        return ret;
    }

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->tx_user_idx = hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, ctx_action_event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{dmac_mgmt_tx_wur_mode_setup::hmac_tx_mgmt return Err=%d.}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : dmac_mgmt_tx_wur_mode_teardown
 功能描述  : send wur_mode_teardown frame
*****************************************************************************/
osal_u32 dmac_mgmt_tx_wur_mode_teardown(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;

    osal_u32 ret;

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->tx_user_idx = hmac_user->assoc_id;
    tx_ctl->ac = WLAN_WME_AC_MGMT;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, ctx_action_event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{dmac_mgmt_tx_wur_mode_teardown::hmac_tx_mgmt Err=%d.}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wur_tx_mode_setup_success
 功能描述  : 发送WUR MODE SETUP帧成功处理函数
*****************************************************************************/
osal_u32 hmac_wur_tx_mode_setup_success(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *mac_payload)
{
    mac_wur_mode_setup_frame_stru *wur_setup_frame = (mac_wur_mode_setup_frame_stru *)mac_payload;
    mac_wur_mode_ie_stru *wur_mode_element = &(wur_setup_frame->wur_mode_element);
    mac_wur_param_ap_field_stru *wur_param_ap_field =
        (mac_wur_param_ap_field_stru *)(mac_payload + sizeof(mac_wur_mode_setup_frame_stru));
    osal_u32 ret;

    /* RSP STATUS不为ACCEPT, WUR寄存器不配置，直接返回 */
    if (wur_mode_element->wur_mode_rsp_status != MAC_WUR_MODE_ACCEPT) {
        return OAL_SUCC;
    }

    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return OAL_SUCC;
    }
    /* 发送WUR MODE SETUP帧成功,更新相关信息 */
    ret = hmac_wur_setup_succ_update(hmac_vap, hmac_user, wur_mode_element, wur_param_ap_field);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_wur_tx_mode_setup_success:wur_setup_succ_update return [%d]", ret);
        return ret;
    }

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_wur_tx_mode_setup_success::id[%d] wur_status[%d] wur_ps_enqueue[%d].}",
        hmac_user->assoc_id, hmac_user->wur_info.wur_status, hmac_user->wur_info.wur_ps_enqueue);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_wur_action_tx_complete_process
 功能描述  : 判断是否有WUR MODE setup/teardown帧发送完成，成功则处理
*****************************************************************************/
osal_void hmac_wur_action_tx_complete_process(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u8 dscr_status)
{
    osal_u8 *mac_header = oal_netbuf_header(netbuf);
    osal_u8 *mac_payload = oal_netbuf_rx_data(netbuf);
    mac_tx_ctl_stru *cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(cb->tx_user_idx);
    osal_u32 ret;

    if ((dscr_status != DMAC_TX_SUCC) || (hmac_user == OSAL_NULL)) {
        return;
    }

    /* 非WUR MODE setup/teardown帧，直接返回 */
    if (mac_get_frame_type_and_subtype(mac_header) != (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        return;
    }

    if ((mac_payload[0] != MAC_ACTION_CATEGORY_WUR) ||
        ((mac_payload[1] != MAC_WUR_ACTION_MODE_SETUP) && (mac_payload[1] != MAC_WUR_ACTION_MODE_TEARDOWN))) {
        return;
    }

    /* wur mode setup/teardown帧发送完成处理 */
    if (mac_payload[1] == MAC_WUR_ACTION_MODE_SETUP) {
        ret = hmac_wur_tx_mode_setup_success(hmac_vap, hmac_user, mac_payload);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_frame_tx_complete_process:tx_mode_setup ERR.}");
            return;
        }
    } else if (mac_payload[1] == MAC_WUR_ACTION_MODE_TEARDOWN) {
        hmac_wur_mode_teardown_success(hmac_vap, hmac_user);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_encap_qos_data_header
 功能描述  : 组qos data的mac header
*****************************************************************************/
osal_void hmac_wur_encap_qos_data_header(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 ac, oal_netbuf_stru *net_buf)
{
    mac_ieee80211_qos_frame_stru *mac_header;
    osal_u16 tx_direction;

    /* null帧发送方向From AP || To AP */
    tx_direction =  WLAN_FRAME_FROM_AP;
    /* 填写帧头 */
    (void)memset_s(oal_netbuf_header(net_buf), sizeof(mac_ieee80211_qos_frame_stru),
        0, sizeof(mac_ieee80211_qos_frame_stru));
    mac_hdr_set_frame_control(oal_netbuf_header(net_buf),
        ((osal_u16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS) | tx_direction));
    /* 设置ADDR1为DA */
    oal_set_mac_addr((oal_netbuf_header(net_buf) + WLAN_HDR_ADDR1_OFFSET),
        hmac_user->user_mac_addr);
    /* 设置ADDR2为BSSID */
    oal_set_mac_addr((oal_netbuf_header(net_buf) + WLAN_HDR_ADDR2_OFFSET),
        mac_mib_get_station_id(hmac_vap));
    /* 设置ADDR3为SA */
    oal_set_mac_addr((oal_netbuf_header(net_buf) + WLAN_HDR_ADDR3_OFFSET),
        mac_mib_get_station_id(hmac_vap));

    mac_header = (mac_ieee80211_qos_frame_stru *)oal_netbuf_header(net_buf);
    mac_header->qc_tid = wlan_wme_ac_to_tid(ac);
    mac_header->qc_eosp = 0;
    mac_header->frame_control.power_mgmt = 0;

    /* WUR ack policy为NO ACK */
    mac_header->qc_ack_polocy = WLAN_TX_NO_ACK;
}

/*****************************************************************************
 函 数 名  : hmac_wur_send_qos_data
 功能描述  : 组帧并发送假Qos data帧作为WUR帧的载体
*****************************************************************************/
osal_u32 hmac_wur_send_qos_data(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 ac, osal_u8 wur_frame_type)
{
    oal_netbuf_stru *net_buf;
    osal_u32 ret;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;

    net_buf = hmac_config_create_al_tx_packet(OOS_DATA_LEN, 1);
    if (net_buf == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_send_qos_data::net_buf failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    oal_set_netbuf_prev(net_buf, OSAL_NULL);
    oal_set_netbuf_next(net_buf, OSAL_NULL);

    /* 组qos data的mac header */
    hmac_wur_encap_qos_data_header(hmac_vap, hmac_user, ac, net_buf);

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    (void)memset_s((osal_u8 *)tx_ctrl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    /* 填写tx部分 */
    tx_ctrl->ack_policy = WLAN_TX_NO_ACK;
    tx_ctrl->ac = ac;
    tx_ctrl->retried_num = 0;
    tx_ctrl->tid = wlan_wme_ac_to_tid(ac);
    tx_ctrl->tx_vap_index = hmac_vap->vap_id;
    tx_ctrl->tx_user_idx = hmac_user->assoc_id;
    /* 填写tx rx公共部分 */
    tx_ctrl->frame_header_length = sizeof(mac_ieee80211_qos_frame_stru);
    tx_ctrl->mpdu_num = 1;
    tx_ctrl->msdu_num = 1;
    tx_ctrl->netbuf_num = 1;
    tx_ctrl->mpdu_payload_len = OOS_DATA_LEN;
    tx_ctrl->tx_user_idx = hmac_user->assoc_id;
    tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;
    tx_ctrl->is_wur = wur_frame_type; /* 标记WUR帧以及具体帧类型 */

    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_wur_send_qos_data::hmac_tx_process_data failed[%d].}", ret);
        hmac_tx_excp_free_netbuf(net_buf);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_wur_is_all_support_hdr
 功能描述  : 组播判断是否所有的用户支持高速率的WUR帧
*****************************************************************************/
osal_u32 hmac_wur_is_all_support_hdr(const hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *dlist_tmp = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    osal_u32 ret = OSAL_TRUE;

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_11AX, "hmac_wur_is_all_support_hdr::hmac_user is null.");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 跳过非WUR用户 */
        if (hmac_wur_get_user_status(hmac_user) == HMAC_USER_WUR_MODE_OFF) {
            continue;
        }

        /* 是否支持高速率的WUR帧 */
        if (hmac_user->wur_cap_ie.wur_basic_ppdu_hdr_support_20mhz == OSAL_FALSE) {
            ret = OSAL_FALSE;
            break;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wur_write_wake_up_frame
 功能描述  : 把WUR唤醒帧内容写入明文寄存器
*****************************************************************************/
osal_void hmac_wur_write_wake_up_frame(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt, osal_u8 wur_frame_type)
{
    wur_wakeup_reg_param_stru wur_wakeup_param;
    osal_u16 id;

    id = (((hmac_vap->bssid[4] & 0x0F) << 8) | (hmac_vap->bssid[5] & 0xFF)); /* bssid低12位 4,5:取低位, 8:移位拼接 */

    if (wur_frame_type <= WUR_TYPE_BEACON_HDR) {
        /* 填写WUR BEACON帧transmitted id */
        hal_write_wur_beacon(id);
    } else if (wur_frame_type <= WUR_TYPE_FL_WAKE_HDR) {
        if (is_multi == WUR_BSS_PARAM_WAKEUP) {
            /* BSS参数更新唤醒帧，group标记位不置位，只更新counter计数 */
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_write_wake_up_frame:BSS wakeup no timer.}");
            wur_wakeup_param.counter = wur_bss_update_cnt; /* BSS参数更新计数 */
            wur_wakeup_param.group_buffer = 0;
            wur_wakeup_param.id = id; /* Transmitter ID */
        } else {
            /* 单播 组播唤醒帧，group标记位置位，不更新counter计数 */
            wur_wakeup_param.counter = 0;
            wur_wakeup_param.group_buffer = is_multi; /* 指示广播 组播缓存 */
            wur_wakeup_param.id = (is_multi ? id : hmac_user->assoc_id); /* Transmitter ID/单播WUR ID */
        }

        /* 填写WUR FL WAKE-UP帧 */
        hal_write_wur_fl_wake(&wur_wakeup_param);
    } else if (wur_frame_type <= WUR_TYPE_SHORT_WAKE_HDR) {
        /* 填写WUR SHORT WAKE-UP帧WUR ID */
        hal_write_wur_short_wake(hmac_user->assoc_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_send_wake_up
 功能描述  : 组帧并发送WUR唤醒帧
*****************************************************************************/
osal_u32 hmac_wur_send_wake_up(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt)
{
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hal_device_stru *device = (hal_device_stru *)hal_device;
    osal_u8 wur_frame_type = WUR_TYPE_FL_WAKE_LDR;

    /* 组WUR唤醒帧 */
    ret = hmac_wur_encap_wake_up_frame(hmac_vap, hmac_user, is_multi, wur_bss_update_cnt, &wur_frame_type);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_wur_send_wake_up:wur_encap_wake_up_frame return [%d]", ret);
        return ret;
    }

    /* 发送WUR帧借用Qos data帧为载体, 默认高优先级队列 */
    ret = hmac_wur_send_qos_data(hmac_vap, hmac_user, device->wur_ac, wur_frame_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_wur_send_wake_up::ERR[%d] hmac_wur_send_qos_data}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wur_unicast_wake_process
 功能描述  : WUR服务周期内单播唤醒wur sta情况处理
*****************************************************************************/
osal_void hmac_wur_unicast_wake_process(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *dlist_tmp = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;

    /* 在WUR中断开始时遍历所有的关联用户
       1. 判断用户是否处于WUR Mode ON状态，并且有缓存数据标志位
       2. 根据用户能力发送WUR唤醒帧或者短唤醒帧 */
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        /* 不处于WUR服务周期，直接返回 */
        if (hmac_vap->wur_in_service == OSAL_FALSE) {
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_unicast_wake_process::service end}");
            return;
        }

        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_11AX, "hmac_wur_unicast_wake_process::hmac_user is null.");
            return;
        }

        oam_warning_log1(0, OAM_SF_11AX, "{hmac_wur_unicast_wake_process::user_id=%d}", hmac_user->assoc_id);

        /* 用户处于wur mode suspend时，跳过WUR周期处理 */
        if (hmac_user->wur_info.wur_status == HMAC_USER_WUR_MODE_SUSPEND) {
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_unicast_wake_process:: wur mode suspend, not process}");
            continue;
        }

        /* 判断用户是否处于WUR Mode状态，并且是否有缓存数据标志位 */
        if (hmac_user->wur_info.wur_status == HMAC_USER_WUR_MODE_ON &&
            hmac_user->wur_info.unicast_buffered == OSAL_TRUE) {
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_unicast_wake_process::unicast_buffered, wake up}");

            ret = hmac_wur_send_wake_up(hmac_vap, hmac_user, WUR_UNICAST_WAKEUP, 0);
            if (ret != OAL_SUCC) {
                /* 发送唤醒帧异常打印，继续后续发送 */
                oam_error_log1(0, OAM_SF_11AX,
                    "{hmac_wur_unicast_wake_process::ERR hmac_wur_send_wake_up return=%d}", ret);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_multi_wake_process
 功能描述  : WUR服务周期内组播唤醒wur sta情况处理
*****************************************************************************/
osal_void hmac_wur_multi_wake_process(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    hmac_user_stru *hmac_multi_user = OSAL_NULL;
    mac_vap_wur_bss_stru *wur_bss_last = (&hmac_vap->wur_bss->wur_bss_last); /* 记录之前的BSS参数 */

    hmac_multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (hmac_multi_user == OSAL_NULL) {
        oam_error_log1(hmac_vap->vap_id, OAM_SF_WIFI_BEACON,
            "{hmac_wur_multi_wake_process::multi user[%d] fail.}", hmac_vap->multi_user_idx);
        return;
    }

    /* BSS参数更新检测，并且仍处于WUR服务周期 */
    if (wur_bss_last->wakeup_flag == OSAL_TRUE && hmac_vap->wur_in_service == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_multi_wake_process::BSS update wake up.}");

        ret = hmac_wur_send_wake_up(hmac_vap, hmac_multi_user, WUR_BSS_PARAM_WAKEUP, wur_bss_last->update_cnt);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_11AX,
                "{hmac_wur_multi_wake_process::ERR BSS hmac_wur_send_wake_up return=%d}", ret);
        }

        wur_bss_last->wakeup_flag = OSAL_FALSE; /* BSS参数更新发一次 */
    }

    /* 组播缓存检测，并且仍处于WUR服务周期 */
    if (hmac_multi_user->wur_info.multi_buffered == OSAL_TRUE && hmac_vap->wur_in_service == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_multi_wake_process::multi_buffered, wake up}");

        ret = hmac_wur_send_wake_up(hmac_vap, hmac_multi_user, WUR_MULTICAST_WAKEUP, 0);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_11AX,
                "{hmac_wur_multi_wake_process::ERR group hmac_wur_send_wake_up return=%d}", ret);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_sp_start_event
 功能描述  : 处理wur服务周期开始中断上报事件
*****************************************************************************/
osal_s32 hmac_wur_sp_start_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    return hmac_wur_sp_start_process(hmac_vap, msg);
}

/*****************************************************************************
 函 数 名  : hmac_wur_sp_end_event
 功能描述  : 处理wur服务周期结束中断上报事件
*****************************************************************************/
osal_s32 hmac_wur_sp_end_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    return hmac_wur_sp_end_process(hmac_vap, msg);
}

/*****************************************************************************
 函 数 名  : hmac_wur_ps_enqueue_process
 功能描述  : WUR下进节能队列的处理
*****************************************************************************/
osal_void hmac_wur_ps_enqueue_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *tx_ctrl)
{
    /* 单播数据且用户处于WUR_ON状态，进缓存队列
       组播数据且当前有仍处于WUR_ON状态的用户，进缓存队列 */
    if (is_ap(hmac_vap) &&
        (((hmac_wur_get_user_status(hmac_user) >= HMAC_USER_WUR_MODE_ON) && (tx_ctrl->ismcast == OSAL_FALSE)) ||
        (hmac_vap->cur_wur_user_num != 0 && tx_ctrl->ismcast == OSAL_TRUE))) {
        /* APUT,用户处于WUR，单播/组播缓存数据需要标记 */
        if (tx_ctrl->ismcast == OSAL_FALSE) {
            hmac_user->wur_info.unicast_buffered = OSAL_TRUE;
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_ps_enqueue_process::unicast is_buffered}");
        } else {
            hmac_user->wur_info.multi_buffered = OSAL_TRUE;
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_ps_enqueue_process::multicast is_buffered}");
        }

        /* WUR单/组播帧也需要TIM置位 */
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 1);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_is_bss_update
 功能描述  : 检查BSS参数是否更新
*****************************************************************************/
osal_u8 hmac_wur_is_bss_update(hmac_vap_stru *hmac_vap)
{
    mac_vap_wur_bss_stru *last = (&hmac_vap->wur_bss->wur_bss_last); /* 记录之前的BSS参数 */
    mac_vap_wur_bss_stru *cur = (&hmac_vap->wur_bss->wur_bss_cur); /* 记录最新的BSS参数 */
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_CSA_PRESENT_IN_BCN);

    /* dsss参数，wmm参数修改 */
    if ((last->dsss_channel_num != cur->dsss_channel_num) ||
        (last->wmm_params_update_count != cur->wmm_params_update_count)) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_is_bss_update::dsss || wmm}");
        return OSAL_TRUE;
    }

    /* ht/vht/he operation参数修改 */
    if ((memcmp(&(last->ht_opern_tmp), &(cur->ht_opern_tmp), sizeof(mac_ht_opern_stru)) != OAL_SUCC) ||
        (memcmp(&(last->vht_opern_tmp), &(cur->vht_opern_tmp), sizeof(mac_vht_opern_stru)) != OAL_SUCC) ||
        (memcmp(&(last->he_opern_tmp), &(cur->he_opern_tmp), sizeof(mac_he_opern_stru)) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_is_bss_update::ht || vht || he}");
        return OSAL_TRUE;
    }

    /* 包含csa就是更新 */
    if (fhook != OSAL_NULL && ((hmac_csa_ap_csa_present_in_bcn_cb)fhook)(hmac_vap->vap_id) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_is_bss_update::csa}");
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_wur_bss_update_process
 功能描述  :BSS参数更新处理
*****************************************************************************/
osal_void hmac_wur_bss_update_process(hmac_vap_stru *hmac_vap)
{
    mac_vap_wur_bss_stru *last = OSAL_NULL;
    mac_vap_wur_bss_stru *cur = OSAL_NULL;
    osal_s32 ret;

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    last = &(hmac_vap->wur_bss->wur_bss_last); /* 记录之前的BSS参数 */
    cur = &(hmac_vap->wur_bss->wur_bss_cur); /* 记录最新的BSS参数 */

    /* 当前没有WUR_ON用户,不做BSS参数比较 */
    if (hmac_vap->cur_wur_user_num == 0) {
        return;
    }

    cur->is_first = OSAL_FALSE; /* 不是第一次记录BSS参数 */
    cur->update_cnt = last->update_cnt; /* 保存之前的bss计数 */

    /* 第一次检测BSS参数只保存，其他需要进行比较,有更新置位，计数++ */
    if (last->is_first == OSAL_FALSE) {
        if (hmac_wur_is_bss_update(hmac_vap) == OSAL_TRUE) {
            cur->wakeup_flag = OSAL_TRUE;
            cur->update_cnt++;
            oam_warning_log1(0, OAM_SF_11AX, "{hmac_wur_bss_update_process::update_cnt[%d]",
                cur->update_cnt);
        } else {
            cur->wakeup_flag = OSAL_FALSE;
        }
    }

    /* 保留最新的BSS参数，留作比较 */
    ret = memcpy_s(last, sizeof(mac_vap_wur_bss_stru), cur, sizeof(mac_vap_wur_bss_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_bss_update_process::MEMCOPY ERROR}");
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_ht_oper_ie_save
 功能描述  : BSS参数中HT operation element信息保存
*****************************************************************************/
osal_void hmac_wur_ht_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_ht_opern_stru *ht_opern_tmp)
{
    if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE) {
        /* WUR BSS参数保存 */
        mac_ht_opern_stru *ht_opern_temp = &(hmac_vap->wur_bss->wur_bss_cur.ht_opern_tmp);
        osal_s32 ret = memcpy_s(ht_opern_temp, sizeof(mac_ht_opern_stru),
            ht_opern_tmp, sizeof(mac_ht_opern_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_ht_oper_ie_save::MEMCOPY ERROR}");
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_vht_oper_ie_save
 功能描述  : BSS参数中VHT operation element信息保存
*****************************************************************************/
osal_void hmac_wur_vht_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_vht_opern_stru *vht_opern_tmp)
{
    if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE) {
        /* WUR BSS参数保存 */
        mac_vht_opern_stru *vht_opern_temp = &(hmac_vap->wur_bss->wur_bss_cur.vht_opern_tmp);
        osal_s32 ret = memcpy_s(vht_opern_temp, sizeof(mac_vht_opern_stru),
            vht_opern_tmp, sizeof(mac_vht_opern_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_vht_oper_ie_save::MEMCOPY ERROR}");
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_he_oper_ie_save
 功能描述  : BSS参数中HE operation element信息保存
*****************************************************************************/
osal_void hmac_wur_he_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_frame_he_operation_param_stru *he_opern_param,
    const mac_frame_he_bss_color_info_stru *he_bss_color, const mac_frame_he_mcs_nss_bit_map_stru *mcs_bit_map)
{
    if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE) {
        /* WUR BSS参数保存 */
        mac_he_opern_stru *he_opern_tmp = &(hmac_vap->wur_bss->wur_bss_cur.he_opern_tmp);
        osal_s32 ret = memcpy_s(&(he_opern_tmp->he_opern_param), sizeof(mac_frame_he_operation_param_stru),
            he_opern_param, sizeof(mac_frame_he_operation_param_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_he_oper_ie_save::MEMCOPY ERROR he_opern}");
        }

        ret = memcpy_s(&(he_opern_tmp->he_bss_color), sizeof(mac_frame_he_bss_color_info_stru),
            he_bss_color, sizeof(mac_frame_he_bss_color_info_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_he_oper_ie_save::MEMCOPY ERROR bss_color}");
        }

        ret = memcpy_s(&(he_opern_tmp->mcs_bit_map), sizeof(mac_frame_he_mcs_nss_bit_map_stru),
            mcs_bit_map, sizeof(mac_frame_he_mcs_nss_bit_map_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_wur_he_oper_ie_save::MEMCOPY ERROR mcs}");
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_wur_set_param_en
 功能描述  : 设置WUR相关参数使能
*****************************************************************************/
osal_void hmac_wur_set_param_en(osal_u8 wur_twbtt_bcn_en, osal_u8 wur_period_en,
    osal_u8 wur_frame_type)
{
    g_wur_twbtt_bcn_en = wur_twbtt_bcn_en;
    g_wur_period_en = wur_period_en;
    g_wur_frame_type = wur_frame_type;
}

/*****************************************************************************
 功能描述  : WUR特性预留接口，去处理接收的节能帧
*****************************************************************************/
osal_u32 hmac_wur_psm_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *net_buf)
{
    unref_param(hmac_vap);
    unref_param(hmac_user);
    unref_param(net_buf);
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 处理接收WUR Mode Setup帧 同步参数从hmac到dmac
*****************************************************************************/
osal_u32 hmac_wur_mode_setup_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 action_type)
{
    osal_u8 cur_wur_status = hmac_user->wur_info.wur_status;
    /* 在已建立WUR的情况下接收到WUR Mode Setup帧，修改WUR状态，其他不处理 */
    if (action_type == MAC_WUR_ENTER_WUR_MODE && cur_wur_status == HMAC_USER_WUR_MODE_SUSPEND) {
        /* 切回WUR收发，缓存开始 */
        hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_ON;
        hmac_user->wur_info.wur_ps_enqueue = OSAL_TRUE;
        hmac_vap->cur_wur_user_num++; /* 用户将切回WUR收发 */

        /* 为了wur与fast-ps分开，不受影响 */
        if (hmac_user->ps_mode == OSAL_TRUE) {
            hmac_user->ps_mode = OSAL_FALSE;
            hmac_user->ps_structure.ps_time_count = 0;
            hmac_vap->ps_user_num--;
        }
    } else if (action_type == MAC_WUR_ENTER_WUR_MODE_SUSPEND && cur_wur_status == HMAC_USER_WUR_MODE_ON) {
        hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_SUSPEND;
        hmac_vap->cur_wur_user_num--; /* 用户已切回主收发 */
    } else {
        return OAL_CONTINUE;
    }

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_wur_mode_setup_update::id=[%d] last_status=[%d] wur_status=[%d].}",
        hmac_user->assoc_id, cur_wur_status, hmac_user->wur_info.wur_status);
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 填写WUR Mode Setup帧的WUR周期的开始时间
*****************************************************************************/
osal_u32 hmac_wur_fill_start_time(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, const oal_netbuf_stru *netbuf)
{
    const hal_to_dmac_vap_stru *hal_vap = hmac_vap->hal_vap;
    mac_wur_mode_setup_frame_stru *wur_setup_frame = OSAL_NULL;
    mac_wur_param_ap_field_stru *wur_param_ap_field = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u32 tsf_h = 0;
    osal_u32 tsf_l = 0;
    osal_u64 current_tsf;
    osal_u16 offset_twbtt = hmac_vap->wur_basic_param.offset_twbtt;
    osal_u16 wur_beacon_period = hmac_vap->wur_basic_param.wur_beacon_period;
    osal_u32 wurbcn_offset_wur = hmac_vap->wur_basic_param.wurbcn_offset_wur;
    unref_param(hmac_user);
    unref_param(ctx_action_event);

    /* 将索引指向frame body起始位置 */
    data = oal_netbuf_tx_data(netbuf);
    wur_setup_frame = (mac_wur_mode_setup_frame_stru *)data;
    wur_param_ap_field = (mac_wur_param_ap_field_stru *)(data + sizeof(mac_wur_mode_setup_frame_stru));

    /* 回复的WUR MODE SETUP需要填start_time */
    if (wur_setup_frame->wur_mode_element.wur_param_control_field.wur_duty_cycle_start_time_present) {
        hal_vap_tsf_get_64bit(hal_vap, &tsf_h, &tsf_l);
        current_tsf = (((osal_u64)tsf_h) << HMAC_WUR_BITS_OFFSET) | (osal_u64)tsf_l;

        /* 第一次开始时间需要控制到TWBTT第二个周期前，后续以第一次开始时间取最近的WUR周期开始时间 */
        if (hmac_vap->wur_user_num == 0) {
            wur_param_ap_field->wur_duty_cycle_start_time =
                ((current_tsf / (wur_beacon_period * WUR_TU)) + 2) /* 2:第二个TWBTT前 */
                * (wur_beacon_period * WUR_TU) - (offset_twbtt + wurbcn_offset_wur) * WUR_TU;
        } else {
            wur_param_ap_field->wur_duty_cycle_start_time = current_tsf + (hmac_vap->wur_param_htd.cycle_period -
                (current_tsf - hmac_vap->wur_basic_param.start_time) % hmac_vap->wur_param_htd.cycle_period);
        }

        oam_warning_log2(0, OAM_SF_11AX, "{hmac_wur_fill_wur_start_time:cur_tsf=[%d] start_time=[%d].}",
            current_tsf, wur_param_ap_field->wur_duty_cycle_start_time);
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 设置TWBTT，在每个TWBTT中断下发WUR BEACON帧
*****************************************************************************/
osal_u32 hmac_wur_twbtt_send_beacon(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u16 wur_offset_twbtt, wur_bcn_period, id;
    osal_u8 wur_twbtt_bcn_en;
    osal_u8 *bcn_payload = OSAL_NULL;
    wur_bcn_reg_param_stru wur_bcn_param;

    /* 开启TWBTT并发送WUR BEACON帧 */
    if (g_wur_twbtt_bcn_en == 0) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_twbtt_send_beacon:twbtt not open.}");
        return OAL_CONTINUE;
    }

    wur_offset_twbtt = hmac_vap->wur_basic_param.offset_twbtt;
    wur_bcn_period = hmac_vap->wur_basic_param.wur_beacon_period;
    /* 4 5：取低位 8：移位拼接 */
    id = (((hmac_vap->bssid[4] & 0x0F) << 8) | (hmac_vap->bssid[5] & 0xFF)); /* 未加密取的bssid低12位 */
    wur_twbtt_bcn_en = WUR_TYPE_BEACON_LDR;
    /* 是否支持高速率的WUR帧 */
    if (hmac_user->wur_cap_ie.wur_basic_ppdu_hdr_support_20mhz == OSAL_TRUE) {
        wur_twbtt_bcn_en += WUR_HDR;
    }

    oam_warning_log2(0, OAM_SF_11AX, "{hmac_wur_twbtt_send_beacon:twbtt_offset=[%d] wur_bcn_epriod=[%d].}",
        wur_offset_twbtt, wur_bcn_period);

    /* 可以复用软ap申请的beacon帧地址 */
    if (hmac_vap->beacon_buffer == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WIFI_BEACON, "{hmac_wur_twbtt_send_beacon::beacon buf = NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    bcn_payload = oal_netbuf_header(hmac_vap->beacon_buffer);

    /* 填充WUR BEACON帧的发送参数 */
    wur_bcn_param.wur_bcn_period = wur_bcn_period;
    wur_bcn_param.pkt_ptr = (uintptr_t)bcn_payload;
    wur_bcn_param.pkt_len = 0xb0; /* WUR帧特定修改 */
    wur_bcn_param.phy_tx_mode = 0x0001c800; /* WUR帧特定修改 */
    wur_bcn_param.tx_data_rate = 0x004B06D0; /* WUR帧特定修改 */

    /* 填写WUR BEACON帧明文寄存器 */
    hal_write_wur_beacon(id);

    /* 填写vap发送wur beacon帧的地址、长度、速率、功率参数 */
    hal_vap_send_wur_beacon_pkt(&wur_bcn_param);

    /* 配置硬件发送WUR BEACON帧寄存器使能 DR EN */
    hal_vap_set_wur_beacon_en(wur_twbtt_bcn_en - 1, OSAL_TRUE);

    /* 使能TWBTT */
    hal_vap_set_wur_twbtt(wur_offset_twbtt, OSAL_TRUE);

    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 设置WUR服务期，类似TWT操作
*****************************************************************************/
osal_void hmac_wur_set_cycle_period(const hmac_vap_stru *hmac_vap, osal_u64 start_time)
{
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    wur_reg_param_stru wur_param;

    /* 测试用，开启WUR周期 */
    if (g_wur_period_en == 0) {
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_set_cycle_period: wur not open.}");
        return;
    }

    hal_vap = hmac_vap->hal_vap;

    /* WUR服务周期参数填写 */
    wur_param.start_tsf = start_time;
    wur_param.interval = hmac_vap->wur_param_htd.cycle_period;
    wur_param.duration = hmac_vap->wur_param_htd.cycle_service_period;

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_wur_set_cycle_period: start_tsf=[%d] interval=[%d] duration=[%d].}",
        wur_param.start_tsf, wur_param.interval, wur_param.duration);

    /* 配置WUR服务周期寄存器使能 */
    hal_vap_set_wur(hal_vap, &wur_param, 0xFF);
}

/*****************************************************************************
 功能描述  : 发送WUR MODE SETUP帧成功,更新相关信息
*****************************************************************************/
osal_u32 hmac_wur_setup_succ_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_mode_ie_stru *wur_mode_element, const mac_wur_param_ap_field_stru *wur_param_ap_field)
{
    osal_u64 start_time = 0;
    osal_u32 ret = OAL_CONTINUE;

    /* 为了wur与fast-ps分开，不受影响 */
    if (hmac_user->ps_mode == OSAL_TRUE) {
        hmac_user->ps_mode = OSAL_FALSE;
        hmac_user->ps_structure.ps_time_count = 0;
        hmac_vap->ps_user_num--;
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_wur_tx_mode_setup_success::cancel fast-ps.}");
    }

    /* 发送WUR Mode Setup帧完成，修改WUR状态 */
    if (wur_mode_element->action_type == MAC_WUR_ENTER_WUR_MODE_RSP) {
        hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_ON;
    } else if (wur_mode_element->action_type == MAC_WUR_ENTER_WUR_MODE_SUSPEND_RSP) {
        hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_SUSPEND;
    }

    /* 标记下发数据直接缓存，WUR用户数量增加 */
    hmac_user->wur_info.wur_ps_enqueue = OSAL_TRUE;
    hmac_vap->wur_user_num++;
    hmac_vap->cur_wur_user_num++;

    /* 暂停user，暂停该user的每一个tid */
    hmac_user_pause(hmac_user);

    /* 只支持一套WUR周期参数，只初始化一次 */
    if (hmac_vap->wur_user_num == 1) {
        if (wur_mode_element->wur_param_control_field.wur_duty_cycle_start_time_present) {
            start_time = wur_param_ap_field->wur_duty_cycle_start_time;
            hmac_vap->wur_basic_param.start_time = start_time; /* 记录WUR周期第一次开始的时间 */
        }

        /* 使能TWBTT发送wur beacon帧 */
        ret = hmac_wur_twbtt_send_beacon(hmac_vap, hmac_user);
        if (ret != OAL_CONTINUE) {
            oam_error_log0(0, OAM_SF_WIFI_BEACON, "{hmac_wur_setup_succ_update::hmac_wur_twbtt_send_beacon failed.}");
            return ret;
        }

        /* 使能WUR周期 */
        hmac_wur_set_cycle_period(hmac_vap, start_time);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 发送/接收WUR MODE TEARDOWN帧成功处理函数
*****************************************************************************/
osal_u32 hmac_wur_mode_teardown_success(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hal_to_dmac_vap_stru *hal_vap = hmac_vap->hal_vap;

    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return OAL_FAIL;
    }
    if (hmac_wur_get_user_status(hmac_user) >= HMAC_USER_WUR_MODE_ON) {
        /* suspend状态已经不处于WUR状态 */
        if (hmac_wur_get_user_status(hmac_user) == HMAC_USER_WUR_MODE_ON) {
            hmac_vap->cur_wur_user_num--;
        }

        /* WUR状态关闭，标记下发数据直接下发，WUR用户数量减少 */
        hmac_user->wur_info.wur_status = HMAC_USER_WUR_MODE_OFF;
        hmac_user->wur_info.wur_ps_enqueue = OSAL_FALSE;
        hmac_vap->wur_user_num--;

        /* 若关联用户没有WUR用户，暂停WUR服务周期和TWBTT */
        if (hmac_vap->wur_user_num == 0) {
            /* 关闭TWBTT发送WUR BEACON帧使能 */
            hal_vap_set_wur_beacon_en(0, OSAL_FALSE);
            /* 关闭TWBTT使能 */
            hal_vap_set_wur_twbtt(0, OSAL_FALSE);
            /* 关闭WUR服务周期使能 */
            hal_vap_set_wur(hal_vap, OSAL_NULL, 0);
        }

        oam_warning_log3(0, OAM_SF_11AX,
            "{hmac_wur_tx_rx_mode_teardown_success::id=[%d]wur_user_num[%d]cur_wur_user_num[%d].}",
            hmac_user->assoc_id, hmac_vap->wur_user_num, hmac_vap->cur_wur_user_num);
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 组WUR唤醒帧,返回WUR帧类型
*****************************************************************************/
osal_u32 hmac_wur_encap_wake_up_frame(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt, osal_u8 *wur_frame_type)
{
    osal_u8 frame_type = WUR_TYPE_FL_WAKE_LDR; /* 默认WUR FL WAKE UP帧 低速率 */

    /* 确定WUR唤醒帧类型 */
    if (is_multi >= WUR_MULTICAST_WAKEUP) {
        /* 是否都支持高速率的WUR帧 */
        if (hmac_wur_is_all_support_hdr(hmac_vap) == OSAL_TRUE) {
            frame_type += WUR_HDR;
        }
    } else if (is_multi == WUR_UNICAST_WAKEUP) {
        /* 是否支持高速率的WUR帧 */
        if (hmac_user->wur_cap_ie.wur_basic_ppdu_hdr_support_20mhz == OSAL_TRUE) {
            frame_type += WUR_HDR;
        }

        /* 是否支持WUR短唤醒帧 */
        if (hmac_user->wur_cap_ie.wur_short_wakeup_frame_support == OSAL_TRUE) {
            frame_type += WUR_SHORT;
        }
    }

    /* 测试用，单播唤醒令可接管 */
    if ((g_wur_frame_type > 0) && (is_multi == WUR_UNICAST_WAKEUP)) {
        frame_type = g_wur_frame_type;
    }

    oam_warning_log1(0, OAM_SF_11AX, "{hmac_wur_encap_wake_up_frame:wakeup_frame_type[%d].}", frame_type);

    /* 写WUR唤醒帧内容进明文寄存器 */
    hmac_wur_write_wake_up_frame(hmac_vap, hmac_user, is_multi, wur_bss_update_cnt, frame_type);

    *wur_frame_type = frame_type;

    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 判断WUR组播缓存是否可以下发
*****************************************************************************/
osal_u8 hmac_wur_is_multi_flush(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    /* 当前没有处于WUR状态的STA，组播可以当前dtim下发 */
    if (hmac_vap->cur_wur_user_num == 0) {
        return OSAL_TRUE;
    }

    /* 当前有处于WUR状态的STA，WUR FL WAKE UP帧已发送,需要进行组播缓存下发判断 */
    if (hmac_user->wur_info.send_multi_wakeup == OSAL_TRUE) {
        osal_u16 bcn_interval = (osal_u16)mac_mib_get_beacon_period(hmac_vap);
        osal_u8 dtim_peiod = (osal_u8)mac_mib_get_dot11_dtim_period(hmac_vap);
        osal_u32 max_trans_delay = ((osal_u32)(hmac_vap->max_trans_delay + 1) * 256); /* 256: 协议换算 */
        osal_u32 total_time = (osal_u32)bcn_interval * (osal_u32)dtim_peiod * hmac_user->wur_info.multi_time_cnt;

        /* 等待最大的WUR STA转换时延过去，取最近的dtim时刻下发，至少等一个dtim */
        if ((total_time > max_trans_delay) || (hmac_user->wur_info.multi_time_cnt == 0xFF)) {
            hmac_user->wur_info.multi_time_cnt = 0;
            hmac_user->wur_info.send_multi_wakeup = OSAL_FALSE;
            oam_warning_log2(0, OAM_SF_WIFI_BEACON,
                "{hmac_wur_is_multi_flush::total[%d]>max_delay[%d]}", total_time, max_trans_delay);
            return OSAL_TRUE;
        }

        hmac_user->wur_info.multi_time_cnt++;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 处理wur服务周期开始中断上报事件
*****************************************************************************/
osal_u32 hmac_wur_sp_start_process(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);

    hmac_vap->wur_in_service = OSAL_TRUE; /* 当前处于WUR服务周期 */

    /* 若没有sta处于WUR ON的状态，WUR周期不需要唤醒处理 */
    if (hmac_vap->cur_wur_user_num == 0) {
        return OAL_SUCC;
    }

    /* 组播唤醒wur sta处理 */
    hmac_wur_multi_wake_process(hmac_vap);

    /* 单播唤醒wur sta处理 */
    hmac_wur_unicast_wake_process(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 处理wur服务周期结束中断上报事件
*****************************************************************************/
osal_s32 hmac_wur_sp_end_process(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);

    hmac_vap->wur_in_service = OSAL_FALSE; /* 当前不处于WUR服务周期 */

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 判断是否有WUR帧发送完成，成功则处理
*****************************************************************************/
osal_u32 hmac_wur_frame_tx_complete_process(osal_u8 dscr_status, mac_tx_ctl_stru *cb,
    hal_to_dmac_device_stru *hal_device)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 reg = hal_get_tx_vector_word6(); /* vector word6 */
    osal_u8 wur_flag = (reg & 0x2) >> 1; /* 0x2: 取bit1是wur_flag */
    osal_u8 wur_type;
    osal_u8 group_buffered;

    unref_param(hal_device);
    /* 判断tx_vector word6的bit1是不是wur_flag且发送成功 */
    if (dscr_status == 1 && wur_flag == 1) {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(cb->tx_user_idx);
        if (hmac_user == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_TX,
                "{hmac_wur_action_tx_complete_process::hmac_user[%d] null.}", cb->tx_user_idx);
            return OAL_CONTINUE;
        }

        reg = hal_get_rpt_tx_data_word0(); /* data0 word0 */
        wur_type = (reg & 0x7); /* 0x7: 取bit0-2是wur_type */
        group_buffered = (reg & 0x20) >> 5; /* 0x20: bit5是group bu */
        if (wur_type == 1 && group_buffered == 1) {
            /* 标记组播缓存的WUR FL WAKE UP帧已发送，组播缓存计时下发 */
            if (hmac_user->wur_info.send_multi_wakeup == OSAL_FALSE) {
                hmac_user->wur_info.send_multi_wakeup = OSAL_TRUE;
            }
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_wur_send_setup_etc
 功能描述  : 发送WUR Mode Setup帧处理
*****************************************************************************/
osal_s32 hmac_config_wur_send_setup(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;

    mac_cfg_wur_param_htd_stru *wur_cfg = (mac_cfg_wur_param_htd_stru *)msg->data;

    oam_warning_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_send_setup_etc.}");

    /* ~{;qH!SC;'6TS&5DKwR}~} */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, wur_cfg->mac_addr);
    if (hmac_user == OSAL_NULL) {
        oam_error_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_send_setup_etc::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (msg->data == OSAL_NULL) {
        oam_error_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_send_setup_etc::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_mgmt_tx_wur_mode_setup(hmac_vap, hmac_user, &(wur_cfg->wur_param_htd));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_wur_send_teardown_etc
 功能描述  : 发送WUR Mode Teardown帧处理
*****************************************************************************/
osal_s32 hmac_config_wur_send_teardown(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;

    mac_cfg_wur_mode_teardown_param_stru *wur_mode_teardown = (mac_cfg_wur_mode_teardown_param_stru *)msg->data;

    oam_warning_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_receive_teardown_etc.}");

    /* ~{;qH!SC;'6TS&5DKwR}~} */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, wur_mode_teardown->mac_addr);
    if (hmac_user == OSAL_NULL) {
        oam_error_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_receive_teardown_etc::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (msg->data == OSAL_NULL) {
        oam_error_log0(hmac_vap->vap_id, OAM_SF_11AX, "{hmac_config_wur_receive_teardown_etc::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_mgmt_tx_wur_mode_teardown(hmac_vap, hmac_user);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_wur_enable_option_etc
 功能描述  : 配置WUR TWBTT发WUR BEACON帧、WUR周期开启以及发WUR唤醒帧类型
*****************************************************************************/
osal_s32 hmac_config_wur_enable_option(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_wur_enable_option_param_stru *wur_enable_option = (mac_cfg_wur_enable_option_param_stru *)msg->data;

    hmac_vap->cap_flag.wur_enable = wur_enable_option->wur_enable ? OSAL_TRUE : OSAL_FALSE;
    wur_enable_option->wur_basic_param = hmac_vap->wur_basic_param; /* hmac~{3uJ<;/4+5]5=~}dmac */

    /* 设置soft-ap的WUR功能 */
    if (!is_ap(hmac_vap)) {
        return OAL_SUCC;
    }

    /* WUR使能，开始动态申请WUR_BSS参数结构体 */
    if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE && hmac_vap->wur_bss == OSAL_NULL) {
        hmac_vap->wur_bss = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_vap_wur_bss_group_stru), OAL_TRUE);
        if (osal_unlikely(hmac_vap->wur_bss == OSAL_NULL)) {
            oam_error_log0(0, OAM_SF_11AX, "{hmac_config_wur_enable_option::Alloc memory for wur bss param failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        (osal_void)memset_s(hmac_vap->wur_bss, sizeof(mac_vap_wur_bss_group_stru),
            0, sizeof(mac_vap_wur_bss_group_stru));
        hmac_vap->wur_bss->wur_bss_last.is_first = OSAL_TRUE; /* 第一次BSS参数标记 */
    }
    /* 使能WUR相关开关 */
    hmac_wur_set_param_en(wur_enable_option->twbtt_bcn_en, wur_enable_option->wur_period_en,
        wur_enable_option->wur_frame_type);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_wur_update_event_etc
 功能描述  : 将WUR更新事件抛向dmac层
*****************************************************************************/
osal_u32 hmac_ap_wur_update_event(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_update_source_enum_uint8 wur_update_source)
{
    hmac_ctx_update_wur_stru update_wur_cfg_event;
    osal_u32 ret;

    update_wur_cfg_event.update_source = wur_update_source;
    update_wur_cfg_event.user_idx = hmac_user->assoc_id;
    update_wur_cfg_event.wur_param_htd = hmac_vap->wur_param_htd;
    update_wur_cfg_event.wur_cap_ie = hmac_user->wur_cap_ie;

    ret = hmac_wur_update(hmac_vap, &update_wur_cfg_event);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_wur_mode_setup_header
 功能描述  : 组WUR MODE SETUP帧的头部信息
*****************************************************************************/
osal_void hmac_mgmt_encap_wur_mode_setup_header(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_mode_setup_frame_stru *wur_setup_frame, osal_u8 *data)
{
    mac_wur_mode_ie_stru *wur_mode_element = &(wur_setup_frame->wur_mode_element);

    (void)memset_s((osal_u8 *)wur_setup_frame, sizeof(mac_wur_mode_setup_frame_stru),
        0, sizeof(mac_wur_mode_setup_frame_stru));

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup::da_mac_addr[%02x XX XX XX %02x %02x]!.}",
        /* 0、4表示MAC地址位 */
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[4],
        hmac_user->user_mac_addr[5]); /* 5，mac地址的第一个byte和后两个byte */

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置 DA address1: STA MAC地址 */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);

    /* 设置 SA address2: BSSID */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: BSSID */
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, mac_mib_get_station_id(hmac_vap));
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    wur_setup_frame->category = MAC_ACTION_CATEGORY_WUR;
    wur_setup_frame->wur_action = MAC_WUR_ACTION_MODE_SETUP;

    /* 传入信息元素 */
    wur_mode_element->element_id = MAC_EID_HE;
    wur_mode_element->element_id_ext = MAC_EID_EXT_WUR_MODE;
    wur_mode_element->len = sizeof(mac_wur_mode_ie_stru) - 2; /* 2: element_id + param_filed */
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_fill_wur_mode_ie_ap
 功能描述  : 组WUR MODE SETUP帧的WUR MODE element信息
*****************************************************************************/
osal_void hmac_mgmt_fill_wur_mode_ie_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_mode_setup_frame_stru *wur_setup_frame, mac_wur_param_ap_field_stru *wur_param_ap_field,
    mac_wur_param_htd_stru *wur_param_htd)
{
    mac_wur_mode_ie_stru *wur_mode_element = &(wur_setup_frame->wur_mode_element);

    (void)memset_s((osal_u8 *)wur_param_ap_field, sizeof(mac_wur_param_ap_field_stru),
                   0, sizeof(mac_wur_param_ap_field_stru));

    /* ap */
    wur_setup_frame->dialog_token = hmac_vap->wur_param_htd.dialog_token;
    wur_mode_element->len += sizeof(mac_wur_param_ap_field_stru);
    wur_mode_element->action_type = hmac_vap->wur_param_htd.action_type;
    wur_mode_element->wur_mode_rsp_status = MAC_WUR_MODE_ACCEPT;
    /* 与默认提供的WUR服务周期间隔，服务周期时长比较，换算单位:4us与256us */
    if (((wur_param_htd->cycle_period % (hmac_vap->wur_basic_param.min_cycle_period * 4)) != 0) ||
        ((wur_param_htd->cycle_service_period % (hmac_vap->wur_basic_param.min_duration * 256)) != 0)) { /* 256换算 */
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_setup_frame::ERR period}");
        wur_mode_element->wur_mode_rsp_status = MAC_WUR_MODE_DENIED_PERIOD;
    }
    wur_mode_element->wur_param_control_field.wur_duty_cycle_start_time_present = 1; /* wur ap填写开始时间 */
    wur_mode_element->wur_param_control_field.wur_group_id_list_present = 0;
    wur_mode_element->wur_param_control_field.proposed_wur_param_present = 0;
    wur_param_ap_field->wur_id = hmac_user->assoc_id; /* AID标识 */
    wur_param_ap_field->wur_channel_offset = 0;
    wur_param_ap_field->wur_duty_cycle_start_time = 0; /* 去dmac再填写 */
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_wur_mode_setup_sta
 功能描述  : 组sta发的WUR MODE SETUP帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_encap_wur_mode_setup_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, mac_wur_param_htd_stru *wur_param_htd, osal_u16 *frame_len)
{
    mac_wur_mode_setup_frame_stru wur_setup_frame;
    mac_wur_mode_ie_stru *wur_mode_element = &(wur_setup_frame.wur_mode_element);
    mac_wur_param_sta_field_stru wur_param_sta_field;
    osal_u8 wur_setup_frame_len;
    osal_u32 ret;

    /* 填充WUR MODE SETUP帧头部信息 */
    hmac_mgmt_encap_wur_mode_setup_header(hmac_vap, hmac_user, &wur_setup_frame, data);

    /* 填充WUR MODE SETUP帧的WUR MODE IE信息 */
    (void)memset_s((osal_u8 *)&wur_param_sta_field, sizeof(mac_wur_param_sta_field_stru),
                   0, sizeof(mac_wur_param_sta_field_stru));

    /* sta */
    wur_setup_frame.dialog_token = wur_param_htd->dialog_token;
    wur_mode_element->len += sizeof(mac_wur_param_sta_field_stru);
    wur_mode_element->action_type = wur_param_htd->action_type;
    wur_mode_element->wur_mode_rsp_status = MAC_WUR_MODE_RESERVED; /* wur sta reserved */
    wur_mode_element->wur_param_control_field.wur_duty_cycle_start_time_present = 0;
    wur_mode_element->wur_param_control_field.wur_group_id_list_present = 0;
    wur_mode_element->wur_param_control_field.proposed_wur_param_present = 1;
    wur_param_sta_field.wur_duty_cycle_serive_period = wur_param_htd->cycle_service_period;
    wur_param_sta_field.duty_cycle_period = wur_param_htd->cycle_period;

    /* 把组帧结果赋值给netbuf的data部分 */
    wur_setup_frame_len = sizeof(mac_wur_mode_setup_frame_stru);
    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN), wur_setup_frame_len,
                   (osal_u8 *)&wur_setup_frame, wur_setup_frame_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup::memcpy error1}");
        return OAL_FAIL;
    }

    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN + wur_setup_frame_len),
                   sizeof(mac_wur_param_sta_field_stru),
                   (osal_u8 *)&wur_param_sta_field, sizeof(mac_wur_param_sta_field_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup::memcpy error2}");
        return OAL_FAIL;
    } else {
        *frame_len = (MAC_80211_FRAME_LEN + wur_setup_frame_len + sizeof(mac_wur_param_sta_field_stru));
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_wur_mode_setup_ap
 功能描述  : 组ap发的WUR MODE SETUP帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_encap_wur_mode_setup_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, mac_wur_param_htd_stru *wur_param_htd, osal_u16 *frame_len)
{
    mac_wur_mode_setup_frame_stru wur_setup_frame;
    mac_wur_param_ap_field_stru wur_param_ap_field;
    osal_u8 wur_setup_frame_len;
    osal_u32 ret;

    /* 填充WUR MODE SETUP帧头部信息 */
    hmac_mgmt_encap_wur_mode_setup_header(hmac_vap, hmac_user, &wur_setup_frame, data);

    /* 填充WUR MODE SETUP帧的WUR MODE IE信息 */
    hmac_mgmt_fill_wur_mode_ie_ap(hmac_vap, hmac_user, &wur_setup_frame, &wur_param_ap_field, wur_param_htd);

    /* 把组帧结果赋值给netbuf的data部分 */
    wur_setup_frame_len = sizeof(mac_wur_mode_setup_frame_stru);
    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN), wur_setup_frame_len,
                   (osal_u8 *)&wur_setup_frame, wur_setup_frame_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup_ap::memcpy header error}");
        return OAL_FAIL;
    }

    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN + wur_setup_frame_len),
        sizeof(mac_wur_param_ap_field_stru), (osal_u8 *)&wur_param_ap_field, sizeof(mac_wur_param_ap_field_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup_ap::memcpy mode ie error}");
    } else {
        *frame_len = (MAC_80211_FRAME_LEN + wur_setup_frame_len + sizeof(mac_wur_param_ap_field_stru));
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_wur_mode_setup
 功能描述  : 组WUR MODE SETUP帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_encap_wur_mode_setup(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, mac_wur_param_htd_stru *wur_param_htd, osal_u16 *frame_len)
{
    osal_u32 ret;
    if ((data == OSAL_NULL) || (wur_param_htd == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup::data:%p wur:%p}", data, wur_param_htd);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wur_param_htd->wur_role == 0) {
        ret = hmac_mgmt_encap_wur_mode_setup_sta(hmac_vap, hmac_user, data, wur_param_htd, frame_len);
    } else {
        ret = hmac_mgmt_encap_wur_mode_setup_ap(hmac_vap, hmac_user, data, wur_param_htd, frame_len);
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_setup::failed [%d]}", wur_param_htd->wur_role);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_wur_mode_setup
 功能描述  : 发送WUR MODE SETUP帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_tx_wur_mode_setup(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_param_htd_stru *wur_param_htd)
{
    oal_netbuf_stru *wur_mode_setup = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    hmac_ctx_action_event_stru wlan_ctx_action = {0};
    osal_u32 ret;
    osal_u16 frame_len;  /* 帧的长度 */

    /* 申请WUR MODE SETUP管理帧内存 */
    wur_mode_setup = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (wur_mode_setup == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_setup::wur_mode_setup null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wur_mode_setup->prev = OSAL_NULL;
    wur_mode_setup->next = OSAL_NULL;

    /* 调用封装管理帧接口 */
    ret = hmac_mgmt_encap_wur_mode_setup(hmac_vap, hmac_user, oal_netbuf_data(wur_mode_setup),
        wur_param_htd, &frame_len);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_setup::encap err=%d}", ret);
        oal_netbuf_free(wur_mode_setup);
        return ret;
    }

    wlan_ctx_action.frame_len = frame_len;
    wlan_ctx_action.hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.action_category = MAC_ACTION_CATEGORY_WUR;
    wlan_ctx_action.action = MAC_WUR_ACTION_MODE_SETUP;
    wlan_ctx_action.user_idx = hmac_user->assoc_id;

    ret = memcpy_s((osal_u8 *)(oal_netbuf_data(wur_mode_setup) + frame_len), sizeof(hmac_ctx_action_event_stru),
                   (osal_u8 *)&wlan_ctx_action, sizeof(hmac_ctx_action_event_stru));
    if (ret != EOK) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, wur_mode_setup);
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_setup::MEMCOPY ERROR}");
        oal_netbuf_free(wur_mode_setup);
        return OAL_FAIL;
    }

    oal_netbuf_put(wur_mode_setup, (frame_len + sizeof(hmac_ctx_action_event_stru)));

    /* 初始化CB */
    (void)memset_s(oal_netbuf_cb(wur_mode_setup), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(wur_mode_setup);
    tx_ctl->mpdu_payload_len = frame_len + sizeof(hmac_ctx_action_event_stru);
    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_ACTION;
    tx_ctl->frame_subtype = WLAN_ACTION_WUR_MODE_SETUP;

    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, wur_mode_setup, frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_setup::event send err=%d}", ret);
    }

    return ret;
}
/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_wur_mode_teardown
 功能描述  : 组WUR MODE TEARDOWN帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_encap_wur_mode_teardown(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, osal_u16 *frame_len)
{
    mac_wur_mode_teardown_frame_stru wur_teardown_frame;
    osal_u32 ret;

    if (data == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_teardown::data:%p}", data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 规则6.6：禁止使用内存操作类危险函数 例外(1)对固定长度的数组进行初始化，或对固定长度的结构体进行内存初始化 */
    (void)memset_s((osal_u8 *)&wur_teardown_frame, sizeof(mac_wur_mode_teardown_frame_stru),
        0, sizeof(mac_wur_mode_teardown_frame_stru));

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_teardown:: mac_addr[%02x XX XX XX %02x %02x]!.}",
         /* 0、4mac地址位 */
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[4],
        hmac_user->user_mac_addr[5]);  /* 5mac地址位 */

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置 DA address1: STA MAC地址  */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);

    /* 设置 SA address2: BSSID */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: BSSID */
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, mac_mib_get_station_id(hmac_vap));
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /* WUR MODE TEARDOWN帧 无帧体 */
    wur_teardown_frame.category = MAC_ACTION_CATEGORY_WUR;
    wur_teardown_frame.wur_action = MAC_WUR_ACTION_MODE_TEARDOWN;

    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN), sizeof(mac_wur_mode_teardown_frame_stru),
                   (osal_u8 *)&wur_teardown_frame, sizeof(mac_wur_mode_teardown_frame_stru));
    if (ret == EOK) {
        *frame_len = (MAC_80211_FRAME_LEN + sizeof(mac_wur_mode_teardown_frame_stru));  /* 长度不包括FCS,安全校验 */
        return OAL_SUCC;
    } else {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_wur_mode_teardown::memcpy error}");
        return OAL_FAIL;
    }
}
/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_wur_mode_teardown
 功能描述  : 发送WUR MODE TEARDOWN帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_tx_wur_mode_teardown(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_stru *wur_mode_teardown = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    hmac_ctx_action_event_stru wlan_ctx_action = {0};
    osal_u32 ret;
    osal_u16 frame_len;  /* 帧的长度 */

    /* 申请WUR MODE SETUP管理帧内存 */
    wur_mode_teardown = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (wur_mode_teardown == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_teardown::wur_mode_teardown null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wur_mode_teardown->prev = OSAL_NULL;
    wur_mode_teardown->next = OSAL_NULL;

    /* 调用封装管理帧接口 */
    ret = hmac_mgmt_encap_wur_mode_teardown(hmac_vap, hmac_user, oal_netbuf_data(wur_mode_teardown), &frame_len);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_teardown::encap err=%d}", ret);
        oal_netbuf_free(wur_mode_teardown);
        return ret;
    }

    wlan_ctx_action.frame_len = frame_len;
    wlan_ctx_action.hdr_len = MAC_80211_FRAME_LEN;
    wlan_ctx_action.action_category = MAC_ACTION_CATEGORY_WUR;
    wlan_ctx_action.action = MAC_WUR_ACTION_MODE_TEARDOWN;
    wlan_ctx_action.user_idx = hmac_user->assoc_id;

    ret = memcpy_s((osal_u8 *)(oal_netbuf_data(wur_mode_teardown) + frame_len), sizeof(hmac_ctx_action_event_stru),
        (osal_u8 *)&wlan_ctx_action, sizeof(hmac_ctx_action_event_stru));
    if (ret != EOK) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, wur_mode_teardown);
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_teardown::MEMCOPY ERROR}");
        oal_netbuf_free(wur_mode_teardown);
        return OAL_FAIL;
    }

    oal_netbuf_put(wur_mode_teardown, (frame_len + sizeof(hmac_ctx_action_event_stru)));

    /* 初始化CB */
    (void)memset_s(oal_netbuf_cb(wur_mode_teardown), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(wur_mode_teardown);
    tx_ctl->mpdu_payload_len = frame_len + sizeof(hmac_ctx_action_event_stru);
    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_ACTION;
    tx_ctl->frame_subtype = WLAN_ACTION_WUR_MODE_TEARDOWN;

    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, wur_mode_teardown, frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_wur_mode_teardown::event send err=%d}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_wur_param_update
 功能描述  : 从空口接收WUR MODE SETUP帧,对参数进行更新处理的函数
*****************************************************************************/
osal_u32 hmac_ap_rx_wur_param_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    mac_wur_mode_setup_frame_stru *wur_setup_frame = (mac_wur_mode_setup_frame_stru *)payload;
    mac_wur_param_sta_field_stru *wur_param_sta_field = (mac_wur_param_sta_field_stru *)(payload +
        sizeof(mac_wur_mode_setup_frame_stru));
    osal_u8 action_type = wur_setup_frame->wur_mode_element.action_type;
    osal_u16 cycle_period_unit = 4 * hmac_vap->wur_basic_param.cycle_period_unit; /* 单位：4us */
    osal_u32 cycle_period = wur_param_sta_field->duty_cycle_period * cycle_period_unit;
    osal_u32 cycle_service_period = wur_param_sta_field->wur_duty_cycle_serive_period;
    osal_u32 ret;

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_ap_rx_wur_param_update::action_type:%d, serive_period:%d, period:%d}",
        action_type, cycle_service_period, cycle_period);

    /* WUR STA 服务周期参数记录 */
    hmac_vap->wur_param_htd.wur_role = 1; /* AP */
    hmac_vap->wur_param_htd.dialog_token = wur_setup_frame->dialog_token;
    hmac_vap->wur_param_htd.action_type = action_type;
    hmac_vap->wur_param_htd.cycle_period = cycle_period;
    hmac_vap->wur_param_htd.cycle_service_period = cycle_service_period;

    oam_warning_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_param_update::token:[%d]}",
        wur_setup_frame->dialog_token);

    /* 事件分发至dmac，更新wur相关参数 */
    ret = hmac_ap_wur_update_event(hmac_vap, hmac_user, MAC_WUR_UPDATE_SOURCE_SETUP);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_param_update::update failed error=[%d]}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_wur_mode_setup_frame
 功能描述  : 从空口接收WUR MODE SETUP帧的处理函数
*****************************************************************************/
osal_u32 hmac_ap_rx_wur_mode_setup_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    mac_wur_mode_setup_frame_stru *wur_setup_frame = (mac_wur_mode_setup_frame_stru *)payload;
    osal_u8 action_type = wur_setup_frame->wur_mode_element.action_type;
    osal_u32 ret;

    /* WUR 参数校验 */
    /* 非WUR element，错误返回 */
    if ((wur_setup_frame->wur_mode_element.element_id != MAC_EID_HE) ||
        (wur_setup_frame->wur_mode_element.element_id_ext != MAC_EID_EXT_WUR_MODE)) {
        oam_warning_log2(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_setup_frame:ERR id=%d, id_ext=%d}",
            wur_setup_frame->wur_mode_element.element_id, wur_setup_frame->wur_mode_element.element_id_ext);
        return OAL_FAIL;
    }

    /* action type 校验，错误返回 */
    if ((action_type > MAC_WUR_ENTER_WUR_MODE) || (action_type == MAC_WUR_ENTER_WUR_MODE_RSP) ||
        (action_type == MAC_WUR_ENTER_WUR_MODE_SUSPEND_RSP)) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_setup_frame::action_type ERR=%d}", action_type);
        return OAL_FAIL;
    }

    /* WUR帧相关参数的更新 */
    ret = hmac_ap_rx_wur_param_update(hmac_vap, hmac_user, payload);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_setup_frame::update failed error=[%d]}", ret);
        return ret;
    }

    /* 解析完成后，若是直接状态改变帧，改变user状态不需响应;若是请求帧，准备发送WUR MODE SETUP帧响应 */
    if ((action_type == MAC_WUR_ENTER_WUR_MODE) || (action_type == MAC_WUR_ENTER_WUR_MODE_SUSPEND)) {
        return ret;
    } else if (action_type == MAC_WUR_ENTER_WUR_MODE_REQ) {
        hmac_vap->wur_param_htd.action_type = MAC_WUR_ENTER_WUR_MODE_RSP;
    } else if (action_type == MAC_WUR_ENTER_WUR_MODE_SUSPEND_REQ) {
        hmac_vap->wur_param_htd.action_type = MAC_WUR_ENTER_WUR_MODE_SUSPEND_RSP;
    }

    /* 回复WUR mode setup帧 */
    ret = hmac_mgmt_tx_wur_mode_setup(hmac_vap, hmac_user, &hmac_vap->wur_param_htd);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_setup_frame:: tx wur mode frame failed =[%d]}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_wur_mode_teardown_frame
 功能描述  : 从空口接收WUR MODE TEARDOWN帧的处理函数
*****************************************************************************/
osal_u32 hmac_ap_rx_wur_mode_teardown_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;

    oam_warning_log0(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_teardown_frame.}");

    /* 事件分发至dmac，更新wur相关状态机 */
    ret = hmac_ap_wur_update_event(hmac_vap, hmac_user, MAC_WUR_UPDATE_SOURCE_TEARDOWN);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_ap_rx_wur_mode_teardown_frame::update failed error=[%d]}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_wur_mode_teardown_frame
 功能描述  :解析WUR能力字段的处理函数
*****************************************************************************/
osal_u32 hmac_proc_wur_cap_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *wur_cap_ie)
{
    osal_u32 ret;

    /* 把user信息存到hmac_user在将数据同步到dmac_user */
    oam_warning_log3(0, OAM_SF_ASSOC, "{hmac_proc_wur_cap_ie::wur cap ie, bands[0x%x] cap_info[0x%x]-[0x%x]}",
                     wur_cap_ie[3], wur_cap_ie[4], wur_cap_ie[5]); /* 3:bands 4-5:cap_info */

    (void)memset_s((osal_u8 *)&(hmac_user->wur_cap_ie), sizeof(mac_user_wur_cap_ie_stru),
                   0, sizeof(mac_user_wur_cap_ie_stru));

    ret = memcpy_s(&(hmac_user->wur_cap_ie), sizeof(mac_user_wur_cap_ie_stru),
                   wur_cap_ie + MAC_IE_EXT_HDR_LEN, sizeof(mac_user_wur_cap_ie_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_proc_wur_cap_ie::MEMCOPY ERROR}");
        return ret;
    }

    /* 事件分发至dmac，更新wur相关能力信息 */
    ret = hmac_ap_wur_update_event(hmac_vap, hmac_user, MAC_WUR_UPDATE_SOURCE_ASSOC);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_proc_wur_cap_ie::update failed error=[%d]}", ret);
    }
    return ret;
}

osal_void hmac_ap_up_rx_action_wur(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *data)
{
    /* 检查能力开关 */
    if (hmac_vap->cap_flag.wur_enable == OAL_TRUE) {
        if (data[MAC_ACTION_OFFSET_ACTION] == MAC_WUR_ACTION_MODE_SETUP) {
            hmac_ap_rx_wur_mode_setup_frame(hmac_vap, hmac_user, data);
        } else if (data[MAC_ACTION_OFFSET_ACTION] == MAC_WUR_ACTION_MODE_TEARDOWN) {
            hmac_ap_rx_wur_mode_teardown_frame(hmac_vap, hmac_user);
        } else {
            oam_warning_log1(0, OAM_SF_RX, "{hmac_ap_up_rx_action_wur::WUR action code[%d] invalid.}",
                data[MAC_ACTION_OFFSET_ACTION]);
        }
    }
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
