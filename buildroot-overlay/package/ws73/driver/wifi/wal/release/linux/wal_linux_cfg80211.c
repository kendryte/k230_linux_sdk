/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 * Description: Linux cfg80211接口
 * Create: 2012年11月8日
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wal_linux_cfg80211.h"
#include "oal_cfg80211.h"
#include "mac_ie.h"
#include "hmac_resource.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef _PRE_WLAN_DFR_STAT
#include "hmac_dfr.h"
#endif
#include "wal_regdb.h"
#include "wal_linux_util.h"
#include "wal_linux_netdev.h"
#include "wal_linux_ioctl.h"
#include "wlan_msg.h"
#include "oal_kernel_file.h"
#include "msg_rekey_offload_rom.h"

#include "osal_types.h"
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) \
    && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "plat_pm_wlan.h"
#include "wal_linux_cfgvendor.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif

#include "wlan_msg.h"
#include "hmac_feature_dft.h"
#include "wal_common.h"
#include "diag_log_common.h"
#include "common_dft.h"
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#include "msg_rekey_offload_rom.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "msg_psm_rom.h"
#endif
#include "common_dft_rom.h"
#include "soc_diag_msg_id.h"
#include "hmac_feature_interface.h"
#include "hmac_11r.h"
#include "hmac_wapi.h"
#include "common_log_dbg_rom.h"
#ifdef CONTROLLER_CUSTOMIZATION
#include "hmac_mgmt_sta.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#endif

#include "wal_linux_android_cmd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define WAL_BCN_BSSID_LENGTH_ADDR    (37)
/* 初始化ko时，只初始化wlan0对应的vap数量值 */
#define WLAN_INIT_WLAN0_VAP_CNT             (1)
/* 初始化ko时，初始化wlan0及p2p0对应的vap数量值 */
#define WLAN_INIT_WLAN0_P2P0_VAP_CNT        (2)
/* 初始化ko时，初始化wlan0、wlan1及p2p0对应的vap数量值 */
#define WLAN_INIT_WLAN0_WLAN1_P2P0_VAP_CNT  (3)
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define WF51_A_RATES           (wf51_rates + 4)
#define WF51_A_RATES_SIZE      8
#define WF51_G_RATES           (wf51_rates + 0)
#define WF51_G_RATES_SIZE      12

#define TMP_5G_CENTER_FREQ 5210
#define TMP_5G_BANDWIDTH 3
/* 设备支持的速率 */
OAL_STATIC oal_ieee80211_rate wf51_rates[] = {
    ratetab_ent(10,  0x1,   0),
    ratetab_ent(20,  0x2,   0),
    ratetab_ent(55,  0x4,   0),
    ratetab_ent(110, 0x8,   0),
    ratetab_ent(60,  0x10,  0),
    ratetab_ent(90,  0x20,  0),
    ratetab_ent(120, 0x40,  0),
    ratetab_ent(180, 0x80,  0),
    ratetab_ent(240, 0x100, 0),
    ratetab_ent(360, 0x200, 0),
    ratetab_ent(480, 0x400, 0),
    ratetab_ent(540, 0x800, 0),
};

/* 2.4G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_2ghz_info[] = {
    chan2g(1, 2412, 0),
    chan2g(2, 2417, 0),
    chan2g(3, 2422, 0),
    chan2g(4, 2427, 0),
    chan2g(5, 2432, 0),
    chan2g(6, 2437, 0),
    chan2g(7, 2442, 0),
    chan2g(8, 2447, 0),
    chan2g(9, 2452, 0),
    chan2g(10, 2457, 0),
    chan2g(11, 2462, 0),
    chan2g(12, 2467, 0),
    chan2g(13, 2472, 0),
    chan2g(14, 2484, 0),
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    chan2g(15, 2512, 0),
    chan2g(16, 2384, 0),
#endif
};

/* 5G 频段 */
OAL_STATIC oal_ieee80211_channel g_ast_supported_channels_5ghz_info[] = {
    chan5g(36, 0),
    chan5g(40, 0),
    chan5g(44, 0),
    chan5g(48, 0),
    chan5g(52, 0),
    chan5g(56, 0),
    chan5g(60, 0),
    chan5g(64, 0),
    chan5g(100, 0),
    chan5g(104, 0),
    chan5g(108, 0),
    chan5g(112, 0),
    chan5g(116, 0),
    chan5g(120, 0),
    chan5g(124, 0),
    chan5g(128, 0),
    chan5g(132, 0),
    chan5g(136, 0),
    chan5g(140, 0),
    chan5g(144, 0),
    chan5g(149, 0),
    chan5g(153, 0),
    chan5g(157, 0),
    chan5g(161, 0),
    chan5g(165, 0),
    /* 4.9G */
    chan4_9g(184, 0),
    chan4_9g(188, 0),
    chan4_9g(192, 0),
    chan4_9g(196, 0),
};

/* 设备支持的加密套件 */
OAL_STATIC const osal_u32 g_ast_wlan_supported_cipher_suites[] = {
    WITP_WLAN_CIPHER_SUITE_WEP40,
    WITP_WLAN_CIPHER_SUITE_WEP104,
    WITP_WLAN_CIPHER_SUITE_TKIP,
    WITP_WLAN_CIPHER_SUITE_CCMP,
#ifdef _PRE_WLAN_FEATURE_GCMP_256_CCMP256
    WITP_WLAN_CIPHER_SUITE_GCMP,
    WITP_WLAN_CIPHER_SUITE_GCMP_256,
    WITP_WLAN_CIPHER_SUITE_CCMP_256,
#endif
    WITP_WLAN_CIPHER_SUITE_AES_CMAC,
    WITP_WLAN_CIPHER_SUITE_BIP_CMAC_256,
    WITP_WLAN_CIPHER_SUITE_SMS4,
};

/* 2.4G 频段信息 */
oal_ieee80211_supported_band g_st_supported_band_2ghz_info = {
    .channels   = g_ast_supported_channels_2ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_2ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates   = WF51_G_RATES,
    .n_bitrates = WF51_G_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
        .mcs.rx_mask[0] = 0xFF,
        .mcs.rx_mask[1] = 0xFF,
    },
};

/* 5G 频段信息 */
OAL_STATIC oal_ieee80211_supported_band g_st_supported_band_5ghz_info = {
    .channels   = g_ast_supported_channels_5ghz_info,
    .n_channels = sizeof(g_ast_supported_channels_5ghz_info) / sizeof(oal_ieee80211_channel),
    .bitrates   = WF51_A_RATES,
    .n_bitrates = WF51_A_RATES_SIZE,
    .ht_cap = {
        .ht_supported = OAL_TRUE,
        .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40,
    },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    .vht_cap = {
        .vht_supported = OAL_TRUE,
        .cap = IEEE80211_VHT_CAP_SHORT_GI_80 | IEEE80211_VHT_CAP_HTC_VHT |
            IEEE80211_VHT_CAP_SHORT_GI_160 | IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ,
    },
#endif
};

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC oal_ieee80211_iface_limit sta_p2p_limits[] = {
    {
        .max = 2,   /* 最大连接数2 */
        .types = BIT(NL80211_IFTYPE_STATION),
    },
    {
        .max = 1,   /* 最大1个AP */
        .types = BIT(NL80211_IFTYPE_AP),
    },
    {
        .max = 2,   /* 最大连接数2 */
        .types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
sta_p2p_iface_combinations[] = {
    {
        .num_different_channels = 2,    /* 不同信道个数是2 */
        .max_interfaces = 3,            /* 最大接口数3 */
        .limits = sta_p2p_limits,
        .n_limits = OAL_ARRAY_SIZE(sta_p2p_limits),
    },
};

#else  /* 非p2p场景 */
OAL_STATIC oal_ieee80211_iface_limit ap_no_limits[] = {
    {
        .max = 4,   /* 最大连接数4 */
        .types = BIT(NL80211_IFTYPE_AP),
    },
    {
        .max = 1,
        .types = BIT(NL80211_IFTYPE_STATION),
    },
};

OAL_STATIC oal_ieee80211_iface_combination
ap_iface_combinations[] = {
    {
        .num_different_channels = 2,    /* 不同信道个数是2 */
        .max_interfaces = 4,            /* 最大接口数4 */
        .limits = ap_no_limits,
        .n_limits = OAL_ARRAY_SIZE(ap_no_limits),
    },
};

#endif

/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes wal_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
    [NL80211_IFTYPE_ADHOC] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4)
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_AP_VLAN] = {
        /* copy AP */
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
    [NL80211_IFTYPE_P2P_GO] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
        BIT(IEEE80211_STYPE_DISASSOC >> 4) |
        BIT(IEEE80211_STYPE_AUTH >> 4) |
        BIT(IEEE80211_STYPE_DEAUTH >> 4) |
        BIT(IEEE80211_STYPE_ACTION >> 4)
    },
#if defined(_PRE_WLAN_FEATURE_P2P)
    [NL80211_IFTYPE_P2P_DEVICE] = {
        .tx = 0xffff,
        .rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
        BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
    },
#endif /* WL_CFG80211_P2P_DEV_IF */
};

#endif   /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */

/* 每个bit 表示cookie array 中是否使用，1 - 已使用；0 - 未使用 */
osal_u8 g_uc_cookie_array_bitmap_etc = 0;
cookie_arry_stru g_cookie_array_etc[WAL_COOKIE_ARRAY_SIZE];

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
/*****************************************************************************
 功能描述  : 查找内核下发的beacon_info中的wmm ie中wmm uapsd是否使能
 输入参数  : osal_u8 *frame_body, osal_s32 l_len

 输出参数  : 无
 返 回 值  : uapsd使能，返回OAL_TRUE，否则，返回OAL_FALSE

 修改历史      :
  1.日    期   : 2015年8月24日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_u32 wal_find_wmm_uapsd_etc(osal_u8 *frame_body, osal_s32 l_len)
{
    osal_s32    l_index = 0;

    /* 判断 WMM UAPSD 是否使能 */
    while (l_index < l_len) {
        if ((frame_body[l_index] == MAC_EID_WMM) &&
            (osal_memcmp(frame_body + l_index + 2, g_wmm_oui_etc, MAC_OUI_LEN) == 0) && /* 偏移2字节 */
            (frame_body[l_index + 2 + MAC_OUI_LEN] == MAC_OUITYPE_WMM) && /* 偏移2字节 */
            (frame_body[l_index + MAC_WMM_QOS_INFO_POS] & BIT7)) {
            return OAL_TRUE;
        } else {
            l_index += (MAC_IE_HDR_LEN + frame_body[l_index + 1]);
        }
    }

    return OAL_FALSE;
}
#endif

/*****************************************************************************
 功能描述  : 在内核下发的信息字段中解析到是否含有wmm字段，即打开或者关闭wmm
 输入参数  : hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年7月16日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_u32 wal_cfg80211_open_wmm_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_open_wmm_etc::hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {wal_cfg80211_open_wmm_etc::this is config vap! can't get info.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* host侧,开关WMM，更新mib信息位中的Qos位置 */
    return hmac_config_open_wmm(hmac_vap, len, param);
}

/*****************************************************************************
 功能描述  : 解析内核传递过来的beacon信息中的WPA/WPA2 信息元素
 输入参数  : oal_beacon_parameters *beacon_info
             mac_beacon_param_stru *beacon_param
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2013年12月11日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_void wal_parse_wpa_wpa2_ie_etc(oal_beacon_parameters *beacon_info,
    mac_beacon_param_stru *beacon_param)
{
    oal_ieee80211_mgmt      *mgmt;
    osal_u16               capability_info;
    osal_u8               *ie_tmp;

    if (beacon_info->head_len < sizeof(oal_ieee80211_mgmt)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_parse_wpa_wpa2_ie_etc::beacon len [%d] error", beacon_info->head_len);
        return;
    }
    /* 判断是否加密 */
    mgmt = (oal_ieee80211_mgmt *)beacon_info->head;

    capability_info = mgmt->u.beacon.capab_info;
    beacon_param->privacy = OAL_FALSE;

    if (WLAN_WITP_CAPABILITY_PRIVACY & capability_info) {
        beacon_param->privacy = OAL_TRUE;

        /* 查找 RSN 信息元素 */
        ie_tmp = mac_find_ie_etc(MAC_EID_RSN, beacon_info->tail, beacon_info->tail_len);
        if (ie_tmp != OAL_PTR_NULL) {
            if (memcpy_s(beacon_param->rsn_ie, sizeof(beacon_param->rsn_ie),
                ie_tmp, MAC_MAX_RSN_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_parse_wpa_wpa2_ie_etc::memcpy_s rsn error}");
            }
        }

        /* 查找 WPA 信息元素，并解析出认证类型 */
        ie_tmp = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, beacon_info->tail,
            beacon_info->tail_len);
        if (ie_tmp != OAL_PTR_NULL) {
            if (memcpy_s(beacon_param->wpa_ie, sizeof(beacon_param->wpa_ie),
                ie_tmp, MAC_MAX_RSN_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_parse_wpa_wpa2_ie_etc::memcpy_s wpa ie error}");
            }
        }
    }
}

/*****************************************************************************
 功能描述  : 解析内核传递过来beacon信息中的Wmm信息元素
 输入参数  : oal_beacon_parameters *beacon_info

 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年7月16日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_u32 wal_parse_wmm_ie_etc(oal_net_device_stru *net_dev, hmac_vap_stru *hmac_vap,
    oal_beacon_parameters *beacon_info)
{
    osal_u8               *wmm_ie;
    osal_u16               len = (osal_u16)sizeof(osal_u8);
    osal_u8                wmm;
    osal_u32               ret;

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    mac_cfg_uapsd_sta_stru  uapsd_param = {0};
#endif

    /*  查找wmm_ie  */
    wmm_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
        beacon_info->tail, beacon_info->tail_len);
    /* wmm ie未找到，则说明wmm 关 */
    wmm = (wmm_ie == OSAL_NULL) ? OSAL_FALSE : OSAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    /*  找到wmm ie，顺便判断下uapsd是否使能 */
    if (wmm_ie != OSAL_NULL) {
        /* 判断WMM信息元素后偏移8字节 的bit7位是否为1,1表示uapsd使能 */
        if (wal_find_wmm_uapsd_etc(beacon_info->tail, beacon_info->tail_len) == OSAL_FALSE) {
            /* 对应UAPSD 关 */
            uapsd_param.uapsd_switch = OSAL_FALSE;
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {wal_parse_wmm_ie_etc::uapsd is disabled}", hmac_vap->vap_id);
        }

        /* 填写 msg 消息体 */
#ifdef _PRE_WLAN_FEATURE_P2P
#if defined(_PRE_PRODUCT_ID_MP13_HOST)
        /* 解决Go不支持UAPSD的问题 */
        /* 该功能目前按照驱动设置来实现，后续需要结合上层设置 */
        if (is_p2p_go(hmac_vap)) {
            /* 2表示通过beacon帧打开uapsd，此时不设置uapsd参数 */
            uapsd_param.uapsd_switch = 2;
            oam_info_log1(0, OAM_SF_CFG,
                          "vap_id[%d] {wal_parse_wmm_ie_etc:: is Go, set uapsd to OPEN)}", hmac_vap->vap_id);
        }
#elif defined(_PRE_PRODUCT_ID_HI1102_HOST)
        uapsd_param.uapsd_switch = OSAL_FALSE;
#endif
        oam_info_log2(0, OAM_SF_CFG, "vap_id[%d] {wal_parse_wmm_ie_etc:: uapsd = %d)}",
                      hmac_vap->vap_id, uapsd_param.uapsd_switch);
#endif

        ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_THRUPUT_TEST,
            (osal_u8 *)&uapsd_param, OAL_SIZEOF(osal_u32));
        if (OAL_UNLIKELY(ret != EXT_SUCCESS)) {
            ret = EXT_FAILURE;
            oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {wal_parse_wmm_ie_etc::uapsd switch set failed[%d].}",
                             hmac_vap->vap_id, ret);
        }
    }
#endif
    /*  wmm 开启/关闭 标记  */
    ret = wal_cfg80211_open_wmm_etc(hmac_vap, len, &wmm);
    if (ret != EXT_SUCCESS) {
        ret = OSAL_FALSE;
        oam_warning_log0(0, OAM_SF_TX, "{wal_parse_wmm_ie_etc::can not open wmm!}");
    }

    return ret;
}

