/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: HMAC层 STA模式SME文件.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_sme_sta.h"
#include "mac_regdomain.h"
#include "hmac_main.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_mgmt_classifier.h"
#ifdef _PRE_WIFI_DMT
#include "hal_witp_dmt_if.h"
#include "dmt_stub.h"
#include "frame_public.h"
#endif
#include "hmac_wps.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "hmac_dfx.h"
#include "wlan_msg.h"
#include "hmac_feature_interface.h"
#include "hmac_wps.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SME_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define WLAN_11B_SUPPORT_RATE_NUM   4
#define WLAN_11B_SUPPORT_RATE_1M    0x82
#define WLAN_11B_SUPPORT_RATE_2M    0x84
#define WLAN_11B_SUPPORT_RATE_5M    0x8b
#define WLAN_11B_SUPPORT_RATE_11M   0x96

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
    g_handle_rsp_func_sta: 处理发给sme的消息
*****************************************************************************/
OAL_STATIC hmac_sme_handle_rsp_func g_handle_rsp_func_sta[HMAC_SME_RSP_BUTT] = {
    hmac_handle_scan_rsp_sta_etc,
    hmac_handle_join_rsp_sta_etc,
    hmac_handle_auth_rsp_sta_etc,
    hmac_handle_asoc_rsp_sta_etc,
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

OAL_STATIC osal_void hmac_cfg80211_scan_comp_cb(void *p_scan_record);
osal_void hmac_send_rsp_to_sme_sta_etc(hmac_vap_stru *hmac_vap, hmac_sme_rsp_enum_uint8 type,
    osal_u8 *msg);
static osal_u32 hmac_cfg80211_scan_channel_pre_check(mac_cfg80211_scan_param_stru *cfg80211_scan_param);

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_void hmac_cfg80211_dump_survey_fill_info(oal_wiphy_stru *wiphy, wlan_scan_chan_stats_stru *record,
    osal_s32 idx, hmac_survey_info_stru *info)
{
    (osal_void)memset_s(info, OAL_SIZEOF(hmac_survey_info_stru), 0, OAL_SIZEOF(hmac_survey_info_stru));

    info->time = record[idx].total_stats_time_us / 1000; /* 1000单位换算 */
    /* 1000单位换算 */
    info->time_busy = (record[idx].total_stats_time_us - record[idx].total_free_time_20m_us) / 1000;
    info->time_ext_busy = 0;
    info->time_rx = record[idx].total_recv_time_us / 1000; /* 1000单位换算 */
    info->time_tx = record[idx].total_send_time_us / 1000; /* 1000单位换算 */

    if (record[idx].free_power_cnt && record[idx].free_power_stats_20m < 0) {
        info->noise = record[idx].free_power_stats_20m / record[idx].free_power_cnt;
        info->free_power_cnt = record[idx].free_power_cnt;
        info->free_power_stats_20m = record[idx].free_power_stats_20m;
    }

    info->channel = oal_ieee80211_get_channel(wiphy, oal_ieee80211_channel_to_frequency(record[idx].channel_number,
        mac_get_band_by_channel_num(record[idx].channel_number)));

    oam_info_log4(0, OAM_SF_ANY, "survey:idx %d %d %x %d", idx, info->noise, info->filled,
        (info->channel) ? info->channel->center_freq : 0xffff);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_dump_survey_etc
 功能描述  :向内核上报信道统计结果
*****************************************************************************/
osal_s32 hmac_cfg80211_dump_survey_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    osal_s32 idx, hmac_survey_info_stru *info)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    wlan_scan_chan_stats_stru *record = OAL_PTR_NULL;

    if ((wiphy == OAL_PTR_NULL) || (netdev == OAL_PTR_NULL) || (info == OAL_PTR_NULL) || idx < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey_etc::null para or idx=%d!}", idx);
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)(netdev->ml_priv);
    if (!hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey_etc::hmac_vap = NULL}");
        return -OAL_EINVAL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (!hmac_device) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey_etc::hmac_dev = NULL}");
        return -OAL_EINVAL;
    }

    if (hmac_device->scan_mgmt.is_scanning) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_cfg80211_dump_survey_etc::drop request while scan running}");
        return -OAL_EINVAL;
    }

    record = hmac_device->scan_mgmt.scan_record_mgmt.chan_results;

    if (idx >= hmac_device->scan_mgmt.scan_record_mgmt.chan_numbers) {
        return -ENOENT;
    }

    hmac_cfg80211_dump_survey_fill_info(wiphy, record, idx, info);

    return 0;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_cfg80211_scan_comp_cb
 功能描述  : wpa_supplicant下发的扫描请求的回调函数，用于对扫描完成时对结果的处理
 输入参数  : void  *p_scan_record，扫描记录，包括扫描发起者信息和扫描结果
*****************************************************************************/
OAL_STATIC osal_void hmac_cfg80211_scan_comp_cb(void *p_scan_record)
{
    hmac_scan_record_stru           *scan_record = (hmac_scan_record_stru *)p_scan_record;
    hmac_vap_stru                   *hmac_vap = OAL_PTR_NULL;
    hmac_scan_rsp_stru               scan_rsp;

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_scan_comp_cb::hmac_vap is null.");
        return;
    }

    /* 上报扫描结果结构体初始化 */
    memset_s(&scan_rsp, OAL_SIZEOF(scan_rsp), 0, OAL_SIZEOF(scan_rsp));

    scan_rsp.result_code = scan_record->scan_rsp_status;

    /* 扫描结果发给sme */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_SCAN_RSP, (osal_u8 *)&scan_rsp);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_scan_comp
 功能描述  : wpa_supplicant下发的扫描请求的回调函数，用于对扫描完成时对结果的处理
 输入参数  : void  *p_scan_record，扫描记录，包括扫描发起者信息和扫描结果
*****************************************************************************/
osal_void hmac_cfg80211_scan_comp(hmac_scan_record_stru *p_scan_record)
{
    hmac_vap_stru                   *hmac_vap = OAL_PTR_NULL;
    hmac_scan_rsp_stru               scan_rsp;

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(p_scan_record->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_scan_comp::hmac_vap is null.");
        return;
    }

    /* 上报扫描结果结构体初始化 */
    memset_s(&scan_rsp, OAL_SIZEOF(scan_rsp), 0, OAL_SIZEOF(scan_rsp));

    scan_rsp.result_code = p_scan_record->scan_rsp_status;

    /* 扫描结果发给sme */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_SCAN_RSP, (osal_u8 *)&scan_rsp);

    return;
}

