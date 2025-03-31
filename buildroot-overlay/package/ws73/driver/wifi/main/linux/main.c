/*
 * Copyright (c) CompanyNameMagicTag. 2020-2021. All rights reserved.
 * Description: Header file of main.c.
 * Author: Huanghe
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#define SOC_LOG_TAG "[WIFI_MAIN]"
#include "main.h"
#include "wal_linux_ioctl.h"
#include "hmac_config.h"
#ifdef _PRE_FEATURE_USB_ID_TABLE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/usb.h>
#endif
#endif

#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#include "hmac_scan.h"
#endif
#include "plat_pm_wlan.h"
#include "wlan_thruput_debug.h"
#include "hmac_vap.h"
#include "hcc_host_if.h"
#include "mac_device_ext.h"
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#endif
#include "wal_linux_netdev.h"
#include "soc_ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_C

#ifdef _PRE_FEATURE_USB_ID_TABLE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* wifi usb id 声明 */
static const struct usb_device_id g_wifi_usb_table[] = {
    { USB_DEVICE(0x12D1, 0x897d) },
    { USB_DEVICE(0xFFFF, 0x3733) },
    { }
};

MODULE_DEVICE_TABLE(usb, g_wifi_usb_table);
#endif
#endif

/*****************************************************************************
    函数声明
*****************************************************************************/
void platform_module_exit(osal_u16 bitmap);
static void builder_module_exit(osal_u16 bitmap);

/*****************************************************************************
    函数实现
*****************************************************************************/

/*****************************************************************************
 功能描述 : 选择性卸载不同模块 bitmap: BIT0:oal BIT1:oam BIT2:sdt BIT3:frw
            BIT4:hal BIT5:dmac BIT6:alg BIT7:hmac BIT8:wal
*****************************************************************************/
static void builder_module_exit(osal_u16 bitmap)
{
    if (BIT8 & bitmap) {
        wal_main_exit_etc();
    }
    if (BIT7 & bitmap) {
        hmac_main_exit_etc();
    }
    wlan_close_ex();
    if (BIT6 & bitmap) {
        frw_main_exit_etc();
    }

    return;
}

#ifdef _PRE_WLAN_DFR_STAT
static void  hwifi_cfg_self_healing_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SELF_HEALING_ENABLE, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_self_healing_enable_etc((osal_u32)priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SELF_HEALING_PERIOD, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_self_healing_period_etc((osal_u32)priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SELF_HEALING_CNT, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_self_healing_cnt_etc((osal_u32)priv_value);
    }
    return;
}
#endif

static void hwifi_cfg_mac_device_capability_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
    mac_device_capability_stru *mac_device_capability = OSAL_NULL;

    mac_device_capability = mac_device_get_capability();
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BW_MAX_WITH, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].channel_width = (wlan_bw_cap_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFER, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].su_bfmer_is_supp = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFEE, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].su_bfmee_is_supp = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFER, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].mu_bfmer_is_supp = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFEE, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].mu_bfmee_is_supp = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LDPC, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].ldpc_is_supp = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BA_32BIT_BITMAP, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].ba_bitmap_support_32bit = (oal_bool_enum_uint8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MTID_AGGR_RX, &priv_value);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].mtid_aggregation_rx_support = (osal_u8)priv_value;
    }

    return;
}

static void hwifi_cfg_rx_stbc_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
    mac_device_capability_stru *mac_device_capability = OSAL_NULL;

    mac_device_capability = mac_device_get_capability();
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RX_STBC, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: rx_stbc[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        mac_device_capability[0].rx_stbc_is_supp = (osal_u8)priv_value;
    }
    return;
}

#ifdef _PRE_WLAN_TCP_OPT
static void hwifi_cfg_tcp_ack_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TCP_ACK_FILTER_EN, &priv_value);
    wifi_printf("hwifi_cfg_tcp_ack_param_init: tcp_ack_filter_enable[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        g_st_tcp_ack_filter.tcp_ack_filter_en = priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TCP_ACK_MAX_NUM_START_PROCESS, &priv_value);
    wifi_printf("hwifi_cfg_tcp_ack_param_init: tcp_ack_max_num_start_process[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        g_st_tcp_ack_filter.tcp_ack_max_num_start_process = priv_value;
    }
    return;
}
#endif