static osal_void wal_cfg80211_config_vap_iftype_etc(hmac_cfg_add_vap_param_stru *add_vap_param)
{
    oal_net_device_stru *net_dev = add_vap_param->net_dev;
    oal_wireless_dev_stru *wdev = net_dev->ieee80211_ptr;

    if (add_vap_param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_AP) {
        wdev->iftype = NL80211_IFTYPE_AP;
    } else if (add_vap_param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_STA) {
        wdev->iftype = NL80211_IFTYPE_STATION;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (add_vap_param->add_vap.p2p_mode == WLAN_P2P_DEV_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (add_vap_param->add_vap.p2p_mode == WLAN_P2P_CL_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (add_vap_param->add_vap.p2p_mode == WLAN_P2P_GO_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */

    if (wdev != OSAL_NULL) {
        oam_warning_log3(0, OAM_SF_CFG, "{wal_cfg80211_config_vap_iftype_etc: add vap_mode[%d] p2p_mode[%d] iftype %d}",
            add_vap_param->add_vap.vap_mode, add_vap_param->add_vap.p2p_mode, wdev->iftype);
    }
}

/*****************************************************************************
 功能描述  : CFG80211 接口添加网络设备
 输入参数  : hmac_cfg_add_vap_param_stru *add_vap_param
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年1月3日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_cfg80211_add_vap_etc(hmac_cfg_add_vap_param_stru *add_vap_param)
{
    /* P2P change interface时不需要重新初始化和注册netdev.
     * nl80211 netlink pre diot 中会获取rntl_lock互斥锁， 注册net_device 会获取rntl_lock互斥锁，造成了死锁 */
    hmac_vap_stru              *cfg_hmac_vap = OAL_PTR_NULL;
    oal_net_device_stru        *net_dev = add_vap_param->net_dev;
    oal_wireless_dev_stru      *wdev = net_dev->ieee80211_ptr;
    mac_wiphy_priv_stru        *wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    osal_s32 ret;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif
    hmac_cfg_add_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    /* 获取mac device */
    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(wiphy_priv->hmac_device->cfg_vap_id);
    if (OAL_UNLIKELY(cfg_hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_CFG, "{cfg_hmac_vap is null vap_id:%d!}", wiphy_priv->hmac_device->cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (OAL_UNLIKELY(cfg_hmac_vap->net_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_add_vap_etc::cfg_net_dev is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wal_cfg80211_config_vap_iftype_etc(add_vap_param);

    vap_info.net_dev     = net_dev;
    vap_info.add_vap.vap_mode     = add_vap_param->add_vap.vap_mode;
    vap_info.add_vap.cfg_vap_indx = cfg_hmac_vap->vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    vap_info.add_vap.p2p_mode      = add_vap_param->add_vap.p2p_mode;
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    vap_info.add_vap.disable_capab_2ght40 = wlan_customize_etc->disable_capab_2ght40;
    wal_add_vap_set_cust(&vap_info);
#endif

    ret = wal_sync_post2hmac_no_rsp(cfg_hmac_vap->vap_id, WLAN_MSG_W2H_CFG_ADD_VAP,
        (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d] {wal_cfg80211_add_vap_etc::fail ret %d}", cfg_hmac_vap->vap_id, ret);
        wifi_printf("vap_id[%d] {wal_cfg80211_add_vap_etc::add vap fail ret %d!}\r\n", cfg_hmac_vap->vap_id, ret);
        return -OAL_EINVAL;
    }
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) \
    && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib_etc(net_dev); /* set random mac to mib ; for 02-cb */
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : CFG80211 接口删除网络设备
 输入参数  : hmac_cfg_del_vap_param_stru *del_vap_param
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年1月3日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_cfg80211_del_vap_etc(hmac_cfg_del_vap_param_stru *del_vap_param)
{
    hmac_cfg_del_vap_param_stru   del_vap_map = {0};
    oal_net_device_stru         *net_dev;
    hmac_vap_stru               *hmac_vap;
    osal_u32 ret;

    if (OAL_UNLIKELY(del_vap_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::del_vap_param null ptr !}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    net_dev = del_vap_param->net_dev;
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_vap_etc::vap is deleted, return sucess}");
        return OAL_SUCC;
    }

    /* 设备在up状态不允许删除，必须先down */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((hmac_vap->vap_state != MAC_VAP_STATE_INIT) && (hmac_vap->vap_state != MAC_VAP_STATE_STA_SCAN_COMP)) {
#else
    if (hmac_vap->vap_state != MAC_VAP_STATE_INIT) {
#endif
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::device busy, down it first %d!}",
            hmac_vap->vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* 不需要释放net_device 结构下的wireless_dev 成员 */
    del_vap_map.net_dev = net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    del_vap_map.add_vap.p2p_mode = del_vap_param->add_vap.p2p_mode;
#endif

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_DEL_VAP,
        (osal_u8 *)&del_vap_map, OAL_SIZEOF(hmac_cfg_del_vap_param_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_del_vap_etc::wal_sync_post2hmac_no_rsp fail ret %d!}", ret);
        wifi_printf("{wal_cfg80211_del_vap_etc::wal_sync_post2hmac_no_rsp fail ret %d!}\r\n", ret);
        return OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 触发上报CAC事件
*****************************************************************************/
osal_s32 wal_cfg80211_cac_report(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 判断指数速率集和扩展速率集中是否包含6M速率作为基本速率
 输入参数  : osal_u8   *supported_rates_ie
             osal_u8    supported_rates_num
             osal_u8   *extended_supported_rates_ie
             osal_u8    extended_supported_rates_num
 输出参数  : 无
 返 回 值  : OAL_STATIC oal_bool_enum : OAL_TRUE    支持
                                        OAL_FALSE   不支持

 修改历史      :
  1.日    期   : 2014年4月23日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_bool_enum wal_check_support_basic_rate_6M(osal_u8 *supp_rates_ie,
    osal_u8  supp_rates_num, osal_u8 *ext_supp_rates_ie, osal_u8  ext_supp_rates_num)
{
    osal_u8     loop;
    oal_bool_enum support = OAL_FALSE;
    for (loop = 0; loop < supp_rates_num; loop++) {
        if (supp_rates_ie == OAL_PTR_NULL) {
            break;
        }
        if (supp_rates_ie[2 + loop] == 0x8c) { /* 2表示IE头部长度,0x8c表示6M */
            support = OAL_TRUE;
        }
    }

    for (loop = 0; loop < ext_supp_rates_num; loop++) {
        if (ext_supp_rates_ie == OAL_PTR_NULL) {
            break;
        }
        if (ext_supp_rates_ie[2 + loop] == 0x8c) { /* 2表示IE头部长度,0x8c表示6M */
            support = OAL_TRUE;
        }
    }

    return support;
}

OAL_STATIC osal_u32 wal_parse_protocol_2g(oal_beacon_parameters *beacon_info, wlan_protocol_enum_uint8 *pen_protocol)
{
    osal_u8   *supp_rates_ie        = OAL_PTR_NULL;
    osal_u8   *ext_supp_rates_ie    = OAL_PTR_NULL;
    osal_u8    supp_rates_num       = 0;
    osal_u8    ext_supp_rates_num   = 0;
    osal_u16   offset;

    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    supp_rates_ie = mac_find_ie_etc(MAC_EID_RATES, beacon_info->head + 24 + offset, /* 偏移24字节 */
        beacon_info->head_len - offset);
    if (supp_rates_ie != OAL_PTR_NULL) {
        supp_rates_num = supp_rates_ie[1];
    }
    ext_supp_rates_ie = mac_find_ie_etc(MAC_EID_XRATES, beacon_info->tail, beacon_info->tail_len);
    if (ext_supp_rates_ie != OAL_PTR_NULL) {
        ext_supp_rates_num = ext_supp_rates_ie[1];
    }

    if (supp_rates_num + ext_supp_rates_num == 4) { /* 支持的速率个数为4 */
        *pen_protocol = WLAN_LEGACY_11B_MODE;
        return OAL_SUCC;
    }
    if (supp_rates_num + ext_supp_rates_num  == 8) { /* 支持的速率个数为8 */
        *pen_protocol = WLAN_LEGACY_11G_MODE;
        return OAL_SUCC;
    }
    if (supp_rates_num + ext_supp_rates_num == 12) { /* 支持的速率个数为12 */
        /* 根据基本速率区分为 11gmix1 还是 11gmix2 */
        /* 如果基本速率集支持 6M , 则判断为 11gmix2 */
        *pen_protocol = WLAN_MIXED_ONE_11G_MODE;
        if (wal_check_support_basic_rate_6M(supp_rates_ie, supp_rates_num,
            ext_supp_rates_ie, ext_supp_rates_num) == OAL_TRUE) {
            *pen_protocol = WLAN_MIXED_TWO_11G_MODE;
        }
        return OAL_SUCC;
    }
    return OAL_FAIL;
}

OAL_STATIC osal_u32 wal_set_protocol_mode(const osal_u8 *ht_ie, const osal_u8 *vht_ie,
    const osal_u8 *he_ie, wlan_protocol_enum_uint8 *protocol)
{
    if (hwifi_get_protocol_enable() == 1) {
        /* 强制设置AP 为11ax 模式 */
        *protocol = WLAN_HE_MODE;
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_protocol_mode::default set protocol (11ax)!}");
        return OAL_SUCC;
    }

    if (he_ie != OSAL_NULL) {
        /* 设置AP 为11ax 模式 */
        *protocol = WLAN_HE_MODE;
    } else if (vht_ie != OSAL_NULL) {
        /* 设置AP 为11ac 模式 */
        *protocol = WLAN_VHT_MODE;
    } else if (ht_ie != OSAL_NULL) {
        /* 设置AP 为11n 模式 */
        *protocol = WLAN_HT_MODE;
    } else {
        return OAL_CONTINUE;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_protocol_mode::hostapd set protocol (%d)!}", *protocol);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 解析协议模式
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年6月12日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_parse_protocol_mode(
    wlan_channel_band_enum_uint8 band,
    oal_beacon_parameters       *beacon_info,
    osal_u8                   *ht_ie,
    osal_u8                   *vht_ie,
    osal_u8                   *he_ie,
    wlan_protocol_enum_uint8    *pen_protocol)
{
    if (wal_set_protocol_mode(ht_ie, vht_ie, he_ie, pen_protocol) != OAL_CONTINUE) {
        return OAL_SUCC;
    }

    if (he_ie != OAL_PTR_NULL) {
        *pen_protocol = WLAN_HE_MODE;
        return OAL_SUCC;
    }
    if (vht_ie != OAL_PTR_NULL) {
        /* 设置AP 为11ac 模式 */
        *pen_protocol = WLAN_VHT_MODE;
        return OAL_SUCC;
    }
    if (ht_ie != OAL_PTR_NULL) {
        *pen_protocol = WLAN_HT_MODE;
        return OAL_SUCC;
    }

    /* hostapd 先设置频段，后设置add beacon 因此在add beacon流程中，可以使用频段信息和速率信息设置AP 模式(a/b/g) */
    if (band == WLAN_BAND_5G) {
        *pen_protocol = WLAN_LEGACY_11A_MODE;
        return OAL_SUCC;
    }
    if (band == WLAN_BAND_2G) {
        if (wal_parse_protocol_2g(beacon_info, pen_protocol) == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    /* 其他情况，认为配置不合理 */
    *pen_protocol = WLAN_PROTOCOL_BUTT;

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 解析内核传递过来的beacon信息中的ht_vht 信息元素
 输入参数  : oal_beacon_parameters *beacon_info
             mac_beacon_param_stru *beacon_param
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2014年4月4日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_parse_ht_vht_ie(hmac_vap_stru *hmac_vap, oal_beacon_parameters *beacon_info,
    mac_beacon_param_stru  *beacon_param)
{
    mac_frame_ht_cap_stru  *ht_cap;
    mac_vht_cap_info_stru  *vht_cap;

    osal_u8 *ht_ie  = mac_find_ie_etc(MAC_EID_HT_CAP, beacon_info->tail, beacon_info->tail_len);
    osal_u8 *vht_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, beacon_info->tail, beacon_info->tail_len);
    osal_u8 *he_ie = mac_find_ie_etc(MAC_EID_HE, beacon_info->tail, beacon_info->tail_len);
    /* 解析协议模式 */
    osal_u32 ul_ret = wal_parse_protocol_mode(hmac_vap->channel.band, beacon_info, ht_ie, vht_ie, he_ie,
        &beacon_param->protocol);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_parse_ht_vht_ie::err code[%d]!}", hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (is_p2p_go(hmac_vap) && hmac_get_p2p_common_enable_info() == 1) {
        /* 只有解析出来带HE字段并且设置了HE协议模式，才是HE */
        if ((beacon_param->protocol == WLAN_HE_MODE) && (hmac_get_p2p_protocol_info() == P2P_PROTOCOL_AX)) {
            beacon_param->protocol = WLAN_HE_MODE;
        } else {
            beacon_param->protocol = WLAN_HT_MODE;
        }
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

    /* 解析short gi能力 */
    if (ht_ie == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    /* 使用ht cap ie中数据域的2个字节 */
    if (ht_ie[1] < OAL_SIZEOF(mac_frame_ht_cap_stru)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {invalid ht cap ie len[%d]!}", hmac_vap->vap_id, ht_ie[1]);
        return OAL_SUCC;
    }

    ht_cap = (mac_frame_ht_cap_stru *)(ht_ie + MAC_IE_HDR_LEN);

    beacon_param->shortgi_20 = (osal_u8)ht_cap->short_gi_20mhz;
    beacon_param->shortgi_40 = 0;

    if ((hmac_vap->channel.en_bandwidth > WLAN_BAND_WIDTH_20M)
        && (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        beacon_param->shortgi_40 = (osal_u8)ht_cap->short_gi_40mhz;
    }

    if (vht_ie == OAL_PTR_NULL) {
        return OAL_SUCC;
    }
    /* 使用vht cap ie中数据域的4个字节 */
    if (vht_ie[1] < OAL_SIZEOF(mac_vht_cap_info_stru)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {invalid vht cap ie len[%d]!}", hmac_vap->vap_id, vht_ie[1]);
        return OAL_SUCC;
    }

    vht_cap = (mac_vht_cap_info_stru *)(vht_ie + MAC_IE_HDR_LEN);
    beacon_param->shortgi_80 = 0;

    if ((hmac_vap->channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS)
        && (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        beacon_param->shortgi_80 = vht_cap->short_gi_80mhz;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
OAL_STATIC osal_s32 uapi_cfg80211_sched_scan_stop(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev);
#else
OAL_STATIC osal_s32 uapi_cfg80211_sched_scan_stop(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    osal_u64 reqid);
#endif
#endif
#endif

/*****************************************************************************
 功能描述  : 停止p2p remain on channel
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2017年07月19日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_p2p_stop_roc(hmac_vap_stru *hmac_vap, oal_net_device_stru *netdev)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device_stru  *hmac_device;
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_P2P, "vap_id[%d] {wal_p2p_stop_roc:: hmac_device[%d] null!}",
                       hmac_vap->vap_id, hmac_vap->device_id);
        return -OAL_EFAIL;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) {
        if (hmac_device->scan_mgmt.is_scanning != OAL_TRUE) {
            oam_error_log1(0, OAM_SF_P2P,
                "vap_id[%d] {wal_p2p_stop_roc::not in scan state but vap is listen state!}", hmac_vap->vap_id);
            return OAL_SUCC;
        }

        hmac_vap->wait_roc_end = OAL_TRUE;
        OAL_INIT_COMPLETION(&(hmac_vap->roc_end_ready));
        wal_force_scan_complete_etc(netdev, OAL_TRUE);
        if (oal_wait_for_completion_timeout(&(hmac_vap->roc_end_ready),
            (osal_u32)oal_msecs_to_jiffies(200)) == 0) { /* 200ms转成jiffies */
            oam_error_log1(0, OAM_SF_P2P,
                "vap_id[%d] {wal_p2p_stop_roc::cancel old roc timout!}", hmac_vap->vap_id);
            return -OAL_EFAIL;
        }
    }
#endif
    return OAL_SUCC;
}

osal_s32 wal_cfg80211_scan_start_wait_condition(const void *param)
{
    hmac_scan_stru *scan_mgmt = (hmac_scan_stru *)param;
    return (OAL_PTR_NULL == scan_mgmt->request);
}

/* 信道测量扫描打断处理函数 */
static void wal_cfg80211_scan_abort_chan_meas(oal_net_device_stru *netdev,
    hmac_device_stru *hmac_device)
{
    // 如果wlan0下发扫描，打断当前正在进行的信道测量扫描
    if ((hmac_device->scan_mgmt.is_scanning == OSAL_TRUE) &&
        hmac_device->scan_mgmt.scan_record_mgmt.is_chan_meas_scan == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SCAN,
                         "{wal_cfg80211_scan_abort_chan_meas::wlan0 scan but chan meas scan is running, abort!}");
        wal_force_scan_complete_etc(netdev, OSAL_TRUE);
    }
}

OAL_STATIC osal_s32 wal_cfg80211_scan_start(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_cfg80211_scan_request_stru *request)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_CONNECT_CHECK);

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if ((hmac_vap == NULL) || (((hmac_vap->vap_state == MAC_VAP_STATE_UP) ||
        (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) && (hmac_vap->not_scan_flag == 1))) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_scan_start::do not scan, null %d}", (hmac_vap == NULL) ? 1 : 0);
        return -OAL_EFAIL;
    }

    if (fhook != OSAL_NULL) {
        if (is_p2p_scan_req(request) && (((hmac_wapi_connected_etc_cb)fhook)(hmac_vap->device_id) == OAL_TRUE)) {
            oam_warning_log0(0, OAM_SF_CFG, "{stop p2p scan under wapi!}");
            return -OAL_EFAIL;
        }
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{uapi_cfg80211_scan::scan failed, hmac_device = null device_id:%d}",
                       hmac_vap->device_id);
        return -OAL_EFAIL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);

    common_log_dbg4(0, OAM_SF_CFG, "vap_id[%d] {uapi_cfg80211_scan::new scan. n_channels[%d], ie_len[%d], n_ssid[%d]}",
                     hmac_vap->vap_id, request->n_channels, request->ie_len, request->n_ssids);

    if (request->n_channels == 0) {
        return -OAL_EFAIL;
    }

    /* gong 不可以重叠扫描, 若上次未结束，需要返回busy  */
    /* 等待上次的扫描请求完成 */
    if (osal_wait_interruptible(&scan_mgmt->wait_queue, wal_cfg80211_scan_start_wait_condition,
        scan_mgmt) < 0) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {uapi_cfg80211_scan::start new scan fail}", hmac_vap->vap_id);
        return -OAL_EFAIL;
    }

    /* p2p normal scan自己可以打断自己上一次的roc */
    if (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {uapi_cfg80211_scan::stop roc scan.}", hmac_vap->vap_id);
        if (wal_p2p_stop_roc(hmac_vap, netdev) < 0) {
            return -OAL_EFAIL;
        }
    }

    /* 保存当前内核下发的扫描请求到本地 */
    scan_mgmt->request = request;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    /* 如果当前调度扫描在运行，先暂停调度扫描 */
    if (scan_mgmt->sched_scan_req != OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {uapi_cfg80211_scan::stop sched scan.}", hmac_vap->vap_id);
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        uapi_cfg80211_sched_scan_stop(wiphy, netdev, 0);
#else
        uapi_cfg80211_sched_scan_stop(wiphy, netdev);
#endif
    }
#endif
#endif
    wal_cfg80211_scan_abort_chan_meas((oal_net_device_stru *)netdev, hmac_device);

    /* 进入扫描 */
    if (wal_scan_work_func_etc(scan_mgmt, netdev, request, hmac_vap) != OAL_SUCC) {
        scan_mgmt->request = OAL_PTR_NULL;
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 内核调用启动扫描的接口函数
*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
OAL_STATIC osal_s32 uapi_cfg80211_scan(oal_wiphy_stru *wiphy, oal_cfg80211_scan_request_stru *request)
#else
OAL_STATIC osal_s32 uapi_cfg80211_scan(oal_wiphy_stru *wiphy,
    oal_net_device_stru *netdev, oal_cfg80211_scan_request_stru *request)
#endif
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    oal_net_device_stru            *netdev;
#endif

    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    if ((wiphy == OAL_PTR_NULL) || (request == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
            "{uapi_cfg80211_scan::null ptr, wiphy[%p], request[%p]!}", (uintptr_t)wiphy, (uintptr_t)request);
        return -OAL_EFAIL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_scan::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    netdev = request->wdev->netdev;
#endif
    if (netdev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_scan::scan failed, null ptr, netdev = null!}");
        return -OAL_EFAIL;
    }

    ret = wal_cfg80211_scan_start(wiphy, netdev, request);
    return (ret > 0) ? -OAL_EFAIL : ret;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
/*****************************************************************************
 功能描述  : 根据rsn_ie获取PMKID
 输入参数  : [1]rsn_ie
 返 回 值  : 无
******************************************************************************/
osal_u16 wal_get_rsn_pmkid(const osal_u8 *rsn_ie)
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
        oam_warning_log1(0, OAM_SF_WPA, "{wal_get_rsn_pmkid::invalid rsn ie len[%d].}", rsn_ie[1]);
        return 0;
    }
    us_index += 8; /* 偏移8 byte，获取pairwise_count */
    pairwise_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{wal_get_rsn_pmkid::invalid pairwise_count[%d].}", pairwise_count);
        return 0;
    }
    us_index += 2 + 4 * (osal_u8)pairwise_count; /* 再偏移(2 + 4 * pairwise_count) byte，获取akm_count */
    akm_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    if (akm_count > MAC_AUTHENTICATION_SUITE_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{wal_get_rsn_pmkid::invalid akm_count[%d].}", akm_count);
        return 0;
    }
    us_index += 2 + 4 * (osal_u8)akm_count; /* 再偏移(2 + 4 * akm_count) byte，获取rsn_capability */
    us_index += 2;                           /* 再偏移rsn_capability的长度 */

    pmkid_count = oal_make_word16(rsn_ie[us_index], rsn_ie[us_index + 1]);
    return pmkid_count;
}
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
/*****************************************************************************
 功能描述  : 设置STA connect 加密信息
 输入参数  : mac_connect_param_stru              *connect_param
             oal_cfg80211_conn_stru              *sme
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_u32

 修改历史      :
  1.日    期   : 2014年1月24日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_void wal_set_crypto_info(mac_conn_param_stru *conn_param, oal_cfg80211_conn_stru *sme)
{
    osal_u8 *ie_tmp = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_UPDATE_CONNECT_PARAM);
    if (fhook != OSAL_NULL) {
        conn_param->wapi = ((hmac_wapi_update_connect_param_cb)fhook)(sme->crypto.wpa_versions);
    }

    if (sme->privacy) {
        if (sme->key_len != 0) {
            /* 设置wep加密信息 */
            conn_param->wep_key               = sme->key;
            conn_param->wep_key_len            = sme->key_len;
            conn_param->wep_key_index          = sme->key_idx;

            return;
        }

        /* 优先查找 RSN 信息元素 */
        ie_tmp = mac_find_ie_etc(MAC_EID_RSN, (osal_u8 *)sme->ie, (osal_s32)sme->ie_len);
        if (ie_tmp != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_FEATURE_WPA3
            osal_u16 pmkid_count = wal_get_rsn_pmkid(ie_tmp);
            conn_param->crypto.have_pmkid = pmkid_count ? 1 : 0;
#endif
            hmac_ie_get_rsn_cipher(ie_tmp, &conn_param->crypto);
            oam_warning_log2(0, OAM_SF_WPA, "wal_set_crypto_info::get_rsn_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                             conn_param->crypto.pair_suite[0], conn_param->crypto.pair_suite[1]);
        } else {
            /* WPA */
            ie_tmp = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
                (osal_u8 *)sme->ie, (osal_s32)sme->ie_len);
            if (ie_tmp != OAL_PTR_NULL) {
                hmac_ie_get_wpa_cipher(ie_tmp, &conn_param->crypto);
                oam_warning_log2(0, OAM_SF_WPA,
                    "wal_set_crypto_info::get_wpa_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                    conn_param->crypto.pair_suite[0], conn_param->crypto.pair_suite[1]);
            }
        }
    }
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
/*****************************************************************************
 功能描述  : 判断是否为P2P DEVICE .如果是P2P device，则不允许关联。
 输入参数  : oal_net_device_stru *net_device
 输出参数  : 无
 返 回 值  : oal_bool_enum_uint8 OAL_TRUE:P2P DEVICE 设备，
                                 OAL_FALSE:非P2P DEVICE 设备

 修改历史      :
  1.日    期   : 2015年5月5日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 wal_is_p2p_device(oal_net_device_stru *net_device)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)net_device->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_TRUE;
    }

    if (is_p2p_dev(hmac_vap)) {
        return OAL_TRUE;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */
    return OAL_FALSE;
}

OAL_STATIC osal_s32 wal_cfg80211_prepare_connect(oal_wiphy_stru *wiphy,
    oal_cfg80211_conn_stru *sme, mac_conn_param_stru *mac_conn_param)
{
    oal_cfg80211_bss_stru *bss = OAL_PTR_NULL;

    /* 解析内核下发的 ssid */
    mac_conn_param->ssid = (osal_u8 *)sme->ssid;
    mac_conn_param->ssid_len = (osal_u8)sme->ssid_len;

#if (defined(_PRE_PRODUCT_ID_HOST))
    /* BEGIN roaming failure between different encryption methods ap. */
    if (sme->bssid) {
        mac_conn_param->bssid = (osal_u8 *)sme->bssid;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
    } else if (sme->bssid_hint) {
        mac_conn_param->bssid = (osal_u8 *)sme->bssid_hint;
#endif
    }
    /* END roaming failure between different encryption methods ap. */
#else
    /* 解析内核下发的 bssid */
    mac_conn_param->bssid = (osal_u8 *)sme->bssid;
#endif
    if (mac_conn_param->bssid == NULL) {
        bss = oal_cfg80211_get_bss_etc(wiphy, NULL, NULL, (osal_u8 *)sme->ssid, (osal_u8)sme->ssid_len);
        if (bss == NULL) {
            oam_warning_log0(0, OAM_SF_ASSOC, "{uapi_cfg80211_connect:: Reject connect with bss=null}");
            return -OAL_EFAIL;
        }

        mac_conn_param->bssid = bss->bssid;

        if (mac_conn_param->bssid == NULL) {
            oam_warning_log0(0, OAM_SF_ASSOC, "{uapi_cfg80211_connect:: Reject connect: no bssid}");
            return -OAL_EFAIL;
        }
    }
    /* 解析内核下发的安全相关参数 */
    wal_connect_set_auth_type(sme->auth_type, &mac_conn_param->auth_type);

    /* 设置加密能力 */
    mac_conn_param->privacy = sme->privacy;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    /* 获取内核下发的pmf是使能的结果 */
    mac_conn_param->mfp = sme->mfp;
    oam_warning_log4(0, OAM_SF_ANY,
                     "{uapi_cfg80211_connect::start a new connect, ssid_len[%d], auth_type[%d], privacy[%d], mfp[%d]}",
                     sme->ssid_len, sme->auth_type, sme->privacy, sme->mfp);
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    mac_conn_param->sae_pwe = sme->crypto.sae_pwe;
#endif

    /* 设置加密参数 */
    wal_set_crypto_info(mac_conn_param, sme);

    /* 设置关联P2P/WPS ie */
    mac_conn_param->ie = (osal_u8 *)sme->ie;
    mac_conn_param->ie_len = (osal_u32)(sme->ie_len);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 解析内核下发的关联命令，sta启动关联
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年8月27日
    修改内容   : 新生成函数
  2.日    期   : 2013年10月24日
    修改内容   : 增加加密认证相关的处理

*****************************************************************************/
OAL_STATIC osal_s32  uapi_cfg80211_connect(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_device, oal_cfg80211_conn_stru *sme)
{
    mac_conn_param_stru    mac_conn_param = {0};
    osal_s32              ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    if ((wiphy == OAL_PTR_NULL) || (net_device == OAL_PTR_NULL) || (sme == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{uapi_cfg80211_connect::connect fail, wiphy, netdev, sme %p, %p, %p}",
            (uintptr_t)wiphy, (uintptr_t)net_device, (uintptr_t)sme);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_connect::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    /* begin:禁止采用p2p device设备启动关联 */
    if (wal_is_p2p_device(net_device)) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_cfg80211_connect:connect stop, p2p device should not connect.");
        return -OAL_EINVAL;
    }
    /* end:禁止采用p2p device设备启动关联 */

    /* 初始化驱动连接参数 */
    ret = wal_cfg80211_prepare_connect(wiphy, sme, &mac_conn_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* BEGIN:关联前需要取消正在进行的扫描，
     * 避免关联命令下发时vap 处在扫描状态，驱动不执行关联命令，导致关联失败 */
    wal_force_scan_complete_etc(net_device, OAL_TRUE);
    /* END:关联前需要取消正在进行的扫描，
     * 避免关联命令下发时vap 处在扫描状态，驱动不执行关联命令，导致关联失败 */

    /* 抛事件给驱动，启动关联 */
    ret = wal_cfg80211_start_connect_etc(net_device, &mac_conn_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_connect::wal_cfg80211_start_connect_etc fail %d!}", ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年8月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_disconnect(oal_wiphy_stru *wiphy, oal_net_device_stru *net_device,
    osal_u16 reason_code)
{
    mac_cfg_kick_user_param_stru kick_user_param;
    osal_s32                    l_ret;
    hmac_user_stru               *hmac_user;
    hmac_vap_stru                *hmac_vap;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    if ((wiphy == OAL_PTR_NULL) || (net_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
            "{uapi_cfg80211_disconnect::input param pointer is null,wiphy, netdev %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)net_device);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_disconnect::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    hmac_vap = (hmac_vap_stru *)net_device->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_disconnect::mac vap has already been del!}");
        return OAL_SUCC;
    }

    /* 填写和sta关联的ap mac 地址 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_cfg80211_disconnect::mac_res_get_hmac_user_etc hmac_user is null, user idx[%d]!}",
                         hmac_vap->assoc_vap_id);
#ifdef CONTROLLER_CUSTOMIZATION
        /* Android resume唤醒定制修改: android唤醒后会主动下发disconnect,由于休眠时device被复位, 会走到此分支,
           在这里主动上报确保wpa_supplicant disconnect流程正常 */
        hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_DEAUTH_LV_SS);
#endif
        return OAL_SUCC;
    }

    /* 解析内核下发的connect参数 */
    (osal_void)memset_s(&kick_user_param, sizeof(mac_cfg_kick_user_param_stru), 0,
        sizeof(mac_cfg_kick_user_param_stru));

    /* 解析内核下发的去关联原因  */
    kick_user_param.reason_code = reason_code;

    if (memcpy_s(kick_user_param.mac_addr, sizeof(kick_user_param.mac_addr),
        hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_disconnect::memcpy_s error}");
    }
    l_ret = wal_cfg80211_start_disconnect_etc(net_device, &kick_user_param);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 配置ptk,gtk等密钥到物理层
 输入参数  : *wiphy
             *netdev
             key_index
             pairwise
             *mac_addr
             *params
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2013年8月19日
    修改内容   : 新生成函数
  2.日    期   : 2013年12月28日
    修改内容   : 使用局部变量替代malloc，以减少释放内存的复杂度

*****************************************************************************/
// OAL_STATIC osal_s32 uapi_cfg80211_add_key(oal_wiphy_stru *wiphy,
//     oal_net_device_stru *netdev,
// #ifdef CONTROLLER_CUSTOMIZATION
//     osal_s32 link_id,
// #endif
//     osal_u8 key_index,
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
//     bool pairwise,
// #endif
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
//     OAL_CONST osal_u8 *mac_addr,
// #else
//     osal_u8 *mac_addr,
// #endif
//     oal_key_params_stru *params)
static int uapi_cfg80211_add_key(
    struct wiphy *wiphy,
    struct net_device *netdev,
    int link_id,          // 新增参数
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    struct key_params *params
)
{
    mac_addkey_param_stru payload_params = {0};
    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* DFR流程 AP恢复时允许add_key 其余情况返回 */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_STA_AP_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_add_key::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 1.1 入参检查 */
    if ((wiphy == OAL_PTR_NULL) || (netdev == OAL_PTR_NULL) || (params == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::Param ERROR, wiphy, netdev, params %p, %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev, (uintptr_t)params);
        return -OAL_EINVAL;
    }

    /* 1.2 key长度检查，防止拷贝越界 */
    if ((params->key_len > OAL_WPA_KEY_LEN) || (params->key_len < 0) ||
        (params->seq_len > OAL_WPA_SEQ_LEN) || (params->seq_len < 0)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::Param Check ERROR! key_len[%x]  seq_len[%x]!}",
                       (osal_s32)params->key_len, (osal_s32)params->seq_len);
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    payload_params.key_index = key_index;

    memset_s(payload_params.mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    if (mac_addr != OAL_PTR_NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        if (memcpy_s(payload_params.mac_addr, sizeof(payload_params.mac_addr), mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::memcpy_s mac_addr error}");
        }
    }

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    payload_params.pairwise = pairwise;
#else
    payload_params.pairwise = (mac_addr != OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
#endif

    /* 2.2 获取相关密钥值 */
    payload_params.key.key_len = params->key_len;
    payload_params.key.seq_len = params->seq_len;
    payload_params.key.cipher  = params->cipher;
    if (memcpy_s(payload_params.key.key, sizeof(payload_params.key.key), params->key,
        (osal_u32)params->key_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::memcpy_s key error}");
    }
    if (memcpy_s(payload_params.key.seq, sizeof(payload_params.key.seq), params->seq,
        (osal_u32)params->seq_len) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::memcpy_s seq error}");
    }
    oam_info_log3(0, OAM_SF_ANY, "{uapi_cfg80211_add_key::key_len:%d, seq_len:%d, cipher:0x%08x!}",
                  params->key_len, params->seq_len, params->cipher);
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_ADD_KEY,
        (osal_u8 *)&payload_params, OAL_SIZEOF(payload_params));

    return (ret > 0) ? -OAL_EFAIL : ret;
}

/*****************************************************************************
功能描述  : 从已保存信息里获取ptk,gtk等密钥
输入参数  : *wiphy
            *netdev
            key_index
            pairwise
            *mac_addr
            *cookie
            *callback
输出参数  : osal_u32
返 回 值  : 0:成功,其他:失败
调用函数  : 无
被调函数  : 无

修改历史      :
 1.日    期   : 2013年8月19日
   修改内容   : 新生成函数
 2.日    期   : 2014年1月4日
   修改内容   : 使用局部变量替代malloc，以减少释放内存的复杂度

*****************************************************************************/
// OAL_STATIC osal_s32 uapi_cfg80211_get_key(oal_wiphy_stru *wiphy,
//     oal_net_device_stru *netdev,
// #ifdef CONTROLLER_CUSTOMIZATION
//     osal_s32 link_id,
// #endif
//     osal_u8 key_index,
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
//     bool pairwise,
// #endif
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
//     OAL_CONST osal_u8 *mac_addr,
// #else
//     osal_u8 *mac_addr,
// #endif
//     void *cookie, void (*callback)(void *cookie, oal_key_params_stru *))
static int uapi_cfg80211_get_key(
    struct wiphy *wiphy,
    struct net_device *netdev,
    int link_id,          // 新增参数
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    void *cookie,
    void (*callback)(void *cookie, struct key_params *params)
)
{
    mac_getkey_param_stru         payload_params  = {0};
    osal_u8                     auc_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_s32 ret;
    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_get_key::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    /* 1.1 入参检查 */
    if (OAL_UNLIKELY(wiphy == OAL_PTR_NULL || netdev == OAL_PTR_NULL ||
        cookie == OAL_PTR_NULL || callback == OAL_PTR_NULL)) {
        oam_error_log4(0, OAM_SF_ANY,
            "{uapi_cfg80211_get_key::Param Check ERROR,wiphy, netdev, cookie, callback %p, %p, %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev, (uintptr_t)cookie, (uintptr_t)callback);
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    payload_params.netdev   = netdev;
    payload_params.key_index = key_index;

    if (mac_addr != OAL_PTR_NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        if (memcpy_s(auc_mac_addr, sizeof(auc_mac_addr), mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_get_key::memcpy_s error}");
        }
        payload_params.mac_addr = auc_mac_addr;
    } else {
        payload_params.mac_addr = OAL_PTR_NULL;
    }

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    payload_params.pairwise  = pairwise;
#else
    payload_params.pairwise = (mac_addr != OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
#endif

    payload_params.cookie = cookie;
    payload_params.callback = callback;
    oam_info_log2(0, OAM_SF_ANY, "{uapi_cfg80211_get_key::idx:%d, pairwise:%d!}", key_index, payload_params.pairwise);
    if (mac_addr != OAL_PTR_NULL) {
        oam_info_log4(0, OAM_SF_ANY, "{uapi_cfg80211_get_key::MAC ADDR: %02X:%02X:%02X:%02X:XX:XX}",
            /* 0:1:2:3:数组下标 */
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
    } else {
        oam_info_log0(0, OAM_SF_ANY, "{uapi_cfg80211_get_key::MAC ADDR IS null!}");
    }
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_GET_KEY,
        (osal_u8 *)&payload_params, OAL_SIZEOF(payload_params));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_get_key::ret val[%d],change to -1}", ret);
        /* 对linux kernel的异常返回值要与内核异常定义匹配; 用自定义值可能会导致内核处理异常卡死 */
        ret = -OAL_EFAIL;
    }
    return ret;
}

/*****************************************************************************
 功能描述  : 把ptk,gtk等密钥从物理层删除
 输入参数  : *wiphy
             *netdev
              key_index
              pairwise
             *mac_addr
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2013年8月19日
    修改内容   : 新生成函数
  2.日    期   : 2014年1月4日
    修改内容   : 使用局部变量替代malloc，以减少释放内存的复杂度

*****************************************************************************/
// OAL_STATIC osal_s32 uapi_cfg80211_remove_key(oal_wiphy_stru *wiphy,
//     oal_net_device_stru *netdev,
// #ifdef CONTROLLER_CUSTOMIZATION
//     osal_s32 link_id,
// #endif
//     osal_u8 key_index,
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37))
//     bool pairwise,
// #endif
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
//     OAL_CONST osal_u8 *mac_addr)
// #else
//     osal_u8 *mac_addr)
// #endif
OAL_STATIC osal_s32 uapi_cfg80211_remove_key(oal_wiphy_stru *wiphy,
    oal_net_device_stru *netdev,
    osal_s32 link_id,
    osal_u8 key_index,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37))
    bool pairwise,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    OAL_CONST osal_u8 *mac_addr)
