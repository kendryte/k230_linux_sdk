/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * 文 件 名   : hmac_twt.c
 * 生成日期   : 2023年7月29日
 * 功能描述   : twt
 */

/* 1 头文件包含 */
#include "hmac_twt.h"
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_tx_mgmt.h"
#include "hmac_psm_ap.h"
#include "hmac_feature_dft.h"

#include "hal_device.h"

#include "mac_frame.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "hmac_config.h"
#include "msg_twt_rom.h"
#include "hmac_ccpriv.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_hook.h"
#include "oal_netbuf_data.h"
#include "wal_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TWT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/* 2 全局变量定义 */
sta_twt_para_stru *g_sta_twt_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL};

/* 函数声明 */

/* 3 函数实现 */
WIFI_TCM_TEXT OSAL_STATIC sta_twt_para_stru *hmac_get_sta_twt_info(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }
    return g_sta_twt_info[vap_id];
}

WIFI_TCM_TEXT OSAL_STATIC osal_u8 hmac_device_twt_custom_is_open(osal_u8 vap_id)
{
    if (hmac_get_sta_twt_info(vap_id) == OSAL_NULL) {
        return OSAL_FALSE;
    }
    return ((g_sta_twt_info[vap_id]->sta_twt_custom_para.twt_responder_support == OSAL_TRUE) ||
            (g_sta_twt_info[vap_id]->sta_twt_custom_para.twt_requester_support == OSAL_TRUE));
}

OSAL_STATIC osal_void hmac_twt_set_resp_bit(osal_u8 vap_id, osal_u8 value)
{
    if (hmac_get_sta_twt_info(vap_id) == OSAL_NULL) {
        return;
    }
    g_sta_twt_info[vap_id]->sta_twt_custom_para.twt_responder_support = value;
}

OSAL_STATIC osal_void hmac_twt_set_req_bit(osal_u8 vap_id, osal_u8 value)
{
    if (hmac_get_sta_twt_info(vap_id) == OSAL_NULL) {
        return;
    }
    g_sta_twt_info[vap_id]->sta_twt_custom_para.twt_requester_support = value;
}

OSAL_STATIC osal_u8 hmac_twt_get_req_bit(osal_u8 vap_id)
{
    if (hmac_get_sta_twt_info(vap_id) == OSAL_NULL) {
        return OSAL_FALSE;
    }
    return g_sta_twt_info[vap_id]->sta_twt_custom_para.twt_requester_support;
}

/*****************************************************************************
 函 数 名  : hmac_twt_handle_ps
 功能描述  : handle twt start/end interrupt
*****************************************************************************/
OSAL_STATIC osal_void hmac_twt_handle_ps(hmac_vap_stru *hmac_vap, osal_u8 pause)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 mac_vap_id_old = hmac_device->mac_vap_id;
    sta_twt_para_stru *twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);

    if (twt_info->sta_cfg_twt_para.twt_ps_pause == pause) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_twt_handle_ps::hmac_vap->twt_cfg.twt_ps_pause == pause[%d].}", pause);
        return;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_twt_handle_ps::hmac_user[%d] null.}", hmac_vap->assoc_vap_id);
        return;
    }

    /* TWT SP之外，关闭收发 */
    if (pause == OSAL_TRUE) {
        /* 记录当前twt节能状态 */
        twt_info->sta_cfg_twt_para.twt_ps_pause = OSAL_TRUE;
        hmac_user_pause(hmac_user);

        /* 告知MAC用户进入节能模式, 记录睡眠时vap id */
        hmac_device->mac_vap_id = hmac_vap->vap_id;
    } else {
        hmac_user_resume(hmac_user); /* 恢复该用户的硬件队列的发送 */
        /* 记录当前twt节能状态 */
        twt_info->sta_cfg_twt_para.twt_ps_pause = OSAL_FALSE;
        /* 将缓存队列的数据发送 */
        hmac_psm_queue_flush(hmac_vap, hmac_user);
    }
    if (mac_vap_id_old != hmac_device->mac_vap_id) {
        /* 如果休眠vap_id发生变化，则同步 */
        hmac_device_sync(hmac_device);
    }
}

OSAL_STATIC osal_void hmac_twt_status_d2h_sync(hmac_vap_stru *hmac_vap, mac_d2hd_twt_cfg_stru *twt_cfg)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hmac_user_stru *hmac_user = OSAL_NULL;
    sta_twt_para_stru *sta_twt_info = OSAL_NULL;

    if (hmac_device_twt_custom_is_open(hmac_vap->vap_id) != OSAL_TRUE) {
        return;
    }

    sta_twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    sta_twt_info->sta_cfg_twt_para.twt_session_status = twt_cfg->twt_session_status;
    sta_twt_info->sta_cfg_twt_para.twt_ps_pause = OSAL_FALSE;
    hal_device->twt_session_enable =
        (sta_twt_info->sta_cfg_twt_para.twt_session_status == TWT_SESSION_ON) ? OSAL_TRUE : OSAL_FALSE;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(twt_cfg->user_idx);
    if (hmac_user != OSAL_NULL) {
        /* 恢复tid队列 */
        hmac_user_resume(hmac_user);

        /* 释放节能队列 */
        hmac_psm_queue_flush(hmac_vap, hmac_user);
    } else {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_twt_status_d2hd_sync:: find dmac user fail!.}");
    }
}

