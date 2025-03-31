/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac beacon rom
 * Create: 2020-7-8
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "oal_types.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "hmac_scan.h"
#include "hmac_tx_mgmt.h"
#include "hmac_csa_sta.h"
#include "hmac_feature_dft.h"
#include "wal_utils.h"
#include "frw_util_notifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HMAC_OBSS_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_u8 g_obss_sta_scan_disable = OSAL_TRUE;
/*****************************************************************************
 函 数 名  : mac_ie_proc_obss_ie
 功能描述  : 处理Overlapping BSS Scan Parameters IE，并更新STA相应MIB项
*****************************************************************************/
osal_u32 hmac_ie_proc_obss_scan_ie_etc(hmac_vap_stru *hmac_vap, const osal_u8 *payload)
{
    osal_u16 trigger_scan_intvl;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_ie_proc_obss_scan_ie_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /********************Overlapping BSS Scan Parameters element******************
     |ElementID |Length |OBSS    |OBSS   |BSS Channel   |OBSS Scan  |OBSS Scan   |
     |          |       |Scan    |Scan   |Width Trigger |Passive    |Active Total|
     |          |       |Passive |Active |Scan Interval |Total Per  |Per         |
     |          |       |Dwell   |Dwell  |              |Channel    |Channel     |
     ----------------------------------------------------------------------------
     |1         |1      |2       |2      |2             |2          |2           |
     ----------------------------------------------------------------------------
     |BSS Width   |OBSS Scan|
     |Channel     |Activity |
     |Transition  |Threshold|
     |Delay Factor|         |
     ------------------------
     |2           |2        |
    ***************************************************************************/
    if (payload[1] < MAC_OBSS_SCAN_IE_LEN) {
        return OAL_FAIL;
    }

    trigger_scan_intvl = oal_make_word16(payload[6], payload[7]); /* 6, 7 */
    if (trigger_scan_intvl == 0) {
        hmac_vap_set_peer_obss_scan_etc(hmac_vap, OSAL_FALSE);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    mac_mib_set_obss_scan_passive_dwell(hmac_vap, oal_make_word16(payload[2], payload[3])); /* 2, 3 */
    mac_mib_set_obss_scan_active_dwell(hmac_vap, oal_make_word16(payload[4], payload[5]));  /* 4, 5 */
    /* obss扫描周期最小180秒,最大600S, 初始化默认为300秒 */
    mac_mib_set_bss_width_trigger_scan_interval(hmac_vap,
        osal_min(osal_max(trigger_scan_intvl, 180), 600)); /* 180, 600 */
    mac_mib_set_obss_scan_passive_total_per_channel(hmac_vap, oal_make_word16(payload[8], payload[9])); /* 8, 9 */
    mac_mib_set_obss_scan_active_total_per_channel(hmac_vap, oal_make_word16(payload[10], payload[11])); /* 10, 11 */
    mac_mib_set_bss_width_channel_transition_delay_factor(hmac_vap,
        oal_make_word16(payload[12], payload[13])); /* 12, 13 */
    mac_mib_set_obss_scan_activity_threshold(hmac_vap, oal_make_word16(payload[14], payload[15])); /* 14, 15 */
    hmac_vap_set_peer_obss_scan_etc(hmac_vap, OSAL_TRUE);

    return OAL_SUCC;
}
/*****************************************************************************
 功能描述  : 准备obss扫描参数
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_prepare_obss_scan_params(mac_scan_req_stru *scan_params, const hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 chan_num_2g = 0;
    osal_u8 channel_num = 0;

    /* 1.设置发起扫描的vap id */
    scan_params->vap_id = hmac_vap->vap_id;

    /* 2.设置初始扫描请求的参数 */
    scan_params->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_params->scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
    scan_params->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    scan_params->max_scan_count_per_channel = 1;
    scan_params->scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;

    /* OBSS扫描通配ssid */
    scan_params->mac_ssid_set[0].ssid[0] = '\0';
    scan_params->ssid_num = 1;
    /* OBSS扫描设置Source MAC ADDRESS */
    if ((hmac_device->is_random_mac_addr_scan != 0) && ((hmac_device->mac_oui[0] != 0) ||
        (hmac_device->mac_oui[1] != 0) || (hmac_device->mac_oui[2] != 0))) { /* 确保第2位不为0 */
        scan_params->sour_mac_addr[0] = (hmac_device->mac_oui[0] & 0xfe); /* 保证是单播mac */
        scan_params->sour_mac_addr[1] = hmac_device->mac_oui[1]; /* 获取第1位地址数据 */
        scan_params->sour_mac_addr[2] = hmac_device->mac_oui[2]; /* 获取第2位数据 */
        osal_get_trng_bytes(&scan_params->sour_mac_addr[3], 3); /* 设置第3-5位的地址，随机生成 */
        scan_params->is_random_mac_addr_scan = OSAL_TRUE;
    } else {
        (osal_void)memcpy_s((osal_void *)(scan_params->sour_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);
    }

    /* OBSS扫描扫描只指定1个bssid，为广播地址 */
    (osal_void)memcpy_s((osal_void *)(scan_params->bssid[0]), WLAN_MAC_ADDR_LEN,
        (osal_void *)(BROADCAST_MACADDR), WLAN_MAC_ADDR_LEN);
    scan_params->bssid_num = 1;

    /* 设置扫描模式为OBSS扫描 */
    scan_params->scan_mode = WLAN_SCAN_MODE_BACKGROUND_OBSS;

    /* 准备OBSS扫描的信道 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        /* 从当前信道左右偏移5个信道，计算OBSS扫描信道
           1) 当前信道idx小于等于5，则从0开始，到idx+5,
           2) 大于5小于8，应该从idx-5到idx+5,
           3) 大于8，则是从idx-5到13 */

        osal_u8 low_channel_idx = 0;
        osal_u8 high_channel_idx = 0;
        osal_u8 channel_idx;
        if (hmac_vap->channel.chan_number <= 13) {   /* 2G频段信道1-13 */
            /* 信道数目小于等于5,从0开始扫描信道;信道数目大于5,从curr_channel_num - 5进行扫描 */
            low_channel_idx = (hmac_vap->channel.chan_number <= 5) ? 0 : (hmac_vap->channel.chan_number - 5);
            /* 信道数目小于等于8,信道最大不超过curr_channel_num + 5,   信道数目大于5, 信道最大值不超过13 */
            high_channel_idx = (hmac_vap->channel.chan_number <= 8) ? (hmac_vap->channel.chan_number + 5) : 13;
        }

        oam_warning_log1(0, OAM_SF_SCAN, "{prepare_obss_scan_params,2G channel is %d}", hmac_vap->channel.chan_number);
        /* 准备2.4G下OBSS扫描信道 */
        for (channel_idx = low_channel_idx; channel_idx <= high_channel_idx; channel_idx++) {
            /* 判断信道是不是在管制域内 */
            if (hmac_is_channel_idx_valid_etc(WLAN_BAND_2G, channel_idx) == OAL_SUCC) {
                hmac_get_channel_num_from_idx_etc(WLAN_BAND_2G, channel_idx, &channel_num);

                scan_params->channel_list[chan_num_2g].chan_number = channel_num;
                scan_params->channel_list[chan_num_2g].band = WLAN_BAND_2G;
                scan_params->channel_list[chan_num_2g].chan_idx = channel_idx;
                chan_num_2g++;
            }
        }

        /* 更新本次扫描的信道总数 */
        scan_params->channel_nums = chan_num_2g;
    }

    /* 如果当前扫描信道的总数为0，返回错误，不执行扫描请求 */
    return (scan_params->channel_nums == 0) ? OAL_FAIL : OAL_SUCC;
}