OAL_STATIC osal_void hmac_cfg80211_scan_params(const hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param, mac_scan_req_stru *scan_params)
{
    osal_u8 loop, len, ssid_num;
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u32 ret;
    hmac_vap_stru *mac_vap_temp = OAL_PTR_NULL;

    /* p2p DEV扫描且GO连接时，每信道只扫描一次 */
    if (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE) {
        ret = mac_device_find_up_p2p_go_etc(hmac_device, &mac_vap_temp);
        if ((ret == OAL_SUCC) && (mac_vap_temp != OAL_PTR_NULL)) {
            scan_params->max_scan_count_per_channel = 1;
        }
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */
    unref_param(hmac_device);
    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) ||
        ((hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) && (hmac_vap->user_nums > 0))) {
        scan_params->max_scan_count_per_channel = 1;
    }

    ssid_num = osal_min((osal_u8)(cfg80211_scan_param->ssid_num), WLAN_SCAN_REQ_MAX_SSID);

    for (loop = 0; loop < ssid_num; loop++) {
        len = cfg80211_scan_param->ssids[loop].ssid_len;
        if (cfg80211_scan_param->ssids[loop].ssid_len >= sizeof(scan_params->mac_ssid_set[loop].ssid)) {
            len = (osal_u8)sizeof(scan_params->mac_ssid_set[loop].ssid) - 1; /* 预留1位结束符 */
            oam_warning_log1(0, OAM_SF_SCAN, "hmac_cfg80211_scan_params:%u", cfg80211_scan_param->ssids[loop].ssid_len);
        }
        if (memcpy_s(scan_params->mac_ssid_set[loop].ssid, sizeof(scan_params->mac_ssid_set[loop].ssid),
            cfg80211_scan_param->ssids[loop].ssid, len) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_cfg80211_scan_params::memcpy ssid fail.}");
        }
        /* ssid末尾置'\0' */
        scan_params->mac_ssid_set[loop].ssid[len] = '\0';

#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
            wifi_printf("ssid_num[%d],loop[%d],len[%d],[%s]\r\n", ssid_num, loop, len,
                scan_params->mac_ssid_set[loop].ssid);
        }
#endif
    }

    /* 如果上层下发了指定ssid，则每次扫描发送的probe req帧的个数为下发的ssid个数 */
    scan_params->max_send_probe_req_count_per_channel = (osal_u8)cfg80211_scan_param->ssid_num;

    /* 如果上层未下发指定ssid，则采用默认值 */
    if (scan_params->max_send_probe_req_count_per_channel == 0) {
        scan_params->mac_ssid_set[0].ssid[0] = '\0';   /* 通配ssid */
        scan_params->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    }
    /* 3:则调整发送超时时间 */
    if ((scan_params->max_send_probe_req_count_per_channel > 3) && (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
        /* 如果指定SSID个数大于3个,则调整发送超时时间为40ms,默认发送等待超时20ms */
        scan_params->scan_time = WLAN_LONG_ACTIVE_SCAN_TIME;
    }

    scan_params->ssid_num = (osal_u8)cfg80211_scan_param->ssid_num;
    oal_set_mac_addr(scan_params->bssid[0], BROADCAST_MACADDR);
    scan_params->bssid_num = 1;  /* 初始扫描请求指定1个bssid，为广播地址 */

    return;
}

OAL_STATIC osal_u8 hmac_cfg80211_scan_channel_5g(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param, oal_bool_enum_uint8 chan_cfg_flag)
{
    osal_u32 ret;
    osal_u8 loop;
    osal_u8 channel_number;
    osal_u8 channel_idx = 0;
    osal_u8 chan_num_5g = 0;
    /* 5G初始扫描信道 */
    for (loop = 0; loop < cfg80211_scan_param->num_channels_5g; loop++) {
        channel_number = (osal_u8)cfg80211_scan_param->channels_5g[loop];

        /* 判断信道是不是在管制域内  */
        ret = hmac_is_channel_num_valid_etc(WLAN_BAND_5G, channel_number);
        if (ret == OAL_SUCC) {
            ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_5G, channel_number, &channel_idx);
            if (ret != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_cfg80211_scan_channel::index fail. channel:%u}",
                    hmac_vap->vap_id, channel_number);
            }
            /* 当用信道配置生效时，会按照5G列表的bit位将信道列表过滤 */
            if ((chan_cfg_flag == OAL_TRUE) && ((hmac_vap->channel_list_5g & (1 << channel_idx)) == 0x0)) {
                continue;
            }
            if (chan_num_5g >= WLAN_MAX_CHANNEL_NUM) {
                oam_warning_log2(0, OAM_SF_SCAN, "hmac_cfg80211_scan_channel_5g:5g num [%d], WLAN_MAX_CHANNEL_NUM [%d]",
                    chan_num_5g, WLAN_MAX_CHANNEL_NUM);
                continue;
            }

            scan_params->channel_list[chan_num_5g].band = WLAN_BAND_5G;
            scan_params->channel_list[chan_num_5g].chan_number = channel_number;
            scan_params->channel_list[chan_num_5g].chan_idx = channel_idx;

            scan_params->channel_nums++;
            chan_num_5g++;
        }
    }
    return chan_num_5g;
}

OAL_STATIC osal_u8 hmac_cfg80211_scan_channel_2g(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param, oal_bool_enum_uint8 chan_cfg_flag, osal_u8 chan_num_5g)
{
    osal_u32 ret;
    osal_u8 loop;
    osal_u8 channel_number;
    osal_u8 channel_idx = 0;
    osal_u8 chan_num_2g = 0;

    hmac_device_stru *hmac_dev = OAL_PTR_NULL;
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    hmac_dev = hmac_res_get_mac_dev_etc(0);
    scan_mgmt = &(hmac_dev->scan_mgmt);
    scan_mgmt->scan_2g_ch_list_map = 0;

   /* 2G初始扫描信道 */
    for (loop = 0; loop < cfg80211_scan_param->num_channels_2g; loop++) {
        channel_number = (osal_u8)cfg80211_scan_param->channels_2g[loop];

        /* 判断信道是不是在管制域内 */
        ret = hmac_is_channel_num_valid_etc(WLAN_BAND_2G, channel_number);
        if (ret == OAL_SUCC) {
            ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_2G, channel_number, &channel_idx);
            if (ret != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_cfg80211_scan_channel::index fail.channel: %u.}",
                    hmac_vap->vap_id, channel_number);
            }
            /* 当用信道配置生效时，会按照2.4G列表的bit位将信道列表过滤 */
            if ((chan_cfg_flag == OAL_TRUE) && ((hmac_vap->channel_list_2g & (1 << channel_idx)) == 0x0)) {
                continue;
            }
            if ((chan_num_5g + chan_num_2g) >= WLAN_MAX_CHANNEL_NUM) {
                oam_warning_log3(0, OAM_SF_SCAN,
                    "hmac_cfg80211_scan_channel_2g:5g num [%d], 2g num [%d], WLAN_MAX_CHANNEL_NUM [%d]",
                    chan_num_5g, chan_num_2g, WLAN_MAX_CHANNEL_NUM);
                continue;
            }

            scan_params->channel_list[chan_num_5g + chan_num_2g].band = WLAN_BAND_2G;
            scan_params->channel_list[chan_num_5g + chan_num_2g].chan_number = channel_number;
            scan_params->channel_list[chan_num_5g + chan_num_2g].chan_idx = channel_idx;

            /* 将请求的扫描信道对应的bit位置1 */
            scan_mgmt->scan_2g_ch_list_map |= (BIT0 << channel_number);

            scan_params->channel_nums++;
            chan_num_2g++;
        }
    }
    return chan_num_2g;
}

