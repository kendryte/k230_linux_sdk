/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: linux ioctl iwlist配置命令
 * Create: 2022年12月26日
 */

/*****************************************************************************
  1 其他头文件包
*****************************************************************************/
#ifdef _PRE_WLAN_WIRELESS_EXT
#include "wal_linux_iwlist.h"
#include "hmac_mgmt_sta.h"
#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

const osal_u32 g_phyrate_11n_1stream_ht20_longgi[] = {
    65, 130, 195, 260, 390, 520, 585, 650 /* mcs 0~7 单位1M 变成 100k */
};

const osal_u32 g_phyrate_11ac_1stream_vht20_longgi[] = {
    65, 130, 195, 260, 390, 520, 585, 650, 780, 867 /* mcs 0~9 单位1M 变成 100k */
};

const osal_u32 g_phyrate_11ax_1stream_he20_1600us[] = {
    81, 163, 244, 325, 488, 650, 731, 813, 975, 1083, 1219, 1354 /* mcs 0~11 单位1M 变成 100k */
};

static osal_void iw_get_he_gi(wlan_gi_type_enum_uint8 gi_type, osal_u16 *gi_fz, osal_u16 *gi_fm)
{
    switch (gi_type) {
        case WLAN_GI_SHORT:
            *gi_fz = 144; /* 144 规避浮点型 144.0 */
            *gi_fm = 136; /* 136 */
            return;
        case WLAN_GI_LONG:
            *gi_fz = 1;
            *gi_fm = 1;
            return;
        case WLAN_GI_MID:
            *gi_fz = 144; /* 144 规避浮点型 144.0 */
            *gi_fm = 160; /* 160 规避浮点型 160.0 */
            return;
        default:
            return;
    }

    return;
}

static osal_void iw_get_ht_vht_gi(wlan_gi_type_enum_uint8 gi_type, osal_u16 *gi_fz, osal_u16 *gi_fm)
{
    switch (gi_type) {
        case WLAN_GI_SHORT:
            *gi_fz = 10; /* 10 规避浮点型 10.0 */
            *gi_fm = 9; /* 9 */
            return;
        case WLAN_GI_LONG:
            *gi_fz = 1;
            *gi_fm = 1;
            return;
        default:
            return;
    }

    return;
}

static osal_void iw_get_bandwidth_value(wlan_bw_cap_enum_uint8 bandwidth, osal_u16 *bw_fz, osal_u16 *bw_fm)
{
    switch (bandwidth)  {
        case WLAN_BW_CAP_20M:
            *bw_fz = 1;
            *bw_fm = 1;
            return;
        case WLAN_BW_CAP_40M:
            *bw_fz = 108; /* 108 规避浮点型 108.0 */
            *bw_fm = 52; /* 52 */
            return;
        case WLAN_BW_CAP_80M:
            *bw_fz = 234; /* 234 规避浮点型 234.0 */
            *bw_fm = 52; /* 52 */
            return;
        case WLAN_BW_CAP_160M:
            *bw_fz = 468; /* 468 规避浮点型 468.0 */
            *bw_fm = 52; /* 52 */
            return;
        default:
            return;
    }

    return;
}

static osal_void iw_get_he_bandwidth_value(wlan_bw_cap_enum_uint8 bandwidth, osal_u16 *bw_fz, osal_u16 *bw_fm)
{
    switch (bandwidth)  {
        case WLAN_BW_CAP_20M:
            *bw_fz = 1;
            *bw_fm = 1;
            return;
        case WLAN_BW_CAP_40M:
            *bw_fz = 234; /* 234 规避浮点型 234.0 */
            *bw_fm = 117; /* 117 */
            return;
        case WLAN_BW_CAP_80M:
            *bw_fz = 490; /* 490 规避浮点型 490.0 */
            *bw_fm = 117; /* 117 */
            return;
        case WLAN_BW_CAP_160M:
            *bw_fz = 980; /* 980 规避浮点型 980.0 */
            *bw_fm = 117; /* 117 */
            return;
        default:
            return;
    }

    return;
}

