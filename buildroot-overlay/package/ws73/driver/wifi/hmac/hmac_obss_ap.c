/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac beacon rom
 * Create: 2020-7-8
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
#include "hmac_obss_ap.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "oal_types.h"
#include "oal_netbuf_data.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "hmac_tx_mgmt.h"
#include "hmac_mgmt_sta.h"
#include "hmac_csa_ap.h"
#include "hmac_feature_interface.h"
#include "hmac_feature_dft.h"
#include "hmac_ccpriv.h"
#include "frw_util_notifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HMAC_OBSS_AP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/*****************************************************************************
 函 数 名  : hmac_encap_notify_chan_width
 功能描述  : 组装Notify Channel Width Action帧
*****************************************************************************/
osal_u16 hmac_encap_notify_chan_width(const hmac_vap_stru *hmac_vap, const osal_u8 *data, const osal_u8 *da)
{
    osal_u16 len;
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header((oal_netbuf_stru *)data);
    osal_u8 *payload_addr = oal_netbuf_data_offset((oal_netbuf_stru *)data, MAC_80211_FRAME_LEN);

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
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control((osal_u8 *)mac_header, WLAN_FC0_SUBTYPE_ACTION);
    /*  Set DA  */
    (osal_void)memcpy_s(mac_header->address1, WLAN_MAC_ADDR_LEN, da, WLAN_MAC_ADDR_LEN);
    /*  Set SA  */
    (osal_void)memcpy_s(mac_header->address2, WLAN_MAC_ADDR_LEN, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);
    /*  Set SSID  */
    (osal_void)memcpy_s(mac_header->address3, WLAN_MAC_ADDR_LEN,  hmac_vap->bssid, WLAN_MAC_ADDR_LEN);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number((osal_u8 *)mac_header, 0);
    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*     Notify Channel Width frame Action field format                    */
    /* --------------------------------------------------------------------- */
    /* |   Category   |HT Action       |  Channel Width           | */
    /* --------------------------------------------------------------------- */
    /* |1             |1               | 1Byte                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    payload_addr[0] = MAC_ACTION_CATEGORY_HT; /* 0表示帧格式中第0位Category */
    payload_addr[1] = MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH; /* 1表示帧格式中第1位HT Action的值 */
    payload_addr[2] = (hmac_vap->channel.en_bandwidth > WLAN_BAND_WIDTH_20M) ? 1 : 0; /* 2表示帧中第2位Channel Width值 */

    len = MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN;
    return len;
}

/*****************************************************************************
 函 数 名  : hmac_send_ht_notify_chan_width
 功能描述  : 发送 ht  notify chan width帧
*****************************************************************************/
osal_u32 hmac_send_notify_chan_width(hmac_vap_stru *hmac_vap, const osal_u8 *data)
{
    osal_u16 frame_len;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL || data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_send_notify_chan_width::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 无用户直接返回 */
    if (hmac_vap->user_nums == 0) {
        oam_warning_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_send_notify_chan_width::no user.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    /* 申请notify_chan_width 帧空间 */
    netbuf = (oal_netbuf_stru *)oal_netbuf_alloc_ext(OAL_MGMT_NETBUF,
        MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_send_notify_chan_width::alloc netbuff failed(size %d) in normal_netbuf.}",
            hmac_vap->vap_id, MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 封装Notify Channel Width帧 */
    frame_len = hmac_encap_notify_chan_width(hmac_vap, (osal_u8 *)netbuf, data);
    if (frame_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oam_warning_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_send_notify_chan_width::hmac_encap_notify_chan_width error.}", hmac_vap->vap_id);
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf); /* 获取cb结构体 */
    tx_ctl->mpdu_payload_len = frame_len; /* dmac发送需要的mpdu长度 */
    hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, data);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    ret = hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return ret;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_ap_get_beacon_channel_num
 功能描述  : 处理obss来的beacon帧/probe response帧中的信息元素，得到主次信道号