static void hwifi_cfg_frw_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TXDATA_QUE_LIMIT, &priv_value);
    if (ret == OAL_SUCC) {
        frw_set_data_queue_limit((osal_u32)priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_UDP_HCC_FC_DROP, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_udp_hcc_fc_drop((osal_u8)priv_value);
    }
    return;
}

static void hwifi_cfg_ampdu_amsdu_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_AMSDU_NUM, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_amsdu_num(priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_AMSDU_TX_ON, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_amsdu_tx_active(priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_AMPDU_AMSDU_TX_ON, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_ampdu_amsdu_tx_active(priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_INIT_AMPDU_TX_MAX_NUM, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_ampdu_tx_max_num(priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_INIT_AMPDU_RX_MAX_NUM, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_ampdu_rx_max_num(priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_INIT_AMPDU_TX_BAW_SIZE, &priv_value);
    if (ret == OAL_SUCC) {
        hwifi_set_ampdu_tx_baw_size(priv_value);
    }
    return;
}

static void hwifi_cfg_user_ap_num_param_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
    osal_u8 *user_num_addr = mac_get_user_res_num_addr();

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_USER_NUM, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: user num[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        *user_num_addr = (priv_value < 1) ? 1 :
            ((priv_value > WLAN_ASSOC_USER_MAX_NUM) ? WLAN_ASSOC_USER_MAX_NUM : (oal_bool_enum_uint8)priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HMAC_MAX_AP_NUM, &priv_value);
    if (ret != OAL_SUCC || priv_value <= 0) {
        // 如果读取失败,使用默认值200进行初始化
        priv_value = 200;
    }
    hwifi_set_hmac_max_ap_num_etc(priv_value);
}

#ifdef _PRE_WLAN_FEATURE_DAQ
OAL_STATIC osal_void hwifi_cfg_data_sample_param_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DATA_SAMPLE, &priv_value);
    wifi_printf("hwifi_cfg_data_sample_param_init: data_sample[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_data_sample((priv_value <= 0) ? OSAL_FALSE : OSAL_TRUE);
    }
    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_APF
OAL_STATIC osal_void hwifi_cfg_apf_enable_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_APF_ENABLE, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: apf enable[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_apf_enable((priv_value <= 0) ? OSAL_FALSE : OSAL_TRUE);
    }
    return;
}
#endif

OAL_STATIC osal_void hwifi_cfg_protocol_enable_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_PROTOCOL_ENABLE, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: 11ax protocol enable[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_protocol_enable((priv_value <= 0) ? OSAL_FALSE : OSAL_TRUE);
    }
    return;
}

#ifdef _PRE_WLAN_SMOOTH_PHASE
OAL_STATIC osal_void hwifi_cfg_smooth_phase_en_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SMOOTH_PHASE_EN, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: smooth_phase_en[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_smooth_phase_en((priv_value == 0) ? OSAL_FALSE : OSAL_TRUE);
    }
    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
OAL_STATIC osal_void hwifi_cfg_wow_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_WOW_EVENT, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: wow_event[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_wow_event((osal_u32)priv_value);
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_WOW_ENABLE, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: wow_enable[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        hwifi_set_wow_enable((osal_u8)priv_value);
    }

    return;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
OAL_STATIC osal_void hwifi_cfg_roam_init(osal_void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_ROAM_TRIGGER_RSSI_2G, &priv_value);
    if (ret == OAL_SUCC) {
        wlan_customize_etc->c_roam_trigger_b = (osal_s8)priv_value;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_ROAM_DELTA_RSSI_2G, &priv_value);
    if (ret == OAL_SUCC) {
        wlan_customize_etc->c_roam_delta_b = (osal_s8)priv_value;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_11R
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_OVER_DS_EN, &priv_value);
    wifi_printf("hwifi_cfg_host_global_init_param: over_ds_en[%d] ret[%d]\r\n", priv_value, ret);
    if (ret == OAL_SUCC) {
        mac_set_pst_mac_voe_custom_over_ds_param((priv_value == 0) ? OSAL_FALSE : OSAL_TRUE);
    }
#endif
    return;
}
#endif