OSAL_STATIC osal_s32 hmac_twt_update_d2h_sync(hmac_vap_stru *hmac_vap, mac_d2hd_twt_sync_info_stru *twt_cfg)
{
    switch (twt_cfg->cfg_type) {
        case TWT_UPDATE_SESSION_CFG:
            hmac_twt_status_d2h_sync(hmac_vap, &(twt_cfg->twt_cfg));
            break;
        case TWT_UPDATE_PS_PAUSE_CFG:
            hmac_get_sta_twt_info(hmac_vap->vap_id)->sta_cfg_twt_para.twt_ps_pause = OSAL_TRUE;
            break;
        case TWT_PS_HANDLE_CFG:
            hmac_twt_handle_ps(hmac_vap, twt_cfg->twt_cfg.twt_ps_pause);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_twt_is_ps_pause
 功能描述  : 判断TWT是否处于省电发送暂停状态
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u8 hmac_twt_is_ps_pause(const hmac_vap_stru *hmac_vap)
{
    return (hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para.twt_ps_pause;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_twt_setup_req
 功能描述  : TWT SETUP函数，需要将HMAC模块的生成的信息, 在DMAC稍作修改
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_setup_req(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf)
{
    mac_individual_twt_setup_frame_stru *setup_frame = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u32 ret;

    /* 如果dbac或同频同信道启动P2P，则不建立twt会话 */
    if (hal_device_calc_up_vap_num(hmac_vap->hal_device) >= 2) { /* 2: TWT和P2P不能共存 */
        oam_warning_log0(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_req:: p2p is running, do not setup twt session.}");
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return OAL_FAIL;
    }

    /* 将索引指向frame body起始位置 */
    data = oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);
    setup_frame = (mac_individual_twt_setup_frame_stru *)(data);
    setup_frame->twt_element.twt = 0;

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, ctx_action_event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_setup_req::hmac_tx_mgmt return Err=%d.}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : TWT information函数，需要将HMAC模块的生成的next-twt信息, 在DMAC作修改
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_information_req(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1;
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, ctx_action_event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_information_req::hmac_tx_mgmt return Err=%d.}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : send TWT teardown frame
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_teardown_req(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;

    osal_u32 ret;

    /* 将索引指向frame body起始位置 */
    data = oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);
    oam_warning_log3(0, OAM_SF_11AX, "{tx teardown frame:CATEGORY[%02x] ACTION[%02x] FLOW ID[%02x]!.}",
        data[HMAC_TWT_CATEGORY], data[HMAC_TWT_ACTION], data[HMAC_TWT_FLOW_ID]);

    /* 填写netbuf的cb字段，共发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    tx_ctl->mpdu_num = 1; /* 管理帧只有一个 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, ctx_action_event->frame_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_mgmt_tx_twt_teardown_req::hmac_tx_mgmt return Err=%d.}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_sta_twt_update_event_etc
 功能描述  : 将twt更新事件抛向dmac层，teardown，setup，information，association处理的最终汇合
 输入参数  : hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_ctx_update_twt_stru *update_twt_cfg
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_twt_update_event_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_update_source_enum_uint8 twt_update_source)
{
    mac_ctx_update_twt_stru update_twt_cfg_event;
    osal_s32 ret;
    frw_msg msg_info = {0};

    update_twt_cfg_event.update_source = twt_update_source;
    update_twt_cfg_event.user_idx      = hmac_user->assoc_id;
    update_twt_cfg_event.twt_cfg       = (hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para;

    /* 只有twt information帧才有next_twt_size */
    if (update_twt_cfg_event.update_source != MAC_TWT_UPDATE_SOURCE_INFORMATION) {
        update_twt_cfg_event.twt_cfg.next_twt_size = 0;
    }
    msg_info.data = (osal_u8 *)&update_twt_cfg_event;
    msg_info.data_len = sizeof(mac_ctx_update_twt_stru);

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_UPDATE_TWT, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_twt_update_event_etc::frw_send_msg_to_device failed[%d]!.}", ret);
    }

    return (osal_u32)ret;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : mac_set_twt_element_field_etc
 功能描述  : 用于设置TWT 信息元素位，TWT_SETUP与ASSOCIATION均会调用
 输入参数  : mac_twt_ie_individual * twt_element，mac_cfg_twt_basic_param_stru *twt_basic_param
*****************************************************************************/
OSAL_STATIC osal_void mac_set_twt_element_field_etc(mac_twt_ie_individual_stru *twt_element,
    mac_cfg_twt_basic_param_stru *twt_basic_param)
{
    twt_element->element_id = MAC_EID_TWT;
    twt_element->len = sizeof(mac_twt_ie_individual_stru) - 2; /* control之前有2 byte数据 */

    twt_element->control.ndp_paging_indicator   = 0;
    twt_element->control.responder_pm_mode      = 0;
    twt_element->control.negotiation            = 0;  /* 默认支持单播TWT */
    twt_element->control.twt_info_frame_disable = 0;  /* 默认支持twt information帧 */
    twt_element->control.wake_duration_unit     = twt_basic_param->wake_duration_unit; /* min_duration的单位 */
    twt_element->control.resv                   = 0;

    twt_element->request_type.request          = 1;
    twt_element->request_type.setup_command    = twt_basic_param->setup_command; /* 由user决定 */
    twt_element->request_type.trigger          = twt_basic_param->trigger;
    twt_element->request_type.implicit         = 1;  /* 默认隐式TWT */
    twt_element->request_type.flow_type        = twt_basic_param->flow_type;
    twt_element->request_type.flow_id          = twt_basic_param->flow_id;   /* 由user决定 */
    twt_element->request_type.intrval_exponent = twt_basic_param->intrval_exponent; /* unit:us */
    twt_element->request_type.protection       = 0;

    twt_element->twt              = twt_basic_param->twt;
    twt_element->min_duration     = twt_basic_param->min_duration;     /* user decide, unit: 256us */
    twt_element->intrval_mantissa = twt_basic_param->intrval_mantissa; /* user decide, unit:us */
    twt_element->channel          = 0;

    /* 输出维测信息 */
    oam_warning_log4(0, OAM_SF_11AX, "{mac_set_twt_element_field_etc::command=%d,flowType=%d,flowId=%d,trigger=%d}",
        twt_element->request_type.setup_command, twt_element->request_type.flow_type,
        twt_element->request_type.flow_id, twt_element->request_type.trigger);

    oam_warning_log4(0, OAM_SF_11AX, "{mac_set_twt_element_field_etc::twt=%d,duration=%d,exponent=%d,mantissa=%d}",
        twt_element->twt, twt_element->min_duration,
        twt_element->request_type.intrval_exponent, twt_element->intrval_mantissa);
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_twt_setup_req
 功能描述  : 组twt_setup帧
 输入参数  : hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
             osal_u8 *data, mac_cfg_twt_basic_param_stru *twt_basic_param
 输出参数  : twt setup帧不包含FCS字段的MAC帧长度
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_encap_twt_setup_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, mac_cfg_twt_basic_param_stru *twt_basic_param, osal_u16 *frame_len)
{
    mac_individual_twt_setup_frame_stru setup_frame;
    errno_t ret;

    if ((data == OSAL_NULL) || (twt_basic_param == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req null param}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para.dialog_token++;
    oam_warning_log1(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req: sta_cfg_twt_para.dialog=%d}",
        (hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para.dialog_token);

    /* 规则6.6：禁止使用内存操作类危险函数 例外(1)对固定长度的数组进行初始化，或对固定长度的结构体进行内存初始化 */
    (void)memset_s((osal_u8 *)&setup_frame, sizeof(mac_individual_twt_setup_frame_stru),
        0, sizeof(mac_individual_twt_setup_frame_stru));

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req:: mac_addr[%02x XX XX XX %02x %02x]!.}",
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[4], /* 4 */
        hmac_user->user_mac_addr[5]); /* 5 */

    /******************************************************************************************************************/
    /*                   TWT element                                                                                  */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Control |TWT Parameter Information|                                                       */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1          |1      |1       |variable                 |                                                       */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置 DA address1: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /******************************************************************************************************************/
    /*                   TWT argument                                                                                 */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |setup_cmd |flow_type |flow_ID |Reserve |                                                                      */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1               |1             |1          |1           |                                                     */
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    setup_frame.category     = MAC_ACTION_CATEGORY_S1G;
    setup_frame.action       = MAC_S1G_ACTION_TWT_SETUP;
    setup_frame.dialog_token = (hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para.dialog_token;

    /* 传入信息元素 */
    mac_set_twt_element_field_etc(&(setup_frame.twt_element), twt_basic_param);

    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN), sizeof(mac_individual_twt_setup_frame_stru),
                   (osal_u8 *)&setup_frame, sizeof(mac_individual_twt_setup_frame_stru));
    if (ret == EOK) {
        *frame_len = (MAC_80211_FRAME_LEN + sizeof(mac_individual_twt_setup_frame_stru));  /* 长度不包括FCS,安全校验 */
        return OAL_SUCC;
    } else {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req::memcpy error}");
        return OAL_FAIL;
    }
}

/*****************************************************************************
 功能描述  : 组装twt_information_req帧
*****************************************************************************/
OSAL_STATIC osal_void hmac_mgmt_encap_twt_information_req(hmac_vap_stru *hmac_vap, osal_u8 *data,
    mac_twt_information_field_stru *information_filed, osal_u16 *frame_len)
{
    mac_twt_information_frame_stru *information_frame = OSAL_NULL;
    osal_u16 information_size;

    if (data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_information_req_etc::null data}");
        *frame_len = 0;
        return;
    }

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置 DA address1: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    information_frame = (mac_twt_information_frame_stru *)(data + MAC_80211_FRAME_LEN);
    information_frame->category = MAC_ACTION_CATEGORY_S1G;
    information_frame->action = MAC_S1G_ACTION_TWT_INFORMATION;

    information_frame->twt_information_filed.flow_id = information_filed->flow_id;
    information_frame->twt_information_filed.next_twt_subfield_size = information_filed->next_twt_subfield_size;
    information_frame->twt_information_filed.response_requested = 0;   /* 暂停/重启都无需对端回复information */
    information_frame->twt_information_filed.next_twt_request = 0;     /* 暂停/重启都无需对端提供next_twt */
    information_frame->twt_information_filed.all_twt = 0;              /* 只有一套参数 */

    /* next-size有0（暂停，不带twt）和3（恢复，带64bit的twt参数两种情况 */
    if (information_filed->next_twt_subfield_size == 0) {
        information_size = sizeof(mac_twt_information_frame_stru) - 8; /* 8表示减去next-twt的8字节 */
    } else {
        information_size = sizeof(mac_twt_information_frame_stru);
    }

    *frame_len = (MAC_80211_FRAME_LEN + information_size);   /* 返回的帧长度中不包括FCS */
}
#endif