static osal_u32 iw_get_11n_rate(osal_u8 mcs, wlan_bw_cap_enum_uint8 bandwidth, osal_u8 is_shortgi,
                                osal_u8 nss)
{
    osal_u32 phyrate = 0;
    osal_u16 bw_fz = 0; /* 计算bw 的分子 */
    osal_u16 bw_fm = 0; /* 计算bw 的分母 */
    osal_u16 gi_fz = 0;
    osal_u16 gi_fm = 0;
    osal_u32 stream_number = nss + 1;
    osal_u32 array_size = osal_array_size(g_phyrate_11n_1stream_ht20_longgi);
    if (stream_number > PHYRATE_STREAM_MAX || mcs  >= array_size) {
        return 0;
    }

    iw_get_bandwidth_value(bandwidth, &bw_fz, &bw_fm);
    iw_get_ht_vht_gi(is_shortgi, &gi_fz, &gi_fm);

    if (bw_fz == 0 || bw_fm == 0 || gi_fz == 0 || gi_fm == 0) {
        return 0;
    }

    /* 单位 100k */
    phyrate = (g_phyrate_11n_1stream_ht20_longgi[mcs] * bw_fz * gi_fz * stream_number) / (gi_fm * bw_fm);
    return phyrate;
}

osal_u32 iw_get_11ac_rate(osal_u8 mcs, wlan_bw_cap_enum_uint8 bandwidth, osal_u8 is_shortgi, osal_u8 nss)
{
    osal_u32 phyrate;
    osal_u16 bw_fz = 0; /* 计算bw 的分子 */
    osal_u16 bw_fm = 0; /* 计算bw 的分母 */
    osal_u16 gi_fz = 0;
    osal_u16 gi_fm = 0;
    osal_u32 stream_number = nss + 1;
    osal_u32 array_size = osal_array_size(g_phyrate_11ac_1stream_vht20_longgi);
    if (stream_number > PHYRATE_STREAM_MAX || mcs  >= array_size) {
        return 0;
    }

    iw_get_bandwidth_value(bandwidth, &bw_fz, &bw_fm);
    iw_get_ht_vht_gi(is_shortgi, &gi_fz, &gi_fm);

    if (bw_fz == 0 || bw_fm == 0 || gi_fz == 0 || gi_fm == 0) {
        return 0;
    }

    /* 单位 100k */
    phyrate = (g_phyrate_11ac_1stream_vht20_longgi[mcs] * bw_fz * gi_fz * stream_number) / (gi_fm * bw_fm);
    return phyrate;
}

static osal_u32 iw_get_11ax_rate(osal_u8 mcs, wlan_bw_cap_enum_uint8 bandwidth, wlan_gi_type_enum_uint8 gi_type,
                                 osal_u8 nss)
{
    osal_u32 phyrate;
    osal_u16 bw_fz = 0; /* 计算bw 的分子 */
    osal_u16 bw_fm = 0; /* 计算bw 的分母 */
    osal_u16 gi_fz = 0;
    osal_u16 gi_fm = 0;
    osal_u32 stream_number = nss + 1;
    osal_u32 array_size = osal_array_size(g_phyrate_11ax_1stream_he20_1600us);
    if (stream_number > HE_PHYRATE_STREAM_MAX || mcs >= array_size) {
        return 0;
    }

    iw_get_he_bandwidth_value(bandwidth, &bw_fz, &bw_fm);
    iw_get_he_gi(gi_type, &gi_fz, &gi_fm);
    if (bw_fz == 0 || bw_fm == 0 || gi_fz == 0 || gi_fm == 0) {
        return 0;
    }

    /* 1 dcm 默认保留1 单位 100k */
    phyrate = (g_phyrate_11ax_1stream_he20_1600us[mcs] * bw_fz * gi_fz * stream_number * 1) / (gi_fm * bw_fm);
    return phyrate;
}

static osal_s8 *iwe_stream_mac_addr_proess(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                           osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    iwe->cmd = SIOCGIWAP;
    iwe->u.ap_addr.sa_family = ARPHRD_ETHER;

    memcpy_s(iwe->u.ap_addr.sa_data, WLAN_MAC_ADDR_LEN, bss_dscr->bssid, WLAN_MAC_ADDR_LEN);
    start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_ADDR_LEN);

    return start;
}

