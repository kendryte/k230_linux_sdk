/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of the corresponding operation function implementation of board, chip, device.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "hmac_feature_main.h"
#include "hmac_feature_interface.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_DEVICE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/
mac_device_voe_custom_stru g_st_mac_voe_custom_param = {OSAL_TRUE, OSAL_TRUE, OSAL_TRUE, OSAL_TRUE};

osal_u32 g_random_scan_oui = 0;
/* 每个chip下的mac device部分能力定制化 */
/* 02和51每个chip下只有1个device，51双芯片时每个chip的能力是相等的 */
/* 03 DBDC开启时mac device个数取2(考虑静态DBDC), 否则取1 */
/* 默认初始化值为动态DBDC，取HAL Device0的能力赋值 */
mac_device_capability_stru g_st_mac_device_capability[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    {
        WLAN_HAL0_NSS_NUM,       /* nss num 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_BW_MAX_WIDTH,  /* mib_bw_supp_width 03 ini FPGA 20_40M,ASIC 160M */
        WLAN_HAL0_NB_IS_EN,      /* NB */
        WLAN_HAL0_1024QAM_IS_EN, /* 1024QAM */
        WLAN_HAL0_11MC_IS_EN,    /* 80211 MC */
        WLAN_HAL0_LDPC_IS_EN,    /* ldpc coding */
        WLAN_HAL0_TX_STBC_IS_EN, /* tx stbc 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_RX_STBC_IS_EN, /* rx stbc 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_SU_BFER_IS_EN, /* su bfer 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_SU_BFEE_IS_EN, /* su bfee 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_MU_BFER_IS_EN, /* mu bfer 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_MU_BFEE_IS_EN, /* mu bfee 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
        WLAN_HAL0_11AX_IS_EN,    /* 11ax */
        0,
        0,
        0,
        0
    }
};

#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_bool_enum_uint8 g_en_wmmac_switch_etc = OAL_TRUE;
#endif

/* 动态/静态DBDC,这里指的是每个chip上mac device的频道能力 */
osal_u8 g_auc_mac_device_radio_cap[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
#ifdef _PRE_WLAN_SUPPORT_5G
    MAC_DEVICE_2G_5G,
#else
    MAC_DEVICE_2G,
#endif
#if (WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP > 1)
    MAC_DEVICE_DISABLE
#endif
};

mac_device_capability_stru *g_pst_mac_device_capability = &g_st_mac_device_capability[0];

osal_u8 g_hmac_user_res_num = WLAN_ASOC_USER_MAX_NUM_LIMIT;

/*****************************************************************************
    函数声明
*****************************************************************************/
static void hmac_device_set_state_etc(hmac_device_stru *hmac_device, osal_u8 device_state);

/*****************************************************************************
    函数实现
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u8 mac_get_user_res_num(osal_void)
{
    return g_hmac_user_res_num;
}

osal_u8 *mac_get_user_res_num_addr(osal_void)
{
    return &g_hmac_user_res_num;
}

mac_device_voe_custom_stru *mac_get_pst_mac_voe_custom_param(osal_void)
{
    return &g_st_mac_voe_custom_param;
}

osal_void mac_set_pst_mac_voe_custom_over_ds_param(osal_u8 over_ds_en)
{
    g_st_mac_voe_custom_param.en_11r_ds = over_ds_en;
}

mac_device_capability_stru *mac_device_get_capability(osal_void)
{
    return g_pst_mac_device_capability;
}

osal_u8 *mac_get_device_radio_cap(osal_void)
{
    return g_auc_mac_device_radio_cap;
}

static osal_u32 mac_device_init_protocol_band_cap(hmac_device_stru *hmac_device, osal_u8 device_id)
{
    hmac_device->band_cap = (hmac_device_check_5g_enable(device_id) == OSAL_TRUE) ?
        WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_device->protocol_cap = WLAN_PROTOCOL_CAP_HE;
#else
#ifdef _PRE_WLAN_FEATURE_11AC2G
    hmac_device->protocol_cap = WLAN_PROTOCOL_CAP_VHT;
#else
    hmac_device->protocol_cap = WLAN_PROTOCOL_CAP_HT;
#endif
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : device添加时，特性数据结构内存分配和初始化
*****************************************************************************/
OAL_STATIC osal_s32 hmac_device_add_feature(hmac_device_stru *hmac_device)
{
    osal_u32 ret = OAL_SUCC;
    ret = hmac_device_feature_init(hmac_device);
    return (osal_s32)ret;
}