static osal_u32 hmac_cfg80211_scan_channel_pre_check(mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    if (((cfg80211_scan_param->num_channels_5g != 0) && (cfg80211_scan_param->channels_5g == OAL_PTR_NULL)) ||
        ((cfg80211_scan_param->num_channels_2g != 0) && (cfg80211_scan_param->channels_2g == OAL_PTR_NULL))) {
        oam_error_log4(0, OAM_SF_SCAN, "hmac_cfg80211_scan_channel:null ptr:5g num[%d] ptr[%p], 2g num[%d] ptr[%p]",
            cfg80211_scan_param->num_channels_5g, (uintptr_t)cfg80211_scan_param->channels_5g,
            cfg80211_scan_param->num_channels_2g, (uintptr_t)cfg80211_scan_param->channels_2g);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_cfg80211_scan_channel(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
    mac_cfg80211_scan_param_stru *cfg80211_scan_param)
{
    osal_u8 chan_num_5g = 0;
    osal_u8 chan_num_2g = 0;
    oal_bool_enum_uint8 chan_cfg_flag;

    /* 用户信道配置标志位，当2.4G和5G的BIT位都不为0是为真，否则为假 */
    chan_cfg_flag = ((hmac_vap->channel_list_5g != 0x0) || (hmac_vap->channel_list_2g != 0x0));

    if (hmac_cfg80211_scan_channel_pre_check(cfg80211_scan_param) != OAL_SUCC) {
        return;
    }

    chan_num_5g = hmac_cfg80211_scan_channel_5g(hmac_vap, scan_params, cfg80211_scan_param, chan_cfg_flag);

    chan_num_2g = hmac_cfg80211_scan_channel_2g(hmac_vap, scan_params, cfg80211_scan_param, chan_cfg_flag, chan_num_5g);

    // 如果满足条件, 进行信道随机化处理
    hmac_scan_handle_channel_randomzie(scan_params, chan_num_5g, chan_num_2g);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_prepare_scan_req_sta
 功能描述  : 解析内核下发的扫描命令参数
*****************************************************************************/
OAL_STATIC osal_void hmac_cfg80211_prepare_scan_req_sta(hmac_vap_stru *hmac_vap,
    mac_scan_req_stru *scan_params, osal_s8 *param)
{
    mac_cfg80211_scan_param_stru *cfg80211_scan_param = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    if (scan_params == OAL_PTR_NULL || param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_cfg80211_prepare_scan_req_sta:null ptr");
        return;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap[%d]hmac_cfg80211_prepare_scan_req_sta:hmac_res_get_mac_dev_etc fail:%u",
            hmac_vap->vap_id, hmac_vap->device_id);
        return;
    }

    cfg80211_scan_param = (mac_cfg80211_scan_param_stru *)param;

    scan_params->bss_type  = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->scan_type = cfg80211_scan_param->scan_type;

    /* 设置扫描时间 */
    if (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE) {
        scan_params->scan_time = hmac_scan_get_scan_para_from_config(PARA_TYPE_SCAN_TIME);
    } else {
        scan_params->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
    }

    scan_params->probe_delay = 0;
#if defined(_PRE_WLAN_FEATURE_WS92_MERGE) && defined(_PRE_WLAN_FEATURE_ACS)
    if (cfg80211_scan_param->acs_scan_flag == OAL_TRUE) {
        scan_params->scan_func = MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
    } else {
        scan_params->scan_func = MAC_SCAN_FUNC_BSS;  /* 默认扫描bss */
    }
#else
    scan_params->scan_func = MAC_SCAN_FUNC_BSS;  /* 默认扫描bss */
    scan_params->scan_func |= MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
#endif
    scan_params->fn_cb = hmac_cfg80211_scan_comp_cb;   /* 扫描完成回调函数 */
    scan_params->max_scan_count_per_channel = hmac_scan_get_scan_para_from_config(PARA_TYPE_SCAN_COUNT);
    scan_params->single_probe_times = hmac_scan_get_scan_para_from_config(PARA_TYPE_SCAN_PROBE_SEND_TIMES);

    hmac_cfg80211_scan_params(hmac_vap, hmac_device, cfg80211_scan_param, scan_params);
    hmac_cfg80211_scan_channel(hmac_vap, scan_params, cfg80211_scan_param);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* WLAN/P2P 共存时，判断是否p2p0 发起扫描 */
    scan_params->is_p2p0_scan = cfg80211_scan_param->is_p2p0_scan;
    if (cfg80211_scan_param->is_p2p0_scan) {
        scan_params->bss_type = 0;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */
}

/*****************************************************************************
 函 数 名  : hmac_prepare_join_req_etc
 功能描述  : 准备加入请求
*****************************************************************************/
osal_void hmac_prepare_join_req_etc(hmac_join_req_stru *join_req, mac_bss_dscr_stru *bss_dscr)
{
    memset_s(join_req, OAL_SIZEOF(hmac_join_req_stru), 0, OAL_SIZEOF(hmac_join_req_stru));
    if (memcpy_s(&(join_req->bss_dscr), sizeof(mac_bss_dscr_stru), bss_dscr, sizeof(mac_bss_dscr_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_prepare_join_req_etc data::memcpy err.}");
    }
    join_req->join_timeout = WLAN_JOIN_START_TIMEOUT;
    join_req->probe_delay  = WLAN_PROBE_DELAY_TIME;
}

/*****************************************************************************
 函 数 名  : hmac_prepare_auth_req
 功能描述  : 准备认证请求
*****************************************************************************/
osal_void hmac_prepare_auth_req(hmac_vap_stru *hmac_vap, hmac_auth_req_stru *auth_req)
{
    memset_s(auth_req, OAL_SIZEOF(hmac_auth_req_stru), 0, OAL_SIZEOF(hmac_auth_req_stru));

    /* 增加AUTH报文次数后，将超时时间减半 */
    auth_req->timeout = (osal_u16)mac_mib_get_AuthenticationResponseTimeOut(hmac_vap) >> 1;
}

/*****************************************************************************
 函 数 名  : hmac_prepare_asoc_req
 功能描述  : 准备关联请求
*****************************************************************************/
OAL_STATIC osal_void hmac_prepare_asoc_req(hmac_vap_stru *hmac_vap, hmac_asoc_req_stru *asoc_req)
{
    memset_s(asoc_req, OAL_SIZEOF(hmac_asoc_req_stru), 0, OAL_SIZEOF(hmac_asoc_req_stru));

    /* 增加ASOC报文次数后，将超时时间减半 */
    asoc_req->assoc_timeout = (osal_u16)mac_mib_get_dot11AssociationResponseTimeOut(hmac_vap) >> 1;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 函 数 名  : hmac_cfg80211_check_can_start_sched_scan_etc
 功能描述  : 检测是否可以启动调度扫描
*****************************************************************************/
osal_u32 hmac_cfg80211_check_can_start_sched_scan_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru      *hmac_device;
    hmac_vap_stru          *hmac_vap_tmp = OAL_PTR_NULL;
    osal_u32             ul_ret;

    /* 获取mac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_cfg80211_check_can_start_sched_scan_etc::hmac_device[%d] null.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果vap的模式不是STA，则返回，不支持其它模式的vap的调度扫描 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_cfg80211_check_can_start_sched_scan_etc::vap mode[%d] don't support sched scan.}",
            hmac_vap->vap_id, hmac_vap->vap_mode);

        hmac_device->scan_mgmt.sched_scan_req = OAL_PTR_NULL;
        hmac_device->scan_mgmt.sched_scan_complete = OAL_TRUE;
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 如果存在当前device存在up的vap，则不启动调度扫描 */
    ul_ret = mac_device_find_up_vap_etc(hmac_device, &hmac_vap_tmp);
    if (ul_ret == OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_cfg80211_check_can_start_sched_scan_etc::exist up vap, don't start sched scan.}",
            hmac_vap->vap_id);

        hmac_device->scan_mgmt.sched_scan_req = OAL_PTR_NULL;
        hmac_device->scan_mgmt.sched_scan_complete = OAL_TRUE;
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_start_sched_scan_etc
 功能描述  : 根据内核下发命令启动扫描
*****************************************************************************/
osal_s32 hmac_cfg80211_start_sched_scan_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_pno_scan_stru *cfg80211_pno_scan = OAL_PTR_NULL;
    mac_pno_scan_stru pno_scan_params;
    osal_u32 ul_ret;
    osal_u8 is_random_mac_scan;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif
    /* ~{2NJ}:O7(PT<l2i~} */
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL) || OAL_UNLIKELY(msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN,
            "{hmac_cfg80211_start_sched_scan_etc::input null %p %p.}", hmac_vap, msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ~{IO2c4+5DJG5XV7#,Rr4KPhR*H!V5;qH!5=UfU}5D~}pno~{I(Ch2NJ}KyTZ5D5XV7~} */
    cfg80211_pno_scan = (mac_pno_scan_stru *)msg->data;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_cfg80211_start_sched_scan_etc::device id[%d],hmac_device null.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    /* 拷贝上层下发的pno扫描参数 */
    if (memcpy_s(&pno_scan_params, sizeof(mac_pno_scan_stru), cfg80211_pno_scan, sizeof(mac_pno_scan_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_cfg80211_start_sched_scan_etc data::memcpy err.}");
    }

    /* 统一在外层释放 */

    /* 检测当前device是否可以启动调度扫描 */
    ul_ret = hmac_cfg80211_check_can_start_sched_scan_etc(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        return (osal_s32)ul_ret;
    }

    /* 设置PNO调度扫描结束时，如果有结果上报，则上报扫描结果的回调函数 */
    pno_scan_params.fn_cb = hmac_cfg80211_scan_comp_cb;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    is_random_mac_scan = wlan_customize_etc->random_mac_addr_scan;
#else
    is_random_mac_scan = hmac_device->scan_mgmt.is_random_mac_addr_scan;
#endif

    /* 设置发送的probe req帧中源mac addr */
    pno_scan_params.is_random_mac_addr_scan = is_random_mac_scan ? OSAL_TRUE : OSAL_FALSE;
    hmac_scan_set_sour_mac_addr_in_probe_req_etc(hmac_vap, pno_scan_params.sour_mac_addr, is_random_mac_scan);

    /* 状态机调用: hmac_scan_proc_scan_req_event_etc */
    ul_ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_SCHED_SCAN_REQ, (osal_void *)(&pno_scan_params));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_cfg80211_start_sched_scan_etc::hmac_fsm_call_func_sta_etc fail[%d].}",
            hmac_vap->vap_id, ul_ret);
        hmac_device->scan_mgmt.sched_scan_req     = OAL_PTR_NULL;
        hmac_device->scan_mgmt.sched_scan_complete = OAL_TRUE;
        return (osal_s32)ul_ret;
    }

    return OAL_SUCC;
}
#endif

static osal_u32 hmac_save_scan_app_ie(hmac_vap_stru *hmac_vap, mac_cfg80211_scan_param_stru *mac_cfg80211_scan_param)
{
    oal_app_ie_stru *app_ie = OSAL_NULL;
    osal_u32 ret;

    app_ie = (oal_app_ie_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(oal_app_ie_stru), OSAL_TRUE);
    if (osal_unlikely(app_ie == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_save_scan_app_ie::oal_memalloc failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    memset_s(app_ie, sizeof(oal_app_ie_stru), 0, sizeof(oal_app_ie_stru));

    if ((mac_cfg80211_scan_param->ie != OSAL_NULL) && (mac_cfg80211_scan_param->ie_len != 0)) {
        app_ie->ie_len = mac_cfg80211_scan_param->ie_len;
        if (memcpy_s(app_ie->ie, WLAN_WPS_IE_MAX_SIZE, mac_cfg80211_scan_param->ie, app_ie->ie_len) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_save_scan_app_ie::memcpy app_ie->ie fail.}");
        }
    }

    if (app_ie->ie_len == 0) {
        oal_mem_free(app_ie, OSAL_TRUE);
        return OAL_SUCC;
    }

    app_ie->app_ie_type = OAL_APP_PROBE_REQ_IE;
    ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, app_ie, app_ie->app_ie_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_save_scan_app_ie::hmac_config_set_app_ie_to_vap_etc fail, err_code=%d.}", ret);
    }
    oal_mem_free(app_ie, OSAL_TRUE);
    return ret;
}

OAL_STATIC osal_u32 hmac_cfg80211_scan_app_ie(hmac_vap_stru *hmac_vap,
    mac_cfg80211_scan_param_stru *mac_cfg80211_scan_param, mac_scan_req_stru *scan_params)
{
    osal_u32 ret;
#ifdef _PRE_WLAN_FPGA_DIGITAL
    osal_u8 channel;
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u8 p2p_listen_channel = 0xff; /* 初始化为无效值 */

    /* P2P0 扫描时记录P2P listen channel */
    if (mac_cfg80211_scan_param->is_p2p0_scan == OAL_TRUE) {
        hmac_find_p2p_listen_channel_etc(hmac_vap, (osal_u16)(mac_cfg80211_scan_param->ie_len),
            (osal_u8 *)(mac_cfg80211_scan_param->ie), &p2p_listen_channel);
        scan_params->p2p0_listen_channel = p2p_listen_channel;
    }
#endif
    unref_param(scan_params);
    ret = hmac_save_scan_app_ie(hmac_vap, mac_cfg80211_scan_param);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_cfg80211_scan_app_ie:: set app ie fail, err_code=%d.}", ret);
        return ret;
    }