#else
    osal_u8 *mac_addr)
#endif
{
    osal_s32 ret;
    mac_removekey_param_stru payload_params  = {0};

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    /* 1.1 入参检查 */
    if (OAL_UNLIKELY(wiphy == OSAL_NULL || netdev == OSAL_NULL || netdev->ml_priv == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::Param ERROR, wiphy, netdev %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_remove_key::dfr ing... status[%d]", wal_dfr_get_status());
        return OAL_SUCC;
    }
#endif
    /* 2.1 消息参数准备 */
    payload_params.key_index = key_index;
    memset_s(payload_params.mac_addr, OAL_MAC_ADDR_LEN, 0, OAL_MAC_ADDR_LEN);
    if (mac_addr != OAL_PTR_NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        if (memcpy_s(payload_params.mac_addr, sizeof(payload_params.mac_addr), mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::memcpy_s error}");
        }
    }

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37))
    payload_params.pairwise  = pairwise;
#else
    if (mac_addr != OAL_PTR_NULL) {
        payload_params.pairwise = OAL_TRUE;
        oam_info_log4(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::MAC ADDR: %02X:%02X:%02X:%02X:XX:XX}",
            /* 0:1:2:3:数组下标 */
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
    } else {
        payload_params.pairwise = OAL_FALSE;
        oam_info_log0(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::MAC ADDR IS null!}");
    }
#endif

    oam_info_log2(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::key_index:%d, pairwise:%d!}",
        key_index, payload_params.pairwise);
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_REMOVE_KEY,
        (osal_u8 *)&payload_params, OAL_SIZEOF(payload_params));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_remove_key::msg fail:%d!}", ret);
        wifi_printf("{uapi_cfg80211_remove_key::msg fail:%d!}\r\n", ret);
    }
    return (ret > 0) ? -OAL_EFAIL : ret;
}

/*****************************************************************************
 功能描述  : 使配置的密钥生效
 输入参数  : *wiphy
             *netdev
              key_index
              unicast
              multicast
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2013年8月15日
    修改内容   : 新生成函数
  2.日    期   : 2014年07月31日
    修改内容   : 合并设置数据帧默认密钥和设置管理帧默认密钥函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_set_default_key(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
// #ifdef CONTROLLER_CUSTOMIZATION
    osal_s32 link_id,
// #endif
    osal_u8 key_index,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
    bool unicast, bool multicast
#endif
)
{
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 使配置的密钥生效.PMF 特性使用，配置管理密钥
 输入参数  : *wiphy
             *netdev
              key_index
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2014年11月21日
    修改内容   : 新生成函数

  2.日    期   : 2016年8月29日
    修改内容   : pmf适配

*****************************************************************************/
osal_s32 uapi_cfg80211_set_default_mgmt_key_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
// #ifdef CONTROLLER_CUSTOMIZATION
    osal_s32 link_id,
// #endif
    osal_u8 key_index)
{
    return OAL_SUCC;
}
/*****************************************************************************
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2016年6月25日
    修改内容   : 新生成函数

*****************************************************************************/

osal_u8 wal_cfg80211_convert_value_to_vht_width(osal_s32 l_channel_value)
{
    osal_u8 channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;

    switch (l_channel_value) {
        case 20:    /* channel width 20M */
        case 40:    /* channel width 40M */
            channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
            break;
        case 80:    /* channel width 80M */
            channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_80;
            break;
        case 160:   /* channel width 160M */
            channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_160;
            break;
        default:
            break;
    }

    return channel_vht_width;
}

/*****************************************************************************
 功能描述  : 设置设备工作的频段，信道和带宽
 输入参数  : struct oal_wiphy_stru *wiphy
             struct oal_ieee80211_channel *chan
             oal_nl80211_channel_type channel_type
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月26日
    修改内容   : 新生成函数
  1.日    期   : 2013年10月26日
    修改内容   : 将< KERNEL_VERSION(3, 6, 0)改成<= KERNEL_VERSION(2, 6, 34)
*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34))
OAL_STATIC osal_s32 uapi_cfg80211_set_channel(oal_wiphy_stru *wiphy, oal_ieee80211_channel *chan,
    oal_nl80211_channel_type channel_type)
{
    /* 通过HOSTAPD 设置信道采wal_witp_set_channelnel */
    oam_warning_log0(0, OAM_SF_ANY,
                     "{uapi_cfg80211_set_channel::should not call this function. call uapi_witp_set_channel!}");
    return -OAL_EFAIL;
}
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 5, 7))
OAL_STATIC osal_s32 uapi_cfg80211_set_channel(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_ieee80211_channel *chan, oal_nl80211_channel_type channel_type)
{
    osal_u32 channel;
    osal_s32 ret;
    hmac_vap_stru *hmac_vap;

    if (wiphy == OAL_PTR_NULL || chan == OAL_PTR_NULL || netdev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_channel::invalid params}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_channel::hmac_vap is NULL}");
        return -OAL_EINVAL;
    }

    hmac_vap->channel.band = chan->band;
    hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
    if (channel_type == NL80211_CHAN_HT40MINUS) {
        hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
    } else if (channel_type == NL80211_CHAN_HT40PLUS) {
        hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
    }

    channel = (osal_u32)oal_ieee80211_frequency_to_channel((osal_s32)chan->center_freq);
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CURRENT_CHANEL,
        (osal_u8 *)&channel, OAL_SIZEOF(channel));
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_channel::failed.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 设置wiphy设备的 参数，RTS 门限阈值，分片门限阈值
 输入参数  : oal_wiphy_stru *wiphy
             osal_u32 changed
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    修改内容   : 新生成函数

*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
OAL_STATIC osal_s32 uapi_cfg80211_set_wiphy_params(oal_wiphy_stru *wiphy, osal_u32 changed)
{
    /* 通过HOSTAPD 设置RTS 门限，分片门限 uapi_witp_set_frag uapi_witp_set_rts */
    oam_warning_log0(0, OAM_SF_CFG,
        "{uapi_cfg80211_set_wiphy_params::should not call this function.call uapi_witp_set_frag/uapi_witp_set_rts!}");
    return OAL_SUCC;
}
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
/*****************************************************************************
 功能描述  : 下发安全配置消息
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_set_beacon_cfg(oal_net_device_stru *dev, oal_beacon_parameters *beacon_info,
    mac_beacon_param_stru *beacon_param, osal_u8 vap_id, hmac_vap_stru *hmac_vap)
{
    osal_u32               ul_ret;
    osal_s32                l_ret;

    /* 获取 WPA/WPA2 信息元素 */
    if (beacon_info->tail == OAL_PTR_NULL || beacon_info->head == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_set_beacon::beacon frame error tail = %d, head = %d!}", vap_id,
            beacon_info->tail, beacon_info->head);
        return -OAL_EINVAL;
    }

    wal_parse_wpa_wpa2_ie_etc(beacon_info, beacon_param);

    ul_ret = wal_parse_ht_vht_ie(hmac_vap, beacon_info, beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_set_beacon::Failed to parse HT/VHT ie!}", vap_id);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 驱动对14信道协议模式不做限制,由hostapd管控 */
    if ((hmac_vap->channel.chan_number == 14) && (beacon_param->protocol != WLAN_LEGACY_11B_MODE)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_set_beacon::ch14 protocol is not 11b!}", vap_id);
    }
#endif

    /* 设置操作类型 */
    beacon_param->operation_type = MAC_SET_BEACON;
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(dev), WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON,
        (osal_u8 *)beacon_param, OAL_SIZEOF(mac_beacon_param_stru));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_add_beacon::Failed to start addset beacon!}", vap_id);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置AP参数.包括设置beacon interval, dtim_period 等参数
 输入参数  : oal_wiphy_stru *wiphy
             oal_net_device *dev
             oal_beacon_parameters *info
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2015年6月24日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_set_beacon(oal_wiphy_stru        *wiphy, oal_net_device_stru   *dev,
    oal_beacon_parameters *beacon_info)
{
    mac_beacon_param_stru    beacon_param;  /* beacon info struct */
    hmac_vap_stru            *hmac_vap;
    osal_u32               ul_ret;
    osal_s32                l_ret;
    osal_u8                vap_id;
    osal_u8               *ie;
    osal_u8                ssid_len;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_set_beacon::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    if ((wiphy == OAL_PTR_NULL) || (dev == OAL_PTR_NULL) || (beacon_info == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY,
            "{uapi_cfg80211_set_beacon::wiphy = %x, dev = %x, beacon_info = %x!}", wiphy, dev,
            beacon_info);
        return -OAL_EINVAL;
    }

    if (beacon_info->head == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_set_beacon::beacon_info->head %x!}", beacon_info->head);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = dev->ml_priv;
    vap_id   = hmac_vap->vap_id;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    (osal_void)memset_s(&beacon_param, sizeof(mac_beacon_param_stru), 0, sizeof(mac_beacon_param_stru));
    beacon_param.interval    = beacon_info->interval;
    beacon_param.dtim_period = beacon_info->dtim_period;

    ie = hmac_get_ssid_etc(beacon_info->head + MAC_80211_FRAME_LEN,
        (beacon_info->head_len - MAC_80211_FRAME_LEN), &ssid_len);

    /* 隐藏SSID两种情况:(1)长度为0，(2)长度为实际ssid长度，但内容为空 */
    beacon_param.hidden_ssid = (mac_is_hide_ssid(ie, ssid_len) == OAL_TRUE) ? 1 : 0;

    /* 下发安全配置信息 */
    l_ret = wal_cfg80211_set_beacon_cfg(dev, beacon_info, &beacon_param, vap_id, hmac_vap);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        下发SSID配置消息
    *****************************************************************************/
    if (ssid_len != 0) {
        l_ret = wal_set_ssid(dev, ie, ssid_len);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_add_beacon::send ssid cfg msg fail}", vap_id);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_add_beacon_encap_beacon(oal_net_device_stru *net_dev, oal_beacon_parameters *bcn_info,
    mac_beacon_param_stru *beacon)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    osal_u8 vap_id = hmac_vap->vap_id;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    beacon->interval    = bcn_info->interval;
    beacon->dtim_period = bcn_info->dtim_period;

    /* 获取 WPA/WPA2 信息元素 */
    if (bcn_info->tail == OAL_PTR_NULL || bcn_info->head == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_add_beacon::beacon frame error tail = %d, head = %d!}",
            vap_id, bcn_info->tail, bcn_info->head);
        return -OAL_EINVAL;
    }

    wal_parse_wpa_wpa2_ie_etc(bcn_info, beacon);

    if (wal_parse_ht_vht_ie(hmac_vap, bcn_info, beacon) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_add_beacon::Failed to parse HT/VHT ie!}", vap_id);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 驱动对14信道协议模式不做限制,由hostapd管控 */
    if ((hmac_vap->channel.chan_number == 14) && (beacon->protocol != WLAN_LEGACY_11B_MODE)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_add_beacon::ch14 protocol is not 11b!}", vap_id);
    }
#endif

    /* 设置操作类型 */
    beacon->operation_type = MAC_ADD_BEACON;
    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_add_beacon_event_proc(oal_net_device_stru *net_dev, osal_u8 *ie,
    osal_u8 ssid_len, mac_beacon_param_stru *beacon)
{
    osal_s32 l_ret;
    mac_cfg_ssid_param_stru *ssid_param = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    osal_u8 vap_id = hmac_vap->vap_id;

    l_ret = wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON,
        (osal_u8 *)beacon, OAL_SIZEOF(mac_beacon_param_stru));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "vap_id[%d] {uapi_cfg80211_add_beacon::Failed to start addset beacon!}", vap_id);
        return -OAL_EFAIL;
    }

    /*****************************************************************************
        2. 下发SSID配置消息
    *****************************************************************************/
    if (ssid_len != 0) {
        l_ret = wal_set_ssid(net_dev, ie, ssid_len);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                             "vap_id[%d] {uapi_cfg80211_add_beacon::fail to send ssid cfg msg!}", vap_id);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 设置AP参数.包括设置beacon interval, dtim_period 等参数
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_add_beacon(oal_wiphy_stru *wiphy, oal_net_device_stru *net_dev,
    oal_beacon_parameters *beacon_info)
{
    mac_beacon_param_stru beacon = {0};  /* beacon info struct */
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_s32 ret;
    osal_u8 vap_id;
    osal_u8 *ie = OAL_PTR_NULL;
    osal_u8 ssid_len = 0;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_add_beacon::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    if ((wiphy == OAL_PTR_NULL) || (net_dev == OAL_PTR_NULL) || (beacon_info == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{uapi_cfg80211_add_beacon::wiphy = %x, net_dev = %x, beacon_info = %x!}",
            wiphy, net_dev, beacon_info);
        return -OAL_EINVAL;
    }

    if (beacon_info->head == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_add_beacon::beacon_info->head %x!}", beacon_info->head);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    vap_id = hmac_vap->vap_id;

    ie = hmac_get_ssid_etc(beacon_info->head + MAC_80211_FRAME_LEN,
        (beacon_info->head_len - MAC_80211_FRAME_LEN), &ssid_len);
    /* 隐藏SSID两种情况:(1)长度为0，(2)长度为实际ssid长度，但内容为空 */
    beacon.hidden_ssid = (mac_is_hide_ssid(ie, ssid_len) == OAL_TRUE) ? 1 : 0;

    /* 封装beacon */
    if (wal_cfg80211_add_beacon_encap_beacon(net_dev, beacon_info, &beacon) != OAL_SUCC) {
        return -OAL_EINVAL;
    }

    /* 往dmac发事件 */
    if (wal_cfg80211_add_beacon_event_proc(net_dev, ie, ssid_len, &beacon) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* 3.配置Wmm信息元素 */
    if (wal_parse_wmm_ie_etc(net_dev, hmac_vap, beacon_info) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_add_beacon::Failed to parse wmm ie!}", vap_id);
        return -OAL_EINVAL;
    }

    /* 5.启动AP */
#if defined(_PRE_PRODUCT_ID_HOST)
    ret = wal_start_vap_etc(net_dev);
#else
    ret = wal_netdev_open_etc(net_dev, OAL_FALSE);
#endif
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_add_beacon::failed to start ap, error[%d]}", vap_id, ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 停止AP
 输入参数  : oal_wiphy_stru        *wiphy
             oal_net_device_stru *netdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_del_beacon(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev)
{
    hmac_vap_stru      *hmac_vap;
    osal_s32          l_ret;
    osal_u8          vap_id;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_del_beacon::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 参数合法性检查 */
    if ((wiphy == OAL_PTR_NULL) || (netdev == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_cfg80211_del_beacon::wiphy = %x, netdev = %x!}",
                       wiphy, netdev);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_beacon::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    vap_id = hmac_vap->vap_id;

    /* 判断是否为非ap模式 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_del_beacon::vap is not in ap mode!}", vap_id);
        return -OAL_EINVAL;
    }

    l_ret = wal_down_vap(netdev, vap_id);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY,
                         "vap_id[%d] {uapi_cfg80211_del_beacon::failed to stop ap, error[%d]}", vap_id, l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))

/*****************************************************************************
 功能描述  : 启动ap
 输入参数  : oal_net_device_stru   *netdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_set_ssid(oal_net_device_stru *netdev,
    osal_u8 *ssid_ie, osal_u8 ssid_len)
{
    osal_s32                    l_ret;
    l_ret = wal_set_ssid(netdev, (osal_s8 *)ssid_ie, ssid_len);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_ssid::fail to send ssid cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置vap的auth类型
 输入参数  : oal_net_device_stru   *netdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2016年10月07日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_set_auth_mode(oal_net_device_stru *netdev, osal_u8 auth_algs)
{
    osal_s32                    l_ret;
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_AUTH_MODE,
        (osal_u8 *)&auth_algs, OAL_SIZEOF(auth_algs));

    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_set_auth_mode::send auth msg failed[0x%x]}", (osal_u32)l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_fill_beacon_param_check(oal_net_device_stru *netdev,
    oal_beacon_data_stru *beacon_info, mac_beacon_param_stru *beacon_param)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    if (netdev == OAL_PTR_NULL || beacon_info == OAL_PTR_NULL || beacon_param == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ANY,
            "{wal_cfg80211_fill_beacon_param::para NULL. netdev=0x%p, beacon_info=0x%p, beacon_param=0x%p",
            (uintptr_t)netdev, (uintptr_t)beacon_info, (uintptr_t)beacon_param);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::hmac_vap = %p}", (uintptr_t)hmac_vap);
        return -OAL_EINVAL;
    }

    /*****************************************************************************
        1.安全配置ie消息等
    *****************************************************************************/
    if (beacon_info->tail == OAL_PTR_NULL || beacon_info->head == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_fill_beacon_param::beacon frame error tail = %p, head = %p!}",
            hmac_vap->vap_id, (uintptr_t)beacon_info->tail, (uintptr_t)beacon_info->head);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_encap_beacon_parameters(oal_net_device_stru *netdev,
    oal_beacon_data_stru *beacon_info,
    mac_beacon_param_stru *beacon_param,
    osal_u8 *beacon_buf)
{
    oal_beacon_parameters beacon_info_tmp = {0};
    osal_u32 beacon_head_len = (osal_u32)beacon_info->head_len;
    osal_u32 beacon_tail_len = (osal_u32)beacon_info->tail_len;
    osal_u32 ul_ret;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)netdev->ml_priv;

    if (memcpy_s(beacon_buf, beacon_head_len, beacon_info->head, beacon_head_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_encap_beacon_parameters::memcpy_s head error}");
    }
    if (memcpy_s(beacon_buf + beacon_head_len, beacon_tail_len, beacon_info->tail, beacon_tail_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_encap_beacon_parameters::memcpy_s tail error}");
    }

    /* 为了复用51的解析接口，将新内核结构中的内容赋值给51接口识别的结构体，进而获取信息元素 */
    beacon_info_tmp.head     = beacon_buf;
    beacon_info_tmp.head_len = (osal_s32)beacon_head_len;
    beacon_info_tmp.tail     = beacon_buf + beacon_head_len;
    beacon_info_tmp.tail_len = (osal_s32)beacon_tail_len;

    /* 获取 WPA/WPA2 信息元素 */
    wal_parse_wpa_wpa2_ie_etc(&beacon_info_tmp, beacon_param);

    /* 此接口需要修改，linux上没问题，但是win32有错 */
    ul_ret = wal_parse_ht_vht_ie(hmac_vap, &beacon_info_tmp, beacon_param);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "vap_id[%d] {wal_cfg80211_fill_beacon_param::failed to parse HT/VHT ie!}", hmac_vap->vap_id);
        return -OAL_EINVAL;
    }

    /* 解析是否隐藏SSID */
    if (beacon_buf[WAL_BCN_BSSID_LENGTH_ADDR] == 0) {
        beacon_param->hidden_ssid = 1;
    }

    /* 配置Wmm信息元素 */
    ul_ret = wal_parse_wmm_ie_etc(netdev, hmac_vap, &beacon_info_tmp);
    if (ul_ret != OAL_SUCC) {
        /* 此处在整改前存在内存泄漏 */
        oam_warning_log1(0, OAM_SF_ANY,
                         "vap_id[%d] {wal_cfg80211_fill_beacon_param::Failed to parse wmm ie!}", hmac_vap->vap_id);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 将要下发的修改的beacon帧参数填入到入参结构体中
*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_fill_beacon_param(oal_net_device_stru *netdev,
    oal_beacon_data_stru *beacon_info, mac_beacon_param_stru *beacon_param)
{
    osal_s32 ret;
    osal_u8 *beacon_buf = OAL_PTR_NULL;
    osal_u16 beacon_head_len;
    osal_u16 beacon_tail_len;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)netdev->ml_priv;

    ret = wal_cfg80211_fill_beacon_param_check(netdev, beacon_info, beacon_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    beacon_head_len = (osal_u16)beacon_info->head_len;
    beacon_tail_len = (osal_u16)beacon_info->tail_len;
    /* 运算溢出保护 */
    if (beacon_head_len + beacon_tail_len > WLAN_MGMT_NETBUF_SIZE) {
        oam_error_log2(0, OAM_SF_ANY, "wal_cfg80211_fill_beacon_param::beacon_head_len[%d],beacon_tail_len[%d]",
            beacon_head_len, beacon_tail_len);
        return -OAL_EINVAL;
    }
    beacon_buf = (osal_u8 *)(oal_memalloc(beacon_head_len + beacon_tail_len));
    if (beacon_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_fill_beacon_param::puc_beacon_info_tmp memalloc failed.}", hmac_vap->vap_id);
        return -OAL_EINVAL;
    }

    memset_s(beacon_buf, beacon_head_len + beacon_tail_len, 0, beacon_head_len + beacon_tail_len);
    ret = wal_cfg80211_encap_beacon_parameters(netdev, beacon_info, beacon_param, beacon_buf);
    oal_free(beacon_buf);
    beacon_buf = OAL_PTR_NULL;
    if (ret != OAL_SUCC) {
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((hmac_vap->channel.chan_number == 14) && (beacon_param->protocol != WLAN_LEGACY_11B_MODE)) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_fill_beacon_param::ch 14 support 11bgn/ax, protocol is %d!}",
            hmac_vap->vap_id, beacon_param->protocol);
    }
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 修改ap beacon帧配置参数
 输入参数  : oal_wiphy_stru          *wiphy
             oal_net_device_stru     *netdev
             struct cfg80211_beacon_data *info
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_change_beacon(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_beacon_data_stru *beacon_info)
{
    mac_beacon_param_stru        beacon_param;  /* beacon info struct */
    osal_s32                    l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    oam_info_log0(0, OAM_SF_ANY, "{uapi_cfg80211_change_beacon::enter here.}");
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_change_beacon::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == netdev) || (OAL_PTR_NULL == beacon_info)) {
        oam_error_log3(0, OAM_SF_ANY,
            "{uapi_cfg80211_change_beacon::wiphy = %p, netdev = %p, beacon_info = %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev, (uintptr_t)beacon_info);
        return -OAL_EINVAL;
    }

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    (osal_void)memset_s(&beacon_param, sizeof(mac_beacon_param_stru), 0, sizeof(mac_beacon_param_stru));

    l_ret = wal_cfg80211_fill_beacon_param(netdev, beacon_info, &beacon_param);
    if (OAL_SUCC != l_ret) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_change_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    beacon_param.operation_type = MAC_SET_BEACON;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON,
        (osal_u8 *)&beacon_param, OAL_SIZEOF(beacon_param));
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_change_beacon::Failed to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 将内核下发的带宽枚举转换成真实的带宽宽度值
 输入参数  : osal_s32 l_channel_width
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2015年1月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_convert_width_to_value(osal_s32 l_channel_width)
{
    osal_s32 chan_width_value = 0;

    switch (l_channel_width) {
        case 0:
        case 1:
            chan_width_value = 20;     /* channel width 20M */
            break;
        case 2:                        /* 2 for channel width 40M */
            chan_width_value = 40;     /* channel width 40M */
            break;
        case 3:                        /* 3 for channel width 40M */
        case 4:                        /* 4 for channel width 40M */
            chan_width_value = 80;     /* channel width 80M */
            break;
        case 5:                        /* 5 for channel width 160M */
            chan_width_value = 160;    /* channel width 160M */
            break;
        default:
            break;
    }

    return chan_width_value;
}

OAL_STATIC wlan_channel_bandwidth_enum_uint8 wal_cfg80211_set_channel_bandwidth(osal_s32 l_bandwidth_value,
    osal_s32 l_channel, osal_s32 center_freq0, osal_s32 center_freq1, osal_u8 vht_width)
{
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;

    if (l_bandwidth_value == 80 || l_bandwidth_value == 160) {  /* channel width is 80M or 160M */
        en_bandwidth = mac_get_bandwith_from_center_freq_seg0_seg1(vht_width,
            (osal_u8)l_channel, (osal_u8)center_freq0, (osal_u8)center_freq1);
    } else if (l_bandwidth_value == 40) {                       /* channel width is 40M */
        switch (center_freq0 - l_channel) {
            case -2:                                            /* -2 for channel width 40M minus */
                en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;
            case 2:                                             /* 2 for channel width 40M plus */
                en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                en_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return en_bandwidth;
}

OAL_STATIC osal_s32 wal_cfg80211_set_channel_info_event_proc(oal_net_device_stru *netdev,
    oal_ieee80211_channel *channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth,
    osal_u8 vap_id)
{
    osal_s32 l_ret;
    l_ret = wal_set_channel_info(vap_id, (osal_u8)channel->hw_value, channel->band, en_bandwidth);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{wal_cfg80211_set_channel_info::wal_set_channel_info return err code: [%d]!}", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 设置信道信息
*****************************************************************************/
OAL_STATIC osal_s32 wal_cfg80211_set_channel_info(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev)
{
    oal_cfg80211_chan_def *chan_def = OAL_PTR_NULL;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    osal_s32 l_channel;
    osal_s32 l_center_freq1;
    osal_s32 l_center_freq2;
    osal_s32 l_bandwidth;
    osal_s32 l_bandwidth_value;
    osal_u8 vht_width;
    osal_s32 l_ret;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_u8 vap_id;
    osal_s32 center_freq0;
    osal_s32 center_freq1;

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    vap_id   = hmac_vap->vap_id;
#ifdef CONTROLLER_CUSTOMIZATION
    chan_def = &(netdev->ieee80211_ptr->u.ap.preset_chandef);
#else
    // chan_def = &(netdev->ieee80211_ptr->preset_chandef);
    chan_def = &(netdev->ieee80211_ptr->u.ap.preset_chandef);
#endif

    /* 5G SoftAP暂时写死中心频率5210, 频宽80M(width 3), 待hostapd升级后删除 */
    if (chan_def->chan->band == OAL_IEEE80211_BAND_5GHZ) {
        chan_def->center_freq1 = TMP_5G_CENTER_FREQ;
        chan_def->width = TMP_5G_BANDWIDTH;
    }
    l_bandwidth    = chan_def->width;
    l_center_freq1 = chan_def->center_freq1;
    l_center_freq2 = chan_def->center_freq2;
    l_channel      = chan_def->chan->hw_value;

    oam_warning_log4(0, OAM_SF_ANY,
        "{wal_cfg80211_set_channel_info::l_bandwidth = %d, l_center_freq1 = %d, l_center_freq2 = %d, l_channel = %d.}",
        l_bandwidth, l_center_freq1, l_center_freq2, l_channel);

    /* 判断信道在不在管制域内 */
    l_ret = (osal_s32)hmac_is_channel_num_valid_etc(chan_def->chan->band, (osal_u8)l_channel);
    if (l_ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_set_channel_info::channel num is invalid. band, ch num [%d] [%d]!}",
            vap_id, chan_def->chan->band, l_channel);
        return -OAL_EINVAL;
    }

    /* 进行内核带宽值和WITP 带宽值转换 */
    center_freq0 = oal_ieee80211_frequency_to_channel(l_center_freq1);
    center_freq1 = oal_ieee80211_frequency_to_channel(l_center_freq2);
    l_bandwidth_value = wal_cfg80211_convert_width_to_value(l_bandwidth);
    vht_width = wal_cfg80211_convert_value_to_vht_width(l_bandwidth_value);
    if (l_bandwidth_value == 0) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_set_channel_info::channel width is invalid, l_bandwidth = %d.",
            vap_id, l_bandwidth);
        return -OAL_EINVAL;
    }
    en_bandwidth = wal_cfg80211_set_channel_bandwidth(l_bandwidth_value, l_channel, center_freq0,
        center_freq1, vht_width);