OSAL_STATIC osal_u32 hmac_twt_fill_ctx_and_cb(hmac_vap_stru *hmac_vap, oal_netbuf_stru *twt_frame,
    osal_u16 length, osal_u32 sub_type, osal_u32 action_sub_type)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    hmac_ctx_action_event_stru wlan_ctx_action = {0};
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    errno_t ret;

    if (hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 赋值要传入Dmac的信息 */
    wlan_ctx_action.frame_len       = length;
    wlan_ctx_action.hdr_len         = MAC_80211_FRAME_LEN;
    wlan_ctx_action.action_category = MAC_ACTION_CATEGORY_S1G;
    wlan_ctx_action.action          = (osal_u8)action_sub_type;
    wlan_ctx_action.user_idx        = hmac_user->assoc_id;

    ret = memcpy_s((osal_u8 *)(oal_netbuf_data(twt_frame) + length), sizeof(hmac_ctx_action_event_stru),
                   (osal_u8 *)&wlan_ctx_action, sizeof(hmac_ctx_action_event_stru));
    if (ret != EOK) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, twt_frame);
        oam_error_log2(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_twt_fill_ctx_and_cb::memcpy err=%d}", hmac_vap->vap_id, ret);
        oal_netbuf_free(twt_frame);
        return OAL_FAIL;
    }

    oal_netbuf_put(twt_frame, (length + sizeof(hmac_ctx_action_event_stru)));

    /* 初始化CB */
    (void)memset_s(oal_netbuf_cb(twt_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(twt_frame);
    tx_ctl->mpdu_payload_len = length + sizeof(hmac_ctx_action_event_stru);
    tx_ctl->frame_type = WLAN_CB_FRAME_TYPE_ACTION;
    tx_ctl->frame_subtype = (wlan_cb_frame_subtype_enum_uint8)sub_type;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 功能描述  : 组装twt_information_req帧发送
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_information(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_information_field_stru *information_filed)
{
    oal_netbuf_stru    *twt_information_req    = OSAL_NULL;
    osal_u16 length;    /* 帧长度 */
    osal_u32 ret;

    unref_param(hmac_user);

    /* 申请TWT INFORMATION管理帧内存 */
    twt_information_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (twt_information_req == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_mgmt_tx_twt_information::information_req null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_information_req->prev = OSAL_NULL;
    twt_information_req->next = OSAL_NULL;

    /* 调用封装管理帧接口 */
    hmac_mgmt_encap_twt_information_req(hmac_vap, oal_netbuf_data(twt_information_req), information_filed, &length);
    if (length == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_mgmt_tx_twt_information::encap err=%d}", hmac_vap->vap_id);
        oal_netbuf_free(twt_information_req);
        return OAL_FAIL;
    }

    ret = hmac_twt_fill_ctx_and_cb(hmac_vap, twt_information_req, length,
        WLAN_ACTION_TWT_INFORMATION_REQ, MAC_S1G_ACTION_TWT_INFORMATION);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, twt_information_req, length); /* 执行失败再函数内释放内存 */
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_mgmt_tx_twt_information::event send err=%d}", hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 组twt_setup帧发送
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_setup(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_cfg_twt_basic_param_stru *twt_basic_param)
{
    oal_netbuf_stru    *twt_setup_req          = OSAL_NULL;
    osal_u32 ret;
    osal_u16 frame_len;  /* 帧的长度 */

    /* 申请TWT SETUP管理帧内存 */
    twt_setup_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (twt_setup_req == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_setup_req->prev = OSAL_NULL;
    twt_setup_req->next = OSAL_NULL;

    /* 调用封装管理帧接口 */
    ret = hmac_mgmt_encap_twt_setup_req(hmac_vap, hmac_user, oal_netbuf_data(twt_setup_req),
        twt_basic_param, &frame_len);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_mgmt_tx_twt_setup::encap err=%d}", hmac_vap->vap_id, ret);
        oal_netbuf_free(twt_setup_req);
        return ret;
    }

    ret = hmac_twt_fill_ctx_and_cb(hmac_vap, twt_setup_req, frame_len,
        WLAN_ACTION_TWT_SETUP_REQ, MAC_S1G_ACTION_TWT_SETUP);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return hmac_tx_mgmt_send_event_etc(hmac_vap, twt_setup_req, frame_len);
}
#endif

/*****************************************************************************
 功能描述  : 组装twt_teardown_req帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_encap_twt_teardown_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *data, osal_u8 flow_id, osal_u16 *frame_len)
{
    mac_twt_teardown_stru twt_teardown;
    errno_t ret;

    if (data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_teardown_req::null data}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = memset_s((osal_u8 *)&twt_teardown, OAL_SIZEOF(mac_twt_teardown_stru), 0, OAL_SIZEOF(mac_twt_teardown_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_teardown_req::MEMSET Teardown Frame ERROR}");
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_setup_req:: mac_addr[%02x XX XX XX %02x %02x]!.}",
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[4], /* 4 */
        hmac_user->user_mac_addr[5]); /* 5 */

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置 DA address1: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(data + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(data + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    twt_teardown.category = MAC_ACTION_CATEGORY_S1G;
    twt_teardown.action   = MAC_S1G_ACTION_TWT_TEARDOWN;

    twt_teardown.flow_id          = flow_id; /* 由user决定 */
    twt_teardown.resv1            = 0;
    twt_teardown.nego_type        = 0; /* 默认设置为单播TWT的teardown */
    twt_teardown.teardown_all_twt = 0;

    ret = memcpy_s((osal_u8 *)(data + MAC_80211_FRAME_LEN), sizeof(mac_twt_teardown_stru),
                   (osal_u8 *)&twt_teardown, sizeof(mac_twt_teardown_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_mgmt_encap_twt_teardown_req::memcpy error}");
        return OAL_FAIL;
    }

    *frame_len = (MAC_80211_FRAME_LEN + sizeof(mac_twt_teardown_stru));   /* 返回的帧长度中不包括FCS */
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 组装twt_teardown_req帧发送
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_mgmt_tx_twt_teardown(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 flow_id)
{
    oal_netbuf_stru    *twt_teardown_req          = OSAL_NULL;
    osal_u16 length;    /* 帧长度 */
    osal_u32 ret;

    /* 申请TWT TEARDOWN管理帧内存 */
    twt_teardown_req = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (twt_teardown_req == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] hmac_mgmt_tx_twt_teardown::malloc null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_teardown_req->prev = OSAL_NULL;
    twt_teardown_req->next = OSAL_NULL;

    /* 调用封装管理帧接口 */
    ret = hmac_mgmt_encap_twt_teardown_req(hmac_vap, hmac_user, oal_netbuf_data(twt_teardown_req), flow_id, &length);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_mgmt_tx_twt_teardown::encap err=%d}", hmac_vap->vap_id, ret);
        oal_netbuf_free(twt_teardown_req);
        return ret;
    }

    ret = hmac_twt_fill_ctx_and_cb(hmac_vap, twt_teardown_req, length,
        WLAN_ACTION_TWT_TEARDOWN_REQ, MAC_S1G_ACTION_TWT_TEARDOWN);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, twt_teardown_req, length);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_11AX, "vap[%d] hmac_mgmt_tx_twt_teardown::event send err=%d", hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述 : 寻找处在UP状态的 wlan sta
