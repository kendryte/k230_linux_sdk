/*
 * Copyright (c) CompanyNameMagicTag. 2013-2024. All rights reserved.
 * 文 件 名   : hmac_mgmt_ap.c
 * 生成日期   : 2013年6月18日
 * 功能描述   : AP侧管理面处理
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_mgmt_ap.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "oal_cfg80211.h"
#endif
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "mac_frame.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_rx_data.h"
#include "hmac_uapsd.h"
#include "hmac_tx_amsdu.h"
#include "mac_ie.h"
#include "mac_user_ext.h"
#include "hmac_user.h"
#include "hmac_11i.h"
#include "hmac_11w.h"
#include "hmac_protection.h"
#include "hmac_chan_mgmt.h"
#include "hmac_smps.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "oal_netbuf_data.h"
#include "hmac_blacklist.h"
#include "hmac_dfs.h"
#include "hmac_blockack.h"
#include "hmac_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#include "mac_resource_ext.h"
#include "hmac_main.h"
#include "hmac_tx_mgmt.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_sta.h"
#include "oal_net.h"
#include "hmac_beacon.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_power.h"
#include "hmac_alg_notify.h"

#include "hmac_single_proxysta.h"
#include "hmac_csa_ap.h"
#include "hmac_feature_interface.h"
#include "hmac_11k.h"
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
#include "plat_misc.h"
#endif
#include "hmac_frag.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MGMT_AP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_SAE_PMKSA_CACHE_NUM 16 /* 最大16个 */
#define HMAC_SAE_MAX_PMKSA_EXPIRE 43200 /* 43200 seconds = 12h */
#define HMAC_SAE_PMKSA_CHECK_PERIOD (300 * 1000) /* 300 * 1000 */

#ifndef oal_make_word16
/* 待linux liteos融合后删除 */
#define oal_make_word16(lsb, msb) ((((osal_u16)(msb) << 8) & 0xFF00) | (lsb))
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_ap_check_probe_req(hmac_vap_stru *hmac_vap, osal_u8 *probe_req,
    mac_ieee80211_frame_stru *frame_hdr);
OAL_STATIC osal_u16  hmac_mgmt_encap_probe_response(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *ra, oal_bool_enum_uint8 is_p2p_req);

/*****************************************************************************
 函 数 名  : hmac_handle_disconnect_rsp_ap_etc
 功能描述  : AP(驱动)上报去关联某个STA
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
osal_u8  g_ul_print_wakeup_mgmt_etc = OAL_FALSE;

osal_void hmac_set_print_wakeup_mgmt_etc(osal_u8 val)
{
    g_ul_print_wakeup_mgmt_etc = val;
}

osal_u8 hmac_get_print_wakeup_mgmt_etc(osal_void)
{
    return g_ul_print_wakeup_mgmt_etc;
}
#endif

osal_void hmac_handle_disconnect_rsp_ap_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)

{
    osal_s32 ret;
    frw_msg msg_info = {0};

    msg_info.data = (osal_u8 *)hmac_user->user_mac_addr;
    msg_info.data_len = WLAN_MAC_ADDR_LEN;
    /* 2、3表示mac地址位 */
    oam_warning_log4(0, OAM_SF_AUTH, "{hmac_handle_disconnect_rsp_ap_etc::del_user[%02X:%02X:%02X:%02X:XX:XX]}",
        /* 打印第 0, 1 位mac */
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1],
        /* 打印第 2, 3 位mac */
        hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_STA_DISCONNECT_AP, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_handle_connect_rsp_ap
 功能描述  : AP(驱动)上报新关联上某个STA
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_handle_connect_rsp_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_asoc_user_req_ie_stru asoc_user_req_info = {0};
    osal_s32 ret;
    frw_msg msg_info = {0};

    /* 上报内核的关联sta发送的关联请求帧ie信息 */
    /* LINUX >= KERNEL_VERSION(3,10,44) 内核才会上报关联请求帧ie信息 */
    asoc_user_req_info.assoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    asoc_user_req_info.assoc_req_ie_len  = hmac_user->assoc_req_ie_len;

    /* 关联的STA mac地址 */
    if (memcpy_s((osal_u8 *)asoc_user_req_info.user_mac_addr, sizeof(asoc_user_req_info.user_mac_addr),
        hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_handle_connect_rsp_ap::memcpy_s mac2 error}");
    }
    msg_info.data = (osal_u8 *)&asoc_user_req_info;
    msg_info.data_len = sizeof(hmac_asoc_user_req_ie_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_STA_CONNECT_AP, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return;
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_update_auth_mib
 功能描述  : 更新认证mib 打印调测信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_mgmt_update_auth_mib(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_rsp)
{
    osal_u16  status;
    osal_u8   addr1[6] = {0};
    osal_u8   *mac_header = oal_netbuf_header(auth_rsp);

    status = mac_get_auth_status(mac_header);

    mac_get_address1(mac_header, (osal_u8 *)addr1);

    if (status != MAC_SUCCESSFUL_STATUSCODE) {
        /* DEBUG */
        oam_info_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_mgmt_update_auth_mib::Authentication of STA Failed.Status Code=%d.}",
            hmac_vap->vap_id, status);
    }
}

#ifdef _PRE_WLAN_FEATURE_WPA3
OAL_STATIC osal_u32 hmac_ap_sae_auth_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req)
{
    osal_u8  mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u16 user_index;
    mac_rx_ctl_stru *rx_ctl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(auth_req);
    /* 解析auth transaction number */
    osal_u16 auth_seq = mac_get_auth_seq_num(oal_netbuf_header(auth_req), mac_get_rx_cb_payload_len(rx_ctl));
    if ((auth_seq > WLAN_AUTH_TRASACTION_NUM_TWO) || (auth_seq < WLAN_AUTH_TRASACTION_NUM_ONE)) {
        oam_warning_log1(0, OAM_SF_AUTH, "{hmac_ap_sae_auth_proc::auth recieve invalid seq, auth seq [%d]}", auth_seq);
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_AUTH, "{hmac_ap_sae_auth_proc::auth recieve valid seq, auth seq [%d]}", auth_seq);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(auth_req), mac_addr);
    if (hmac_addr_is_zero_etc(mac_addr)) {
        oam_warning_log4(0, OAM_SF_AUTH,
            "{hmac_ap_sae_auth_proc::user mac:%02X:%02X:%02X:%02X:XX:XX is all 0 and invaild!}",
            /* 0:1:2:3:数组下标 */
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
        return OAL_FAIL;
    }

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac_addr);
    if (hmac_user == OAL_PTR_NULL) {
        osal_u32 ret = hmac_user_add_etc(hmac_vap, mac_addr, &user_index);
        if (ret != OAL_SUCC) {
            if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_ap_sae_auth_proc:add_user fail,users config spec}");
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            } else {
                oam_error_log1(0, OAM_SF_ANY, "{hmac_ap_sae_auth_proc:add_user fail %d}", user_index);
                return OAL_FAIL;
            }
        }
    } else {
        hmac_user->auth_alg = mac_get_auth_algo_num(auth_req);
        /* 重置节能位 */
        hmac_mgmt_reset_psm_etc(hmac_vap, hmac_user->assoc_id);
        /* 重新开始auth认证时复位当前状态 */
        if (auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE) {
            if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
                hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_BUTT);
            } else {
                hmac_user->flag_change_user_state_to_auth_complete = OAL_TRUE;
            }
        }
    }
    hmac_rx_mgmt_send_to_host_etc(hmac_vap, auth_req);

    oam_warning_log4(0, OAM_SF_AUTH,
        "{hmac_ap_sae_auth_proc::report Auth Req to host,user mac:%02X:%02X:%02X:%02X:XX:XX.}",
        /* 0:1:2:3:数组下标 */
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
    return OAL_SUCC;
}

OAL_STATIC osal_u16 hmac_check_rsn_pmkid_num(const osal_u8 *rsn_ie)
{
    osal_u16  pairwise_count;
    osal_u16  akm_count;
    osal_u16  pmkid_count;
    osal_u16  us_index = 0;

    if (rsn_ie == OAL_PTR_NULL) {
        return 0;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*            |         4*m                |     2           |   4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    if (rsn_ie[1] < MAC_MIN_RSN_LEN) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::invalid rsn ie len[%d].}", rsn_ie[1]);
        return 0;
    }

    us_index += 8; /* 偏移8 byte，获取pairwise_count */
    pairwise_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::invalid pairwise_count[%d].}", pairwise_count);
        return 0;
    }
    us_index += 2 + 4 * (osal_u8)pairwise_count; /* 再偏移(2 + 4 * pairwise_count) byte，获取akm_count */
    akm_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (akm_count > MAC_AUTHENTICATION_SUITE_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::invalid akm_count[%d].}", akm_count);
        return 0;
    }
    us_index += 2 + 4 * (osal_u8)akm_count; /* 再偏移(2 + 4 * akm_count) byte，获取rsn_capability */
    /* 不带rsn_capability直接返回 */
    if (us_index == (rsn_ie[1] + MAC_IE_HDR_LEN)) {
        return 0;
    }
    us_index += 2; /* 2: 再偏移rsn_capability的长度 */

    /* 判断是否携带PMKID */
    if (us_index == (rsn_ie[1] + MAC_IE_HDR_LEN)) {
        return 0;
    }

    pmkid_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    return pmkid_count;
}

OAL_STATIC osal_u16 hmac_sae_check_rsn_pmkid_match_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *rsn_ie, osal_u16 pmkid_count_offset, osal_u8 *is_match)
{
    osal_u16 index;
    osal_u16 pmkid_count;
    osal_u16 idx = pmkid_count_offset;

    pmkid_count = oal_make_word16(rsn_ie[idx], rsn_ie[idx + 1]);
    oam_warning_log1(0, OAM_SF_AUTH, "rx asoc req,  use sae conn, pmkid count[%d]\n", pmkid_count);
    if (pmkid_count != 0) {
        idx += 2; /* 2: pmkid count size */
        for (index = 0; index < pmkid_count; index++) {
            hmac_sae_pmksa_entry_info *entry = hmac_sae_find_pmkid_by_macaddr(&(hmac_vap->pmksa_mgmt),
                hmac_user->user_mac_addr);
            if (entry == OAL_PTR_NULL) {
                break;
            }
            if (osal_memcmp(entry->pmkid, &(rsn_ie[idx]), WLAN_PMKID_LEN) == 0) {
                *is_match = OSAL_TRUE;
                break;
            }
            idx += WLAN_PMKID_LEN;
        }
    }
    return pmkid_count;
}

OAL_STATIC osal_s32 hmac_sae_check_rsn_pmkid_match(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *rsn_ie, osal_u8 *is_match)
{
    osal_u16 us_index = 0;
    osal_u8 oui[MAC_OUI_LEN] = {0x00, 0x0F, 0xAC}; /* RSNA OUI 定义 0x000FAC */
    osal_u16 pairwise_count, akm_count, index;
    osal_u8 use_sae;
    *is_match = OAL_FALSE;
    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*            |         4*m                |     2           |   4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    if (rsn_ie == OAL_PTR_NULL || rsn_ie[1] < MAC_MIN_RSN_LEN) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::rsn ie null[%d] or len invalid.}",
            rsn_ie == OAL_PTR_NULL ? 1 : 0);
        return -1;
    }

    us_index += 8; /* 偏移8 byte，获取pairwise_count */
    pairwise_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::invalid pairwise_count[%d].}", pairwise_count);
        return -1;
    }
    us_index += 2 + 4 * (osal_u8)pairwise_count; /* 再偏移(2 + 4 * pairwise_count) byte，获取akm_count */
    akm_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (akm_count > MAC_AUTHENTICATION_SUITE_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::invalid akm_count[%d].}", akm_count);
        return -1;
    }
    us_index += 2; /* 2: akm count size */
    use_sae = OAL_FALSE;
    for (index = 0; index < akm_count;) {
        if (osal_memcmp(oui, rsn_ie + us_index + index, MAC_OUI_LEN) != 0) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_check_rsn_pmkid_num::RSN auth OUI illegal!}\r\n");
            return -1;
        }
        if (rsn_ie[us_index + index + MAC_OUI_LEN] == WLAN_AUTH_SUITE_SAE_SHA256) {
            use_sae = OSAL_TRUE;
        }
        index += 4; /* 4: rsn akm suite size */
    }
    if (use_sae != OSAL_TRUE) {
        return 0;
    }
    hmac_user->cap_info.use_wpa3 = OSAL_TRUE;
    us_index += 4 * (osal_u8)akm_count; /* 再偏移(4 * akm_count) byte，获取rsn_capability */
    /* 不带rsn_capability直接返回 */
    if (us_index == (rsn_ie[1] + MAC_IE_HDR_LEN)) {
        return 0;
    }
    us_index += 2; /* 2: 再偏移rsn_capability的长度 */

    /* 判断是否携带PMKID或者SAE连接 */
    if ((us_index == (rsn_ie[1] + MAC_IE_HDR_LEN)) || (hmac_user->auth_alg == WLAN_WITP_AUTH_SAE)) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_check_rsn_pmkid_num::auth_alg[%u], no pmkid.}", hmac_user->auth_alg);
        return 0;
    }
    return hmac_sae_check_rsn_pmkid_match_etc(hmac_vap, hmac_user, rsn_ie, us_index, is_match);
}

OAL_STATIC osal_u16 hmac_ap_assoc_req_wpa3_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *rsn_ie)
{
    if (hmac_vap->cap_flag.wpa3 == OAL_TRUE) {
        osal_u8 is_match = OAL_FALSE;
        osal_s32 rets = hmac_sae_check_rsn_pmkid_match(hmac_vap, hmac_user, rsn_ie, &is_match);
        oam_warning_log2(0, OAM_SF_ASSOC, "Check pmkid ret[%d], is_match[%d]\n", rets, is_match);
        if (rets == -1) {
            oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_sae_check_rsn_pmkid_match failed.}");
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
            return MAC_UNSPEC_FAIL;
        } else if ((rets > 0) && (is_match == OAL_FALSE)) {
            oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_sae_check_rsn_pmkid_match inmatch.}");
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
            return MAC_STATUS_INVALID_PMKID;
        }
    } else {
        osal_u16 num = hmac_check_rsn_pmkid_num(rsn_ie);
        if (num > 0) {
            oam_warning_log0(0, OAM_SF_ASSOC, "{unsupported pmkid cache when wpa3 not En.}");
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
            return MAC_STATUS_INVALID_PMKID;
        }
    }
    return MAC_SUCCESSFUL_STATUSCODE;
}

