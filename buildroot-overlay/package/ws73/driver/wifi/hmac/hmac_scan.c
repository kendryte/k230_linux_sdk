/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac scan rom
 * Create: 2020-7-8
 */
#include "hmac_scan.h"
#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_mgmt_sta.h"
#include "frw_ext_if.h"
#include "oal_util.h"
#include "common_dft.h"
#include "hmac_chan_meas.h"
#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_mbo.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#include "hmac_roam_if.h"

#include "wlan_msg.h"

#include "hmac_feature_dft.h"
#include "oal_netbuf_data.h"
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oal_net.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_regdomain.h"
#include "mac_resource_ext.h"
#include "mac_device_ext.h"
#include "mac_ie.h"
#include "hmac_main.h"
#include "hmac_fcs.h"
#include "hmac_tx_mgmt.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_device.h"
#include "hmac_mgmt_sta.h"
#include "hmac_alg_notify.h"
#include "hmac_alg_config.h"
#include "hmac_sta_pm.h"
#include "hal_device_fsm.h"
#include "hal_chip.h"
#include "hmac_feature_dft.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_classifier.h"
#include "wlan_msg.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ps.h"
#include "hmac_power.h"
#include "hmac_beacon.h"
#include "hmac_mgmt_bss_comm.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#endif
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "dmac_common_inc_rom.h"
#include "hmac_sr_sta.h"
#include "hmac_feature_main.h"
#include "hmac_dfx.h"
#include "hmac_dnb_sta.h"
#ifdef _PRE_WLAN_FEATURE_DBAC
#include "alg_dbac_hmac.h"
#endif
#include "hmac_feature_interface.h"
#include "frw_util_notifier.h"
#include "hmac_11k.h"
#include "hmac_promisc.h"
#include "common_log_dbg_rom.h"

#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
#include "hmac_sta_channel_scoring.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SCAN_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_scan_rom_cb g_hmac_scan_rom_cb = {OSAL_NULL, OSAL_NULL};

/* sta扫描报文发送次数, 默认1次; 某个信道单次扫描停留时间, 默认20ms; 信道扫描次数, 默认2次; liteos使用定义时的初始值 */
hmac_scan_config_info g_hmac_scan_config_info = {1, WLAN_DEFAULT_ACTIVE_SCAN_TIME, 2, 0};

osal_u8 g_scan_probe_req_all_ie = 1; /* 扫描probe req报文是否发送所有 ie字段 */
osal_u8 g_scan_probe_req_del_wps_ie = 1; /* sta扫描probe req报文是否删除wps ie字段 */
hmac_scan_chan_offset_bandwidth g_chan_offset_bandwidth[4] = { /* 4 中心频率相对于主20偏移情况总数 */
    {6, WLAN_BAND_WIDTH_80PLUSPLUS},
    {2, WLAN_BAND_WIDTH_80MINUSPLUS},
    {-2, WLAN_BAND_WIDTH_80PLUSMINUS},
    {-6, WLAN_BAND_WIDTH_80MINUSMINUS},
};

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
OAL_STATIC osal_u32 hmac_scan_start_pno_sched_scan_timer(const hmac_vap_stru *arg);
#endif
OAL_STATIC osal_u32  hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device);
OAL_STATIC osal_s32 hmac_scan_proc_scanned_bss_etc(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, mac_scanned_result_extend_info_stru *scan_result_ext_info);
OAL_STATIC osal_void  hmac_scan_proc_last_scan_record(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_set_vap_mac_addr_process(const mac_scan_req_stru *scan_params,
    const hal_scan_params_stru *hal_scan_params, hmac_vap_stru *hmac_vap, osal_bool random_mac)
{
    if (osal_unlikely(scan_params == OSAL_NULL || hal_scan_params == OSAL_NULL || hmac_vap == OSAL_NULL ||
        hmac_vap->hal_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_set_vap_mac_addr_process::param null.}");
        return;
    }

    if (random_mac == OSAL_TRUE) {
        /* 设置mib和硬件的macaddr为随机mac addr */
        (osal_void)memcpy_s((osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN,
            (osal_void *)(scan_params->sour_mac_addr), WLAN_MAC_ADDR_LEN);
        hal_vap_set_macaddr(hmac_vap->hal_vap, scan_params->sour_mac_addr, WLAN_MAC_ADDR_LEN);

        hmac_vap->random_mac = OSAL_TRUE;
        oam_warning_log4(0, OAM_SF_SCAN,
            "{hmac_scan_set_vap_mac_addr_process::rand[%02X:%02X:%02X:%02X:XX:XX]}",
            scan_params->sour_mac_addr[0], scan_params->sour_mac_addr[1],
            scan_params->sour_mac_addr[2], scan_params->sour_mac_addr[3]); /* 2,3为mac下标 */
    } else {
        /* 扫描结束，恢复原先mib和硬件寄存器的mac addr */
        (osal_void)memcpy_s((osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN,
            (osal_void *)(hal_scan_params->original_mac_addr), WLAN_MAC_ADDR_LEN);
        hal_vap_set_macaddr(hmac_vap->hal_vap, hal_scan_params->original_mac_addr, WLAN_MAC_ADDR_LEN);

        hmac_vap->random_mac = OSAL_FALSE;
        oam_warning_log4(0, OAM_SF_SCAN,
                         "{hmac_scan_set_vap_mac_addr_process:: resume original mac_addr:%02X:%02X:%02X:%02X:XX:XX.}",
                         hal_scan_params->original_mac_addr[0], hal_scan_params->original_mac_addr[1], /* 1为mac下标 */
                         hal_scan_params->original_mac_addr[2], hal_scan_params->original_mac_addr[3]); /* 2,3为mac下标 */
    }

    /* 将mib中的mac同步到dmac, 避免随机mac扫描时单播probe rsp因不匹配mib中的mac 被过滤 */
    hmac_vap_sync(hmac_vap);
    return;
}

/*****************************************************************************
 功能描述  : 随机mac addr扫描状态下，扫描开始时，更新vap mac addr为随机值，扫描完成后，恢复为默认值
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_set_vap_mac_addr_by_scan_state(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 is_scan_start)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    mac_scan_req_stru *scan_params = OSAL_NULL;
    hal_scan_params_stru *hal_scan_params = OSAL_NULL;
    hal_to_dmac_device_stru *another_hal_device = OSAL_NULL;
    hal_scan_params_stru *sec_hal_scan_params = OSAL_NULL;

    /* 获取扫描参数 */
    scan_params = &(hmac_device->scan_params);

    /* 1. 非随机mac addr扫描，直接返回，无需重设帧过滤寄存器 */
    /* 2. p2p扫描不支持随机mac addr */
    if ((scan_params->is_random_mac_addr_scan == OSAL_FALSE) || (scan_params->is_p2p0_scan == OSAL_TRUE)) {
        oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_set_vap_mac_addr_by_scan_state:random[%u] p2p[%u]",
            scan_params->is_random_mac_addr_scan, scan_params->is_p2p0_scan);
        return;
    }

    /* 获取dmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_params->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_set_vap_mac_addr_by_scan_state:: id[%u].}", scan_params->vap_id);
        return;
    }

    /* 判断当前非P2P场景，进行随机MAC ADDR的设置 */
    if (!is_legacy_vap(hmac_vap)) {
        oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_set_vap_mac_addr_by_scan_state: p2p vap}");
        return;
    }

    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_scan_set_vap_mac_addr_by_scan_state:: hal null.}", hmac_vap->vap_id);
        return;
    }

    another_hal_device = hmac_device_get_another_h2d_dev(hmac_device, hal_device);
    hal_scan_params = &(hal_device->hal_scan_params);
    sec_hal_scan_params = &(another_hal_device->hal_scan_params);

    /* 扫描开始时，重新设置帧过滤寄存器 */
    if (is_scan_start == OSAL_TRUE) {
        /* 保存原先的mac addr, 即便单sta非并发的背景扫描也会多次进来 */
        if (scan_params->is_radom_mac_saved == 0) {
            (osal_void)memcpy_s((osal_void *)(hal_scan_params->original_mac_addr), WLAN_MAC_ADDR_LEN,
                (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);
            scan_params->is_radom_mac_saved = OSAL_TRUE;
            scan_params->radom_mac_saved_to_dev = 0;
        } else if (scan_params->radom_mac_saved_to_dev != 0) {
            (osal_void)memcpy_s((osal_void *)(hal_scan_params->original_mac_addr), WLAN_MAC_ADDR_LEN,
                (osal_void *)(sec_hal_scan_params->original_mac_addr), WLAN_MAC_ADDR_LEN);
        }
        hmac_scan_set_vap_mac_addr_process(scan_params, hal_scan_params, hmac_vap, OSAL_TRUE);
    } else {
        /* 扫描结束，恢复原先mib和硬件寄存器的mac addr */
        hmac_scan_set_vap_mac_addr_process(scan_params, hal_scan_params, hmac_vap, OSAL_FALSE);
    }

    return;
}

/*****************************************************************************
 功能描述  : 上报扫描结果到host侧，不区分02还是51，统一上报
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_report_scanned_bss(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    dmac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_scanned_result_extend_info_stru scan_result_ext_info;
    osal_void *fhook = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;
#endif
    /* 获取帧信息 */
    osal_u16 frame_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);
    if (frame_len <= MAC_80211_FRAME_LEN) {
        oal_netbuf_free(netbuf);
        oam_error_log2(0, OAM_SF_SCAN, "vap[%d] hmac_scan_mgmt_beacon_and_rsp:len %d err", hmac_vap->vap_id, frame_len);
        return OAL_FAIL;
    }

    /* 获取该buffer的控制信息 */
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    /* 清空上报扫描结果的扩展字段信息并进行赋值 */
    (osal_void)memset_s(&scan_result_ext_info, sizeof(mac_scanned_result_extend_info_stru),
        0, sizeof(mac_scanned_result_extend_info_stru));
    scan_result_ext_info.rssi = (osal_s32)rx_ctrl->rx_statistic.rssi_dbm;
    scan_result_ext_info.ant0_rssi = rx_ctrl->rx_statistic.ant0_rssi;
    scan_result_ext_info.ant1_rssi = rx_ctrl->rx_statistic.ant1_rssi;
    scan_result_ext_info.bss_type = hmac_device->scan_params.bss_type;
    scan_result_ext_info.channel = hmac_get_dsss_ie_channel_num(hmac_vap, 1); /* 增加标志位1，表示获取扫描结果 */

    /* 获取TSF TIMER时间戳 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_GET_TSF);
    if (fhook != OSAL_NULL) {
        ((hmac_11k_get_tsf_cb)fhook)(hmac_vap, &scan_result_ext_info, rx_ctrl);
    }

    frame_len -= MAC_80211_FRAME_LEN;
#ifdef _PRE_WLAN_FEATURE_M2S
    scan_result_ext_info.support_max_nss = hmac_m2s_get_bss_max_nss(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    /* 获取帧信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctrl->rx_info));
    frame_body = oal_netbuf_rx_data(netbuf);
    scan_result_ext_info.num_sounding_dim = hmac_m2s_scan_get_num_sounding_dim(netbuf, frame_len);

    /* 只有probe rsp帧中宣称不支持OPMODE，对端才不支持OPMODE，beacon和assoc
     * rsp帧中宣称的OPMODE信息不可信,因此当我们解析probe rsp时我们发的probe req帧中最好要带ext cap字段 */
    if (frame_hdr->frame_control.sub_type == WLAN_PROBE_RSP) {
        scan_result_ext_info.support_opmode = hmac_m2s_get_bss_support_opmode(hmac_vap, frame_body, frame_len);
    }
#endif

    return (osal_u32)hmac_scan_proc_scanned_bss_etc(hmac_vap, netbuf, &scan_result_ext_info);
}

/*****************************************************************************
 功能描述  : 检查收到的beacon/probe response是不是要扫描的bss
*****************************************************************************/
osal_u32 hmac_scan_check_bss_type(const osal_u8 *frame_body, const mac_scan_req_stru *scan_params)
{
    mac_cap_info_stru *cap_info;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    cap_info = (mac_cap_info_stru *)(frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN);

#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
        oam_info_log3(0, OAM_SF_SCAN, "{hmac_scan_check_bss_type::bss_type[%u] ess[%u] ibss[%u].}",
            scan_params->bss_type, cap_info->ess, cap_info->ibss);
    }
#endif

    if ((scan_params->bss_type == WLAN_MIB_DESIRED_BSSTYPE_INFRA) && (cap_info->ess != 1)) {
        return OAL_FAIL;
    }

    if ((scan_params->bss_type == WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT) && (cap_info->ibss != 1)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据index返回扫描时使用hal device
*****************************************************************************/
OAL_STATIC hal_to_dmac_device_stru *hmac_scan_find_hal_device(const hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap, osal_u8 idx)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if ((hmac_device == OSAL_NULL) || (hmac_vap == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_scan_find_hal_device::hmac_device=%p", (uintptr_t)hmac_device);
        return OSAL_NULL;
    }
    /* 优先返回dmac_vap所挂接的hal device */
    if (idx == 0) {
        return hmac_vap->hal_device;
    }

    return hal_device;
}
/*****************************************************************************
 功能描述  : device在扫描状态是否收到包
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_check_rev_frame(const hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    osal_u8 frame_channel_num)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    /* 优先检查dmac_vap 所挂接的hal device */
    hal_device = hmac_scan_find_hal_device(hmac_device, hmac_vap, 0);
    if (hal_device == OSAL_NULL) {
        oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_check_rev_frame::hal_device null.}");
        return;
    }

    if ((hal_device->current_chan_number == frame_channel_num) &&
        (hal_device->hal_scan_params.working_in_home_chan == OSAL_FALSE)) {
        if (hal_device->current_chan_number > MAX_CHANNEL_NUM_FREQ_2G) {
            hal_device->hal_scan_params.scan_ap_num_in_5p0++;
        } else {
            hal_device->hal_scan_params.scan_ap_num_in_2p4++;
        }

        /* 当前已经收到包，证明在此信道扫到了bss，置标志 */
        hal_device->hal_scan_params.scan_curr_chan_find_bss_flag = OAL_TRUE;
    }
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 功能描述  : 在pno扫描的情况下，检查收到的beacon/probe response是不是要扫描的bss，不是则不上报
*****************************************************************************/
osal_u32 hmac_scan_check_bss_in_pno_scan(osal_u8 *frame_body, osal_s32 frame_body_len,
    const mac_pno_scan_stru * const pno_scan_info, osal_s32 rssi)
{
    osal_u8 *ssid = OSAL_NULL;
    osal_s32 loop;
    osal_char ac_ssid[WLAN_SSID_MAX_LEN];
    osal_char *ac_ssid_ptr = ac_ssid;
    osal_u8 ssid_len = 0;

    if (osal_unlikely(pno_scan_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_check_bss_in_pno_scan::param null.}");
        return OAL_FAIL;
    }

    /* 如果该帧的信号小于pno扫描可上报的阈值，则返回失败，此帧不上报 */
    if (rssi < pno_scan_info->rssi_thold) {
        oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_check_bss_in_pno_scan::rssi[%d] not in thold[%d].}",
            rssi, pno_scan_info->rssi_thold);
        return OAL_FAIL;
    }

    (osal_void)memset_s(ac_ssid, sizeof(ac_ssid), 0, sizeof(ac_ssid));

    /* 获取管理帧中的ssid IE信息 */
    ssid = hmac_get_ssid_etc(frame_body, frame_body_len, &ssid_len);
    if ((ssid == OSAL_NULL) || (ssid_len >= sizeof(ac_ssid))) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_check_bss_in_pno_scan::ssid null or ssid_len out of range.}");
        return OAL_FAIL;
    }

    if (memcpy_s(ac_ssid_ptr, sizeof(ac_ssid), ssid, ssid_len) == EOK) {
        ac_ssid[ssid_len] = '\0';
    }

    /* 从pno参数中查找本ssid是否存在，如果存在，则可以上报 */
    for (loop = 0; loop < pno_scan_info->ssid_count; loop++) {
        /* 如果ssid相同，返回成功 */
        if (oal_memcmp(ac_ssid_ptr, pno_scan_info->match_ssid_set[loop].ssid, (ssid_len + 1)) == 0) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_check_bss_in_pno_scan::ssid match success.}");
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

osal_u32 hmac_scan_mgmt_beacon_and_rsp_pno(hmac_device_stru *hmac_device, osal_u8 *frame_body,
    osal_u16 frame_len, dmac_rx_ctl_stru *rx_ctrl)
{
    /* 如果是pno调度扫描，则需要进行rssi和ssid的过滤 */
    if (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_PNO) {
        /* 检测本bss是否可以上报，在pno扫描的场景下 */
        if (hmac_scan_check_bss_in_pno_scan(frame_body, (osal_s32)(frame_len - MAC_80211_FRAME_LEN),
            &(hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params.sched),
            (osal_s32)rx_ctrl->rx_statistic.rssi_dbm) != OAL_SUCC) {
            /* PNO扫描,没有扫描到需要关联的AP,不需要上报报文或扫描结果到host */
            return OAL_FAIL;
        }

        /* 如果是扫描到了第一个匹配的ssid，置扫描到了匹配的ssid标记位为真 */
        if (hmac_device->pno_sched_scan_mgmt->is_found_match_ssid != OSAL_TRUE) {
            hmac_device->pno_sched_scan_mgmt->is_found_match_ssid = OSAL_TRUE;

            /* 停止pno调度扫描定时器 */
            hmac_scan_stop_pno_sched_scan_timer(hmac_device->pno_sched_scan_mgmt);
        }
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC osal_void hmac_scan_mgmt_beacon_and_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 *report_bss, osal_u8 *go_on)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u8 *frame_body = oal_netbuf_rx_data(netbuf);
    osal_u16 frame_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);
    osal_u8 frame_channel_num;
    osal_u32 ret;

    *report_bss = OSAL_FALSE;

    /* 检测扫描到bss的类型 */
    if (hmac_scan_check_bss_type(frame_body, &(hmac_device->scan_params)) != OAL_SUCC) {
        return;
    }

    // 如果是obss扫描，不上报扫描结果，只在从信道检测到了beacon帧或者probe rsp帧，拋事件到host，让20/40共存逻辑处理
    if (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_OBSS) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
        hmac_detect_2040_te_a_b(hmac_vap, frame_body, frame_len, offset, rx_ctrl->rx_info.channel_number);
#endif
        /* OBSS扫描不需要继续上报报文内容或扫描结果到host */
        *go_on = OSAL_FALSE;
        return;
    }

    if (frame_len <= MAC_80211_FRAME_LEN) {
        *go_on = OSAL_FALSE;
        oam_error_log2(0, OAM_SF_SCAN, "vap[%d] hmac_scan_mgmt_beacon_and_rsp:len %d err", hmac_vap->vap_id, frame_len);
        return;
    }

    /* 处于扫描状态，且接收到的bss 信道信息与当前扫描信道相同，才标识当前信道扫描到BSS */
    frame_channel_num = hmac_ie_get_chan_num_etc(frame_body, (frame_len - MAC_80211_FRAME_LEN),
        MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN, rx_ctrl->rx_info.channel_number);

    hmac_scan_check_rev_frame(hmac_device, hmac_vap, frame_channel_num);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    /* 如果是pno调度扫描，则需要进行rssi和ssid的过滤 */
    if (hmac_scan_mgmt_beacon_and_rsp_pno(hmac_device, frame_body, frame_len, rx_ctrl) != OAL_SUCC) {
        /* PNO扫描,没有扫描到需要关联的AP,不需要上报报文或扫描结果到host */
        *go_on = OSAL_FALSE;
        return;
    }
#endif
    if (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_SDP) {
        /* SDP扫描不需要继续上报报文内容或扫描结果到host */
        *go_on = OSAL_FALSE;
        return;
    }
    /* 其它模式扫描，上报扫描到的扫描结果 */
    ret = hmac_scan_report_scanned_bss(hmac_vap, netbuf);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap[%d] {hmac_scan_mgmt_beacon_and_rsp::failed %d.}", hmac_vap->vap_id, ret);
    }
    *report_bss = OSAL_TRUE;
    return;
}

/*****************************************************************************
 功能描述  : device在扫描状态下的接收管理帧处理
*****************************************************************************/
osal_u32 hmac_scan_mgmt_filter(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, oal_bool_enum_uint8 *report_bss,
    osal_u8 *go_on)
{
    /* !!! 注意:dmac_rx_process_mgmt_not_report 会根据pen_report_bss 决定是否需要释放netbuf,根据pen_go_on标志是否需要继续上报 */
    /* 如果pen_report_bss返回为TRUE代表扫描接口已经上报,hmac_rx_filter_mgmt则不会释放netbuf,
       否则根据pen_go_on标志继续上报或释放netbuf
    */
    dmac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;

    /* 获取该buffer的控制信息 */
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    /* 获取帧信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_hdr(&(rx_ctrl->rx_info));
    if ((frame_hdr->frame_control.sub_type == WLAN_BEACON) || (frame_hdr->frame_control.sub_type == WLAN_PROBE_RSP)) {
        hmac_scan_mgmt_beacon_and_rsp(hmac_vap, netbuf, report_bss, go_on);
#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
            oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_mgmt_filter::report_bss[%u] go_on[%u].}", *report_bss, *go_on);
        }
#endif
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 封装probe request帧 MAC header
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_encap_probe_req_frame_header(oal_netbuf_stru *mgmt_buf, const osal_u8 *bssid)
{
    osal_u8 *mac_header = oal_netbuf_header(mgmt_buf);
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_PROBE_REQ);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置地址1，广播地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, BROADCAST_MACADDR);

    /* 设置地址2的MAC地址(p2p扫描为p2p的地址，其它为本机地址，如果随机mac addr扫描开启，则为随机地址) */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, hmac_device->scan_params.sour_mac_addr);

    /* 地址3，广播地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, bssid);
}

OAL_STATIC osal_void hmac_set_ssid(osal_u8 *payload_addr, const osal_char *ssid, osal_u8 *ie_len)
{
    payload_addr[0] = MAC_EID_SSID;
    if (ssid[0] == '\0') { /* 通配SSID */
        payload_addr[1] = 0;
        *ie_len = MAC_IE_HDR_LEN; /* 偏移buffer指向下一个ie */
    } else {
        payload_addr[1] = (osal_u8)strlen(ssid);
        /* 获取载体中的第2位地址 */
        if (memcpy_s(&(payload_addr[2]), payload_addr[1], ssid, payload_addr[1]) != EOK) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_set_ssid::memcpy_s ssid len[%u] fail.}", payload_addr[1]);
            payload_addr[1] = 0;
            *ie_len = MAC_IE_HDR_LEN; /* 偏移buffer指向下一个ie */
            return;
        }
        *ie_len = MAC_IE_HDR_LEN + payload_addr[1]; /* 偏移buffer指向下一个ie */
    }
}

/*****************************************************************************
 功能描述      : sta扫描probe req报文是否删除wps ie字段
*****************************************************************************/
osal_u8 hmac_scan_get_probe_req_del_wps_ie_cfg(osal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    return g_scan_probe_req_del_wps_ie;
#else
    /* 未开启自定义模式，默认1, 删除wps IE */
    return 1;
#endif
}

/*****************************************************************************
 功能描述      : sta扫描probe req报文是否删除wps ie字段
*****************************************************************************/
osal_void hmac_scan_set_probe_req_del_wps_ie_cfg(osal_u8 probe_req_del_wps_ie)
{
    g_scan_probe_req_del_wps_ie = probe_req_del_wps_ie;
    wifi_printf("g_scan_probe_req_del_wps_ie = %d\r\n", g_scan_probe_req_del_wps_ie);
}

/*****************************************************************************
 功能描述      : 扫描probe req报文是否发送所有 ie字段
*****************************************************************************/
osal_u8 hmac_scan_get_probe_req_all_ie_cfg(osal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    return g_scan_probe_req_all_ie;
#else
    /* 未开启自定义模式，默认1, 携带所有IE */
    return 1;
#endif
}

/*****************************************************************************
 功能描述      : 扫描probe req报文是否发送所有 ie字段
*****************************************************************************/
osal_void hmac_scan_set_probe_req_all_ie_cfg(osal_u8 probe_req_all_ie)
{
    g_scan_probe_req_all_ie = probe_req_all_ie;
}

/*****************************************************************************
 功能描述      : 设置ini配置的扫描参数
*****************************************************************************/
osal_void hmac_scan_set_config_info(hmac_scan_config_info *config_info)
{
    if (config_info == OSAL_NULL) {
        return;
    }

    (void)memcpy_s(&g_hmac_scan_config_info, sizeof(g_hmac_scan_config_info), config_info, sizeof(*config_info));
}

osal_u8 hmac_scan_get_scan_para_from_config(osal_u8 type)
{
    osal_u8 value = 0xff;

    if (type >= PARA_TYPE_BUTT) {
        oam_warning_log1(0, 0, "{hmac_scan_get_scan_para_from_config::type %d err.}", type);
        return value;
    }

    if (type == PARA_TYPE_SCAN_PROBE_SEND_TIMES) {
        value = g_hmac_scan_config_info.probe_req_send_times;
    } else if (type == PARA_TYPE_SCAN_TIME) {
        value = g_hmac_scan_config_info.single_scan_time;
#ifdef _PRE_WLAN_FEATURE_P2P
        /* P2P连接时安卓下发扫描会导致P2P掉坑, 此场景为避免ini中扫描时间修改过大导致掉坑太多, 将扫描时间调整为默认20ms */
        if ((mac_device_is_p2p_connected_etc(hmac_res_get_mac_dev_etc(0)) == OAL_SUCC) &&
            (value > WLAN_DEFAULT_ACTIVE_SCAN_TIME)) {
            value = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
        }
#endif
    } else if (type == PARA_TYPE_SCAN_COUNT) {
        value = g_hmac_scan_config_info.scan_count;
    }
    return value;
}

/*****************************************************************************
 功能描述  : 封装probe request帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_scan_encap_probe_req_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *mgmt_buf,
    const osal_u8 *bssid, const osal_char *ssid, osal_u8 is_send_cap_ie)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 *payload_addr = oal_netbuf_data_offset(mgmt_buf, MAC_80211_FRAME_LEN);
    osal_u8 *payload_addr_origin = payload_addr;
    osal_u16 app_ie_len;
    osal_u8 dsss_channel_num, ie_len;
    osal_void *fhook = OSAL_NULL;

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
    hmac_scan_encap_probe_req_frame_header(mgmt_buf, bssid);

    /*************************************************************************/
    /*                       Probe Request Frame - Frame Body                */
    /* --------------------------------------------------------------------- */
    /* |SSID |Supported Rates |Extended supp rates| HT cap|Extended cap      */
    /* --------------------------------------------------------------------- */
    /* |2-34 |   3-10         | 2-257             |  28   | 3-8              */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 设置SSID */
    /***************************************************************************
    ***************************************************************************/
    hmac_set_ssid(payload_addr, ssid, &ie_len);
    payload_addr += ie_len;

    /* 设置支持的速率集 */
    hmac_set_supported_rates_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 获取dsss ie内的channel num */
    dsss_channel_num = hmac_get_dsss_ie_channel_num(hmac_vap, 1); /* 设置dsss参数集 */

    /* 设置dsss参数集 */
    mac_set_dsss_params_etc(hmac_vap, payload_addr, &ie_len, dsss_channel_num);
    payload_addr += ie_len;

    /* 设置extended supported rates信息 */
    hmac_set_exsup_rates_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充扩展IE信息 */
    mac_add_extend_app_ie_etc(hmac_vap, &payload_addr, PROBE_REQ_IE);

    /* PNO扫描,probe request报文只包含信道和速率集信息元素,减少发送报文长度 */
    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_PNO)) {
        return (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);
    }

    /* 填充WPS信息 */
    hmac_add_app_ie_etc((osal_void *)hmac_vap, payload_addr, &app_ie_len, OAL_APP_PROBE_REQ_IE);
    payload_addr += app_ie_len;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_SCAN_ADD_MBO_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_mbo_scan_add_mbo_ie_cb)fhook)(hmac_vap, payload_addr, &ie_len);
        payload_addr += ie_len;
    }

    /* ini中配置为0 并且 业务场景中要求不发送能力IE时 才不携带 */
    if ((hmac_scan_get_probe_req_all_ie_cfg() == 0) && (is_send_cap_ie == OAL_FALSE)) {
        return (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);
    }

    /* 填充HT Capabilities信息 */
    hmac_set_ht_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充Extended Capabilities信息 */
    hmac_set_ext_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;

    /* 填充vht capabilities信息 */
    if ((hmac_vap->channel.band != WLAN_BAND_2G)) {
        hmac_set_vht_capabilities_ie_etc(hmac_vap, payload_addr, &ie_len);
        payload_addr += ie_len;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 填充he capabilities信息 */
    hmac_set_he_capabilities_ie(hmac_vap, payload_addr, &ie_len);
    payload_addr += ie_len;
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DNB_ADD_PRIVATE_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_set_dnb_cap_cb)fhook)(hmac_vap, &payload_addr);
    }

    return (osal_u16)((payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);
}

/*****************************************************************************
 功能描述  : 发送探测请求帧
*****************************************************************************/
osal_u32 hmac_scan_send_probe_req_frame(hmac_vap_stru *hmac_vap, const osal_u8 *bssid, const osal_char *ssid,
    osal_u32 ssid_len, osal_u8 is_send_cap_ie)
{
    oal_netbuf_stru *mgmt_buf;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;
    osal_u16 mgmt_len;
    osal_u8 *mac_header = OSAL_NULL;
    osal_u8 *saddr = OSAL_NULL;
    unref_param(ssid_len);

    if (osal_unlikely(hmac_vap == OSAL_NULL || ssid == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_send_probe_req_frame::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_send_probe_req_frame::alloc netbuf failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装probe request帧 */
    mgmt_len = hmac_scan_encap_probe_req_frame(hmac_vap, mgmt_buf, bssid, ssid, is_send_cap_ie);
    if ((mgmt_len - MAC_80211_FRAME_LEN) > WLAN_SMGMT_NETBUF_SIZE) {
        oam_error_log3(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_send_probe_req_frame::payload_len=[%d] over net_buf_size=[%d].}",
            hmac_vap->vap_id, mgmt_len - MAC_80211_FRAME_LEN, WLAN_SMGMT_NETBUF_SIZE);
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    if (ether_is_multicast(bssid) == 0) {
        /* 发送单播探测帧 */
        mac_header = oal_netbuf_header(mgmt_buf);
        saddr = hmac_vap_get_mac_addr_etc(hmac_vap);
        oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, bssid);
        oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, saddr);
        tx_ctl->tx_user_idx = (osal_u8)hmac_vap->assoc_vap_id;
    } else {
        /* 发动广播探测帧 */
        tx_ctl->tx_user_idx = (osal_u8)hmac_vap->multi_user_idx; /* probe request帧是广播帧 */
        tx_ctl->ismcast = OSAL_TRUE;
    }

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);   /* 扫描发送probe request 帧不入队 */
    if (ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 扫描完成后调用的接口
*****************************************************************************/
osal_u32 hmac_scan_proc_scan_complete_event(hmac_vap_stru *hmac_vap, mac_scan_status_enum_uint8 scan_rsp_status)
{
    mac_scan_rsp_stru scan_rsp_info;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 设置扫描完成时状态，是被拒绝，还是执行成功 */
    if (hmac_device->scan_params.abort_scan_flag == OSAL_TRUE) {
        scan_rsp_info.scan_rsp_status = MAC_SCAN_ABORT;
    } else {
        scan_rsp_info.scan_rsp_status = scan_rsp_status;
    }

    scan_rsp_info.cookie = hmac_device->scan_params.cookie;

    common_log_dbg4(0, OAM_SF_SCAN,
        "{hmac_scan_proc_scan_complete_event::status:%d, vap channel:%d, cookie[%x], scan_mode[%d]}",
        scan_rsp_status, hmac_vap->channel.chan_number,
        scan_rsp_info.cookie, hmac_device->scan_params.scan_mode);
    common_log_dbg1(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_complete_event::abort_flag[%u]}",
        hmac_device->scan_params.abort_scan_flag);
    hmac_scan_proc_scan_comp_event_etc(hmac_vap, &scan_rsp_info);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 扫描参数准备
*****************************************************************************/
OAL_STATIC osal_void hmac_prepare_for_scan(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params)
{
    osal_u8 idx;
    hal_scan_info_stru scan_info = { 0 };

    /* 竞争到扫描权限后，将扫描参数拷贝到mac deivce结构体下，此时拷贝，也是为了防止扫描参数被覆盖情况 */
    (osal_void)memcpy_s(&(hmac_device->scan_params), sizeof(mac_scan_req_stru),
        scan_req_params, sizeof(mac_scan_req_stru));

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P0 扫描时记录P2P listen channel */
    if (scan_req_params->is_p2p0_scan == OSAL_TRUE) {
        hmac_vap->p2p_listen_channel = scan_req_params->p2p0_listen_channel;

        /* 在device中记录当前P2P的监听信道，hmac_vap中保存的vap信息在删除vap后丢失，直接不扫描再次连接会为0 */
        if (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE) {
            hmac_device->p2p_listen_channel = hmac_vap->p2p_listen_channel;
        }
    }
#endif

    /* 记录扫描开始时间,扫描结束后看整体扫描时间 */
    hmac_device->scan_timestamp = (osal_u32)osal_get_time_stamp_ms();

    for (idx = 0; idx < hmac_device->scan_params.channel_nums; idx++) {
        if (hmac_device->scan_params.channel_list[idx].band == WLAN_BAND_2G) {
            scan_info.num_channels_2g++;
        } else if (hmac_device->scan_params.channel_list[idx].band == WLAN_BAND_5G) {
            scan_info.num_channels_5g++;
        } else {
        }
    }

    scan_info.scan_mode = hmac_device->scan_params.scan_mode;
    /* 随机mac相关default值 */
    hmac_device->scan_params.is_radom_mac_saved = 0;
    hmac_device->scan_params.radom_mac_saved_to_dev = (BIT0 | BIT1);

    scan_info.scan_time = hmac_device->scan_params.scan_time;
    scan_info.scan_channel_interval = hmac_device->scan_params.scan_channel_interval;
    scan_info.max_scan_count_per_channel = hmac_device->scan_params.max_scan_count_per_channel;
    scan_info.work_time_on_home_channel = hmac_device->scan_params.work_time_on_home_channel;

    /* hal device 准备扫描channel参数 */
    hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_SCAN_BEGIN,
                            sizeof(hal_scan_info_stru), (osal_u8*)&scan_info);
}