#ifdef _PRE_PRODUCT_ID_HOST
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

/*****************************************************************************
 功能描述      : 扫描参数配置
*****************************************************************************/
static void hwifi_cfg_scan_para_cfg_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
    hmac_scan_config_info config_info = {0};

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SCAN_PROBE_SEND_TIMES, &priv_value);
    wifi_printf("hwifi_cfg_scan_para_cfg_init probe send times, ret %d priv_value %d\r\n", ret, priv_value);
    /* 如果读取失败, 或者在范围[1-7]之外 */
    if (ret != OAL_SUCC || ((priv_value < 1) || (priv_value > 7))) {
        /* 使用默认值1进行初始化 */
        priv_value = 1;
    }
    config_info.probe_req_send_times = priv_value;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SCAN_DEFAULT_TIME, &priv_value);
    wifi_printf("get scan time, ret %d priv_value %d\r\n", ret, priv_value);
    /* 如果读取失败, 或者在范围[20-50]之外 */
    if (ret != OAL_SUCC || ((priv_value < 20) || (priv_value > 50))) {
        /* 使用默认值20进行初始化 */
        priv_value = 20;
    }
    config_info.single_scan_time = priv_value;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SCAN_DEFAULT_COUNT, &priv_value);
    wifi_printf("get scan count, ret %d priv_value %d\r\n", ret, priv_value);
    /* 如果读取失败, 或者在范围[1-3]之外 */
    if (ret != OAL_SUCC || ((priv_value < 1) || (priv_value > 3))) {
        /* 使用默认值2进行初始化 */
        priv_value = 2;
    }
    config_info.scan_count = priv_value;

    hmac_scan_set_config_info(&config_info);

    return;
}

/*****************************************************************************
 功能描述      : 扫描probe req报文是否发送所有 ie字段
*****************************************************************************/
static void  hwifi_cfg_scan_probe_all_ie_cfg_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SCAN_PROBE_REQ_ALL_IE, &priv_value);
    wifi_printf("hwifi_cfg_scan_probe_ie_cfg_init ret %d priv_value %d\r\n", ret, priv_value);

    if (ret != OAL_SUCC || priv_value < 0) {
        /* 如果读取失败,使用默认值1进行初始化, 默认携带所有IE */
        priv_value = 1;
    }

    hmac_scan_set_probe_req_all_ie_cfg(priv_value);
    return;
}

/*****************************************************************************
 功能描述      : STA扫描probe req报文是否删除 WPS ie字段
*****************************************************************************/
static void  hwifi_cfg_scan_probe_del_wps_ie_cfg_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SCAN_PROBE_REQ_DEL_WPS_IE, &priv_value);
    wifi_printf("hwifi_cfg_scan_probe_del_wps_ie_cfg_init ret %d priv_value %d\r\n", ret, priv_value);

    if (ret != OAL_SUCC || priv_value < 0) {
        /* 如果读取失败,使用默认值1进行初始化, 默认删除 WPS IE */
        priv_value = 1;
    }

    hmac_scan_set_probe_req_del_wps_ie_cfg((osal_u8)priv_value);

    return;
}

/*****************************************************************************
 功能描述      : 是否开启随机mac扫描
*****************************************************************************/
static void  hwifi_cfg_random_mac_addr_scan_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RANDOM_MAC_ADDR_SCAN, &priv_value);
    wifi_printf("hwifi_cfg_random_mac_addr_scan_init ret %d priv_value %d\r\n", ret, priv_value);

    if (ret != OAL_SUCC || priv_value < 0) {
        /* 如果读取失败,使用默认值1进行初始化, 默认开启随机mac扫描 */
        priv_value = 1;
    }

    hwifi_scan_set_random_mac_addr_scan(priv_value);
    return;
}