/* 检查assoc req帧的OWE携带情况 */
OAL_STATIC osal_void hmac_ap_check_owe_capa(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_s32 payload_len, mac_status_code_enum_uint16 *status_code)
{
    osal_u32 ret;
    osal_u8 *rsn_ie = OSAL_NULL;
    osal_u8 *owe_dh = OSAL_NULL;
    mac_crypto_settings_stru crypto = {0};
    osal_u8 suite_idx = 0;
    osal_u16 owe_group = 0;
    osal_u32 akm_suites[WLAN_PAIRWISE_CIPHER_SUITES] = {0};

    hmac_user->hmac_cap_info.owe = OAL_FALSE;
    hmac_user->owe_ie_len = 0;
    hmac_user->owe_ie = OSAL_NULL;

    mac_mib_get_rsn_akm_suites(hmac_vap, akm_suites, sizeof(akm_suites));
    if ((akm_suites[0] != MAC_RSN_AKM_OWE) && (akm_suites[1] != MAC_RSN_AKM_OWE)) {
        return;
    }

    /* 优先查找 RSN 信息元素 */
    rsn_ie = mac_find_ie_etc(MAC_EID_RSN, payload, payload_len);
    if (rsn_ie == OSAL_NULL) {
        return;
    }

    /* 1. check OWE AKM */
    ret = hmac_ie_get_rsn_cipher(rsn_ie, &crypto);
    if (ret != OAL_SUCC) {
        return;
    }

    /* AKM Suite */
    for (suite_idx = 0; suite_idx < WLAN_AUTHENTICATION_SUITES; suite_idx++) {
        if (crypto.akm_suite[suite_idx] == MAC_RSN_AKM_OWE) {
            hmac_user->hmac_cap_info.owe = OAL_TRUE;
            break;
        }
    }
    if (hmac_user->hmac_cap_info.owe == OAL_FALSE) {
        return;
    }

    oam_warning_log3(0, OAM_SF_WPA, "hmac_ap_check_owe_capa::owe:%d, akm_suite[0]:0x%x akm_suite[1]:0x%x",
        hmac_user->hmac_cap_info.owe, crypto.akm_suite[0], crypto.akm_suite[1]);

    /* 2. check OWE Diffie-Hellman Parameter Element */
    /*************************************************************************/
    /*             OWE Diffie-Hellman Parameter element                      */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Element ID |  element-specific data            */
    /* |           |        | Extension  |  group        | public key        */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    1       |      2        |                   */
    /* --------------------------------------------------------------------- */
    /* |    255    |variable|    32      |   19/20/21    |                   */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    owe_dh = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_OWE_DH_PARAM, payload, payload_len);
    if (owe_dh == OSAL_NULL) {
        hmac_user->hmac_cap_info.owe = OAL_FALSE;
        return;
    }
    owe_dh += MAC_IE_HDR_LEN; /* skip EID and length */
    owe_group = *(owe_dh + 1);
    owe_group += (*(owe_dh + 2)) << 8; /* group len:2, 左移8位 */
    if ((owe_group == 0) || ((owe_group != MAC_OWE_GROUP_19) &&
        (owe_group != MAC_OWE_GROUP_20) && (owe_group != MAC_OWE_GROUP_21))) {
        *status_code = MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED; /* OWE group not matched */
    }
    oam_warning_log3(0, OAM_SF_WPA, "hmac_ap_check_owe_capa::associated STA owe=%d owe_group:%d, status:%d",
        hmac_user->hmac_cap_info.owe, owe_group, *status_code);

    return;
}

OAL_STATIC osal_void hmac_handle_owe_info_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_asoc_user_req_ie_stru asoc_user_req_info = {0};
    osal_s32 ret;
    frw_msg msg_info = {0};

    /* 上报内核的关联sta发送的关联请求帧ie信息 */
    asoc_user_req_info.assoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    asoc_user_req_info.assoc_req_ie_len   = hmac_user->assoc_req_ie_len;

    /* 关联的STA mac地址 */
    if (memcpy_s((osal_u8 *)asoc_user_req_info.user_mac_addr, sizeof(asoc_user_req_info.user_mac_addr),
        hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_handle_owe_info_ap::memcpy_s mac error}");
    }
    msg_info.data = (osal_u8 *)&asoc_user_req_info;
    msg_info.data_len = sizeof(hmac_asoc_user_req_ie_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_STA_OWE_INFO, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return;
    }

    return;
}
#endif

OAL_STATIC osal_void hmac_ap_rx_send_auth_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req, osal_u8 *chtxt)
{
    oal_netbuf_stru *auth_rsp = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u16 auth_rsp_len;
    mac_tx_ctl_stru *tx_ctrl = OAL_PTR_NULL;
    osal_u32 ret;
    osal_u32 pedding_data = 0;
    frw_msg msg_info;
    osal_u8 user_addr[6] = {0};
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    auth_rsp = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (auth_rsp == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_ap_rx_send_auth_rsp::auth_rsp null.}", hmac_vap->vap_id);
        return;
    }

    memset_s(oal_netbuf_cb(auth_rsp), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    auth_rsp_len = hmac_encap_auth_rsp_etc(hmac_vap, auth_rsp, auth_req, chtxt);
    if (auth_rsp_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_ap_rx_send_auth_rsp::auth_rsp_len is 0.}",
            hmac_vap->vap_id);
        mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)user_addr);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, (osal_u8 *)user_addr, AP_AUTH_RSP_ENCAP_ERR,
                USER_CONN_FAIL);
        }
        oal_netbuf_free(auth_rsp);
        return;
    }

    oal_netbuf_put(auth_rsp, auth_rsp_len);

    hmac_mgmt_update_auth_mib(hmac_vap, auth_rsp);

    /* 获取cb字段 */
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_rsp);
    /* 发送认证响应帧之前，将用户的节能状态复位 */
    /* hmac_encap_auth_rsp中user id的字段值非法，表明组帧失败, 直接回复失败的认证响应帧 */
    if (mac_get_cb_tx_user_idx(tx_ctrl) == MAC_INVALID_USER_ID) {
        oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {hmac_ap_rx_send_auth_rsp::user id[%d] auth fail.}",
            hmac_vap->vap_id, mac_get_cb_tx_user_idx(tx_ctrl));
    } else {
        hmac_user = mac_res_get_hmac_user_etc(mac_get_cb_tx_user_idx(tx_ctrl));
        if (hmac_user == OAL_PTR_NULL) {
#ifdef _PRE_WLAN_DFT_STAT
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, auth_rsp);
#endif
            oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_ap_rx_send_auth_rsp::hmac_user null.}",
                hmac_vap->vap_id);
            mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)user_addr);
            if (fhook != OSAL_NULL) {
                ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, (osal_u8 *)user_addr, AP_AUTH_FIND_USER_ERR,
                    USER_CONN_FAIL);
            }
            oal_netbuf_free(auth_rsp);
            return;
        }
        hmac_mgmt_reset_psm_etc(hmac_vap, mac_get_cb_tx_user_idx(tx_ctrl));
    }

    msg_info.data = (osal_u8 *)&pedding_data;
    msg_info.data_len = OAL_SIZEOF(osal_u32);
    hmac_config_scan_abort_etc(hmac_vap, &msg_info);

    /* 无论认证成功或者失败，都抛事件给dmac发送认证帧 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, auth_rsp, auth_rsp_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(auth_rsp);
        oam_warning_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_ap_rx_send_auth_rsp::hmac_tx_mgmt_send_event_etc failed[%d].}",
            hmac_vap->vap_id, ret);
        mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)user_addr);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, (osal_u8 *)user_addr, AP_AUTH_RSP_SEND_ERR,
                USER_CONN_FAIL);
        }
    }
}
/*
 * 函 数 名  : hmac_ap_re_rx_auth_protect_check
 * 功能描述  : 过滤接收到的重传auth帧
 * 修改历史      :
 *  1.日    期   : 2021年11月27日
 *    修改内容   : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 hmac_ap_re_rx_auth_protect_check(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *auth_req)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_ctrl = OAL_PTR_NULL;
    hmac_auth_re_rx_protect_stru *auth_re_rx_protect = OAL_PTR_NULL;
    hmac_auth_re_rx_protect_stru last_auth_re_rx_protect;
    osal_u32 time_diff;
    mac_ieee80211_frame_stru *frame_hdr = OAL_PTR_NULL;
    mac_header_frame_control_stru *frame_control = OAL_PTR_NULL;
    osal_u8  addr[WLAN_MAC_ADDR_LEN]  = {0};
    osal_u16 user_index = MAC_INVALID_USER_ID;
    osal_u16 auth_seq;

    rx_ctrl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(auth_req);
    if (osal_unlikely(rx_ctrl == NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_re_rx_auth_protect_check: rx_ctrl is null, drop auth!}");
        return OAL_TRUE;
    }

    auth_seq = mac_get_auth_seq_num(oal_netbuf_header(auth_req), mac_get_rx_cb_payload_len(rx_ctrl));
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(rx_ctrl);
    mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)addr);
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, (const osal_u8 *)addr, &user_index) != OAL_SUCC) {
        return OAL_FALSE;
    }

    /* 获取hmac用户 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_index);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    auth_re_rx_protect = &hmac_user->auth_re_rx_protect;
    memcpy_s(&last_auth_re_rx_protect, sizeof(hmac_auth_re_rx_protect_stru),
        auth_re_rx_protect, sizeof(hmac_auth_re_rx_protect_stru));
    auth_re_rx_protect->rx_auth_timestamp = (osal_u32)oal_time_get_stamp_ms();
    auth_re_rx_protect->rx_auth_seqnum = mac_get_seq_num((osal_u8 *)frame_hdr);
    auth_re_rx_protect->is_rx_auth = OAL_TRUE;
    /* 首帧auth不过滤 */
    if (last_auth_re_rx_protect.is_rx_auth == OAL_FALSE) {
        return OAL_FALSE;
    }
    /* OPEN下 auth seq num != 1,过滤 */
    if (mac_get_auth_algo_num(auth_req) == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        if (auth_seq != WLAN_AUTH_TRASACTION_NUM_ONE) {
            oam_warning_log1(0, OAM_SF_AUTH, "hmac_ap_re_rx_auth_protect_check invalid auth seq[%d]", auth_seq);
            return OAL_TRUE;
        }
    }

    /* 非重传帧不过滤 */
    frame_control = &(frame_hdr->frame_control);
    time_diff = oal_time_get_runtime(last_auth_re_rx_protect.rx_auth_timestamp, auth_re_rx_protect->rx_auth_timestamp);
    if ((frame_control->retry != OAL_TRUE) || (time_diff >= HMAC_RETRY_AUTH_PROTECT_DURATION)) {
        return OAL_FALSE;
    }
    oam_warning_log2(0, OAM_SF_AUTH, "user[%d] drop auth, diff = %dms", hmac_user->assoc_id, time_diff);
    return OAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_ap_rx_auth_req
 功能描述  : 处理认证请求帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_ap_rx_auth_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req)
{
    osal_u8         chtxt[WLAN_CHTXT_SIZE] = {0};
    osal_u8         chtxt_index;
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
    osal_u8         randnum[40 * WORD_WIDTH] = {0}; /* 40随机数个数 */
    osal_s32        ret;
#endif

    if ((hmac_vap == OAL_PTR_NULL) || (auth_req == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::param null.}");
        return;
    }
    /* 过滤重复auth帧 */
    if (hmac_ap_re_rx_auth_protect_check(hmac_vap, auth_req) == OAL_TRUE) {
        return;
    }

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
    ret = uapi_drv_cipher_trng_get_random_bytes(randnum, 40 * WORD_WIDTH); /* 每次上送给内核40个随机数 */
    if (ret != OAL_SUCC) {
        oam_info_log1(0, OAM_SF_CONN, "vap_id[%d] {hmac_ap_rx_auth_req:: trng get random failed}",
            hmac_vap->vap_id);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (mac_get_auth_algo_num(auth_req) == WLAN_WITP_AUTH_SAE) &&
        hmac_vap->cap_flag.wpa3 == OSAL_TRUE) {
        hmac_ap_sae_auth_proc(hmac_vap, auth_req);
        return;
    }
#endif

    oam_warning_log1(0, OAM_SF_CONN, "vap_id[%d] {hmac_ap_rx_auth_req:: AUTH_REQ rx}",
        hmac_vap->vap_id);

    if (mac_get_auth_algo_num(auth_req) == WLAN_WITP_AUTH_SHARED_KEY) {
        /* 获取challenge text */
        for (chtxt_index = 0; chtxt_index < WLAN_CHTXT_SIZE; chtxt_index++) {
            chtxt[chtxt_index] = oal_gen_random((osal_u32)oal_time_get_stamp_ms(), OSAL_TRUE);
        }
    }

    /* AP接收到STA发来的认证请求帧组相应的认证响应帧 */
    hmac_ap_rx_send_auth_rsp(hmac_vap, auth_req, (osal_u8 *)chtxt);
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_deauth_req
 功能描述  : 处理接收去认证帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_rx_deauth_req(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    oal_bool_enum_uint8 is_protected)
{
    osal_u8 *sa = OAL_PTR_NULL;
    osal_u8 *da = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u16 err_code;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
#ifdef _PRE_WLAN_FEATURE_PMF
    osal_u32 ret;
#endif

    if ((hmac_vap == OAL_PTR_NULL) || (mac_hdr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_rx_deauth_req::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);

    err_code = *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN));

    /* 增加接收到去认证帧时的维测信息 */
    oam_warning_log1(0, OAM_SF_CONN, "{hmac_ap_rx_deauth_req:: DEAUTH rx, reason code = %d}", err_code);
    oam_warning_log4(0, OAM_SF_CONN, "{hmac_ap_rx_deauth_req:: DEAUTH rx, sa[%2X:%2X:%2X:%2X:XX:XX]}",
        sa[0], sa[1], sa[2], sa[3]); /* 1、2、3表示MAC地址位 */

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, sa);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_AUTH, "vap_id[%d] {aput rx deauth frame, hmac_user null.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 检查是否需要发送SA query request */
    if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code_etc(hmac_user, is_protected, mac_hdr) == OAL_SUCC)) {
        /* 在关联状态下收到未加密的ReasonCode 6/7需要开启SA Query流程 */
        ret = hmac_start_sa_query_etc(hmac_vap, hmac_user,
            hmac_user->cap_info.pmf_active);
        if (ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }
        return OAL_SUCC;
    }
