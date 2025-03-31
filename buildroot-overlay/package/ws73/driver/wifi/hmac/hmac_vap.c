/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_vap.c
 * 生成日期   : 2012年10月19日
 * 功能描述   : vap模块主文件
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "wal_common.h"
#include "frw_util_notifier.h"
#include "oal_netbuf_ext.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oam_struct.h"
#include "wlan_spec.h"
#include "mac_ie.h"
#include "hmac_resource.h"
#include "hmac_tx_amsdu.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_sr_sta.h"
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#include "hmac_psm_sta.h"
#include "hmac_psm_ap.h"
#include "hmac_ampdu_config.h"
#include "hmac_wmm.h"
#include "hmac_m2u.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif
#include "hmac_blockack.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#include "hmac_roam_if.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#ifdef _PRE_WLAN_FEATURE_ISOLATION
#include "hmac_isolation.h"
#endif
#include "hmac_blacklist.h"

#include "hmac_btcoex.h"
#include "hmac_feature_interface.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif

#include "hmac_dnb_sta.h"
#include "hmac_feature_main.h"
#include "wlan_msg.h"

#include "hal_chip.h"
#include "hal_device_fsm.h"

#include "hmac_scan.h"
#include "hmac_fcs.h"
#include "hmac_chan_mgmt.h"
#include "hmac_sta_pm.h"
#include "hmac_power.h"
#include "hmac_beacon.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_user.h"
#include "hmac_tx_mpdu_queue.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_alg_notify.h"
#include "oal_netbuf_data.h"
#include "oal_list.h"
#include "plat_pm_wlan.h"
#include "hmac_feature_interface.h"
#ifdef _PRE_WLAN_FEATURE_DBAC
#include "alg_dbac_hmac.h"
#endif
#include "hmac_csi.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hmac_data_acq.h"
#endif
#include "hmac_uapsd_sta.h"
#include "hmac_11r.h"
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "hmac_obss_ap.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
#include "hmac_sta_channel_scoring.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_VAP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HOST)
hmac_vap_stru g_ast_hmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif

osal_u8  g_uc_host_rx_ampdu_amsdu = OSAL_FALSE;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) \
    && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC osal_s32  hmac_cfg_vap_if_open(oal_net_device_stru *dev);
OAL_STATIC osal_s32  hmac_cfg_vap_if_close(oal_net_device_stru *dev);
OAL_STATIC osal_s32  hmac_cfg_vap_if_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev);
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_s32  hmac_cfg_vap_if_open(oal_net_device_stru *dev);
OAL_STATIC osal_s32  hmac_cfg_vap_if_close(oal_net_device_stru *dev);
OAL_STATIC oal_net_dev_tx_enum  hmac_cfg_vap_if_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev);
#endif

OAL_STATIC oal_net_device_ops_stru gst_vap_net_dev_cfg_vap_ops = {
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) \
    && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    .ndo_open               = hmac_cfg_vap_if_open,
    .ndo_stop               = hmac_cfg_vap_if_close,
    .ndo_start_xmit         = hmac_cfg_vap_if_xmit,
#else
    .ndo_get_stats          = oal_net_device_get_stats,
    .ndo_open               = oal_net_device_open,
    .ndo_stop               = oal_net_device_close,
    .ndo_start_xmit         = OAL_PTR_NULL,
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
    .ndo_set_multicast_list = OAL_PTR_NULL,
#endif
    .ndo_do_ioctl           = oal_net_device_ioctl,
    .ndo_change_mtu         = oal_net_device_change_mtu,
    .ndo_init               = oal_net_device_init,
    .ndo_set_mac_address    = oal_net_device_set_macaddr
#endif
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    .ndo_get_stats          = oal_net_device_get_stats,
    .ndo_open               = hmac_cfg_vap_if_open,
    .ndo_stop               = hmac_cfg_vap_if_close,
    .ndo_start_xmit         = hmac_cfg_vap_if_xmit,
    .ndo_set_multicast_list = OAL_PTR_NULL,
    .ndo_do_ioctl           = oal_net_device_ioctl,
    .ndo_change_mtu         = oal_net_device_change_mtu,
    .ndo_init               = oal_net_device_init,
#endif
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/
osal_void hmac_set_host_rx_ampdu_amsdu(osal_u8 val)
{
    g_uc_host_rx_ampdu_amsdu = val;
}

osal_u8 hmac_get_host_rx_ampdu_amsdu(osal_void)
{
    return g_uc_host_rx_ampdu_amsdu;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s8 *hmac_config_index2string_etc(osal_u32 index, osal_s8 *string[], osal_u32 max_str_nums)
{
    if (index >= max_str_nums) {
        return (osal_s8 *)"unkown";
    }
    return string[index];
}

OSAL_STATIC osal_s8 *hmac_config_protocol2string_etc(osal_u32 protocol)
{
    osal_char *pac_protocol2string[] = {
        "11a", "11b", "error", "11bg", "11g", "11n", "11ac", "11nonly", "11aconly", "11ng", "11ax", "error"};
    return hmac_config_index2string_etc(protocol, (osal_s8 **)pac_protocol2string,
                                            OAL_SIZEOF(pac_protocol2string) / OAL_SIZEOF(osal_char *));
}

OSAL_STATIC osal_s8 *hmac_config_band2string_etc(osal_u32 band)
{
    osal_char        *pac_band2string[]     = {"2.4G", "5G", "error"};
    return hmac_config_index2string_etc(band, (osal_s8 **)pac_band2string,
                                            OAL_SIZEOF(pac_band2string) / OAL_SIZEOF(osal_char *));
}

OSAL_STATIC osal_s8 *hmac_config_bw2string_etc(osal_u32 bw)
{
    osal_char        *pac_bw2string[]       = {"20M", "40+", "40-", "80++", "80+-", "80-+", "80--", "error"};
    return hmac_config_index2string_etc(bw, (osal_s8 **)pac_bw2string,
                                            OAL_SIZEOF(pac_bw2string) / OAL_SIZEOF(osal_char *));
}

OSAL_STATIC osal_s8 *hmac_config_ciper2string_etc(osal_u32 ciper2)
{
    osal_char        *pac_ciper2string[]    = {"GROUP", "WEP40", "TKIP", "RSV", "CCMP", "WEP104", "BIP", "NONE"};
    return hmac_config_index2string_etc(ciper2, (osal_s8 **)pac_ciper2string,
                                            OAL_SIZEOF(pac_ciper2string) / OAL_SIZEOF(osal_char *));
}

OSAL_STATIC osal_s8 *hmac_config_akm2string_etc(osal_u32 akm2)
{
    osal_char        *pac_akm2string[]     = {"RSV", "1X", "PSK", "FT_1X", "FT_PSK", "1X_SHA256", "PSK_SHA256", "NONE"};
    return hmac_config_index2string_etc(akm2, (osal_s8 **)pac_akm2string,
                                            OAL_SIZEOF(pac_akm2string) / OAL_SIZEOF(osal_char *));
}
#endif

OAL_STATIC osal_void hmac_vap_init_ap_mode(hmac_vap_stru *hmac_vap)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_ATTACH);
    /* 组播转单播初始化函数 */
    if (fhook != OSAL_NULL) {
        ((hmac_m2u_attach_cb)fhook)(hmac_vap);
    }

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    /* AP模式默认业务识别功能开启 */
    mac_mib_set_TxTrafficClassifyFlag(hmac_vap, OAL_SWITCH_ON);
#endif
}

OAL_STATIC osal_void hmac_vap_init_sta_mode(hmac_vap_stru *hmac_vap)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_ATTACH);
    /* 组播转单播初始化函数 */
    if (fhook != OSAL_NULL) {
        ((hmac_m2u_attach_cb)fhook)(hmac_vap);
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_mib_set_spectrum_management_implemented(hmac_vap, OAL_TRUE);
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
    hmac_vap->cfg_sta_pm_manual = 0xFF;
    hmac_vap->ps_mode           = 1;
#endif

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    /* STA模式默认业务识别功能开启 */
    mac_mib_set_TxTrafficClassifyFlag(hmac_vap, OAL_SWITCH_ON);
#endif
}

OAL_STATIC osal_void hmac_vap_init_feature(hmac_vap_stru *hmac_vap)
{
    OSAL_INIT_LIST_HEAD(&(hmac_vap->pmksa_list_head));

    /* 初始化重排序超时时间 */
    hmac_vap->rx_timeout[WLAN_WME_AC_BK] = HMAC_BA_RX_BK_TIMEOUT;
    hmac_vap->rx_timeout[WLAN_WME_AC_BE] = HMAC_BA_RX_BE_TIMEOUT;
    hmac_vap->rx_timeout[WLAN_WME_AC_VI] = HMAC_BA_RX_VI_TIMEOUT;
    hmac_vap->rx_timeout[WLAN_WME_AC_VO] = HMAC_BA_RX_VO_TIMEOUT;

#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap->del_net_device    = OAL_PTR_NULL;
#endif
#ifdef _PRE_WLAN_TCP_OPT
    hmac_vap->hmac_tcp_ack[HCC_TX].filter_info.ack_limit = DEFAULT_TX_TCP_ACK_THRESHOLD;
    hmac_vap->hmac_tcp_ack[HCC_RX].filter_info.ack_limit = DEFAULT_RX_TCP_ACK_THRESHOLD;
    hmac_tcp_opt_init_filter_tcp_ack_pool_etc(hmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_vap->pmksa_mgmt.pmksa_num = 0;
    OSAL_INIT_LIST_HEAD(&(hmac_vap->pmksa_mgmt.pmksa_list_head));
    osal_spin_lock_init(&(hmac_vap->pmksa_mgmt.lock));
#endif
}

OAL_STATIC osal_void hmac_vap_init_params(hmac_vap_stru *hmac_vap)
{
   /* 初始化预设参数 */
    hmac_vap->preset_para.protocol        = hmac_vap->protocol;
    hmac_vap->preset_para.en_bandwidth       = hmac_vap->channel.en_bandwidth;
    hmac_vap->preset_para.band            = hmac_vap->channel.band;
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    hmac_vap->max_scan_count_per_channel    = 0; /* 扫描次数 */
    hmac_vap->scan_time                     = 0; /* 扫描驻留时间 */
    hmac_vap->scan_channel_interval         = 0; /* 间隔n个信道，切回工作信道工作一段时间 */
    hmac_vap->work_time_on_home_channel     = 0; /* 背景扫描时，返回工作信道工作的时间 */
    hmac_vap->scan_type = 0; /* 扫描模式, 0-主动模式, 1-被动模式, 供ccpriv命令使用, 跟正常宏定义的值刚好相反 */
#endif

    /* 初始化配置私有结构体 */
    osal_wait_init(&hmac_vap->mgmt_tx.wait_queue);

#ifdef _PRE_WLAN_FEATURE_11D
    hmac_vap->updata_rd_by_ie_switch = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    hmac_wds_init_table(hmac_vap);
#endif

    // 默认支持接收方向的AMPDU+AMSDU联合聚合,通过定制化决定
    hmac_vap->rx_ampduplusamsdu_active = g_uc_host_rx_ampdu_amsdu;
    osal_wait_init(&hmac_vap->query_wait_q);
}

/*****************************************************************************
 函 数 名  : hmac_vap_init_etc
 功能描述  : 初始化要添加的hmac vap的一些特性信息
 输入参数  : 指向要添加的vap的指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/

osal_u32  hmac_vap_init_etc(hmac_vap_stru              *hmac_vap,
    osal_u8                   chip_id,
    osal_u8                   device_id,
    osal_u8                   vap_id,
    hmac_cfg_add_vap_param_stru *param)
{
    osal_u32 ret;

    ret = mac_vap_init_etc(hmac_vap, chip_id, device_id, vap_id, param);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY,  "vap_id[%d] {hmac_vap_init_etc:hmac_vap_init_etc failed[%d]}", vap_id, ret);
        return ret;
    }

    ret = hmac_config_add_vap_feature(hmac_vap);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,  "vap_id[%d]{hmac_vap_init_etc:hmac_vap_feature_arr_alloc failed.}", vap_id);
        return ret;
    }

    hmac_vap_init_params(hmac_vap);

    /* 根据配置VAP，将对应函数挂接在业务VAP，区分AP、STA和WDS模式 */
    switch (param->add_vap.vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            hmac_vap_init_ap_mode(hmac_vap);
            break;
        case WLAN_VAP_MODE_BSS_STA:
            hmac_vap_init_sta_mode(hmac_vap);
            break;
        case WLAN_VAP_MODE_CONFIG:
            /* 配置VAP直接返回 */
            return OAL_SUCC;
        default:
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_init_etc::unsupported mod=%d.}",
                             hmac_vap->vap_id, param->add_vap.vap_mode);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_vap_init_feature(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_desired_country_etc
 功能描述  : 读取期望的国家
*****************************************************************************/
osal_s8  *hmac_vap_get_desired_country_etc(osal_u8 vap_id)
{
    hmac_vap_stru   *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_get_desired_country_etc::hmac_vap null.}", vap_id);
        return OAL_PTR_NULL;
    }

    return hmac_vap->ac_desired_country;
}
#ifdef _PRE_WLAN_FEATURE_11D
/*****************************************************************************
 函 数 名  : hmac_vap_get_updata_rd_by_ie_switch_etc
 功能描述  : 读取是否根据关联ap更新国家码信息
*****************************************************************************/
osal_u32  hmac_vap_get_updata_rd_by_ie_switch_etc(osal_u8 vap_id, oal_bool_enum_uint8 *update_rd_by_ie_sw)
{
    hmac_vap_stru   *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_get_updata_rd_by_ie_switch_etc::hmac_vap null.}");
        return OAL_PTR_NULL;
    }

    *update_rd_by_ie_sw = hmac_vap->updata_rd_by_ie_switch;
    return OAL_SUCC;
}
#endif
/*****************************************************************************
 函 数 名  : hmac_vap_get_net_device_etc
 功能描述  : 通过vap id获取 net_device
*****************************************************************************/
oal_net_device_stru  *hmac_vap_get_net_device_etc(osal_u8 vap_id)
{
    hmac_vap_stru   *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_get_net_device_etc::hmac_vap null.}", vap_id);
        return OAL_PTR_NULL;
    }

    return (hmac_vap->net_device);
}


/*****************************************************************************
 函 数 名  : hmac_vap_creat_netdev_etc
 功能描述  : 获取hmac_vap结构体中的私有配置项
*****************************************************************************/
osal_u32  hmac_vap_creat_netdev_etc(hmac_vap_stru *hmac_vap, osal_s8 *netdev_name, osal_u8 name_len, osal_s8 *mac_addr)

{
    oal_net_device_stru *net_device;
    osal_u32           ul_return;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (netdev_name == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_vap_creat_netdev_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    net_device = oal_net_alloc_netdev(0, netdev_name, oal_ether_setup);
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    net_device = oal_net_alloc_netdev((osal_char *)netdev_name, name_len);
#endif
    if (osal_unlikely(net_device == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_vap_creat_netdev_etc::net_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如下对netdevice的赋值暂时按如下操作 */
    OAL_NETDEVICE_OPS(net_device)             = &gst_vap_net_dev_cfg_vap_ops;
    OAL_NETDEVICE_DESTRUCTOR(net_device)      = oal_net_free_netdev;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
    oal_netdevice_master(net_device)          = OAL_PTR_NULL;
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    oal_netdevice_master(net_device)          = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(net_device)         = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(net_device)  = 5; /* watchdog_timeo配置为5 */
    (osal_void)memcpy_s(OAL_NETDEVICE_MAC_ADDR(net_device), WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
    net_device->ml_priv = hmac_vap;
    oal_netdevice_qdisc(net_device, OAL_PTR_NULL);

    ul_return = (osal_u32)oal_net_register_netdev(net_device);
    if (osal_unlikely(ul_return != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_creat_netdev_etc::oal_net_register_netdev failed.}",
                         hmac_vap->vap_id);
        oal_net_free_netdev(net_device);
        return ul_return;
    }

    hmac_vap->net_device = net_device;
    /* 包括'\0' */
    (osal_void)memcpy_s(hmac_vap->name, OAL_IF_NAME_SIZE, net_device->name, OAL_IF_NAME_SIZE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_destroy_etc
 功能描述  : 销毁hmac vap的处理函数
 输入参数  : 指向要销毁的vap指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/
osal_s32  hmac_vap_destroy_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_down_vap_param_stru   down_vap;
    hmac_cfg_del_vap_param_stru    del_vap_param;
    osal_s32                    l_ret;
    frw_msg msg_info;
    unref_param(msg);
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_vap_destroy_etc::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 先down vap */
#ifdef _PRE_WLAN_FEATURE_P2P
    down_vap.p2p_mode = hmac_vap->p2p_mode;
#endif
    down_vap.net_dev = hmac_vap->net_device;
    msg_info.data = (osal_u8 *)&down_vap;
    msg_info.data_len = OAL_SIZEOF(mac_cfg_down_vap_param_stru);

    l_ret = hmac_config_down_vap_etc(hmac_vap, &msg_info);
    oam_warning_log2(0, OAM_SF_DFT, "hmac_vap_destroy_etc:destroy vap[%d],send DISASSOC,err code [%d]",
        hmac_vap->vap_id, MAC_DISAS_LV_SS);
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_destroy_etc::hmac_config_down_vap_etc failed[%d].}",
                         hmac_vap->vap_id, l_ret);
        return l_ret;
    }

    /* 然后再delete vap */
    del_vap_param.add_vap.p2p_mode = hmac_vap->p2p_mode;
    del_vap_param.add_vap.vap_mode = hmac_vap->vap_mode;
    msg_info.data = (osal_u8 *)&del_vap_param;
    msg_info.data_len = OAL_SIZEOF(hmac_cfg_del_vap_param_stru);
    l_ret = hmac_config_del_vap_etc(hmac_vap, &msg_info);
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_destroy_etc::hmac_config_del_vap_etc failed[%d].}",
                         hmac_vap->vap_id, l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_vap_check_ht_protection(mac_protection_stru *protection,
    hmac_user_stru *hmac_user, oal_bool_enum prev_asoc_ht, oal_bool_enum prev_asoc_non_ht)
{
    mac_user_ht_hdl_stru *ht_hdl = &(hmac_user->ht_hdl);

    if (ht_hdl->ht_capable == OAL_FALSE) { /* STA不支持HT */
        /* 如果STA之前没有与AP关联 */
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            protection->sta_non_ht_num++;
        } else if (prev_asoc_ht == OAL_TRUE) { /* 如果STA之前已经作为HT站点与AP关联 */
            protection->sta_non_ht_num++;

            if ((ht_hdl->supported_channel_width == OAL_FALSE) && (protection->sta_20m_only_num != 0)) {
                protection->sta_20m_only_num--;
            }

            if ((ht_hdl->ht_green_field == OAL_FALSE) && (protection->sta_non_gf_num != 0)) {
                protection->sta_non_gf_num--;
            }

            if ((ht_hdl->lsig_txop_protection == OAL_FALSE) && (protection->sta_no_lsig_txop_num != 0)) {
                protection->sta_no_lsig_txop_num--;
            }
        } else { /* STA 之前已经作为非HT站点与AP关联 */
        }
    } else { /* STA支持HT */
        /*  如果STA之前已经以non-HT站点与AP关联, 则将sta_non_ht_num减1 */
        if ((prev_asoc_non_ht == OAL_TRUE) && (protection->sta_non_ht_num != 0)) {
            protection->sta_non_ht_num--;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_check_ht_capabilities_ap_etc
 功能描述  : 检查请求关联的STA的 HT Capabilities element
*****************************************************************************/
osal_u16 hmac_vap_check_ht_capabilities_ap_etc(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 msg_len,
    hmac_user_stru *hmac_user, osal_bool is_tkip)
{
    oal_bool_enum           prev_asoc_ht = OAL_FALSE;
    oal_bool_enum           prev_asoc_non_ht = OAL_FALSE;
    mac_user_ht_hdl_stru   *pst_ht_hdl      = &(hmac_user->ht_hdl);
    mac_protection_stru    *protection;
    osal_u8              *ie          = OAL_PTR_NULL;

    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OAL_FALSE) {
        return MAC_SUCCESSFUL_STATUSCODE;
    }
    /* 检查STA是否是作为一个HT capable STA与AP关联 */
    if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) && (pst_ht_hdl->ht_capable == OAL_TRUE)) {
        hmac_user_set_ht_capable_etc(hmac_user, OAL_FALSE);
        prev_asoc_ht = OAL_TRUE;
    /* 检查STA是否是作为一个non HT capable STA与AP关联 */
    } else if (hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) {
        prev_asoc_non_ht = OAL_TRUE;
    }

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, (osal_s32)msg_len);
    if (ie != OAL_PTR_NULL && ie[1] >= MAC_HT_CAP_LEN) {
        /* 不允许HT STA 使用 TKIP/WEP 加密 */
        if (mac_is_wep_enabled(hmac_vap)) {
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] "
                "{hmac_vap_check_ht_capabilities_ap_etc::Rejecting a HT STA because of its Pairwise Cipher[%d].}",
                hmac_vap->vap_id, hmac_user->key_info.cipher_type);
            return MAC_MISMATCH_HTCAP;
        }

        /* 搜索 HT Capabilities Element */
        hmac_search_ht_cap_ie_ap_etc(hmac_vap, hmac_user, ie, 0, prev_asoc_ht);

        /* 支持ht能力，但是空间流速率集为0，要置对端user为不支持ht能力，从而以11a或者11g速率关联 */
        if ((pst_ht_hdl->rx_mcs_bitmask[3] == 0) && (pst_ht_hdl->rx_mcs_bitmask[2] == 0) && /* 2,3:引索值 */
            (pst_ht_hdl->rx_mcs_bitmask[1] == 0) && (pst_ht_hdl->rx_mcs_bitmask[0]) == 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_vap_check_ht_capabilities_ap_etc::STA support ht capability but support none space_stream.}");
            /* 对端ht能力置为不支持 */
            hmac_user_set_ht_capable_etc(hmac_user, OAL_FALSE);
        }
        /* 支持ht能力，但是使用tkip算法，要置对端user为不支持ht能力 */
        if (is_tkip == OSAL_TRUE) {
            hmac_user_set_ht_capable_etc(hmac_user, OAL_FALSE);
        }
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, payload, (osal_s32)msg_len);
    if (ie != OAL_PTR_NULL) {
        hmac_ie_proc_ext_cap_ie_etc(hmac_user, ie);
    }
#endif  /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */

    /* 走到这里，说明sta已经被统计到ht相关的统计量中 */
    hmac_user->user_stats_flag.no_ht_stats_flag = OAL_TRUE;
    hmac_user->user_stats_flag.no_gf_stats_flag = OAL_TRUE;
    hmac_user->user_stats_flag.only_stats_20m_flag = OAL_TRUE;
    hmac_user->user_stats_flag.no_lsig_txop_stats_flag = OAL_TRUE;
    hmac_user->user_stats_flag.no_40dsss_stats_flag = OAL_TRUE;

    protection = &(hmac_vap->protection);

    hmac_vap_check_ht_protection(protection, hmac_user, prev_asoc_ht, prev_asoc_non_ht);

    return MAC_SUCCESSFUL_STATUSCODE;
}

/*****************************************************************************

 函 数 名  : hmac_search_txbf_cap_ie_ap_etc
 功能描述  : 检查请求关联的STA的 TXBF Capabilities element
*****************************************************************************/
OSAL_STATIC osal_void  hmac_search_txbf_cap_ie_ap_etc(mac_user_ht_hdl_stru *pst_ht_hdl, osal_u32 info_elem)
{
    osal_u32 tmp_txbf_elem = info_elem;

    pst_ht_hdl->imbf_receive_cap                = (tmp_txbf_elem & BIT0);
    pst_ht_hdl->receive_staggered_sounding_cap  = ((tmp_txbf_elem & BIT1) >> 1);
    pst_ht_hdl->transmit_staggered_sounding_cap = ((tmp_txbf_elem & BIT2) >> 2); /* 2偏移值 */
    pst_ht_hdl->receive_ndp_cap                 = ((tmp_txbf_elem & BIT3) >> 3); /* 3偏移值 */
    pst_ht_hdl->transmit_ndp_cap                = ((tmp_txbf_elem & BIT4) >> 4); /* 4偏移值 */
    pst_ht_hdl->imbf_cap                        = ((tmp_txbf_elem & BIT5) >> 5); /* 5偏移值 */
    pst_ht_hdl->calibration                     = ((tmp_txbf_elem & 0x000000C0) >> 6); /* 6偏移值 */
    pst_ht_hdl->exp_csi_txbf_cap                = ((tmp_txbf_elem & BIT8) >> 8); /* 8偏移值 */
    pst_ht_hdl->exp_noncomp_txbf_cap            = ((tmp_txbf_elem & BIT9) >> 9); /* 9偏移值 */
    pst_ht_hdl->exp_comp_txbf_cap               = ((tmp_txbf_elem & BIT10) >> 10); /* 10偏移值 */
    pst_ht_hdl->exp_csi_feedback                = ((tmp_txbf_elem & 0x00001800) >> 11); /* 11偏移值 */
    pst_ht_hdl->exp_noncomp_feedback            = ((tmp_txbf_elem & 0x00006000) >> 13); /* 13偏移值 */

    pst_ht_hdl->exp_comp_feedback               = ((tmp_txbf_elem & 0x0001C000) >> 15); /* 15偏移值 */
    pst_ht_hdl->min_grouping                    = ((tmp_txbf_elem & 0x00060000) >> 17); /* 17偏移值 */
    pst_ht_hdl->csi_bfer_ant_number             = ((tmp_txbf_elem & 0x001C0000) >> 19); /* 19偏移值 */
    pst_ht_hdl->noncomp_bfer_ant_number         = ((tmp_txbf_elem & 0x00600000) >> 21); /* 21偏移值 */
    pst_ht_hdl->comp_bfer_ant_number            = ((tmp_txbf_elem & 0x01C00000) >> 23); /* 23偏移值 */
    pst_ht_hdl->csi_bfee_max_rows               = ((tmp_txbf_elem & 0x06000000) >> 25); /* 25偏移值 */
    pst_ht_hdl->channel_est_cap                 = ((tmp_txbf_elem & 0x18000000) >> 27); /* 27偏移值 */
}