*****************************************************************************/
OSAL_STATIC osal_u32 mac_device_find_up_sta_wlan_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap)
{
    osal_u8 vap_idx;
    hmac_vap_stru *mac_vap_tmp = OSAL_NULL;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        mac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (mac_vap_tmp == OSAL_NULL) {
            oam_warning_log1(0, OAM_SF_SCAN, "mac_device_find_up_sta_wlan_etc::vap is null! vap id is %d",
                hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (((mac_vap_tmp->vap_state == MAC_VAP_STATE_UP) || (mac_vap_tmp->vap_state == MAC_VAP_STATE_PAUSE)) &&
            (mac_vap_tmp->p2p_mode == WLAN_LEGACY_VAP_MODE)) {
            *hmac_vap = mac_vap_tmp;
            return OAL_SUCC;
        }
    }

    *hmac_vap = OSAL_NULL;
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 从twt element中提取twt参数，存储到hmac vap中
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_twt_save_parameters(hmac_vap_stru *hmac_vap,
    mac_twt_ie_individual_stru *twt_element)
{
    osal_u32 wake_unit[2] = {WAKE_DURATION_UNIT0, WAKE_DURATION_UNIT1}; /* 2:数组表示时间单位，0与1分别表示256与1024us */
    sta_twt_para_stru *twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    /*************************************************************************/
    /*                  TWT Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1      |      Variable             |           */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    twt_info->sta_cfg_twt_para.twt_basic_param.setup_command = twt_element->request_type.setup_command;
    twt_info->sta_cfg_twt_para.twt_basic_param.flow_type     = twt_element->request_type.flow_type;
    twt_info->sta_cfg_twt_para.twt_basic_param.flow_id       = twt_element->request_type.flow_id;
    twt_info->sta_cfg_twt_para.twt_basic_param.trigger       = twt_element->request_type.trigger;

    twt_info->sta_cfg_twt_para.twt_basic_param.min_duration     = twt_element->min_duration;
    twt_info->sta_cfg_twt_para.twt_basic_param.intrval_exponent = twt_element->request_type.intrval_exponent;
    twt_info->sta_cfg_twt_para.twt_basic_param.intrval_mantissa = twt_element->intrval_mantissa;
    twt_info->sta_cfg_twt_para.twt_basic_param.twt              = twt_element->twt;

    twt_info->sta_cfg_twt_para.twt_interval = (osal_u32)twt_element->intrval_mantissa <<
                                     twt_element->request_type.intrval_exponent;

    /* wake_duration_unit为1时单位为1024us，为0时为256us */
    twt_info->sta_cfg_twt_para.twt_duration = ((osal_u32)twt_element->min_duration) *
                                     wake_unit[twt_element->control.wake_duration_unit];
    twt_info->sta_cfg_twt_para.twt_basic_param.information_disable = twt_element->control.twt_info_frame_disable;

    twt_info->sta_cfg_twt_para.twt_session_status = TWT_SESSION_ON;
}

/*****************************************************************************
 函 数 名  : hmac_sta_rx_twt_negotiate
 功能描述  : 根据协商命令字段进一步处理，来源有twt_setup以及association
 修改      : 传入的参数为mac_twt_ie_individual_stru类型，同时要告知来源是association还是twt_setup
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_rx_twt_negotiate(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_twt_ie_individual_stru *twt_element, mac_twt_update_source_enum_uint8 twt_update_source)
{
    mac_twt_command_enum_uint8 command = (mac_twt_command_enum_uint8)twt_element->request_type.setup_command;
    osal_u32 ret;

    /* 协商开始 */
    switch (command) {
        case MAC_TWT_COMMAND_ACCEPT:
        case MAC_TWT_COMMAND_ALTERNATE:
            /* 保存参数并发事件 */
            hmac_sta_twt_save_parameters(hmac_vap, twt_element);
            ret = hmac_sta_twt_update_event_etc(hmac_vap, hmac_user, twt_update_source);
            if (ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_11AX,
                               "{hmac_sta_rx_twt_negotiate::hmac_sta_twt_update_event_etc return err=[%d]}", ret);
                return ret;
            }
            break;

        /* 以下内容待补充 */
        case MAC_TWT_COMMAND_DICTATE:
        case MAC_TWT_COMMAND_REJECT:
            oam_info_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_negotiate:: command=%d}", command);
            break;

        default:
            oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_negotiate:: invalid command =[%d]}", command);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 在association response中处理twt元素
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_rx_twt_association_frame_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    mac_twt_ie_individual_stru *twt_element = (mac_twt_ie_individual_stru *)payload;
    osal_u64 twt_interval;
    osal_u32 twt_wake_duration, ret;
    osal_u32 wake_unit[2] = {WAKE_DURATION_UNIT0, WAKE_DURATION_UNIT1}; /* 2:数组表示时间单位，0与1分别表示256与1024us */

    oam_warning_log4(0, OAM_SF_11AX,
        "{hmac_sta_rx_twt_accociation_frame_etc:: flow_id:%d, flow_type:%d, trigger:%d, setup_command:%d}",
        twt_element->request_type.flow_id, twt_element->request_type.flow_type, twt_element->request_type.trigger,
        twt_element->request_type.setup_command);

    /* 长度校验! */
    if (twt_element->len != (sizeof(mac_twt_ie_individual_stru) - 2)) { /* 控制位之前有2个byte */
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_association_frame_etc::error len:%d}", twt_element->len);
        return;
    }

    /* 检测TWT命令字段，MAC_TWT_ACCEPT以内错误 */
    if (twt_element->request_type.setup_command < MAC_TWT_COMMAND_ACCEPT) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_association_frame_etc::error command=%d}",
                       twt_element->request_type.setup_command);
        return;
    }

    /* interval及min_duration获取，min_duration单位需要判断 */
    twt_interval = (osal_u64)twt_element->intrval_mantissa << twt_element->request_type.intrval_exponent;
    twt_wake_duration = ((osal_u32)twt_element->min_duration) * wake_unit[twt_element->control.wake_duration_unit];

    /* twt_interval与twt_min_duration校验 */
    if ((twt_interval > 0xffffffff) || (twt_interval < (osal_u64)twt_wake_duration)) { /* 0xffffffff: 寄存器最大位数 */
        oam_error_log3(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_sta_rx_twt_association_frame_etc::interval[%d], duration[%d]!}",
            hmac_vap->vap_id, twt_interval, twt_wake_duration);
        return;
    }

    /* 进行协商，参数传入了twt更新来源 */
    ret = hmac_sta_rx_twt_negotiate(hmac_vap, hmac_user, twt_element, MAC_TWT_UPDATE_SOURCE_ASSOCIATION);
    if (ret != OAL_SUCC) {
        /* 输出错误 */
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_association_frame_etc:: negotiate error = [%d]}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_rx_twt_para_verify_etc
 功能描述  : STA从空口接收TWT setup帧的参数校验函数
 输入参数  : hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,  mac_individual_twt_setup_frame_stru *setup_frame
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_rx_twt_para_verify_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_individual_twt_setup_frame_stru *setup_frame)
{
    osal_u64 twt_interval;
    osal_u32 twt_wake_duration, ret;
    osal_u32 wake_unit[2] = {WAKE_DURATION_UNIT0, WAKE_DURATION_UNIT1}; /* 2:数组表示时间单位，0与1分别表示256与1024us */
    osal_u8 command       = (osal_u8)setup_frame->twt_element.request_type.setup_command;
    sta_twt_para_stru *twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);

    /* 非TWT element */
    if (setup_frame->twt_element.element_id != MAC_EID_TWT) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame:id=%d}", setup_frame->twt_element.element_id);
        return OAL_FAIL;
    }

    /* 若TWT已存在，且收到的TWT与当前TWT的id不匹配时错误 */
    if ((twt_info->sta_cfg_twt_para.twt_session_status != TWT_SESSION_OFF) &&
        (twt_info->sta_cfg_twt_para.twt_basic_param.flow_id != setup_frame->twt_element.request_type.flow_id)) {
        oam_error_log2(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::unmatched id1=%d,id2=%d}",
            twt_info->sta_cfg_twt_para.twt_basic_param.flow_id, setup_frame->twt_element.request_type.flow_id);
        return OAL_FAIL;
    }

    /* 检测TWT命令字段，MAC_TWT_COMMAND_DEMAND以内错误 */
    if (command < MAC_TWT_COMMAND_ACCEPT) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::command ERR=%d}", command);
        return OAL_FAIL;
    }

    /* 检查dialog token */
    if ((setup_frame->dialog_token != 0) && (setup_frame->dialog_token != twt_info->sta_cfg_twt_para.dialog_token)) {
        oam_error_log2(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::dialog token diff.rx=%d,tx=%d}",
            setup_frame->dialog_token, twt_info->sta_cfg_twt_para.dialog_token);
        return OAL_FAIL;
    }

    /* interval及min_duration获取，min_duration单位需要判断 */
    twt_interval = ((osal_u64)setup_frame->twt_element.intrval_mantissa) <<
                   setup_frame->twt_element.request_type.intrval_exponent;

    twt_wake_duration = ((osal_u32)setup_frame->twt_element.min_duration) *
                        wake_unit[setup_frame->twt_element.control.wake_duration_unit];

    /* twt_interval与twt_min_duration校验 */
    if (twt_interval < (osal_u64)twt_wake_duration) {
        oam_error_log2(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame:i=%d<d=%d}", twt_interval, twt_wake_duration);
        return OAL_FAIL;
    }

    /* 0xffffffff: 收到AP发来的Accept且interval参数不合理，发送teardown撤销AP侧的TWT协商 */
    if (twt_interval > 0xffffffff) {
        if (command == MAC_TWT_COMMAND_ACCEPT) {
            oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::twt interval[%d]}", twt_interval);
            ret = hmac_mgmt_tx_twt_teardown(hmac_vap, hmac_user,
                (osal_u8)setup_frame->twt_element.request_type.flow_id);
            if (ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::teardown err[%d]}", ret);
                return ret;
            }
        }
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : STA从空口接收TWT setup帧的处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_rx_twt_setup_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_individual_twt_setup_frame_stru *setup_frame)
{
    osal_u32 ret;

    /******************************************************************/
    /*       TWT setup Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | TWT element     |               */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      |  Variable  |                    */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    /*************************************************************************/
    /*                  TWT Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Control | TWT parameter Information            */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 1      |      Variable             |           */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    oam_warning_log3(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame:: flow_id:%d, flow_type:%d, setup_command:%d}",
        setup_frame->twt_element.request_type.flow_id, setup_frame->twt_element.request_type.flow_type,
        setup_frame->twt_element.request_type.setup_command);

    /* 判断twt开关是否打开 */
    if (hmac_twt_get_req_bit(hmac_vap->vap_id) == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame::twt requester support off}");
        return OAL_FAIL;
    }

    /* 进行TWT参数校验 */
    ret = hmac_sta_rx_twt_para_verify_etc(hmac_vap, hmac_user, setup_frame);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame:: in valid para=[%d]}", ret);
        return ret;
    }

    /* 进行TWT协商 */
    ret = hmac_sta_rx_twt_negotiate(hmac_vap, hmac_user, &(setup_frame->twt_element), MAC_TWT_UPDATE_SOURCE_SETUP);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_setup_frame:: negotiation failed error=[%d]}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 从空口接收到TWT teardown帧的处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_rx_twt_teardown_frame(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 *payload)
{
    mac_twt_teardown_stru    *twt_teardown          = (mac_twt_teardown_stru *)payload;
    osal_u32 ret;
    osal_u8 dialog_token;
    sta_twt_para_stru *twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);

    /******************************************************************/
    /*       TWT teardown Frame - Frame Body                         */
    /* --------------------------------------------------------------- */
    /* | Category | Action | TWT Flow   |                             */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      |    1       |                             */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/
    oam_warning_log3(0, OAM_SF_11AX, "vap_id[%d] {hmac_sta_rx_twt_teardown_frame::flow_id:%d, nego_type:%d}",
                     hmac_vap->vap_id,
                     twt_teardown->flow_id, twt_teardown->nego_type);

    /* 只支持删除单播TWT会话 */
    if (twt_teardown->nego_type != 0) {
        oam_warning_log2(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_sta_rx_twt_teardown_frame::nego_type[%d] Error}", hmac_vap->vap_id,
            twt_teardown->nego_type);
        return OAL_FAIL;
    }

    /* 检查TWT是否在运行 */
    if (twt_info->sta_cfg_twt_para.twt_session_status == TWT_SESSION_OFF) {
        oam_warning_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_sta_rx_twt_teardown_frame::twt already disable}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (twt_info->sta_cfg_twt_para.twt_basic_param.flow_id != twt_teardown->flow_id) {
        oam_error_log3(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_sta_rx_twt_teardown_frame:: the flow id [%d] is invalid,our flow id is [%d]}",
            hmac_vap->vap_id, twt_teardown->flow_id, twt_info->sta_cfg_twt_para.twt_basic_param.flow_id);
        return OAL_FAIL;
    }

    /* 暂存，为下面清零后恢复 */
    dialog_token = twt_info->sta_cfg_twt_para.dialog_token;

    (void)memset_s((osal_u8 *)&(twt_info->sta_cfg_twt_para), sizeof(mac_cfg_twt_stru), 0, sizeof(mac_cfg_twt_stru));

    twt_info->sta_cfg_twt_para.twt_basic_param.flow_id = twt_teardown->flow_id;
    twt_info->sta_cfg_twt_para.dialog_token            = dialog_token;

    /* 调用发事件函数,事件分发至dmac */
    ret = hmac_sta_twt_update_event_etc(hmac_vap, hmac_user, MAC_TWT_UPDATE_SOURCE_TEARDOWN);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_sta_rx_twt_teardown_frame:: teardown failed error=[%d]}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_get_next_twt
 功能描述  : 从twt information帧中获取next twt
 输入参数  : hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u8 next_twt_subfd_size
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_u64 hmac_get_next_twt(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u8 next_twt_subfd_size)
{
    osal_u32 next_twt_low;
    osal_u64 next_twt_high;
    osal_u64 next_twt;

    switch (next_twt_subfd_size) {
        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0:
            next_twt = 0;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32: /* 32 bits */
            next_twt_low  = payload[0];
            next_twt_low |= (payload[1] << 8); /* 填充bit8~15 */
            next_twt_low |= (payload[2] << 16); /* payload[2]填充bit16~23 */
            next_twt_low |= (payload[3] << 24); /* payload[3]填充bit24~31 */
            next_twt = next_twt_low;
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48: /* 48 bits */
            next_twt_low  = payload[0];
            next_twt_low |= (payload[1] << 8);  /* 填充bit8~15 */
            next_twt_low |= (payload[2] << 16); /* payload[2]填充bit16~23 */
            next_twt_low |= (payload[3] << 24); /* payload[3]填充bit24~31 */

            next_twt_high  = payload[4];        /* payload[4]填充bit0~7 */
            next_twt_high |= (payload[5] << 8); /* payload[5]填充bit8~15 */

            next_twt  = next_twt_low;
            next_twt |= (next_twt_high << 32); /* next_twt_high放入高32bit */
            break;

        case HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64: /* 64 bits */
            next_twt_low  = payload[0];
            next_twt_low |= (payload[1] << 8);  /* payload[1]填充bit8~15 */
            next_twt_low |= (payload[2] << 16); /* payload[2]填充bit16~23 */
            next_twt_low |= (payload[3] << 24); /* payload[3]填充bit24~31 */

            next_twt_high  = payload[4];         /* payload[4]填充bit0~7 */
            next_twt_high |= (payload[5] << 8);  /* payload[5]填充bit8~15 */
            next_twt_high |= (payload[6] << 16); /* payload[6]填充bit16~23 */
            next_twt_high |= ((osal_u32)payload[7] << 24); /* payload[7]填充bit24~31 */

            next_twt  = next_twt_low;
            next_twt |= (next_twt_high << 32); /* next_twt_high放入高32bit */
            break;

        default:
            next_twt = 0;
            oam_error_log2(0, OAM_SF_11AX,
                "vap_id[%d] {hmac_get_next_twt:: invalid next twt size [%d]!}", hmac_vap->vap_id,
                next_twt_subfd_size);
            break;
    }

    return next_twt;
}

