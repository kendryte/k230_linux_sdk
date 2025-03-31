/*
 * Copyright (c) CompanyNameMagicTag. 2018-2024. All rights reserved.
 * 文 件 名   : hmac_11v.c
 * 生成日期   : 2018年4月7日
 * 功能描述   : 11v 功能处理
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_11v.h"
#include "oal_ext_if.h"
#include "oal_netbuf_ext.h"
#include "oal_netbuf_data.h"
#include "oal_mem_hcm.h"
#include "mac_frame.h"
#include "mac_resource_ext.h"
#include "mac_ie.h"
#include "mac_vap_ext.h"
#include "mac_user_ext.h"
#include "frw_ext_if.h"
#include "wlan_types_common.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_roam_main.h"
#include "hmac_scan.h"
#include "hmac_feature_dft.h"
#include "hmac_mbo.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_11V_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_11v_vap_info_stru *g_11v_vap_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

hmac_user_11v_ctrl_stru *g_11v_user_info[WLAN_USER_MAX_USER_LIMIT];

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_bsst_req_action_parses(hmac_bsst_req_info_stru *bsst_req_info,
    osal_u8 *data, const osal_u16 frame_len, const hmac_user_stru *hmac_user);
OAL_STATIC osal_void hmac_rx_bsst_req_action_handle(hmac_vap_stru *hmac_vap,
    const hmac_bsst_req_info_stru *bsst_req_info, hmac_user_11v_ctrl_stru *pst_11v_ctrl_info,
    const hmac_user_stru *hmac_user);
OAL_STATIC osal_void hmac_get_neighbor_parses_all(hmac_neighbor_bss_info_stru *bss_list_alloc,
    osal_u8 *data, const osal_u16 len, const osal_u8 bss_number);
OAL_STATIC osal_void hmac_get_neighbor_parses_bssid(hmac_neighbor_bss_info_stru *bss_list_alloc,
    const osal_u8 bss_list_index, const osal_u8 *ie_data);
/* 将Neighbor Report IE结构体从帧数据中解析出来 */
OAL_STATIC hmac_neighbor_bss_info_stru *hmac_get_neighbor_ie(osal_u8 *data, osal_u16 len,
    osal_u8 *bss_num);
OAL_STATIC osal_u32 hmac_tx_bsst_rsp_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_bsst_rsp_info_stru *bsst_rsp_info);

OAL_STATIC hmac_11v_vap_info_stru *hmac_11v_get_vap_info(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }

    return g_11v_vap_info[vap_id];
}

OAL_STATIC hmac_user_11v_ctrl_stru *hmac_11v_get_user_info(osal_u8 user_idx)
{
    if (osal_unlikely(user_idx >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_NULL;
    }

    return g_11v_user_info[user_idx];
}

OAL_STATIC hmac_11v_vap_roam_info_stru *hmac_11v_get_roam_info(osal_u8 vap_id)
{
    hmac_11v_vap_info_stru *hmac_11v_vap_info = hmac_11v_get_vap_info(vap_id);

    if (hmac_11v_vap_info == OSAL_NULL) {
        return OSAL_NULL;
    }

    return hmac_11v_vap_info->roam_info_11v;
}

/*****************************************************************************
 函 数 名  : hmac_11v_roam_scan_check
 功能描述  : STA 11v trigger类型的漫游后，检查漫游扫描结果

 输入参数  : hmac_vap_stru *hmac_vap
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_11v_roam_scan_check(hmac_vap_stru *hmac_vap)
{
    hmac_user_stru            *hmac_user;
    hmac_user_11v_ctrl_stru   *pst_11v_ctrl_info;
    hmac_roam_info_stru       *roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    hmac_11v_vap_roam_info_stru *roam_info_11v = hmac_11v_get_roam_info(hmac_vap->vap_id);

    if (roam_info == OAL_PTR_NULL || roam_info_11v == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_11v_roam_scan_check::roam_info/roam_info_11v IS NULL}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 获取发送端的用户指针 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_11v_roam_scan_check::hmac_user is NULL}");
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    pst_11v_ctrl_info = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);
    if (pst_11v_ctrl_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_11v_ctrl_info->mac_11v_callback_fn == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    if (pst_11v_ctrl_info->uc_11v_roam_scan_times < MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT) {
        /* 触发单信道扫描漫游 */
        pst_11v_ctrl_info->uc_11v_roam_scan_times++;
        oam_warning_log3(0, OAM_SF_ANY, "{One channel scan roam,11v_roam_scan_times=[%d],limit_times=[%d]channel=[%d]}",
            pst_11v_ctrl_info->uc_11v_roam_scan_times,
            MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT, roam_info_11v->bsst_rsp_info.chl_num);
        hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_1, OAL_FALSE, ROAM_TRIGGER_11V);
    } else if (pst_11v_ctrl_info->uc_11v_roam_scan_times == MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT) {
        /* 触发全信道扫描漫游 */
        pst_11v_ctrl_info->uc_11v_roam_scan_times++;
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_11v_roam_scan_check::Trigger ALL Channel scan roam.}");
        hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_BUTT, OAL_FALSE, ROAM_TRIGGER_11V);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_bsst_req_candidate_info_check
 功能描述  : 检查channel是否在管制域内或是否存在扫描列表内
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_bsst_req_candidate_info_check(hmac_vap_stru *hmac_vap, osal_u8 channel,
    osal_u8 *bssid)
{
    wlan_channel_band_enum_uint8            channel_band;
    osal_u32                              check;
    mac_bss_dscr_stru                      *bss_dscr;

    channel_band = mac_get_band_by_channel_num(channel);
    check        = hmac_is_channel_num_valid_etc(channel_band, channel);
    if (check != OAL_SUCC) {
        /* 对于无效channel如果bssid存在扫描列表中则继续11v漫游流程 */
        bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap,
            bssid);
        if (bss_dscr != OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_CFG, "in bssinfo channel=[%d]", bss_dscr->st_channel.chan_number);
            /* 0:1:2:3:数组下标 */
            oam_warning_log4(0, OAM_SF_CFG, "bssid:%02X:%02X:%02X:%02X:XX:XX", bssid[0], bssid[1], bssid[2], bssid[3]);
            oam_warning_log1(0, OAM_SF_CFG, "channel=[%d] is invalid", channel);
            return OAL_SUCC;
        }

        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/* 根据接收到的BTM REQ中信息，找出偏好最高的，合适的进行切换 */
OAL_STATIC hmac_neighbor_bss_info_stru *hmac_handle_neighbor_list(const hmac_bsst_req_info_stru *bsst_req_info,
    hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, oal_bool_enum_uint8 *need_roam)
{
    /* 根据终端需求实现11v漫游 */
    osal_u32 ret;
    osal_u8  index;
    hmac_neighbor_bss_info_stru *best_neighbor_bss = OAL_PTR_NULL;

    if ((bsst_req_info->neighbor_bss_list == OAL_PTR_NULL) || (bsst_req_info->bss_list_num == 0)) {
        return OAL_PTR_NULL;
    }

    /* 遍历list 找出偏好值最高的作为切换 */
    for (index = 0; index < bsst_req_info->bss_list_num; index++) {
        /* 广播地址 无需处理 */
        if (ETHER_IS_BROADCAST(bsst_req_info->neighbor_bss_list[index].auc_mac_addr)) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_handle_neighbor_list::bsst req candidate bssid is broadcast}");
            continue;
        }
        /* 关联AP BSSID 无需处理 */
        if (osal_memcmp(hmac_user->user_mac_addr,
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr, WLAN_MAC_ADDR_LEN) == 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_handle_neighbor_list::bsst req candidate bssid is the same with the associated AP}");
            continue;
        }

        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_handle_neighbor_list::candidate bssid=%02x:%02x:%02x:%02x:xx:xx.}",
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr[0], /* 打印数组元素0 */
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr[1], /* 打印数组元素1 */
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr[2], /* 打印数组元素2 */
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr[3]); /* 打印数组元素3 */

        oam_warning_log3(0, OAM_SF_ANY, "{hmac_handle_neighbor_list::index[%u] candidate_perf[%u], dst AP's chan=%u}",
            index, bsst_req_info->neighbor_bss_list[index].candidate_perf,
            bsst_req_info->neighbor_bss_list[index].chl_num);

        /* 检查channel num 是否有效 */
        ret = hmac_rx_bsst_req_candidate_info_check(hmac_vap,
            bsst_req_info->neighbor_bss_list[index].chl_num,
            bsst_req_info->neighbor_bss_list[index].auc_mac_addr);
        if (ret != OAL_SUCC) {
            continue;
        }

        if (best_neighbor_bss == OAL_PTR_NULL) {
            best_neighbor_bss = &(bsst_req_info->neighbor_bss_list[index]);
            continue;
        }

        /* 比较ap所给的偏好值，偏好越高，信号越好 */
        if (best_neighbor_bss->candidate_perf < bsst_req_info->neighbor_bss_list[index].candidate_perf) {
            oam_warning_log2(0, OAM_SF_ANY, "hmac_handle_neighbor_list::old perf[%d] -> new[%d]\n",
                best_neighbor_bss->candidate_perf, bsst_req_info->neighbor_bss_list[index].candidate_perf);
            best_neighbor_bss = &(bsst_req_info->neighbor_bss_list[index]);
        }
    }

    *need_roam = (best_neighbor_bss == OAL_PTR_NULL) ? OAL_FALSE : OAL_TRUE;
    return best_neighbor_bss;
}