#ifdef _PRE_WLAN_FPGA_DIGITAL
    for (channel = 0; channel < WLAN_MAX_CHANNEL_NUM; channel++) {
        scan_params->channel_list[channel].en_bandwidth = hmac_vap->channel.en_bandwidth;
    }
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_cfg80211_start_scan_sta_etc
 功能描述  : 根据内核下发命令启动扫描
*****************************************************************************/
osal_s32 hmac_cfg80211_start_scan_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_scan_req_stru scan_params;
    mac_cfg80211_scan_param_stru *scan_param = OAL_PTR_NULL;
    osal_u32 ret, all_len;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_SCAN_DEL_WPS_IE);

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL) || OAL_UNLIKELY(msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_cfg80211_start_scan_sta_etc::null ptr, %p %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_res_get_mac_dev_etc(hmac_vap->device_id) == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap[%d]{hmac_cfg80211_start_scan_sta_etc:mac_device null}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    memset_s(&scan_params, OAL_SIZEOF(scan_params), 0, OAL_SIZEOF(scan_params));
    scan_param = (mac_cfg80211_scan_param_stru *)msg->data;

    /* 内存布局为 mac_cfg80211_scan_param_stru结构体 + 2g信道信息 + 5g信道信息 + ie */
    all_len = (osal_u32)OAL_SIZEOF(mac_cfg80211_scan_param_stru) + scan_param->ie_len +
        scan_param->num_channels_all * (osal_u32)OAL_SIZEOF(osal_u32) * 2; /* 2g 和 5g */
    if (msg->data_len != all_len) {
        oam_error_log3(0, OAM_SF_SCAN, "{hmac_cfg80211_start_scan_sta_etc::hmac_vap[%p] data_len[%u], all_len[%u].}",
            (uintptr_t)hmac_vap, msg->data_len, all_len);
        return OAL_FAIL;
    }

    scan_param->channels_2g = (osal_u32 *)(scan_param + 1);
    scan_param->channels_5g = scan_param->channels_2g + scan_param->num_channels_all;
    scan_param->ie = (osal_u8 *)(scan_param->channels_5g + scan_param->num_channels_all);

    /* 将内核下发的扫描参数更新到驱动扫描请求结构体中 */
    hmac_cfg80211_prepare_scan_req_sta(hmac_vap, &scan_params, (osal_s8 *)scan_param);

    /* 设置P2P/WPS IE 信息到 vap 结构体中 */
    /* BEGIN: WLAN发送的 probe request 不携带P2P IE */
    if (is_legacy_vap(hmac_vap)) {
        /* ini中配置为1则删除wps ie, wfa认证时要求保留wps ie, 需要配置为0 */
        if (fhook != OSAL_NULL) {
            ((hmac_scan_del_wps_ie_cb)fhook)(hmac_vap, &scan_param->ie_len, scan_param->ie,
                hmac_scan_get_probe_req_del_wps_ie_cfg());
        }
    }
    /* END: WLAN发送的 probe request 不携带P2P IE */

    ret = hmac_cfg80211_scan_app_ie(hmac_vap, scan_param, &scan_params);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_cfg80211_start_scan_sta_etc::hmac_cfg80211_scan_app_ie [%d].}", ret);
        return OAL_FAIL;
    }

    /* 状态机调用: hmac_scan_proc_scan_req_event_etc，hmac_scan_proc_scan_req_event_exception_etc */
    ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_SCAN_REQ, (osal_void *)(&scan_params));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap[%d]hmac_cfg80211_start_scan_sta_etc::fsm err[%d]", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_initiate_join_etc
 功能描述  : 发起加入