*****************************************************************************/
static osal_u32 hmac_ap_get_beacon_channel_num(osal_u8 *payload, osal_u32 payload_len, osal_u32 *primary_channel,
    osal_u32 *secondary_channel, osal_u8 curr_chan)
{
    osal_u8 *ie = OSAL_NULL;
    mac_ht_opern_stru *ht_operation = OSAL_NULL;
    mac_sec_ch_off_enum sec_chan_offset;
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    // HT情况下，通过OPERATION字段得到主次信道号
    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, payload + offset, (osal_s32)(payload_len - offset));
    if ((ie != OSAL_NULL) && (ie[1] >= 1)) {
        ht_operation = (mac_ht_opern_stru *)(ie + MAC_IE_HDR_LEN);
        *primary_channel = (osal_u32)ht_operation->primary_channel;
        sec_chan_offset = (mac_sec_ch_off_enum)ht_operation->secondary_chan_offset;
        if (sec_chan_offset == MAC_SCB) {
            if (*primary_channel < 4) { /* 信道编号4 */
                *secondary_channel = *primary_channel - 4; /* 信道编号减4 */
            } else {
                *secondary_channel = 0;
            }
        } else if (sec_chan_offset == MAC_SCA) {
            *secondary_channel = *primary_channel + 4; /* 信道编号加4 */
        } else {
            *secondary_channel = 0;
        }

        return OAL_SUCC;
    }
    // legacy情况下，通过DSSS Param set ie中得到
    *primary_channel = hmac_ie_get_chan_num_etc(payload, (osal_u16)payload_len, offset, curr_chan);
    *secondary_channel = 0;

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_ap_process_obss_bandwidth_depend_freq(hmac_vap_stru *hmac_vap,
    wlan_channel_bandwidth_enum_uint8 bandwidth, osal_u8 *payload, osal_u16 payload_len)
{
    osal_u32 sec_freq_chan;
    osal_u32 pri, sec, sec_chan, pri_chan;

    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        // 获取主信道、次信道中心频点
        osal_u32 pri_freq = (osal_u32)hmac_regdomain_get_freq_map_2g_etc()[hmac_vap->channel.chan_number - 1].freq;
        osal_u32 sec_freq = (bandwidth == WLAN_BAND_WIDTH_40PLUS) ? (pri_freq + 20) : /* 向左扩展20M带宽 */
            (pri_freq - 20); /* 向右扩展20M带宽 */

        // 2.4G共存检测,检测范围是40MHz带宽中心频点为中心,左右各25MHZ
        osal_u32 affected_start = ((pri_freq + sec_freq) >> 1) - 25; /* 检测范围是40MHz带宽中心频点为中心, 其中左边25MHZ */
        osal_u32 affected_end = ((pri_freq + sec_freq) >> 1) + 25; /* 检测范围是40MHz带宽中心频点为中心, 其中右边25MHZ */

        // 获取对方AP主次信道号
        hmac_ap_get_beacon_channel_num(payload, payload_len, &pri_chan, &sec_chan, hmac_vap->channel.chan_number);

        // 计算对方AP频点
        pri = (osal_u32)hmac_regdomain_get_freq_map_2g_etc()[pri_chan - 1].freq;
        sec = pri;

        // 该BSS为40MHz带宽,计算次信道频点
        if (sec_chan != 0) {
            sec = (sec_chan < pri_chan) ? (pri - 20) : (pri + 20); /* 向左扩展20M带宽，或向右扩展20M带宽 */
        }

        // 对方AP不在干扰范围，无需处理
        if ((pri < affected_start || pri > affected_end) && (sec < affected_start || sec > affected_end)) {
            return;
        }

        if (!(pri_freq == pri)) { // 有叠频，且不是同一主信道，则当前AP不能使用40M
            hmac_chan_start_40m_recovery_timer(hmac_vap);
            if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
                (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
                hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
            }
        }
    } else {
        // 5G获取本AP的次信道号
        sec_freq_chan = (bandwidth == WLAN_BAND_WIDTH_40PLUS) ?
            (hmac_vap->channel.chan_number + 4) : /* 向右偏移4个中心频率 */
            (hmac_vap->channel.chan_number - 4); /* 向左偏移4个中心频率 */

        // 获取对方AP主次信道号
        hmac_ap_get_beacon_channel_num(payload, payload_len, &pri_chan, &sec_chan, hmac_vap->channel.chan_number);
        if ((sec_chan == 0) && (sec_freq_chan == pri_chan)) {
            // 5G情况，对方为20M，且在次信道，则当前AP不能使用40M
            hmac_chan_start_40m_recovery_timer(hmac_vap);
            if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
                (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
                hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
            }
        }
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_ap_process_obss_bandwidth
 功能描述  : AP处理obss带宽
*****************************************************************************/
osal_void hmac_ap_process_obss_bandwidth(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 payload_len)
{
    wlan_channel_bandwidth_enum_uint8 bandwidth; // 当前40M带宽或者目标40M带宽

    if (mac_mib_get_2040_switch_prohibited(hmac_vap) == OSAL_FALSE) {
        if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
            (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
            // 当前为40M，需要判断是否切到20M
            bandwidth = hmac_vap->channel.en_bandwidth;
        } else if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_20M) &&
            ((hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_40PLUS) ||
            (hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_40MINUS))) {
            // 当前为20M,判断能否恢复40M
            bandwidth = hmac_vap->bandwidth_40m;
        } else {
            // 其余情况，不用判断该AP信道OBSS
            return;
        }
        /* 依据频率进一步处理 */
        hmac_ap_process_obss_bandwidth_depend_freq(hmac_vap, bandwidth, payload, payload_len);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_ap_is_40mhz_intol_bit_set
 功能描述  : 解析收到帧中的HT Capabilities IE，检测"40MHz不允许"是否被置位
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_ap_is_40mhz_intol_bit_set(osal_u8 *payload_data, osal_u16 len)
{
    osal_u16 ht_cap_info;
    osal_u8 *ht_cap = OSAL_NULL;
    osal_u16 payload_len = len;
    osal_u8 *payload = payload_data;

    if (payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ap_is_40mhz_intol_bit_set::payload_len[%d]}", payload_len);
        return OSAL_FALSE;
    }

    payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    ht_cap = mac_find_ie_etc(MAC_EID_HT_CAP, payload, payload_len);
    if (ht_cap != OSAL_NULL && ht_cap[1] >= MAC_HT_CAP_LEN) {
        ht_cap_info = oal_make_word16(ht_cap[MAC_IE_HDR_LEN], ht_cap[MAC_IE_HDR_LEN + 1]);
        if ((ht_cap_info & BIT14) != 0) { /* Forty MHz Intolerant osal_bit ?1 */
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 从obss Beacon/Probe Response帧中更新信道扫描信息(供后续AP信道选择用)
*****************************************************************************/
OSAL_STATIC osal_void hmac_ap_update_2040_chan_info(const hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 payload_len, osal_u8 pri_chan_idx, mac_sec_ch_off_enum_uint8 sec_ch_offset)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_ap_ch_info_stru *ap_ch_list = hmac_device->ap_channel_list;
    wlan_channel_band_enum_uint8 band = hmac_vap->channel.band;
    osal_u8 sec_ch_idx_offset = mac_get_sec_ch_idx_offset(band);
    osal_u8 sec_chan_idx = 0;

    if (hmac_ap_is_40mhz_intol_bit_set(payload, payload_len) == OSAL_TRUE) {
        hmac_device->intol_bit_recd_40mhz = OSAL_TRUE;
    }

    if (ap_ch_list[pri_chan_idx].ch_type != MAC_CH_TYPE_PRIMARY) {
        ap_ch_list[pri_chan_idx].ch_type = MAC_CH_TYPE_PRIMARY;
    }

    if (sec_ch_offset != MAC_SCN) {
        if (sec_ch_offset == MAC_SCA) {
            sec_chan_idx = pri_chan_idx + sec_ch_idx_offset;
        } else if (sec_ch_offset == MAC_SCB) {
            sec_chan_idx = pri_chan_idx - sec_ch_idx_offset;
        }
        if (hmac_is_channel_idx_valid_etc(band, sec_chan_idx) != OAL_SUCC) {
            return;
        }

        if (sec_chan_idx >= MAC_MAX_SUPP_CHANNEL) {
            oam_warning_log2(0, OAM_SF_2040, "vap_id[%d] {hmac_ap_update_2040_chan_info::invalid sec_chan_idx[%d].}",
                hmac_vap->vap_id, sec_chan_idx);
            return;
        }
        if (ap_ch_list[sec_chan_idx].ch_type != MAC_CH_TYPE_PRIMARY) {
            if (ap_ch_list[sec_chan_idx].ch_type != MAC_CH_TYPE_SECONDARY) {
                ap_ch_list[sec_chan_idx].ch_type = MAC_CH_TYPE_SECONDARY;
            }
        }
    }
}

osal_void hmac_ap_handle_40_intol(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 payload_len)
{
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        if (hmac_ap_is_40mhz_intol_bit_set(payload, payload_len) == OSAL_TRUE) {
            hmac_chan_start_40m_recovery_timer(hmac_vap);
            if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
                (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
                hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
                return;
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_rx_notify_channel_width
 功能描述  : AP在UP状态下处理"Notify Channel Width"帧
*****************************************************************************/
osal_void hmac_ap_rx_notify_channel_width(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *data)
{
    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }

    /* 更新的"STA Channel Width" field */
    hmac_ie_proc_chwidth_field_etc(hmac_vap, hmac_user, (data[MAC_ACTION_OFFSET_ACTION + 1] & BIT0));
}

/*****************************************************************************
 函 数 名  : hmac_chan_reval_status
 功能描述  : 根据OBSS扫描结果，重新选择一条最合适的工作信道
*****************************************************************************/
static osal_void hmac_chan_reval_status(hmac_vap_stru *hmac_vap)
{
    osal_u8 new_channel = 0;
    wlan_channel_bandwidth_enum_uint8 new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_IS_SWITCH);

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_reval_status::param null.}");
        return;
    }

    /* 如果AP已经准备进行信道切换，则直接返回，不做任何处理 */
    if (fhook != OSAL_NULL && ((hmac_csa_ap_is_ch_switch_cb)fhook)(hmac_vap->vap_id) == OSAL_TRUE) {
        return;
    }

    ret = hmac_chan_select_channel_for_operation(hmac_vap, &new_channel, &new_bandwidth);
    if (ret != OAL_SUCC) {
        return;
    }

    ret = hmac_is_channel_num_valid_etc(hmac_vap->channel.band, new_channel);
    if ((ret != OAL_SUCC) || (new_bandwidth >= WLAN_BAND_WIDTH_BUTT)) {
        oam_warning_log3(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_chan_reval_status::Could not start network using the selected ch[%d] or bw[%d].}",
            hmac_vap->vap_id, new_channel, new_bandwidth);
        return;
    }

    if (new_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_chan_multi_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number, new_bandwidth);
        ret = hmac_send_notify_chan_width(hmac_vap, BROADCAST_MACADDR);
        if (ret != OAL_SUCC) {
            /* 带宽切换帧发送失败对业务无影响，beacon帧中也有带宽信息 */
            oam_warning_log2(0, OAM_SF_SCAN,
                "vap_id[%d] {hmac_chan_reval_status::hmac_send_notify_chan_width return %d.}", hmac_vap->vap_id, ret);
        }
    } else {
        oam_info_log1(0, 0, "vap_id[%d] {hmac_chan_reval_status::no ch change after re-evaluation.}", hmac_vap->vap_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_get_40mhz_possibility
 功能描述  : 判断在当前环境(扫描结果)下，能否建立40MHz BSS
*****************************************************************************/
static oal_bool_enum_uint8 hmac_chan_get_40mhz_possibility(const hmac_vap_stru *hmac_vap,
    hmac_eval_scan_report_stru *chan_scan_report)
{
    oal_bool_enum_uint8 forty_m_hz_poss = OSAL_FALSE;

    if (osal_unlikely(hmac_vap == OSAL_NULL || chan_scan_report == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_get_40mhz_possibility::param null.}");
        return OSAL_FALSE;
    }

    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_get_40mhz_possibility}", hmac_vap->vap_id);

    if (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OSAL_TRUE) {
        forty_m_hz_poss = hmac_chan_get_2040_op_chan_list(hmac_vap, chan_scan_report);
    }

    return forty_m_hz_poss;
}
static osal_void hmac_chan_initiate_switch_to_20mhz_ap(hmac_vap_stru *hmac_vap)
{
    /* 设置VAP带宽模式为20MHz */
    hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    hmac_vap->ch_switch_info.bw_switch_status = WLAN_BW_SWITCH_40_TO_20;
}

/*****************************************************************************
 函 数 名  : hmac_chan_multi_switch_to_20MHz_ap
 功能描述  : 设置device下所有VAP信道参数，准备切换至20MHz运行
*****************************************************************************/
osal_void hmac_chan_multi_switch_to_20mhz_ap(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_multi_switch_to_20mhz_ap::param null.}");
        return;
    }

    oam_info_log2(0, OAM_SF_2040,
        "vap_id[%d] {hmac_chan_multi_switch_to_20mhz_ap::bit_2040_channel_switch_prohibited=%d}",
        hmac_vap->vap_id, mac_mib_get_2040_switch_prohibited(hmac_vap));

    /* dmac vap保存切换之前的带宽能力 */
    hmac_vap->bandwidth_40m = hmac_vap->channel.en_bandwidth;

    /* 如果不允许切换带宽，则直接返回 */
    if (mac_mib_get_2040_switch_prohibited(hmac_vap) == OSAL_TRUE) {
        return;
    }

    hmac_chan_initiate_switch_to_20mhz_ap(hmac_vap);

    hmac_chan_multi_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number, WLAN_BAND_WIDTH_20M);
    ret = hmac_send_notify_chan_width(hmac_vap, BROADCAST_MACADDR);
    if (ret != OAL_SUCC) {
        /* 带宽切换帧发送失败对业务无影响，beacon帧中也有带宽信息 */
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_multi_switch_to_20mhz_ap::notify_chan_width return %d.}", hmac_vap->vap_id, ret);
    }
}

