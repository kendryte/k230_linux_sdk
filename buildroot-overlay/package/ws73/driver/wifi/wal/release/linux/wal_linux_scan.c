/*
 * Copyright (c) CompanyNameMagicTag 2001-2023. All rights reserved.
 * Description: 与内核接口相关的scan功能函数
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wal_linux_scan.h"
#include "oal_net.h"
#include "oal_cfg80211.h"
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "wal_linux_util.h"
#include "wal_linux_cfg80211.h"
#include "wal_main.h"
#include "wal_linux_rx_rsp.h"
#include "wal_linux_ioctl.h"

#include "wal_config.h"
#include "hmac_vap.h"
#include "hmac_device.h"
#include "mac_device_ext.h"
#include "hmac_resource.h"

#include "plat_pm_wlan.h"

#include <net/cfg80211.h>
#include "common_dft.h"
#include "common_log_dbg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_SCAN_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

osal_u32 is_p2p_scan_req(oal_cfg80211_scan_request_stru *request)
{
    if (request == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{is_p2p_scan_req::param request is NULL}");
        return OAL_FALSE;
    }

    return ((request->n_ssids > 0) && (request->ssids != NULL) &&
            (request->ssids[0].ssid_len == osal_strlen("DIRECT-")) &&
            (0 == osal_memcmp(request->ssids[0].ssid, "DIRECT-", osal_strlen("DIRECT-"))));
}

/*****************************************************************************
 函 数 名  : wal_inform_bss_frame
 功能描述  : 逐个上报ssid消息给内核
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void  wal_inform_bss_frame(wal_scanned_bss_info_stru *scanned_bss_info, osal_void *p_data)
{
    oal_cfg80211_bss_stru *cfg80211_bss = OAL_PTR_NULL;
    oal_wiphy_stru *wiphy = OAL_PTR_NULL;
    oal_ieee80211_channel_stru *ieee80211_channel = OAL_PTR_NULL;
    osal_timeval tv;

    if ((scanned_bss_info == OAL_PTR_NULL) || (p_data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "{wal_inform_bss_frame::input param pointer is null, scanned_bss_info[%p], p_data[%p]!}",
                       (uintptr_t)scanned_bss_info, (uintptr_t)p_data);
        return;
    }

    wiphy = (oal_wiphy_stru *)p_data;

    ieee80211_channel = oal_ieee80211_get_channel(wiphy, (osal_s32)scanned_bss_info->s_freq);
    if (ieee80211_channel == NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{wal_inform_bss_frame::get channel failed, wrong s_freq[%d]}",
                         (osal_s32)scanned_bss_info->s_freq);
        return;
    }

    scanned_bss_info->l_signal = scanned_bss_info->l_signal * 100; /* 上报到上层时，rssi扩大100倍 */

    /* 由于驱动缓存扫描结果，导致cts认证2次扫描的bss的timestamp时间一致(后一次没有扫描到) */
    osal_gettimeofday(&tv);
    scanned_bss_info->mgmt->u.probe_resp.timestamp = ((u64)tv.tv_sec * 1000000) /* 1S = 1000000us */
            + tv.tv_usec;

    /* 逐个上报内核bss 信息 */
    cfg80211_bss = oal_cfg80211_inform_bss_frame_etc(wiphy, ieee80211_channel,
        scanned_bss_info->mgmt, scanned_bss_info->mgmt_len, scanned_bss_info->l_signal, GFP_ATOMIC);
    if (cfg80211_bss != NULL) {
        oal_cfg80211_put_bss_etc(wiphy, cfg80211_bss);
    }

    return;
}