*****************************************************************************/
osal_u32 hmac_sta_initiate_join_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_join_req_stru join_req;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (bss_dscr == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ASSOC, "{hmac_sta_initiate_join_etc::input null hmac_vap[%p] bss_dscr[%p].}",
            (uintptr_t)hmac_vap, (uintptr_t)bss_dscr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->protocol == WLAN_LEGACY_11B_MODE) {
        hmac_vap->supp_rates[0] = WLAN_11B_SUPPORT_RATE_1M;
        hmac_vap->supp_rates[1] = WLAN_11B_SUPPORT_RATE_2M;
        hmac_vap->supp_rates[2] = WLAN_11B_SUPPORT_RATE_5M;  /* 2 元素索引 */
        hmac_vap->supp_rates[3] = WLAN_11B_SUPPORT_RATE_11M; /* 3 元素索引 */
        mac_mib_set_SupportRateSetNums(hmac_vap, WLAN_11B_SUPPORT_RATE_NUM);
    } else {
        /* 将需要关联的速率放入hmac_vap中 */
        if (memcpy_s(hmac_vap->supp_rates, WLAN_MAX_SUPP_RATES, bss_dscr->supp_rates,
            bss_dscr->num_supp_rates) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_sta_initiate_join_etc data::memcpy err.}");
        }
        mac_mib_set_SupportRateSetNums(hmac_vap, bss_dscr->num_supp_rates);
    }

    hmac_prepare_join_req_etc(&join_req, bss_dscr);

    /* 状态机调用  */
    ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_JOIN_REQ, (osal_void *)(&join_req));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_initiate_join_etc::hmac_fsm_call_func_sta_etc fail[%d].}", hmac_vap->vap_id, ret);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_AUTH_FSM_ERR,
                USER_CONN_FAIL);
        }
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_initiate_auth_etc
 功能描述  : 发起认证
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_initiate_auth_etc(hmac_vap_stru *hmac_vap)
{
    hmac_auth_req_stru  auth_req;
    osal_u32          ul_ret;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_initiate_auth_etc: hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_prepare_auth_req(hmac_vap, &auth_req);

    /* 状态机调用  */
    ul_ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_AUTH_REQ, (osal_void *)(&auth_req));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_initiate_join_etc::hmac_fsm_call_func_sta_etc fail[%d].}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_initiate_asoc_etc
 功能描述  : 发起关联
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_initiate_asoc_etc(hmac_vap_stru *hmac_vap)
{
    hmac_asoc_req_stru asoc_req;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_initiate_asoc_etc::hmac_vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_prepare_asoc_req(hmac_vap, &asoc_req);

    /* 状态机调用  */
    ret = hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_ASOC_REQ, (osal_void *)(&asoc_req));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] hmac_sta_initiate_asoc_etc::hmac_fsm_call_func_sta_etc fail[%d]",
            hmac_vap->vap_id, ret);

        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_ASOC_FSM_ERR,
                USER_CONN_FAIL);
        }
        return ret;
    }

    return OAL_SUCC;
}


#ifdef _PRE_WIFI_DMT
/*****************************************************************************
 函 数 名  : hmac_join_bss_sta
 功能描述  : 加入扫描到的期望的AP列表
*****************************************************************************/
OAL_STATIC osal_void hmac_join_bss_sta(hmac_vap_stru *hmac_vap)
{
    mac_bss_dscr_stru   *bss_dscr;
    osal_u32           bss_idx;
    hmac_device_stru    *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] hmac_join_bss_sta::hmac_device null",
                         hmac_vap->vap_id);
        return;
    }

    /* 是否有期望的AP存在 */
    if (hmac_device->desired_bss_num > 0) {
        /* 从后往前取期望的bss */
        bss_idx   = hmac_device->desired_bss_idx[hmac_device->desired_bss_num - 1];
        bss_dscr =
            hmac_scan_find_scanned_bss_dscr_by_index_etc(hmac_vap->device_id, bss_idx);

        hmac_device->desired_bss_num--;      /* 期望加入的AP个数-1 */

        oam_info_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_join_bss_sta::try to join bss[%d].}",
                      hmac_vap->vap_id, bss_idx);

        /* 加入请求 */
        hmac_sta_initiate_join_etc(hmac_vap, bss_dscr);
    }
}

