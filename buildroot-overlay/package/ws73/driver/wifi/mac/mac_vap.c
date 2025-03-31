/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of mac vap.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"
#include "mac_ie.h"
#include "mac_resource_ext.h"
#include "hmac_user.h"
#include "hmac_config.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_RTS
#include "alg_rts.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#include "hmac_blacklist.h"
#include "hmac_feature_interface.h"
#include "hmac_11v.h"
#include "common_log_dbg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_VAP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    函数声明
*****************************************************************************/
static void hmac_init_mib_etc(hmac_vap_stru *hmac_vap);
#ifdef _PRE_WLAN_FEATURE_11AX
static void hmac_vap_init_11ax_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
#endif
static void mac_vap_init_mib_11n_rom_cb(hmac_vap_stru *hmac_vap);
static void mac_vap_ch_mib_by_bw_cb(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    wlan_channel_bandwidth_enum_uint8 bandwidth);
static void hmac_vap_init_11n_rates_cb(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
static void hmac_vap_init_11ac_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC void hmac_vap_init_mib_11ax(hmac_vap_stru  *hmac_vap);
#endif
static osal_void hmac_vap_init_bw_by_chip_and_protocol(hmac_vap_stru *hmac_vap);

/*****************************************************************************
    全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu = { 1, 0, 100, 50, 0 }; /* 100与50分别为amsdu与ampdu共同聚合门限 */
#endif
#ifdef _PRE_WLAN_TCP_OPT
mac_tcp_ack_filter_stru g_st_tcp_ack_filter = { 0, 0, 15, 10, 2, 0 }; /* 40与20分别为tcp-ack过滤门限 */
#endif
mac_rx_buffer_size_stru g_st_rx_buffer_size_stru = { 0 };
/* 300、200、25000、5000分别为small_amsdu的rx与pps门限 */
mac_small_amsdu_switch_stru g_st_small_amsdu_switch = { 1, 0, 300, 200, 25000, 5000, 0 };

mac_tcp_ack_buf_switch_stru g_st_tcp_ack_buf_switch = { 1, 0, 100, 30, 0 }; /* 100与30分别为tcp-ack buf门限 */

/*****************************************************************************
    全局变量定义
*****************************************************************************/
const mac_vap_cb g_st_mac_vap_rom_cb = {
    NULL,
    mac_vap_init_mib_11n_rom_cb,
    NULL,
    NULL,
    NULL,
    mac_vap_ch_mib_by_bw_cb, // ch_mib_by_bw_cb
    NULL,            // init_11ac_rates_cb
    hmac_vap_init_11n_rates_cb,
    NULL, // init_privacy_cb
    NULL, // init_rates_by_prot_cb
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_vap_init_11ax_rates,
    NULL,
#else
    NULL,
    NULL,
#endif
};

/* WME初始参数定义，按照OFDM初始化 AP模式 值来自于TGn 9 Appendix D: Default WMM AC Parameters */
/* 修改BE/BK的cwmin的大小由4改为3;此修改影响认证4.2.23&4.2.7&4.2.8,同频干扰由抗干扰动态edca调整算法实现,现由3改为4 */
const mac_wme_param_stru g_ast_wmm_initial_params_ap[WLAN_WME_AC_BUTT] = {
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          4,     6,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        7,          4,     10,    0,
    },

    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        1,          3,     4,     3008,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        1,          2,     3,     1504,
    },
};

/* WMM初始参数定义，按照OFDM初始化 STA模式 */
const mac_wme_param_stru g_ast_wmm_initial_params_sta[WLAN_WME_AC_BUTT] = {
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          3,     10,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        7,          4,     10,     0,
    },

    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          3,     4,     3008,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          2,     3,     1504,
    },
};

/* WMM初始参数定义，aput建立的bss中STA的使用的EDCA参数 */
const mac_wme_param_stru g_ast_wmm_initial_params_bss[WLAN_WME_AC_BUTT] = {
    /* BE */
    {3, 4, 10, 0},

    /* BK */
    {7, 4, 10, 0},

    /* VI */
    {2, 3, 4, 3008},

    /* VO */
    {2, 2, 3, 1504},
};
/*****************************************************************************
    函数实现
*****************************************************************************/
mac_tcp_ack_buf_switch_stru *mac_get_pst_tcp_ack_buf_switch(osal_void)
{
    return &g_st_tcp_ack_buf_switch;
}

mac_small_amsdu_switch_stru *mac_get_pst_small_amsdu_switch(osal_void)
{
    return &g_st_small_amsdu_switch;
}

mac_rx_buffer_size_stru *mac_get_pst_rx_buffer_size_stru(osal_void)
{
    return &g_st_rx_buffer_size_stru;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述 : 初始化p2p dev, gc的 11ax 特殊mib能力位信息
*****************************************************************************/
OAL_STATIC void hmac_vap_init_p2p_dev_gc_spec_mib_11ax(hmac_vap_stru *hmac_vap)
{
    /* 11ax接口定制特性 -- 关闭gc的trigger相关能力位,只有station具备trigger能力, p2p dev的也要关闭 */
    if (!is_sta(hmac_vap)) {
        return;
    }

    if ((is_p2p_cl(hmac_vap)) || (is_p2p_dev(hmac_vap))) {
        mac_mib_set_he_trigger_mac_padding_duration(hmac_vap, 0);
        mac_mib_set_he_bsr_control_implemented(hmac_vap, OSAL_FALSE);
        mac_mib_set_he_ht_vht_trigger_option_implemented(hmac_vap, OSAL_FALSE);
        mac_mib_set_he_num_sounding_dims_below_80mhz(hmac_vap, 0);
        mac_mib_set_he_ng16_mufeedback_support(hmac_vap, OSAL_FALSE);
        mac_mib_set_he_codebooksize_mufeedback_support(hmac_vap, OSAL_FALSE);
        mac_mib_set_he_partial_bw_dlmumimo_implemented(hmac_vap, OSAL_FALSE);
    } else {
        mac_mib_set_he_bsr_control_implemented(hmac_vap, OSAL_TRUE);
        mac_mib_set_he_ht_vht_trigger_option_implemented(hmac_vap, OSAL_TRUE);
        mac_mib_set_he_num_sounding_dims_below_80mhz(hmac_vap, 1);
        mac_mib_set_he_ng16_mufeedback_support(hmac_vap, hmac_vap->ng16_mu_feedback);
        mac_mib_set_he_codebooksize_mufeedback_support(hmac_vap, OSAL_TRUE);
        mac_mib_set_he_partial_bw_dlmumimo_implemented(hmac_vap, OSAL_TRUE);
    }
}

/*****************************************************************************
 功能描述 : 初始化11ax mib信息
*****************************************************************************/
OAL_STATIC void hmac_vap_init_mib_11ax(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_11AX, "hmac_vap_init_mib_11ax::mac_dev[%d] null.",
            hmac_vap->device_id);

        return;
    }

    mac_mib_set_he_ldpc_coding_in_payload(hmac_vap, mac_device_get_cap_ldpc(hmac_device)); /* 支持LDPC编码 */
    mac_mib_set_he_su_beamformer(hmac_vap, mac_device_get_cap_subfer(hmac_device));
    mac_mib_set_he_su_beamformee(hmac_vap, mac_device_get_cap_subfee(hmac_device));
    mac_mib_set_he_mu_beamformer(hmac_vap, mac_device_get_cap_mubfer(hmac_device));
    mac_mib_set_he_rx_stbc_beamformer(hmac_vap, mac_device_get_cap_rxstbc(hmac_device));

    /* 11ax接口定制特性 -- 关闭gc的trigger相关能力位,只有station具备trigger能力, p2p dev的也要关闭 */
    hmac_vap_init_p2p_dev_gc_spec_mib_11ax(hmac_vap);
    /* 11ax接口定制特性 -- STA和SoftAP的PPE字段都为1 */
    mac_mib_set_ppe_thresholds_required(hmac_vap, OSAL_TRUE);
    if (!is_legacy_sta(hmac_vap)) {
        mac_mib_set_he_oper_bss_color_disable(hmac_vap, OSAL_TRUE);
        return;
    }

    /* MAC Capabilities Info */
    mac_mib_set_he_option_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_he_ht_control_field_supported(hmac_vap, OAL_TRUE);
    mac_mib_set_he_twt_option_activated(hmac_vap, OSAL_FALSE);
    mac_mib_set_he_trigger_mac_padding_duration(hmac_vap, MAC_TRIGGER_FRAME_PADDING_DURATION16US); /* 16us */
    mac_mib_set_he_max_ampdu_length(hmac_vap, 0);                                                 /* 0=与VHT相同 */

    /* PHY Capabilities Info */
    if (hmac_device->band_cap == WLAN_BAND_CAP_2G_5G) {
        mac_mib_set_he_DualBandSupport(hmac_vap, OAL_TRUE); /* 支持双频 */
    }

    mac_mib_set_trs_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_he_psr_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_support_response_nfrp_trigger(hmac_vap, OAL_FALSE);
    mac_mib_set_he_mtid_aggr_rx_support(hmac_vap, mac_device_get_cap_rx_mtid(hmac_device));
    mac_mib_set_ba_32bit_bitmap_implemented(hmac_vap, mac_device_get_cap_32bitmap(hmac_device));
}

/*****************************************************************************
 功能描述 : 初始化11ax mib中mcs信息
*****************************************************************************/
OAL_STATIC void mac_vap_init_11ax_mcs_singlenss(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_tx_max_he_mcs_map_stru *tx_max_mcs_map;
    mac_tx_max_he_mcs_map_stru *rx_max_mcs_map;

    /* 获取mib值指针 */
    rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(hmac_vap));
    tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(hmac_vap));

    if (bandwidth == WLAN_BAND_WIDTH_20M) {
        /* 20MHz带宽的情况下，支持MCS0-MCS7 ==0 限制MCS */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    } else if ((bandwidth == WLAN_BAND_WIDTH_40MINUS) || (bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS7 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    } else if ((bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) && (bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS7 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    }
}

/*****************************************************************************
 功能描述 : 初始化11ax mib中mcs信息
*****************************************************************************/
OAL_STATIC void mac_vap_init_11ax_mcs_doublenss(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_tx_max_he_mcs_map_stru *tx_max_mcs_map;
    mac_rx_max_he_mcs_map_stru *rx_max_mcs_map;

    /* 获取mib值指针 */
    rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(hmac_vap));
    tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(hmac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS7 限制MCS */
    if (bandwidth == WLAN_BAND_WIDTH_20M) {
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    } else if ((bandwidth == WLAN_BAND_WIDTH_40MINUS) || (bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS7 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AX);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AX);
    } else if ((bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) && (bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS7 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AX);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AX);
    }
}

/*****************************************************************************
功能描述 : 初始化11x速率
*****************************************************************************/
static void hmac_vap_init_11ax_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
    mac_mib_set_he_rx_mcs_map(hmac_vap, 0xFFFF);
    mac_mib_set_he_tx_mcs_map(hmac_vap, 0xFFFF);

    if (mac_device_get_nss_num(hmac_device) == WLAN_SINGLE_NSS) {
        /* 1个空间流的情况 */
        mac_vap_init_11ax_mcs_singlenss(hmac_vap, hmac_vap->channel.en_bandwidth);
    } else if (mac_device_get_nss_num(hmac_device) == WLAN_DOUBLE_NSS) {
        /* 2个空间流的情况 */
        mac_vap_init_11ax_mcs_doublenss(hmac_vap, hmac_vap->channel.en_bandwidth);
    } else {
        oam_error_log1(0, OAM_SF_11AX, "hmac_vap_init_11ax_rates::invalid nss_num[%d].",
            mac_device_get_nss_num(hmac_device));
    }
}
#endif

/*****************************************************************************
 功能描述 : rom回调初始化11n的mib
*****************************************************************************/
static void mac_vap_init_mib_11n_rom_cb(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_trans_stagger_sounding_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_receive_stagger_sounding_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_compressed_bf_feedback_option_implemented(hmac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
    mac_mib_set_num_compressed_bf_matrix_support_antenna(hmac_vap, 1);
}

static void hmac_vap_init_11n_rates_cb(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    unref_param(hmac_vap);
    unref_param(hmac_device);
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((hmac_vap->protocol == WLAN_HT_MODE) && (hmac_vap->cap_flag.support_11ac2g == OAL_TRUE) &&
        (hmac_vap->channel.band == WLAN_BAND_2G)) {
        hmac_vap_init_11ac_rates(hmac_vap, hmac_device);
    }
#endif
}

/*****************************************************************************
 功能描述 : rom回调初始化40m使能mib值
*****************************************************************************/
static void mac_vap_ch_mib_by_bw_cb(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
#ifndef WIN32
    oal_bool_enum_uint8 val;

    /* 此时vap的频段信息还未赋值，因此不能用mac_mib_set_FortyMHzOperationImplemented设置40M使能MIB值 */
    val = (bandwidth != WLAN_BAND_WIDTH_20M);

    if (band == WLAN_BAND_2G) {
        mac_mib_set_2g_forty_mhz_operation_implemented(hmac_vap, val);
    } else {
        mac_mib_set_5g_forty_mhz_operation_implemented(hmac_vap, val);
    }
#endif
}

static osal_void mac_vap_init_base(hmac_vap_stru *hmac_vap, osal_u8 chip_id, osal_u8 device_id, osal_u8 vap_id,
    const hmac_cfg_add_vap_param_stru *param)
{
    hmac_vap->chip_id = chip_id;
    hmac_vap->device_id = device_id;
    hmac_vap->vap_id = vap_id;
    hmac_vap->vap_mode = param->add_vap.vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap->p2p_mode = param->add_vap.p2p_mode; /* mib初始化时需要使用,原代码 hmac_device_set_vap_id_etc才会赋值,故提前一下 */
#endif

    hmac_vap->voice_aggr = OAL_FALSE;
    hmac_vap->random_mac = OAL_FALSE;
    hmac_vap->bw_fixed = OAL_FALSE;
    hmac_vap->beamformee_sts_below_80mhz = VHT_BFEE_NTX_SUPP_STS_CAP - 1;
    hmac_vap->ng16_su_feedback = 1;
    hmac_vap->ng16_mu_feedback = 1;

    oal_set_mac_addr_zero(hmac_vap->bssid);

    /* cache user 锁初始化 */
    osal_spin_lock_init(&hmac_vap->cache_user_lock);

    OSAL_INIT_LIST_HEAD(&hmac_vap->mac_user_list_head);

    /* 初始化支持2.4G 11ac私有增强 */
#ifdef _PRE_WLAN_FEATURE_11AC2G
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_vap->cap_flag.support_11ac2g = param->add_vap.is_11ac2g_enable;
#else
    hmac_vap->cap_flag.support_11ac2g = OAL_TRUE;
#endif
#endif
    /* 默认APUT不支持随环境进行自动2040带宽切换 */
    hmac_vap->cap_flag.autoswitch_2040 = OAL_FALSE;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* 根据定制化刷新2g ht40能力 */
    hmac_vap->cap_flag.disable_2ght40 = param->add_vap.disable_capab_2ght40;
    hmac_vap->er_su_disable = param->add_vap.er_su_disable;
    hmac_vap->dcm_constellation_tx = param->add_vap.dcm_constellation_tx;
    hmac_vap->bandwidth_extended_range = param->add_vap.bandwidth_extended_range;
#else
    hmac_vap->cap_flag.disable_2ght40 = OAL_FALSE;
    hmac_vap->er_su_disable = 0;
    hmac_vap->dcm_constellation_tx = 0x3; /* 0:不支持DCM,1:BPSK,2:QPSK,3:16-QAM */
    hmac_vap->bandwidth_extended_range = 1;
#endif
}
/*****************************************************************************
 功能描述 : 初始化vap 特性能力标识
*****************************************************************************/
static osal_void mac_vap_init_feature_cap(hmac_vap_stru *hmac_vap)
{
    /* 初始化特性标识 */
    hmac_vap->cap_flag.dsss_cck_mode_40mhz = OAL_TRUE;

    /* 初始化特性标识 */
    hmac_vap->cap_flag.uapsd = OSAL_FALSE;

    /* 初始化dpd能力 */
    hmac_vap->cap_flag.dpd_enbale = OAL_TRUE;

    hmac_vap->cap_flag.dpd_done = OAL_FALSE;
    /* 初始化TDLS prohibited关闭 */
    hmac_vap->cap_flag.tdls_prohibited = OAL_FALSE;
    /* 初始化TDLS channel switch prohibited关闭 */
    hmac_vap->cap_flag.tdls_channel_switch_prohibited = OAL_FALSE;

    /* 初始化KeepALive开关 */
    hmac_vap->cap_flag.keepalive = OAL_TRUE;
    /* 初始化安全特性值 */
    hmac_vap->cap_flag.wpa = OAL_FALSE;
    hmac_vap->cap_flag.wpa2 = OAL_FALSE;

    hmac_vap_set_peer_obss_scan_etc(hmac_vap, OAL_FALSE);
}
static osal_void mac_vap_init_vap_mode_bss(hmac_vap_stru *hmac_vap)
{
    /* 设置vap参数默认值 */
    hmac_vap->assoc_vap_id = MAC_INVALID_USER_ID;
    hmac_vap->cache_user_id = MAC_INVALID_USER_ID;
    hmac_vap->tx_power = WLAN_MAX_TXPOWER;
    hmac_vap->protection.protection_mode = WLAN_PROT_NO;

    /* 初始化协议模式与带宽为非法值，需通过配置命令配置 */
    hmac_vap->channel.band = WLAN_BAND_BUTT;
    hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_BUTT;
    hmac_vap->channel.chan_number = 0;
    hmac_vap->protocol = WLAN_PROTOCOL_BUTT;
    hmac_vap->max_protocol = WLAN_PROTOCOL_MODE_BUTT;

    /* 设置自动保护开启 */
    hmac_vap->protection.auto_protection = OAL_SWITCH_ON; /* 设置自动保护开启 */
    mac_vap_init_feature_cap(hmac_vap);

    memset_s(hmac_vap->app_ie, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM,
        0, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM);

    /* 设置初始化rx nss值,之后按协议初始化 */
    hmac_vap->vap_rx_nss = WLAN_NSS_LIMIT;

    /* 创建vap时 初始状态为init */
    hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_INIT);
}