typedef struct {
    hmac_user_stru *hmac_user_sta;
    osal_u8 *payload;
} hmac_user_stru_payload;
OAL_STATIC osal_void hmac_set_ht_base_cap(hmac_vap_stru *hmac_vap, hmac_user_stru_payload *user_paylooad,
    osal_u16 *current_start, oal_bool_enum prev_asoc_ht, mac_user_ht_hdl_stru *ht_hdl)
{
    osal_u8 ht_green_field;
    osal_u16 tmp_info_elem;
    osal_u16 current_offset = *current_start;
    hmac_user_stru *mac_user = user_paylooad->hmac_user_sta;

    hmac_user_get_ht_hdl_etc(user_paylooad->hmac_user_sta, ht_hdl);

    /* 带有 HT Capability Element 的 STA，标示它具有HT capable. */
    ht_hdl->ht_capable = 1;
    current_offset += MAC_IE_HDR_LEN;

    /***************************************************************************
                    解析 HT Capabilities Info Field
        ***************************************************************************/
    tmp_info_elem = OAL_MAKE_WORD16(user_paylooad->payload[current_offset], user_paylooad->payload[current_offset + 1]);
    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    ht_hdl->ldpc_coding_cap = (tmp_info_elem & BIT0);
    /* 检查STA所支持的信道宽度 B1，0:仅20M运行，1:20M与40M运行 */
    ht_hdl->supported_channel_width = (osal_u16)mac_ie_proc_ht_supported_channel_width_etc(mac_user, hmac_vap,
        (osal_u8)((tmp_info_elem & BIT1) >> 1), prev_asoc_ht);
    ht_hdl->sm_power_save = (hmac_ie_proc_sm_power_save_field_etc(mac_user,
        (osal_u8)(tmp_info_elem & (BIT3 | BIT2)) >> 2));  /* 检查空间复用节能模式 B2~B3 , 2为偏移值 */
    /* 检查Greenfield 支持情况 B4， 0:不支持，1:支持 */
    ht_green_field = (osal_u8)((tmp_info_elem & BIT4) >> 4); /* 4偏移值 */
    ht_hdl->ht_green_field = mac_ie_proc_ht_green_field_etc(mac_user, hmac_vap, ht_green_field, prev_asoc_ht);
    /* 检查20MHz Short-GI B5,  0:不支持，1:支持，之后与AP取交集  */
    ht_hdl->short_gi_20mhz = ((tmp_info_elem & BIT5) >> 5); /* 5偏移值 */
    ht_hdl->short_gi_20mhz &= mac_mib_get_short_gi_option_in_twenty_implemented(hmac_vap);
    /* 检查40MHz Short-GI B6,  0:不支持，1:支持，之后与AP取交集 */
    ht_hdl->short_gi_40mhz = ((tmp_info_elem & BIT6) >> 6); /* 6偏移值 */
    ht_hdl->short_gi_40mhz &= mac_mib_get_short_gi_option_in_forty_implemented(hmac_vap);
    /* 检查支持接收STBC PPDU B8,  0:不支持，1:支持 */
    ht_hdl->rx_stbc = ((tmp_info_elem & 0x0300) >> 8); /* 8偏移值 */
    /* 检查最大A-MSDU长度 B11，0:3839字节, 1:7935字节 */
    ht_hdl->max_amsdu_length = ((tmp_info_elem & BIT11) >> 11); /* 11偏移值 */
    user_paylooad->hmac_user_sta->amsdu_maxsize = ((tmp_info_elem & BIT11) == 0) ? WLAN_MIB_MAX_AMSDU_LENGTH_SHORT :
                                          WLAN_MIB_MAX_AMSDU_LENGTH_LONG;
    mac_user->amsdu_maxsize = user_paylooad->hmac_user_sta->amsdu_maxsize;
    /*
        * 检查在40M上DSSS/CCK的支持情况 B12
        * 在非Beacon帧或probe rsp帧中时
        * 0: STA在40MHz上不使用DSSS/CCK, 1: STA在40MHz上使用DSSS/CCK
        */
    ht_hdl->dsss_cck_mode_40mhz = ((tmp_info_elem & BIT12) >> 12); /* 12偏移值 */

    if ((ht_hdl->dsss_cck_mode_40mhz == 0) && (ht_hdl->supported_channel_width == 1)) {
        hmac_vap->protection.sta_no_40dsss_cck_num++;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 检查Forty MHz Intolerant */
    ht_hdl->forty_mhz_intolerant = ((tmp_info_elem & BIT14) >> 14); /* 14偏移值 */
#endif
    /*  检查对L-SIG TXOP 保护的支持情况 B15, 0:不支持，1:支持 */
    ht_hdl->lsig_txop_protection = (osal_u16)hmac_ie_proc_lsig_txop_protection_support_etc(mac_user, hmac_vap,
        (osal_u8)((tmp_info_elem & BIT15) >> 15), prev_asoc_ht); /* 15偏移值 */

    current_offset += MAC_HT_CAPINFO_LEN;

    /***************************************************************************
                        解析 A-MPDU Parameters Field
        ***************************************************************************/
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    ht_hdl->max_rx_ampdu_factor = (user_paylooad->payload[current_offset] & 0x03);
    /* 提取 the Minimum MPDU Start Spacing (B2 - B4) */
    ht_hdl->min_mpdu_start_spacing = (user_paylooad->payload[current_offset] >> 2) & 0x07;

    current_offset += MAC_HT_AMPDU_PARAMS_LEN;
    *current_start = current_offset;
}


/*****************************************************************************
 函 数 名  : hmac_search_ht_cap_ie_ap_etc
 功能描述  : 在关联请求请求中搜索HT Cap IE
*****************************************************************************/
osal_u32  hmac_search_ht_cap_ie_ap_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u16 offset, oal_bool_enum prev_asoc_ht)
{
    osal_u8                   mcs_bmp_index;
    osal_u16                  tmp_info_elem;
    osal_u16                  tmp_txbf_low;
    osal_u32                  tmp_txbf_elem;
    mac_user_ht_hdl_stru        ht_hdl;
    hmac_device_stru           *hmac_device;
    hmac_user_stru_payload      user_payload;
    mac_user_ht_hdl_stru       *pst_ht_hdl = &ht_hdl;
    osal_u16 current_offset = offset;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_search_ht_cap_ie_ap_etc::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    user_payload.hmac_user_sta = hmac_user;
    user_payload.payload = payload;
    hmac_set_ht_base_cap(hmac_vap, &user_payload, &current_offset, prev_asoc_ht, pst_ht_hdl);

    /***************************************************************************
                        解析 Supported MCS Set Field
    ***************************************************************************/
    for (mcs_bmp_index = 0; mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; mcs_bmp_index++) {
        pst_ht_hdl->rx_mcs_bitmask[mcs_bmp_index] =
            /* 该能力是user自身的，不应该和vap自身取小 */
            (*(osal_u8 *)(payload + current_offset + mcs_bmp_index));
    }

    pst_ht_hdl->rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    current_offset += MAC_HT_SUP_MCS_SET_LEN;

    /***************************************************************************
                        解析 HT Extended Capabilities Info Field
    ***************************************************************************/
    tmp_info_elem = OAL_MAKE_WORD16(payload[current_offset], payload[current_offset + 1]);
    /* 提取 HTC support Information */
    if ((tmp_info_elem & BIT10) != 0) {
        pst_ht_hdl->htc_support = 1;
    }

    current_offset += MAC_HT_EXT_CAP_LEN;

    /***************************************************************************
                        解析 Tx Beamforming Field
    ***************************************************************************/
    tmp_info_elem = OAL_MAKE_WORD16(payload[current_offset], payload[current_offset + 1]);
    /* 2 3偏移值 */
    tmp_txbf_low  = OAL_MAKE_WORD16(payload[current_offset + 2], payload[current_offset + 3]);
    tmp_txbf_elem = OAL_MAKE_WORD32(tmp_info_elem, tmp_txbf_low);
    hmac_search_txbf_cap_ie_ap_etc(pst_ht_hdl, tmp_txbf_elem);

    hmac_user_set_ht_hdl_etc(hmac_user, pst_ht_hdl);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    hmac_chan_update_40m_intol_user_etc(hmac_vap);
    oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_search_ht_cap_ie_ap_etc::en_40m_intol_user[%d].}",
                     hmac_vap->vap_id, hmac_vap->en_40m_intol_user);

    /* 如果存在40M不容忍的user，则不允许AP在40MHz运行 */
    if (hmac_vap->en_40m_intol_user == OAL_TRUE) {
        if (hmac_vap->cap_flag.autoswitch_2040 != 0) {
            hmac_40m_intol_sync_data(hmac_vap, WLAN_BAND_WIDTH_BUTT, hmac_vap->en_40m_intol_user);
        }
    } else {
        mac_mib_set_forty_mhz_intolerant(hmac_vap, OAL_FALSE);
    }
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
/*****************************************************************************
 函 数 名  : hmac_vap_net_start_subqueue_etc
 功能描述  : 使能某个VAP队列的某个队列
*****************************************************************************/
OAL_STATIC osal_void hmac_vap_wake_subq(osal_u8 vap_id, osal_u16 queue_idx)
{
    oal_net_device_stru    *net_device = NULL;
    hmac_vap_stru          *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    net_device = hmac_vap->net_device;
    if (net_device == NULL) {
        return;
    }

    oal_net_wake_subqueue(net_device, queue_idx);
}
/*****************************************************************************
 函 数 名  : hmac_vap_net_start_subqueue_etc
 功能描述  : 使能所有VAP队列的某个队列
*****************************************************************************/
osal_void hmac_vap_net_start_subqueue_etc(hcc_queue_type queue_idx)
{
    osal_u8               vap_id;
    OAL_STATIC osal_u8    g_uc_start_subq_flag = 0;
#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    osal_u16 queue_mapping = hcc_data_queue_to_wlan(queue_idx);
#else
    osal_u16 queue_mapping = queue_idx;
#endif
    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */

    if (g_uc_start_subq_flag == 0) {
        g_uc_start_subq_flag = 1;

        /* vap id从低到高恢复 跳过配置vap */
        for (vap_id = 1; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
            hmac_vap_wake_subq(vap_id, queue_mapping);
        }
    } else {
        g_uc_start_subq_flag = 0;

        /* vap id从高到低恢复 */
        for (vap_id = WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id > 1; vap_id--) {
            hmac_vap_wake_subq(vap_id - 1, queue_mapping);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_stop_subq
 功能描述  : 停止某个VAP队列的某个队列
*****************************************************************************/
OAL_STATIC osal_void  hmac_vap_stop_subq(osal_u8 vap_id, osal_u16 queue_idx)
{
    hmac_vap_stru          *hmac_vap = NULL;
    oal_net_device_stru    *net_device = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    net_device = hmac_vap->net_device;
    if (net_device == NULL) {
        return;
    }

    oal_net_stop_subqueue(net_device, queue_idx);
}

/*****************************************************************************
 函 数 名  : hmac_vap_net_stop_subqueue_etc
 功能描述  : 使能所有VAP队列的某个队列
*****************************************************************************/
osal_void hmac_vap_net_stop_subqueue_etc(hcc_queue_type queue_idx)
{
    osal_u8               vap_id;
    OAL_STATIC osal_u8    g_uc_stop_subq_flag = 0;
#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    osal_u16 queue_mapping = hcc_data_queue_to_wlan(queue_idx);
#else
    osal_u16 queue_mapping = queue_idx;
#endif
    /* 自旋锁内，任务和软中断都被锁住，不需要FRW锁 */

    /* 由按照VAP ID顺序停止subq，改为不依据VAP ID顺序 */
    if (g_uc_stop_subq_flag == 0) {
        g_uc_stop_subq_flag = 1;

        for (vap_id = 1; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
            hmac_vap_stop_subq(vap_id, queue_mapping);
        }
    } else {
        g_uc_stop_subq_flag = 0;

        for (vap_id = WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id > 1; vap_id--) {
            hmac_vap_stop_subq(vap_id - 1, queue_mapping);
        }
    }
}

#endif

/*****************************************************************************
 函 数 名  : hmac_handle_disconnect_rsp_etc
*****************************************************************************/
osal_void hmac_handle_disconnect_rsp_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_reason_code_enum_uint16  disasoc_reason)
{
    /* 修改 state & 删除 user */
    switch (hmac_vap->vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            /* 抛事件上报内核，已经去关联某个STA */
            hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);
            break;

        case WLAN_VAP_MODE_BSS_STA:
            /* 上报内核sta已经和某个ap去关联 */
            hmac_sta_handle_disassoc_rsp_etc(hmac_vap, disasoc_reason);
            break;
        default:
            break;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_pmksa_etc
*****************************************************************************/
osal_u8 *hmac_vap_get_pmksa_etc(hmac_vap_stru *hmac_vap, osal_u8 *bssid)
{
    hmac_pmksa_cache_stru              *pmksa_cache = OAL_PTR_NULL;
    struct osal_list_head                *pmksa_entry;
    struct osal_list_head                *pmksa_entry_tmp;

    if (hmac_vap == OAL_PTR_NULL || bssid == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_vap_get_pmksa_etc param null}\r\n");
        return OAL_PTR_NULL;
    }

    if (osal_list_empty(&(hmac_vap->pmksa_list_head)) != 0) {
        return OAL_PTR_NULL;
    }

    osal_list_for_each_safe(pmksa_entry, pmksa_entry_tmp, &(hmac_vap->pmksa_list_head)) {
        pmksa_cache = osal_list_entry(pmksa_entry, hmac_pmksa_cache_stru, entry);
        if (oal_compare_mac_addr(bssid, pmksa_cache->bssid) == 0) {
            osal_list_del(pmksa_entry);
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
            oam_warning_log4(0, OAM_SF_CFG, "{hmac_vap_get_pmksa_etc:: FIND Pmksa of [%02X:%02X:%02X:%02X:XX:XX]}",
                             bssid[0], bssid[1], bssid[2], bssid[3]);  /* 0 1 2 3索引值 */
            break;
        }
        pmksa_cache = OAL_PTR_NULL;
    }

    if (pmksa_cache) {
        osal_list_add(&(pmksa_cache->entry), &(hmac_vap->pmksa_list_head));
        return pmksa_cache->pmkid;
    }
    return OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_tx_get_mac_vap_etc
 功能描述  : 获取VAP，并判断VAP状态
 输入参数  : event event结构体
 输出参数  : hmac_vap_stru VAP结构体
 调用函数  : hmac_tx_wlan_to_wlan_ap_etc
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 hmac_tx_get_mac_vap_etc(osal_u8 vap_id, hmac_vap_stru **hmac_vap)
{
    hmac_vap_stru         *hmac_vap_tmp;

    /* 获取vap结构信息 */
    hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (osal_unlikely(hmac_vap_tmp == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_get_mac_vap_etc::hmac_vap null}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP模式判断 */
    if (hmac_vap_tmp->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] hmac_tx_get_mac_vap_etc::vap_mode error[%d]",
            hmac_vap_tmp->vap_id, hmac_vap_tmp->vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* VAP状态判断 */
    if ((hmac_vap_tmp->vap_state != MAC_VAP_STATE_UP) && (hmac_vap_tmp->vap_state != MAC_VAP_STATE_PAUSE)) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] hmac_tx_get_mac_vap_etc::vap_state[%d] error",
            hmac_vap_tmp->vap_id, hmac_vap_tmp->vap_state);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    *hmac_vap = hmac_vap_tmp;

    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) \
    && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
OAL_STATIC osal_s32  hmac_cfg_vap_if_open(oal_net_device_stru *dev)
{
    dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC osal_s32  hmac_cfg_vap_if_close(oal_net_device_stru *dev)
{
    dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_cfg_vap_if_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev)
{
    unref_param(dev);
    if (buf) {
        oal_netbuf_free(buf);
    }
    return OAL_NETDEV_TX_OK;
}
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_s32  hmac_cfg_vap_if_close(oal_net_device_stru *dev)
{
    dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC osal_s32  hmac_cfg_vap_if_open(oal_net_device_stru *dev)
{
    dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

OAL_STATIC oal_net_dev_tx_enum  hmac_cfg_vap_if_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev)
{
    unref_param(dev);
    if (buf) {
        oal_netbuf_free(buf);
    }
    return OAL_NETDEV_TX_OK;
}
#endif

/* 本函数会校验vap是否已经创建并且处于正常运行状态, 如果vap正处于创建或者删除的过程中，请勿调用此函数 */
WIFI_TCM_TEXT hmac_vap_stru *hmac_get_valid_vap(osal_u32 idx)
{
    if (idx == 0) { /* 配置vap不校验vap有效性  */
        return &(g_ast_hmac_vap[idx]);
    } else if ((idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) && (g_ast_hmac_vap[idx].init_flag == MAC_VAP_VAILD)) {
        return &(g_ast_hmac_vap[idx]);
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "err para in hmac_get_vap");
        return NULL;
    }
}

hmac_vap_stru *hmac_get_vap(osal_u32 idx)
{
    if (idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return &(g_ast_hmac_vap[idx]);
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "err para in hmac_get_vap");
        return NULL;
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void mac_vap_intrrupt_enable(osal_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    preempt_enable();
    local_bh_enable();
#endif
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void mac_vap_intrrupt_disable(osal_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    preempt_disable();
    local_bh_disable();
#endif
}

wlan_protocol_mode_enum_uint8 hmac_get_protocol_mode_by_wlan_protocol(wlan_protocol_enum_uint8 protocol)
{
    wlan_protocol_mode_enum_uint8 protocol_mode;
    switch (protocol) {
        case WLAN_LEGACY_11B_MODE:
            protocol_mode = WLAN_PROTOCOL_LEGACY_11B_MODE;
            break;
        case WLAN_LEGACY_11A_MODE:
            protocol_mode = WLAN_PROTOCOL_LEGACY_11A_MODE;
            break;
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            protocol_mode = WLAN_PROTOCOL_LEGACY_11G_MODE;
            break;
        case WLAN_HT_MODE:
        case WLAN_HT_11G_MODE:
        case WLAN_HT_ONLY_MODE:
            protocol_mode = WLAN_PROTOCOL_HT_MODE;
            break;
        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            protocol_mode = WLAN_PROTOCOL_VHT_MODE;
            break;
        case WLAN_HE_MODE:
            protocol_mode = WLAN_PROTOCOL_HE_MODE;
            break;
        default:
            protocol_mode = WLAN_PROTOCOL_HE_MODE;
    }
    return protocol_mode;
}

osal_bool hmac_vap_need_update_protocol(const hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 new_wlan_protocol)
{
    wlan_protocol_mode_enum_uint8 new_protocol_mode = hmac_get_protocol_mode_by_wlan_protocol(new_wlan_protocol);
    if (new_protocol_mode > hmac_vap->max_protocol) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

osal_u8 hmac_vap_is_connecting(hmac_vap_stru *mac_vap)
{
    struct osal_list_head *entry = NULL;
    struct osal_list_head *user_list_head = NULL;
    hmac_user_stru *user_tmp = NULL;
    /* 遍历vap下所有用户, 检查有没有在关联中的 */
    user_list_head = &(mac_vap->mac_user_list_head);
    for (entry = user_list_head->next; entry != user_list_head;) {
        user_tmp = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (user_tmp->user_asoc_state != MAC_USER_STATE_ASSOC) {
            return OSAL_TRUE;
        }
        /* 指向双向链表下一个 */
        entry = entry->next;
    }
    return OSAL_FALSE;
}

oal_module_symbol(hmac_vap_get_net_device_etc);
oal_module_symbol(hmac_vap_get_desired_country_etc);
oal_module_symbol(hmac_vap_destroy_etc);

#ifdef _PRE_WLAN_FEATURE_11D
oal_module_symbol(hmac_vap_get_updata_rd_by_ie_switch_etc);
#endif

// from dmac_vap_rom.c
OSAL_STATIC osal_void hmac_sta_bw_switch_fsm_init_entry(osal_void *ctx);
OSAL_STATIC osal_void hmac_sta_bw_switch_fsm_normal_entry(osal_void *ctx);
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void  *const event_data);
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_normal_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *const event_data);
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_verify_20m_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_verify_40m_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_invalid_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_bw_fsm_trans_to_state(hmac_sta_bw_switch_fsm_info_stru *bw_fsm, osal_u8 state);
OSAL_STATIC oal_bool_enum_uint8 hmac_vap_is_fakeq_empty(hmac_vap_stru *hmac_vap);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_fsm_state_info g_sta_bw_switch_fsm_info[] = {
    {
        HMAC_STA_BW_SWITCH_FSM_INIT,
        "INIT",
        hmac_sta_bw_switch_fsm_init_entry,
        OSAL_NULL,
        hmac_sta_bw_switch_fsm_init_event,
    },
    {
        HMAC_STA_BW_SWITCH_FSM_NORMAL,
        "NORMAL",
        hmac_sta_bw_switch_fsm_normal_entry,
        OSAL_NULL,
        hmac_sta_bw_switch_fsm_normal_event,
    },
    {
        HMAC_STA_BW_SWITCH_FSM_VERIFY20M,
        "VERIFY20M",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_bw_switch_fsm_verify_20m_event,
    },
    {
        HMAC_STA_BW_SWITCH_FSM_VERIFY40M,
        "VERIFY40M",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_bw_switch_fsm_verify_40m_event,
    },
    {
        HMAC_STA_BW_SWITCH_FSM_INVALID,
        "INVALID",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_bw_switch_fsm_invalid_event,
    }
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_vap_get_legacy_rate_and_protocol
 功能描述  : 获取不同频段下的11b或11ag或管理帧的初始速率和协议
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_get_legacy_rate_and_protocol(const hmac_vap_stru * const hmac_vap,
    wlan_channel_band_enum_uint8 band, osal_u8 *protocol_mode, osal_u8 *legacy_rate)
{
    const hmac_set_rate_stru *rate;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    rate = &(hmac_vap->vap_curr_rate);

    if ((band == WLAN_BAND_2G)  && (is_legacy_vap(hmac_vap))) {
        /* P2P 设备不能发送11b 速率的帧 */
        *protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
        *legacy_rate   = WLAN_PHY_RATE_1M;

        if (rate->capable_flag.capable.support_11b != OSAL_TRUE) {
            if (osal_likely(rate->capable_flag.capable.support_11ag == OSAL_TRUE)) {
                *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            } else {
                oam_error_log1(0, 0,
                    "{hmac_vap_get_legacy_rate_and_protocol::hmac_vap[%d] 2g not support all protocol",
                    hmac_vap->vap_id);
                return OAL_FAIL;
            }
        }
    } else {
        *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        *legacy_rate   = WLAN_PHY_RATE_6M;

        if (rate->capable_flag.capable.support_11ag != OSAL_TRUE) {
            /* 11b only模式依然会刷5g */
            oam_warning_log1(0, OAM_SF_ANY,
                "{hmac_vap_get_legacy_rate_and_protocol::hmac_vap[%d] 5g is not support 11ag, now reset dmac vap",
                hmac_vap->vap_id);
            return OAL_FAIL;
        }
    }
    /* hmac_vap 改变速率信息的时候，如果对应能力位不支持，只会改变能力位，不会刷速率，所以速率永远是正确的 */
    *legacy_rate   = rate->min_rate[*protocol_mode];
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_non_ucast_data_rate_and_protocol
 功能描述  : 获取非单播数据帧的初始速率和协议
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_get_non_ucast_data_rate_and_protocol(const hmac_vap_stru *hmac_vap,
    osal_u8 *protocol_mode, osal_u8 *legacy_rate)
{
    const hmac_set_rate_stru *rate = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    rate = &hmac_vap->vap_curr_rate;
    *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    *legacy_rate = WLAN_PHY_RATE_6M;

    switch (hmac_vap->protocol) {
        /* 11b 1M */
        case WLAN_LEGACY_11B_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_HT_MODE:
            hmac_vap_get_legacy_rate_and_protocol(hmac_vap, hmac_vap->channel.band, protocol_mode, legacy_rate);
            break;

        /* OFDM 6M */
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_11G_MODE:
            *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *legacy_rate   = rate->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE];

            /* hmac_vap 改变速率信息的时候，如果对应能力位不支持，只会改变能力位，不会刷速率，所以速率永远是正确的 */
            if (osal_unlikely(rate->capable_flag.capable.support_11ag != OSAL_TRUE)) {
                /* 存在2.4G支持11ac，但不支持11ag速率的AP，该场景下仅打印warning */
                if (hmac_vap->protocol == WLAN_VHT_MODE &&
                    hmac_vap->channel.band == WLAN_BAND_2G) {
                    oam_warning_log1(0, OAM_SF_ANY,
                        "hmac_vap_get_non_ucast_data_rate_and_protocol:vap_id[%d] 2g without 11ag", hmac_vap->vap_id);
                } else {
                    oam_error_log1(0, OAM_SF_ANY,
                        "hmac_vap_get_non_ucast_data_rate_and_protocol:vap_id[%d] 5g without 11ag", hmac_vap->vap_id);
                    return OAL_FAIL;
                }
            }
            break;

        /* OFDM 24M */
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
        case WLAN_HE_MODE:
            *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *legacy_rate   = WLAN_PHY_RATE_24M;
            if ((hmac_vap->protocol == WLAN_HT_ONLY_MODE && (rate->capable_flag.capable.ht_capable == OSAL_FALSE)) ||
                (hmac_vap->protocol == WLAN_VHT_ONLY_MODE && (rate->capable_flag.capable.vht_capable == OSAL_FALSE))) {
                oam_error_log0(0, 0, "{hmac_vap_get_non_ucast_data_rate_and_protocol::not support HT or VHT.}");
            }
            break;

        default:
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_vap_get_non_ucast_data_rate_and_protocol::invalid protocol[%d].}",
                hmac_vap->vap_id, hmac_vap->protocol);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_init_rate_info
 功能描述  : 将速率信息设置成初始值
*****************************************************************************/
osal_u32  hmac_vap_init_rate_info(hmac_set_rate_stru *surr_rate)
{
    if (osal_unlikely(surr_rate == OSAL_NULL)) {
        return OAL_FALSE;
    }

    surr_rate->capable_flag.value = 0xF; // 全部为1
    surr_rate->min_rate[WLAN_11B_PHY_PROTOCOL_MODE] = WLAN_PHY_RATE_1M;
    surr_rate->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE] = WLAN_PHY_RATE_6M;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_param_init
 功能描述  : 初始化部分dmac vap信息1
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_param_init(hmac_vap_stru *hmac_vap, osal_u8 vap_id, osal_u8 tim_bitmap_len)
{
    /* 初始化特性标识中BEACON帧不轮流发送 */
    hmac_vap->beacon_tx_policy = HMAC_BEACON_TX_POLICY_SINGLE;

    /* 初始化节能相关参数 */
    hmac_vap->dtim_count  = WLAN_DTIM_DEFAULT;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 双芯片实现时bitmap_len计算要乘以device(也即是chip)个数  */
        hmac_vap->tim_bitmap_len = tim_bitmap_len;
        hmac_vap->tim_bitmap = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, hmac_vap->tim_bitmap_len, OAL_TRUE);
        if (hmac_vap->tim_bitmap == OSAL_NULL) {
            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_param_init::alloc tim_bitmap memory(length:%d)fail!}",
                vap_id, hmac_vap->tim_bitmap_len);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        (osal_void)memset_s(hmac_vap->tim_bitmap, hmac_vap->tim_bitmap_len, 0, hmac_vap->tim_bitmap_len);
        /* TIM bitmap len is default 1 */
        hmac_vap->tim_bitmap[0] = 1;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_vap_param_init_ext
 功能描述  : 初始化部分dmac vap信息2
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_param_init_ext(hmac_vap_stru *hmac_vap)
{
    osal_u8 qid;

    /* 初始化RSSI值，作平滑用 */
    hmac_vap->query_stats.signal = OAL_RSSI_INIT_MARKER;
    hmac_vap->query_stats.signal_mgmt_data = OAL_RSSI_INIT_MARKER;
    hmac_vap->query_stats.snr_ant0 = OAL_SNR_INIT_VALUE;
    hmac_vap->query_stats.snr_ant1 = OAL_SNR_INIT_VALUE;
    hmac_vap->beacon_timeout = HMAC_DEF_BCN_TIMEOUT;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap->in_tbtt_offset = HMAC_DEF_AP_TBTT_OFFSET;
    } else {
        hmac_vap->in_tbtt_offset = HAL_PM_DEF_TBTT_OFFSET;
    }
    hmac_vap->bcn_tout_max_cnt = HMAC_DEF_BCN_TOUT_MAX_CNT;

    /* 初始化记录ap在切换到20M之前的带宽为BUTT */
    hmac_vap->bandwidth_40m = WLAN_BAND_WIDTH_BUTT;

    /* 初始化虚假发送队列 */
    for (qid = 0; qid < HAL_TX_QUEUE_NUM; qid++) {
        oal_netbuf_list_head_init(&(hmac_vap->tx_dscr_queue_fake[qid].buff_header));
        hmac_vap->tx_dscr_queue_fake[qid].mpdu_cnt = 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_init
 功能描述  : 初始化要添加的dmac vap的一些特性信息
*****************************************************************************/
osal_u32 hmac_vap_init(hmac_vap_stru *hmac_vap, osal_u8 vap_id, const mac_cfg_add_vap_param_stru *param)
{
    osal_u32 ret;

    ret = hmac_vap_param_init(hmac_vap, vap_id, param->tim_bitmap_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    hmac_vap_param_init_ext(hmac_vap);

    /* 初始化dmac_vap 的速率信息 */
    if (hmac_vap_init_rate_info(&hmac_vap->vap_curr_rate) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_vap_init::hmac_vap_init_rate_info err!}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_init_non_ucast_tx_rate
 功能描述  : 初始化除单播数据帧以外所有帧的发送速率
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_init_non_ucast_tx_rate(const hmac_vap_stru *hmac_vap, hal_tx_txop_alg_stru *tx_cast,
    wlan_channel_band_enum_uint8 band, oal_bool_enum is_mgmt_flag)
{
    osal_u8       protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    osal_u8       legacy_rate   = WLAN_PHY_RATE_6M;

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_vap_init_non_ucast_tx_rate::hmac_vap null}");
        return;
    }
#endif
    hal_txop_alg_get_tx_dscr(tx_cast)[0].tx_count             = 3; /* 3表示传输次数 */
    hal_txop_alg_get_tx_dscr(tx_cast)[0].stbc_mode            = 0;
    hal_txop_alg_get_tx_dscr(tx_cast)[0].rts_cts_protect_mode = 0;
    hal_txop_alg_get_tx_dscr(tx_cast)[0].txbf_mode            = 0;
    hal_txop_alg_get_tx_dscr(tx_cast)[0].nss_rate.phy_mode.gi_type = 0;
    tx_cast->phy_info.lsig_txop = OSAL_FALSE;

    if (is_mgmt_flag) {
        /* 获取管理帧速率和协议 */
        hmac_vap_get_legacy_rate_and_protocol(hmac_vap, band, &protocol_mode, &legacy_rate);

        hal_txop_alg_get_tx_dscr(tx_cast)[0].tx_chain_sel = hmac_vap->hal_device->cfg_cap_info->single_tx_chain;
    } else {
        /* 获取非单播数据帧速率和协议 */
        hmac_vap_get_non_ucast_data_rate_and_protocol(hmac_vap, &protocol_mode, &legacy_rate);
        /* 11b 1M速率使用长前导码 */
        if (osal_likely(hmac_vap->hal_device != OSAL_NULL)) {
            hal_txop_alg_get_tx_dscr(tx_cast)[0].tx_chain_sel = (protocol_mode == WLAN_11B_PHY_PROTOCOL_MODE) ?
                hmac_vap->hal_device->cfg_cap_info->single_tx_chain :
                hmac_vap->hal_device->cfg_cap_info->phy2dscr_chain;
        }
    }

    hal_txop_alg_get_tx_dscr(tx_cast)[0].nss_rate.phy_mode.preamble_mode =
        hal_phy_11b_1m_rate(protocol_mode, legacy_rate) ? 1 : 0;
    hal_txop_alg_get_tx_dscr(tx_cast)[0].nss_rate.he_nss_mcs.he_mcs   = legacy_rate;
    hal_txop_alg_get_tx_dscr(tx_cast)[0].protocol_mode = protocol_mode;

    (osal_void)memcpy_s(&(hal_txop_alg_get_tx_dscr(tx_cast)[1]), sizeof(hal_tx_ctrl_desc_rate_stru),
        &(hal_txop_alg_get_tx_dscr(tx_cast)[0]), sizeof(hal_tx_ctrl_desc_rate_stru));
    /* 对描述符的第2位进行赋值 */
    (osal_void)memcpy_s(&(hal_txop_alg_get_tx_dscr(tx_cast)[2]), sizeof(hal_tx_ctrl_desc_rate_stru),
        &(hal_txop_alg_get_tx_dscr(tx_cast)[0]), sizeof(hal_tx_ctrl_desc_rate_stru));
    /* 对描述符的第3位进行赋值 */
    (osal_void)memcpy_s(&(hal_txop_alg_get_tx_dscr(tx_cast)[3]), sizeof(hal_tx_ctrl_desc_rate_stru),
        &(hal_txop_alg_get_tx_dscr(tx_cast)[0]), sizeof(hal_tx_ctrl_desc_rate_stru));

    if (!is_mgmt_flag) {
        hal_txop_alg_get_tx_dscr(tx_cast)[1].tx_count = 0;
        hal_txop_alg_get_tx_dscr(tx_cast)[2].tx_count = 0; /* 2表示描述符的第2位 */
        hal_txop_alg_get_tx_dscr(tx_cast)[3].tx_count = 0; /* 3表示描述符的第3位 */
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_init_tx_mgmt_rate
 功能描述  : 初始化管理帧发送速率参数
*****************************************************************************/
osal_void  hmac_vap_init_tx_mgmt_rate(const hmac_vap_stru *hmac_vap, hal_tx_txop_alg_stru *tx_mgmt_cast)
{
    if (osal_unlikely(tx_mgmt_cast == OSAL_NULL)) {
        return;
    }

    /* 初始化2.4G参数 */
    hmac_vap_init_non_ucast_tx_rate(hmac_vap, &tx_mgmt_cast[WLAN_BAND_2G], WLAN_BAND_2G, OSAL_TRUE);

    /* 初始化5G参数 */
    hmac_vap_init_non_ucast_tx_rate(hmac_vap, &tx_mgmt_cast[WLAN_BAND_5G], WLAN_BAND_5G, OSAL_TRUE);
}

/*****************************************************************************
 函 数 名  : hmac_vap_tx_frame_init
 功能描述  : 初始化除单播数据帧以外帧的发送速率
*****************************************************************************/
osal_u32  hmac_vap_init_tx_frame_params(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FALSE;
    }

    if (mgmt_rate_init_flag == OSAL_TRUE) {
        /* 初始化单播管理帧参数 */
        hmac_vap_init_tx_mgmt_rate(hmac_vap, hmac_vap->tx_mgmt_ucast);

        /* 初始化组播、广播管理帧参数 */
        hmac_vap_init_tx_mgmt_rate(hmac_vap, hmac_vap->tx_mgmt_bmcast);
    }

    return OAL_SUCC;
}

#ifdef BOARD_ASIC_WIFI
/* STA去关联/VAP删除刷新MAC时钟 */
OSAL_STATIC osal_void hmac_vap_exit_update_mac_clk(hmac_vap_stru *hmac_vap)
{
    osal_u8 vap_idx;
    mac_channel_stru *channel = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);

    if (hmac_vap->protocol == WLAN_HE_MODE && hmac_vap->hal_device->ax_vap_cnt != 0) {
        hmac_vap->hal_device->ax_vap_cnt--;
        hmac_hal_device_sync(hmac_vap->hal_device);
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap_tmp == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap null.}", hmac_device->vap_id[vap_idx]);
            return;
        }

        if (hmac_vap_tmp->vap_id == hmac_vap->vap_id) {
            continue;
        }

        channel = &(hmac_vap_tmp->channel);
        /* 当前存在其他AX设备或者40M带宽设备，则刷新MAC时钟 */
        if (hmac_vap_tmp->protocol == WLAN_HE_MODE ||
            (channel->en_bandwidth >= WLAN_BAND_WIDTH_40PLUS && channel->en_bandwidth <= WLAN_BAND_WIDTH_40MINUS)) {
            return;
        }
    }
    oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_vap_exit_update_mac_clk flush mac clk -> 80M}", hmac_vap->vap_id);
    hal_update_mac_freq(hmac_vap->hal_device, WLAN_BAND_WIDTH_20M);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_vap_sta_reset
 功能描述  : STA与某个AP去关联后的复位操作
*****************************************************************************/
osal_u32 hmac_vap_sta_reset(hmac_vap_stru *hmac_vap)
{
    osal_u8                       bssid[WLAN_MAC_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 删除BSSID */
    hal_set_sta_bssid(hmac_vap->hal_vap, bssid, WLAN_MAC_ADDR_LEN);

    /* 停止STA tsf */
    hal_disable_tsf_tbtt(hmac_vap->hal_vap);
    hmac_sta_stop_tbtt_timer(hmac_vap);
    /* 恢复slottime类型为short */
    hal_cfg_slottime_type(0);

    /* 恢复TXOP不使能状态 */
    if (hmac_device->txop_enable) {
        hmac_device->txop_enable = OSAL_FALSE;
        hal_vap_set_machw_txop_limit_bkbe(
            (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE),
            (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK));
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_TRUE) {
        /* HE STA在去关联之后，初始化txop_duration_rts_threshold mib，清除对应寄存器 */
        mac_mib_set_txop_duration_rts_threshold(hmac_vap, 0);
        hal_vap_set_txop_duration_threshold(0, 0);
    }
    hal_set_11ax_vap_idx(0);
#endif
#ifdef BOARD_ASIC_WIFI
    /* STA去关联时刷新MAC时钟 */
    hmac_vap_exit_update_mac_clk(hmac_vap);
#endif
    return OAL_SUCC;
}

/*****************************************************************************
函 数 名  : hmac_tx_switch_tx_queue
功能描述  : 交换两个虚假队列里的帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_tx_switch_tx_queue(oal_netbuf_queue_header_stru *fake_queue1,
    oal_netbuf_queue_header_stru *fake_queue2)
{
    osal_u8                       q_idx;
    oal_netbuf_head_stru          head_tmp;
    osal_u32 fake_queue1_pkt_num, fake_queue2_pkt_num;

    for (q_idx = 0; q_idx < HAL_TX_QUEUE_BUTT; q_idx++) {
        fake_queue1_pkt_num = fake_queue1[q_idx].mpdu_cnt;
        fake_queue2_pkt_num = fake_queue2[q_idx].mpdu_cnt;

        oal_netbuf_list_head_init(&head_tmp);
        oal_netbuf_splice_init(&fake_queue1[q_idx].buff_header, &head_tmp);
        oal_netbuf_splice_init(&fake_queue2[q_idx].buff_header, &fake_queue1[q_idx].buff_header);
        oal_netbuf_splice_init(&head_tmp, &fake_queue2[q_idx].buff_header);
        fake_queue1[q_idx].mpdu_cnt = fake_queue2_pkt_num;
        fake_queue2[q_idx].mpdu_cnt = fake_queue1_pkt_num;
    }

    return OAL_SUCC;
}

 /*****************************************************************************
 函 数 名  : hmac_same_channel_down_vap_handle_fakeq
 功能描述  : 同信道有一个去关联时fake队列的处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_same_channel_down_vap_handle_fakeq(const hal_to_dmac_device_stru *hal_device,
    hmac_vap_stru *down_vap, hmac_vap_stru *up_vap)
{
    osal_s8 q_num;
    oal_bool_enum_uint8 down_fakeq_empty;
    oal_bool_enum_uint8 up_fakeq_empty;
    const hmac_vap_stru *mac_dscr_vap = OSAL_NULL;
    oal_netbuf_queue_header_stru *fake_queue = OSAL_NULL;
    oal_netbuf_head_stru *head = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    unref_param(hal_device);
    up_fakeq_empty   = hmac_vap_is_fakeq_empty(up_vap);
    down_fakeq_empty = hmac_vap_is_fakeq_empty(down_vap);
    /* 如果都空,不需要后续处理 */
    if ((up_fakeq_empty == OSAL_TRUE) && (down_fakeq_empty == OSAL_TRUE)) {
        return;
    }

    /* 不可能存在两个fakeq都有dscr */
    if ((up_fakeq_empty != OSAL_TRUE) && (down_fakeq_empty != OSAL_TRUE)) {
        hmac_device_check_fake_queues_empty();
        return;
    }

    if (osal_unlikely(down_vap == OSAL_NULL)) {
        return;
    }

    mac_dscr_vap = (up_fakeq_empty != OSAL_TRUE) ? up_vap : down_vap;
    fake_queue = ((hmac_vap_stru *)mac_dscr_vap)->tx_dscr_queue_fake;

    oam_warning_log4(0, OAM_SF_SCAN,
        "{hmac_the_same_channel_down_switch_fakeq::up vap[%d]up fakeq empty[%d],down vap[%d]down fakeq empty[%d].}",
        up_vap->vap_id, up_fakeq_empty, down_vap->vap_id, down_fakeq_empty);

    /* 遍历6个发送队列 一定要先处理高优先级队列防止普通优先级队列发送完成产生管理帧入错队列 */
    for (q_num = HAL_TX_QUEUE_BUTT - 1; q_num >= 0; q_num--) {
        head = &fake_queue[(osal_u8)q_num].buff_header;
        for (netbuf = oal_netbuf_peek(head); netbuf != OSAL_NULL; netbuf = oal_netbuf_peek_next(netbuf, head)) {
            tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
            if (tx_ctl->tx_vap_index != down_vap->vap_id) {
                continue;
            }
            oal_netbuf_unlink(netbuf, head);
            oal_netbuf_free(netbuf);
            fake_queue[(osal_u8)q_num].mpdu_cnt--;
        }
    }

    /* 描述符都挂在down的vap上需要放到up的vap上 */
    if (mac_dscr_vap == down_vap) {
        hmac_vap_fake_queue_empty_assert(up_vap, THIS_FILE_ID);
        hmac_tx_switch_tx_queue(fake_queue, ((hmac_vap_stru *)(up_vap))->tx_dscr_queue_fake);
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_down_condition_notify
 功能描述  : vap down处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_down_condition_notify(const hmac_vap_stru * const up_vap, hmac_vap_stru *down_vap)
{
    hal_to_dmac_device_stru     *hal_device = OSAL_NULL;

    hmac_alg_vap_down_notify(down_vap);

    if (up_vap != OSAL_NULL) {
        /* 中间经过dbdc的处理,up vap如果在辅路此时已经切回主路重新获取hal device */
        hal_device = ((hmac_vap_stru *)down_vap)->hal_device;
        if (osal_unlikely(hal_device == OSAL_NULL)) {
            return;
        }
        /* 切换主信道至up vap所在的信道，使扫描能够正常回归主信道。
         * 存在不同信道vap时，说明在dbac场景下，dbac处理过程中能够正常切换信道 */
        if ((up_vap->channel.chan_number != hal_device->current_chan_number) ||
            (up_vap->channel.en_bandwidth != hal_device->wifi_channel_status.en_bandwidth)) {
            hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
            oam_warning_log4(0, OAM_SF_DBAC,
                "hmac_vap_down_condition_notify::has up vap,its bw[%d] chan[%d]!=hal_bw[%d] chan[%d],need switch chan",
                up_vap->channel.en_bandwidth,
                up_vap->channel.chan_number,
                hal_device->wifi_channel_status.en_bandwidth,
                hal_device->current_chan_number);

            hal_device->hal_scan_params.home_channel = up_vap->channel;
            hmac_scan_switch_channel_back(hmac_device, hal_device);
        }
    }
    /* 先down vap再去hmac_send_connect_result_to_dmac_sta,这里只能检查自己的fakeq */
    (osal_void)hmac_vap_fake_queue_empty_assert(down_vap, THIS_FILE_ID);
}


/*****************************************************************************
 函 数 名  : hmac_vap_down_notify
 功能描述  : vap down处理
*****************************************************************************/
osal_void hmac_vap_down_notify(hmac_vap_stru *down_vap)
{
    hmac_vap_stru                *up_vap = OSAL_NULL;
    hmac_device_stru             *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru      *hal_device = OSAL_NULL;

    if (osal_unlikely(down_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DBAC, "{hmac_vap_down_notify::down_vap NULL!}");
        return;
    }

    hal_device = ((hmac_vap_stru *)down_vap)->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_DBAC, "vap_id[%d] {hmac_vap_down_notify::hal_device NULL!}", down_vap->vap_id);
        return;
    }

    oam_warning_log4(0, OAM_SF_DBAC,
        "vap_id[%d] {hmac_vap_down_notify::vap[%d] vap mode[%d],p2p mode[%d]is down!!!}", down_vap->vap_id,
        down_vap->vap_id, down_vap->vap_mode, down_vap->p2p_mode);

    up_vap = mac_device_find_another_up_vap_etc(hmac_device, down_vap->vap_id);

    /* 扫描中退出DBAC 提前终止扫描 */
    /* 扫描中无论是否DBAC启动，都要先停止扫描，防止有VAP背景扫描时，
       有VAP进行去关联，使另一个VAP恢复up状态发送数据，此时数据会被扫描切换信道时全部释放，从而BA移窗卡死 */
    if ((mac_is_dbac_enabled(hmac_device) == OSAL_TRUE) && (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        oam_warning_log3(0, OAM_SF_SCAN,
            "vap_id[%d] hmac_vap_down_notify::device is scanning[%d],mode[%d]. stop scan", down_vap->vap_id,
            hmac_device->curr_scan_state, hmac_device->scan_params.scan_mode);

        if (up_vap != OSAL_NULL) {
            /* 同信道,提前剥离down vap的描述符 */
            if (mac_fcs_is_same_channel(&(down_vap->channel), &(up_vap->channel)) == OSAL_TRUE) {
                hmac_same_channel_down_vap_handle_fakeq(hal_device, down_vap, up_vap);
            }
        }

        hmac_scan_abort(hmac_device);
    }

    /* 清空down的vap的虚假队列 */
    hmac_vap_clear_fake_queue(down_vap);

    hmac_vap_down_condition_notify(up_vap, down_vap);
}

/*****************************************************************************
 函 数 名  : hmac_one_packet_send_null_data
 功能描述  : 通过one packet机制发送null报文
*****************************************************************************/
OSAL_STATIC osal_void  hmac_one_packet_send_null_data(hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 ps)
{
    mac_fcs_cfg_stru fcs_cfg;
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;

    unref_param(hmac_device);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    (osal_void)memset_s(&fcs_cfg, sizeof(mac_fcs_cfg_stru), 0, sizeof(mac_fcs_cfg_stru));

    fcs_cfg.vap_id = hmac_vap->vap_id;
    /* 20表示保护时间 */
    hmac_fcs_prepare_one_packet_cfg(hmac_vap, &(fcs_cfg.one_packet_cfg), 20);

    /* 准备报文 */
    if (fcs_cfg.one_packet_cfg.protect_type == HAL_FCS_PROTECT_TYPE_NULL_DATA) {
        mac_header = (mac_ieee80211_frame_stru *)fcs_cfg.one_packet_cfg.protect_frame;
        mac_header->frame_control.power_mgmt = ps;
    }

    hmac_fcs_start_send_one_packet(&fcs_cfg);
}
#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 函 数 名  : hmac_vap_restart_dbac
 功能描述  : 通过某up的vap重启dbac
*****************************************************************************/
osal_void hmac_vap_restart_dbac(hmac_vap_stru  *hmac_vap)
{
    hmac_device_stru         *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru            *another_mac_vap;
    osal_u8                   another_mac_vap_id;
    osal_u32                  ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    if (mac_is_dbac_running(hmac_device) == OSAL_FALSE) {
        oam_info_log3(0, 0, "vap_id[%d]{hmac_vap_restart_dbac::not running enable[%d] running[%d]!}",
            hmac_vap->vap_id, hmac_device->en_dbac_enabled, hmac_device->en_dbac_running);
        return;
    }

    ret  = hal_device_find_another_up_vap(hmac_vap->hal_device, hmac_vap->vap_id, &another_mac_vap_id);
    if (ret != OAL_SUCC) {
        oam_info_log2(0, 0, "vap_id[%d]{hmac_vap_restart_dbac::find_another_up_vap fail[%d]!}", hmac_vap->vap_id, ret);
        return;
    }

    another_mac_vap = mac_res_get_hmac_vap(another_mac_vap_id);
    if (another_mac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_DBAC, "{hmac_vap_restart_dbac:: get_mac_vap[%d] fail!}", another_mac_vap_id);
        return;
    }

    oam_warning_log2(0, OAM_SF_DBAC, "{hmac_vap_restart_dbac::now vap[%d]another vap[%d]restart dbac!!!}",
        hmac_vap->vap_id, another_mac_vap_id);

    /* 通知算法DBAC/DBDC */
    hmac_alg_vap_down_notify(hmac_vap);
    hmac_alg_vap_up_notify(hmac_vap);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_get_user_and_send_null_data
 功能描述  : 根据vap id获取用户信息.如果节电状态为active,则发送null报文
*****************************************************************************/
static osal_u32 hmac_get_user_and_send_null_data(hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    const mac_sta_pm_handler_stru *sta_pm_handler = OSAL_NULL;

    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_DBAC,
            "{hmac_get_user_and_send_null_data::get_dmac_user failed! assoc_vap_id=%d,vap_mode:%d}",
            hmac_vap->assoc_vap_id, hmac_vap->vap_mode);
#ifdef _PRE_WLAN_FEATURE_P2P
        oam_warning_log1(0, OAM_SF_DBAC, "{p2p_mode:%d}", hmac_get_p2p_mode_etc(hmac_vap));
#endif
        return OAL_FAIL;
    }

    sta_pm_handler = &hmac_vap->sta_pm_handler;
    if (sta_pm_handler->oal_fsm.cur_state == STA_PWR_SAVE_STATE_ACTIVE) {
        hmac_one_packet_send_null_data(hmac_device, hmac_vap, OSAL_FALSE);
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_vap_resume_tx_by_chl
 功能描述  : 切信道后，恢复目的信道上VAP的发送,这里负责虚假队列包搬移到硬件队列!!!
*****************************************************************************/
osal_void  hmac_vap_resume_tx_by_chl(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device,
    mac_channel_stru *dst_channel)
{
    osal_s32 ret;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    osal_u8 vap_idx;
    frw_msg msg_info = {0};
    osal_void *fhook = OAL_PTR_NULL;

    /* 只看自己的hal device上找 */
    osal_u8 up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_SCAN, "{hmac_vap_resume_tx_by_chl::hmac_vap is null.}");
            continue;
        }

        if (mac_fcs_is_same_channel(&hmac_vap->channel, dst_channel) != OSAL_TRUE) {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_ROAM
        /* 当回到漫游vap所在的信道时，不需要切vap的状态。 */
        if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
            ret = (osal_s32)hmac_get_user_and_send_null_data(hmac_device, hmac_vap);
            if (ret != OAL_CONTINUE) {
                return;
            }
            continue;
        }
#endif // _PRE_WLAN_FEATURE_ROAM
        /* 存在进入这个函数,信道相同,找到的vap不是pause的吗? */
        if (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) {
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_VAP_RESUME_TX_BY_CHL);
            if (fhook != OSAL_NULL) {
                ((hmac_btcoex_vap_resume_tx_by_chl_cb)fhook)(hmac_vap, hal_device);
            }
        }
        /* 针对p2p设备，不做屏蔽 */
        if ((hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) && (hmac_vap->p2p_mode == WLAN_LEGACY_VAP_MODE)) {
            continue;
        }
        /* 判断目的信道上的vap模式, 如果是sta则需要发节能位置0的null data */
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            ret = (osal_s32)hmac_get_user_and_send_null_data(hmac_device, hmac_vap);
            if (ret != OAL_CONTINUE) {
                return;
            }
        }

        // 先恢复队列，然后开启软件队列发包, MSG_QUEUE_CFG
        frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg_info);
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_RESTORE_HW_QUEUE, &msg_info, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_vap_resume_tx_by_chl::frw_send_msg_to_device1 failed[%d].}", ret);
        }

        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
        if (hmac_vap->hal_vap != OSAL_NULL) {
            hal_vap_beacon_resume(hmac_vap->hal_vap);
        }
        /* 恢复device侧调度 */
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_RESUME_TX, &msg_info, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_vap_resume_tx_by_chl::frw_send_msg_to_device1 failed[%d].}", ret);
        }
    }

    hmac_tx_schedule(hal_device, WLAN_WME_AC_BE);
}