osal_void mac_device_set_random_scan_oui(osal_u32 scan_oui)
{
    g_random_scan_oui = scan_oui;
}

static osal_void mac_device_init(hmac_device_stru *hmac_device)
{
    /* 初始化默认管制域 */
    hmac_init_regdomain_etc();

    /* 初始化信道列表 */
    hmac_init_channel_list_etc();

    /* 默认关闭DBAC特性 */
#ifdef _PRE_WLAN_FEATURE_DBAC
    hmac_device->en_dbac_enabled = OAL_TRUE;
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_device->bss_id_list.num_networks = 0;
    mac_set_2040bss_switch(hmac_device, OAL_FALSE);
#endif
    hmac_device->uc_in_suspend = OAL_FALSE;
    hmac_device->wapi = OAL_FALSE;

    /*
     * 初始化随机mac oui为0(3个字节都是0),确保只有Android下发有效mac
     * oui才进行随机mac地址扫描(在随机mac扫描开关打开的情况下)
     */
    hmac_device->is_random_mac_addr_scan = OAL_FALSE;
    hmac_device->mac_oui[0] = (osal_u8)((g_random_scan_oui >> 16) & 0xFF); /* 16 右移位数 */
    hmac_device->mac_oui[1] = (osal_u8)((g_random_scan_oui >> 8) & 0xFF); /* 8 右移位数 */
    hmac_device->mac_oui[2] = (osal_u8)(g_random_scan_oui & 0xFF);   /* OUI第2字节 */

    oal_eth_random_addr(hmac_device->scan_mgmt.random_mac, sizeof(hmac_device->scan_mgmt.random_mac));
    hmac_device->scan_mgmt.random_mac[0] = hmac_device->mac_oui[0] & 0xfe;  /* 保证是单播mac */
    hmac_device->scan_mgmt.random_mac[1] = hmac_device->mac_oui[1];
    hmac_device->scan_mgmt.random_mac[2] = hmac_device->mac_oui[2]; /* mac地址数组第2位mac */
}

/*****************************************************************************
 功能描述 : 初始化device级别参数
*****************************************************************************/
osal_u32 mac_device_init_etc(hmac_device_stru *hmac_device, osal_u8 device_id)
{
    osal_u32 ret;

    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "mac_device_init_etc::hmac_device null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(hmac_device, OAL_SIZEOF(hmac_device_stru), 0, OAL_SIZEOF(hmac_device_stru));

    ret = (osal_u32)hmac_device_add_feature(hmac_device);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "mac_device_init_etc::hmac_device_feature_arr_alloc fail.");
        return ret;
    }

    /* 初始化device的索引 */
    hmac_device->device_id = device_id;

    /* 初始化device级别的一些参数 */
    hmac_device->max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    hmac_device->max_band = WLAN_BAND_BUTT;
    hmac_device->max_channel = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device->beacon_interval = WLAN_BEACON_INTVAL_DEFAULT;
#endif

    /* 将定制化的信息保存到mac device结构体下 */
    /* 初始化mac device的能力 */
    (osal_void)memcpy_s(&hmac_device->device_cap, OAL_SIZEOF(mac_device_capability_stru),
        &mac_device_get_capability()[0], OAL_SIZEOF(mac_device_capability_stru));
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 对于03来说，另一个device只支持SISO,配置为MIMO SAVE状态是否没有影响，再确认; 这些能力在m2s切换中会动态变化 */
    mac_device_get_mode_smps(hmac_device) = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
#endif

    hmac_device->device_state = OAL_TRUE;

    /* 默认关闭wmm,wmm超时计数器设为0 */
    hmac_device->en_wmm = OAL_TRUE;         // merge
    hmac_device->wmm = OSAL_TRUE;

    /* 根据芯片版本初始化device能力信息 */
    ret = mac_device_init_protocol_band_cap(hmac_device, device_id);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 初始化vap num统计信息 */
    hmac_device->vap_num = 0;
    hmac_device->sta_num = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device->st_p2p_info.p2p_mode = 0;
    hmac_device->st_p2p_info.primary_net_device = OAL_PTR_NULL; /* 初始化主net_device 为空指针 */
    hmac_device->set_mode_iftype = NL80211_IFTYPE_UNSPECIFIED;