static osal_u32 mac_vap_init_protocol_and_cap(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_device_capability_stru *device_capability = mac_device_get_capability();
#endif

    switch (hmac_device->protocol_cap) {
        case WLAN_PROTOCOL_CAP_LEGACY:
        case WLAN_PROTOCOL_CAP_HT:
            hmac_vap->protocol = WLAN_HT_MODE;
            hmac_vap->max_protocol = WLAN_PROTOCOL_HT_MODE;
            break;

        case WLAN_PROTOCOL_CAP_VHT:
            hmac_vap->protocol = WLAN_VHT_MODE;
            hmac_vap->max_protocol = WLAN_PROTOCOL_VHT_MODE;
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_PROTOCOL_CAP_HE:
            hmac_vap->protocol = (device_capability[0].switch_11ax == OAL_TRUE) ? WLAN_HE_MODE :
#ifdef _PRE_WLAN_FEATURE_11AC2G
                WLAN_VHT_MODE;
#else
                WLAN_HT_MODE;
#endif
            hmac_vap->max_protocol = (device_capability[0].switch_11ax == OAL_TRUE) ? WLAN_PROTOCOL_HE_MODE :
#ifdef _PRE_WLAN_FEATURE_11AC2G
                WLAN_PROTOCOL_VHT_MODE;
#else
                WLAN_PROTOCOL_HT_MODE;
#endif
            break;
#endif

        default:
            oam_warning_log1(0, OAM_SF_CFG, "mac_vap_init:protocol_cap[%d] not support", hmac_device->protocol_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}

static osal_u32 mac_vap_init_protocol_band_cap(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
    if (osal_unlikely(mac_vap_init_protocol_and_cap(hmac_vap, hmac_device) != OAL_SUCC)) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    switch (mac_device_get_cap_bw(hmac_device)) {
        case WLAN_BW_CAP_20M:
            mac_vap_set_cap_bw(hmac_vap, WLAN_BAND_WIDTH_20M);
            break;

        case WLAN_BW_CAP_40M:
            mac_vap_set_cap_bw(hmac_vap, WLAN_BAND_WIDTH_40MINUS);
            break;

        case WLAN_BW_CAP_80M:
            mac_vap_set_cap_bw(hmac_vap, WLAN_BAND_WIDTH_80PLUSMINUS);
            break;

        default:
            oam_error_log1(0, OAM_SF_CFG,
                "mac_vap_init_etc::bandwidth_cap[%d] is not supportted.", mac_device_get_cap_bw(hmac_device));
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    switch (hmac_device->band_cap) {
        case WLAN_BAND_CAP_2G:
            hmac_vap->channel.band = WLAN_BAND_2G;
            break;

        case WLAN_BAND_CAP_5G:
        case WLAN_BAND_CAP_2G_5G:
            hmac_vap->channel.band = WLAN_BAND_5G;
            break;

        default:
            oam_warning_log1(0, OAM_SF_CFG, "mac_vap_init_etc:band_cap[%d] not supportted", hmac_device->band_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_P2P
static osal_u32 mac_vap_init_p2p_protocol_band_cap(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (hmac_get_p2p_protocol_info() == P2P_PROTOCOL_AX) {
        hmac_vap->protocol = WLAN_HE_MODE;
        hmac_vap->max_protocol = WLAN_PROTOCOL_HE_MODE;
    } else {
#endif
        hmac_vap->protocol = WLAN_HT_MODE;
        hmac_vap->max_protocol = WLAN_PROTOCOL_HT_MODE;
#ifdef _PRE_WLAN_FEATURE_11AX
    }
#endif

    if (hmac_get_p2p_common_band_info() == P2P_BAND_20M) {
        mac_vap_set_cap_bw(hmac_vap, WLAN_BAND_WIDTH_20M);
        mac_mib_set_dot11_vap_max_bandwidth(hmac_vap, WLAN_BW_CAP_20M);
    } else {
        mac_vap_set_cap_bw(hmac_vap, WLAN_BAND_WIDTH_40MINUS);
        mac_mib_set_dot11_vap_max_bandwidth(hmac_vap, WLAN_BW_CAP_40M);
    }

    switch (hmac_device->band_cap) {
        case WLAN_BAND_CAP_2G:
            hmac_vap->channel.band = WLAN_BAND_2G;
            break;

        case WLAN_BAND_CAP_5G:
        case WLAN_BAND_CAP_2G_5G:
            hmac_vap->channel.band = WLAN_BAND_5G;
            break;

        default:
            oam_warning_log1(0, OAM_SF_CFG,
                "mac_vap_init_p2p_protocol_band_cap:band_cap[%d] not supportted", hmac_device->band_cap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    return OAL_SUCC;
}
#endif

static osal_void mac_vap_init_txbf_cap(hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#if defined(_PRE_PRODUCT_ID_HOST)
        /* 从定制化获取是否开启AMSDU_AMPDU */
        mac_mib_set_AmsduPlusAmpduActive(hmac_vap, g_st_tx_large_amsdu.host_large_amsdu_en);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        hmac_vap->cap_flag.opmode = 1;
#endif
}

static osal_u32 mac_vap_init_mode(hmac_vap_stru *hmac_vap)
{
    switch (hmac_vap->vap_mode) {
        case WLAN_VAP_MODE_CONFIG:
            hmac_vap->init_flag = MAC_VAP_VAILD; // CFG VAP也需置位保证不重复初始化
            osal_adapt_atomic_set(&(hmac_vap->use_cnt), 1);
            return OAL_SUCC;
        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_BSS_AP:
            mac_vap_init_vap_mode_bss(hmac_vap);
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "mac_vap_init_mode::invalid vap mode[%d].", hmac_vap->vap_mode);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_CONTINUE;
}

static osal_u32 mac_vap_init_protocol_band(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device, hmac_cfg_add_vap_param_stru *param)
{
    unref_param(param);
    /* 开关打开并且是P2P模式时，需要重新设置P2P的协议模式和带宽 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((param->add_vap.p2p_mode == WLAN_P2P_CL_MODE) && (hmac_get_p2p_common_enable_info() == 1)) {
        return mac_vap_init_p2p_protocol_band_cap(hmac_vap, hmac_device);
    }
#endif
    return mac_vap_init_protocol_band_cap(hmac_vap, hmac_device);
}

/*****************************************************************************
 功能描述 : mac vap init
*****************************************************************************/
osal_u32 mac_vap_init_etc(hmac_vap_stru *hmac_vap, osal_u8 chip_id, osal_u8 device_id, osal_u8 vap_id,
    hmac_cfg_add_vap_param_stru *param)
{
    osal_u8 *addr;
    osal_u32 ret;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_device_capability_stru *mac_device_capability = OSAL_NULL;
#endif

    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "mac_vap_init_etc::hmac_device[%d] null!", device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_init_base(hmac_vap, chip_id, device_id, vap_id, param);

    ret = mac_vap_init_mode(hmac_vap);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 申请MIB内存空间 */
    hmac_vap->mib_info = oal_mem_alloc(OAL_MEM_POOL_ID_MIB, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), OAL_TRUE);
    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "mac_vap_init_etc::pst_mib_info alloc null, size[%d].",
            OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(hmac_vap->mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), 0, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));

    /* 设置mac地址 */
    addr = mac_mib_get_station_id(hmac_vap);
    oal_set_mac_addr(addr, hmac_device->hw_addr);
    addr[WLAN_MAC_ADDR_LEN - 1] += vap_id;

    /* 初始化mib值 */
    hmac_init_mib_etc(hmac_vap);

    mac_vap_init_txbf_cap(hmac_vap);

    /* sta以最大能力启用 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 初始化sta协议模式为11ax */
        if (mac_vap_init_protocol_band(hmac_vap, hmac_device, param) != OAL_SUCC) {
            hmac_vap_free_mib_etc(hmac_vap);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT; // mem free?
        }

        if (hmac_vap_init_by_protocol_etc(hmac_vap, hmac_vap->protocol) != OAL_SUCC) {
            hmac_vap_free_mib_etc(hmac_vap);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
        hmac_vap_init_rates_etc(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_11AX
        mac_device_capability = mac_device_get_capability();
        hmac_vap->custom_switch_11ax = mac_device_capability[0].switch_11ax;
#endif
    }

    osal_adapt_atomic_set(&(hmac_vap->use_cnt), 1);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置vap工作在新的工作带宽
*****************************************************************************/
osal_u8 hmac_vap_set_bw_check(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth)
{
    wlan_channel_bandwidth_enum_uint8 band_with_sta_old;
    osal_u8 change;

    band_with_sta_old = mac_vap_get_cap_bw(hmac_vap);
    mac_vap_set_cap_bw(hmac_vap, sta_new_bandwidth);

    /* 判断是否需要通知硬件切换带宽 */
    change = (band_with_sta_old == sta_new_bandwidth) ? MAC_NO_CHANGE : MAC_BW_CHANGE;

    oam_warning_log2(0, OAM_SF_ASSOC, "hmac_vap_set_bw_check::bw[%d]->[%d].", band_with_sta_old,
        sta_new_bandwidth);

    if (change == MAC_BW_CHANGE) {
        hmac_vap_sync(hmac_vap);
    }
    return change;
}

/*****************************************************************************
 功能描述 : 获取ap或者sta的配置指针
*****************************************************************************/
mac_wme_param_stru *hmac_get_wmm_cfg_etc(wlan_vap_mode_enum_uint8 vap_mode)
{
    /* 参考认证项配置，没有按照协议配置，WLAN_VAP_MODE_BUTT表示是ap广播给sta的edca参数 */
    if (vap_mode == WLAN_VAP_MODE_BUTT) {
        return (mac_wme_param_stru *)g_ast_wmm_initial_params_bss;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return (mac_wme_param_stru *)g_ast_wmm_initial_params_ap;
    }
    return (mac_wme_param_stru *)g_ast_wmm_initial_params_sta;
}

/*****************************************************************************
 功能描述 : 设置StationID值，即mac地址
*****************************************************************************/
osal_u32 mac_mib_set_station_id_etc(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param)
{
    mac_cfg_staion_id_param_stru *param_temp;
    unref_param(len);
    param_temp = (mac_cfg_staion_id_param_stru *)param;

    oal_set_mac_addr(mac_mib_get_station_id(hmac_vap), param_temp->station_id);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置bss type mib值
*****************************************************************************/
osal_u32 hmac_mib_set_bss_type_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param)
{
    osal_s32 value;
    unref_param(len);
    value = *((osal_s32 *)param);

    mac_mib_set_desired_bss_type(hmac_vap, (osal_u8)value);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 读取bss type mib值
*****************************************************************************/
osal_u32 hmac_mib_get_bss_type_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param)
{
    *((osal_s32 *)param) = mac_mib_get_desired_bss_type(hmac_vap);

    *len = OAL_SIZEOF(osal_s32);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置ssid mib值
*****************************************************************************/
osal_u32 hmac_mib_set_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param)
{
    mac_cfg_ssid_param_stru *param_temp;
    osal_u8 ssid_len;
    osal_u8 *mib_ssid;
    unref_param(len);
    param_temp = (mac_cfg_ssid_param_stru *)param;
    ssid_len = param_temp->ssid_len; /* 长度不包括字符串结尾'\0' */
#ifdef _PRE_WLAN_FEATURE_DFR
    hmac_dfr_fill_ap_recovery_info(WLAN_MSG_W2H_CFG_SSID, param, hmac_vap);
#endif
    if (ssid_len > WLAN_SSID_MAX_LEN - 1) {
        ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    mib_ssid = mac_mib_get_desired_ssid(hmac_vap);

    (osal_void)memcpy_s(mib_ssid, ssid_len, param_temp->ac_ssid, ssid_len);
    mib_ssid[ssid_len] = '\0';

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 读取ssid mib值
*****************************************************************************/
osal_u32 hmac_mib_get_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param)
{
    mac_cfg_ssid_param_stru *param_temp;
    osal_u8 ssid_len;
    osal_u8 *mib_ssid;

    mib_ssid = mac_mib_get_desired_ssid(hmac_vap);
    ssid_len = (osal_u8)osal_strlen((osal_char *)mib_ssid);

    param_temp = (mac_cfg_ssid_param_stru *)param;

    param_temp->ssid_len = ssid_len;
    (osal_void)memcpy_s(param_temp->ac_ssid, ssid_len, mib_ssid, ssid_len);

    *len = OAL_SIZEOF(mac_cfg_ssid_param_stru);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置dtim period的值
*****************************************************************************/
osal_u32 hmac_mib_set_dtim_period_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param)
{
    osal_s32 value;
    unref_param(len);
    value = *((osal_s32 *)param);

    mac_mib_set_dot11dtimperiod(hmac_vap, (osal_u32)value);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 读取dtim period的值
*****************************************************************************/
osal_u32 hmac_mib_get_dtim_period_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param)
{
    *((osal_u32 *)param) = mac_mib_get_dot11_dtim_period(hmac_vap);

    *len = OAL_SIZEOF(osal_u32);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置short preamble MIB值
*****************************************************************************/
osal_u32 hmac_mib_set_shpreamble_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param)
{
    osal_s32 value;
    unref_param(len);
    value = *((osal_s32 *)param);

    if (value != 0) {
        mac_mib_set_short_preamble_option_implemented(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_short_preamble_option_implemented(hmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 获取short preamble MIB值
*****************************************************************************/
osal_u32 hmac_mib_get_shpreamble_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param)
{
    osal_s32 value;

    value = mac_mib_get_short_preamble_option_implemented(hmac_vap);

    *((osal_s32 *)param) = value;

    *len = OAL_SIZEOF(value);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 检查dmac list的检查
*****************************************************************************/
static osal_u32 hmac_vap_user_exist_etc(const struct osal_list_head *new_code, struct osal_list_head *head)
{
    struct osal_list_head *user_list_head;
    struct osal_list_head *member_entry;

    osal_list_for_each_safe(member_entry, user_list_head, head) {
        if (new_code == member_entry) {
            oam_error_log0(0, OAM_SF_ASSOC, "{oal_dlist_check_head:dmac user doule add.}");
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述 : 增加关联用户,获取hash值并加入hash表中. vap:对象内存指针,以及user对象
            指针(user对象需要在调用此函数前申请并赋值)
*****************************************************************************/
osal_u32 hmac_vap_add_assoc_user_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    struct osal_list_head *dlist_head;
    osal_ulong irq_save;

    if (hmac_vap_user_exist_etc(&(hmac_user->user_dlist), &(hmac_vap->mac_user_list_head)) == OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_vap_add_assoc_user_etc::user[%d] already exist.}", hmac_user->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 加入双向链表表头 */
    dlist_head = &(hmac_vap->mac_user_list_head);
    osal_list_add(&(hmac_user->user_dlist), dlist_head);
    osal_spin_lock_irqsave(&hmac_vap->cache_user_lock, &irq_save);

    /* 更新cache user */
    oal_set_mac_addr(hmac_vap->cache_user_mac_addr, hmac_user->user_mac_addr);
    hmac_vap->cache_user_id = hmac_user->assoc_id;

    osal_spin_unlock_irqrestore(&hmac_vap->cache_user_lock, &irq_save);

    /* 记录STA模式下的与之关联的VAP的id */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_set_assoc_id_etc(hmac_vap, hmac_user->assoc_id);
    }

    /* vap已关联 user个数++ */
    hmac_vap->user_nums++;

    return OAL_SUCC;
}

/* 将用户从双向链表和hash表中删除 */
static osal_u32 mac_vap_del_user_dlist_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret = OAL_FAIL;
    hmac_user_stru *user_temp = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *dlist_tmp = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    osal_u8 txop_ps_user_cnt = 0;
#endif

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        user_temp = (hmac_user_stru *)osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (user_temp == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ASSOC, "{hmac_vap_del_user_etc:user_temp null,user_idx[%d]}", hmac_user->assoc_id);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_TXOPPS
        if (user_temp->vht_hdl.vht_txop_ps) {
            txop_ps_user_cnt++;
        }
#endif

        if (oal_compare_mac_addr(hmac_user->user_mac_addr, user_temp->user_mac_addr) == 0) {
            /* 从双向链表中拆掉 */
            osal_list_del(&(hmac_user->user_dlist));

            ret = OAL_SUCC;

            /* 初始化相应成员 */
            hmac_user->is_multi_user = OAL_FALSE;
            memset_s(hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
            hmac_user->vap_id = 0xff;
            hmac_user->device_id = 0xff;
            hmac_user->chip_id = 0xff;
            hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
        }
    }
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 没有关联的用户则去初始化vap能力 */
    if (txop_ps_user_cnt == 0) {
        hmac_vap_set_txopps(hmac_vap, OAL_FALSE);
    }
#endif

    return ret;
}

/*****************************************************************************
 功能描述 : 删除用户,将用户从双向链表和hash表中删除. vap:对象指针,以及user对象指
            针(user对象需要在调用此函数前找到,并且此函数不负责释放user内存,由删除
            者调用USER_DESTORY函数释放内存)
*****************************************************************************/
osal_u32 hmac_vap_del_user_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret = OAL_FAIL;
    osal_ulong irq_save;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_vap_del_user_etc::hmac_vap null,user_idx is %d}", hmac_user->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    osal_spin_lock_irqsave(&hmac_vap->cache_user_lock, &irq_save);

    /* 与cache user id对比,相等则清空cache user */
    if (hmac_user->assoc_id == hmac_vap->cache_user_id) {
        oal_set_mac_addr_zero(hmac_vap->cache_user_mac_addr);
        /* 解决用户已删除，cache id和mac addr均为0，导致发送完成中断下半部依旧可以找到已删除的用户 */
        hmac_vap->cache_user_id = MAC_INVALID_USER_ID;
    }

    ret = mac_vap_del_user_dlist_etc(hmac_vap, hmac_user);

    osal_spin_unlock_irqrestore(&hmac_vap->cache_user_lock, &irq_save);

    if (ret == OAL_SUCC) {
        /* vap已关联 user个数-- */
        if (hmac_vap->user_nums != 0) {
            hmac_vap->user_nums--;
        }
        /* STA模式下将关联的VAP的id置为非法值 */
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_vap_set_assoc_id_etc(hmac_vap, MAC_INVALID_USER_ID);
        }
        /* 当前GO与SoftAp不能共存，当Ap类VAP的最后一个下挂用户下线时，直接关闭keepalive定时器，避免空转 */
        if (hmac_vap->user_nums == 0 && hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP &&
            hmac_device->keepalive_timer.is_registerd == OSAL_TRUE) {
            oam_warning_log0(0, OAM_SF_KEEPALIVE,
                "{hmac_vap_del_user_etc::GO/SoftAp's all users are offline,destroy keepalive timer}");
            frw_destroy_timer_entry(&(hmac_device->keepalive_timer));
        }
        return OAL_SUCC;
    }

    oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_vap_del_user_etc::del user failed,user idx[%d].}", hmac_user->assoc_id);

    return OAL_FAIL;
}

/* 根据user mac hash值查找user对象 */
static osal_void mac_vap_find_user_by_macaddr_etc(hmac_vap_stru *hmac_vap, const osal_u8 *sta_mac_addr,
    osal_u16 *user_idx)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
        hmac_user = (hmac_user_stru *)osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            oam_error_log0(0, OAM_SF_ANY,
                "{hmac_vap_find_user_by_macaddr_etc::hmac_user null}");
            continue;
        }

        /* 相同的MAC地址 */
        if (oal_compare_mac_addr(hmac_user->user_mac_addr, sta_mac_addr) == 0) {
            *user_idx = hmac_user->assoc_id;
            /* 更新cache user */
            oal_set_mac_addr(hmac_vap->cache_user_mac_addr, hmac_user->user_mac_addr);
            hmac_vap->cache_user_id = hmac_user->assoc_id;
        }
    }
}

/*****************************************************************************
 功能描述 : 根据user MAC地址查找user对象
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_vap_find_user_by_macaddr_etc(hmac_vap_stru *hmac_vap,
    const osal_u8 *sta_mac_addr, osal_u16 *user_idx)
{
    hmac_user_stru *hmac_user;
    osal_ulong irq_save;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (sta_mac_addr == OAL_PTR_NULL) || ((user_idx == OAL_PTR_NULL)))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_find_user_by_macaddr_etc::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.本函数*pus_user_idx先初始化为无效值，防止调用者没有初始化，可能出现使用返回值异常;
       2.根据查找结果刷新*pus_user_idx值，如果是有效，返回SUCC,无效MAC_INVALID_USER_ID返回FAIL;
       3.调用函数根据首先根据本函数返回值做处理，其次根据*pus_user_idx进行其他需要的判断和操作 */
    *user_idx = MAC_INVALID_USER_ID;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user == OAL_PTR_NULL) {
            return OAL_FAIL;
        }

        if (oal_compare_mac_addr(hmac_user->user_mac_addr, sta_mac_addr) == 0) {
            *user_idx = hmac_vap->assoc_vap_id;
        }
    } else {
        osal_spin_lock_irqsave(&hmac_vap->cache_user_lock, &irq_save);
        /* 与cache user对比 , 相等则直接返回cache user id */
        if (oal_compare_mac_addr(hmac_vap->cache_user_mac_addr, sta_mac_addr) == 0) {
            /* 用户删除后，user macaddr和cache user macaddr地址均为0，但实际上用户已经删除，此时user id无效 */
            *user_idx = hmac_vap->cache_user_id;
        } else {
            mac_vap_find_user_by_macaddr_etc(hmac_vap, sta_mac_addr, user_idx);
        }

        osal_spin_unlock_irqrestore(&hmac_vap->cache_user_lock, &irq_save);
    }

    /* user id有效,返回SUCC给调用者处理;user id无效,返回user id为MAC_INVALID_USER_ID,并返回查找结果FAIL给调用者处理 */
    if (*user_idx == MAC_INVALID_USER_ID) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 根据user MAC地址查找user对象
*****************************************************************************/
osal_u32 hmac_device_find_user_by_macaddr_etc(osal_u8 *sta_mac_addr, osal_u16 *user_idx)
{
    hmac_device_stru *hmac_device;
    hmac_vap_stru *hmac_vap;
    osal_u8 vap_idx;
    osal_u32 ret;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_device_find_user_by_macaddr_etc:get_dev return null ");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 对device下的所有vap进行遍历 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        /* 配置vap不需要处理 */
        if (hmac_device->vap_id[vap_idx] == hmac_device->cfg_vap_id) {
            continue;
        }

        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OAL_PTR_NULL) {
            continue;
        }

        /* 只处理AP模式 */
        if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
            continue;
        }

        ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, sta_mac_addr, user_idx);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述 : 初始化wme参数, 除sta之外的模式
*****************************************************************************/
osal_u32 hmac_vap_init_wme_param_etc(hmac_vap_stru *hmac_vap)
{
    const mac_wme_param_stru *wmm_param;
    const mac_wme_param_stru *wmm_param_sta;
    osal_u8 ac_type;

    wmm_param = hmac_get_wmm_cfg_etc(hmac_vap->vap_mode);
    if (wmm_param == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        /* VAP自身的EDCA参数 */
        mac_mib_set_qap_edca_table_aifsn(hmac_vap, ac_type, wmm_param[ac_type].aifsn);
        mac_mib_set_qap_edca_table_cwmin(hmac_vap, ac_type, wmm_param[ac_type].logcwmin);
        mac_mib_set_qap_edca_table_cwmax(hmac_vap, ac_type, wmm_param[ac_type].logcwmax);
        mac_mib_set_qap_edca_table_txop_limit(hmac_vap, ac_type, wmm_param[ac_type].txop_limit);
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* AP模式时广播给STA的EDCA参数，只在AP模式需要初始化此值，使用WLAN_VAP_MODE_BUTT， */
        wmm_param_sta = hmac_get_wmm_cfg_etc(WLAN_VAP_MODE_BUTT);

        for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
            mac_mib_set_edca_table_index(hmac_vap, ac_type, ac_type + 1); /* 注: 协议规定取值1 2 3 4 */
            mac_mib_set_edca_table_aifsn(hmac_vap, ac_type, wmm_param_sta[ac_type].aifsn);
            mac_mib_set_edca_table_cwmin(hmac_vap, ac_type, wmm_param_sta[ac_type].logcwmin);
            mac_mib_set_edca_table_cwmax(hmac_vap, ac_type, wmm_param_sta[ac_type].logcwmax);
            mac_mib_set_edca_table_txop_limit(hmac_vap, ac_type, wmm_param_sta[ac_type].txop_limit);
        }
    }

    return OAL_SUCC;
}

static INLINE__ void mac_mib_init_antenna(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_antenna_selection_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_trans_explicit_csi_feedback_as_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_trans_indices_feedback_as_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_csi_feedback_as_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_receive_antenna_selection_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_trans_sounding_ppdu_option_implemented(hmac_vap, OAL_FALSE);
}

static INLINE__ void mac_mib_init_beamforming(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_implicit_trans_bf_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_csi_trans_bf_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_non_compress_bf_matrix_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_trans_bf_csi_feedback_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_explicit_non_compressed_bf_feedback_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_num_bf_csi_support_antenna(hmac_vap, 0);
    mac_mib_set_num_non_compressed_bf_matrix_support_antenna(hmac_vap, 0);
}

static INLINE__ void mac_mib_init_ndp(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_receive_ndp_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_transmit_ndp_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_calibration_option_implemented(hmac_vap, OAL_FALSE);
}

/*****************************************************************************
 功能描述 : 初始化11n的mib
*****************************************************************************/
static void hmac_vap_init_mib_11n(hmac_vap_stru *hmac_vap)
{
    wlan_mib_ieee802dot11_stru *mib_info;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_vap_init_mib_11n::hmac_device null.}");
        return;
    }

    mib_info = hmac_vap->mib_info;

    mac_mib_set_high_throughput_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_ldpc_coding_option_implemented(hmac_vap, mac_device_get_cap_ldpc(hmac_device));
    mac_mib_set_tx_stbc_option_implemented(hmac_vap, mac_device_get_cap_txstbc(hmac_device));

    /* LDPC 和 STBC Activated能力初始化为device的能力，用于STA的协议协商，到关联请求时再更新该mib值  */
    mac_mib_set_LDPCCodingOptionActivated(hmac_vap, mac_device_get_cap_ldpc(hmac_device));
    mac_mib_set_TxSTBCOptionActivated(hmac_vap, HT_TX_STBC_DEFAULT_VALUE);

    mac_mib_set_2g_forty_mhz_operation_implemented(hmac_vap, (mac_device_get_cap_bw(hmac_device) > WLAN_BW_CAP_20M) &&
         (hmac_vap->cap_flag.disable_2ght40 == OSAL_FALSE));
    mac_mib_set_5g_forty_mhz_operation_implemented(hmac_vap, OAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 开启SMPS宏,能力需要按照mac device能力来刷，后续m2s会动态刷新 */
    mac_mib_set_smps(hmac_vap, mac_device_get_mode_smps(hmac_device));
#endif

    mac_mib_set_ht_greenfield_option_implemented(hmac_vap, HT_GREEN_FILED_DEFAULT_VALUE);
    mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_2g_short_gi_option_in_forty_implemented(hmac_vap,
        (hmac_vap->cap_flag.disable_2ght40 == OSAL_FALSE));
    mac_mib_set_5g_short_gi_option_in_forty_implemented(hmac_vap, OAL_TRUE);

    mac_mib_set_rx_stbc_option_implemented(hmac_vap, mac_device_get_cap_rxstbc(hmac_device));
    mib_info->wlan_mib_ht_sta_cfg.dot11_max_amsdu_length = 0;

#if (defined(_PRE_PRODUCT_ID_HIMPXX_DEV) || defined(_PRE_PRODUCT_ID_HOST))
    mac_mib_set_lsig_txop_protection(hmac_vap, OAL_FALSE);
#else
    mac_mib_set_lsig_txop_protection(hmac_vap, OAL_TRUE);
#endif

    mac_mib_set_max_ampdu_len_exponent(hmac_vap, 3);         /* max ampdu len exponent is 3 */
    mac_mib_set_min_mpdu_start_spacing(hmac_vap, 5);         /* min mpdu start spacing is 5 */
    mac_mib_set_pco_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_transition_time(hmac_vap, 3);                /* transition time is 3 */
    mac_mib_set_mcs_fdbk(hmac_vap, OAL_FALSE);
    mac_mib_set_htc_sup(hmac_vap, OAL_FALSE);
    mac_mib_set_rd_rsp(hmac_vap, OAL_FALSE);

    /* NDP配置 */
    mac_mib_init_ndp(hmac_vap);

    /* Beamforming配置 */
    mac_mib_init_beamforming(hmac_vap);

    /* 天线选择能力信息 */
    mac_mib_init_antenna(hmac_vap);

    /* obss信息 */
    mac_mib_init_obss_scan(hmac_vap);

    /* 默认使用2040共存 */
    mac_mib_init_2040(hmac_vap);

    if (g_st_mac_vap_rom_cb.mac_vap_init_mib_11n_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.mac_vap_init_mib_11n_cb(hmac_vap);
    }
}