osal_u32 hmac_scan_handle_scan_req_entry_ext(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params, osal_u8 *continue_flag)
{
    osal_u8 idx;
    osal_u8 is_5g_enable;

    unref_param(hmac_device);
    common_log_dbg4(0, OAM_SF_SCAN,
        "hmac_scan_handle_scan_req_entry_ext:channel_num[%d] in [%d]ms scan_mode[0x%x],max_scan_count_per_channel[%d]",
        scan_req_params->channel_nums,
        scan_req_params->scan_time,
        scan_req_params->scan_mode,
        scan_req_params->max_scan_count_per_channel);

    /* 如果设备不支持5g并且扫描到了5g信道则将continue_flag置0，调用中对continue_flag有判断 */
    is_5g_enable = (osal_u8)hmac_device_check_5g_enable(hmac_vap->device_id);
    for (idx = 0; idx < scan_req_params->channel_nums; idx++) {
        if (scan_req_params->channel_list[idx].chan_number > MAX_CHANNEL_NUM_FREQ_2G && is_5g_enable == OAL_FALSE) {
            *continue_flag = 0;
            break;
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 全系统扫描请求执行的总入口
*****************************************************************************/
osal_u32 hmac_scan_handle_scan_req_entry(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params)
{
    osal_u8 curr_scan_state_pre;
    osal_u8 continue_flag = 1;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(hmac_device == OSAL_NULL || hmac_vap == OSAL_NULL || scan_req_params == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN, "scan_req_entry:dev(%p) vap(%p)", (uintptr_t)hmac_device, (uintptr_t)hmac_vap);
        return OAL_FAIL;
    }

    curr_scan_state_pre = hmac_device->curr_scan_state;
    if (hmac_scan_handle_scan_req_entry_ext(hmac_device, hmac_vap, scan_req_params, &continue_flag) != OAL_CONTINUE) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_scan_handle_scan_req_entry:scan_req_entry_cb return fail");
        return OAL_FAIL;
    }

    if (scan_req_params->scan_mode == WLAN_SCAN_MODE_BACKGROUND_ALG_INTF_DET) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_handle_scan_req_entry:cca can't be here!");
        return OAL_FAIL;
    }
    /* 如果处于扫描状态，则直接返回 */
    /* 如果处于常发常收状态，则直接返回 */
    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) || (continue_flag == 0) ||
        ((hmac_vap->al_tx_flag == EXT_SWITCH_ON) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA))) {
        oam_warning_log3(0, OAM_SF_SCAN, "vap[%d] hmac_scan_handle_scan_req_entry:cann't scan, vap[%d], func[0x%02x]",
            hmac_vap->vap_id, hmac_device->scan_params.vap_id, hmac_device->scan_params.scan_func);
        oam_warning_log4(0, OAM_SF_SCAN, "{hmac_scan_handle_scan_req_entry::cur[%d],req[%d],cookie[%x],flag[%d]}",
            hmac_device->scan_params.scan_mode, scan_req_params->scan_mode, scan_req_params->cookie,
            hmac_vap->al_tx_flag);

        /* 如果是上层下发的扫描请求，直接抛扫描完成事件; OBSS扫描则返回结束，等待下一次定时器超时再发起扫描 */
        if (scan_req_params->scan_mode < WLAN_SCAN_MODE_BACKGROUND_OBSS) {
            /* 更新扫描下发的cookie 值 */
            hmac_device->scan_params.cookie = scan_req_params->cookie;

            /* 抛扫描完成事件，扫描请求被拒绝 */
            return hmac_scan_proc_scan_complete_event(hmac_vap, MAC_SCAN_REFUSED);
        }
        return OAL_SUCC;
    }

    /* 设置device当前扫描状态为运行状态 */
    hmac_device->curr_scan_state = MAC_SCAN_STATE_RUNNING;
    if (curr_scan_state_pre != hmac_device->curr_scan_state) {
        /* 扫描状态发生变化，同步 */
        hmac_device_sync(hmac_device);
    }

    /* 入网开始，处于扫描状态，通知BT */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_SET_BTCOEX_WIFI_STATUS);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_hal_set_btcoex_wifi_status_cb)fhook)(hmac_vap, OSAL_TRUE);
    }

    /* 初始化扫描信道索引 */
    hmac_prepare_for_scan(hmac_device, hmac_vap, scan_req_params);
    /* 同步扫描参数到dmac */
    hmac_scan_param_sync(scan_req_params);
    hmac_scan_switch_channel_off(hmac_device);

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    hmac_alg_anti_intf_switch(hmac_vap->hal_device, OSAL_FALSE);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
osal_void hmac_scan_init_channel(mac_scan_req_stru *scan_params)
{
    osal_u8 chan_idx;
    osal_u8 chan_num_2g = 0;
    osal_u8 chan_num_5g = 0;
    osal_u8 chan_number;

    if (osal_unlikely(scan_params == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_init_channel::param null.}");
        return;
    }

#ifdef _PRE_WLAN_SUPPORT_5G
    /* 5G初始扫描信道, 全信道扫描 */
    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
        /* 判断信道是不是在管制域内 */
        if (hmac_is_channel_idx_valid_etc(WLAN_BAND_5G, chan_idx) == OAL_SUCC) {
            hmac_get_channel_num_from_idx_etc(WLAN_BAND_5G, chan_idx, &chan_number);

            scan_params->channel_list[chan_num_5g].chan_number = chan_number;
            scan_params->channel_list[chan_num_5g].band = WLAN_BAND_5G;
            scan_params->channel_list[chan_num_5g].chan_idx = chan_idx;
            scan_params->channel_nums++;
            chan_num_5g++;
        }
    }
#endif
    /* 2G初始扫描信道, 全信道扫描 */
    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
        /* 判断信道是不是在管制域内 */
        if (hmac_is_channel_idx_valid_etc(WLAN_BAND_2G, chan_idx) == OAL_SUCC) {
            hmac_get_channel_num_from_idx_etc(WLAN_BAND_2G, chan_idx, &chan_number);

            scan_params->channel_list[chan_num_2g + chan_num_5g].chan_number = chan_number;
            scan_params->channel_list[chan_num_2g + chan_num_5g].band = WLAN_BAND_2G;
            scan_params->channel_list[chan_num_2g + chan_num_5g].chan_idx = chan_idx;
            scan_params->channel_nums++;
            chan_num_2g++;
        }
    }

    return;
}

/*****************************************************************************
 功能描述  : 准备PNO调度扫描参数
*****************************************************************************/
osal_void hmac_scan_prepare_pno_scan_params(mac_scan_req_stru *scan_params, const hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 扫描请求参数清零 */
    (osal_void)memset_s(scan_params, sizeof(mac_scan_req_stru), 0, sizeof(mac_scan_req_stru));

    /* 设置发起扫描的vap id */
    scan_params->vap_id = hmac_vap->vap_id;
    scan_params->probe_delay = 0;

    /* 设置扫描用的pro req的src mac地址 */
    if (hmac_device->pno_sched_scan_mgmt != OSAL_NULL) {
        mac_pno_scan_param *pno_sched_params = &hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params;
        (osal_void)memcpy_s((osal_void *)(scan_params->sour_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(pno_sched_params->sched.sour_mac_addr), WLAN_MAC_ADDR_LEN);
        scan_params->is_random_mac_addr_scan = pno_sched_params->sched.is_random_mac_addr_scan;

        /* 设置初始扫描请求的参数 */
        scan_params->bss_type = pno_sched_params->ctrl.bss_type;
        scan_params->scan_type = pno_sched_params->ctrl.scan_type;
        scan_params->scan_mode = pno_sched_params->ctrl.scan_mode;
        scan_params->scan_time = pno_sched_params->ctrl.scan_time;
        scan_params->scan_func = pno_sched_params->ctrl.scan_func;
        scan_params->max_scan_count_per_channel = pno_sched_params->ctrl.max_scan_count_per_channel;
        scan_params->max_send_probe_req_count_per_channel = pno_sched_params->ctrl.max_send_probe_req_count_per_channel;
    }

    oam_warning_log4(0, OAM_SF_ANTI_INTF,
        "{hmac_scan_prepare_pno_scan_params::bss_type(%d), scan_type(%d) scan_mode(%d) scan_time(%d)!}",
        scan_params->bss_type, scan_params->scan_type, scan_params->scan_mode, scan_params->scan_time);
    oam_warning_log3(0, OAM_SF_ANTI_INTF,
        "{hmac_scan_prepare_pno_scan_params::scan_func(%d), scan_count(%d) send_count(%d)!}",
        scan_params->scan_func, scan_params->max_scan_count_per_channel,
        scan_params->max_send_probe_req_count_per_channel);

    /* 设置扫描请求的ssid信息 */
    scan_params->mac_ssid_set[0].ssid[0] = '\0'; /* 通配ssid */
    scan_params->ssid_num = 1;

    /* 设置扫描请求只指定1个bssid，为广播地址 */
    (osal_void)memcpy_s((osal_void *)(scan_params->bssid[0]), WLAN_MAC_ADDR_LEN,
        (osal_void *)(BROADCAST_MACADDR), WLAN_MAC_ADDR_LEN);
    scan_params->bssid_num = 1;

    hmac_scan_init_channel(scan_params);
    return;
}

/*****************************************************************************
 功能描述  : pno rtc调度扫描定时器超时，重新发起pno扫描
*****************************************************************************/
#if (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
osal_void hmac_scan_pno_scan_timeout_fn(osal_void)
#else
osal_void hmac_scan_pno_scan_timeout_fn(osal_u32 data)
#endif
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_scan_req_stru scan_req_params = {0};
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    unref_param(data);
#endif

    if (osal_unlikely(hmac_device->pno_sched_scan_mgmt == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_pno_scan_timeout_fn::pno_sched_scan_mgmt null.}");
        return;
    }

    hmac_vap = (hmac_vap_stru *)hmac_device->pno_sched_scan_mgmt->hmac_vap;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_pno_scan_timeout_fn::hmac_vap null.}");
        return;
    }

    /* 初始化设置为: 未扫描到匹配的bss */
    hmac_device->pno_sched_scan_mgmt->is_found_match_ssid = OSAL_FALSE;

    /* 准备PNO扫描参数，准备发起扫描 */
    hmac_scan_prepare_pno_scan_params(&scan_req_params, hmac_vap);

    /* 重新发起扫描 */
    if (hmac_scan_handle_scan_req_entry(hmac_device, hmac_vap, &scan_req_params) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_scan_pno_scan_timeout_fn::hmac_scan_handle_scan_req_entry err!}");
    }

    /* 重新启动PNO调度扫描定时器 */
    if (hmac_scan_start_pno_sched_scan_timer(hmac_vap) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_scan_pno_scan_timeout_fn::hmac_scan_start_pno_sched_scan_timer err!}");
    }
    return;
}

OAL_STATIC osal_u32 hmac_scan_pno_mgmt_etc(mac_pno_sched_scan_mgmt_stru *pno_mgmt)
{
    if (pno_mgmt == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_pno_mgmt_etc:: pno_mgmt is NULL}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval < MSEC_TO_SEC) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_pno_mgmt_etc:: pno scan interval[%d] time too short.}",
            hmac_vap->vap_id, pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval);
        return OAL_FAIL;
    }

    oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_pno_mgmt_etc::pno stop.}");
    hmac_scan_stop_pno_sched_scan_timer(pno_mgmt);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 启动PNO调度扫描定时器，使得device进入深睡后，也能够唤醒
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_start_pno_sched_scan_timer(const hmac_vap_stru *arg)
{
    const hmac_vap_stru *hmac_vap = arg;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_pno_sched_scan_mgmt_stru *pno_mgmt = OSAL_NULL;
    osal_u32 ret;
    osal_u32 freq = 0;
    frw_msg msg_info = {0};

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_start_pno_sched_scan_timer::param null.}");
        return OAL_FAIL;
    }

    /* 获取pno管理结构体 */
    pno_mgmt = hmac_device->pno_sched_scan_mgmt;
    ret = hmac_scan_pno_mgmt_etc(pno_mgmt);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

#if (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    frw_msg_init(OSAL_NULL, 0, (osal_u8 *)&freq, sizeof(osal_u32), &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_PM_GET_LOWSPEED_CLK_FREQ, &msg_info, OSAL_TRUE);
    if ((ret != OAL_SUCC) || (freq == 0)) {
        /* 获取频率失败，使用32000 (32K)固定频率 */
        freq = 32000;
    }

    oam_warning_log3(0, 0, "vap_id[%d] {hmac_scan_start_pno_sched_scan_timer::freq[%u] ret[%d]}",
        hmac_vap->vap_id, freq, ret);
#else
    ret = osal_adapt_timer_init(&pno_mgmt->pno_sched_scan_timer, hmac_scan_pno_scan_timeout_fn, 0,
        pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_start_pno_sched_scan_timer::timer_init error[0x%x].}", ret);
        return OAL_FAIL;
    }
    ret = osal_adapt_timer_mod(&pno_mgmt->pno_sched_scan_timer,
        pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval);
#endif
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_start_pno_sched_scan_timer::timer_start error[0x%x].}", ret);
        return OAL_FAIL;
    }

    pno_mgmt->hmac_vap = (osal_void *)hmac_vap;
    oam_warning_log3(0, OAM_SF_SCAN,
        "vap_id[%d] {hmac_scan_start_pno_sched_scan_timer:: start pno timer succ, timeout[%u]ms delay[%u].}",
        hmac_vap->vap_id, pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval,
        pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval);

    /* 当前PNO扫描定时器上层初始化是60s(PNO_SCHED_SCAN_INTERVAL),定时器到期后每次再延长60秒 */
    pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval += (60 * MSEC_TO_SEC);
    if (pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval > (300 * MSEC_TO_SEC)) { /* 最长不超过300s */
        pno_mgmt->pno_sched_scan_params.sched.pno_scan_interval = (300 * MSEC_TO_SEC);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 关闭PNO调度扫描定时器
*****************************************************************************/
osal_void hmac_scan_stop_pno_sched_scan_timer(mac_pno_sched_scan_mgmt_stru *pno_mgmt)
{
    if (osal_unlikely(pno_mgmt == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_stop_pno_sched_scan_timer::pno_mgmt null.}");
        return;
    }

    pno_mgmt->hmac_vap = OSAL_NULL;
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (pno_mgmt->pno_sched_scan_timer.timer != OSAL_NULL) {
        osal_timer_stop(&pno_mgmt->pno_sched_scan_timer);
        osal_adapt_timer_destroy(&pno_mgmt->pno_sched_scan_timer);
    }
    /* reset to initial state */
    pno_mgmt->pno_sched_scan_timer.timer = OSAL_NULL;
    pno_mgmt->pno_sched_scan_timer_id = 0;
#endif
}

/*****************************************************************************
 功能描述  : 处理上层下发的PNO调度扫描请求事件，调用扫描入口函数进行扫描
*****************************************************************************/
osal_s32 hmac_scan_proc_sched_scan_req_event(hmac_vap_stru *hmac_vap, mac_pno_scan_param *param_point)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_scan_req_stru scan_req_params = {0};
    mac_pno_sched_scan_mgmt_stru *pno_sched_scan_mgmt = OSAL_NULL;

    /* 参数合法性检查 */
    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(param_point == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_sched_scan_req_event::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_device->pno_sched_scan_mgmt != OSAL_NULL) {
        oal_mem_free(hmac_device->pno_sched_scan_mgmt, OAL_TRUE);
        hmac_device->pno_sched_scan_mgmt = OSAL_NULL;
    }
    /* 申请用于管理PNO调度扫描的内存，动态申请，节省内存(564K,只有一片) */
    pno_sched_scan_mgmt = (mac_pno_sched_scan_mgmt_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_pno_sched_scan_mgmt_stru), OAL_TRUE);
    if (osal_unlikely(pno_sched_scan_mgmt == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_SCAN, "{get pno sched scan mem size=%d err}", sizeof(mac_pno_sched_scan_mgmt_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(pno_sched_scan_mgmt, sizeof(mac_pno_sched_scan_mgmt_stru), 0,
                        sizeof(mac_pno_sched_scan_mgmt_stru));

    hmac_device->pno_sched_scan_mgmt = pno_sched_scan_mgmt;
    /* 将PNO调度扫描请求参数直接拷贝到device结构体下，避免netbuf传来传去 */
    (osal_void)memcpy_s(&(hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params), sizeof(mac_pno_scan_param),
        param_point, sizeof(mac_pno_scan_param));

    /* 打印PNO调度扫描参数 */
    oam_warning_log4(0, OAM_SF_SCAN,
        "{pno scan params ssid_count[%d],rssi_thold[%d],interval[%d],repeat_times[%d].}",
        hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params.sched.ssid_count,
        hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params.sched.rssi_thold,
        hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params.sched.pno_scan_interval,
        hmac_device->pno_sched_scan_mgmt->pno_sched_scan_params.sched.pno_scan_repeat);

    /* 准备pno扫描请求参数 */
    hmac_scan_prepare_pno_scan_params(&scan_req_params, hmac_vap);

    /* 设置PNO调度扫描的次数为0 */
    hmac_device->pno_sched_scan_mgmt->curr_pno_sched_scan_times = 0;
    hmac_device->pno_sched_scan_mgmt->is_found_match_ssid = OSAL_FALSE;

    /* 调用扫描入口，执行扫描 */
    if (hmac_scan_handle_scan_req_entry(hmac_device, hmac_vap, &scan_req_params) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_scan_proc_sched_scan_req_event::hmac_scan_handle_scan_req_entry err!}");
    }

    /* 启动pno调度扫描的rtc时钟定时器，可唤醒深睡的device */
    if (hmac_scan_start_pno_sched_scan_timer(hmac_vap) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_scan_proc_sched_scan_req_event::hmac_scan_start_pno_sched_scan_timer err!}");
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : PNO指定SSID扫描,最多可指定16个ssid
*****************************************************************************/
osal_void hmac_pno_scan_send_probe_with_ssid(const hmac_device_stru * const hmac_device,
    const hal_to_dmac_device_stru * const hal_device, osal_u8 band)
{
    mac_pno_sched_scan_mgmt_stru *pno_sched_scan_mgmt = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 band_tmp;
    osal_u8 loop;
    osal_u32 ret;

    if (hmac_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_pno_scan_send_probe_with_ssid::hmac_device is null.}");
        return;
    }

    pno_sched_scan_mgmt = hmac_device->pno_sched_scan_mgmt;
    if (pno_sched_scan_mgmt == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_pno_scan_send_probe_with_ssid::pno_sched_scan_mgmt is null.}");
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_pno_scan_send_probe_with_ssid::hmac_vap null.}");
        return;
    }

    /* 如果st_scan_params.vap_id = 0，可能内存被踩，要直接返回错误 */
    if (hmac_vap->mib_info == OSAL_NULL) {
        oam_error_log4(0, OAM_SF_SCAN,
            "{hmac_pno_scan_send_probe_with_ssid:: vap is null,vap_id[%d], cb[%p], scan_func[%d], scan_count[%d].}",
            hmac_device->scan_params.vap_id, (uintptr_t)hmac_device->scan_params.fn_cb,
            hmac_device->scan_params.scan_func, hal_device->hal_scan_params.curr_channel_scan_count);
        return;
    }

    /* 发探测请求时，需要临时更新vap的band信息，防止5G发11b */
    band_tmp = hmac_vap->channel.band;

    hmac_vap->channel.band = band;

    for (loop = 0; loop < pno_sched_scan_mgmt->pno_sched_scan_params.sched.ssid_count; loop++) {
        if (pno_sched_scan_mgmt->pno_sched_scan_params.sched.match_ssid_set[loop].scan_ssid == OSAL_TRUE) {
            /* 如果是隐藏SSID,则指定SSID扫描 */
            ret = hmac_scan_send_probe_req_frame(hmac_vap, BROADCAST_MACADDR,
                (osal_char *)pno_sched_scan_mgmt->pno_sched_scan_params.sched.match_ssid_set[loop].ssid,
                WLAN_SSID_MAX_LEN, OAL_FALSE);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_SCAN,
                    "{hmac_pno_scan_send_probe_with_ssid::hmac_scan_send_probe_req_frame failed[%u].}", ret);
            }
        }
    }
    hmac_vap->channel.band = band_tmp;
    return;
}

osal_void hmac_scan_proc_sched_scan_fill_scan_param(hmac_vap_stru *hmac_vap, mac_pno_scan_param *scan_param,
    mac_pno_scan_stru *pno_scan_params)
{
    (osal_void)memset_s(scan_param, sizeof(*scan_param), 0, sizeof(*scan_param));
    (osal_void)memcpy_s(&scan_param->sched, sizeof(scan_param->sched), pno_scan_params, sizeof(*pno_scan_params));
    scan_param->ctrl.max_scan_count_per_channel =
        hmac_vap->max_scan_count_per_channel ? hmac_vap->max_scan_count_per_channel : 1;
    /* PNO指定SSID扫描,扫描时间为原来的2倍 */
    scan_param->ctrl.scan_time =
        hmac_vap->scan_time ? hmac_vap->scan_time : (WLAN_DEFAULT_ACTIVE_SCAN_TIME * 2);  /* 2倍时间 */
    if (hmac_vap->work_time_on_home_channel) {
        scan_param->ctrl.work_time_on_home_channel = hmac_vap->work_time_on_home_channel;
    }
    scan_param->ctrl.bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_param->ctrl.scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_param->ctrl.scan_mode = WLAN_SCAN_MODE_BACKGROUND_PNO;
    scan_param->ctrl.scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
    scan_param->ctrl.max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;

    oam_warning_log3(0, OAM_SF_SCAN, "{hmac_scan_proc_sched_scan_req_event_etc::count:%d time:%d on_home:%d.}",
                     scan_param->ctrl.max_scan_count_per_channel, scan_param->ctrl.scan_time,
                     scan_param->ctrl.work_time_on_home_channel);
}

/*****************************************************************************
 功能描述  : 处理pno调度扫描请求的入口
*****************************************************************************/
osal_u32  hmac_scan_proc_sched_scan_req_event_etc(hmac_vap_stru *hmac_vap, osal_void *p_params)
{
    hmac_device_stru           *hmac_device;
    hmac_scan_record_stru      *scan_record;
    mac_pno_scan_stru          *pno_scan_params;
    mac_pno_scan_param          scan_param;
    osal_u32                  ul_ret;

    /* 参数合法性检查 */
    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (p_params == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event_etc::param null, %p %p.}", hmac_vap, p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pno_scan_params = (mac_pno_scan_stru *)p_params;

    /* 判断PNO调度扫描下发的过滤的ssid个数小于等于0 */
    if (pno_scan_params->ssid_count <= 0) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_sched_scan_req_event_etc::ssid_count <=0.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_proc_sched_scan_req_event_etc::hmac_device[%d] null.}",
            hmac_vap->vap_id,
            hmac_vap->device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ul_ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, (const hmac_device_stru *)hmac_device);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_proc_sched_scan_req_event_etc::Because of error[%d], can't dispatch scan req.}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 清空上一次的扫描结果 */
    hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->scan_mgmt.scan_record_mgmt);
    scan_record->chip_id   = hmac_device->chip_id;
    scan_record->device_id = hmac_device->device_id;
    scan_record->vap_id    = hmac_vap->vap_id;
    scan_record->p_fn_cb      = pno_scan_params->fn_cb;

    hmac_scan_proc_sched_scan_fill_scan_param(hmac_vap, &scan_param, pno_scan_params);

    ul_ret = hmac_scan_proc_sched_scan_req_event(hmac_vap, &scan_param);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 处理上层下发的扫描请求事件，调用扫描入口函数进行扫描
*****************************************************************************/
OAL_STATIC osal_s32 hmac_scan_proc_scan_req_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_scan_req_stru *h2d_scan_req_params = OSAL_NULL;

    if ((osal_bool)osal_unlikely(hmac_vap == OSAL_NULL) || (osal_bool)osal_unlikely(msg == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    common_log_dbg2(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event::rx_flag(%d) tx_flag(%d).}",
        hmac_vap->al_rx_flag, hmac_vap->al_tx_flag);

    if ((hmac_vap->al_rx_flag == OSAL_TRUE) || (hmac_vap->al_tx_flag == OSAL_TRUE)) {
        return OAL_SUCC;
    }

    /* 获取到扫描请求参数 */
    h2d_scan_req_params = (mac_scan_req_stru *)(msg->data);

    /* 保存随机mac扫描开关，用于dmac发起的随机扫描 */
    hmac_device->is_random_mac_addr_scan = h2d_scan_req_params->is_random_mac_addr_scan;

    /* host侧发起的扫描请求的处理 */
    return (osal_s32)hmac_scan_handle_scan_req_entry(hmac_device, hmac_vap, h2d_scan_req_params);
}

/*****************************************************************************
 功能描述  : 切信道时，通知算法
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_switch_channel_notify_alg(hal_to_dmac_device_stru *scan_hal_device,
    hmac_vap_stru *hmac_vap, const mac_channel_stru *channel)
{
    mac_channel_stru channel_tmp;
    hal_to_dmac_device_stru *original_hal_device = OSAL_NULL;

    /* 参数合法性检查 */
    if ((hmac_vap == OSAL_NULL) || (channel == OSAL_NULL) || (scan_hal_device == OSAL_NULL)) {
        oam_error_log3(0, OAM_SF_SCAN,
            "{hmac_scan_switch_channel_notify_alg::hmac_vap[%p],channel[%p].scan_hal_device[%p]}",
            (uintptr_t)hmac_vap, (uintptr_t)channel, (uintptr_t)scan_hal_device);
        return;
    }

    /* 记录当前vap下的信道信息 */
    channel_tmp = hmac_vap->channel;

    /* 不同的hal device扫描,通知算法不同的扫描hal device */
    original_hal_device = hmac_vap->hal_device;
    hmac_vap->hal_device = scan_hal_device;

    /* 记录要切换到信道的频段，切换信道 */
    hmac_vap->channel.band = channel->band;
    hmac_vap->channel.chan_number = channel->chan_number;
    hmac_vap->channel.chan_idx = channel->chan_idx;
    hmac_vap->channel.en_bandwidth = channel->en_bandwidth;

    /* 刷新发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_REFRESH);

    /* 通知算法 */
    hmac_alg_cfg_channel_notify(hmac_vap, CH_BW_CHG_TYPE_SCAN);

    /* 通知算法后，恢复vap原有信道信息 */
    hmac_vap->channel = channel_tmp;
    hmac_vap->hal_device = original_hal_device;

    return;
}

/*****************************************************************************
 功能描述  : 单VAP发保护帧的方式切离信道
*****************************************************************************/
osal_void hmac_switch_channel_off(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_channel_stru * const dst_chl, osal_u16 protect_time)
{
    mac_fcs_cfg_stru fcs_cfg;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    unref_param(hmac_device);
    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "{hmac_switch_channel_off::vap id [%d],hal_device null}", hmac_vap->vap_id);
        return;
    }

    /* 记录切离的信道，供扫描完后切回 */
    hal_device->hal_scan_params.home_channel = hmac_vap->channel;

    (osal_void)memset_s(&fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));
    fcs_cfg.src_chl = hmac_vap->channel;
    fcs_cfg.dst_chl = *dst_chl;
    fcs_cfg.hal_device = hal_device;
    fcs_cfg.vap_id = hmac_vap->vap_id;

    hmac_fcs_prepare_one_packet_cfg(hmac_vap, &(fcs_cfg.one_packet_cfg), protect_time);

    /* 调用FCS切信道接口 保存当前硬件队列的帧到自己的虚假队列,后续DBAC不需要切换队列 */
    hmac_fcs_start(&fcs_cfg);

    common_log_dbg3(0, OAM_SF_SCAN,
        "vap_id[%d] {hmac_switch_channel_off::switch src channel[%d] to dst channel[%d].}", hmac_vap->vap_id,
        hmac_vap->channel.chan_number, fcs_cfg.dst_chl.chan_number);

    return;
}
#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 功能描述  : DBAC模式下扫描切离信道
*****************************************************************************/
osal_void hmac_dbac_switch_channel_off(hmac_device_stru *hmac_device, hmac_vap_stru *mac_vap1,
    hmac_vap_stru *mac_vap2, const mac_channel_stru * const dst, osal_u16 protect_time)
{
    mac_fcs_cfg_stru fcs_cfg;
    hal_to_dmac_device_stru *hal_device = ((hmac_vap_stru *)mac_vap1)->hal_device;
    hal_to_dmac_device_stru *hal_device2 = ((hmac_vap_stru *)mac_vap2)->hal_device;
    hmac_vap_stru *current_chan_vap = NULL;

    if ((hal_device == OSAL_NULL) || (hal_device2 == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_DBAC, "{hmac_dbac_switch_channel_off::hal_device=0x%x hal_device2=0x%x}",
                       (uintptr_t)hal_device, (uintptr_t)hal_device2);
        return;
    }
    if (hal_device != hal_device2) {
        oam_error_log0(0, OAM_SF_DBAC, "{hmac_dbac_switch_channel_off::diff hal_device}");
        return;
    }

    if (mac_is_dbac_running(hmac_device) == OSAL_TRUE) {
        if (alg_dbac_hmac_is_vap_stop(mac_vap1->hal_vap->vap_id) == OSAL_FALSE) {
            current_chan_vap = mac_vap1;
        } else {
            current_chan_vap = mac_vap2;
        }
    } else {
        if (hal_device->current_chan_number == mac_vap1->channel.chan_number) {
            current_chan_vap = mac_vap1;
        } else {
            current_chan_vap = mac_vap2;
        }
    }

    /* 暂停DBAC切信道 */
    hmac_alg_dbac_pause(mac_vap1);

    (osal_void)memset_s(&fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));
    fcs_cfg.dst_chl = *dst;
    fcs_cfg.src_chl = current_chan_vap->channel;
    fcs_cfg.hal_device = hal_device;
    fcs_cfg.vap_id = current_chan_vap->vap_id;

    hmac_fcs_prepare_one_packet_cfg(current_chan_vap, &(fcs_cfg.one_packet_cfg), protect_time);

    oam_warning_log2(0, OAM_SF_DBAC,
        "hmac_dbac_switch_channel_off::switch chan off when dbac running. curr chan num:%d, fake_q_vap_id:%d",
        current_chan_vap->channel.chan_number, current_chan_vap->vap_id);

    if (hal_device->current_chan_number != current_chan_vap->channel.chan_number) {
        oam_warning_log2(0, OAM_SF_DBAC,
            "hmac_dbac_switch_channel_off::switch chan off when dbac running. hal chan num:%d, curr vap chan num:%d.",
            hal_device->current_chan_number, current_chan_vap->channel.chan_number);

        fcs_cfg.one_packet_cfg.protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

    hmac_fcs_start(&fcs_cfg);
}
/*****************************************************************************
 功能描述  : DBAC模式下扫描切离信道
*****************************************************************************/
osal_void hmac_scan_dbac_switch_channel_off(hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap, hmac_vap_stru *mac_vap2)
{
    osal_u8 scan_chan_idx;
    osal_u16 protect_time;
    hal_to_dmac_device_stru *hal_device;

    if (osal_unlikely(hmac_device == OSAL_NULL || hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_dbac_switch_channel_off::param null.}");
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_dbac_switch_channel_off::hmac_vap_stru null.}");
        return;
    }
    scan_chan_idx = get_hal_dev_current_scan_idx(hal_device);
    protect_time = hmac_device->scan_params.scan_time;

    hmac_dbac_switch_channel_off(hmac_device, hmac_vap, mac_vap2,
                                 &(hmac_device->scan_params.channel_list[scan_chan_idx]), protect_time);
}

#endif

/*****************************************************************************
 功能描述  : 比较两个hal device以及channel
*****************************************************************************/
OAL_STATIC osal_u32 hmac_check_hal_device(hal_to_dmac_device_stru *hal_device,
    const hal_to_dmac_device_stru *hal_device2, hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2)
{
    if (hal_device != hal_device2) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_the_same_channel_switch_channel_off::diff hal_device}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap1->channel.chan_number != mac_vap2->channel.chan_number) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "hmac_the_same_channel_switch_channel_off::vap1 channel num[%d] != vap2 channel num[%d].",
                       mac_vap1->channel.chan_number, mac_vap2->channel.chan_number);
        return OAL_FAIL;
    }

    /* 记录切离时最大带宽的信道，供同信道共存扫描完后切回 */
    if (mac_vap1->channel.en_bandwidth >= mac_vap2->channel.en_bandwidth) {
        hal_device->hal_scan_params.home_channel = mac_vap1->channel;
    } else {
        hal_device->hal_scan_params.home_channel = mac_vap2->channel;
    }

    /* 暂停两个VAP的发送 */
    hmac_vap_pause_tx(mac_vap1);
    hmac_vap_pause_tx(mac_vap2);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 扫描两up vap同信道切离工作信道
*****************************************************************************/
OAL_STATIC osal_u32 hmac_the_same_channel_switch_channel_off(hmac_device_stru *hmac_device,
    hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2, const mac_channel_stru * const dst_chl, osal_u16 protect_time)
{
    mac_fcs_cfg_stru fcs_cfg;
    hmac_vap_stru *vap_sta = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device2 = ((hmac_vap_stru *)mac_vap2)->hal_device;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = ((hmac_vap_stru *)mac_vap1)->hal_device;
    unref_param(hmac_device);
    if (hal_device == OSAL_NULL || hal_device2 == OSAL_NULL) {
        oam_error_log4(0, OAM_SF_SCAN,
            "hmac_the_same_channel_switch_channel_off:device1[%p] vap1[%d] device2[%p] vap2[%d],HAL_DEVICE null",
            (uintptr_t)hal_device, mac_vap1->vap_id, (uintptr_t)hal_device2, mac_vap2->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_check_hal_device(hal_device, hal_device2, mac_vap1, mac_vap2);
    if (ret != OAL_SUCC) {
        return ret;
    }

    (osal_void)memset_s(&fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));
    fcs_cfg.dst_chl = *dst_chl;
    fcs_cfg.hal_device = hal_device;

    oam_warning_log3(0, OAM_SF_SCAN,
        "dmac_the_same_channel_switch_channel::curr hal chan[%d], dst channel[%d], fakeq vap id[%d]}",
        hal_device->current_chan_number, fcs_cfg.dst_chl.chan_number, mac_vap1->vap_id);

    /* 同频双STA模式，需要起两次one packet */
    if (mac_vap1->vap_mode == WLAN_VAP_MODE_BSS_STA && mac_vap2->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 准备VAP1的fcs参数 */
        fcs_cfg.src_chl = mac_vap1->channel;
        hmac_fcs_prepare_one_packet_cfg(mac_vap1, &(fcs_cfg.one_packet_cfg), protect_time);

        /* 准备VAP2的fcs参数 */
        fcs_cfg.src_chl2 = mac_vap2->channel;
        hmac_fcs_prepare_one_packet_cfg(mac_vap2, &(fcs_cfg.one_packet_cfg2), protect_time);
        /* 减小第二次one packet的保护时长，从而减少总时长 */
        fcs_cfg.one_packet_cfg2.timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT2;
        fcs_cfg.hal_device = hal_device;
        fcs_cfg.vap_id = mac_vap1->vap_id;

        hmac_fcs_start_enhanced(&fcs_cfg);
        /* 同频STA+GO模式，只需要STA起一次one packet */
    } else {
        vap_sta = (mac_vap1->vap_mode == WLAN_VAP_MODE_BSS_STA) ? mac_vap1 : mac_vap2;
        fcs_cfg.src_chl = vap_sta->channel;
        fcs_cfg.vap_id = vap_sta->vap_id;
        hmac_fcs_prepare_one_packet_cfg(vap_sta, &(fcs_cfg.one_packet_cfg), protect_time);

        /* 调用FCS切信道接口 保存当前硬件队列的帧到扫描虚假队列 */
        hmac_fcs_start(&fcs_cfg);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_scan_do_channel_off_process(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap, mac_channel_stru *next_scan_channel)
{
    osal_u8 up_vap_num, idx;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *vap_tmp[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OSAL_NULL};

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (idx = 0; idx < up_vap_num; idx++) {
        vap_tmp[idx] = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[idx]);
        if (vap_tmp[idx] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_SCAN, "hmac_scan_do_switch_channel_off::vap[%d] IS NULL.", mac_vap_id[idx]);
            return;
        }
    }

    /* 并发扫描的时候如果已经是随机mac，需要复原初始mac */
    if (hmac_vap->random_mac == OSAL_TRUE) {
        mac_scan_req_stru *s_param = &(hmac_device->scan_params);
        if ((s_param->is_radom_mac_saved != 0) && (s_param->radom_mac_saved_to_dev < WLAN_DEVICE_MAX_NUM_PER_CHIP)) {
            hal_scan_params_stru *hal_scan_param = &((&(hal_get_chip_stru()->device))->hal_device_base.hal_scan_params);

            (osal_void)memcpy_s((osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN,
                (osal_void *)(hal_scan_param->original_mac_addr), WLAN_MAC_ADDR_LEN);
        } else {
            oam_error_log3(0, OAM_SF_SCAN, "[%d]hmac_scan_do_channel_off_process:radom_mac_saved[%d] saved_to_dev[%d]",
                hmac_vap->vap_id, s_param->is_radom_mac_saved, s_param->radom_mac_saved_to_dev);
        }
    }

    /* 处在UP状态的VAP数量是否为2 */
    if (up_vap_num == 2) {
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OSAL_NULL);

#ifdef _PRE_WLAN_FEATURE_DBAC
        if (mac_is_dbac_running(hmac_device)) {
            hmac_scan_dbac_switch_channel_off(hmac_device, vap_tmp[0], vap_tmp[1]);
            return;
        }
#endif
        hmac_the_same_channel_switch_channel_off(hmac_device, vap_tmp[0], vap_tmp[1],
            next_scan_channel, hal_device->hal_scan_params.scan_time);
        return;
    }

    /* work状态下也有可能是0个up vap,p2p go 扫描完成状态 */
    if (up_vap_num == 0) {
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OSAL_NULL);
        oam_error_log1(0, 0, "hmac_scan_do_switch_channel_off::vap[%d] is down!!!", hmac_vap->vap_id);
        /* 非工作模式可以清硬件队列，切换信道需要清fifo，传入TRUE */
        hmac_mgmt_switch_channel(hal_device, next_scan_channel, OSAL_TRUE);
        return;
    }

    hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OSAL_NULL);
    for (idx = 0; idx < up_vap_num; idx++) {
        hmac_vap_pause_tx(vap_tmp[idx]);
    }

    for (idx = 0; idx < up_vap_num; idx++) {
        hmac_switch_channel_off(hmac_device, vap_tmp[idx], next_scan_channel, hal_device->hal_scan_params.scan_time);
    }

    return;
}