/*****************************************************************************
 函 数 名  : hmac_vap_update_snr_info
 功能描述  : 更新vap下snr信息
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_vap_update_snr_info(hmac_vap_stru *hmac_vap,
    const dmac_rx_ctl_stru * const rx_ctrl)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    /* AMPDU最后一个mpdu才更新snr信息，否则上报值为0 */
    if (rx_ctrl->rx_status.last_mpdu_flag == OSAL_TRUE) {
        hmac_vap->query_stats.snr_ant0 = (osal_char)rx_ctrl->rx_statistic.snr_ant0;
        hmac_vap->query_stats.snr_ant1 = (osal_char)rx_ctrl->rx_statistic.snr_ant1;
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_work_set_channel
 功能描述  : 设置信道
*****************************************************************************/
osal_void hmac_vap_work_set_channel(hmac_vap_stru *hmac_vap)
{
    hmac_vap_stru    *vap_up = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_channel_stru channel;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_device_find_up_sta_etc(hmac_device, &vap_up);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 如果有处在UP状态的VAP，此次入网切信道需要发保护帧 */
        vap_up = mac_device_find_another_up_vap_etc(hmac_device, hmac_vap->vap_id);
    } else {
        oam_error_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_vap_work_set_channel::error mac vap mode [%d].}", hmac_vap->vap_id, hmac_vap->vap_mode);
        return;
    }
    hmac_alg_cfg_channel_notify(hmac_vap, CH_BW_CHG_TYPE_MOVE_WORK);
    /* 多个VAP情况下 根据实际最大的带宽设置硬件带宽 */
    channel = hmac_vap->channel;
    hmac_chan_select_real_channel(hmac_device, &channel, channel.chan_number);
    hmac_mgmt_connect_set_channel(hmac_device, hmac_vap->hal_device, vap_up, &channel);
    /* 初始化发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_INIT);
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_attach
 功能描述  : bw switch fsm 状态机处理函数注册
*****************************************************************************/
osal_void hmac_sta_bw_switch_fsm_attach(hmac_vap_stru *hmac_vap)
{
    hmac_sta_bw_switch_fsm_info_stru       *handler      = OSAL_NULL;
    osal_u8                               fsm_name[16] = {0}; /* 16表示数组的大小 */
    osal_u32                              ret;
    oal_fsm_create_stru fsm_create_stru;

    hmac_vap->sta_bw_switch_fsm = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(hmac_sta_bw_switch_fsm_info_stru), OAL_TRUE);
    if (hmac_vap->sta_bw_switch_fsm == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_bw_switch_fsm_attach::sta_bw_switch_fsm null.}", hmac_vap->vap_id);
        return;
    }
    (osal_void)memset_s(hmac_vap->sta_bw_switch_fsm, sizeof(hmac_sta_bw_switch_fsm_info_stru),
        0, sizeof(hmac_sta_bw_switch_fsm_info_stru));
    handler = (hmac_sta_bw_switch_fsm_info_stru *)hmac_vap->sta_bw_switch_fsm;

    /* 准备一个唯一的fsmname */
    fsm_name[0] = 0; /* fsm_name的第0位赋值 core_id固定为0 */
    fsm_name[1] = 0; /* fsm_name的第1位赋值,chip_id固定为0 */
    fsm_name[2] = 0;  /* fsm_name的第2位赋值 device_id固定为0 */
    fsm_name[3] = hmac_vap->vap_id; /* fsm_name的第3位赋值 */
    fsm_name[4] = hmac_vap->vap_mode; /* fsm_name的第4位赋值 */
    fsm_name[5] = 'B'; /* fsm_name的第5位赋值 */
    fsm_name[6] = 'W'; /* fsm_name的第6位赋值 */

    fsm_create_stru.oshandle = hmac_vap;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = hmac_vap;

    ret = oal_fsm_create(&fsm_create_stru,
                         &(handler->oal_fsm),
                         HMAC_STA_BW_SWITCH_FSM_INIT,
                         g_sta_bw_switch_fsm_info,
                         sizeof(g_sta_bw_switch_fsm_info) / sizeof(oal_fsm_state_info));
    if (ret != OAL_SUCC) {
        handler->is_fsm_attached = OSAL_FALSE;
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] hmac_sta_bw_switch_fsm_attach::oal_fsm_create fail.", hmac_vap->vap_id);
        return;
    }

    handler->is_fsm_attached     = OSAL_TRUE;
    oam_warning_log3(0, OAM_SF_ASSOC,
        "vap_id[%d] hmac_sta_bw_switch_fsm_attach::vap_mode=%d p2p_mode=%d attached succ.",
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->p2p_mode);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_detach
 功能描述  : bw switch fsm 状态机处理函数去注册