/*****************************************************************************
 功能描述 : 初始化11ac mib中mcs信息
*****************************************************************************/
OAL_STATIC void hmac_vap_init_11ac_mcs_singlenss(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_tx_max_mcs_map_stru *tx_max_mcs_map;
    mac_rx_max_mcs_map_stru *rx_max_mcs_map;

    /* 获取mib值指针 */
    rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(hmac_vap));
    tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(hmac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (bandwidth == WLAN_BAND_WIDTH_20M) {
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
#else
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
#endif
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AC);
    } else if ((bandwidth == WLAN_BAND_WIDTH_40MINUS) || (bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS9 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AC);
    } else if ((bandwidth == WLAN_BAND_WIDTH_80MINUSMINUS) || (bandwidth == WLAN_BAND_WIDTH_80MINUSPLUS) ||
        (bandwidth == WLAN_BAND_WIDTH_80PLUSMINUS) || (bandwidth == WLAN_BAND_WIDTH_80PLUSPLUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS9 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AC);
    }
}

/*****************************************************************************
 功能描述 : 初始化11ac mib中mcs信息
*****************************************************************************/
OAL_STATIC void hmac_vap_init_11ac_mcs_doublenss(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_tx_max_mcs_map_stru *tx_max_mcs_map;
    mac_rx_max_mcs_map_stru *rx_max_mcs_map;

    /* 获取mib值指针 */
    rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(hmac_vap));
    tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(hmac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (bandwidth == WLAN_BAND_WIDTH_20M) {
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
#else
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
#endif
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AC);
    } else if ((bandwidth == WLAN_BAND_WIDTH_40MINUS) || (bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
        /* 40MHz带宽的情况下，支持MCS0-MCS9 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AC);
    } else if ((bandwidth == WLAN_BAND_WIDTH_80MINUSMINUS) || (bandwidth == WLAN_BAND_WIDTH_80MINUSPLUS) ||
        (bandwidth == WLAN_BAND_WIDTH_80PLUSMINUS) || (bandwidth == WLAN_BAND_WIDTH_80PLUSPLUS)) {
        /* 80MHz带宽的情况下，支持MCS0-MCS9 */
        rx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        rx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        tx_max_mcs_map->max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AC);
        mac_mib_set_us_tx_highest_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AC);
    }
}

/*****************************************************************************
 功能描述 : 初始化11ac mib信息
*****************************************************************************/
OAL_STATIC void hmac_vap_init_mib_11ac(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_vap_init_mib_11ac::mac_dev[%d] null.}",
            hmac_vap->device_id);
        return;
    }

    mac_mib_set_vht_option_implemented(hmac_vap, OAL_TRUE);

    mac_mib_set_maxmpdu_length(hmac_vap, WLAN_MIB_VHT_MPDU_7991);

    mac_mib_set_vht_ldpc_coding_option_implemented(hmac_vap, mac_device_get_cap_ldpc(hmac_device));
    mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_vht_channel_width_option_implemented(hmac_vap,
        hmac_device_trans_bandwith_to_vht_capinfo(mac_device_get_cap_bw(hmac_device)));
    mac_mib_set_vht_tx_stbc_option_implemented(hmac_vap, mac_device_get_cap_txstbc(hmac_device));
    mac_mib_set_vht_rx_stbc_option_implemented(hmac_vap, mac_device_get_cap_rxstbc(hmac_device));

    /* TxBf相关 */
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_mib_set_vht_su_bfer_option_implemented(hmac_vap, mac_device_get_cap_subfer(hmac_device));
    mac_mib_set_vht_su_bfee_option_implemented(hmac_vap, mac_device_get_cap_subfee(hmac_device));
    mac_mib_set_vht_num_sounding_dimensions(hmac_vap, mac_device_get_nss_num(hmac_device));
    mac_mib_set_vht_mu_bfer_option_implemented(hmac_vap, mac_device_get_cap_mubfer(hmac_device));
    mac_mib_set_vht_mu_bfee_option_implemented(hmac_vap, mac_device_get_cap_mubfee(hmac_device));
#else
    mac_mib_set_vht_su_bfer_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_vht_su_bfee_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_vht_num_sounding_dimensions(hmac_vap, WLAN_SINGLE_NSS);
    mac_mib_set_vht_mu_bfer_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_vht_mu_bfee_option_implemented(hmac_vap, OAL_FALSE);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_mib_set_txopps(hmac_vap, OAL_TRUE);
#endif

    mac_mib_set_vht_ctrl_field_cap(hmac_vap, OAL_FALSE);

    mac_mib_set_vht_max_rx_ampdu_factor(hmac_vap, 5); /* 5 = 2^(13+factor)-1 字节 */

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    if (is_ap(hmac_vap)) {
        mac_mib_set_operating_mode_notification_implemented(hmac_vap, OAL_FALSE);
    } else {
        mac_mib_set_operating_mode_notification_implemented(hmac_vap, OAL_TRUE);
    }
#endif
}

/*****************************************************************************
 功能描述 : 初始化11i mib信息
*****************************************************************************/
static void hmac_vap_init_mib_11i(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_rsnaactivated(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_rsnamfpr(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_rsnamfpc(hmac_vap, OAL_FALSE);
    mac_mib_set_pre_auth_actived(hmac_vap, OAL_FALSE);
    mac_mib_set_privacyinvoked(hmac_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(hmac_vap);
    mac_mib_set_rsnacfg_gtksareplaycounters(hmac_vap, 0);
    mac_mib_set_rsnacfg_ptksareplaycounters(hmac_vap, 0);
}

OSAL_STATIC void mac_vap_init_mib_11k(hmac_vap_stru *hmac_vap)
{
#if (defined(_PRE_PRODUCT_ID_HOST) || defined(_PRE_PRODUCT_ID_HIMPXX_DEV))
    if (!is_legacy_sta(hmac_vap)) {
        return;
    }
#endif
    mac_mib_set_dot11_radio_measurement_activated(hmac_vap, OAL_TRUE);
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_mib_set_dot11RMBeaconActiveMeasurementActivated(hmac_vap, OAL_TRUE);
        mac_mib_set_dot11RMBeaconPassiveMeasurementActivated(hmac_vap, OAL_TRUE);
        mac_mib_set_dot11RMBeaconTableMeasurementActivated(hmac_vap, OAL_TRUE);
    }

    mac_mib_set_dot11RMLinkMeasurementActivated(hmac_vap, OAL_FALSE);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_mib_set_dot11RMNeighborReportActivated(hmac_vap, OAL_TRUE);
        mac_mib_set_dot11RMChannelLoadMeasurementActivated(hmac_vap, OAL_FALSE);
    }
}

/*****************************************************************************
功能描述 : 初始化11a 11g速率
*****************************************************************************/
OAL_STATIC void hmac_vap_init_legacy_rates(hmac_vap_stru *hmac_vap, mac_data_rate_stru *rates)
{
    osal_u8 rate_index;
    osal_u8 curr_rate_index = 0;
    mac_data_rate_stru *orig_rate;
    mac_data_rate_stru *curr_rate;
    osal_u8 rates_num;

    /* 初始化速率集 */
    rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    hmac_vap->curr_sup_rates.rate.rs_nrates = MAC_NUM_DR_802_11A;
    hmac_vap->curr_sup_rates.br_rate_num = MAC_NUM_BR_802_11A;
    hmac_vap->curr_sup_rates.nbr_rate_num = MAC_NUM_NBR_802_11A;
    hmac_vap->curr_sup_rates.min_rate = 6;     /* 6：最小速率 */
    hmac_vap->curr_sup_rates.max_rate = 24;    /* 24：最大速率 */

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (rate_index = 0; rate_index < rates_num; rate_index++) {
        orig_rate = &rates[rate_index];
        curr_rate = &(hmac_vap->curr_sup_rates.rate.rs_rates[curr_rate_index]);

        /* Basic Rates */
        if ((orig_rate->mbps == 6) || (orig_rate->mbps == 12) || (orig_rate->mbps == 24)) { /* 6/12/24 Mbps */
            (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));
            curr_rate->mac_rate |= 0x80;
            curr_rate_index++;
        } else if ((orig_rate->mbps == 9) || (orig_rate->mbps == 18) ||   /* 9/18 Mbps */
            (orig_rate->mbps == 36) || (orig_rate->mbps == 48) ||         /* 36/48 Mbps */
            (orig_rate->mbps == 54)) {                                       /* 54 Mbps */
            (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));
            curr_rate_index++;
        }

        if (curr_rate_index == hmac_vap->curr_sup_rates.rate.rs_nrates) {
            break;
        }
    }
}