static osal_void hmac_chan_select_channel_proc(const hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device,
    hmac_eval_scan_report_stru *chan_scan_report, hmac_chan_select_stru *chan_param)
{
    oal_bool_enum_uint8 rslt = OSAL_FALSE;
    osal_u16 least_networks = 0xFFFF;
    osal_u8 chan_idx;

    /* 判断在当前环境下是否能够建立40MHz BSS */
    osal_u8 forty_m_hz_poss = hmac_chan_get_40mhz_possibility(hmac_vap, chan_scan_report);

    /* 用户选择了主信道 */
    if (chan_param->user_chan_idx != 0xFF) {
        /* 如果能够建立40MHz BSS，并且用户也希望建立40MHz */
        if ((forty_m_hz_poss == OSAL_TRUE) && (chan_param->user_chan_offset != MAC_SCN)) {
            if (chan_param->user_chan_offset == MAC_SCA) {
                rslt = hmac_chan_is_40mhz_sca_allowed(hmac_vap, chan_scan_report, chan_param->user_chan_idx,
                    chan_param->user_chan_offset);
            } else if (chan_param->user_chan_offset == MAC_SCB) {
                rslt = hmac_chan_is_40mhz_scb_allowed(hmac_vap, chan_scan_report, chan_param->user_chan_idx,
                    chan_param->user_chan_offset);
            }
        }

        chan_param->chan_offset = (rslt == OSAL_TRUE) ? chan_param->user_chan_offset : chan_param->chan_offset;

        chan_param->least_busy_chan_idx = chan_param->user_chan_idx;
        return;
    }
    /* 用户没有选择信道，自动选择一条最不繁忙的信道(对) */
    for (chan_idx = 0; chan_idx < chan_param->num_supp_chan; chan_idx++) {
        osal_u32 ret;
        if ((chan_scan_report[chan_idx].chan_op & HMAC_OP_ALLOWED) == 0) {
            continue;
        }

        /* 判断主信道索引号是否有效 */
        ret = hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, chan_idx);
        if (ret != OAL_SUCC) {
            continue;
        }

        /* 能够建立40MHz BSS */
        if ((forty_m_hz_poss != OSAL_TRUE)) {
            /* 获取当前信道邻近的BSS个数 */
            osal_u16 cumulative_bss = hmac_chan_get_cumulative_networks(hmac_device, hmac_vap->channel.band, chan_idx);
            /* 选择邻近BSS最少的一条信道作为"当前最不繁忙信道" */
            if (cumulative_bss < least_networks) {
                least_networks = cumulative_bss;
                chan_param->least_busy_chan_idx = chan_idx;
            }
            continue;
        }
        /* 判断主信道上(右)面的信道是否可作为次信道 */
        rslt = hmac_chan_is_40mhz_sca_allowed(hmac_vap, chan_scan_report, chan_idx, chan_param->user_chan_offset);
        if ((rslt == OSAL_TRUE) && (chan_scan_report[chan_idx].aus_num_networks[HMAC_NETWORK_SCA] < least_networks)) {
            /* 如果这条信道最不繁忙，则选择这条信道作为"当前最不繁忙信道" */
            least_networks = chan_scan_report[chan_idx].aus_num_networks[HMAC_NETWORK_SCA];
            chan_param->least_busy_chan_idx = chan_idx;
            chan_param->chan_offset = MAC_SCA;
        }

        /* 判断主信道下(左)面的信道是否可作为次信道 */
        rslt = hmac_chan_is_40mhz_scb_allowed(hmac_vap, chan_scan_report, chan_idx, chan_param->user_chan_offset);
        if ((rslt == OSAL_TRUE) && (chan_scan_report[chan_idx].aus_num_networks[HMAC_NETWORK_SCB] < least_networks)) {
            /* 如果这条信道最不繁忙，则选择这条信道作为"当前最不繁忙信道" */
            least_networks = chan_scan_report[chan_idx].aus_num_networks[HMAC_NETWORK_SCB];
            chan_param->least_busy_chan_idx = chan_idx;
            chan_param->chan_offset = MAC_SCB;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_select_channel_for_operation
 功能描述  : 挑选一条信道(对)
*****************************************************************************/
osal_u32 hmac_chan_select_channel_for_operation(const hmac_vap_stru *hmac_vap, osal_u8 *new_channel,
    wlan_channel_bandwidth_enum_uint8 *new_bandwidth)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 max_supp_channel = MAC_MAX_SUPP_CHANNEL;
    osal_u32 ret;
    hmac_chan_select_stru chan_param;
    hmac_eval_scan_report_stru *chan_scan_report = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || new_channel == OSAL_NULL || new_bandwidth == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_select_channel_for_operation::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&chan_param, sizeof(hmac_chan_select_stru), 0, sizeof(hmac_chan_select_stru));
    chan_param.chan_offset = MAC_SCN;
    chan_param.least_busy_chan_idx = 0xFF;
    chan_param.num_supp_chan = mac_get_num_supp_channel(hmac_vap->channel.band);

    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_select_channel_for_operation}", hmac_vap->vap_id);

    chan_scan_report = (hmac_eval_scan_report_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        max_supp_channel * sizeof(hmac_eval_scan_report_stru), OAL_TRUE);
    if (osal_unlikely(chan_scan_report == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_chan_select_channel_for_operation::chan_scan_report memory alloc failed, size[%d].}",
            hmac_vap->vap_id, (max_supp_channel * sizeof(*chan_scan_report)));
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_chan_init_chan_scan_report(hmac_vap, chan_scan_report, chan_param.num_supp_chan);

    /* 如自动信道选择没有开启，则获取用户选择的主信道号，以及带宽模式 */
    if (mac_device_is_auto_chan_sel_enabled(hmac_device) == OSAL_FALSE) {
        if (mac_is_dbac_enabled(hmac_device)) {
            ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, hmac_vap->channel.chan_number,
                &chan_param.user_chan_idx);
        } else {
            ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, hmac_device->max_channel,
                &chan_param.user_chan_idx);
        }
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, 0, "vap[%d]hmac_chan_select_channel_for_operation:get fail[%d]", hmac_vap->vap_id, ret);
            oal_mem_free((osal_void *)chan_scan_report, OAL_TRUE);

            return ret;
        }

        chan_param.user_chan_offset = mac_get_sco_from_bandwidth(hmac_vap->ch_switch_info.user_pref_bandwidth);

        oam_warning_log4(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_chan_select_channel_for_operation::User Preferred Channel id=%d sco=%d. dbac_enable[%u]}",
            hmac_vap->vap_id, chan_param.user_chan_idx, chan_param.user_chan_offset, mac_is_dbac_enabled(hmac_device));
    }

    hmac_chan_select_channel_proc(hmac_vap, hmac_device, chan_scan_report, &chan_param);

    hmac_get_channel_num_from_idx_etc(hmac_vap->channel.band, chan_param.least_busy_chan_idx, new_channel);

    *new_bandwidth = mac_get_bandwidth_from_sco(chan_param.chan_offset);

    oam_info_log3(0, OAM_SF_SCAN, "vap[%d]{hmac_chan_select_channel_for_operation:Selected Channel=%d, Bandwidth=%d.}",
        hmac_vap->vap_id, (*new_channel), (*new_bandwidth));

    oal_mem_free((osal_void *)chan_scan_report, OAL_TRUE);

    return OAL_SUCC;
}