#endif

    mac_device_init(hmac_device);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 卸载device的操作函数(调用之前，已从chip上删除)
*****************************************************************************/
osal_u32 mac_device_exit_etc(hmac_device_stru *hmac_device)
{
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "mac_device_exit_etc::device null.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device->vap_num = 0;
    hmac_device->sta_num = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device->st_p2p_info.p2p_mode = 0;
#endif

    mac_res_free_dev_etc(hmac_device->device_id);

    hmac_device_set_state_etc(hmac_device, OAL_FALSE);

    return OAL_SUCC;
}

static void hmac_device_set_state_etc(hmac_device_stru *hmac_device, osal_u8 device_state)
{
    hmac_device->device_state = device_state;
}

/*****************************************************************************
 功能描述 : 最大支持带宽换到vht capability info中的带宽
*****************************************************************************/
wlan_mib_vht_supp_width_enum_uint8 hmac_device_trans_bandwith_to_vht_capinfo(wlan_bw_cap_enum_uint8 max_op_bd)
{
    switch (max_op_bd) {
        case WLAN_BW_CAP_20M:
        case WLAN_BW_CAP_40M:
        case WLAN_BW_CAP_80M:
            return WLAN_MIB_VHT_SUPP_WIDTH_80;

        default:
            oam_error_log1(0, OAM_SF_ANY, "hmac_device_trans_bandwith_to_vht_capinfo:bandwith:%d invalid.", max_op_bd);
            return WLAN_MIB_VHT_SUPP_WIDTH_BUTT;
    }
}

/*****************************************************************************
 功能描述 : 检查mac device中是否使能5g
*****************************************************************************/
osal_u32 hmac_device_check_5g_enable(osal_u8 device_id)
{
    osal_u8 device_id_per_chip;
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_device_check_5g_enable:get dev fail device_id[%d]", device_id);
        return OAL_FALSE;
    }

    /* 03两个业务device,00 01,取不同定制化,51双芯片00 11,取同一个定制化 */
    device_id_per_chip = device_id - hmac_device->chip_id;

    return ((g_auc_mac_device_radio_cap[device_id_per_chip] & MAC_DEVICE_5G) != 0);
}

/*****************************************************************************
 功能描述 : 获取所有速率
*****************************************************************************/
osal_void *hmac_device_get_all_rates_etc(const hmac_device_stru *hmac_device)
{
    return (osal_void *)hmac_device->mac_rates_11g;
}

/*****************************************************************************
 函 数 名: hmac_get_device_bw_mode
 功能描述  : 获取device下的最大带宽(业务sta)
*****************************************************************************/
WIFI_TCM_TEXT wlan_bw_cap_enum_uint8 hmac_get_device_bw_mode(const hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    wlan_channel_bandwidth_enum_uint8 max_vap_bw = WLAN_BAND_WIDTH_20M;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    /* 遍历所有vap，并记录最大工作带宽 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "hmac_get_device_bw_mode::hmac_vap[%d] is NULL.", mac_vap_id[vap_index]);
            continue;
        }

        if (hmac_vap->channel.en_bandwidth < WLAN_BAND_WIDTH_BUTT) {
            max_vap_bw = osal_max(max_vap_bw, hmac_vap->channel.en_bandwidth);
        }
    }

    if (max_vap_bw == WLAN_BAND_WIDTH_20M) {
        return WLAN_BW_CAP_20M;
    } else if ((max_vap_bw == WLAN_BAND_WIDTH_40MINUS) || (max_vap_bw == WLAN_BAND_WIDTH_40PLUS)) {
        return WLAN_BW_CAP_40M;
    } else if ((max_vap_bw >= WLAN_BAND_WIDTH_80PLUSPLUS) && (max_vap_bw <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        return WLAN_BW_CAP_80M;
    } else {
        return WLAN_STD_BW_CAP_BUTT;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