/*****************************************************************************
功能描述 : 初始化11b速率
*****************************************************************************/
OAL_STATIC void hmac_vap_init_11b_rates(hmac_vap_stru *hmac_vap, mac_data_rate_stru *rates)
{
    osal_u8 rate_index;
    osal_u8 curr_rate_index = 0;
    mac_data_rate_stru *orig_rate;
    mac_data_rate_stru *curr_rate;
    osal_u8 rates_num;

    /* 初始化速率集 */
    rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    hmac_vap->curr_sup_rates.rate.rs_nrates = MAC_NUM_DR_802_11B;
    hmac_vap->curr_sup_rates.br_rate_num = 0;
    hmac_vap->curr_sup_rates.nbr_rate_num = MAC_NUM_NBR_802_11B;
    hmac_vap->curr_sup_rates.min_rate = 1;     /* 1是最小速率 */
    hmac_vap->curr_sup_rates.max_rate = 2;     /* 2是最大速率 */

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (rate_index = 0; rate_index < rates_num; rate_index++) {
        orig_rate = &rates[rate_index];
        curr_rate = &(hmac_vap->curr_sup_rates.rate.rs_rates[curr_rate_index]);

        /*  Basic Rates  */
        if ((orig_rate->mbps == 1) || (orig_rate->mbps == 2) ||       /* 2Mbps */
            ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
            ((orig_rate->mbps == 5) || (orig_rate->mbps == 11)))) {   /* 5Mbps 11Mbps of STA */
            hmac_vap->curr_sup_rates.br_rate_num++;
            (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));
            curr_rate->mac_rate |= 0x80;
            curr_rate_index++;
        } else if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            ((orig_rate->mbps == 5) || (orig_rate->mbps == 11))) {    /* 5Mbps 11Mbps of AP */
            (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));
            curr_rate_index++;
        } else {
            continue;
        }

        if (curr_rate_index == hmac_vap->curr_sup_rates.rate.rs_nrates) {
            break;
        }
    }
}

/*****************************************************************************
功能描述 : 初始化11b速率
*****************************************************************************/
OAL_STATIC void hmac_vap_init_11g_mixed_one_rates(hmac_vap_stru *hmac_vap, mac_data_rate_stru *rates)
{
    osal_u8 rate_index;
    mac_data_rate_stru *orig_rate;
    mac_data_rate_stru *curr_rate;
    osal_u8 rates_num;

    /* 初始化速率集 */
    rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    hmac_vap->curr_sup_rates.rate.rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    hmac_vap->curr_sup_rates.br_rate_num = MAC_NUM_BR_802_11G_MIXED_ONE;
    hmac_vap->curr_sup_rates.nbr_rate_num = MAC_NUM_NBR_802_11G_MIXED_ONE;
    hmac_vap->curr_sup_rates.min_rate = 1;     /* 1是最小速率 */
    hmac_vap->curr_sup_rates.max_rate = 11;    /* 11是最大速率 */

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (rate_index = 0; rate_index < rates_num; rate_index++) {
        orig_rate = &rates[rate_index];
        curr_rate = &(hmac_vap->curr_sup_rates.rate.rs_rates[rate_index]);

        (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));

        /* Basic Rates */
        if ((orig_rate->mbps == 1) || (orig_rate->mbps == 2) ||   /* 2Mbps */
            (orig_rate->mbps == 5) || (orig_rate->mbps == 11)) {  /* 5Mbps 11Mbps */
            curr_rate->mac_rate |= 0x80;
        }
    }
}

/*****************************************************************************
功能描述 : 初始化11g mixed two速率
*****************************************************************************/
OAL_STATIC void hmac_vap_init_11g_mixed_two_rates(hmac_vap_stru *hmac_vap, mac_data_rate_stru *rates)
{
    osal_u8 rate_index;
    mac_data_rate_stru *orig_rate;
    mac_data_rate_stru *curr_rate;
    osal_u8 rates_num;

    /* 初始化速率集 */
    rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* 初始化速率个数，基本速率个数，非基本速率个数 */
    hmac_vap->curr_sup_rates.rate.rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    hmac_vap->curr_sup_rates.br_rate_num = MAC_NUM_BR_802_11G_MIXED_TWO;
    hmac_vap->curr_sup_rates.nbr_rate_num = MAC_NUM_NBR_802_11G_MIXED_TWO;
    hmac_vap->curr_sup_rates.min_rate = 1;
    hmac_vap->curr_sup_rates.max_rate = 24;    /* 24是最大速率 */

    /* 将速率拷贝到VAP结构体下的速率集中 */
    for (rate_index = 0; rate_index < rates_num; rate_index++) {
        orig_rate = &rates[rate_index];
        curr_rate = &(hmac_vap->curr_sup_rates.rate.rs_rates[rate_index]);

        (osal_void)memcpy_s(curr_rate, sizeof(mac_data_rate_stru), orig_rate, sizeof(mac_data_rate_stru));

        /* Basic Rates */
        if ((orig_rate->mbps == 1) || (orig_rate->mbps == 2) || (orig_rate->mbps == 5) || /* 2Mbps 5Mbps */
            (orig_rate->mbps == 11) || (orig_rate->mbps == 6) ||      /* 11Mbps 6Mbps */
            (orig_rate->mbps == 12) || (orig_rate->mbps == 24)) {     /* 12Mbps 24Mbps */
            curr_rate->mac_rate |= 0x80;
        }
    }
}

/*****************************************************************************
功能描述 : 初始化11n速率
*****************************************************************************/
static void hmac_vap_init_11n_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* 初始化速率集 */

    /* MCS相关MIB值初始化 */
    mac_mib_set_tx_mcs_set_defined(hmac_vap, OAL_TRUE);
    mac_mib_set_tx_rx_mcs_set_not_equal(hmac_vap, OAL_FALSE);
    mac_mib_set_tx_unequal_modulation_supported(hmac_vap, OAL_FALSE);

    /* 将MIB值的MCS MAP清零 */
    memset_s(mac_mib_get_SupportedMCSTx(hmac_vap), WLAN_HT_MCS_BITMASK_LEN,
        0, WLAN_HT_MCS_BITMASK_LEN);
    memset_s(mac_mib_get_supported_mcs_rx_value(hmac_vap), WLAN_HT_MCS_BITMASK_LEN,
        0, WLAN_HT_MCS_BITMASK_LEN);

    if (mac_device_get_nss_num(hmac_device) == WLAN_SINGLE_NSS) { /* 1个空间流 */
        mac_mib_set_tx_maximum_num_spatial_streams_supported(hmac_vap, 1);
        mac_mib_set_supported_mcs_rx_value(hmac_vap, 0, 0xFF); /* 支持 RX MCS 0-7，8位全置为1 */
        mac_mib_set_SupportedMCSTxValue(hmac_vap, 0, 0xFF); /* 支持 TX MCS 0-7，8位全置为1 */

        mac_mib_set_highest_supported_data_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11N);

        if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
            (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
            /* 40M 支持MCS32 */
            mac_mib_set_supported_mcs_rx_value(hmac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1. 4是MCS bitmap */
            mac_mib_set_SupportedMCSTxValue(hmac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1. 4是MCS bitmap */
            mac_mib_set_highest_supported_data_rate(hmac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11N);
        }
    } else if (mac_device_get_nss_num(hmac_device) == WLAN_DOUBLE_NSS) { /* 2个空间流 */
        mac_mib_set_tx_maximum_num_spatial_streams_supported(hmac_vap, 2);
        mac_mib_set_supported_mcs_rx_value(hmac_vap, 0, 0xFF); /* 支持 RX MCS 0-7，8位全置为1 */
        mac_mib_set_supported_mcs_rx_value(hmac_vap, 1, 0xFF); /* 支持 RX MCS 8-15，8位全置为1 */

        mac_mib_set_SupportedMCSTxValue(hmac_vap, 0, 0xFF); /* 支持 TX MCS 0-7，8位全置为1 */
        mac_mib_set_SupportedMCSTxValue(hmac_vap, 1, 0xFF); /* 支持 TX MCS 8-15，8位全置为1 */

        mac_mib_set_highest_supported_data_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11N);

        if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) ||
            (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS)) {
            /* 40M 支持的最大速率为300M */
            mac_mib_set_supported_mcs_rx_value(hmac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1. 4是MCS bitmap */
            mac_mib_set_SupportedMCSTxValue(hmac_vap, 4, 0x01); /* 支持 RX MCS 32,最后一位为1. 4是MCS bitmap */
            mac_mib_set_highest_supported_data_rate(hmac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11N);
        }
    }

    if (g_st_mac_vap_rom_cb.init_11n_rates_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.init_11n_rates_cb(hmac_vap, hmac_device);
    }
}

/*****************************************************************************
功能描述 : 初始化11n速率
*****************************************************************************/
static void hmac_vap_init_11ac_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF */
    mac_mib_set_vht_rx_mcs_map(hmac_vap, 0xFFFF);
    mac_mib_set_vht_tx_mcs_map(hmac_vap, 0xFFFF);

    if (mac_device_get_nss_num(hmac_device) == WLAN_SINGLE_NSS) {
        /* 1个空间流的情况 */
        hmac_vap_init_11ac_mcs_singlenss(hmac_vap, hmac_vap->channel.en_bandwidth);
    } else if (mac_device_get_nss_num(hmac_device) == WLAN_DOUBLE_NSS) {
        /* 2个空间流的情况 */
        hmac_vap_init_11ac_mcs_doublenss(hmac_vap, hmac_vap->channel.en_bandwidth);
    } else {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_vap_init_11ac_rates::invalid nss_num[%d].}",
            mac_device_get_nss_num(hmac_device));
    }

    if (g_st_mac_vap_rom_cb.init_11ac_rates_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.init_11ac_rates_cb(hmac_vap, hmac_device);
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
功能描述 : 初始化11x速率
*****************************************************************************/
OSAL_STATIC void mac_vap_rom_init_11ax_rates(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    if (g_st_mac_vap_rom_cb.init_11ax_rates_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.init_11ax_rates_cb(hmac_vap, hmac_device);
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述 : 初始化p2p vap的速率集
*****************************************************************************/
static void hmac_vap_init_p2p_rates_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 vap_protocol,
    mac_data_rate_stru *rates)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_vap_init_p2p_rates_etc::mac_dev[%d] null.}",
            hmac_vap->device_id);

        return;
    }

    hmac_vap_init_legacy_rates(hmac_vap, rates);

    /* 在listen channel的probe respons 携带速率集有重复IE. */
    (osal_void)memcpy_s(&hmac_vap->sta_sup_rates_ie[WLAN_BAND_5G], OAL_SIZEOF(mac_curr_rateset_stru),
        &hmac_vap->curr_sup_rates, OAL_SIZEOF(mac_curr_rateset_stru));
    (osal_void)memcpy_s(&hmac_vap->sta_sup_rates_ie[WLAN_BAND_2G], OAL_SIZEOF(mac_curr_rateset_stru),
        &hmac_vap->curr_sup_rates, OAL_SIZEOF(mac_curr_rateset_stru));

    if (vap_protocol == WLAN_VHT_MODE) {
        hmac_vap_init_11n_rates(hmac_vap, hmac_device);
        hmac_vap_init_11ac_rates(hmac_vap, hmac_device);
    } else {
        hmac_vap_init_11n_rates(hmac_vap, hmac_device);
    }
}
#endif

static osal_void mac_vap_init_dev_rates_by_protocol_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 proto,
    hmac_device_stru *hmac_device, mac_data_rate_stru *rates)
{
    if ((proto == WLAN_HE_MODE) || (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && proto == WLAN_VHT_MODE)) {
        /* 用于STA全信道扫描 5G时 填写支持速率集ie */
        hmac_vap_init_legacy_rates(hmac_vap, rates);
        memcpy_s(&hmac_vap->sta_sup_rates_ie[WLAN_BAND_5G], sizeof(mac_curr_rateset_stru),
            &hmac_vap->curr_sup_rates, sizeof(hmac_vap->curr_sup_rates));

        /* 用于STA全信道扫描 2G时 填写支持速率集ie */
        hmac_vap_init_11g_mixed_one_rates(hmac_vap, rates);
        memcpy_s(&hmac_vap->sta_sup_rates_ie[WLAN_BAND_2G], sizeof(mac_curr_rateset_stru),
            &hmac_vap->curr_sup_rates, sizeof(hmac_vap->curr_sup_rates));

        hmac_vap_init_11n_rates(hmac_vap, hmac_device);
        hmac_vap_init_11ac_rates(hmac_vap, hmac_device);
#ifdef _PRE_WLAN_FEATURE_11AX
        if (proto == WLAN_HE_MODE) {
            mac_vap_rom_init_11ax_rates(hmac_vap, hmac_device);
        }
#endif
    } else if ((proto == WLAN_VHT_ONLY_MODE) || (proto == WLAN_VHT_MODE)) {
#ifdef _PRE_WLAN_FEATURE_11AC2G
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
            hmac_vap_init_11g_mixed_one_rates(hmac_vap, rates);
        } else {
            hmac_vap_init_legacy_rates(hmac_vap, rates);
        }
#else
        hmac_vap_init_legacy_rates(hmac_vap, rates);
#endif
        hmac_vap_init_11n_rates(hmac_vap, hmac_device);
        hmac_vap_init_11ac_rates(hmac_vap, hmac_device);
    } else if ((proto == WLAN_HT_ONLY_MODE) || (proto == WLAN_HT_MODE)) {
        if (hmac_vap->channel.band == WLAN_BAND_5G) {
            hmac_vap_init_legacy_rates(hmac_vap, rates);
        } else if (hmac_vap->channel.band == WLAN_BAND_2G) {
            hmac_vap_init_11g_mixed_one_rates(hmac_vap, rates);
        }

        hmac_vap_init_11n_rates(hmac_vap, hmac_device);
    } else if ((proto == WLAN_LEGACY_11A_MODE) || (proto == WLAN_LEGACY_11G_MODE)) {
        hmac_vap_init_legacy_rates(hmac_vap, rates);
    } else if (proto == WLAN_LEGACY_11B_MODE) {
        hmac_vap_init_11b_rates(hmac_vap, rates);
    } else if (proto == WLAN_MIXED_ONE_11G_MODE) {
        hmac_vap_init_11g_mixed_one_rates(hmac_vap, rates);
    } else if (proto == WLAN_MIXED_TWO_11G_MODE) {
        hmac_vap_init_11g_mixed_two_rates(hmac_vap, rates);
    }
}

static void hmac_vap_init_rates_by_protocol_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 vap_protocol,
    mac_data_rate_stru *rates)
{
    hmac_device_stru *hmac_device;

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_vap_init_rates_by_protocol_etc::hmac_res_get_mac_dev_etc fail.device_id:[%d].}",
            hmac_vap->device_id);
        return;
    }

    /* STA模式默认协议模式是11ac，初始化速率集为所有速率集 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (!is_legacy_vap(hmac_vap)) {
        hmac_vap_init_p2p_rates_etc(hmac_vap, vap_protocol, rates);
        return;
    }
#endif

    mac_vap_init_dev_rates_by_protocol_etc(hmac_vap, vap_protocol, hmac_device, rates);
}

/*****************************************************************************
功能描述 : 初始化速率集
*****************************************************************************/
void hmac_vap_init_rates_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;
    mac_data_rate_stru *rates;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_vap_init_rates_etc::mac_dev[%d] null.}", hmac_vap->device_id);
        return;
    }

    /* 初始化速率集 */
    rates = hmac_device_get_all_rates_etc(hmac_device);

    hmac_vap_init_rates_by_protocol_etc(hmac_vap, hmac_vap->protocol, rates);
}

/*****************************************************************************
功能描述 : STA初始化速率集
*****************************************************************************/
void mac_sta_init_bss_rates_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_device_stru *hmac_device;
    wlan_protocol_enum_uint8 vap_protocol;
    mac_data_rate_stru *rates;
    osal_u32 i, j;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_vap_init_rates_etc::mac_dev[%d] null.}",
            hmac_vap->device_id);
        return;
    }

    /* 初始化速率集 */
    rates = hmac_device_get_all_rates_etc(hmac_device);
    if (bss_dscr == OAL_PTR_NULL) {
        vap_protocol = hmac_vap->protocol;
        hmac_vap_init_rates_by_protocol_etc(hmac_vap, vap_protocol, rates);
        return;
    }

    for (i = 0; i < bss_dscr->num_supp_rates; i++) {
        for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
            if ((rates[j].mac_rate & 0x7f) == (bss_dscr->supp_rates[i] & 0x7f)) {
                rates[j].mac_rate = bss_dscr->supp_rates[i];
                break;
            }
        }
    }

    vap_protocol = hmac_vap->protocol;
    hmac_vap_init_rates_by_protocol_etc(hmac_vap, vap_protocol, rates);
}