static osal_void hmac_ap_up_rx_2040_into(hmac_vap_stru *hmac_vap, const osal_u8 *data, osal_u32 index,
    hmac_device_stru *hmac_device)
{
    osal_u8 len;
    osal_u8 chan_idx = 0;
    osal_u8 loop;
    oal_bool_enum_uint8 reval_chan = OSAL_FALSE;
    osal_u32 index_tmp = index;

    if (data[index_tmp] != MAC_EID_2040_INTOLCHREPORT) {
        return;
    }
    /* 20/40 BSS Intolerant Channel Report IE */
    len = data[index_tmp + 1];
    if (len == 0) {
        return;
    }

    index_tmp += (MAC_IE_HDR_LEN + 1); /* skip Element ID、Length、Operating Class */
    oam_info_log2(0, 0, "vap_id[%d] {hmac_ap_up_rx_2040_into::Chan Report with len=%d.}", hmac_vap->vap_id, len);

    for (loop = 0; loop < len - 1; loop++, index_tmp++) {
        osal_u32 ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, data[index_tmp], &chan_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_2040,
                "vap_id[%d] {hmac_ap_up_rx_2040_into::get_ch_idx failed[%d].}", hmac_vap->vap_id, ret);
            continue;
        }

        ret = hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, chan_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log3(0, 0, "vap_id[%d] {hmac_ap_up_rx_2040_into::channel(idx=%d) not valid, return[%d].}",
                hmac_vap->vap_id, chan_idx, ret);
            continue;
        }

        if (hmac_device->ap_channel_list[chan_idx].ch_type != MAC_CH_TYPE_PRIMARY) {
            hmac_device->ap_channel_list[chan_idx].ch_type = MAC_CH_TYPE_PRIMARY;
            reval_chan = OSAL_TRUE;
        }
    }

    if (reval_chan != OSAL_TRUE) {
        return;
    }
    oam_warning_log1(0, 0, "vap_id[%d] {hmac_ap_up_rx_2040_into::Re-evaluation needed because ch status changed.}",
        hmac_vap->vap_id);

    /* 如果允许信道或者带宽切换，才进行切换 */
    if (mac_mib_get_2040_switch_prohibited(hmac_vap) == 0) {
        hmac_chan_start_40m_recovery_timer(hmac_vap);
        /* 重新评估是否需要进行信道切换 */
        hmac_chan_reval_status(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_2040_coext
 功能描述  : AP在UP状态下的接收"20/40共存管理帧"的处理
*****************************************************************************/
osal_void hmac_ap_up_rx_2040_coext(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    osal_u8 *data = OSAL_NULL;
    osal_u32 index = MAC_ACTION_OFFSET_ACTION + 1;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hmac_vap == OSAL_NULL || netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_ap_up_rx_2040_coext::param null.}");
        return;
    }

    oam_info_log0(0, 0, "{hmac_ap_up_rx_2040_coext::handle 2040 coext.}");

    /* 5GHz频段忽略 20/40 BSS共存管理帧 */
    if ((hmac_vap->channel.band == WLAN_BAND_5G) ||
        (mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_FALSE)) {
        oam_info_log1(0, 0, "vap_id[%d] {hmac_ap_up_rx_2040_coext::Now in 5GHz.}", hmac_vap->vap_id);
        return;
    }

    /* 获取帧体指针 */
    data = oal_netbuf_rx_data(netbuf);
    /* 20/40 BSS Coexistence IE */
    if (data[index] == MAC_EID_2040_COEXT) {
        osal_u8 coext_info = data[index + MAC_IE_HDR_LEN];

        oam_info_log2(0, 0, "vap_id[%d] {hmac_ap_up_rx_2040_coext::20/40 Coex mgmt frame received, Coext Info=0x%x.}",
            hmac_vap->vap_id, coext_info);
        /* BIT1 - Forty MHz Intolerant */
        /* BIT2 - 20 MHz BSS Width Request */
        if ((coext_info & (BIT1 | BIT2)) != 0) {
            hmac_chan_start_40m_recovery_timer(hmac_vap);
            /* 如果BIT1和BIT2任意为1，且当前运行信道宽度大于20MHz，则AP需要切换到20MHz运行 */
            if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
                (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
                hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
                return;
            } else {
                hmac_device->intol_bit_recd_40mhz = OSAL_TRUE;
            }
        }

        index += (MAC_IE_HDR_LEN + data[index + 1]);
    }

    hmac_ap_up_rx_2040_into(hmac_vap, data, index, hmac_device);
}