*****************************************************************************/
osal_void hmac_sta_bw_switch_fsm_detach(hmac_vap_stru *hmac_vap)
{
    hmac_sta_bw_switch_fsm_info_stru         *bw_fsm    = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)hmac_vap->sta_bw_switch_fsm;
    if (bw_fsm == OSAL_NULL) {
        return;
    }

    if (bw_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_bw_switch_fsm_detach::bw_fsm not attatched}", hmac_vap->vap_id);
        return;
    }

    oal_mem_free(bw_fsm, OAL_TRUE);
    hmac_vap->sta_bw_switch_fsm = OSAL_NULL;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_need_new_verify
 功能描述  : normal状态的event接口
*****************************************************************************/
osal_u8 hmac_sta_bw_switch_need_new_verify(const hmac_vap_stru *hmac_vap, wlan_bw_cap_enum_uint8  bw_becaon_new)
{
    wlan_bw_cap_enum_uint8 vap_bw_cap;

    if (osal_unlikely(hmac_vap == OSAL_NULL) || is_legacy_sta(hmac_vap) == OSAL_FALSE) {
        return HMAC_STA_BW_VERIFY_SWITCH_BUTT;
    }

    if (mac_vap_get_curren_bw_state(hmac_vap) == HMAC_STA_BW_SWITCH_FSM_NORMAL) {
        hmac_vap_get_bandwidth_cap_etc(hmac_vap, &vap_bw_cap);

        if ((vap_bw_cap == WLAN_BW_CAP_20M) && (bw_becaon_new == WLAN_BW_CAP_40M)) {
            return HMAC_STA_BW_VERIFY_20M_TO_40M;
        } else if ((vap_bw_cap == WLAN_BW_CAP_40M) && (bw_becaon_new == WLAN_BW_CAP_20M)) {
            return HMAC_STA_BW_VERIFY_40M_TO_20M;
        }
    } else if (mac_vap_get_curren_bw_state(hmac_vap) == HMAC_STA_BW_SWITCH_FSM_VERIFY20M) {
        if (bw_becaon_new == WLAN_BW_CAP_40M) {
            return HMAC_STA_BW_VERIFY_20M_TO_40M;
        }
    } else if (mac_vap_get_curren_bw_state(hmac_vap) == HMAC_STA_BW_SWITCH_FSM_VERIFY40M) {
        if (bw_becaon_new == WLAN_BW_CAP_20M) {
            return HMAC_STA_BW_VERIFY_40M_TO_20M;
        }
    }

    return HMAC_STA_BW_VERIFY_SWITCH_BUTT;
}

/*****************************************************************************
 函 数 名  : hmac_bw_fsm_trans_to_state
 功能描述  : bw状态机状态切换函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bw_fsm_trans_to_state(hmac_sta_bw_switch_fsm_info_stru *bw_fsm, osal_u8 state)
{
    oal_fsm_stru   *oal_fsm = OSAL_NULL;

    if (osal_unlikely(bw_fsm == OSAL_NULL)) {
        return OAL_FAIL;
    }

    oal_fsm = &(bw_fsm->oal_fsm);
    oam_warning_log2(0, OAM_SF_ASSOC, "hmac_bw_fsm_trans_to_state:trans from %d to %d",
                     oal_fsm->cur_state, state);

    return oal_fsm_trans_to_state(oal_fsm, state);
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_init
 功能描述  : 状态机下的统计信息清空
*****************************************************************************/
osal_void hmac_sta_bw_switch_fsm_init(const hmac_vap_stru * const hmac_vap)
{
    hmac_sta_bw_switch_fsm_info_stru *sta_bw_switch_fsm;  /* 带宽切换状态机 */

    sta_bw_switch_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);
    sta_bw_switch_fsm->verify_fail_cnt_20m = 0;
    sta_bw_switch_fsm->verify_fail_cnt_40m = 0;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_init_entry
 功能描述  : 状态机下的统计信息清空
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_bw_switch_fsm_init_entry(osal_void *ctx)
{
    hmac_vap_stru                    *hmac_vap = (hmac_vap_stru *)ctx;

    hmac_sta_bw_switch_fsm_init(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_normal_entry
 功能描述  : 状态机下的统计信息清空
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_bw_switch_fsm_normal_entry(osal_void *ctx)
{
    hmac_vap_stru                    *hmac_vap = (hmac_vap_stru *)ctx;

    hmac_sta_bw_switch_fsm_init(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_init_event
 功能描述  : init状态的event接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_init_event(osal_void  *ctx, osal_u16  event,
    osal_u16 event_data_len,  osal_void  * const event_data)
{
    hmac_vap_stru   *hmac_vap  = (hmac_vap_stru *)ctx;
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm = OSAL_NULL;

    unref_param(event_data_len);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);
    if (event_data == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_bw_switch_fsm_init_event::event_data == NULL but no use!}");
    }
    switch (event) {
        case HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        default:
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] hmac_sta_bw_switch_fsm_init_event::get invalid event[%d]",
                hmac_vap->vap_id, event);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_normal_event
 功能描述  : normal状态的event接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_normal_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void * const event_data)
{
    hmac_vap_stru   *hmac_vap  = (hmac_vap_stru *)ctx;
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);

    unref_param(event_data_len);
    if (event_data == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_bw_switch_fsm_normal_event::event_data == NULL but no use!}");
    }
    switch (event) {
        case HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] hmac_sta_bw_switch_fsm_normal_event::event[%d] received!",
                hmac_vap->vap_id, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            hmac_sta_bw_switch_fsm_init(hmac_vap);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_BEACON_20M:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_VERIFY20M);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_BEACON_40M:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_VERIFY40M);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_rx_assemble_to_bandwith
 功能描述  : 20M/40M校验数据帧接收上报工作带宽转换
*****************************************************************************/
OSAL_STATIC wlan_bw_cap_enum_uint8 hmac_sta_bw_rx_assemble_to_bandwith(hal_channel_assemble_enum_uint8 bw)
{
    if (bw == WLAN_BAND_ASSEMBLE_20M) {
        return WLAN_BW_CAP_20M;
    } else if (bw == WLAN_BAND_ASSEMBLE_40M || bw == WLAN_BAND_ASSEMBLE_40M_DUP) {
        return WLAN_BW_CAP_40M;
    }

    return WLAN_BW_CAP_BUTT;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_verify_20m_event
 功能描述  : verify_20m状态的event接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_verify_20m_event(osal_void  *ctx, osal_u16  event,
    osal_u16 event_data_len,  osal_void  *event_data)
{
    hmac_vap_stru                    *hmac_vap  = (hmac_vap_stru *)ctx;
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);

    switch (event) {
        case HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            oam_warning_log2(0, 0, "vap_id[%d] hmac_sta_bw_switch_fsm_verify_20m_event::event[%d] received!",
                hmac_vap->vap_id, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_BEACON_40M:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_VERIFY40M);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE:
            if (event_data && (event_data_len == sizeof(dmac_rx_ctl_stru))) {
                dmac_rx_ctl_stru *rx_ctl  = (dmac_rx_ctl_stru *)event_data;
                osal_u8 bw = (osal_u8)(rx_ctl->rx_status.freq_bandwidth_mode & 0x0F);
                hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(
                    rx_ctl->rx_info.ta_user_idx);
                if (hmac_user == OSAL_NULL) {
                    oam_warning_log2(0, OAM_SF_RX,
                        "vap_id[%d] {hmac_sta_bw_switch_fsm_verify_20m_event::hmac_user[%d] null.}",
                        hmac_vap->vap_id, rx_ctl->rx_info.ta_user_idx);
                    return OAL_ERR_CODE_PTR_NULL;
                }

                if (hmac_sta_bw_rx_assemble_to_bandwith(bw) == WLAN_BW_CAP_20M) {
                    hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_NORMAL);
                    hmac_sta_set_bandwith_handler(hmac_vap, WLAN_BW_CAP_20M);
                } else {
                    bw_fsm->verify_fail_cnt_20m++;
                }

                if (bw_fsm->verify_fail_cnt_20m > HMAC_BW_VERIFY_MAX_THRESHOLD) {
                    oam_warning_log0(0, OAM_SF_ASSOC,
                        "hmac_sta_bw_switch_fsm_verify_20m_event::verify counts outnumbers threshold caused invalid!");
                    hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INVALID);
                }
            }
            break;

        case HMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] hmac_sta_bw_switch_fsm_verify_20m_event::get invalid event[%d]",
                hmac_vap->vap_id, event);
            break;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_verify_40m_proc(osal_u16 event_data_len, osal_void *event_data,
    hmac_vap_stru *hmac_vap, hmac_sta_bw_switch_fsm_info_stru *bw_fsm)
{
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)event_data;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 bw;

    if (event_data == OSAL_NULL || event_data_len != sizeof(dmac_rx_ctl_stru)) {
        return OAL_FAIL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(rx_ctl->rx_info.ta_user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_RX, "vap[%d] hmac_sta_bw_switch_fsm_verify_40m_proc:hmac_user[%d] null",
            hmac_vap->vap_id, rx_ctl->rx_info.ta_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(&rx_ctl->rx_info));
    if (ether_is_multicast(frame_hdr->address1) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    bw = (osal_u8)(rx_ctl->rx_status.freq_bandwidth_mode & 0x0F);
    if (hmac_sta_bw_rx_assemble_to_bandwith(bw) == WLAN_BW_CAP_40M) {
        hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_NORMAL);
        hmac_user_set_bandwith_handler(hmac_vap, hmac_user, WLAN_BW_CAP_40M);
    } else {
        bw_fsm->verify_fail_cnt_40m++;
    }

    if (bw_fsm->verify_fail_cnt_40m > HMAC_BW_VERIFY_MAX_THRESHOLD) {
        oam_warning_log0(0, OAM_SF_ASSOC,
            "hmac_sta_bw_switch_fsm_verify_40m_proc::verify counts outnumbers threshold caused invalid!");
        hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INVALID);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_verify_40m_event
 功能描述  : verify_40m状态的event接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_verify_40m_event(osal_void  *ctx, osal_u16  event,
    osal_u16 event_data_len,  osal_void  *event_data)
{
    hmac_vap_stru                    *hmac_vap  = (hmac_vap_stru *)ctx;
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);

    switch (event) {
        case HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] hmac_sta_bw_switch_fsm_verify_40m_event::event[%d] received!",
                hmac_vap->vap_id, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_BEACON_20M:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_VERIFY20M);
            break;

        case HMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE:
            if (hmac_sta_bw_switch_fsm_verify_40m_proc(event_data_len, event_data, hmac_vap, bw_fsm) != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;

        case HMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] hmac_sta_bw_switch_fsm_verify_40m_event::get invalid event[%d]",
                hmac_vap->vap_id, event);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_init_event
 功能描述  : init状态的event接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_bw_switch_fsm_invalid_event(osal_void  *ctx, osal_u16  event,
    osal_u16 event_data_len,  osal_void *event_data)
{
    hmac_vap_stru   *hmac_vap  = (hmac_vap_stru *)ctx;
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);

    unref_param(event_data_len);
    if (event_data == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_bw_switch_fsm_invalid_event::event_data == NULL but no use!}");
    }
    switch (event) {
        case HMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            hmac_bw_fsm_trans_to_state(bw_fsm, HMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_flush_rate
 功能描述  : 刷新dmac_vap速率信息
*****************************************************************************/
osal_void hmac_vap_flush_rate(hmac_vap_stru *hmac_vap, const hmac_set_rate_stru * const rate)
{
    hmac_set_rate_stru *vap_rate;
    vap_rate = &(hmac_vap->vap_curr_rate);
    /* 保证probe_req帧为1M速率发送 */
    if (hmac_vap_init_rate_info(vap_rate) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_vap_flush_rate::hmac_vap_init_rate_info err!}");
    }

    if (hmac_vap_init_tx_frame_params(hmac_vap, OSAL_TRUE) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_vap_flush_rate::hmac_vap_init_tx_frame_params err!}");
    }
    /* 赋值11b对应的速率 */
    if (rate->capable_flag.capable.support_11b == OSAL_TRUE) {
        vap_rate->min_rate[WLAN_11B_PHY_PROTOCOL_MODE] = rate->min_rate[WLAN_11B_PHY_PROTOCOL_MODE];
    }
    /* 赋值11ag对应的速率 */
    if (rate->capable_flag.capable.support_11ag == OSAL_TRUE) {
        vap_rate->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE] = rate->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE];
    }
    vap_rate->capable_flag.value = rate->capable_flag.value;

    oam_warning_log2(0, OAM_SF_ANY,
        "vap_id[%d] {hmac_vap_flush_rate::vap_rate->un_capable_flag.value[%d]}", hmac_vap->vap_id,
        vap_rate->capable_flag.value);
}

/*****************************************************************************
 函 数 名  : mac_vap_pause_tx
 功能描述  : 暂停VAP, 置vap状态为pause，期望调度判断此状态: 如果为pause则停止调度
*****************************************************************************/
osal_void  mac_vap_pause_tx(hmac_vap_stru *hmac_vap)
{
    if (osal_likely(hmac_vap != OSAL_NULL)) {
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_PAUSE);
    }
}

/*****************************************************************************
 功能描述  : 虚假队列不为空时打印error
*****************************************************************************/
osal_u8 hmac_vap_fake_queue_empty_assert(hmac_vap_stru *hmac_vap, osal_u32 file_id)
{
    oal_netbuf_stru               *netbuf = OSAL_NULL;
    mac_tx_ctl_stru               *tx_ctl = OSAL_NULL;
    oal_netbuf_queue_header_stru  *fake_queue = OSAL_NULL;
    osal_u8                         ret = OSAL_TRUE;
    osal_u8 q_idx;
    adjust_hso_param_stru adjust_hso_param = {0};

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }
    fake_queue = hmac_vap->tx_dscr_queue_fake;
    for (q_idx = 0; q_idx < HAL_TX_QUEUE_BUTT; q_idx++) {
        if (oal_netbuf_list_empty(&fake_queue[q_idx].buff_header) == OSAL_FALSE) {
            /* 虚假队列不为空，将第一个帧通过OTA上报上来 */
            netbuf = (oal_netbuf_stru *)fake_queue[q_idx].buff_header.next;
            tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
            adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME;
            adjust_hso_param.channel = hmac_vap->channel.chan_number;
            dft_report_80211_frame((osal_u8*)mac_get_frame_header_addr(tx_ctl), tx_ctl->frame_header_length,
                (osal_u8*)oal_netbuf_tx_data(netbuf), tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length,
                &adjust_hso_param);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
            hmac_sniffer_80211_frame((osal_u8*)mac_get_frame_header_addr(tx_ctl), tx_ctl->frame_header_length,
                (osal_u8*)oal_netbuf_tx_data(netbuf), tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length,
                SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME);
#endif

            oam_error_log3(0, OAM_SF_DBAC, "fake q is not empty.hmac_vap id[%d],ac:%d, file_id:%d",
                hmac_vap->vap_id, q_idx, file_id);
            ret = OSAL_FALSE;
        }
    }
    return ret;
}