/*****************************************************************************
 功能描述 : 根据AP的operation ie获取ap的工作带宽,sta根据ap携带的ht和vth能力更新
             user和vap的带宽信息
*****************************************************************************/
osal_u8 hmac_vap_get_ap_usr_opern_bandwidth(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_user_ht_hdl_stru *mac_ht_hdl;
    mac_vht_hdl_stru *mac_vht_hdl;
    wlan_channel_bandwidth_enum_uint8 bandwidth_ap = WLAN_BAND_WIDTH_20M;
    wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *mac_he_hdl;
#endif

    /* 更新用户的带宽能力cap */
    hmac_user_update_ap_bandwidth_cap(hmac_user);
    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl = &(hmac_user->ht_hdl);
    mac_vht_hdl = &(hmac_user->vht_hdl);

    /******************* VHT BSS operating channel width ****************

     -----------------------------------------------------------------
     |HT Oper Chl Width |VHT Oper Chl Width |BSS Oper Chl Width|
     -----------------------------------------------------------------
     |       0          |        0          |    20MHZ         |
     -----------------------------------------------------------------
     |       1          |        0          |    40MHZ         |
     -----------------------------------------------------------------
     |       1          |        1          |    80MHZ         |
     -----------------------------------------------------------------
     |       1          |        2          |    160MHZ        |
     -----------------------------------------------------------------
     |       1          |        3          |    80+80MHZ      |
     -----------------------------------------------------------------

    **********************************************************************/

    if (mac_vht_hdl->vht_capable == OAL_TRUE) {
        bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(mac_vht_hdl->channel_width,
            hmac_vap->channel.chan_number, mac_vht_hdl->channel_center_freq_seg0,
            mac_vht_hdl->channel_center_freq_seg1);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl = &(hmac_user->he_hdl);
    if ((mac_he_hdl->he_capable == OAL_TRUE) &&
        (mac_he_hdl->he_oper_ie.he_oper_param.vht_operation_info_present == OSAL_TRUE)) {
        bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(
            mac_he_hdl->he_oper_ie.vht_operation_info.channel_width, hmac_vap->channel.chan_number,
            mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg0,
            mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg1);
    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((mac_ht_hdl->ht_capable == OAL_TRUE) && (bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) &&
        (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OAL_TRUE)) {
        /* 更新带宽模式 */
        bandwidth_ap = mac_get_bandwidth_from_sco(mac_ht_hdl->secondary_chan_offset);
    }

    /* 带宽不能超过mac device的最大能力 */
    sta_new_bandwidth = hmac_vap_get_bandwith(hmac_vap, bandwidth_ap);
    if (hmac_regdomain_channel_is_support_bw(sta_new_bandwidth, hmac_vap->channel.chan_number) == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "{hmac_vap_get_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
            hmac_vap->channel.chan_number, sta_new_bandwidth);
        sta_new_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return sta_new_bandwidth;
}

/*****************************************************************************
 功能描述 : 设置vap的发送功率
*****************************************************************************/
void hmac_vap_set_tx_power_etc(hmac_vap_stru *hmac_vap, osal_u8 tx_power)
{
    hmac_vap->tx_power = tx_power;
}

/*****************************************************************************
 功能描述 : 设置vap的aid
*****************************************************************************/
void hmac_vap_set_aid_etc(hmac_vap_stru *hmac_vap, osal_u16 aid)
{
    hmac_vap->sta_aid = aid;
    hmac_vap_sync(hmac_vap);
}

/*****************************************************************************
 功能描述 : 设置vap的assoc_vap_id 该参数只在STA有效
*****************************************************************************/
void hmac_vap_set_assoc_id_etc(hmac_vap_stru *hmac_vap, osal_u16 assoc_vap_id)
{
    hmac_vap->assoc_vap_id = assoc_vap_id;
}

/*****************************************************************************
 功能描述 : 设置vap的assoc_vap_id 该参数只在STA有效
*****************************************************************************/
void hmac_vap_set_uapsd_cap_etc(hmac_vap_stru *hmac_vap, osal_u8 uapsd_cap)
{
    hmac_vap->uapsd_cap = uapsd_cap;
}

/*****************************************************************************
 功能描述 : 设置vap的组播用户id
*****************************************************************************/
void hmac_vap_set_multi_user_idx_etc(hmac_vap_stru *hmac_vap, osal_u16 multi_user_idx)
{
    hmac_vap->multi_user_idx = multi_user_idx;
}

/*****************************************************************************
 功能描述 : 设置vap的接收nss
*****************************************************************************/
void hmac_vap_set_rx_nss_etc(hmac_vap_stru *hmac_vap, wlan_nss_enum_uint8 rx_nss)
{
    hmac_vap->vap_rx_nss = rx_nss;
}

/*****************************************************************************
 功能描述 : 设置vap的wmm update count
*****************************************************************************/
void hmac_vap_set_wmm_params_update_count_etc(hmac_vap_stru *hmac_vap, osal_u8 update_count)
{
    hmac_vap->uc_wmm_params_update_count = update_count;
}

/*****************************************************************************
 功能描述 : 设置vap的hide ssid
*****************************************************************************/
void hmac_vap_set_hide_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    hmac_vap->cap_flag.hide_ssid = value;
}

/*****************************************************************************
 函 数 名  : mac_vap_get_obss_scan
 功能描述  : 获取vap的bit_peer_obss_scan

*****************************************************************************/
osal_u8 hmac_vap_get_peer_obss_scan_etc(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->cap_flag.peer_obss_scan;
}

/*****************************************************************************
 功能描述 : 设置vap的peer_obss_scan
*****************************************************************************/
void hmac_vap_set_peer_obss_scan_etc(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    hmac_vap->cap_flag.peer_obss_scan = value;
}

/*****************************************************************************
功能描述 : 将用户态 IE 信息拷贝到内核态中
*****************************************************************************/
osal_u32 hmac_vap_save_app_ie_etc(hmac_vap_stru *hmac_vap, oal_app_ie_stru *app_ie, app_ie_type_uint8 type)
{
    osal_u8 *ie = OAL_PTR_NULL;
    osal_u32 ie_len;
    oal_app_ie_stru tmp_app_ie;

    memset_s(&tmp_app_ie, OAL_SIZEOF(tmp_app_ie), 0, OAL_SIZEOF(tmp_app_ie));

    if (type >= OAL_APP_IE_NUM) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_vap_save_app_ie_etc::invalid type[%d].}", type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ie_len = app_ie->ie_len;

    /* 如果输入WPS 长度为0， 则直接释放VAP 中资源 */
    if (ie_len == 0) {
        if (hmac_vap->app_ie[type].ie != OAL_PTR_NULL) {
            oal_mem_free(hmac_vap->app_ie[type].ie, OAL_TRUE);
        }
        hmac_vap->app_ie[type].ie = OAL_PTR_NULL;
        hmac_vap->app_ie[type].ie_len = 0;
        return OAL_SUCC;
    }

    /* 检查该类型的IE是否需要申请内存 */
    if ((hmac_vap->app_ie[type].ie_len < ie_len) || (hmac_vap->app_ie[type].ie == NULL)) {
        /* 这种情况不应该出现，维测需要 */
        if (hmac_vap->app_ie[type].ie == NULL && hmac_vap->app_ie[type].ie_len != 0) {
            oam_error_log1(0, OAM_SF_CFG, "hmac_vap_save_app_ie_etc::invalid len[%d]", hmac_vap->app_ie[type].ie_len);
            return OAL_FAIL;
        }

        /* 如果以前的内存空间小于新信息元素需要的长度，则需要重新申请内存 */
        ie = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, (osal_u16)(ie_len), OAL_TRUE);
        if (ie == OAL_PTR_NULL) {
            oam_warning_log2(0, OAM_SF_CFG,
                "{hmac_vap_save_app_ie_etc::LOCAL_MEM_POOL is empty!,len[%d], type[%d].}", app_ie->ie_len, type);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        oal_mem_free(hmac_vap->app_ie[type].ie, OAL_TRUE);

        hmac_vap->app_ie[type].ie = ie;
        hmac_vap->app_ie[type].ie_len = (osal_u16)ie_len;
    }

    (osal_void)memcpy_s((void *)hmac_vap->app_ie[type].ie, ie_len, (void *)app_ie->ie, ie_len);
    hmac_vap->app_ie[type].ie_len = (osal_u16)ie_len;
    if (type >= OAL_APP_EXTEND_IE1 && type <= OAL_APP_EXTEND_IE4) {
        hmac_vap->app_ie[type].ie_type_bitmap = app_ie->ie_type_bitmap;
    }
    common_log_dbg4(0, OAM_SF_CFG, "{hmac_vap_save_app_ie_etc::IE:[0x%2x][0x%2x][0x%2x][0x%2x]}",
        *(hmac_vap->app_ie[type].ie), *(hmac_vap->app_ie[type].ie + 1),
        *(hmac_vap->app_ie[type].ie + 2), *(hmac_vap->app_ie[type].ie + ie_len - 1)); /* ie偏移2字节 */
    return OAL_SUCC;
}

/*****************************************************************************
功能描述 : 清除保存的ie
*****************************************************************************/
osal_void hmac_vap_clear_app_ie_etc(hmac_vap_stru *hmac_vap, app_ie_type_uint8 type)
{
    if (type < OAL_APP_IE_NUM) {
        if (hmac_vap->app_ie[type].ie != OAL_PTR_NULL) {
            oal_mem_free(hmac_vap->app_ie[type].ie, OAL_TRUE);
            hmac_vap->app_ie[type].ie = OAL_PTR_NULL;
        }
        hmac_vap->app_ie[type].ie_len = 0;
    }
}

void hmac_vap_free_mib_etc(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->mib_info != OAL_PTR_NULL) {
        wlan_mib_ieee802dot11_stru *mib_info = hmac_vap->mib_info;
        /* 先置空再释放 */
        hmac_vap->mib_info = OAL_PTR_NULL;
        oal_mem_free(mib_info, OAL_TRUE);
    }
}

static osal_void mac_vap_exit_resource_etc(hmac_vap_stru *hmac_vap)
{
    osal_u8 index;

    /* 释放与hmac有关的内存 */
    hmac_vap_free_mib_etc(hmac_vap);

    /* 释放WPS信息元素内存 */
    for (index = 0; index < OAL_APP_IE_NUM; index++) {
        hmac_vap_clear_app_ie_etc(hmac_vap, index);
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_dec_p2p_num_etc(hmac_vap);
#endif
}

/*****************************************************************************
功能描述 : mac vap exit
*****************************************************************************/
osal_u32 hmac_vap_exit_etc(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    osal_u8 index;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_exit_etc::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap->init_flag = MAC_VAP_INVAILD;

    /* 先释放vap关联的相关资源，与之关联的资源释放完后，再执行通用的vap释放流程 */
    mac_vap_exit_resource_etc(hmac_vap);

    /* 业务vap已经删除，从device中去掉 */
    for (index = 0; index < hmac_device->vap_num; index++) {
        /* 从device中找到vap id */
        if (hmac_device->vap_id[index] == hmac_vap->vap_id) {
            /* 如果不是最后一个vap，则把最后一个vap id移动到这个位置，使得该数组是紧凑的 */
            if (index < (hmac_device->vap_num - 1)) {
                hmac_device->vap_id[index] = hmac_device->vap_id[hmac_device->vap_num - 1];
                break;
            }
        }
    }

    /* 删除vap 时，如果vap_num 非0 才--， 否则输出error 日志 */
    if (hmac_device->vap_num != 0) {
        /* device下的vap总数减1 */
        hmac_device->vap_num--;
    } else {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_vap_exit_etc::vap_num is zero. sta_num = %d}", hmac_device->sta_num);
    }

    /* 清除数组中已删除的vap id，保证非零数组元素均为未删除vap */
    hmac_device->vap_id[hmac_device->vap_num] = 0;

    /* device下sta个数减1 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 删除vap 时，如果sta_num 非0 才--， 否则输出error 日志 */
        if (hmac_device->sta_num != 0) {
            hmac_device->sta_num--;
        } else {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_vap_exit_etc::sta_num is zero vap_num = %d}", hmac_device->vap_num);
        }
    }

    hmac_vap->protocol = WLAN_PROTOCOL_BUTT;
    hmac_vap->max_protocol = WLAN_PROTOCOL_MODE_BUTT;

    /* 最后1个vap删除时，清除device级带宽信息 */
    if (hmac_device->vap_num == 0) {
        hmac_device->max_channel = 0;
        hmac_device->max_band = WLAN_BAND_BUTT;
        hmac_device->max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    /* 删除之后将vap的状态置位非法 */
    hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_BUTT);

    return OAL_SUCC;
}

static osal_void mac_init_mib_other_etc(hmac_vap_stru *hmac_vap)
{
    osal_u8 idx;

    mac_mib_set_short_preamble_option_implemented(hmac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
    mac_mib_set_pbcc_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_channel_agility_present(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_multi_domain_capability_activated(hmac_vap, OAL_TRUE);
    mac_mib_set_spectrum_management_required(hmac_vap, OAL_TRUE);
    mac_mib_set_dot11_extended_channel_switch_activated(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_qos_option_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_dot11_apsd_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_qbss_load_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_short_slot_time_option_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_short_slot_time_option_activated(hmac_vap, OAL_TRUE); // 11g 性能问题
    mac_mib_set_dot11_radio_measurement_activated(hmac_vap, OAL_FALSE);

    mac_mib_set_dsss_ofdm_option_activated(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11ImmediateBlockAckOptionImplemented(hmac_vap, OAL_TRUE);
    mac_mib_set_dot11_delayed_block_ack_option_implemented(hmac_vap, OAL_FALSE);
    mac_mib_set_AuthenticationResponseTimeOut(hmac_vap, WLAN_AUTH_TIMEOUT);

    mac_mib_set_ht_protection(hmac_vap, WLAN_MIB_HT_NO_PROTECTION);
    mac_mib_set_rifs_mode(hmac_vap, OAL_TRUE);
    mac_mib_set_non_gf_entities_present(hmac_vap, OAL_FALSE);
    mac_mib_set_lsig_txop_full_protection_activated(hmac_vap, OAL_TRUE);

    mac_mib_set_dual_cts_protection(hmac_vap, OAL_FALSE);
    mac_mib_set_pco_activated(hmac_vap, OAL_FALSE);

    mac_mib_set_dot11AssociationResponseTimeOut(hmac_vap, WLAN_ASSOC_TIMEOUT);
    mac_mib_set_dot11AssociationSAQueryMaximumTimeout(hmac_vap, WLAN_SA_QUERY_MAXIMUM_TIME);
    mac_mib_set_dot11AssociationSAQueryRetryTimeout(hmac_vap, WLAN_SA_QUERY_RETRY_TIME);

    /* WEP 缺省Key表初始化 */
    for (idx = 0; idx < WLAN_NUM_DOT11WEPDEFAULTKEYVALUE; idx++) {
        /* 大小初始化为 WEP-40  */
        mac_mib_set_wep_etc(hmac_vap, idx, WLAN_WEP_40_KEY_SIZE);
    }

    /*    相关私有表初始化  */
    mac_mib_set_privacyinvoked(hmac_vap, OAL_FALSE);
    mac_set_wep_default_keyid(hmac_vap, 0);

    /* 更新wmm参数初始值 */
    if (osal_unlikely(hmac_vap_init_wme_param_etc(hmac_vap) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_init_mib_other_etc::hmac_vap_init_wme_param_etc return FAIL}");
    }

    /* 11i */
    hmac_vap_init_mib_11i(hmac_vap);

    /* 默认11n 11ac使能关闭，配置协议模式时打开 */
    hmac_vap_init_mib_11n(hmac_vap);
    hmac_vap_init_mib_11ac(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_vap_init_mib_11ax(hmac_vap);
#endif

    /* staut低功耗mib项初始化 */
    mac_mib_set_powermanagementmode(hmac_vap, WLAN_MIB_PWR_MGMT_MODE_ACTIVE);

    /* 11k */
    mac_vap_init_mib_11k(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_mib_set_multi_bssid_implement(hmac_vap, OSAL_FALSE);
#endif
}

/*****************************************************************************
 功能描述 : mib初始化函数
*****************************************************************************/
static void hmac_init_mib_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_init_mib_etc::mac_dev[%d] null.}", hmac_vap->device_id);
        return;
    }

    /* 公共特性mib值初始化 */
    mac_mib_set_dot11dtimperiod(hmac_vap, WLAN_DTIM_DEFAULT);
    mac_mib_set_rts_threshold(hmac_vap, WLAN_RTS_MAX);
#ifdef _PRE_WLAN_FEATURE_RTS
    alg_rts_config_threshold(hmac_vap, hmac_vap->mib_info->wlan_mib_operation.dot11_rts_threshold);
#endif
    mac_mib_set_FragmentationThreshold(hmac_vap, WLAN_FRAG_THRESHOLD_MAX);
    mac_mib_set_desired_bss_type(hmac_vap, WLAN_MIB_DESIRED_BSSTYPE_INFRA);
    mac_mib_set_beacon_period(hmac_vap, WLAN_BEACON_INTVAL_DEFAULT);
    mac_mib_set_dot11_vap_max_bandwidth(hmac_vap, mac_device_get_cap_bw(hmac_device));
    /* 设置vap最大用户数 */
    mac_mib_set_MaxAssocUserNums(hmac_vap, mac_chip_get_max_asoc_user(hmac_vap->chip_id));
    mac_mib_set_WPSActive(hmac_vap, OAL_FALSE);

    /* 2040共存信道切换开关初始化 */
#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P设备关闭20/40代管切换 */
    if (is_p2p_mode(hmac_vap->p2p_mode)) {
        mac_mib_set_2040_switch_prohibited(hmac_vap, OAL_TRUE);
    } else { /* 非P2P设备开启20/40代管切换 */
        mac_mib_set_2040_switch_prohibited(hmac_vap, OAL_FALSE);
    }
#else
    mac_mib_set_2040_switch_prohibited(hmac_vap, OAL_FALSE);
#endif

    /* 初始化认证类型为OPEN */
    mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);

    /* 默认设置为自动触发BA回话的建立 */
    mac_mib_set_AddBaMode(hmac_vap, WLAN_ADDBA_MODE_AUTO);

    mac_mib_set_CfgAmpduTxAtive(hmac_vap, OAL_TRUE);
    mac_mib_set_RxBASessionNumber(hmac_vap, 0);
    mac_mib_set_TxBASessionNumber(hmac_vap, 0);

    /* 默认不amsdu/ampdu联合聚合功能不开启;用于小包优化tplink/syslink下行冲包兼容性问题,先关闭02的ampdu+amsdu */
    mac_mib_set_CfgAmsduTxAtive(hmac_vap, hwifi_get_amsdu_tx_active());
    mac_mib_set_AmsduPlusAmpduActive(hmac_vap, hwifi_get_ampdu_amsdu_tx_active());

    mac_init_mib_other_etc(hmac_vap);
}

/*****************************************************************************
 功能描述 : legacy协议初始化vap能力
*****************************************************************************/
void hmac_vap_cap_init_legacy(hmac_vap_stru *hmac_vap)
{
    hmac_vap->cap_flag.rifs_tx_on = OAL_FALSE;

    /* 非VHT不使能 txop ps */
    hmac_vap->cap_flag.txop_ps = OAL_FALSE;

    if (hmac_vap->mib_info != OAL_PTR_NULL) {
        mac_mib_set_txopps(hmac_vap, OAL_FALSE);
    }

    return;
}

/*****************************************************************************
 功能描述 : ht vht协议初始化vap能力
*****************************************************************************/
osal_u32 hmac_vap_cap_init_htvht(hmac_vap_stru *hmac_vap)
{
    hmac_vap->cap_flag.rifs_tx_on = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ASSOC,
            "{hmac_vap_cap_init_htvht::pst_mib_info null,vap mode[%d] state[%d] user num[%d].}", hmac_vap->vap_mode,
            hmac_vap->vap_state, hmac_vap->user_nums);
        return OAL_FAIL;
    }

    hmac_vap->cap_flag.txop_ps = OAL_FALSE;
    if ((hmac_vap->protocol == WLAN_VHT_MODE || hmac_vap->protocol == WLAN_VHT_ONLY_MODE
#ifdef _PRE_WLAN_FEATURE_11AX
        || hmac_vap->protocol == WLAN_HE_MODE
#endif
        ) && mac_mib_get_txopps(hmac_vap)) {
        mac_mib_set_txopps(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_txopps(hmac_vap, OAL_FALSE);
    }
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 调整VHT 2G相关MIB能力
*****************************************************************************/
static void hmac_vap_config_vht_mib_by_band(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->channel.band != WLAN_BAND_2G) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_11AC2G
#ifdef _PRE_WLAN_11AC_20M_ONLY
    if (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_20M) {
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_FALSE);
        return;
    }
#endif

    if (hmac_vap->protocol == WLAN_HT_MODE && hmac_vap->cap_flag.support_11ac2g == OAL_TRUE) {
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_TRUE);
    }