/*****************************************************************************
 功能描述  : 扫描切离工作信道
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_do_switch_channel_off(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap)
{
    mac_channel_stru *next_scan_channel = OSAL_NULL;
    osal_u8 scan_channel_idx;
    hmac_vap_stru *hmac_vap_temp = OSAL_NULL;
    osal_u32 ret;

    /* 此路扫描完成 */
    if (hal_device->hal_scan_params.curr_scan_state == MAC_SCAN_STATE_IDLE) {
        oam_error_log2(0, OAM_SF_SCAN,
            "vap_id[%d] hmac_scan_do_switch_channel_off::but this hal device is scan channel[%d]complete",
            hmac_vap->vap_id, hal_device->hal_scan_params.channel_nums);
        return;
    }

    scan_channel_idx = get_hal_dev_current_scan_idx(hal_device);
    if ((scan_channel_idx > hmac_device->scan_params.channel_nums) ||
        (hal_device->hal_scan_params.scan_chan_idx > hal_device->hal_scan_params.channel_nums)) {
        oam_error_log3(0, OAM_SF_SCAN,
            "hmac_scan_do_switch_channel_off::hal device scan[%d]channel,total[%d]channel,now channel idx[%d],",
            scan_channel_idx, hal_device->hal_scan_params.channel_nums, hal_device->hal_scan_params.channel_nums);
    }

    next_scan_channel = &(hmac_device->scan_params.channel_list[scan_channel_idx]);

#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
        wifi_printf("vap_id[%d] hmac_scan_do_switch_channel_off::switch channel to idx %d, num %d, bandwidth[%d]\r\n",
            hmac_vap->vap_id, scan_channel_idx, next_scan_channel->chan_number, next_scan_channel->en_bandwidth);
    }
#endif
    common_log_dbg4(0, OAM_SF_SCAN, "vap[%d] hmac_scan_do_switch_channel_off:switch channel to idx %d num %d band[%d]",
        hmac_vap->vap_id, scan_channel_idx, next_scan_channel->chan_number, next_scan_channel->en_bandwidth);

    next_scan_channel->en_bandwidth = WLAN_BAND_WIDTH_20M; /* 扫描时固定为20M, 回主信道时取mac vap的channel值 */
    hmac_scan_switch_channel_notify_alg(hal_device, hmac_vap, next_scan_channel);

    hal_device->hal_scan_params.last_channel_band = next_scan_channel->band;

    /* 非工作状态可以直接切离,不需要保护 */
    ret = mac_device_find_up_vap_etc(hmac_device, &hmac_vap_temp);
    if ((ret != OAL_SUCC) || (hmac_vap_temp == OAL_PTR_NULL) ||
        (hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_WORK_STATE)) {
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OSAL_NULL);
        /* 扫描开PA前把这次信道统计结果标记为有效,避免第一次信道统计结果丢失 */
        hal_device->intf_det_invalid = OSAL_FALSE;
        /* 非工作模式可以清硬件队列，切换信道需要清fifo，传入TRUE */
        hmac_mgmt_switch_channel(hal_device, next_scan_channel, OSAL_TRUE);
        return;
    }

    hmac_scan_do_channel_off_process(hmac_device, hal_device, hmac_vap, next_scan_channel);
}

/*****************************************************************************
 功能描述  : 扫描开始包括：配信道，随机mac，以及发pro req，起超时定时器
*****************************************************************************/
osal_void hmac_scan_one_channel_start(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 is_scan_start)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    osal_void *fhook = OSAL_NULL;
#endif

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_one_channel_start::hmac_vap null.}");
        return;
    }

    /* 切信道:包括直接切离以及保护切离 */
    hmac_scan_do_switch_channel_off(hmac_device, hal_device, hmac_vap);

    /* 扫描开始以及从home channel切走时需要配置随机mac地址 */
    if (is_scan_start == OSAL_TRUE) {
        hmac_scan_set_vap_mac_addr_by_scan_state(hmac_device, hal_device, OSAL_TRUE);
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 处于ps状态下，扫描等到bt的ps结束后来执行，置scan begin状态给btcoex */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
    if (fhook != OSAL_NULL && hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
        ((hmac_btcoex_set_sw_preempt_type_cb)fhook)(hal_device, HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN);
    } else {
#endif
        /* 在begin之前，switch_channel_off会将hal device切换到scan状态 */
        hmac_scan_begin(hmac_device, hal_device);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    }
#endif
}
/*****************************************************************************
 功能描述  : 扫描切离home信道
*****************************************************************************/
osal_void hmac_scan_switch_channel_off(const hmac_device_stru *hmac_device)
{
    wlan_scan_mode_enum_uint8 scan_mode = hmac_device->scan_params.scan_mode;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id); /* 发起扫描的VAP */

    if ((scan_mode >= WLAN_SCAN_MODE_BUTT) || (hmac_vap == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_switch_channel_off::param error, mode[%d]!.}", scan_mode);
        return;
    }

    hmac_scan_one_channel_start(hmac_vap->hal_device, OSAL_TRUE);
}

/*****************************************************************************
 功能描述  : 背景扫描切回home信道
*****************************************************************************/
oal_bool_enum_uint8 hmac_scan_switch_channel_back(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device)
{
    hal_scan_params_stru *hal_scan_status = OSAL_NULL;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_update_dfs_channel_scan_param::param null.}");
        return OSAL_FALSE;
    }
    hal_scan_status = &(hal_device->hal_scan_params);
#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_running(hmac_device) && hmac_alg_dbac_is_pause(hal_device)) {
        if (hal_scan_status->working_in_home_chan == OSAL_FALSE) {
            /* clear fifo when dbac resume */
            hal_save_machw_phy_pa_status(hal_device);
            hal_disable_machw_phy_and_pa();
            /* 清fifo和删除tx队列中所有帧 */
            hmac_clear_hw_fifo_tx_queue(hal_device);
            hal_recover_machw_phy_and_pa(hal_device);
        }
        /* dbac场景只需恢复dbac，由dbac自行切到工作信道 */
        hmac_alg_dbac_resume(hal_device, OSAL_TRUE);
        return OSAL_FALSE;
    }
#endif

    common_log_dbg1(0, OAM_SF_SCAN, "{hmac_scan_switch_channel_back:working_in_home_chan[%u]}",
        hal_scan_status->working_in_home_chan);
    if (hal_scan_status->working_in_home_chan == OSAL_FALSE) {
        /* 切回工作VAP所在的信道 */
        common_log_dbg3(0, OAM_SF_SCAN, "hmac_scan_switch_channel_back:device switch home channel[%d],band[%d],bw[%d]",
            hal_scan_status->home_channel.chan_number, hal_scan_status->home_channel.band,
            hal_scan_status->home_channel.en_bandwidth);

#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
            wifi_printf("{hmac_scan_switch_channel_back::device switch home channel[%d], band[%d], bw[%d]}\r\n",
                hal_scan_status->home_channel.chan_number, hal_scan_status->home_channel.band,
                hal_scan_status->home_channel.en_bandwidth);
        }
#endif
        /* 切换信道需要清fifo，传入TRUE */
        hmac_mgmt_switch_channel(hal_device, &(hal_scan_status->home_channel), OSAL_TRUE);

        /* 恢复home信道上被暂停的发送,包括虚假队列包的搬移 */
        hmac_vap_resume_tx_by_chl(hmac_device, hal_device, &(hal_scan_status->home_channel));

        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}
/*****************************************************************************
 功能描述  : 判断是否需要切回工作信道工作，如果背景扫描模式为支持切回工作信道，
             且每扫描4个信道才切回工作信道
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_scan_need_switch_home_channel(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 reamin_chans;
    hal_scan_params_stru *scan_params = &(hal_device->hal_scan_params);

    /* 背景扫描需要切回工作信道 */
    /* need_switch_back_home_channel 必须最先判断 */
    if (scan_params->need_switch_back_home_channel == OSAL_TRUE) {
        if (scan_params->scan_channel_interval == 0) {
            oam_warning_log0(0, OAM_SF_SCAN,
                             "{hmac_scan_need_switch_home_channel::scan_channel_interval is 0, set default value 6!}");
            scan_params->scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_DEFAULT;
        }

        if (scan_params->channel_nums < (scan_params->scan_chan_idx + 1)) {
            oam_error_log2(0, OAM_SF_SCAN,
                           "{hmac_scan_need_switch_home_channel::scan channel nums[%d] < scan idx[%d]!!!}",
                           scan_params->channel_nums, scan_params->scan_chan_idx);
            return OSAL_FALSE;
        }

        reamin_chans = scan_params->channel_nums - (scan_params->scan_chan_idx + 1);

        /* 剩余的信道数 >= scan_channel_interval / 2 */
        if (reamin_chans >= (scan_params->scan_channel_interval >> 1)) {
            return (scan_params->scan_chan_idx % scan_params->scan_channel_interval == 0);
        }
    }

    return OSAL_FALSE;
}
/*****************************************************************************
 功能描述  : 当前信道扫描超时处理函数，判断是否扫描完成还是继续切换下一信道
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_check_scan_results(const hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device)
{
    hal_scan_params_stru *hal_scan_params = &(hal_device->hal_scan_params);

    if ((hal_scan_params->curr_scan_state != MAC_SCAN_STATE_IDLE) ||
        (hmac_device->scan_params.is_p2p0_scan == OSAL_TRUE) || !((hmac_device->scan_params.scan_mode ==
        WLAN_SCAN_MODE_FOREGROUND) || (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_STA))) {
        return OAL_SUCC;
    }

    if ((hal_scan_params->scan_ap_num_in_2p4 <= 0x2) || (hal_scan_params->scan_ap_num_in_5p0 <= 0x2)) {
#ifdef _PRE_WLAN_DFT_STAT
        hal_device_fsm_info(hal_device);
#endif
    }
    return OAL_SUCC;
}
/*****************************************************************************
 功能描述  : 更新雷达信道扫描参数
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_update_dfs_channel_scan_param(const hmac_device_stru *hmac_device,
    const hal_to_dmac_device_stru *hal_device, const mac_channel_stru *mac_channel,
    osal_u16 *scan_time, oal_bool_enum_uint8 *send_probe_req)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    const hal_scan_params_stru *scan_params = &(hal_device->hal_scan_params);

    /* 非雷达信道，需要发送probe req，扫描时间从扫描参数中获取 */
    if (mac_is_dfs_channel(mac_channel->band, mac_channel->chan_number) == OSAL_FALSE) {
        *send_probe_req = OSAL_TRUE;
        *scan_time = scan_params->scan_time;
        return;
    }

    /* 如果当前为关联状态，且关联AP 信道和本次扫描信道相同，则认为该信道无雷达，直接发送probe req 发起扫描 */
    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_dfs_channel_scan_param::get vap [%d] fail.}",
            hmac_device->scan_params.vap_id);
        *send_probe_req = OSAL_FALSE;
        *scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
        return;
    }

    if (is_ap(hmac_vap)) {
        *send_probe_req = OSAL_FALSE;
        *scan_time = scan_params->scan_time;
        return;
    }

    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) &&
        (mac_channel->chan_number == hmac_vap->channel.chan_number)) {
        *send_probe_req = OSAL_TRUE;
        *scan_time = scan_params->scan_time;
        return;
    }

    /* 雷达信道第一次扫描，不发送probe req ，且在该信道暂停60ms
     * 雷达信道第二次扫描，如果在第一次扫描时候没有发现有AP，
     * 则退出本信道扫描
     * 如果在第一次扫描时候发现有AP，
     * 则发送probe req ，且在该信道暂停20ms
     */
    if (scan_params->curr_channel_scan_count == 0) {
        /* 雷达信道第一次扫描，不发送probe req ，在该信道监听60ms */
        *send_probe_req = OSAL_FALSE;
        *scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
    } else {
        if (scan_params->scan_curr_chan_find_bss_flag == OSAL_TRUE) {
            /* 雷达信道第二次扫描，且第一次扫描时有发现AP需要发送probe req */
            *send_probe_req = OSAL_TRUE;
            *scan_time = scan_params->scan_time;
        } else {
            /* 雷达信道第二次扫描，且第一次扫描时没有发现AP,不发送probe,超时时间设置为0 */
            *send_probe_req = OSAL_FALSE;
            *scan_time = 0;
        }
    }
    return;
}
/*****************************************************************************
 功能描述  : 单路hal device扫描完成切回工作信道处理
*****************************************************************************/
osal_void hmac_scan_handle_switch_channel_back(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, const hal_scan_params_stru *hal_scan_params)
{
    osal_u32 ret;
    oal_bool_enum_uint8 switched;
    osal_u8 mac_vap_id;
    hmac_vap_stru *hmac_home_vap = OSAL_NULL;

    switched = hmac_scan_switch_channel_back(hmac_device, hal_device);
    if (switched != OSAL_TRUE) {
        return;
    }

    /* 对于切回home信道的场景需要获取home信道vap通知算法 */
    ret = hal_device_find_one_up_vap(hal_device, &mac_vap_id);
    if (ret != OAL_SUCC) {
        return;
    }

    hmac_home_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id);
    if (hmac_home_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "{hmac_scan_handle_switch_channel_back::vap id[%d],hmac_vap is null.}", mac_vap_id);
        return;
    }

    if (hal_scan_params->home_channel.chan_number == hmac_home_vap->channel.chan_number) {
        hmac_scan_switch_channel_notify_alg(hal_device, hmac_home_vap, &(hal_scan_params->home_channel));
    }

    return;
}

/*****************************************************************************
 功能描述  : 此路扫描完成优先回home channel工作
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_back_home_channel(const hal_scan_params_stru *hal_scan_params,
    osal_u8 scan_complete_event, hal_to_dmac_device_stru *hal_device, const hmac_vap_stru * const hmac_vap,
    hmac_device_stru *hmac_device)
{
    osal_void *fhook = OSAL_NULL;
    if (hal_scan_params->need_switch_back_home_channel == OSAL_TRUE) {
        /* abort的话，强制结束扫描，避免引起其他问题, abort之后恢复到idle或者work状态了，
            ps机制按照正常流程走，可以处于save或者normal状态，btcoex只是做维测，确认下频繁程度 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
        if (scan_complete_event == HAL_DEVICE_EVENT_SCAN_ABORT && fhook != OSAL_NULL) {
            ((hmac_btcoex_set_sw_preempt_type_cb)fhook)(hal_device, HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT);
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_single_hal_device_scan_complete:: do scan abort right now!}",
                hmac_vap->vap_id);
        }

        /* 抛事件会清扫描的标记,标记必须在切完信道后再清,否则会再次清硬件队列配信道 */
        hal_device_handle_event(hal_device, scan_complete_event, 0, OSAL_NULL);

        /* scan end和switch back都需要先切hal device状态，再配置home信道工作 */
        hmac_scan_handle_switch_channel_back(hmac_device, hal_device, hal_scan_params);
    } else {
        /* clear fifo when not need back home channel */
        hal_save_machw_phy_pa_status(hal_device);
        hal_disable_machw_phy_and_pa();
        /* 清fifo和删除tx队列中所有帧 */
        hmac_clear_hw_fifo_tx_queue(hal_device);
        hal_recover_machw_phy_and_pa(hal_device);
        hal_device_handle_event(hal_device, scan_complete_event, 0, OSAL_NULL);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
        if (fhook != OSAL_NULL && scan_complete_event == HAL_DEVICE_EVENT_SCAN_ABORT) {
            oam_warning_log2(0, 0, "vap_id[%d] {scan abort not need back home channel bt_sw_preempt_type[%d]!}",
                hmac_vap->vap_id, hal_device->btcoex_sw_preempt.sw_preempt_type);
            ((hmac_btcoex_set_sw_preempt_type_cb)fhook)(hal_device, HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT);
        }
#endif
    }
    if (scan_complete_event != HAL_DEVICE_EVENT_SCAN_ABORT) {
        hmac_scan_check_scan_results(hmac_device, hal_device);
    }
    return;
}

/*****************************************************************************
 功能描述  : 单路hal device扫描完成
*****************************************************************************/
OAL_STATIC osal_void hmac_single_hal_device_scan_complete(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap, osal_u8 scan_complete_event)
{
    hal_scan_params_stru *hal_scan_params = OSAL_NULL;
    osal_u8 do_p2p_listen;
    osal_u32 run_time;

    if (osal_unlikely(hmac_device == OSAL_NULL || hal_device == OSAL_NULL || hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_single_hal_device_scan_complete::param null.}");
        return;
    }

    hal_scan_params = &(hal_device->hal_scan_params);

    if (hal_scan_params->curr_scan_state == MAC_SCAN_STATE_IDLE) {
        oam_warning_log1(0, OAM_SF_SCAN,
                         "{hmac_single_hal_device_scan_complete::hal dev is already scan complete[%d].}",
                         hal_scan_params->curr_scan_state);
        return;
    }

    /* 扫描发送probe tx dscr打印维测控制 */
    hal_scan_params->tx_dscr_debug_ctl = HAL_SCAN_TX_DSCR_DBG_CTL_INIT;

    /* listen时修改vap信道为listen信道，listen结束后需要恢复 p2p listen不可能是并发 */
    do_p2p_listen = hmac_device->scan_params.scan_func & MAC_SCAN_FUNC_P2P_LISTEN;
    if (do_p2p_listen != 0) {
        hmac_vap->channel = hmac_device->p2p_vap_channel;

        /* P2P监听状态结束，恢复另一路p2p设备发送 */
        if ((hmac_vap->hal_device != hal_device) &&
            (hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE)) {
            if (osal_likely(hmac_vap->hal_vap != OSAL_NULL)) {
                hal_vap_beacon_resume(hmac_vap->hal_vap);
            }
            hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
        }
    }

    run_time = osal_get_runtime(hmac_device->scan_timestamp, (osal_u32)osal_get_time_stamp_ms());

    common_log_dbg4(0, OAM_SF_SCAN,
        "{hmac_single_hal_device_scan_complete:total time[%d]ms,chan[%d] need_back_home[%d] complete_event[%d]}",
        run_time, hal_scan_params->channel_nums, hal_scan_params->need_switch_back_home_channel, scan_complete_event);

    /* 打印到hso的同时, 将扫描时间输出到串口, 便于直观看到, p2p的监听信道 channel_list[0] */
    wifi_printf(
        "hmac_single_hal_device_scan_complete:vap[%d] time[%d] chan_cnt[%d] chan_0[%d] back[%d] event[%d] mode[%d]\r\n",
        hmac_vap->vap_id, run_time, hal_scan_params->channel_nums, hmac_device->scan_params.channel_list[0].chan_number,
        hal_scan_params->need_switch_back_home_channel, scan_complete_event, hal_scan_params->scan_mode);
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
    if (is_legacy_sta(hmac_vap)) {
        hmac_write_channel_scoring_record(hmac_device, hal_scan_params->channel_nums);
    }
#endif
    hmac_scan_set_vap_mac_addr_by_scan_state(hmac_device, hal_device, OSAL_FALSE);
    /* 此路扫描完成优先回home channel工作 */
    hmac_scan_back_home_channel(hal_scan_params, scan_complete_event, hal_device, hmac_vap, hmac_device);
    return;
}

/*****************************************************************************
 功能描述  : 获取组装probe req请求时，需要组装的ssid，ssid信息根据内核下发的ssid中获取，
             每信道扫描时，依次将携带这些ssid的probe req帧发送出去
             p2p设备只发送指定ssid的probe req帧。
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_get_ssid_ie_info(const hmac_device_stru *hmac_device, osal_char *ssid,
    osal_u8 ssid_size, osal_u8 index)
{
    hmac_vap_stru *hmac_vap;
    errno_t ret;

    /* begin:P2P 设备扫描时，需要携带指定ssid 信息，不能发送ssid 内容为0 的扫描 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "vap_id[%d] {hmac_scan_get_ssid_ie_info::hmac_vap null.}", hmac_device->scan_params.vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (is_legacy_vap(hmac_vap)) {
        /* 根据索引号，准备probe req帧的ssid信息 */
        ret = memcpy_s(ssid, ssid_size, hmac_device->scan_params.mac_ssid_set[index].ssid,
            sizeof(hmac_device->scan_params.mac_ssid_set[index].ssid));
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_set_vap_mac_addr_process:: memcpy_s failed.}");
            return OAL_FAIL;
        }
    } else {
        /* P2P 设备扫描，需要获取指定ssid 信息，对P2P 设备，扫描时只扫描一个指定ssid */
        ret = memcpy_s(ssid, ssid_size, hmac_device->scan_params.mac_ssid_set[0].ssid,
            sizeof(hmac_device->scan_params.mac_ssid_set[0].ssid));
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_set_vap_mac_addr_process:: memcpy_s failed.}");
            return OAL_FAIL;
        }
    }
    /* end:P2P 设备扫描时，需要携带指定ssid 信息，不能发送ssid 内容为0 的扫描 */
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 上报当前信道的信道测量结果
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_report_channel_statistics_result(const hal_to_dmac_device_stru *hal_device,
    osal_u8 scan_idx)
{
    hmac_crx_chan_result_stru chan_result_param = {0};

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_report_channel_statistics_result::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    chan_result_param.scan_idx = scan_idx;
    chan_result_param.chan_result = hal_device->chan_result;

    hmac_scan_process_chan_result_event_etc(&chan_result_param);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 发送广播Probe request帧
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_send_bcast_probe(const hmac_device_stru *hmac_device, osal_u8 band, osal_u8 index)
{
    osal_char           ac_ssid[WLAN_SSID_MAX_LEN] = {'\0'};
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 ret;
    osal_u8 band_tmp;
    osal_u8 is_send_cap_ie = OAL_FALSE;
    osal_u8 loop;
    osal_u8 send_times;

    if (hmac_device->vap_num == 0) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_send_bcast_probe::vap_num=0.}");
        return OAL_FAIL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_send_bcast_probe::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*
       如果st_scan_params.vap_id = 0，可能内存被踩，要直接返回错误，
       防止后面访问配置vap的造成空指针死机，同时增加维测，把st_scan_params
    */
    if (hmac_vap->mib_info == OSAL_NULL) {
        oam_error_log3(0, OAM_SF_SCAN,
            "{hmac_scan_send_bcast_probe:: vap mib info is null,vap_id[%d], p_fn_cb[%p], scan_func[%d].}",
            hmac_device->scan_params.vap_id, (uintptr_t)hmac_device->scan_params.fn_cb,
            hmac_device->scan_params.scan_func);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发探测请求时，需要临时更新vap的band信息，防止5G发11b */
    band_tmp = hmac_vap->channel.band;

    hmac_vap->channel.band = band;

    /* 获取本次扫描请求帧中需要携带的ssid ie信息 */
    ret = hmac_scan_get_ssid_ie_info(hmac_device, ac_ssid, WLAN_SSID_MAX_LEN, index);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_send_bcast_probe::get ssid failed, error[%d].}", ret);
        return ret;
    }

    /* P2P需要携带能力IE */
    if (!is_legacy_vap(hmac_vap)) {
        is_send_cap_ie = OAL_TRUE;
    }

    /* 单个probereq发送次数,相当于host侧重发次数, 最多7次, 0表示未进行配置 */
    send_times = (hmac_device->scan_params.single_probe_times == 0) ? 1 : hmac_device->scan_params.single_probe_times;

    /* 发送probe req帧 */
    for (loop = 0; loop < send_times; loop++) {
        ret = hmac_scan_send_probe_req_frame(hmac_vap, BROADCAST_MACADDR, ac_ssid, WLAN_SSID_MAX_LEN, is_send_cap_ie);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_scan_send_bcast_probe::hmac_mgmt_send_probe_request failed[%d].}",
                hmac_vap->vap_id, ret);
            break;
        }
    }

    hmac_vap->channel.band = band_tmp;

    return ret;
}

OAL_STATIC osal_u32 hmac_scan_time_out_switch_channel(hal_to_dmac_device_stru *hal_device,
    hal_scan_params_stru *hal_scan_params, hmac_device_stru *hmac_device)
{
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    osal_void *fhook = OSAL_NULL;
#endif

    if ((hmac_device->scan_params.scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS)) != OSAL_FALSE) {
        hmac_scan_report_channel_statistics_result(hal_device, (get_hal_dev_current_scan_idx(hal_device)));
        /* 清空信道测量结果，准备下一次信道测量值的统计 */
        (osal_void)memset_s(&(hal_device->chan_result), sizeof(wlan_scan_chan_stats_stru),
            0, sizeof(wlan_scan_chan_stats_stru));
    }
    hal_scan_params->scan_chan_idx += 1; /* 切换信道 */
    /* 此次扫描请求完成，做一些收尾工作 */
    if (hal_scan_params->scan_chan_idx >= hal_scan_params->channel_nums) {
        hmac_scan_prepare_end(hmac_device, hal_device);
        return OAL_SUCC;
    }
    if (hmac_scan_need_switch_home_channel(hal_device) == OSAL_TRUE) {
        /* 需要回home channel时，如果处于ps状态，需要延迟，等到ps机制完成后回home channel */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
        if (fhook != OSAL_NULL && hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
            ((hmac_btcoex_set_sw_preempt_type_cb)fhook)(hal_device, HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT);
        } else {
#endif
            hmac_scan_switch_home_channel_work(hmac_device, hal_device);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        }
#endif
    } else {
        hmac_scan_one_channel_start(hal_device, OSAL_FALSE);
    }

    return OAL_CONTINUE;
}
/*****************************************************************************
 功能描述  : 当前信道扫描超时处理函数，判断是否扫描完成还是继续切换下一信道
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_curr_channel_scan_time_out(osal_void *arg)
{
    hal_to_dmac_device_stru *hal_device = (hal_to_dmac_device_stru *)arg;
    hal_scan_params_stru *hal_scan_params = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    osal_void *fhook = OSAL_NULL;
#endif

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_curr_channel_scan_time_out::arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_scan_params = &(hal_device->hal_scan_params);

    /* 获取发起扫描的dmac vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap[%d]curr_channel_scan_time_out null vap", hmac_device->scan_params.vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
        oam_info_log4(0, OAM_SF_SCAN, "scan_time_out:func%u count%u per%u chn%u",
            hmac_device->scan_params.scan_func, hal_scan_params->curr_channel_scan_count,
            hal_scan_params->max_scan_count_per_channel, hal_scan_params->scan_chan_idx);
    }
#endif
    hal_scan_params->curr_channel_scan_count++;
    /* 根据扫描参数中信道扫描最大次数，判断是否切换信道号 */
    if (hal_scan_params->curr_channel_scan_count >= hal_scan_params->max_scan_count_per_channel) {
        if (hmac_scan_time_out_switch_channel(hal_device, hal_scan_params, hmac_device) == OAL_SUCC) {
            return OAL_SUCC;
        }
    } else {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_SET_SW_PREEMPT_TYPE);
        if (fhook != OSAL_NULL && hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
            ((hmac_btcoex_set_sw_preempt_type_cb)fhook)(hal_device, HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN);
        } else {
#endif
            hmac_scan_begin(hmac_device, hal_device);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        }
#endif
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_scan_whether_create_timer(hal_to_dmac_device_stru *hal_device, const osal_u16 scan_time,
    const mac_channel_stru *mac_channel)
{
    /* 雷达信道且扫描时间为0，不触发定时器操作 */
    if ((scan_time == 0) && (mac_is_dfs_channel(mac_channel->band, mac_channel->chan_number) != OSAL_FALSE)) {
        hmac_scan_curr_channel_scan_time_out(hal_device);
        return;
    }

    /* 放在发送完成所有probe
     * request报文再启动定时器,防止指定SSID扫描报文过多,定时器时间内都在发送,用于接收扫描结果时间过短 */
    /* 启动扫描定时器 */
    frw_create_timer_entry(&(hal_device->hal_scan_params.scan_timer), hmac_scan_curr_channel_scan_time_out,
                           scan_time, hal_device, OAL_FALSE);
    return;
}

OAL_STATIC osal_void hmac_scan_begin_process(const hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, const mac_scan_req_stru *scan_params, hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    osal_u8 band, do_bss_scan, do_meas, loop, scan_chan_idx;
    oal_bool_enum_uint8 send_probe_req;
    osal_u16 scan_time;

    unref_param(hmac_vap);

    scan_chan_idx = get_hal_dev_current_scan_idx(hal_device);
    band = scan_params->channel_list[scan_chan_idx].band;
    do_bss_scan = scan_params->scan_func & MAC_SCAN_FUNC_BSS;

    /* 如果当前扫描模式需要统计信道信息，则使能对应寄存器 */
    do_meas = scan_params->scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS);
    if (do_meas != 0) {
        hal_chan_measure_stru meas = {0};
        /* 使能信道测量中断 */
        meas.statics_period = HMAC_SCAN_CHANNEL_STATICS_PERIOD_US;
        meas.meas_period = HMAC_SCAN_CHANNEL_MEAS_PERIOD_MS;
        hal_chan_measure_start(&meas);
    }

    /* 扫描第一次切信道打印 probe req tx dscr控制 */
    if (hal_device->hal_scan_params.tx_dscr_debug_ctl == HAL_SCAN_TX_DSCR_DBG_CTL_INIT) {
        hal_device->hal_scan_params.tx_dscr_debug_ctl = HAL_SCAN_TX_DSCR_DBG_CTL_PRINT_PERMIT;
    }

    /* dfs信道判断，如果是雷达信道，执行被动扫描 */
    hmac_scan_update_dfs_channel_scan_param(hmac_device, hal_device,
                                            &(scan_params->channel_list[scan_chan_idx]), &scan_time, &send_probe_req);

    /* ACTIVE方式下发送广播RPOBE REQ帧 */
    if ((do_bss_scan != 0) && (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE) && (send_probe_req == OSAL_TRUE)) {
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
        /* PNO指定SSID扫描,最多指定16个SSID */
        if (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_PNO) {
            hmac_pno_scan_send_probe_with_ssid(hmac_device, hal_device, band);
        }
#endif
        /* 每次信道发送的probe req帧的个数 */
        for (loop = 0; loop < scan_params->max_send_probe_req_count_per_channel; loop++) {
            ret = hmac_scan_send_bcast_probe(hmac_device, band, loop);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_begin::hmac_scan_send_bcast_probe failed[%d].}", ret);
            }
        }
    }

    hmac_scan_whether_create_timer(hal_device, scan_time, &(scan_params->channel_list[scan_chan_idx]));
    return;
}

/*****************************************************************************
 功能描述  : 启动一个扫描，其中，扫描的所有参数由dmac_scan_prepare准备
*****************************************************************************/
osal_void hmac_scan_begin(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device)
{
    hal_to_dmac_device_stru *original_hal_device = OSAL_NULL;
    mac_scan_req_stru *scan_params = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if ((hmac_device == OSAL_NULL) || (hal_device == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_scan_begin::para NULL, mac:%p hal:%p .}",
            (uintptr_t)hmac_device, (uintptr_t)hal_device);
        return;
    }

    /* 此路优先扫描完成不再发pro req */
    if (hal_device->hal_scan_params.curr_scan_state == MAC_SCAN_STATE_IDLE) {
        oam_error_log1(0, OAM_SF_SCAN,
                       "{hmac_scan_begin::now chan idx[%d] scan is completed.}",
                       hal_device->hal_scan_params.scan_chan_idx);
        return;
    }

    /* 此路被pause不能扫描 */
    if (hal_device->hal_scan_params.scan_pause_bitmap != 0) {
        oam_warning_log2(0, OAM_SF_SCAN,
                         "{hmac_scan_begin::scan is paused[%x].scan chan idx[%d]}",
                         hal_device->hal_scan_params.scan_pause_bitmap,
                         hal_device->hal_scan_params.scan_chan_idx);
        return;
    }

    /* 获取扫描参数 */
    scan_params = &(hmac_device->scan_params);
    hmac_vap = mac_res_get_hmac_vap(scan_params->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_begin::hmac_vap null.}", scan_params->vap_id);
        return;
    }

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN, (osal_void *)hmac_device);

    /* 扫描过程中切换指针是否有问题 */
    original_hal_device = hmac_vap->hal_device;
    hmac_vap->hal_device = hal_device;

    hmac_scan_begin_process(hmac_device, hal_device, scan_params, hmac_vap);

    hmac_vap->hal_device = original_hal_device;

    /* p2p listen时需要更改VAP的信道，组probe rsp帧(DSSS ie, ht ie)需要。listen结束后恢复 */
    if ((scan_params->scan_func & MAC_SCAN_FUNC_P2P_LISTEN) != 0) {
        /* P2P监听状态，需要pause处于work状态的p2p设备 */
        if ((hmac_vap->hal_device != hal_device) &&
            (hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE)) {
            hmac_vap_pause_tx(hmac_vap);
        }

        hmac_device->p2p_vap_channel = hmac_vap->channel;

        /* 扫描响应帧携带的DSSS Parameter表示STA可识别的信道号错误 */
        hmac_vap->channel = scan_params->channel_list[0];
    }

    return;
}