#endif

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    mac_rx_get_da((mac_ieee80211_frame_stru *)mac_hdr, &da);
    if ((ETHER_IS_MULTICAST(da) != OAL_TRUE) &&
        (is_protected != hmac_user->cap_info.pmf_active)) {
        oam_error_log3(0, OAM_SF_AUTH, "vap_id[%d] {hmac_ap_rx_deauth_req::PMF check failed %d %d.}",
            hmac_vap->vap_id,
            is_protected, hmac_user->cap_info.pmf_active);
        return OAL_FAIL;
    }
    /* 抛事件上报内核，已经去关联某个STA */
    hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr, AP_RCV_STA_DEAUTH,
            USER_CONN_OFFLINE);
    }
    hmac_user_del_etc(hmac_vap, hmac_user);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_user_sort_op_rates
 功能描述  : 按一定顺序将user中速率重新排序
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void  hmac_user_sort_op_rates(hmac_user_stru *hmac_user)
{
    osal_u8  loop;
    osal_u8  num_rates;
    osal_u8  min_rate;
    osal_u8  temp_rate;  /* 临时速率，用于数据交换 */
    osal_u8  index;
    osal_u8  temp_index; /* 临时索引，用于数据交换 */

    num_rates = hmac_user->op_rates.rs_nrates;

    for (loop = 0; loop < num_rates; loop++) {
        /* 记录当前速率为最小速率 */
        min_rate    = (hmac_user->op_rates.rs_rates[loop] & 0x7F);
        temp_index  = loop;

        /* 依次查找最小速率 */
        for (index = loop + 1; index < num_rates; index++) {
            /* 记录的最小速率大于如果当前速率 */
            if (min_rate > (hmac_user->op_rates.rs_rates[index] & 0x7F)) {
                /* 更新最小速率 */
                min_rate   = (hmac_user->op_rates.rs_rates[index] & 0x7F);
                temp_index = index;
            }
        }

        temp_rate = hmac_user->op_rates.rs_rates[loop];
        hmac_user->op_rates.rs_rates[loop] = hmac_user->op_rates.rs_rates[temp_index];
        hmac_user->op_rates.rs_rates[temp_index] = temp_rate;
    }

    /*******************************************************************
      重排11g模式的可操作速率，使11b速率都聚集在11a之前
      802.11a 速率:6、9、12、18、24、36、48、54Mbps
      802.11b 速率:1、2、5.5、11Mbps
      由于按由小到大排序后802.11b中的速率11Mbps在802.11a中，下标为5
      所以从第五位进行检验并排序。
    *******************************************************************/
    if (hmac_user->op_rates.rs_nrates == MAC_DATARATES_PHY_80211G_NUM) { /* 11g_compatibility mode */
        if ((hmac_user->op_rates.rs_rates[5] & 0x7F) == 0x16) { /* 11Mbps */ /* 5排序操作 */
            temp_rate = hmac_user->op_rates.rs_rates[5]; /* 5排序操作 */
            hmac_user->op_rates.rs_rates[5] = hmac_user->op_rates.rs_rates[4]; /* 5、4排序操作 */
            hmac_user->op_rates.rs_rates[4] = hmac_user->op_rates.rs_rates[3]; /* 4、3排序操作 */
            hmac_user->op_rates.rs_rates[3] = temp_rate; /* 3排序操作 */
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_update_sta_cap_info
 功能描述  : AP侧设置STA的能力信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8  hmac_ap_up_update_sta_cap_info(hmac_vap_stru *hmac_vap, osal_u16 us_cap_info,
    const hmac_user_stru *hmac_user, mac_status_code_enum_uint16 *pen_status_code)
{
    osal_u32                             ul_ret;
    mac_cap_info_stru *cap_info  = (mac_cap_info_stru *)(&us_cap_info);

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (pen_status_code == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ap_up_update_sta_cap_info::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* check bss capability info MAC,忽略MAC能力不匹配的STA */
    ul_ret = hmac_check_bss_cap_info_etc(us_cap_info, hmac_vap);
    if (ul_ret != OAL_TRUE) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ap_up_update_sta_cap_info::hmac_check_bss_cap_info_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        *pen_status_code = MAC_UNSUP_CAP;
        return OAL_FALSE;
    }

#ifdef _PRE_WIFI_DEBUG
    /* 如果以上各能力信息均满足关联要求，则继续处理其他能力信息 */
    mac_vap_check_bss_cap_info_phy_ap_etc(us_cap_info, hmac_vap);
#endif

    if ((cap_info->privacy == 0) &&
        (hmac_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP)) {
        *pen_status_code = MAC_UNSPEC_FAIL;
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_check_wpa_cipher_ap
 功能描述  : 检查关联请求中的wpaie的加密套件是否符合本地要求
 输入参数  : hmac_vap_stru                * hmac_vap
             osal_u8                   * ie
             osal_u32                    msg_len
             osal_u8                     uc_80211i_mode
             osal_u8                     offset
             mac_status_code_enum_uint16 * pen_status_code
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_u16 hmac_check_wpa_cipher_ap(hmac_vap_stru *hmac_vap, osal_u8 *ie)
{
    mac_crypto_settings_stru      crypto;

    if (hmac_ie_get_wpa_cipher(ie, &crypto) != OAL_SUCC) {
        return MAC_INVALID_INFO_ELMNT;
    }

    if (mac_mib_get_wpa_group_suite(hmac_vap) != crypto.group_suite) {
        return MAC_INVALID_GRP_CIPHER;
    }

    if (mac_mib_wpa_pair_match_suites(hmac_vap, crypto.pair_suite) == 0) {
        return MAC_INVALID_PW_CIPHER;
    }

    if (mac_mib_wpa_akm_match_suites(hmac_vap, crypto.akm_suite) == 0) {
        return MAC_INVALID_AKMP_CIPHER;
    }

    return MAC_SUCCESSFUL_STATUSCODE;
}
/*****************************************************************************
 函 数 名  : hmac_check_rsn_cipher_ap
 功能描述  : 检查关联请求中的rsnie的加密套件是否符合本地要求
 输入参数  : hmac_vap_stru                * hmac_vap
             osal_u8                   * ie
             osal_u32                    msg_len
             osal_u8                     uc_80211i_mode
             osal_u8                     offset
             mac_status_code_enum_uint16 * pen_status_code
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_u16 hmac_check_rsn_cipher_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *ie)
{
    mac_crypto_settings_stru     crypto;
    osal_u16                   rsn_cap;

    if (hmac_ie_get_rsn_cipher(ie, &crypto) != OAL_SUCC) {
        return MAC_INVALID_INFO_ELMNT;
    }

    if (mac_mib_get_rsn_group_suite(hmac_vap) != crypto.group_suite) {
        return MAC_INVALID_GRP_CIPHER;
    }

    if (mac_mib_rsn_pair_match_suites(hmac_vap, crypto.pair_suite) == 0) {
        return MAC_INVALID_PW_CIPHER;
    }

    if (mac_mib_rsn_akm_match_suites(hmac_vap, crypto.akm_suite) == 0) {
        return MAC_INVALID_AKMP_CIPHER;
    }

    rsn_cap = hmac_get_rsn_capability_etc(ie);
    /* 预认证能力检查 */
    if (mac_mib_get_pre_auth_actived(hmac_vap) != (rsn_cap & BIT0)) {
        return MAC_INVALID_RSN_INFO_CAP;
    }

    /* 本地强制，对端没有MFP能力 */
    if ((mac_mib_get_dot11_rsnamfpr(hmac_vap) == OSAL_TRUE) && ((rsn_cap & BIT7) == 0)) {
        return MAC_MFP_VIOLATION;
    }
    /* 对端强制，本地没有MFP能力 */
    if ((mac_mib_get_dot11_rsnamfpc(hmac_vap) == OSAL_FALSE) && ((rsn_cap & BIT6) != 0)) {
        return MAC_MFP_VIOLATION;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    if ((mac_mib_get_dot11_rsnamfpc(hmac_vap) == OSAL_TRUE) && ((rsn_cap & BIT7) != 0)) {
        hmac_user_set_pmf_active_etc(hmac_user, OSAL_TRUE);
    }
#endif
#ifdef _PRE_WLAN_FEATURE_WPA3
    if ((hmac_user->cap_info.use_wpa3 == OAL_TRUE) && (hmac_vap->cap_flag.wpa3 == OAL_TRUE)) {
        if ((rsn_cap & BIT7) == OAL_FALSE) {
            oam_warning_log0(0, OAM_SF_WPA, "{hmac_check_rsn_cipher_ap::sae refuse with NON MFP.}");
            return MAC_MFP_VIOLATION;
        }
    }
#endif
    return MAC_SUCCESSFUL_STATUSCODE;
}

/*****************************************************************************
 函 数 名  : hmac_check_rsn_ap
 功能描述  : AP检查(重)关联请求中的rsn/wpa 信息元素
             误码
 输出参数  : 无
 返 回 值  : 如不符合本地能力，需要返回相应的错误码
*****************************************************************************/
OAL_STATIC osal_u16 hmac_check_rsn_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u32 msg_len)
{
    osal_u8                   *rsn_ie      = OAL_PTR_NULL;
    osal_u8                   *wpa_ie      = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_WPA3
    osal_u16                  status;
#endif
    /* 若本地没有rsn能力,忽略检查ie。以增加兼容性 */
    if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_FALSE) {
        return MAC_SUCCESSFUL_STATUSCODE;
    }

    /* 获取RSN和WPA IE信息 */
    rsn_ie = mac_find_ie_etc(MAC_EID_RSN, payload, (osal_s32)msg_len);
    wpa_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, payload, (osal_s32)msg_len);
    if ((rsn_ie == OAL_PTR_NULL) && (wpa_ie == OAL_PTR_NULL)) {
        /* 非wps关联过程仍需要检查ie */
        if (mac_mib_get_WPSActive(hmac_vap) == OAL_TRUE) {
            return MAC_SUCCESSFUL_STATUSCODE;
        } else {
            return MAC_SUCCESSFUL_STATUSCODE;
        }
    }
#ifdef _PRE_WLAN_FEATURE_WPA3
    /* WPA3 asoc req帧处理 */
    if (hmac_user->hmac_cap_info.owe == OSAL_FALSE) {
        status = hmac_ap_assoc_req_wpa3_process(hmac_vap, hmac_user, rsn_ie);
        if (status != MAC_SUCCESSFUL_STATUSCODE) {
            return status;
        }
    }
#endif
    if ((rsn_ie != OAL_PTR_NULL) && ((hmac_vap->cap_flag.wpa2 == OAL_TRUE)
#ifdef _PRE_WLAN_FEATURE_WPA3
        || (hmac_vap->cap_flag.wpa3 == OAL_TRUE)
#endif
        )) {
        return hmac_check_rsn_cipher_ap(hmac_vap, hmac_user, rsn_ie);
    }

    if ((hmac_vap->cap_flag.wpa == OAL_TRUE) && (wpa_ie != OAL_PTR_NULL)) {
        return hmac_check_wpa_cipher_ap(hmac_vap, wpa_ie);
    }

    return MAC_CIPHER_REJ;
}

/*****************************************************************************
 函 数 名  : hmac_is_erp_sta
 功能描述  : 查看sta是否属于erp站点
 输入参数  : hmac_user_stru               hmac_user
 输出参数  : 无
 返 回 值  : OAL_TRUE: sta为erp站点
            OAL_FALSE:sta为非erp站点(11b站点)
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_is_erp_sta(const hmac_user_stru *hmac_user)
{
    osal_u32            loop;
    oal_bool_enum_uint8   is_erp_sta;

    /* 确认是否是erp 站点 */
    if (hmac_user->op_rates.rs_nrates <= MAC_NUM_DR_802_11B) {
        is_erp_sta = OAL_FALSE;
        for (loop = 0; loop < hmac_user->op_rates.rs_nrates; loop++) {
            /* 如果支持速率不在11b的1M, 2M, 5.5M, 11M范围内，则说明站点为支持ERP的站点 */
            if (((hmac_user->op_rates.rs_rates[loop] & 0x7F) != 0x2) &&
                ((hmac_user->op_rates.rs_rates[loop] & 0x7F) != 0x4) &&
                ((hmac_user->op_rates.rs_rates[loop] & 0x7F) != 0xb) &&
                ((hmac_user->op_rates.rs_rates[loop] & 0x7F) != 0x16)) {
                is_erp_sta = OAL_TRUE;
                break;
            }
        }
    } else {
        is_erp_sta = OAL_TRUE;
    }

    return is_erp_sta;
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_update_legacy_capability
 功能描述  : AP处理assoc req 中的capability info
 输入参数  : hmac_vap   : hmac vap结构体指针
             hmac_user  : hmac user结构体指针
             us_cap_info    : 帧体中cap info信息
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_up_update_legacy_capability(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u16 cap)
{
    mac_protection_stru  *protection = &(hmac_vap->protection);

    /* 如果STA不支持short slot */
    if ((cap & MAC_CAP_SHORT_SLOT) != MAC_CAP_SHORT_SLOT) {
        /* 如果STA之前没有关联， 或者之前以支持short slot站点身份关联，需要update处理 */
        if ((hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) ||
            (hmac_user->hmac_cap_info.short_slot_time == OAL_TRUE)) {
            protection->sta_no_short_slot_num++;
        }

        hmac_user->hmac_cap_info.short_slot_time = OAL_FALSE;
    } else { /* 如果STA支持short slot */
        /* 如果STA以不支持short slot站点身份关联，需要update处理 */
        if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) && (protection->sta_no_short_slot_num != 0) &&
            (hmac_user->hmac_cap_info.short_slot_time == OAL_FALSE)) {
            protection->sta_no_short_slot_num--;
        }

        hmac_user->hmac_cap_info.short_slot_time = OAL_TRUE;
    }

    hmac_user->user_stats_flag.no_short_slot_stats_flag = OAL_TRUE;

    /* 如果STA不支持short preamble */
    if ((cap & MAC_CAP_SHORT_PREAMBLE) != MAC_CAP_SHORT_PREAMBLE) {
        /* 如果STA之前没有关联， 或者之前以支持short preamble站点身份关联，需要update处理 */
        if ((hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) ||
            (hmac_user->hmac_cap_info.short_preamble == OAL_TRUE)) {
            protection->sta_no_short_preamble_num++;
        }

        hmac_user->hmac_cap_info.short_preamble = OAL_FALSE;
    } else { /* 如果STA支持short preamble */
        /* 如果STA之前以不支持short preamble站点身份关联，需要update处理 */
        if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) && (protection->sta_no_short_preamble_num != 0) &&
            (hmac_user->hmac_cap_info.short_preamble == OAL_FALSE)) {
            protection->sta_no_short_preamble_num--;
        }

        hmac_user->hmac_cap_info.short_preamble = OAL_TRUE;
    }

    hmac_user->user_stats_flag.no_short_preamble_stats_flag = OAL_TRUE;

    /* 确定user是否是erp站点 */
    /* 如果STA不支持ERP */
    if (hmac_is_erp_sta(hmac_user) == OAL_FALSE) {
        /* 如果STA之前没有关联， 或者之前以支持ERP站点身份关联，需要update处理 */
        if ((hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) || (hmac_user->hmac_cap_info.erp == OAL_TRUE)) {
            protection->sta_non_erp_num++;
        }

        hmac_user->hmac_cap_info.erp = OAL_FALSE;
    } else { /* 如果STA支持ERP */
        /* 如果STA之前以不支持ERP身份站点关联，需要update处理 */
        if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) &&
            (hmac_user->hmac_cap_info.erp == OAL_FALSE) && (protection->sta_non_erp_num != 0)) {
            protection->sta_non_erp_num--;
        }

        hmac_user->hmac_cap_info.erp = OAL_TRUE;
    }

    hmac_user->user_stats_flag.no_erp_stats_flag = OAL_TRUE;

    if ((cap & MAC_CAP_SPECTRUM_MGMT) != MAC_CAP_SPECTRUM_MGMT) {
        hmac_user_set_spectrum_mgmt_etc(hmac_user, OAL_FALSE);
    } else {
        hmac_user_set_spectrum_mgmt_etc(hmac_user, OAL_TRUE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_go_is_auth
 功能描述  : 判断go是否处于关联状态
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
oal_bool_enum hmac_go_is_auth(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head   *entry = OAL_PTR_NULL;
    struct osal_list_head   *dlist_tmp = OAL_PTR_NULL;
    hmac_user_stru        *hmac_user = OAL_PTR_NULL;

    if (hmac_vap->p2p_mode != WLAN_P2P_GO_MODE) {
        return OAL_FALSE;
    }

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_ap_is_auth::user_tmp null.}", hmac_vap->vap_id);
            continue;
        }
        if (hmac_user->user_asoc_state < MAC_USER_STATE_ASSOC) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

OAL_STATIC osal_void hmac_ap_up_user_update_vht(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    hmac_user_stru *hmac_user, osal_u32 msg_len)
{
    osal_u8 *vendor_vht_ie;
    osal_u8 *ie_tmp = OAL_PTR_NULL;
    osal_u32 vendor_vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
    vendor_vht_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE,
        payload, (osal_s32)msg_len);
    if ((vendor_vht_ie) && (vendor_vht_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        wifi_printf("vap_id[%d] {hmac_ap_up_user_update_vht::find broadcom/epigram vendor ie, enable hidden "
            "support_11ac2g}\r\n",
            hmac_vap->vap_id);

        /* 进入此函数代表user支持2G 11ac */
        ie_tmp = mac_find_ie_etc(MAC_EID_VHT_CAP, vendor_vht_ie + vendor_vht_ie_offset,
            (osal_s32)(vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
        if (ie_tmp != OAL_PTR_NULL) {
            hmac_user->user_vendor_vht_capable = OAL_TRUE;
            hmac_proc_vht_cap_ie_etc(hmac_vap, hmac_user, ie_tmp);
        } else { /* 表示支持5G 20M mcs9 */
            hmac_user->user_vendor_novht_capable = OAL_TRUE;
        }
    }
}

OAL_STATIC osal_bool hmac_is_sta_use_tkip(const hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 msg_len)
{
    osal_u8 *rsn_ie = OSAL_NULL;
    osal_u8 *wpa_ie = OSAL_NULL;
    mac_crypto_settings_stru crypto;

    /* 获取RSN和WPA IE信息 */
    rsn_ie = mac_find_ie_etc(MAC_EID_RSN, payload, (osal_s32)msg_len);
    wpa_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, payload, (osal_s32)msg_len);
    if ((rsn_ie == OSAL_NULL) && (wpa_ie == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    if ((rsn_ie != OSAL_NULL) && (hmac_vap->cap_flag.wpa2 == OAL_TRUE)) {
        if (hmac_ie_get_rsn_cipher(rsn_ie, &crypto) != OAL_SUCC) {
            return OSAL_FALSE;
        }

        /* 仅有一个单播密钥套件，且为tkip，套件1为空 */
        if ((crypto.pair_suite[0] == MAC_RSN_CHIPER_TKIP) && (crypto.pair_suite[1] == 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_is_sta_use_tkip::STA use wpa2 tkip.}");
            return OSAL_TRUE;
        }
    }

    if ((wpa_ie != OAL_PTR_NULL) && (hmac_vap->cap_flag.wpa == OAL_TRUE)) {
        if (hmac_ie_get_wpa_cipher(wpa_ie, &crypto) != OAL_SUCC) {
            return OSAL_FALSE;
        }
        /* 仅有一个单播密钥套件，且为tkip，套件1为空 */
        if ((crypto.pair_suite[0] == MAC_WPA_CHIPER_TKIP) && (crypto.pair_suite[1] == 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_is_sta_use_tkip::STA use wpa tkip.}");
            return OSAL_TRUE;
        }
    }
    return OSAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC osal_void hmac_ap_up_user_update_he(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    hmac_user_stru *hmac_user, osal_u32 msg_len, osal_bool is_tkip)
{
    osal_u8 *ie_tmp = OAL_PTR_NULL;

    memset_s(&(hmac_user->he_hdl), OAL_SIZEOF(mac_he_hdl_stru),
             0, OAL_SIZEOF(mac_he_hdl_stru));
    ie_tmp = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, payload, (osal_s32)msg_len);
    /* 支持ht能力，但是使用tkip算法，要置对端user为不支持he能力 */
    if ((ie_tmp != OSAL_NULL) && (is_tkip == OSAL_FALSE)) {
        hmac_proc_he_cap_ie(hmac_vap, hmac_user, ie_tmp);
    }
}

osal_s32 hmac_config_set_11ax_softap_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u16 softap_param;
    osal_u8 enable;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP || hmac_vap->protocol != WLAN_HE_MODE) {
        return OAL_FAIL;
    }

    softap_param = *(osal_u16 *)msg->data;
    enable = (softap_param & 0xFF00) == 0 ? OSAL_FALSE : OSAL_TRUE;
    mac_mib_set_he_ht_control_field_supported(hmac_vap, enable);
    mac_mib_set_he_om_control_support(hmac_vap, enable);
    if (enable == OSAL_TRUE) {
        mac_mib_set_he_oper_bss_color_disable(hmac_vap, OSAL_FALSE);
        mac_mib_set_he_oper_bss_color(hmac_vap, (softap_param & 0xFF));
        hal_set_bss_color(hmac_vap->hal_vap, mac_mib_get_he_oper_bss_color(hmac_vap));
    } else {
        mac_mib_set_he_oper_bss_color_disable(hmac_vap, OSAL_TRUE);
        mac_mib_set_he_oper_bss_color(hmac_vap, 0);
    }
    hal_set_bss_color_enable(enable);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_ap_up_update_sta_user
 功能描述  : AP侧更新STA信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_up_update_sta_user(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    osal_u8 *payload, osal_u32 msg_len, hmac_user_stru *hmac_user, mac_status_code_enum_uint16 *pen_status_code)
{
    osal_u16                  us_cap_info;
    mac_status_code_enum_uint16 ret_val;
    osal_u8                  *ie_tmp = OAL_PTR_NULL;
    wlan_bw_cap_enum_uint8      bandwidth_cap = WLAN_BW_CAP_BUTT;
    wlan_bw_cap_enum_uint8      bwcap_ap;        /* ap自身带宽能力 */
    osal_u32                  ul_ret;
    wlan_bw_cap_enum_uint8      bwcap_vap;
    osal_u8                   avail_mode;
    osal_u32                  remain_len = 0;
    osal_void *fhook = OAL_PTR_NULL;
    osal_bool is_tkip;
    osal_u8 *payload_tmp = payload;
    osal_u32 msg_len_tmp = msg_len;

    *pen_status_code = MAC_SUCCESSFUL_STATUSCODE;

    /***************************************************************************
        检查AP是否支持当前正在关联的STA的所有能力
        |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|Reserved|
    ***************************************************************************/
    us_cap_info = OAL_MAKE_WORD16(payload_tmp[0], payload_tmp[1]);

    ul_ret = hmac_ap_up_update_sta_cap_info(hmac_vap, us_cap_info, hmac_user, pen_status_code);
    if (ul_ret != OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::hmac_ap_up_update_sta_cap_info failed[%d], status_code=%d.}",
            hmac_vap->vap_id, ul_ret, *pen_status_code);
        return ul_ret;
    }

    payload_tmp += (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);
    msg_len_tmp -= (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);

    if (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_REASSOC_REQ | WLAN_FC0_TYPE_MGT)) {
        payload_tmp += WLAN_MAC_ADDR_LEN;
        msg_len_tmp -= WLAN_MAC_ADDR_LEN;
    }

    /* 判断SSID,长度或内容不一致时,认为是SSID不一致，考虑兼容性找不到ie时不处理 */
    ie_tmp = mac_find_ie_sec_etc(MAC_EID_SSID, payload_tmp, (osal_s32)msg_len_tmp, &remain_len);
    if ((ie_tmp != OAL_PTR_NULL) && (remain_len > 2)) { /* remain len is 2 */
        if ((ie_tmp[1] != (osal_u8)osal_strlen((osal_char *)mac_mib_get_desired_ssid(hmac_vap))) ||
            /* 2表示DesiredSSID */
            (osal_memcmp(&ie_tmp[2], mac_mib_get_desired_ssid(hmac_vap), ie_tmp[1]) != 0)) {
            *pen_status_code = MAC_UNSPEC_FAIL;
            oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_update_sta_user::ssid mismatch.}",
                hmac_vap->vap_id);
            return OAL_FAIL;
        }
    }

    /* 当前用户已关联 */
    ul_ret = hmac_ie_proc_assoc_user_legacy_rate(payload_tmp, msg_len_tmp, hmac_user);
    if (ul_ret != OAL_SUCC) {
        *pen_status_code = MAC_UNSUP_RATE;
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_update_sta_user::rates mismatch ret[%d].}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }
    /* 按一定顺序重新排列速率 */
    hmac_user_sort_op_rates(hmac_user);

    /* 更新对应STA的legacy协议能力 */
    ul_ret = hmac_ap_up_update_legacy_capability(hmac_vap, hmac_user, us_cap_info);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_ap_up_update_sta_user::hmac_ap_up_update_legacy_capability failed[%d].}", ul_ret);
    }

    /* 更新对应STA的RRM能力 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_AP_UPDATE_RRM_CAP);
    if (fhook != OSAL_NULL) {
        ((hmac_ap_up_update_rrm_capability_cb)fhook)(hmac_user, us_cap_info, payload_tmp, msg_len_tmp);
    }

    is_tkip = hmac_is_sta_use_tkip(hmac_vap, payload_tmp, msg_len_tmp);
    /* 检查HT capability以及Extend capability是否匹配，并进行处理  */
    ret_val = hmac_vap_check_ht_capabilities_ap_etc(hmac_vap, payload_tmp, msg_len_tmp, hmac_user, is_tkip);
    if (ret_val != MAC_SUCCESSFUL_STATUSCODE) {
        *pen_status_code = ret_val;
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::Reject STA because of ht_capability[%d]}", hmac_vap->vap_id,
            ret_val);
        return ret_val;
    }

    /* 更新AP中保护相关mib量 */
    ul_ret = hmac_user_protection_sync_data(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::protection update mib failed, ret=%d.}", hmac_vap->vap_id, ul_ret);
    }

    /* 更新对应STA的协议能力 update_asoc_entry_prot(ae, msa, rx_len, cap_info, is_p2p); */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_UPDATE_USER_PARA);
    if (fhook != OSAL_NULL && ((hmac_uapsd_update_user_para_etc_cb)fhook)(payload_tmp,
            mac_get_frame_type_and_subtype(mac_hdr), msg_len_tmp, hmac_user) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_ap_up_update_sta_user::hmac_ap_up_update_asoc_entry_prot failed[%d].}", ul_ret);
    }

    /* 更新QoS能力 */
    hmac_mgmt_update_assoc_user_qos_table_etc(payload_tmp, (osal_u16)msg_len_tmp, hmac_user);

    /* 更新11ac VHT capabilities ie */
    ie_tmp = mac_find_ie_etc(MAC_EID_VHT_CAP, payload_tmp, (osal_s32)msg_len_tmp);
    if (ie_tmp != OAL_PTR_NULL) {
#ifndef _PRE_WLAN_FEATURE_11AC2G
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
        } else {
#endif
        hmac_proc_vht_cap_ie_etc(hmac_vap, hmac_user, ie_tmp);
#ifndef _PRE_WLAN_FEATURE_11AC2G
        }