/*****************************************************************************
 函 数 名  : hmac_prepare_bss_list
 功能描述  : 生成期望加入的bss列表
*****************************************************************************/
osal_void  hmac_prepare_bss_list(hmac_vap_stru *hmac_vap)
{
    osal_s8               *pc_mib_ssid;
    osal_u32              bss_idx = 0;
    hmac_device_stru       *hmac_device;
    hmac_bss_mgmt_stru     *bss_mgmt;
    hmac_scanned_bss_info  *scanned_bss;
    mac_bss_dscr_stru      *bss_dscr;
    struct osal_list_head    *entry;
    osal_u8               device_id;

    /* 获取device id */
    device_id = hmac_vap->device_id;

    /* 获取hmac device结构体 */
    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ASSOC, "hmac_prepare_bss_list::hmac_device null");
        return;
    }

    pc_mib_ssid = (osal_s8 *)mac_mib_get_desired_ssid(hmac_vap);
    /* 没有设置期望的ssid, 继续发起扫描 */
    if (pc_mib_ssid[0] == 0) {
        oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_prepare_bss_list::desired ssid is not set.}",
                      hmac_vap->vap_id);
        return;
    }

    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 更新期望的ap列表，遍历扫描到的bss, 看是否是期望的 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);

        if (osal_strcmp(pc_mib_ssid, bss_dscr->ac_ssid) == 0) {
            /* 检测期望join 的AP能力信息是否匹配 */
            if (OAL_SUCC !=
                hmac_check_capability_mac_phy_supplicant_etc(hmac_vap, bss_dscr)) {
                continue;
            }

            bss_dscr->desired = OAL_TRUE;
            /* 记录期望的AP在bss list中的索引号 */
            hmac_device->desired_bss_idx[hmac_device->desired_bss_num] = (osal_u8)bss_idx;
            hmac_device->desired_bss_num++;
        } else {
            bss_dscr->desired = OAL_FALSE;
        }

        bss_idx++;
    }

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    hmac_join_bss_sta(hmac_vap);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_handle_scan_rsp_sta_etc
 功能描述  : 处理扫描结果
*****************************************************************************/
osal_void hmac_handle_scan_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg)
{
    frw_msg msg_info = {0};

    msg_info.data = msg;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(hmac_scan_rsp_stru);

    frw_asyn_host_post_msg(WLAN_MSG_H2W_SCAN_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);

#ifdef _PRE_WIFI_DMT
    /* 准备期望加入的bss列表 */
    hmac_prepare_bss_list(hmac_vap);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_handle_join_rsp_sta_etc
 功能描述  : 处理加入结果
*****************************************************************************/
osal_void hmac_handle_join_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg)
{
    hmac_join_rsp_stru  *join_rsp = (hmac_join_rsp_stru *)msg;

    if (join_rsp->result_code == HMAC_MGMT_SUCCESS) {
        oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_handle_join_rsp_sta_etc::join succ.}",
                      hmac_vap->vap_id);

        /* 初始化AUTH次数 */
        mac_mib_set_StaAuthCount(hmac_vap, 1);
        hmac_sta_initiate_auth_etc(hmac_vap);
    } else {
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] hmac_handle_join_rsp_sta_etc::join fail[%d]",
                         hmac_vap->vap_id, join_rsp->result_code);
        hmac_process_del_sta(hmac_vap->vap_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_report_connect_failed_result_etc
 功能描述  : 发送关联失败结果到wpa_supplicant
*****************************************************************************/
osal_void hmac_report_connect_failed_result_etc(hmac_vap_stru *hmac_vap, mac_status_code_enum_uint16 reason_code)
{
    hmac_asoc_rsp_stru asoc_rsp;
    hmac_user_stru *hmac_user;
    osal_s32 ret;
    frw_msg msg_info = {0};

    memset_s(&asoc_rsp, sizeof(hmac_asoc_rsp_stru), 0, sizeof(hmac_asoc_rsp_stru));
    asoc_rsp.result_code       = HMAC_MGMT_TIMEOUT;
    asoc_rsp.status_code       = reason_code;

    /* 获取用户指针 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user != OAL_PTR_NULL) {
        /* 扫描超时需要释放对应HMAC VAP下的关联请求buff */
        asoc_rsp.asoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    }

    msg_info.data = (osal_u8 *)&asoc_rsp;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(hmac_asoc_rsp_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_ASOC_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_handle_auth_rsp_sta_etc
 功能描述  : 处理认证结果
*****************************************************************************/
osal_void hmac_handle_auth_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg)
{
    hmac_auth_rsp_stru *auth_rsp = (hmac_auth_rsp_stru *)msg;
    hmac_user_stru *hmac_user;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if (auth_rsp->status_code == HMAC_MGMT_SUCCESS) {
        oam_info_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_handle_auth_rsp_sta_etc::auth succ.}", hmac_vap->vap_id);

        /* 初始化AOSC次数 */
        mac_mib_set_StaAssocCount(hmac_vap, 1);
        hmac_sta_initiate_asoc_etc(hmac_vap);
        return;
    }

    oam_warning_log3(0, OAM_SF_AUTH, "vap_id[%d] hmac_handle_auth_rsp_sta_etc::auth fail[%d],cnt[%d]",
                     hmac_vap->vap_id, auth_rsp->status_code, mac_mib_get_StaAuthCount(hmac_vap));
#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_TX) != 0) {
        wifi_printf("[service]vap_id[%d] hmac_handle_auth_rsp_sta_etc::auth fail[%d],cnt[%d]\r\n",
            hmac_vap->vap_id, auth_rsp->status_code, mac_mib_get_StaAuthCount(hmac_vap));
    }
#endif
    if ((auth_rsp->status_code == MAC_UNSUPT_ALG || mac_mib_get_StaAuthCount(hmac_vap) >= MAX_AUTH_CNT) &&
        (mac_mib_get_authentication_mode(hmac_vap) == WLAN_WITP_AUTH_AUTOMATIC)) {
        mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_SHARED_KEY);
        /* 需要将状态机设置为 */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

        /* 更新AUTH的次数 */
        mac_mib_set_StaAuthCount(hmac_vap, 0);

        /* 重新发起关联动作 */
        hmac_sta_initiate_auth_etc(hmac_vap);

        return;
    }

    if (mac_mib_get_StaAuthCount(hmac_vap) < MAX_AUTH_CNT) {
        /* 需要将状态机设置为 */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

        /* 更新AUTH的次数 */
        mac_mib_incr_StaAuthCount(hmac_vap);

        /* 重新发起关联动作 */
        hmac_sta_initiate_auth_etc(hmac_vap);

        return;
    }
    hmac_sta_set_last_reason_code(auth_rsp->status_code);
    /* 上报关联失败到wpa_supplicant */
    hmac_report_connect_failed_result_etc(hmac_vap, auth_rsp->status_code);
    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_AUTH_TIMEOUT_ERR,
            USER_CONN_FAIL);
    }

    /* 获取用户指针 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user != OAL_PTR_NULL) {
        /* 删除对应用户 */
        hmac_user_del_etc(hmac_vap, hmac_user);
    } else {
        /* The MAC state is changed to fake up state. Further MLME     */
        /* requests are processed in this state.                       */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        oam_warning_log2(0, OAM_SF_AUTH, "vap[%d] auth hmac_user[%d] NULL.", hmac_vap->vap_id, hmac_vap->assoc_vap_id);
    }

    /* 删完用户后再调此接口,即使有auth的发送完成中断上来,删用户后不会再重传 */
    hmac_process_del_sta(hmac_vap->vap_id);
}