OAL_STATIC osal_void hmac_scan_end_process(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    wlan_scan_mode_enum_uint8 scan_mode)
{
    osal_void *fhook = OSAL_NULL;
    unref_param(hmac_device);
    /* 根据扫描模式进行对应扫描结果的处理 */
    switch (scan_mode) {
        case WLAN_SCAN_MODE_FOREGROUND:
            /* 上报扫描完成事件，扫描状态为成功 */
            (osal_void)hmac_scan_proc_scan_complete_event(hmac_vap, MAC_SCAN_SUCCESS);
            break;
        case WLAN_SCAN_MODE_BACKGROUND_STA:
        case WLAN_SCAN_MODE_BACKGROUND_AP:
        case WLAN_SCAN_MODE_ROAM_SCAN:
            /* 上报扫描完成事件，扫描状态为成功 */
            (osal_void)hmac_scan_proc_scan_complete_event(hmac_vap, MAC_SCAN_SUCCESS);
            break;
        case WLAN_SCAN_MODE_BACKGROUND_OBSS:
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
            hmac_scan_proc_obss_scan_complete_event(hmac_vap);
#endif
            break;
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
        case WLAN_SCAN_MODE_BACKGROUND_PNO:
            /* 是否扫描到了匹配的ssid，如果是，上报扫描结果; 否则进入睡眠 */
            if (hmac_device->pno_sched_scan_mgmt->is_found_match_ssid == OSAL_TRUE) {
                /* 上报扫描完成事件，扫描状态为成功 */
                (osal_void)hmac_scan_proc_scan_complete_event(hmac_vap, MAC_SCAN_PNO);

                /* 释放PNO管理结构体内存 */
                oal_mem_free(hmac_device->pno_sched_scan_mgmt, OAL_TRUE);
                hmac_device->pno_sched_scan_mgmt = OSAL_NULL;
            }
            break;
#endif
        default:
            break;
    }

    // 扫描结束
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_SET_BTCOEX_WIFI_STATUS);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_hal_set_btcoex_wifi_status_cb)fhook)(hmac_vap, OSAL_FALSE);
    }

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    hmac_alg_anti_intf_switch(hmac_vap->hal_device, OSAL_TRUE); /* 算法要求改为 WLAN_ANTI_INTF_EN_PROBE */
#endif

    return;
}

/*****************************************************************************
 功能描述  : 一个扫描请求完成时的收尾函数
*****************************************************************************/
osal_void hmac_scan_end(hmac_device_stru *hmac_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    wlan_scan_mode_enum_uint8 scan_mode;
    osal_u8 curr_scan_state_pre = hmac_device->curr_scan_state;

    /* 获取dmac vap */
    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_end::hmac_vap is null.}");

        /* 恢复device扫描状态为空闲状态 */
        hmac_device->curr_scan_state = MAC_SCAN_STATE_IDLE;
        if (curr_scan_state_pre != hmac_device->curr_scan_state) {
            /* 扫描状态发生变化，同步 */
            hmac_device_sync(hmac_device);
        }
        return;
    }

    /* 获取扫描模式 */
    scan_mode = hmac_device->scan_params.scan_mode;

    common_log_dbg2(0, OAM_SF_SCAN, "{hmac_scan_end::scan_mode[%d],vap_id[%d].}", scan_mode, hmac_vap->vap_id);
    /* 恢复device扫描状态为空闲状态, 先设置状态, 因为接下来上报扫描结果的消息变为同步了,在漫游场景下会有scan abort处理,
       导致认为扫描还在进行中,再次走到这里的end流程来 */
    hmac_device->curr_scan_state = MAC_SCAN_STATE_IDLE;
    /* 扫描状态更新后通知CSA模块 */
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_SCAN_END, (osal_void *)hmac_device);
    hmac_device->scan_params.scan_mode = WLAN_SCAN_MODE_BUTT;
    hmac_scan_end_process(hmac_device, hmac_vap, scan_mode);

    if (curr_scan_state_pre != hmac_device->curr_scan_state) {
        /* 扫描状态发生变化，同步 */
        hmac_device_sync(hmac_device);
    }

    return;
}

/*****************************************************************************
 功能描述  : 准备结束扫描
*****************************************************************************/
osal_void hmac_scan_prepare_end(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    if (osal_unlikely(hmac_device == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_prepare_end::param null.}");
        return;
    }

    /* 获取发起扫描的dmac vap结构信息 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_prepare_end::mac_res_get_hmac_vap fail}", hmac_device->scan_params.vap_id);
        return;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_END_SCAN_CHECK_PS);
    if (fhook == OSAL_NULL || ((hmac_btcoex_hal_end_scan_check_ps_cb)fhook)(hal_device, hmac_vap) != OSAL_TRUE) {
        hmac_single_hal_device_scan_complete(hmac_device, hal_device, hmac_vap, HAL_DEVICE_EVENT_SCAN_END);
        hmac_scan_end(hmac_device);
    }
}

/*****************************************************************************
 功能描述  : 结束扫描
*****************************************************************************/
osal_void hmac_scan_abort(hmac_device_stru *hmac_device)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) {
        return;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_abort::dmac vap is null.}");
        return;
    }

    hal_device = hmac_device->hal_device[0];
    if (hal_device != OSAL_NULL) {
        /* 删除扫描定时器 */
        if (hal_device->hal_scan_params.scan_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&(hal_device->hal_scan_params.scan_timer));
        }

        if (hal_device->hal_scan_params.curr_scan_state == MAC_SCAN_STATE_RUNNING) {
            hmac_single_hal_device_scan_complete(hmac_device, hal_device, hmac_vap, HAL_DEVICE_EVENT_SCAN_ABORT);
        }
    }

    hmac_device->scan_params.abort_scan_flag = OSAL_TRUE;
    hmac_scan_end(hmac_device);
    hmac_device->scan_params.abort_scan_flag = OSAL_FALSE;

    oam_warning_log0(0, OAM_SF_SCAN, "hmac_scan_abort: scan has been aborted");
}

/*****************************************************************************
 功能描述  : 背景扫描时切回工作信道工作超时，继续扫描
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_switch_home_channel_work_timeout(osal_void *arg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = (hal_to_dmac_device_stru *)arg;

    /* 判断是否还需要继续进行扫描，如果此时扫描状态为非运行状态，说明扫描已经停止，无需再继续扫描 */
    if (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_scan_switch_home_channel_work_timeout::scan aborted.}",
            hmac_device->scan_params.vap_id);
        return OAL_SUCC;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id {hmac_scan_switch_home_channel_work_timeout::hmac_vap[%d] null.}",
            hmac_device->scan_params.vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_scan_one_channel_start(hal_device, OSAL_TRUE);

    /* 清空信道测量结果 */
    (osal_void)memset_s(&(hal_device->chan_result), sizeof(wlan_scan_chan_stats_stru),
        0, sizeof(wlan_scan_chan_stats_stru));

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 背景扫描时切回到工作信道进行工作
*****************************************************************************/
osal_void hmac_scan_switch_home_channel_work(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device)
{
    hal_scan_params_stru *scan_params = &(hal_device->hal_scan_params);

    /* 切回工作信道工作时，根据是否为随机mac addr扫描，恢复vap原先的mac addr */
    hmac_scan_set_vap_mac_addr_by_scan_state(hmac_device, hal_device, OSAL_FALSE);

    /* 并发扫描回主信道时，需要先调度device状态机，再配信道，否则辅路可能没有11b
     * reuse却继续发11b速率的帧 */
    hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK, 0, OSAL_NULL);

    /* 背景扫描 切回工作信道 */
    (osal_void)hmac_scan_switch_channel_back(hmac_device, hal_device);

    /* 扫描回主信道标志，由device状态机中剥离 */
    hal_device->hal_scan_params.working_in_home_chan = OSAL_TRUE;

    /* 判零检查 */
    if (scan_params->work_time_on_home_channel == 0) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "{hmac_scan_switch_home_channel_work:work_time_on_home_channel is 0, set it to default [%d]ms!}",
            MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT);
        scan_params->work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    }

    /* 启动间隔定时器，在工作信道工作一段时间后，切回扫描信道进行扫描 */
    frw_create_timer_entry(&(scan_params->scan_timer), hmac_scan_switch_home_channel_work_timeout,
                           scan_params->work_time_on_home_channel, hal_device, OAL_FALSE);

    return;
}

/*****************************************************************************
 功能描述  : DMAC SCAN模块的初始化函数，负责申请SCANNER的内存及初始化其成员变量
*****************************************************************************/
osal_u32 hmac_scan_init(hmac_device_stru *hmac_device)
{
    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_init::param null.}");
        return OAL_FAIL;
    }
    /* 初始化device扫描状态为空闲 */
    hmac_device->curr_scan_state = MAC_SCAN_STATE_IDLE;
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_scan_get_ch_measurement_result_process(hal_ch_statics_irq_event_stru *stats_result,
    hal_to_dmac_device_stru *hal_device, hal_ch_mac_statics_stru *mac_stats, frw_msg *msg)
{
    hal_ch_mac_statics_stru *event_mac_stats = OSAL_NULL;
    /* MAC信道测量统计时间 */
    osal_u32 trx_time_us = stats_result->ch_rx_time_us + stats_result->ch_tx_time_us;
    stats_result->ch_stats_time_us = (stats_result->ch_stats_time_us) << HMAC_SCAN_SHIFT; // 单位由1kus转换为1us
    stats_result->pri20_free_time_us =
        hmac_scan_get_valid_free_time(trx_time_us, stats_result->ch_stats_time_us, stats_result->pri20_free_time_us);
    stats_result->pri40_free_time_us =
        hmac_scan_get_valid_free_time(trx_time_us, stats_result->ch_stats_time_us, stats_result->pri40_free_time_us);
    stats_result->sec20_free_time_us =
        hmac_scan_get_valid_free_time(trx_time_us, stats_result->ch_stats_time_us, stats_result->sec20_free_time_us);
    hal_device->chan_result.total_free_time_20m_us += stats_result->pri20_free_time_us;
    hal_device->chan_result.total_free_time_40m_us += stats_result->pri40_free_time_us;
    hal_device->chan_result.total_free_time_sec20m_us += stats_result->sec20_free_time_us;
    hal_device->chan_result.total_recv_time_us += stats_result->ch_rx_time_us;
    hal_device->chan_result.total_send_time_us += stats_result->ch_tx_time_us;
    hal_device->chan_result.total_stats_time_us += stats_result->ch_stats_time_us;

    /* offload架构下,基于non direct帧统计来识别同频干扰 */
    /* 统计MAC FCS正确帧时间 */
    /* 无需判空 */
    event_mac_stats = (hal_ch_mac_statics_stru *)(msg->data);

    trx_time_us = event_mac_stats->rx_direct_time + event_mac_stats->tx_time;
    event_mac_stats->rx_nondir_time =
        hmac_scan_get_valid_free_time(trx_time_us, stats_result->ch_stats_time_us, event_mac_stats->rx_nondir_time);
    mac_stats->rx_direct_time += event_mac_stats->rx_direct_time;
    mac_stats->rx_nondir_time += event_mac_stats->rx_nondir_time;
    mac_stats->tx_time += event_mac_stats->tx_time;
    return;
}

/*****************************************************************************
 功能描述  : 获取硬件信道统计 测量结果
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_get_ch_statics_measurement_result_ram(const hmac_device_stru * const hmac_device,
    hal_to_dmac_device_stru *hal_device, frw_msg *msg)
{
    hal_ch_statics_irq_event_stru stats_result;
    osal_u8 chan_idx;

    if (osal_unlikely(hmac_device == OSAL_NULL || hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_get_ch_statics_measurement_result_ram::param null.}");
        return;
    }

    /* 读取结果 */
    (osal_void)memset_s(&stats_result, sizeof(stats_result), 0, sizeof(stats_result));

    /* MAC测量时PHY通知统计信息 */
    hal_get_ch_statics_result(&stats_result);
    /* PHY信道功率测量信息 */
    hal_get_ch_measurement_result_ram(&stats_result);
    hal_save_machw_phy_pa_status(hal_device);
    /* 当前的统计周期如果有PA关，则当前的统计周期不计算在内 */
    if (hal_device->intf_det_invalid == OSAL_TRUE) {
        if (hal_device->is_mac_pa_enabled == OSAL_TRUE) {
            hal_device->intf_det_invalid = OSAL_FALSE;
        }
        return;
    }
    chan_idx = get_hal_dev_current_scan_idx(hal_device);
    hal_device->chan_result.stats_valid = 1;
    hal_device->chan_result.channel_number = hmac_device->scan_params.channel_list[chan_idx].chan_number;
    if (stats_result.pri20_idle_power < 0) {
        hal_device->chan_result.free_power_stats_20m += (osal_s8)stats_result.pri20_idle_power; /* 主20M信道空闲功率 */
        hal_device->chan_result.free_power_stats_40m += (osal_s8)stats_result.pri40_idle_power; /* 主40M信道空闲功率 */
        hal_device->chan_result.phy_total_stats_time_ms += stats_result.phy_ch_estimate_time_ms;
        hal_device->chan_result.free_power_cnt += 1;
    }
    hmac_scan_get_ch_measurement_result_process(&stats_result, hal_device, &hal_device->mac_ch_stats, msg);
}

/*****************************************************************************
 功能描述  : HAL层中的信道统计/测量完成中断对应的事件处理函数
*****************************************************************************/
osal_s32 hmac_scan_channel_statistics_complete(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 do_meas; /* 本次扫描是否要获取信道测量的结果 */
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_ch_stats_result scan_result;
    unref_param(hmac_vap);

    /* 寻找对应的DEVICE结构以及相应的ACS结构 */
    hal_device = hal_chip_get_hal_device();

    /* 如果当前测量已经被优先级更高的业务中断,则放弃当前测量结果 */
    do_meas = hmac_device->scan_params.scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS);
    if (do_meas != 0 && (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        /* 读取硬件统计测量结果 */
        hmac_scan_get_ch_statics_measurement_result_ram(hmac_device, hal_device, msg);
    }
    hal_device->chan_result.stats_cnt++;
    /* CCA检测使用信道测量中断内判断结束和正常扫描解耦 */
    if (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) {
        (osal_void)memset_s(&scan_result, sizeof(alg_intf_det_ch_stats_result), 0,
            sizeof(alg_intf_det_ch_stats_result));
        if (memcpy_s(&scan_result, sizeof(alg_intf_det_ch_stats_result),
            msg->data, sizeof(alg_intf_det_ch_stats_result)) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_channel_statistics_complete::memcpy_s fail}");
        }
        (osal_void)memcpy_s(&(hal_device->chan_result), sizeof(wlan_scan_chan_stats_stru),
            &(scan_result.chan_result_total), sizeof(wlan_scan_chan_stats_stru));
        (osal_void)memcpy_s(&(hal_device->mac_ch_stats), sizeof(hal_ch_mac_statics_stru),
            &(scan_result.mac_ch_stats_total), sizeof(hal_ch_mac_statics_stru));
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        hmac_alg_anti_intf_get_tx_time(hal_device);
#endif
        if (hmac_alg_scan_ch_complete_notify(hal_device) != OAL_ERR_CODE_PTR_NULL) {
            hmac_scan_calcu_channel_ratio(hal_device);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取硬件信道统计 测量结果
*****************************************************************************/
osal_void hmac_scan_get_ch_statics_measurement_result(const hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device)
{
    hal_ch_statics_irq_event_stru    stats_result;
    osal_u8                        chan_idx;
    osal_u32                       trx_time_us;

    if ((hmac_device == OSAL_NULL) || (hal_device == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN,
            "{hmac_scan_get_ch_statics_measurement_result::hmac_device[%p], hal_device[%p]}",
            (uintptr_t)hmac_device, (uintptr_t)hal_device);
        return;
    }

    /* 读取结果 */
    (osal_void)memset_s(&stats_result, sizeof(stats_result), 0, sizeof(stats_result));
    hal_get_ch_statics_result(&stats_result);
    hal_get_ch_measurement_result(&stats_result);

    /* 当前的统计周期如果有PA关，则当前的统计周期不计算在内 */
    if (hal_device->intf_det_invalid == OSAL_TRUE) {
        if (hal_device->is_mac_pa_enabled == OSAL_TRUE) {
            hal_device->intf_det_invalid = OSAL_FALSE;
        }
        return;
    }

    chan_idx  = get_hal_dev_current_scan_idx(hal_device);

    /* 复制信道统计/测量结果信息 */
    hal_device->chan_result.stats_valid = 1;
    hal_device->chan_result.channel_number = hmac_device->scan_params.channel_list[chan_idx].chan_number;

    hal_device->chan_result.free_power_stats_20m  += (osal_s8)stats_result.pri20_idle_power; /* 主20M信道空闲功率 */
    hal_device->chan_result.free_power_stats_40m  += (osal_s8)stats_result.pri40_idle_power; /* 主40M信道空闲功率 */
    /* 全部80M信道空闲功率 */
    hal_device->chan_result.free_power_stats_80m  += (osal_s8)stats_result.pri80_idle_power;
    hal_device->chan_result.free_power_cnt += 1;

    trx_time_us = stats_result.ch_rx_time_us + stats_result.ch_tx_time_us;
    stats_result.pri20_free_time_us = hmac_scan_get_valid_free_time(trx_time_us,
        stats_result.ch_stats_time_us, stats_result.pri20_free_time_us);
    stats_result.pri40_free_time_us = hmac_scan_get_valid_free_time(trx_time_us,
        stats_result.ch_stats_time_us, stats_result.pri40_free_time_us);
    stats_result.sec20_free_time_us = hmac_scan_get_valid_free_time(trx_time_us,
        stats_result.ch_stats_time_us, stats_result.sec20_free_time_us);

    hal_device->chan_result.total_free_time_20m_us += stats_result.pri20_free_time_us;
    hal_device->chan_result.total_free_time_40m_us += stats_result.pri40_free_time_us;
    hal_device->chan_result.total_free_time_80m_us += stats_result.sec20_free_time_us;
    hal_device->chan_result.total_recv_time_us     += stats_result.ch_rx_time_us;
    hal_device->chan_result.total_send_time_us     += stats_result.ch_tx_time_us;
    hal_device->chan_result.total_stats_time_us    += stats_result.ch_stats_time_us;

    if (g_hmac_scan_rom_cb.hmac_scan_get_ch_statics_measurement_result != OSAL_NULL) {
        g_hmac_scan_rom_cb.hmac_scan_get_ch_statics_measurement_result(hal_device, &stats_result);
    }
}
/*****************************************************************************
 功能描述  : 获取信道繁忙度和空闲功率
*****************************************************************************/
osal_void hmac_scan_calcu_channel_ratio(hal_to_dmac_device_stru   *hal_device)
{
    wlan_scan_chan_stats_stru      *chan_result = OSAL_NULL;
    wlan_chan_ratio_stru           *chan_ratio = OSAL_NULL;

    /* 判断入参合法性 */
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_calcu_channel_ratio: input pointer is null!}");
        return;
    }

    chan_result = &(hal_device->chan_result);
    chan_ratio  = &(hal_device->chan_ratio);

    /* 获取统计的信道结果:繁忙度/千分之X */
    if (chan_result->total_stats_time_us == 0) {
        chan_ratio->chan_ratio_20m = 0;
        chan_ratio->chan_ratio_40m = 0;
        chan_ratio->chan_ratio_80m = 0;
    } else {
        chan_ratio->chan_ratio_20m =
            (osal_u16)hmac_scan_get_duty_cyc_ratio(chan_result, chan_result->total_free_time_20m_us);
        chan_ratio->chan_ratio_40m =
            (osal_u16)hmac_scan_get_duty_cyc_ratio(chan_result, chan_result->total_free_time_40m_us);
        chan_ratio->chan_ratio_80m =
            (osal_u16)hmac_scan_get_duty_cyc_ratio(chan_result, chan_result->total_free_time_80m_us);
    }

    /* 获取空闲功率 */
    if (chan_result->free_power_cnt == 0) {
        chan_ratio->free_power_20m = 0;
        chan_ratio->free_power_40m = 0;
        chan_ratio->free_power_80m = 0;
    } else {
        chan_ratio->free_power_20m =
            (osal_s8)(chan_result->free_power_stats_20m / (osal_s16)chan_result->free_power_cnt);
        chan_ratio->free_power_40m =
            (osal_s8)(chan_result->free_power_stats_40m / (osal_s16)chan_result->free_power_cnt);
        chan_ratio->free_power_80m =
            (osal_s8)(chan_result->free_power_stats_80m / (osal_s16)chan_result->free_power_cnt);
    }

    if (g_hmac_scan_rom_cb.hmac_scan_calcu_channel_ratio != OSAL_NULL) {
        g_hmac_scan_rom_cb.hmac_scan_calcu_channel_ratio(hal_device);
    }
}

/*****************************************************************************
 功能描述  : 打印扫描到的bss信息
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_print_scan_params(const mac_scan_req_stru *scan_params, hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_SCAN) != 0) {
        wifi_printf(
            "hmac_scan_print_scan_params::Now Scan channel_num[%d] in [%d]ms with scan_func[0x%x], ssid_num[%d]\r\n",
            scan_params->channel_nums, scan_params->scan_time, scan_params->scan_func, scan_params->ssid_num);

        wifi_printf(
            "hmac_scan_print_scan_params:rand_mac[%d],p2p_scan[%d],max_scan_count_per_channel[%d],back home[%d]\r\n",
            scan_params->is_random_mac_addr_scan, scan_params->is_p2p0_scan,
            scan_params->max_scan_count_per_channel, scan_params->need_switch_back_home_channel);

        wifi_printf(
            "hmac_scan_print_scan_params:mode[%d],work_time_on_home[%d],scan_channel_interval[%d],probe_req[%d]\r\n",
            scan_params->scan_mode, scan_params->work_time_on_home_channel,
            scan_params->scan_channel_interval, scan_params->max_send_probe_req_count_per_channel);
    }
#endif
    oam_warning_log4(0, OAM_SF_SCAN,
        "hmac_scan_print_scan_params::Now Scan channel_num[%d] in [%d]ms with scan_func[0x%x], and ssid_num[%d]!",
        scan_params->channel_nums, scan_params->scan_time, scan_params->scan_func, scan_params->ssid_num);

    oam_warning_log4(0, OAM_SF_SCAN,
        "hmac_scan_print_scan_params::random_mac[%d],p2p_scan[%d], max_scan_count_per_channel[%d], need back home[%d]!",
        scan_params->is_random_mac_addr_scan, scan_params->is_p2p0_scan,
        scan_params->max_scan_count_per_channel, scan_params->need_switch_back_home_channel);

    oam_warning_log4(0, OAM_SF_SCAN,
        "hmac_scan_print_scan_params:scan_mode[%d],work_time_on_home[%d],scan_channel_interval[%d],probe_req_count[%d]",
        scan_params->scan_mode, scan_params->work_time_on_home_channel,
        scan_params->scan_channel_interval, scan_params->max_send_probe_req_count_per_channel);
    return;
}

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
 功能描述  : 打印扫描到的bss信息
*****************************************************************************/
osal_void hmac_scan_print_scanned_bss_info_etc(osal_u8 device_id)
{
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_bss_mgmt_stru *bss_mgmt = OSAL_NULL;
    hmac_scanned_bss_info *scanned_bss = OSAL_NULL;
    mac_bss_dscr_stru *bss_dscr = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_print_scanned_bss_info_etc::hmac_device null.}");
        return;
    }

    /* 获取指向扫描结果的管理结构体地址 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历扫描到的bss信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);

        /* 仅显示新申请到的BSS帧 */
        if (scanned_bss->bss_dscr_info.new_scan_bss == OAL_TRUE) {
            scanned_bss->bss_dscr_info.new_scan_bss = OAL_FALSE;
           /* 上报beacon帧或者probe rsp帧 */
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_SCAN_FRAME_REPORT_HSO);
            if (fhook != OSAL_NULL) {
                ((dfx_scan_frame_report_hso_cb)fhook)(bss_dscr);
            }
        }
    }

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return;
}
#endif

/*****************************************************************************
 功能描述  : 申请内存，存储扫描到的bss信息
*****************************************************************************/

OAL_STATIC hmac_scanned_bss_info *hmac_scan_alloc_scanned_bss(osal_u32 mgmt_len)
{
    hmac_scanned_bss_info    *scanned_bss;
    osal_u32 size = (osal_u32)OAL_SIZEOF(hmac_scanned_bss_info) + mgmt_len -
        (osal_u8)OAL_SIZEOF(scanned_bss->bss_dscr_info.mgmt_buff);

    /* 申请内存，存储扫描到的bss信息 */
    scanned_bss = oal_memalloc(size);
    if (OAL_UNLIKELY(scanned_bss == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_SCAN,
            "{hmac_scan_alloc_scanned_bss::alloc memory failed for storing scanned result.}");
        return OAL_PTR_NULL;
    }

    /* 为申请的内存清零 */
    memset_s(scanned_bss, size, 0, size);

    /* 初始化链表头节点指针 */
    OSAL_INIT_LIST_HEAD(&(scanned_bss->dlist_head));

    return scanned_bss;
}

/*****************************************************************************
 功能描述  : 将扫描到的bss添加到链表
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_add_bss_to_list(hmac_scanned_bss_info *scanned_bss,
    hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru  *bss_mgmt;      /* 管理扫描结果的结构体 */
    struct osal_list_head *entry;
    struct osal_list_head *entry_tmp = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss_tmp;
    /* 是否已插入 */
    osal_bool has_insert = OSAL_FALSE;

    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    scanned_bss->bss_dscr_info.new_scan_bss = OAL_TRUE;

    /* 对链表写操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));
    if (bss_mgmt->bss_list_head.next == &(bss_mgmt->bss_list_head)) {
        osal_list_add_tail(&(scanned_bss->dlist_head), &(bss_mgmt->bss_list_head));
        goto out;
    }
    /* 添加扫描结果到链表中,按照rssi从大到小排序,并更新扫描到的bss计数 */
    osal_list_for_each_safe(entry, entry_tmp, &(bss_mgmt->bss_list_head)) {
        scanned_bss_tmp = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        /* 插入到第一个比自己rssi小的节点的前方, 确保链表是rssi从大到小排序 */
        if (scanned_bss->bss_dscr_info.c_rssi <= scanned_bss_tmp->bss_dscr_info.c_rssi) {
            continue;
        }
        osal_list_add(&(scanned_bss->dlist_head), entry->prev);
        has_insert = OSAL_TRUE;
        break;
    }
    /* 没插入,说明整个链表都是比自己rssi大的,此时应当插入尾部 */
    if (has_insert == OSAL_FALSE) {
        osal_list_add_tail(&(scanned_bss->dlist_head), &(bss_mgmt->bss_list_head));
    }
out:
    bss_mgmt->bss_num++;
    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 将扫描到的bss添加到链表
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_del_bss_from_list_nolock(hmac_scanned_bss_info *scanned_bss,
    hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru  *bss_mgmt;      /* 管理扫描结果的结构体 */

    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 从链表中删除节点，并更新扫描到的bss计数 */
    osal_list_del(&(scanned_bss->dlist_head));

    bss_mgmt->bss_num--;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 清除上次扫描请求相关的扫描记录信息: 包括扫描到的bss信息，并释放内存空间、以及其他信息清零
             并删除扫描超时定时器
*****************************************************************************/
osal_void hmac_scan_clean_scan(hmac_scan_stru *scan)
{
    hmac_scan_record_stru           *scan_record;
    struct osal_list_head             *entry;
    hmac_scanned_bss_info           *scanned_bss;
    hmac_bss_mgmt_stru              *bss_mgmt;
    osal_spinlock lock_backup;

    /* 参数合法性检查 */
    if (scan == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::scan is null.}");
        return;
    }

    scan_record = &scan->scan_record_mgmt;

    /* 1.一定要先清除扫描到的bss信息，再进行清零处理 */
    bss_mgmt = &(scan_record->bss_mgmt);

    /* 对链表写操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历链表，删除扫描到的bss信息 */
    while (osal_list_empty(&(bss_mgmt->bss_list_head)) == OAL_FALSE) {
        entry = bss_mgmt->bss_list_head.next; // first list entry
        osal_list_del(entry);
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);

        bss_mgmt->bss_num--;

        /* 释放扫描队列里的内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    osal_spin_unlock(&(bss_mgmt->lock));
    /* lock中有内存申请，此处不能被清零 */
    (osal_void)memcpy_s(&lock_backup, sizeof(lock_backup), &(bss_mgmt->lock), sizeof(bss_mgmt->lock));

    /* 2.其它信息清零 */
    memset_s(scan_record, OAL_SIZEOF(hmac_scan_record_stru), 0, OAL_SIZEOF(hmac_scan_record_stru));
    scan_record->scan_rsp_status = MAC_SCAN_STATUS_BUTT;     /* 初始化扫描完成时状态码为无效值 */
    scan_record->vap_last_state  = MAC_VAP_STATE_BUTT;       /* 必须置BUTT,否则aput停扫描会vap状态恢复错 */

    /* 还原lock信息，lock在device去初始化时（即hmac_scan_exit_etc中）释放 */
    (osal_void)memcpy_s(&(bss_mgmt->lock), sizeof(bss_mgmt->lock), &lock_backup, sizeof(lock_backup));

    /* 3.重新初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_record->bss_mgmt);
    OSAL_INIT_LIST_HEAD(&(bss_mgmt->bss_list_head));

    /* 4.删除扫描超时定时器 */
    if (scan->scan_timeout.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(scan->scan_timeout));
    }

    oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::cleaned scan record success.}");

    return;
}


/*****************************************************************************
 功能描述  : 判断输入bssid参数是否是关联的AP的bssid,用于不老化已经关联的AP
*****************************************************************************/
OAL_STATIC osal_s32 hmac_is_connected_ap_bssid(osal_u8 device_id, osal_u8 bssid[WLAN_MAC_ADDR_LEN])
{
    osal_u8                    vap_idx;
    hmac_vap_stru               *hmac_vap;
    hmac_device_stru            *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_is_connected_ap_bssid::hmac_res_get_mac_dev_etc return null.}");
        return OAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{hmac_is_connected_ap_bssid::mac_res_get_hmac_vap fail! vap id is %d}",
                             hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (is_legacy_vap(hmac_vap) &&
            (hmac_vap->vap_state == MAC_VAP_STATE_UP)) {
            if (osal_memcmp(bssid, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) == 0) {
                /* 不老化当前关联的AP */
                oam_info_log1(0, OAM_SF_SCAN, "vap_id[%d]", hmac_vap->vap_id);
                oam_info_log4(0, OAM_SF_SCAN,
                    "{hmac_is_connected_ap_bssid::connected AP bssid:%02X:%02X:%02X:%02X:XX:XX}",
                    /* 0:1:2:3:数组下标 */
                    bssid[0], bssid[1], bssid[2], bssid[3]);

                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 根据入参删除扫描链表元素, expire_time 表示删除超过这个时间的链表元素
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_clean_expire_scanned_bss(hmac_vap_stru *hmac_vap,
    hmac_scan_record_stru *scan_record, osal_u32 expire_time)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *entry_tmp = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *bss_mgmt = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;
    osal_u32 curr_time_stamp;
    unref_param(hmac_vap);

    /* 参数合法性检查 */
    if (scan_record == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_expire_scanned_bss::scan record is null.}");
        return;
    }

    /* 管理扫描的bss结果的结构体 */
    bss_mgmt = &(scan_record->bss_mgmt);

    curr_time_stamp = (osal_u32)oal_time_get_stamp_ms();

    /* 对链表写操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历链表，删除上一次扫描结果中到期的bss信息 */
    osal_list_for_each_safe(entry, entry_tmp, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr = &(scanned_bss->bss_dscr_info);

        /* 保留加入链表expire_time时间之内的链表元素 */
        if (curr_time_stamp - bss_dscr->timestamp < expire_time) {
            oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_expire_scanned_bss::do not remove the BSS not expired.}");
            continue;
        }

        /* 不老化当前正在关联的AP */
        if (hmac_is_connected_ap_bssid(scan_record->device_id, bss_dscr->bssid) != 0) {
            continue;
        }

        /* 从链表中删除节点，并更新扫描到的bss计数 */
        osal_list_del(&(scanned_bss->dlist_head));
        bss_mgmt->bss_num--;

        /* 释放对应内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    osal_spin_unlock(&(bss_mgmt->lock));
    return;
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_CFGID_DEBUG
OSAL_STATIC osal_s32 hmac_config_set_not_scan_flag(hmac_vap_stru *hmac_vap, mac_cfg_set_scan_param *scan_param)
{
    oam_warning_log2(0, OAM_SF_CFG,
        "{hmac_config_set_not_scan_flag:vap_id[%d] scan flag[%d].}", hmac_vap->vap_id, scan_param->value);

    if (scan_param->value > 1) { /* 1个bit只能支持 0 和 1 */
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    /* 对该变量赋值为1, 关联状态下 不进行扫描. 配置为0时 恢复扫描; 86是复用扫描时间配置字段 */
    /* AT+CCPRIV=wlan0,set_scan_param,8,1 */
    hmac_vap->not_scan_flag = (osal_u8)scan_param->value;
    return OAL_SUCC;
}

osal_s32 hmac_config_set_scan_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    mac_cfg_set_scan_param *scan_param = (mac_cfg_set_scan_param *)msg->data;

    switch (scan_param->type) {
        case SCAN_PARAM_COUNT:
            hmac_vap->max_scan_count_per_channel = (osal_u8)scan_param->value;
            break;
        case SCAN_PARAM_STAY_TIME:
            hmac_vap->scan_time = scan_param->value;
            break;
        case SCAN_PARAM_ON_HOME_TIME:
            hmac_vap->work_time_on_home_channel = scan_param->value;
            break;
        case SCAN_PARAM_CHANNEL_LIST:
            hmac_config_set_channel_list(hmac_vap, scan_param);
            break;
        case SCAN_PARAM_INTERVAL:
            if (scan_param->value > OAL_ARRAY_SIZE(scan_param->chan_list)) {
                oam_warning_log1(0, OAM_SF_CFG, "hmac_config_set_scan_param::scan interval err[%d]", scan_param->value);
                return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }
            hmac_vap->scan_channel_interval = (osal_u8)scan_param->value;
            break;
        case SCAN_PARAM_SCAN_MODE:
            if (scan_param->value >= WLAN_SCAN_TYPE_BUTT) {
                oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_scan_param::type error[%d].}", scan_param->value);
                return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }
            /* scan_type 扫描模式, 0-主动模式, 1-被动模式, 供ccpriv命令使用, 跟正常宏定义的值刚好相反 */
            /* AT+CCPRIV=wlan0,set_scan_param,7,1 */
            hmac_vap->scan_type = (osal_u8)scan_param->value;
            break;
        case SCAN_PARAM_NOT_SCAN:
            if (hmac_config_set_not_scan_flag(hmac_vap, scan_param) != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;
        case SCAN_PARAM_DEL_SCAN_LIST:
            /* AT+CCPRIV=wlan0,set_scan_param,9,1 实际上只用到第一个参数9,第二个参数未使用 */
            hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
            if (hmac_device == OAL_PTR_NULL) {
                break;
            }
            hmac_scan_clean_expire_scanned_bss(hmac_vap, &(hmac_device->scan_mgmt.scan_record_mgmt), 0);
            oam_info_log1(0, OAM_SF_ANY, "vap[%d] hmac_scan_clean_expire_scanned_bss by cmd.", hmac_vap->vap_id);
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hmac_config_set_scan_param: unvalid type[%d]}", scan_param->type);
            break;
    }

    return OAL_SUCC;
}
#endif
/*****************************************************************************
 功能描述  : 根据bss index查找对应的bss dscr结构信息
*****************************************************************************/
#ifdef _PRE_WIFI_DMT
mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index_etc(osal_u8  device_id, osal_u32 bss_index)
{
    struct osal_list_head             *entry;
    hmac_scanned_bss_info           *scanned_bss;
    hmac_device_stru                *hmac_device;
    hmac_bss_mgmt_stru              *bss_mgmt;
    osal_u8                        loop;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::hmac_device is null.}");
        return OAL_PTR_NULL;
    }

    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 对链表删操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 如果索引大于总共扫描的bss个数，返回异常 */
    if (bss_index >= bss_mgmt->bss_num) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::no such bss in bss list!}");

        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        return OAL_PTR_NULL;
    }

    loop = 0;
    /* 遍历链表，返回对应index的bss dscr信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);

        /* 相同的bss index返回 */
        if (bss_index == loop) {
            /* 解锁 */
            osal_spin_unlock(&(bss_mgmt->lock));
            return &(scanned_bss->bss_dscr_info);
        }

        loop++;
    }
    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return OAL_PTR_NULL;
}
#endif