#endif
#ifdef _PRE_WLAN_FEATURE_1024QAM

        ie_tmp = hmac_find_vendor_ie_etc(MAC_EXT_VENDER_IE, MAC_OSAL_1024QAM_IE, payload_tmp, msg_len_tmp);
        if (ie_tmp != OAL_PTR_NULL) {
            hmac_user->cap_info.cap_1024qam = OAL_TRUE;
        }
#endif
    } else if (hmac_vap->protocol == WLAN_VHT_ONLY_MODE) {
        /* 不允许不支持11ac STA关联11aconly 模式的AP */
        oam_warning_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user:AP 11ac only, but STA not support 11ac}", hmac_vap->vap_id);
        return MAC_MISMATCH_VHTCAP;
    }
#ifndef _PRE_WLAN_FEATURE_11AC2G
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
    } else {
#endif
    /* APUT根据BRCM VENDOR OUI 适配2G 11AC */
    if (hmac_user->vht_hdl.vht_capable == OAL_FALSE) {
        hmac_ap_up_user_update_vht(hmac_vap, payload_tmp, hmac_user, msg_len_tmp);
    }
#ifndef _PRE_WLAN_FEATURE_11AC2G
    }
#endif
    /* 检查接收到的ASOC REQ消息中的SECURITY参数.如出错,则返回对应的错误码 */
    hmac_user_init_key_etc(hmac_user);
    ret_val = hmac_check_rsn_ap(hmac_vap, hmac_user, payload_tmp, msg_len_tmp);
    if (ret_val != MAC_SUCCESSFUL_STATUSCODE) {
        *pen_status_code = ret_val;
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_update_sta_user::hmac_check_rsn_ap fail[%d].}",
            hmac_vap->vap_id, ret_val);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_ap_up_user_update_he(hmac_vap, payload_tmp, hmac_user, msg_len_tmp, is_tkip);
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    if (hmac_vap->cap_flag.wur_enable == OAL_TRUE) {
        /* 解析关联请求帧中的WUR CAP信息 */
        ie_tmp = hmac_find_ie_ext_ie(MAC_EID_WUR, MAC_EID_EXT_WUR_CAP, payload_tmp, (osal_s32)msg_len_tmp);
        if (ie_tmp != OSAL_NULL) {
            hmac_proc_wur_cap_ie(hmac_vap, hmac_user, ie_tmp);
        }
    }
#endif

    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode_etc(hmac_vap, hmac_user);

    avail_mode = g_auc_avail_protocol_mode_etc[hmac_vap->protocol][hmac_user->protocol_mode];
    if (avail_mode == WLAN_PROTOCOL_BUTT) {
        oam_warning_log4(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::user not allowed:vap mode=%d, user mode=%d,user avail mode=%d.}",
            hmac_vap->vap_id,
            hmac_vap->protocol, hmac_user->protocol_mode, hmac_user->avail_protocol_mode);
        *pen_status_code = MAC_UNSUP_CAP;
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((hmac_vap->protocol == WLAN_HT_MODE) && (hmac_user->protocol_mode == WLAN_VHT_MODE) &&
        (hmac_vap->cap_flag.support_11ac2g == OAL_TRUE) &&
        (hmac_vap->channel.band == WLAN_BAND_2G)) {
        avail_mode = WLAN_VHT_MODE;
    }