OAL_STATIC osal_void hmac_handle_asoc_fail_proc(hmac_vap_stru *hmac_vap, const hmac_asoc_rsp_stru *asoc_rsp)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u8 max_reassoc_count;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    max_reassoc_count = MAX_ASOC_CNT;
#ifdef _PRE_WLAN_FEATURE_WPA3
    if ((mac_mib_get_authentication_mode(hmac_vap) == WLAN_WITP_AUTH_SAE) || (hmac_vap->sae_have_pmkid == OSAL_TRUE)) {
        max_reassoc_count = (asoc_rsp->status_code == MAC_STATUS_INVALID_PMKID) ? 1 : max_reassoc_count;
    } else if ((mac_mib_get_authentication_mode(hmac_vap) == WLAN_WITP_AUTH_OPEN_SYSTEM) &&
        (asoc_rsp->status_code == MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED)) {
        /* owe group try next, need send to wpa Immediately */
        max_reassoc_count = 1;
    }
#endif
    oam_warning_log3(0, OAM_SF_DFT, "hmac_handle_asoc_fail_proc: status_code[%d], max_count[%d], current_count[%d]",
        asoc_rsp->status_code, max_reassoc_count, mac_mib_get_StaAssocCount(hmac_vap));
    if (mac_mib_get_StaAssocCount(hmac_vap) >= max_reassoc_count) {
        hmac_sta_set_last_reason_code(asoc_rsp->status_code);
        /* 上报关联失败到wpa_supplicant */
        hmac_report_connect_failed_result_etc(hmac_vap, asoc_rsp->status_code);
    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_ASOC_TIMEOUT_ERR,
            USER_CONN_FAIL);
    }

        /* 获取用户指针 */
        hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user != NULL) {
            /* 发送去认证帧到AP */
            hmac_mgmt_send_deauth_frame_etc(hmac_vap,
                hmac_user->user_mac_addr, MAC_AUTH_NOT_VALID, OAL_FALSE);
            oam_warning_log1(0, OAM_SF_DFT, "hmac_handle_asoc_fail_proc:assoc fail.send DEAUTH,err code[%d]",
                MAC_AUTH_NOT_VALID);

            /* 删除对应用户 */
            hmac_user_del_etc(hmac_vap, hmac_user);
        } else {
            /* 设置状态为FAKE UP */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
            oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] asoc:hmac_user[%d] NULL.",
                hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        }

        /* 同步DMAC状态 */
        hmac_process_del_sta(hmac_vap->vap_id);
    } else {
        /* 需要将状态机设置为 */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);

        /* 发起ASOC的次数 */
        mac_mib_incr_StaAssocCount(hmac_vap);

        /* 重新发起关联动作 */
        hmac_sta_initiate_asoc_etc(hmac_vap);
    }
}

/*
 * 功能描述：STA关联成功处理
 */
void hmac_handle_assoc_rsp_succ_sta(const hmac_vap_stru *hmac_vap, hmac_asoc_rsp_stru *asoc_rsp)
{
    osal_u8 *mgmt_data;
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (hmac_vap == NULL || asoc_rsp == NULL) {
        return;
    }

    oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_handle_asoc_rsp_sta_etc::asoc succ.}",
        hmac_vap->vap_id);
    /* asoc_rsp 帧拷贝一份上报上层,防止帧内容上报wal侧处理后被hmac侧释放 */
    mgmt_data = (osal_u8 *)oal_memalloc(asoc_rsp->asoc_rsp_ie_len);
    if (mgmt_data == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_handle_assoc_rsp_succ_sta::mgmt_data alloc null.}",
            hmac_vap->vap_id);
        return;
    }
    if (memcpy_s(mgmt_data, asoc_rsp->asoc_rsp_ie_len, (osal_u8 *)asoc_rsp->asoc_rsp_ie_buff,
        asoc_rsp->asoc_rsp_ie_len) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_handle_assoc_rsp_succ_sta data::memcpy err.}");
    }
    asoc_rsp->asoc_rsp_ie_buff = mgmt_data;

    msg_info.data = (osal_u8 *)asoc_rsp;
    msg_info.data_len = OAL_SIZEOF(hmac_asoc_rsp_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_ASOC_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oal_free(asoc_rsp->asoc_rsp_ie_buff);
        asoc_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
        return;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_handle_asoc_rsp_sta_etc
 功能描述  : 处理认证结果
*****************************************************************************/
osal_void hmac_handle_asoc_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg)
{
    hmac_asoc_rsp_stru *asoc_rsp = (hmac_asoc_rsp_stru *)msg;

    if (asoc_rsp->result_code == HMAC_MGMT_SUCCESS) {
        hmac_handle_assoc_rsp_succ_sta(hmac_vap, asoc_rsp);
        return;
    } else {
        oam_warning_log3(0, OAM_SF_AUTH, "vap_id[%d] {hmac_handle_asoc_rsp_sta_etc::asoc fail[%d], assoc_cnt[%d]}",
            hmac_vap->vap_id,
            asoc_rsp->result_code, mac_mib_get_StaAssocCount(hmac_vap));
#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_TX) != 0) {
            wifi_printf("[service]vap_id[%d] {hmac_handle_asoc_rsp_sta_etc::asoc fail[%d], assoc_cnt[%d]}\r\n",
                hmac_vap->vap_id, asoc_rsp->result_code, mac_mib_get_StaAssocCount(hmac_vap));
        }
#endif
        hmac_handle_asoc_fail_proc(hmac_vap, asoc_rsp);
    }
}

/*****************************************************************************
 函 数 名  : hmac_send_rsp_to_sme
 功能描述  : 将连接状态机结果上报给SME
*****************************************************************************/
osal_void  hmac_send_rsp_to_sme_sta_etc(hmac_vap_stru *hmac_vap, hmac_sme_rsp_enum_uint8 type, osal_u8 *msg)
{
#ifdef _PRE_WIFI_DMT
    /* 进行DMT结果上报，用例验证预期结果 */
    hmac_scan_rsp_stru              *scan_rsp;
    hmac_join_rsp_stru              *join_rsp;
    hmac_auth_rsp_stru              *auth_rsp;
    hmac_asoc_rsp_stru              *asoc_rsp;

    EXPECT_EVENT_STATUS              st_event_status;

    st_event_status.ucMsgType   = type;
    st_event_status.usMsgStatus = hmac_vap->vap_state;
    st_event_status.usMsgLength = sizeof(EXPECT_EVENT_STATUS);

    if (type == HMAC_SME_SCAN_RSP) {
        scan_rsp = (hmac_scan_rsp_stru *)msg;
        st_event_status.usMsgSubStatus = scan_rsp->result_code;
    } else if (type == HMAC_SME_JOIN_RSP) {
        join_rsp = (hmac_join_rsp_stru *)msg;
        st_event_status.usMsgSubStatus = join_rsp->result_code;
    } else if (type == HMAC_SME_AUTH_RSP) {
        auth_rsp = (hmac_auth_rsp_stru *)msg;
        st_event_status.usMsgSubStatus = auth_rsp->status_code;
    } else if (type == HMAC_SME_ASOC_RSP) {
        asoc_rsp = (hmac_asoc_rsp_stru *)msg;
        st_event_status.usMsgSubStatus = asoc_rsp->result_code;
    }

    oam_info_log3(0, OAM_SF_ANY, "[EVENT_REPORT] RspType=%d VapState=%d ResultCode=%d.",
        st_event_status.ucMsgType, st_event_status.usMsgStatus, st_event_status.usMsgSubStatus);

    DmtStub_Sta2DmtSendMsg(DEV_DMT_EVENT_STATUS_MSG, (osal_u8 *)&st_event_status, sizeof(EXPECT_EVENT_STATUS));
#endif

    g_handle_rsp_func_sta[type](hmac_vap, msg);
}