/*****************************************************************************
 函 数 名  : wal_update_bss_etc
 功能描述  : 更新指定bssid 的bss 信息
 输入参数  : oal_wiphy_stru  *wiphy
             hmac_bss_mgmt_stru  *bss_mgmt
             osal_u8   *bssid
 输出参数  : 无
*****************************************************************************/
osal_void wal_update_bss_etc(oal_wiphy_stru *wiphy, hmac_bss_mgmt_stru *bss_mgmt, osal_u8 *bssid)
{
    wal_scanned_bss_info_stru scanned_bss_info;
    oal_cfg80211_bss_stru *cfg80211_bss = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;
    osal_u8 chan_number;
    oal_ieee80211_band_enum_uint8 band;

    if (wiphy == OAL_PTR_NULL || bss_mgmt == OAL_PTR_NULL || bssid == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ASSOC, "wal_update_bss_etc: null para.");
        return;
    }

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 从本地扫描结果中获取bssid 对应的bss 信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr = &(scanned_bss->bss_dscr_info);

        /* 从扫描结果中对比关联的MAC 地址，如果相同，则提前退出 */
        if (osal_memcmp(bssid, bss_dscr->bssid, WLAN_MAC_ADDR_LEN) == 0) {
            break;
        }
        bss_dscr = OAL_PTR_NULL;
    }

    if (bss_dscr == OAL_PTR_NULL) {
        oam_warning_log4(0, OAM_SF_ASSOC, "wal_update_bss_etc:not find rel bss from scan res %02X:%02X:%02X:%02X:XX:XX",
            /* 0:1:2:3:数组下标 */
            bssid[0], bssid[1], bssid[2], bssid[3]);

        /* 解除锁 */
        osal_spin_unlock(&(bss_mgmt->lock));

        return;
    }

    /* 查找是否在内核中，如果没有该bss  ，则添加bss ，否则更新bss 时间 */
    cfg80211_bss = oal_cfg80211_get_bss_etc(wiphy, OAL_PTR_NULL, bssid,
        (osal_u8 *)(bss_dscr->ac_ssid), osal_strlen((const osal_s8 *)bss_dscr->ac_ssid));
    if (cfg80211_bss != OAL_PTR_NULL) {
        oal_cfg80211_put_bss_etc(wiphy, cfg80211_bss);
    } else {
        chan_number = bss_dscr->st_channel.chan_number;
        band        = (oal_ieee80211_band_enum_uint8)bss_dscr->st_channel.band;

        /* 初始化 */
        (osal_void)memset_s(&scanned_bss_info, sizeof(wal_scanned_bss_info_stru), 0, sizeof(wal_scanned_bss_info_stru));

        /* 填写BSS 信号强度 */
        scanned_bss_info.l_signal    = bss_dscr->c_rssi;

        /* 填bss所在信道的中心频率 */
        scanned_bss_info.s_freq      = (osal_s16)oal_ieee80211_channel_to_frequency(chan_number, band);

        /* 填管理帧指针和长度 */
        scanned_bss_info.mgmt    = (oal_ieee80211_mgmt_stru *)(bss_dscr->mgmt_buff);
        scanned_bss_info.mgmt_len = bss_dscr->mgmt_len;

        /* 获取上报的扫描结果的管理帧的帧头 */
        frame_hdr  = (mac_ieee80211_frame_stru *)bss_dscr->mgmt_buff;

        /* 如果扫描请求接收到的帧类型有beacon类型，统一修改为probe rsp类型上报，
           为了解决上报内核的扫描结果beacon帧不够敏感的问题，此问题，在01出现过 */
        if (frame_hdr->frame_control.sub_type == WLAN_BEACON) {
            /* 修改beacon帧类型为probe rsp */
            frame_hdr->frame_control.sub_type = WLAN_PROBE_RSP;
        }

        wal_inform_bss_frame(&scanned_bss_info, wiphy);
    }

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return;
}