/*****************************************************************************
 函 数 名  : hmac_chan_is_40MHz_scb_allowed
 功能描述  : 对于给定的主信道，判断能否建立SCB类型的40MHz BSS
*****************************************************************************/
oal_bool_enum_uint8 hmac_chan_is_40mhz_scb_allowed(const hmac_vap_stru *hmac_vap,
    const hmac_eval_scan_report_stru *chan_scan_report, osal_u8 pri_chan_idx, mac_sec_ch_off_enum_uint8 chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8 band;
    osal_u8 sec_ch_idx_offset;
    osal_u8 sec_ch_idx = 0;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL || chan_scan_report == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_is_40mhz_scb_allowed::param null.}");
        return OSAL_FALSE;
    }

    band = hmac_vap->channel.band;
    sec_ch_idx_offset = mac_get_sec_ch_idx_offset(band);

    oam_info_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_chan_is_40mhz_scb_allowed::pri_chan_idx=%d, user_chan_offset=%d}", hmac_vap->vap_id,
        pri_chan_idx, chan_offset);

    if (chan_offset == MAC_SCA) {
        return OSAL_FALSE;
    }

    if ((mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_TRUE) &&
        (band == WLAN_BAND_2G)) {
        if ((chan_scan_report[pri_chan_idx].chan_op & HMAC_SCB_ALLOWED) == 0) {
            return OSAL_FALSE;
        }
    }

    oam_info_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_chan_is_40mhz_scb_allowed::sec_ch_idx_offset=%u, band=%u}", hmac_vap->vap_id,
        sec_ch_idx_offset, band);

    if (pri_chan_idx >= sec_ch_idx_offset) {
        sec_ch_idx = pri_chan_idx - sec_ch_idx_offset;
    } else {
        return OSAL_FALSE;
    }

    ret = hmac_is_channel_idx_valid_etc(band, sec_ch_idx);
    if (ret != OAL_SUCC) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
#else
    unref_param(hmac_vap);
    unref_param(chan_scan_report);
    unref_param(pri_chan_idx);
    unref_param(chan_offset);

    return OSAL_FALSE;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_chan_init_chan_scan_report
 功能描述  : 这函数看看到底要不要
*****************************************************************************/
osal_void hmac_chan_init_chan_scan_report(const hmac_vap_stru *hmac_vap,
    hmac_eval_scan_report_stru *chan_scan_report, osal_u8 num_supp_chan)
{
    osal_u8 idx;
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#endif
    if (osal_unlikely(hmac_vap == OSAL_NULL || chan_scan_report == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_init_chan_scan_report::param null.}");
        return;
    }

    (osal_void)memset_s(chan_scan_report, num_supp_chan * sizeof(*chan_scan_report),
        0, num_supp_chan * sizeof(*chan_scan_report));

    for (idx = 0; idx < num_supp_chan; idx++) {
#ifdef _PRE_WLAN_FEATURE_DFS
        if (mac_vap_get_dfs_enable(hmac_vap) == OSAL_TRUE) {
            if ((hmac_device->ap_channel_list[idx].ch_status != MAC_CHAN_NOT_SUPPORT) &&
                (hmac_device->ap_channel_list[idx].ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
                chan_scan_report[idx].chan_op |= HMAC_OP_ALLOWED;
            }
        } else {
            chan_scan_report[idx].chan_op |= HMAC_OP_ALLOWED;
        }
#else
        chan_scan_report[idx].chan_op |= HMAC_OP_ALLOWED;
#endif
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_is_40MHz_sca_allowed
 功能描述  : 对于给定的主信道，判断能否建立SCA类型的40MHz BSS
*****************************************************************************/
oal_bool_enum_uint8  hmac_chan_is_40mhz_sca_allowed(
    const hmac_vap_stru                 *hmac_vap,
    const hmac_eval_scan_report_stru   *chan_scan_report,
    osal_u8                     pri_chan_idx,
    mac_sec_ch_off_enum_uint8     user_chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8 band;
    osal_u8 num_supp_chan;
    osal_u8 sec_chan_idx;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL || chan_scan_report == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_is_40mhz_sca_allowed::param null.}");
        return OSAL_FALSE;
    }

    band = hmac_vap->channel.band;
    num_supp_chan = mac_get_num_supp_channel(band);

    oam_info_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_chan_is_40mhz_sca_allowed::pri_chan_idx=%d, user_chan_offset=%d}", hmac_vap->vap_id,
        pri_chan_idx, user_chan_offset);

    if (user_chan_offset == MAC_SCB) {
        return OSAL_FALSE;
    }

    if ((mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_TRUE) &&
        (band == WLAN_BAND_2G)) {
        if ((chan_scan_report[pri_chan_idx].chan_op & HMAC_SCA_ALLOWED) == 0) {
            return OSAL_FALSE;
        }
    }

    sec_chan_idx = pri_chan_idx + mac_get_sec_ch_idx_offset(band);
    if (sec_chan_idx >= num_supp_chan) {
        return OSAL_FALSE;
    }

    ret = hmac_is_channel_idx_valid_etc(band, sec_chan_idx);
    if (ret != OAL_SUCC) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
#else
    unref_param(hmac_vap);
    unref_param(chan_scan_report);
    unref_param(pri_chan_idx);
    unref_param(user_chan_offset);

    return OSAL_FALSE;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_chan_get_cumulative_networks
 功能描述  : 获取当前信道邻近(+/- 3个信道)的BSS个数