/*****************************************************************************
 功能描述  : 查找相同的bssid的bss是否出现过
*****************************************************************************/
hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid_etc(hmac_bss_mgmt_stru *bss_mgmt, const osal_u8 *bssid)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;

    if (bss_mgmt == OAL_PTR_NULL || bssid == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_bssid_etc::param null.}");
        return OAL_PTR_NULL;
    }

    /* 遍历链表，查找链表中是否已经存在相同bssid的bss信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        if (scanned_bss == OAL_PTR_NULL) {
            continue;
        }
        /* 相同的bssid地址 */
        if (oal_compare_mac_addr(scanned_bss->bss_dscr_info.bssid, bssid) == 0) {
            return scanned_bss;
        }
    }

    return OAL_PTR_NULL;
}

/*****************************************************************************
 功能描述  : 根据mac地址获取扫描结果中的bss信息
*****************************************************************************/
osal_void *hmac_scan_get_scanned_bss_by_bssid(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    hmac_bss_mgmt_stru             *bss_mgmt;          /* 管理扫描的bss结果的结构体 */
    hmac_scanned_bss_info          *scanned_bss_info;
    hmac_device_stru               *hmac_device;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::hmac_device is null, dev id[%d].}",
                         hmac_vap->device_id);
        return OAL_PTR_NULL;
    }

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    osal_spin_lock(&(bss_mgmt->lock));

    scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, mac_addr);
    if (scanned_bss_info == OAL_PTR_NULL) {
        oam_warning_log4(0, OAM_SF_SCAN,
                         "{hmac_scan_get_scanned_bss_by_bssid::find the bss failed[%02X:%02X:%02X:%02X:XX:XX]}",
                         /* 0:1:2:3:数组下标 */
                         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);

        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        return OAL_PTR_NULL;
    }

    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return &(scanned_bss_info->bss_dscr_info);
}

/*****************************************************************************
 功能描述  : 更新wmm相关信息
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_scan_update_bss_list_wmm(mac_bss_dscr_stru   *bss_dscr,
    osal_u8           *frame_body,
    osal_u16           frame_len)
{
    osal_u8  *ie;

    bss_dscr->wmm_cap   = OAL_FALSE;
    bss_dscr->uapsd_cap = OAL_FALSE;

    ie = hmac_get_wmm_ie_etc(frame_body, frame_len);
    if (ie != OAL_PTR_NULL) {
        bss_dscr->wmm_cap = OAL_TRUE;

        /* ---------------------------------------------------------------------------------  */
        /* WMM Information/Parameter Element Format                                           */
        /* ---------------------------------------------------------------------------------- */
        /* EID | IE LEN | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
        /* ---------------------------------------------------------------------------------  */
        /* 1   |   1    |  3  | 1       | 1          | 1       | 1       | ---------------- | */
        /* ---------------------------------------------------------------------------------  */
        /* ie[1] IE len 不包含EID和LEN字段,获取QoSInfo，uc_ie_len必须大于7字节长度 */
        /* Check if Bit 7 is set indicating U-APSD capability */
        if ((ie[1] >= 7) && ((ie[8] & BIT7) != 0)) { /* wmm ie的第8个字节是QoS info字节 */
            bss_dscr->uapsd_cap = OAL_TRUE;
        }
    } else {
        ie = mac_find_ie_etc(MAC_EID_HT_CAP, frame_body, frame_len);
        if (ie != OAL_PTR_NULL) {
            /* 仅配置11ac、11a,不配11n，beacon无WMM信息，
            有HT信息short GI for 20M为0，以支持qos关联，eapol帧在VO队列发送不出去 */
            /* -------------------------------------------------------------- */
            /* HT capability Information/Parameter Element Format             */
            /* -------------------------------------------------------------- */
            /* EID | IE LEN |  HT capability Info |                 based   | */
            /* -------------------------------------------------------------- */
            /* 1   |   1    |         2           | ------------------------| */
            /* -------------------------------------------------------------- */
            /* ie[1] IE len 不包含EID和LEN字段,获取HT cap Info，uc_ie_len必须大于2字节长度 */
            /* ht cap的第 2,3个字节是HT capability Info信息 */
            /* Check if Bit 5 is set indicating short GI for 20M capability */
            if ((ie[1] >= 2) && ((ie[2] & BIT5) != 0)) {
                bss_dscr->wmm_cap = OAL_TRUE;
            }
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11D
/*****************************************************************************
 功能描述  : 解析country IE
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_update_bss_list_country(mac_bss_dscr_stru   *bss_dscr,
    osal_u8           *frame_body,
    osal_u16           frame_len)
{
    osal_u8 *ie;

    ie = mac_find_ie_etc(MAC_EID_COUNTRY, frame_body, frame_len);
    /* 国家码不存在, 全部标记为0 */
    if (ie == OSAL_NULL) {
        bss_dscr->ac_country[0] = 0;
        bss_dscr->ac_country[1] = 0;
        bss_dscr->ac_country[2] = 0; /* 2数组下标 */
        return;
    } else {
        bss_dscr->country_ie = ie;
    }
    /* 国家码采用2个字节,IE LEN必须大于等于2 */
    if (ie[1] >= 2) {
        bss_dscr->ac_country[0] = (osal_s8)ie[MAC_IE_HDR_LEN];
        bss_dscr->ac_country[1] = (osal_s8)ie[MAC_IE_HDR_LEN + 1];
        bss_dscr->ac_country[2] = 0; /* 2数组下标 */
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
/*****************************************************************************
 功能描述  : 判断对方是否携带1024qam
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_update_bss_list_1024qam(mac_bss_dscr_stru *bss_dscr,
    osal_u8         *frame_body,
    osal_u16         frame_len)
{
    osal_u8 *ie;
    ie = hmac_find_vendor_ie_etc(MAC_EXT_VENDER_IE, MAC_OSAL_1024QAM_IE, frame_body, frame_len);
    if (ie == OAL_PTR_NULL) {
        bss_dscr->support_1024qam = OAL_FALSE;
    } else {
        bss_dscr->support_1024qam = OAL_TRUE;
    }
}
#endif

/*****************************************************************************
 功能描述  : 更新11n相关信息
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_update_11i(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    bss_dscr->rsn_ie = mac_find_ie_etc(MAC_EID_RSN, frame_body, (osal_s32)(frame_len));
    bss_dscr->wpa_ie =
        hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, frame_body, (osal_s32)(frame_len));
#ifdef _PRE_WLAN_WIRELESS_EXT
    bss_dscr->wps_ie =
        hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPS, frame_body, (osal_s32)(frame_len));
    bss_dscr->wapi_ie = mac_find_ie_etc(MAC_EID_WAPI, frame_body, (osal_s32)(frame_len));
#endif
}

/*****************************************************************************
 功能描述  : 更新11n相关信息
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_update_bss_list_11n(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8               *ie;
    mac_ht_opern_stru       *ht_op;
    osal_u8                sec_chan_offset;
    wlan_bw_cap_enum_uint8   ht_cap_bw = WLAN_BW_CAP_20M;
    wlan_bw_cap_enum_uint8   ht_op_bw  = WLAN_BW_CAP_20M;

    /* 11n */
    ie = mac_find_ie_etc(MAC_EID_HT_CAP, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= 2)) { /* 2 ie长度 */
        /* ie[2]是HT Capabilities Info的第1个字节 */
        bss_dscr->ht_capable = OAL_TRUE;     /* 支持ht */
        bss_dscr->ht_ldpc = (ie[2] & BIT0);           /* ie[2] BIT0 支持ldpc */
        ht_cap_bw = ((ie[2] & BIT1) >> 1);                /* ie[2] BIT1 取出支持的带宽 */
        bss_dscr->ht_stbc = ((ie[2] & BIT7) >> 7); /* 7 右移位数;ie[2] BIT7 */
#ifdef _PRE_WLAN_WIRELESS_EXT
        bss_dscr->ht_cap_info = *(osal_u16 *)(ie + 2); // 偏移量2，开始就是其内容
#endif
    }

    /* 默认20M,如果帧内容未携带HT_OPERATION则可以直接采用默认值 */
    bss_dscr->channel_bandwidth = WLAN_BAND_WIDTH_20M;

    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= 2)) { // 2 增加ie长度异常检查
        ht_op  = (mac_ht_opern_stru *)(ie + MAC_IE_HDR_LEN);

        /* 提取次信道偏移 */
        sec_chan_offset = ht_op->secondary_chan_offset;

        /* 防止ap的channel width=0, 但channel offset = 1或者3 此时以channel width为主 */
        /* ht cap 20/40 enabled && ht operation 40 enabled */
        if ((ht_op->sta_chan_width != 0) && (ht_cap_bw > WLAN_BW_CAP_20M)) { // cap > 20M才取channel bw
            if (sec_chan_offset == MAC_SCB) {
                bss_dscr->channel_bandwidth =  WLAN_BAND_WIDTH_40MINUS;
                ht_op_bw = WLAN_BW_CAP_40M;
            } else if (sec_chan_offset == MAC_SCA) {
                bss_dscr->channel_bandwidth =  WLAN_BAND_WIDTH_40PLUS;
                ht_op_bw = WLAN_BW_CAP_40M;
            }
        }
    }

    /* 将AP带宽能力取声明能力的最小值，防止AP异常发送超过带宽能力数据，造成数据不通 */
    bss_dscr->bw_cap = OAL_MIN(ht_cap_bw, ht_op_bw);
#ifdef _PRE_WLAN_WIRELESS_EXT
    bss_dscr->real_bw = (bss_dscr->channel_bandwidth == WLAN_BAND_WIDTH_20M) ? WLAN_BW_CAP_20M : WLAN_BW_CAP_40M;
#endif
    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= 1)) {
        /* Extract 20/40 BSS Coexistence Management Support */
        bss_dscr->coex_mgmt_supp = (ie[2] & BIT0);
    }
}

/*****************************************************************************
 功能描述  : 更新11ac相关bss信息
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_update_bss_list_11ac(mac_bss_dscr_stru *bss_dscr,
    osal_u8 *frame_body, osal_u16 frame_len, osal_ulong is_vendor_ie)
{
    osal_u8 vht_chan_width, chan_center_freq, supp_ch_width, index, offset;

    osal_u8 *ie = mac_find_ie_etc(MAC_EID_VHT_CAP, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= MAC_VHT_CAP_IE_LEN)) {
        bss_dscr->vht_capable = OAL_TRUE;     /* 支持vht */

        /* 说明vendor中携带VHT ie，则设置标志位，assoc req中也需携带vendor+vht ie */
        if (is_vendor_ie == OAL_TRUE) {
            bss_dscr->vendor_vht_capable = OAL_TRUE;
        }

        /* 提取Supported Channel Width Set */
        supp_ch_width = ((ie[2] & (BIT3 | BIT2)) >> 2); /* 2 ie数组下标 */

        if (supp_ch_width == 0) {
            bss_dscr->bw_cap = WLAN_BW_CAP_80M;   /* 80MHz */
        } else if (supp_ch_width == 1) {
            bss_dscr->bw_cap = WLAN_BW_CAP_160M;  /* 160MHz */
        } else if (supp_ch_width == 2) { /* 2 Channel Width Set */
            bss_dscr->bw_cap = WLAN_BW_CAP_80M;   /* 80MHz */
        }
    } else {
        /* 私有vendor中不包含vht ie，适配BCM 5g 20M 私有协议 */
        if (is_vendor_ie == OAL_TRUE) {
            bss_dscr->vendor_novht_capable = OAL_TRUE;
        }
    }

    ie = mac_find_ie_etc(MAC_EID_VHT_OPERN, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= MAC_VHT_OPERN_LEN)) {
        vht_chan_width   = ie[2]; /* 2 ie数组下标 */
        chan_center_freq = ie[3]; /* 3 ie数组下标 */

        /* 更新带宽信息 */
        if (vht_chan_width != 1 && vht_chan_width != 3) { /* 1:80MHz 或者 3:80+80MHz */
            return;
        }
        offset = chan_center_freq - bss_dscr->st_channel.chan_number;
        for (index = 0; index < 4; index++) { /* 4 中心频率相对于主20偏移情况总数 */
            if (offset == g_chan_offset_bandwidth[index].channel_num_offset) {
                bss_dscr->channel_bandwidth = g_chan_offset_bandwidth[index].bandwidth;
            }
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC osal_u32 hmac_scan_update_bss_list_11ax_he_cap(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8 *ie;
    mac_frame_he_cap_ie_stru he_cap_value;
    osal_u32 ul_ret;

    /* HE CAP */
    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, frame_body, frame_len);
    if ((ie == OAL_PTR_NULL) || (ie[1] < MAC_HE_CAP_MIN_LEN)) {
        return OAL_SUCC;
    }

    memset_s(&he_cap_value, OAL_SIZEOF(he_cap_value), 0, OAL_SIZEOF(he_cap_value));
    /* 解析固定长度部分:MAC_Cap+PHY Cap + HE-MCS NSS(<=80MHz) */
    ul_ret = hmac_ie_parse_he_cap(ie, &he_cap_value);
    if (ul_ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 如果对端不支持LDCP,则降为不支持11AX,eg.AX86U兼容性问题 */
    if (he_cap_value.he_phy_cap.ldpc_coding_in_paylod == OAL_FALSE) {
        bss_dscr->he_capable = OAL_FALSE;
    } else {
        bss_dscr->he_capable = OAL_TRUE; /* 支持HE */
    }

    if (bss_dscr->st_channel.band == WLAN_BAND_2G) {
        /* 2G */
        if ((he_cap_value.he_phy_cap.channel_width_set & 0x1) != 0) {
            /* Bit0 2G 是否支持40MHz */
            bss_dscr->bw_cap = WLAN_BW_CAP_40M;
        }
    } else {
        if ((he_cap_value.he_phy_cap.channel_width_set & 0x8) != 0) { /* B3-5G 80+80MHz */
            bss_dscr->bw_cap = WLAN_BW_CAP_80PLUS80;
        } else if ((he_cap_value.he_phy_cap.channel_width_set & 0x4) != 0) { /* B2-160MHz */
            bss_dscr->bw_cap = WLAN_BW_CAP_160M;
        } else if ((he_cap_value.he_phy_cap.channel_width_set & 0x2) != 0) { /* B2-5G支持80MHz */
            bss_dscr->bw_cap = WLAN_BW_CAP_80M;
        } else {
            bss_dscr->bw_cap = WLAN_BW_CAP_20M;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_scan_update_bss_list_11ax_he_oper(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8 *ie;
    mac_frame_he_oper_ie_stru he_oper_ie_value;
    osal_u32 ul_ret;
    osal_u8 index, offset;

    /* HE Oper */
    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_OPERATION, frame_body, frame_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= MAC_HE_OPERAION_MIN_LEN)) {
        memset_s(&he_oper_ie_value, OAL_SIZEOF(he_oper_ie_value), 0, OAL_SIZEOF(he_oper_ie_value));
        ul_ret = hmac_ie_parse_he_oper(ie, &he_oper_ie_value);
        if (ul_ret != OAL_SUCC) {
            return OAL_FAIL;
        }

        if (he_oper_ie_value.he_oper_param.vht_operation_info_present != 1) {
            return OAL_SUCC;
        }

        if (he_oper_ie_value.vht_operation_info.channel_width != 1) {
            return OAL_SUCC;
        }
        /* 80MHz */
        offset = he_oper_ie_value.vht_operation_info.center_freq_seg0 - bss_dscr->st_channel.chan_number;
        for (index = 0; index < 4; index++) { /* 4 中心频率相对于主20偏移情况总数 */
            if (offset == g_chan_offset_bandwidth[index].channel_num_offset) {
                bss_dscr->channel_bandwidth = g_chan_offset_bandwidth[index].bandwidth;
            }
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 更新11ax相关信息
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_update_bss_list_11ax(mac_bss_dscr_stru   *bss_dscr,
    osal_u8           *frame_body,
    osal_u16           frame_len)
{
    osal_u32 ul_ret;

    ul_ret = hmac_scan_update_bss_list_11ax_he_cap(bss_dscr, frame_body, frame_len);
    if (ul_ret != OAL_SUCC) {
        return;
    }

    hmac_scan_update_bss_list_11ax_he_oper(bss_dscr, frame_body, frame_len);
    return;
}

#endif

/*****************************************************************************
 功能描述  : 更新协议类 bss信息
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  hmac_scan_update_bss_list_protocol(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body, osal_u16 frame_len)
{
    osal_u8 *ie;
    osal_void *fhook = OAL_PTR_NULL;
    osal_u16 offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
    osal_u8 *fra_body = frame_body;
    osal_u16 fra_len = frame_len;

    fra_body += MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    fra_len   -= MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /**************************************************************************/
    /*                       Beacon Frame - Frame Body                        */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                               */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                               */
    /**************************************************************************/

    /* wmm */
    hmac_scan_update_bss_list_wmm(bss_dscr, fra_body, fra_len);

    /* 11i */
    hmac_scan_update_11i(bss_dscr, fra_body, fra_len);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 11d */
    hmac_scan_update_bss_list_country(bss_dscr, fra_body, fra_len);
#endif

    /* 11n */
    hmac_scan_update_bss_list_11n(bss_dscr, fra_body, fra_len);

    /* rrm */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_UPDATE_BSS_LIST_RRM);
    if (fhook != OSAL_NULL) {
        ((hmac_scan_update_bss_list_rrm_cb)fhook)(bss_dscr, fra_body, fra_len);
    }

    /* 11ac */
    hmac_scan_update_bss_list_11ac(bss_dscr,  fra_body, fra_len, OAL_FALSE);

    /* 11ax */
#ifdef _PRE_WLAN_FEATURE_11AX
    if (is_11ax_vap(hmac_vap)) {
        hmac_scan_update_bss_list_11ax(bss_dscr, fra_body, fra_len);
    }
#endif
    /* 查找私有vendor ie */
    ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE, fra_body, fra_len);
    if ((ie != OAL_PTR_NULL) && (ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        hmac_scan_update_bss_list_11ac(bss_dscr, ie + offset_vendor_vht,
            ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER, OAL_TRUE);
    }

#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_scan_update_bss_list_1024qam(bss_dscr, fra_body, fra_len);
#endif

    /* 检测AP是否带含有assoc disallowed attr的MBO IE */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_UPDATE_IS_ASSOC_ALLOWED);
    if (fhook != OSAL_NULL) {
        ((hmac_scan_update_bss_assoc_disallowed_attr_cb)fhook)(hmac_vap, bss_dscr, fra_body, fra_len);
    }

#ifdef _PRE_WLAN_MP13_DDC_BUGFIX
    ie = hmac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_ATHEROSC, MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC, fra_body, fra_len);
    bss_dscr->ddc_whitelist_chip_oui = (ie != OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_UPDATE_BSS_LIST_PROTOCOL);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_update_bss_list_protocol_cb)fhook)(bss_dscr, fra_body, fra_len);
    }
}

/*****************************************************************************
 功能描述  : 检查速率
*****************************************************************************/
OAL_STATIC osal_u8  hmac_scan_check_bss_supp_rates_etc(hmac_device_stru *hmac_device,
    osal_u8 *rate, osal_u8 bss_rate_num, osal_u8 *update_rate, osal_u8 update_rate_max_num)
{
    mac_data_rate_stru            *rates;
    osal_u32                     i, j, k;
    osal_u8                      rate_num = 0;

    rates   = hmac_device_get_all_rates_etc(hmac_device);

    for (i = 0; i < bss_rate_num; i++) {
        for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
            if ((is_equal_rates(rates[j].mac_rate, rate[i]) == 0) || (rate_num >= update_rate_max_num)) {
                continue;
            }
            /* 去除重复速率 */
            for (k = 0; k < rate_num; k++) {
                if (is_equal_rates(update_rate[k], rate[i])) {
                    break;
                }
            }
            /* 当不存在重复速率时，k等于rate_num */
            if (k == rate_num) {
                update_rate[rate_num++] = rate[i];
            }

            break;
        }
    }

    return rate_num;
}

/*****************************************************************************
 功能描述  : support_rates和extended_rates去除重复速率，一并合入扫描结果的速率集中
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_rm_repeat_sup_exsup_rates(mac_bss_dscr_stru  *bss_dscr, osal_u8 *rates,
    osal_u8 exrate_num)
{
    int i, j;
    for (i = 0; i < exrate_num; i++) {
        /* 去除重复速率 */
        for (j = 0; j < bss_dscr->num_supp_rates; j++) {
            if (is_equal_rates(rates[i], bss_dscr->supp_rates[j])) {
                break;
            }
        }

        /* 只有不存在重复速率时，j等于bss_dscr->num_supp_rates */
        if (j == bss_dscr->num_supp_rates && bss_dscr->num_supp_rates < WLAN_USER_MAX_SUPP_RATES) {
            bss_dscr->supp_rates[bss_dscr->num_supp_rates++] = rates[i];
        }
    }
}

/*****************************************************************************
 功能描述  : 更新扫描到bss的速率集
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32 hmac_scan_update_bss_list_rates(mac_bss_dscr_stru *bss_dscr,
    osal_u8         *frame_body,
    osal_u16         frame_len)
{
    osal_u8   *ie;
    osal_u8    num_rates    = 0;
    osal_u8    num_ex_rates;
    osal_u8    offset;
    osal_u8    rates[MAC_DATARATES_PHY_80211G_NUM] = {0};

    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 设置Beacon帧的field偏移量 */
    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    ie = mac_find_ie_etc(MAC_EID_RATES, frame_body + offset, frame_len - offset);
    if (ie != OAL_PTR_NULL) {
        num_rates = hmac_scan_check_bss_supp_rates_etc(hmac_device, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        /* 斐讯FIR304商用AP 11g模式，发送的支持速率集个数为12，
           不符合协议规定，为增加兼容性，修改判断分支为可选速率集上限16个 */
        if (num_rates > WLAN_USER_MAX_SUPP_RATES) {
            oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates::num_rates=%d.}", num_rates);
            num_rates = WLAN_USER_MAX_SUPP_RATES;
        }

        if (memcpy_s(bss_dscr->supp_rates, sizeof(bss_dscr->supp_rates), rates, num_rates) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_scan_update_bss_list_rates::memcpy_s error}");
        }
        bss_dscr->num_supp_rates = num_rates;
    }

    ie = mac_find_ie_etc(MAC_EID_XRATES, frame_body + offset, frame_len - offset);
    if (ie != OAL_PTR_NULL) {
        num_ex_rates =
            hmac_scan_check_bss_supp_rates_etc(hmac_device, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        if (num_rates + num_ex_rates > WLAN_USER_MAX_SUPP_RATES) { /* 超出支持速率个数 */
            oam_warning_log2(0, OAM_SF_SCAN,
                "{hmac_scan_update_bss_list_rates::number of rates too large, num_rates=%d, num_ex_rates=%d.}",
                num_rates, num_ex_rates);
        }

        if (num_ex_rates > 0) {
            /* support_rates和extended_rates去除重复速率，一并合入扫描结果的速率集中 */
            hmac_scan_rm_repeat_sup_exsup_rates(bss_dscr, rates, num_ex_rates);
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
功能描述:获取802.11新增的element,并将它们存到扫描到的BSS描述结构体中
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_get_11ax_elements(mac_bss_dscr_stru       *bss_dscr,
    osal_u8                 *frame_body,
    osal_u16                 frame_body_len)
{
    /* 解析 Multiple BSSID Configuration */
    bss_dscr->multiple_bssid_configuration.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->multiple_bssid_configuration.element_body_len,
        MAC_EID_EXT_MULTIPLE_BSSID_CONFIGURATION);
    /* 解析 HE Capabilities */
    bss_dscr->he_capabilities.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->he_capabilities.element_body_len,
        MAC_EID_EXT_HE_CAP);
    if (bss_dscr->he_capabilities.element_body != OSAL_NULL) {
        bss_dscr->he_capable = OSAL_TRUE;
    }
    /* 解析 HE Operation */
    bss_dscr->he_operation.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->he_operation.element_body_len,
        MAC_EID_EXT_HE_OPERATION);
    /* 解析 TWT: 暂不解析，不知道其Element ID */
    /* 解析 UORA Parameter Set */
    bss_dscr->uora_parameter_set.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->uora_parameter_set.element_body_len,
        MAC_EID_EXT_UORA_PARAMETER_SET);
    /* 解析 BSS Color Change Announcement */
    bss_dscr->bss_color_change_announcement.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->bss_color_change_announcement.element_body_len,
        MAC_EID_EXT_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT);
    /* 解析 Spatial Reuse */
    bss_dscr->spatial_reuse.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->spatial_reuse.element_body_len, MAC_EID_EXT_HE_SRP);
    /* 解析 MU EDCA Parameter Set */
    bss_dscr->mu_edca_parameter_set.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->mu_edca_parameter_set.element_body_len, MAC_EID_EXT_HE_EDCA);
    /* 解析 ESS Report */
    bss_dscr->ess_report.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->ess_report.element_body_len, MAC_EID_EXT_ESS_REPORT);
    /* 解析 NDP Feedback Report Parameter Set */
    bss_dscr->ndp_feedback_report_parameter_set.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->ndp_feedback_report_parameter_set.element_body_len,
        MAC_EID_EXT_NFR_PARAM_SET);
    /* 解析 HE BSS Load */
    bss_dscr->he_bss_load.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->he_bss_load.element_body_len, MAC_EID_EXT_HE_BSS_LOAD);
    /* 解析 HE 6GHz Band Capabilities */
    bss_dscr->he_6ghz_band_capabilities.element_body = mac_get_ext_ie_body(frame_body,
        (osal_s32)frame_body_len, &bss_dscr->he_6ghz_band_capabilities.element_body_len,
        MAC_EID_EXT_HE_6GHZ_BAND_CAPABILITIES);
}
#endif

OAL_STATIC osal_u8 hmac_get_multi_bssid_capa(osal_u8 *frame_body, osal_u16 frame_body_len)
{
    mac_ext_cap_ie_stru *ext_cap = OSAL_NULL;
    osal_u8 *ie = OSAL_NULL;
    osal_u8 *fra_body = frame_body;
    osal_u16 fra_len = frame_body_len;

    fra_body     += MAC_DEVICE_BEACON_OFFSET;
    fra_len -= MAC_DEVICE_BEACON_OFFSET;

    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, fra_body, fra_len);
    if ((ie != OSAL_NULL) && (ie[1] > EXT_CAPA_GET_MULTI_BSSID_LEN)) { /* 帧体大于2byte，才包含Multiple BSSID能力位 */
        ext_cap = (mac_ext_cap_ie_stru *)(&(ie[MAC_IE_HDR_LEN]));
        return ext_cap->multiple_bssid;
    } else {
        return OSAL_FALSE;
    }
}
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
static osal_u32 hmac_scan_proc_check_prefix_ssid(const oal_cfg80211_ssid_stru *req_ssid,
    const osal_u8 *ssid, osal_u8 ssid_len)
{
    osal_u8 req_ssid_len;
    osal_u8 loop;
    if (OAL_UNLIKELY(ssid == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_check_prefix_ssid::ssid null.}");
        return OAL_FAIL;
    }
    req_ssid_len = req_ssid[0].ssid_len;
    if (req_ssid_len <= ssid_len) {
        for (loop = 0; loop < req_ssid_len; loop++) {
            if (req_ssid[0].ssid[loop] != ssid[loop]) {
                break;
            }
        }
        if (loop == req_ssid_len) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
#endif

OAL_STATIC oal_net_device_stru* hmac_scan_proc_get_dev(hmac_scan_stru *scan_mgmt)
{
    oal_net_device_stru *netdev = OAL_PTR_NULL;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    netdev = (scan_mgmt->request->wdev == OAL_PTR_NULL ? OAL_PTR_NULL : scan_mgmt->request->wdev->netdev);
#else
    netdev = scan_mgmt->request->dev;
#endif
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    netdev = (scan_mgmt->request->wdev == OAL_PTR_NULL ? OAL_PTR_NULL : scan_mgmt->request->wdev->netdev);
#endif

    return netdev;
}

osal_u32 hmac_scan_proc_check_ssid(const osal_u8 *ssid, osal_u8 ssid_len)
{
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    oal_net_device_stru *netdev = OAL_PTR_NULL;
    oal_cfg80211_ssid_stru *ssids = OAL_PTR_NULL;
    osal_u32 ssid_num, index;
    osal_u8 req_ssid_len;
    hmac_device_stru *hmac_dev = hmac_res_get_mac_dev_etc(0); /* 获取hmac device 结构 */

    if (ssid == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    scan_mgmt = &(hmac_dev->scan_mgmt);
    if ((scan_mgmt->request == OAL_PTR_NULL) || (scan_mgmt->request->ssids == OAL_PTR_NULL)) {
        return OAL_SUCC;
    }

    netdev = hmac_scan_proc_get_dev(scan_mgmt);
    if ((netdev == OAL_PTR_NULL) || (netdev->ml_priv == OAL_PTR_NULL)) {
        return OAL_FAIL;
    }

    /* P2P场景下因扫描SSID不确定是否携带DIRECT-前缀格式，因此不在驱动做SSID和前缀SSID过滤 */
    if (!is_legacy_vap((const hmac_vap_stru *)netdev->ml_priv)) {
        return OAL_SUCC;
    }
    /* 当下发的ssid中有通配ssid时，不进行任何过滤 */
    for (index = 0; index < scan_mgmt->request->n_ssids; index++) {
        if (scan_mgmt->request->ssids[index].ssid[0] == '\0') {
            return OAL_SUCC;
        }
    }

    ssids = scan_mgmt->request->ssids;
    ssid_num = scan_mgmt->request->n_ssids;
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    /* linux 内核接口中没有 prefix_ssid_scan_flag */
    if (scan_mgmt->request->prefix_ssid_scan_flag == OAL_TRUE) {
        return hmac_scan_proc_check_prefix_ssid(ssids, ssid, ssid_len);
    }
#endif
    /* request 在驱动判断指定ssid扫描最大个数之前赋值，需在这里进行检查 */
    /* 如果未指定ssid,则直接返回 */
    if (ssid_num == 0) {
        return OAL_SUCC;
    }
    for (index = 0; index < ssid_num; index++) {
        req_ssid_len = ssids[index].ssid_len;
        if (req_ssid_len > OAL_IEEE80211_MAX_SSID_LEN) {
            req_ssid_len = OAL_IEEE80211_MAX_SSID_LEN;
        }
        if ((req_ssid_len != ssid_len) || (oal_memcmp(ssid, ssids[index].ssid, req_ssid_len) != 0)) {
            continue;
        }
        return OAL_SUCC;
    }
    return OAL_FAIL;
}

/*
 * 功能描述: 使用报文中的信息更新bss dscr
 * 注意事项: bss_dscr是出参，其余是入参
 */
OAL_STATIC osal_u32 hmac_scan_update_bss_dscr_from_frame(mac_bss_dscr_stru *bss_dscr, osal_u8 channel_number,
    osal_u8 *mgmt_frame, hmac_vap_stru *hmac_vap, osal_u16 frame_body_len)
{
    osal_u8                    ssid_len;
    osal_u8                   *ssid = OSAL_NULL;                 /* 指向beacon帧中的ssid */
    mac_ieee80211_frame_stru    *frame_header = OSAL_NULL;
    osal_u8                   *frame_body = OSAL_NULL;
    osal_u16                   offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_u8                    frame_channel;

    osal_u32 ret;
    /* 获取管理帧的帧头和帧体指针 */
    frame_header  = (mac_ieee80211_frame_stru *)mgmt_frame;
    frame_body    = mgmt_frame + MAC_80211_FRAME_LEN;

    /* 获取管理帧中的信道 */
    frame_channel = hmac_ie_get_chan_num_etc(frame_body, frame_body_len, offset, channel_number);

    /*****************************************************************************
        解析beacon/probe rsp帧，记录到bss_dscr
    *****************************************************************************/
    /* 解析并保存ssid */
    ssid = hmac_get_ssid_etc(frame_body, (osal_s32)frame_body_len, &ssid_len);
    if ((ssid != OAL_PTR_NULL) && (ssid_len != 0)) {
        /* 将查找到的ssid保存到bss描述结构体中 */
        if (memcpy_s(bss_dscr->ac_ssid, sizeof(bss_dscr->ac_ssid), ssid, ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_scan_update_bss_dscr_from_frame::memcpy_s error}");
        }
        bss_dscr->ac_ssid[ssid_len] = '\0';
    }

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    osal_adapt_kthread_lock(); /* linux系统下 待适配该函数 */
#endif
    ret = hmac_scan_proc_check_ssid(ssid, ssid_len);
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    osal_adapt_kthread_unlock(); /* linux系统下 待适配该函数 */
#endif
    if (ret != OAL_SUCC) {
        bss_dscr->need_drop = OSAL_TRUE;
        return ret;
    }

    /* 解析bssid */
    oal_set_mac_addr(bss_dscr->auc_mac_addr, frame_header->address2);
    oal_set_mac_addr(bss_dscr->bssid, frame_header->address3);
    bss_dscr->us_cap_info = *((osal_u16 *)(frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN));
    /* 解析beacon周期与tim周期 */
    bss_dscr->beacon_period = hmac_get_beacon_period_etc(frame_body);
    bss_dscr->dtim_period   = hmac_get_dtim_period_etc(frame_body, frame_body_len);
    bss_dscr->dtim_cnt      = hmac_get_dtim_cnt_etc(frame_body, frame_body_len);

    /* 信道 */
    bss_dscr->st_channel.chan_number = frame_channel;
    bss_dscr->st_channel.band        = mac_get_band_by_channel_num(frame_channel);

    /* 记录速率集 */
    hmac_scan_update_bss_list_rates(bss_dscr, frame_body, frame_body_len);

    /* 获取ap的Multiple BSSID capability */
    bss_dscr->multi_bssid_capa = hmac_get_multi_bssid_capa(frame_body, frame_body_len);

    /* 协议类相关信息元素的获取 */
    hmac_scan_update_bss_list_protocol(hmac_vap, bss_dscr, frame_body, frame_body_len);
    return OAL_SUCC;
}


/*****************************************************************************
 功能描述  : 更新描述扫描结构的bss dscr结构体中的need_drop参数
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_bss_dscr_need_drop(hmac_scanned_bss_info *scanned_bss,
    osal_u8 *frame_body, osal_u16 frame_body_len, osal_u8 channel_number,
    const mac_scanned_result_extend_info_stru *scan_result_extend)
{
    hmac_scan_stru              *scan_mgmt = OAL_PTR_NULL;
    hmac_device_stru            *hmac_dev = OAL_PTR_NULL;
    osal_u16                   offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_u8                    frame_channel;
    osal_u32                    ret = OAL_FAIL;

    /* 获取管理帧中的信道 */
    frame_channel = hmac_ie_get_chan_num_etc(frame_body, frame_body_len, offset, channel_number);
    /* 如果收到的帧信道和当前扫描信道不一致，则检查帧信道是否在扫描信道列表中, 若不在则过滤 */
    hmac_dev = hmac_res_get_mac_dev_etc(0);
    scan_mgmt = &(hmac_dev->scan_mgmt);
    if ((frame_channel != scan_result_extend->channel) &&
        (scan_mgmt->scan_2g_ch_list_map & (BIT0 << frame_channel)) == OAL_FALSE) {
        scanned_bss->bss_dscr_info.need_drop = OAL_TRUE;
        ret = OAL_SUCC;
    }

    return ret;
}


/*****************************************************************************
 功能描述  : 更新描述扫描结构的bss dscr结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_update_bss_dscr_from_extend(mac_bss_dscr_stru *bss_dscr,
    mac_scanned_result_extend_info_stru *scan_result_extend, osal_u8 *frame_body, hmac_vap_stru *hmac_vap,
    osal_u16 frame_len)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    osal_u16 frame_body_len = frame_len - MAC_80211_FRAME_LEN;
#endif
    osal_void *fhook = OAL_PTR_NULL;

    /* bss基本信息 */
    bss_dscr->bss_type = scan_result_extend->bss_type;
    bss_dscr->c_rssi      = (osal_s8)scan_result_extend->rssi;
    bss_dscr->c_ant0_rssi = (osal_s8)scan_result_extend->ant0_rssi;
    bss_dscr->c_ant1_rssi = (osal_s8)scan_result_extend->ant1_rssi;

#if defined (_PRE_WLAN_WIRELESS_EXT) || defined (_PRE_WLAN_FEATURE_M2S)
    bss_dscr->support_max_nss = scan_result_extend->support_max_nss;
#endif
    /* 03记录支持的最大空间流 */
#ifdef _PRE_WLAN_FEATURE_M2S
    bss_dscr->support_opmode = scan_result_extend->support_opmode;
    bss_dscr->num_sounding_dim = scan_result_extend->num_sounding_dim;
#endif

    /* update st_channel.bandwidth in case hmac_sta_update_join_req_params_etc usage error */
    bss_dscr->st_channel.en_bandwidth = bss_dscr->channel_bandwidth;
    if (hmac_get_channel_idx_from_num_etc(bss_dscr->st_channel.band, bss_dscr->st_channel.chan_number,
        &bss_dscr->st_channel.chan_idx) == OAL_ERR_CODE_INVALID_CONFIG) {
        oam_warning_log3(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr_from_extend:chan[%d],band[%d],idx[%d]get chnl fail",
            bss_dscr->st_channel.chan_number, bss_dscr->st_channel.band, bss_dscr->st_channel.chan_idx);
    }

    /* 更新时间戳 */
    bss_dscr->timestamp = (osal_u32)oal_time_get_stamp_ms();
    bss_dscr->mgmt_len = frame_len;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_SET_TSF);
    if (fhook != OSAL_NULL) {
        ((hmac_11k_set_tsf_cb)fhook)(hmac_vap, bss_dscr, scan_result_extend);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 获取802.11新增的element,并将它们存到扫描到的BSS描述结构体中 */
    hmac_scan_get_11ax_elements(bss_dscr, frame_body, frame_body_len);
#else
    unref_param(frame_body);
#endif
}

/*****************************************************************************
 功能描述  : 更新描述扫描结构的bss dscr结构体
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_update_bss_dscr(hmac_scanned_bss_info *scanned_bss,
    mac_scanned_result_extend_info_stru *scan_result_extend, osal_u8 vap_id, osal_u16 frame_len,
    osal_u8 channel_number)
{
    osal_u8                   *frame_body;
    mac_bss_dscr_stru           *bss_dscr;
    osal_u8                   *mgmt_frame;
    osal_u16                   frame_body_len;

    /* 获取hmac vap */
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::hmac_vap is null.}");
        return OAL_FAIL;
    }

    /* 获取管理帧体 */
    mgmt_frame = scanned_bss->bss_dscr_info.mgmt_buff;

    /* 获取管理帧的帧头和帧体指针 */
    frame_body    = mgmt_frame + MAC_80211_FRAME_LEN;

    if (frame_len < MAC_TAG_PARAM_OFFSET) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::frame_len[%d]err}", frame_len);
        return OAL_FAIL;
    }
    frame_body_len = frame_len - MAC_80211_FRAME_LEN;

    if (hmac_scan_bss_dscr_need_drop(scanned_bss, frame_body, frame_body_len, channel_number,
        (const mac_scanned_result_extend_info_stru *)scan_result_extend) == OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 更新bss信息 */
    bss_dscr = &(scanned_bss->bss_dscr_info);

    hmac_scan_update_bss_dscr_from_frame(bss_dscr, channel_number, mgmt_frame, hmac_vap, frame_body_len);

    hmac_scan_update_bss_dscr_from_extend(bss_dscr, scan_result_extend, frame_body, hmac_vap, frame_len);

    return OAL_SUCC;
}