#else
    mac_mib_set_vht_option_implemented(hmac_vap, OAL_FALSE);
#endif
}

/*****************************************************************************
 功能描述 : 依据VAP 协议模式,初始化vap HT/VHT相应MIB能力
*****************************************************************************/
osal_u32 hmac_vap_config_vht_ht_mib_by_protocol_etc(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        /* 规避死机 */
        oam_error_log3(0, OAM_SF_ASSOC,
            "{hmac_vap_config_vht_ht_mib_by_protocol_etc::pst_mib_info null,vap mode[%d] state[%d] user num[%d].}",
            hmac_vap->vap_mode, hmac_vap->vap_state, hmac_vap->user_nums);
        return OAL_FAIL;
    }
    /* 根据协议模式更新 HT/VHT mib值 */
    if (hmac_vap->protocol == WLAN_HT_MODE || hmac_vap->protocol == WLAN_HT_ONLY_MODE) {
        mac_mib_set_high_throughput_option_implemented(hmac_vap, OAL_TRUE);
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_FALSE);

#ifdef _PRE_WLAN_FEATURE_11AX
        mac_mib_set_he_option_implemented(hmac_vap, OAL_FALSE);
#endif
    } else if (hmac_vap->protocol == WLAN_VHT_MODE || hmac_vap->protocol == WLAN_VHT_ONLY_MODE) {
        mac_mib_set_high_throughput_option_implemented(hmac_vap, OAL_TRUE);
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_TRUE);
#ifdef _PRE_WLAN_FEATURE_11AX
        mac_mib_set_he_option_implemented(hmac_vap, OAL_FALSE);
    } else if (hmac_vap->protocol == WLAN_HE_MODE) {
        mac_mib_set_high_throughput_option_implemented(hmac_vap, OAL_TRUE);
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_TRUE);
        mac_mib_set_he_option_implemented(hmac_vap, OAL_TRUE);