OAL_STATIC osal_void hmac_11v_free_url_and_bsslist(hmac_bsst_req_info_stru *bsst_req_info)
{
    if (bsst_req_info->session_url != OAL_PTR_NULL) {
        oal_mem_free(bsst_req_info->session_url, OAL_TRUE);
        bsst_req_info->session_url = OAL_PTR_NULL;
    }
    if (bsst_req_info->neighbor_bss_list != OAL_PTR_NULL) {
        oal_mem_free(bsst_req_info->neighbor_bss_list, OAL_TRUE);
        bsst_req_info->neighbor_bss_list = OAL_PTR_NULL;
    }
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_11V_FREE_BSS_LIST, OSAL_NULL);
}

/*****************************************************************************
 函 数 名  : hmac_rx_bsst_req_action
 功能描述  : STA接收AP发送的bss transition request帧
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_bsst_req_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru               *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru                *rx_info = OAL_PTR_NULL;
    osal_u16                      frame_len;
    osal_u8                      *data = OAL_PTR_NULL;
    hmac_bsst_req_info_stru         bsst_req_info;
    hmac_user_11v_ctrl_stru        *pst_11v_ctrl_info = OAL_PTR_NULL;
    hmac_user_stru                 *hmac_other_user = OAL_PTR_NULL;
    osal_u32                      ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 开关未打开不处理 */
    if (mac_mib_get_mgmt_option_bss_transition_activated(hmac_vap) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action:: BSSTransitionActivated is disabled}");
        return OAL_SUCC;
    }

    hmac_other_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_other_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action::hmac_user is NULL}");
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    pst_11v_ctrl_info = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);
    if (pst_11v_ctrl_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pst_11v_ctrl_info, OAL_SIZEOF(hmac_user_11v_ctrl_stru), 0, OAL_SIZEOF(hmac_user_11v_ctrl_stru));

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    rx_info  = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));
    /* 获取帧体指针 */
    data     = mac_get_rx_payload_addr(rx_info, pst_netbuf);
    frame_len = mac_get_rx_cb_payload_len(rx_info);  /* 帧体长度 */
    /* 帧体的最小长度为7 小于7则格式异常 */
    if (frame_len < HMAC_11V_REQUEST_FRAME_BODY_FIX) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action:: frame length error %d.}", frame_len);
        return OAL_FAIL;
    }

    /* 将帧的各种参数解析出来 供上层调用 */
    /* 解析Token 如果与当前用户下不一致 刷新Token */
    if (data[2] != pst_11v_ctrl_info->user_bsst_token) { /* 2 索引 */
        pst_11v_ctrl_info->user_bsst_token = data[2];    /* 2 索引 */
    }
    /* 解析request mode */
    memset_s(&bsst_req_info, sizeof(bsst_req_info), 0, sizeof(bsst_req_info));
    ret = hmac_rx_bsst_req_action_parses(&bsst_req_info, data, frame_len, hmac_other_user);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 获取合适的最佳bssid，rsp返回 */
    hmac_rx_bsst_req_action_handle(hmac_vap, &bsst_req_info, pst_11v_ctrl_info, hmac_other_user);

    /* 释放指针 */
    hmac_11v_free_url_and_bsslist(&bsst_req_info);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_bsst_req_action_parses(hmac_bsst_req_info_stru *bsst_req_info,
    osal_u8 *data, const osal_u16 frame_len, const hmac_user_stru *hmac_user)
{
    osal_u16 handle_len;
    osal_u16 url_count;

    bsst_req_info->request_mode.candidate_list_include = data[3] & BIT0; /* 3 索引 */
    bsst_req_info->request_mode.abridged = ((data[3] & BIT1) != 0) ? OSAL_TRUE : OSAL_FALSE;    /* 3 索引 */
    bsst_req_info->request_mode.bss_disassoc_imminent = \
        ((data[3] & BIT2) != 0) ? OSAL_TRUE : OSAL_FALSE; /* 3 索引 */
    bsst_req_info->request_mode.termination_include = ((data[3] & BIT3) != 0) ? \
        OSAL_TRUE : OSAL_FALSE; /* 3 索引 */
    bsst_req_info->request_mode.ess_disassoc_imminent = \
        ((data[3] & BIT4) != 0) ? OSAL_TRUE : OSAL_FALSE; /* 3 索引 */

    bsst_req_info->disassoc_time = ((osal_u16)(data[5]) << 8) | data[4]; /* 4,5 索引, 左移8位 */
    bsst_req_info->validity_interval = data[6]; /* 6 索引 */
    handle_len = 7;              /* 前面7个字节已被处理完 */
    /* 12字节的termination duration 如果有的话 */
    if (bsst_req_info->request_mode.termination_include != 0 &&
        frame_len >= handle_len + HMAC_11V_TERMINATION_TSF_LENGTH + 2) { /* 2 长度 */
        handle_len += MAC_IE_HDR_LEN;                /* 去掉元素头 */
        (osal_void)memcpy_s(bsst_req_info->term_duration.termination_tsf, HMAC_11V_TERMINATION_TSF_LENGTH,
            data + handle_len, HMAC_11V_TERMINATION_TSF_LENGTH);
        handle_len += HMAC_11V_TERMINATION_TSF_LENGTH;
        /* 左移8位 */
        bsst_req_info->term_duration.duration_min = (((osal_u16)data[handle_len + 1]) << 8) | (data[handle_len]);
        handle_len += 2; /* 2 长度 */
    }
    /* 解析URL */
    /* URL字段 如果有的话 URL第一个字节为URL长度 申请动态内存保存 */
    bsst_req_info->session_url = OAL_PTR_NULL;
    if (bsst_req_info->request_mode.ess_disassoc_imminent != 0 && frame_len >= handle_len + 1 + data[handle_len]) {
        if (data[handle_len] != 0) {
            /* 申请内存数量加1 用于存放字符串结束符 */
            url_count = data[handle_len] * OAL_SIZEOF(osal_u8) + 1;
            bsst_req_info->session_url = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, url_count,
                OAL_TRUE);
            if (bsst_req_info->session_url == OAL_PTR_NULL) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action_parses:: session_url alloc fail.}");
                return OAL_FAIL;
            }
            (void)memcpy_s(bsst_req_info->session_url, data[handle_len], data + (handle_len + 1), data[handle_len]);
            /* 转化成字符串 */
            bsst_req_info->session_url[data[handle_len]] = '\0';
        }
        handle_len += (data[handle_len] + 1);
    }
    /* Candidate bss list由于STA的Response frame为可选 需要解析出来放在此结构体中 供上层处理 */
    bsst_req_info->neighbor_bss_list = OAL_PTR_NULL;
    if ((bsst_req_info->request_mode.candidate_list_include != 0) && (frame_len >= handle_len)) {
        data += handle_len;
        bsst_req_info->neighbor_bss_list = hmac_get_neighbor_ie(data, frame_len - handle_len,
            &(bsst_req_info->bss_list_num));
    }

    oam_warning_log4(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action_parses:: associated address=%02x:%02x:%02x:%02x:xx:xx}",
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2],     /* 1,2 MAC地址 */
        hmac_user->user_mac_addr[3]);    /* 3 MAC地址 */
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action_parses: bss_list_num=%d}", bsst_req_info->bss_list_num);
    return OAL_CONTINUE;
}