/*****************************************************************************
 功能描述  : 清除虚假缓存队列
*****************************************************************************/
osal_void hmac_vap_clear_fake_queue(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_queue_header_stru *fake_queue = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_s8 q_id;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    fake_queue = hmac_vap->tx_dscr_queue_fake;
    /* 遍历6个发送队列 一定要先处理高优先级队列防止普通优先级队列发送完成产生管理帧入错队列 */
    for (q_id = HAL_TX_QUEUE_BUTT - 1; q_id >= 0; q_id--) {
        while (oal_netbuf_list_empty(&fake_queue[(osal_u8)q_id].buff_header) == OSAL_FALSE) {
            netbuf = oal_netbuf_delist(&fake_queue[(osal_u8)q_id].buff_header);
            if (netbuf == OSAL_NULL) {
                oam_error_log2(0, OAM_SF_DBAC, "{vap_id[%d] hmac_vap_clear_fake_queue: netbuf is null q_id[%d] }",
                    hmac_vap->vap_id, q_id);
                break;
            }
            oal_netbuf_free(netbuf);
        }
        fake_queue[(osal_u8)q_id].mpdu_cnt = 0;
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_vap_get_fake_queue_pkt_nums(osal_void)
{
    osal_s8 q_id;
    osal_u32 mpdu_cnts = 0;
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
            oam_warning_log1(0, 0, "hmac_vap_get_fake_queue_pkt_nums vap[%d] null", hmac_device->vap_id[vap_idx]);
            continue;
        }

        for (q_id = HAL_TX_QUEUE_BUTT - 1; q_id >= 0; q_id--) {
            if (oal_netbuf_list_empty(&hmac_vap->tx_dscr_queue_fake[(osal_u8)q_id].buff_header) == OSAL_FALSE) {
                mpdu_cnts += hmac_vap->tx_dscr_queue_fake[(osal_u8)q_id].mpdu_cnt;
            }
        }
    }

    return mpdu_cnts;
}

/*****************************************************************************
 功能描述  : 发送虚假缓存报文
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_vap_send_fake_queue(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_queue_header_stru *fake_queue = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_s8 q_id;
    osal_u16 len;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    fake_queue = hmac_vap->tx_dscr_queue_fake;
    for (q_id = HAL_TX_QUEUE_BUTT - 1; q_id >= 0; q_id--) {
        while (oal_netbuf_list_empty(&fake_queue[(osal_u8)q_id].buff_header) == OSAL_FALSE) {
            netbuf = oal_netbuf_delist(&fake_queue[(osal_u8)q_id].buff_header);
            if (netbuf == OSAL_NULL) {
                oam_error_log2(0, OAM_SF_DBAC, "{vap_id[%d] hmac_vap_send_fake_queue: netbuf is null q_id[%d] }",
                    hmac_vap->vap_id, q_id);
                break;
            }
            fake_queue[(osal_u8)q_id].mpdu_cnt--;

            tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
            len = tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length;
            tx_ctl->is_fake_buffer = OSAL_TRUE;

            ret = hmac_tx_mgmt(hmac_vap, netbuf, len, OSAL_TRUE);
            if (ret != OAL_SUCC) {
                oal_netbuf_free(netbuf);
            }
        }
    }
}

/*****************************************************************************
 功能描述  : 发送所有vap下的虚假缓存报文
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_vap_send_all_fake_queue(osal_void)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            return;
        }

        if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
            continue;
        }
#ifdef _PRE_WLAN_FEATURE_DBAC
        /* DBAC/DBDC运行时判断管理帧是否发送 */
        if (mac_is_dbac_running(hmac_device) && alg_dbac_hmac_is_vap_stop(hmac_vap->hal_vap->vap_id) == OSAL_TRUE) {
            oam_warning_log1(0, 0, "hmac_vap_send_all_fake_queue:vap %d pkt buffered again", hmac_vap->vap_id);
            continue;
        }
#endif
        hmac_vap_send_fake_queue(hmac_vap);
    }
}

/*****************************************************************************
 功能描述  : 刷新 vap pause bitmap
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_vap_refresh_tx_bitmap(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    if (mac_is_dbac_running(hmac_device)) {
        hmac_device->vap_pause_bitmap = hal_gp_get_dbac_vap_stop_bitmap();
    } else {
        hmac_device->vap_pause_bitmap = 0;
    }
}

/*****************************************************************************
 功能描述  : 判断此vap是否pause
*****************************************************************************/
WIFI_TCM_TEXT oal_bool_enum_uint8 hmac_vap_is_pause_tx_bitmap(hmac_vap_stru *hmac_vap)
{
    osal_u8 hal_vap_id;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u32 bitmap = hmac_device->vap_pause_bitmap;
    if (hmac_vap == OSAL_NULL) {
        return OAL_FALSE;
    }
    hal_vap_id = hmac_vap->hal_vap->vap_id;
    if (hal_vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log1(0, OAM_SF_DBAC, "{vap id[%d] hmac_vap_is_pause_tx_bitmap fail}", hal_vap_id);
        return OAL_TRUE;
    }

    if ((bitmap & osal_bit(hal_vap_id)) != 0) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_vap_pause_tx
 功能描述  : 暂停此VAP的发送
*****************************************************************************/
osal_void  hmac_vap_pause_tx(hmac_vap_stru *hmac_vap)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
        return;
    }

    mac_vap_pause_tx(hmac_vap);
    if (hmac_vap->hal_vap != OSAL_NULL) {
        hal_vap_beacon_suspend(hmac_vap->hal_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_is_fakeq_empty
 功能描述  : 获取某个fakeq是否空
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_vap_is_fakeq_empty(hmac_vap_stru *hmac_vap)
{
    osal_u8                       q_idx;
    oal_netbuf_queue_header_stru *fake_queue;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    fake_queue = hmac_vap->tx_dscr_queue_fake;
    for (q_idx = 0; q_idx < HAL_TX_QUEUE_BUTT; q_idx++) {
        if (oal_netbuf_list_empty(&fake_queue[q_idx].buff_header) == OSAL_FALSE) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_vap_is_in_p2p_listen
 功能描述  : DMAC判断是否是在p2p listen状态
*****************************************************************************/
osal_u32  hmac_vap_is_in_p2p_listen(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (hmac_device->scan_params.scan_func == MAC_SCAN_FUNC_P2P_LISTEN)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_bw_switch_fsm_post_event
 功能描述  : 触发状态机事件处理函数
*****************************************************************************/
osal_u32 hmac_sta_bw_switch_fsm_post_event(const hmac_vap_stru *hmac_vap, osal_u16 type, osal_u16 datalen,
    osal_u8 *data)
{
    hmac_sta_bw_switch_fsm_info_stru *handler      = OSAL_NULL;
    osal_u32                        ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }
    handler = (hmac_sta_bw_switch_fsm_info_stru *)(hmac_vap->sta_bw_switch_fsm);

    if (osal_unlikely((type >= HMAC_STA_BW_SWITCH_EVENT_BUTT) || (handler == OSAL_NULL))) {
        oam_error_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] hmac_sta_bw_switch_fsm_post_event:: event type[%d] error!",
            hmac_vap->vap_id, type);
        return OAL_FAIL;
    }

    if (handler->is_fsm_attached == OSAL_FALSE) {
        oam_warning_log2(0, OAM_SF_ASSOC,
                         "vap_id[%d] hmac_sta_bw_switch_fsm_post_event::bw_fsm_attached = %d!",
                         hmac_vap->vap_id, handler->is_fsm_attached);
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(handler->oal_fsm), type, datalen, data);

    return ret;
}

/*****************************************************************************
函 数 名  : hmac_sta_bw_fsm_get_current_state
功能描述  :获取当前状态
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u8 hmac_sta_bw_fsm_get_current_state(const hmac_vap_stru *hmac_vap)
{
    hmac_sta_bw_switch_fsm_info_stru *bw_fsm_handler = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return HMAC_STA_BW_SWITCH_FSM_BUTT;
    }

    bw_fsm_handler = (hmac_sta_bw_switch_fsm_info_stru *)hmac_vap->sta_bw_switch_fsm;
    if (bw_fsm_handler == OSAL_NULL) {
        return HMAC_STA_BW_SWITCH_FSM_BUTT;
    }

    return bw_fsm_handler->oal_fsm.cur_state;
}

#ifdef _PRE_WLAN_DFR_STAT
/*****************************************************************************
 函 数 名  : hmac_start_self_healing_period_check_task
 功能描述  : 设置自愈周期检测任务开启
 输入参数  : oal_net_device_stru *net_dev 网络接口
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_start_self_healing_period_check_task(hmac_vap_stru  *hmac_vap)
{
    osal_s32 ret;
    frw_msg msg_info;
    period_check_cfg period_check_param;

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    period_check_param.switch_enable = hwifi_get_self_healing_enable_etc();
    period_check_param.timeout = hwifi_get_self_healing_period_etc();
    period_check_param.cnt_period = hwifi_get_self_healing_cnt_etc();

    msg_info.data = (osal_u8 *)&period_check_param;
    msg_info.data_len = OAL_SIZEOF(period_check_param);

    ret = wal_async_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_PERIOD_CHECK_TASK, (osal_u8 *)&msg_info,
        OAL_SIZEOF(msg_info), FRW_POST_PRI_HIGH);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_start_self_healing_period_check_task::Start_vap failed[%d].}",
                         hmac_vap->vap_id, ret);
    }
    return (osal_u32)ret;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_start_vap_event_etc
 功能描述  : 抛start vap事件
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_config_start_vap_event_etc(hmac_vap_stru    *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag)
{
    osal_u32                    ul_ret;

    ul_ret = (osal_u32)hmac_config_start_vap(hmac_vap, mgmt_rate_init_flag);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap_event_etc::Start_vap failed[%d].}",
                         hmac_vap->vap_id, ul_ret);
    }
#ifdef _PRE_WLAN_DFR_STAT
    if ((ul_ret == OAL_SUCC) && (hwifi_get_self_healing_enable_etc() != 0)) {
        /* 确认启动自愈流程 */
        ul_ret = hmac_start_self_healing_period_check_task(hmac_vap);
    }
#endif
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_START_VAP, hmac_vap);
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_normal_check_legacy_vap_num
 功能描述  : 添加vap时检查现有传统模式（非proxy sta）下vap num是否符合要求
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32 hmac_normal_check_legacy_vap_num(hmac_device_stru *hmac_device,
    wlan_vap_mode_enum_uint8 vap_mode)
{
    /* VAP个数判断 */
    if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if ((hmac_device->sta_num == WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) &&
            (hmac_device->vap_num == WLAN_AP_STA_COEXIST_VAP_NUM)) {
            /* AP STA共存场景，只能创建4个AP + 1个STA */
            oam_warning_log0(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 4AP + 1STA, cannot create another AP.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        if ((hmac_device->vap_num - hmac_device->sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            oam_warning_log2(0, OAM_SF_CFG,
                "{hmac_normal_check_legacy_vap_num::ap num exceeds the supported spec,vap_num[%u],sta_num[%u].}",
                hmac_device->vap_num, hmac_device->sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_device->sta_num >= WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE) {
            /* 已创建的STA个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 2+ AP.can not create STA any more[%d].}",
                             hmac_device->sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_normal_check_vap_num
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32  hmac_config_normal_check_vap_num(hmac_device_stru *hmac_device,
    hmac_cfg_add_vap_param_stru *param)
{
    wlan_vap_mode_enum_uint8   vap_mode;

#ifdef _PRE_WLAN_FEATURE_P2P
    if ((is_p2p_mode(param->add_vap.p2p_mode)) && (hmac_device->st_p2p_info.p2p_mode != 0)) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
#endif

    vap_mode = param->add_vap.vap_mode;
    return hmac_normal_check_legacy_vap_num(hmac_device, vap_mode);
}

/*****************************************************************************
 函 数 名  : hmac_config_check_vap_num
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_config_check_vap_num(hmac_device_stru *hmac_device, hmac_cfg_add_vap_param_stru *param)
{
    return hmac_config_normal_check_vap_num(hmac_device, param);
}

static osal_u32 hmac_config_add_vap_preproc_para_check(hmac_cfg_add_vap_param_stru *param,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap[%d] {hmac_config_add_vap_preproc_para_check::dev null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((param->add_vap.vap_mode != WLAN_VAP_MODE_BSS_AP) && (param->add_vap.vap_mode != WLAN_VAP_MODE_BSS_STA) &&
        (param->add_vap.vap_mode != WLAN_VAP_MODE_WDS)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap[%d] {hmac_config_add_vap_preproc_para_check:vap_mode %d invalid.}",
            hmac_vap->vap_id, param->add_vap.vap_mode);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

static osal_u32 hmac_config_add_vap_preproc(hmac_cfg_add_vap_param_stru *param,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u32 ret, vap_idx;
    hmac_vap_stru *hmac_vap_tmp = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_void *fhook = OSAL_NULL;
#endif

    if (hmac_config_add_vap_preproc_para_check(param, hmac_device, hmac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 73规格不支持双sta 或 双ap 共存 */
    for (vap_idx = 0; vap_idx < WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL || hmac_vap_tmp->vap_id == param->add_vap.vap_id) {
            continue;
        }

        if ((hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_STA && hmac_vap_tmp->p2p_mode == WLAN_LEGACY_VAP_MODE &&
            param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_STA && param->add_vap.p2p_mode == WLAN_LEGACY_VAP_MODE) ||
            ((param->add_vap.vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_AP &&
            (hmac_vap_tmp->vap_state != MAC_VAP_STATE_INIT && hmac_vap_tmp->vap_state != MAC_VAP_STATE_BUTT)))) {
            oam_warning_log3(0, OAM_SF_CFG, "hmac_config_add_vap_preproc:has a sta/ap now,vapid[%d] mode[%d] state[%d]",
                hmac_vap_tmp->vap_id, param->add_vap.vap_mode, hmac_vap_tmp->vap_state);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if ((param->add_vap.p2p_mode == WLAN_P2P_CL_MODE) || (param->add_vap.p2p_mode == WLAN_P2P_GO_MODE)) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_P2P_TEARDOWN_SESSION);
        if (fhook != OSAL_NULL) {
            ret = ((hmac_p2p_teardown_twt_session_cb)fhook)(hmac_device);
            if (ret != OAL_SUCC) {
                return ret;
            }
        }
    }
#endif

    /* VAP个数判断 */
    ret = hmac_config_check_vap_num(hmac_device, param);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap[%d]{hmac_config_add_vap_preproc:check_vap_num fail[%d]}", hmac_vap->vap_id, ret);
        return ret;
    }

    /* 避免打开wifi的同时，dfr恢复vap，同时在netdev_priv下挂vap */
    if (param->net_dev->ml_priv != OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_add_vap_preproc::vap created.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

static osal_u32 hmac_config_add_vap_proc(hmac_cfg_add_vap_param_stru *cfg_param,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap_new, osal_u8 vap_id)
{
    osal_u32 ret;

    cfg_param->add_vap.vap_id = vap_id;

    /* 初始清0 */
    memset_s(hmac_vap_new, OAL_SIZEOF(hmac_vap_stru), 0, OAL_SIZEOF(hmac_vap_stru));
    /* 初始化HMAC VAP */
    ret = hmac_vap_init_etc(hmac_vap_new, hmac_device->chip_id, hmac_device->device_id, vap_id, cfg_param);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_proc::hmac_vap_init_etc failed[%d].}",
            vap_id, ret);

        /* 释放hmac_vap特性指针数组内存 */
        hmac_config_del_vap_feature(hmac_vap_new);
        /* 异常处理，释放内存 */
        hmac_vap_free_mib_etc(hmac_vap_new);
        hmac_res_free_mac_vap_etc(vap_id);
        return ret;
    }
    /* 设置反挂的net_device指针 */
    hmac_vap_new->net_device = cfg_param->net_dev;

    /* 包括'\0' */
    if (memcpy_s(hmac_vap_new->name, sizeof(hmac_vap_new->name), cfg_param->net_dev->name, OAL_IF_NAME_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_add_vap_proc::memcpy_s error}");
    }

    /* 将申请到的mac_vap空间挂到net_device ml_priv指针上去 */
    cfg_param->net_dev->ml_priv = hmac_vap_new;

    memset_s(hmac_vap_new->dscp_tid_map, HMAC_MAX_DSCP_VALUE_NUM, HMAC_DSCP_VALUE_INVALID, HMAC_MAX_DSCP_VALUE_NUM);

    /* 申请hmac组播用户 */
    hmac_user_add_multi_user_etc(hmac_vap_new, &cfg_param->add_vap.muti_user_id);
    hmac_vap_set_multi_user_idx_etc(hmac_vap_new, cfg_param->add_vap.muti_user_id);

    hmac_device_set_vap_id_etc(hmac_device, hmac_vap_new, vap_id, cfg_param, OAL_TRUE);

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_add_vap_etc
 功能描述  : 创建HMAC 业务VAP
 输入参数  : hmac_vap   : 指向配置vap
             len    : 参数长度
             param : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_add_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_vap_stru                 *hmac_vap_new = OAL_PTR_NULL;
    osal_u32                     ret;
    osal_s32                       lret;
    hmac_device_stru               *dev;
    osal_u8                      vap_id;
    hmac_cfg_add_vap_param_stru    *cfg_param = (hmac_cfg_add_vap_param_stru *)msg->data;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_add_vap_etc::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dev = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    ret = hmac_config_add_vap_preproc(cfg_param, dev, hmac_vap);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    /* 从资源池申请hmac vap */
    ret = mac_res_alloc_hmac_vap(&vap_id);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_add_vap_etc:alloc fail[%d]}", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    /* 从资源池获取新申请到的hmac vap */
    hmac_vap_new = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap_new == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_etc::hmac_vap null.}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从资源池中获取hal vap id */
    ret = mac_get_hal_vap_id(cfg_param->add_vap.p2p_mode, cfg_param->add_vap.vap_mode,
        &cfg_param->add_vap.hal_vap_id);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_add_vap_etc:alloc fail[%d]}", vap_id, ret);
        return (osal_s32)ret;
    }

    ret = hmac_config_add_vap_proc(cfg_param, dev, hmac_vap_new, vap_id);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    lret = hmac_config_add_vap(hmac_vap_new, msg);
    if (osal_unlikely(lret != OAL_SUCC)) {
        /* 此处回退有误，需要对应mac_device_set_vap_id，做回退操作 */
        hmac_device_set_vap_id_etc(dev, hmac_vap_new, vap_id, cfg_param, OAL_FALSE);
        hmac_user_del_multi_user_etc(hmac_vap_new);

        /* 释放hmac_vap特性指针数组内存 */
        hmac_config_del_vap_feature(hmac_vap_new);
        /* 异常处理，释放内存 */
        hmac_vap_free_mib_etc(hmac_vap_new);
        hmac_res_free_mac_vap_etc(vap_id);
        /* add vap 失败，需要将net_device ml_priv 设置为NULL,避免下次开wifi 不添加vap，导致打开wifi 失败 */
        cfg_param->net_dev->ml_priv = OAL_PTR_NULL;

        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_etc::failed[%d]}", vap_id, lret);
        return lret;
    }

    /* BEGIN: P2P GO 支持的最大用户为4 */
    if (is_p2p_go(hmac_vap_new)) {
        hmac_config_set_max_user_etc(hmac_vap_new, 0, WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC);
    }
    /* END: P2P GO 支持的最大用户为4 */

    hmac_vap_new->init_flag = MAC_VAP_VAILD;

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_vap_new);

    hmac_vap_sync(hmac_vap_new);
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_add_vap_etc:ok!multi user idx[%d]}", hmac_vap_new->multi_user_idx);
    oam_warning_log4(0, OAM_SF_CFG, "{hmac_config_add_vap_etc::vap mode[%d] p2p mode[%d] vap id[%d] hal vap id[%d]!}",
        cfg_param->add_vap.vap_mode, cfg_param->add_vap.p2p_mode,
        cfg_param->add_vap.vap_id, cfg_param->add_vap.hal_vap_id);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_device_vap_num
 功能描述  : 获取device下的业务vap个数
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_get_device_vap_num(osal_void)
{
    hmac_device_stru        *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    return hmac_device->vap_num;
}

osal_void hmac_config_del_vap_feature(hmac_vap_stru *hmac_vap)
{
    /* 配置vap不做特性内存申请和释放 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        return;
    }
#ifdef _PRE_WLAN_FEATURE_FTM
    /* vap删除时，释放FTM申请的内存空间 */
    hmac_vap_ftm_deinit(hmac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_proc_vap_deinit(hmac_vap);
#endif
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP_FEATURE, hmac_vap);
    hmac_vap_feature_deinit(hmac_vap);
}

osal_u32 hmac_config_add_vap_feature(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret = OAL_SUCC;
    osal_void *fhook = OSAL_NULL;

    /* 配置vap不做特性内存申请和释放 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        return OAL_SUCC;
    }

    ret = hmac_vap_feature_init(hmac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    ret = hmac_m2s_proc_vap_init(hmac_vap);
    if (ret != OAL_SUCC) {
        return ret;
    }
#endif
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PROC_VAP_ADD);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_btcoex_proc_vap_add_cb)fhook)(hmac_vap);
    }

    return ret;
}

OSAL_STATIC osal_u32 hmac_config_del_vap_pre(hmac_vap_stru *hmac_vap, osal_bool is_cfg)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_device_stru                *hmac_device;
#endif
    if (hmac_vap->vap_state != MAC_VAP_STATE_INIT) {
        oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_del_vap_etc::delete failed. state=%d,mode=%d}",
                         hmac_vap->vap_id, hmac_vap->vap_state, hmac_vap->vap_mode);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_device = mac_res_get_dev_etc(mac_vap->device_id);
    if (osal_unlikely(mac_device != OSAL_NULL) && hmac_vap->vap_base_info.vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 取消 CAC 定时器 并且恢复硬件发送 */
        hmac_dfs_cac_stop_etc(hmac_device, hmac_vap);
        hmac_dfs_off_cac_stop_etc(hmac_device, hmac_vap);
    }
#endif
    osal_spin_lock_destroy(&hmac_vap->cache_user_lock);
    /* 如果是配置VAP, 去注册配置vap对应的net_device, 释放，返回 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        /* 在注销netdevice之前先将指针赋为空 */
        oal_net_device_stru   *net_device = hmac_vap->net_device;
        hmac_vap->net_device = OAL_PTR_NULL;
        oal_smp_mb();
        if (is_cfg == OSAL_TRUE) {
            mac_vap_intrrupt_enable();
        oal_net_unregister_netdev(net_device);
            mac_vap_intrrupt_disable();
        } else {
            oal_net_unregister_netdev(net_device);
        }
        hmac_vap_free_mib_etc(hmac_vap);
        hmac_res_free_mac_vap_etc(hmac_vap->vap_id);
        return  OAL_SUCC;
    }

    /* 业务vap net_device已在WAL释放，此处置为null */
    hmac_vap->net_device = OAL_PTR_NULL;
    return OAL_CONTINUE;
}

OSAL_STATIC osal_u32 hmac_config_del_vap_proc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_DETACH);

    /* 组播转单播的detach */
    if (fhook != OSAL_NULL) {
        ((hmac_m2u_detach_cb)fhook)(hmac_vap);
    }

    /* 业务vap已删除，从device上去掉 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_del_vap_etc::get dev failed.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 清理所有的timer */
    if (hmac_vap->mgmt_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->mgmt_timer));
    }
    if (hmac_vap->scan_timeout.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->scan_timeout));
    }
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    if (hmac_vap->scanresult_clean_timeout.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->scanresult_clean_timeout));
    }
#endif

    /* 删除vap时删除TCP ACK的队列 */
#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_opt_deinit_list_etc(hmac_vap);
#endif

    /* 释放pmksa */
    ret = (osal_u32)hmac_config_flush_pmksa_etc(hmac_vap, OAL_PTR_NULL);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_del_vap_etc::hmac_config_flush_pmksa_etc fail[%d]}",
            hmac_vap->vap_id, ret);
    }
    hmac_vap_exit_etc(hmac_vap, hmac_device);

#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_device->vap_num == 0) {
#if (!defined(_PRE_PRODUCT_ID_HOST))
        /* wlan0网络设备一直存在 */
        hmac_device->st_p2p_info.primary_net_device = OAL_PTR_NULL;
#endif
    }
#endif

    /* 释放hmac_vap特性指针数组内存 */
    hmac_config_del_vap_feature(hmac_vap);

    /* 异常处理，释放内存 */
    hmac_vap_free_mib_etc(hmac_vap);
    hmac_res_free_mac_vap_etc(hmac_vap->vap_id);
    return OAL_CONTINUE;
}

OSAL_STATIC osal_void hmac_vap_del_pre(hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_TCP_OPT
    osal_u32 dir_index;
    osal_u32 lock_size;
    osal_u16 queue_index;
#endif

    osal_adapt_wait_destroy(&hmac_vap->mgmt_tx.wait_queue);
    osal_adapt_wait_destroy(&hmac_vap->query_wait_q);

#ifdef _PRE_WLAN_TCP_OPT
    for (dir_index = 0; dir_index < ARRAY_SIZE(hmac_vap->hmac_tcp_ack); dir_index++) {
        osal_spin_lock_destroy(&(hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack.hmac_tcp_ack_lock));

        lock_size = ARRAY_SIZE(hmac_vap->hmac_tcp_ack[dir_index].data_queue_lock);
        for (queue_index = 0; queue_index < lock_size; queue_index++) {
            osal_spin_lock_destroy(&(hmac_vap->hmac_tcp_ack[dir_index].data_queue_lock[queue_index]));
        }
    }
#endif
#ifdef _PRE_WLAN_FEATURE_WPA3
    osal_spin_lock_destroy(&(hmac_vap->pmksa_mgmt.lock));
#endif
}