#endif

    /* 获取用户与VAP协议模式交集 */
    hmac_user_set_avail_protocol_mode_etc(hmac_user, avail_mode);
    hmac_user_set_cur_protocol_mode_etc(hmac_user, hmac_user->avail_protocol_mode);

    oam_warning_log4(0, OAM_SF_ASSOC,
        "vap[%d] {hmac_ap_up_update_sta_user:vap protocol:%d,user protocol_mode:%d,avail_protocol_mode:%d}",
        hmac_vap->vap_id, hmac_vap->protocol, hmac_user->protocol_mode, hmac_user->avail_protocol_mode);

    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates_etc(hmac_vap, hmac_user);

    /* 获取用户的带宽能力 */
    hmac_user_get_sta_cap_bandwidth_etc(hmac_user, &bandwidth_cap);

    /* 获取vap带宽能力与用户带宽能力的交集 */
    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bwcap_ap);
    bwcap_vap = OAL_MIN(bwcap_ap, bandwidth_cap);
    hmac_user_set_bandwidth_info_etc(hmac_user, bwcap_vap, bwcap_vap);

    oam_warning_log4(0, OAM_SF_ASSOC,
        "vap[%d] {hmac_ap_up_update_sta_user:mac_vap bandwidth:%d,mac_user bandwidth:%d,cur_bandwidth:%d}",
        hmac_vap->vap_id, bwcap_ap, bandwidth_cap, hmac_user->cur_bandwidth);

    /* 获取用户与VAP空间流交集 */
    ul_ret = hmac_user_set_avail_num_space_stream_etc(hmac_user, hmac_vap->vap_rx_nss);
    if (ul_ret != OAL_SUCC) {
        *pen_status_code = MAC_UNSPEC_FAIL;
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::mac_user_set_avail_num_space_stream failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }

    /* 根据asoc req中的smps能力位更新user信息 */
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 根据smps更新空间流能力 */
    if (!is_vap_single_nss(hmac_vap) && !is_user_single_nss(hmac_user)) {
        hmac_smps_update_user_status(hmac_vap, hmac_user);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 处理Operating Mode Notification 信息元素 */
    ul_ret = hmac_check_opmode_notify_etc(hmac_vap, mac_hdr, payload_tmp, msg_len_tmp, hmac_user);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_sta_user::hmac_check_opmode_notify_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }
#endif

    return MAC_SUCCESSFUL_STATUSCODE;
}

/*****************************************************************************
 函 数 名  : hmac_ap_save_user_assoc_req
 功能描述  : AP 保存STA 的关联请求帧信息，以备上报内核
 输入参数  : hmac_user_stru *hmac_user
             osal_u8 *payload
             osal_u8 *len
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_ap_save_user_assoc_req(hmac_user_stru *hmac_user, osal_u8 *payload, osal_u32 payload_len,
    osal_u8 mgmt_frm_type)
{
    osal_u32 ul_ret;

    /* AP 保存STA 的关联请求帧信息，以备上报内核 */
    ul_ret = hmac_user_free_asoc_req_ie(hmac_user);
    if (ul_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_ap_save_user_assoc_req :: hmac_user_free_asoc_req_ie fail.}");
        return OAL_FAIL;
    }

    /* 目前11r没有实现，所以处理重关联帧的流程和关联帧一样，11r实现后此处需要修改 */
    return hmac_user_set_asoc_req_ie(hmac_user,
                                     payload + MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN,
                                     payload_len - MAC_CAP_INFO_LEN - MAC_LIS_INTERVAL_IE_LEN,
                                     (osal_u8)(mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ));
}

#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC osal_u32 hmac_ap_check_rsn_valid(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                            osal_u8 mgmt_frm_type, osal_u8 *payload, osal_u32 payload_len)
{
    osal_u16  ret;
    osal_u32 msg_len = payload_len;
    osal_u8 *payload_tmp = payload;

    payload_tmp += (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);
    msg_len -= (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);
    if (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        payload_tmp += WLAN_MAC_ADDR_LEN;
        msg_len -= WLAN_MAC_ADDR_LEN;
    }
    ret = hmac_check_rsn_ap(hmac_vap, hmac_user, payload_tmp, msg_len);
    if (ret != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log0(0, OAM_SF_ASSOC,
            "{hmac_ap_up_rx_asoc_sa_query::RSN is mismached.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
OAL_STATIC osal_u32 hmac_ap_up_rx_asoc_req_pmf_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_status_code_enum_uint16 *status_code, osal_u8 mgmt_frm_type, osal_u8 *payload, osal_u32 payload_len)
{
    osal_u32                      rslt;
    if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_user->cap_info.pmf_active == OAL_TRUE)) {
        rslt = hmac_ap_check_rsn_valid(hmac_vap, hmac_user, mgmt_frm_type, payload, payload_len);
        if (rslt != OAL_SUCC) {
            return rslt;
        }
        rslt = hmac_start_sa_query_etc(hmac_vap, hmac_user,
            hmac_user->cap_info.pmf_active);
        if (rslt != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_start_sa_query_etc failed[%d].}",
                hmac_user->vap_id, rslt);
            return rslt;
        }
        oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_asoc_req::set status_code is MAC_REJECT_TEMP.}",
            hmac_user->vap_id);
        *status_code = MAC_REJECT_TEMP;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC osal_u32 hmac_ap_send_asoc_rsp(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_status_code_enum_uint16 status_code, osal_u8 *sta_addr)
{
    oal_netbuf_stru *asoc_rsp = OAL_PTR_NULL;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    osal_u32 asoc_rsp_len = 0;
    oal_net_device_stru *net_device = OAL_PTR_NULL;
    osal_u32 rslt;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    osal_void *fhook_clear_defrag = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_DERAG_CLEAR);

    asoc_rsp = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (asoc_rsp == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_asoc_req::asoc_rsp null.}", hmac_user->vap_id);
        /* 异常返回之前删除user */
        hmac_user_del_etc(hmac_vap, hmac_user);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(asoc_rsp);
    memset_s(tx_ctl, OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    if (hmac_user->mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ) {
        asoc_rsp_len = hmac_mgmt_encap_asoc_rsp_ap_etc(hmac_vap, sta_addr, hmac_user,
            status_code, OAL_NETBUF_HEADER(asoc_rsp));
    } else if (hmac_user->mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) {
        asoc_rsp_len = hmac_mgmt_encap_asoc_rsp_ap_etc(hmac_vap, sta_addr, hmac_user,
            status_code, OAL_NETBUF_HEADER(asoc_rsp));
        if (fhook_clear_defrag != OSAL_NULL) { /* 重关联流程中清除user下的分片缓存，防止重关联或者rekey流程报文重组attack */
            ((hmac_user_clear_defrag_res_cb)fhook_clear_defrag)(hmac_user);
        }
    }

    if (asoc_rsp_len == 0) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_asoc_req::encap msg fail.}", hmac_user->vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr,
                AP_ASOC_RSP_ENCAP_ERR, USER_CONN_FAIL);
        }
        /* 异常返回之前删除user */
        hmac_user_del_etc(hmac_vap, hmac_user);
        oal_netbuf_free(asoc_rsp);
        return OAL_FAIL;
    }

    oal_netbuf_put(asoc_rsp, asoc_rsp_len);

    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_mpdu_len(tx_ctl) = (osal_u16)asoc_rsp_len;

    /* 发送关联响应帧之前，将用户的节能状态复位 */
    hmac_mgmt_reset_psm_etc(hmac_vap, mac_get_cb_tx_user_idx(tx_ctl));

    /* 判断当前状态，如果用户已经关联成功则向上报用户离开信息 */
    if (hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) {
        net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
        if (net_device != OAL_PTR_NULL) {
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(_PRE_PRODUCT_ID_HOST)
            oal_kobject_uevent_env_sta_leave_etc(net_device, sta_addr);
#endif
        }
    }

    if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_ASSOC);
    }
    rslt = hmac_tx_mgmt_send_event_etc(hmac_vap, asoc_rsp, (osal_u16)asoc_rsp_len);
    if (rslt != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_tx_mgmt_send_event_etc failed[%d].}", hmac_user->vap_id, rslt);
        oal_netbuf_free(asoc_rsp);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr,
                AP_ASOC_RSP_SEND_ERR, USER_CONN_FAIL);
        }
        /* 异常返回之前删除user */
        hmac_user_del_etc(hmac_vap, hmac_user);
        return rslt;
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_ap_up_rx_asoc_req_change_user_state_to_auth(hmac_user_stru *hmac_user)
{
    if (hmac_user->flag_change_user_state_to_auth_complete) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{hmac_ap_up_rx_asoc_req_change_user_state_to_auth::rx (re)assoc req. change user to auth complete.}");
        hmac_user->flag_change_user_state_to_auth_complete = OAL_FALSE;
        hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_asoc_req
 功能描述  : AP侧接收到对应STA的关联请求消息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_ap_up_rx_asoc_req(
    hmac_vap_stru                  *hmac_vap,
    osal_u8                       mgmt_frm_type,
    osal_u8                      *mac_hdr,
    osal_u32                      mac_hdr_len,
    osal_u8                      *payload,
    osal_u32                      payload_len)
{
    osal_u32                      rslt;
    hmac_user_stru                 *hmac_user = OAL_PTR_NULL;
    osal_u16                      user_idx = 0;
    mac_status_code_enum_uint16     status_code;
    osal_u8                       sta_addr[WLAN_MAC_ADDR_LEN];
    osal_s32 len;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    unref_param(mac_hdr_len);

    mac_get_address2(mac_hdr, (osal_u8 *)sta_addr);

    rslt = hmac_vap_find_user_by_macaddr_etc(hmac_vap, (const osal_u8 *)sta_addr, &user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_vap_find_user_by_macaddr_etc fail[%d].send DEAUTH,err code[%d]}",
            hmac_vap->vap_id, rslt, MAC_ASOC_NOT_AUTH);
        oam_warning_log4(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::user mac:%02X:%02X:%02X:%02X:XX:XX.}",
            /* 0:1:2:3:数组下标 */
            sta_addr[0], sta_addr[1], sta_addr[2], sta_addr[3]);
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, (osal_u8 *)sta_addr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return rslt;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::hmac_user[%d] null.send DEAUTH,err code[%d]}",
            user_idx, MAC_ASOC_NOT_AUTH);

        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, (osal_u8 *)sta_addr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_ap_up_rx_asoc_req_change_user_state_to_auth(hmac_user);

    if (hmac_user->mgmt_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&hmac_user->mgmt_timer);
    }
    hmac_user->mgmt_frm_type = mgmt_frm_type;

    status_code = MAC_SUCCESSFUL_STATUSCODE;
    len = ((mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ? (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN +
        WLAN_MAC_ADDR_LEN) : (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN));
#ifdef _PRE_WLAN_FEATURE_WPA3
    /* check OWE capability in assoc req frame, must before check pmkid */
    hmac_ap_check_owe_capa(hmac_vap, hmac_user, payload + len, (osal_s32)payload_len - len, &status_code);
#endif

    /* 是否符合触发SA query流程的条件 */
#ifdef _PRE_WLAN_FEATURE_PMF
    rslt = hmac_ap_up_rx_asoc_req_pmf_process(hmac_vap, hmac_user, &status_code, mgmt_frm_type, payload, payload_len);
    if (rslt != OAL_SUCC) {
        return rslt;
    }
#endif

    if ((status_code != MAC_REJECT_TEMP) && (status_code != MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED)) {
        /* 当可以查找到用户时,说明当前USER的状态为已关联或已认证完成 处理用户相关信息以及能力交互 */
        oam_warning_log4(0, OAM_SF_CONN,
            "{hmac_ap_up_rx_asoc_req:: ASSOC_REQ rx : user mac:%02X:%02X:%02X:%02X:XX:XX.}",
            /* 0:1:2:3:数组下标 */
            sta_addr[0], sta_addr[1], sta_addr[2], sta_addr[3]);
        if (hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) {
            oam_warning_log0(0, OAM_SF_ASSOC,
                             "{hmac_ap_up_rx_asoc_req::user associated, unexpected (re)assoc req no handle!}");
            return OAL_FAIL;
        }
        rslt = hmac_ap_up_update_sta_user(hmac_vap, mac_hdr, payload, payload_len, hmac_user,
            &status_code);
        if (status_code != MAC_SUCCESSFUL_STATUSCODE) {
            oam_warning_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_ap_up_update_sta_user failed[%d].}",
                hmac_user->vap_id, status_code);
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
        }

        /* 同步ap带宽，能力等信息到dmac */
        hmac_chan_sync_etc(hmac_vap, hmac_vap->channel.chan_number,
            mac_vap_get_cap_bw(hmac_vap), OAL_FALSE);

        /* 根据用户支持带宽能力，协商出当前带宽，dmac offload架构下，同步带宽信息到device */
        rslt = hmac_config_user_info_syn_etc(hmac_vap, hmac_user);
        if (rslt != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::usr_info_syn failed[%d].}",
                hmac_vap->vap_id, rslt);
        }

        if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
            rslt = hmac_init_security_etc(hmac_vap, (const osal_u8 *)sta_addr);
            if (rslt != OAL_SUCC) {
                oam_error_log3(0, OAM_SF_ASSOC,
                    "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_init_security_etc failed[%d] status_code[%d].}",
                    hmac_user->vap_id, rslt, MAC_UNSPEC_FAIL);
                status_code = MAC_UNSPEC_FAIL;
            }

            rslt = hmac_init_user_security_port_etc(hmac_vap, hmac_user);
            if (rslt != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_ASSOC,
                    "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_init_user_security_port_etc failed[%d].}",
                    hmac_vap->vap_id, rslt);
            }
        }

        if ((rslt != OAL_SUCC) || (status_code != MAC_SUCCESSFUL_STATUSCODE)) {
            oam_warning_log3(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_ap_up_update_sta_user fail rslt[%d] status_code[%d].",
                hmac_vap->vap_id, rslt, status_code);
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
            if (fhook != OSAL_NULL) {
                ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr,
                    AP_ASOC_NEGO_CAPABILITY_ERR, USER_CONN_FAIL);
            }
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        if (is_p2p_go(hmac_vap) &&
            (hmac_find_vendor_ie_etc(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, payload + len,
                (osal_s32)payload_len - len) == OAL_PTR_NULL)) {
            oam_info_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::GO got assoc request from legacy device, length = [%d]}",
                hmac_user->vap_id, payload_len);
            hmac_disable_p2p_pm_etc(hmac_vap);
        }
#endif

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        /* 调用check接口校验用户是否支持4地址 是则将用户的4地址传输能力位置1 */
        if (hmac_vap->wds_table.wds_vap_mode == WDS_MODE_ROOTAP) {
            if (hmac_vmsta_check_user_a4_support(mac_hdr, mac_hdr_len + payload_len)) {
                hmac_user->is_wds = OAL_TRUE;
                oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_asoc_req::user surpport 4 address.}",
                    hmac_user->vap_id);
            }
        }
