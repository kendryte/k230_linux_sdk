/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_alg.c
 * 生成日期   : 2015年3月18日
 * 功能描述   : 漫游模块算法实现
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_roam_alg.h"
#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_roam_main.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blacklist.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_feature_interface.h"
#include "hmac_11v.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ROAM_ALG_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : roam alg初始化
*****************************************************************************/
osal_void hmac_roam_alg_init_etc(hmac_roam_info_stru *roam_info, osal_s8 c_current_rssi)
{
    hmac_roam_alg_stru *roam_alg = OAL_PTR_NULL;

    if (roam_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_init_etc::param null.}");
        return;
    }
    oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_init_etc c_current_rssi = %d.}", c_current_rssi);

    roam_alg = &(roam_info->alg);
    if (c_current_rssi == ROAM_RSSI_LINKLOSS_TYPE) {
        roam_info->st_static.trigger_linkloss_cnt++;
    } else {
        roam_info->st_static.trigger_rssi_cnt++;
    }

    roam_alg->max_capacity       = 0;
    roam_alg->max_capacity_bss  = OAL_PTR_NULL;
    roam_alg->c_current_rssi        = c_current_rssi;
    roam_alg->c_max_rssi            = 0;
    roam_alg->another_bss_scaned = 0;
    /* 首次关联时初始化 roam_alg->invalid_scan_cnt   = 0x0; */
    roam_alg->max_rssi_bss      = OAL_PTR_NULL;

    return;
}

/*****************************************************************************
 功能描述  : 调整rssi增益
*****************************************************************************/
OAL_STATIC osal_s8 hmac_roam_alg_adjust_rssi_increase(hmac_roam_info_stru *roam_info,
    mac_bss_dscr_stru *bss_dscr)
{
    osal_s8  c_current_rssi;
    osal_s8  c_target_rssi;
    osal_u8 delta_rssi;
    osal_u8 flag = OSAL_FALSE;

    if (bss_dscr->st_channel.band == WLAN_BAND_5G) {
        delta_rssi = roam_info->config.delta_rssi_5g;
    } else {
        delta_rssi = roam_info->config.delta_rssi_2g;
    }

    c_current_rssi = roam_info->alg.c_current_rssi;
    c_target_rssi  = bss_dscr->c_rssi;

    if (delta_rssi < ROAM_RSSI_DIFF_4_DB) {
        delta_rssi = ROAM_RSSI_DIFF_4_DB;
    }

    if (c_current_rssi >= ROAM_RSSI_NE70_DB) {
        flag = OSAL_TRUE;
        goto out;
    }

    if (delta_rssi >= ROAM_RSSI_DIFF_4_DB + 2) { /* 2DB */
        /* 步进2DB至4DB */
        delta_rssi -= 2;
    }

    if (c_current_rssi >= ROAM_RSSI_NE75_DB) {
        flag = OSAL_TRUE;
        goto out;
    }

    if (delta_rssi >= ROAM_RSSI_DIFF_4_DB + 2) { /* 2DB */
        /* 步进2DB至4DB */
        delta_rssi -= 2;
    }

    if (c_current_rssi >= ROAM_RSSI_NE80_DB) {
        flag = OSAL_TRUE;
        goto out;
    }

out:
    oam_warning_log3(0, OAM_SF_ROAM,
        "{hmac_roam_alg_adjust_rssi_increase::target_rssi[%d], current_rssi[%d], delta_rssi[%d].}",
        c_target_rssi, c_current_rssi, (osal_s8)delta_rssi);
    return flag == OSAL_TRUE ? (c_target_rssi - c_current_rssi - (osal_s8)delta_rssi) :
        (c_target_rssi - c_current_rssi - ROAM_RSSI_DIFF_4_DB);
}