OSAL_STATIC osal_s32 hmac_config_del_vap_sync_device(hmac_vap_stru *hmac_vap, frw_msg *msg,
    hmac_cfg_del_vap_param_stru *del_vap_param)
{
    osal_s32 ret;

    ret = hmac_config_del_vap(hmac_vap, msg);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_del_vap_etc::hmac_config_send_event_etc failed[%d].}",
                       hmac_vap->vap_id, ret);
        // 不退出，保证Devce挂掉的情况下可以下电。
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{hmac_config_del_vap_sync_device::Del succ.vap_mode[%d], p2p_mode[%d], multi user idx[%d], device_id[%d]}",
        hmac_vap->vap_mode, del_vap_param->add_vap.p2p_mode, hmac_vap->multi_user_idx, hmac_vap->device_id);
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_del_vap_etc
 功能描述  : 删除vap
 输入参数  : hmac_vap   : 指向vap的指针
             len    : 参数长度
             param : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_del_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32                     ret;
    hmac_cfg_del_vap_param_stru    *del_vap_param;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap_etc::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    del_vap_param = (hmac_cfg_del_vap_param_stru *)msg->data;
    ret = hmac_config_del_vap_sync_device(hmac_vap, msg, del_vap_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hmac_vap_del_pre(hmac_vap);
    mac_vap_intrrupt_disable();
    if (hmac_vap->init_flag == MAC_VAP_INVAILD) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap_etc::hmac_vap is INVAILD.}");
        mac_vap_intrrupt_enable();
        return OAL_FAIL;
    }
    /* 自减后，判断是否为0 */
    if (osal_adapt_atomic_dec_return(&(hmac_vap->use_cnt)) == 0) {
        frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_vap);
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        /* 删除相应的WDS定时器 */
        if (hmac_vap->wds_table.wds_timer.is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(&(hmac_vap->wds_table.wds_timer));
        }
        /* vap销毁前清除所有wds信息 */
        hmac_wds_reset_sta_mapping_table(hmac_vap);
        hmac_wds_reset_neigh_table(hmac_vap);
#endif
        ret = (osal_s32)hmac_config_del_vap_pre(hmac_vap, OSAL_TRUE);
        if (ret != OAL_CONTINUE) {
            mac_vap_intrrupt_enable();
            return ret;
        }

        ret = (osal_s32)hmac_config_del_vap_proc(hmac_vap);
        if (ret != OAL_CONTINUE) {
            mac_vap_intrrupt_enable();
            return ret;
        }
    }

    mac_vap_intrrupt_enable();
    return OAL_SUCC;
}

/* 转发流程中VAP引用计数为0时，安全删除VAP及下挂数据结构 */
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_s32 hmac_config_safe_del_vap(hmac_vap_stru *hmac_vap)
{
    osal_s32 ret;

    if (osal_adapt_atomic_dec_return(&(hmac_vap->use_cnt)) != 0) {
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_safe_del_vap:start del vap}", hmac_vap->vap_id);

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_vap);
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    /* 删除相应的WDS定时器 */
    if (hmac_vap->wds_table.wds_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->wds_table.wds_timer));
    }
    /* vap销毁前清除所有wds信息 */
    hmac_wds_reset_sta_mapping_table(hmac_vap);
    hmac_wds_reset_neigh_table(hmac_vap);
#endif

    ret = (osal_s32)hmac_config_del_vap_pre(hmac_vap, OSAL_FALSE);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
    ret = (osal_s32)hmac_config_del_vap_proc(hmac_vap);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_def_chan_etc
 功能描述  : 配置默认频带，信道，带宽
 输入参数  : hmac_vap : 指向vap
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_config_def_chan_etc(hmac_vap_stru *hmac_vap)
{
    osal_u8                     channel;
    mac_cfg_mode_param_stru       param;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if (((hmac_vap->channel.band == WLAN_BAND_BUTT) ||
         (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT) ||
         (hmac_vap->protocol == WLAN_PROTOCOL_BUTT)) && (!is_p2p_go(hmac_vap))) {
        param.band = WLAN_BAND_2G;
        param.en_bandwidth = WLAN_BAND_WIDTH_20M;
        param.protocol = WLAN_HT_MODE;
        hmac_config_set_mode_etc(hmac_vap, OAL_SIZEOF(param), (osal_u8 *)&param);
    }

    if ((hmac_vap->channel.chan_number == 0) && (!is_p2p_go(hmac_vap))) {
        hmac_vap->channel.chan_number = WLAN_DEFAULT_CHAN_NUM;
        channel = hmac_vap->channel.chan_number;
        msg_info.data = (osal_u8 *)&channel;
        msg_info.data_len = OAL_SIZEOF(osal_u32);
        hmac_config_set_freq_etc(hmac_vap, &msg_info);
    }

    return OAL_SUCC;
}

static osal_u32 hmac_config_start_vap_try_scan(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    unref_param(hmac_device);
    if (is_legacy_vap(hmac_vap)) {
        /* 02使用hostapd进行初始扫描，51使用驱动初始扫描 */
        hmac_config_def_chan_etc(hmac_vap);
    }
    return OAL_CONTINUE;
}

static osal_u32 hmac_config_start_vap_ap_mode(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    osal_u32 ret;

    /* 设置AP侧状态机为 WAIT_START */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);

    ret = hmac_config_start_vap_try_scan(hmac_vap, hmac_device);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 这里 status 等于 MAC_CHNL_AV_CHK_NOT_REQ(无需检测) 或者 MAC_CHNL_AV_CHK_COMPLETE(检测完成) */
    /* 检查协议 频段 带宽是否设置 */
    if ((hmac_vap->channel.band == WLAN_BAND_BUTT) || (hmac_vap->protocol == WLAN_PROTOCOL_BUTT) ||
        (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT)) {
        if (is_p2p_go(hmac_vap)) {
            /* wpa_supplicant 会先设置vap up， 此时并未给vap 配置信道、带宽和协议模式信息，
               wpa_supplicant 在cfg80211_start_ap 接口配置GO 信道、带宽和协议模式信息，
               故此处如果没有设置信道、带宽和协议模式，直接返回成功，不返回失败。 */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
            oam_warning_log4(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_start_vap_ap_mode::set band bw protocol first.band[%d], bw[%d], protocol[%d]}",
                hmac_vap->vap_id, hmac_vap->channel.band, hmac_vap->channel.en_bandwidth, hmac_vap->protocol);
            return OAL_SUCC;
        } else {
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
            oam_error_log1(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_start_vap_ap_mode::set band bandwidth protocol first.}", hmac_vap->vap_id);
            return OAL_FAIL;
        }
    }

    /* 检查信道号是否设置 */
    if ((hmac_vap->channel.chan_number == 0) && (!is_p2p_go(hmac_vap))) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap_ap_mode::set channel number first.}");
        return OAL_FAIL;
    }

    /* 设置bssid */
    memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);
    /* 入网优化，不同频段下的能力不一样 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_FALSE);
    } else {
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_TRUE);
    }

    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_UP);
    return OAL_CONTINUE;
}

static osal_u32 hmac_config_start_vap_proc(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, mac_cfg_start_vap_param_stru *start_param)
{
    osal_u32 ret;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ret = hmac_config_start_vap_ap_mode(hmac_vap, hmac_device);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_FEATURE_P2P
#ifndef WIN32
        /* 启动p2p device时，vap_param的p2p模式和mac_vap的p2p模式不同 */
        /* 从p2p device转化为gc，若p2p device启动时置fake up状态
        会导致不配置vap_ctrl寄存器，gc mac地址无效化 */
        if (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE) {
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        } else
#endif
        /* p2p0和p2p-p2p0 共VAP 结构，对于p2p cl不用修改vap 状态 */
        /* 不执行扫描的话，直接创建p2p cl会在init状态，此时需要置fake up */
            if ((start_param->p2p_mode != WLAN_P2P_CL_MODE) ||
                ((start_param->p2p_mode == WLAN_P2P_CL_MODE) && (hmac_vap->vap_state == MAC_VAP_STATE_INIT))) {
#endif
                hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
#ifdef _PRE_WLAN_FEATURE_P2P
            }
#endif
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
        if (is_legacy_sta(hmac_vap)) {
            ret = hmac_init_channel_scoring_record();
            if (ret != OAL_SUCC) {
                return ret;
            }
        }
#endif
    } else {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap_proc::Do not surport other mode[%d].}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
    }

    hmac_vap_init_rates_etc(hmac_vap);
    ret = hmac_config_start_vap_event_etc(hmac_vap, start_param->mgmt_rate_init_flag);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_start_vap_proc::hmac_config_send_event_etc failed[%d]}",
            hmac_vap->vap_id, ret);
        return ret;
    }

#if defined(_PRE_WLAN_FEATURE_DFS)
    if (is_legacy_ap(hmac_vap)) {
        hmac_dfs_try_cac_etc(hmac_device, hmac_vap);
    }
#endif

    oam_warning_log4(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_start_vap_proc::host start vap ok. vap mode[%d],p2p mode[%d]bw[%d]}",
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->p2p_mode, mac_vap_get_cap_bw(hmac_vap));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_start_vap_etc
 功能描述  : hmac启用VAP
 输入参数  : hmac_vap : 指向vap
             len      : 参数长度
             param   : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_start_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru              *hmac_device;
    mac_cfg_start_vap_param_stru *start_param = (mac_cfg_start_vap_param_stru *)msg->data;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_start_vap_etc::param null");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_DAQ
    if (hmac_get_data_acq_used() == OAL_TRUE) {
        oam_error_log1(0, OAM_SF_CFG,
            "vap_id[%d]{hmac_config_start_vap_etc::DAQ has been enabled.Please reset the board.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }
#endif
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d]hmac_config_start_vap_etc::hmac_device null", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap_etc::the vap has been deleted.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (hmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START) ||
        (hmac_vap->vap_state == MAC_VAP_STATE_STA_FAKE_UP)) { /* 如果已经在up状态，则返回成功 */
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap_etc::state=%d, duplicate start again}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return OAL_SUCC;
    }

    return (osal_s32)hmac_config_start_vap_proc(hmac_vap, hmac_device, start_param);
}

OSAL_STATIC osal_u32 hmac_config_down_vap_pre(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry;
    struct osal_list_head *dlist_tmp;
    hmac_user_stru *hmac_user;
    oal_bool_enum_uint8 is_protected;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    /* vap down前清除所有wds信息 */
    hmac_wds_reset_sta_mapping_table(hmac_vap);
    hmac_wds_reset_neigh_table(hmac_vap);
#endif

    /* 刪除SR的定时器 */
    if (hmac_vap->collision_handler.collision_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->collision_handler.collision_timer));
    }
    oam_warning_log0(0, OAM_SF_DFT, "hmac_config_down_vap_pre:start del user");
    /* 遍历vap下所有用户, 删除用户 */
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_down_vap_pre::user_tmp null}", hmac_vap->vap_id);
            continue;
        }

        /* 管理帧加密是否开启 */
        is_protected = hmac_user->cap_info.pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame_etc(hmac_vap, hmac_user->user_mac_addr, MAC_DISAS_LV_SS, is_protected);
        oam_warning_log1(0, OAM_SF_DFT, "hmac_config_down_vap_pre:send DISASSOC frame, err_code = %d.",
            MAC_DISAS_LV_SS);

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
        /* 删除用户事件上报给上层 */
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);
        } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_DISAS_LV_SS);
        }
#endif

        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr, AP_DOWN_DEL_STA,
                USER_CONN_OFFLINE);
        }
        /* 删除用户 */
        hmac_user_del_etc(hmac_vap, hmac_user);
    }

    /* VAP下user链表应该为空 */
    if (osal_list_empty(&hmac_vap->mac_user_list_head) == OAL_FALSE) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_down_vap_pre::list is not empty}", hmac_vap->vap_id);
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

static osal_u32 hmac_config_down_vap_proc(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, osal_u16 len, osal_u8 *param)
{
    osal_u32 ret;
    frw_msg msg_info;

    unref_param(hmac_device);
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    oam_warning_log0(0, OAM_SF_CFG, "hmac_config_down_vap_proc enter");
    /* sta模式时 将desired ssid MIB项置空，并清空配置协议标志 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->mib_info != OAL_PTR_NULL) {
            memset_s(mac_mib_get_desired_ssid(hmac_vap), WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
        } else {
            oam_error_log1(0, OAM_SF_CFG, "vap_id[%d]hmac_config_down_vap_proc:mib info is NULL", hmac_vap->vap_id);
        }
        oam_info_log0(0, OAM_SF_CFG, "{hmac_config_down_vap_proc:sta protocol cfg clear}");
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
        if (is_legacy_sta(hmac_vap)) {
            hmac_deinit_channel_scoring_record();
        }
#endif
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
#ifdef _PRE_WLAN_FEATURE_DFS
        /* 取消 CAC 定时器 并且恢复硬件发送 */
        hmac_dfs_cac_stop_etc(hmac_device, hmac_vap, OSAL_TRUE);
        hmac_dfs_off_cac_stop_etc(hmac_device, hmac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_WPA3
        hmac_sae_clean_all_pmksa(hmac_vap);
        if (hmac_vap->pmksa_timer.is_registerd) {
            frw_destroy_timer_entry(&(hmac_vap->pmksa_timer));
        }
#endif
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    msg_info.data = param;
    msg_info.data_len = len;
    ret = (osal_u32)hmac_config_down_vap(hmac_vap, &msg_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d]hmac_config_down_vap_proc:send fail[%d]", hmac_vap->vap_id, ret);
        return ret;
    }

    /* 也同步下去,不host device写两遍和staut接口统一 */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_down_vap_etc
 功能描述  : 停用vap
 输入参数  : hmac_vap : 指向vap
             len      : 参数长度
             param   : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_down_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u32 ret;
    hmac_user_stru *multi_user = OAL_PTR_NULL;
    mac_cfg_down_vap_param_stru *cfg_param = (mac_cfg_down_vap_param_stru *)msg->data;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_down_vap_etc::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果vap已经在down的状态，直接返回 */
    if ((cfg_param->net_dev == OAL_PTR_NULL) || (hmac_vap->vap_state == MAC_VAP_STATE_INIT)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_down_vap_etc::dev is null or vap already down.}");
        return OAL_SUCC;
    }

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_vap);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL) || (multi_user == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_down_vap_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_down_vap_pre(hmac_vap);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_down_vap_etc::hmac_config_down_vap_pre fail ret[%u].}", ret);
        return (osal_s32)ret;
    }

    ret = hmac_config_down_vap_proc(hmac_vap, hmac_device, msg->data_len, msg->data);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_down_vap_etc::hmac_config_down_vap_proc fail ret[%u].}", ret);
        return (osal_s32)ret;
    }

#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_sae_clean_all_pmksa(hmac_vap);
    if (hmac_vap->pmksa_timer.is_registerd) {
        frw_destroy_timer_entry(&(hmac_vap->pmksa_timer));
    }
#endif
    mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);

#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_radar_wait_etc(hmac_device, hmac_vap);
#endif

    oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_down_vap_etc:: SUCC! Now remaining %d vaps in device[%d]}",
                     hmac_vap->vap_id, hmac_device->vap_num, hmac_device->device_id);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
OSAL_STATIC osal_s32 hmac_vap_config_sprintf_wpa_info(hmac_vap_stru *hmac_vap, osal_s8 *pc_print_buff,
    osal_u32 string_len, osal_char *wpa)
{
    osal_u32                   group_suit;
    osal_u32                   pair_suites[2] = {0};
    osal_u32                   akm_suites[2] = {0};
    osal_s32                   string_tmp_len;

    if (strcmp(wpa, "WPA") == 0) {
        group_suit = mac_mib_get_wpa_group_suite(hmac_vap);
        mac_mib_get_wpa_pair_suites(hmac_vap, pair_suites, sizeof(pair_suites));
        mac_mib_get_wpa_akm_suites(hmac_vap, akm_suites, sizeof(akm_suites));
    } else {
        group_suit = mac_mib_get_rsn_group_suite(hmac_vap);
        mac_mib_get_rsn_pair_suites(hmac_vap, pair_suites, sizeof(pair_suites));
        mac_mib_get_rsn_akm_suites(hmac_vap, akm_suites, sizeof(akm_suites));
    }

    string_tmp_len = (osal_s32)snprintf_s((osal_char *)(pc_print_buff + string_len),
        (OAM_REPORT_MAX_STRING_LEN - string_len - 1), (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
        "Privacy Invoked: \nRSNA-%s \n"
        "GRUOP       PAIREWISE0[Actived]         PAIRWISE1[Actived]   AUTH1[Active]   AUTH2[Active]\n"
        "%s        %s[%s]          %s[%s]             %s[%s]             %s[%s]\n",
        wpa,
        hmac_config_ciper2string_etc((osal_u8)group_suit),
        hmac_config_ciper2string_etc((osal_u8)pair_suites[0]),
        (pair_suites[0] == 0) ? "Actived" : "Inactived",
        hmac_config_ciper2string_etc((osal_u8)pair_suites[1]),
        (pair_suites[1] == 0) ? "Actived" : "Inactived",
        hmac_config_akm2string_etc((osal_u8)akm_suites[0]),
        (akm_suites[0] == 0) ? "Actived" : "Inactived",
        hmac_config_akm2string_etc((osal_u8)akm_suites[1]),
        (akm_suites[1] == 0) ? "Actived" : "Inactived");
    return string_tmp_len;
}

/*****************************************************************************
 函 数 名  : hmac_config_vap_info_etc
 功能描述  : 打印vap参数信息
 输入参数  : hmac_vap  : mac_vap
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_vap_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru             *hmac_device;
    osal_s32                     l_ret;
    wlan_protocol_enum_uint8     disp_protocol;
    hmac_user_stru              *multi_user;
    hmac_user_stru              *hmac_user;
    osal_u8                    loop;
    osal_char *wap_type = OAL_PTR_NULL;
    osal_char         *pc_print_buff;
    osal_u32           string_len;
    osal_s32            l_string_tmp_len;
    unref_param(msg);
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info_etc::this is config vap! can't get info.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info_etc::hmac_device null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_print_buff = (osal_char *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info_etc::pc_print_buff null.}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len    = 0;

    l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
        (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
        (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
        "vap id: %u  device id: %u  chip id: %u\n"
        "vap state: %u\n"
        "vap mode: %u   P2P mode:%u\n"
        "ssid: %.32s\n"
        "hide_ssid :%u\n"
        "vap nss[%u] devicve nss[%u]\n",
        hmac_vap->vap_id, hmac_vap->device_id, hmac_vap->chip_id,
        hmac_vap->vap_state,
        hmac_vap->vap_mode,
        hmac_vap->p2p_mode,
        mac_mib_get_desired_ssid(hmac_vap),
        hmac_vap->cap_flag.hide_ssid,
        hmac_vap->vap_rx_nss,
        mac_device_get_nss_num(hmac_device));
    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (osal_u32)l_string_tmp_len;

    /* AP/STA协议模式显示 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user != NULL) {
            disp_protocol = hmac_user->cur_protocol_mode;
            l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
                (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
                (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
                "protocol: sta|ap[%s|%s],\n"
                "user bandwidth_cap:[%u] avail_bandwidth[%u] cur_bandwidth[%u],\n"
                "user id[%u] user nss:[%u] user avail nss[%u]\n",
                hmac_config_protocol2string_etc(hmac_vap->protocol),
                hmac_config_protocol2string_etc(disp_protocol),
                hmac_user->bandwidth_cap,
                hmac_user->avail_bandwidth,
                hmac_user->cur_bandwidth,
                hmac_vap->assoc_vap_id,
                hmac_user->user_num_spatial_stream,
                hmac_user->avail_num_spatial_stream);
        }
    } else {
        disp_protocol = hmac_vap->protocol;
        l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            "protocol: %s\n",
            hmac_config_protocol2string_etc(disp_protocol));
    }

    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (osal_u32)l_string_tmp_len;

    l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
        (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
        (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
        "band: %s  bandwidth: %s\n"
        "channel number:%u \n"
        "associated user number:%u/%u \n"
        "Beacon interval:%u \n"
        "vap feature info:\n"
        "amsdu     uapsd   wpa   wpa2   wps  keepalive\n"
        "%u         %u       %u    %u     %u     %u\n"
        "vap cap info:\n"
        "shpreamble  shslottime  nobeacon  shortgi   2g11ac \n"
        "%u           %u          %u         %u         %u\n"
        "tx power: %u \n"
        "protect mode: %u, auth mode: %u\n"
        "erp aging cnt: %u, ht aging cnt: %u\n"
        "auto_protection: %u\nobss_non_erp_present: %u\nobss_non_ht_present: %u\n"
        "txop_protect_mode: %u\n"
        "no_short_slot_num: %u\nno_short_preamble_num: %u\nnon_erp_num: %u\n"
        "non_ht_num: %u\nnon_gf_num: %u\n20M_only_num: %u\n"
        "no_40dsss_cck_num: %u\nno_lsig_txop_num: %u\n",
        hmac_config_band2string_etc(hmac_vap->channel.band),
        hmac_config_bw2string_etc(hmac_vap->protocol <= WLAN_MIXED_TWO_11G_MODE ?
        WLAN_BAND_WIDTH_20M : hmac_vap->channel.en_bandwidth),
        hmac_vap->channel.chan_number,
        hmac_vap->user_nums,
        mac_mib_get_MaxAssocUserNums(hmac_vap),
        mac_mib_get_beacon_period(hmac_vap),
        mac_mib_get_CfgAmsduTxAtive(hmac_vap),
        hmac_vap->cap_flag.uapsd,
        hmac_vap->cap_flag.wpa,
        hmac_vap->cap_flag.wpa2,
        mac_mib_get_WPSActive(hmac_vap),
        hmac_vap->cap_flag.keepalive,
        mac_mib_get_short_preamble_option_implemented(hmac_vap),
        mac_mib_get_short_slot_time_option_implemented(hmac_vap),
        hmac_vap->no_beacon,
        mac_mib_get_short_gi_option_in_twenty_implemented(hmac_vap),
        hmac_vap->cap_flag.support_11ac2g,
        hmac_vap->tx_power,
        hmac_vap->protection.protection_mode,
        mac_mib_get_authentication_mode(hmac_vap),
        hmac_vap->protection.obss_non_erp_aging_cnt,
        hmac_vap->protection.obss_non_ht_aging_cnt,
        hmac_vap->protection.auto_protection,
        hmac_vap->protection.obss_non_erp_present,
        hmac_vap->protection.obss_non_ht_present,
        hmac_vap->protection.lsig_txop_protect_mode,
        hmac_vap->protection.sta_no_short_slot_num,
        hmac_vap->protection.sta_no_short_preamble_num,
        hmac_vap->protection.sta_non_erp_num,
        hmac_vap->protection.sta_non_ht_num,
        hmac_vap->protection.sta_non_gf_num,
        hmac_vap->protection.sta_20m_only_num,
        hmac_vap->protection.sta_no_40dsss_cck_num,
        hmac_vap->protection.sta_no_lsig_txop_num);
    if (l_string_tmp_len < 0) {
        goto sprint_fail;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    dft_report_params_etc((osal_u8 *)pc_print_buff, (osal_u16)osal_strlen(pc_print_buff),
        SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);

    /* 上述日志量超过OAM_REPORT_MAX_STRING_LEN，分多次oam_print */
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len    = 0;

    /* WPA/WPA2 加密参数 */
    if (mac_mib_get_privacyinvoked(hmac_vap) == OAL_TRUE) {
        multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
        if (multi_user == OAL_PTR_NULL) {
            oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info_etc::multi_user[%d] null.}",
                             hmac_vap->vap_id, hmac_vap->multi_user_idx);
            oal_mem_free(pc_print_buff, OAL_TRUE);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_TRUE && (hmac_vap->cap_flag.wpa == 1)) {
            wap_type = "WPA";
            l_string_tmp_len = hmac_vap_config_sprintf_wpa_info(hmac_vap, (osal_s8 *)pc_print_buff, string_len,
                wap_type);
            if (l_string_tmp_len < 0) {
                goto sprint_fail;
            }
            string_len += (osal_u32)l_string_tmp_len;
        }
        if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_TRUE && (hmac_vap->cap_flag.wpa2 == 1)) {
            wap_type = "WPA2";
            l_string_tmp_len = hmac_vap_config_sprintf_wpa_info(hmac_vap, (osal_s8 *)pc_print_buff, string_len,
                wap_type);
            if (l_string_tmp_len < 0) {
                goto sprint_fail;
            }
            string_len += (osal_u32)l_string_tmp_len;
        }

        l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            "MULTI_USER: cipher_type:%s, key_type:%u \n",
            hmac_config_ciper2string_etc(multi_user->key_info.cipher_type),
            multi_user->user_tx_info.security.cipher_key_type);
        if (l_string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (osal_u32)l_string_tmp_len;
    } else {
        l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            "Privacy NOT Invoked\n");
        if (l_string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (osal_u32)l_string_tmp_len;
    }

    /* APP IE 信息 */
    for (loop = 0; loop < OAL_APP_IE_NUM; loop++) {
        l_string_tmp_len = (osal_s32)snprintf_s(pc_print_buff + string_len,
            (OAM_REPORT_MAX_STRING_LEN - string_len - 1), (OAM_REPORT_MAX_STRING_LEN - string_len - 1),
            "APP IE:type= %u, len = %u\n", loop, hmac_vap->app_ie[loop].ie_len);
        if (l_string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (osal_u32)l_string_tmp_len;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    dft_report_params_etc((osal_u8 *)pc_print_buff, (osal_u16)osal_strlen(pc_print_buff),
        SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    oal_mem_free(pc_print_buff, OAL_TRUE);

    l_ret = hmac_config_vap_info(hmac_vap);
    if (osal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info_etc::hmac_config_vap_info failed[%d].}",
                       hmac_vap->vap_id, l_ret);
    }

    return l_ret;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vap_info_etc:: snprintf_s return error!}");
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    dft_report_params_etc((osal_u8 *)pc_print_buff, (osal_u16)osal_strlen(pc_print_buff),
        SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    oal_mem_free(pc_print_buff, OAL_TRUE);

    return OAL_FAIL;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_sta_vap_info_syn_etc
 功能描述  : hmac_offload架构下同步sta vap信息到 dmac
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_sta_vap_info_syn_etc(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->hal_device->current_11ax_working == OSAL_TRUE) {
        hal_set_phy_aid(hmac_vap->sta_aid);
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        /* 判断aid有效性 */
        if (hmac_vap->sta_aid == 0 || hmac_vap->sta_aid >= 2008) { // sta的aid值不为0，也不能大于等于2008
            oam_error_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_config_sta_vap_info_syn_etc::aid invalid[%d]}",
                hmac_vap->vap_id, hmac_vap->sta_aid);
            return OAL_FAIL;
        }

        /* STA关联时设置aid寄存器 */
        hal_set_mac_aid(hmac_vap->hal_vap, hmac_vap->sta_aid);