#endif // _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    }

#ifdef _PRE_WLAN_FEATURE_WPA3
    /* APUT received OWE assoc req && check valid OWE DH Parameter Element, report assoc req to hostapd;
     * otherwise, APUT send assoc rsp frame with reject status code */
    oam_warning_log2(0, OAM_SF_CONN, "{hmac_ap_up_rx_asoc_req::owe=%u, status_code=%u}",
        hmac_user->hmac_cap_info.owe, status_code);
    if ((hmac_user->hmac_cap_info.owe != OAL_TRUE) || (status_code != MAC_SUCCESSFUL_STATUSCODE))
#endif
    {
        rslt = hmac_ap_send_asoc_rsp(hmac_vap, hmac_user, status_code, (osal_u8 *)sta_addr);
        if (rslt != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_ap_up_rx_asoc_req::hmac_ap_send_asoc_rsp failed[%d].}", hmac_vap->vap_id, rslt);
            return rslt;
        }
    }
        if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
            /* 为了解决wavetest仪器MCS9 shortGI上行性能低的问题:wavetest测试场景下，AGC固定绑定通道0 */
            rslt = hmac_config_user_rate_info_syn_etc(hmac_vap,
                hmac_user);
            if (rslt != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_ASSOC,
                    "vap_id[%d] {hmac_sta_wait_asoc_rx_etc::hmac_config_user_rate_info_syn_etc failed[%d].}",
                    hmac_vap->vap_id, rslt);
            }

            /*  user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
            hmac_user_add_notify_alg_etc(hmac_vap, user_idx);

            /* AP 保存STA 的关联请求帧信息，以备上报内核 */
            hmac_ap_save_user_assoc_req(hmac_user, payload, payload_len, mgmt_frm_type);
#ifdef _PRE_WLAN_FEATURE_WPA3
            if (hmac_user->hmac_cap_info.owe == OAL_TRUE) {
                /* 上报WAL层(WAL上报内核) STA OWE INFO */
                hmac_handle_owe_info_ap(hmac_vap, hmac_user);
                oam_warning_log1(0, OAM_SF_CONN, "{hmac_ap_up_rx_asoc_req::send sta owe info!STA_indx=%u}", user_idx);
            } else
#endif
            {
                /* 上报WAL层(WAL上报内核) AP关联上了一个新的STA */
                hmac_handle_connect_rsp_ap(hmac_vap, hmac_user);
                oam_warning_log1(0, OAM_SF_CONN, "{hmac_ap_up_rx_asoc_req::ASSOC_RSP tx,STA assoc AP SUCC!STA_indx=%u}",
                    user_idx);
            }

            oam_warning_log4(0, OAM_SF_CONN, "{hmac_ap_up_rx_asoc_req::user mac:%02X:%02X:%02X:%02X:XX:XX}",
                /* 0:1:2:3:数组下标 */
                sta_addr[0], sta_addr[1], sta_addr[2], sta_addr[3]);
        } else if (status_code != MAC_REJECT_TEMP) {
            /* AP检测STA失败，将其删除 */
            if ((hmac_vap->mib_info->wlan_mib_privacy.dot11_rsnamfpc == OAL_FALSE &&
                status_code == MAC_INVALID_AKMP_CIPHER)
#ifdef _PRE_WLAN_FEATURE_WPA3
                || status_code == MAC_STATUS_INVALID_PMKID
#endif
            ) {
                oam_warning_log2(0, OAM_SF_DFT, "hmac_ap_up_rx_asoc_req:assoc status code[%d],send DEAUTH,err code[%d]",
                    status_code, MAC_AUTH_NOT_VALID);
                hmac_mgmt_send_deauth_frame_etc(hmac_vap,
                    hmac_user->user_mac_addr, MAC_AUTH_NOT_VALID, OAL_FALSE);
            }
#ifdef _PRE_WLAN_FEATURE_WPA3
            if ((hmac_user->cap_info.use_wpa3 == OAL_TRUE) && (hmac_vap->cap_flag.wpa3 == OAL_TRUE)) {
                /* wpa3需上报内核删除用户，hostapd侧在auth成功后创建了sta信息 */
                oam_warning_log0(0, OAM_SF_CONN, "hmac_ap_up_rx_asoc_req::use wpa3 need send disconnet sta");
                hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);
            }
#endif
            hmac_user_del_etc(hmac_vap, hmac_user);
        }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC osal_u32 hmac_ap_up_rx_disasoc_sa_query(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    if (hmac_start_sa_query_etc(hmac_vap, hmac_user,
        hmac_user->cap_info.pmf_active) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_rx_disasoc_sa_query::hmac_start_sa_query_etc failed.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_disasoc
 功能描述  : AP侧接收到对应STA的去关联请求消息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_up_rx_disasoc(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    osal_u32 mac_hdr_len, osal_u8 *payload, osal_u32 payload_len, oal_bool_enum_uint8 is_protected)
{
    osal_u32 ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u8 *da = OAL_PTR_NULL;
    osal_u8 *sa = OAL_PTR_NULL;
    osal_u8 sta_addr[WLAN_MAC_ADDR_LEN];
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    unref_param(mac_hdr_len);
    unref_param(payload);
    unref_param(payload_len);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_up_rx_disasoc::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_get_address2(mac_hdr, (osal_u8 *)sta_addr);

    /* 增加接收到去关联帧时的维测信息 */
    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);

    oam_warning_log1(0, OAM_SF_CONN, "{DISASSOC rx, err_code[%d].}", *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)));
    /* 0、1、2、3表示MAC相应位数 */
    oam_warning_log4(0, OAM_SF_CONN, "{recv disassoc frame %02X:%02X:%02X:%02X:XX:XX.}", sa[0], sa[1], sa[2], sa[3]);

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, (const osal_u8 *)sta_addr);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_disasoc::send DEAUTH,err code[%d]}", MAC_NOT_ASSOCED);
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, (osal_u8 *)sta_addr, MAC_NOT_ASSOCED, OAL_FALSE);

        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_disasoc::ap rx a disaasoc req.}", hmac_vap->vap_id);

    if (hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) {
#ifdef _PRE_WLAN_FEATURE_PMF
        /* 检查是否需要发送SA query request */
        if (hmac_pmf_check_err_code_etc(hmac_user, is_protected, mac_hdr) == OAL_SUCC) {
            /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
            return hmac_ap_up_rx_disasoc_sa_query(hmac_vap, hmac_user);
        }
#endif

        /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
        mac_rx_get_da((mac_ieee80211_frame_stru *)mac_hdr, &da);
        if ((ETHER_IS_MULTICAST(da) != OAL_TRUE) &&
            (is_protected != hmac_user->cap_info.pmf_active)) {
            oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {PMF check failed,[%d].}", hmac_vap->vap_id, is_protected);
            return OAL_FAIL;
        }

        hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);

        /* 抛事件上报内核，已经去关联某个STA */
        hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);

        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr, AP_RCV_STA_DISASOC,
                USER_CONN_OFFLINE);
        }
        /* 有些网卡去关联时只发送DISASOC,也将删除其在AP内部的数据结构 */
        ret = hmac_user_del_etc(hmac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_user_del_etc failed[%d].}", hmac_vap->vap_id, ret);
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_ap_up_rx_action_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *data)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            hmac_mgmt_rx_addba_req_etc(hmac_vap, hmac_user, data);
            break;

        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp_etc(hmac_vap, hmac_user, data);
            break;

        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba_etc(hmac_vap, hmac_user, data);
            break;

        default:
            break;
    }
}

OAL_STATIC osal_void hmac_ap_up_rx_action_public(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u8 *data)
{
    unref_param(hmac_vap);
    unref_param(netbuf);
    if (data[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_VENDOR_SPECIFIC) {
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (hmac_ie_check_p2p_action_etc(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
            }
#endif  /* _PRE_WLAN_FEATURE_P2P */
    }
}

#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC osal_void hmac_ap_up_rx_action_sa_query(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 is_protected, const osal_u8 *data)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req_etc(hmac_vap, netbuf, is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp_etc(hmac_vap, netbuf, is_protected);
            break;
        default:
            break;
    }
}
#endif

/* 处理不同Category的ACTION帧 */
OAL_STATIC osal_void hmac_ap_up_rx_action_proc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, osal_u8 *data, oal_bool_enum_uint8 is_protected)
{
    /* Category */
    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA:
            hmac_ap_up_rx_action_ba(hmac_vap, hmac_user, data);
            break;
        case MAC_ACTION_CATEGORY_PUBLIC:
            hmac_ap_up_rx_action_public(hmac_vap, netbuf, data);
            break;
        case MAC_ACTION_CATEGORY_HT:
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
                case MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH:
                    break;
#endif
                case MAC_HT_ACTION_BUTT:
                default:
                    break;
            }
            break;
#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY:
            hmac_ap_up_rx_action_sa_query(hmac_vap, netbuf, is_protected, data);
            break;
#endif
        case MAC_ACTION_CATEGORY_VHT:
            if (data[MAC_ACTION_OFFSET_ACTION] == MAC_VHT_ACTION_BUTT) {
                break;
            }
            break;
        case MAC_ACTION_CATEGORY_VENDOR:
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (hmac_ie_check_p2p_action_etc(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
                break;
            }
#endif
            break;

       /* WUR action帧接收处理 */
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        case MAC_ACTION_CATEGORY_WUR:
            hmac_ap_up_rx_action_wur(hmac_vap, hmac_user, data);
            break;
#endif
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_action
 功能描述  : AP在UP状态下的接收ACTION帧处理
 输入参数  : hmac_vap: HMAC VAP指针
             pst_netbuf  : 管理帧所在的sk_buff
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_ap_up_rx_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 is_protected)
{
    dmac_rx_ctl_stru               *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u8                      *data = OAL_PTR_NULL;
    mac_ieee80211_frame_stru       *frame_hdr = OAL_PTR_NULL;          /* 保存mac帧的指针 */
    hmac_user_stru                 *hmac_user = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设备所接受的action全部上报 */
    if (is_p2p_mode(hmac_vap->p2p_mode)) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_ap_up_rx_action:p2p device mode[%d] send to host}",
            hmac_vap->vap_id, hmac_vap->p2p_mode);
        hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
    }
#endif

    /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(&rx_ctrl->rx_info);

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, frame_hdr->address2);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_ap_up_rx_action::hmac_vap_find_user_by_macaddr_etc failed.}",
            hmac_vap->vap_id);
        return;
    }

    /* 获取帧体指针 */
    data = (osal_u8 *)mac_get_rx_cb_mac_header_addr(&rx_ctrl->rx_info) + rx_ctrl->rx_info.mac_header_len;
    /* 处理不同类型的ACTION帧 */
    hmac_ap_up_rx_action_proc(hmac_vap, hmac_user, netbuf, data, is_protected);
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_probe_req
 功能描述  : AP在UP状态下的接收WPS probe req帧处理
 输入参数  : hmac_vap: HMAC VAP指针
             pst_netbuf  : 管理帧所在的sk_buff
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void  hmac_ap_up_rx_probe_req_wps(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru           *pst_rx_ctrl;
    mac_rx_ctl_stru            *rx_info;
    oal_ieee80211_band_enum_uint8         band;
    osal_slong                     l_freq;

    pst_rx_ctrl     = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    rx_info     = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));

    /* 获取AP 当前信道 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        band = OAL_IEEE80211_BAND_2GHZ;
    } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
        band = OAL_IEEE80211_BAND_5GHZ;
    } else {
        band = OAL_IEEE80211_NUM_BANDS;
    }
    l_freq = oal_ieee80211_channel_to_frequency(hmac_vap->channel.chan_number, band);

    /* 上报接收到的probe req 管理帧 */
    hmac_send_mgmt_to_host_etc(hmac_vap, pst_netbuf, rx_info->frame_len, l_freq);
}

OAL_STATIC osal_void hmac_ap_up_rx_mgmt_proc(hmac_vap_stru *hmac_vap, hmac_wlan_crx_event_stru *mgmt_rx_event,
    mac_rx_ctl_stru *rx_info, osal_u8 *mac_hdr)
{
    osal_u8 sub_type = mac_get_frame_sub_type(mac_hdr);

    switch (sub_type) {
        case WLAN_FC0_SUBTYPE_AUTH:
            hmac_ap_rx_auth_req(hmac_vap, mgmt_rx_event->netbuf);
            break;

        case WLAN_FC0_SUBTYPE_DEAUTH:
            hmac_ap_rx_deauth_req(hmac_vap, mac_hdr, (osal_u8)mac_is_protectedframe(mac_hdr));
            break;

        case WLAN_FC0_SUBTYPE_ASSOC_REQ:
        case WLAN_FC0_SUBTYPE_REASSOC_REQ:
            hmac_ap_up_rx_asoc_req(hmac_vap, sub_type, mac_hdr, rx_info->mac_header_len, (osal_u8 *)(mac_hdr) +
                rx_info->mac_header_len, (rx_info->frame_len - rx_info->mac_header_len));
            break;

        case WLAN_FC0_SUBTYPE_DISASSOC:
            hmac_ap_up_rx_disasoc(hmac_vap, mac_hdr, rx_info->mac_header_len,
                (osal_u8 *)(mac_hdr) + rx_info->mac_header_len,
                (rx_info->frame_len - rx_info->mac_header_len), (osal_u8)mac_is_protectedframe(mac_hdr));
            break;

        case WLAN_FC0_SUBTYPE_ACTION:
            hmac_ap_up_rx_action(hmac_vap, mgmt_rx_event->netbuf, (osal_u8)mac_is_protectedframe(mac_hdr));
            break;

        case WLAN_FC0_SUBTYPE_PROBE_REQ:
            hmac_ap_up_rx_probe_req_wps(hmac_vap, mgmt_rx_event->netbuf);
            break;

        default:
            break;
    }
}

OAL_STATIC osal_u32 hmac_ap_up_rx_mgmt(hmac_vap_stru *hmac_vap, hmac_wlan_crx_event_stru *mgmt_rx_event,
    dmac_rx_ctl_stru *rx_ctrl, mac_rx_ctl_stru *rx_info, osal_u8 *mac_hdr)
{
    osal_void *fhook = OSAL_NULL;

#if defined(_PRE_WLAN_WAKEUP_SRC_PARSE)
    if (g_ul_print_wakeup_mgmt_etc == OAL_TRUE) {
        g_ul_print_wakeup_mgmt_etc = OAL_FALSE;
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_ap_up_rx_mgmt::wakeup mgmt type[0x%x]}",
            hmac_vap->vap_id, mac_get_frame_sub_type(mac_hdr));
    }