/*****************************************************************************
 功能描述      : 设置随机mac oui
*****************************************************************************/
static void  hwifi_cfg_random_mac_addr_scan_oui_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RANDOM_SCAN_OUI, &priv_value);
    wifi_printf("hwifi_cfg_random_mac_addr_scan_oui_init ret %d priv_value %d\r\n", ret, priv_value);

    if (ret != OAL_SUCC) {
        /* 如果读取失败,使用默认值0进行初始化, OUI的值为0 */
        priv_value = 0;
    }

    mac_device_set_random_scan_oui(priv_value);
    return;
}

/*****************************************************************************
 功能描述      : sniffer抓包申请文件最大值配置
*****************************************************************************/
static void  hwifi_cfg_pcap_file_len_max_cfg_init(void)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_PCAP_FILE_LEN_MAX, &priv_value);
    wifi_printf("hwifi_cfg_pcap_file_len_max_cfg_init ret %d priv_value %d\r\n", ret, priv_value);

    if (ret != OAL_SUCC || priv_value < 0) {
        /* 如果读取失败,使用默认值12进行初始化 */
        priv_value = 12;
    }

    hwifi_set_sniffer_file_len_max(priv_value);
    return;
}

/*****************************************************************************
 功能描述      : 从ini文件读取配置的net dev name
****************************************************************************/
osal_void hwifi_get_configed_ifname(osal_void)
{
    osal_s32 ret;
    osal_s8 ifname[OAL_IF_NAME_SIZE] = {0};

    memset_s(ifname, sizeof(ifname), 0, sizeof(ifname));
#ifdef _PRE_WLAN_NO_SUPPORT_INI
    ret = strcpy_s(ifname, OAL_IF_NAME_SIZE, "wlan0");
#else
    ret = get_cust_conf_string_etc(INI_MODU_WIFI, "wlan_ifname0", ifname, sizeof(ifname));
#endif
    if ((ret != INI_SUCC) || (osal_strlen(ifname) == 0)) {
        oam_warning_log1(0, 0, "{hwifi_get_configed_ifname:find ifname0 fail ret %d.}", ret);
        wal_set_wlan_name_default(NETDEV_STA_MAC_INDEX);
    } else {
        wal_set_wlan_name_config(NETDEV_STA_MAC_INDEX, ifname, osal_strlen(ifname));
    }

    memset_s(ifname, sizeof(ifname), 0, sizeof(ifname));
#ifdef _PRE_WLAN_NO_SUPPORT_INI
    ret = strcpy_s(ifname, OAL_IF_NAME_SIZE, "wlan1");
#else
    ret = get_cust_conf_string_etc(INI_MODU_WIFI, "wlan_ifname1", ifname, sizeof(ifname));
#endif
    if ((ret != INI_SUCC) || (osal_strlen(ifname) == 0)) {
        oam_warning_log1(0, 0, "{hwifi_get_configed_ifname:find ifname1 fail ret %d.}", ret);
        wal_set_wlan_name_default(NETDEV_AP_MAC_INDEX);
    } else {
        wal_set_wlan_name_config(NETDEV_AP_MAC_INDEX, ifname, osal_strlen(ifname));
    }

    memset_s(ifname, sizeof(ifname), 0, sizeof(ifname));
#ifdef _PRE_WLAN_NO_SUPPORT_INI
    ret = strcpy_s(ifname, OAL_IF_NAME_SIZE, "p2p0");
#else
    ret = get_cust_conf_string_etc(INI_MODU_WIFI, "wlan_ifname2", ifname, sizeof(ifname));
#endif
    if ((ret != INI_SUCC) || (osal_strlen(ifname) == 0)) {
        oam_warning_log1(0, 0, "{hwifi_get_configed_ifname:find ifname2 fail ret %d.}", ret);
        wal_set_wlan_name_default(NETDEV_P2P_MAC_INDEX);
    } else {
        wal_set_wlan_name_config(NETDEV_P2P_MAC_INDEX, ifname, osal_strlen(ifname));
    }
}