OAL_STATIC osal_void wal_inform_all_bss_init(mac_bss_dscr_stru *bss_dscr, wal_scanned_bss_info_stru *scanned_bss_info,
    osal_u8 chan_number, oal_ieee80211_band_enum_uint8 band)
{
    mac_ieee80211_frame_stru *frame_hdr = OAL_PTR_NULL;

    if (bss_dscr->c_rssi > WAL_SCAN_RSSI_MAX) {
        scanned_bss_info->l_signal = WAL_SCAN_RSSI_MAX;
    } else {
        scanned_bss_info->l_signal = bss_dscr->c_rssi;
    }

    /* 填bss所在信道的中心频率 */
    scanned_bss_info->s_freq      = (osal_s16)oal_ieee80211_channel_to_frequency(chan_number, band);

    /* 填管理帧指针和长度 */
    scanned_bss_info->mgmt    = (oal_ieee80211_mgmt_stru *)(bss_dscr->mgmt_buff);
    scanned_bss_info->mgmt_len = bss_dscr->mgmt_len;

    /* 获取上报的扫描结果的管理帧的帧头 */
    frame_hdr = (mac_ieee80211_frame_stru *)bss_dscr->mgmt_buff;

    /*
        * 如果扫描请求接收到的帧类型有beacon类型，统一修改为probe rsp类型上报，
        * 为了解决上报内核的扫描结果beacon帧不够敏感的问题，此问题，在01出现过
        */
    if (frame_hdr->frame_control.sub_type == WLAN_BEACON) {
        /* 修改beacon帧类型为probe rsp */
        frame_hdr->frame_control.sub_type = WLAN_PROBE_RSP;
    }
}

/*****************************************************************************
 函 数 名  : wal_inform_all_bss_etc
 功能描述  : 上报所有的bss到内核
 输入参数  : oal_wiphy_stru  *wiphy,
             hmac_bss_mgmt_stru  *bss_mgmt,
             osal_u8   vap_id
*****************************************************************************/
osal_void  wal_inform_all_bss_etc(oal_wiphy_stru  *wiphy, hmac_bss_mgmt_stru  *bss_mgmt, osal_u8   vap_id)
{
    mac_bss_dscr_stru              *bss_dscr;
    hmac_scanned_bss_info          *scanned_bss;
    struct osal_list_head            *entry;
    wal_scanned_bss_info_stru       scanned_bss_info;
    osal_u32                      bss_num_not_in_regdomain = 0;
    osal_u32                      bss_num = 0;
    osal_u8                       chan_number;
    oal_ieee80211_band_enum_uint8             band;

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历扫描到的bss信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);
        if (bss_dscr == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {wal_inform_all_bss_etc::scan result invalid.}", vap_id);
            continue;
        }

        chan_number = bss_dscr->st_channel.chan_number;
        band        = (oal_ieee80211_band_enum_uint8)bss_dscr->st_channel.band;

        /* 判断信道是不是在管制域内，如果不在，则不上报内核 */
        if (hmac_is_channel_num_valid_etc(band, chan_number) != OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_SCAN, "vap[%d]{wal_inform_all_bss_etc::channel[%d] band[%d] not in regdomain.}",
                vap_id, chan_number, band);
            bss_num_not_in_regdomain++;
            continue;
        }

        /* 上报WAL_SCAN_REPORT_LIMIT以内的扫描结果 */
        if (oal_time_after((osal_ulong)oal_time_get_stamp_ms(),
            (osal_ulong)(bss_dscr->timestamp + WAL_SCAN_REPORT_LIMIT))) {
            continue;
        }

        /* 初始化 */
        (osal_void)memset_s(&scanned_bss_info, sizeof(wal_scanned_bss_info_stru), 0, sizeof(wal_scanned_bss_info_stru));
        wal_inform_all_bss_init(bss_dscr, &scanned_bss_info, chan_number, band);

        /* 上报扫描结果给内核 */
        wal_inform_bss_frame(&scanned_bss_info, wiphy);
        bss_num++;
    }

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    wifi_printf("scan: inform bss %d,other %d,vap %d\r\n",
        (bss_mgmt->bss_num - bss_num_not_in_regdomain), bss_num_not_in_regdomain, vap_id);

    common_log_dbg4(0, OAM_SF_SCAN, "vap_id:%d{wal_inform_all_bss_etc::bss %d not in reg,to kernal bss num:%d in %d}",
        vap_id, bss_num_not_in_regdomain, bss_num, (bss_mgmt->bss_num - bss_num_not_in_regdomain));
}