#endif
        hmac_config_rx_he_rom_en_handle(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
        /* 依据vap下TXOP PS开关，重设寄存器开关 */
        hmac_txopps_set_machw(hmac_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */
    }

    hmac_vap_sync(hmac_vap);
    return OAL_SUCC;
}

osal_s32 hmac_config_del_vap_success(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru  *hmac_device;
    osal_u8            vap_num;
    unref_param(msg);

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap_success::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_del_vap_success:: device[%d] null!}", hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap_num = (osal_u8)hmac_get_device_vap_num();
    /* chip下的所有device的业务vap个数为0,才能给device下电 */
    if (vap_num == 0) {
#ifdef _PRE_WLAN_FEATURE_P2P
        /* 在的set_mode的流程中避免删除vap后下电 */
        if ((hmac_device->set_mode_iftype == NL80211_IFTYPE_P2P_CLIENT) ||
            (hmac_device->set_mode_iftype == NL80211_IFTYPE_P2P_GO) ||
            (hmac_device->set_mode_iftype == NL80211_IFTYPE_STATION)) {
            return OAL_SUCC;
        }
#endif
        if (wlan_close() != OAL_ERR_CODE_FOBID_CLOSE_DEVICE) {
            /* 关闭wifi,清空扫描结构体 start */
            hmac_scan_clean_scan(&(hmac_device->scan_mgmt));
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_del_vap_success:: hmac_device[%d]}", hmac_vap->device_id);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
            hmac_config_host_dev_exit_etc(hmac_vap, msg);
#endif
        }
        /* 临时 解决出现device exit后 再次启动时不能RX的问题 */
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_init_event_process_etc
 功能描述  : dmac 抛给hmac的初始化回复事件
 输入参数  : event_mem: 事件结构体
 返 回 值  : 错误码
*****************************************************************************/
static osal_s32 hmac_init_event_process_etc(dmac_tx_event_stru    *ctx_event)
{             /* 事件结构体 */
    mac_data_rate_stru    *data_rate = (mac_data_rate_stru *)ctx_event->data;
    hmac_device_stru      *hmac_device = OAL_PTR_NULL;

    /* 同步mac支持的速率集信息 */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL || data_rate == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_init_event_process_etc::hmac_device or data_rate null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    if (memcpy_s((osal_u8 *)(hmac_device->mac_rates_11g), sizeof(hmac_device->mac_rates_11g),
        (osal_u8 *)data_rate, sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_init_event_process_etc::memcpy_s error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置vap初始化
*****************************************************************************/
osal_s32 hmac_cfg_vap_init_event(hmac_device_stru *hmac_device)
{
    dmac_tx_event_stru *dtx_event = OSAL_NULL;
    osal_u16 frame_len;
    osal_s32 ret;

    if (hmac_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg_vap_init_event::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* p2p时，有可能下发两次事件 */
    if (frw_get_init_state_etc() == FRW_INIT_STATE_ALL_SUCC) {
        return OAL_SUCC;
    }
    frame_len = sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM;
    dtx_event = (dmac_tx_event_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(dmac_tx_event_stru) + frame_len, OAL_TRUE);
    if (dtx_event == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg_vap_init_event::dtx_event null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(dtx_event, sizeof(dmac_tx_event_stru) + frame_len, 0, sizeof(dmac_tx_event_stru) + frame_len);
    dtx_event->frame_len = frame_len;

    /*************************************************************************/
    /*            netbuf data域的上报的data rate                              */
    /* --------------------------------------------------------------------  */
    /*              | mac_rates_11g info     |                            */
    /* --------------------------------------------------------------------  */
    /*              | sizeof(mac_data_rate_stru) |                           */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /************************************************************************/
    /* 将速率集的信息拷贝到data中，上报hmac */
    (osal_void)memcpy_s(dtx_event->data, frame_len, (osal_u8 *)(hmac_device->mac_rates_11g), frame_len);
    ret = hmac_init_event_process_etc(dtx_event);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_cfg_vap_init_event::init_event_process_etc fail[%d]}", ret);
    }
    oal_mem_free(dtx_event, OAL_TRUE);

    frw_set_init_state_etc(FRW_INIT_STATE_ALL_SUCC);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_cfg_vap_h2d_etc
 功能描述  : cfg vap h2d
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32   hmac_config_cfg_vap_h2d_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret1;
    hmac_device_stru      *hmac_device;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_cfg_vap_h2d_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_cfg_vap_h2d_etc::hmac_res_get_mac_dev_etc fail. vap_id[%u]}",
                       hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret1 = hmac_cfg_vap_init_event(hmac_device);

    return ret1;
}

#ifdef _PRE_WLAN_DFT_STAT
osal_s32 hmac_config_get_vap_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    ret = hmac_dft_get_vap_stat(hmac_vap, msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_get_vap_stat::hmac_dft_get_vap_stat failed[%d].}",
            ret);
        wifi_printf("{hmac_config_get_vap_stat::hmac_dft_get_vap_stat failed[%d].}",
            ret);
    }
    return ret;
}
#endif

/*****************************************************************************
 功能描述  : mac device 维护当前device所有up vap个数，避免数据收发流程增加mips
 输入参数  : mac_vap : 业务vap
 返 回 值: void
*****************************************************************************/
OSAL_STATIC osal_void hmac_config_update_up_vap_num(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 up_vap_num_old = hmac_device->up_vap_num;

    hmac_device->up_vap_num = (osal_u8)hmac_device_calc_up_vap_num_etc(hmac_device);

    /* 如果up_vap_num发生变化，则同步 */
    if (up_vap_num_old != hmac_device->up_vap_num) {
        hmac_device_sync(hmac_device);
    }
}

OSAL_STATIC osal_void hmac_config_vap_state(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    mac_vap_state_enum_uint8 old_state, mac_vap_state_enum_uint8 new_state)
{
    osal_void *fhook = OAL_PTR_NULL;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;

    if (new_state == MAC_VAP_STATE_STA_FAKE_UP) {
        /* 从INIT-FAKEUP不需要执行down vap,解决P版本下扫描遇见p2p0创建,导致扫描被abort */
        if ((old_state != new_state) && (old_state != MAC_VAP_STATE_INIT)) {
            hmac_vap_down_notify(hmac_vap); /* staut vap down时需要通知算法并做一些fake队列的清理工作 */
            hmac_vap->cap_flag.keepalive = OSAL_FALSE; /* 关闭keepalive */
        }
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_VAP_DOWN, sizeof(hal_to_dmac_vap_stru),
                                (osal_u8 *)(hmac_vap->hal_vap));
    } else if (new_state == MAC_VAP_STATE_STA_JOIN_COMP) {
        // mac vap迁移到MAC_VAP_STATE_STA_JOIN_COMP状态时，如果对应的mac_device正在扫描，那么需要先将扫描停止，
        // 因为hal处于扫描状态时无法处理HAL_DEVICE_EVENT_JOIN_COMP事件
        if (mac_is_dbac_enabled(hmac_device)) {
            /* 必须在抛事件给hal device状态机之前先down停止扫描等,否则可能存在join的时候hal在扫描 */
            hmac_scan_abort(hmac_device);
        }
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_JOIN_COMP, sizeof(hal_to_dmac_vap_stru),
                                (osal_u8 *)(hmac_vap->hal_vap));
        hmac_vap_work_set_channel(hmac_vap); /* sta关联前，hal dev状态机为work状态下配置工作信道 */

        /* send probe req with real MAC address before assoc req */
        (osal_void)memcpy_s((osal_void *)(hmac_device->scan_params.sour_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);
        if (hmac_scan_send_probe_req_frame(hmac_vap, hmac_vap->bssid,
            (osal_char *)mac_mib_get_desired_ssid(hmac_vap), WLAN_SSID_MAX_LEN, OAL_TRUE) != OAL_SUCC) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_vap_state:: failed.}", hmac_vap->vap_id);
        }
    } else if (new_state == MAC_VAP_STATE_UP) {
        hmac_vap->cap_flag.keepalive = OSAL_TRUE; /* MAC_VAP UP时要开启host侧keepalive能力，避免后续同步导致device侧被覆盖 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_VAP_CONFIG_SW_PREEMPT_SUBTYPE);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_vap_config_sw_preempt_subtype_cb)fhook)(hal_device);
        }
        hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_VAP_UP, sizeof(hal_to_dmac_vap_stru),
                                (osal_u8 *)(hmac_vap->hal_vap));
        hal_init_pm_info(hmac_vap->hal_vap);
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
        hmac_psm_update_bcn_rf_chain(hmac_vap, (osal_char)oal_get_real_rssi(hmac_vap->query_stats.signal));
#endif
        /* 设置接收beacon超时中断的时间 */
        hal_set_beacon_timeout(hmac_vap->beacon_timeout);
    } else if (new_state == MAC_VAP_STATE_ROAMING) {
        // CSA相关代码已移动至CSA模块内部
    } else {
        oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_state:old[%d]->new[%d]}",
            hmac_vap->vap_id, old_state, new_state);
    }
}

/*****************************************************************************
 功能描述  : 配置vap_state
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_vap_state_syn(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 new_vap_state)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_vap_state_enum_uint8 old_vap_state = hmac_vap->vap_state;

    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hmac_vap_state_change_notify_stru state_change_notify;

    /* 必须放到向hal device抛事件之前,否则存在hal device找不到up vap,但vap状态还是up */
    hmac_vap_state_change_etc(hmac_vap, new_vap_state);

    memset_s(&state_change_notify, sizeof(state_change_notify), 0, sizeof(state_change_notify));
    state_change_notify.hmac_vap = hmac_vap;
    state_change_notify.new_state = new_vap_state;
    state_change_notify.old_state = old_vap_state;
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_VAP_STATE_CHANGE, (osal_void *)&state_change_notify);

    /* staut模式hal状态机只关心up,fakeup两个状态 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_config_vap_state(hmac_device, hmac_vap, old_vap_state, new_vap_state);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) { /* aput模式hal状态机只关心up和非up状态 */
        if (new_vap_state == MAC_VAP_STATE_UP) {
            // mac vap迁移到MAC_VAP_STATE_UP状态时，如果对应的mac_device正在扫描，那么需要先将扫描停止，
            // 因为hal处于扫描状态时无法处理HAL_DEVICE_EVENT_VAP_UP事件
            if (mac_is_dbac_enabled(hmac_device)) {
                hmac_scan_abort(hmac_device); /* 必须在抛事件给hal
                                                device状态机之前先down停止扫描等,否则可能存在join的时候hal在扫描 */
            }
#ifdef _PRE_WLAN_FEATURE_M2S
            /* ap模式启动，需要在dbdc之前，5g c1 siso的话无法启动dbdc时，进行规格siso切换判断 */
            if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap)) {
                oam_warning_log1(0, OAM_SF_M2S,
                    "vap_id[%d] {hmac_config_vap_state_syn:: need to update vap sw to SISO.}", hmac_vap->vap_id);
                /* 整体方案修订为: 只刷新软件能力，如果有其他业务要执行切换继续执行，优先级冲突通过能力来判断约束 */
                hmac_m2s_spec_update_vap_sw_capbility(hmac_device, hmac_vap, WLAN_SINGLE_NSS);
            }
#endif
            hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_VAP_UP, sizeof(hal_to_dmac_vap_stru),
                                    (osal_u8 *)(hmac_vap->hal_vap));
            hmac_vap_work_set_channel(hmac_vap); /* aput up后，hal dev状态机为work状态下配置工作信道 */
        } else {
            hal_device_handle_event(hal_device, HAL_DEVICE_EVENT_VAP_DOWN, sizeof(hal_to_dmac_vap_stru),
                                    (osal_u8 *)(hmac_vap->hal_vap));
        }
    }

    hmac_config_update_up_vap_num();
    return OAL_SUCC;
}

static osal_void hmac_config_update_bandwidth(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device)
{
    wlan_bw_cap_enum_uint8 vap_bw_cap;

    /* set mib BandWidth */
    vap_bw_cap = (hmac_device->device_cap.channel_width > hal_device->cfg_cap_info->wlan_bw_max) ?
        hal_device->cfg_cap_info->wlan_bw_max : hmac_device->device_cap.channel_width;
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((hmac_vap->p2p_mode == WLAN_P2P_CL_MODE) && (hmac_get_p2p_common_enable_info() == 1) &&
        (hmac_get_p2p_common_band_info() == P2P_BAND_20M)) {
        vap_bw_cap = P2P_BAND_20M;
    }
#endif
    mac_mib_set_dot11_vap_max_bandwidth(hmac_vap, vap_bw_cap);
}

/*****************************************************************************
 功能描述  : 根据device能力设置mib参数，同时hmac_offload架构下dmac的mib能力同步到hmac
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_u32 hmac_config_vap_mib_update(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    oal_bool_enum_uint8 mu_beamformee_cap;
#ifdef _PRE_WLAN_FEATURE_11AX
    oal_bool_enum_uint8 ret;
#endif

    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_mib_update::HAL_DEVICE null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* set mib BandWidth */
    hmac_config_update_bandwidth(hmac_vap, hmac_device, hal_device);

    /* 更新VHT相关mib能力 */
    mac_mib_set_vht_channel_width_option_implemented(hmac_vap,
        hmac_device_trans_bandwith_to_vht_capinfo(mac_mib_get_dot11_vap_max_bandwidth(hmac_vap)));

#ifdef _PRE_WLAN_FEATURE_11AX
    /* set 11ax hal cap */
    hmac_vap->hal_cap_11ax = ((hal_device->cfg_cap_info->is_supp_11ax == OSAL_TRUE) && (is_legacy_sta(hmac_vap))) ?
        OSAL_TRUE : OSAL_FALSE;
    ret = is_11ax_vap(hmac_vap) ? OSAL_TRUE : OSAL_FALSE;
    mac_mib_set_he_option_implemented(hmac_vap, ret);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_vap->radar_detector_is_supp = hal_device->cfg_cap_info->radar_detector_is_supp;
#endif

    /* 仅支持发送能力 */
    mu_beamformee_cap = ((hmac_device->device_cap.mu_bfmee_is_supp == OSAL_TRUE) &&
        (hal_device->cfg_cap_info->mu_bfmee_is_supp == OSAL_TRUE)) ? OSAL_TRUE : OSAL_FALSE;
    mac_mib_set_vht_mu_bfee_option_implemented(hmac_vap, mu_beamformee_cap);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_mib_set_txopps(hmac_vap, hal_device->cfg_cap_info->txopps_is_supp);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据hal device能力，同时设置dmac vap能力并同步到hmac
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_u32 hmac_config_vap_cap_update(hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip;
    hal_to_dmac_device_stru *hal_device;

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_cap_update::HAL_DEVICE null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 刷新is_1024qam和nb */
    hmac_vap->cap_flag.is_1024qam = hal_device->cfg_cap_info->is_supp_1024qam;
    hmac_vap->cap_flag.nb = hal_device->cfg_cap_info->nb_is_supp;

    /* 根据hal chip定制化2g5g rf能力，置mac vap cap，并同步到host，用户ap和sta启动 */
    hmac_vap->cap_flag.custom_siso_2g = ((hal_chip->rf_custom_mgr.nss_num_2g == WLAN_SINGLE_NSS) ?
        OSAL_TRUE : OSAL_FALSE);
    hmac_vap->cap_flag.custom_siso_5g = ((hal_chip->rf_custom_mgr.nss_num_2g == WLAN_SINGLE_NSS) ?
        OSAL_TRUE : OSAL_FALSE);

    oam_warning_log_alter(0, OAM_SF_CFG,
        "{hmac_config_vap_mib_update::vap_mode=[%d],1024_cap=[%d],nb_cap=[%d],custom_siso_2g[%d],custom_siso_5g[%d]!}",
        /* 5代表5后面有5个打印参数 */
        5, hmac_vap->vap_mode, hmac_vap->cap_flag.is_1024qam,
        hmac_vap->cap_flag.nb, hmac_vap->cap_flag.custom_siso_2g, hmac_vap->cap_flag.custom_siso_5g);

    /* 同步刷新host侧txbf mib位 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_vht_su_bfee_option_implemented(hmac_vap, OAL_FALSE);
        mac_mib_set_vht_su_bfer_option_implemented(hmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : dmac vap add hal device
 输入参数  : mac_vap : 业务vap，而不是配置VAP
           param   : 参数
 返 回 值: osal_u32
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_add_single_hal_vap(hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap,
    hal_to_dmac_vap_stru **hal_vap_out, const mac_cfg_add_vap_param_stru *param)
{
    osal_u8 vap_idx = param->vap_id;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    unref_param(vap_id);
#if defined(_PRE_WLAN_FEATURE_P2P)
    /* 同一个vap ，能够挂接多个hal_vap */
    /* 由于MAC 硬件对于P2P 相关寄存器的限制。
     * GO 只能创建在hal_vap_id = 1,
     * CL 只能创建在hal_vap_id = 5,
     * 需要将P2P 模式传到hal_add_vap 函数中  */
    /*
     * |7      4|3      0|
     * |p2p_mode|vap_mode|
     */
    /* 因为p2p_mode在高4位，所有需要将其左移4位与vap_mode取或 */
    hal_add_vap(hal_device, (param->vap_mode) | (osal_u8)(param->p2p_mode << 4), param->hal_vap_id, &hal_vap);
#else
    hal_add_vap(hal_device, param->vap_mode, param->hal_vap_id, &hal_vap);
#endif

    /* 统一在最外层释放前面已经申请的资源 */
    if (hal_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_vap_add_single_hal_device::hal_vap null.}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 保存mac层的vap id */
    hal_vap->mac_vap_id = vap_idx;

    /* 配置MAC地址 */
    hal_vap_set_macaddr(hal_vap, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);

    /* 配置硬件WMM参数 */
    hmac_config_set_machw_wmm(hmac_vap);

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    if (param->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hal_set_mac_aid(hal_vap, 0);
    }
#endif

#if defined(_PRE_PM_TBTT_OFFSET_PROBE)
    hal_tbtt_offset_probe_init(hal_vap);
#endif

    *hal_vap_out = hal_vap;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除多个hal vap
 输入参数  : hmac_device :
           hmac_vap
           param   : 参数
 返 回 值: void
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_del_hal_vap(const hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    const osal_u8 *para)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u8 hal_vap_id;
    hmac_cfg_del_vap_param_stru *del_vap_param = (hmac_cfg_del_vap_param_stru *)para;
    unref_param(hmac_vap);
    hal_vap_id = del_vap_param->add_vap.hal_vap_id;
    hal_device = hmac_device->hal_device[0];
    if (hal_device != OSAL_NULL) {
        hal_del_vap(hal_device, hal_vap_id);
    }
}
/*****************************************************************************
 功能描述  : 添加多个hal vap
 输入参数  : hmac_device
           hmac_vap
           param
 返 回 值: osal_u32
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_add_hal_vap(const hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    hal_to_dmac_vap_stru **hal_vap_out, const mac_cfg_add_vap_param_stru *param)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_to_dmac_vap_stru    *hal_vap = OSAL_NULL;
    osal_u32 ret;

    hal_device = hmac_device->hal_device[0];
    if (hal_device != OSAL_NULL) {
        ret = hmac_vap_add_single_hal_vap(hal_device, hmac_vap, &hal_vap, param);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG,
                "{hmac_vap_add_hal_vap:: vap[%d]add hal vap failed.}", hmac_vap->vap_id);
            return ret;
        }
    }
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    if (hal_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_vap_add_hal_vap:: hal_vap == NULL.}");
        return OAL_FAIL;
    }
#endif
    // 动态dbdc返回主路的,非dbdc返回hal device0的
    // 静态dbdc只跑一次,返回自己挂接的hal device上的hal vap
    *hal_vap_out = hal_vap;

    return OAL_SUCC;
}
/*****************************************************************************
 功能描述  : 静态/动态DBDC删除hal vap
 输入参数  : hmac_device
           hmac_vap
           param
 返 回 值: void
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_del_hal_device(hmac_vap_stru *hmac_vap, const osal_u8 *para)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_fsm_detach(hmac_vap->hal_device);
#endif
    hmac_vap_del_hal_vap(hmac_device, hmac_vap, para);

    hmac_vap->hal_device = OSAL_NULL;
    hmac_vap->hal_vap = OSAL_NULL;
    hmac_vap->hal_chip = OSAL_NULL;
}
/*****************************************************************************
 功能描述  : 静态/动态DBDC添加hal vap
 输入参数  : hmac_device
           hmac_vap
           param
 返 回 值: osal_u32
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_vap_add_hal_device(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_cfg_add_vap_param_stru *param)
{
    osal_u32 ret;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    hal_to_dmac_device_stru *orig_hal_device = OSAL_NULL;
    hal_chip_stru *hal_chip = OSAL_NULL;

    ret = hmac_vap_add_hal_vap(hmac_device, hmac_vap, &hal_vap, param);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_vap_add_hal_device:add hal vap failed}", hmac_vap->vap_id);
        return ret;
    }

    /* 根据芯片id，获取hal_to_dmac_chip_stru结构体 */
    hal_chip = hal_get_chip_stru();

    hmac_vap->hal_vap = hal_vap;

    /* 根据hal vap来获取hal device */
    orig_hal_device = hal_chip_get_hal_device();
    /* 保存到dmac vap结构体中，方便获取 */
    hmac_vap->hal_device = orig_hal_device;
    hmac_vap->hal_chip = &hal_chip->hal_chip_base;

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_config_add_vap_proc_ext(hmac_vap_stru *hmac_vap, const mac_cfg_add_vap_param_stru *param_temp,
    hmac_device_stru *hmac_device)
{
    osal_u32 ret;

    if (hmac_vap->init_flag == MAC_VAP_VAILD) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_config_add_vap_proc_ext::add vap double init!}", hmac_vap->vap_id);
    }

    ret = hmac_vap_init(hmac_vap, param_temp->vap_id, param_temp);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 静态/动态DBDC添加hal vap */
    ret = hmac_vap_add_hal_device(hmac_device, hmac_vap, param_temp);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_proc_ext::add_hal_vap failed, vap id[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_id);
        return ret;
    }

    /* 根据device en_wmm设置vap的wmm功能 */
    hmac_config_set_wmm_register(hmac_vap, hmac_device->wmm);

    return OAL_SUCC;
}