/*****************************************************************************
 功能描述  : obss扫描定时器超时，发起obss扫描
*****************************************************************************/
OAL_STATIC osal_u32 hmac_scan_obss_timeout_fn(osal_void *arg)
{
    hmac_vap_stru *hmac_vap;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_scan_req_stru scan_req_params;
    osal_u32 ret;
    osal_u8 scan_now = OSAL_FALSE;

    hmac_vap = (hmac_vap_stru *)arg;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_obss_timeout_fn::hmac_vap null.}");
        return OAL_FAIL;
    }

    oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_obss_timeout_fn::[%p] [%p].}",
        (uintptr_t)hmac_device->obss_scan_timer.timeout_arg, (uintptr_t)hmac_vap);
    // 残留事件到期
    if (hmac_device->obss_scan_timer.timeout_arg != (osal_void *)hmac_vap) {
        return OAL_FAIL;
    }

    /* 如果在obss scan timer启动期间动态修改了sta能力导致sta不支持obss扫描，
     * 则关闭obss scan timer
     * 此处是唯一中止obss扫描的地方!!!
     */
    if (hmac_mgmt_need_obss_scan(hmac_vap) == OSAL_FALSE) {
        hmac_vap->obss_scan_timer_remain = 0;
        hmac_vap->obss_scan_timer_started = OSAL_FALSE;
        return OAL_SUCC;
    }

    if (hmac_vap->obss_scan_timer_remain == 0) {
        scan_now = OSAL_TRUE;
    }

    hmac_scan_start_obss_timer(hmac_vap);

    /* 进入扫描入口，执行obss扫描 */
    if (scan_now == OSAL_TRUE) {
        if (g_obss_sta_scan_disable == OSAL_TRUE) {
            wifi_printf("OBSS SCAN ABORT\r\n");
            return OAL_SUCC;
        }
        /* 准备OBSS扫描参数，准备发起扫描 */
        (osal_void)memset_s(&scan_req_params, sizeof(mac_scan_req_stru), 0, sizeof(mac_scan_req_stru));
        ret = hmac_scan_prepare_obss_scan_params(&scan_req_params, hmac_vap);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_update_obss_scan_params::scan num is 0, band[%d]!}",
                hmac_vap->channel.band);
            return ret;
        }

        oam_warning_log1(0, OAM_SF_SCAN,
                         "vap_id[%d] {hmac_scan_obss_timeout_fn:: start scan}", hmac_vap->vap_id);
        return hmac_scan_handle_scan_req_entry(hmac_device, hmac_vap, &scan_req_params);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 启动obss扫描定时器