static osal_s8 *iwe_stream_essid_proess(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                        osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    /* Add the ESSID */
    iwe->cmd = SIOCGIWESSID;
    iwe->u.data.flags = 1;
    iwe->u.data.length = (strlen(bss_dscr->ac_ssid) > WLAN_SSID_MAX_LEN - 1) ?
                          WLAN_SSID_MAX_LEN - 1 : strlen(bss_dscr->ac_ssid); /* 32 max */
    start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->ac_ssid);

    return start;
}

static osal_s8 *iwe_stream_protocol_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                            osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    osal_s32 string_len = 0;
    wlan_protocol_enum_uint8 protocol_mode;

    osal_u32 ret = hmac_scan_get_user_protocol_etc(bss_dscr, &protocol_mode);
    if (ret != OAL_SUCC) {
        return start;
    }

    switch (protocol_mode) {
        case WLAN_HE_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11AX");
            break;
        case WLAN_VHT_ONLY_MODE:
        case WLAN_VHT_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11AC");
            break;
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_MODE:
            if (bss_dscr->st_channel.band == WLAN_BAND_2G) {
                string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11bgn");
            } else if (bss_dscr->st_channel.band == WLAN_BAND_5G) {
                string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11an");
            }
            break;
        case WLAN_MIXED_TWO_11G_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11g only");
            break;
        case WLAN_MIXED_ONE_11G_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11bg");
            break;
        case WLAN_LEGACY_11G_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11g");
            break;
        case WLAN_LEGACY_11B_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11b");
            break;
        case WLAN_LEGACY_11A_MODE:
            string_len = snprintf_s(iwe->u.name, IFNAMSIZ, IFNAMSIZ -1, "%s", "IEEE 802.11a");
            break;

        default:
            oam_warning_log0(0, OAM_SF_ANY, "{iwe_stream_protocol_process::protocol_mode not match.}");
            break;
    }

    if (string_len < 0) {
        /* no handle */
        oam_warning_log0(0, OAM_SF_ANY, "{iwe_stream_protocol_process::snprintf_s fail str len < 0}");
    }

    /* Add the protocol name */
    iwe->cmd = SIOCGIWNAME;
    start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_CHAR_LEN);
    return start;
}

static osal_s8 *iwe_stream_chan_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                        osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    osal_u16 freq = oal_ieee80211_channel_to_frequency(bss_dscr->st_channel.chan_number, bss_dscr->st_channel.band);
    /* Add frequency/channel */
    iwe->cmd = SIOCGIWFREQ;
    iwe->u.freq.m = freq * 100000; /* 100000 转换 */
    iwe->u.freq.e = 1;
    iwe->u.freq.i = bss_dscr->st_channel.chan_number;
    start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_FREQ_LEN);
    return start;
}

static osal_s8 *iwe_stream_mode_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                        osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    /* Add mode */
    if (bss_dscr->us_cap_info & (WLAN_CAPABILITY_IBSS | WLAN_CAPABILITY_BSS)) {
        iwe->cmd = SIOCGIWMODE;
        iwe->u.mode =(bss_dscr->us_cap_info & WLAN_CAPABILITY_BSS) ? IW_MODE_MASTER : IW_MODE_ADHOC;
        start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_UINT_LEN);
    }

    return start;
}

static osal_s8 *iwe_stream_encryption_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                              osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    /* Add encryption capability */
    iwe->cmd = SIOCGIWENCODE;
    if (bss_dscr->us_cap_info & WLAN_CAPABILITY_PRIVACY) {
        iwe->u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
    } else {
        iwe->u.data.flags = IW_ENCODE_DISABLED;
    }

    iwe->u.data.length = 0;
    start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->ac_ssid);
    return start;
}