/*****************************************************************************
 功能描述  : 从空口接收TWT information帧的处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_rx_twt_information_frame(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 *payload_len)
{
    osal_u8 *payload = payload_len;
    /* 2表示information field前有2byte(category and action) */
    mac_twt_information_field_stru *information_field = (mac_twt_information_field_stru *)(payload + 2);
    osal_u32 ret;
    osal_u8  next_twt_subfd_size;
    osal_u64 next_twt;
    sta_twt_para_stru *twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);

    /******************************************************************/
    /*       TWT information Frame - Frame Body                       */
    /* --------------------------------------------------------------- */
    /* | Category | Action | TWT information_field | Next TWT         */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1           |  Variable                   */
    /* --------------------------------------------------------------- */
    /*                                                                */
    /******************************************************************/

    /* 检查TWT是否在运行 */
    if (twt_info->sta_cfg_twt_para.twt_session_status == TWT_SESSION_OFF) {
        oam_warning_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_sta_rx_twt_information_frame::twt is disable!}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 检查flow id */
    if (twt_info->sta_cfg_twt_para.twt_basic_param.flow_id != information_field->flow_id) {
        oam_warning_log3(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_sta_rx_twt_information_frame:: the flow id [%d] is invalid,our flow id is [%d]}",
            hmac_vap->vap_id, information_field->flow_id, twt_info->sta_cfg_twt_para.twt_basic_param.flow_id);
        return OAL_FAIL;
    }

    next_twt_subfd_size = information_field->next_twt_subfield_size;
    payload += sizeof(mac_twt_information_field_stru) + 2;     /* 2表示information field前有两个byte */

    next_twt = hmac_get_next_twt(hmac_vap, payload, next_twt_subfd_size);

    /*  这里只更新twt start time和next twt size */
    twt_info->sta_cfg_twt_para.twt_basic_param.twt = next_twt;
    twt_info->sta_cfg_twt_para.next_twt_size       = next_twt_subfd_size;
    oam_warning_log3(0, OAM_SF_11AX,
        "vap_id[%d] {hmac_sta_rx_twt_information_frame:: next_twt=%ld,next_twt_size=[%d]}",
        hmac_vap->vap_id, next_twt, next_twt_subfd_size);

    /* 事件分发至dmac */
    ret = hmac_sta_twt_update_event_etc(hmac_vap, hmac_user, MAC_TWT_UPDATE_SOURCE_INFORMATION);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX,
            "{hmac_sta_rx_twt_information_frame::information update failed error=[%d]}", ret);
    }
    return ret;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 功能描述  : 建立TWT SETUP会话的配置命令
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_twt_setup_req(hmac_vap_stru *hmac_vap, mac_cfg_twt_setup_req_param_stru *twt_setup_req)
{
    osal_u32 twt_wake_duration, ret;
    osal_u64 twt_interval;
    osal_u8  vap_num;
    /* 2:~{J}Wi1mJ>J1<d5%N;#,~}0~{Sk~}1~{7V1p1mJ>~}256~{Sk~}1024us */
    osal_u32 wake_unit[2] = {WAKE_DURATION_UNIT0, WAKE_DURATION_UNIT1};

    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_user_stru  *hmac_user  = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, twt_setup_req->mac_addr);

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_twt_get_req_bit(hmac_vap->vap_id) == OSAL_FALSE) {
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_setup_req::not support}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if ((hmac_device == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_config_twt_setup_req null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果是已经启动多vap的情况下，不能建立TWT会话:
    非DBDC场景，如果启动了3个及以上的vap，说明是wlan0/p2p共存，此时不能建立TWT会话 */
    vap_num = (osal_u8)hmac_get_device_vap_num();
    if (vap_num >= HMAC_TWT_P2P_RUNNING_VAP_NUM) {
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_setup_req::vap_num[%d]}",
                       hmac_vap->vap_id, vap_num);
        return OAL_FAIL;
    }

    /* 对于STA而言MAC_TWT_COMMAND_DEMAND以内的command参数是合法的 */
    if (twt_setup_req->twt_basic_param.setup_command > MAC_TWT_COMMAND_DEMAND) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_config_twt_setup_req::err_command!}");
        return OAL_FAIL;
    }

    /* 获取twt_interval与min_duration并校验 */
    twt_interval = ((osal_u64)twt_setup_req->twt_basic_param.intrval_mantissa) <<
                   twt_setup_req->twt_basic_param.intrval_exponent;
    twt_wake_duration = ((osal_u32)twt_setup_req->twt_basic_param.min_duration) *
                        wake_unit[twt_setup_req->twt_basic_param.wake_duration_unit];

    /* 0xffffffff: twt interval不超过32bit */
    if ((twt_interval > 0xffffffff) || (twt_interval < (osal_u64)twt_wake_duration)) {
        oam_error_log2(0, OAM_SF_11AX, "{hmac_config_twt_setup_req:In=%d,du=%d}", twt_interval, twt_wake_duration);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ret = hmac_mgmt_tx_twt_setup(hmac_vap, hmac_user, &(twt_setup_req->twt_basic_param));
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_setup_req::twt setup error=%d}",
                       hmac_vap->vap_id, ret);
    }
    return (osal_s32)ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_twt_teardown_req_etc
 功能描述  : delete TWT SETUP会话的配置命令
 输入参数  : hmac_vap_stru *mac_vap, osal_u16 len, osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_twt_teardown_req_etc(hmac_vap_stru *hmac_vap,
    mac_cfg_twt_teardown_req_param_stru *twt_teardown_req)
{
    hmac_user_stru *hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, twt_teardown_req->mac_addr);
    mac_cfg_twt_basic_param_stru twt_basic_param;
    osal_u32 ret;
    sta_twt_para_stru *twt_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    if (twt_info == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_config_twt_teardown_req_etc::twt_info null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (twt_info->sta_cfg_twt_para.twt_session_status == TWT_SESSION_OFF) {
        oam_error_log4(0, OAM_SF_11AX,
            "{hmac_config_twt_teardown_req_etc::the twt session of the ap[%x:%x:%x:%x:xx:xx] hasn't seted up yet!.}",
            twt_teardown_req->mac_addr[0], twt_teardown_req->mac_addr[1], twt_teardown_req->mac_addr[2], /* 2 */
            twt_teardown_req->mac_addr[3]); /* 3 */
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (twt_teardown_req->twt_flow_id != twt_info->sta_cfg_twt_para.twt_basic_param.flow_id) {
        oam_error_log3(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_config_twt_teardown_req_etc::flow id diff:seted up id[%d],request teardown id[%d]!.}",
            hmac_vap->vap_id, twt_info->sta_cfg_twt_para.twt_basic_param.flow_id, twt_teardown_req->twt_flow_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    twt_basic_param.flow_id = twt_teardown_req->twt_flow_id;

    ret = hmac_mgmt_tx_twt_teardown(hmac_vap, hmac_user, (osal_u8)twt_basic_param.flow_id);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_teardown_req_etc::twt teardown err=%d}",
                       hmac_vap->vap_id, ret);
    }
    return (osal_s32)ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_twt_receive_setup_etc
 功能描述  : STA接收twt setup帧处理
 输入参数  : hmac_vap_stru *mac_vap, osal_u16 len, osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_twt_receive_setup_etc(hmac_vap_stru *hmac_vap,
    mac_individual_twt_setup_frame_stru *setup_frame)
{
    hmac_user_stru *hmac_user  = OSAL_NULL;
    osal_u8           mac_addr[] = {0x34, 0x46, 0xEC, 0x6B, 0x95, 0x20};

    if (hmac_vap == OSAL_NULL || setup_frame == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_11AX, "{hmac_config_twt_teardown_req_etc:: hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_receive_setup_etc.}", hmac_vap->vap_id);

    /* ~{;qH!SC;'6TS&5DKwR}~} */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac_addr);
    if (hmac_user == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_teardown_req_etc::hmac_user null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_sta_rx_twt_setup_frame(hmac_vap, hmac_user, setup_frame);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : STA发送twt informaiuton帧处理
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_twt_information_req(hmac_vap_stru *hmac_vap,
    mac_cfg_twt_information_req_param_stru *twt_information_req)
{
    hmac_user_stru  *hmac_user  = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, twt_information_req->mac_addr);
    sta_twt_para_stru *twt_info = OSAL_NULL;
    osal_u32 ret;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    if (twt_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* TWT会话不存在 */
    if (twt_info->sta_cfg_twt_para.twt_session_status == TWT_SESSION_OFF) {
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_information_req::twt session not exit}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* TWT-information使能 */
    if (twt_info->sta_cfg_twt_para.twt_basic_param.information_disable == OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_information_req::twt information disable}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* TWT-flow-id一致 */
    if (twt_information_req->twt_information_filed.flow_id != twt_info->sta_cfg_twt_para.twt_basic_param.flow_id) {
        oam_error_log3(0, OAM_SF_11AX,
            "vap_id[%d] {hmac_config_twt_information_req::twt flow id isn't same:seted up id[%d],request id[%d]!.}",
            hmac_vap->vap_id,
            twt_info->sta_cfg_twt_para.twt_basic_param.flow_id, twt_information_req->twt_information_filed.flow_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ret = hmac_mgmt_tx_twt_information(hmac_vap, hmac_user, &(twt_information_req->twt_information_filed));
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_11AX, "vap_id[%d] {hmac_config_twt_information_req::twt setup error=%d}",
                       hmac_vap->vap_id, ret);
    }
    return (osal_s32)ret;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_twt_setup_req
 功能描述  : 建立TWT SETUP会话的调测命令
 输入参数  : oal_net_device_stru *net_dev, osal_s8 *param
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_twt_setup_req(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_char arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 index, value;
    osal_s32 ret;
    osal_u64 value_set[9];  /* 9:用来暂存从命令中读取的twt参数，共有9个 */
    /* 本命令总计10个参数,新增wake duration unit */
    osal_u32  compare[10] = {0, 0x7, 0x1, 0x7, 0x1, 0, 0x1f, 0xff, 0xffff, 0x1};
    mac_cfg_twt_setup_req_param_stru twt_setup_req_param;

    (osal_void)memset_s(&twt_setup_req_param, OAL_SIZEOF(mac_cfg_twt_setup_req_param_stru), 0,
        OAL_SIZEOF(mac_cfg_twt_setup_req_param_stru));

    /*
     * 设置AMPDU关闭的配置命令
     * ccpriv "wlan0 twt_setup_req xx xx xx xx xx xx(mac地址)
     * setup_cmd flow_type flow_ID trigger twt interval_exponent min_duration interval_mantissa duration_unit"
     */
    for (index = 0; index < 10; index++) { /* 本命令总计10个参数 */
        ret = hmac_ccpriv_get_one_arg(&param, (osal_s8 *)arg, OAL_SIZEOF(arg));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_twt_setup_req::parse arg failed [%d]!}", ret);
            return ret;
        }

        if (index == 0) {       /* 0: mac addr */
            oal_strtoaddr((const osal_char *)arg, twt_setup_req_param.mac_addr);
            continue;
        }

        value = (osal_u32)oal_atoi((const osal_s8 *)arg);
        if ((index != 5) && (value > compare[index])) { /* 5指twt,其最大支持64bit, 不做参数校验 */
            oam_error_log2(0, OAM_SF_11AX, "{hmac_ccpriv_twt_setup_req::index[%d] is not correct[%d]!", index, value);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        value_set[index - 1] = value;   /* index >= 1使用数组暂存参数，循环结束后一并读取 */
    }
    twt_setup_req_param.twt_basic_param.setup_command      = (osal_u16)value_set[0]; /* 0:value_set[0]含setup-command */
    twt_setup_req_param.twt_basic_param.flow_type          = (osal_u16)value_set[1]; /* 1:value_set[1]含flow type */
    twt_setup_req_param.twt_basic_param.flow_id            = (osal_u16)value_set[2]; /* 2:value_set[2]含flow id */
    twt_setup_req_param.twt_basic_param.trigger            = (osal_u16)value_set[3]; /* 3:value_set[3]含trigger */
    twt_setup_req_param.twt_basic_param.twt                = value_set[4];           /* 4:value_set[4]含twt */
    twt_setup_req_param.twt_basic_param.intrval_exponent   = (osal_u16)value_set[5]; /* 5:value_set[5]含exponent */
    twt_setup_req_param.twt_basic_param.min_duration       = (osal_u8)value_set[6];  /* 6:value_set[6]含min duration */
    twt_setup_req_param.twt_basic_param.intrval_mantissa   = (osal_u16)value_set[7]; /* 7:value_set[7]含mantissa */
    twt_setup_req_param.twt_basic_param.wake_duration_unit = (osal_u16)value_set[8]; /* 8:value_set[8]含duration unit */

    ret = hmac_config_twt_setup_req(hmac_vap, &twt_setup_req_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_twt_setup_req::config failed [%d]!}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_twt_teardown_req
 功能描述  : 建立TWT TEARDOWN会话的调测命令
 输入参数  : oal_net_device_stru *net_dev, osal_s8 *param
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_twt_teardown_req(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_char arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    mac_cfg_twt_teardown_req_param_stru teardown_req_param;

    (osal_void)memset_s(&teardown_req_param, OAL_SIZEOF(mac_cfg_twt_teardown_req_param_stru), 0,
        OAL_SIZEOF(mac_cfg_twt_teardown_req_param_stru));

    /*
     * 设置AMPDU关闭的配置命令:
     * ccpriv "wlan0 twt_teardown_req xx xx xx xx xx xx(mac地址) flow_id"
     */
    oam_warning_log0(0, OAM_SF_11AX, "{hmac_ccpriv_twt_teardown_req!}");

    /* 获取mac地址 */
    ret = hmac_ccpriv_get_one_arg(&param, (osal_s8 *)arg, OAL_SIZEOF(arg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_teardown_req::wal_get_cmd_one_arg err=%d!}", ret);
        return (osal_s32)ret;
    }
    oal_strtoaddr(arg, teardown_req_param.mac_addr);

    /* 获取twt_flow_id */
    ret = hmac_ccpriv_get_u8_with_check_max(&param, 0x07, &teardown_req_param.twt_flow_id);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX,
            "{hmac_ccpriv_twt_teardown_req::wal_cmd_get_u8_with_check_max return err=%d!}", ret);
        return (osal_s32)ret;
    }

    /* 抛事件到wal层处理 */
    ret = hmac_config_twt_teardown_req_etc(hmac_vap, &teardown_req_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_teardown_req::return err code[%d]!}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_twt_receive_setup
 功能描述  : STA接收TWT Setup帧的调测命令
 输入参数  : oal_net_device_stru *net_dev, osal_s8 *param
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_twt_receive_setup(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u32 index;
    osal_s32 ret;
    osal_s8 arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u64 value_set[TWT_RX_PARA_NUM];  /* 11:用来暂存从命令中读取的twt参数，共有11个 */
    /* 本命令总计11个参数 */
    osal_u32 compare[TWT_RX_PARA_NUM] = {0xff, 0xff, 0x7, 0x1, 0x7, 0x1, 0, 0x1f, 0xff, 0xffff, 0x1};
    mac_individual_twt_setup_frame_stru setup_frame;

    (osal_void)memset_s(&setup_frame, OAL_SIZEOF(mac_individual_twt_setup_frame_stru), 0,
        OAL_SIZEOF(mac_individual_twt_setup_frame_stru));

    /*
     * 设置收到TWT Setup帧的配置命令:
     * 1.dialog_token 2.EID 3.setup_cmd 4.flow_type 5.flow_ID 6.trigger 7.twt 8.interval_exponent
     * 9.min_duration 10.interval_mantissa 11.duration_unit"
     */
    oam_warning_log0(0, OAM_SF_11AX, "{hmac_ccpriv_twt_receive_setup!}");
    /* 11: 本命令总计11个参数 */
    for (index = 0; index < TWT_RX_PARA_NUM; index++) {
        ret = hmac_ccpriv_get_one_arg(&param, arg, OAL_SIZEOF(arg));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_teardown_req::wal_get_cmd_one_arg err=%d!}", ret);
            return ret;
        }

        value_set[index] = (osal_u64)oal_atoi((const osal_s8 *)arg);
        /* 6:6号参数twt不检验 */
        if ((index != 6) && (value_set[index] > compare[index])) {
            oam_error_log2(0, OAM_SF_11AX, "{hmac_ccpriv_twt_receive_setup::indx[%d] err[%d]!",
                index, value_set[index]);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }
    setup_frame.dialog_token = 0;   /* 0：dialogtoken赋值 */
    setup_frame.twt_element.element_id = MAC_EID_TWT;
    setup_frame.twt_element.request_type.setup_command    = (osal_u16)value_set[2];  /* 2：setupcmd */
    setup_frame.twt_element.request_type.flow_type        = (osal_u16)value_set[3];  /* 3：flow type */
    setup_frame.twt_element.request_type.flow_id          = (osal_u16)value_set[4];  /* 4：flow id */
    setup_frame.twt_element.request_type.trigger          = (osal_u16)value_set[5];  /* 5:trigger初始化1 */
    setup_frame.twt_element.twt                           = value_set[6];          /* 6:初始化twt为100000 */
    setup_frame.twt_element.request_type.intrval_exponent = (osal_u16)value_set[7];  /* 7:初始化intrval_exponent为5 */
    setup_frame.twt_element.min_duration                  = (osal_u8)value_set[8];   /* 8:初始化min_duration为10 */
    setup_frame.twt_element.intrval_mantissa              = (osal_u16)value_set[9];  /* 9:初始化intrval_mantissa为100 */
    setup_frame.twt_element.control.wake_duration_unit    = (osal_u8)value_set[10];  /* 10:初始化unit为0,256us */

    /* 抛事件到wal层处理 */
    ret = hmac_config_twt_receive_setup_etc(hmac_vap, &setup_frame);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_receive_setup::return err code[%d]!}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 发送TWT information调测,共输入mac地址，flow-id，pause三个参数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_twt_information_req(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_char arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32  ret;
    osal_s32 index;
    osal_u64  value_set[2];  /* 6:用来暂存从命令中读取的twt参数，flow id, pause */
    osal_u32  compare[2] = {0x7, 0x1}; /* 本命令总计2个参数 */
    mac_cfg_twt_information_req_param_stru information_req;

    (osal_void)memset_s(&information_req, OAL_SIZEOF(mac_cfg_twt_information_req_param_stru), 0,
        OAL_SIZEOF(mac_cfg_twt_information_req_param_stru));

    /*
     * echo "wlan0 twt_information_req xx xx xx xx xx xx(mac地址)
     * 0:flow_ID  1:pause(TRUE for pause, FAlSE for resume)"
     */
    oam_warning_log0(0, OAM_SF_11AX, "{hmac_ccpriv_twt_information_req!}");
    /* 获取mac地址 */
    ret = hmac_ccpriv_get_one_arg(&param, (osal_s8 *)arg, OAL_SIZEOF(arg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_teardown_req::wal_get_cmd_one_arg err=%d!}", ret);
        return ret;
    }
    oal_strtoaddr(arg, information_req.mac_addr);

    /* 2表示一共有两个参数 */
    for (index = 0; index < 2; index++) {
        if (hmac_ccpriv_get_one_arg(&param, (osal_s8 *)arg, OAL_SIZEOF(arg)) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_information_req::get param err, index=[%d].}", index);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        value_set[index] = (osal_u64)oal_atoi((const osal_s8 *)arg) & compare[index];
    }

    information_req.twt_information_filed.flow_id = (osal_u8)value_set[0];   /* 第一个参数flow-id */
    /* 0x3表示next-twt都按64位发送 */
    information_req.twt_information_filed.next_twt_subfield_size = (value_set[1] == OSAL_TRUE) ? 0 : 0x3;

    /* 抛事件到wal层处理 */
    ret = hmac_config_twt_information_req(hmac_vap, &information_req);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_ccpriv_twt_information_req::return err code[%d]!}", ret);
    }

    return ret;
}
#endif
/*****************************************************************************
 功能描述  : 去关联时清除TWT寄存器，清除标志位
*****************************************************************************/
OSAL_STATIC osal_bool hmac_twt_deinit(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap;
    sta_twt_para_stru *sta_twt_info = OSAL_NULL;

    if (hmac_user == OSAL_NULL) {
        return OSAL_FALSE;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (is_legacy_sta(hmac_vap) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (hmac_device_twt_custom_is_open(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    sta_twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    if (sta_twt_info->sta_cfg_twt_para.twt_session_status != TWT_SESSION_OFF) {
        (osal_void)memset_s(&(sta_twt_info->sta_cfg_twt_para), sizeof(mac_cfg_twt_stru), 0, sizeof(mac_cfg_twt_stru));
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_twt_is_session_enable
 功能描述  : twt会话是否开启
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u8 hmac_twt_is_session_enable(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_device_twt_custom_is_open(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    return ((hmac_get_sta_twt_info(hmac_vap->vap_id))->sta_cfg_twt_para.twt_session_status == TWT_SESSION_ON) ?
        OSAL_TRUE : OSAL_FALSE;
}

WIFI_TCM_TEXT OSAL_STATIC osal_bool hmac_is_twt_processed_not_need_buff(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_FALSE;
    }

    if (hmac_get_sta_twt_info(hmac_vap->vap_id) == OSAL_NULL) {
        return OSAL_TRUE;
    }

    if ((hmac_twt_is_session_enable(hmac_vap) == OSAL_TRUE) && (hmac_twt_is_ps_pause(hmac_vap) == OSAL_FALSE)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

WIFI_TCM_TEXT OSAL_STATIC osal_bool hmac_is_twt_need_buff(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_get_sta_twt_info(hmac_vap->vap_id) == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if ((hmac_twt_is_session_enable(hmac_vap) == OSAL_TRUE) && (hmac_twt_is_ps_pause(hmac_vap) == OSAL_TRUE)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

OSAL_STATIC osal_u32 hmac_twt_tx_action(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL || ctx_action_event == OSAL_NULL ||
        netbuf == OSAL_NULL || tx_ctl == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (tx_ctl->frame_subtype == WLAN_ACTION_TWT_SETUP_REQ) {
        return hmac_mgmt_tx_twt_setup_req(hmac_vap, hmac_user, ctx_action_event, netbuf);
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_TWT_TEARDOWN_REQ) {
        return hmac_mgmt_tx_twt_information_req(hmac_vap, hmac_user, ctx_action_event, netbuf);
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_TWT_INFORMATION_REQ) {
        return hmac_mgmt_tx_twt_teardown_req(hmac_vap, hmac_user, ctx_action_event, netbuf);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : P2P与TWT共存场景下，自动终止twt会话
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_p2p_teardown_twt_session(hmac_device_stru *hmac_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 ret = OAL_SUCC;
    osal_u8  vap_num;
    sta_twt_para_stru *twt_info = OSAL_NULL;

    vap_num = (osal_u8)hmac_get_device_vap_num();
    /* 非DBDC场景，如果启动了3个及以上的vap，说明是wlan0/p2p共存，此时如果已经建立twt会则需要删除 */
    if (vap_num >= 2) {
        /* 先找到wlan vap */
        mac_device_find_up_sta_wlan_etc(hmac_device, &hmac_vap);
        if (hmac_vap == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_p2p_teardown_twt_session::hmac_vap null.}");
            return ret;
        }

        if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
            return OAL_SUCC;
        }

        /* 获取用户对应的索引 */
        hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user == OSAL_NULL) {
            /* 已经删除了的话，属于正常 */
            oam_warning_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_p2p_teardown_twt_session::hmac_user[%d] null.}", hmac_vap->vap_id,
                hmac_vap->assoc_vap_id);
            return ret;
        }

        twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
        if (twt_info == OSAL_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 如果该vap的twt会话已经开启，则删除twt会话 */
        if (twt_info->sta_cfg_twt_para.twt_session_status != TWT_SESSION_OFF) {
            ret = hmac_mgmt_tx_twt_teardown(hmac_vap, hmac_user, twt_info->sta_cfg_twt_para.twt_basic_param.flow_id);
        }
    }
    return ret;
}

OSAL_STATIC osal_void hmac_process_update_twt(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u16 msg_len)
{
    osal_u8 *tmp_ie = OAL_PTR_NULL;

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    /* 寻找是否有TWT信息元素， tmp_ie指向信息元素 */
    tmp_ie = mac_find_ie_etc(MAC_EID_TWT, payload, msg_len);
    if (tmp_ie != OSAL_NULL) {
        /* 判断TWT能力位是否满足 */
        if (hmac_twt_get_req_bit(hmac_vap->vap_id) == OSAL_TRUE) {
            hmac_sta_rx_twt_association_frame_etc(hmac_vap, hmac_user, tmp_ie);
        }
    }
}

OSAL_STATIC osal_u32 hmac_sta_up_rx_action_s1g(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u8 *data = OSAL_NULL;
    hmac_user_stru *hmac_user;
    sta_twt_para_stru *twt_info;

    if (hmac_vap == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.type != WLAN_MANAGEMENT || frame_hdr->frame_control.sub_type != WLAN_ACTION) {
        return OAL_CONTINUE;
    }

    data = oal_netbuf_rx_data(*netbuf);
    if ((data[MAC_ACTION_OFFSET_CATEGORY] != MAC_ACTION_CATEGORY_S1G) ||
        ((data[MAC_ACTION_OFFSET_ACTION] != MAC_S1G_ACTION_TWT_SETUP) &&
        (data[MAC_ACTION_OFFSET_ACTION] != MAC_S1G_ACTION_TWT_TEARDOWN) &&
        (data[MAC_ACTION_OFFSET_ACTION] != MAC_S1G_ACTION_TWT_INFORMATION))) {
        return OAL_CONTINUE;
    }

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, frame_hdr->address2);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_up_rx_action_s1g::find_user_by_macaddr_etc failed.}",
                         hmac_vap->vap_id);
        return OAL_CONTINUE;
    }

    twt_info = hmac_get_sta_twt_info(hmac_vap->vap_id);
    if (twt_info == OSAL_NULL) {
        return OAL_FAIL;
    }

    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_S1G_ACTION_TWT_SETUP:
            hmac_sta_rx_twt_setup_frame(hmac_vap, hmac_user, (mac_individual_twt_setup_frame_stru *)data);
            break;

        case MAC_S1G_ACTION_TWT_TEARDOWN:
            hmac_sta_rx_twt_teardown_frame(hmac_vap, hmac_user, data);
            break;

        case MAC_S1G_ACTION_TWT_INFORMATION:
            hmac_sta_rx_twt_information_frame(hmac_vap, hmac_user, data);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_config_twt_status_sync(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_d2hd_twt_sync_info_stru *cfg = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || msg == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_twt_status_sync:: param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_get_sta_twt_info(hmac_vap->vap_id) == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg = (mac_d2hd_twt_sync_info_stru *)msg->data;
    return hmac_twt_update_d2h_sync(hmac_vap, cfg);
}

OSAL_STATIC osal_void hmac_set_ext_cap_twt_req(hmac_vap_stru *hmac_vap, mac_ext_cap_ie_stru *ext_cap_ie,
    osal_u8 *ie_len)
{
    if (mac_mib_get_he_twt_option_activated(hmac_vap) == OSAL_FALSE) {
        return;
    }

    ext_cap_ie->twt_requester_support = 1;
    (*ie_len) = MAC_XCAPS_EX_TWT_LEN;
}

OSAL_STATIC osal_bool hmac_twt_sta_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }
    vap_id = hmac_vap->vap_id;
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (g_sta_twt_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] hmac_twt_sta_vap_add mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(sta_twt_para_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] hmac_twt_sta_vap_add malloc null!", vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(sta_twt_para_stru), 0, sizeof(sta_twt_para_stru));
    g_sta_twt_info[vap_id] = (sta_twt_para_stru *)mem_ptr;

    hmac_twt_set_resp_bit(vap_id, OSAL_TRUE);
    hmac_twt_set_req_bit(vap_id, OSAL_TRUE);
    mac_mib_set_he_twt_option_activated(hmac_vap, OSAL_TRUE);

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_exit_etc
 功能描述  : roam模块控制信息卸载
 输入参数  : hmac_vap 需要卸载roam模块的vap
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OSAL_STATIC osal_bool hmac_twt_sta_vap_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    vap_id = hmac_vap->vap_id;
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (g_sta_twt_info[vap_id] == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_twt_sta_vap_del::sta_uapsd_info is NULL.}",
                         hmac_vap->vap_id);
        return OSAL_TRUE;
    }

    oal_mem_free(g_sta_twt_info[vap_id], OAL_TRUE);
    g_sta_twt_info[vap_id] = OAL_PTR_NULL;

    return OSAL_TRUE;
}

hmac_netbuf_hook_stru twt_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_FEATURE,
    .priority = HMAC_HOOK_PRI_LOW,
    .hook_func = hmac_sta_up_rx_action_s1g,
};