/*****************************************************************************
 函 数 名  : wal_set_scan_channel
 功能描述  : 提取内核下发扫描信道相关参数
*****************************************************************************/
OAL_STATIC osal_void wal_set_scan_channel(oal_cfg80211_scan_request_stru *request,
    mac_cfg80211_scan_param_stru *scan_param, hmac_vap_stru *hmac_vap)
{
    osal_u32 loop, is_5g_enable;
    osal_u32 num_chan_2g = 0;
    osal_u32 num_chan_5g = 0;

    scan_param->channels_2g = (osal_u32 *)(scan_param + 1);
    scan_param->channels_5g = (osal_u32 *)(scan_param->channels_2g + request->n_channels);

    is_5g_enable = hmac_device_check_5g_enable(hmac_vap->device_id);

    for (loop = 0; loop < request->n_channels; loop++) {
        osal_u16  center_freq;
        osal_u32  chan;

        center_freq = request->channels[loop]->center_freq;

        /* 根据中心频率，计算信道号 */
        chan = (osal_u32)oal_ieee80211_frequency_to_channel((osal_s32)center_freq);

        if (center_freq <= WAL_MAX_FREQ_2G) {
            scan_param->channels_2g[num_chan_2g++] = chan;
        } else {
            if (is_5g_enable == OAL_FALSE) {
                continue;
            }
            scan_param->channels_5g[num_chan_5g++] = chan;
        }
    }

    scan_param->num_channels_2g = (osal_u8)num_chan_2g;
    scan_param->num_channels_5g = (osal_u8)num_chan_5g;
    scan_param->num_channels_all = request->n_channels;

    return;
}

/*****************************************************************************
 函 数 名  : wal_set_scan_ssid
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void wal_set_scan_ssid(oal_cfg80211_scan_request_stru *request,
    mac_cfg80211_scan_param_stru *scan_param)
{
    osal_s32 loop;
    osal_s32 ssid_num;

    if ((request == OAL_PTR_NULL) || (scan_param == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
            "{wal_set_scan_ssid::scan failed, null ptr, request[%p], scan_param[%p].}",
            (uintptr_t)request, (uintptr_t)scan_param);

        return;
    }

    scan_param->scan_type = OAL_ACTIVE_SCAN; /* active scan */
    scan_param->ssid_num   = 0;

    /* 取内核下发的ssid的个数 */
    ssid_num = request->n_ssids;
    if (ssid_num > WLAN_SCAN_REQ_MAX_SSID) {
        /* 如果用户下发的指定ssid的个数大于驱动支持的最大个数，则取驱动支持的指定ssid的最大个数 */
        ssid_num = WLAN_SCAN_REQ_MAX_SSID;
    }

    /* 将用户下发的ssid信息拷贝到对应的结构体中 */
    if ((ssid_num > 0) && (request->ssids != OAL_PTR_NULL)) {
        scan_param->ssid_num = ssid_num;

        for (loop = 0; loop < ssid_num; loop++) {
            scan_param->ssids[loop].ssid_len = request->ssids[loop].ssid_len;
            if (scan_param->ssids[loop].ssid_len > OAL_IEEE80211_MAX_SSID_LEN) {
                oam_warning_log2(0, OAM_SF_ANY,
                                 "{wal_set_scan_ssid::ssid scan set failed, ssid_len[%d] is exceed, max[%d].}",
                                 scan_param->ssids[loop].ssid_len, OAL_IEEE80211_MAX_SSID_LEN);

                scan_param->ssids[loop].ssid_len = OAL_IEEE80211_MAX_SSID_LEN;
            }

            if (memcpy_s(scan_param->ssids[loop].ssid, sizeof(scan_param->ssids[loop].ssid),
                request->ssids[loop].ssid, scan_param->ssids[loop].ssid_len) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_set_scan_ssid::memcpy_s error}");
            }
        }
    }
}