*****************************************************************************/
osal_u16  hmac_chan_get_cumulative_networks(
    const hmac_device_stru                     *hmac_device,
    wlan_channel_bandwidth_enum_uint8    band,
    osal_u8                            pri_chan_idx)
{
    osal_u16   cumulative_networks = 0;
    osal_u8    num_supp_chan = mac_get_num_supp_channel(band);
    osal_u8    affected_ch_idx_off = mac_get_affected_ch_idx_offset(band);
    osal_u8    affected_chan_lo, affected_chan_hi;
    osal_u8    chan_idx;
    osal_u32   ret;

    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_chan_get_cumulative_networks::param null.}");
        return 0;
    }

    affected_chan_lo = (pri_chan_idx >= affected_ch_idx_off) ? (pri_chan_idx - affected_ch_idx_off) : 0;

    affected_chan_hi = (num_supp_chan > pri_chan_idx + affected_ch_idx_off) ?
                       (pri_chan_idx + affected_ch_idx_off) : (num_supp_chan - 1);

    for (chan_idx = affected_chan_lo; chan_idx <= affected_chan_hi; chan_idx++) {
        ret = hmac_is_channel_idx_valid_etc(band, chan_idx);
        if (ret == OAL_SUCC) {
            cumulative_networks += hmac_device->ap_channel_list[pri_chan_idx].num_networks;
        }
    }

    oam_info_log3(0, OAM_SF_M2S,
        "{hmac_chan_get_cumulative_networks::chan_lo is %u chan_hi is %u cumulative_networks is %u.}",
        affected_chan_lo, affected_chan_hi, cumulative_networks);
    return cumulative_networks;
}

osal_s32 hmac_40m_intol_update_ap(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_bandwidth_stru *band_prot = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    band_prot = (mac_bandwidth_stru *)msg->data;
    oam_warning_log4(0, OAM_SF_2040,
        "{hmac_40m_intol_update_ap::vap mode=%d,current bw=%d,updated bw=%d,en_40m_intol_user=%d}", hmac_vap->vap_mode,
        hmac_vap->channel.en_bandwidth, band_prot->bandwidth_40m, band_prot->intol_user_40m);

    /* 只有AP需要处理 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OAL_SUCC;
    }

    if (band_prot->bandwidth_40m != WLAN_BAND_WIDTH_BUTT) {
        hmac_vap->bandwidth_40m = band_prot->bandwidth_40m;

        /* hostapd配置的40m切换成20m需要开启定时器 */
        if (((hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_40PLUS) ||
            (hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_40MINUS)) &&
            (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_20M)) {
            hmac_chan_start_40m_recovery_timer(hmac_vap);
        }
        /* hostapd配置20m 关闭定时器 */
        if (hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_20M) {
            hmac_chan_stop_40m_recovery_timer(hmac_vap);
        }
    }

    if (band_prot->intol_user_40m == OSAL_TRUE) {
        mac_mib_set_forty_mhz_intolerant(hmac_vap, OSAL_TRUE);
        hmac_chan_start_40m_recovery_timer(hmac_vap);
        if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
            (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
            /* AP准备切换置20MHz运行 */
            hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
        } else {
            hmac_device->intol_bit_recd_40mhz = OSAL_TRUE;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_40m_intol_sync_data
 功能描述  : 初始化带宽或者用户增加导致带宽发生变化
             需要通知dmac
 输入参数  : hmac_vap : hmac vap结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
***************************************************************************/
osal_void hmac_40m_intol_sync_data(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth_40m,
    oal_bool_enum_uint8 intol_user_40m)
{
    mac_bandwidth_stru          band_prot;
    frw_msg                     msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    memset_s(&band_prot, OAL_SIZEOF(mac_bandwidth_stru), 0, OAL_SIZEOF(mac_bandwidth_stru));

    band_prot.bandwidth_40m = bandwidth_40m;
    band_prot.intol_user_40m = intol_user_40m;
    msg_info.data = (osal_u8 *)&band_prot;
    msg_info.data_len = OAL_SIZEOF(band_prot);
    hmac_40m_intol_update_ap(hmac_vap, &msg_info);
}

/*****************************************************************************
 函 数 名  : hmac_chan_update_40m_intol_user_etc
 功能描述  : 更新en_40M_intol_user
 输出参数  : 无
*****************************************************************************/
osal_void hmac_chan_update_40m_intol_user_etc(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head     *entry = OSAL_NULL;
    hmac_user_stru           *hmac_user = OSAL_NULL;

    osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_update_40m_intol_user_etc::user null pointer.}",
                hmac_vap->vap_id);
            continue;
        } else {
            if (hmac_user->ht_hdl.forty_mhz_intolerant != 0) {
                hmac_vap->en_40m_intol_user = OAL_TRUE;
                return;
            }
        }
    }

    hmac_vap->en_40m_intol_user = OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_chan_stop_40M_recovery_timer
 功能描述  : 启动obss扫描定时器
*****************************************************************************/
osal_void hmac_chan_stop_40m_recovery_timer(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->recovery_timer_40m.is_registerd == OSAL_FALSE) {
        return;
    }
    frw_destroy_timer_entry(&(hmac_vap->recovery_timer_40m));
    oam_warning_log0(0, OAM_SF_2040, "{hmac_chan_stop_40m_recovery_timer}");
}

OAL_STATIC osal_u8 hmac_chan_40mhz_cal(const hmac_vap_stru *hmac_vap, hmac_chan_mgmt_40mhz_cal *chan_cal)
{
    osal_u8 num_supp_chan = mac_get_num_supp_channel(hmac_vap->channel.band);
    osal_u8 affected_ch_offset = mac_get_affected_ch_idx_offset(hmac_vap->channel.band);
    osal_u8 sec_ch_idx_offset = mac_get_sec_ch_idx_offset(hmac_vap->channel.band);
    osal_u32 ret;

    ret = hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, chan_cal->pri_chan_idx);
    if (ret != OAL_SUCC) {
        return OSAL_FALSE;
    }

    if (chan_cal->sec_chan_offset == MAC_SCA) {
        /* 计算次信道索引 */
        if (num_supp_chan > chan_cal->pri_chan_idx + sec_ch_idx_offset) {
            chan_cal->sec_chan_idx = chan_cal->pri_chan_idx + sec_ch_idx_offset;
        } else {
            return OSAL_FALSE;
        }

        chan_cal->network_type = HMAC_NETWORK_SCA;
        chan_cal->allowed_bit = HMAC_SCA_ALLOWED;

        /* 计算受影响的信道下限索引 */
        chan_cal->affected_chan_lo = (chan_cal->pri_chan_idx >= affected_ch_offset) ?
            (chan_cal->pri_chan_idx - affected_ch_offset) : 0;

        /* 计算受影响的信道上限索引 */
        chan_cal->affected_chan_hi = (num_supp_chan > chan_cal->sec_chan_idx + affected_ch_offset) ?
            (chan_cal->sec_chan_idx + affected_ch_offset) : (num_supp_chan - 1);
    } else if (chan_cal->sec_chan_offset == MAC_SCB) {
        /* 计算次信道索引 */
        if (chan_cal->pri_chan_idx >= sec_ch_idx_offset) {
            chan_cal->sec_chan_idx = chan_cal->pri_chan_idx - sec_ch_idx_offset;
        } else {
            return OSAL_FALSE;
        }

        chan_cal->network_type = HMAC_NETWORK_SCB;
        chan_cal->allowed_bit = HMAC_SCB_ALLOWED;

        /* 计算受影响的信道下限索引 */
        chan_cal->affected_chan_lo = (chan_cal->sec_chan_idx >= affected_ch_offset) ?
            (chan_cal->sec_chan_idx - affected_ch_offset) : 0;

        /* 计算受影响的信道上限索引 */
        chan_cal->affected_chan_hi = (num_supp_chan > chan_cal->pri_chan_idx + affected_ch_offset) ?
            (chan_cal->pri_chan_idx + affected_ch_offset) : (num_supp_chan - 1);
    } else {
        return OSAL_FALSE;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_chan_40MHz_possibly
 功能描述  : 判断能否在给定的"主信道 + 次信道偏移量"上，建立起40MHz BSS
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_chan_40mhz_possibly(const hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap, osal_u8 pri_chan_idx, mac_sec_ch_off_enum_uint8 sec_chan_offset,
    hmac_eval_scan_report_stru *chan_scan_report)
{
    hmac_chan_mgmt_40mhz_cal chan_cal;
    oal_bool_enum_uint8 flag_2040_op = OSAL_TRUE;
    osal_u32 ret;
    osal_u8 ch_idx;
    const mac_ap_ch_info_stru *ap_channel_list = OSAL_NULL;

    memset_s(&chan_cal, sizeof(chan_cal), 0, sizeof(chan_cal));
    chan_cal.pri_chan_idx = pri_chan_idx;
    chan_cal.sec_chan_offset = sec_chan_offset;
    oam_info_log3(0, 0, "vap_id[%d] {hmac_chan_40mhz_possibly::pri_chan_idx=%d,sec_chan_offset=%d}", hmac_vap->vap_id,
        pri_chan_idx, sec_chan_offset);

    ret = hmac_chan_40mhz_cal(hmac_vap, &chan_cal);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
    ret = hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, chan_cal.sec_chan_idx);
    if (ret != OAL_SUCC) {
        return OSAL_FALSE;
    }

    if (chan_cal.affected_chan_hi >= MAC_MAX_SUPP_CHANNEL) {
        return OSAL_FALSE;
    }

    /* 对于给定的"主信道 + 次信道偏移量"所波及的范围内(中心频点 +/- 5个信道)，判断能否建立40MHz BSS */
    for (ch_idx = chan_cal.affected_chan_lo; ch_idx <= chan_cal.affected_chan_hi; ch_idx++) {
        ret = hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, ch_idx);
        if (ret != OAL_SUCC) {
            continue;
        }

        ap_channel_list = &(hmac_device->ap_channel_list[ch_idx]);

        /* 如果这条信道上存在BSS */
        if (ap_channel_list->ch_type != MAC_CH_TYPE_NONE) {
            /* 累加这条信道上扫描到的BSS个数 */
            chan_scan_report[pri_chan_idx].aus_num_networks[chan_cal.network_type] += ap_channel_list->num_networks;

            /* 新BSS的主信道可以与已有的20/40MHz BSS的主信道重合 */
            /* 新BSS的次信道可以与已有的20/40MHz BSS的次信道重合 */
            if ((ch_idx == pri_chan_idx && ap_channel_list->ch_type == MAC_CH_TYPE_PRIMARY) ||
                (ch_idx == chan_cal.sec_chan_idx && ap_channel_list->ch_type == MAC_CH_TYPE_SECONDARY)) {
                    continue;
            }
            flag_2040_op = OSAL_FALSE;
        }
    }

    /* 如果20/40MHz共存没有使能，则认为可以建立40MHz BSS，除非用户设置了"40MHz不允许"位 */
    /* 5GHz情况下不关心20/40M共存配置和40M不允许设置，一定建立40MHz BSS */
    if (((mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_FALSE) &&
        (mac_mib_get_forty_mhz_intolerant(hmac_vap) == OSAL_FALSE)) || (hmac_vap->channel.band == WLAN_BAND_5G)) {
        flag_2040_op = OSAL_TRUE;
    }
    if (flag_2040_op == OSAL_TRUE) {
        chan_scan_report[pri_chan_idx].chan_op |= chan_cal.allowed_bit;
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_chan_get_2040_op_chan_list
 功能描述  : 解析信道扫描结果，评估每条信道是否能够建立BSS
*****************************************************************************/
oal_bool_enum_uint8 hmac_chan_get_2040_op_chan_list(const hmac_vap_stru *hmac_vap,
    hmac_eval_scan_report_stru *chan_scan_report)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 chan_idx;
    osal_u8 num_supp_chan = mac_get_num_supp_channel(hmac_vap->channel.band);
    oal_bool_enum_uint8 forty_m_hz_poss = OSAL_FALSE;
    oal_bool_enum_uint8 flag;

    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_get_2040_op_chan_list}", hmac_vap->vap_id);

    for (chan_idx = 0; chan_idx < num_supp_chan; chan_idx++) {
        /* 判断能否建立SCA类型的40MHz BSS */
        flag = hmac_chan_40mhz_possibly(hmac_device, hmac_vap, chan_idx, MAC_SCA, chan_scan_report);
        if (flag == OSAL_TRUE) {
            forty_m_hz_poss = OSAL_TRUE;
        }

        /* 判断能否建立SCB类型的40MHz BSS */
        flag = hmac_chan_40mhz_possibly(hmac_device, hmac_vap, chan_idx, MAC_SCB, chan_scan_report);
        if (flag == OSAL_TRUE) {
            forty_m_hz_poss = OSAL_TRUE;
        }
    }

    /* 2.4GHz下，如果"40MHz不允许"位是否被设置，则不允许建立40MHz BSS */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        if ((hmac_device->intol_bit_recd_40mhz == OSAL_TRUE) ||
            (mac_mib_get_forty_mhz_intolerant(hmac_vap) == OSAL_TRUE)) {
            forty_m_hz_poss = OSAL_FALSE;
        }
    }

    return forty_m_hz_poss;
}

/*****************************************************************************
 函 数 名  : hmac_chan_prepare_for_40M_recovery
 功能描述  : 在恢复40M之前，要清除obss扫描结果，40M禁止位，20M request位等信息
*****************************************************************************/
osal_u32 hmac_chan_prepare_for_40m_recovery(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    osal_u8 old_bandwidth = hmac_vap->channel.en_bandwidth;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    hmac_device->intol_bit_recd_40mhz = OSAL_FALSE;

    /* 设置VAP带宽模式为40MHz */
    hmac_vap->channel.en_bandwidth = bandwidth;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    hmac_vap->ch_switch_info.bw_switch_status = WLAN_BW_SWITCH_20_TO_40;

    if (old_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_chan_40M_recovery_timeout_fn
 功能描述  : 40M恢复定时器超时，恢复40M带宽
*****************************************************************************/
OAL_STATIC osal_u32 hmac_chan_40m_recovery_timeout_fn(osal_void *arg)
{
    osal_u32 ret;

    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_chan_40m_recovery_timeout_fn::hmac_vap null.}");
        return OAL_FAIL;
    }

    oam_info_log0(0, OAM_SF_2040, "{hmac_chan_40m_recovery_timeout_fn::40M recovery timer time out.}");

    if (mac_mib_get_2040_switch_prohibited(hmac_vap) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 如果ap初始带宽为20M, 则停止定时器 */
    if ((hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_20M) || (hmac_vap->bandwidth_40m == WLAN_BAND_WIDTH_BUTT)) {
        oam_warning_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::no need 40M recovery because init 20M.}", hmac_vap->vap_id);
        hmac_chan_stop_40m_recovery_timer(hmac_vap);
        return OAL_SUCC;
    }

    if (hmac_vap->intol_user_40m == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::no need recove because intol sta assoc}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
        (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
        oam_warning_log1(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::no need recovery because already 40M}", hmac_vap->vap_id);
        hmac_chan_stop_40m_recovery_timer(hmac_vap);
        return OAL_SUCC;
    }

    ret = hmac_chan_prepare_for_40m_recovery(hmac_vap, hmac_vap->bandwidth_40m);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::prepare_recovery fail! ret = [%d]", hmac_vap->vap_id, ret);
    }
    oam_warning_log2(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::back to bw=%d", hmac_vap->vap_id,
        hmac_vap->bandwidth_40m);

    hmac_chan_multi_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number, hmac_vap->bandwidth_40m);
    ret = hmac_send_notify_chan_width(hmac_vap, BROADCAST_MACADDR);
    if (ret != OAL_SUCC) {
        /* 带宽切换帧发送失败对业务无影响，beacon帧中也有带宽信息 */
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_40m_recovery_timeout_fn::notify_chan_width return %d.}", hmac_vap->vap_id, ret);
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_chan_start_40M_recovery_timer
 功能描述  : 启动obss扫描定时器