#ifdef _PRE_WLAN_FEATURE_P2P
    if (is_p2p_go(hmac_vap) && hmac_get_p2p_common_enable_info() == 1) {
        if (hmac_get_p2p_common_band_info() == P2P_BAND_20M) {
            en_bandwidth = WLAN_BAND_WIDTH_20M;
        } else if (l_channel < 5) { /* 5表示5信道 */
            en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        } else {
            en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
        }
    }
#endif
    wifi_printf("[%s] [%d] en_bandwidth[%d]\r\n", __func__, __LINE__, en_bandwidth);
    /* 抛事件到wal层处理 */
    return wal_cfg80211_set_channel_info_event_proc(netdev, chan_def->chan, en_bandwidth, vap_id);
}

OAL_STATIC osal_s32 wal_cfg80211_start_ap_set_ssid(oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings,
    osal_u8 vap_id)
{
    osal_u8 *ssid_ie = OAL_PTR_NULL;
    osal_u8 ssid_len;
    osal_u8 auc_ssid_ie[32] = {0};
    osal_s32 l_ssid_len = ap_settings->ssid_len;
    osal_s32 l_ret;

    if ((l_ssid_len > 32) || (l_ssid_len <= 0)) {   /* SSID字符长度32 */
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_start_ap::ssid len error, len[%d].}", vap_id, l_ssid_len);
        return -OAL_EFAIL;
    }
    (osal_void)memset_s(auc_ssid_ie, sizeof(auc_ssid_ie), 0, sizeof(auc_ssid_ie));
    if (memcpy_s(auc_ssid_ie, sizeof(auc_ssid_ie), ap_settings->ssid, (osal_u32)l_ssid_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_start_ap_set_ssid::memcpy_s error}");
    }
    ssid_ie = auc_ssid_ie;
    ssid_len = (osal_u8)l_ssid_len;

    if (ssid_len != 0) {
        l_ret = wal_cfg80211_set_ssid(netdev, ssid_ie, ssid_len);
        if (l_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY,
                "vap_id[%d] {uapi_cfg80211_start_ap::fail to send ssid cfg msg,[0x%x]}", vap_id, (osal_u32)l_ret);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_start_ap_set_auth_mode(oal_net_device_stru *netdev,
    oal_nl80211_auth_type_enum_uint8 auth_type,
    osal_u8 vap_id)
{
    osal_s32 l_ret;
    wlan_auth_alg_mode_enum_uint8 auth_algs;

    if (auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM) {
        auth_algs = WLAN_WITP_AUTH_OPEN_SYSTEM;
    } else if (auth_type == NL80211_AUTHTYPE_SHARED_KEY) {
        auth_algs = WLAN_WITP_AUTH_SHARED_KEY;
    } else if (auth_type == NL80211_AUTHTYPE_AUTOMATIC) {
        auth_algs = WLAN_WITP_AUTH_AUTOMATIC;
    } else {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_start_ap::auth_type error[%d].}", vap_id, auth_type);
        return -OAL_EFAIL;
    }

    l_ret = wal_cfg80211_set_auth_mode(netdev, auth_algs);
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_start_ap::fail to send auth_tpye cfg msg,[0x%x]}", vap_id, (osal_u32)l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_cfg80211_start_ap_set_beacon(oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings,
    osal_u8 vap_id)
{
    mac_beacon_param_stru beacon_param = {0};  /* beacon info struct */
    oal_beacon_data_stru *beacon_info = OAL_PTR_NULL;
    osal_s32 l_ret;

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    beacon_param.interval    = ap_settings->beacon_interval;
    beacon_param.dtim_period = ap_settings->dtim_period;
#ifdef _PRE_WLAN_FEATURE_WPA3
    beacon_param.sae_pwe = ap_settings->crypto.sae_pwe;
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    beacon_param.hidden_ssid = (ap_settings->hidden_ssid == 1);

    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_cfg80211_fill_beacon_param::beacon_interval=%d, dtim_period=%d, hidden_ssid=%d!}",
        ap_settings->beacon_interval, ap_settings->dtim_period, ap_settings->hidden_ssid);
#endif
    beacon_info = &(ap_settings->beacon);
    l_ret = wal_cfg80211_fill_beacon_param(netdev, beacon_info, &beacon_param);
    if (l_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_start_ap::failed to fill beacon param, [0x%x]}", vap_id, (osal_u32)l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    beacon_param.operation_type = MAC_ADD_BEACON;
#ifdef _PRE_WLAN_FEATURE_DFR
    // 保存beacon信息用作自愈
    hmac_dfr_fill_ap_recovery_info(WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON, &beacon_param,
        (hmac_vap_stru *)netdev->ml_priv);
#endif
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON,
        (osal_u8 *)&beacon_param, OAL_SIZEOF(beacon_param));

    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_start_ap::failed to start addset beacon, [0x%x]}", vap_id, (osal_u32)l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 启动AP,配置AP 参数。
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_start_ap(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_ap_settings_stru *ap_settings)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_s32 ret;
    osal_u8 vap_id;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    oam_info_log0(0, OAM_SF_ANY, "{uapi_cfg80211_start_ap::enter here.}");
#ifdef _PRE_WLAN_FEATURE_DFR
    /* DFR流程 AP恢复时允许start_ap 其余情况返回 */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_STA_AP_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_start_ap::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 参数合法性检查 */
    if ((wiphy == OAL_PTR_NULL) || (netdev == OAL_PTR_NULL) || (ap_settings == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{uapi_cfg80211_start_ap::wiphy=%p,netdev=%p,ap_settings=%p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev, (uintptr_t)ap_settings);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_start_ap::hmac_vap = %p}", (uintptr_t)hmac_vap);
        return -OAL_EINVAL;
    }

    vap_id = hmac_vap->vap_id;

    /*****************************************************************************
        1.设置信道
    *****************************************************************************/
    ret = wal_cfg80211_set_channel_info(wiphy, netdev);
    if (ret != OAL_SUCC) {
        goto return_handle;
    }

    /*****************************************************************************
        2.1 设置ssid信息
    *****************************************************************************/
    ret = wal_cfg80211_start_ap_set_ssid(netdev, ap_settings, vap_id);
    if (ret != OAL_SUCC) {
        goto return_handle;
    }

    /*****************************************************************************
        2.2 设置auth mode信息
    *****************************************************************************/
    ret = wal_cfg80211_start_ap_set_auth_mode(netdev, ap_settings->auth_type, vap_id);
    if (ret != OAL_SUCC) {
        goto return_handle;
    }

    /*****************************************************************************
        3.设置beacon时间间隔、tim period以及安全配置消息等
    *****************************************************************************/
    ret = wal_cfg80211_start_ap_set_beacon(netdev, ap_settings, vap_id);
    if (ret != OAL_SUCC) {
        goto return_handle;
    }

    /*****************************************************************************
        4.启动ap
    *****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HOST)
    ret = wal_start_vap_etc(netdev);
#else
    ret = wal_netdev_open_etc(netdev, OAL_FALSE);
#endif

return_handle:
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d] {uapi_cfg80211_start_ap::failed to start ap, error[%d]}", vap_id, ret);
    }
    return (ret > 0) ? -OAL_EFAIL : ret;
}

/*****************************************************************************
 功能描述  : 停止AP
 输入参数  : oal_wiphy_stru        *wiphy
             oal_net_device_stru *netdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
// OAL_STATIC osal_s32 uapi_cfg80211_stop_ap(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev
// #ifdef CONTROLLER_CUSTOMIZATION
//     , osal_u32 link_id
// #endif
// )
static int uapi_cfg80211_stop_ap(
    struct wiphy *wiphy,
    struct net_device *netdev,
    unsigned int link_id  // 新增参数
)
{
    hmac_vap_stru                *hmac_vap;
    osal_s32                    l_ret;
    osal_u8                    vap_id;

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode;
    oal_wireless_dev_stru   *wdev;
#endif
    mac_cfg_start_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* DFR流程, 仅允许P2P恢复时接受此命令(GO DOWN需要) */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_P2P_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_stop_ap::dfr ing... status[%d]", wal_dfr_get_status());
        return OAL_SUCC;
    }
#endif
    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == netdev)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_cfg80211_stop_ap::wiphy = %p, netdev = %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev);
        return -OAL_EINVAL;
    }

    /* 获取vap id */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (OAL_PTR_NULL == hmac_vap) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_stop_ap::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    vap_id   = hmac_vap->vap_id;

    /* 判断是否为非ap模式 */
    if (WLAN_VAP_MODE_BSS_AP != hmac_vap->vap_mode) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_stop_ap::vap is not in ap mode!}", vap_id);
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    wdev    = netdev->ieee80211_ptr;
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (WLAN_P2P_BUTT == p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_stop_ap::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}");
        return -OAL_EINVAL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_stop_ap::p2p_mode=%u}", p2p_mode);

    vap_info.p2p_mode = p2p_mode;
#endif

    vap_info.net_dev = netdev;

    l_ret = wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_DOWN_VAP, (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d] {uapi_cfg80211_stop_ap::failed to stop ap, error[%d]}", vap_id, l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 修改bss参数信息
 输入参数  : oal_wiphy_stru        *wiphy
             oal_net_device_stru   *netdev
             oal_bss_parameters    *bss_params
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_change_bss(oal_wiphy_stru        *wiphy, oal_net_device_stru   *netdev,
    oal_bss_parameters    *bss_params)
{
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 功能描述  : 打印上层下发的调度扫描请求信息
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年6月19日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_void  wal_cfg80211_print_sched_scan_req_info(oal_cfg80211_sched_scan_request_stru *request)
{
    osal_s8      ac_tmp_buff[200];
    osal_s32     l_loop;
    osal_s32 ret_len;

    /* 打印基本参数 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        request->n_channels, request->scan_plans[0].interval, request->flags,
        request->min_rssi_thold);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        request->n_channels, request->interval, request->flags, request->min_rssi_thold);
#else
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]ms,flags[%d],rssi_thold[%d]",
        request->n_channels, request->interval, request->flags, request->rssi_thold);
#endif

    /* 打印ssid集合的信息 */
    for (l_loop = 0; l_loop < request->n_match_sets; l_loop++) {
        memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
        ret_len = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff),
            "mactch_sets[%d] info, ssid_len[%d], ssid: %.32s.\n",
            l_loop, request->match_sets[l_loop].ssid.ssid_len, request->match_sets[l_loop].ssid.ssid);
        if (ret_len < 0) {
            oam_warning_log1(0, OAM_SF_ANY,
                "wal_cfg80211_print_sched_scan_req_info ret_len %d", ret_len);
        }
        dft_report_params_etc(ac_tmp_buff, osal_strlen(ac_tmp_buff), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    }

    for (l_loop = 0; l_loop < request->n_ssids; l_loop++) {
        memset_s(ac_tmp_buff, OAL_SIZEOF(ac_tmp_buff), 0, OAL_SIZEOF(ac_tmp_buff));
        ret_len = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff),
            "ssids[%d] info, ssid_len[%d], ssid: %.32s.\n",
            l_loop, request->ssids[l_loop].ssid_len, request->ssids[l_loop].ssid);
        if (ret_len < 0) {
            oam_warning_log1(0, OAM_SF_ANY,
                "wal_cfg80211_print_sched_scan_req_info ret_len %d", ret_len);
        }
        dft_report_params_etc(ac_tmp_buff, osal_strlen(ac_tmp_buff), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    }

    return;
}

/*****************************************************************************
 功能描述  : 检查ssid是否在ssid_list列表里面
 输入参数  : struct oal_cfg80211_ssid_stru *ssid,
             struct oal_cfg80211_ssid_stru *ssid_list
             osal_s32 l_count
 输出参数  : 无
 返 回 值  : oal_bool_enum_uint8

 修改历史      :
  1.日    期   : 2016年8月2日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 wal_pno_scan_with_assigned_ssid(oal_cfg80211_ssid_stru *ssid,
    oal_cfg80211_ssid_stru *ssid_list, osal_s32 l_count)
{
    osal_s32 l_loop;

    if ((OAL_PTR_NULL == ssid) || (OAL_PTR_NULL == ssid_list)) {
        return OAL_FALSE;
    }

    for (l_loop = 0; l_loop < l_count; l_loop++) {
        if ((ssid->ssid_len == ssid_list[l_loop].ssid_len) &&
            (osal_memcmp(ssid->ssid, ssid_list[l_loop].ssid, ssid->ssid_len) == 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

static osal_s32 wal_cfg80211_sched_scan_do_start(oal_net_device_stru *netdev,
    oal_cfg80211_sched_scan_request_stru *request,
    hmac_scan_stru *scan_mgmt)
{
    mac_pno_scan_stru pno_scan_info;
    oal_cfg80211_ssid_stru *scan_ssid_list = OAL_PTR_NULL;
    oal_cfg80211_ssid_stru *ssid_tmp = OAL_PTR_NULL;
    osal_s32 ret;
    osal_s32 l_loop;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_cfg80211_sched_scan_do_start:dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    /* 初始化pno扫描的结构体信息 */
    memset_s(&pno_scan_info, OAL_SIZEOF(pno_scan_info), 0, OAL_SIZEOF(pno_scan_info));

    /* 将内核下发的匹配的ssid集合复制到本地 */
    scan_ssid_list = OAL_PTR_NULL;
    if (request->n_ssids > 0) {
        scan_ssid_list = request->ssids;
    }

    for (l_loop = 0; l_loop < OAL_MIN(request->n_match_sets, MAX_PNO_SSID_COUNT); l_loop++) {
        ssid_tmp = &(request->match_sets[l_loop].ssid);
        if (ssid_tmp->ssid_len >= WLAN_SSID_MAX_LEN) {
            oam_warning_log1(0, OAM_SF_SCAN, "{uapi_cfg80211_sched_scan_start:ssid_len [%d]! err}", ssid_tmp->ssid_len);
            continue;
        }
        if (memcpy_s(pno_scan_info.match_ssid_set[l_loop].ssid, sizeof(pno_scan_info.match_ssid_set[l_loop].ssid),
            ssid_tmp->ssid, ssid_tmp->ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_do_start::memcpy_s error}");
        }
        pno_scan_info.match_ssid_set[l_loop].ssid[ssid_tmp->ssid_len] = '\0';
        pno_scan_info.match_ssid_set[l_loop].scan_ssid =
            wal_pno_scan_with_assigned_ssid(ssid_tmp, scan_ssid_list, request->n_ssids);
        pno_scan_info.ssid_count++;
    }

    /* 其它参数赋值 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    pno_scan_info.rssi_thold = request->min_rssi_thold;
#else
    pno_scan_info.rssi_thold = request->rssi_thold;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    pno_scan_info.pno_scan_interval = PNO_SCHED_SCAN_INTERVAL;        /* 驱动自己定义为30s */
    pno_scan_info.pno_scan_repeat = MAX_PNO_REPEAT_TIMES;

    /* 保存当前的PNO调度扫描请求指针 */
    scan_mgmt->sched_scan_req = request;
    scan_mgmt->sched_scan_complete = OAL_FALSE;

    /* 维测打印上层下发的调度扫描请求参数信息 */
    wal_cfg80211_print_sched_scan_req_info(request);

    /* 下发pno扫描请求到hmac */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_START_SCHED_SCAN,
        (osal_u8 *)&pno_scan_info, OAL_SIZEOF(mac_pno_scan_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "{uapi_cfg80211_sched_scan_start::wal_sync_post2hmac_no_rsp fail[%d]!}", ret);
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 调度扫描启动
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_sched_scan_start(oal_wiphy_stru *wiphy,
    oal_net_device_stru *netdev, oal_cfg80211_sched_scan_request_stru *request)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_sched_scan_start::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 参数合法性检查 */
    if ((wiphy == OAL_PTR_NULL) || (netdev == OAL_PTR_NULL) || (request == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_CFG,
            "{uapi_cfg80211_sched_scan_start::null ptr, wiphy[%p], netdev[%p], request[%p]!}",
            wiphy, netdev, request);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    hmac_vap    = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_sched_scan_start:: hmac_vap is null!}");
        return -OAL_EINVAL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_PTR_NULL == hmac_device) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_sched_scan_start:: hmac_device is null!}");
        return -OAL_EINVAL;
    }

    scan_mgmt = &(hmac_device->scan_mgmt);
    /* 如果当前设备处于扫描状态，abort当前扫描 */
    if (scan_mgmt->request != OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_sched_scan_start:: device is busy, stop current scan!}");

        wal_force_scan_complete_etc(netdev, OAL_TRUE);
    }

    /* 检测内核下发的需要匹配的ssid集合的个数是否合法 */
    if (request->n_match_sets <= 0) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {uapi_cfg80211_sched_scan_start::match_sets = %d!}",
            hmac_vap->vap_id, request->n_match_sets);
        return -OAL_EINVAL;
    }

    ret = wal_cfg80211_sched_scan_do_start(netdev, request, scan_mgmt);
    return (ret > 0) ? -OAL_EFAIL : ret;
}

/*****************************************************************************
 功能描述  : 调度扫描关闭
 输入参数  : oal_wiphy_stru                       *wiphy
             oal_net_device_stru                  *netdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2015年6月9日
    修改内容   : 新生成函数

*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
OAL_STATIC osal_s32 uapi_cfg80211_sched_scan_stop(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    osal_u64 reqid)
#else
OAL_STATIC osal_s32 uapi_cfg80211_sched_scan_stop(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev)
#endif
{
    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_sched_scan_stop::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == netdev)) {
        oam_error_log2(0, OAM_SF_CFG,
            "{uapi_cfg80211_sched_scan_stop::input param pointer is null, wiphy[%p], netdev[%p]!}",
            wiphy, netdev);
        return -OAL_EINVAL;
    }

    ret = wal_stop_sched_scan_etc(netdev);
    return (ret > 0) ? -OAL_EFAIL : ret;
}

#endif
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
static osal_s32 wal_cfg80211_change_virtual_intf_check(oal_wiphy_stru *wiphy,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
                                                        oal_net_device_stru *net_dev,
#else
                                                        osal_s32 l_ifindex, oal_net_device_stru **net_dev,
#endif
                                                        oal_vif_params_stru *params, enum nl80211_iftype type)
{
    /* 1.1 入参检查 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    if (net_dev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_check::dev is null!}");
        return -OAL_EINVAL;
    }
#else
    oal_net_device_stru *net_dev = oal_dev_get_by_index(l_ifindex);
    if (net_dev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_check::dev is null!}");
        return -OAL_EINVAL;
    }
    *net_dev = net_dev;
    oal_dev_put(net_dev); /* 调用oal_dev_get_by_index后，必须调用oal_dev_put使net_dev的引用计数减一 */
#endif
    if ((wiphy == OAL_PTR_NULL) || (params == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG,
            "{wal_cfg80211_change_virtual_intf_check::wiphy or pul_flag or params ptr null,error %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)params);
        return -OAL_EINVAL;
    }

    /* 检查VAP 当前模式和目的模式是否相同，如果相同则直接返回 */
    if (net_dev->ieee80211_ptr->iftype == type) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_cfg80211_change_virtual_intf_check::same iftype[%d],do not need change !}", type);
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{wal_cfg80211_change_virtual_intf_check::[%d][%d]}",
                     (net_dev->ieee80211_ptr->iftype), type);
    return OAL_CONTINUE;
}

static osal_s32 wal_cfg80211_get_vap_mode_by_iftype(oal_net_device_stru *net_dev,
    enum nl80211_iftype type, wlan_p2p_mode_enum_uint8 *p2p_mode, wlan_vap_mode_enum_uint8 *vap_mode)
{
    switch (type) {
        case NL80211_IFTYPE_MONITOR:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_ADHOC:
            oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_get_vap_mode_by_iftype:currently not support type[%d]}", type);
            return -OAL_EINVAL;

        case NL80211_IFTYPE_STATION:
#if defined(_PRE_PRODUCT_ID_HOST)
            if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
                /* 下电APUT，切换netdev状态为station */
                return wal_netdev_stop_ap_etc(net_dev);
            }
#endif
            if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) ||
                (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT)) {
                *vap_mode = WLAN_VAP_MODE_BSS_STA;
                *p2p_mode = WLAN_P2P_DEV_MODE;
                break;
            }
            net_dev->ieee80211_ptr->iftype = type;
            oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_change_virtual_intf_etc::change to station}");
            return OAL_SUCC;
        case NL80211_IFTYPE_P2P_CLIENT:
            if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) {
                oam_warning_log1(0, 0, "wal_cfg80211_get_vap_mode_by_iftype:not support this change[%d]", type);
                return -OAL_EINVAL;
            }
            *vap_mode = WLAN_VAP_MODE_BSS_STA;
            *p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_AP_VLAN:
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT) {
                oam_warning_log1(0, 0, "wal_cfg80211_get_vap_mode_by_iftype:not support this change[%d]", type);
                return -OAL_EINVAL;
            }
            *vap_mode = WLAN_VAP_MODE_BSS_AP;
            *p2p_mode = WLAN_P2P_GO_MODE;
            break;
        default:
            oam_error_log1(0, 0, "{wal_cfg80211_get_vap_mode_by_iftype::currently do not support this type[%d]}", type);
            return -OAL_EINVAL;
    }
    return OAL_CONTINUE;
}

#endif /* _PRE_WLAN_FEATURE_P2P */

osal_s32 wal_cfg80211_change_virtual_intf(oal_net_device_stru *net_dev, hmac_cfg_del_vap_param_stru *del_vap_param,
    hmac_cfg_add_vap_param_stru *add_vap_param, enum nl80211_iftype type)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    /* 在GC/GO的连接中将p2p_dev_clear置1，避免在删除vap成功后出现没vap存在情况下时清空hmac_device，清除上次的扫描结果 */
    hmac_device->set_mode_iftype = type;
    /* 如果当前模式和目的模式不同，则需要:
        1. 停止 VAP
        2. 删除 VAP
        3. 重新创建对应模式VAP
        4. 启动VAP
    */
    /* 停止VAP */
    wal_netdev_stop_etc(net_dev);
    /* 删除VAP */
    del_vap_param->net_dev = net_dev;
    /* 设备p2p 模式需要从net_device 中获取 */
    del_vap_param->add_vap.p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(net_dev->ieee80211_ptr->iftype);
    if (wal_cfg80211_del_vap_etc(del_vap_param)) {
        return -OAL_EFAIL;
    }

    /* 重新创建对应模式VAP */
    add_vap_param->net_dev = net_dev;

    if (osal_unlikely(wal_cfg80211_add_vap_etc(add_vap_param) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_change_virtual_intf_etc:wal_cfg80211_add_vap_etc return FAIL}");
        return -OAL_EFAIL;
    }

    /* 启动VAP */
    wal_netdev_open_etc(net_dev, OAL_FALSE);
#endif // end of #ifdef _PRE_WLAN_FEATURE_P2P
    return OAL_CONTINUE;
}

/*****************************************************************************
 转换AP，STA 状态
*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
osal_s32 uapi_cfg80211_change_virtual_intf_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *net_dev,
    enum nl80211_iftype type, osal_u32 *pul_flags, oal_vif_params_stru *params)
#else
osal_s32 uapi_cfg80211_change_virtual_intf_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *net_dev,
    enum nl80211_iftype type, oal_vif_params_stru *params)
#endif
#else
osal_s32 uapi_cfg80211_change_virtual_intf_etc(oal_wiphy_stru *wiphy, osal_s32 l_ifindex,
    enum nl80211_iftype type, osal_u32 *pul_flags, oal_vif_params_stru *params)
#endif
{
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_P2P_BUTT;
    wlan_vap_mode_enum_uint8 vap_mode = WLAN_VAP_MODE_BUTT;
    hmac_cfg_del_vap_param_stru del_vap_param = {0};
    hmac_cfg_add_vap_param_stru add_vap_param = {0};
    osal_s32 ret;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* 如果是DFR流程中, 仅在P2P恢复时允许change */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_P2P_RECOVERY)) {
        /* 记录下来 */
        wal_dfr_save_p2p_change_record((hmac_vap_stru *)net_dev->ml_priv);
        return -OAL_EINVAL;
    }
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34))
    oal_net_device_stru *net_dev = OAL_PTR_NULL;
    ret = wal_cfg80211_change_virtual_intf_check(wiphy, l_ifindex, &net_dev, params, type);
#else
    ret = wal_cfg80211_change_virtual_intf_check(wiphy, net_dev, params, type);
#endif
    if (ret != OAL_CONTINUE) {
        return (ret > 0) ? -OAL_EFAIL : ret;
    }

    ret = wal_cfg80211_get_vap_mode_by_iftype(net_dev, type, &p2p_mode, &vap_mode);
    if ((ret != OAL_CONTINUE)) {
        return (ret > 0) ? -OAL_EFAIL : ret;
    }

#if defined(_PRE_PRODUCT_ID_HOST)
    if (type == NL80211_IFTYPE_AP) {
        oam_warning_log0(0, OAM_SF_CFG, "uapi_cfg80211_change_virtual_intf_etc, change to ap.");
        ret = wal_setup_ap_etc(net_dev);
        return (ret > 0) ? -OAL_EFAIL : ret;
    }
#endif

    /* 设备为P2P 设备才需要进行change virtual interface */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_change_virtual_intf_etc::dev->ml_priv fail!}");
        return -OAL_EFAIL;
    }

    add_vap_param.add_vap.vap_mode = vap_mode;
    add_vap_param.add_vap.p2p_mode = p2p_mode;
    ret = wal_cfg80211_change_virtual_intf(net_dev, &del_vap_param, &add_vap_param, type);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
    oam_warning_log3(0, OAM_SF_CFG, "uapi_cfg80211_change_virtual_intf_etc, vap_mode[%d], p2p_mode[%d] iftype[%d].",
        vap_mode, p2p_mode, type);