OAL_STATIC osal_u32 wal_set_scan_ie(oal_cfg80211_scan_request_stru *request,
    mac_cfg80211_scan_param_stru *scan_param, osal_u32 ie_len)
{
    scan_param->ie = (osal_u8 *)(scan_param + 1) + request->n_channels * sizeof(osal_u32) * 2; /* 2g 和 5g */
    scan_param->ie_len = ie_len;
    /* 有可能不携带ie,所以需要判断ie指针有效性 */
    if ((request->ie != OSAL_NULL) && (ie_len != 0)) {
        if (memcpy_s(scan_param->ie, ie_len, request->ie, ie_len) != EOK) { /* ie_len 已经对 request->ie_len 做过处理 */
            oam_error_log1(0, OAM_SF_SCAN, "{wal_set_scan_ie::memcpy request->ie fail, ie_len[%u].}", ie_len);
            return OAL_FAIL;
        }
    } else {
        scan_param->ie_len = 0;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_wait_for_scan_complete_time_out
 功能描述  : 等待扫描完成超时处理函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  wal_wait_for_scan_timeout_fn(void *p_arg)
{
    hmac_vap_stru                  *hmac_vap = (hmac_vap_stru *)p_arg;
    hmac_device_stru               *hmac_device;
    hmac_bss_mgmt_stru             *bss_mgmt;
    hmac_scan_stru                 *scan_mgmt;
    oal_wiphy_stru                 *wiphy;

    oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {wal_wait_for_scan_timeout_fn:: 5 seconds scan timeout proc.}",
                     hmac_vap->vap_id);

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，扫描异常保护中，上报内核扫描状态为扫描完成 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_SCAN)) {
        /* 改变vap状态到SCAN_COMP */
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{wal_wait_for_scan_complete_time_out::hmac_device[%d] is null.}",
                         hmac_vap->device_id);
        return OAL_FAIL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);
    wiphy      = hmac_device->wiphy;

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 对于内核下发的扫描request资源加锁 */
    osal_mutex_lock(&(scan_mgmt->scan_request_mutexlock));

    if (scan_mgmt->request != OAL_PTR_NULL) {
        /* 上报扫描到的所有的bss */
        wal_inform_all_bss_etc(wiphy, bss_mgmt, hmac_vap->vap_id);

        /* 通知 kernel scan 已经结束 */
        oal_cfg80211_scan_done_etc(scan_mgmt->request, 0);

        scan_mgmt->request = OAL_PTR_NULL;
        scan_mgmt->complete = OAL_TRUE;
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_wait_for_scan_timeout_fn::release scan_requset}");

        /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
        oal_smp_mb();
        osal_wait_wakeup(&scan_mgmt->wait_queue);
    }

    /* 通知完内核，释放资源后解锁 */
    osal_mutex_unlock(&(scan_mgmt->scan_request_mutexlock));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_start_timer_for_scan_timeout
 功能描述  : 启动扫描定时器做扫描超时保护处理
 输入参数  : osal_u8   vap_id, 发起扫描的vap id
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void  wal_start_timer_for_scan_timeout(osal_u8   vap_id)
{
    hmac_vap_stru               *hmac_vap = OAL_PTR_NULL;
    hmac_device_stru            *hmac_device = OAL_PTR_NULL;
    osal_u32                   timeout;

    /* 超时时间为5秒 */
    timeout = WAL_MAX_SCAN_TIME_PER_SCAN_REQ;

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "vap_id[%d] {wal_start_timer_for_scan_timeout::hmac_vap is null!}", vap_id);
        return;
    }

    /* 获取mac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "vap_id[%d] {wal_start_timer_for_scan_timeout::hmac_device is null!}", vap_id);
        return;
    }

    common_log_dbg2(0, OAM_SF_SCAN, "vap_id[%d] {wal_start_timer_for_scan_timeout timeout_val[%u]!}", vap_id, timeout);

    /* 启动扫描保护定时器，在指定时间没有上报扫描结果，主动上报扫描完成 */
    frw_create_timer_entry(&(hmac_vap->scan_timeout),
                           wal_wait_for_scan_timeout_fn,
                           timeout,
                           hmac_vap,
                           OAL_FALSE);

    return;
}