*****************************************************************************/
osal_void hmac_scan_start_obss_timer(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u32 new_timer;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_scan_start_obss_timer::param null.}");
        return;
    }

    /* 重置扫描定时器 */
    if (hmac_vap->obss_scan_timer_remain == 0) {
        /* 1000为时间进制 */
        hmac_vap->obss_scan_timer_remain = (osal_u32)(1000 * mac_mib_get_bss_width_trigger_scan_interval(hmac_vap));
    }

    new_timer = (hmac_vap->obss_scan_timer_remain > HMAC_SCAN_MAX_TIMER) ?
                HMAC_SCAN_MAX_TIMER : hmac_vap->obss_scan_timer_remain;
    hmac_vap->obss_scan_timer_remain -= new_timer;
    oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_start_obss_timer::remain=%d new_timer=%d}",
                  hmac_vap->obss_scan_timer_remain, new_timer);
    frw_create_timer_entry(&(hmac_device->obss_scan_timer), hmac_scan_obss_timeout_fn, new_timer,
        hmac_vap, OSAL_FALSE);
    hmac_vap->obss_scan_timer_started = OSAL_TRUE;

    return;
}
/*****************************************************************************
 功能描述  : 删除obss扫描定时器
*****************************************************************************/
osal_void hmac_scan_destroy_obss_timer(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if ((hmac_device->obss_scan_timer.is_registerd != 0) &&
        hmac_device->obss_scan_timer.timeout_arg == (osal_void *)hmac_vap) { // same as hmac_vap
        frw_destroy_timer_entry(&hmac_device->obss_scan_timer);
        hmac_device->obss_scan_timer.timeout_arg = OSAL_NULL;
        hmac_vap->obss_scan_timer_started = OSAL_FALSE;
    }
}