#endif /* _PRE_WLAN_FEATURE_P2P */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    net_dev->ieee80211_ptr->iftype = type;
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 增加用户
 输入参数  : oal_wiphy_stru *wiphy
             oal_net_device *dev
             osal_u8 *mac         用户mac 地址
             oal_station_parameters_stru *sta_parms 用户参数
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2013年11月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_add_station(oal_wiphy_stru *wiphy, oal_net_device_stru *dev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    const
#endif
    osal_u8 *mac, oal_station_parameters_stru *sta_parms)
{
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除用户
 输入参数  : oal_wiphy_stru *wiphy
             oal_net_device *dev
             osal_u8 *mac         用户mac 地址。如果mac = NULL,删除所有用户
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_del_station(oal_wiphy_stru        *wiphy,
    oal_net_device_stru *dev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
        struct station_del_parameters *params
#else
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
        const
#endif
        osal_u8           *mac
#endif
)
{
    hmac_vap_stru                  *hmac_vap        = OAL_PTR_NULL;
    mac_cfg_kick_user_param_stru  kick_user_param;
    osal_s32                     int_user_count_ok = 0;
    osal_s32                     uint_ret            = OAL_FAIL;
    osal_u8                     mac_boardcast[OAL_MAC_ADDR_LEN];
    osal_u16                    reason_code = MAC_INACTIVITY;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    osal_u8                    *mac;
    if (params == OAL_PTR_NULL) {
        return -OAL_EFAUL;
    }
    mac = (osal_u8 *)params->mac;

    reason_code = params->reason_code;
#endif

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* DFR流程 AP恢复时允许add_key 其余情况返回 */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_STA_AP_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_del_station::dfr ing... status[%d]", wal_dfr_get_status());
        return OAL_SUCC;
    }
#endif
    if (OAL_PTR_NULL == wiphy || OAL_PTR_NULL == dev) {
        return -OAL_EFAUL;
    }

    hmac_vap = dev->ml_priv;
    if (OAL_PTR_NULL == hmac_vap) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_station::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAUL;
    }

    /* 判断是否是AP模式 */
    if (WLAN_VAP_MODE_BSS_AP != hmac_vap->vap_mode) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_del_station::WLAN_VAP_MODE_BSS_AP != vap_mode[%d]!}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        return -OAL_EINVAL;
    }

    if (OAL_PTR_NULL == mac) {
        (osal_void)memset_s(mac_boardcast, sizeof(mac_boardcast), 0xff, sizeof(mac_boardcast));
        mac = mac_boardcast;
        oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_del_station::delete all user}", hmac_vap->vap_id);
    } else {
        oam_info_log4(0, OAM_SF_ANY, "{uapi_cfg80211_del_station::deleting user:%02X:%02X:%02X:%02X:XX:XX!}",
            /* 0:1:2:3:数组下标 */
            mac[0], mac[1], mac[2], mac[3]);
    }

    kick_user_param.reason_code = reason_code;
    if (memcpy_s(kick_user_param.mac_addr, sizeof(kick_user_param.mac_addr), mac, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_station::memcpy_s error}");
    }
    uint_ret = wal_cfg80211_start_disconnect_etc(dev, &kick_user_param);
    if (OAL_SUCC != uint_ret) {
        /* 由于删除的时候可能用户已经删除，此时再进行用户查找，会返回错误，输出ERROR打印，修改为warning */
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d] {uapi_cfg80211_del_station::fail[%d]}", hmac_vap->vap_id, uint_ret);
        return -OAL_EINVAL;
    } else {
        int_user_count_ok++;
    }

    oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_del_station::%d user is deleted!}",
        hmac_vap->vap_id, int_user_count_ok);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除用户
 输入参数  : oal_wiphy_stru *wiphy
             oal_net_device *dev
             osal_u8 *mac         用户mac 地址
             oal_station_parameters_stru *sta_parms 用户参数
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2013年11月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_change_station(oal_wiphy_stru                 *wiphy,
    oal_net_device_stru         *dev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    const
#endif
    osal_u8 *mac, oal_station_parameters_stru *sta_parms)
{
    return OAL_SUCC;
}

OAL_STATIC osal_void wal_cfg80211_fill_station_stats(oal_station_info_stru *sta_info,
    mac_station_info_stru *stats)
{
    sta_info->signal = stats->signal;
    sta_info->rx_packets = stats->rx_packets;
    sta_info->tx_packets = stats->tx_packets;
    sta_info->rx_bytes  = stats->rx_bytes;
    sta_info->tx_bytes  = stats->tx_bytes;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    /* 适配linux 4.0.修改为BIT(NL80211_STA_INFO_SIGNAL)等. */
    sta_info->tx_retries       = stats->tx_retries;
    sta_info->tx_failed        = stats->tx_failed;
    sta_info->rx_dropped_misc  = stats->rx_dropped_misc;

    /* 11ac模式下 由于私有速率的存在内核无法识别 修改为通过legacy上报 更改相应的标志位 */
    if (stats->txrate.flags & RATE_INFO_FLAGS_VHT_MCS) {
        /* dmac已将单位修改为100kbps */
        sta_info->txrate.legacy = stats->txrate.legacy;
        /* 清除标志位RATE_INFO_FLAGS_VHT_MCS RATE_INFO_FLAGS_MCS */
        sta_info->txrate.flags  = stats->txrate.flags & 0xFC; // 0xFC 标志位
    } else {
        sta_info->txrate.legacy = (osal_u16)(stats->txrate.legacy); /* 内核中单位为100kbps, 驱动上报的也是, 不用*10 */
        sta_info->txrate.flags  = stats->txrate.flags; /* 目前各协议下都是获取用户最佳速率, 统一使用legacy, flags是 0 */
    }
    sta_info->txrate.mcs    = stats->txrate.mcs;
    sta_info->txrate.nss    = stats->txrate.nss;
    sta_info->txrate.bw     = stats->txrate.bw;

#else
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    sta_info->tx_retries       = stats->tx_retries;
    sta_info->tx_failed        = stats->tx_failed;
    sta_info->rx_dropped_misc  = stats->rx_dropped_misc;
#endif

    sta_info->txrate.legacy = (osal_u16)(stats->txrate.legacy * 10); /* 内核中单位为100kbps */
    sta_info->txrate.flags  = stats->txrate.flags;
    sta_info->txrate.mcs    = stats->txrate.mcs;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    sta_info->txrate.nss    = stats->txrate.nss;
#endif

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
}

OAL_STATIC osal_void wal_cfg80211_fill_flag_stats(oal_station_info_stru *sta_info,
    mac_station_info_stru *stats)
{
    osal_u8 flag = 0;
    /*
     * 速率flag因内核版本而异，而DMAC依3.8.0为准给出全部结果，在此提取并转换flags
     * linux < 3.5.0
     *     RATE_INFO_FLAGS_MCS             = 1<<0,
     *     RATE_INFO_FLAGS_40_MHZ_WIDTH    = 1<<1,
     *     RATE_INFO_FLAGS_SHORT_GI        = 1<<2,
     *
     * linux >= 3.5.0 && linux < 3.8.0
     *    RATE_INFO_FLAGS_MCS             = 1<<0,
     *    RATE_INFO_FLAGS_40_MHZ_WIDTH    = 1<<1,
     *    RATE_INFO_FLAGS_SHORT_GI        = 1<<2,
     *    RATE_INFO_FLAGS_60G             = 1<<3
     *
     * linux >= 3.8.0&& linux < 4.0.0
     *    RATE_INFO_FLAGS_MCS             = BIT(0),
     *    RATE_INFO_FLAGS_VHT_MCS         = BIT(1),
     *    RATE_INFO_FLAGS_40_MHZ_WIDTH    = BIT(2),
     *    RATE_INFO_FLAGS_80_MHZ_WIDTH    = BIT(3),
     *    RATE_INFO_FLAGS_80P80_MHZ_WIDTH = BIT(4),
     *    RATE_INFO_FLAGS_160_MHZ_WIDTH   = BIT(5),
     *    RATE_INFO_FLAGS_SHORT_GI        = BIT(6),
     *    RATE_INFO_FLAGS_60G             = BIT(7),
     *
     * linux >= 4.0.0 && linux < 4.19.0
     *    RATE_INFO_FLAGS_MCS           = BIT(0),
     *    RATE_INFO_FLAGS_VHT_MCS       = BIT(1),
     *    RATE_INFO_FLAGS_SHORT_GI      = BIT(2),
     *    RATE_INFO_FLAGS_60G           = BIT(3),
     *
     * linux >= 4.19.0 && liunx < 5.4.0
     *    RATE_INFO_FLAGS_MCS			= BIT(0),
     *    RATE_INFO_FLAGS_VHT_MCS		= BIT(1),
	 *    RATE_INFO_FLAGS_SHORT_GI		= BIT(2),
	 *    RATE_INFO_FLAGS_60G			= BIT(3),
	 *    RATE_INFO_FLAGS_HE_MCS		= BIT(4),
     *
     * linux >= 5.4.0 && linux < 5.12.0
     *    RATE_INFO_FLAGS_MCS			= BIT(0),
	 *    RATE_INFO_FLAGS_VHT_MCS		= BIT(1),
	 *    RATE_INFO_FLAGS_SHORT_GI		= BIT(2),
	 *    RATE_INFO_FLAGS_DMG			= BIT(3),
	 *    RATE_INFO_FLAGS_HE_MCS		= BIT(4),
	 *    RATE_INFO_FLAGS_EDMG			= BIT(5),
     *
     * linux >= 5.12.0
     *    RATE_INFO_FLAGS_MCS			= BIT(0),
	 *    RATE_INFO_FLAGS_VHT_MCS		= BIT(1),
	 *    RATE_INFO_FLAGS_SHORT_GI		= BIT(2),
	 *    RATE_INFO_FLAGS_DMG			= BIT(3),
	 *    RATE_INFO_FLAGS_HE_MCS		= BIT(4),
	 *    RATE_INFO_FLAGS_EDMG			= BIT(5),
	 *    RATE_INFO_FLAGS_EXTENDED_SC_DMG       = BIT(6),
     *
     * rate_info_bw
     *    RATE_INFO_BW_5
     *    RATE_INFO_BW_10
     *    RATE_INFO_BW_20
     *    RATE_INFO_BW_40
     *    RATE_INFO_BW_80
     *    RATE_INFO_BW_160
     * linux >= 4.19.0
     *    RATE_INFO_BW_HE_RU,
     */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_MCS) ? RATE_INFO_FLAGS_MCS : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH) ? RATE_INFO_FLAGS_40_MHZ_WIDTH : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_SHORT_GI) ? RATE_INFO_FLAGS_SHORT_GI : 0);
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_60G) ? RATE_INFO_FLAGS_60G : 0);
#endif
    /* 11b/g/n 20M /40M场景下,单WiFi冲包，IW读取的TCP上行与UDP上行TX速率速率与实际物理层速率不一致。 */
#elif defined(LINUX_VERSION_CODE) &&  (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    flag  = stats->txrate.flags;
#elif defined(LINUX_VERSION_CODE) &&  (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_MCS) ? RATE_INFO_FLAGS_MCS : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_VHT_MCS) ? RATE_INFO_FLAGS_VHT_MCS : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_SHORT_GI) ? RATE_INFO_FLAGS_SHORT_GI : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_60G) ? RATE_INFO_FLAGS_60G : 0);
#else
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_MCS) ? RATE_INFO_FLAGS_MCS : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_VHT_MCS) ? RATE_INFO_FLAGS_VHT_MCS : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_SHORT_GI) ? RATE_INFO_FLAGS_SHORT_GI : 0);
    flag |= ((stats->txrate.flags & MAC_RATE_INFO_FLAGS_60G) ? RATE_INFO_FLAGS_DMG : 0);
#endif
#endif
#if defined(LINUX_VERSION_CODE) &&  (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    stats->txrate.bw = stats->txrate.flags & MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH ? RATE_INFO_BW_40 :
        (stats->txrate.flags & MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH ? RATE_INFO_BW_80 :
        (stats->txrate.flags & MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH ?  RATE_INFO_BW_160 : RATE_INFO_BW_20));
#endif
    stats->txrate.flags = flag;
    return;
}

/*****************************************************************************
 station_info结构赋值
*****************************************************************************/
OAL_STATIC osal_void wal_cfg80211_fill_station_info(oal_station_info_stru *sta_info,
    mac_station_info_stru *stats)
{
    wal_cfg80211_fill_flag_stats(sta_info, stats);
    wal_cfg80211_fill_station_stats(sta_info, stats);

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    /* 适配linux 4.0.修改为BIT(NL80211_STA_INFO_SIGNAL)等. */
    sta_info->filled |= BIT(NL80211_STA_INFO_SIGNAL);
    sta_info->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);
    sta_info->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
    sta_info->filled |= BIT(NL80211_STA_INFO_RX_DROP_MISC);
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
#else
    sta_info->filled |= STATION_INFO_SIGNAL;
    sta_info->filled |= STATION_INFO_RX_PACKETS;
    sta_info->filled |= STATION_INFO_TX_PACKETS;

    sta_info->filled |= STATION_INFO_RX_BYTES;
    sta_info->filled |= STATION_INFO_TX_BYTES;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    sta_info->filled |= STATION_INFO_TX_RETRIES;
    sta_info->filled |= STATION_INFO_TX_FAILED;
    sta_info->filled |= STATION_INFO_RX_DROP_MISC;
#endif
    sta_info->filled |= STATION_INFO_TX_BITRATE;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
}

/*****************************************************************************
 功能描述  : update rssi once a second
 输出参数  : osal_u8
 调用函数  : uapi_cfg80211_get_station

 修改历史      :
  1.日    期   : 2015年8月20日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u8 wal_cfg80211_get_station_filter_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac)
{
    hmac_user_stru *hmac_user;
    osal_u32      current_time = (osal_u32)oal_time_get_stamp_ms();
    osal_u32      runtime;

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac);
    if (OAL_PTR_NULL == hmac_user) {
        oam_warning_log1(0, OAM_SF_CFG,
            "vap_id[%d] {wal_cfg80211_get_station_filter_etc::user is null.}", hmac_vap->vap_id);
        return OAL_FALSE;
    }

    runtime = (osal_u32)oal_time_get_runtime(hmac_user->rssi_last_timestamp, current_time);
    if (WAL_GET_STATION_THRESHOLD > runtime) {
        return OAL_FALSE;
    }

    hmac_user->rssi_last_timestamp = current_time;
    return OAL_TRUE;
}

OAL_STATIC osal_s32 wal_cfg80211_get_station_event_proc(oal_net_device_stru *dev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    const osal_u8 *mac, osal_u8 vap_id)
#else
    osal_u8 *mac, osal_u8 vap_id)
#endif
{
    osal_s32 ret;
    osal_s32 auc_value[WLAN_MAC_ADDR_LEN] = {0};
    frw_msg  cfg_info;
    hmac_stat_info_rsp_stru sta_info;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    hmac_vap = dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_dual_antenna::dev->ml_priv, return null!}");
        return WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;
    }

    if (memcpy_s(auc_value, sizeof(auc_value), mac, WLAN_MAC_ADDR_LEN) != EOK) {
        return -OAL_EFAIL;
    }

    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&auc_value, OAL_SIZEOF(auc_value), (osal_u8 *)&sta_info, OAL_SIZEOF(sta_info), &cfg_info);

    ret = send_sync_cfg_to_host(hmac_vap->vap_id, WLAN_MSG_W2H_C_CFG_QUERY_STATUS, &cfg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {wal_cfg80211_get_station_event_proc::send event return err code %d!}", vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

static osal_void wal_cfg80211_fill_station_info_ext(oal_station_info_stru *sta_info, hmac_vap_stru *hmac_vap)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
#ifdef KERNEL_4_9_ADAP
    sta_info->filled |= BIT(NL80211_STA_INFO_NOISE);
    sta_info->noise = hmac_vap->station_info.noise;
    sta_info->filled |= BIT(NL80211_STA_INFO_SNR);
    sta_info->snr = OAL_MAX(hmac_vap->station_info_extend.c_snr_ant0,
                            hmac_vap->station_info_extend.c_snr_ant1);
    sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
    sta_info->chload = hmac_vap->station_info.chload;
#endif
#endif
#endif
}

/*****************************************************************************
 功能描述  : 获取station信息
 输入参数  : oal_wiphy_stru        *wiphy,
           oal_net_device_stru   *dev,
           osal_u8             *mac,
           oal_station_info_stru *sta_info
*****************************************************************************/
osal_s32 uapi_cfg80211_get_station(oal_wiphy_stru *wiphy, oal_net_device_stru *dev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    const
#endif
    osal_u8 *mac, oal_station_info_stru *sta_info)
{
    hmac_vap_stru       *hmac_vap = OSAL_NULL;
    osal_s32             ret;
    osal_u8              vap_id;
    mac_station_info_stru info = {0};

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_get_station::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    if ((wiphy == OSAL_NULL) || (dev == OSAL_NULL) || (mac == OSAL_NULL) || (sta_info == OSAL_NULL)) {
        oam_error_log3(0, 0, "uapi_cfg80211_get_station::null ptr, wiphy[0x%p],dev[0x%p],mac[0x%p]!",
            (uintptr_t)wiphy, (uintptr_t)dev, (uintptr_t)mac);
        return -OAL_EINVAL;
    }

    hmac_vap = dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_get_station::dev->ml_priv, return null!}");
        return -OAL_EINVAL;
    }

    if (mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac) == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {uapi_cfg80211_get_station::user is null.}", hmac_vap->vap_id);
        (osal_void)memset_s(&info, sizeof(info), 0, sizeof(info));
        wal_cfg80211_fill_station_info(sta_info, &info);
        return OAL_SUCC;
    }

    vap_id = hmac_vap->vap_id;

    /* 固定时间内最多更新一次RSSI */
    if (wal_cfg80211_get_station_filter_etc(hmac_vap, (osal_u8 *)mac) == OSAL_FALSE) {
        wal_cfg80211_fill_station_info(sta_info, &hmac_vap->station_info);
        return OAL_SUCC;
    }
    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    ret = wal_cfg80211_get_station_event_proc(dev, mac, vap_id);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d]{uapi_cfg80211_get_station: error[0x%x]!}", vap_id, (osal_u32)ret);
        return (ret > 0) ? -OAL_EFAIL : ret;
    }

    /* 正常结束  */
    wal_cfg80211_fill_station_info(sta_info, &hmac_vap->station_info);
    wal_cfg80211_fill_station_info_ext(sta_info, hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除用户
 输入参数  : oal_wiphy_stru *wiphy
             oal_net_device *dev
             osal_u8 *mac         用户mac 地址
             oal_station_parameters_stru *sta_parms 用户参数
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2013年11月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_dump_station(oal_wiphy_stru *wiphy, oal_net_device_stru *dev,
    osal_s32 int_index, osal_u8 *mac, oal_station_info_stru *sta_info)
{
    return OAL_SUCC;
}

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述  : 删除P2P group
 输入参数  : hmac_device_stru *hmac_device
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_void

 修改历史      :
  1.日    期   : 2015年9月28日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_del_p2p_group_etc(hmac_device_stru *hmac_device)
{
    osal_u8                    vap_idx;
    hmac_vap_stru               *hmac_vap;
    oal_net_device_stru         *net_dev;
    hmac_cfg_del_vap_param_stru   del_vap_param;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == hmac_vap)) {
            oam_warning_log1(0, OAM_SF_P2P, "{wal_del_p2p_group_etc::get hmac vap resource fail! vap id is %d}",
                             hmac_device->vap_id[vap_idx]);
            continue;
        }

        net_dev = hmac_vap->net_device;
        if (OAL_UNLIKELY(OAL_PTR_NULL == net_dev)) {
            oam_warning_log1(0, OAM_SF_P2P, "{wal_del_p2p_group_etc::get net device fail! vap id is %d}",
                             hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (is_p2p_go(hmac_vap) || is_p2p_cl(hmac_vap)) {
            memset_s(&del_vap_param, OAL_SIZEOF(del_vap_param), 0, OAL_SIZEOF(del_vap_param));
            del_vap_param.net_dev = net_dev;
            del_vap_param.add_vap.vap_mode = hmac_vap->vap_mode;
            del_vap_param.add_vap.p2p_mode = hmac_get_p2p_mode_etc(hmac_vap);
            oam_warning_log3(0, OAM_SF_P2P,
                "vap_id[%d] {wal_del_p2p_group_etc:: vap mode[%d], p2p mode[%d]}",
                hmac_vap->vap_id, del_vap_param.add_vap.vap_mode, del_vap_param.add_vap.p2p_mode);
            /* 删除已经存在的P2P group */
            wal_force_scan_complete_etc(net_dev, OAL_TRUE);
            wal_stop_vap_etc(net_dev);
            if (OAL_SUCC == wal_cfg80211_del_vap_etc(&del_vap_param)) {
                wal_cfg80211_unregister_netdev_etc(net_dev);
            }
        }
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 内核注册指定类型的net_device,用于需要解mutex lock的应用
 输入参数  : hmac_device_stru *hmac_device
             oal_net_device_stru *net_dev
 输出参数  : 无
 返 回 值  : 无

 修改历史      :
  1.日    期   : 2015年7月24日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_cfg80211_register_netdev_etc(oal_net_device_stru *net_dev)
{
    osal_u8    rollback_lock = OAL_FALSE;
    osal_u32   ul_ret;

    /* nl80211 netlink pre diot 中会获取rntl_lock互斥锁.
       注册net_device 会获取rntl_lock互斥锁，造成了死锁 */
    if (rtnl_is_locked()) {
        rtnl_unlock();
        rollback_lock = OAL_TRUE;
    }

    /* 内核注册net_device, 只返回0 */
    ul_ret = (osal_u32)oal_net_register_netdev(net_dev);

    if (rollback_lock) {
        rtnl_lock();
    }

    return ul_ret;
}

/*****************************************************************************
 功能描述  : 内核去注册指定类型的net_device,用于需要解mutex lock的应用
 输入参数  : hmac_device_stru *hmac_device
             oal_net_device_stru *net_dev
 输出参数  : 无
 返 回 值  : 无

 修改历史      :
  1.日    期   : 2015年7月24日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void wal_cfg80211_unregister_netdev_etc(oal_net_device_stru *net_dev)
{
    osal_u8    rollback_lock = OAL_FALSE;

    if (rtnl_is_locked()) {
        rtnl_unlock();
        rollback_lock = OAL_TRUE;
    }

    /* 去注册netdev */
    oal_net_unregister_netdev(net_dev);

    if (rollback_lock) {
        rtnl_lock();
    }
}
#ifdef _PRE_WLAN_FEATURE_P2P
osal_s32 wal_cfg80211_add_virtual_wait_condition(const void *param)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)param;
    return (hmac_get_p2p_status_etc(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_FALSE);
}
#endif
/*****************************************************************************
 功能描述  : 添加指定类型的net_device
 输入参数  : oal_wiphy_stru      *wiphy
             OAL_CONST osal_u8 *name
             enum nl80211_iftype  type
             osal_u32          *pul_flags
             oal_vif_params_stru *params
 输出参数  : 无
 返 回 值  : OAL_STATIC oal_wireless_dev_stru *

 修改历史      :
  1.日    期   : 2014年11月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_wireless_dev_stru *uapi_cfg80211_add_virtual_intf(oal_wiphy_stru     *wiphy,
    const char          *name,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    unsigned char name_assign_type,
#endif
    enum nl80211_iftype type,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    osal_u32 *pul_flags,
#endif
    oal_vif_params_stru *params)
{
    oal_wireless_dev_stru      *wdev    = OAL_PTR_NULL;
    wlan_p2p_mode_enum_uint8    p2p_mode;
    wlan_vap_mode_enum_uint8    vap_mode;
    oal_net_device_stru        *net_dev;
    mac_wiphy_priv_stru        *wiphy_priv;
    hmac_device_stru           *hmac_device;
    osal_u8                   cfg_vap_id;
    hmac_vap_stru              *cfg_hmac_vap;
    hmac_vap_stru              *hmac_vap;
    oal_net_device_stru        *cfg_net_dev;
    osal_u32                  ul_ret;
    osal_s32                   l_ret;
    osal_s8                   ac_name[OAL_IF_NAME_SIZE] = {0};
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u8                   rollback_lock = 0;
    osal_s32                   l_timeout;
#endif
    hmac_cfg_add_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_add_virtual_intf::dfr ing... status[%d]", wal_dfr_get_status());
        return ERR_PTR(-EINVAL);
    }
#endif
    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == name) || (OAL_PTR_NULL == params)) {
        oam_error_log3(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf:: ptr null, wiphy %p, name %p, params %p!}",
            (uintptr_t)wiphy, (uintptr_t)name, (uintptr_t)params);
        return ERR_PTR(-EINVAL);
    }

    /* 入参检查无异常后赋值，并调用OAL统一接口 */
    wiphy_priv = oal_wiphy_priv(wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::wiphy_priv is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }
    hmac_device = wiphy_priv->hmac_device;
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::hmac_device is null!}\r\n");
        return ERR_PTR(-EINVAL);
    }

    oam_warning_log1(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::type[%d]!}\r\n", type);

    switch (type) {
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_MESH_POINT:
        case NL80211_IFTYPE_MONITOR:
            oam_error_log1(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}", type);
            return ERR_PTR(-EINVAL);
#ifdef _PRE_WLAN_FEATURE_P2P
        case NL80211_IFTYPE_P2P_DEVICE:
            net_dev       = hmac_device->st_p2p_info.p2p_net_device;
            wdev          = net_dev->ieee80211_ptr;
            return wdev;
#endif
        case NL80211_IFTYPE_P2P_CLIENT:
            vap_mode = WLAN_VAP_MODE_BSS_STA;
            p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_STATION:
            vap_mode = WLAN_VAP_MODE_BSS_STA;
            p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            vap_mode = WLAN_VAP_MODE_BSS_AP;
            p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_AP:
            vap_mode = WLAN_VAP_MODE_BSS_AP;
            p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::Unsupported interface type[%d]!}", type);
            return ERR_PTR(-EINVAL);
    }
    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = oal_dev_get_by_name((const osal_s8 *)name);
    if (OAL_PTR_NULL != net_dev) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);

        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::the net_device is already exist!}");
        wdev = net_dev->ieee80211_ptr;
        return wdev;
    }

    /* 添加net_device 前先判断当前是否正在删除net_device 状态，
        如果正在删除net_device，则等待删除完成，再添加 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_get_p2p_status_etc(hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING) == OAL_TRUE) {
        /* 等待删除完成 */
        if (rtnl_is_locked()) {
            rtnl_unlock();
            rollback_lock = OAL_TRUE;
        }
        oam_warning_log0(0, OAM_SF_ANY,
            "{uapi_cfg80211_add_virtual_intf:Released the lock and wait till IF_DEL is complete!}");

        l_timeout = osal_wait_timeout_interruptible(&hmac_device->netif_change_event,
            wal_cfg80211_add_virtual_wait_condition, hmac_device, WAL_MAX_WAIT_TIME);
        /* put back the rtnl_lock again */
        if (rollback_lock) {
            rtnl_lock();
        }

        if (l_timeout > 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::IF DEL is Success!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::timeount < 0, return -EAGAIN!}");
            return ERR_PTR(-EAGAIN);
        }
    }

    /* 检查wifi 驱动中，P2P group 是否已经创建，如果P2P group 已经创建，
        则将该P2P group 删除，并且重新创建P2P group */
    if ((hmac_device->st_p2p_info.p2p_mode == WLAN_P2P_GO_MODE) ||
        (hmac_device->st_p2p_info.p2p_mode == WLAN_P2P_CL_MODE)) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::found exist p2p group, delet it first!}");
        if (OAL_SUCC != wal_del_p2p_group_etc(hmac_device)) {
            return ERR_PTR(-EAGAIN);
        }
    }