OAL_STATIC osal_void hmac_rx_bsst_req_action_handle(hmac_vap_stru *hmac_vap,
    const hmac_bsst_req_info_stru *bsst_req_info, hmac_user_11v_ctrl_stru *pst_11v_ctrl_info,
    const hmac_user_stru *hmac_user)
{
    hmac_bsst_rsp_info_stru         bsst_rsp_info;
    oal_bool_enum_uint8             need_roam = OAL_TRUE;
    hmac_neighbor_bss_info_stru    *best_neighbor_bss = OAL_PTR_NULL;
    hmac_11v_vap_roam_info_stru    *roam_info_11v = hmac_11v_get_roam_info(hmac_vap->vap_id);
    osal_void *fhook = OSAL_NULL;

    if (roam_info_11v == OSAL_NULL) {
        return;
    }

    best_neighbor_bss = hmac_handle_neighbor_list(bsst_req_info, hmac_vap, hmac_user, &need_roam);
    if (best_neighbor_bss != OAL_PTR_NULL) {
        memset_s(&bsst_rsp_info, sizeof(bsst_rsp_info), 0, sizeof(bsst_rsp_info));
        bsst_rsp_info.status_code = 0;    /* 默认设置为同意切换 */
        bsst_rsp_info.termination_delay = 0;    /* 仅当状态码为5时有效，此次无意义设为0 */
        bsst_rsp_info.chl_num = best_neighbor_bss->chl_num;
        (osal_void)memcpy_s(bsst_rsp_info.target_bss_addr, WLAN_MAC_ADDR_LEN,
            best_neighbor_bss->auc_mac_addr, WLAN_MAC_ADDR_LEN);

        /* register BSS Transition Response callback function:
         * so that check roaming scan results firstly, and then send bsst rsp frame with right status code */
        pst_11v_ctrl_info->mac_11v_callback_fn = hmac_tx_bsst_rsp_action;

        fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_SET_DELAY_PARAM);
        if (fhook != OSAL_NULL) {
            ((hmac_set_bss_re_assoc_delay_params_cb)fhook)(bsst_req_info, hmac_user, hmac_vap, need_roam);
        }

        memcpy_s(&(roam_info_11v->bsst_rsp_info), sizeof(bsst_rsp_info), &bsst_rsp_info, sizeof(bsst_rsp_info));
        if ((need_roam == OAL_TRUE) && (bsst_req_info->bss_list_num == 1)) {
            pst_11v_ctrl_info->uc_11v_roam_scan_times = 1;
            hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_1, OAL_FALSE, ROAM_TRIGGER_11V);
        } else if (need_roam == OAL_TRUE) {
            hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_BUTT, OAL_FALSE, ROAM_TRIGGER_11V);
        }
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_rx_bsst_req_action:: not find best bss sending BTM rsp}");
    }

    return;
}

OSAL_STATIC osal_void hmac_tx_bsst_add_user_token(hmac_user_stru *hmac_user)
{
    hmac_user_11v_ctrl_stru *ctrl_info_11v = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);

    if (ctrl_info_11v == OSAL_NULL) {
        return;
    }

    if (ctrl_info_11v->user_bsst_token == HMAC_11V_TOKEN_MAX_VALUE) {
        ctrl_info_11v->user_bsst_token = 1;
    } else {
        ctrl_info_11v->user_bsst_token++;
    }
}

/*****************************************************************************
 函 数 名  : hmac_encap_bsst_rsp_action
 功能描述  : 组装BSS TRANSITION RESPONSE帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u16 hmac_encap_bsst_rsp_action(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user,
    hmac_bsst_rsp_info_stru *bsst_rsp_info,
    oal_netbuf_stru *buffer)
{
    osal_u16               us_index;
    osal_u8               *mac_header   = OAL_PTR_NULL;
    osal_u8               *payload_addr = OAL_PTR_NULL;
    hmac_user_11v_ctrl_stru *pst_11v_ctrl_info;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (bsst_rsp_info == OAL_PTR_NULL) ||
        (buffer == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_encap_bsst_rsp_action::null param.}");
        return 0;
    }

    pst_11v_ctrl_info = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);
    if (pst_11v_ctrl_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_header   = oal_netbuf_header(buffer);
    payload_addr = mac_netbuf_get_payload(buffer);
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    /* DA is address of STA addr */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);
    /* SA的值为本身的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));
    /* TA的值为VAP的BSSID */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /*************************************************************************************************************/
    /*                                  Set the contents of the frame body                                       */
    /*************************************************************************************************************/
    /*************************************************************************************************************/
    /*                       BSS Transition Response Frame - Frame Body                                      */
    /* ----------------------------------------------------------------------------------------------------------*/
    /* |Category |Action | Token| Status Code | Termination Delay | Target BSSID |   BSS Candidate List Entry    */
    /* --------------------------------------------------------------------------------------------------------- */
    /* |1        |1      | 1    |  1          | 1                 | 0-6          |    Optional                   */
    /* --------------------------------------------------------------------------------------------------------- */
    /*                                                                                                           */
    /*************************************************************************************************************/

    /* 将索引指向frame body起始位置 */
    us_index = 0;
    /* 设置Category */
    payload_addr[us_index] = MAC_ACTION_CATEGORY_WNM;
    us_index++;
    /* 设置Action */
    payload_addr[us_index] = MAC_WNM_ACTION_BSS_TRANSITION_MGMT_RESPONSE;
    us_index++;
    /* 设置Dialog Token */
    payload_addr[us_index] = pst_11v_ctrl_info->user_bsst_token;
    us_index++;
    /* 设置Status Code */
    payload_addr[us_index] = bsst_rsp_info->status_code;
    us_index++;
    /* 设置Termination Delay */
    payload_addr[us_index] = bsst_rsp_info->termination_delay;
    us_index++;
    /* 设置Target BSSID */
    if (bsst_rsp_info->status_code == 0) {
        (osal_void)memcpy_s(payload_addr + us_index, WLAN_MAC_ADDR_LEN,
            bsst_rsp_info->target_bss_addr, WLAN_MAC_ADDR_LEN);
        us_index += WLAN_MAC_ADDR_LEN;
    }
    /* 可选的候选AP列表 不添加 减少带宽占用 */
    return (osal_u16)(us_index + MAC_80211_FRAME_LEN);
}