/*****************************************************************************
功能描述:根据ssid ap的特点,保留带ssid扫描结果并不更新时间戳
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_scan_is_hidden_ssid(osal_u8 vap_id, hmac_scanned_bss_info *new_bss,
    const hmac_scanned_bss_info *old_bss)
{
    if ((new_bss->bss_dscr_info.ac_ssid[0] == '\0') &&
        (old_bss->bss_dscr_info.ac_ssid[0] != '\0')) {
        /*  隐藏SSID,若保存过此AP信息,且ssid不为空,此次通过BEACON帧扫描到此AP信息,且SSID为空,则不进行更新 */
        oam_warning_log4(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_is_hidden_ssid::find hide ssid:%.2x:%.2x:%.2x,ignore this update.}", vap_id,
            new_bss->bss_dscr_info.bssid[3], /* 3 数组下标 */
            new_bss->bss_dscr_info.bssid[4], /* 4 数组下标 */
            new_bss->bss_dscr_info.bssid[5]); /* 5 数组下标 */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 是否需要更新覆盖之前扫描结果
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_scan_need_update_old_scan_result_etc(osal_u8 vap_id,
    const hmac_scanned_bss_info *new_bss, const hmac_scanned_bss_info *old_bss)
{
    unref_param(vap_id);
    /* 该逻辑仅针对一轮扫描过程中出现beacon、probe rsp时的处理，
       如果之前扫描结果为probe rsp，此次为beacon，则不覆盖，否则覆盖 --start */
    if ((((mac_ieee80211_frame_stru *)old_bss->bss_dscr_info.mgmt_buff)->frame_control.sub_type ==
        WLAN_PROBE_RSP) &&
        (((mac_ieee80211_frame_stru *)new_bss->bss_dscr_info.mgmt_buff)->frame_control.sub_type ==
        WLAN_BEACON) &&
        (old_bss->bss_dscr_info.new_scan_bss == OAL_TRUE)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 功能描述  : 检测帧中信道信息是否合理
*****************************************************************************/
OAL_STATIC osal_u8 hmac_scan_check_chan(oal_netbuf_stru *pst_netbuf, hmac_scanned_bss_info *scanned_bss)
{
    dmac_rx_ctl_stru   *pst_rx_ctrl;
    osal_u8           curr_chan;
    osal_u8          *frame_body;
    osal_u16          frame_body_len;
    osal_u16          offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_u8          *ie_start_addr;
    osal_u8           chan_num;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    curr_chan = pst_rx_ctrl->rx_info.channel_number;
    frame_body = scanned_bss->bss_dscr_info.mgmt_buff + MAC_80211_FRAME_LEN;
    frame_body_len = (osal_u16)(scanned_bss->bss_dscr_info.mgmt_len - MAC_80211_FRAME_LEN);

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_DSPARMS, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != OAL_PTR_NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[2]; /* 2-第3字节 */
        if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(chan_num), chan_num) != OAL_SUCC) {
            return  OAL_FALSE;
        }
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr =
        mac_find_ie_etc(MAC_EID_HT_OPERATION, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != OAL_PTR_NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[2]; /* 2-第3字节 */
        if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(chan_num), chan_num) != OAL_SUCC) {
            return  OAL_FALSE;
        }
    }

    chan_num = scanned_bss->bss_dscr_info.st_channel.chan_number;
    if (((curr_chan > chan_num) && (curr_chan - chan_num >= 3)) || /* 偏离3个信道 */
        ((curr_chan < chan_num) && (chan_num - curr_chan >= 3))) { /* 偏离3个信道 */
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC  osal_void hmac_scan_free_scan_result_netbuf(hmac_scanned_bss_info *scanned_bss,
    oal_netbuf_stru *bss_mgmt_netbuf, osal_u8 multi_flag)
{
    if (scanned_bss != OSAL_NULL) {
        oal_free(scanned_bss);
    }

    /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
    if (multi_flag == OSAL_FALSE) {
        oal_netbuf_free(bss_mgmt_netbuf);
    }
    return;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存的bss的最大值
*****************************************************************************/
OAL_STATIC osal_u16 hmac_scan_get_bss_list_max_num(void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    return hwifi_get_hmac_max_ap_num_etc();
#else
    // 未开启自定义模式，默认200
    return 200;
#endif
}

/*****************************************************************************
 功能描述 : 删除链表头部/尾部的bss,未加锁,使用时注意并发问题 del_head true表示删除头,false表示删除尾
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_del_bss_list_head_or_tail_noblock(hmac_device_stru *hmac_device, osal_bool del_head)
{
    struct osal_list_head *entry;
    hmac_scanned_bss_info *scanned_bss;
    hmac_bss_mgmt_stru *bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    if (osal_list_empty(&(bss_mgmt->bss_list_head)) == OAL_TRUE) {
        return OAL_FAIL;
    }
    if (del_head == OSAL_TRUE) {
        entry = bss_mgmt->bss_list_head.next; // first list entry
    } else {
        entry = bss_mgmt->bss_list_head.prev;
    }
    osal_list_del(entry);
    scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
    bss_mgmt->bss_num--;
    oam_warning_log2(0, OAM_SF_SCAN,
        "{hmac_scan_del_bss_head_noblock::delete oldest bss, now bss sum[%d], bss limit[%d]}",
        bss_mgmt->bss_num, hmac_scan_get_bss_list_max_num());
    dft_report_params_etc((osal_u8 *)scanned_bss->bss_dscr_info.ac_ssid,
        (osal_u16)osal_strlen(scanned_bss->bss_dscr_info.ac_ssid), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    /* 释放删除头部节点的内存 */
    oal_free(scanned_bss);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 如果超过上限, 删掉rssi最小的bss
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_handle_min_rssi_bss(hmac_device_stru *hmac_device)
{
    // 获取管理扫描的bss结果的结构体
    hmac_bss_mgmt_stru *bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    // 判断当前bss数是否超过阈值,若超过则删除最老的bss(即链表头部bss,此bss时间戳最小)
    if (bss_mgmt->bss_num >= hmac_scan_get_bss_list_max_num()) {
        hmac_scan_del_bss_list_head_or_tail_noblock(hmac_device, OSAL_FALSE);
    }
}

/*****************************************************************************
 功能描述      : 根据当前的ap数量,更新hmac侧bss超过阈值以及信道随机化的状态
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_update_bss_list_exceed_info_etc(hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru *bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    hmac_bss_list_exceed_info_stru *bss_list_exceed_info = &(bss_mgmt->bss_list_exceed_info);
    if (bss_mgmt->bss_num >= hmac_scan_get_bss_list_max_num()) {
        // 当前保存的ap数超过阈值时,exceed_count加一,当其值超过exceed_limit时(最大为exceed_limit),信道随机化
        bss_list_exceed_info->exceed_count++;
        if (bss_list_exceed_info->exceed_count >= bss_list_exceed_info->exceed_limit) {
            bss_list_exceed_info->exceed_count = bss_list_exceed_info->exceed_limit;
            bss_list_exceed_info->randomize = OSAL_TRUE;
        }
        oam_info_log2(0, OAM_SF_CUSTOM, "hmac_scan_update_bss_list_exceed_info_etc::exceed_count[%d],randomize[%d]",
            bss_list_exceed_info->exceed_count, bss_list_exceed_info->randomize);
    } else {
        // 当前保存的ap数小于阈值时,exceed_count减一,当其值为0时(最小为0),取消信道随机化
        if (bss_list_exceed_info->exceed_count > 0) {
            bss_list_exceed_info->exceed_count--;
        }
        if (bss_list_exceed_info->exceed_count == 0) {
            bss_list_exceed_info->randomize = OSAL_FALSE;
        }
    }
}

/*****************************************************************************
 功能描述  : 判断是否将新申请的scan
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_insert_new_record(osal_u8 vap_id,
    hmac_scanned_bss_info *new_scanned_bss, oal_netbuf_stru *bss_mgmt_netbuf, osal_u8 multi_flag)
{
    hmac_scanned_bss_info *old_scanned_bss = OSAL_NULL;
    osal_u32                curr_time_stamp;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru *frame_header = OSAL_NULL;
#endif
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 获取管理扫描的bss结果的结构体 */
    hmac_bss_mgmt_stru *bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    /* 对链表删操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));
    /* 判断相同bssid的bss是否已经扫描到 */
    old_scanned_bss = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, new_scanned_bss->bss_dscr_info.bssid);
    if (old_scanned_bss == OAL_PTR_NULL) {
        // 更新hmac_ap_list_info
        hmac_scan_update_bss_list_exceed_info_etc(hmac_device);
        // 处理信号强度最弱的节点
        hmac_scan_handle_min_rssi_bss(hmac_device);
        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        /* 将扫描结果添加到链表中 扫描结果按照rssi强度添加到链表 */
        hmac_scan_add_bss_to_list(new_scanned_bss, hmac_device);
        /* 处理信号强度最弱的节点, 超过上限时删除 */
        /* 释放帧的内容 */
        hmac_scan_free_scan_result_netbuf(OSAL_NULL, bss_mgmt_netbuf, multi_flag);
        return;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 只有probe rsp帧中ext cap 宣称支持OPMODE时，对端才确实支持OPMODE，beacon帧和assoc rsp帧中信息不可信 */
    frame_header = (mac_ieee80211_frame_stru *)new_scanned_bss->bss_dscr_info.mgmt_buff;
    if (frame_header->frame_control.sub_type == WLAN_PROBE_RSP) {
        old_scanned_bss->bss_dscr_info.support_opmode = new_scanned_bss->bss_dscr_info.support_opmode;
    }
#endif

    /* 如果老的扫描的bss的信号强度大于当前扫描到的bss的信号强度，更新当前扫描到的信号强度为最强的信号强度 */
    if (old_scanned_bss->bss_dscr_info.c_rssi > new_scanned_bss->bss_dscr_info.c_rssi) {
        /* 1s中以内就采用之前的BSS保存的RSSI信息，否则就采用新的RSSI信息 */
        curr_time_stamp = (osal_u32)oal_time_get_stamp_ms();
        if ((curr_time_stamp - old_scanned_bss->bss_dscr_info.timestamp) < HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE) {
            new_scanned_bss->bss_dscr_info.c_rssi = old_scanned_bss->bss_dscr_info.c_rssi;
        }
    }

    if (hmac_scan_is_hidden_ssid(vap_id, new_scanned_bss, old_scanned_bss) == OAL_TRUE) {
        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        /* 释放申请的存储bss信息的内存 与 帧的内容 */
        hmac_scan_free_scan_result_netbuf(new_scanned_bss, bss_mgmt_netbuf, multi_flag);
        return;
    }

    if (hmac_scan_need_update_old_scan_result_etc(vap_id, new_scanned_bss, old_scanned_bss) == OAL_FALSE ||
        hmac_scan_check_chan(bss_mgmt_netbuf, new_scanned_bss) == OAL_FALSE) {
        old_scanned_bss->bss_dscr_info.timestamp = (osal_u32)oal_time_get_stamp_ms();
        old_scanned_bss->bss_dscr_info.c_rssi = new_scanned_bss->bss_dscr_info.c_rssi;

        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));

        hmac_scan_free_scan_result_netbuf(new_scanned_bss, bss_mgmt_netbuf, multi_flag);
        return;
    }

    /* 从链表中将原先扫描到的相同bssid的bss节点删除 */
    hmac_scan_del_bss_from_list_nolock(old_scanned_bss, hmac_device);
    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    /* 将扫描结果添加到链表中 */
    hmac_scan_add_bss_to_list(new_scanned_bss, hmac_device);

    /* 释放老的存储bss信息的内存 与 帧的内容 */
    hmac_scan_free_scan_result_netbuf(old_scanned_bss, bss_mgmt_netbuf, multi_flag);
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述  : 动态申请暂存nontransmitted BSSIDs Profile的内存
*****************************************************************************/
OAL_STATIC hmac_scanned_nontrans_bssid_info *hmac_scan_alloc_nontrans_bss(osal_void)
{
    hmac_scanned_nontrans_bssid_info *nontrans_bssid = OSAL_NULL;

    /* 申请内存，存储扫描到的bss信息 */
    nontrans_bssid = oal_memalloc(sizeof(hmac_scanned_nontrans_bssid_info));
    if (nontrans_bssid == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_alloc_nontrans_bssid::alloc memory failed for nontrans_bssid.}");
        return OSAL_NULL;
    }

    /* 为申请的内存清零 */
    memset_s(nontrans_bssid, sizeof(hmac_scanned_nontrans_bssid_info), 0, sizeof(hmac_scanned_nontrans_bssid_info));

    /* 初始化链表头节点指针 */
    OSAL_INIT_LIST_HEAD(&(nontrans_bssid->dlist_head));

    return nontrans_bssid;
}

/*****************************************************************************
 功能描述  : 将扫描到的bss添加到链表
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_add_nontrans_bss_to_list(hmac_bss_mgmt_stru *bss_mgmt,
    hmac_scanned_nontrans_bssid_info *nontrans_bssid)
{
    /* 对链表写操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 添加扫描结果到链表中，并更新扫描到的bss计数 */
    osal_list_add_tail(&(nontrans_bssid->dlist_head), &(bss_mgmt->bss_list_head));

    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));
}

/*****************************************************************************
 功能描述  : 删除解析multi bssid element中暂存的nontrans_bss信息
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_free_nontrans_bss_info(hmac_bss_mgmt_stru *bss_mgmt)
{
    struct osal_list_head              *entry              = OSAL_NULL;
    hmac_scanned_nontrans_bssid_info *nontrans_bssid_tmp = OSAL_NULL;

    while (osal_list_empty(&(bss_mgmt->bss_list_head)) == OAL_FALSE) {
        entry = bss_mgmt->bss_list_head.next; // first list entry
        osal_list_del(entry);
        nontrans_bssid_tmp = osal_list_entry(entry, hmac_scanned_nontrans_bssid_info, dlist_head);

        oal_free(nontrans_bssid_tmp);
    }
}

/*****************************************************************************
 功能描述  : 扫描中multi bssid element的处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_nontrans_bss_profile_malloc(osal_u8 *subelement_addr, hmac_bss_mgmt_stru *bss_mgmt,
    hmac_scanned_nontrans_bssid_info **nontrans_bssid, osal_u32 *bss_memcpy_offset)
{
    if (subelement_addr[2] == MAC_EID_NONTRANS_BSSID_CAPA) { /* 2:找到sub eid中的首个eid */
        bss_mgmt->bss_num++;
        *bss_memcpy_offset = 0;

        if ((bss_mgmt->bss_num > 1) && ((*nontrans_bssid) != OSAL_NULL)) {
            hmac_scan_add_nontrans_bss_to_list(bss_mgmt, (*nontrans_bssid));
        }

        /* 动态申请内存，将subelement中的element copy到动态申请的内存中 */
        (*nontrans_bssid) = hmac_scan_alloc_nontrans_bss();
        if ((*nontrans_bssid) == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_get_nontrans_bss_profile:alloc memory err}");
            hmac_scan_free_nontrans_bss_info(bss_mgmt);
            return OAL_FAIL;
        }
    }

    if ((*nontrans_bssid) != OSAL_NULL) {
        /* 将subelement中的element copy到上述动态申请的内存中 */
        if (memcpy_s((*nontrans_bssid)->bssid_profile + (*bss_memcpy_offset),
            /* 2:element基地址 */
            WLAN_MGMT_NETBUF_SIZE - (*bss_memcpy_offset), &subelement_addr[2], subelement_addr[1]) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_get_nontrans_bss_profile::copy err}");
            hmac_scan_free_nontrans_bss_info(bss_mgmt);
            return OAL_FAIL;
        }

        (*bss_memcpy_offset) += subelement_addr[1];
        (*nontrans_bssid)->frame_len = *bss_memcpy_offset;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_scan_proc_nontrans_bss_subelement(osal_u8 *ie_start_addr, osal_u32 *bss_memcpy_offset,
    hmac_scanned_nontrans_bssid_info **nontrans_bssid, hmac_bss_mgmt_stru *bss_mgmt)
{
    osal_u8 *nontrans_bss_addr = OSAL_NULL;
    osal_u8 *subelement_addr = OSAL_NULL;
    osal_u32 nontrans_bss_offset = 1; /* 1:MaxBSSID Indicator占1byte */
    osal_u8 nontrans_bss_len;
    osal_u32 ret;

    nontrans_bss_len = ie_start_addr[1];
    /* 按照顺序解析Nontransmitted BSSID Profile subelement */
    nontrans_bss_addr = &ie_start_addr[MAC_IE_HDR_LEN];

    while (nontrans_bss_offset < (osal_u32)nontrans_bss_len) {
        subelement_addr = nontrans_bss_addr + nontrans_bss_offset;
        nontrans_bss_offset += (subelement_addr[1] + MAC_IE_HDR_LEN);
        if (nontrans_bss_offset > (osal_u32)nontrans_bss_len) {
            break;
        }

        if (subelement_addr[0] != MAC_SUB_EID_NONTRANS_BSSID_PROFILE) {
            continue;
        }

        ret = hmac_scan_nontrans_bss_profile_malloc(subelement_addr, bss_mgmt, nontrans_bssid, bss_memcpy_offset);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 扫描中multi bssid element的处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_get_nontrans_bss_profile(osal_u8 *mgmt_frame, osal_u16 mgmt_len,
    hmac_bss_mgmt_stru *bss_mgmt, osal_u8 *maxbssid_indicator)
{
    /* 解析Multiple BSSID element */
    osal_u8 *ie_start_addr = OSAL_NULL;
    osal_u16 offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    hmac_scanned_nontrans_bssid_info *nontrans_bssid = OSAL_NULL;
    osal_u32 bss_memcpy_offset = 0;
    osal_u8  nontrans_bss_len;
    osal_u32 ret;

    while (offset < mgmt_len) {
        ie_start_addr = mac_find_ie_etc(MAC_EID_MULTIPLE_BSSID, mgmt_frame + offset, mgmt_len - offset);
        /* 所有的Multiple BSSID element查找完毕 */
        if (ie_start_addr == OSAL_NULL) {
            break;
        }

        nontrans_bss_len = ie_start_addr[1];
        offset           = (ie_start_addr - mgmt_frame) + nontrans_bss_len + MAC_IE_HDR_LEN;

        *maxbssid_indicator = ie_start_addr[MAC_IE_HDR_LEN]; /* 偏移获取max bssid_indicator */
        if (((*maxbssid_indicator) > HMAC_MULTI_MAX_BSSID_INDICATOR) || ((*maxbssid_indicator) == 0)) { /* 最大值为8 */
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_get_nontrans_bss_profile:maxbssid_indicator[%d]err}",
                (*maxbssid_indicator));
            hmac_scan_free_nontrans_bss_info(bss_mgmt);
            return OAL_FAIL;
        }

        /* 确定本Multiple BSSID element包含有效的subelement */
        if (nontrans_bss_len <= MULTI_BSSID_ELEMENT_MIN_LEN) {
            continue;
        }

        ret = hmac_scan_proc_nontrans_bss_subelement(ie_start_addr, &bss_memcpy_offset, &nontrans_bssid, bss_mgmt);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    /* 最后1获取的1个nontrans bss profile加入链表 */
    if (bss_mgmt->bss_num > 0) {
        hmac_scan_add_nontrans_bss_to_list(bss_mgmt, nontrans_bssid);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取nontransmitted BSSID = BSSID_A | BSSID_B
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_get_nontrans_bssid(osal_u8 bssid_idx, osal_u8 maxbssid_indicator,
    osal_u8 *trans_bssid, osal_u8 *nontrans_bssid, osal_u8 bssid_size)
{
    osal_u8 diff_bit, same_bit;

    if ((maxbssid_indicator > HMAC_MULTI_MAX_BSSID_INDICATOR) || (maxbssid_indicator == 0) ||
        (bssid_idx >= (1 << maxbssid_indicator)) || (bssid_idx == 0)) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_scan_get_nontrans_bssid::bssid_idx[%d] err,maxbssid_indicator[%d]}",
            bssid_idx, maxbssid_indicator);
        return OAL_FAIL;
    }

    /* multi bssid set中，前48-maxbssid_indicator+1 bit与trans_bssid相同 */
    /* 即高5byte和trans_bssid相同，只有最低的1byte有差异 */
    if (memcpy_s(nontrans_bssid, (bssid_size - 1), trans_bssid, (bssid_size - 1)) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_get_nontrans_bssid::memcpy err}");
        return OAL_FAIL;
    }

    /* 取trans_bssid[5]的高(8-maxbssid_indicator) bit进行运算,
       nontransmitted bssid的bssid[5]的高(8-maxbssid_indicator) bit和transmitted bssid的值一样 */
    same_bit = (trans_bssid[WLAN_MAC_ADDR_LEN - 1] >> maxbssid_indicator) << maxbssid_indicator;

    /* 取trans_bssid[5] 的低maxbssid_indicator bit进行运算 */
    diff_bit = ((1 << maxbssid_indicator) - 1) & trans_bssid[WLAN_MAC_ADDR_LEN - 1];
    diff_bit = (osal_u8)(((osal_u32)(diff_bit + bssid_idx)) % ((osal_u32)(1 << maxbssid_indicator)));

    nontrans_bssid[bssid_size - 1] = same_bit | diff_bit;

    return OAL_SUCC;
}

static inline osal_u32 oal_free_and_return_fail(hmac_scanned_bss_info *nontrans_bss,
    hmac_scanned_nontrans_bssid_info *bss_profile)
{
    oal_free(bss_profile);
    oal_free(nontrans_bss);
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 处理multi bssid element的subelement中的前3个固定顺序的element
 Nontransmitted BSSID Capability element、SSID element、Multiple BSSID-Index element
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_proc_multi_fixed_element(hmac_scanned_bss_info *nontrans_bss,
    hmac_scanned_nontrans_bssid_info *bss_profile, osal_u16 *profile_offset, osal_u16 *beacon_offset,
    hmac_scanned_bssid_index_info *bssid_index_info)
{
    osal_u8  cap_info_offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN;
    osal_u8  ssid_offset     = MAC_TAG_PARAM_OFFSET;
    osal_u8  nontrans_bssid[WLAN_MAC_ADDR_LEN] = {0};
    mac_ieee80211_frame_stru *frame_header = (mac_ieee80211_frame_stru *)(nontrans_bss->bss_dscr_info.mgmt_buff);
    mac_scanned_all_bss_info *all_bss_info = bssid_index_info->all_bss_info;

    /* 处理Nontransmitted BSSID Capability element */
    if (bss_profile->bssid_profile[0] != MAC_EID_NONTRANS_BSSID_CAPA) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_fixed_element::Get NonTrans BSSID err}");
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }
    nontrans_bss->bss_dscr_info.mgmt_buff[cap_info_offset] = bss_profile->bssid_profile[MAC_IE_HDR_LEN];
    nontrans_bss->bss_dscr_info.mgmt_buff[cap_info_offset + 1] = bss_profile->bssid_profile[MAC_IE_HDR_LEN + 1];

    /* 处理SSID element */
    *profile_offset = bss_profile->bssid_profile[1] + MAC_IE_HDR_LEN;
    if (bss_profile->bssid_profile[(*profile_offset)] != MAC_EID_SSID) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_fixed_element::Get NonTrans BSSID err}");
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }

    if (memcpy_s(nontrans_bss->bss_dscr_info.mgmt_buff + ssid_offset,
        WLAN_MGMT_NETBUF_SIZE - ssid_offset,
        &(bss_profile->bssid_profile[(*profile_offset)]),
        (bss_profile->bssid_profile[(*profile_offset) + 1] + MAC_IE_HDR_LEN)) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_fixed_element::copy err}");
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }
    *beacon_offset = ssid_offset + (bss_profile->bssid_profile[(*profile_offset) + 1] + MAC_IE_HDR_LEN);

    /* 处理Multiple BSSID-Index element */
    (*profile_offset) += (bss_profile->bssid_profile[(*profile_offset) + 1] + MAC_IE_HDR_LEN);
    if (bss_profile->bssid_profile[(*profile_offset)] != MAC_EID_MULTI_BSSID_IDX) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_fixed_element::Get Multiple BSSID-Index err}");
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }

    all_bss_info->bssid_idx = bss_profile->bssid_profile[(*profile_offset) + MAC_IE_HDR_LEN];
    if (bss_profile->bssid_profile[(*profile_offset) + 1] == MULTI_DTIM_PERIOD_COUNT_LEN) {
        bssid_index_info->dtim_period = bss_profile->bssid_profile[(*profile_offset) + MAC_IE_HDR_LEN + 1];
        bssid_index_info->dtim_count  = bss_profile->bssid_profile[(*profile_offset) + MAC_IE_HDR_LEN + 2]; /* 2 偏移 */
        bssid_index_info->period_count_valid = 1;
    } else {
        bssid_index_info->period_count_valid = 0;
    }

    if (hmac_scan_get_nontrans_bssid(all_bss_info->bssid_idx, all_bss_info->maxbssid_indicator,
        all_bss_info->trans_bssid, nontrans_bssid, sizeof(nontrans_bssid)) != OAL_SUCC) {
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }
    if (memcpy_s(frame_header->address3, WLAN_MAC_ADDR_LEN, nontrans_bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_fixed_element::copy err}");
        return oal_free_and_return_fail(nontrans_bss, bss_profile);
    }

    nontrans_bss->bss_dscr_info.all_bss_info = *all_bss_info;

    /* 获取前3个element的总偏移 */
    (*profile_offset) += (bss_profile->bssid_profile[(*profile_offset) + 1] + MAC_IE_HDR_LEN);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 解析获取nontransmitted BSSIDs profile中的Non-Inheritance element，获取固定的偏移地址等信息
             将nontransmitted BSSIDs profile中除了前3个固定element和Non-Inheritance element之外的其他element
             copy到动态申请的scan帧内存
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_proc_multi_skip_particular_eid(hmac_scanned_bss_info *nontrans_bss,
    hmac_scanned_nontrans_bssid_info *bss_profile, osal_u8 *skip_addr, osal_u16 *beacon_offset, osal_u16 profile_offset)
{
    osal_u16 subelement_len;

    skip_addr = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_NON_INHERITANCE,
        bss_profile->bssid_profile, (osal_s32)bss_profile->frame_len);
    if (skip_addr != OSAL_NULL) {
        subelement_len = (skip_addr - bss_profile->bssid_profile - profile_offset);
    } else {
        subelement_len = (osal_u16)(bss_profile->frame_len - profile_offset);
    }

    if (subelement_len > 0) {
        if ((WLAN_MGMT_NETBUF_SIZE <= (*beacon_offset)) ||
            (memcpy_s(nontrans_bss->bss_dscr_info.mgmt_buff + (*beacon_offset),
                (WLAN_MGMT_NETBUF_SIZE - (*beacon_offset)), &(bss_profile->bssid_profile[profile_offset]),
                subelement_len) != EOK)) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_multi_skip_particular_eid::copy err}");
            oal_free(bss_profile);
            oal_free(nontrans_bss);
            return OAL_FAIL;
        }
    }
    (*beacon_offset) += subelement_len;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 确定指定的element是否属于Non-Inheritance element中标识的非继承element
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_check_non_inheritance(osal_u8 beacon_eid, osal_u8 beacon_ext_eid,
    osal_u8 *non_inheritance_addr)
{
    osal_u8 idx, eid_num;

    if (non_inheritance_addr == OSAL_NULL) {
        return OAL_FAIL;
    }

    eid_num = non_inheritance_addr[3]; /* 3:List Of Element IDs field中的Length偏移 */

    if (beacon_eid < MAC_EID_HE) {
        for (idx = 0; idx < eid_num; idx++) {
            if (non_inheritance_addr[4 + idx] == beacon_eid) { /* 4:偏移获取Element ID List */
                return OAL_SUCC;
            }
        }
        return OAL_FAIL;
    } else {
        for (idx = 0; idx < eid_num; idx++) {
            if (non_inheritance_addr[5 + eid_num + idx] == beacon_ext_eid) { /* 5:偏移获取Element ID Extension List */
                return OAL_SUCC;
            }
        }
        return OAL_FAIL;
    }
}