#endif
    } else {
        mac_mib_set_high_throughput_option_implemented(hmac_vap, OAL_FALSE);
        mac_mib_set_vht_option_implemented(hmac_vap, OAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_11AX
        mac_mib_set_he_option_implemented(hmac_vap, OAL_FALSE);
#endif
    }

    hmac_vap_config_vht_mib_by_band(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 初始化rx nss
*****************************************************************************/
void hmac_vap_init_rx_nss_by_protocol_etc(hmac_vap_stru *hmac_vap)
{
    wlan_protocol_enum_uint8 protocol;
    hmac_device_stru *hmac_device;

    protocol = hmac_vap->protocol;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_vap_init_rx_nss_by_protocol_etc::hmac_device[%d] null.}", hmac_vap->device_id);
        return;
    }

    switch (protocol) {
        case WLAN_HT_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
#endif
            hmac_vap->vap_rx_nss = WLAN_DOUBLE_NSS;
            break;
        case WLAN_PROTOCOL_BUTT:
            hmac_vap->vap_rx_nss = WLAN_NSS_LIMIT;
            return;

        default:
            hmac_vap->vap_rx_nss = WLAN_SINGLE_NSS;
            break;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_vap->vap_rx_nss = OAL_MIN(hmac_vap->vap_rx_nss, OAL_MIN(mac_device_get_nss_num(hmac_device),
        mac_m2s_cali_nss_from_smps_mode(mac_device_get_mode_smps(hmac_device))));
#else
    hmac_vap->vap_rx_nss = OAL_MIN(hmac_vap->vap_rx_nss, mac_device_get_nss_num(hmac_device));
#endif
}

/*****************************************************************************
 功能描述 : 依据协议初始化vap相应能力
*****************************************************************************/
osal_u32 hmac_vap_init_by_protocol_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 protocol)
{
    hmac_vap->protocol = protocol;

    if (protocol < WLAN_HT_MODE) {
        hmac_vap_cap_init_legacy(hmac_vap);
    } else {
        if (hmac_vap_cap_init_htvht(hmac_vap) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    hmac_vap_init_bw_by_chip_and_protocol(hmac_vap);
    /* 根据协议模式更新mib值 */
    if (hmac_vap_config_vht_ht_mib_by_protocol_etc(hmac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 根据协议更新初始化空间流个数 */
    hmac_vap_init_rx_nss_by_protocol_etc(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 根据带宽模式获取带宽宽度
*****************************************************************************/
wlan_bw_cap_enum_uint8 hmac_vap_bw_mode_to_bw(wlan_channel_bandwidth_enum_uint8 mode)
{
    wlan_channel_bandwidth_enum_uint8 bw_mode = mode;
    static wlan_bw_cap_enum_uint8 g_bw_mode_tbl[] = {
        WLAN_BW_CAP_20M, // WLAN_BAND_WIDTH_20M
        WLAN_BW_CAP_40M, // WLAN_BAND_WIDTH_40PLUS
        WLAN_BW_CAP_40M, // WLAN_BAND_WIDTH_40MINUS
        WLAN_BW_CAP_80M, // WLAN_BAND_WIDTH_80PLUSPLUS
        WLAN_BW_CAP_80M, // WLAN_BAND_WIDTH_80PLUSMINUS
        WLAN_BW_CAP_80M, // WLAN_BAND_WIDTH_80MINUSPLUS
        WLAN_BW_CAP_80M, // WLAN_BAND_WIDTH_80MINUSMINUS
        WLAN_BW_CAP_5M,  // WLAN_BAND_WIDTH_5M
        WLAN_BW_CAP_10M, // WLAN_BAND_WIDTH_10M
        WLAN_BW_CAP_40M, // WLAN_BAND_WIDTH_40M
        WLAN_BW_CAP_80M, // WLAN_BAND_WIDTH_80M
    };

    if (mode >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_vap_bw_mode_to_bw::invalid mode = %d, force 20M", mode);
        bw_mode = 0;
    }

    return g_bw_mode_tbl[bw_mode];
}

/*****************************************************************************
 功能描述 : 根据带宽更改vap的mib值
*****************************************************************************/
void hmac_vap_change_mib_by_bandwidth_etc(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    wlan_mib_ieee802dot11_stru *mib_info;
    wlan_bw_cap_enum_uint8 bw;

    mib_info = hmac_vap->mib_info;

    if (mib_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_change_mib_by_bandwidth_etc::pst_mib_info null.}");
        return;
    }

    /* 更新40M使能mib, 默认使能 */
    mac_mib_set_forty_mhz_operation_implemented(hmac_vap, OAL_TRUE);

    // 去掉shortGI强制使能，使用iwpriv命令设置，否则用户配置会被覆盖
    /* 更新short gi使能mib, 默认全使能，根据带宽信息更新 */
    mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap, OAL_TRUE);
    mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap, OSAL_TRUE);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if ((hmac_vap->channel.band == WLAN_BAND_2G) && (hmac_vap->cap_flag.disable_2ght40 == OAL_TRUE)) {
        mac_mib_set_forty_mhz_operation_implemented(hmac_vap, OAL_FALSE);
    }
#endif

    // 去掉shortGI强制使能，使用iwpriv命令设置，否则用户配置会被覆盖
    mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap, OAL_TRUE);

    bw = hmac_vap_bw_mode_to_bw(bandwidth);
    if (bw == WLAN_BW_CAP_20M) {
        mac_mib_set_forty_mhz_operation_implemented(hmac_vap, OAL_FALSE);
    }

    if (g_st_mac_vap_rom_cb.ch_mib_by_bw_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.ch_mib_by_bw_cb(hmac_vap, band, bandwidth);
    }
}

/*****************************************************************************
 功能描述 : 设置mib信息中当前信道
*****************************************************************************/
osal_u32 mac_vap_set_current_channel_etc(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band, osal_u8 channel)
{
    osal_u8 channel_idx = 0;
    osal_u32 ret;

    /* 检查信道号 */
    ret = hmac_is_channel_num_valid_etc(band, channel);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 根据信道号找到索引号 */
    ret = hmac_get_channel_idx_from_num_etc(band, channel, &channel_idx);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hmac_vap->channel.chan_number = channel;
    hmac_vap->channel.band = band;
    hmac_vap->channel.chan_idx = channel_idx;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : VAP状态迁移事件以消息形式上报SDT
*****************************************************************************/
void hmac_vap_state_change_etc(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 vap_state)
{
    hmac_vap->vap_state = vap_state;
    hmac_vap_sync(hmac_vap);
}

#ifdef _PRE_WIFI_DEBUG
/*****************************************************************************
 功能描述 : 检查能力信息中与PHY相关的信息
*****************************************************************************/
oal_bool_enum_uint8 mac_vap_check_bss_cap_info_phy_ap_etc(osal_u16 cap_info, hmac_vap_stru *hmac_vap)
{
    mac_cap_info_stru *cap_info_temp = (mac_cap_info_stru *)(&cap_info);

    if (hmac_vap->channel.band != WLAN_BAND_2G) {
        return OAL_TRUE;
    }

    /* PBCC */
    if ((mac_mib_get_pbcc_option_implemented(hmac_vap) == OAL_FALSE) && (cap_info_temp->pbcc == 1)) {
        oam_warning_log0(0, OAM_SF_ANY, "{mac_vap_check_bss_cap_info_phy_ap_etc::PBCC is different.}");
    }

    /* Channel Agility */
    if ((mac_mib_get_channel_agility_present(hmac_vap) == OAL_FALSE) && (cap_info_temp->channel_agility == 1)) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{mac_vap_check_bss_cap_info_phy_ap_etc::Channel Agility is different.}");
    }

    /* DSSS-OFDM Capabilities */
    if ((mac_mib_get_dsss_ofdm_option_activated(hmac_vap) == OAL_FALSE) && (cap_info_temp->dsss_ofdm == 1)) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{mac_vap_check_bss_cap_info_phy_ap_etc::DSSS-OFDM Capabilities is different.}");
    }

    return OAL_TRUE;
}
#endif

/*****************************************************************************
 功能描述 : 获取vap带宽能力
*****************************************************************************/
void hmac_vap_get_bandwidth_cap_etc(const hmac_vap_stru *hmac_vap, wlan_bw_cap_enum_uint8 *cap)
{
    wlan_bw_cap_enum_uint8 band_cap = WLAN_BW_CAP_20M;

    if (mac_vap_get_cap_bw(hmac_vap) == WLAN_BAND_WIDTH_40PLUS ||
        mac_vap_get_cap_bw(hmac_vap) == WLAN_BAND_WIDTH_40MINUS) {
        band_cap = WLAN_BW_CAP_40M;
    } else if (mac_vap_get_cap_bw(hmac_vap) >= WLAN_BAND_WIDTH_80PLUSPLUS &&
        mac_vap_get_cap_bw(hmac_vap) <= WLAN_BAND_WIDTH_80MINUSMINUS) {
        band_cap = WLAN_BW_CAP_80M;
    }

    *cap = band_cap;
}

/*****************************************************************************
 功能描述 : 根据初始化的dev带宽能力和bss的带宽能力决定当前需要使用的带宽
*****************************************************************************/
osal_u8 hmac_vap_get_bandwith(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bss_cap)
{
    wlan_channel_bandwidth_enum_uint8 band_with = WLAN_BAND_WIDTH_20M;
    wlan_bw_cap_enum_uint8 dev_cap = WLAN_BW_CAP_20M;

    if (bss_cap >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_vap_get_bandwith:bss cap is invaild dev_cap[%d] to bss_cap[%d]",
            dev_cap, bss_cap);
        return band_with;
    }

    dev_cap = mac_mib_get_dot11_vap_max_bandwidth(hmac_vap);
#ifdef _PRE_WLAN_11AX_20M_ONLY
    if (hmac_vap->protocol == WLAN_HE_MODE) {
        dev_cap = WLAN_BW_CAP_20M;
    }
#endif
#ifdef _PRE_WLAN_11AC_20M_ONLY
    if (hmac_vap->protocol == WLAN_VHT_MODE) {
        dev_cap = WLAN_BW_CAP_20M;
    }
#endif

    switch (dev_cap) {
        case WLAN_BW_CAP_20M:
            break;

        case WLAN_BW_CAP_40M:
            if (bss_cap <= WLAN_BAND_WIDTH_40MINUS) {
                band_with = bss_cap;
            } else if ((bss_cap >= WLAN_BAND_WIDTH_80PLUSPLUS) && (bss_cap <= WLAN_BAND_WIDTH_80PLUSMINUS)) {
                band_with = WLAN_BAND_WIDTH_40PLUS;
            } else if ((bss_cap >= WLAN_BAND_WIDTH_80MINUSPLUS) && (bss_cap <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
                band_with = WLAN_BAND_WIDTH_40MINUS;
            }
            break;

        case WLAN_BW_CAP_80M:
            if (bss_cap <= WLAN_BAND_WIDTH_80MINUSMINUS) {
                band_with = bss_cap;
            }

            break;
        default:
            oam_error_log2(0, OAM_SF_ANY, "hmac_vap_get_bandwith: bandwith dev_cap[%d] to bss_cap[%d]", dev_cap,
                bss_cap);
            break;
    }
    return band_with;
}

#ifdef _PRE_WIFI_DEBUG
osal_u32 mac_dump_protection_etc(hmac_vap_stru *hmac_vap)
{
    mac_protection_stru *protection = OSAL_NULL;
    wlan_mib_dot11operationentry_stru *mib = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        return OAL_FAIL;
    }

    protection = &hmac_vap->protection;
    mib = &hmac_vap->mib_info->wlan_mib_operation;

    oam_info_log4(0, OAM_SF_ANY,
        "dot11RIFSMode=%d,dot11LSIGTXOPFullProtectionActivated=%d,dot11NonGFEntitiesPresent=%d,protection_mode=%d",
        mib->dot11_rifs_mode, mib->dot11_lsigtxop_full_protection_activated, mib->dot11_non_gf_entities_present,
        protection->protection_mode);
    oam_info_log4(0, OAM_SF_ANY,
        "obss_non_erp_aging_cnt=%d,obss_non_ht_aging_cnt=%d,auto_protection=%d,obss_non_erp_present=%d",
        protection->obss_non_erp_aging_cnt, protection->obss_non_ht_aging_cnt, protection->auto_protection,
        protection->obss_non_erp_present);
    oam_info_log3(0, OAM_SF_ANY,
        "obss_non_ht_present=%d,lsig_txop_protect_mode=%d,sta_no_short_slot_num=%d", protection->obss_non_ht_present,
        protection->lsig_txop_protect_mode, protection->sta_no_short_slot_num);
    oam_info_log4(0, OAM_SF_ANY,
        "sta_no_short_preamble_num=%d,sta_non_erp_num=%d,sta_non_ht_num=%d,sta_non_gf_num=%d",
        protection->sta_no_short_preamble_num, protection->sta_non_erp_num, protection->sta_non_ht_num,
        protection->sta_non_gf_num);
    oam_info_log3(0, OAM_SF_ANY,
        "sta_20M_only_num=%d,sta_no_40dsss_cck_num=%d,sta_no_lsig_txop_num=%d", protection->sta_20m_only_num,
        protection->sta_no_40dsss_cck_num, protection->sta_no_lsig_txop_num);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 功能描述 : 获取pmf的配置能力
*****************************************************************************/
OSAL_STATIC osal_u32 mac_vap_init_pmf(hmac_vap_stru  *hmac_vap, osal_u8 pmf_cap, osal_u8 mgmt_proteced)
{
    switch (pmf_cap) {
        case MAC_PMF_DISABLED:
            mac_mib_set_dot11_rsnamfpc(hmac_vap, OAL_FALSE);
            mac_mib_set_dot11_rsnamfpr(hmac_vap, OAL_FALSE);
            break;
        case MAC_PMF_ENABLED:
            mac_mib_set_dot11_rsnamfpc(hmac_vap, OAL_TRUE);
            mac_mib_set_dot11_rsnamfpr(hmac_vap, OAL_FALSE);
            break;
        case MAC_PMF_REQUIRED:
            mac_mib_set_dot11_rsnamfpc(hmac_vap, OAL_TRUE);
            mac_mib_set_dot11_rsnamfpr(hmac_vap, OAL_TRUE);
            break;
        default:
            return OAL_FAIL;
    }

    if (mgmt_proteced == MAC_NL80211_MFP_REQUIRED) {
        hmac_vap->user_pmf_cap = OAL_TRUE;
    } else {
        hmac_vap->user_pmf_cap = OAL_FALSE;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : add wep key逻辑,抛事件到DMAC,wep密钥保存在组播用户中
*****************************************************************************/
osal_u32 hmac_vap_add_wep_key(hmac_vap_stru *hmac_vap, osal_u8 key_idx, osal_u8 key_len, osal_u8 *key)
{
    hmac_user_stru *multi_user = OAL_PTR_NULL;
    wlan_priv_key_param_stru *wep_key = OAL_PTR_NULL;
    osal_u32 cipher_type = WITP_WLAN_CIPHER_SUITE_WEP40;
    osal_u8 wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;

    /* wep 密钥最大为4个 */
    if (key_idx >= WLAN_MAX_WEP_KEY_COUNT) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch (key_len) {
        case WLAN_WEP40_KEY_LEN:
            wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            cipher_type = WITP_WLAN_CIPHER_SUITE_WEP40;
            break;
        case WLAN_WEP104_KEY_LEN:
            wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            cipher_type = WITP_WLAN_CIPHER_SUITE_WEP104;
            break;
        default:
            return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    /* WEP密钥信息记录到组播用户中 */
    multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    mac_mib_set_privacyinvoked(hmac_vap, OAL_TRUE);

    /* 初始化组播用户的安全信息 */
    multi_user->key_info.cipher_type = wep_cipher_type;
    multi_user->key_info.default_index = key_idx;
    multi_user->key_info.igtk_key_index = 0xff; /* wep时设置为无效 */
    multi_user->key_info.gtk = 0;

    wep_key = &multi_user->key_info.key[key_idx];

    wep_key->cipher = cipher_type;
    wep_key->key_len = (osal_u32)key_len;

    (osal_void)memcpy_s(wep_key->key, WLAN_WPA_KEY_LEN, key, WLAN_WPA_KEY_LEN);

    /* 挪出去 初始化组播用户的发送信息 */
    multi_user->user_tx_info.security.cipher_key_type = key_idx + HAL_KEY_TYPE_PTK;
    multi_user->user_tx_info.security.cipher_protocol_type = wep_cipher_type;

    hmac_user_sync(multi_user);
    return OAL_SUCC;
}

static osal_void hmac_vap_init_crypto_suite(hmac_vap_stru *hmac_vap, mac_crypto_settings_stru *crypto)
{
    if (crypto->wpa_versions == WITP_WPA_VERSION_1) {
        hmac_vap->cap_flag.wpa = OAL_TRUE;
        mac_mib_set_wpa_pair_suites(hmac_vap, crypto->pair_suite);
        mac_mib_set_wpa_akm_suites(hmac_vap, crypto->akm_suite);
        mac_mib_set_wpa_group_suite(hmac_vap, crypto->group_suite);
    } else if (crypto->wpa_versions == WITP_WPA_VERSION_2) {
#ifdef _PRE_WLAN_FEATURE_WPA3
        hmac_vap->cap_flag.wpa3 = (crypto->akm_suite[0] == 0x08ac0f00) ? OSAL_TRUE : OSAL_FALSE; /* SAE */
        hmac_vap->cap_flag.wpa3 |= (crypto->akm_suite[1] == 0x08ac0f00) ? OSAL_TRUE : OSAL_FALSE; /* SAE */
#endif
        hmac_vap->cap_flag.wpa2 = OSAL_TRUE;
        mac_mib_set_rsn_pair_suites(hmac_vap, crypto->pair_suite);
        mac_mib_set_rsn_akm_suites(hmac_vap, crypto->akm_suite);
        mac_mib_set_rsn_group_suite(hmac_vap, crypto->group_suite);
        mac_mib_set_rsn_group_mgmt_suite(hmac_vap, crypto->group_mgmt_suite);
    }
}

/*****************************************************************************
 功能描述 : 根据内核下发的关联能力，赋值加密相关的mib 值
*****************************************************************************/
osal_u32 hmac_vap_init_privacy_etc(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec)
{
    mac_crypto_settings_stru *crypto;
#ifdef _PRE_WLAN_FEATURE_PMF
    osal_u32 ret;
#endif
    mac_mib_set_privacyinvoked(hmac_vap, OAL_FALSE);
    /* 初始化 RSNActive 为FALSE */
    mac_mib_set_rsnaactivated(hmac_vap, OAL_FALSE);
    /* 清除加密套件信息 */
    mac_mib_init_rsnacfg_suites(hmac_vap);

    hmac_vap->cap_flag.wpa = OAL_FALSE;
    hmac_vap->cap_flag.wpa2 = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_vap->cap_flag.wpa3 = OAL_FALSE;
#endif

    /* 刷新pmf能力,解决从pmf ap下关联至open ap, pmf未更新导致收发异常 */
#ifdef _PRE_WLAN_FEATURE_PMF
    ret = mac_vap_init_pmf(hmac_vap, conn_sec->pmf_cap, conn_sec->mgmt_proteced);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_WPA, "{mac_11i_init_privacy::mac_11w_init_privacy failed[%d].}", ret);
        return ret;
    }
#endif
    /* 不加密 */
    if (conn_sec->privacy == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* WEP加密 */
    if (conn_sec->wep_key_len != 0) {
        return hmac_vap_add_wep_key(hmac_vap, conn_sec->wep_key_index, conn_sec->wep_key_len,
            conn_sec->wep_key);
    }

    /* WPA/WPA2加密 */
    crypto = &(conn_sec->crypto);

    /* 初始化RSNA mib 为 TRUR */
    mac_mib_set_privacyinvoked(hmac_vap, OAL_TRUE);
    mac_mib_set_rsnaactivated(hmac_vap, OAL_TRUE);

    /* 初始化单播密钥套件 */
    hmac_vap_init_crypto_suite(hmac_vap, crypto);

    if (g_st_mac_vap_rom_cb.init_privacy_cb != OAL_PTR_NULL) {
        g_st_mac_vap_rom_cb.init_privacy_cb(hmac_vap, conn_sec);
    }

    return OAL_SUCC;
}

osal_void mac_mib_set_wep_etc(hmac_vap_stru *hmac_vap, osal_u8 key_id, osal_u8 key_value)
{
    if (key_id >= WLAN_NUM_DOT11WEPDEFAULTKEYVALUE) {
        return;
    }

    /* 初始化wep相关MIB信息 */
    memset_s(hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_wep_dflt_key[key_id].dot11_wep_default_key_value,
        WLAN_MAX_WEP_STR_SIZE, 0, WLAN_MAX_WEP_STR_SIZE);
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_wep_dflt_key[key_id]
        .dot11_wep_default_key_value[WLAN_WEP_SIZE_OFFSET] = key_value;
}

/*****************************************************************************
 功能描述 : 根据mac地址获取mac_user指针
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_user_stru *hmac_vap_get_user_by_addr_etc(hmac_vap_stru *hmac_vap,
    const osal_u8 *mac_addr, size_t mac_len)
{
    osal_u32 ret;
    osal_u16 user_idx = 0xffff;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    if (mac_len > WLAN_MAC_ADDR_LEN) {
        return OAL_PTR_NULL;
    }

    /* 根据mac addr找到sta索引 */
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_vap_get_user_by_addr_etc::find_user_by_macaddr failed[%d].}", ret);
        if (mac_addr != OAL_PTR_NULL) {
            oam_warning_log4(0, OAM_SF_ANY, "{hmac_vap_get_user_by_addr_etc::mac[%x:%x:%x:%x:XX:XX] cant be found!}",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* macaddr第0,1,2,3字节 */
        }
        return OAL_PTR_NULL;
    }

    /* 根据sta索引找到user内存区域 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_vap_get_user_by_addr_etc::user[%d] ptr null.}", user_idx);
    }

    return hmac_user;
}

static osal_u32 hmac_vap_set_security_enable(hmac_vap_stru *hmac_vap, mac_beacon_param_stru *beacon_param)
{
    osal_u32 ret;
    mac_crypto_settings_stru crypto;
    osal_u16 rsn_cap;

    /* 使能加密 */
    mac_mib_set_privacyinvoked(hmac_vap, OAL_TRUE);

    memset_s(&crypto, OAL_SIZEOF(mac_crypto_settings_stru), 0, OAL_SIZEOF(mac_crypto_settings_stru));

    if (beacon_param->wpa_ie[0] == MAC_EID_VENDOR) {
        hmac_vap->cap_flag.wpa = OAL_TRUE;
        mac_mib_set_rsnaactivated(hmac_vap, OAL_TRUE);
        mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
        hmac_ie_get_wpa_cipher(beacon_param->wpa_ie, &crypto);
        mac_mib_set_wpa_group_suite(hmac_vap, crypto.group_suite);
        mac_mib_set_wpa_pair_suites(hmac_vap, crypto.pair_suite);
        mac_mib_set_wpa_akm_suites(hmac_vap, crypto.akm_suite);
    }

    if (beacon_param->rsn_ie[0] == MAC_EID_RSN) {
        mac_mib_set_rsnaactivated(hmac_vap, OAL_TRUE);
        mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
        ret = hmac_ie_get_rsn_cipher(beacon_param->rsn_ie, &crypto);
        if (ret != OAL_SUCC) {
            return ret;
        }
#ifdef _PRE_WLAN_FEATURE_WPA3
        hmac_vap->cap_flag.wpa3 = (crypto.akm_suite[0] == 0x08ac0f00) ? OSAL_TRUE : OSAL_FALSE; /* SAE */
        hmac_vap->cap_flag.wpa3 |= (crypto.akm_suite[1] == 0x08ac0f00) ? OSAL_TRUE : OSAL_FALSE; /* SAE */
#endif
        hmac_vap->cap_flag.wpa2 = OSAL_FALSE;
        if ((crypto.akm_suite[0] == 0x02ac0f00) || (crypto.akm_suite[1] == 0x02ac0f00) || /* PSK */
            ((crypto.akm_suite[0] == 0x12ac0f00) || (crypto.akm_suite[1] == 0x12ac0f00)) || /* OWE */
            (crypto.akm_suite[0] == 0x06ac0f00) || (crypto.akm_suite[1] == 0x06ac0f00)) { /* PSK-256 */
            hmac_vap->cap_flag.wpa2 = OSAL_TRUE;
        }
        rsn_cap = hmac_get_rsn_capability_etc(beacon_param->rsn_ie);
        mac_mib_set_rsn_group_suite(hmac_vap, crypto.group_suite);
        mac_mib_set_rsn_pair_suites(hmac_vap, crypto.pair_suite);
        mac_mib_set_rsn_akm_suites(hmac_vap, crypto.akm_suite);
        mac_mib_set_rsn_group_mgmt_suite(hmac_vap, crypto.group_mgmt_suite);
        /* RSN 能力 */
        mac_mib_set_dot11_rsnamfpr(hmac_vap, ((rsn_cap & BIT6) != 0) ? OAL_TRUE : OAL_FALSE);
        mac_mib_set_dot11_rsnamfpc(hmac_vap, ((rsn_cap & BIT7) != 0) ? OAL_TRUE : OAL_FALSE);
        mac_mib_set_pre_auth_actived(hmac_vap, rsn_cap & BIT0);
        mac_mib_set_rsnacfg_ptksareplaycounters(hmac_vap, (osal_u8)(rsn_cap & 0x0C) >> 2);   /* 右移2 bit */
        mac_mib_set_rsnacfg_gtksareplaycounters(hmac_vap, (osal_u8)(rsn_cap & 0x30) >> 4);   /* 右移4 bit */
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置vap的安全相关的信息
*****************************************************************************/
osal_u32 hmac_vap_set_security(hmac_vap_stru *hmac_vap, mac_beacon_param_stru *beacon_param)
{
    hmac_user_stru *multi_user;

    if ((hmac_vap == OAL_PTR_NULL) || (beacon_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_WPA, "{mac_vap_add_beacon::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 清除之前的加密配置信息 */
    mac_mib_set_privacyinvoked(hmac_vap, OAL_FALSE);
    mac_mib_set_rsnaactivated(hmac_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(hmac_vap);
    hmac_vap->cap_flag.wpa = OAL_FALSE;
    hmac_vap->cap_flag.wpa2 = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_vap->cap_flag.wpa3 = OAL_FALSE;
    hmac_vap->sae_pwe = beacon_param->sae_pwe;
#endif
    mac_mib_set_dot11_rsnamfpr(hmac_vap, OAL_FALSE);
    mac_mib_set_dot11_rsnamfpc(hmac_vap, OAL_FALSE);

    /* 清除组播密钥信息 */
    multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{mac_vap_add_beacon::multi_user[%d] null.}", hmac_vap->multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (beacon_param->privacy == OAL_FALSE) {
        /*
         * 清除组播用户加密信息位置由vap down移动到此处;解决vap down/up后STA关联APUT失败问题;只在非加密场景下清除,
         * 加密场景会重新设置覆盖
         */
        hmac_user_init_key_etc(multi_user);
        multi_user->user_tx_info.security.cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
        return OAL_SUCC;
    }

    return hmac_vap_set_security_enable(hmac_vap, beacon_param);
}

/*****************************************************************************
 功能描述 : 根据en_key_type，调用相应的函数，更新vap信息
*****************************************************************************/
osal_u32 hmac_vap_add_key_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 key_id,
    mac_key_params_stru *key)
{
    osal_u32 ret;

    switch ((osal_u8)key->cipher) {
        case WLAN_80211_CIPHER_SUITE_WEP_40:
        case WLAN_80211_CIPHER_SUITE_WEP_104:
            /* 设置mib */
            mac_mib_set_privacyinvoked(hmac_vap, OAL_TRUE);
            mac_mib_set_rsnaactivated(hmac_vap, OAL_FALSE);
            /* 设置组播密钥套件应该放在set default key */
            ret = hmac_user_add_wep_key_etc(hmac_user, key_id, key);
            /* set default key接口未适配，此处写死使用index 0 */
            if ((ret == OAL_SUCC) && (key_id == 0)) {
                mac_set_wep_default_keyid(hmac_vap, key_id);
            }
            break;
        case WLAN_80211_CIPHER_SUITE_TKIP:
        case WLAN_80211_CIPHER_SUITE_CCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP_256:
        case WLAN_80211_CIPHER_SUITE_CCMP_256:
            ret = hmac_user_add_rsn_key_etc(hmac_user, key_id, key);
            break;
        case WLAN_80211_CIPHER_SUITE_BIP:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_128:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_256:
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            ret = hmac_user_add_bip_key_etc(hmac_user, key_id, key);
            break;
        default:
            return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    return ret;
}

/*****************************************************************************
 功能描述 : 初始化STA加密情况下，数据过滤的参数。
*****************************************************************************/
void hmac_vap_init_user_security_port_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    if (mac_mib_get_rsnaactivated(hmac_vap) != OAL_TRUE) {
        hmac_user_set_port_etc(hmac_user, OAL_TRUE);
        return;
    }
}

/*****************************************************************************
 功能描述 : 基于device增量或者删除一个vap，并且维护vap记数
*****************************************************************************/
void hmac_device_set_vap_id_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, osal_u8 vap_idx,
    const hmac_cfg_add_vap_param_stru *vap_param, osal_u8 is_add_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u8 vap_tmp_idx;
    hmac_vap_stru *tmp_vap;
#endif

    if (is_add_vap != 0) {
        hmac_device->vap_id[hmac_device->vap_num++] = vap_idx;

        if (vap_param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_device->sta_num++;

            hmac_vap->assoc_vap_id = MAC_INVALID_USER_ID;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        hmac_vap->p2p_mode = vap_param->add_vap.p2p_mode;
        hmac_inc_p2p_num_etc(hmac_vap);
        if (!is_p2p_go(hmac_vap)) {
            return;
        }

        for (vap_tmp_idx = 0; vap_tmp_idx < hmac_device->vap_num; vap_tmp_idx++) {
            tmp_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_tmp_idx]);
            if (tmp_vap == OAL_PTR_NULL) {
                oam_error_log1(0, OAM_SF_SCAN, "{hmac_config_add_vap::hmac_vap null,vap_idx=%d.}",
                    hmac_device->vap_id[vap_tmp_idx]);
                continue;
            }

            if ((tmp_vap->vap_state == MAC_VAP_STATE_UP) && (tmp_vap != hmac_vap)) {
                hmac_vap->channel.band          = tmp_vap->channel.band;
                hmac_vap->channel.en_bandwidth  = tmp_vap->channel.en_bandwidth;
                hmac_vap->channel.chan_number   = tmp_vap->channel.chan_number;
                hmac_vap->channel.chan_idx      = tmp_vap->channel.chan_idx;
                break;
            }
        }
#endif
    } else {
        hmac_device->vap_id[hmac_device->vap_num--] = 0;

        if (vap_param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_device->sta_num--;

            hmac_vap->assoc_vap_id = MAC_INVALID_USER_ID;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        hmac_vap->p2p_mode = vap_param->add_vap.p2p_mode;
        hmac_dec_p2p_num_etc(hmac_vap);
#endif
    }
}

/*****************************************************************************
 功能描述 : 寻找处在UP状态的VAP
*****************************************************************************/
osal_u32 mac_device_find_up_vap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap)
{
    osal_u8 vap_idx;
    hmac_vap_stru *mac_vap_temp;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        mac_vap_temp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(mac_vap_temp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);

            *hmac_vap = OAL_PTR_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if (mac_vap_temp->vap_state == MAC_VAP_STATE_UP || mac_vap_temp->vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
            || (mac_vap_temp->vap_state == MAC_VAP_STATE_LISTEN && mac_vap_temp->user_nums > 0)
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (mac_vap_temp->vap_state == MAC_VAP_STATE_ROAMING)
#endif // _PRE_WLAN_FEATURE_ROAM
        ) {
            *hmac_vap = mac_vap_temp;

            return OAL_SUCC;
        }
    }

    *hmac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述 : 寻找处在UP状态的AP VAP
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 mac_device_find_up_ap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap_temp;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_temp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap_temp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if ((hmac_vap_temp->vap_state == MAC_VAP_STATE_UP || hmac_vap_temp->vap_state == MAC_VAP_STATE_PAUSE) &&
            (hmac_vap_temp->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            *hmac_vap = hmac_vap_temp;

            return OAL_SUCC;
        }
    }

    *hmac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述 : 计算up的vap个数
*****************************************************************************/
osal_u32 hmac_device_calc_up_vap_num_etc(hmac_device_stru *hmac_device)
{
    hmac_vap_stru *hmac_vap_tmp;
    osal_u8 vap_idx;
    osal_u8 up_ap_num = 0;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d", hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP) || (hmac_vap_tmp->vap_state == MAC_VAP_STATE_PAUSE)
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (hmac_vap_tmp->vap_state == MAC_VAP_STATE_ROAMING)
#endif // _PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_P2P
            || (hmac_vap_tmp->vap_state == MAC_VAP_STATE_LISTEN && hmac_vap_tmp->user_nums > 0)
#endif
        ) {
            up_ap_num++;
        }
    }

    return up_ap_num;
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述 : 寻找处在UP状态的 P2P_GO
*****************************************************************************/
osal_u32 mac_device_find_up_p2p_go_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap_temp;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_temp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap_temp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((hmac_vap_temp->vap_state == MAC_VAP_STATE_UP || hmac_vap_temp->vap_state == MAC_VAP_STATE_PAUSE) &&
            (hmac_vap_temp->p2p_mode == WLAN_P2P_GO_MODE)) {
            *hmac_vap = hmac_vap_temp;

            return OAL_SUCC;
        }
    }

    *hmac_vap = OAL_PTR_NULL;

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述 : 判断p2p设备是否关连
*****************************************************************************/
osal_u32 mac_device_is_p2p_connected_etc(hmac_device_stru *hmac_device)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        if ((is_p2p_go(hmac_vap) || is_p2p_cl(hmac_vap)) && (hmac_vap->user_nums > 0)) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
#endif