/*****************************************************************************
 功能描述 : 初始化定制化ini文件host侧全局变量
*****************************************************************************/
osal_s32 hwifi_cfg_host_global_init_param(void)
{
    hwifi_cfg_mac_device_capability_param_init();
    hwifi_cfg_rx_stbc_param_init();

#ifdef _PRE_WLAN_TCP_OPT
    hwifi_cfg_tcp_ack_param_init();
#endif

    hwifi_cfg_frw_param_init();

    /* ampdu/amsdu聚合相关定制化参数配置 */
    hwifi_cfg_ampdu_amsdu_param_init();

    hwifi_cfg_user_ap_num_param_init();

    hwifi_cfg_scan_para_cfg_init();

    hwifi_cfg_scan_probe_all_ie_cfg_init();

    hwifi_cfg_scan_probe_del_wps_ie_cfg_init();

    hwifi_cfg_random_mac_addr_scan_init();

    hwifi_cfg_random_mac_addr_scan_oui_init();

    hwifi_cfg_pcap_file_len_max_cfg_init();

#ifdef _PRE_WLAN_DFR_STAT
    hwifi_cfg_self_healing_param_init();
#endif

#ifdef _PRE_WLAN_FEATURE_DAQ
    hwifi_cfg_data_sample_param_init();
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    hwifi_cfg_apf_enable_init();
#endif

    hwifi_cfg_protocol_enable_init();

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
    hwifi_cfg_wow_init();
#endif

#ifdef _PRE_WLAN_SMOOTH_PHASE
    hwifi_cfg_smooth_phase_en_init();
#endif

#if defined(_PRE_WLAN_DFT_STAT) && !defined(_PRE_WLAN_NO_SUPPORT_INI)
    hwifi_get_external_record_file_name();
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    hwifi_cfg_roam_init();
#endif

    hwifi_get_configed_ifname();

    return OAL_SUCC;
}
#endif /* _PRE_PLAT_FEATURE_CUSTOMIZE */