/*****************************************************************************
 函 数 名  : wal_cfg80211_start_scan_etc
*****************************************************************************/
STATIC osal_u32  wal_cfg80211_start_scan_etc(oal_net_device_stru *net_dev,
    mac_cfg80211_scan_param_stru *scan_param, osal_u32 all_len)
{
    osal_s32 ret;

    if (scan_param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_cfg80211_start_scan_etc::scan failed, null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 传递的是 指针 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_CFG80211_START_SCAN,
        (osal_u8 *)scan_param, all_len);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] {wal_cfg80211_start_scan_etc::return err code %d, all_len[%u]}",
                         wal_util_get_vap_id(net_dev), ret, all_len);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : wal_scan_work_func_etc
 功能描述  : 解析内核下发扫描命令相关参数，启动扫描
*****************************************************************************/
osal_u32 wal_scan_work_func_etc(hmac_scan_stru *scan_mgmt, oal_net_device_stru *netdev,
    oal_cfg80211_scan_request_stru *request, hmac_vap_stru *hmac_vap)
{
    mac_cfg80211_scan_param_stru *scan_param = OSAL_NULL;
    osal_u32 ret;
    osal_u32 all_len;
    osal_u32 ie_len;

    if (scan_mgmt == OSAL_NULL || netdev == OSAL_NULL || request == OSAL_NULL || hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_scan_work_func_etc::params error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ie_len = (request->ie_len > WLAN_WPS_IE_MAX_SIZE) ? WLAN_WPS_IE_MAX_SIZE : request->ie_len;
    /* 2g 和 5g */
    all_len = (osal_u32)(sizeof(mac_cfg80211_scan_param_stru) + request->n_channels * sizeof(osal_u32) * 2 + ie_len);

    /* 申请 结构体以及信道,IE的内存, 内存布局为 mac_cfg80211_scan_param_stru结构体 + 2g信道信息 + 5g信道信息 + ie */
    scan_param = (mac_cfg80211_scan_param_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, all_len, OSAL_FALSE);
    if (scan_param == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "{wal_scan_work_func_etc::scan failed, alloc memory[%u] failed!}", all_len);
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(scan_param, all_len, 0, all_len);

    /* 解析内核下发的扫描信道列表 */
    wal_set_scan_channel(request, scan_param, hmac_vap);

    /* 解析内核下发的ssid */
    wal_set_scan_ssid(request, scan_param);

    /* 解析内核下发的ie */
    if (wal_set_scan_ie(request, scan_param, ie_len) != OAL_SUCC) {
        oal_mem_free(scan_param, OSAL_FALSE);
        return OAL_FAIL;
    }

    /* P2P WLAN/P2P 特性情况下，根据扫描的ssid 判断是否为p2p device 发起的扫描，
        ssid 为"DIRECT-"则认为是p2p device 发起的扫描 */
    /* 解析下发扫描的device 是否为p2p device(p2p0) */
    scan_param->is_p2p0_scan = OAL_FALSE;

    if (is_p2p_scan_req(request)) {
        scan_param->is_p2p0_scan = OAL_TRUE;
    }

    /* 在事件前防止异步调度完成扫描后,发生同步问题 */
    scan_mgmt->complete = OAL_FALSE;

    /* 启动定时器，进行扫描超时处理 */
    /* 首先启动，post event过程中可能触发调度，导致扫描完成事件先于定时器启动，最终导致超时定时器残留 */
    wal_start_timer_for_scan_timeout(hmac_vap->vap_id);

    /* 抛事件，通知驱动启动扫描 */
    ret = wal_cfg80211_start_scan_etc(netdev, scan_param, all_len);
    if (ret != OAL_SUCC) {
        if (hmac_vap && hmac_vap->scan_timeout.is_registerd) {
            frw_destroy_timer_entry(&hmac_vap->scan_timeout);
        }
        scan_mgmt->complete = OAL_TRUE;
        oam_warning_log1(0, OAM_SF_CFG, "{wal_scan_work_func_etc::wal_cfg80211_start_scan_etc failed, err[%d]!}", ret);
    }

    /* 统一在  外层释放栈内存，避免异常分支的重复释放 */
    oal_mem_free(scan_param, OSAL_FALSE);

    return ret;
}

/*****************************************************************************
 函 数 名  : wal_send_scan_abort_msg_etc
 功能描述  : 终止扫描
 输入参数  : oal_net_device_stru   *net_dev
 输出参数  : 无
*****************************************************************************/
osal_s32 wal_send_scan_abort_msg_etc(oal_net_device_stru   *net_dev)
{
    osal_u32                      pedding_data = 0;       /* 填充数据，不使用，只是为了复用接口 */
    osal_s32                       l_ret;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SCAN_ABORT,
        (osal_u8 *)&pedding_data, OAL_SIZEOF(pedding_data));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "{wal_send_scan_abort_msg_etc::fail to stop scan, error[%d]}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*************** **************************************************************
 函 数 名  : wal_force_scan_complete_etc
 功能描述  : 通知扫描完成
 输入参数  : oal_net_device_stru   *net_dev,
             oal_bool_enum          is_aborted
 输出参数  : 无
*****************************************************************************/
osal_s32 wal_force_scan_complete_etc(oal_net_device_stru *net_dev, oal_bool_enum is_aborted)
{
    hmac_vap_stru           *hmac_vap;
    hmac_device_stru        *hmac_device;
    hmac_scan_stru          *scan_mgmt;

    hmac_vap  = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{wal_force_scan_complete_etc::iftype:[%d]}", net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {wal_force_scan_complete_etc::hmac_dev=null}", hmac_vap->vap_id);
        return -OAL_EINVAL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);

    /* 如果是来自内部的扫描 */
    if (scan_mgmt->request == OAL_PTR_NULL) {
        /* 判断是否存在内部扫描，如果存在，也需要停止 */
        if ((scan_mgmt->is_scanning == OAL_TRUE) &&
            (hmac_vap->vap_id == scan_mgmt->scan_record_mgmt.vap_id)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {wal_force_scan_complete_etc:stop scan}", hmac_vap->vap_id);
            /* 终止扫描 */
            wal_send_scan_abort_msg_etc(net_dev);
        }

        return OAL_SUCC;
    }

    /* 对于内核下发的扫描request资源加锁 */
    osal_mutex_lock(&(scan_mgmt->scan_request_mutexlock));

    /* 如果是上层下发的扫描请求，则通知内核扫描结束，内部扫描不需通知 */
    if ((scan_mgmt->request != OAL_PTR_NULL) && oal_wdev_match(net_dev, scan_mgmt->request)) {
        /* 删除等待扫描超时定时器 */
        if (hmac_vap->scan_timeout.is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(&(hmac_vap->scan_timeout));
        }

        /* 上报内核扫描结果 */
        wal_inform_all_bss_etc(hmac_device->wiphy,
                               &(scan_mgmt->scan_record_mgmt.bss_mgmt), hmac_vap->vap_id);

        /* 通知内核扫描终止 */
        oal_cfg80211_scan_done_etc(scan_mgmt->request, is_aborted);

        scan_mgmt->request = OAL_PTR_NULL;
        scan_mgmt->complete = OAL_TRUE;
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_force_scan_complete_etc::release scan_request}");

        /* 通知完内核，释放资源后解锁 */
        osal_mutex_unlock(&(scan_mgmt->scan_request_mutexlock));
        /* 下发device终止扫描 */
        wal_send_scan_abort_msg_etc(net_dev);

        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d]{wal_force_scan_complete_etc::scan abort}", hmac_vap->vap_id);

        /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
        oal_smp_mb();
        osal_wait_wakeup(&scan_mgmt->wait_queue);
    } else {
        /* 通知完内核，释放资源后解锁 */
        osal_mutex_unlock(&(scan_mgmt->scan_request_mutexlock));
    }

    return OAL_SUCC;
}