/*****************************************************************************
 函 数 名  : hmac_tx_bsst_rsp_action
 功能描述  : STA发送bss transition response帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_tx_bsst_rsp_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_bsst_rsp_info_stru *bsst_rsp_info)
{
    oal_netbuf_stru               *bsst_rsp_buf;
    osal_u16                      frame_len;
    mac_tx_ctl_stru               *tx_ctl;
    osal_u32                      ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (bsst_rsp_info == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tx_bsst_rsp_action::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请bss transition request管理帧内存 */
    bsst_rsp_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (bsst_rsp_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_tx_bsst_rsp_action::bsst_rsq_buf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(bsst_rsp_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(bsst_rsp_buf, OAL_PTR_NULL);

    /* 调用封装管理帧接口 */
    frame_len = hmac_encap_bsst_rsp_action(hmac_vap, hmac_user, bsst_rsp_info, bsst_rsp_buf);
    if (frame_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(bsst_rsp_buf);
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_tx_bsst_rsp_action::encap frame failed.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }
    /* 初始化CB */
    memset_s(oal_netbuf_cb(bsst_rsp_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(bsst_rsp_buf);
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_MGMT;
    mac_get_cb_mpdu_len(tx_ctl) = frame_len;

    oal_netbuf_put(bsst_rsp_buf, frame_len);

    oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d]{hmac_tx_bsst_rsp_action:11v bsst rsp frame,frame_len=%d frametype=%d}",
        hmac_vap->vap_id, frame_len, mac_get_cb_frame_type(tx_ctl));

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, bsst_rsp_buf, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(bsst_rsp_buf);
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_tx_bsst_rsp_action::send failed.}", hmac_vap->vap_id);
        return ret;
    }

    /* STA发送完Response后 一次交互流程就完成了 需要将user下的Token值加1 供下次发送使用 */
    hmac_tx_bsst_add_user_token(hmac_user);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_neighbor_ie
 功能描述  : 从数据中解析出Neighbor Report IE
 输入参数  : bss_list: Neighbor Report IE结构体地址指针
 输出参数  : buffer : 帧体指针
             bss_num : 帧体包含的BSS数量
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
hmac_neighbor_bss_info_stru *hmac_get_neighbor_ie(osal_u8 *data, osal_u16 len, osal_u8 *bss_num)
{
    osal_u8   *ie_data_find = OAL_PTR_NULL;
    osal_u16  len_find = len;
    osal_u8   bss_number = 0;
    osal_u8  *ie_data = OAL_PTR_NULL;
    hmac_neighbor_bss_info_stru *bss_list_alloc = OAL_PTR_NULL;

    if ((data == OAL_PTR_NULL) || (bss_num == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_get_neighbor_ie::null pointer.}");
        if (bss_num != OAL_PTR_NULL) {
            *bss_num = 0;
        }
        return OAL_PTR_NULL;
    }
    /* 传入的帧长度为0，则不需要进行解析了 */
    if (len == 0) {
        *bss_num = 0;
        return OAL_PTR_NULL;
    }
    ie_data_find = data;

    /* 先确认含有多少个neighbor list */
    while (ie_data_find != OAL_PTR_NULL) {
        ie_data =  mac_find_ie_etc(MAC_EID_NEIGHBOR_REPORT, ie_data_find, len_find);
        /* 没找到则退出循环 */
        if (ie_data == OAL_PTR_NULL) {
            break;
        }
        bss_number++;                                   /* Neighbor Report IE 数量加1 */
        if (len_find >= ie_data[1] + MAC_IE_HDR_LEN) {
            ie_data_find += (ie_data[1] + MAC_IE_HDR_LEN);
            len_find -= (ie_data[1] + MAC_IE_HDR_LEN);
        } else {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_get_neighbor_ie::ie len[%d] greater than remain frame len[%d]!}",
                ie_data[1] + MAC_IE_HDR_LEN, len_find);
            return OAL_PTR_NULL;
        }
    }

    /* 如果neighbor ie 长度为0 直接返回 */
    if (bss_number == 0) {
        *bss_num = 0;
        return OAL_PTR_NULL;
    }
    /* 数据还原后再次从头解析数据 */
    bss_list_alloc = (hmac_neighbor_bss_info_stru *) oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        bss_number * OAL_SIZEOF(hmac_neighbor_bss_info_stru), OAL_TRUE);
    if (bss_list_alloc == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_get_neighbor_ie::bss_list null pointer.}");
        *bss_num = 0;
        return OAL_PTR_NULL;
    }

    hmac_get_neighbor_parses_all(bss_list_alloc, data, len, bss_number);
    *bss_num = bss_number;

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_11V_NEW_BSS_LIST, &bss_number);

    return bss_list_alloc;
}

/*****************************************************************************
 功能描述  : Neighbor Report Element子信息元素(Subelement)解析处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_neighbor_sub_element_porc(osal_u8 **ie_data, hmac_neighbor_bss_info_stru *bss_list_alloc,
    osal_u8 *sub_ie_len, osal_u8 bss_list_index)
{
    osal_u8 type = (osal_u8)(*ie_data)[0];
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_HANDLE_MBO_IE);

    if (((type == HMAC_NEIGH_SUB_ID_BSS_CANDIDATE_PERF) &&
        (*sub_ie_len < HMAC_11V_PERFERMANCE_ELEMENT_LEN + MAC_IE_HDR_LEN)) ||
        ((type == HMAC_NEIGH_SUB_ID_TERM_DURATION) &&
        (*sub_ie_len < HMAC_11V_TERMINATION_ELEMENT_LEN + MAC_IE_HDR_LEN)) ||
        ((fhook != OSAL_NULL) && (type == HMAC_NEIGH_SUB_ID_VENDOR_SPECIFIC) &&
        (*sub_ie_len < (*ie_data)[1] + MAC_IE_HDR_LEN))) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_neighbor_sub_element_porc sub_ie_len invalid, type[%d]}", type);
        return OAL_FAIL;
    } else if (*sub_ie_len < (*ie_data)[1] + MAC_IE_HDR_LEN) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_neighbor_sub_element_porc sub_ie_len invalid}");
        return OAL_FAIL;
    }

    switch (type) {
        case HMAC_NEIGH_SUB_ID_BSS_CANDIDATE_PERF:    /* 占用3个字节 */
            bss_list_alloc[bss_list_index].candidate_perf = (*ie_data)[2];   /* 2 索引 */
            (*sub_ie_len) -= (HMAC_11V_PERFERMANCE_ELEMENT_LEN + MAC_IE_HDR_LEN);
            (*ie_data) += (HMAC_11V_PERFERMANCE_ELEMENT_LEN + MAC_IE_HDR_LEN);
            break;

        case HMAC_NEIGH_SUB_ID_TERM_DURATION: /* 占用12个字节 */
            memcpy_s(bss_list_alloc[bss_list_index].term_duration.termination_tsf,
                HMAC_11V_TERMINATION_TSF_LENGTH, (*ie_data) + 2, 8);    /* 2 偏移, 8/8 长度 */
            bss_list_alloc[bss_list_index].term_duration.duration_min =
                (((osal_u16)(*ie_data)[11]) << 8) | ((*ie_data)[10]);   /* 10,11 索引,左移8位 */
            (*sub_ie_len) -= (HMAC_11V_TERMINATION_ELEMENT_LEN + MAC_IE_HDR_LEN);
            (*ie_data) += (HMAC_11V_TERMINATION_ELEMENT_LEN + MAC_IE_HDR_LEN);
            break;

        case HMAC_NEIGH_SUB_ID_VENDOR_SPECIFIC:
            if (fhook != OSAL_NULL) {
                ((hmac_handle_ie_specific_mbo_cb)fhook)((*ie_data), bss_list_alloc, bss_list_index);
                (*sub_ie_len) -= (osal_u8)((*ie_data)[1] + MAC_IE_HDR_LEN);
                (*ie_data) += ((*ie_data)[1] + MAC_IE_HDR_LEN);
            }
            break;

        /* 其他IE跳过 不处理 */
        default:
            (*sub_ie_len) -= ((*ie_data)[1] + MAC_IE_HDR_LEN);
            (*ie_data) += ((*ie_data)[1] + MAC_IE_HDR_LEN);
            break;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_get_neighbor_parses_all(hmac_neighbor_bss_info_stru *bss_list_alloc,
    osal_u8 *data, const osal_u16 len, const osal_u8 bss_number)
{
    osal_u16 len_find;
    osal_u8  bss_list_index;
    osal_u8  minmum_ie_len = 13;
    osal_u8  sub_ie_len;
    osal_u8  neighbor_ie_len;
    osal_u8 *ie_data = OAL_PTR_NULL;
    osal_u8 *ie_data_find = OAL_PTR_NULL;
    osal_u32 ret;

    ie_data_find = data;
    len_find = len;

    for (bss_list_index = 0; bss_list_index < bss_number; bss_list_index++) {
        /* 前面已经查询过一次，这里不会返回空，所以不作判断 */
        ie_data =  mac_find_ie_etc(MAC_EID_NEIGHBOR_REPORT, ie_data_find, len_find);
        neighbor_ie_len = ie_data[1];            // 元素长度
        hmac_get_neighbor_parses_bssid(bss_list_alloc, bss_list_index, ie_data);

        if (neighbor_ie_len <= minmum_ie_len) {
            ie_data_find += (ie_data[1] + MAC_IE_HDR_LEN);
            len_find -= (ie_data[1] + MAC_IE_HDR_LEN);
            continue;
        }

        /* 解析Subelement 长度大于最小ie长度才存在subelement 只处理3 4 subelement */
        sub_ie_len = neighbor_ie_len - minmum_ie_len;        /* subelement长度 */
        ie_data += (minmum_ie_len + MAC_IE_HDR_LEN);           /* 帧体数据移动到subelement处 */
        while (sub_ie_len > 0) {
            if ((ie_data[0] == HMAC_NEIGH_SUB_ID_TERM_DURATION) &&
                (memcpy_s(bss_list_alloc[bss_list_index].term_duration.termination_tsf,
                HMAC_11V_TERMINATION_TSF_LENGTH, ie_data + 2, 8) != EOK)) { /* 2 偏移, 8/8 长度 */
                oam_error_log0(0, OAM_SF_ANY, "{hmac_get_neighbor_parses_all:: memcpy_s failed}");
                return;
            }
            if (ie_data[0] == HMAC_NEIGH_SUB_ID_VENDOR_SPECIFIC && sub_ie_len < 10) { // 子ie至少包含10个字节
                oam_error_log1(0, OAM_SF_ANY, "hmac_get_neighbor_parses_all::MBO len[%d]", sub_ie_len);
                return;
            }
            ret = hmac_neighbor_sub_element_porc(&ie_data, bss_list_alloc, &sub_ie_len, bss_list_index);
            if (ret != OAL_SUCC) {
                return;
            }
        }
        ie_data_find += (ie_data[1] + MAC_IE_HDR_LEN);
        len_find -= (ie_data[1] + MAC_IE_HDR_LEN);
    }

    return;
}