osal_s32 host_module_init_etc(void)
{
    osal_s32 ret;
    osal_u16 bitmap;

    stat_timer_enable();
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#ifndef _PRE_WLAN_NO_SUPPORT_INI
    /* 读定制化配置文件&NVRAM */
    hwifi_custom_host_read_cfg_init();
#endif
    /* 配置host全局变量值 */
    ret = hwifi_cfg_host_global_init_param();
    if (ret != OAL_SUCC) {
        wifi_printf("host_module_init_etc: hwifi_cfg_host_global_init_param return error code:%d\r\n", ret);
    }
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

    ret = hmac_main_init_etc();
    if (ret != OAL_SUCC) {
        wifi_printf("host_module_init_etc: hmac_main_init_etc return error code:%d\r\n", ret);
        return ret;
    }

    ret = wal_main_init_etc();
    if (ret != OAL_SUCC) {
        wifi_printf("host_module_init_etc:wal_main_init_etc return error code:%d\r\n", ret);
        bitmap = BIT7;
        builder_module_exit(bitmap);
        return ret;
    }

    g_uc_custom_cali_done_etc = OAL_FALSE;
    /* 开机校准和定制化参数下发 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_cfg_init_cb_reg(hwifi_hcc_customize_h2d_data_cfg);
#endif
    wlan_open_ex();

    wlan_get_addr_from_plat();
    /* 启动完成后，输出打印 */
    wifi_printf("host_module_init_etc:: host_main_init finish!\r\n");

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : host侧初始化总入口
*****************************************************************************/
osal_s32 mpxx_host_main_init(void)
{
    osal_s32 ret;
    osal_u32 start_timestamp;
    osal_u32 current_timestamp;
    osal_u32 run_time;

    wifi_set_init_start_time();
    wal_insmod_ko_begin();
    ret = frw_main_init_etc();
    if (ret != OAL_SUCC) {
        wifi_printf("host_main_init: frw_main_init_etc return error code: %d", ret);
        return ret;
    }
    ret = host_module_init_etc();
    if (ret != OAL_SUCC) {
        wifi_printf("host_main_init: host_module_init_etc return error code: %d", ret);
        return ret;
    }

    wal_ccpriv_register_inetaddr_notifier_etc();
    wal_ccpriv_register_inet6addr_notifier_etc();

    /* 此打印输出用于产测自动化启动检测 */
    wifi_printf("WiFi Init OK.\n");

    /* 启动完成后，输出打印 */
    start_timestamp = wifi_get_init_start_time();
    current_timestamp = osal_get_time_stamp_ms();
    run_time = osal_get_runtime(start_timestamp, current_timestamp);
    wifi_printf("wifi_host_init_finish![wifi_init cost %d ms].\n", run_time);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : wsxx项目host侧卸载总入口
*****************************************************************************/
void mpxx_host_main_exit(void)
{
    osal_u16 bitmap;

    wal_ccpriv_unregister_inetaddr_notifier_etc();
    wal_ccpriv_unregister_inet6addr_notifier_etc();

    bitmap = BIT6 | BIT7 | BIT8;
    builder_module_exit(bitmap);

    return;
}

#ifndef CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT

osal_s32 g_wifi_init_flag_etc = 0;
osal_s32 g_wifi_init_ret_etc;

/* built-in */
static ssize_t wifi_sysfs_set_init(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    char mode[128] = {0};

    if (buf == NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%20s", mode, sizeof(mode)) != 1)) {
        wifi_printf("set value one param!\n");
        return -OAL_EINVAL;
    }

    if (sysfs_streq("init", mode)) {
        /* init */
        if (g_wifi_init_flag_etc == 0) {
            g_wifi_init_ret_etc = mpxx_host_main_init();
            g_wifi_init_flag_etc = 1;
        } else {
            wifi_printf("double init!\n");
        }
    } else {
        wifi_printf("invalid input:%s\n", mode);
    }

    return count;
}

static ssize_t wifi_sysfs_get_init(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (g_wifi_init_flag_etc == 1) {
        if (g_wifi_init_ret_etc == OAL_SUCC) {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret, "running\n");
        } else {
            ret += snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret, "boot failed ret=%d\n", g_wifi_init_ret_etc);
        }
    } else {
        ret += snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret, "uninit\n");
    }

    return ret;
}

static DEVICE_ATTR(wifi, S_IRUGO | S_IWUSR, wifi_sysfs_get_init, wifi_sysfs_set_init);

static struct attribute *wifi_init_sysfs_entries[] = {
    &dev_attr_wifi.attr,
    NULL
};

static struct attribute_group wifi_init_attribute_group = {
    .attrs = wifi_init_sysfs_entries,
};

osal_s32 wifi_sysfs_init_etc(void)
{
    osal_s32 ret;
    oal_kobject *root_boot_object = NULL;

    root_boot_object = oal_get_sysfs_root_boot_object_etc();
    if (root_boot_object == NULL) {
        wifi_printf("[E]get root boot sysfs object failed!\n");
        return -OAL_EBUSY;
    }

    ret = sysfs_create_group(root_boot_object, &wifi_init_attribute_group);
    if (ret) {
        wifi_printf("sysfs create plat boot group fail.ret=%d\n", ret);
        ret = -OAL_ENOMEM;
        return ret;
    }

    return ret;
}

void wifi_sysfs_exit_etc(void)
{
    /* need't exit,built-in */
    return;
}

oal_module_init(wifi_sysfs_init_etc);
oal_module_exit(wifi_sysfs_exit_etc);
#endif
#else
oal_module_init(mpxx_host_main_init);
oal_module_exit(mpxx_host_main_exit);
#endif

oal_module_license("GPL");

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