/*****************************************************************************
 功能描述  : 将bssid添加到列表中
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_alg_add_bsslist(hmac_roam_bss_list_stru *roam_bss_list, osal_u8 *bssid,
    roam_blacklist_type_enum_uint8 list_type)
{
    hmac_roam_bss_info_stru *cur_bss = OAL_PTR_NULL;
    osal_u8                mac_zero[WLAN_MAC_ADDR_LEN] = {0};
    osal_u32               timeout, current_index;

    hmac_roam_bss_info_stru *oldest_bss = OAL_PTR_NULL;
    hmac_roam_bss_info_stru *zero_bss   = OAL_PTR_NULL;
    osal_u32 now = (osal_u32)oal_time_get_stamp_ms();

    unref_param(list_type);

    for (current_index = 0; current_index < ROAM_LIST_MAX; current_index++) {
        cur_bss = &roam_bss_list->bss[current_index];
        timeout = (osal_u32)cur_bss->timeout;
        if (oal_compare_mac_addr(cur_bss->bssid, bssid) == 0) {
            /* 优先查找已存在的记录，如果名单超时更新时间戳，否则更新count */
            if (osal_get_runtime(cur_bss->time_stamp, now) > timeout) {
                cur_bss->time_stamp = now;
                cur_bss->count      = 1;
                return OAL_SUCC;
            }
            cur_bss->count++;
            if (cur_bss->count == cur_bss->count_limit) {
                cur_bss->time_stamp = now;
            }
            return OAL_SUCC;
        }

        /* 记录第一个空记录 */
        if (zero_bss != OAL_PTR_NULL) {
            continue;
        }

        if (oal_compare_mac_addr(cur_bss->bssid, (const osal_u8 *)mac_zero) == 0) {
            zero_bss = cur_bss;
            continue;
        }

        /* 记录一个非空最老记录 */
        if (oldest_bss == OAL_PTR_NULL) {
            oldest_bss = cur_bss;
        } else if (osal_get_runtime(cur_bss->time_stamp, now) >
            osal_get_runtime(oldest_bss->time_stamp, now)) {
            oldest_bss = cur_bss;
        }
    }

    if (zero_bss == OAL_PTR_NULL) {
        zero_bss = oldest_bss;
    }

    if (zero_bss != OAL_PTR_NULL) {
        oal_set_mac_addr(zero_bss->bssid, bssid);
        zero_bss->time_stamp = now;
        zero_bss->count      = 1;
        return OAL_SUCC;
    }
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 检查列表中是否存在该bssid
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_roam_alg_find_in_bsslist(hmac_roam_bss_list_stru *roam_bss_list,
    osal_u8 *bssid)
{
    hmac_roam_bss_info_stru    *cur_bss = OAL_PTR_NULL;
    osal_u32                  current_index;
    osal_u32                  now;
    osal_u32                  timeout;
    osal_u32                  delta_time;
    osal_u16                  count_limit;

    now = (osal_u32)oal_time_get_stamp_ms();

    for (current_index = 0; current_index < ROAM_LIST_MAX; current_index++) {
        cur_bss = &roam_bss_list->bss[current_index];
        timeout     = cur_bss->timeout;
        count_limit =  cur_bss->count_limit;

        if (oal_compare_mac_addr(cur_bss->bssid, bssid) == 0) {
            /* 如果在超时时间内出现count_limit次以上记录 */
            delta_time = osal_get_runtime(cur_bss->time_stamp, now);
            if ((delta_time <= timeout) && (cur_bss->count >= count_limit)) {
                return OAL_TRUE;
            }
            return OAL_FALSE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 将bssid添加到黑名单列表中
*****************************************************************************/
osal_u32 hmac_roam_alg_add_blacklist_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid,
    roam_blacklist_type_enum_uint8 list_type)
{
    osal_u32 ul_ret;
    if ((roam_info == OAL_PTR_NULL) || (bssid == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_roam_alg_add_bsslist(&roam_info->alg.blacklist, bssid, list_type);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_add_blacklist_etc::hmac_roam_alg_add_list failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 检查黑名单列表中是否存在该bssid
*****************************************************************************/
oal_bool_enum_uint8 hmac_roam_alg_find_in_blacklist_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid)
{
    if ((roam_info == OAL_PTR_NULL) || (bssid == OAL_PTR_NULL)) {
        return OAL_FALSE;
    }

    return hmac_roam_alg_find_in_bsslist(&roam_info->alg.blacklist, bssid);
}

/*****************************************************************************
 功能描述  : 将bssid添加到历史优选AP列表中
*****************************************************************************/
osal_u32 hmac_roam_alg_add_history_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid)
{
    osal_u32 ul_ret;

    if ((roam_info == OAL_PTR_NULL) || (bssid == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_roam_alg_add_bsslist(&roam_info->alg.history, bssid, ROAM_BLACKLIST_TYPE_NORMAL_AP);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_add_history_etc::hmac_roam_alg_add_list failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_roam_alg_fill_channel_list(mac_scan_req_stru *scan_params, osal_u8 index,
    osal_u8 chan_number, osal_u8 band, osal_u8 chan_idx)
{
    scan_params->channel_list[index].chan_number = chan_number;
    scan_params->channel_list[index].band        = band;
    scan_params->channel_list[index].chan_idx    = chan_idx;
}

OAL_STATIC osal_void hmac_roam_alg_scan_orthogonal(const hmac_roam_info_stru *roam_info,
    mac_scan_req_stru *scan_params)
{
    osal_u32 ret;
    osal_u8 chan_idx;
    osal_u8 chan_number;

    switch (roam_info->config.scan_orthogonal) {
        case ROAM_SCAN_CHANNEL_ORG_3:
            hmac_roam_alg_fill_channel_list(scan_params, 0, 1, WLAN_BAND_2G, 0);

            hmac_roam_alg_fill_channel_list(scan_params, 1, 6, WLAN_BAND_2G, 5); /* 6信道  5信道索引号 */

            hmac_roam_alg_fill_channel_list(scan_params, 2, 11, WLAN_BAND_2G, 10); /* 元素2 11信道 10信道索引号 */

            scan_params->channel_nums = 3; /* 扫描信道个数为3 */
            break;
        case ROAM_SCAN_CHANNEL_ORG_4:
            hmac_roam_alg_fill_channel_list(scan_params, 0, 1, WLAN_BAND_2G, 0);

            hmac_roam_alg_fill_channel_list(scan_params, 1, 5, WLAN_BAND_2G, 4); /* 5信道 4信道索引号 */

            hmac_roam_alg_fill_channel_list(scan_params, 2, 7, WLAN_BAND_2G, 6); /* 数组元素2，7信道 6信道索引号 */

            hmac_roam_alg_fill_channel_list(scan_params, 3, 13, WLAN_BAND_2G, 12); /* 元素3，13信道 12信道索引号 */

            scan_params->channel_nums = 4; /* 扫描信道个数为4 */
            break;
        default:
            for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
                ret = hmac_is_channel_idx_valid_etc(WLAN_BAND_2G, chan_idx);
                if (ret != OAL_SUCC) {
                    continue;
                }
                hmac_get_channel_num_from_idx_etc(WLAN_BAND_2G, chan_idx, &chan_number);
                hmac_roam_alg_fill_channel_list(scan_params, scan_params->channel_nums, chan_number,
                    WLAN_BAND_2G, chan_idx);
                scan_params->channel_nums++;
            }
            break;
    }
}

osal_u32 hmac_roam_alg_scan_channel_init_etc(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params)
{
    hmac_vap_stru      *hmac_vap = OAL_PTR_NULL;
    mac_channel_stru   *channel  = OAL_PTR_NULL;
    osal_u32          ul_ret;
    osal_u8           chan_idx, chan_number;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_SET_SCAN_PARAMS);

    if ((roam_info == OAL_PTR_NULL) || (scan_params == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_scan_channel_init_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap  = roam_info->hmac_vap;

    channel = &(hmac_vap->channel);
    scan_params->channel_nums  = 0;

    if (roam_info->config.scan_orthogonal == ROAM_SCAN_CHANNEL_ORG_1) {
        oam_info_log3(0, OAM_SF_ROAM, "{scan one channel: ChanNum[%d], Band[%d], ChanIdx[%d]}",
            channel->chan_number, channel->band, channel->chan_idx);

        if (roam_info->roam_trigger != ROAM_TRIGGER_11V) {
            hmac_roam_alg_fill_channel_list(scan_params, 0, channel->chan_number, channel->band, channel->chan_idx);
            scan_params->channel_nums = 1;
            return OAL_SUCC;
        }

        if (fhook != OSAL_NULL) {
            ((hmac_11v_set_scan_params_cb)fhook)(scan_params, roam_info);
        }
        return OAL_SUCC;
    }

    if ((roam_info->config.scan_band & ROAM_BAND_2G_BIT) != 0) {
        hmac_roam_alg_scan_orthogonal(roam_info, scan_params);
    }
    if ((roam_info->config.scan_band & ROAM_BAND_5G_BIT) != 0) {
        for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
            ul_ret = hmac_is_channel_idx_valid_etc(WLAN_BAND_5G, chan_idx);
            if (ul_ret != OAL_SUCC) {
                continue;
            }
            hmac_get_channel_num_from_idx_etc(WLAN_BAND_5G, chan_idx, &chan_number);
            hmac_roam_alg_fill_channel_list(scan_params, scan_params->channel_nums, chan_number,
                WLAN_BAND_5G, chan_idx);
            scan_params->channel_nums++;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_alg_get_capacity_by_rssi(wlan_protocol_enum_uint8 protocol,
    wlan_bw_cap_enum_uint8 bw_cap, osal_s8 c_rssi)
{
    hmac_roam_rssi_capacity_stru *rssi_table = OAL_PTR_NULL;
    osal_u8 index;
    hmac_roam_rssi_capacity_stru rssi_tab_11b[ROAM_RSSI_LEVEL] = { {-88, 0, 81}, {-92, 0, 49}, {-94, 0, 9} };
    hmac_roam_rssi_capacity_stru rssi_tab_11g_ofdm[ROAM_RSSI_LEVEL] = { {-75, 0, 293}, {-82, 0, 173}, {-90, 0, 54} };
    hmac_roam_rssi_capacity_stru rssi_tab_11a_ofdm[ROAM_RSSI_LEVEL] = { {-75, 0, 293}, {-82, 0, 173}, {-90, 0, 54} };
    hmac_roam_rssi_capacity_stru rssi_tab_ht20_ofdm[ROAM_RSSI_LEVEL] = { {-72, 0, 588}, {-80, 0, 353}, {-90, 0, 58} };
    hmac_roam_rssi_capacity_stru rssi_tab_ht40_ofdm[ROAM_RSSI_LEVEL] = { {-75, 0, 1281}, {-77, 0, 705},
        {-87, 0, 116} };
    hmac_roam_rssi_capacity_stru rssi_tab_vht80_ofdm[ROAM_RSSI_LEVEL] = { {-72, 0, 2562}, {-74, 0, 1410},
        {-84, 0, 232} };

    switch (protocol) {
        case WLAN_LEGACY_11A_MODE:
            rssi_table = rssi_tab_11a_ofdm;
            break;

        case WLAN_LEGACY_11B_MODE:
            rssi_table = rssi_tab_11b;
            break;

        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            rssi_table = rssi_tab_11g_ofdm;
            break;

        case WLAN_HT_MODE:
        case WLAN_VHT_MODE:
            if (bw_cap == WLAN_BW_CAP_20M) {
                rssi_table = rssi_tab_ht20_ofdm;
            } else if (bw_cap == WLAN_BW_CAP_40M) {
                rssi_table = rssi_tab_ht40_ofdm;
            } else {
                rssi_table = rssi_tab_vht80_ofdm;
            }
            break;

        default:
            break;
    }

    if (rssi_table == OAL_PTR_NULL) {
        return 0;
    }

    for (index = 0; index < ROAM_RSSI_LEVEL; index++) {
        if (rssi_table[index].c_rssi <= c_rssi) {
            /* capacity_kbps的单位是100Kb，此处乘以100，单位换算为kb */
            return ((osal_u32)rssi_table[index].capacity_kbps * 100);
        }
    }

    return 0;
}

OAL_STATIC osal_u32 hmac_roam_alg_calc_avail_channel_capacity(mac_bss_dscr_stru *bss_dscr)
{
    osal_u32                 capacity;
    osal_u32                 avail_channel_cap;
    osal_u32                 ul_ret;
    osal_u8                  channel_utilization;
    osal_u8                 *obss_ie = OAL_PTR_NULL;
    osal_u8                  ie_offset;
    wlan_protocol_enum_uint8   protocol;

    ul_ret = hmac_sta_get_user_protocol_etc(bss_dscr, &protocol);
    if (ul_ret != OAL_SUCC) {
        return 0;
    }
    /***************************************************************************
        ------------------------------------------------------------------------
        |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
        ------------------------------------------------------------------------
        |1   |1   |2            |1                  |2                         |
        ------------------------------------------------------------------------
        ***************************************************************************/
    ie_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    obss_ie = mac_find_ie_etc(MAC_EID_QBSS_LOAD, (osal_u8 *)(bss_dscr->mgmt_buff + ie_offset),
        (osal_s32)(bss_dscr->mgmt_len - ie_offset));
    /* 长度要到达ChannelUtilization这个域，至少为3 */
    if (obss_ie && (obss_ie[1] >= 3)) {
        channel_utilization = *(obss_ie + 4); /* +4:得到channel_utilization的初始地址 */
    } else {
        return 0;
    }

    capacity = hmac_roam_alg_get_capacity_by_rssi(protocol, bss_dscr->bw_cap, bss_dscr->c_rssi);

    /* 文档规定信道利用率计算方法:normalized to 255 */
    avail_channel_cap = capacity * (255 - channel_utilization) / 255 / ROAM_CONCURRENT_USER_NUMBER;

    return avail_channel_cap;
}

/*****************************************************************************
 功能描述 : 触发强信号漫游前,校验pmf能力位
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_alg_bss_pmf_capa_check(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
#ifdef _PRE_WLAN_FEATURE_PMF
    osal_u16 rsn_cap_info = 0;
    osal_bool pmf_cap;
    osal_bool pmf_require;

    if (bss_dscr->rsn_ie != OSAL_NULL) {
        /* 获取目的AP的RSN CAP信息 */
        rsn_cap_info = hmac_get_rsn_capability_etc(bss_dscr->rsn_ie);
    }

    /* 漫游时使用上次pmf能力位 并判断与漫游的目的ap能力位是否匹配 */
    pmf_cap = mac_mib_get_dot11_rsnamfpc(hmac_vap);
    pmf_require = mac_mib_get_dot11_rsnamfpr(hmac_vap);
    if ((pmf_require == OSAL_TRUE) && ((rsn_cap_info & BIT7) == 0)) {      /* 本地强制，对端没有MFP能力 */
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_alg_bss_pmf_capa_check:: vap require pmf but ap don't have}");
        return OAL_FAIL;
    }
    if ((pmf_cap == OSAL_FALSE) && ((rsn_cap_info & BIT6) != 0)) {    /* 对端强制，本地没有MFP能力 */
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_alg_bss_pmf_capa_check:: vap no pmf cap and ap required}");
        return OAL_FAIL;
    }
#endif

    return OAL_SUCC;
}

osal_u32 hmac_roam_alg_bss_in_ess_etc(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru              *hmac_vap = OAL_PTR_NULL;
    hmac_roam_alg_stru         *roam_alg = OAL_PTR_NULL;
    mac_cfg_ssid_param_stru     cfg_ssid;
    osal_u8                     stru_len;
    osal_u32                    scan_expire_time, curr_time;

    if ((roam_info == OAL_PTR_NULL) || (bss_dscr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->hmac_vap;
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    hmac_mib_get_ssid_etc(hmac_vap, &stru_len, (osal_u8 *)(&cfg_ssid));

    if ((osal_strlen(bss_dscr->ac_ssid) != cfg_ssid.ssid_len) ||
        (osal_memcmp(cfg_ssid.ac_ssid, bss_dscr->ac_ssid, cfg_ssid.ssid_len) != 0)) {
        return OAL_SUCC;
    }

    /* 挑选 scan_expire_time 时间以内的有效bss */
    scan_expire_time = (osal_u32)mac_is_dfs_channel(WLAN_BAND_5G, bss_dscr->st_channel.chan_number) == OAL_TRUE ?
        HMAC_SCAN_MAX_VALID_SCANNED_DFS_EXPIRE : HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE;
    curr_time = (osal_u32)oal_time_get_stamp_ms();
    if (oal_time_after((osal_ulong)curr_time, (osal_ulong)(bss_dscr->timestamp + scan_expire_time)) != 0) {
        oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess_etc:: bss[%02x:%02x:%02x:%02x:XX:XX]}",
            bss_dscr->bssid[0], bss_dscr->bssid[1], bss_dscr->bssid[2], bss_dscr->bssid[3]); /* mac地址0/1/2/3 */
        oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_in_ess_etc::expire scan time %d ms, diff_time=%d}",
            scan_expire_time, curr_time - bss_dscr->timestamp);
        return OAL_SUCC;
    }

    /* pmf能力位校验不通过的,不漫游 */
    if (hmac_roam_alg_bss_pmf_capa_check(hmac_vap, bss_dscr) != OAL_SUCC) {
        return OAL_SUCC;
    }

    roam_alg = &(roam_info->alg);
    /* 是否扫描到了当前关联的 bss, 仅置位，不过滤 */
    if (oal_compare_mac_addr(hmac_vap->bssid, bss_dscr->bssid) != 0) {
        roam_alg->another_bss_scaned = 1;

        /* Roaming Scenario Recognition
         * dense AP standard: RSSI>=-60dB, candidate num>=5;
         *                    RSSI<-60dB && RSSI >=-75dB, candidate num>=10;
         */
        roam_alg->candidate_bss_num++;
        if (bss_dscr->c_rssi >= roam_info->config.c_candidate_good_rssi) {
            roam_alg->candidate_good_rssi_num++;
        } else if ((bss_dscr->c_rssi < roam_info->config.c_candidate_good_rssi) &&
            bss_dscr->c_rssi >= ROAM_RSSI_NE75_DB) {
            roam_alg->candidate_weak_rssi_num++;
        }

        if (bss_dscr->c_rssi > roam_alg->c_max_rssi) {
            roam_alg->c_max_rssi = bss_dscr->c_rssi;
        }
    } else {
        roam_alg->c_current_rssi = bss_dscr->c_rssi;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 校验加密相关 和 rssi
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_alg_bss_check_privacy_and_rssi(hmac_roam_info_stru *roam_info,
    mac_bss_dscr_stru *bss_dscr, hmac_vap_stru *hmac_vap, hmac_roam_alg_stru *roam_alg)
{
    osal_u32 avail_channel_cap;
    osal_s8 tmp_rssi;
    osal_u8 *pmkid = OAL_PTR_NULL;
    osal_s8 delta_rssi;
    mac_cap_info_stru *cap_info = OAL_PTR_NULL;

    if (roam_info->current_bss_ignore == OAL_FALSE) {
        /* 排除当前bss的rssi值计算，本地已经保存了dmac上报的rssi */
        if (oal_compare_mac_addr(hmac_vap->bssid, bss_dscr->bssid) == 0) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    /* wep的bss直接过滤掉 */
    cap_info = (mac_cap_info_stru *)&bss_dscr->us_cap_info;
    if ((bss_dscr->rsn_ie == OAL_PTR_NULL) && (bss_dscr->wpa_ie == OAL_PTR_NULL) &&
        (cap_info->privacy != 0)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /*  open加密方式到wpa/wpa2直接过滤掉 */
    if ((cap_info->privacy == 0) != (mac_mib_get_privacyinvoked(hmac_vap) != OAL_TRUE)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* c_current_rssi为0时，表示linkloss上报的触发，不需要考虑rssi增益 */
    delta_rssi = hmac_roam_alg_adjust_rssi_increase(roam_info, bss_dscr);
    if (delta_rssi <= 0) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    avail_channel_cap = hmac_roam_alg_calc_avail_channel_capacity(bss_dscr);
    if ((avail_channel_cap != 0) && ((roam_alg->max_capacity_bss == OAL_PTR_NULL) ||
        (avail_channel_cap > roam_alg->max_capacity))) {
        /* 暂时不考虑容量 */
    }

    tmp_rssi = bss_dscr->c_rssi;
    /* 对于已存在pmk缓存的bss进行加分处理 */
    pmkid = hmac_vap_get_pmksa_etc(hmac_vap, bss_dscr->bssid);
    if (pmkid != OAL_PTR_NULL) {
        tmp_rssi += ROAM_RSSI_DIFF_4_DB;
    }

    if (bss_dscr->st_channel.band == WLAN_BAND_5G) {
        tmp_rssi += ROAM_RSSI_DIFF_4_DB; /* 5G AP have 4dB weight for higher priority */
    }

    if ((roam_alg->max_rssi_bss == OAL_PTR_NULL) || (tmp_rssi > roam_alg->c_max_rssi)) {
        roam_alg->c_max_rssi = tmp_rssi;
        roam_alg->max_rssi_bss = bss_dscr;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 检查
*****************************************************************************/
osal_u32 hmac_roam_alg_bss_check_etc(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_roam_alg_stru *roam_alg = OAL_PTR_NULL;
    mac_cfg_ssid_param_stru cfg_ssid;
    osal_u32 ret;
    osal_u8 stru_len;
    osal_void *fhook;

    if ((roam_info == OAL_PTR_NULL) || (bss_dscr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_check_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = roam_info->hmac_vap;
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    hmac_mib_get_ssid_etc(hmac_vap, &stru_len, (osal_u8 *)(&cfg_ssid));

    if ((osal_strlen(bss_dscr->ac_ssid) != cfg_ssid.ssid_len) ||
        (osal_memcmp(cfg_ssid.ac_ssid, bss_dscr->ac_ssid, cfg_ssid.ssid_len) != 0)) {
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }
    roam_alg = &(roam_info->alg);

    if (roam_info->current_bss_ignore == OAL_FALSE) {
        if (oal_time_after((osal_ulong)oal_time_get_stamp_ms(),
            (osal_ulong)(bss_dscr->timestamp + HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE)) != 0) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    /* 检查黑名单 */
    ret = hmac_roam_alg_find_in_blacklist_etc(roam_info, bss_dscr->bssid);
    if (ret == OAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_alg_bss_check_etc:: [%02X:%02X:%02X:%02X:XX:XX] in blacklist!}",
            /* 打印mac的第0位，第1位，第2位，第3位 */
            bss_dscr->bssid[0], bss_dscr->bssid[1], bss_dscr->bssid[2], bss_dscr->bssid[3]);
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* check bssid blacklist from Framework setting */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BLACKLIST_FILTER_ETC);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_blacklist_filter_etc_cb)fhook)(hmac_vap, bss_dscr->bssid);
        if (ret == OAL_TRUE) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    return hmac_roam_alg_bss_check_privacy_and_rssi(roam_info, bss_dscr, hmac_vap, roam_alg);
}

oal_bool_enum_uint8 hmac_roam_alg_need_to_stop_roam_trigger_etc(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru              *hmac_vap = OAL_PTR_NULL;
    hmac_roam_alg_stru         *roam_alg = OAL_PTR_NULL;

    if (roam_info == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    hmac_vap = roam_info->hmac_vap;
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    roam_alg  = &(roam_info->alg);

    if (roam_alg->another_bss_scaned == OSAL_TRUE) {
        roam_alg->invalid_scan_cnt = 0xff;
        return OAL_FALSE;
    }

    if (roam_alg->invalid_scan_cnt == 0xff) {
        return OAL_FALSE;
    }

    if (roam_alg->invalid_scan_cnt++ > 4) { /* 最大次数为4 */
        roam_alg->invalid_scan_cnt = 0xff;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

mac_bss_dscr_stru *hmac_roam_alg_select_bss_etc(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru              *hmac_vap = OAL_PTR_NULL;
    mac_bss_dscr_stru          *bss_dscr = OAL_PTR_NULL;
    hmac_roam_alg_stru         *roam_alg = OAL_PTR_NULL;
    osal_s8                    c_delta_rssi;
    osal_s8                    c_max_rssi = 0;

    if (roam_info == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    hmac_vap = roam_info->hmac_vap;
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    roam_alg = &(roam_info->alg);

    /* 取得最大 rssi 的 bss */
    bss_dscr = roam_alg->max_rssi_bss;

    if ((roam_alg->max_capacity_bss != OAL_PTR_NULL) &&
        (roam_alg->max_capacity >= ROAM_THROUGHPUT_THRESHOLD)) {
        /* 取得最大 capacity 的 bss */
        bss_dscr = roam_alg->max_capacity_bss;
    }

    if (bss_dscr == OAL_PTR_NULL) {
        /* should not be here */
        return OAL_PTR_NULL;
    }

    if (roam_info->current_bss_ignore == OAL_FALSE) {
        /* 过滤当前关联的 bss */
        if (oal_compare_mac_addr(hmac_vap->bssid, bss_dscr->bssid) == 0) {
            return OAL_PTR_NULL;
        }
    }

    /* rssi增益处理 */
    c_delta_rssi = hmac_roam_alg_adjust_rssi_increase(roam_info, bss_dscr);
    if (c_delta_rssi <= 0) {
        return OAL_PTR_NULL;
    }

    if (bss_dscr->st_channel.band == WLAN_BAND_5G) {
        c_max_rssi = roam_alg->c_max_rssi - ROAM_RSSI_DIFF_4_DB;
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_select_bss::roam_to 5G candidate AP, max_rssi=%d}",
            c_max_rssi);
        oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_alg_select_bss::[%02X:%02X:%02X:%02X:XX:XX]}",
            /* 0:1:2:3:数组下标 */
            bss_dscr->bssid[0], bss_dscr->bssid[1], bss_dscr->bssid[2], bss_dscr->bssid[3]);
    } else {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_alg_select_bss::roam_to candidate AP, max_rssi=%d}",
            roam_alg->c_max_rssi);
        oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_alg_select_bss::[%02X:%02X:%02X:%02X:XX:XX]}",
            /* 0:1:2:3:数组下标 */
            bss_dscr->bssid[0], bss_dscr->bssid[1], bss_dscr->bssid[2], bss_dscr->bssid[3]);
    }

    return bss_dscr;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