*****************************************************************************/
osal_void hmac_chan_start_40m_recovery_timer(hmac_vap_stru *hmac_vap)
{
    osal_u32 timeout;

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    /* 乘以2后需乘以1000转化时间单位 */
    timeout = mac_mib_get_bss_width_trigger_scan_interval(hmac_vap) * 2 * 1000;

    oam_info_log2(0, 0, "vap_id[%d] {hmac_chan_start_40m_recovery_timer::timeout=%d}", hmac_vap->vap_id,
        timeout);
    if ((hmac_vap->recovery_timer_40m.is_registerd != OSAL_TRUE)) {
        frw_create_timer_entry(&(hmac_vap->recovery_timer_40m), hmac_chan_40m_recovery_timeout_fn, timeout, hmac_vap,
            OSAL_TRUE);
    } else {
        frw_timer_restart_timer(&(hmac_vap->recovery_timer_40m), timeout, OSAL_TRUE);
    }
}

static osal_u32 hmac_find_ap_chan_idx(const hmac_vap_stru *hmac_vap, mac_channel_stru *channel, const osal_u8 *ie)
{
    osal_u32 ret;

    channel->chan_number = ie[MAC_IE_HDR_LEN];

    channel->band = (channel->chan_number <= MAC_CHANNEL_FREQ_2_BUTT) ? WLAN_BAND_2G : WLAN_BAND_5G;

    ret = hmac_get_channel_idx_from_num_etc(channel->band, channel->chan_number, &channel->chan_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ap_get_chan_idx_of_network::ht operate get channelidx failed(band [%d], channel[%d])!}",
            hmac_vap->vap_id, channel->band, channel->chan_number);
        return ret;
    }

    ret = hmac_is_channel_idx_valid_etc(channel->band, channel->chan_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_find_ap_chan_idx::channel_idx_validfailed(band [%d], channel[%d], idx[%d])!}",
            hmac_vap->vap_id, channel->band, channel->chan_number, channel->chan_idx);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  :hmac_ap_store_network
 功能描述  : 采集收到的帧所包含的BSSID
*****************************************************************************/
static osal_void hmac_ap_store_network(hmac_device_stru *hmac_device, oal_netbuf_stru *netbuf)
{
    mac_bss_id_list_stru *bss_id_list = &hmac_device->bss_id_list;
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u8 network_bssid[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 *network_bssid_ptr = network_bssid;
    oal_bool_enum_uint8 already_present = OSAL_FALSE;
    osal_u8 loop;
    mac_ieee80211_frame_stru *frame_hdr;

    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctrl->rx_info));
    /* 获取帧体中的BSSID */
    mac_get_bssid((osal_u8 *)frame_hdr, network_bssid_ptr, WLAN_MAC_ADDR_LEN);

    /* 忽略广播BSSID */
    if (oal_compare_mac_addr(BROADCAST_MACADDR, network_bssid_ptr) == 0) {
        return;
    }

    /* 判断是否已经保存了该BSSID */
    for (loop = 0; (loop < bss_id_list->num_networks) && (loop < WLAN_MAX_SCAN_BSS_PER_CH); loop++) {
        if (oal_compare_mac_addr(bss_id_list->bssid_array[loop], network_bssid_ptr) == 0) {
            already_present = OSAL_TRUE;
            break;
        }
    }

    /* 来自一个新的BSS的帧，保存该BSSID */
    if ((already_present == OSAL_FALSE) && (bss_id_list->num_networks < WLAN_MAX_SCAN_BSS_PER_CH)) {
        (osal_void)memcpy_s((osal_u8 *)bss_id_list->bssid_array[bss_id_list->num_networks], WLAN_MAC_ADDR_LEN,
            (osal_u8 *)network_bssid, WLAN_MAC_ADDR_LEN);
        bss_id_list->num_networks++;
    }
}

/*****************************************************************************
 函 数 名  :hmac_ap_wait_start_rx_obss_beacon
 功能描述  : AP在WAIT START状态下，处理obss来的Beacon帧/Probe Response帧
*****************************************************************************/
osal_void hmac_ap_wait_start_rx_obss_beacon(hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    osal_u8 *payload = oal_netbuf_rx_data(netbuf);
    osal_u16 len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);
    osal_u8 *ie = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    mac_sec_ch_off_enum_uint8 sec_ch_offset = MAC_SCN;
#endif
    mac_channel_stru channel = {0, 0, 0, 0xFF}; /* 初始化chan_idx为0xff无效值 */

    if (len < MAC_BEACON_OFFSET) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_ap_wait_start_rx_obss_beacon::len=%d error!}", hmac_vap->vap_id, len);
        return;
    }
    /* 首先查找beacon中的HT信息 */
    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, (payload + MAC_BEACON_OFFSET), (len - MAC_BEACON_OFFSET));
    if (ie != OSAL_NULL && ie[1] >= MAC_HT_OPERN_LEN) {
        if (hmac_find_ap_chan_idx(hmac_vap, &channel, ie) == OAL_SUCC) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
            sec_ch_offset = (ie[MAC_IE_HDR_LEN + 1] & 0x03);
#endif
        }
    } else {
        /* 没有找到HT信息，然后查找beacon 中的DS信息 */
        ie = mac_find_ie_etc(MAC_EID_DSPARMS, (payload + MAC_BEACON_OFFSET), (len - MAC_BEACON_OFFSET));
        if ((ie != OSAL_NULL) && (hmac_find_ap_chan_idx(hmac_vap, &channel, ie) != OAL_SUCC)) {
            return;
        }
    }
    /* 没有找到信息信息 直接返回 */
    if (channel.chan_idx == 0xFF) {
        return;
    }

    hmac_ap_store_network(hmac_device, netbuf);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_ap_update_2040_chan_info(hmac_vap, payload, len, channel.chan_idx, sec_ch_offset);
#endif
}

osal_u32 hmac_ap_obss_init(osal_void)
{
    return OAL_SUCC;
}

osal_void hmac_ap_obss_deinit(osal_void)
{
    return;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