/*****************************************************************************
 功能描述  : 逐一解析transmitted BSSID的beacon的element,若为Multiple BSSID / Multiple BSSID Configuration element,忽略;
             若在Non-Inheritance element中列出,忽略;
             若不在Non-Inheritance element中列出,在nontransmitted BSSIDs profile中已经存在,忽略；
             若不在Non-Inheritance element中列出,在nontransmitted BSSIDs profile中不存在,copy到动态申请的scan帧内存中.
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_proc_trans_beacon_eid(hmac_scanned_bss_info *nontrans_bss,
    hmac_scanned_nontrans_bssid_info *bss_profile, osal_u8 *skip_addr, osal_u16 *beacon_offset,
    dmac_tx_event_stru *dtx_event)
{
    osal_u16 offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_u16 mgmt_offset;
    osal_u8  eid, ext_eid;
    osal_u8 *element_addr = OSAL_NULL;
    osal_u8 *mgmt_frame = (osal_u8 *)OAL_NETBUF_DATA(dtx_event->netbuf);

    while (offset < dtx_event->frame_len) {
        eid     = mgmt_frame[offset];
        ext_eid = (eid < MAC_EID_HE) ? MAC_EID_EXT_BUT : mgmt_frame[offset + MAC_IE_HDR_LEN];

        mgmt_offset = offset;
        offset       += (mgmt_frame[offset + 1] + MAC_IE_HDR_LEN);

        if ((eid == MAC_EID_MULTIPLE_BSSID) ||
            ((eid == MAC_EID_HE) && (ext_eid == MAC_EID_EXT_MULTIPLE_BSSID_CONFIGURATION))) {
            continue;
        }

        if (hmac_scan_check_non_inheritance(eid, ext_eid, skip_addr) == OAL_SUCC) {
            continue;
        }

        if (eid < MAC_EID_HE) {
            element_addr = mac_find_ie_etc(eid, bss_profile->bssid_profile, (osal_s32)bss_profile->frame_len);
        } else {
            element_addr = hmac_find_ie_ext_ie(eid, ext_eid, bss_profile->bssid_profile,
                (osal_s32)bss_profile->frame_len);
        }

        if (element_addr != OSAL_NULL) {
            continue;
        } else {
            if ((WLAN_MGMT_NETBUF_SIZE <= (*beacon_offset)) ||
                (memcpy_s(nontrans_bss->bss_dscr_info.mgmt_buff + (*beacon_offset),
                    (WLAN_MGMT_NETBUF_SIZE - (*beacon_offset)), &(mgmt_frame[mgmt_offset]),
                    (mgmt_frame[mgmt_offset + 1] + MAC_IE_HDR_LEN)) != EOK)) {
                oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_trans_beacon_eid::copy err}");
                oal_free(bss_profile);
                oal_free(nontrans_bss);
                return OAL_FAIL;
            }
            (*beacon_offset) += (mgmt_frame[mgmt_offset + 1] + MAC_IE_HDR_LEN);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据所有有效的nontransmitted bss profile，获取maxbssid_indicator、trans_bssid、bssid_bitmap等信息
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_set_all_bss_info(osal_u8 *mgmt_frame, hmac_bss_mgmt_stru *bss_mgmt,
    mac_scanned_all_bss_info *all_bss_info, osal_u8 maxbssid_indicator)
{
    struct osal_list_head              *entry          = OSAL_NULL;
    hmac_scanned_nontrans_bssid_info *bss_profile    = OSAL_NULL;
    osal_u8                            *bssid_idx_addr = OSAL_NULL;
    mac_ieee80211_frame_stru         *frame_header   = (mac_ieee80211_frame_stru *)mgmt_frame;

    osal_u8 bss_profile_num = 0;

    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        bss_profile = osal_list_entry(entry, hmac_scanned_nontrans_bssid_info, dlist_head);

        bssid_idx_addr = mac_find_ie_etc(MAC_EID_MULTI_BSSID_IDX, bss_profile->bssid_profile,
            (osal_s32)bss_profile->frame_len);
        if (bssid_idx_addr != OSAL_NULL) {
            bss_profile_num++;
        }
    }

    if (bss_profile_num > 0) {
        all_bss_info->multi_bss_eid_exist = 1;
        all_bss_info->maxbssid_indicator = maxbssid_indicator;
        oal_set_mac_addr(all_bss_info->trans_bssid, frame_header->address3);
        return OAL_SUCC;
    } else {
        return OAL_FAIL;
    }
}

static osal_u32 hmac_scan_multi_bssid_get_two_bss_info(hmac_bss_mgmt_stru *bss_mgmt, osal_u8 *mgmt_frame,
    hmac_scanned_nontrans_bssid_info **bss_profile, hmac_scanned_bss_info **nontrans_bss)
{
    struct osal_list_head *entry = OSAL_NULL;
    hmac_scanned_nontrans_bssid_info *bss_profile_tmp = OSAL_NULL;
    hmac_scanned_bss_info *nontrans_bss_tmp = OSAL_NULL;

    entry = bss_mgmt->bss_list_head.next; // first list entry
    osal_list_del(entry);
    bss_profile_tmp = osal_list_entry(entry, hmac_scanned_nontrans_bssid_info, dlist_head);

    /* 申请存储扫描结果的内存 */
    nontrans_bss_tmp = hmac_scan_alloc_scanned_bss(WLAN_MGMT_NETBUF_SIZE);
    if (nontrans_bss_tmp == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_multi_bssid_get_two_bss_info::hmac_scan_alloc_scanned_bss failed}");
        oal_free(bss_profile_tmp);
        return OAL_FAIL;
    }

    /* 先将transmitted BSSID的beacon的mac帧头+3个固定元素字段，copy到动态申请的scan帧内存中 */
    if (memcpy_s(nontrans_bss_tmp->bss_dscr_info.mgmt_buff, WLAN_MGMT_NETBUF_SIZE, mgmt_frame,
        MAC_TAG_PARAM_OFFSET) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_multi_bssid_get_two_bss_info::copy err}");
        oal_free(bss_profile_tmp);
        oal_free(nontrans_bss_tmp);
        return OAL_FAIL;
    }

    *bss_profile = bss_profile_tmp;
    *nontrans_bss = nontrans_bss_tmp;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 扫描中multi bssid的处理
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_multi_bssid_proc(mac_scanned_result_extend_info_stru *scan_extend,
    dmac_tx_event_stru *dtx_event, osal_u8 channel_number, osal_u8 vap_id, mac_scanned_all_bss_info *all_bss_info)
{
    osal_u8  maxbssid_index = 0;
    osal_u16 profile_offset = 0;
    osal_u16 beacon_offset  = 0;
    oal_netbuf_stru *bss_mgmt_netbuf = dtx_event->netbuf;
    osal_u8 *skip_addr  = OSAL_NULL;
    osal_u8 *mgmt_frame = (osal_u8 *)OAL_NETBUF_DATA(bss_mgmt_netbuf);

    hmac_scanned_nontrans_bssid_info *bss_profile  = OSAL_NULL;
    hmac_scanned_bss_info            *nontrans_bss = OSAL_NULL;
    hmac_scanned_bssid_index_info bssid_index_info;

    hmac_bss_mgmt_stru bss_mgmt;
    memset_s(&bss_mgmt, sizeof(bss_mgmt), 0, sizeof(bss_mgmt));
    memset_s(&bssid_index_info, sizeof(bssid_index_info), 0, sizeof(bssid_index_info));

    /* 初始化链表头节点指针 */
    OSAL_INIT_LIST_HEAD(&(bss_mgmt.bss_list_head));
    osal_spin_lock_init(&(bss_mgmt.lock));

    /* 以Nontransmitted BSSID Capability element(SSID element、Multiple BSSID-Index element)为分割符，
       找到实际的nontransmitted BSSIDs数目;为每一个nontransmitted BSSID动态申请内存，存储对应的element */
    if (hmac_scan_get_nontrans_bss_profile(mgmt_frame, dtx_event->frame_len, &bss_mgmt, &maxbssid_index) != OAL_SUCC) {
        return;
    }

    /* 根据所有有效的nontransmitted bss profile，获取maxbssid_indicator、trans_bssid、bssid_bitmap等信息 */
    if (hmac_scan_set_all_bss_info(mgmt_frame, &bss_mgmt, all_bss_info, maxbssid_index) != OAL_SUCC) {
        return;
    }

    /* 根据mac_bss_dscr_stru中nontransmitted BSSIDs大小，分别动态申请内存(mac_bss_dscr_stru+帧内存);
       逐一解析beacon帧原element和nontransmitted BSSID的element(关注Non-Inheritance element)，
       获取nontransmitted BSSID对应的beacon信息及其对应的mac_bss_dscr_stru信息 */
    while (osal_list_empty(&(bss_mgmt.bss_list_head)) == OAL_FALSE) {
        if (hmac_scan_multi_bssid_get_two_bss_info(&bss_mgmt, mgmt_frame, &bss_profile, &nontrans_bss) != OAL_SUCC) {
            continue;
        }

        /* 取动态申请的nontransmitted BSSIDs profile中的前3个固定element，更新上述copy的内容 */
        bssid_index_info.all_bss_info = all_bss_info;
        if (hmac_scan_proc_multi_fixed_element(nontrans_bss, bss_profile, &profile_offset, &beacon_offset,
            &bssid_index_info) != OAL_SUCC) {
            continue;
        }

        /* 解析获取nontransmitted BSSIDs profile中的Non-Inheritance element，获取固定的偏移地址等信息 */
        /* 将nontransmitted BSSIDs profile中除了前3个固定element和Non-Inheritance element之外的其他element
           copy到动态申请的scan帧内存中 */
        if (hmac_scan_proc_multi_skip_particular_eid(nontrans_bss, bss_profile, skip_addr, &beacon_offset,
            profile_offset) != OAL_SUCC) {
            continue;
        }

        /* 逐一解析transmitted BSSID的beacon的element,若为Multiple BSSID / Multiple BSSID Configuration element,忽略
           若在Non-Inheritance element中列出,忽略;
           若不在Non-Inheritance element中列出,在nontransmitted BSSIDs profile中已经存在,忽略；
           若不在Non-Inheritance element中列出,在nontransmitted BSSIDs profile中不存在,copy到动态申请的scan帧内存中 */
        if (hmac_scan_proc_trans_beacon_eid(nontrans_bss, bss_profile, skip_addr, &beacon_offset,
            dtx_event) != OAL_SUCC) {
            continue;
        }

        /* 删除动态申请的nontransmitted BSSIDs profile内存 */
        oal_free(bss_profile);

        /* 判决将动态申请的scan信息存于scan链表，beacon_offset即为重组的nontrans bss beacon帧长度 */
        if (hmac_scan_update_bss_dscr(nontrans_bss, scan_extend, vap_id, beacon_offset, channel_number) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_multi_bssid_proc::update_bss_dscr failed.}");
            oal_free(nontrans_bss);
        }

        /* 基于nontransmitted BSSID实际的DTIM period/count，更新对应的扫描信息 */
        if (bssid_index_info.period_count_valid == 1) {
            nontrans_bss->bss_dscr_info.dtim_period = bssid_index_info.dtim_period;
            nontrans_bss->bss_dscr_info.dtim_cnt    = bssid_index_info.dtim_count;
        }

        /* 判决是否将当前Nontransmitted BSSID的scan信息存入链表 */
        hmac_scan_insert_new_record(vap_id, nontrans_bss, bss_mgmt_netbuf, OSAL_TRUE);
    }
}
#endif

/*****************************************************************************
 功能描述  : 更新每个信道的扫描结果
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_update_bss_dscr_from_netbuf(hmac_vap_stru *hmac_vap, dmac_tx_event_stru *dtx_event,
    oal_netbuf_stru *bss_mgmt_netbuf, mac_scanned_result_extend_info_stru *scaned_extend_info, osal_u16 mgmt_len)
{
    osal_u32 ret;
    osal_u8 channel_number;
    mac_scanned_all_bss_info all_bss_info = {0};
    osal_u8 *mgmt_frame = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    /* 申请存储扫描结果的内存 */
    hmac_scanned_bss_info *new_scanned_bss = hmac_scan_alloc_scanned_bss(mgmt_len);
    if (OAL_UNLIKELY(new_scanned_bss == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_update_bss_dscr_from_netbuf::alloc failed.}", vap_id);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(bss_mgmt_netbuf);
        return OAL_PTR_NULL;
    }

    /* 更新描述扫描结果的bss dscr结构体 */
    channel_number = ((dmac_rx_ctl_stru *)oal_netbuf_cb(bss_mgmt_netbuf))->rx_info.channel_number;
    mgmt_frame = (osal_u8 *)OAL_NETBUF_DATA(bss_mgmt_netbuf);
    /* 注意此处目的mgmt_buff结尾有alloc，长度应使用mgmt_len */
    if (memcpy_s(new_scanned_bss->bss_dscr_info.mgmt_buff, (osal_u32)mgmt_len, mgmt_frame, (osal_u32)mgmt_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_scan_update_bss_dscr_from_netbuf::memcpy_s error}");
    }

    ret = hmac_scan_update_bss_dscr(new_scanned_bss, scaned_extend_info, vap_id, mgmt_len, channel_number);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_SCAN, "vapid[%d]{hmac_scan_proc_scanned_bss_etc::update failed[%d]}", vap_id, ret);
        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(bss_mgmt_netbuf);
        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);
        return ret;
    }

    /* 如果之前的判断需要过滤 */
    if (new_scanned_bss->bss_dscr_info.need_drop == OSAL_TRUE) {
        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(bss_mgmt_netbuf);
        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if ((new_scanned_bss->bss_dscr_info.multi_bssid_capa == OSAL_TRUE) &&
        (mac_mib_get_multi_bssid_implement(hmac_vap) == OSAL_TRUE)) {
        hmac_scan_multi_bssid_proc(scaned_extend_info, dtx_event, channel_number, vap_id, &all_bss_info);
    }
#endif

    /* 刷新multiple bssid相关参数。若无有效multiple bssid element，这些参数无效，为默认值0 */
    all_bss_info.bssid_idx = 0;
    new_scanned_bss->bss_dscr_info.all_bss_info = all_bss_info;

    /* 判决是否将原始scan信息存入链表 */
    hmac_scan_insert_new_record(vap_id, new_scanned_bss, bss_mgmt_netbuf, OSAL_FALSE);
    return OAL_SUCC;
}

static osal_u8 hmac_get_chan_num_from_frame(osal_u8 *frame_body, osal_u16 frame_len, osal_u16 offset)
{
    osal_u8 chan_num = 0;
    osal_u8 *ie_start_addr = OSAL_NULL;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_DSPARMS, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[2];    /* DSSS Param set ie的第2字节 */
        return chan_num;
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_HT_OPERATION, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[2];    /* DSSS Param set ie的第2字节 */
        return chan_num;
    }

    return chan_num;
}

osal_void hmac_scan_delete_bss(hmac_vap_stru *hmac_vap, const osal_u8 *bssid)
{
    hmac_bss_mgmt_stru *bss_mgmt = OSAL_NULL; /* 管理扫描的bss结果的结构体 */
    hmac_scanned_bss_info *scanned_bss_info = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_delete_bss::hmac_device is null, dev id[%d].}",
            hmac_vap->device_id);
        return;
    }

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    osal_spin_lock(&(bss_mgmt->lock));
    scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, (osal_u8 *)bssid);
    if (scanned_bss_info == OSAL_NULL) {
        oam_info_log4(0, OAM_SF_SCAN,
            "{hmac_scan_delete_bss::find the bss failed[%02X:%02X:%02X:%02X:XX:XX]}",
            bssid[0], bssid[1], bssid[2], bssid[3]); /* 打印 0, 1, 2, 3 位mac */

        /* 解锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        return;
    }

    /* 从链表中删除节点，并更新扫描到的bss计数 */
    hmac_scan_del_bss_from_list_nolock(scanned_bss_info, hmac_device);

    /* 解锁 */
    osal_spin_unlock(&(bss_mgmt->lock));
    /* 释放删掉的节点内存 */
    oal_free(scanned_bss_info);
    oam_warning_log4(0, OAM_SF_SCAN, "{hmac_scan_delete_bss::delete bss success[%2x:%2x:%2x:%2x:xx:xx]}",
        bssid[0], bssid[1], bssid[2], bssid[3]); /* 0,1,2,3：mac地址位 */
    return;
}

static void hmac_scan_kick_invalid_user_for_sta(hmac_vap_stru *hmac_vap, const osal_u8 *bssid)
{
    mac_cfg_kick_user_param_stru kick_user_param;
    osal_s32 ret;
    frw_msg msg_info;

    /* 非sta模式直接返回 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }
    /* bss mac地址不同直接返回 */
    if (memcmp(bssid, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) != 0) {
        return;
    }
    /* 当前未关联直接返回 */
    if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
        return;
    }

    (osal_void)memset_s(&kick_user_param, OAL_SIZEOF(kick_user_param), 0, OAL_SIZEOF(kick_user_param));
    kick_user_param.reason_code = MAC_UNSPEC_REASON;
    if (memcpy_s(kick_user_param.mac_addr, sizeof(kick_user_param.mac_addr), hmac_vap->bssid,
        sizeof(hmac_vap->bssid)) != EOK) {
        oam_warning_log0(0, OAM_SF_SCAN, "hmac_scan_kick_invalid_user_for_sta::memcpy_s fail!}");
        return;
    }
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)&kick_user_param;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(kick_user_param);

    /* 踢掉该vap的用户 */
    ret = hmac_config_kick_user_etc(hmac_vap, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_kick_invalid_user_for_sta::hmac_config_kick_user_etc fail!}", hmac_vap->vap_id);
    } else {
        oam_warning_log4(0, OAM_SF_SCAN, "{hmac_scan_kick_invalid_user_for_sta::kick success[%2x:%2x:%2x:%2x:xx:xx]}",
            bssid[0], bssid[1], bssid[2], bssid[3]); /* 1,2,3：mac地址位 */
    }
    return;
}

static osal_u32 hmac_scan_check_invalid_bss_by_channel(hmac_vap_stru *hmac_vap,
    osal_u8 *mgmt_frame, osal_u16 mgmt_len)
{
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_u8 frame_channel;
    mac_ieee80211_frame_stru *frame_header = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;

    /* 获取管理帧的帧头和帧体指针 */
    frame_header = (mac_ieee80211_frame_stru *)mgmt_frame;
    frame_body = mgmt_frame + MAC_80211_FRAME_LEN;

    /* 获取管理帧中的信道 */
    frame_channel = hmac_get_chan_num_from_frame(frame_body, (mgmt_len - MAC_80211_FRAME_LEN), offset);
    /* 帧中无信道信息，则继续执行添加bss */
    if (frame_channel == 0) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_check_invalid_bss_by_channel::not get frame channel.}");
        return OAL_CONTINUE;
    }

    /* 检查定制化5g开关是否使能 */
    if (hmac_device_check_5g_enable(hmac_vap->device_id) == OAL_FALSE) {
        if (frame_channel > MAX_CHANNEL_NUM_FREQ_2G) {
            return OAL_FAIL;
        }
    }

    /* 帧中信道合法，则继续执行添加bss */
    if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(frame_channel), frame_channel) == OAL_SUCC) {
        return OAL_CONTINUE;
    }
    /* 更新的帧中信道不合法，说明此AP已切换至不支持的信道，删除其对应bss信息 */
    oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_check_invalid_bss_by_channel::channel[%d] not in range.}",
        frame_channel);
    hmac_scan_kick_invalid_user_for_sta(hmac_vap, (const osal_u8 *)frame_header->address3);
    hmac_scan_delete_bss(hmac_vap, (const osal_u8 *)frame_header->address3);
    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_scan_proc_scanned_bss_etc
 功能描述  : 接收每个信道的扫描结果到host侧进行处理
*****************************************************************************/
OAL_STATIC osal_s32 hmac_scan_proc_scanned_bss_etc(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, mac_scanned_result_extend_info_stru *scan_result_ext_info)
{
    /* 获取事件头和事件结构体指针 */
    oal_netbuf_stru *bss_mgmt_netbuf = OSAL_NULL;
    dmac_tx_event_stru dtx_event = {0};
    osal_u16 mgmt_len;
    osal_u8 *mgmt_frame = OSAL_NULL;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_void *promis_fhook = hmac_get_feature_fhook(HMAC_FHOOK_PROMIS_SEND_CUSTOM_FRAME);
#endif
    if ((hmac_vap == OSAL_NULL) || (netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_scan_proc_scanned_bss_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    bss_mgmt_netbuf = netbuf;
    dtx_event.netbuf = bss_mgmt_netbuf;
    dtx_event.frame_len = (osal_u16)OAL_NETBUF_LEN(bss_mgmt_netbuf);

    mgmt_len = dtx_event.frame_len;
    mgmt_frame = (osal_u8 *)OAL_NETBUF_DATA(bss_mgmt_netbuf);
    if (hmac_scan_check_invalid_bss_by_channel(hmac_vap, mgmt_frame, mgmt_len) != OAL_CONTINUE) {
        oal_netbuf_free(bss_mgmt_netbuf);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (promis_fhook != OSAL_NULL) {
        ((hmac_promis_send_custom_frame_cb)promis_fhook)(hmac_vap, bss_mgmt_netbuf);
    }
#endif

    return (osal_s32)hmac_scan_update_bss_dscr_from_netbuf(hmac_vap, &dtx_event, bss_mgmt_netbuf,
        scan_result_ext_info, mgmt_len);
}

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
 功能描述  : 打印信道统计信息
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_print_channel_statistics_info(hmac_scan_record_stru *scan_record)
{
    wlan_scan_chan_stats_stru    *chan_stats = scan_record->chan_results;
    osal_u8                    idx;

    /* 检测本次扫描是否开启了信道测量，如果没有直接返回 */
    if (chan_stats[0].stats_valid == 0) {
        oam_info_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_print_channel_statistics_info:: curr scan don't enable channel measure.\n}",
            scan_record->vap_id);
        return;
    }

    /* 打印信道测量结果 */
    common_log_dbg1(0, OAM_SF_SCAN,
        "vap_id[%d] {hmac_scan_print_channel_statistics_info:: The chan measure result: \n}", scan_record->vap_id);

    for (idx = 0; idx < scan_record->chan_numbers; idx++) {
        oam_info_log4(0, OAM_SF_SCAN, "vap_id[%d] {Chan num[%d] Stats cnt[%d] Stats valid[%d]\n",
            scan_record->vap_id, chan_stats[idx].channel_number, chan_stats[idx].stats_cnt,
            chan_stats[idx].stats_valid);
        oam_info_log4(0, OAM_SF_SCAN, "Stats time us[%d] Free time 20M[%d] 40M[%d]; Free power 40M[%d]\n",
            chan_stats[idx].total_stats_time_us, chan_stats[idx].total_free_time_20m_us,
            chan_stats[idx].total_free_time_40m_us, (osal_s32)chan_stats[idx].free_power_stats_40m);
        oam_info_log4(0, OAM_SF_SCAN, "Send time[%d] Recv time[%d] Free power cnt[%d]; Free power 20M[%d]\n",
            chan_stats[idx].total_send_time_us, chan_stats[idx].total_recv_time_us, chan_stats[idx].free_power_cnt,
            (osal_s32)chan_stats[idx].free_power_stats_20m);
#ifdef _PRE_WLAN_SUPPORT_5G
        oam_info_log2(0, OAM_SF_SCAN, "Free power 80M[%d] Free time 80M[%d]\n",
            (osal_s32)chan_stats[idx].free_power_stats_80m, chan_stats[idx].total_free_time_80m_us);
#endif
    }

    return;
}

/*****************************************************************************
 功能描述  : 扫描完成，打印相关维测信息，包括扫描执行时间，扫描返回状态码，扫描到的bss信息等
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_print_scan_record_info(hmac_vap_stru *hmac_vap,
    hmac_scan_record_stru *scan_record)
{
    unref_param(hmac_vap);
// liteos系统，不调用linux内核函数
#if defined(_PRE_WLAN_FEATURE_WS73) && !defined(_PRE_OS_VERSION_LITEOS)
#if defined(_PRE_PRODUCT_ID_HOST)
    osal_u64 timestamp_diff = osal_get_time_stamp_ms() - scan_record->scan_start_time; /* 获取扫描间隔时间戳 */

    /* 调用内核接口，打印此次扫描耗时 */
    common_log_dbg4(0, OAM_SF_SCAN,
        "{hmac_scan_print_scan_record_info::scan comp, scan_status[%d],vap ch_num:%d,"
        " cookie[%x], duration time is: [%llu]ms.}",
        scan_record->scan_rsp_status,
        hmac_vap->channel.chan_number,
        scan_record->ull_cookie,
        timestamp_diff);
#endif
#endif
    /* 如果是信道测量发起的扫描，不需要打印bss信息 */
    if (scan_record->is_chan_meas_scan != OSAL_TRUE) {
        /* 打印扫描到的bss信息 */
        hmac_scan_print_scanned_bss_info_etc(scan_record->device_id);
    }

    /* 信道测量结果 */
    hmac_scan_print_channel_statistics_info(scan_record);

    return;
}
#endif

OAL_STATIC osal_void hmac_scan_proc_scan_comp_event_etc_proc(hmac_scan_stru *scan_mgmt,
    mac_scan_rsp_stru *d2h_scan_rsp_info, hmac_vap_stru *hmac_vap)
{
    osal_void *fhook;

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (d2h_scan_rsp_info->scan_rsp_status != MAC_SCAN_PNO) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_SCAN)) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    }

    /* 作为ap 40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_mgmt->scan_record_mgmt.vap_last_state != MAC_VAP_STATE_BUTT)) {
        if (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) {
            hmac_vap_state_change_etc(hmac_vap, scan_mgmt->scan_record_mgmt.vap_last_state);
        } else {
            hmac_fsm_change_state_etc(hmac_vap, scan_mgmt->scan_record_mgmt.vap_last_state);
        }
        scan_mgmt->scan_record_mgmt.vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */

    /* 根据device上报的扫描结果，上报sme */
    /* 将扫描执行情况(扫描执行成功、还是失败等返回结果)记录到扫描运行记录结构体中 */
    scan_mgmt->scan_record_mgmt.scan_rsp_status = d2h_scan_rsp_info->scan_rsp_status;
    scan_mgmt->scan_record_mgmt.ull_cookie         = d2h_scan_rsp_info->cookie;

#ifdef _PRE_WLAN_DFT_STAT
    hmac_scan_print_scan_record_info(hmac_vap, &(scan_mgmt->scan_record_mgmt));
#endif

    /* 如果扫描回调函数不为空，则调用回调函数,abort也要通知内核防止内核死等 */
    if (scan_mgmt->scan_record_mgmt.p_fn_cb != OAL_PTR_NULL) {
        /* 终止扫描无需调用回调,防止终止扫描结束对随后发起PNO扫描的影响 */
#ifdef _PRE_BSLE_GATEWAY
        /* BSLE网关形态需要循环下发扫描，在调用回调之前，需要把扫描状态置为false */
        scan_mgmt->is_scanning = OAL_FALSE;
#endif
        scan_mgmt->scan_record_mgmt.p_fn_cb(&(scan_mgmt->scan_record_mgmt));
    } else if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (d2h_scan_rsp_info->scan_rsp_status != MAC_SCAN_ABORT_SYNC)) {
        /* 解决上层去关联命令时host上报扫描abort状态后dmac又抛上来一次扫描abort状态导致时序乱了的问题 */
        hmac_cfg80211_scan_comp(&(scan_mgmt->scan_record_mgmt));
    }

    /* 设置当前处于非扫描状态 */
    if (d2h_scan_rsp_info->scan_rsp_status != MAC_SCAN_PNO) {
        /* PNO扫描没有置此位为OAL_TRUE,PNO扫描结束,不能影响随后的常规扫描 */
        scan_mgmt->is_scanning = OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) {
        hmac_p2p_listen_timeout_etc(hmac_vap, OSAL_NULL);
    }

    if (hmac_vap->wait_roc_end == OAL_TRUE) {
        /* 有此标记就通知结束,防止wal层下发扫描的时候在扫描,下发abort,
           但此时刚好扫描正常结束,abort未执行,这里就未complete */
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_comp_event_etc::scan rsp status[%d]}",
                         hmac_vap->vap_id, d2h_scan_rsp_info->scan_rsp_status);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        OAL_COMPLETE(&(hmac_vap->roc_end_ready));
#endif
        hmac_vap->wait_roc_end = OAL_FALSE;
    }
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_CHECK_BKSCAN);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_check_bkscan_result_etc_cb)fhook)(hmac_vap, &(scan_mgmt->scan_record_mgmt));
    }
}


/*****************************************************************************
 功能描述  : DMAC扫描完成事件处理
*****************************************************************************/
osal_s32 hmac_scan_proc_scan_comp_event_etc(hmac_vap_stru *hmac_vap, mac_scan_rsp_stru *scan_rsp_info)
{
    hmac_device_stru                   *hmac_device = OSAL_NULL;
    hmac_scan_stru                     *scan_mgmt = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (scan_rsp_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event_etc::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);

    /* 防止compete事件和正在处理的扫描不一致 */
    if ((hmac_vap->vap_id != scan_mgmt->scan_record_mgmt.vap_id) ||
        (scan_rsp_info->cookie != scan_mgmt->scan_record_mgmt.ull_cookie)) {
        oam_warning_log4(0, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_comp_event_etc::Report vap(%d) Scan_complete(cookie %d),"
            " but there have anoter vap(%d) scaning(cookie %d) !}",
            hmac_vap->vap_id,
            scan_rsp_info->cookie,
            scan_mgmt->scan_record_mgmt.vap_id,
            scan_mgmt->scan_record_mgmt.ull_cookie);
        return OAL_SUCC;
    }

    common_log_dbg2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_comp_event_etc::scan status:%d !}",
        hmac_vap->vap_id, scan_rsp_info->scan_rsp_status);

    /* 删除扫描超时保护定时器 */
    if ((scan_mgmt->scan_timeout.is_registerd == OAL_TRUE) &&
        (scan_rsp_info->scan_rsp_status != MAC_SCAN_PNO)) {
        /* PNO没有启动扫描定时器,考虑到取消PNO扫描,立即下发普通扫描,PNO扫描结束事件对随后的普通扫描的影响 */
        frw_destroy_timer_entry(&(scan_mgmt->scan_timeout));
    }

    hmac_scan_proc_scan_comp_event_etc_proc(scan_mgmt, scan_rsp_info, hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 异常扫描请求,抛事件到wal 层，执行扫描完成
*****************************************************************************/
osal_u32 hmac_scan_proc_scan_req_event_exception_etc(hmac_vap_stru *hmac_vap, osal_void *p_params)
{
    hmac_scan_rsp_stru scan_rsp;
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (p_params == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_exception_etc::param null, %p %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 不支持发起扫描的状态发起了扫描 */
    oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_req_event_exception_etc::vap state is=%x.}",
                     hmac_vap->vap_id,
                     hmac_vap->vap_state);

    memset_s(&scan_rsp, OAL_SIZEOF(hmac_scan_rsp_stru), 0, OAL_SIZEOF(hmac_scan_rsp_stru));

    scan_rsp.result_code = MAC_SCAN_REFUSED;
#ifdef _PRE_WLAN_FEATURE_ROAM
    /* When STA is roaming, scan req return success instead of failure,
       in case roaming failure which will cause UI scan list null  */
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        scan_rsp.result_code = MAC_SCAN_SUCCESS;
    }
#endif
    scan_rsp.num_dscr    = 0;

    msg_info.data = (osal_void *)(&scan_rsp);
    msg_info.data_len = (osal_u16)OAL_SIZEOF(hmac_scan_rsp_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_SCAN_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    return (osal_u32)ret;
}

/*****************************************************************************
 功能描述  : 设置probe req帧中携带的源mac addr，如果随机mac addr特性开启，则携带随机mac addr
*****************************************************************************/
osal_void hmac_scan_set_sour_mac_addr_in_probe_req_etc(hmac_vap_stru *hmac_vap,
    osal_u8 *sour_mac_addr, oal_bool_enum_uint8 is_rand_mac_scan)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (sour_mac_addr == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
            "{hmac_scan_set_sour_mac_addr_in_probe_req_etc::param null,hmac_vap:%p,sour_mac_addr:%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)sour_mac_addr);
        return;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req_etc::hmac_device is null.}");
        return;
    }

    /* SCAN_RANDOM_WITH_OUI：按照mac_oui机制产生随机mac addr，非P2P场景，设置随机mac addr到probe req帧中 */
    if ((is_rand_mac_scan == SCAN_RANDOM_WITH_OUI) && (is_legacy_vap(hmac_vap)) &&
        ((hmac_device->mac_oui[0] != 0) || (hmac_device->mac_oui[1] != 0) ||
        (hmac_device->mac_oui[2] != 0))) { /* MAC判断;2 数组下标 */
        /* 更新随机mac 地址,使用下发随机MAC OUI 生成的随机mac 地址更新到本次扫描 */
        oal_set_mac_addr(sour_mac_addr, hmac_device->scan_mgmt.random_mac);
    } else {
        /* 设置地址为自己的MAC地址 */
        oal_set_mac_addr(sour_mac_addr, mac_mib_get_station_id(hmac_vap));

        /* SCAN_RANDOM_FORCE:强制随机mac，非P2P场景 */
        if ((is_rand_mac_scan == SCAN_RANDOM_FORCE) && (is_legacy_vap(hmac_vap))) {
            osal_u8 random_mac[WLAN_MAC_ADDR_LEN] = {0};
            oal_eth_random_addr(random_mac, sizeof(random_mac));
            /* 根据基础mac，改变3、4、5最后三位做随机mac */
            sour_mac_addr[3] = random_mac[3]; /* 3 MAC判断 */
            sour_mac_addr[4] = random_mac[4]; /* 4 MAC修改 */
            sour_mac_addr[5] = random_mac[5]; /* 5 MAC判断 */
        }
    }

    return;
}