static osal_u32 iwe_stream_get_max_rate(mac_bss_dscr_stru *bss_dscr, wlan_protocol_enum_uint8 protocol_mode)
{
    osal_u8 wlan_gi = WLAN_GI_LONG;
    osal_u8 wlan_mcs;
    mac_frame_ht_cap_stru *ht_cap_info = (mac_frame_ht_cap_stru *)(uintptr_t)(&(bss_dscr->ht_cap_info));

    if (protocol_mode == WLAN_HE_MODE) {
        wlan_mcs = 11; /* 11表示11ax的MCS最大速率为MCS11 */
        wlan_gi = WLAN_GI_SHORT;
        return iw_get_11ax_rate(wlan_mcs, bss_dscr->real_bw, wlan_gi, bss_dscr->support_max_nss);
    }

    /* vht */
    if (protocol_mode == WLAN_VHT_ONLY_MODE || protocol_mode == WLAN_VHT_MODE) {
        /* 当前的WS73只有2.4G，所以扫描到的只会是20M和40M */
        wlan_mcs = (bss_dscr->real_bw == WLAN_BW_CAP_20M) ? WLAN_VHT_MCS8 : WLAN_VHT_MCS9;
        if (bss_dscr->real_bw == WLAN_BW_CAP_20M) {
            wlan_gi = (ht_cap_info->short_gi_20mhz == 1) ? WLAN_GI_SHORT : WLAN_GI_LONG;
        } else if (bss_dscr->real_bw == WLAN_BW_CAP_40M) {
            wlan_gi = (ht_cap_info->short_gi_40mhz == 1) ? WLAN_GI_SHORT : WLAN_GI_LONG;
        }
        return iw_get_11ac_rate(wlan_mcs, bss_dscr->real_bw, wlan_gi, bss_dscr->support_max_nss);
    }

    if (protocol_mode == WLAN_HT_ONLY_MODE || protocol_mode == WLAN_HT_MODE) {
        wlan_mcs = WLAN_HT_MCS7;
        if (bss_dscr->real_bw == WLAN_BW_CAP_20M) {
            wlan_gi = (ht_cap_info->short_gi_20mhz == 1) ? WLAN_GI_SHORT : WLAN_GI_LONG;
        } else if (bss_dscr->real_bw == WLAN_BW_CAP_40M) {
            wlan_gi = (ht_cap_info->short_gi_40mhz == 1) ? WLAN_GI_SHORT : WLAN_GI_LONG;
        }
        return iw_get_11n_rate(wlan_mcs, bss_dscr->real_bw, wlan_gi, bss_dscr->support_max_nss);
    }

    if (protocol_mode == WLAN_MIXED_TWO_11G_MODE || protocol_mode == WLAN_MIXED_ONE_11G_MODE ||
        protocol_mode == WLAN_LEGACY_11G_MODE || protocol_mode == WLAN_LEGACY_11A_MODE) {
        return 54 * 10; /* 54 10 单位 100k */
    }

    if (protocol_mode == WLAN_LEGACY_11B_MODE) {
        return 11 * 10; /* 11 10 单位 100k */
    }

    wifi_printf("mode[%d], nss[%d], bw[%d]\r\n", protocol_mode, bss_dscr->support_max_nss, bss_dscr->real_bw);
    return 0;
}

static osal_s8 *iwe_stream_rate_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                        osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    osal_u16 i;
    osal_s32 string_len;
    osal_u32 ret;
    osal_u32 max_rate = 0;
    osal_u32 ext_rate = 0;
    osal_u8 custom[MAX_CUSTOM_LEN] = {0};
    osal_u8 custom_tmp[MAX_CUSTOM_LEN] = {0};
    wlan_protocol_enum_uint8 protocol_mode;

    ret = hmac_scan_get_user_protocol_etc(bss_dscr, &protocol_mode);
    if (ret != OAL_SUCC) {
        wifi_printf("iwe_stream_rate_process :: hmac_sta_get_user_protocol_etc failed\r\n");
        return start;
    }

    max_rate = iwe_stream_get_max_rate(bss_dscr, protocol_mode);
    if (max_rate == 0) {
        wifi_printf("mode[%d], bw[%d]\r\n", protocol_mode, bss_dscr->channel_bandwidth);
    }

    iwe->cmd = SIOCGIWRATE;
    iwe->u.bitrate.fixed = iwe->u.bitrate.disabled = 0;
    iwe->u.bitrate.value = max_rate * 100000; /* 100000 100k -> 1b */
    start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_PARAM_LEN);

    for (i = 0; i < bss_dscr->num_supp_rates; i++) {
        /* 0x7f 取出实际速率 */
        ext_rate = bss_dscr->supp_rates[i] & 0x7f;
        string_len = snprintf_s(custom_tmp, MAX_CUSTOM_LEN, MAX_CUSTOM_LEN - 1,
                                "%s%d%s ", custom_tmp, (ext_rate >> 1), (ext_rate & 1) ? ".5" : ""); /* .5 5.5 速率 */
        if (string_len < 0) {
            break;
        }
    }

    string_len = snprintf_s(custom, MAX_CUSTOM_LEN, MAX_CUSTOM_LEN - 1, " Rates (Mb/s): %s", custom_tmp);
    if (string_len < 0) {
        // no handle
    }
    iwe->cmd = IWEVCUSTOM;
    iwe->u.data.length = (osal_u32)strlen(custom);
    start = iwe_stream_add_point(info, start, stop, iwe, custom);
    return start;
}