#endif

    /* 获取配置VAP 结构 */
    cfg_vap_id   = hmac_device->cfg_vap_id;
    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(cfg_vap_id);
    if (OAL_PTR_NULL == cfg_hmac_vap) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_add_virtual_intf::cfg_hmac_vap is null vap_id:%d!}", cfg_vap_id);
        return ERR_PTR(-ENODEV);
    }
    cfg_net_dev = cfg_hmac_vap->net_device;
    if (memcpy_s(ac_name, sizeof(ac_name), name, osal_strlen(name)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::memcpy_s error}");
    }
#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(0, ac_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
#else
    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev(0, ac_name, oal_ether_setup);
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == net_dev)) {
        oam_error_log1(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_add_virtual_intf::net_dev null ptr error!}", cfg_hmac_vap->vap_id);
        return ERR_PTR(-ENOMEM);
    }

    wdev =
        (oal_wireless_dev_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == wdev)) {
        oam_error_log1(0, OAM_SF_ANY,
            "vap_id[%d] {uapi_cfg80211_add_virtual_intf::alloc mem, wdev is null ptr}", cfg_hmac_vap->vap_id);
        /* 异常处理，释放内存 */
        oal_net_free_netdev(net_dev);
        return ERR_PTR(-ENOMEM);
    }

    (osal_void)memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));

    /* 对netdevice进行赋值 */
    /* 对新创建的net_device 初始化对应参数 */
#if defined (_PRE_WLAN_WIRELESS_EXT) && defined(CONFIG_WIRELESS_EXT)
    net_dev->wireless_handlers             = &g_st_iw_handler_def_etc;
#endif
    net_dev->netdev_ops                    = &g_st_wal_net_dev_ops_etc;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    net_dev->ethtool_ops                   = &g_st_wal_ethtool_ops_etc;
#endif
    OAL_NETDEVICE_DESTRUCTOR(net_dev)      = oal_net_free_netdev;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
    oal_netdevice_master(net_dev)          = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(net_dev)         = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(net_dev)  = 5;
    OAL_NETDEVICE_WDEV(net_dev)            = wdev;
    oal_netdevice_qdisc(net_dev, OAL_PTR_NULL);

    wdev->iftype = type;
    wdev->wiphy  = hmac_device->wiphy;
    wdev->netdev = net_dev; /* 给wdev 中的net_device 赋值 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((WLAN_LEGACY_VAP_MODE == p2p_mode) && (WLAN_VAP_MODE_BSS_STA == vap_mode)) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        if (hmac_device->st_p2p_info.primary_net_device == OAL_PTR_NULL) {
            /* 创建wlan0 在加载wifi 驱动时，不需要通过该接口 */
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_add_virtual_intf::should not go here!}",
                cfg_hmac_vap->vap_id);
            oal_mem_free(wdev, OAL_FALSE);
            oal_net_free_netdev(net_dev);
            return ERR_PTR(-ENODEV);
        }
    }
#endif
    OAL_NETDEVICE_FLAGS(net_dev) &= ~OAL_IFF_RUNNING;   /* 将net device的flag设为down */

    ul_ret = wal_cfg80211_register_netdev_etc(net_dev);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret)) {
        /* 注册不成功，释放资源 */
        oal_mem_free(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return ERR_PTR(-EBUSY);
    }

    l_ret = wal_add_vap(net_dev, cfg_vap_id, vap_mode, p2p_mode);
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::wal_add_vap return err code: [%d]!}", l_ret);
        goto ERR_STEP;
    }

#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) \
    && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    wal_set_random_mac_to_mib_etc(net_dev); /* set random mac to mib ; for mp12-cb */
#endif

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == hmac_vap)) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_add_virtual_intf::net_dev->ml_priv is null ptr.}");
        goto ERR_STEP;
    }
    /* 设置VAP UP */
    wal_netdev_open_etc(net_dev, OAL_FALSE);

    oam_warning_log2(0, OAM_SF_CFG, "{uapi_cfg80211_add_virtual_intf::succ. type[%d],vap_id[%d]!}",
        type, hmac_vap->vap_id);

    return wdev;

    /* 异常处理 */
ERR_STEP:
    wal_cfg80211_unregister_netdev_etc(net_dev);
    /* 先去注册，后释放 */
    oal_mem_free(wdev, OAL_FALSE);
    return ERR_PTR(-EAGAIN);
}

static osal_s32 wal_cfg80211_del_virtual_intf_event_proc(oal_wireless_dev_stru *wdev,
    oal_net_device_stru *net_dev, hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_s32 l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode;
#endif
    hmac_cfg_del_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    /* 初始化删除vap 参数 */
    vap_info.net_dev = net_dev;
#ifdef _PRE_WLAN_FEATURE_P2P
    wdev = net_dev->ieee80211_ptr;
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY,
            "{uapi_cfg80211_del_virtual_intf::wal_wireless_iftype_to_mac_p2p_mode_etc return BUTT}");
        return -OAL_EINVAL;
    }

    vap_info.add_vap.p2p_mode = hmac_get_p2p_mode_etc(hmac_vap);
#endif

    /* 设置删除net_device 标识 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_add_virtual_intf::hmac_device is null!}", hmac_vap->device_id);
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_set_p2p_status_etc(&hmac_device->p2p_intf_status, P2P_STATUS_IF_DELETING);

    /* 启动linux work 删除net_device */
    hmac_vap->del_net_device = net_dev;
#endif

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_DEL_VAP,
        (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_del_virtual_intf::return err code %d}", l_ret);
        wifi_printf("{uapi_cfg80211_del_virtual_intf::DEL_VAP return err code %d}\r\n", l_ret);
        l_ret = -OAL_EFAIL;
    }

    oam_warning_log1(0, OAM_SF_ANY,
        "{uapi_cfg80211_del_virtual_intf::hmac_device->p2p_intf_status %d, del result: %d}", l_ret);

    return l_ret;
}

/*****************************************************************************
 删除对应VAP
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_del_virtual_intf(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev)
{
    /* 异步去注册net_device */
    oal_net_device_stru *net_dev = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_del_virtual_intf::dfr ing..status[%d]", wal_dfr_get_status());
        return OAL_SUCC;
    }
#endif
    if (OAL_UNLIKELY((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == wdev))) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_cfg80211_del_virtual_intf::wiphy or wdev null ptr error %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)wdev);
        return -OAL_EINVAL;
    }

    net_dev = wdev->netdev;
    if (net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_virtual_intf::net_dev is null by netdev!}");
        return -OAL_EINVAL;
    }

    hmac_vap  = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_PTR_NULL == hmac_vap) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_del_virtual_intf::mac_vap is null, mode[%d]}", net_dev->ieee80211_ptr->iftype);
        return -OAL_EINVAL;
    }

    oal_net_tx_stop_all_queues(net_dev);
    wal_netdev_stop_etc(net_dev);
    /* 在释放完net_device 后释放wireless device */

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_cfg80211_del_virtual_intf_event_proc(wdev, net_dev, hmac_vap);
    return (ret > 0) ? -OAL_EFAIL : ret;
}
#else  /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44) */
/*****************************************************************************
 功能描述  : 用于LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44)
 输入参数  : hmac_device_stru *hmac_device
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_void

 修改历史      :
  1.日    期   : 2015年10月21日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_del_p2p_group_etc(hmac_device_stru *hmac_device)
{
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 删除cookie 列表中超时的cookie
 输入参数  : cookie_arry_stru *cookie_array
             osal_u32 current_time
 输出参数  : 无
 返 回 值  : osal_void

 修改历史      :
  1.日    期   : 2015年1月6日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void wal_check_cookie_timeout(cookie_arry_stru  *cookie_array,
    osal_u8       *cookie_bitmap,
    osal_u32       current_time)
{
    osal_u8           loops;
    cookie_arry_stru   *tmp_cookie;

    oam_warning_log0(0, OAM_SF_CFG, "{wal_check_cookie_timeout::time_out!}");
    for (loops = 0; loops < WAL_COOKIE_ARRAY_SIZE; loops++) {
        tmp_cookie = &cookie_array[loops];
        if (oal_time_is_before(tmp_cookie->record_time + oal_msecs_to_jiffies(WAL_MGMT_TX_TIMEOUT_MSEC))) {
            /* cookie array 中保存的cookie 值超时 */
            /* 清空cookie array 中超时的cookie */
            tmp_cookie->record_time = 0;
            tmp_cookie->ull_cookie     = 0;
            /* 清除占用的cookie bitmap位 */
            oal_bit_clear_bit_one_byte(cookie_bitmap, loops);
        }
    }
}

/*****************************************************************************
 功能描述  : 删除指定idx 的cookie
 输入参数  : cookie_arry_stru   *cookie_array
             osal_u8       *cookie_bitmap
             osal_u8        cookie_idx
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年1月6日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_del_cookie_from_array(cookie_arry_stru   *cookie_array,
    osal_u8       *cookie_bitmap,
    osal_u8        cookie_idx)
{
    cookie_arry_stru   *tmp_cookie;

    /* 清除对应cookie bitmap 位 */
    oal_bit_clear_bit_one_byte(cookie_bitmap, cookie_idx);

    /* 清空cookie array 中超时的cookie */
    tmp_cookie = &cookie_array[cookie_idx];
    tmp_cookie->ull_cookie     = 0;
    tmp_cookie->record_time = 0;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 添加cookie 到cookie array 中
 输入参数  : cookie_arry_stru *cookie_array
             osal_u8 *cookie_bitmap
             osal_u64 *pull_cookie
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年1月6日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_add_cookie_to_array(cookie_arry_stru *cookie_array,
    osal_u8       *cookie_bitmap,
    osal_u64      *pull_cookie,
    osal_u8       *cookie_idx)
{
    osal_u8           idx;
    cookie_arry_stru   *tmp_cookie;

    if (*cookie_bitmap == 0xFF) {
        /* cookie array 满，返回错误 */
        oam_warning_log0(0, OAM_SF_CFG, "{wal_add_cookie_to_array::array full!}");
        return OAL_FAIL;
    }

    /* 将cookie 添加到array 中 */
    idx = oal_bit_find_first_zero_one_byte(*cookie_bitmap);
    oal_bit_set_bit_one_byte(cookie_bitmap, idx);

    tmp_cookie = &cookie_array[idx];
    tmp_cookie->ull_cookie      = *pull_cookie;
    tmp_cookie->record_time  = OAL_TIME_JIFFY;

    *cookie_idx = idx;
    return OAL_SUCC;
}

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/*****************************************************************************
 功能描述  : 取消发送管理帧等待
 输入参数  : oal_wiphy_stru        *wiphy
             oal_wireless_dev_stru   *wdev
             osal_u64               ull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年11月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_mgmt_tx_cancel_wait(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, osal_u64 ull_cookie)
{
    return -OAL_EFAIL;
}
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

/*****************************************************************************
 功能描述  : 增加一个pmk缓存
 输入参数  : oal_wiphy_stru           *wiphy
             oal_wireless_dev_stru    *wdev
             struct ieee80211_channel *chan
             osal_u32                duration
             osal_u64               *pull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 修改历史      :
  1.日    期   : 2016年4月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_set_pmksa(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    mac_cfg_pmksa_param_stru                *cfg_pmksa;
    osal_s32                                l_ret;
    mac_cfg_pmksa_param_stru cfg_pmksa_param;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_set_pmksa::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    (osal_void)memset_s(&cfg_pmksa_param, OAL_SIZEOF(cfg_pmksa_param), 0, OAL_SIZEOF(cfg_pmksa_param));
    cfg_pmksa = &cfg_pmksa_param;

    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == net_device) || (OAL_PTR_NULL == pmksa)) {
        oam_error_log3(0, OAM_SF_CFG,
            "{uapi_cfg80211_set_pmksa::param null! wiphy[%p], net_device[%p], pmksa[%p]!!}",
            (uintptr_t)wiphy, (uintptr_t)net_device, (uintptr_t)pmksa);
        return -OAL_EINVAL;
    }

    if ((OAL_PTR_NULL == pmksa->bssid) || (OAL_PTR_NULL == pmksa->pmkid)) {
        oam_error_log2(0, OAM_SF_CFG, "{uapi_cfg80211_set_pmksa::param null! bssid[%p] pmkid[%p]}",
            (uintptr_t)pmksa->bssid, (uintptr_t)pmksa->pmkid);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(cfg_pmksa->bssid, sizeof(cfg_pmksa->bssid), pmksa->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_pmksa::memcpy_s bssid error}");
    }
    if (memcpy_s(cfg_pmksa->pmkid, sizeof(cfg_pmksa->pmkid), pmksa->pmkid, WLAN_PMKID_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_pmksa::memcpy_s pmkid error}");
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_device), WLAN_MSG_W2H_CFG_CFG80211_SET_PMKSA,
        (osal_u8 *)&cfg_pmksa_param, OAL_SIZEOF(cfg_pmksa_param));
    if (OAL_SUCC != l_ret) {
        oam_error_log1(0, OAM_SF_CFG, "{uapi_cfg80211_set_pmksa::wal_sync_post2hmac_no_rsp fail[%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除一个pmk缓存
 输入参数  : oal_wiphy_stru           *wiphy
             oal_wireless_dev_stru    *wdev
             struct ieee80211_channel *chan
             osal_u32                duration
             osal_u64               *pull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 修改历史      :
  1.日    期   : 2016年4月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_del_pmksa(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    mac_cfg_pmksa_param_stru                *cfg_pmksa;
    osal_s32                                l_ret;
    mac_cfg_pmksa_param_stru cfg_pmksa_param;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_del_pmksa::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    (osal_void)memset_s(&cfg_pmksa_param, OAL_SIZEOF(cfg_pmksa_param), 0, OAL_SIZEOF(cfg_pmksa_param));
    cfg_pmksa = &cfg_pmksa_param;

    if ((wiphy == OAL_PTR_NULL) || (net_device == OAL_PTR_NULL) || (pmksa == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_CFG,
            "{uapi_cfg80211_del_pmksa::param null! wiphy[%p], net_device[%p], pmksa[%p]!!}",
            (uintptr_t)wiphy, (uintptr_t)net_device, (uintptr_t)pmksa);
        return -OAL_EINVAL;
    }

    if ((pmksa->bssid == OAL_PTR_NULL) || (pmksa->pmkid == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{uapi_cfg80211_del_pmksa::param null! bssid[%p] pmkid[%p]}",
            (uintptr_t)pmksa->bssid, (uintptr_t)pmksa->pmkid);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(cfg_pmksa->bssid, sizeof(cfg_pmksa->bssid), pmksa->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_pmksa::memcpy_s bssid error}");
    }
    if (memcpy_s(cfg_pmksa->pmkid, sizeof(cfg_pmksa->pmkid), pmksa->pmkid, WLAN_PMKID_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_del_pmksa::memcpy_s pmkid error}");
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_device), WLAN_MSG_W2H_CFG_CFG80211_DEL_PMKSA,
        (osal_u8 *)&cfg_pmksa_param, OAL_SIZEOF(cfg_pmksa_param));
    if (OAL_SUCC != l_ret) {
        oam_error_log1(0, OAM_SF_CFG, "{uapi_cfg80211_del_pmksa::wal_sync_post2hmac_no_rsp fail[%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 清除所有pmk缓存
 输入参数  : oal_wiphy_stru           *wiphy
             oal_wireless_dev_stru    *wdev
             struct ieee80211_channel *chan
             osal_u32                duration
             osal_u64               *pull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 修改历史      :
  1.日    期   : 2016年4月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_flush_pmksa(oal_wiphy_stru *wiphy, oal_net_device_stru *net_device)
{
    osal_s32                       l_ret;
    hmac_vap_stru                   *hmac_vap;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_flush_pmksa::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == net_device)) {
        oam_error_log2(0, OAM_SF_CFG, "{uapi_cfg80211_flush_pmksa::param null! wiphy[%p], net_device[%p]!!}",
            (uintptr_t)wiphy, (uintptr_t)net_device);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_vap_stru 结构，以防wal_alloc_cfg_event返回ERROR */
    hmac_vap = (hmac_vap_stru *)net_device->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_flush_pmksa::can't get mac vap from netdevice priv data!}");
        return -OAL_EFAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_device), WLAN_MSG_W2H_CFG_CFG80211_FLUSH_PMKSA,
        (osal_u8 *)0, 0);
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_cfg80211_flush_pmksa::wal_sync_post2hmac_no_rsp fail[%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_drv_remain_on_channel_check_vap(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, wlan_ieee80211_roc_type_uint8 roc_type)
{
    hmac_vap_stru *other_vap = OAL_PTR_NULL;
    osal_u8 vap_idx;

    /* check all vap state in case other vap is connecting now */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        other_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (other_vap == OAL_PTR_NULL) {
            oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {wal_drv_remain_on_channel::vap is null! vap id is %d}",
                hmac_vap->vap_id, hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (((other_vap->vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
            (other_vap->vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (MAC_VAP_STATE_ROAMING == other_vap->vap_state)
#endif
        ) {
            oam_warning_log3(0, OAM_SF_P2P,
                "vap_id[%d] {wal_drv_remain_on_channel::invalid ROC[%d] for roaming or connecting state[%d]!}",
                hmac_vap->vap_id, roc_type, other_vap->vap_state);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_drv_remain_on_channel_check(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev,
    struct ieee80211_channel *chan, osal_u64 *pull_cookie,
    wlan_ieee80211_roc_type_uint8 roc_type)
{
    oal_net_device_stru *netdev = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_CONNECT_CHECK);

    if ((wiphy == OAL_PTR_NULL) || (wdev == OAL_PTR_NULL) ||
        (chan == OAL_PTR_NULL) || (pull_cookie == OAL_PTR_NULL)) {
        oam_error_log4(0, OAM_SF_P2P,
            "{wal_drv_remain_on_channel::null ptr, wiphy[%p], wdev[%p], chan[%p], pull_cookie[%p]",
            (uintptr_t)wiphy, (uintptr_t)wdev, (uintptr_t)chan, (uintptr_t)pull_cookie);
        return -OAL_EINVAL;
    }

    netdev = wdev->netdev;
    if (netdev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::netdev ptr is null!}");
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_drv_remain_on_channel::hmac_device ptr is null!}\r\n");
        return -OAL_EINVAL;
    }

    if (fhook != OSAL_NULL) {
        if (((hmac_wapi_connected_etc_cb)fhook)(hmac_vap->device_id) == OAL_TRUE) {
            oam_warning_log0(0, OAM_SF_CFG, "{stop p2p remaining under wapi!}");
            return -OAL_EINVAL;
        }
    }

    /* check all vap state in case other vap is connecting now */
    return wal_drv_remain_on_channel_check_vap(hmac_vap, hmac_device, roc_type);
}

OAL_STATIC osal_s32 wal_drv_remain_on_channel_event_prepare(struct ieee80211_channel *chan,
    osal_u32 duration, wlan_ieee80211_roc_type_uint8 roc_type,
    mac_remain_on_channel_param_stru *remain_param)
{
    osal_u16 center_freq = chan->center_freq;
    osal_s32 l_channel = (osal_s32)oal_ieee80211_frequency_to_channel((osal_s32)center_freq);

    remain_param->listen_channel  = (osal_u8)l_channel;
    remain_param->listen_duration = duration;
    remain_param->listen_channel_para  = *chan;
    remain_param->listen_channel_type =  WLAN_BAND_WIDTH_20M;
    remain_param->roc_type =  roc_type;

    if (chan->band == OAL_IEEE80211_BAND_2GHZ) {
        remain_param->band = WLAN_BAND_2G;
    } else if (chan->band == OAL_IEEE80211_BAND_5GHZ) {
        remain_param->band = WLAN_BAND_5G;
    } else {
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_drv_remain_on_channel_event_proc(oal_wireless_dev_stru *wdev,
    mac_remain_on_channel_param_stru *remain_param)
{
    oal_net_device_stru *netdev = wdev->netdev;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    osal_s32 l_ret;
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_REMAIN_ON_CHANNEL,
        (osal_u8 *)remain_param, OAL_SIZEOF(mac_remain_on_channel_param_stru));
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P,
            "vap_id[%d] {wal_drv_remain_on_channel::wal_sync_post2hmac_no_rsp return err code:[%d]!}",
            hmac_vap->vap_id, l_ret);
        return -OAL_EFAIL;
    }
    /* 待73适配HSO后，unref_param可以删除 */
    unref_param(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 保持在指定信道
*****************************************************************************/
OAL_STATIC osal_s32 wal_drv_remain_on_channel(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev,
    struct ieee80211_channel *chan, osal_u32 duration, osal_u64 *pull_cookie,
    wlan_ieee80211_roc_type_uint8 roc_type)
{
    oal_net_device_stru *netdev = OAL_PTR_NULL;
    mac_remain_on_channel_param_stru remain_param = {0};
    osal_s32 l_ret;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    l_ret = wal_drv_remain_on_channel_check(wiphy, wdev, chan, pull_cookie, roc_type);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    /* tx mgmt roc 优先级低,可以被自己的80211 roc以及80211 scan打断 */
    netdev = wdev->netdev;
    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN) {
        oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {wal_drv_remain_on_channel::new roc type[%d],cancel old roc!}",
            hmac_vap->vap_id, roc_type);
        l_ret = wal_p2p_stop_roc(hmac_vap, netdev);
        if (l_ret < 0) {
            return -OAL_EFAIL;
        }
    }

    /* 2.1 消息参数准备 */
    l_ret = wal_drv_remain_on_channel_event_prepare(chan, duration, roc_type, &remain_param);
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P, "vap_id[%d] {wal_drv_remain_on_channel::wrong band roc_type[%d]!}",
            hmac_vap->vap_id, chan->band);
        return l_ret;
    }

    if (roc_type == IEEE80211_ROC_TYPE_NORMAL) {
        /* 将cookie+1值提前，保证驱动侧扫描的cookie值一致避免两次扫描cookie值弄混的现象 */
        /* 设置cookie 值. cookie值上层调用需要判断是否是这次的发送导致的callback */
        hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
        *pull_cookie = ++hmac_device->st_p2p_info.ull_last_roc_id;
        if (*pull_cookie == 0) {
            *pull_cookie = ++hmac_device->st_p2p_info.ull_last_roc_id;
        }

        /* 保存cookie 值，下发给HMAC 和DMAC */
        remain_param.cookie = hmac_device->st_p2p_info.ull_last_roc_id;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_drv_remain_on_channel_event_proc(wdev, &remain_param);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    if (roc_type == IEEE80211_ROC_TYPE_NORMAL) {
        /* 上报暂停在指定信道成功 */
        oal_cfg80211_ready_on_channel_etc(wdev, *pull_cookie, chan, duration, GFP_KERNEL);
    }

    oam_warning_log4(0, OAM_SF_P2P,
        "{wal_drv_remain_on_channel::SUCC! l_channel=%d, duration=%d, cookie 0x%x, roc type=%d!}",
        remain_param.listen_channel, duration, *pull_cookie, roc_type);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 保持在指定信道
 输入参数  : oal_wiphy_stru           *wiphy
             oal_wireless_dev_stru    *wdev
             struct ieee80211_channel *chan
             osal_u32                duration
             osal_u64               *pull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年11月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_remain_on_channel(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, struct ieee80211_channel *chan, osal_u32 duration,
    osal_u64 *pull_cookie)
{
    osal_s32 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_remain_on_channel::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    ret = wal_drv_remain_on_channel(wiphy, wdev, chan, duration, pull_cookie, IEEE80211_ROC_TYPE_NORMAL);
    return (ret > 0) ? -OAL_EFAIL : ret;
}

/*****************************************************************************
 功能描述  : 停止保持在指定信道
 输入参数  : oal_wiphy_stru           *wiphy
             oal_wireless_dev_stru   *wdev
             osal_u64               ull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年11月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, osal_u64 ull_cookie)
{
    mac_remain_on_channel_param_stru on_channel_para = {0};
    oal_net_device_stru             *netdev;
    osal_s32                        l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_cancel_remain_on_channel::dfr ing..[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == wdev)) {
        oam_error_log2(0, OAM_SF_P2P,
            "{uapi_cfg80211_cancel_remain_on_channel::wiphy or wdevis null,error %p, %p}",
            (uintptr_t)wiphy, (uintptr_t)wdev);
        return -OAL_EINVAL;
    }

    netdev = wdev->netdev;
    if (OAL_PTR_NULL == netdev) {
        oam_error_log0(0, OAM_SF_P2P, "{uapi_cfg80211_cancel_remain_on_channel::netdev ptr is null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    oam_warning_log1(0, OAM_SF_P2P, "uapi_cfg80211_cancel_remain_on_channel[0x%x].", ull_cookie);
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
        (osal_u8 *)&on_channel_para, OAL_SIZEOF(on_channel_para));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
            "{uapi_cfg80211_cancel_remain_on_channel::wal_sync_post2hmac_no_rsp return err code: [%d]!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 从cookie array 中查找相应cookie index
 输入参数  : osal_u8 *cookie_bitmap
             osal_u8 *cookie_idx
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年7月31日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_check_cookie_from_array(osal_u8 *cookie_bitmap, osal_u8 cookie_idx)
{
    /* 从cookie bitmap中查找相应的cookie index，如果位图为0，表示已经被del */
    if (*cookie_bitmap & (BIT(cookie_idx))) {
        return OAL_SUCC;
    }
    /* 找不到则返回FAIL */
    return OAL_FAIL;
}

osal_s32 wal_mgmt_do_tx_wait_condition(const void *param)
{
    oal_mgmt_tx_stru *pst_mgmt_tx = (oal_mgmt_tx_stru *)param;
    return (OAL_TRUE == pst_mgmt_tx->mgmt_tx_complete);
}

/*****************************************************************************
 功能描述  : WAL 层发送从wpa_supplicant  接收到的管理帧
 输入参数  : oal_net_device_stru    *netdev        发送管理帧设备
             mac_mgmt_frame_stru    *mgmt_tx_param 发送管理帧参数
             bool                    offchan        是否需要切离信道
             osal_u32              wait           等待时间
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_u32 OAL_SUCC 发送成功
                                   OAL_FAIL 发送失败

 修改历史      :
  1.日    期   : 2015年8月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_mgmt_do_tx(oal_net_device_stru    *netdev,
    mac_mgmt_frame_stru *mgmt_tx_param,
    bool                 offchan,
    osal_u32           wait)
{
    hmac_vap_stru                   *hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    oal_mgmt_tx_stru                *pst_mgmt_tx = OAL_PTR_NULL;
    osal_s32                        l_ret;
    osal_slong                          i_leftime;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_mgmt_do_tx::can't get mac vap from netdevice priv data.}");
        return OAL_FAIL;
    }

    /*
     如果是Go, 仍然继续发送(比如Go Device Discoverability Request)
     如果不是Go，则需要offchanel但驱动已经不在roc状态了,此action帧不需要发送
    */
    if (!is_p2p_go(hmac_vap) && offchan == OAL_TRUE &&
        (hmac_vap->vap_state != MAC_VAP_STATE_LISTEN)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {wal_mgmt_do_tx::hmac_vap state[%d]not in listen!}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return DMAC_TX_INVALID; // 返回TX_INVALID,软件不再重传,告诉上层此次tx mgmt失败结束
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_MGMT_TX,
        (osal_u8 *)mgmt_tx_param, OAL_SIZEOF(mac_mgmt_frame_stru));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        wifi_printf("{wal_mgmt_do_tx::wal_sync_post2hmac_no_rsp return err code %d!}\r\n", l_ret);
        return OAL_FAIL;
    }

    pst_mgmt_tx = &(hmac_vap->mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status  = OAL_FAIL;
    /* 增加打印，以看到超时时间 */
    oam_info_log1(0, OAM_SF_ANY, "{wal_mgmt_do_tx:: It will wait %d ms at most for the tx status report!}", wait);
    /* 改成和上一级函数的等待时间一致 */
    i_leftime = osal_wait_timeout_interruptible(&pst_mgmt_tx->wait_queue, wal_mgmt_do_tx_wait_condition, pst_mgmt_tx,
        wait);
    if (i_leftime == 0) {
        /* 定时器超时 */
        wifi_printf("{wal_mgmt_do_tx::mgmt tx timeout!}\r\n");
        return OAL_FAIL;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        wifi_printf("{wal_mgmt_do_tx::mgmt tx timer error!}\r\n");
        return OAL_FAIL;
    } else {
        /* 正常结束  */
        oam_info_log0(0, OAM_SF_ANY, "{wal_mgmt_do_tx::mgmt tx commpleted!}");

        /* 正常发送结束，返回发送完成状态 */
        return pst_mgmt_tx->mgmt_tx_status;
    }
}

/*****************************************************************************
 功能描述  : 发送管理帧
 输入参数  : oal_wiphy_stru              *wiphy
             oal_wireless_dev_stru       *wdev
             struct ieee80211_channel    *chan
             bool                         offchan
             osal_u32                   wait
             OAL_CONST osal_u8         *buf
             size_t                       len
             bool                         no_cck
             bool                         dont_wait_for_ack
             osal_u64                  *pull_cookie
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年11月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_mgmt_tx(oal_wiphy_stru                *wiphy,
    oal_wireless_dev_stru       *wdev,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
        struct cfg80211_mgmt_tx_params *params,
#else
        oal_ieee80211_channel       *chan,
        bool                         offchan,
        osal_u32                   wait,
        OAL_CONST osal_u8         *buf,
        oal_size_t                   len,
        bool                         no_cck,
        bool                         dont_wait_for_ack,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) */
        osal_u64                  *pull_cookie)
{
    oal_net_device_stru             *netdev;
    hmac_device_stru                *hmac_device;
    OAL_CONST oal_ieee80211_mgmt    *mgmt;
    osal_s32                        ul_ret = 0;
    mac_mgmt_frame_stru              mgmt_tx;
    osal_u8                        cookie_idx;
    osal_u8                        retry;
    mac_p2p_info_stru               *p2p_info;
    hmac_vap_stru                   *hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;
    oal_wireless_dev_stru           *roc_wireless_dev = wdev;
    unsigned long                    start_time_stamp;
    bool                             need_offchan = OAL_FALSE;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    oal_ieee80211_channel       *chan;
    OAL_CONST osal_u8         *buf;
    oal_size_t                   len;
    osal_u32                   wait;
    bool                         offchan = OAL_FALSE;
    if (params == OAL_PTR_NULL) {
        return -OAL_EINVAL;
    }
    chan   = params->chan;
    buf    = params->buf;
    len     = params->len;
    offchan = params->offchan;
    wait    = params->wait;
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_mgmt_tx::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == wdev) ||
        (OAL_PTR_NULL == pull_cookie) || (OAL_PTR_NULL == buf)) {
        oam_error_log4(0, OAM_SF_CFG,
            "{uapi_cfg80211_mgmt_tx::ptr is null: wiphy[%p] wdev[%p] chan[%p] pull_cookie[%p]}",
            (uintptr_t)wiphy, (uintptr_t)wdev, (uintptr_t)chan, (uintptr_t)pull_cookie);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    netdev = wdev->netdev;
    if (OAL_PTR_NULL == netdev) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::netdev ptr is null!}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (OAL_PTR_NULL == hmac_vap) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::can't get mac vap from netdevice priv data!}");
        return -OAL_EINVAL;
    }