/*****************************************************************************
 函 数 名  : hmac_send_rsp_to_sme_ap_etc
*****************************************************************************/
osal_void hmac_send_rsp_to_sme_ap_etc(hmac_vap_stru *hmac_vap, hmac_ap_sme_rsp_enum_uint8 type,
    osal_u8 *msg)
{
    /* 目前只有DMT使用 */
    unref_param(hmac_vap);
    unref_param(type);
    unref_param(msg);
#ifdef _PRE_WIFI_DMT
    /* 进行DMT结果上报，用例验证预期结果 */
    hmac_ap_start_rsp_stru   *ap_start_rsp;
    EXPECT_EVENT_STATUS       st_event_status;

    st_event_status.ucMsgType   = type;
    st_event_status.usMsgStatus = hmac_vap->vap_state;
    st_event_status.usMsgLength = sizeof(EXPECT_EVENT_STATUS);

    if (type == HMAC_AP_SME_START_RSP) {
        ap_start_rsp = (hmac_ap_start_rsp_stru *)msg;
        st_event_status.usMsgSubStatus = ap_start_rsp->result_code;
    }

    oam_info_log3(0, OAM_SF_ANY, "[EVENT_REPORT] RspType=%d VapState=%d ResultCode=%d.",
        st_event_status.ucMsgType, st_event_status.usMsgStatus, st_event_status.usMsgSubStatus);

    DmtStub_Sta2DmtSendMsg(DEV_DMT_EVENT_STATUS_MSG, (osal_u8 *)&st_event_status, sizeof(EXPECT_EVENT_STATUS));
#endif
}

/*****************************************************************************
 功能描述  : 向lwip上报sta关联/去关联事件
*****************************************************************************/
osal_void hmac_report_assoc_state_sta(const hmac_vap_stru *hmac_vap, const osal_u8 *mac_addr, osal_u8 assoc)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    /* 抛加入完成事件到WAL */
    hmac_sta_report_assoc_info_stru sta_asoc = {0};
    osal_s8 rssi; /* bss的信号强度 */
#ifdef _PRE_WLAN_FEATURE_MESH
    osal_u8 is_td_mesh;
#endif

    if (assoc == OSAL_TRUE) {
        /* 获取hmac device 结构 */
        hmac_device_stru *hmac_dev = hmac_res_get_mac_dev_etc(0);
        /* 获取管理扫描的bss结果的结构体 */
        hmac_bss_mgmt_stru *bss_mgmt = &(hmac_dev->scan_mgmt.scan_record_mgmt.bss_mgmt);
        /* 对链表删操作前加锁 */
        osal_adapt_kthread_lock();
        hmac_scanned_bss_info *scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, mac_addr);
        if (scanned_bss_info == OSAL_NULL) {
            oam_warning_log4(hmac_vap->vap_id, OAM_SF_CFG, "hmac_report_assoc_state_sta:fail:%02X:%02X:%02X:%02X:XX:XX",
                             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0:1:2:3 元素索引 */
            /* 解锁 */
            osal_adapt_kthread_unlock();
            return;
        }
        rssi = scanned_bss_info->bss_dscr_info.c_rssi;
#ifdef _PRE_WLAN_FEATURE_MESH
        is_td_mesh = scanned_bss_info->bss_dscr_info.is_td_mesh;
#endif
        /* 解锁 */
        osal_adapt_kthread_unlock();

        oam_info_log0(hmac_vap->vap_id, OAM_SF_ASSOC, "{hmac_report_assoc_state_sta::Asoc Report!}");

        sta_asoc.is_assoc = OSAL_TRUE;
        sta_asoc.rssi = (td_u8)get_abs(rssi);
#ifdef _PRE_WLAN_FEATURE_MESH
        sta_asoc.conn_to_mesh = is_td_mesh;
#endif
        if (memcpy_s(sta_asoc.auc_mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "hmac_report_assoc_state_sta:: mem safe function err!");
            return;
        }
    } else {
        oam_info_log0(hmac_vap->vap_id, OAM_SF_ASSOC, "{hmac_report_assoc_state_sta::Disasoc Report!}");

        sta_asoc.is_assoc = OSAL_FALSE;
        sta_asoc.rssi = WLAN_RSSI_DUMMY_MARKER;
        sta_asoc.conn_to_mesh = OSAL_FALSE;
        if (memcpy_s(sta_asoc.auc_mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "hmac_report_assoc_state_sta:: mem safe function err!");
            return;
        }
    }

    frw_msg msg_info = {0};
    msg_info.data = (osal_u8 *)&sta_asoc;
    msg_info.data_len = OAL_SIZEOF(sta_asoc);

    (osal_void)frw_asyn_host_post_msg(WLAN_MSG_H2W_STA_CONN_RESULT, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
#endif
    return;
}

osal_void hmac_set_scan_param(hmac_vap_stru *hmac_vap, osal_u8 channel, mac_scan_req_stru *scan_param)
{
    osal_u32 ul_ret;
    osal_u8 ap_chan_idx = 0;
    wlan_channel_band_enum_uint8 chan_band;

    scan_param->channel_nums = 1;
    chan_band = mac_get_band_by_channel_num(channel);
    ul_ret = hmac_get_channel_idx_from_num_etc(chan_band, channel, &ap_chan_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RRM,
            "hmac_set_scan_param: hmac_get_channel_idx_from_num_etc fail,error_code=%d", ul_ret);
        return;
    }

    scan_param->channel_list[0].chan_number = channel;
    scan_param->channel_list[0].chan_idx = ap_chan_idx;
    scan_param->channel_list[0].band = chan_band;

    scan_param->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_param->scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
    scan_param->scan_type = WLAN_SCAN_TYPE_ACTIVE;

    scan_param->bssid_num = 1;
    scan_param->vap_id = hmac_vap->vap_id;
    scan_param->scan_func = MAC_SCAN_FUNC_BSS;

    scan_param->max_scan_count_per_channel = 1;
    scan_param->probe_delay = 0;
    scan_param->work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    scan_param->need_switch_back_home_channel = OAL_TRUE;
    scan_param->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    scan_param->scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    scan_param->fn_cb = hmac_cfg80211_scan_comp_cb;
    return;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