OAL_STATIC osal_u32 hmac_scan_set_scan_params(mac_scan_req_stru *scan_params, hmac_vap_stru *hmac_vap,
    hmac_vap_stru *mac_vap_temp, hmac_device_stru *hmac_device)
{
    wlan_vap_mode_enum_uint8 vap_mode;

    /* 判断vap的类型，如果是sta则为sta的背景扫描，如果是ap，则是ap的背景扫描，其它类型的vap暂不支持背景扫描 */
    vap_mode = hmac_vap->vap_mode;
    if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (scan_params->scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
            /* 修改扫描参数为sta的背景扫描 */
            scan_params->scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
        }
    } else if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 修改扫描参数为sta的背景扫描 */
        scan_params->scan_mode = WLAN_SCAN_MODE_BACKGROUND_AP;
    } else {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_scan_update_scan_params::vap mode[%d], not support bg scan.}", vap_mode);
        return OAL_FAIL;
    }
    scan_params->need_switch_back_home_channel = OAL_TRUE;

    if (hmac_device_calc_up_vap_num_etc(hmac_device) == 1 && !is_legacy_vap(mac_vap_temp) &&
        is_legacy_vap(hmac_vap)) {
        /* 修改扫描信道间隔(2)和回工作信道工作时间(60ms):
           仅仅针对P2P处于关联状态，wlan处于去关联状态,wlan发起的扫描 */
        scan_params->scan_channel_interval = (scan_params->scan_channel_interval == 0) ?
            MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE : scan_params->scan_channel_interval;
        scan_params->work_time_on_home_channel   = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;

        if ((scan_params->scan_time > WLAN_DEFAULT_ACTIVE_SCAN_TIME) &&
            (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
            /* 指定SSID扫描超过3个,会修改每次扫描时间为40ms(默认是20ms) */
            /* P2P关联但wlan未关联场景,考虑到扫描时间增加对p2p wfd场景的影响,设置每信道扫描次数为1次(默认为2次) */
            scan_params->max_scan_count_per_channel = 1;
        }
    } else {
        /* 其他情况默认扫描6个信道回home信道工作100ms */
        scan_params->scan_channel_interval = (scan_params->scan_channel_interval == 0) ?
            MAC_SCAN_CHANNEL_INTERVAL_DEFAULT : scan_params->scan_channel_interval;
        scan_params->work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_scan_update_scan_params_substep(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    mac_scan_req_stru *scan_params, oal_bool_enum_uint8 is_random_mac_scan)
{
    hmac_vap_stru               *hmac_vap_temp;
    osal_u8                    loop, is_5g_enable;
    osal_u8                    chan_cnt = 0;
    osal_u32                   ul_ret;

    /* 1.记录发起扫描的vap id到扫描参数 */
    scan_params->vap_id    = hmac_vap->vap_id;
    scan_params->need_switch_back_home_channel = OAL_FALSE;
    if (scan_params->scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
        scan_params->scan_mode = WLAN_SCAN_MODE_FOREGROUND;
    }

    /* 2.修改扫描模式和信道扫描次数: 根据是否存在up状态下的vap，如果是，则是背景扫描，如果不是，则是前景扫描 */
    ul_ret = mac_device_find_up_vap_etc(hmac_device, &hmac_vap_temp);
    if ((ul_ret == OAL_SUCC) && (hmac_vap_temp != OAL_PTR_NULL)) {
        if (hmac_scan_set_scan_params(scan_params, hmac_vap, hmac_vap_temp, hmac_device) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    /* 3.设置发送的probe req帧中源mac addr */
    scan_params->is_random_mac_addr_scan = (is_random_mac_scan != 0) ? OSAL_TRUE : OSAL_FALSE;
    hmac_scan_set_sour_mac_addr_in_probe_req_etc(hmac_vap, scan_params->sour_mac_addr, is_random_mac_scan);

    is_5g_enable = (osal_u8)hmac_device_check_5g_enable(hmac_vap->device_id);

    for (loop = 0; loop < scan_params->channel_nums; loop++) {
        if (chan_cnt != loop) {
            scan_params->channel_list[chan_cnt] = scan_params->channel_list[loop];
        }
        /* 判断设备是否支持5g   如果不支持则判断信道列表中是否有5g信道 */
        if ((is_5g_enable == OAL_FALSE) && (scan_params->channel_list[loop].chan_number > MAX_CHANNEL_NUM_FREQ_2G)) {
            return OAL_FAIL;
        }
        chan_cnt++;
    }
    if (chan_cnt == 0) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "{hmac_scan_update_scan_params::channels trimed to none!, ori cnt=%d}", scan_params->channel_nums);

        return OAL_FAIL;
    }
    scan_params->channel_nums = chan_cnt;

    return OAL_SUCC;
}


/*****************************************************************************
 功能描述  : 根据device下所有的vap状态以及其它信息，更新扫描参数:
             包括发起扫描者的vap id、扫描模式、每信道扫描次数、probe req帧携带的源mac addr
*****************************************************************************/
OAL_STATIC osal_u32  hmac_scan_update_scan_params(hmac_vap_stru        *hmac_vap,
    mac_scan_req_stru    *scan_params,
    oal_bool_enum_uint8   is_random_mac_scan)
{
    hmac_device_stru            *hmac_device;
    osal_u32                   ul_ret;

    /* 获取mac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_update_scan_params::hmac_device null.}",
            hmac_vap->device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    ul_ret = hmac_scan_update_scan_params_substep(hmac_vap, hmac_device, scan_params, is_random_mac_scan);
    if (ul_ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 检测是否能够发起扫描，如果可以，则记录扫描请求者的信息，并清空上一次扫描结果
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_check_can_enter_scan_state(hmac_vap_stru *hmac_vap)
{
    /* p2p有可能进行监听动作，但是和scan req的优先级一样，因此当上层发起的是扫描请求时，统一可使用下面的接口判断 */
#ifdef _PRE_WLAN_FEATURE_P2P
    return hmac_p2p_check_can_enter_state_etc(hmac_vap, HMAC_FSM_INPUT_SCAN_REQ);
#else
    unref_param(hmac_vap);
    return OAL_SUCC;
#endif
}

/*****************************************************************************
 功能描述  : 检测是否能够发起扫描，如果可以，则记录扫描请求者的信息，并清空上一次扫描结果
*****************************************************************************/
OAL_STATIC osal_u32  hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
    osal_u32  ul_ret;

    /* 1.先检测其它vap的状态从而判断是否可进入扫描状态，使得扫描尽量不打断其它的入网流程 */
    ul_ret = hmac_scan_check_can_enter_scan_state(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_check_is_dispatch_scan_req::Because of err_code[%d], can't enter into scan state.}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 2.判断当前扫描是否正在执行 */
    if (hmac_device->scan_mgmt.is_scanning == OAL_TRUE || hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_check_is_dispatch_scan_req::the scan request is rejected.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 3.判断当前是否正在执行漫游 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_check_is_dispatch_scan_req:: roam reject new scan.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }
#endif // _PRE_WLAN_FEATURE_ROAM

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 处理上一次的扫描记录，接口封装，从而便于可扩展(未来可能使用老化机制判断是否清除)
*****************************************************************************/
OAL_STATIC osal_void  hmac_scan_proc_last_scan_record(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device)
{
    common_log_dbg0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_etc:: start clean last scan record.}");

    /* 本次扫描请求发起时，清除上一次扫描结果中过期的bss信息 */
    hmac_scan_clean_expire_scanned_bss(hmac_vap, &(hmac_device->scan_mgmt.scan_record_mgmt),
        HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE);

    return;
}

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
/*****************************************************************************
 功能描述  : host侧定时清理扫描结果处理函数，减少内存资源占用
*****************************************************************************/
osal_s32 hmac_scan_clean_result(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);

    oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_result::clean driver scan results.}");
    hmac_scan_clean_expire_scanned_bss(hmac_vap, &(hmac_device->scan_mgmt.scan_record_mgmt),
        HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : host侧抛扫描请求时间到device侧，防止因核间通信、抛事件等异常情况，host侧接收不到
             扫描响应的超时回调函数处理，属于扫描模块内的超时保护
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_proc_scan_timeout_fn(void *p_arg)
{
    hmac_device_stru                   *hmac_device = (hmac_device_stru *)p_arg;
    hmac_vap_stru                      *hmac_vap    = OAL_PTR_NULL;
    hmac_scan_record_stru              *scan_record = OAL_PTR_NULL;
    osal_u32                          pedding_data = 0;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    /* 获取扫描记录信息 */
    scan_record = &(hmac_device->scan_mgmt.scan_record_mgmt);

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->vap_id);
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_timeout_fn::hmac_vap null.}",
                       scan_record->vap_id);

        /* 扫描状态恢复为未在执行的状态 */
        hmac_device->scan_mgmt.is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    /* 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_record->vap_last_state != MAC_VAP_STATE_BUTT)) {
        if (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) {
            hmac_vap_state_change_etc(hmac_vap, scan_record->vap_last_state);
        } else {
            hmac_fsm_change_state_etc(hmac_vap, scan_record->vap_last_state);
        }
        scan_record->vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */

    /* 设置扫描响应状态为超时 */
    scan_record->scan_rsp_status = MAC_SCAN_TIMEOUT;
    oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_timeout_fn::scan time out cookie [%x].}",
                     scan_record->vap_id, scan_record->ull_cookie);

    /* 如果扫描回调函数不为空，则调用回调函数 */
    if (scan_record->p_fn_cb != OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_timeout_fn::scan callback func proc.}",
                         scan_record->vap_id);
        scan_record->p_fn_cb(scan_record);
    }

    msg_info.data = (osal_u8 *)&pedding_data;
    msg_info.data_len = OAL_SIZEOF(osal_u32);
    hmac_config_scan_abort_etc(hmac_vap, &msg_info);

    /* 扫描状态恢复为未在执行的状态 */
    hmac_device->scan_mgmt.is_scanning = OAL_FALSE;

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_scan_proc_scan_recoder_proc(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    mac_scan_req_stru *scan_params)
{
    hmac_scan_record_stru      *scan_record = OSAL_NULL;

    /* 处理上一次扫描记录，目前直接清楚上一次结果，后续可能需要老化时间处理 */
    hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->scan_mgmt.scan_record_mgmt);
    scan_record->chip_id      = hmac_device->chip_id;
    scan_record->device_id    = hmac_device->device_id;
    scan_record->vap_id       = scan_params->vap_id;
    scan_record->chan_numbers = scan_params->channel_nums;
    scan_record->p_fn_cb         = scan_params->fn_cb;
    scan_record->is_chan_meas_scan = OSAL_FALSE;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_proc_scan_req_event_etc::save last vap_state:%d}",
            hmac_vap->vap_id, hmac_vap->vap_state);

        scan_record->vap_last_state = hmac_vap->vap_state;
    }

    scan_record->ull_cookie = scan_params->cookie;

    /* 记录扫描开始时间 */
    scan_record->scan_start_time = osal_get_time_stamp_ms();
}
/*****************************************************************************
 功能描述  : 将总信道列表（包含5G与2.4G）中的部分信道随机化
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_randomzie_channel_partial(mac_scan_req_stru *scan_params,
    osal_u8 start_ch_index, osal_u8 len)
{
    osal_u8 index = start_ch_index;
    osal_u8 end = start_ch_index + len;
    osal_u8 random_index = 0;
    osal_u32 random_seed = 0;
    mac_channel_stru tmp_channel;
    for (; index < end; index++) {
        // 产生start_ch_index + 1 ~ end - 1 的随机数 ,即0 ~ ((end - 1) - (start_ch_index + 1)) 加上 (start_ch_index + 1)
        random_seed = (osal_u32)oal_time_get_stamp_ms();
        if (((end - 1) - (start_ch_index + 1)) < 1) {
            return;
        }
        random_index = oal_gen_random(random_seed, 1) % ((end - 1) - (start_ch_index + 1)) + (start_ch_index + 1);
        tmp_channel = scan_params->channel_list[index];
        scan_params->channel_list[index] = scan_params->channel_list[random_index];
        scan_params->channel_list[random_index] = tmp_channel;
    }
}

/*****************************************************************************
 功能描述      : 获取当前hmac侧下发扫描请求时信道是否需要随机化
*****************************************************************************/
OAL_STATIC osal_bool hmac_scan_get_bss_list_exceed_info_randomize(hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru  *bss_mgmt;      /* 管理扫描结果的结构体 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    return bss_mgmt->bss_list_exceed_info.randomize;
}

/*****************************************************************************
 功能描述      : 处理信道随机化
*****************************************************************************/
osal_void hmac_scan_handle_channel_randomzie(mac_scan_req_stru *scan_params, osal_u8 channel_num_5g,
    osal_u8 channel_num_2g)
{
    osal_u8 channel_index;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_scan_get_bss_list_exceed_info_randomize(hmac_device)) {
        // 分别随机化5G与2.4G
        hmac_scan_randomzie_channel_partial(scan_params, 0, channel_num_5g);
        hmac_scan_randomzie_channel_partial(scan_params, channel_num_5g, channel_num_2g);
        // 打印随机化后的信道列表
        for (channel_index = 0; channel_index < scan_params->channel_nums; channel_index++) {
            oam_warning_log2(0, OAM_SF_SCAN,
                "hmac_scan_handle_channel_randomzie::channel_number[%d], channel_index[%d]",
                scan_params->channel_list[channel_index].chan_number,
                scan_params->channel_list[channel_index].chan_idx);
        }
    }
}

OAL_STATIC osal_u32 hmac_scan_proc_scan_send_event(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    mac_scan_req_stru *scan_params)
{
    osal_u32                  scan_timeout;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    /* 如果发起扫描的vap的模式为sta，并且，其关联状态为非up状态，且非p2p监听状态，则切换其扫描状态 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (scan_params->scan_func != MAC_SCAN_FUNC_P2P_LISTEN)) {
        if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
            /* 切换vap的状态为WAIT_SCAN状态 */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_WAIT_SCAN);
        }
    }

    /* AP的启动扫描做特殊处理，当hostapd下发扫描请求时，VAP还处于INIT状态 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_INIT)) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_proc_scan_req_event_etc::ap startup scan.}",
            hmac_vap->vap_id);
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
    }
    /* 打印扫描参数，测试使用 */
    /* 如果是P2P 发起监听，则设置HMAC 扫描超时时间为P2P 监听时间 */
    if (scan_params->scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
        scan_timeout = scan_params->scan_time * 2; /* 2倍 */
    } else {
        scan_timeout = WLAN_DEFAULT_MAX_TIME_PER_SCAN;
    }
    hmac_scan_print_scan_params(scan_params, hmac_vap);

    common_log_dbg3(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_proc_scan_send_event::start scan timer[%d] cookie [%x].}",
        hmac_vap->vap_id, scan_timeout, scan_params->cookie);

    /* 启动扫描保护定时器，防止因拋事件、核间通信失败等情况下的异常保护，定时器初步的超时时间为4.5秒 */
    frw_create_timer_entry(&(hmac_device->scan_mgmt.scan_timeout),
                           hmac_scan_proc_scan_timeout_fn,
                           scan_timeout,
                           hmac_device,
                           OAL_FALSE);

    cfg_msg_init((osal_u8 *)scan_params, OAL_SIZEOF(mac_scan_req_stru), OAL_PTR_NULL, 0, &msg_info);
    (osal_void)hmac_scan_proc_scan_req_event(hmac_vap, &msg_info);
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_scan_proc_scan_fill_param_from_hmac_vap(mac_scan_req_stru *scan_params,
    hmac_vap_stru *hmac_vap)
{
    if ((hmac_vap->max_scan_count_per_channel != 0) || (hmac_vap->scan_time != 0) ||
        (hmac_vap->work_time_on_home_channel != 0)) {
        oam_warning_log3(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_etc::count:%d time:%d on_home:%d.}",
            hmac_vap->max_scan_count_per_channel, hmac_vap->scan_time, hmac_vap->work_time_on_home_channel);
    }
    if (hmac_vap->max_scan_count_per_channel != 0) {
        scan_params->max_scan_count_per_channel = hmac_vap->max_scan_count_per_channel;
    }
    if (hmac_vap->scan_time != 0) {
        /* 校验是否可以修改扫描时间 */
        /* 如果指定SSID个数大于3个,则会调整发送超时时间为40ms, 默认的发送等待超时是20ms */
        if ((scan_params->max_send_probe_req_count_per_channel > 3) &&
            (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE) && (hmac_vap->scan_time < WLAN_LONG_ACTIVE_SCAN_TIME)) {
            oam_warning_log1(0, OAM_SF_SCAN, "scan_fill_param_from_hmac_vap:not set time:%d", hmac_vap->scan_time);
        } else {
            scan_params->scan_time = hmac_vap->scan_time;
        }
    }
    if (hmac_vap->work_time_on_home_channel != 0) {
        scan_params->work_time_on_home_channel = hmac_vap->work_time_on_home_channel;
    }
    if (hmac_vap->scan_channel_interval != 0) {
        scan_params->scan_channel_interval = hmac_vap->scan_channel_interval;
    }
    if (hmac_vap->scan_type != 0) {
        /* scan_type 扫描模式, 0-主动模式, 1-被动模式, 供ccpriv命令使用, 跟正常宏定义的值刚好相反 */
        scan_params->scan_type = WLAN_SCAN_TYPE_PASSIVE;
    }
    if (hmac_vap->single_probe_send_times != 0) {
        /* 单个probe req发送次数,相当于host侧重发次数, 最多7次 */
        scan_params->single_probe_times = hmac_vap->single_probe_send_times;
    }
}

/*****************************************************************************
 功能描述  : 处理扫描请求的总入口
*****************************************************************************/
osal_u32  hmac_scan_proc_scan_req_event_etc(hmac_vap_stru *hmac_vap, osal_void *p_params)
{
    mac_scan_req_stru *scan_params = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;
    osal_u32 ret;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif

    /* 参数合法性检查 */
    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (p_params == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_SCAN, "{param null, %p %p.}", (uintptr_t)hmac_vap, (uintptr_t)p_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_params = (mac_scan_req_stru *)p_params;

    /* 异常判断: 扫描的信道个数为0 */
    if (scan_params->channel_nums == 0) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] channel_nums=0.", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, 0, "vap%d hmac_scan_proc_scan_req_event_etc:%d nul", hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 更新此次扫描请求的扫描参数 */
    if (scan_params->scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
        /* 监听状态下不设置随机MAC地址扫描，避免wlan0 监听状态下发送管理帧失败 */
        ret = hmac_scan_update_scan_params(hmac_vap, scan_params, OAL_FALSE);
    } else {
        /* 更新此次扫描请求的扫描参数 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        ret = hmac_scan_update_scan_params(hmac_vap, scan_params, wlan_customize_etc->random_mac_addr_scan);
#else
        ret = hmac_scan_update_scan_params(hmac_vap, scan_params, hmac_device->scan_mgmt.is_random_mac_addr_scan);
#endif
    }

    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_scan_update_scan_params error[%d]}", hmac_vap->vap_id, ret);
        return ret;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, (const hmac_device_stru *)hmac_device);
    if (ret != OAL_SUCC) {
        /* 扫描被拒绝必须恢复到原来的状态 */
        if (scan_params->scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap_state_change_etc(hmac_vap, hmac_device->st_p2p_info.last_vap_state);
        }

        oam_warning_log4(0, 0, "vap[%d] hmac_scan_check_is_dispatch_scan_req fail[%d] state[%d] p2p last state[%d]",
            hmac_vap->vap_id, ret, hmac_vap->vap_state, hmac_device->st_p2p_info.last_vap_state);
        return ret;
    }

    /* 设置扫描模块处于扫描状态，其它扫描请求将丢弃 */
    hmac_device->scan_mgmt.is_scanning = OAL_TRUE;

    /* 处理扫描发起者的信息 */
    hmac_scan_proc_scan_recoder_proc(hmac_vap, hmac_device, scan_params);
    hmac_scan_proc_scan_fill_param_from_hmac_vap(scan_params, hmac_vap);

    return hmac_scan_proc_scan_send_event(hmac_vap, hmac_device, scan_params);
}

/*****************************************************************************
 功能描述  : 处理dmac上报的信道扫描结果处理DMAC上报的信道测量结果
*****************************************************************************/
osal_s32 hmac_scan_process_chan_result_event_etc(hmac_crx_chan_result_stru *chan_result_param)
{
    hmac_device_stru                   *hmac_device;
    hmac_scan_record_stru              *scan_record;
    osal_u8                             scan_idx;

    if (OAL_UNLIKELY(chan_result_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_process_chan_result_event_etc::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_idx = chan_result_param->scan_idx;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_process_chan_result_event_etc::hmac_device is null.}");
        return OAL_FAIL;
    }

    scan_record = &(hmac_device->scan_mgmt.scan_record_mgmt);

    /* 检查上报的索引是否合法 */
    if (scan_idx >= scan_record->chan_numbers) {
        /* dmac上报的扫描结果超出了要扫描的信道个数 */
        oam_warning_log2(0, OAM_SF_SCAN,
            "{hmac_scan_process_chan_result_event_etc::result from dmac error! scan_idx[%d], chan_numbers[%d].}",
            scan_idx, scan_record->chan_numbers);

        return OAL_FAIL;
    }
    scan_record->chan_results[scan_idx] = chan_result_param->chan_result;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 初始化管理hmac侧bss数量限制及扫描随机化管理的结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_init_bss_list_exceed_info_etc(hmac_bss_list_exceed_info_stru *bss_list_exceed_info)
{
    bss_list_exceed_info->exceed_count = 0;
    bss_list_exceed_info->exceed_limit = 2; // 默认超过阈值限制为2次
    bss_list_exceed_info->randomize = OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 扫描模块初始化
*****************************************************************************/
osal_void  hmac_scan_init_etc(hmac_device_stru *hmac_device)
{
    hmac_scan_stru      *scan_mgmt;
    hmac_bss_mgmt_stru  *bss_mgmt;

    /* 初始化扫描管理结构体信息 */
    scan_mgmt = &(hmac_device->scan_mgmt);
    memset_s(scan_mgmt, OAL_SIZEOF(hmac_scan_stru), 0, OAL_SIZEOF(hmac_scan_stru));
    scan_mgmt->is_scanning = OAL_FALSE;
    scan_mgmt->complete = OAL_TRUE;
    scan_mgmt->scan_record_mgmt.vap_last_state = MAC_VAP_STATE_BUTT;

    /* 初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_mgmt->scan_record_mgmt.bss_mgmt);
    OSAL_INIT_LIST_HEAD(&(bss_mgmt->bss_list_head));
    osal_spin_lock_init(&(bss_mgmt->lock));

    // 初始化bss中ap数量限制管理与信道随机化设置
    hmac_scan_init_bss_list_exceed_info_etc(&(bss_mgmt->bss_list_exceed_info));

    /* 初始化内核下发扫描request资源锁 */
    osal_mutex_init(&(scan_mgmt->scan_request_mutexlock));

    /* 初始化 wiphy_mgmt 结构 */
    osal_wait_init(&scan_mgmt->wait_queue);

    /* 初始化扫描生成随机MAC 地址 */
    oal_eth_random_addr(hmac_device->scan_mgmt.random_mac, sizeof(hmac_device->scan_mgmt.random_mac));

    return;
}

/*****************************************************************************
 功能描述  : hmac扫描模块退出
*****************************************************************************/
osal_void  hmac_scan_exit_etc(hmac_device_stru *hmac_device)
{
    hmac_scan_stru *scan_mgmt = &(hmac_device->scan_mgmt);

    /* 清空扫描记录信息 */
    hmac_scan_clean_scan(scan_mgmt);

    osal_mutex_destroy(&(scan_mgmt->scan_request_mutexlock));
    osal_spin_lock_destroy(&(scan_mgmt->scan_record_mgmt.bss_mgmt.lock));

    /* 清除扫描管理结构体信息 */
    osal_adapt_wait_destroy(&scan_mgmt->wait_queue);
    memset_s(scan_mgmt, OAL_SIZEOF(hmac_scan_stru), 0, OAL_SIZEOF(hmac_scan_stru));
    scan_mgmt->is_scanning = OAL_FALSE;

    return;
}

/*****************************************************************************
 功能描述  : 启动对应device上所有处于WAIT START状态的VAP, 不支持sta
*****************************************************************************/
osal_u32 hmac_start_all_bss_of_device_etc(hmac_device_stru *hmac_device)
{
    osal_u8      idx;
    hmac_vap_stru *hmac_vap;

    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_start_all_bss_of_device_etc::device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(0, OAM_SF_ACS, "{hmac_start_all_bss_of_device_etc:device id=%d}", hmac_device->device_id);

    for (idx = 0; idx < hmac_device->vap_num; idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[idx]);
        if (!hmac_vap) {
            oam_warning_log2(0, OAM_SF_ACS, "hmac_start_all_bss_of_device_etc:null ap, idx=%d id=%d", idx,
                             hmac_device->vap_id[idx]);
            continue;
        }

        if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            ((hmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START)
#ifdef _PRE_WLAN_FEATURE_DBAC
            || (mac_is_dbac_enabled(hmac_device) && (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE))
#endif
            || (hmac_vap->vap_state == MAC_VAP_STATE_UP))) {
            hmac_chan_start_bss_etc(hmac_vap, OAL_PTR_NULL, WLAN_PROTOCOL_BUTT);
        } else {
            oam_warning_log2(0, OAM_SF_ACS,
                "vap_id[%d] vap %d not started\n",
                hmac_vap->vap_id, hmac_device->vap_id[idx]);
            continue;
        }
    }

    return OAL_SUCC;
}

/* 此函数随着scan模块上移打桩于此，需要ACS重写此接口 放到hmac_acs文件中 */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
/*****************************************************************************
 功能描述  : 获取bss的主、次信道
*****************************************************************************/
OAL_STATIC osal_u32 hmac_get_pri_sec_chan(mac_bss_dscr_stru *bss_dscr, osal_u32 *pul_pri_chan,
    osal_u32 *pul_sec_chan)
{
    *pul_pri_chan = *pul_sec_chan = 0;

    *pul_pri_chan = bss_dscr->st_channel.chan_number;

    oam_info_log1(0, OAM_SF_2040, "hmac_get_pri_sec_chan:bss_dscr->st_channel.en_bandwidth = %d\n",
                  bss_dscr->channel_bandwidth);

    if (bss_dscr->channel_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        *pul_sec_chan = *pul_pri_chan + 4; /* 向上扩频20M, 对应4个信道 */
    } else if (bss_dscr->channel_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        *pul_sec_chan = *pul_pri_chan - 4; /* 向下扩频20M, 对应4个信道 */
    } else {
        oam_warning_log1(0, OAM_SF_2040,
            "hmac_get_pri_sec_chan: bss_dscr is not support 40Mhz, *pul_sec_chan = %d\n", *pul_sec_chan);
    }

    oam_info_log2(0, OAM_SF_2040, "*pul_pri_chan = %d, *pul_sec_chan = %d\n\n", *pul_pri_chan, *pul_sec_chan);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 交换主、次信道
*****************************************************************************/
OAL_STATIC osal_void hmac_switch_pri_sec(mac_channel_stru *channel)
{
    if (channel->en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        channel->chan_number = channel->chan_number + 4; /* 向上扩频20M, 对应4个信道 */
        channel->en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
    } else if (channel->en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        channel->chan_number = channel->chan_number - 4; /* 向下扩频20M, 对应4个信道 */
        channel->en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
    } else {
        oam_warning_log1(0, OAM_SF_2040,
            "hmac_switch_pri_sec:en_bandwidth = %d\n not need obss scan\n", channel->en_bandwidth);
    }
}

typedef struct {
    osal_u32 pri_freq;
    osal_u32 sec_freq;
    osal_u32 affected_start;
    osal_u32 affected_end;
} hmac_freq_stru;

OAL_STATIC oal_bool_enum_uint8 hmac_obss_check_40mhz_2g_ext(hmac_bss_mgmt_stru *bss_mgmt,
    hmac_vap_stru *hmac_vap, hmac_freq_stru *hmac_freq, mac_channel_stru *dst_channel)
{
    osal_u32                  pri;
    osal_u32                  sec;
    osal_u32                  sec_chan, pri_chan;

    mac_bss_dscr_stru           *bss_dscr = OSAL_NULL;
    hmac_scanned_bss_info       *scanned_bss = OSAL_NULL;
    struct osal_list_head         *entry = OSAL_NULL;

    if (bss_mgmt == OSAL_NULL || hmac_vap == OSAL_NULL || hmac_freq == OSAL_NULL || dst_channel == OSAL_NULL) {
        return OAL_FALSE;
    }
    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历扫描到的bss信息 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);
        if (bss_dscr == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_2g::bss_dscr is NULL}");
            continue;
        }

        if (bss_dscr->c_rssi < HMAC_OBSS_RSSI_TH) {
            continue;
        }
        pri = (osal_u32)hmac_regdomain_get_freq_map_2g_etc()[bss_dscr->st_channel.chan_number - 1].freq;
        sec = pri;

        oam_info_log3(0, OAM_SF_2040, "vap_id[%d] bss_dscr->st_channel.chan_number = %d, pri = %d\n",
            hmac_vap->vap_id, bss_dscr->st_channel.chan_number, pri);

        /* 获取扫描到的BSS的信道、频点信息 */
        hmac_get_pri_sec_chan(bss_dscr, &pri_chan, &sec_chan);

        /* 该BSS为40MHz带宽,计算次信道频点 */
        if (sec_chan != 0) {
            if (sec_chan < pri_chan) {
                sec = pri - 20; /* 向下扩频20M */
            } else {
                sec = pri + 20; /* 向上扩频20M */
            }
        }

        if ((pri < hmac_freq->affected_start || pri > hmac_freq->affected_end) &&
            (sec < hmac_freq->affected_start || sec > hmac_freq->affected_end)) {
            continue;    /* not within affected channel range */
        }

        if (hmac_freq->pri_freq != pri) { // 有叠频，且不是同一主信道，则当前AP不能使用40M
            oam_warning_log4(0, OAM_SF_2040, "hmac_obss_check_40mhz_2g:40 MHz pri/sec <%d, %d >mismatch BSS <%d, %d>\n",
                hmac_freq->pri_freq, hmac_freq->sec_freq, pri, sec);
            /* 解除锁 */
            osal_spin_unlock(&(bss_mgmt->lock));

            // just trim bandwidth to 2G
            dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;

            return OAL_FALSE;
        }
    }

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return OAL_TRUE;
}


/*****************************************************************************
 功能描述  : 对2G 40MHz带宽情况进行处理，处理规则如下:
            1)  新BSS的主信、次信道与所有已有的20/40MHz BSS的主信道、次信道相同；
            2)  主信道与所有已有的20MHz BSS的运行信道相同。
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_obss_check_40mhz_2g(hmac_vap_stru          *hmac_vap,
    hmac_scan_record_stru *scan_record,
    mac_channel_stru      *dst_channel)
{
    osal_u32 pri_freq;
    osal_u32 sec_freq;
    osal_u32 affected_start;
    osal_u32 affected_end;
    hmac_freq_stru hmac_freq = { 0 };
    hmac_bss_mgmt_stru *bss_mgmt = OSAL_NULL;

    *dst_channel = hmac_vap->channel;

    /* 获取主信道、次信道中心频点 */
    pri_freq = (osal_u32)hmac_regdomain_get_freq_map_2g_etc()[hmac_vap->channel.chan_number - 1].freq;

    if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        sec_freq = pri_freq + 20; /* 向上扩频20M */
    } else if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        sec_freq = pri_freq - 20; /* 向下扩频20M */
    } else {
        oam_warning_log2(0, OAM_SF_2040, "vap_id[%d] hmac_obss_check_40mhz_2g:en_bandwidth = %d not need obss\n",
            hmac_vap->vap_id, hmac_vap->channel.en_bandwidth);
        return OAL_TRUE;
    }

    /* 2.4G共存检测,检测范围是40MHz带宽中心频点为中心,左右各25MHZ */
    affected_start   = ((pri_freq + sec_freq) >> 1) - 25; /* 25MHZ */
    affected_end     = ((pri_freq + sec_freq) >> 1) + 25; /* 25MHZ */

    oam_info_log3(0, OAM_SF_2040, "vap_id[%d] hmac_obss_check_40mhz_2g:40 MHz affected channel range: [%d, %d] MHz",
                  hmac_vap->vap_id, affected_start, affected_end);

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = &(scan_record->bss_mgmt);

    hmac_freq.pri_freq = pri_freq;
    hmac_freq.sec_freq = sec_freq;
    hmac_freq.affected_start = affected_start;
    hmac_freq.affected_end = affected_end;

    return hmac_obss_check_40mhz_2g_ext(bss_mgmt, hmac_vap, &hmac_freq, dst_channel);
}

OAL_STATIC osal_void hmac_obss_check_40mhz_5g_ext(hmac_bss_mgmt_stru *bss_mgmt, osal_u32 pri_chan,
    osal_u32 sec_chan, mac_channel_stru *dst_channel)
{
    osal_u8 match = OAL_FALSE;
    osal_u8 inverse = OAL_FALSE;
    osal_u8 pri_20_bss = OAL_FALSE;
    osal_u8 sec_20_bss = OAL_FALSE;

    struct osal_list_head *entry = OSAL_NULL;
    mac_bss_dscr_stru *bss_dscr = OSAL_NULL;
    hmac_scanned_bss_info *scanned_bss = OSAL_NULL;

    osal_u32                  bss_pri_chan;
    osal_u32                  bss_sec_chan;

    /*
      * Match PRI/SEC channel with any existing HT40 BSS on the same
      * channels that we are about to use (if already mixed order in
      * existing BSSes, use own preference).
      */

    // 是否有与当前AP主次信道完全一致或者相反的40M BSS，另外也许确认是否有在主信道或者次信道的20M BSS
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);
        if (bss_dscr == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_5g::bss_dscr is NULL}");
            continue;
        }
        if (bss_dscr->c_rssi < HMAC_OBSS_RSSI_TH) {
            continue;
        }

        hmac_get_pri_sec_chan(bss_dscr, &bss_pri_chan, &bss_sec_chan);
        if ((pri_chan == bss_pri_chan) && (bss_sec_chan == 0)) {
            pri_20_bss = OAL_TRUE; // 出现在当前AP主信道的20M BSS
        }
        if ((sec_chan == bss_pri_chan) && (bss_sec_chan == 0)) {
            sec_20_bss = OAL_TRUE; // 出现在当前AP次信道的20M BSS
        }
        if ((pri_chan == bss_sec_chan) && (sec_chan == bss_pri_chan)) {
            inverse = OAL_TRUE; // 出现与当前AP主次信道相反的
        }
        if ((pri_chan == bss_pri_chan) && (sec_chan == bss_sec_chan)) {
            match = OAL_TRUE; // 出现与当前AP主次信道一致的
        }
    }

    if ((match == OAL_FALSE) && (inverse == OAL_TRUE) && (pri_20_bss == OAL_FALSE)) {
        // 有相反的40M,没有一致的，且主20没有20M BSS,交换主次信道
        oam_warning_log0(0, OAM_SF_2040,
            "hmac_obss_check_40mhz_5g:switch own primary and secondary channel due to BSS overlap with\n");

        hmac_switch_pri_sec(dst_channel);
    } else if ((match == OAL_FALSE) && (inverse == OAL_TRUE) && (pri_20_bss == OAL_TRUE) && (sec_20_bss == OAL_FALSE)) {
        // 有相反的40M,没有一致的，且主20有20M BSS,次20没有20M BSS,需要交换主次信道并且切换到20M
        oam_warning_log0(0, OAM_SF_2040,
            "hmac_obss_check_40mhz_5g:switch own primary and secondary channel due to BSS overlap and to 20M\n");

        hmac_switch_pri_sec(dst_channel);
        dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    } else if (sec_20_bss == OAL_TRUE) {
        // 次信道有20M BSS，需要切换到20M
        oam_warning_log2(0, OAM_SF_2040, "hmac_obss_check_40mhz_5g:40 MHz pri/sec <%d, %d > to 20M\n",
                         pri_chan, sec_chan);
        dst_channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
}

/*****************************************************************************
 功能描述  : 对5G 40MHz带宽情况进行处理:
            1)  如果AP要在5G频带上建立一个20/40MHz BSS，而该5G频带与一个已有的20/40M BSS占用相同的两个信道的话，
                则一定要保证新建立的20/40MHz BSS的主信道和次信道与已有的BSS的主信道和次信道相匹配。如果已经存在
                两个或者更多的20/40M BSS，而它们的主信道与次信道互不匹配，则AP可以不受限制地选择与任何一个已有的
                BSS相匹配的主信道与次信道;
            2)  协议建议AP不要建立一个次信道被已有的20MHz BSS占用的20/40MHz BSS。反过来，协议也建议AP不要在已有
                的20/40MHz BSS的次信道上建立一个20MHz BSS。
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_obss_check_40mhz_5g(hmac_vap_stru *hmac_vap,
    hmac_scan_record_stru *scan_record, mac_channel_stru *dst_channel)
{
    osal_u32 pri_chan, sec_chan, pri_bss, sec_bss;
    hmac_bss_mgmt_stru *bss_mgmt = OAL_PTR_NULL;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;

    *dst_channel = hmac_vap->channel;

    /* 获取主信道和次信道 */
    pri_chan = hmac_vap->channel.chan_number;
    if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        sec_chan = pri_chan + 4; /* 向上扩频20M, 对应4个信道 */
    } else if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        sec_chan = pri_chan - 4; /* 向下扩频20M, 对应4个信道 */
    } else {
        oam_warning_log2(0, OAM_SF_2040, "vap_id[%d] hmac_obss_check_40mhz_5g: en_bandwidth = %d not need obss scan",
            hmac_vap->vap_id, hmac_vap->channel.en_bandwidth);
        return OAL_TRUE;
    }

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = &(scan_record->bss_mgmt);

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 若在次信道检测到Beacon, 但是主信道上没有, 则需要交换主次信道 */
    pri_bss = sec_bss = 0;
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);

        if (bss_dscr == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_2040, "{hmac_obss_check_40mhz_5g::bss_dscr is NULL}");
            continue;
        }
        if (bss_dscr->c_rssi < HMAC_OBSS_RSSI_TH) {
            continue;
        }

        oam_info_log2(0, OAM_SF_2040, "vap_id[%d] hmac_obss_check_40mhz_5g:bss channel_number = %d\n",
                      hmac_vap->vap_id, bss_dscr->st_channel.chan_number);
        if (bss_dscr->st_channel.chan_number == pri_chan) {
            pri_bss++;
        } else if (bss_dscr->st_channel.chan_number == sec_chan) {
            sec_bss++;
        }
    }

    if (sec_bss != 0 && (pri_bss == 0)) {
        oam_info_log1(0, OAM_SF_2040, "vap_id[%d] hmac_obss_check_40mhz_5g:Switch own primary and secondary channel "
            "to get secondary channel with no Beacons from other BSSes\n", hmac_vap->vap_id);

        hmac_switch_pri_sec(dst_channel);

        /* 此处主次交换完后直接返回即可, 按hostapd-2.4.0版本修改 */
        /* 释放锁 */
        osal_spin_unlock(&(bss_mgmt->lock));
        // 这里不必判断是否需要切换到20M
        return OAL_TRUE;
    }

    hmac_obss_check_40mhz_5g_ext(bss_mgmt, pri_chan, sec_chan, dst_channel);

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    return OAL_TRUE;
}
/*****************************************************************************
 功能描述  : 检查BSS状态，返回目的信道，而不改变vap信道
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_obss_check_40mhz(hmac_vap_stru          *hmac_vap,
    hmac_scan_record_stru *scan_record,
    mac_channel_stru      *dst_channel)
{
    return (hmac_vap->channel.band == WLAN_BAND_2G) ? hmac_obss_check_40mhz_2g(hmac_vap, scan_record,
        dst_channel) : hmac_obss_check_40mhz_5g(hmac_vap, scan_record, dst_channel);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