#ifdef _PRE_WLAN_DFT_STAT
    /* WAL接收协议栈发来的管理帧数目 */
    hmac_pkts_tx_stat(hmac_vap->vap_id, 1, TX_M_FROM_KERNEL_PKTS);
#endif

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_PTR_NULL == hmac_device) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::hmac_device ptr is null!}");
        /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return -OAL_EINVAL;
    }

    p2p_info = &hmac_device->st_p2p_info;
    *pull_cookie = p2p_info->ull_send_action_id++;   /* cookie值上层调用需要判断是否是这次的发送导致的callback */
    if (*pull_cookie == 0) {
        *pull_cookie = p2p_info->ull_send_action_id++;
    }
    mgmt = (const struct ieee80211_mgmt *)buf;
    if (oal_ieee80211_is_probe_resp(mgmt->frame_control)) {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
        /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    memset_s(&mgmt_tx, OAL_SIZEOF(mgmt_tx), 0, OAL_SIZEOF(mgmt_tx));
    mgmt_tx.channel = (!!chan) ? oal_ieee80211_frequency_to_channel(chan->center_freq) : 0;
    ul_ret = wal_add_cookie_to_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pull_cookie, &cookie_idx);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_mgmt_tx::Failed to add cookies, ul_ret[%d]!}",
            hmac_vap->vap_id, ul_ret);
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);
        /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return -OAL_EINVAL;
    } else {
        mgmt_tx.mgmt_frame_id = cookie_idx;
    }

    mgmt_tx.len       = len;
    mgmt_tx.frame    = buf;

    pst_mgmt_tx = &(hmac_vap->mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;

    /* APUT模式不能roc */
    switch (hmac_vap->vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            // 此处应该是channel number，而不是channel index
            oam_warning_log4(0, OAM_SF_ANY,
                "vap_id[%d] {uapi_cfg80211_mgmt_tx::p2p mode[%d], tx mgmt vap channel[%d], mgmt tx channel[%d]",
                hmac_vap->vap_id, hmac_vap->p2p_mode,
                hmac_vap->channel.chan_number, mgmt_tx.channel);
            if ((hmac_vap->channel.chan_number != mgmt_tx.channel) &&
                is_p2p_go(hmac_vap)) {
                if (OAL_PTR_NULL == hmac_device->st_p2p_info.p2p_net_device) {
                    oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_mgmt_tx::go mode but p2p dev is null",
                        hmac_vap->vap_id);
                    /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
                    hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
                    return -OAL_EINVAL;
                }
                // 使用p2p0 roc
                roc_wireless_dev = OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.p2p_net_device);
                need_offchan = OAL_TRUE;
            }
            break;

        /* P2P CL DEV */
        case WLAN_VAP_MODE_BSS_STA:
            if ((OAL_TRUE == offchan) && (wiphy->flags & WIPHY_FLAG_OFFCHAN_TX)) {
                need_offchan = OAL_TRUE;
            }
            if ((WLAN_LEGACY_VAP_MODE == hmac_vap->p2p_mode)
                && (MAC_VAP_STATE_UP == hmac_vap->vap_state)) {
                need_offchan = OAL_FALSE;
            }
            break;

        default:
            break;
    }

    if ((OAL_TRUE == need_offchan) && !chan) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_mgmt_tx::need offchannle but channel is null}",
            hmac_vap->vap_id, offchan);
        /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return -OAL_EINVAL;
    }

    // 不管是否需要切离信道，后面发送管理帧时都需要等待一定时间
    if (0 == wait) {
        wait = WAL_MGMT_TX_TIMEOUT_MSEC;
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_cfg80211_mgmt_tx::wait time is 0, set it to %d ms}",
            hmac_vap->vap_id, wait);
    }

    wifi_printf("{uapi_cfg80211_mgmt_tx::offchannel[%d],vap channel[%d],tx channel[%d]vap state[%d],wait[%d]}\r\n",
        need_offchan, hmac_vap->channel.chan_number, mgmt_tx.channel, hmac_vap->vap_state, wait);

    /* 需要offchannel,按照入参切到相应的信道XXms */
    if (OAL_TRUE == need_offchan) {
        ul_ret = wal_drv_remain_on_channel(wiphy, roc_wireless_dev,
            chan, wait, pull_cookie, IEEE80211_ROC_TYPE_MGMT_TX);
        if (OAL_SUCC != ul_ret) {
            oam_warning_log4(0, OAM_SF_CFG,
                "{uapi_cfg80211_mgmt_tx::wal_drv_remain_on_channel[%d]!!!offchannel[%d].channel[%d],vap state[%d]}",
                ul_ret, need_offchan, mgmt_tx.channel, hmac_vap->vap_state);
            /* WAL管理帧和控制帧发送给HMAC丢弃数目 */
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
            return -OAL_EBUSY;
        }
    }

    start_time_stamp = OAL_TIME_JIFFY;

    /* 发错信道,软件不能一直重传,加入软件重传次数限制 */
    retry = 0;
    /* 发送失败，则尝试重传 */
    do {
        ul_ret = wal_mgmt_do_tx(netdev, &mgmt_tx, need_offchan, wait);
        retry++;
    } while ((ul_ret != DMAC_TX_SUCC) && (ul_ret != DMAC_TX_INVALID) && (retry <= WAL_MGMT_TX_RETRY_CNT)
             && time_before((osal_ulong)OAL_TIME_JIFFY, start_time_stamp + oal_msecs_to_jiffies(wait)));
    /* 请留意，如果是Go端(通过p2p0)在一次会话中第二次(第一次发送已成功)给第三方发送
       Device Discoverability Response等帧时，可能会由于对方不再侦听而超时失败 */
    if (retry > WAL_MGMT_TX_RETRY_CNT) {
        wifi_printf("vap_id[%d] {uapi_cfg80211_mgmt_tx::retry count[%d]>max[%d],tx status[%d],stop tx mgmt}\r\n",
            hmac_vap->vap_id, retry, WAL_MGMT_TX_RETRY_CNT, ul_ret);
    }

    if (ul_ret != DMAC_TX_SUCC) {
        /* 发送失败，处理超时帧的bitmap */
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_FALSE, GFP_KERNEL);
#ifdef _PRE_WLAN_DFT_STAT
        hmac_pkts_tx_stat(hmac_vap->vap_id, 1, TX_M_SEND_HMAC_FAIL_PKTS);
#endif
    } else {
        /* 正常结束  */
        *pull_cookie = g_cookie_array_etc[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pst_mgmt_tx->mgmt_frame_id);
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
#ifdef _PRE_WLAN_DFT_STAT
        /* WAL发送至HMAC管理帧报文成功 */
        hmac_pkts_tx_stat(hmac_vap->vap_id, 1, TX_M_SEND_HMAC_SUCC_PKTS);
#endif
    }

    /* 请留意，如果是Go端(通过p2p0)在一次会话中第二次(第一次发送已成功)给第三方发送
       Device Discoverability Response等帧时，可能会由于对方不再侦听而超时失败 */
    wifi_printf("{uapi_cfg80211_mgmt_tx::vap[%d] tx status [%d], retry cnt[%d]}, delta_time[%d]\r\n",
        hmac_vap->vap_id, ul_ret, retry, oal_jiffies_to_msecs(OAL_TIME_JIFFY - start_time_stamp));
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : HMAC抛mgmt tx status到WAL, 唤醒wait queue
 输入参数  : frw_event_mem_stru *event_mem
 输出参数  : 无
 返 回 值  : osal_s32

 修改历史      :
  1.日    期   : 2014年1月6日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_cfg80211_mgmt_tx_status_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_tx_status_stru *tx_status = (dmac_tx_status_stru *)(msg->data);
    oal_mgmt_tx_stru *mgmt_tx = &(hmac_vap->mgmt_tx);

    mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    mgmt_tx->mgmt_tx_status   = tx_status->dscr_status;
    mgmt_tx->mgmt_frame_id    = tx_status->mgmt_frame_id;

    /* 找不到相应的cookie值，说明已经超时被处理，不需要再唤醒 */
    if (OAL_SUCC == wal_check_cookie_from_array(&g_uc_cookie_array_bitmap_etc, mgmt_tx->mgmt_frame_id)) {
        /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
        oal_smp_mb();
        osal_wait_wakeup(&mgmt_tx->wait_queue);
    }

    return OAL_SUCC;
}

/* P2P 补充缺失的CFG80211接口 */
osal_void uapi_cfg80211_mgmt_frame_register_etc(struct wiphy *wiphy, struct wireless_dev *wdev, osal_u16 frame_type,
    bool reg)
{
    oam_info_log3(0, OAM_SF_CFG,
        "{uapi_cfg80211_mgmt_frame_register_etc::enter.frame_type[0x%04x], reg[%d], if_type[%d]}",
        frame_type, reg, wdev->iftype);
    return;
}

// osal_s32 uapi_cfg80211_set_bitrate_mask_etc(struct wiphy *wiphy, struct net_device *dev,
// #ifdef CONTROLLER_CUSTOMIZATION
//     osal_u32 link_id,
// #endif
//     const u8 *peer,
//     const struct cfg80211_bitrate_mask *mask)
static int uapi_cfg80211_set_bitrate_mask_etc(
    struct wiphy *wiphy,
    struct net_device *netdev,
    unsigned int link_id,  // 新增参数
    const u8 *peer,
    const struct cfg80211_bitrate_mask *mask
)
{
    oam_info_log0(0, OAM_SF_CFG, "{uapi_cfg80211_set_bitrate_mask_etc::enter 000.}");

    return OAL_SUCC;
}

#else

osal_s32 wal_cfg80211_mgmt_tx_wait_condition(const void *param)
{
    oal_mgmt_tx_stru *pst_mgmt_tx = (oal_mgmt_tx_stru *)param;
    return (OAL_TRUE == pst_mgmt_tx->mgmt_tx_complete);
}

/*****************************************************************************
 功能描述  : 发送管理帧
*****************************************************************************/
osal_s32 uapi_cfg80211_mgmt_tx(oal_wiphy_stru                *wiphy,
    oal_wireless_dev_stru       *wdev,
    oal_ieee80211_channel       *chan,
    oal_bool_enum_uint8          offchan,
    osal_u32                   wait,
    OAL_CONST osal_u8         *buf,
    size_t                       len,
    oal_bool_enum_uint8          no_cck,
    oal_bool_enum_uint8          dont_wait_for_ack,
    osal_u64                  *pull_cookie)
{
    oal_net_device_stru             *netdev;
    hmac_device_stru                *hmac_device;
    OAL_CONST oal_ieee80211_mgmt    *mgmt = (const struct ieee80211_mgmt *)buf;
    osal_s32                        l_ret = 0;
    osal_u32                       ul_ret = 0;
    mac_mgmt_frame_stru              mgmt_tx;
    osal_slong                          i_leftime;
    osal_u8                        cookie_idx;
    mac_p2p_info_stru               *p2p_info;
    hmac_vap_stru                   *hmac_vap;
    oal_mgmt_tx_stru                *pst_mgmt_tx;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == wdev) || (OAL_PTR_NULL == chan) ||
        (OAL_PTR_NULL == pull_cookie) || (OAL_PTR_NULL == buf)) {
        oam_error_log4(0, OAM_SF_CFG,
            "{uapi_cfg80211_mgmt_tx::null ptr: wiphy[%x] wdev[%x] chan[%x] pull_cookie[%x]}",
            wiphy, wdev, chan, pull_cookie);
        return -OAL_EINVAL;
    }

    /* 通过net_device 找到对应的hmac_device_stru 结构 */
    netdev = wdev->netdev;
    if (netdev == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::netdev is null!}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)netdev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::hmac_vap is null!}");
        return -OAL_EINVAL;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_mgmt_tx::hmac_device is null!}");
        return -OAL_EINVAL;
    }

    p2p_info = &hmac_device->st_p2p_info;
    /* cookie值上层调用需要判断是否是这次的发送导致的callback */
    *pull_cookie = p2p_info->ull_send_action_id++;
    if (*pull_cookie == 0) {
        *pull_cookie = p2p_info->ull_send_action_id++;
    }
    if (oal_ieee80211_is_probe_resp(mgmt->frame_control)) {
        *pull_cookie = 0; /* set cookie default value */
        /* host should not send PROE RESPONSE,
           device will send immediately when receive probe request packet */
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    memset_s(&mgmt_tx, OAL_SIZEOF(mgmt_tx), 0, OAL_SIZEOF(mgmt_tx));
    mgmt_tx.channel = oal_ieee80211_frequency_to_channel((osal_s32)chan->center_freq);
    ul_ret = wal_add_cookie_to_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pull_cookie, &cookie_idx);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_mgmt_tx::Failed to add cookies!}");
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);
        return -OAL_EINVAL;
    } else {
        mgmt_tx.mgmt_frame_id = cookie_idx;
    }
    mgmt_tx.len = (osal_u16)len;
    mgmt_tx.frame = buf;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_MGMT_TX,
        (osal_u8 *)&mgmt_tx, OAL_SIZEOF(mgmt_tx));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_mgmt_tx::wal_sync_post2hmac_no_rsp err %d!}", l_ret);
        return -OAL_EINVAL;
    }

    pst_mgmt_tx = &(hmac_vap->mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_FALSE;
    pst_mgmt_tx->mgmt_tx_status = OAL_FAIL;
    i_leftime = osal_wait_timeout_interruptible(&pst_mgmt_tx->wait_queue, wal_cfg80211_mgmt_tx_wait_condition,
        pst_mgmt_tx, WAL_MGMT_TX_TIMEOUT_MSEC);

    if (0 == i_leftime) {
        /* 超时还没有上报扫描结束 */
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_mgmt_tx::mgmt tx wait for %ld ms timeout!}", ((osal_u32)WAL_MGMT_TX_TIMEOUT_MSEC));
        wal_check_cookie_timeout(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, OAL_TIME_JIFFY);

    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_mgmt_tx::mgmt tx wait for %ld ms error!}", ((osal_u32)WAL_MGMT_TX_TIMEOUT_MSEC));
    } else {
        /* 正常结束  */
        oam_info_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_mgmt_tx::mgmt tx wait for %ld ms complete!}", ((osal_u32)WAL_MGMT_TX_TIMEOUT_MSEC));
        *pull_cookie = g_cookie_array_etc[pst_mgmt_tx->mgmt_frame_id].ull_cookie;
        wal_del_cookie_from_array(g_cookie_array_etc, &g_uc_cookie_array_bitmap_etc, pst_mgmt_tx->mgmt_frame_id);
    }

    if (pst_mgmt_tx->mgmt_tx_status != DMAC_TX_SUCC) {
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_FALSE, GFP_KERNEL);
        return -OAL_EINVAL;
    } else {
        oal_cfg80211_mgmt_tx_status_etc(wdev, *pull_cookie, buf, len, OAL_TRUE, GFP_KERNEL);
        return OAL_SUCC;
    }
}

/*****************************************************************************
 功能描述  : HMAC抛mgmt tx status到WAL, 唤醒wait queue
 输入参数  : frw_event_mem_stru *event_mem
 输出参数  : 无
 返 回 值  : osal_s32

 修改历史      :
  1.日    期   : 2014年1月6日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_cfg80211_mgmt_tx_status_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_tx_status_stru *tx_status;
    oal_mgmt_tx_stru *pst_mgmt_tx;

    tx_status   = (dmac_tx_status_stru *)(msg->data);
    pst_mgmt_tx = &(hmac_vap->mgmt_tx);
    pst_mgmt_tx->mgmt_tx_complete = OAL_TRUE;
    pst_mgmt_tx->mgmt_tx_status   = tx_status->dscr_status;
    pst_mgmt_tx->mgmt_frame_id    = tx_status->mgmt_frame_id;

    /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
    oal_smp_mb();
    osal_wait_wakeup(&pst_mgmt_tx->wait_queue);
    return OAL_SUCC;
}

#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

/*****************************************************************************
 功能描述  : 启动P2P_DEV
 输入参数  : oal_wiphy_stru       *wiphy
             oal_wireless_dev_stru   *wdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年11月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_start_p2p_device(oal_wiphy_stru       *wiphy, oal_wireless_dev_stru   *wdev)
{
    return -OAL_EFAIL;
}

/*****************************************************************************
 功能描述  : 停止P2P_DEV
 输入参数  : oal_wiphy_stru       *wiphy
             oal_wireless_dev_stru   *wdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2014年11月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC void uapi_cfg80211_stop_p2p_device(oal_wiphy_stru       *wiphy, oal_wireless_dev_stru   *wdev)
{
}

/*****************************************************************************
 功能描述  : 开关低功耗
 输入参数  : oal_wiphy_stru       *wiphy
             oal_wireless_dev_stru   *wdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2015年07月22日
    修改内容   : 新生成函数

*****************************************************************************/
static osal_s32 uapi_cfg80211_set_power_mgmt(oal_wiphy_stru  *wiphy, oal_net_device_stru *netdev, bool enabled,
    osal_s32 timeout)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
#ifndef _PRE_BSLE_GATEWAY
    mac_cfg_ps_open_stru        sta_pm_open;
    osal_s32                    l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_STA_AP_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_set_power_mgmt::dfr ing... status[%d]", wal_dfr_get_status());
        return OAL_SUCC;
    }
#endif
    if (OAL_UNLIKELY((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == netdev))) {
        oam_error_log2(0, OAM_SF_ANY,
            "{uapi_cfg80211_set_power_mgmt::wiphy or wdev null ptrerror %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)netdev);
        return -OAL_EINVAL;
    }

    (osal_void)memset_s(&sta_pm_open, OAL_SIZEOF(sta_pm_open), 0, OAL_SIZEOF(sta_pm_open));

    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    sta_pm_open.pm_enable      = enabled;
    sta_pm_open.pm_ctrl_type   = MAC_STA_PM_CTRL_TYPE_HOST;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_SET_STA_PM_ON,
        (osal_u8 *)&sta_pm_open, OAL_SIZEOF(mac_cfg_ps_open_stru));
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfg80211_set_power_mgmt::fail to send pm cfg msg, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }
#endif
#endif
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11R

/*****************************************************************************
 功能描述  : 停止P2P_DEV
 输入参数  : oal_wiphy_stru       *wiphy
             oal_wireless_dev_stru   *wdev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2015年8月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfg80211_update_ft_ies(oal_wiphy_stru *wiphy,
    oal_net_device_stru *netdev, oal_cfg80211_update_ft_ies_stru *fties)
{
    mac_cfg80211_ft_ies_stru           *mac_ft_ies;
    osal_s32                           l_ret;
    mac_cfg80211_ft_ies_stru cfg_mac_ft_ies;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_update_ft_ies::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EINVAL;
    }
#endif
    (osal_void)memset_s(&cfg_mac_ft_ies, OAL_SIZEOF(cfg_mac_ft_ies), 0, OAL_SIZEOF(cfg_mac_ft_ies));
    mac_ft_ies = &cfg_mac_ft_ies;

    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == netdev) || (OAL_PTR_NULL == fties)) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_update_ft_ies::param is null.}");

        return -OAL_EINVAL;
    }

    if ((fties->ie == OAL_PTR_NULL) || (fties->ie_len == 0)) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_update_ft_ies::unexpect ie or len[%d].}", fties->ie_len);

        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    mac_ft_ies->mdid = fties->md;
    mac_ft_ies->len  = fties->ie_len;
    if (memcpy_s(mac_ft_ies->ie, sizeof(mac_ft_ies->ie), fties->ie, fties->ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_update_ft_ies::memcpy_s error}");
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_SET_FT_IES,
        (osal_u8 *)&cfg_mac_ft_ies, OAL_SIZEOF(cfg_mac_ft_ies));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_update_ft_ies::wal_sync_post2hmac_no_rsp errcode %d}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_11R

#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
/*****************************************************************************
 功能描述  : report channel stat to kernel
 输入参数  : oal_wiphy_stru       *wiphy
             oal_wireless_dev_stru   *wdev
             osal_s32 l_idx
             oal_survey_info_stru *info
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_s32

 修改历史      :
  1.日    期   : 2016年6月1日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 uapi_cfg80211_dump_survey_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev, osal_s32 l_idx,
    oal_survey_info_stru *info)
{
    hmac_survey_info_stru survey_info = {0};
    osal_s32 ret;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_dump_survey_etc::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    ret = hmac_cfg80211_dump_survey_etc(wiphy, netdev, l_idx, &survey_info);

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
    info->time                  = survey_info.time;
    info->time_busy             = survey_info.time_busy;
    info->time_ext_busy         = survey_info.time_ext_busy;
    info->time_rx               = survey_info.time_rx;
    info->time_tx               = survey_info.time_tx;
    info->filled = SURVEY_INFO_TIME | SURVEY_INFO_TIME_BUSY | SURVEY_INFO_TIME_RX | SURVEY_INFO_TIME_TX;
#else
    info->channel_time          = survey_info.time;
    info->channel_time_busy     = survey_info.time_busy;
    info->channel_time_ext_busy = survey_info.time_ext_busy;
    info->channel_time_rx       = survey_info.time_rx;
    info->channel_time_tx       = survey_info.time_tx;
    info->filled = SURVEY_INFO_CHANNEL_TIME | SURVEY_INFO_CHANNEL_TIME_BUSY |
        SURVEY_INFO_CHANNEL_TIME_RX | SURVEY_INFO_CHANNEL_TIME_TX;
#endif

    if (survey_info.free_power_cnt && survey_info.free_power_stats_20m < 0) {
        info->noise = survey_info.noise;
        info->filled |= SURVEY_INFO_NOISE_DBM;
    }
    info->channel = survey_info.channel;

    return (ret > 0) ? -OAL_EFAIL : ret;
}
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0))
OAL_STATIC osal_void uapi_cfg80211_abort_scan(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev)
{
    oal_net_device_stru *netdev;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_abort_scan::dfr ing... status[%d]", wal_dfr_get_status());
        return;
    }
#endif
    /* 1.1 入参检查 */
    if ((OAL_PTR_NULL == wiphy) || (OAL_PTR_NULL == wdev)) {
        oam_error_log2(0, OAM_SF_CFG, "{uapi_cfg80211_abort_scan::wiphy or wdev is null, %p, %p!}",
            (uintptr_t)wiphy, (uintptr_t)wdev);
        return;
    }

    netdev = wdev->netdev;
    if (OAL_PTR_NULL == netdev) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_cfg80211_abort_scan::netdev is null!}");
        return;
    }
    oam_warning_log0(0, OAM_SF_CFG, "{uapi_cfg80211_abort_scan::enter!}");
    wal_force_scan_complete_etc(netdev, OAL_TRUE);
    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