#endif

    /* Bar frame proc here */
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_CTL) {
        if (mac_get_frame_sub_type(mac_hdr) == WLAN_FC0_SUBTYPE_BAR) {
            hmac_up_rx_bar_etc(hmac_vap, rx_ctrl, mgmt_rx_event->netbuf);
        }
    } else if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_RX_MGMT_BLACKLIST_FILTER);
        if (fhook != OSAL_NULL && ((hmac_ap_up_rx_mgmt_blacklist_filter_cb)fhook)(hmac_vap, mac_hdr) == OAL_SUCC) {
            return OAL_SUCC;
        }
        hmac_ap_up_rx_mgmt_proc(hmac_vap, mgmt_rx_event, rx_info, mac_hdr);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_mgmt_etc
 功能描述  : AP在UP状态下的接收管理帧处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_ap_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_wlan_crx_event_stru   *mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru           *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru            *rx_info = OAL_PTR_NULL;
    osal_u8                  *mac_hdr = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_mgmt_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event   = (hmac_wlan_crx_event_stru *)p_param;
    pst_rx_ctrl         = (dmac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->netbuf);
    rx_info         = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));
    mac_hdr         = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info);

    /* AP在UP状态下 接收到的各种管理帧处理 */
    return hmac_ap_up_rx_mgmt(hmac_vap, mgmt_rx_event, pst_rx_ctrl, rx_info, mac_hdr);
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_timeout_fn
 功能描述  : 管理帧超时处理函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_mgmt_timeout_ap_etc(osal_void *p_param)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    hmac_user = (hmac_user_stru *)p_param;
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_mgmt_timeout_ap_etc::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap  = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_mgmt_timeout_ap_etc::hmac_vap null.}",
            hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_user->ch_text != OSAL_NULL) {
        oal_mem_free(hmac_user->ch_text, OAL_TRUE);
        hmac_user->ch_text = OSAL_NULL;
    }

    oam_warning_log3(0, OAM_SF_AUTH,
        "vap_id[%d] {hmac_mgmt_timeout_ap_etc::Wait AUTH timeout!! After %d ms.send DEAUTH,err code[%d]}",
        hmac_user->vap_id, WLAN_AUTH_TIMEOUT, MAC_AUTH_NOT_VALID);

    /* 发送去关联帧消息给STA */
    hmac_mgmt_send_deauth_frame_etc(hmac_vap, hmac_user->user_mac_addr,
        MAC_AUTH_NOT_VALID, OAL_FALSE);

    /* 抛事件上报内核，已经去关联某个STA */
    hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr,
            AP_ASOC_WAIT_TIMEOUT_ERR, USER_CONN_FAIL);
    }
    ret = hmac_user_del_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {hmac_mgmt_timeout_ap_etc::hmac_user_del_etc failed[%d].}",
            hmac_user->vap_id, ret);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_wait_start_misc_etc
 功能描述  : AP在WAIT START状态下的misc事件处理
 输入参数  : hmac_vap: HMAC VAP指针
             p_param     : hmac_misc_input_stru类型指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32  hmac_ap_wait_start_misc_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_misc_input_stru   *misc_input = OAL_PTR_NULL;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_wait_start_misc_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    misc_input = (hmac_misc_input_stru *)p_param;

    switch (misc_input->type) {
        case HMAC_MISC_RADAR:
#ifdef _PRE_WLAN_FEATURE_DFS
            hmac_dfs_ap_wait_start_radar_handler_etc(hmac_vap);
#endif
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_misc_etc
 功能描述  : AP在UP状态下的misc事件处理
 输入参数  : hmac_vap: HMAC VAP指针
             p_param     : hmac_misc_input_stru类型指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32  hmac_ap_up_misc_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_misc_input_stru   *misc_input = OAL_PTR_NULL;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_up_misc_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    misc_input = (hmac_misc_input_stru *)p_param;

    switch (misc_input->type) {
        case HMAC_MISC_RADAR:
#ifdef _PRE_WLAN_FEATURE_DFS
            hmac_dfs_ap_up_radar_handler_etc(hmac_vap);
#endif
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_ap_clean_bss_etc
 功能描述  : 清除BSS下所有关联用户
 输入参数  : hmac_vap_stru *hmac_vap,
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_ap_clean_bss_etc(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *next_entry = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    oal_bool_enum_uint8 is_protected;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_clean_bss_etc::hmac vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除vap下所有已关联用户，并通知内核 */
    osal_list_for_each_safe(entry, next_entry, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        /* 管理帧加密是否开启 */
        is_protected = hmac_user->cap_info.pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame_etc(hmac_vap, hmac_user->user_mac_addr, MAC_DISAS_LV_SS,
            is_protected);
        oam_warning_log1(0, OAM_SF_DFT, "hmac_ap_clean_bss_etc:send DISASSOC frame, err_code = %d.",
            MAC_DISAS_LV_SS);

        /* 通知内核 */
        hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);

        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr,
                STA_UP_OTHER_VAP_CLEAN_BSS, USER_CONN_OFFLINE);
        }
        /* 删除用户 */
        hmac_user_del_etc(hmac_vap, hmac_user);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
hmac_sae_pmksa_entry_info *hmac_sae_alloc_pmksa_entry(osal_void)
{
    hmac_sae_pmksa_entry_info *pmksa_entry = oal_memalloc(sizeof(hmac_sae_pmksa_entry_info));
    if (pmksa_entry == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_sae_alloc_pmksa_entry::alloc memory failed for storing pmksa.}");
        return OAL_PTR_NULL;
    }

    memset_s(pmksa_entry, sizeof(hmac_sae_pmksa_entry_info), 0, sizeof(hmac_sae_pmksa_entry_info));

    OSAL_INIT_LIST_HEAD(&(pmksa_entry->dlist_head));
    osal_spin_lock_init(&(pmksa_entry->lock));
    return pmksa_entry;
}

hmac_sae_pmksa_entry_info *hmac_sae_pmksa_find_oldest_entry(hmac_sae_pmksa_cache_stru *pmksa_mgmt)
{
    struct osal_list_head *pmksa_list_head = &(pmksa_mgmt->pmksa_list_head);
    struct osal_list_head *entry = OAL_PTR_NULL;
    hmac_sae_pmksa_entry_info *pmksa_entry_tmp = OAL_PTR_NULL;
    osal_u32 present_time = oal_get_seconds();
    osal_u32 runtime;
    osal_u32 oldest_runtime = 0;
    hmac_sae_pmksa_entry_info *oldest_pmksa_entry = OAL_PTR_NULL;

    if (pmksa_mgmt->pmksa_num == 0) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_sae_alloc_pmksa_entry::pmksa list is empty.}");
        return OAL_PTR_NULL;
    }
    for (entry = pmksa_list_head->next; entry != pmksa_list_head;) {
        pmksa_entry_tmp = osal_list_entry(entry, hmac_sae_pmksa_entry_info, dlist_head);
        entry = entry->next;
        if (present_time >= pmksa_entry_tmp->timestamp) {
            runtime = present_time - pmksa_entry_tmp->timestamp;
        } else {
            runtime = 0xFFFFFFFF - (pmksa_entry_tmp->timestamp - present_time);
        }

        if (runtime > oldest_runtime) {
            oldest_runtime = runtime;
            oldest_pmksa_entry = pmksa_entry_tmp;
        }
    }
    if (oldest_pmksa_entry != OAL_PTR_NULL) {
        return oldest_pmksa_entry;
    } else {
        return OAL_PTR_NULL;
    }
}

osal_void hmac_sae_add_pmksa_to_list(hmac_vap_stru *hmac_vap, hmac_sae_pmksa_entry_info *pmksa_entry)
{
    hmac_sae_pmksa_cache_stru *pmksa_mgmt = OAL_PTR_NULL;
    struct osal_list_head *node = OAL_PTR_NULL;
    struct osal_list_head *pmksa_list = OAL_PTR_NULL;
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        return;
    }
    pmksa_mgmt = &(hmac_vap->pmksa_mgmt);
    node = &(pmksa_entry->dlist_head);
    pmksa_list = &(pmksa_mgmt->pmksa_list_head);

    osal_spin_lock(&pmksa_mgmt->lock);
    if (pmksa_mgmt->pmksa_num >= HMAC_SAE_PMKSA_CACHE_NUM) {
        hmac_sae_pmksa_entry_info *pmksa_list_oldest = hmac_sae_pmksa_find_oldest_entry(pmksa_mgmt);
        if (pmksa_list_oldest != OAL_PTR_NULL) {
            osal_list_del(&(pmksa_list_oldest->dlist_head));
            oal_free(pmksa_list_oldest);
            pmksa_mgmt->pmksa_num--;
        } else {
            oal_free(pmksa_entry);
            osal_spin_unlock(&pmksa_mgmt->lock);
            return;
        }
    }
    osal_list_add_tail(node, pmksa_list);

    pmksa_mgmt->pmksa_num++;
    osal_spin_unlock(&pmksa_mgmt->lock);
    if (hmac_vap->pmksa_timer.is_registerd == OAL_FALSE) {
        frw_create_timer_entry(&(hmac_vap->pmksa_timer), hmac_sae_clean_expire_pmksa,
            HMAC_SAE_PMKSA_CHECK_PERIOD, hmac_vap, OAL_TRUE);
    }
}

osal_u32 hmac_sae_clean_expire_pmksa(osal_void *arg)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *entry_tmp = OAL_PTR_NULL;
    hmac_sae_pmksa_entry_info *pmksa_entry_tmp = OAL_PTR_NULL;
    osal_u32 curr_time_stamp;
    osal_u32 runtime;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    if (arg == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sae_clean_expire_pmksa::arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap = (hmac_vap_stru *)arg;
    if (hmac_vap->pmksa_mgmt.pmksa_num == 0) {
        if (hmac_vap->pmksa_timer.is_registerd) {
            frw_destroy_timer_entry(&(hmac_vap->pmksa_timer));
        }
        return OAL_SUCC;
    }

    curr_time_stamp = (osal_u32)oal_get_seconds();
    osal_spin_lock(&hmac_vap->pmksa_mgmt.lock);
    osal_list_for_each_safe(entry, entry_tmp, &(hmac_vap->pmksa_mgmt.pmksa_list_head)) {
        pmksa_entry_tmp = osal_list_entry(entry, hmac_sae_pmksa_entry_info, dlist_head);
        if (curr_time_stamp >= pmksa_entry_tmp->timestamp) {
            runtime = curr_time_stamp - pmksa_entry_tmp->timestamp;
        } else {
            runtime = 0xFFFFFFFF - (pmksa_entry_tmp->timestamp - curr_time_stamp);
        }

        if (runtime > HMAC_SAE_MAX_PMKSA_EXPIRE) {
            oam_warning_log4(0, OAM_SF_WPA, "Mac addr[%x:%x:%x:%x:XX:XX]'s pmkid expire, release pmkid.",
                pmksa_entry_tmp->mac_addr[0], pmksa_entry_tmp->mac_addr[1],
                pmksa_entry_tmp->mac_addr[2], pmksa_entry_tmp->mac_addr[3]); /* 打印第0 1 2 3位mac */
            osal_list_del(&(pmksa_entry_tmp->dlist_head));
            hmac_vap->pmksa_mgmt.pmksa_num--;
            osal_spin_lock_destroy(&pmksa_entry_tmp->lock);
            oal_free(pmksa_entry_tmp);
        }
    }
    osal_spin_unlock(&hmac_vap->pmksa_mgmt.lock);
    return OAL_SUCC;
}

osal_void hmac_sae_clean_all_pmksa(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *entry_tmp = OAL_PTR_NULL;
    hmac_sae_pmksa_entry_info *pmksa_entry_tmp = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sae_clean_all_pmksa::hmac_vap is null.}");
        return;
    }

    if (hmac_vap->pmksa_mgmt.pmksa_num == 0) {
        return;
    }

    osal_spin_lock(&hmac_vap->pmksa_mgmt.lock);
    osal_list_for_each_safe(entry, entry_tmp, &(hmac_vap->pmksa_mgmt.pmksa_list_head)) {
        pmksa_entry_tmp = osal_list_entry(entry, hmac_sae_pmksa_entry_info, dlist_head);
        osal_list_del(&(pmksa_entry_tmp->dlist_head));
        hmac_vap->pmksa_mgmt.pmksa_num--;
        oal_free(pmksa_entry_tmp);
    }
    osal_spin_unlock(&hmac_vap->pmksa_mgmt.lock);
    return;
}

hmac_sae_pmksa_entry_info *hmac_sae_find_pmkid_by_macaddr(hmac_sae_pmksa_cache_stru *pmksa_mgmt, osal_u8 *mac_addr)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *entry_tmp = OAL_PTR_NULL;
    hmac_sae_pmksa_entry_info *pmksa_entry = OAL_PTR_NULL;

    if ((mac_addr == OAL_PTR_NULL) || (pmksa_mgmt == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    osal_list_for_each_safe(entry, entry_tmp, &(pmksa_mgmt->pmksa_list_head)) {
        pmksa_entry = osal_list_entry(entry, hmac_sae_pmksa_entry_info, dlist_head);
        /* 比较mac地址 */
        if (oal_compare_mac_addr(pmksa_entry->mac_addr, mac_addr) == 0) {
            return pmksa_entry;
        }
    }

    return OAL_PTR_NULL;
}

/* owe信息更新，上层与底层数据信息有可能不匹配，函数返回SUCC，防止影响消息返回值 */
osal_s32 hmac_update_owe_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u16 user_id = 0;
    hmac_owe_info *owe_info = (hmac_owe_info *)msg->data;
    osal_u32 ret;

    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, owe_info->peer, &user_id) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_update_owe_info::find_user_by_macaddr fail.}");
        return OAL_SUCC;
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_update_owe_info::hmac_user get fail.}");
        return OAL_SUCC;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_update_owe_info::not ap mode.}");
        return OAL_SUCC;
    }

    hmac_user->owe_ie = owe_info->ie;
    hmac_user->owe_ie_len = owe_info->ie_len;
    oam_warning_log2(0, OAM_SF_DFT, "hmac_update_owe_info:status=%u, owe_ie_len=%u.", owe_info->status,
        hmac_user->owe_ie_len);
    ret = hmac_ap_send_asoc_rsp(hmac_vap, hmac_user, owe_info->status, owe_info->peer);
    /* owe ie使用完即清空 */
    hmac_user->owe_ie = OSAL_NULL;
    hmac_user->owe_ie_len = 0;
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_update_owe_info::hmac_ap_send_asoc_rsp failed[%d].}", hmac_vap->vap_id, ret);
        return OAL_SUCC;
    }

    if (owe_info->status == MAC_SUCCESSFUL_STATUSCODE) {
        /* 上报WAL层(WAL上报内核) AP关联上了一个新的STA */
        oam_warning_log0(0, OAM_SF_DFT, "hmac_update_owe_info:new sta send to ap.");
        hmac_handle_connect_rsp_ap(hmac_vap, hmac_user);
    } else {
        oam_warning_log2(0, OAM_SF_DFT, "hmac_update_owe_info:assoc status code[%d],send DEAUTH,err code[%d]",
            owe_info->status, MAC_AUTH_NOT_VALID);
        hmac_mgmt_send_deauth_frame_etc(hmac_vap,
            hmac_user->user_mac_addr, MAC_AUTH_NOT_VALID, OAL_FALSE);
        hmac_user_del_etc(hmac_vap, hmac_user);
    }

    return OAL_SUCC;
}