/*****************************************************************************
 功能描述 : 寻找处在UP状态的两个VAP
*****************************************************************************/
osal_u32 hmac_device_find_2up_vap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_up_vap1,
    hmac_vap_stru **hmac_up_vap2)
{
    hmac_vap_stru *hmac_vap_tmp;
    osal_u8 vap_idx;
    osal_u8 up_vap_num = 0;
    hmac_vap_stru *hmac_vap_up_arr[2] = {0};

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap_tmp is null, vap id is %d", hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP) ||
            (hmac_vap_tmp->vap_state == MAC_VAP_STATE_PAUSE) ||
#ifdef _PRE_WLAN_FEATURE_ROAM
            (hmac_vap_tmp->vap_state == MAC_VAP_STATE_ROAMING) ||
#endif // _PRE_WLAN_FEATURE_ROAM
            (hmac_vap_tmp->vap_state == MAC_VAP_STATE_LISTEN && hmac_vap_tmp->user_nums > 0)) {
            hmac_vap_up_arr[up_vap_num] = hmac_vap_tmp;
            up_vap_num++;
        }
        if (up_vap_num >= 2) { /* 2表示vap个数 */
            break;
        }
    }

    if (up_vap_num < 2) { /* 2表示vap个数 */
        return OAL_FAIL;
    }

    *hmac_up_vap1 = hmac_vap_up_arr[0];
    *hmac_up_vap2 = hmac_vap_up_arr[1];

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
 功能描述 : DEVICE SMPS能力是否可以配置
*****************************************************************************/
osal_u32 hmac_device_find_smps_mode_en(hmac_device_stru *hmac_device,
    wlan_mib_mimo_power_save_enum_uint8 smps_mode)
{
    hmac_vap_stru *hmac_vap = NULL;
    osal_u8 vap_idx;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == NULL) {
            oam_error_log0(0, OAM_SF_SMPS, "{hmac_device_find_smps_mode_en::hmac_vap null.}");
            continue;
        }

        /* 存在不支持HT则MIMO，模式不可配置 */
        if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OAL_FALSE) {
            oam_warning_log0(0, OAM_SF_SMPS, "{hmac_device_find_smps_mode_en::exist none ht vap.}");
            return OAL_FALSE;
        }

        /* 存在一个vap MIB不支持SMPS则MIMO */
        if (mac_mib_get_smps(hmac_vap) == WLAN_MIB_MIMO_POWER_SAVE_BUTT) {
            oam_warning_log0(0, OAM_SF_SMPS,
                "{hmac_device_find_smps_mode_en::exist no support SMPS vap.}");
            return OAL_FALSE;
        }

        /* 获取当前SMPS模式，若未改变则直接返回，模式不可配置来处理(vap和device smps mode始终保持一致) */
        if (smps_mode == mac_mib_get_smps(hmac_vap)) {
            oam_warning_log2(0, OAM_SF_SMPS,
                "{hmac_device_find_smps_mode_en::vap smps mode[%d]unchanged smps mode[%d].}",
                hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_mimo_power_save, smps_mode);
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}
#endif

void hmac_device_set_channel_etc(hmac_device_stru *hmac_device, mac_cfg_channel_param_stru *channel_param)
{
    hmac_device->max_channel = channel_param->channel;
    hmac_device->max_band = channel_param->band;
    hmac_device->max_bandwidth = channel_param->en_bandwidth;
}

void hmac_device_get_channel_etc(hmac_device_stru *hmac_device, mac_cfg_channel_param_stru *channel_param)
{
    channel_param->channel = hmac_device->max_channel;
    channel_param->band = hmac_device->max_band;
    channel_param->en_bandwidth = hmac_device->max_bandwidth;
}
// mac_vap_rom.c
/*****************************************************************************
 函 数 名  : hmac_vap_check_ap_usr_opern_bandwidth
 功能描述  : 根据AP的operation ie读取带宽值与当前vap工作带宽比较
*****************************************************************************/
osal_u32 hmac_vap_check_ap_usr_opern_bandwidth(const hmac_vap_stru *mac_sta, const hmac_user_stru *hmac_user)
{
    const mac_user_ht_hdl_stru *mac_ht_hdl = OSAL_NULL;
    const mac_vht_hdl_stru *mac_vht_hdl = OSAL_NULL;
    wlan_channel_bandwidth_enum_uint8 sta_current_bw;
    wlan_channel_bandwidth_enum_uint8 bandwidth_ap = WLAN_BAND_WIDTH_20M;
    osal_u32 change = 0;
#ifdef _PRE_WLAN_FEATURE_11AX
    const mac_he_hdl_stru *mac_he_hdl = OSAL_NULL;
#endif

    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl = &(hmac_user->ht_hdl);
    mac_vht_hdl = &(hmac_user->vht_hdl);

    sta_current_bw = mac_vap_get_cap_bw(mac_sta);

    if (mac_vht_hdl->vht_capable == OSAL_TRUE) {
        bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(mac_vht_hdl->channel_width,
            mac_sta->channel.chan_number, mac_vht_hdl->channel_center_freq_seg0, mac_vht_hdl->channel_center_freq_seg1);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl = &(hmac_user->he_hdl);
    if ((mac_he_hdl->he_oper_ie.he_oper_param.vht_operation_info_present == OSAL_TRUE) &&
        (mac_he_hdl->he_capable == OSAL_TRUE)) {
        bandwidth_ap =
            mac_get_bandwith_from_center_freq_seg0_seg1(mac_he_hdl->he_oper_ie.vht_operation_info.channel_width,
                mac_sta->channel.chan_number, mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg0,
                mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg1);
    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) && (mac_ht_hdl->ht_capable == OSAL_TRUE) &&
        (mac_mib_get_forty_mhz_operation_implemented(mac_sta) == OSAL_TRUE)) {
        /* 更新带宽模式 */
        bandwidth_ap = mac_get_bandwidth_from_sco(mac_ht_hdl->secondary_chan_offset);
    }

    if ((sta_current_bw != bandwidth_ap) &&
        (hmac_vap_bw_mode_to_bw(bandwidth_ap) <= mac_mib_get_dot11_vap_max_bandwidth(mac_sta))) {
        /* 防止每个beacon都去检查en_bandwidth_ap */
        if (hmac_regdomain_channel_is_support_bw(bandwidth_ap, mac_sta->channel.chan_number) == OSAL_FALSE) {
            bandwidth_ap = WLAN_BAND_WIDTH_20M;
            if (sta_current_bw != bandwidth_ap) {
                /* 防止刷屏打印 */
                oam_warning_log3(0, OAM_SF_ASSOC,
                    "vap_id[%d] {hmac_vap_check_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
                    mac_sta->vap_id, mac_sta->channel.chan_number, bandwidth_ap);
            }
        }

        if (sta_current_bw != bandwidth_ap) {
            change = MAC_BW_DIFF_AP_USER;
            oam_warning_log4(0, OAM_SF_FRAME_FILTER,
                "vap_id[%d] {hmac_vap_check_ap_usr_opern_bandwidth::current_bw = [%d], bw_ap = [%d], change = [%d]}",
                mac_sta->vap_id, sta_current_bw, bandwidth_ap, change);
        }
    }

    return change;
}

static osal_void hmac_vap_init_bw_by_chip_and_protocol(hmac_vap_stru *hmac_vap)
{
    osal_u8 old_bandwidth = hmac_vap->channel.en_bandwidth;
    mac_channel_stru *channel = &hmac_vap->channel;
#ifdef BOARD_ASIC_WIFI
    /* dummy */
#else
    if (channel->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_vap_init_bw_by_chip_and_protocol:: fpga is not support 80M.}\r\n");
        channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
#endif

#ifdef _PRE_WLAN_PLAT_WS83
    /* 规格仅支持11n 40M，不支持11ax 40M，只有最高协议为11n才支持40M配置 */
    if (channel->en_bandwidth >= WLAN_BAND_WIDTH_40PLUS && hmac_vap->protocol != WLAN_HT_MODE &&
        hmac_vap->protocol != WLAN_HT_ONLY_MODE && hmac_vap->protocol != WLAN_HT_11G_MODE) {
        oam_warning_log0(0, OAM_SF_RX,
            "{hmac_vap_init_bw_by_chip_and_protocol:: not support 40M without 11n.}\r\n");
        channel->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
#endif
    if (old_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : mac_vap_get_user_protection_mode_etc
 功能描述  : 获取保护模式类型
*****************************************************************************/
wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode_etc(const hmac_vap_stru *mac_vap_sta,
    const hmac_user_stru *hmac_user)
{
    wlan_prot_mode_enum_uint8 protection_mode = WLAN_PROT_NO;

    if ((mac_vap_sta == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
        return protection_mode;
    }

    /* 在2G频段下，如果AP发送的beacon帧ERP ie中Use Protection bit置为1，则将保护级别设置为ERP保护 */
    if ((mac_vap_sta->channel.band == WLAN_BAND_2G) && (hmac_user->cap_info.erp_use_protect == OSAL_TRUE)) {
        protection_mode = WLAN_PROT_ERP;
    } else if ((hmac_user->ht_hdl.ht_protection == WLAN_MIB_HT_NON_HT_MIXED) ||
        (hmac_user->ht_hdl.ht_protection == WLAN_MIB_HT_NONMEMBER_PROTECTION)) {
        /* 如果AP发送的beacon帧ht operation ie中ht protection字段为mixed或non-member，则将保护级别设置为HT保护 */
        protection_mode = WLAN_PROT_HT;
    } else if (hmac_user->ht_hdl.nongf_sta_present == OSAL_TRUE) {
        /* 如果AP发送的beacon帧ht operation ie中non-gf sta present字段为1，则将保护级别设置为GF保护 */
        protection_mode = WLAN_PROT_GF;
    } else {  /* 剩下的情况不做保护 */
        protection_mode = WLAN_PROT_NO;
    }

    return protection_mode;
}
/*****************************************************************************
 函 数 名  : mac_protection_lsigtxop_check_etc
 功能描述  : 查询是否可以使用lsigtxop保护
*****************************************************************************/
oal_bool_enum mac_protection_lsigtxop_check_etc(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    /* 如果不是11n站点，则不支持lsigtxop保护 */
    if ((hmac_vap->protocol != WLAN_HT_MODE) && (hmac_vap->protocol != WLAN_HT_ONLY_MODE) &&
        (hmac_vap->protocol != WLAN_HT_11G_MODE)) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if ((hmac_user == OSAL_NULL) || (hmac_user->ht_hdl.lsig_txop_protection_full_support == OSAL_FALSE)) {
            return OSAL_FALSE;
        } else {
            return OSAL_TRUE;
        }
    }
    /* BSS 中所有站点都支持Lsig txop protection, 则使用Lsig txop protection机制，开销小, AP和STA采用不同的判断 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (mac_mib_get_lsig_txop_full_protection_activated(hmac_vap) == OSAL_TRUE)) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}
/*****************************************************************************
 函 数 名  : mac_protection_set_lsig_txop_mechanism_etc
 功能描述  : 设置lsig txop保护机制是否开启及保护范围
*****************************************************************************/
osal_void mac_protection_set_lsig_txop_mechanism_etc(hmac_vap_stru *hmac_vap, osal_u8 flag)
{
    /* 数据帧/管理帧发送时候，需要根据bit_lsig_txop_protect_mode值填写发送描述符中的L-SIG TXOP enable位 */
    hmac_vap->protection.lsig_txop_protect_mode = flag;
    oam_warning_log1(0, OAM_SF_CFG, "mac_protection_set_lsig_txop_mechanism_etc:on[%d]?", flag);
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_default_key_id_etc
 功能描述  : 获取mac加密属性default_key_id
*****************************************************************************/
osal_u8 hmac_vap_get_default_key_id_etc(const hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *multi_user;
    osal_u8 default_key_id;

    /* 根据索引，从组播用户密钥信息中查找密钥 */
    multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OSAL_NULL) {
        /* 调用本函数的地方都没有错误返回处理 */
        oam_warning_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_vap_get_default_key_id_etc::multi_user[%d] NULL}",
            hmac_vap->vap_id, hmac_vap->multi_user_idx);
        return 0;
    }

    if ((multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) &&
        (multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104)) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_vap_get_default_key_id_etc::unexpectd cipher_type[%d]}",
            hmac_vap->vap_id, multi_user->key_info.cipher_type);
        return 0;
    }
    default_key_id = multi_user->key_info.default_index;
    if (default_key_id >= WLAN_NUM_TK) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_vap_get_default_key_id_etc::unexpectd keyid[%d]}",
            hmac_vap->vap_id, default_key_id);
        return 0;
    }
    return default_key_id;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_mac_addr_etc
 功能描述  : 获取vap的 mac地址

*****************************************************************************/
osal_u8 *hmac_vap_get_mac_addr_etc(const hmac_vap_stru *hmac_vap)
{
    return mac_mib_get_station_id(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_vap_protection_autoprot_is_enabled_etc
 功能描述  : 查询自动保护机制是否开启
*****************************************************************************/
osal_u8 hmac_vap_protection_autoprot_is_enabled_etc(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->protection.auto_protection;
}

/*****************************************************************************
 函 数 名  : mac_device_find_another_up_vap_etc
 功能描述  : 寻找另一个UP状态的VAP
*****************************************************************************/
hmac_vap_stru *mac_device_find_another_up_vap_etc(const hmac_device_stru *hmac_device, osal_u8 vap_id_self)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }

        if (vap_id_self == hmac_vap->vap_id) {
            continue;
        }

        if (hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
            || (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN  && hmac_vap->user_nums > 0)
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING)
#endif // _PRE_WLAN_FEATURE_ROAM
        ) {
            return hmac_vap;
        }
    }

    return OSAL_NULL;
}

/*****************************************************************************
 函 数 名  : mac_device_find_up_sta_etc
 功能描述  : 寻找处在UP状态的STA
*****************************************************************************/
osal_u32 mac_device_find_up_sta_etc(const hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap)
{
    osal_u8 vap_idx;
    hmac_vap_stru *mac_vap_tmp = OSAL_NULL;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        mac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(mac_vap_tmp == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);

            *hmac_vap = OSAL_NULL;

            return OAL_ERR_CODE_PTR_NULL;
        }

        if ((mac_vap_tmp->vap_state == MAC_VAP_STATE_UP || mac_vap_tmp->vap_state == MAC_VAP_STATE_PAUSE) &&
            (mac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
            *hmac_vap = mac_vap_tmp;

            return OAL_SUCC;
        }
    }

    *hmac_vap = OSAL_NULL;

    return OAL_FAIL;
}

hmac_vap_stru *mac_find_up_legacy_sta_vap(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx = 0;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }
        if (is_legacy_sta(hmac_vap)) {
            return hmac_vap;
        }
    }

    return OSAL_NULL;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