/*****************************************************************************
 功能描述  : 20M 40M共存检查函数，查找带有40M不允许的BSS以及非HT能力信息的BSS
*****************************************************************************/
osal_void hmac_detect_2040_te_a_b(hmac_vap_stru *hmac_vap, osal_u8 *frame_body, osal_u16 frame_len,
    osal_u16 offset, osal_u8 curr_chan)
{
    osal_u8 chan_index = 0;
    oal_bool_enum_uint8 ht_cap = OSAL_FALSE;
    osal_u8 scan_chan_idx;
    osal_u8 real_chan = curr_chan;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 *ie = OSAL_NULL;
    oal_bool_enum_uint8 te_b = OSAL_FALSE;
    osal_u8 *fra_body = frame_body;
    osal_u16 fra_len = frame_len;

    if (hmac_vap->hal_device == OSAL_NULL) {
        return;
    }

    scan_chan_idx = get_hal_dev_current_scan_idx(hmac_vap->hal_device);

    if (fra_len <= offset) {
        hmac_get_channel_idx_from_num_etc((hmac_device->scan_params.channel_list[scan_chan_idx]).band,
                                          real_chan, &chan_index);
        /* Detect Trigger Event - A */
        hmac_vap->ch_switch_info.chan_report_for_te_a |= (1U << chan_index);

        oam_warning_log1(0, OAM_SF_ANY, "{hmac_detect_2040_te_a_b::framebody_len[%d]}", fra_len);
        return;
    }

    fra_len -= offset;
    fra_body += offset;

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, fra_body, fra_len);
    if (ie != OSAL_NULL && ie[1] >= MAC_HT_CAP_LEN) {
        ht_cap = OSAL_TRUE;

        /* Check for the Forty MHz Intolerant bit in HT-Capabilities */
        if ((ie[3] & BIT6) != 0) { // the bit is in the index of 3
            /* Register Trigger Event - B */
            te_b = OSAL_TRUE;
        }
    }

    ie = mac_find_ie_etc(MAC_EID_2040_COEXT, fra_body, fra_len);
    if (ie != OSAL_NULL && ie[1] >= MAC_2040_COEX_LEN) {
        /* Check for the Forty MHz Intolerant bit in Coex-Mgmt IE */
        if ((ie[2] & BIT1) != 0) { // the bit is in the index of 2
            /* Register Trigger Event - B */
            te_b = OSAL_TRUE;
        }
    }

    /* 存在11bg或者40M不容忍设备，则记录相关信道号 */
    if (ht_cap == OSAL_FALSE || te_b == OSAL_TRUE) {
        ie = mac_find_ie_etc(MAC_EID_DSPARMS, fra_body, fra_len);
        if (ie != OSAL_NULL && ie[1] >= MAC_DSPARMS_LEN) {
            real_chan = ie[2]; // the bit is in the index of 2
        }
        hmac_get_channel_idx_from_num_etc((hmac_device->scan_params.channel_list[scan_chan_idx]).band,
                                          real_chan, &chan_index);
        hmac_vap->ch_switch_info.chan_report_for_te_a |= (1U << chan_index);
    }
    hmac_vap->ch_switch_info.te_b = (hmac_vap->ch_switch_info.te_b == OSAL_TRUE) ? OSAL_TRUE : te_b;

    return;
}