STATIC osal_s32  wal_cfg80211_set_rekey_offload_etc(oal_net_device_stru *net_dev,
    mac_rekey_offload_stru *rekey_offload_param)
{
    osal_s32 ret;
    osal_u32 len;

    if (rekey_offload_param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_cfg80211_set_rekey_offload_etc::rekey_offload_param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    len = (osal_u32)sizeof(mac_rekey_offload_stru);
    if (WAL_MSG_WRITE_MAX_LEN < len) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "{wal_cfg80211_set_rekey_offload_etc::len %d > WAL_MSG_WRITE_MAX_LEN %d err!}\r\n", len,
                       WAL_MSG_WRITE_MAX_LEN);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_REKEY,
        (osal_u8 *)rekey_offload_param, len);
#else
    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_REKEY,
        (osal_u8 *)rekey_offload_param, len);
#endif
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SCAN,
            "{wal_cfg80211_set_rekey_offload_etc::wal_sync_send2device_no_rsp return err code %d!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 uapi_cfg80211_set_rekey_offload(oal_wiphy_stru *wiphy, oal_net_device_stru  *netdev,
    oal_cfg80211_gtk_rekey_data *rekey_info)
{
    mac_rekey_offload_stru         rekey_params;
    osal_u32                 ret;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_set_rekey_offload::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAIL;
    }
#endif
    if (memcpy_s(rekey_params.kck, MAC_REKEY_OFFLOAD_KCK_LEN,
                 rekey_info->kck, MAC_REKEY_OFFLOAD_KCK_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_rekey_offload::kck mem safe function err!}");
        return -OAL_EFAIL;
    }

    if (memcpy_s(rekey_params.kek, MAC_REKEY_OFFLOAD_KEK_LEN,
                 rekey_info->kek, MAC_REKEY_OFFLOAD_KEK_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_rekey_offload::kek mem safe function err!}");
        return -OAL_EFAIL;
    }

    if (memcpy_s(rekey_params.replay_ctr, MAC_REKEY_OFFLOAD_REPLAY_LEN,
                 rekey_info->replay_ctr, MAC_REKEY_OFFLOAD_REPLAY_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_set_rekey_offload::replay_ctr mem safe function err!}");
        return -OAL_EFAIL;
    }
    /* 抛事件给驱动 */
    ret = wal_cfg80211_set_rekey_offload_etc(netdev, &rekey_params);
    if (OAL_SUCC != ret) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_cfg80211_set_rekey_offload::uapi_cfg80211_set_rekey_offload %d!}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
/* 上层下发的ext auth status，函数需返回SUCC，否则将导致kernel一直没有返回消息给wpa/hostapd */
osal_s32 uapi_cfg80211_external_auth_status(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_external_auth_stru *param)
{
    osal_u32       ret;
    hmac_external_auth_stru *ext_auth;
    hmac_external_auth_stru mac_ext_auth;

    (osal_void)memset_s(&mac_ext_auth, OAL_SIZEOF(mac_ext_auth), 0, OAL_SIZEOF(mac_ext_auth));
    ext_auth = &mac_ext_auth;

    wal_record_wifi_external_log(WLAN_WIFI_CFG80211_OPS, __func__);
    /* 1 参数合法性检查 */
    if (netdev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_external_auth_status::net_dev is null!");
        return OAL_SUCC;
    }

    if (memcpy_s(ext_auth->bssid, OAL_MAC_ADDR_LEN, param->bssid, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_external_auth_status::memcpy bssid error");
        return OAL_SUCC;
    }
    ext_auth->status = param->status;
    if (param->pmkid != OAL_PTR_NULL) {
        if (memcpy_s(ext_auth->pmkid, WLAN_PMKID_LEN, param->pmkid, WLAN_PMKID_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_external_auth_status::memcpy pmkid error");
            return OAL_SUCC;
        }
        ext_auth->have_pmkid = OAL_TRUE;
    }
    /* 抛事件给驱动 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_EXTERNAL_AUTH_STATUS,
        (osal_u8 *)&mac_ext_auth, OAL_SIZEOF(mac_ext_auth));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_external_auth_status::wal_sync_post2hmac_no_rsp return err code:[%d]!}", ret);
    }

    return OAL_SUCC;
}

/* 上层下发的更新owe ie，函数需返回SUCC，否则导致kernel一直没有返回消息给wpa/hostapd */
OAL_STATIC osal_s32 uapi_cfg80211_update_owe_info(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_cfg80211_update_owe_info *owe_info)
{
    hmac_owe_info info = {0};
    osal_u32 ret;

    info.status = owe_info->status;
    if (memcpy_s(info.peer, sizeof(info.peer), owe_info->peer, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_cfg80211_update_owe_info::memcpy_s mac error");
        return OAL_SUCC;
    }
    info.ie = (osal_u8 *)owe_info->ie;
    info.ie_len = owe_info->ie_len;
    /* must be sync msg for ie point */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_UPDATE_OWE_INFO,
        (osal_u8 *)&info, OAL_SIZEOF(info));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{uapi_cfg80211_update_owe_info::wal_sync_post2hmac_no_rsp return err code:[%d]!}", ret);
        return OAL_SUCC;
    }
    oam_warning_log2(0, OAM_SF_ANY,
        "{uapi_cfg80211_update_owe_info::send owe to hmac, status=%u, ie_len=%u}", info.status, info.ie_len);

    return OAL_SUCC;
}

#endif

/* netlink方式设置p2p ie, 不是传统的ioctl方式 */
#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32 uapi_cfg80211_update_p2p_ie(oal_wiphy_stru *wiphy, oal_net_device_stru *net_dev,
    oal_cfg80211_update_p2p_ie *p2p_ie_info)
{
    osal_u8 *ie = OAL_PTR_NULL;
    osal_u32 ie_len;
    osal_u16 code;

#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfg80211_update_p2p_ie::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_FAIL;
    }
#endif
    if (p2p_ie_info == OAL_PTR_NULL || p2p_ie_info->ie == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfg80211_update_p2p_ie::para invalid.}");
        return -OAL_FAIL;
    }

    code = p2p_ie_info->code;
    ie = p2p_ie_info->ie;
    ie_len = p2p_ie_info->ie_len;

    /* must be sync msg for ie point */
    uapi_android_set_wps_p2p_ie_inner(net_dev, code, ie, ie_len);

    oam_warning_log2(0, OAM_SF_ANY,
        "{uapi_cfg80211_update_p2p_ie::send p2p ie to hmac, code=%u, ie_len=%u}", code, ie_len);

    return OAL_SUCC;
}
#endif

/* 不同操作系统函数指针结构体方式不同 */

OAL_STATIC oal_cfg80211_ops_stru  g_wal_cfg80211_ops = {
    .scan                     = uapi_cfg80211_scan,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    .connect                  = uapi_cfg80211_connect,
    .disconnect               = uapi_cfg80211_disconnect,
#endif
    .add_key                  = uapi_cfg80211_add_key,
    .get_key                  = uapi_cfg80211_get_key,
    .del_key                  = uapi_cfg80211_remove_key,
    .set_default_key          = uapi_cfg80211_set_default_key,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
    .set_default_mgmt_key     = uapi_cfg80211_set_default_mgmt_key_etc,
#else
    .set_default_mgmt_key     = uapi_cfg80211_set_default_key,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 5, 7))
    .set_channel              = uapi_cfg80211_set_channel,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    .set_wiphy_params         = uapi_cfg80211_set_wiphy_params,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(3,4,0))
    .add_beacon               = uapi_cfg80211_add_beacon,
    .set_beacon               = uapi_cfg80211_set_beacon,
    .del_beacon               = uapi_cfg80211_del_beacon,
#else /* mp12 修改AP 配置接口 */
    .change_beacon            = uapi_cfg80211_change_beacon,
    .start_ap                 = uapi_cfg80211_start_ap,
    .stop_ap                  = uapi_cfg80211_stop_ap,
    .change_bss               = uapi_cfg80211_change_bss,
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    .sched_scan_start         = uapi_cfg80211_sched_scan_start,
    .sched_scan_stop          = uapi_cfg80211_sched_scan_stop,
#endif
#endif
    .change_virtual_intf      = uapi_cfg80211_change_virtual_intf_etc,
    .add_station              = uapi_cfg80211_add_station,
    .del_station              = uapi_cfg80211_del_station,
    .change_station           = uapi_cfg80211_change_station,
    .get_station              = uapi_cfg80211_get_station,
    .dump_station             = uapi_cfg80211_dump_station,
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    .dump_survey              = uapi_cfg80211_dump_survey_etc,
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    .set_pmksa                = uapi_cfg80211_set_pmksa,
    .del_pmksa                = uapi_cfg80211_del_pmksa,
    .flush_pmksa              = uapi_cfg80211_flush_pmksa,
    .remain_on_channel        = uapi_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = uapi_cfg80211_cancel_remain_on_channel,
    .mgmt_tx                  = uapi_cfg80211_mgmt_tx,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
    .mgmt_frame_register      = uapi_cfg80211_mgmt_frame_register_etc,
#endif
    .set_bitrate_mask         = uapi_cfg80211_set_bitrate_mask_etc,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    .add_virtual_intf         = uapi_cfg80211_add_virtual_intf,
    .del_virtual_intf         = uapi_cfg80211_del_virtual_intf,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    .mgmt_tx_cancel_wait      = uapi_cfg80211_mgmt_tx_cancel_wait,
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    .start_p2p_device         = uapi_cfg80211_start_p2p_device,
    .stop_p2p_device          = uapi_cfg80211_stop_p2p_device,
    .set_power_mgmt           = uapi_cfg80211_set_power_mgmt,
#ifdef _PRE_WLAN_FEATURE_11R
    .update_ft_ies            = uapi_cfg80211_update_ft_ies,
#endif // _PRE_WLAN_FEATURE_11R
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0))
    .abort_scan               = uapi_cfg80211_abort_scan,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)) */
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
    .set_rekey_data           = uapi_cfg80211_set_rekey_offload,
#endif
#ifdef _PRE_WLAN_FEATURE_WPA3
    .external_auth = uapi_cfg80211_external_auth_status,
    .update_owe_info = uapi_cfg80211_update_owe_info,
#endif
#ifdef CONTROLLER_CUSTOMIZATION
    .update_p2p_ie = uapi_cfg80211_update_p2p_ie,
#endif
};

/*****************************************************************************
 功能描述  : 重新初始化wifi wiphy的bands
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年12月25日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void wal_cfg80211_reset_bands_etc(osal_u8  dev_id)
{
    int i;

    /* 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,
       相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志 */
    for (i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++) {
        g_st_supported_band_2ghz_info.channels[i].flags = 0;
    }

    if (hmac_device_check_5g_enable(dev_id)) {
        for (i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++) {
            g_st_supported_band_5ghz_info.channels[i].flags = 0;
        }
    }
}

oal_ieee80211_supported_band *wal_cfg80211_get_ieee80211_supported_band(void)
{
    return &g_st_supported_band_2ghz_info;
}
/*****************************************************************************
 功能描述  : 保存wifi wiphy的bands
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2016年12月06日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void wal_cfg80211_save_bands_etc(osal_u8  dev_id)
{
    int i;

    /* 每次更新国家码,flags都会被修改,且上次修改的值不会被清除,
       相当于每次修改的国家码都会生效,因此更新国家需要清除flag标志，
       每次设置国家码flag 后，保存当前设置到orig_flags中
     */
    for (i = 0; i < g_st_supported_band_2ghz_info.n_channels; i++) {
        g_st_supported_band_2ghz_info.channels[i].orig_flags = g_st_supported_band_2ghz_info.channels[i].flags;
    }

    if (hmac_device_check_5g_enable(dev_id)) {
        for (i = 0; i < g_st_supported_band_5ghz_info.n_channels; i++) {
            g_st_supported_band_5ghz_info.channels[i].orig_flags = g_st_supported_band_5ghz_info.channels[i].flags;
        }
    }
}

static osal_void wal_cfg_80211_init_p2p_wiphy(oal_wiphy_stru *wiphy, osal_u8 dev_id)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    wiphy->interface_modes |= BIT(NL80211_IFTYPE_P2P_CLIENT) | BIT(NL80211_IFTYPE_P2P_GO);
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    wiphy->mgmt_stypes          = wal_cfg80211_default_mgmt_stypes;
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
#ifdef _PRE_WLAN_FEATURE_P2P
    wiphy->iface_combinations   = sta_p2p_iface_combinations;
    wiphy->n_iface_combinations = (osal_s32)OAL_ARRAY_SIZE(sta_p2p_iface_combinations);
    wiphy->max_remain_on_channel_duration = 5000;   /* 在信道上停留时间5000 时间单位 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX: 使能驱动监听,使能offchannel发送防止发错信道
       WIPHY_FLAG_HAVE_AP_SME 解决GO Beacon register失败问题 */
    wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX | WIPHY_FLAG_HAVE_AP_SME;
#endif
#if defined(_PRE_PRODUCT_ID_HOST)
    /* 注册支持pno调度扫描能力相关信息 */
    wiphy->max_sched_scan_ssids  = MAX_PNO_SSID_COUNT;
    wiphy->max_match_sets        = MAX_PNO_SSID_COUNT;
    wiphy->max_sched_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif
#endif
#else    /* 非p2p场景下起多vap */
    wiphy->mgmt_stypes          = wal_cfg80211_default_mgmt_stypes;
    wiphy->iface_combinations   = ap_iface_combinations;
    wiphy->n_iface_combinations = (osal_s32)OAL_ARRAY_SIZE(ap_iface_combinations);

#endif
#endif   /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44) */
}

static osal_void wal_cfg_80211_init_wiphy(oal_wiphy_stru *wiphy, osal_u8 dev_id)
{
    wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP);

    wal_cfg_80211_init_p2p_wiphy(wiphy, dev_id);
    wiphy->max_scan_ssids             = WLAN_SCAN_REQ_MAX_SSID;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 5, 7))
    wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    wiphy->max_scan_ie_len = WAL_MAX_SCAN_IE_LEN;
    wiphy->cipher_suites = g_ast_wlan_supported_cipher_suites;
    wiphy->n_cipher_suites = (osal_s32)(sizeof(g_ast_wlan_supported_cipher_suites) / sizeof(osal_u32));

    /* 不使能节能 */
    wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) */

    /* BEGIN roaming failure between different encryption methods ap. */
#if defined (_PRE_WLAN_FEATURE_ROAM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
    /* wifi 驱动上报支持FW_ROAM,关联时(cfg80211_connect)候使用bssid_hint 替代bssid。 */
    wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif
    /* END roaming failure between different encryption methods ap. */

#ifdef _PRE_WLAN_FEATURE_DFS
    wiphy->flags |= 0x200; /* 0x200 offload 标志 */
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    /* linux 3.14 版本升级，管制域重新修改 */
    wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
#elif defined(LINUX_VERSION_CODE) &&  (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    /* 管制域配置 */
    wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#else
    /* linux-2.6.30  管制域配置 */
    wiphy->custom_regulatory = true;
#endif
    wiphy->bands[OAL_IEEE80211_BAND_2GHZ] = &g_st_supported_band_2ghz_info; /* 支持的频带信息 2.4G */
    if (hmac_device_check_5g_enable(dev_id)) {
        wiphy->bands[OAL_IEEE80211_BAND_5GHZ] = &g_st_supported_band_5ghz_info; /* 支持的频带信息 5G */
    }
    wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
#ifdef _PRE_WLAN_FEATURE_WPA3
    wiphy->features |= NL80211_FEATURE_SAE;
#endif
}

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
static const struct wiphy_wowlan_support wowlan = {
    .flags = WIPHY_WOWLAN_ANY | WIPHY_WOWLAN_MAGIC_PKT,
    .n_patterns = 0,
    .pattern_max_len = 0,
    .pattern_min_len = 0,
#if defined (LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    .max_pkt_offset = 0,
#endif
};
#endif
static osal_u32 wal_init_vap(hmac_device_stru *hmac_device, osal_u8 dev_id)
{
    osal_s32 ret;

    /* 当前只支持了一个hmac_device, netdev规格:wlan0 p2p0,如果默认要支持3个,则增加 wlan1或ap0 */
    ret = wal_init_wlan_netdev_etc(hmac_device->wiphy, NL80211_IFTYPE_STATION);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P, "wal_init_vap wlan[%u] failed.return:%d", dev_id, ret);
        /* 释放wlan网络设备资源 */
        oal_mem_free(OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.primary_net_device), OAL_TRUE);
        oal_net_unregister_netdev(hmac_device->st_p2p_info.primary_net_device);
        return (osal_u32)ret;
    }
#if defined(WSCFG_WLAN_INIT_VAP_COUNT) &&(WSCFG_WLAN_INIT_VAP_COUNT >= WLAN_INIT_WLAN0_P2P0_VAP_CNT)
#ifdef _PRE_WLAN_FEATURE_P2P
    ret = wal_init_wlan_netdev_etc(hmac_device->wiphy, NL80211_IFTYPE_P2P_DEVICE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P, "wal_init_vap p2p[%u] failed.return:%d", dev_id, ret);
        /* 释放p2p网络设备资源 */
        oal_mem_free(OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.p2p_net_device), OAL_TRUE);
        oal_net_unregister_netdev(hmac_device->st_p2p_info.p2p_net_device);
        return (osal_u32)ret;
    }
#endif
#endif
#if defined(WSCFG_WLAN_INIT_VAP_COUNT) &&(WSCFG_WLAN_INIT_VAP_COUNT >= WLAN_INIT_WLAN0_WLAN1_P2P0_VAP_CNT)
    ret = wal_init_wlan_netdev_etc(hmac_device->wiphy, NL80211_IFTYPE_AP);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_P2P, "wal_init_vap wlan1 failed.return:%d", dev_id, ret);

        /* 释放p2p网络设备资源 */
        oal_mem_free(OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.second_net_device), OAL_TRUE);
        oal_net_unregister_netdev(hmac_device->st_p2p_info.second_net_device);
        return (osal_u32)ret;
    }
#endif

    return (osal_u32)ret;
}
static osal_u32 wal_cfg80211_init_single_chip(hmac_device_stru *hmac_device, osal_u8 dev_id)
{
    mac_wiphy_priv_stru *wiphy_priv = OAL_PTR_NULL;
    osal_s32 ret;

    hmac_device->wiphy = oal_wiphy_new(&g_wal_cfg80211_ops, OAL_SIZEOF(mac_wiphy_priv_stru));
    if (hmac_device->wiphy == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init_single_chip::oal_wiphy_new failed!}");
        return OAL_FAIL;
    }

    /* 初始化wiphy 结构体内容 */
    wal_cfg_80211_init_wiphy(hmac_device->wiphy, dev_id);

#if defined(_PRE_PRODUCT_ID_HOST) && (defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)))
    wal_cfgvendor_init_etc(hmac_device->wiphy);
#endif
#if defined (_PRE_WLAN_FEATURE_WOW_OFFLOAD) && defined (CONFIG_PM)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
    hmac_device->wiphy->wowlan = &wowlan;
#else
    hmac_device->wiphy->wowlan = wowlan;
#endif
#endif
    oam_warning_log0(0, OAM_SF_P2P, "wiphy_register start.");
    ret = oal_wiphy_register(hmac_device->wiphy);
    if (ret != OAL_SUCC) {
        oal_wiphy_free(hmac_device->wiphy);
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::oal_wiphy_register failed!}");
        return (osal_u32)ret;
    }

    /* P2P add_virtual_intf 传入wiphy 参数，在wiphy priv 指针保存wifi 驱动mac_devie_stru 结构指针 */
    wiphy_priv  = (mac_wiphy_priv_stru *)(oal_wiphy_priv(hmac_device->wiphy));
    wiphy_priv->hmac_device = hmac_device;

    return wal_init_vap(hmac_device, dev_id);
}

/*****************************************************************************
 wal_linux_cfg80211加载初始化
*****************************************************************************/
osal_u32 wal_cfg80211_init_etc(osal_void)
{
    osal_u32           ret;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_init_etc::hmac_res_get_mac_dev_etc,dev null!}");
        return OAL_FAIL;
    }

    ret = wal_cfg80211_init_single_chip(hmac_device, 0);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 卸载wihpy
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年9月5日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void  wal_cfg80211_exit_etc(osal_void)
{
    hmac_device_stru   *hmac_device;
    oal_wireless_dev_stru *wdev;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (OAL_PTR_NULL == hmac_device) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_exit_etc::hmac_res_get_mac_dev_etc device is null!}");
        return;
    }
#if defined(_PRE_PRODUCT_ID_HOST)
    /* 释放wlan网络设备资源 */
    wdev = OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.primary_net_device);
    oal_net_unregister_netdev(hmac_device->st_p2p_info.primary_net_device);
    oal_mem_free(wdev, OAL_TRUE);
    /* 释放p2p网络设备资源 */
    if (hmac_device->st_p2p_info.p2p_net_device != NULL) {
        wdev = OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.p2p_net_device);
        oal_net_unregister_netdev(hmac_device->st_p2p_info.p2p_net_device);
        oal_mem_free(wdev, OAL_TRUE);
    }

    if (hmac_device->st_p2p_info.second_net_device != NULL) {
        wdev = OAL_NETDEVICE_WDEV(hmac_device->st_p2p_info.second_net_device);
        oal_net_unregister_netdev(hmac_device->st_p2p_info.second_net_device);
        oal_mem_free(wdev, OSAL_TRUE);
    }
#endif
#if defined(_PRE_PRODUCT_ID_HOST) && (defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)))
    wal_cfgvendor_deinit_etc(hmac_device->wiphy);
#endif

    /* 注销注册 wiphy device */
    oal_wiphy_unregister(hmac_device->wiphy);

    /* 卸载wiphy device */
    oal_wiphy_free(hmac_device->wiphy);

    return;
}

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
功能描述  : 触发上报m2s事件
输出参数  : 无
返 回 值  : osal_s32

1.日    期   : 2018年2月9日
  修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_cfg80211_m2s_status_report(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_m2s_complete_syn_stru *m2s_comp;
    wlan_m2s_mgr_vap_stru *m2s_vap_mgr;
    osal_u8 vap_idx;

    m2s_comp = (hmac_m2s_complete_syn_stru *)(msg->data);

    /* 需要上层提供oal接口 */
    oam_warning_log4(0, OAM_SF_M2S,
        "{wal_cfg80211_m2s_status_report::vap num[%d]device result[%d]state[%d]m2s_mode[%d].}",
        m2s_comp->vap_num, m2s_comp->m2s_result, m2s_comp->m2s_state, m2s_comp->m2s_mode);

    for (vap_idx = 0; vap_idx < m2s_comp->vap_num; vap_idx++) {
        m2s_vap_mgr = &(m2s_comp->m2s_comp_vap[vap_idx]);

        oam_warning_log3(0, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::Notify to host, ap index[%d]action type[%d]state[%d].}",
            vap_idx, m2s_vap_mgr->action_type, m2s_vap_mgr->m2s_result);

        oam_warning_log4(0, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::Notify to host, ap addr->%02x:%02x:%02x:%02x:XX:XX}",
            m2s_vap_mgr->user_mac_addr[0], m2s_vap_mgr->user_mac_addr[1],
            m2s_vap_mgr->user_mac_addr[2], m2s_vap_mgr->user_mac_addr[3]); /* macaddr第2和第3字节 */
    }

    /* 上报内核 */
    oal_cfg80211_m2s_status_report(hmac_vap->net_device, GFP_KERNEL,
        (osal_u8 *)m2s_comp, OAL_SIZEOF(hmac_m2s_complete_syn_stru));

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
/*****************************************************************************
 功能描述  : 驱动上报WPA,触发SAE认证
 修改历史      :
  1.日    期   : 2020年6月24日
    修改内容   : 新生成函数
*****************************************************************************/
osal_s32 wal_cfg80211_trigger_external_auth(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_external_auth_stru external_auth = {0};
    hmac_external_auth_stru *ext_auth = (hmac_external_auth_stru *)(msg->data);

    if (ext_auth->ssid == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_trigger_external_auth::ssid is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (memcpy_s(external_auth.ssid.ssid, sizeof(external_auth.ssid.ssid), ext_auth->ssid, ext_auth->ssid_len) != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_trigger_external_auth::mem safe function err!}");
        oal_mem_free(ext_auth->ssid, OSAL_TRUE);
        ext_auth->ssid = OSAL_NULL;
        return OAL_FAIL;
    }
    external_auth.ssid.ssid_len = ext_auth->ssid_len;

    if (memcpy_s(external_auth.bssid, WLAN_MAC_ADDR_LEN, ext_auth->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_trigger_external_auth::mem safe function err!}");
        oal_mem_free(ext_auth->ssid, OSAL_TRUE);
        ext_auth->ssid = OSAL_NULL;
        return OAL_FAIL;
    }

    external_auth.key_mgmt_suite = ext_auth->key_mgmt_suite;
    external_auth.status         = ext_auth->status;

    if (external_auth.status == MAC_SUCCESSFUL_STATUSCODE) {
        external_auth.action = NL80211_EXTERNAL_AUTH_START;
    } else {
        external_auth.action = NL80211_EXTERNAL_AUTH_ABORT;
    }
    /* 调用内核接口，触发SAE auth流程 */
    oal_cfg80211_external_auth_req(hmac_vap->net_device, (oal_external_auth_stru *)&external_auth);
    oam_warning_log4(0, OAM_SF_ASSOC,
        "{wal_cfg80211_trigger_external_auth mac[%02X:%02X:%02X:%02X:XX:XX] trigger WPA SAE Auth.}",
        /* 0/1/2/3 MAC地址位数 */
        external_auth.bssid[0], external_auth.bssid[1], external_auth.bssid[2], external_auth.bssid[3]);
    oam_warning_log1(0, OAM_SF_ASSOC, "{wal_cfg80211_trigger_external_auth Action[%d]!}", external_auth.action);
    oal_mem_free(ext_auth->ssid, OSAL_TRUE);
    ext_auth->ssid = OSAL_NULL;
    return OAL_SUCC;
}

osal_s32 wal_cfg80211_report_owe_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device = OAL_PTR_NULL;
    hmac_asoc_user_req_ie_stru *asoc_user_req_info = OAL_PTR_NULL;
    oal_cfg80211_update_owe_info owe_info;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_cfg80211_report_owe_info::get net device ptr is null!}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(msg->data);
    if (memcpy_s(owe_info.peer, sizeof(owe_info.peer),
            (osal_u8 *)asoc_user_req_info->user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_report_owe_info::memcpy_s peer error}");
        return OAL_FAIL;
    }
    owe_info.status = MAC_SUCCESSFUL_STATUSCODE;
    owe_info.ie = asoc_user_req_info->assoc_req_ie_buff;
    owe_info.ie_len = asoc_user_req_info->assoc_req_ie_len;
    oal_cfg80211_update_owe_info_event(net_device, &owe_info, GFP_KERNEL);
    oam_warning_log1(0, OAM_SF_ASSOC, "{wal_cfg80211_report_owe_info::report ie_len=%u}", owe_info.ie_len);

    return OAL_SUCC;
}

#endif

osal_s32 wal_report_csi_msg(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oam_warning_log0(0, OAM_SF_ANY, "{wal_report_csi_msg!}");
    dft_report_netbuf_cb_etc(msg->data, msg->data_len, SOC_DIAG_MSG_ID_WIFI_RX_CSI);
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