static osal_void hmac_config_add_sta_vap_init(hmac_vap_stru *hmac_vap)
{
    /* 初始化STA AID 为 0 */
    hmac_vap_set_aid_etc(hmac_vap, 0);
    /* 执行特性初始化vap的函数 */
    /* 执行算法相关VAP初始化接口 */
    if (is_p2p_dev(hmac_vap)) {
        return;
    } else if (is_legacy_vap(hmac_vap)) {
        /* 带宽切换状态机初始化 */
        hmac_sta_bw_switch_fsm_attach(hmac_vap);
    }
    hmac_pm_sta_attach(hmac_vap);
    /* 初始化tbtt定时器 */
    hmac_vap->sta_tbtt.last_tbtt_time = 0;
}

OSAL_STATIC osal_u32 hmac_config_add_vap_ext(const mac_cfg_add_vap_param_stru *param_temp, hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;

    /* 初始化业务VAP，区分AP、STA和WDS模式 */
    switch (param_temp->vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            /* 执行特性初始化vap的函数 */
            /* 设置APUT的tbtt offset */
            hal_set_tbtt_offset(hmac_vap->hal_vap, hmac_vap->in_tbtt_offset);

            /* beacon发送超时继续发送组播队列 */
            hal_set_bcn_timeout_multi_q_enable(hmac_vap->hal_vap, OSAL_TRUE);

            /* 使能PA_CONTROL的vap_control位,否则ap首次扫描收不到pro response */
            hal_vap_set_opmode(hmac_vap->hal_vap, hmac_vap->vap_mode);
            break;

        case WLAN_VAP_MODE_BSS_STA:
            hmac_config_add_sta_vap_init(hmac_vap);
            break;

        case WLAN_VAP_MODE_WDS:
            break;

        default:
            oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_ext::invalid vap mode[%d].",
                hmac_vap->vap_id, param_temp->vap_mode);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 申请创建dmac组播用户 */
    hmac_user_add_multi_user(hmac_vap, param_temp->muti_user_id);
    hmac_vap_set_multi_user_idx_etc(hmac_vap, param_temp->muti_user_id);

    /* 初始化组播、广播管理帧基本参数,
     * 主要用于ap模式在启动扫描时候描述符正确填写广播管理帧参数，后续set_beacon和start_vap操作可以删除 */
    hmac_vap_init_tx_mgmt_rate(hmac_vap, hmac_vap->tx_mgmt_bmcast);

    /* vap创建之后，mib能力需要根据hal device重新刷新并同步到host侧 用于切换到siso之后，go等vap重新创建 */
    /* 用于切换到siso之后，go等vap重新创建 ,开关机时候，vap m2s能力同步到host */
#ifdef _PRE_WLAN_FEATURE_M2S
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (hmac_m2s_d2h_vap_info_syn(hmac_vap) != OAL_SUCC)) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_add_vap_ext::m2s_d2h_vap_info_syn fail}", hmac_vap->vap_id);
    }

#endif

    ret = hmac_config_vap_mib_update(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_ext:: fail! ret =[%d]", hmac_vap->vap_id, ret);
    }

    /* vap相关能力cap能力同步 */
    ret = hmac_config_vap_cap_update(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_add_vap_ext:: fail! ret =[%d]", hmac_vap->vap_id, ret);
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 添加vap的事件调度函数
 输入参数  : mac_vap : 业务vap，而不是配置VAP
           param   : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_add_vap(hmac_vap_stru *vap_new, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_cfg_add_vap_param_stru *param_temp = (mac_cfg_add_vap_param_stru *)msg->data;
    osal_s32 ret;

    if (param_temp->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* +7除以8（右移3位），表示按照字节补齐， +2，单独2个字节放置其他内容 */
        param_temp->tim_bitmap_len = (osal_u8)(2 + ((hmac_board_get_max_user() + 7) >> 3));
    }

    /* 先抛事件至Device侧DMAC，同步添加VAP，若device侧vap添加失败，则不添加ko侧vap */
    ret = frw_send_msg_to_device(hmac_device->cfg_vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_ADD_VAP, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap[%d]{hmac_config_add_vap:frw_send_msg_to_device fail[%d]}", param_temp->vap_id, ret);
        wifi_printf("vap_id[%d] {hmac_config_add_vap::frw_send_msg_to_device failed[%d].}", param_temp->vap_id, ret);
        return ret;
    }

     /* 业务启动，不考虑模式，通知work状态 */
    hal_pm_vote2platform(HAL_PM_WORK);

    ret = (osal_s32)hmac_config_add_vap_proc_ext(vap_new, param_temp, hmac_device);
    if (ret != OAL_SUCC) {
        return ret;
    }

#ifndef _PRE_BSLE_GATEWAY
    /* 初始化linkloss检测工具 */
    hmac_vap_linkloss_init(vap_new);
#endif

    ret = (osal_s32)hmac_config_add_vap_ext(param_temp, vap_new);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    hmac_clear_tx_nonqos_seq_num(vap_new);

#ifdef _PRE_WLAN_FEATURE_PMF
    vap_new->user_pmf_status = 0;
#endif /* #ifdef _PRE_WLAN_FEATURE_PMF */

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_config_del_vap_ext(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    const osal_u8 *param, osal_u8 vap_id)
{
    osal_void *fhook = OSAL_NULL;
#ifdef BOARD_ASIC_WIFI
    /* AP 删除刷新mac时钟,需要在hal device置空前完成 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap_exit_update_mac_clk(hmac_vap);
    }
#endif
    hmac_user_del_multi_user_etc(hmac_vap);

    /* 最后1个vap删除时，清除device级带宽信息 */
    if (hmac_device->vap_num == 0) {
        hmac_device->intol_bit_recd_40mhz = OSAL_FALSE;
    }

    /* 释放tim_bitmap */
    if (hmac_vap->tim_bitmap != OSAL_NULL) {
        oal_mem_free(hmac_vap->tim_bitmap, OAL_TRUE);
        hmac_vap->tim_bitmap = OSAL_NULL;
    }

    /* 共存恢复优先级 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY_CLEAR);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_recover_coex_priority_clear_cb)fhook)(hmac_vap, hmac_device);
    }

    /* 删除CSI相关配置 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSI_EXIT);
    if (fhook != OSAL_NULL) {
        ((hmac_csi_exit_cb)fhook)();
    }

    /* 静态/动态DBDC删除hal vap */
    hmac_vap_del_hal_device(hmac_vap, param);

    /* 删除vap时清空fakeq */
    hmac_vap_clear_fake_queue(hmac_vap);

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_del_vap_ext:: del vap[%d] success, multe user idx[%d].}",
        vap_id, hmac_vap->multi_user_idx);
}

#ifdef _PRE_WLAN_FEATURE_P2P
osal_void hmac_del_vap_p2pinfo_clear(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    hmac_cfg_del_vap_param_stru *del_vap_param)
{
    if (hmac_device == OSAL_NULL || hmac_vap == OSAL_NULL || del_vap_param == OSAL_NULL) {
        return;
    }

    if ((del_vap_param->add_vap.p2p_mode == WLAN_P2P_GO_MODE) ||
        (del_vap_param->add_vap.p2p_mode == WLAN_P2P_CL_MODE)) {
        /* 删除vap时清空P2P节能状态 */
        hmac_device->st_p2p_info.p2p_ps_pause = OSAL_FALSE;
        hmac_device_sync(hmac_device);
        (osal_void)memset_s(&(hmac_vap->p2p_ops_param), sizeof(mac_cfg_p2p_ops_param_stru),
            0, sizeof(mac_cfg_p2p_ops_param_stru));
        (osal_void)memset_s(&(hmac_vap->p2p_noa_param), sizeof(mac_cfg_p2p_noa_param_stru),
            0, sizeof(mac_cfg_p2p_noa_param_stru));
        /* 删除vap时清空P2P节能状态 */
        /* 停止P2P 节能寄存器 */
        hal_vap_set_noa(hmac_vap->hal_vap, 0, 0, 0, 0);
        hal_vap_set_ops(hmac_vap->hal_vap, 0, 0);
    }
}
#endif

/*****************************************************************************
 功能描述  : 删除vap
 输入参数 hmac_vap ap : 业务vap
           len     : 参数长度
           param   : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_del_vap(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_s32 ret;
    hmac_cfg_del_vap_param_stru *del_vap_param = (hmac_cfg_del_vap_param_stru *)msg->data;

    if (hmac_vap == OSAL_NULL || msg->data == OSAL_NULL) {
        return OAL_FAIL;
    }
    /* 配置VAP不做后续删除操作 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        return OAL_SUCC;
    }

    if (hmac_vap->hal_vap == OSAL_NULL) {
        return OAL_FAIL;
    }
    /* 删除vap前保存hal vap id */
    del_vap_param->add_vap.hal_vap_id = hmac_vap->hal_vap->vap_id;

    // 增强可靠性，vap删除时强制关闭TSF
    hal_disable_tsf_tbtt(hmac_vap->hal_vap);
    hmac_sta_stop_tbtt_timer(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_del_vap_p2pinfo_clear(hmac_device, hmac_vap, del_vap_param);
#endif
    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DEL_VAP, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_del_vap::frw_send_msg_to_device failed[%d].}", ret);
        wifi_printf("{hmac_config_del_vap::frw_send_msg_to_device failed[%d].}\r\n", ret);
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_vap->user_pmf_status = 0;
#endif /* #ifdef _PRE_WLAN_FEATURE_PMF */

#ifdef _PRE_WLAN_FEATURE_P2P
    if (del_vap_param->add_vap.p2p_mode != WLAN_P2P_DEV_MODE) {
#endif
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            /* 带宽切换状态机注销 */
            hmac_sta_bw_switch_fsm_detach(hmac_vap);
        }

        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* 删除AP 保护老化定时器 */
            hmac_protection_stop_timer(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
            /* 删除 40M恢复定时器 */
            hmac_chan_stop_40m_recovery_timer(hmac_vap);
#endif
        }

#ifdef _PRE_WLAN_FEATURE_P2P
    }
#endif

    hmac_config_del_vap_ext(hmac_vap, hmac_device, msg->data, hmac_vap->vap_id);
    return OAL_SUCC;
}

#ifndef _PRE_BSLE_GATEWAY
OSAL_STATIC osal_void hmac_config_start_vap_ext(hmac_vap_stru *hmac_vap, const hal_to_dmac_device_stru *hal_device)
{
    osal_char ssid[WLAN_SSID_MAX_LEN] = {'\0'};
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    errno_t ret_err = 0;
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    if ((hmac_vap == OSAL_NULL) || (hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap_ext::hmac_vap null}");
        return;
    }
#endif
    /* AP模式需要在start_vap时候通知bt，sta模式在关联成功后通知 */
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_START_VAP_EXIT, hmac_vap);

    oam_warning_log4(0, OAM_SF_CFG,
        "{hmac_config_start_vap_ext::start vap success.vap_mode[%d] state[%d], p2p mode[%d]bw[%d]", hmac_vap->vap_mode,
        hmac_vap->vap_state, hmac_vap->p2p_mode, mac_vap_get_cap_bw(hmac_vap));

    oam_warning_log4(0, OAM_SF_CHAN,
        "{hmac_config_start_vap_ext::hal vap id=%d, Channel=%d, band=%d, bandwidth=%d.}", hmac_vap->hal_vap->vap_id,
        hmac_vap->channel.chan_number, hmac_vap->channel.band, hmac_vap->channel.en_bandwidth);

    /* APUT主动发送prob req用于更新ht等信息  */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        osal_u32 ret;
        if (hmac_device == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap_ext::hmac_device is NULL}");
            return;
        }
        ret_err = memcpy_s((osal_void *)(hmac_device->scan_params.sour_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);
        if (ret_err != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap_ext::memcpy_s fail}");
            return;
        }
        ret = hmac_scan_send_probe_req_frame(hmac_vap, BROADCAST_MACADDR, ssid, WLAN_SSID_MAX_LEN, OAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_config_start_vap_ext::send_probe_request failed[%d].}",
                hmac_vap->vap_id, ret);
        }
    }

    /* vap创建之后，mib能力需要根据hal device重新刷新并同步到host侧 用于切换到siso之后，go等vap重新创建 */
    /* 用于切换到siso之后，go等vap重新创建 ,开关机时候，vap m2s能力同步到host */
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_config_start_vap_ext_m2s(hmac_vap, hal_device);
#endif
}

OSAL_STATIC osal_u32 hmac_config_start_vap_post_ext(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag,
    hal_to_dmac_device_stru *hal_device)
{
    osal_u32 ret;

    /* 初始化除单播数据帧以外所有帧的发送参数 */
    hmac_vap_init_tx_frame_params(hmac_vap, mgmt_rate_init_flag);

    /* 初始化vap侧发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_INIT);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 创建beacon帧 */
        /* 入网优化，不同频段下的能力不一样 */
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
            mac_mib_set_spectrum_management_required(hmac_vap, OSAL_FALSE);
        } else {
            mac_mib_set_spectrum_management_required(hmac_vap, OSAL_TRUE);
        }

        ret = hmac_beacon_alloc(hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_start_vap::beacon_alloc fail[%d].}", hmac_vap->vap_id, ret);
            return ret;
        }

        hmac_alg_vap_up_notify(hmac_vap);
    }

    hmac_config_start_vap_ext(hmac_vap, hal_device);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : offload情况下vap start
 输入参数  : mac_config_vap : dmac业务vap
 返 回 值: osal_u32
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_config_offload_start_vap(hmac_vap_stru *hmac_vap)
{
    hmac_vap_init_by_protocol_etc(hmac_vap, hmac_vap->protocol);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 设置bssid */
        (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);
        /* 将vap状态改变信息上报 */
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
#ifdef _PRE_WLAN_FEATURE_P2P
        /* P2P0 和P2P-P2P0 共VAP 结构，P2P CL UP时候，不需要设置vap 状态 */
        if (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE) {
#endif
            if (hmac_vap->vap_state != MAC_VAP_STATE_ROAMING) {
                // hmac_vap->channel.chan_number = 0; // 初始化信道号防止信道号跟频段出现不匹配的情况
                hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
            }
#ifdef _PRE_WLAN_FEATURE_P2P
        }
#endif
    }

    /* 初始化速率集 */
    hmac_vap_init_rates_etc(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : dmac启动vap
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_start_vap(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag)
{
    frw_msg msg;
    osal_u32 ret;

    (osal_void)memset_s(&msg, OAL_SIZEOF(msg), 0, OAL_SIZEOF(msg));

    oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap::.protocol[%d]}",
        hmac_vap->vap_id, hmac_vap->protocol);
#ifdef _PRE_WLAN_FEATURE_P2P
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_start_vap::p2p_mode[%d]}", hmac_vap->p2p_mode);
#endif

    ret = hmac_config_offload_start_vap(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_start_vap:: failed[%d].}", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    hal_pm_vote2platform(HAL_PM_WORK);

    /* 配置MAC EIFS_TIME 寄存器 */
    hal_config_eifs_time(hmac_vap->protocol);

    /* 使能PA_CONTROL的vap_control位: bit0~3对应AP0~3 bit4对应sta */
    hal_vap_set_opmode(hmac_vap->hal_vap, hmac_vap->vap_mode);

#ifndef _PRE_BSLE_GATEWAY
    ret = hmac_config_start_vap_post_ext(hmac_vap, mgmt_rate_init_flag, hmac_vap->hal_device);
    if (ret != OAL_SUCC) {
        return (osal_s32)ret;
    }

    // vap up关闭协议低功耗
    hmac_config_mvap_set_sta_pm_on(hmac_vap, OSAL_TRUE);
#endif

    /* 抛事件至Device侧DMAC，同步添加VAP */
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_START_VAP, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_start_vap::vap[%d] msg failed[%d].}", hmac_vap->vap_id, ret);
        wifi_printf("{hmac_config_start_vap::vap[%d] msg failed[%d].}\r\n", hmac_vap->vap_id, ret);
    }
    return (osal_s32)ret;
}

OSAL_STATIC osal_void hmac_config_down_del_vap(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry;
    struct osal_list_head *dlist_tmp;

    /* host删除动作可能因为抛事件异常而在device侧没有执行，dmac_cofig_down_vap时候确认下是否还有用户，做一次保护 */
    if (hmac_vap->user_nums != 0) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_down_del_vap:nums[%d] isnot 0.}", hmac_vap->vap_id,
            hmac_vap->user_nums);

        /* 遍历vap下所有用户, 删除用户, 防止host侧删除用户异常，造成device用户未删除成功，在down的时候做一次删除保护 */
        osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head))
        {
            hmac_user_stru *user_tmp = osal_list_entry(entry, hmac_user_stru, user_dlist);
            if (user_tmp == OSAL_NULL) {
                oam_error_log1(0, 0, "vap_id[%d] {hmac_config_down_del_vap::user_tmp null.}", hmac_vap->vap_id);
                continue;
            }
            hmac_user_del_etc(hmac_vap, user_tmp);
        }
    }
}

OSAL_STATIC osal_void hmac_config_down_vap_ext(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user)
{
    unref_param(hmac_device);
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 节能队列不为空的情况下，释放节能队列中的mpdu */
        hmac_psm_clear_ps_queue(hmac_user);
#ifdef _PRE_WLAN_FEATURE_M2S
        /* sta去关联时，要vap能力回到mimo默认最大能力 */
        if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap)) {
            hmac_m2s_spec_update_vap_sw_capbility(hmac_device, hmac_vap, WLAN_DOUBLE_NSS);
        }
#endif

        /* 释放beacon帧 */
        hmac_beacon_free(hmac_vap);

        hmac_vap_down_notify(hmac_vap);

#ifdef _PRE_WLAN_FEATURE_DFS
        hal_enable_radar_det(0);
#endif
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 关闭sta tsf定时器 */
        hal_disable_tsf_tbtt(hmac_vap->hal_vap);
        hmac_sta_stop_tbtt_timer(hmac_vap);
    }
}

/*****************************************************************************
 功能描述  : 停用vap
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_down_vap(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    unref_param(msg);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_config_down_vap:user[%d] null}", hmac_vap->multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_scan_destroy_obss_timer(hmac_vap);
#endif

    hmac_config_down_del_vap(hmac_vap);

    /* 删除组播用户tid队列中的所有信息 */
    hmac_tid_clear(hmac_user, hmac_device, OSAL_TRUE);

    /* 释放VAP下挂接发送队列的报文, user_del时没有释放，在此按vap统一释放 */
    hmac_vap_clear_tx_queue(hmac_vap);

    /* 清除PA_CONTROL的vap_control位: bit0~3对应AP0~3 bit4对应sta */
    hal_vap_clr_opmode(hmac_vap->hal_vap, hmac_vap->vap_mode);

    hmac_config_down_vap_ext(hmac_device, hmac_vap, hmac_user);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    /* 删除 40M恢复定时器 */
    hmac_chan_stop_40m_recovery_timer(hmac_vap);
#endif

    /* 通知扫描特性 */
    hmac_mgmt_scan_vap_down(hmac_vap);

    oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_down_vap::down vap succ.hmac_vap mode[%d] state[%d].}",
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->vap_state);

    hmac_config_update_up_vap_num();

    // vap down, 设置另一个up vap的低功耗
    hmac_config_mvap_set_sta_pm_on(hmac_vap, OSAL_FALSE);

#ifdef _PRE_WLAN_DFT_STAT
    hmac_dft_vap_stat_deinit(hmac_vap->vap_id);
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WIFI_DEBUG
/*****************************************************************************
 函 数 名  : hmac_config_vap_info
 功能描述  : 显示vap信息
*****************************************************************************/
osal_s32 hmac_config_vap_info(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    mac_protection_stru *pt = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_vap_info::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_vap_info:: hal_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (hmac_user != OSAL_NULL)) {
        oam_warning_log4(0, OAM_SF_CFG, "vap bandwidth=%d, user band_cap=%d,avail band=%d, cur band=%d.",
            hmac_vap->channel.en_bandwidth, hmac_user->bandwidth_cap,
            hmac_user->avail_bandwidth, hmac_user->cur_bandwidth);
        oam_warning_log4(0, OAM_SF_CFG, "user id=%d,user nss_num=%d, avail nss=%d, avail bf nss=%d.",
            hmac_vap->assoc_vap_id, hmac_user->user_num_spatial_stream, hmac_user->avail_num_spatial_stream,
            hmac_user->avail_bf_num_spatial_stream);
    }

    oam_warning_log4(0, OAM_SF_CFG,
        "mac_vap_state=%d, mac vap nss_num=%d, hmac_device nss=%d, hal device nss=%d.", hmac_vap->vap_state,
        hmac_vap->vap_rx_nss, hmac_device->device_cap.nss_num, hal_device->cfg_cap_info->nss_num);
    oam_warning_log4(0, OAM_SF_CFG, "vap_id[%d] device phy_chain=%d, device single_tx_chain=%d, device rf_chain=%d.",
        hmac_vap->vap_id, hal_device->cfg_cap_info->phy_chain, hal_device->cfg_cap_info->single_tx_chain,
        hal_device->cfg_cap_info->rf_chain);
    oam_warning_log3(0, OAM_SF_CFG,
        "{hmac_config_vap_info::current mac vap id[%d]in device state[%d]device workbitmap[%d]}", hmac_vap->vap_id,
        hmac_vap->hal_device->hal_dev_fsm.oal_fsm.cur_state, hmac_vap->hal_device->work_vap_bitmap);

    // print protection info
    pt = &hmac_vap->protection;
    oam_info_log4(0, OAM_SF_CFG,
        "en_protection_mode=%d. obss_non_erp_aging_cnt=%d. obss_non_ht_aging_cnt=%d. auto_protection=%d",
        pt->protection_mode, pt->obss_non_erp_aging_cnt, pt->obss_non_ht_aging_cnt, pt->auto_protection);

    oam_info_log3(0, OAM_SF_CFG,
        "bit_obss_non_erp_present=%d. obss_non_ht_present=%d. lsig_txop_protect_mode=%d.",
        pt->obss_non_erp_present, pt->obss_non_ht_present, pt->lsig_txop_protect_mode);

    oam_info_log4(0, OAM_SF_CFG,
        "uc_sta_no_short_slot_num=%d. sta_no_short_preamble_num=%d. sta_non_erp_num=%d. sta_non_ht_num=%d.",
        pt->sta_no_short_slot_num, pt->sta_no_short_preamble_num, pt->sta_non_erp_num, pt->sta_non_ht_num);

    oam_info_log4(0, OAM_SF_CFG,
        "uc_sta_non_gf_num=%d. sta_20m_only_num=%d. sta_no_40dsss_cck_num=%d. sta_no_lsig_txop_num=%d. ",
        pt->sta_non_gf_num, pt->sta_20m_only_num, pt->sta_no_40dsss_cck_num, pt->sta_no_lsig_txop_num);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_vap_sync
 功能描述  : 同步dmac_vap信息
*****************************************************************************/
osal_s32 hmac_vap_sync(hmac_vap_stru *hmac_vap)
{
    frw_msg msg_info = {0};
    mac_update_dmac_vap_stru vap_info_sync = {0};
    osal_s32 ret;

    if (hmac_vap == OSAL_NULL || hmac_vap->init_flag != MAC_VAP_VAILD) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_vap_sync:: hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步dmac_vap */
    vap_info_sync.vap_state = hmac_vap->vap_state;
    vap_info_sync.sta_aid = hmac_vap->sta_aid;
    vap_info_sync.uapsd_cap = hmac_vap->uapsd_cap;
    vap_info_sync.bcn_tout_max_cnt = hmac_vap->bcn_tout_max_cnt;
    vap_info_sync.in_tbtt_offset = hmac_vap->in_tbtt_offset;
    vap_info_sync.beacon_timeout = hmac_vap->beacon_timeout;
    vap_info_sync.beacon_interval = (osal_u16)mac_mib_get_beacon_period(hmac_vap);
    memcpy_s(vap_info_sync.vap_mac, WLAN_MAC_ADDR_LEN, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);
    vap_info_sync.auth_received = hmac_vap->auth_received;
    vap_info_sync.assoc_rsp_received = hmac_vap->assoc_rsp_received;
    vap_info_sync.protocol = hmac_vap->protocol;
    vap_info_sync.keepalive = hmac_vap->cap_flag.keepalive;
    vap_info_sync.linkloss_rate = hmac_vap->linkloss_info.linkloss_rate;
    memcpy_s(&vap_info_sync.channel, sizeof(mac_channel_stru), &hmac_vap->channel, sizeof(mac_channel_stru));

    /* 抛事件至Device侧DMAC，同步dmac_user_stru */
    frw_msg_init((osal_u8 *)&vap_info_sync, sizeof(mac_update_dmac_vap_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_VAP, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_vap_sync::vap[%d] sync msg failed[%d].}", hmac_vap->vap_id, ret);
        wifi_printf("{hmac_vap_sync::vap[%d] sync msg failed[%d].}\r\n", hmac_vap->vap_id, ret);
    }
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