/*************** **************************************************************
 函 数 名  : wal_force_scan_complete_for_disconnect_scene
 功能描述  : 通知扫描完成
 输入参数  : oal_net_device_stru   *net_dev,
 输出参数  : 无
*****************************************************************************/
osal_s32 wal_force_scan_complete_for_disconnect_scene(oal_net_device_stru   *net_dev)
{
    hmac_vap_stru           *hmac_vap;
    hmac_device_stru        *hmac_device;
    hmac_scan_stru          *scan_mgmt;

    hmac_vap  = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN,
                         "{wal_force_scan_complete_for_disconnect_scene:: mac_vap of net_dev is deleted!iftype:[%d]}",
                         net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {wal_force_scan_complete_for_disconnect_scene::hmac_device[%d] is null!}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return -OAL_EINVAL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);

    /* 如果是来自内部的扫描 */
    if (scan_mgmt->request == OAL_PTR_NULL) {
        /* 判断是否存在内部扫描，如果存在，也需要停止 */
        if ((scan_mgmt->is_scanning == OAL_TRUE) &&
            (hmac_vap->vap_id == scan_mgmt->scan_record_mgmt.vap_id)) {
            oam_warning_log1(0, OAM_SF_SCAN,
                "vap_id[%d] {wal_force_scan_complete_for_disconnect_scene::may be internal scan, stop scan!}",
                hmac_vap->vap_id);
            /* 终止扫描 */
            wal_send_scan_abort_msg_etc(net_dev);
        }

        return OAL_SUCC;
    }

    /*  去关联的时候如果有上层扫描请求，不在此接口里面上报abort完成状态，等到device侧abort完成上报事件 */
    if ((scan_mgmt->request != OAL_PTR_NULL) && oal_wdev_match(net_dev, scan_mgmt->request)) {
        /* 下发device终止扫描 */
        wal_send_scan_abort_msg_etc(net_dev);
        scan_mgmt->scan_record_mgmt.scan_rsp_status = MAC_SCAN_ABORT_SYNC;
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {wal_force_scan_complete_for_disconnect_scene::vap_id[%d] notify kernel scan abort!}",
            hmac_vap->vap_id,
            hmac_vap->vap_id);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN

/*************** **************************************************************
 函 数 名  : wal_stop_sched_scan_etc
 功能描述  : 通知PNO扫描完成
 输入参数  : oal_net_device_stru   *net_dev,
 输出参数  : 无
*****************************************************************************/
osal_s32 wal_stop_sched_scan_etc(oal_net_device_stru *netdev)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_u32 pedding_data = 0;       /* 填充数据，不使用，只是为了复用接口 */
    osal_s32 ret;

    /* 参数合法性检查 */
    if (netdev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc::netdev is null}");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        /* dfr会删除netdev对应的vap并在恢复时重新创建 */
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc:: hmac_vap is null!}");
        return -OAL_EINVAL;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc:: hmac_device[%d] is null!}",
                       hmac_vap->device_id);
        return -OAL_EINVAL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);

    oam_warning_log2(0, OAM_SF_SCAN, "{wal_stop_sched_scan_etc::sched scan req[0x%x],sched scan complete[%d]}",
                     scan_mgmt->sched_scan_req, scan_mgmt->sched_scan_complete);

    if ((scan_mgmt->sched_scan_req != OAL_PTR_NULL) &&
        (scan_mgmt->sched_scan_complete != OAL_TRUE)) {
        /* 如果正常扫描请求未执行，则上报调度扫描结果 */
        if (OAL_PTR_NULL == scan_mgmt->request) {
            oal_cfg80211_sched_scan_result_etc(hmac_device->wiphy);
        }

        scan_mgmt->sched_scan_req     = OAL_PTR_NULL;
        scan_mgmt->sched_scan_complete = OAL_TRUE;
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_C_CFG_CFG80211_STOP_SCHED_SCAN,
            (osal_u8 *)&pedding_data, OAL_SIZEOF(pedding_data));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_SCAN,
                "{wal_stop_sched_scan_etc::fail to stop pno sched scan, error[%d]}", ret);
        }
    }
    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