#endif

OAL_STATIC osal_void hmac_ap_up_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *mgmt_buf,
    const mac_ieee80211_frame_stru *frame_hdr, hmac_ap_up_stru *ap_up_param, const dmac_rx_ctl_stru *rx_ctl)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u16 user_idx = MAC_INVALID_USER_ID;

#ifdef _PRE_WLAN_FEATURE_1024QAM
    osal_u8 ie_len;
#endif
    unref_param(rx_ctl);

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装probe response帧 */
    ap_up_param->mgmt_len = hmac_mgmt_encap_probe_response(hmac_vap, mgmt_buf, frame_hdr->address2,
        ap_up_param->is_p2p_req);

    /* 增加1024 QAM IE */
#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_set_1024qam_vendor_ie((osal_void *)hmac_vap,
        oal_netbuf_tx_data(mgmt_buf) + ap_up_param->mgmt_len - MAC_80211_FRAME_LEN, &ie_len);
    ap_up_param->mgmt_len += ie_len;
#endif

    if (ap_up_param->netbuf_len < (ap_up_param->frame_len - MAC_80211_FRAME_LEN)) {
        oam_error_log3(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_ap_up_proc::payload_len=[%d] over net_buf_size=[%d].}", hmac_vap->vap_id,
            (ap_up_param->frame_len - MAC_80211_FRAME_LEN), ap_up_param->netbuf_len);
    }

    /* 调用发送管理帧接口 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    (osal_void)hmac_vap_find_user_by_macaddr_etc(hmac_vap, frame_hdr->address2, &user_idx);
    tx_ctl->tx_user_idx = (osal_u8)user_idx; /* 若此时对端用户不存在，则填一个非法值，发送完成会直接释放 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    oal_netbuf_put(mgmt_buf, ap_up_param->mgmt_len);
}

osal_void hmac_ap_up_rx_mgmt_with_vendor_ie(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u8 *go_on)
{
    mac_app_ie_stru *app_ie = OSAL_NULL;
    mac_ieee80211_vendor_ie_stru *ie_req = OSAL_NULL;
    osal_u32 ie_oui;
    osal_u8 *payload = OSAL_NULL;
    osal_u16 ie_len;

    if (osal_unlikely(hmac_vap == OSAL_NULL || netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_mgmt_with_vendor_ie::null pointer}");
        return;
    }

    payload = oal_netbuf_rx_data(netbuf);
    ie_len = (osal_u16)(oal_netbuf_rx_frame_len(netbuf) - oal_netbuf_rx_header_len(netbuf));

    app_ie = &hmac_vap->app_ie[OAL_APP_VENDOR_IE];
    if ((hmac_vap->mgmt_report != OAL_TRUE) ||
        (app_ie->ie == OSAL_NULL) || (app_ie->ie_len < (MAC_IE_HDR_LEN + MAC_OUI_LEN))) {
        return;
    }

    ie_req = (mac_ieee80211_vendor_ie_stru *)app_ie->ie;
    ie_oui = (ie_req->oui[0] << 16) | (ie_req->oui[1] << 8) | ie_req->oui[2]; /* 左移 16、8、2 位 */
    if (hmac_find_vendor_ie_etc(ie_oui, ie_req->oui_type, payload, ie_len) != OSAL_NULL) {
        *go_on = OAL_TRUE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_probe_req
 功能描述  : AP在UP状态下接收到probe req帧处理
*****************************************************************************/
osal_u32 hmac_ap_up_rx_probe_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *addr, osal_char rssi, osal_u8 *go_on)
{
    oal_netbuf_stru *mgmt_buf = OSAL_NULL;
    hmac_ap_up_stru ap_up_param;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *probe_req = OSAL_NULL;
    osal_u16 frame_len;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL || netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_ap_up_rx_probe_req::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取帧头信息 */
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info)));
    probe_req = oal_netbuf_rx_data(netbuf);
    frame_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);

    if (hmac_ap_check_probe_req(hmac_vap, probe_req, frame_hdr) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    hmac_ap_up_rx_mgmt_with_vendor_ie(hmac_vap, netbuf, go_on);

    unref_param(addr);
    unref_param(rssi);

    (osal_void)memset_s(&ap_up_param, sizeof(hmac_ap_up_stru), 0, sizeof(hmac_ap_up_stru));

    /* 收到的probe req 帧包含P2P_IE */
    ap_up_param.is_p2p_req =
        (hmac_find_vendor_ie_etc(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, probe_req, frame_len) != OSAL_NULL) ?
        OSAL_TRUE : OSAL_FALSE;
    if ((ap_up_param.is_p2p_req == OSAL_TRUE) &&
        (hmac_vap->app_ie[OAL_APP_PROBE_RSP_IE].ie_len > OAL_MGMT_NETBUF_APP_PROBE_RSP_IE_LEN_LIMIT)) {
        /* p2p IE 长度过大时，申请大包 */
        oam_warning_log2(0, OAM_SF_SCAN, "vap[%d] {hmac_ap_up_rx_probe_req:alloc netbuf with LARGE_SIZE, ie len=[%d].}",
            hmac_vap->vap_id, hmac_vap->app_ie[OAL_APP_PROBE_RSP_IE].ie_len);
        mgmt_buf = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
        ap_up_param.netbuf_len = WLAN_LARGE_NETBUF_SIZE;
    } else {
        mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
        ap_up_param.netbuf_len = WLAN_MGMT_NETBUF_SIZE;
    }
    if (mgmt_buf == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap[%d] {hmac_ap_up_rx_probe_req::alloc netbuff failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ap_up_param.frame_len = frame_len;
    hmac_ap_up_proc(hmac_vap, mgmt_buf, frame_hdr, &ap_up_param, rx_ctl);

    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, ap_up_param.mgmt_len, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_RX, "vap[%d] hmac_ap_up_rx_probe_req:hmac_tx_mgmt fail[%d]", hmac_vap->vap_id, ret);
        oal_netbuf_free(mgmt_buf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_probe_response
 功能描述  : 封装probe response帧
 输入参数  : hmac_vap: HMAC vap
             buffer  : probe response帧内存
             ra      : 接收地址
*****************************************************************************/
OAL_STATIC osal_u16  hmac_mgmt_encap_probe_response(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *ra, oal_bool_enum_uint8 is_p2p_req)
{
    osal_u8        ie_len = 0;
    osal_u8       *mac_header = oal_netbuf_header(netbuf);
    osal_u8       *payload_addr        = oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);
    osal_u8       *payload_addr_origin = payload_addr;
    osal_u16       app_ie_len;
    osal_u8       dsss_channel_num;
    mac_quiet_info quiet_info;

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
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_PROBE_RSP);

    /* 设置地址1为发送probe request帧的STA */
    if (ra != OSAL_NULL) {
        oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, ra);
    }
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));
    if (hmac_vap_is_in_p2p_listen() == OSAL_TRUE) {
        /* 设置地址3为bssid */
        oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, mac_mib_get_station_id(hmac_vap));
    } else {
        /* 设置地址3为bssid */
        oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    }

    /* 设置分片序号, 管理帧为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /*************************************************************************/
    /*                  Probe Response Frame - Frame Body                    */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet      |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3               |8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                              */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* Initialize index */
    payload_addr += MAC_TIME_STAMP_LEN;

    /* 设置beacon interval */
    hmac_set_beacon_interval_field_etc(hmac_vap, payload_addr);
    payload_addr += MAC_BEACON_INTERVAL_LEN;

    /* 设置capability information */
    hmac_set_cap_info_field(hmac_vap, payload_addr);
    payload_addr += MAC_CAP_INFO_LEN;

    /* 设置ssid element */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_set_p2p0_ssid_ie(hmac_vap, payload_addr, &ie_len, WLAN_FC0_SUBTYPE_PROBE_RSP);
        payload_addr += ie_len;
    } else {
        hmac_set_ssid_ie_etc(hmac_vap, payload_addr, &ie_len, WLAN_FC0_SUBTYPE_PROBE_RSP);
        payload_addr += ie_len;
    }
#else
    hmac_set_ssid_ie_etc(hmac_vap, payload_addr, &ie_len, WLAN_FC0_SUBTYPE_PROBE_RSP);
    payload_addr += ie_len;
#endif

    /* 设置支持的速率集 */
    hmac_set_supported_rates_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 获取dsss ie内的channel num */
    dsss_channel_num = hmac_get_dsss_ie_channel_num(hmac_vap, 0); /* 设置dsss参数集 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE) {
        hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
        dsss_channel_num = hmac_device->p2p_listen_channel;
    }
#endif
    mac_set_dsss_params_etc(hmac_vap, payload_addr, &ie_len, dsss_channel_num);
    payload_addr += ie_len;

#ifdef _PRE_WLAN_FEATURE_11D
    /* 填充country信息 */
    mac_set_country_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
#endif

    /* 填充power constraint信息 */
    mac_set_pwrconstraint_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充quiet信息 */
    quiet_info.count = MAC_QUIET_COUNT;
    quiet_info.duration = MAC_QUIET_DURATION;
    quiet_info.period = MAC_QUIET_PERIOD;
    quiet_info.offset = MAC_QUIET_OFFSET;
    mac_set_quiet_ie_etc(hmac_vap, payload_addr, &quiet_info, &ie_len);
    payload_addr += ie_len;

    /* 填充TPC Report信息 */
    mac_set_tpc_report_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充erp信息 */
    mac_set_erp_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充RSN 安全相关信息 */
    hmac_set_rsn_ie_etc(hmac_vap, OSAL_NULL, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充WPA 安全相关信息 */
    hmac_set_wpa_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充wmm信息 */
    hmac_set_wmm_params_ie_etc(hmac_vap, payload_addr, mac_mib_get_dot11_qos_option_implemented(hmac_vap), &ie_len);
    payload_addr += ie_len;

    /* 填充extended supported rates信息 */
    hmac_set_exsup_rates_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充bss load信息 */
    mac_set_bssload_ie(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充HT Capabilities信息 */
    hmac_set_ht_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充HT Operation信息 */
    hmac_set_ht_opern_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充Overlapping BSS Scan信息 */
    hmac_set_obss_scan_params_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充Extended Capabilities信息 */
    hmac_set_ext_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充vht cap信息 */
    hmac_set_vht_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充vht opern信息 */
    hmac_set_vht_opern_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_set_he_capabilities_ie(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
    hmac_set_he_operation_ie(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    hmac_set_opmode_notify_ie_etc((osal_void *)hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
#endif

    if (is_p2p_req == OSAL_TRUE) {
        /* 填充WPS P2P信息 */
        hmac_add_app_ie_etc((osal_void *)hmac_vap, payload_addr, &app_ie_len, OAL_APP_PROBE_RSP_IE);
    } else {
        /* 对于非p2p 设备发起的扫描，回复probe resp 帧不能携带p2p ie 信息，回复WPS 信息 */
        mac_add_wps_ie_etc((osal_void *)hmac_vap, payload_addr, &app_ie_len, OAL_APP_PROBE_RSP_IE);
    }
    payload_addr += app_ie_len;

    /* 填充扩展IE信息 */
    mac_add_extend_app_ie_etc(hmac_vap, &payload_addr, PROBE_RSP_IE);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 填充p2p noa Attribute */
    if ((is_p2p_req == OSAL_TRUE) && (is_p2p_go(hmac_vap)) && (is_p2p_noa_enabled(hmac_vap) ||
        is_p2p_oppps_enabled(hmac_vap))) {
        mac_set_p2p_noa(hmac_vap, payload_addr, &ie_len);
        payload_addr += ie_len;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_set_rsnx_ie(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
#endif

    return (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);
}

/*****************************************************************************
 函 数 名  : hmac_ap_check_ssid
 功能描述  : 检查ssid正确性
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_check_ssid(const hmac_vap_stru *hmac_vap,
    const osal_u8 *probe_req_ssid, const osal_u8 *sta_mac)
{
    osal_u8           prb_req_ssid_len;
    osal_u8          *ssid = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    const osal_u8          *prb_req_ssid = OSAL_NULL;
#endif

    if (osal_unlikely(hmac_vap == OSAL_NULL || probe_req_ssid == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_ap_check_ssid::param null.}");
        return OAL_FAIL;
    }

    unref_param(sta_mac);

    prb_req_ssid_len = probe_req_ssid[1];
#ifdef _PRE_WLAN_FEATURE_P2P
    prb_req_ssid    = &probe_req_ssid[MAC_IE_HDR_LEN];
    /* P2P device 检查ssid 是否为"DIRECT-"，P2P GO 需要和AP 一样，检查probe req ssid 内容 */
    if (is_p2p_cl(hmac_vap) || is_p2p_dev(hmac_vap)) {
        if (is_p2p_wildcard_ssid(prb_req_ssid, prb_req_ssid_len)) {
            return OAL_SUCC;
        }

        return OAL_FAIL;
    }

    /* GO 接收到probe req 携带ssid 内容为DIRECT- ， 需要回复probe rsp 帧 */
    if (is_p2p_go(hmac_vap) && is_p2p_wildcard_ssid(prb_req_ssid, prb_req_ssid_len)) {
        return OAL_SUCC;
    }
#endif

    ssid = mac_mib_get_desired_ssid(hmac_vap);

    if (prb_req_ssid_len == 0) {
        /* 隐藏ssid配置下，不接受ssid len为0的情况 */
        if (hmac_vap->cap_flag.hide_ssid == OSAL_TRUE) {
            return OAL_FAIL;
        }
        return OAL_SUCC;
    }

    if (prb_req_ssid_len != (osal_u8)strlen((osal_char*)ssid)) {
        return OAL_FAIL;
    }

    return ((oal_memcmp(&probe_req_ssid[MAC_IE_HDR_LEN], ssid, prb_req_ssid_len) == 0) ? OAL_SUCC : OAL_FAIL);
}

/*****************************************************************************
 函 数 名  : hmac_ap_check_probe_req
 功能描述  : 检测probe request帧，判断是否要发probe response帧
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8  hmac_ap_check_probe_req(hmac_vap_stru *hmac_vap,
    osal_u8 *probe_req, mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u8 *bssid;

    if (osal_unlikely(hmac_vap == OSAL_NULL || frame_hdr == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_ap_check_probe_req::param null.}");
        return OSAL_FALSE;
    }

    bssid = frame_hdr->address3;
    if (hmac_ap_check_ssid(hmac_vap, probe_req, frame_hdr->address2) != OAL_SUCC) {
        return OSAL_FALSE;
    }

    /* 检查probe request帧中的bssid是不是本AP的bssid或者广播地址 */
    if ((oal_compare_mac_addr(BROADCAST_MACADDR, bssid) != 0) &&
        (oal_compare_mac_addr(hmac_vap->bssid, bssid) != 0)) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