/*****************************************************************************
 功能描述  : OBSS扫描完成后调用的接口,obss扫描不抛事件直接device处理
*****************************************************************************/
osal_void hmac_scan_proc_obss_scan_complete_event(hmac_vap_stru *hmac_vap)
{
    if ((hmac_vap->ch_switch_info.chan_report_for_te_a == OSAL_FALSE) &&
        (hmac_vap->ch_switch_info.te_b == OSAL_FALSE)) {
        return;
    }

    hmac_send_2040_coext_mgmt_frame_sta(hmac_vap);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_update_join_req_params_204080
 功能描述  : 在join之前，根据join request更新相关20/40共存参数(主要是更新带宽模式)
 输入参数  : hmac_vap : MAC VAP结构体(这里是STA模式)
             bss_dscr: 要加入的bss网络信息
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_update_join_req_params_2040_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    /* 如果STA或者AP不支持HT且不支持VHT，则直接返回 */
    if (((mac_mib_get_high_throughput_option_implemented(hmac_vap) == OAL_FALSE) &&
        (mac_mib_get_vht_option_implemented(hmac_vap) == OAL_FALSE)) ||
        ((bss_dscr->ht_capable == OAL_FALSE) && (bss_dscr->vht_capable == OAL_FALSE))) {
        hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
        return;
    }

    /* 使能40MHz */
    /* (1) 用户开启"40MHz运行"特性(即STA侧 dot11FortyMHzOperationImplemented为true) */
    /* (2) AP在40MHz运行 */
    if (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OAL_TRUE) {
        switch (bss_dscr->channel_bandwidth) {
            case WLAN_BAND_WIDTH_40PLUS:
            case WLAN_BAND_WIDTH_80PLUSPLUS:
            case WLAN_BAND_WIDTH_80PLUSMINUS:
                hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;

            case WLAN_BAND_WIDTH_40MINUS:
            case WLAN_BAND_WIDTH_80MINUSPLUS:
            case WLAN_BAND_WIDTH_80MINUSMINUS:
                hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;

            default:
                hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    }

    /* 更新STA侧带宽与AP一致 */
    /* (1) STA AP均支持11AC */
    /* (2) STA支持40M带宽(FortyMHzOperationImplemented为TRUE)，
           定制化禁止2GHT40时，2G下FortyMHzOperationImplemented=FALSE，不更新带宽 */
    /* (3) STA支持80M带宽(即STA侧 dot11VHTChannelWidthOptionImplemented为0) */
    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OAL_TRUE) && (bss_dscr->vht_capable == OAL_TRUE)) {
        if (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OAL_TRUE) {
            /* 不超过mac device最大带宽能力 */
            mac_vap_set_cap_bw(hmac_vap, hmac_vap_get_bandwith(hmac_vap, bss_dscr->channel_bandwidth));
        }
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if ((mac_mib_get_he_option_implemented(hmac_vap) == OAL_TRUE) && (bss_dscr->he_capable == OAL_TRUE)) {
        if (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OAL_TRUE) {
            /* 不超过mac device最大带宽能力 */
            mac_vap_set_cap_bw(hmac_vap, hmac_vap_get_bandwith(hmac_vap, bss_dscr->channel_bandwidth));
        }
    }
#endif

    oam_warning_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_update_join_req_params_2040_etc::channel_bandwidth=%d, mac vap bw[%d].}",
        hmac_vap->vap_id, bss_dscr->channel_bandwidth, mac_vap_get_cap_bw(hmac_vap));

    /* 如果AP和STA同时支持20/40共存管理功能，则使能STA侧频谱管理功能 */
    if ((mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OAL_TRUE) &&
        (bss_dscr->coex_mgmt_supp == 1)) {
        mac_mib_set_spectrum_management_implemented(hmac_vap, OAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmg_need_obss_scan
 功能描述  : 判断是否需要启动obss scan
*****************************************************************************/
osal_u8  hmac_mgmt_need_obss_scan(hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#endif

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_2040, "hmac_mgmt_need_obss_scan::null param");
        return OSAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
        /* 存在P2P设备，就不进行OBSS扫描 */
        if ((hmac_device->st_p2p_info.p2p_mode == WLAN_P2P_GO_MODE) ||
            (hmac_device->st_p2p_info.p2p_mode == WLAN_P2P_CL_MODE)) {
            return OSAL_FALSE;
        }
#endif
    /* 当前vht不启用obss扫描,关联状态下vap有up和pause两种状态,扫描时pause,obss扫描会被停掉 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) &&
        (hmac_vap->channel.band == WLAN_BAND_2G) &&
        (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OSAL_TRUE) &&
        (mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_TRUE) &&
        (hmac_vap_get_peer_obss_scan_etc(hmac_vap) == OSAL_TRUE)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_rx_notify_channel_width
 功能描述  : sta在UP状态下处理"Notify Channel Width"帧上报过滤
*****************************************************************************/
osal_void hmac_rx_notify_channel_width(hmac_vap_stru *hmac_vap, const osal_u8 *data, hmac_user_stru *hmac_user)
{
    wlan_bw_cap_enum_uint8 bwcap_vap;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);

    bwcap_vap = data[MAC_ACTION_OFFSET_ACTION + 1] & BIT0;

    /* 带宽模式未改变or HT40禁止 or需要抑制notify channel width 的action上报 */
    if ((hmac_user->avail_bandwidth == bwcap_vap) ||
        (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OSAL_FALSE)) {
        return;
    }

    /* csa处理过程不跟随带宽变化 */
    if (fhook != OSAL_NULL && ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_TRUE) {
        return;
    }

    if (mac_vap_bw_fsm_beacon_avail(hmac_vap) == OSAL_TRUE) {
        hmac_vap->ch_switch_info.wait_bw_change = OSAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        /* 更新的"STA Channel Width" field */
        hmac_ie_proc_chwidth_field_etc(hmac_vap, hmac_user, bwcap_vap);
#endif
    }
}

/*****************************************************************************
 函 数 名  : hmac_send_2040_coext_mgmt_frame_sta
 功能描述  : STA发送20/40 共存管理帧(根据Trigger Event A & Trigger Event B填写)
*****************************************************************************/
osal_void hmac_send_2040_coext_mgmt_frame_sta(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u16 frame_len;
    osal_u8 coext_info = 0;
    osal_u32 channel_report = hmac_vap->ch_switch_info.chan_report_for_te_a;
    osal_u32 ret;

    /* 根据dot11FortyMHzIntolerant填写此field */
    if ((mac_mib_get_forty_mhz_intolerant(hmac_vap) == OSAL_TRUE) || (hmac_vap->ch_switch_info.te_b == OSAL_TRUE)) {
        coext_info |= BIT1;
    }

    /* 当检测到Trigger Event B时，设置此field为1 */
    if (channel_report != 0) {
        coext_info |= BIT2;
    }

    /* 清除上次扫描结果 */
    hmac_vap->ch_switch_info.chan_report_for_te_a = 0;
    hmac_vap->ch_switch_info.te_b = OSAL_FALSE;

    /* 申请管理帧内存 */
    netbuf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_send_2040_coext_mgmt_frame_sta::netbuf null.}", hmac_vap->vap_id);
        return;
    }

    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);

    /* 封装20/40 共存管理帧 */
    frame_len = hmac_encap_2040_coext_mgmt_etc((osal_void *)hmac_vap, netbuf, coext_info, channel_report);
    if ((frame_len - MAC_80211_FRAME_LEN) > WLAN_SMGMT_NETBUF_SIZE) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oam_error_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_send_2040_coext_mgmt_frame_sta:: probably memory used cross-border.}", hmac_vap->vap_id);
        oal_netbuf_free(netbuf);
        return;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->mpdu_payload_len = frame_len - MAC_80211_FRAME_LEN;
    tx_ctl->tx_user_idx = (osal_u8)hmac_vap->assoc_vap_id; // 发送给关联ap的单播,置合法user idx
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    oam_warning_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_send_2040_coext_mgmt_frame_sta::coext_info=0x%x, channel_report=0x%x}",
        hmac_vap->vap_id, coext_info, channel_report);

    ret = hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_send_2040_coext_mgmt_frame_sta::ret=%d}", hmac_vap->vap_id, ret);
        oal_netbuf_free(netbuf);
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_2040_coext
 功能描述  : STA在UP状态下处理接收到的20/40共存管理帧