OAL_STATIC osal_void hmac_get_neighbor_parses_bssid(hmac_neighbor_bss_info_stru *bss_list_alloc,
    const osal_u8 bss_list_index, const osal_u8 *ie_data)
{
    /* 解析Neighbor Report IE结构体 帧中只含有subelement 3 4，其他subelement已被过滤掉 */
    (osal_void)memcpy_s(bss_list_alloc[bss_list_index].auc_mac_addr, WLAN_MAC_ADDR_LEN,
        ie_data + 2, WLAN_MAC_ADDR_LEN); // 加2取数据部分
    /* 解析BSSID Information */
    bss_list_alloc[bss_list_index].bssid_info.ap_reachability =
        (osal_u8)((osal_u32)((ie_data[8] & BIT1) != 0) | (ie_data[8] & BIT0));  /* bit0-1, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.security =
        ((ie_data[8] & BIT2) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit2, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.key_scope =
        ((ie_data[8] & BIT3) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit3, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.spectrum_mgmt =
        ((ie_data[8] & BIT4) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit4, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.qos  =
        ((ie_data[8] & BIT5) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit5, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.apsd =
        ((ie_data[8] & BIT6) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit6, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.radio_meas =
        ((ie_data[8] & BIT7) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit7, 8 索引 */
    bss_list_alloc[bss_list_index].bssid_info.delay_block_ack =
        ((ie_data[9] & BIT0) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit0, 9 索引 */
    bss_list_alloc[bss_list_index].bssid_info.immediate_block_ack =
        ((ie_data[9] & BIT1) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit1, 9 索引 */
    bss_list_alloc[bss_list_index].bssid_info.mobility_domain =
        ((ie_data[9] & BIT2) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit2, 9 索引 */
    bss_list_alloc[bss_list_index].bssid_info.high_throughput =
        ((ie_data[9] & BIT3) != 0) ? OAL_TRUE : OAL_FALSE;     /* bit3, 9 索引 */
    /* 保留字段不解析 */
    bss_list_alloc[bss_list_index].opt_class = ie_data[12];    /* 12 索引 */
    bss_list_alloc[bss_list_index].chl_num = ie_data[13];      /* 13 索引 */
    bss_list_alloc[bss_list_index].phy_type = ie_data[14];     /* 14 索引 */

    return;
}

/*****************************************************************************
 函 数 名  : hmac_set_neighbor_ie
 功能描述  : 设置Neighbor Report IE
*****************************************************************************/
OAL_STATIC osal_void hmac_set_neighbor_ie(hmac_neighbor_bss_info_stru *neighbor_bss, osal_u8 bss_num, osal_u8 *buffer,
    osal_u16 *total_ie_len)
{
    osal_u8 bss_list_num;
    osal_u8 ie_fix_len = 13; /* 不含可选子元素 则Neighbor Report IE长度为13个字节 */
    osal_u16 total_ie_len_temp = 0;
    osal_u8 ie_len;
    osal_u8 bss_list_index;
    oal_bssid_infomation_stru *bss_info = OSAL_NULL;

    if ((buffer == OSAL_NULL) || (total_ie_len == OSAL_NULL) || (neighbor_bss == OSAL_NULL)) {
        if (total_ie_len != OSAL_NULL) {
            *total_ie_len = 0;
        }
        oam_error_log0(0, OAM_SF_ANY, "{hmac_set_neighbor_ie::null param.buf ie_len bss}");
        return;
    }
    /*  Neighbor Report Information Element Format                                                                */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* | Element ID | Length | BSSID | BSSID Info | Operating Class | Chnl Num | PHY TYPE | Optional Subelement | */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* | 1          | 1      | 6     | 4          | 1               | 1        | 1        | Variable            | */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* 设置Neighbor Report Element */
    bss_list_num = bss_num; /* 用户邻近的BSS数量 */

    if (bss_list_num > HMAC_MAX_BSS_NEIGHBOR_LIST) {
        bss_list_num = HMAC_MAX_BSS_NEIGHBOR_LIST; /* 数量限制为最大值 超过的IE将被丢弃 */
    }
    for (bss_list_index = 0; bss_list_index < bss_list_num; bss_list_index++) {
        ie_len = 0;
        buffer[0] = MAC_EID_NEIGHBOR_REPORT;
        /* 由于只截取了IE元素的一部分,长度变量信息将失效,需要重新计算 */
        /* Neighbor BSSID Adress */
        /* BSSID偏移地址为2 */
        (osal_void)memcpy_s(buffer + 2, WLAN_MAC_ADDR_LEN, neighbor_bss[bss_list_index].auc_mac_addr,
            WLAN_MAC_ADDR_LEN);
        /* Neighbor BSSID informatin */
        bss_info = (oal_bssid_infomation_stru *)(buffer + 8); /* BSSID Info偏移地址为8 */
        (osal_void)memcpy_s((osal_void *)bss_info, sizeof(oal_bssid_infomation_stru),
            (osal_void *)&(neighbor_bss[bss_list_index].bssid_info), sizeof(oal_bssid_infomation_stru));

        buffer[12] = neighbor_bss[bss_list_index].opt_class; /* 12 index */
        buffer[13] = neighbor_bss[bss_list_index].chl_num; /* 13 index */
        buffer[14] = neighbor_bss[bss_list_index].phy_type; /* 14 index */
        ie_len = ie_fix_len + MAC_IE_HDR_LEN;
        /* candidate perference子元素 添加子元素 默认必须存在该子元素 */
        buffer[ie_len++] = HMAC_NEIGH_SUB_ID_BSS_CANDIDATE_PERF;
        buffer[ie_len++] = 1;
        buffer[ie_len++] = neighbor_bss[bss_list_index].candidate_perf;
        /* 存在terminatin duration子元素 则添加子元素 */
        if (neighbor_bss[bss_list_index].term_duration.sub_ie_id == HMAC_NEIGH_SUB_ID_TERM_DURATION) {
            buffer[ie_len++] = HMAC_NEIGH_SUB_ID_TERM_DURATION;
            buffer[ie_len++] = HMAC_11V_TERMINATION_ELEMENT_LEN;
            (osal_void)memcpy_s(buffer + ie_len, HMAC_11V_TERMINATION_TSF_LENGTH,
                neighbor_bss[bss_list_index].term_duration.termination_tsf, HMAC_11V_TERMINATION_TSF_LENGTH);
            ie_len += HMAC_11V_TERMINATION_TSF_LENGTH;
            buffer[ie_len++] = neighbor_bss[bss_list_index].term_duration.duration_min & 0x00FF;
            buffer[ie_len++] = (neighbor_bss[bss_list_index].term_duration.duration_min >> 0x8) & 0x00FF;
        }
        /* 计算IE length值 等于总长度减去IE 头长度 */
        buffer[1] = ie_len - MAC_IE_HDR_LEN;

        /* 处理完一个IE 增减相应的指针和长度 */
        buffer += ie_len;
        /* 将此IE长度增加到总数据长度上 */
        total_ie_len_temp += ie_len;
    }

    *total_ie_len = total_ie_len_temp;
}

/*****************************************************************************
 函 数 名  : hmac_encap_bsst_query_action
 功能描述  : 封装bss transition management action 管理帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_encap_bsst_query_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_bsst_query_info_stru *bsst_query_info, oal_netbuf_stru *buffer)
{
    osal_u16 len = 0;
    osal_u8 *mac_header = OSAL_NULL;
    osal_u8 *payload_addr = OSAL_NULL;
    osal_u8 *payload_addr_origin = OSAL_NULL;
    osal_u16 frame_length;
    osal_u8 frame_index = 0;
    hmac_user_11v_ctrl_stru *ctrl_info_11v = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);

    if (ctrl_info_11v == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_encap_bsst_query_action::null param ctrl_info_11v}");
        return len;
    }

    mac_header = oal_netbuf_header(buffer);
    payload_addr = mac_netbuf_get_payload(buffer);
    if (payload_addr == OSAL_NULL) {
        return len;
    }

    payload_addr_origin = payload_addr;
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    /* 设置地址1，与STA连接的AP MAC地址 */
    (osal_void)memcpy_s(mac_header + WLAN_HDR_ADDR1_OFFSET, WLAN_MAC_ADDR_LEN,
        hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 设置地址2为自己的MAC地址 */
    (osal_void)memcpy_s(mac_header + WLAN_HDR_ADDR2_OFFSET, WLAN_MAC_ADDR_LEN,
        mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);
    /* 地址3，为VAP的MAC地址 */
    (osal_void)memcpy_s(mac_header + WLAN_HDR_ADDR3_OFFSET, WLAN_MAC_ADDR_LEN,
        hmac_vap->bssid, WLAN_MAC_ADDR_LEN);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /*************************************************************************/
    /*                Set the fields in the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*                       Channel Switch Announcement Frame - Frame Body  */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| BSS Tran Reason|BSS Candidate List Entry */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          |  1             |Variable             */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 设置Action的Category   */
    /* 10: WNM */
    payload_addr[frame_index++] = MAC_ACTION_CATEGORY_WNM;
    /* 设置WNM Action Field */
    /* 6: BSS Transition Query Frame */
    payload_addr[frame_index++] = MAC_WNM_ACTION_BSS_TRANSITION_MGMT_QUERY;
    /* 设置Dialog Token 加1处理要放在接收发送完Response后 或者超时后 */
    payload_addr[frame_index++] = ctrl_info_11v->user_bsst_token;
    /* 设置Query Reason */
    payload_addr[frame_index++] = bsst_query_info->reason;
    payload_addr += frame_index;

    /* 该信息应通过扫描筛选相关neighbor信息，待11V扫描接口实现后再封装，目前封装可修改BSSID信息用于触发AP回复BTM.req */
    /* 设置Neighbor Report IE */
    hmac_set_neighbor_ie(bsst_query_info->neighbor_bss_list, bsst_query_info->bss_list_num, payload_addr, &len);
    payload_addr += len;
    frame_length = (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);

    return frame_length;
}

/*****************************************************************************
 函 数 名  : hmac_tx_bsst_query_action
 功能描述  : STA发送bss transition query帧到AP
*****************************************************************************/
OAL_STATIC osal_u32 hmac_tx_bsst_query_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_bsst_query_info_stru *bsst_query_info)
{
    oal_netbuf_stru *mgmt_buf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u16 mgmt_len;
    osal_u32 ret;

    /* 用户能力位也需要支持方能下发 */
    if (hmac_user->cap_info.bss_transition != OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_tx_bsst_query_action::user not support.}");
        return OAL_FAIL;
    }

    /* 申请管理帧内存 */
    mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tx_bsst_query_action::mgmt_buf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);
    /* 封装 BSS Transition Management Query 帧 */
    mgmt_len = hmac_encap_bsst_query_action(hmac_vap, hmac_user, bsst_query_info, mgmt_buf);
    if (mgmt_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_tx_bsst_query_action::encap btq action failed.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    (osal_void)memset_s(oal_netbuf_cb(mgmt_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_MGMT;
    mac_get_cb_mpdu_len(tx_ctl) = mgmt_len;
    oal_netbuf_put(mgmt_buf, mgmt_len);

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, mgmt_buf, mgmt_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_tx_bsst_query_action::tx btq action failed.}", hmac_vap->vap_id);
        return ret;
    }
    return OAL_SUCC;
}

static osal_u32 hmac_tx_bsst_query_update_bss_list(const hmac_vap_stru *hmac_vap, const osal_u8 *param,
    hmac_neighbor_bss_info_stru *neigbor_bss_list)
{
    hmac_trigger_11v_msg_stru msg;
    const osal_u8 *target_bssid_list_0;

    if (osal_unlikely(hmac_vap == OSAL_NULL || param == OSAL_NULL || neigbor_bss_list == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tx_bsst_query_update_bss_list::null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(&msg, sizeof(hmac_trigger_11v_msg_stru), param, sizeof(hmac_trigger_11v_msg_stru));

    if (memcmp(hmac_vap->bssid, msg.mac_addr_list_0, WLAN_MAC_ADDR_LEN) == 0) {
        target_bssid_list_0 = msg.mac_addr_list_1;
        neigbor_bss_list[0].chl_num = msg.chl_num_list_1;
    } else if (memcmp(hmac_vap->bssid, msg.mac_addr_list_1, WLAN_MAC_ADDR_LEN) == 0) {
        target_bssid_list_0 = msg.mac_addr_list_0;
        neigbor_bss_list[0].chl_num = msg.chl_num_list_0;
    } else {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tx_bsst_query_update_bss_list::invalid router bssid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 预期neigbor_bss_list[0]被选中为漫游目标 */
    (osal_void)memcpy_s(neigbor_bss_list[0].auc_mac_addr, WLAN_MAC_ADDR_LEN, target_bssid_list_0, WLAN_MAC_ADDR_LEN);
    (osal_void)memcpy_s(neigbor_bss_list[1].auc_mac_addr, WLAN_MAC_ADDR_LEN, hmac_vap->bssid, WLAN_MAC_ADDR_LEN);
    neigbor_bss_list[1].chl_num = hmac_vap->channel.chan_number;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_trigger_tx_bsst_query
 功能描述  : 调试接口 触发STA发送bss transition query 帧
*****************************************************************************/
OAL_STATIC osal_u32 hmac_trigger_tx_bsst_query(hmac_vap_stru *hmac_vap, osal_u8 *param)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_neighbor_bss_info_stru *neigbor_bss_list = OSAL_NULL;
    hmac_bsst_query_info_stru bsst_query_info;
    osal_u8 neighbor_index = 0;
    osal_u8 neighbor_num = 2;
    osal_u32 ret;

    /* 获取用户 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_trigger_tx_bsst_query::hmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&bsst_query_info, sizeof(bsst_query_info), 0, sizeof(bsst_query_info));

    /* reason 参数应该由上层指定  */
    bsst_query_info.reason = 6; // 发送原因设置为6: Better AP found

    /* neighbor list应该由上层指向，
     暂时封装假信息--否则AP不回 BSST request */
    neigbor_bss_list = (hmac_neighbor_bss_info_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        neighbor_num * sizeof(hmac_neighbor_bss_info_stru), OAL_TRUE);
    if (neigbor_bss_list == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_trigger_tx_bsst_query::neigbor_bss_list NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_tx_bsst_query_update_bss_list(hmac_vap, param, neigbor_bss_list);
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (; neighbor_index < neighbor_num; neighbor_index++) {
        /* ap_reachablility初始化为2，表示当前AP是否可达暂不可知 */
        neigbor_bss_list[neighbor_index].bssid_info.ap_reachability = 0x2;
        neigbor_bss_list[neighbor_index].bssid_info.security = 1;
        neigbor_bss_list[neighbor_index].bssid_info.key_scope = 1;
        neigbor_bss_list[neighbor_index].bssid_info.spectrum_mgmt = 0;
        neigbor_bss_list[neighbor_index].bssid_info.qos = 0;
        neigbor_bss_list[neighbor_index].bssid_info.apsd = 0;
        neigbor_bss_list[neighbor_index].bssid_info.radio_meas = 1;
        neigbor_bss_list[neighbor_index].bssid_info.delay_block_ack = 0;
        neigbor_bss_list[neighbor_index].bssid_info.immediate_block_ack = 0;
        neigbor_bss_list[neighbor_index].bssid_info.mobility_domain = 1;
        neigbor_bss_list[neighbor_index].bssid_info.high_throughput = 1;
        neigbor_bss_list[neighbor_index].bssid_info.resv1 = 0;
        neigbor_bss_list[neighbor_index].opt_class = 0x12;
        neigbor_bss_list[neighbor_index].phy_type = 0x4;
        /* perference data BSSID偏好值 */
        neigbor_bss_list[neighbor_index].candidate_perf = neighbor_index;
    }
    bsst_query_info.neighbor_bss_list = neigbor_bss_list;
    bsst_query_info.bss_list_num = neighbor_num;
    ret = hmac_tx_bsst_query_action(hmac_vap, hmac_user, &bsst_query_info);

    /* 使用完成后释放内存 */
    oal_mem_free(neigbor_bss_list, OAL_TRUE);
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_11v_cfg_bsst_switch
 功能描述  : 11v配置接口 用于打开或者关闭11v特性的bss transition mgmt功能
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_11v_cfg_bsst_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_bool_enum_uint8     en_11v_cfg_switch = (oal_bool_enum_uint8)(*msg->data);

    /* 调用接口配置11v特性开关 */
    mac_mib_set_mgmt_option_bss_transition_activated(hmac_vap, en_11v_cfg_switch);

    oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_11v_cfg_bsst_switch:: Set BSST_Actived=[%d].}",
                     hmac_vap->vap_id,
                     mac_mib_get_mgmt_option_bss_transition_activated(hmac_vap));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11v_sta_tx_query
 功能描述  : 11V 触发接口，触发STA发送bss transition management query 帧
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_11v_sta_tx_query(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32  ul_ret;
    /* ~{=v~}STA~{D#J=OBV'3V4%7"7"KM~}query~{V!~} */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_11v_sta_tx_query::vap mode:[%d] not support this.}",
                         hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (mac_mib_get_mgmt_option_bss_transition_activated(hmac_vap) == OAL_FALSE) {
        oam_warning_log1(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_11v_sta_tx_query::dot11_mgmt_option_bss_transition_activated is FALSE.}",
                         hmac_vap->vap_id);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    ul_ret = hmac_trigger_tx_bsst_query(hmac_vap, msg->data);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_11v_sta_tx_query::hmac_config_send_event_etc failed[%d].}",
                         hmac_vap->vap_id, ul_ret);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_check_11v_scan_result(const hmac_roam_info_stru *roam_info,
    hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 find_bss)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    hmac_user_11v_ctrl_stru *pst_11v_ctrl_info = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_11v_vap_roam_info_stru *roam_info_11v = hmac_11v_get_roam_info(hmac_vap->vap_id);

    /* 获取发送端的用户指针 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_roam_check_11v_scan_result::mac_res_get_hmac_user_etc failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    pst_11v_ctrl_info = hmac_11v_get_user_info((osal_u8)hmac_user->assoc_id);
    if (pst_11v_ctrl_info == OSAL_NULL || roam_info_11v == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log3(0, OAM_SF_RX,
        "vap_id[%d] {hmac_roam_check_11v_scan_result::find_bss=[%d],uc_11v_roam_scan_times=[%d].}",
        hmac_vap->vap_id, find_bss, pst_11v_ctrl_info->uc_11v_roam_scan_times);

    if (find_bss == OAL_TRUE) {
        roam_info_11v->bsst_rsp_info.status_code = WNM_BSS_TM_ACCEPT;
        if (roam_info->config.scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
            /* 找到指定bss,本次11v漫游结束 */
            pst_11v_ctrl_info->uc_11v_roam_scan_times  = MAC_11V_ROAM_SCAN_FULL_CHANNEL_LIMIT;
        }
    } else {
        hmac_vap->roam_scan_valid_rslt = OAL_FALSE;
        if (roam_info->config.scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
            if (pst_11v_ctrl_info->uc_11v_roam_scan_times <= MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT) {
                /* 还需要再次触发漫游扫描 */
                hmac_device->scan_mgmt.is_scanning = OAL_FALSE;
                return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
            }
        }
        /* When candidate BSSID is not in scan results, bss transition rsp with status code = 7 */
        roam_info_11v->bsst_rsp_info.status_code = WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES;
    }

    if (pst_11v_ctrl_info->mac_11v_callback_fn) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_roam_check_11v_scan_result send bsst rsp.}", hmac_vap->vap_id);
        pst_11v_ctrl_info->mac_11v_callback_fn(hmac_vap, hmac_user, &(roam_info_11v->bsst_rsp_info));
        pst_11v_ctrl_info->mac_11v_callback_fn = OAL_PTR_NULL;
    }

    if (find_bss == OAL_FALSE) {
        roam_info_11v->roam_11v_scan_fail++;
        oam_warning_log1(0, OAM_SF_ROAM, "hmac_roam_check_11v_scan_result::candidate list no bss valid, scan fail=%d",
            roam_info_11v->roam_11v_scan_fail);

        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_11v_set_bss_transition(mac_user_cap_info_stru *cap_info, const osal_u8 *cap)
{
    if (cap_info == OSAL_NULL || cap == OSAL_NULL) {
        return;
    }

    /* 提取 BIT19: 支持bss transition */
    cap_info->bss_transition = ((cap[2] & BIT3) == 0) ? OAL_FALSE : OAL_TRUE;   /* ext cap IE的第2字节 */
}

OAL_STATIC osal_void hmac_11v_set_ext_cap_bss_transition(hmac_vap_stru *hmac_vap, mac_ext_cap_ie_stru *ext_cap)
{
    if (hmac_vap == OSAL_NULL || ext_cap == OSAL_NULL) {
        return;
    }

    /* 目前只有sta，非p2p的端口，才支持11v，需要填充11v能力 */
    if (is_legacy_sta(hmac_vap)) {
        /* 首先需先使能wirelessmanagerment标志 */
        /* 然后如果是站点本地能力位和扩展控制变量均支持BSS TRANSITION 设置扩展能力bit位 */
        if ((mac_mib_get_wireless_management_implemented(hmac_vap) == OSAL_TRUE) &&
            (mac_mib_get_mgmt_option_bss_transition_implemented(hmac_vap) == OSAL_TRUE) &&
            (mac_mib_get_mgmt_option_bss_transition_activated(hmac_vap) == OSAL_TRUE)) {
            ext_cap->bss_transition = 1;
        }
    }
}

OAL_STATIC osal_void hmac_11v_set_scan_params(mac_scan_req_stru *scan_params, hmac_roam_info_stru *roam_info)
{
    osal_u32 ul_ret;
    hmac_11v_vap_roam_info_stru *roam_info_11v = OSAL_NULL;

    if (scan_params == OSAL_NULL || roam_info == OSAL_NULL) {
        return;
    }

    roam_info_11v = hmac_11v_get_roam_info(roam_info->hmac_vap->vap_id);
    if (roam_info_11v == OSAL_NULL) {
        return;
    }

    scan_params->channel_list[0].chan_number = roam_info_11v->bsst_rsp_info.chl_num;
    scan_params->channel_list[0].band = mac_get_band_by_channel_num(roam_info_11v->bsst_rsp_info.chl_num);
    ul_ret = hmac_get_channel_idx_from_num_etc(scan_params->channel_list[0].band,
        scan_params->channel_list[0].chan_number, &(scan_params->channel_list[0].chan_idx));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "hmac_11v_set_scan_params:: get_channel_idx fail=%d", ul_ret);
    }
    scan_params->channel_nums = 1;
}