osal_u32 hmac_twt_sta_init(osal_void)
{
    osal_u32 ret;
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"twt_setup_req", hmac_ccpriv_twt_setup_req);
    hmac_ccpriv_register((const osal_s8 *)"twt_teardown_req", hmac_ccpriv_twt_teardown_req);
    hmac_ccpriv_register((const osal_s8 *)"twt_receive_setup", hmac_ccpriv_twt_receive_setup);
    hmac_ccpriv_register((const osal_s8 *)"twt_information_req", hmac_ccpriv_twt_information_req);
#endif
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_twt_deinit);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_twt_sta_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_twt_sta_vap_del);

    frw_msg_hook_register(WLAN_MSG_D2H_C_CFG_SYNC_TWT_STATUS, hmac_config_twt_status_sync);

    hmac_feature_hook_register(HMAC_FHOOK_TWT_IS_SESSION_ENABLE, hmac_twt_is_session_enable);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_NOT_NEED_BUFF, hmac_is_twt_processed_not_need_buff);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_NEED_BUFF, hmac_is_twt_need_buff);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_TX_ACTION, hmac_twt_tx_action);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_P2P_TEARDOWN_SESSION, hmac_p2p_teardown_twt_session);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_PROCESS_UPDATE_INFO, hmac_process_update_twt);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_GET_REQ_BIT, hmac_twt_get_req_bit);
    hmac_feature_hook_register(HMAC_FHOOK_TWT_SET_EXT_CAP_TWT_REQ, hmac_set_ext_cap_twt_req);

    ret = hmac_register_netbuf_hook(&twt_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_roam_sta_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }

    return OAL_SUCC;
}

osal_void hmac_twt_sta_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"twt_setup_req");
    hmac_ccpriv_unregister((const osal_s8 *)"twt_teardown_req");
    hmac_ccpriv_unregister((const osal_s8 *)"twt_receive_setup");
    hmac_ccpriv_unregister((const osal_s8 *)"twt_information_req");
#endif
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_twt_deinit);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_twt_sta_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_twt_sta_vap_del);

    frw_msg_hook_unregister(WLAN_MSG_D2H_C_CFG_SYNC_TWT_STATUS);

    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_IS_SESSION_ENABLE);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_NOT_NEED_BUFF);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_NEED_BUFF);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_TX_ACTION);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_P2P_TEARDOWN_SESSION);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_PROCESS_UPDATE_INFO);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_GET_REQ_BIT);
    hmac_feature_hook_unregister(HMAC_FHOOK_TWT_SET_EXT_CAP_TWT_REQ);

    hmac_unregister_netbuf_hook(&twt_netbuf_hook);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