*****************************************************************************/
osal_void hmac_sta_up_rx_2040_coext(hmac_vap_stru *hmac_vap, const osal_u8 *frame_body)
{
    /* 如果STA不支持"20/40共存管理"特性，则直接忽略AP发过来的"20/40共存管理帧"  非HT站点，不处理此帧 */
    if (mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_FALSE ||
        (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OSAL_FALSE)) {
        oam_info_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_sta_up_rx_2040_coext::Ignoring the 20/40 Coex Management frame.}", hmac_vap->vap_id);
        return;
    }

    /* "Information Request" field */
    if ((frame_body[MAC_ACTION_OFFSET_ACTION + 1 + 1 + 1] & BIT0) != 0) {
        /* 当STA收到一个Information Request为1的帧后，需要回一个20/40共存管理帧 */
        hmac_send_2040_coext_mgmt_frame_sta(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_get_2040bss_sw
 功能描述  : 获取20/40 bss检测开关状态
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 *pus_len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_get_2040bss_sw(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru       *hmac_device;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_get_2040bss_sw::param null,hmac_vap=%p param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)msg);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_get_2040bss_sw:: hmac_device is null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    msg->rsp_len = (osal_u16)OAL_SIZEOF(osal_u32);
    *((osal_u32 *)msg->rsp) = (osal_u32)mac_get_2040bss_switch(hmac_device);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_obss_scan_notify
 功能描述  : 处理Overlapping BSS Scan Parameters IE，并更新STA相应MIB项
*****************************************************************************/
osal_void hmac_mgmt_obss_scan_notify(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_ie_proc_obss_scan_ie::mac_res_get_hmac_vap fail,vap_id:%u.}", hmac_vap->vap_id);
        return;
    }

    /* 20/40共存逻辑开启时，判断是否启动obss扫描定时器 */
    /* STA模式，才需要obss扫描定时器开启定时器 */
    if ((hmac_mgmt_need_obss_scan(hmac_vap) == OSAL_TRUE) && (hmac_vap->obss_scan_timer_started == OSAL_FALSE)) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_ie_proc_obss_scan_ie:: start obss scan}");
        hmac_scan_start_obss_timer(hmac_vap);
    }
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_ccpriv_obss_scan_disable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u32 ret, value;

    unref_param(hmac_vap);

    ret = (osal_u32)hmac_ccpriv_get_digit_with_check_max(&param, 1, &value);
    if (ret != OAL_SUCC) {
        wifi_printf("Error: obss cfg value %d error!\n", value);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_obss_scan_disable::cfg value invalid.}");
        return ret;
    }

    g_obss_sta_scan_disable = value;

    return OAL_SUCC;
}
#endif

osal_u32 hmac_sta_obss_init(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"sta_obss_scan_disable", hmac_ccpriv_obss_scan_disable);
#endif
    return OAL_SUCC;
}

osal_void hmac_sta_obss_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"sta_obss_scan_disable");
#endif
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