OAL_STATIC osal_bool hmac_11v_check_bss_dscr(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_11v_vap_roam_info_stru *roam_info_11v = OSAL_NULL;

    if (roam_info == OSAL_NULL || bss_dscr == OSAL_NULL) {
        return OSAL_FALSE;
    }

    roam_info_11v = hmac_11v_get_roam_info(roam_info->hmac_vap->vap_id);
    if (roam_info_11v == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (roam_info->roam_trigger == ROAM_TRIGGER_11V) {
        if (osal_memcmp(roam_info_11v->bsst_rsp_info.target_bss_addr, bss_dscr->bssid, WLAN_MAC_ADDR_LEN) == 0) {
            return OSAL_TRUE;
        }
    }
    return OSAL_FALSE;
}

OAL_STATIC osal_bool hmac_11v_check_scan_result(hmac_roam_info_stru *roam_info, oal_bool_enum_uint8 flag_in_scan_rslts)
{
    osal_u32 ret;

    if (roam_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (roam_info->roam_trigger == ROAM_TRIGGER_11V) {
        ret = hmac_roam_check_11v_scan_result(roam_info, roam_info->hmac_vap, flag_in_scan_rslts);
        if (ret != OAL_SUCC) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

OAL_STATIC osal_void hmac_11v_trigger_roam_check(const hmac_roam_info_stru *roam_info, hmac_vap_stru *hmac_vap)
{
    if (roam_info == OSAL_NULL || hmac_vap == OSAL_NULL) {
        return;
    }

    /* 重新触发漫游检查 */
    if (roam_info->roam_trigger == ROAM_TRIGGER_11V) {
        hmac_11v_roam_scan_check(hmac_vap);
    }
}

OAL_STATIC osal_u32 hmac_sta_up_rx_action_bsst_req_action(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;

    if (netbuf == OSAL_NULL || hmac_vap == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    data = oal_netbuf_rx_data(*netbuf);

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.type != WLAN_MANAGEMENT || frame_hdr->frame_control.sub_type != WLAN_ACTION) {
        return OAL_CONTINUE;
    }

    if ((data[MAC_ACTION_OFFSET_CATEGORY]) != MAC_ACTION_CATEGORY_WNM ||
        data[MAC_ACTION_OFFSET_ACTION] != MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST) {
        return OAL_CONTINUE;
    }

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, frame_hdr->address2);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_CONTINUE;
    }

    hmac_rx_bsst_req_action(hmac_vap, hmac_user, *netbuf);

    return OAL_CONTINUE;
}

OAL_STATIC osal_void hmac_get_11v_cap(hmac_vap_stru *hmac_vap, osal_s32 *val)
{
    osal_u8 vap_id;
    osal_u32 value = 0;

    if (hmac_vap == OSAL_NULL || val == OSAL_NULL) {
        return;
    }
    value = (osal_u32)(*val);

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    vap_id = hmac_vap->vap_id;
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return;
    }

    if (g_11v_vap_info[vap_id] != OSAL_NULL && g_11v_vap_info[vap_id]->enable_11v == OAL_TRUE) {
        value |= BIT(WAL_WIFI_FEATURE_SUPPORT_11V);
        *val = (osal_s32)value;
    }

    return;
}

OAL_STATIC osal_bool hmac_11v_vap_roam_info_init(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_11v_vap_info_stru *hmac_11v_vap_info = OSAL_NULL;
    osal_void *mem_ptr = OSAL_NULL;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    hmac_11v_vap_info = hmac_11v_get_vap_info(hmac_vap->vap_id);
    if (hmac_11v_vap_info == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "vap_id[%d] hmac_11v_vap_roam_info_init hmac_11v_vap_info null!");
        return OSAL_FALSE;
    }

    if (hmac_11v_vap_info->roam_info_11v != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_11v_vap_roam_info_init mem already malloc!", hmac_vap->vap_id);
        return OSAL_FALSE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_11v_vap_roam_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_11v_vap_roam_info_init mem alloc fail");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_11v_vap_roam_info_stru), 0, sizeof(hmac_11v_vap_roam_info_stru));
    hmac_11v_vap_info->roam_info_11v = mem_ptr;

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11v_vap_roam_info_deinit(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_11v_vap_info_stru *hmac_11v_vap_info = OSAL_NULL;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    hmac_11v_vap_info = hmac_11v_get_vap_info(hmac_vap->vap_id);
    if (hmac_11v_vap_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_11v_vap_info->roam_info_11v != OSAL_NULL) {
        oal_mem_free(hmac_11v_vap_info->roam_info_11v, OAL_TRUE);
        hmac_11v_vap_info->roam_info_11v = OSAL_NULL;
    }

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11v_add_vap(osal_void *notify_data)
{
    osal_u8 vap_id;
    osal_void *mem_ptr = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    mac_device_voe_custom_stru *mac_voe_custom_param = mac_get_pst_mac_voe_custom_param();

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id = hmac_vap->vap_id;
    if (g_11v_vap_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_11v_vap_info_init mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_11v_vap_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_11v_vap_info_init mem alloc fail");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_11v_vap_info_stru), 0, sizeof(hmac_11v_vap_info_stru));
    g_11v_vap_info[vap_id] = (hmac_11v_vap_info_stru *)mem_ptr;
    g_11v_vap_info[vap_id]->enable_11v = mac_voe_custom_param->en_11v;
    mac_mib_set_mgmt_option_bss_transition_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_wireless_management_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_mgmt_option_bss_transition_activated(hmac_vap, mac_voe_custom_param->en_11v);

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11v_del_vap(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id_11v;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id_11v = hmac_vap->vap_id;
    if (g_11v_vap_info[vap_id_11v] != OSAL_NULL) {
        hmac_11v_vap_roam_info_deinit(notify_data);
        oal_mem_free(g_11v_vap_info[vap_id_11v], OAL_TRUE);
        g_11v_vap_info[vap_id_11v] = OSAL_NULL;
    }

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11v_add_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    osal_u8 *mem_ptr = OSAL_NULL;
    osal_u8 user_idx;

    if (osal_unlikely(hmac_user == OSAL_NULL || hmac_user->assoc_id >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_FALSE;
    }

    user_idx = (osal_u8)hmac_user->assoc_id;
    if (g_11v_user_info[user_idx] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "user_idx[%d] hmac_11v_add_user mem already malloc!", user_idx);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_user_11v_ctrl_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "user_idx[%d] hmac_11v_add_user mem alloc fail.", user_idx);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_user_11v_ctrl_stru), 0, sizeof(hmac_user_11v_ctrl_stru));
    /* 注册特性数据结构 */
    g_11v_user_info[user_idx] = (hmac_user_11v_ctrl_stru *)mem_ptr;

    return OSAL_TRUE;
}

OAL_STATIC osal_bool hmac_11v_del_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_user_11v_ctrl_stru *hmac_11v_user_info = OSAL_NULL;
    osal_u8 user_idx;

    if (osal_unlikely(hmac_user == OSAL_NULL || hmac_user->assoc_id >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_FALSE;
    }

    user_idx = (osal_u8)hmac_user->assoc_id;
    hmac_11v_user_info = hmac_11v_get_user_info(user_idx);
    if (hmac_11v_user_info == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "user_idx[%d] hmac_11v_del_user mem already free!", user_idx);
        return OSAL_TRUE;
    }

    oal_mem_free(hmac_11v_user_info, OAL_TRUE);
    g_11v_user_info[user_idx] = OAL_PTR_NULL;

    return OSAL_TRUE;
}

hmac_netbuf_hook_stru g_11v_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_FEATURE,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_sta_up_rx_action_bsst_req_action,
};

osal_u32 hmac_11v_init(osal_void)
{
    osal_u32 ret;

    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11v_add_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11v_del_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_11v_add_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_11v_del_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_INIT, hmac_11v_vap_roam_info_init);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_DEINIT, hmac_11v_vap_roam_info_deinit);
    /* 注册消息 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_11V_TX_QUERY, hmac_11v_sta_tx_query);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_11V_BSST_SWITCH, hmac_11v_cfg_bsst_switch);
    /* 注册对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_11V_SET_BSS_TRANSITION, hmac_11v_set_bss_transition);
    hmac_feature_hook_register(HMAC_FHOOK_11V_SET_EXT_CAP_BSS_TRANSITION, hmac_11v_set_ext_cap_bss_transition);
    hmac_feature_hook_register(HMAC_FHOOK_11V_SET_SCAN_PARAMS, hmac_11v_set_scan_params);
    hmac_feature_hook_register(HMAC_FHOOK_11V_CHECK_BSS_DSCR, hmac_11v_check_bss_dscr);
    hmac_feature_hook_register(HMAC_FHOOK_11V_CHECK_SCAN_RESULT, hmac_11v_check_scan_result);
    hmac_feature_hook_register(HMAC_FHOOK_11V_TRIGGER_ROAM_CHECK, hmac_11v_trigger_roam_check);
    hmac_feature_hook_register(HMAC_FHOOK_GET_11V_CAP, hmac_get_11v_cap);
    /* 注册转发Hook */
    ret = hmac_register_netbuf_hook(&g_11v_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_11k_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }

    return OAL_SUCC;
}

osal_void hmac_11v_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11v_add_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11v_del_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_11v_add_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_11v_del_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_INIT, hmac_11v_vap_roam_info_init);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_DEINIT, hmac_11v_vap_roam_info_deinit);
    /* 去注册消息 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_11V_TX_QUERY);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_11V_BSST_SWITCH);
    /* 去注册对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_SET_BSS_TRANSITION);
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_SET_EXT_CAP_BSS_TRANSITION);
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_SET_SCAN_PARAMS);
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_CHECK_BSS_DSCR);
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_CHECK_SCAN_RESULT);
    hmac_feature_hook_unregister(HMAC_FHOOK_11V_TRIGGER_ROAM_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_GET_11V_CAP);
    /* 去注册转发Hook */
    hmac_unregister_netbuf_hook(&g_11v_netbuf_hook);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