static osal_s8 *iwe_stream_wpa_wpa2_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                            osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    /* 未加密, 无需解析 */
    if ((bss_dscr->us_cap_info & WLAN_CAPABILITY_PRIVACY) == 0) {
        return start;
    }

    /* wep 加密不解析 */
    if ((bss_dscr->wpa_ie == OSAL_NULL) && (bss_dscr->rsn_ie == OSAL_NULL)) {
        return start;
    }

    if (bss_dscr->wpa_ie && bss_dscr->wpa_ie[1] && (bss_dscr->wpa_ie[1] > 0)) {
        iwe->cmd = IWEVGENIE;
        iwe->u.data.length = bss_dscr->wpa_ie[1] + 2; /* 2 开头+len的长度 */
        start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->wpa_ie);
    }

    if (bss_dscr->rsn_ie && bss_dscr->rsn_ie[1] && (bss_dscr->rsn_ie[1] > 0)) {
        iwe->cmd = IWEVGENIE;
        iwe->u.data.length = bss_dscr->rsn_ie[1] + 2; /* 2 开头+len的长度 */
        start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->rsn_ie);
    }

    return start;
}

static osal_s8 *iwe_stream_wps_wapi_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                            osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    if ((bss_dscr->wps_ie == OSAL_NULL) && (bss_dscr->wapi_ie == OSAL_NULL)) {
        return start;
    }

    if (bss_dscr->wps_ie && bss_dscr->wps_ie[1] && (bss_dscr->wps_ie[1] > 0)) {
        iwe->cmd = IWEVGENIE;
        iwe->u.data.length = bss_dscr->wps_ie[1] + 2; /* 2 开头+len的长度 */
        start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->wps_ie);
    }

    if (bss_dscr->wapi_ie && bss_dscr->wapi_ie[1] && (bss_dscr->wapi_ie[1] > 0)) {
        iwe->cmd = IWEVGENIE;
        iwe->u.data.length = bss_dscr->wapi_ie[1] + 2; /* 2 开头+len的长度 */
        start = iwe_stream_add_point(info, start, stop, iwe, bss_dscr->wapi_ie);
    }

    return start;
}

static osal_s8 *iwe_stream_rssi_process(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
                                        osal_s8 *start, osal_s8 *stop, struct iw_event *iwe)
{
    iwe->cmd = IWEVQUAL;
    iwe->u.qual.qual = 0;
    iwe->u.qual.level = (osal_u8)((bss_dscr->c_rssi + 110) * 2); /* rcpi = (rssi + 110）* 2  */
    iwe->u.qual.noise = 0;
    iwe->u.qual.updated |= IW_QUAL_RCPI | IW_QUAL_NOISE_INVALID | IW_QUAL_QUAL_INVALID; /* 只level有效 */

    start = iwe_stream_add_event(info, start, stop, iwe, IW_EV_QUAL_LEN);
    return start;
}

osal_s8 *iwlist_translate_scan(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
    osal_s8 *start, osal_s8 *stop)
{
    struct iw_event iwe;

    memset_s(&iwe, sizeof(iwe), 0, sizeof(iwe));

    start = iwe_stream_mac_addr_proess(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_essid_proess(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_protocol_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_mode_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_rssi_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_chan_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_encryption_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_rate_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_wpa_wpa2_process(bss_dscr, info, start, stop, &iwe);
    start = iwe_stream_wps_wapi_process(bss_dscr, info, start, stop, &iwe);

    return start;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif