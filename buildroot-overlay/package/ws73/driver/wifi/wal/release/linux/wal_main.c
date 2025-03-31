/*
 * Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.
 * Description: wal function with the kernel interface.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wal_main.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hmac_ext_if.h"
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ccpriv.h"
#ifdef _PRE_WLAN_WIRELESS_EXT
#include "wal_linux_iwhandler.h"
#endif
#include "oal_cfg80211.h"
#include "oal_netbuf.h"
#include "wal_config_acs.h"

#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
#ifndef WIFI_LITE_EXTREME
#include "oal_kernel_file.h"
#endif
#endif
#include "hmac_vap.h"
#include "wal_config_sdp.h"
#include "common_dft.h"
#include "wlan_msg.h"

#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#endif
#include "wal_linux_netdev.h"
#include "wal_linux_vap_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_MAIN_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_wakelock_stru   g_st_wal_wakelock_etc;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : wal_event_fsm_init_etc
 功能描述  : 注册事件处理函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无

 修改历史      :
  1.日    期   : 2012年11月16日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void wal_event_init(osal_void)
{
    static const msg_hook_array_stru msg_hook_array[] = {
        {WLAN_MSG_H2W_SCAN_COMP_STA, wal_scan_comp_proc_sta_etc},
        {WLAN_MSG_H2W_STA_CONNECT_AP, wal_connect_new_sta_proc_ap_etc},
        {WLAN_MSG_H2W_STA_DISCONNECT_AP, wal_disconnect_sta_proc_ap_etc},
        {WLAN_MSG_H2W_RX_MGMT, wal_send_mgmt_to_host_etc},
        {WLAN_MSG_H2W_ASOC_COMP_STA, wal_asoc_comp_proc_sta_etc},
        {WLAN_MSG_H2W_RX_CSA_DONE, wal_report_channel_switch_csa},
#ifdef _PRE_WLAN_FEATURE_ROAM
        {WLAN_MSG_H2W_ROAM_COMP_STA, wal_roam_comp_proc_sta_etc},
#endif /* _PRE_WLAN_FEATURE_ROAM */
        {WLAN_MSG_H2W_SDP_RX_DATA, wal_sdp_process_rx_data},
#ifdef _PRE_WLAN_FEATURE_M2S
        {WLAN_MSG_H2W_M2S_STATUS, wal_cfg80211_m2s_status_report},
#endif
        {WLAN_MSG_H2W_MGMT_TX_STATUS, wal_cfg80211_mgmt_tx_status_etc},
        {WLAN_MSG_H2W_RX_CSI, wal_report_csi_msg},
#ifdef _PRE_WLAN_WIRELESS_EXT
        {WLAN_MSG_H2W_IWLIST_SCAN_RSP, wal_wait_hmac_sync_data},
#endif
        {WLAN_MSG_H2W_INSMOD_KO_FINISH, wal_insmod_ko_finish},
    };

    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
}

osal_void wal_event_fsm_init_etc(osal_void)
{
    frw_msg_hook_register(WLAN_MSG_H2W_DISASOC_COMP_STA, wal_disasoc_comp_proc_sta_etc);
    frw_msg_hook_register(WLAN_MSG_H2W_MIC_FAILURE, wal_mic_failure_proc_etc);
#ifdef _PRE_WLAN_FEATURE_P2P
    frw_msg_hook_register(WLAN_MSG_H2W_LISTEN_EXPIRED, wal_p2p_listen_timeout_etc);
#endif
#ifdef _PRE_WLAN_FEATURE_11R
    frw_msg_hook_register(WLAN_MSG_H2W_FT_EVENT_STA, wal_ft_event_proc_sta_etc);
#endif /* _PRE_WLAN_FEATURE_11R */

#ifdef _PRE_WLAN_FEATURE_DFS
    frw_msg_hook_register(WLAN_MSG_H2W_CAC_REPORT, wal_cfg80211_cac_report);
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    frw_msg_hook_register(WLAN_MSG_H2W_EXTERNAL_AUTH, wal_cfg80211_trigger_external_auth);
    frw_msg_hook_register(WLAN_MSG_H2W_STA_OWE_INFO, wal_cfg80211_report_owe_info);
#endif
}
osal_void wal_wake_lock(osal_void)
{
    oal_wake_lock(&g_st_wal_wakelock_etc);
}

osal_void wal_wake_unlock(osal_void)
{
    oal_wake_unlock(&g_st_wal_wakelock_etc);
}

/*****************************************************************************
 函 数 名  : wal_event_fsm_exit_etc
 功能描述  : 卸载事件处理函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC

 修改历史      :
  1.日    期   : 2012年12月28日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  wal_event_fsm_exit_etc(osal_void)
{
    return OAL_SUCC;
}

#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
/* debug sysfs */
OAL_STATIC oal_kobject *g_conn_syfs_wal_object = NULL;

osal_s32 wal_wakelock_info_print_etc(char *buf, osal_s32 buf_len)
{
    osal_s32 ret = 0;

#ifdef CONFIG_PRINTK
    if (g_st_wal_wakelock_etc.locked_addr) {
        ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "wakelocked\n");
    }
#endif

    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "hold %lu locks\n", g_st_wal_wakelock_etc.lock_count);

    return ret;
}

OAL_STATIC ssize_t  wal_get_wakelock_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL || attr == NULL || dev == NULL) {
        wifi_printf("wal_get_wakelock_info::param is null!\n");
        return 0;
    }

    ret += wal_wakelock_info_print_etc(buf, PAGE_SIZE - ret);

    return ret;
}

/* 查询接收数据包的个数 */
OAL_STATIC osal_s32  uapi_atcmdsrv_ioctl_get_rx_pckg_etc(oal_net_device_stru *net_dev, osal_s32 *rx_pckg_succ_num)
{
    osal_s32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_atcmdsrv_ioctl_get_dbb_num::dev->ml_priv, return null!}");
        return -OAL_EINVAL;
    }

    ret = wal_get_rx_pckg(hmac_vap, 1, 0);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_atcmdsrv_ioctl_get_rx_rssi:: return err code %d!}", ret);
        return ret;
    }

    /* 正常结束  */
    *rx_pckg_succ_num = (osal_slong)hmac_vap->atcmdsrv_get_status.rx_pkct_succ_num;
    return OAL_SUCC;
}
OAL_STATIC ssize_t  wal_get_packet_statistics_wlan0_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t                     ret = 0;
    oal_net_device_stru        *net_dev;
    hmac_vap_stru              *hmac_vap;
    osal_s32                   l_rx_pckg_succ_num;
    osal_s32                   l_ret;

    if (buf == NULL || attr == NULL || dev == NULL) {
        wifi_printf("get_packet_statistics_wlan0_info::param is null!\n");
        return 0;
    }

    net_dev = oal_dev_get_by_name(wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX));
    if (net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "{wal_packet_statistics_wlan0_info_print::oal_dev_get_by_name return null ptr!}");
        return ret;
    }
    oal_dev_put(net_dev);   /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    /* 获取VAP结构体 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    /* 如果VAP结构体不存在，返回0 */
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::hmac_vap = OAL_PTR_NULL!}");
        return ret;
    }
    /* 非STA直接返回 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_packet_statistics_wlan0_info_print::vap_mode:%d.}",
            hmac_vap->vap_mode);
        return ret;
    }

    l_ret = uapi_atcmdsrv_ioctl_get_rx_pckg_etc(net_dev, &l_rx_pckg_succ_num);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_packet_statistics_wlan0_info_print::uapi_atcmdsrv_ioctl_get_rx_pckg_etc err:%d!}", l_ret);
        return ret;
    }

    ret +=  snprintf_s(buf, PAGE_SIZE, PAGE_SIZE, "rx_packet=%d\r\n", l_rx_pckg_succ_num);

    return ret;
}

OAL_STATIC DEVICE_ATTR(wakelock, S_IRUGO, wal_get_wakelock_info, NULL);
OAL_STATIC DEVICE_ATTR(packet_statistics_wlan0, S_IRUGO, wal_get_packet_statistics_wlan0_info, NULL);

OAL_STATIC ssize_t  wal_get_dev_wifi_info_print(char *buf, osal_s32 buf_len)
{
    ssize_t                     ret = 0;
    oal_net_device_stru        *net_dev;
    hmac_vap_stru              *hmac_vap;

    if (buf == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_get_dev_wifi_info_print error: buf is null");
        return 0;
    }

    net_dev = oal_dev_get_by_name(wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX));
    if (net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::oal_dev_get_by_name return null ptr!}");
        return ret;
    }
    oal_dev_put(net_dev);   /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    /* 获取VAP结构体 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    /* 如果VAP结构体不存在，返回0 */
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::hmac_vap = OAL_PTR_NULL!}");
        return ret;
    }
    /* 非STA直接返回 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_dev_wifi_info_print::vap_mode:%d.}", hmac_vap->vap_mode);
        return ret;
    }

    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "tx_frame_amount:%u\n",
        hmac_vap->station_info.tx_packets);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "tx_byte_amount:%u\n",
        (osal_u32)hmac_vap->station_info.tx_bytes);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "tx_data_frame_error_amount:%u\n",
        hmac_vap->station_info.tx_failed);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "tx_retrans_amount:%u\n",
        hmac_vap->station_info.tx_retries);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "rx_frame_amount:%u\n",
        hmac_vap->station_info.rx_packets);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "rx_byte_amount:%u\n",
        (osal_u32)hmac_vap->station_info.rx_bytes);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "ap_distance:%u\n",
        hmac_vap->station_info_extend.distance);
    ret +=  snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "disturbing_degree:%u\n",
        hmac_vap->station_info_extend.cca_intr);

    return ret;
}
OAL_STATIC ssize_t  wal_get_dev_wifi_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL || attr == NULL || dev == NULL) {
        wifi_printf("wal_get_dev_wifi_info::param is null!\n");
        return 0;
    }

    ret += wal_get_dev_wifi_info_print(buf, PAGE_SIZE - ret);

    return ret;
}
OAL_STATIC DEVICE_ATTR(dev_wifi_info, S_IRUGO, wal_get_dev_wifi_info, NULL);

OAL_STATIC struct attribute *wal_sysfs_entries[] = {
    &dev_attr_wakelock.attr,
    &dev_attr_packet_statistics_wlan0.attr,
    &dev_attr_dev_wifi_info.attr,
    NULL
};

OAL_STATIC struct attribute_group wal_attribute_group = {
    .attrs = wal_sysfs_entries,
};

OAL_STATIC osal_s32 wal_sysfs_entry_init(osal_void)
{
    osal_s32       ret;
    oal_kobject     *root_object = NULL;
    root_object = oal_get_sysfs_root_object_etc();
    if (root_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::get sysfs root object failed!}");
        return -OAL_EFAIL;
    }

    g_conn_syfs_wal_object = kobject_create_and_add("wal", root_object);
    if (g_conn_syfs_wal_object == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::create wal object failed!}");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(g_conn_syfs_wal_object, &wal_attribute_group);
    if (ret) {
        kobject_put(g_conn_syfs_wal_object);
        oam_error_log0(0, OAM_SF_ANY, "{wal_sysfs_entry_init::sysfs create group failed!}");
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 wal_sysfs_entry_exit(osal_void)
{
    if (g_conn_syfs_wal_object) {
        sysfs_remove_group(g_conn_syfs_wal_object, &wal_attribute_group);
        kobject_put(g_conn_syfs_wal_object);
    }
    return OAL_SUCC;
}
#endif /* #ifndef WIFI_LITE_EXTREME */
#endif

DEFINE_GET_BUILD_VERSION_FUNC(wal);

/*****************************************************************************
 函 数 名  : wal_main_init_etc
 功能描述  : WAL模块初始化总入口，包含WAL模块内部所有特性的初始化。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 初始化返回值，成功或失败原因
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2012年9月18日
    修改内容   : 新生成函数

*****************************************************************************/


#ifdef CONTROLLER_CUSTOMIZATION
typedef oal_ssize_t (* sys_write_ptr)(struct kobject *kobj, struct kobj_attribute *attr,
    const char *pc_buffer, size_t count);
sys_write_ptr g_sys_write_ptr;
typedef oal_ssize_t (* sys_read_ptr)(struct kobject *kobj, struct kobj_attribute *attr, char *pc_buffer);
sys_read_ptr g_sys_read_ptr;

ssize_t uapi_ccpriv_sys_write(struct kobject *kobj, struct kobj_attribute *attr, const char *pc_buffer, size_t count);
#else
typedef oal_ssize_t (* sys_write_ptr)(oal_device_stru *dev, oal_device_attribute_stru *attr,
    const char *pc_buffer, oal_size_t count);
sys_write_ptr g_sys_write_ptr;
typedef oal_ssize_t (* sys_read_ptr)(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer);
sys_read_ptr g_sys_read_ptr;
#endif
osal_void uapi_set_plat_ccpriv_opts(sys_write_ptr write_ptr, sys_read_ptr read_ptr);

/* reboot 业务清理 */
static osal_void wlan_reboot_srv_clean(osal_void)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_net_device_stru *netdev = OSAL_NULL;

    for (vap_idx = 0; vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)hmac_get_vap(vap_idx);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }
        netdev = hmac_vap->net_device;
        if (netdev == OSAL_NULL) {
            continue;
        }
        if (netdev->ieee80211_ptr == OSAL_NULL) {
            continue;
        }

        // 标识netdev关闭
        oal_net_device_close(netdev);
    }

    frw_main_destroy();
}

osal_s32  wal_main_init_etc(osal_void)
{
    osal_u32           ul_ret;
    frw_init_enum_uint16 init_state;

    oal_wake_lock_init(&g_st_wal_wakelock_etc, "wlan_wal_lock");

    init_state = frw_get_init_state_etc();
    /* WAL模块初始化开始时，说明HMAC肯定已经初始化成功 */
    if ((init_state == FRW_INIT_STATE_BUTT) || (init_state < FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_main_init_etc::init_state has a invalid value [%d]!}", init_state);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    wal_event_fsm_init_etc();
    wal_event_init();
    wal_cfg_init();
#ifdef _PRE_WLAN_CFGID_DEBUG
    wal_cfg_debug_init();
#endif

    uapi_set_plat_ccpriv_opts(uapi_ccpriv_sys_write, uapi_ccpriv_sys_read);

    /* 初始化每个device硬件设备对应的wiphy */
    ul_ret = wal_cfg80211_init_etc();
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_main_init_etc::wal_cfg80211_init_etc has a wrong return value [%d]!}",
                         ul_ret);

        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* 在host侧如果WAL初始化成功，即为全部初始化成功 */
    frw_set_init_state_etc(FRW_INIT_STATE_ALL_SUCC);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* DTSxxxxxx 初始化cfg80211 删除网络设备工作队列 */
#endif /* _PRE_WLAN_FEATURE_P2P */

#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
    /* debug sysfs */
    wal_sysfs_entry_init();
#endif
#endif

    plat_register_reboot(wlan_reboot_srv_clean, WLAN_REBOOT_NOTIFY);
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_init();
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wal_set_custom_process_func_etc();
#endif

    return OAL_SUCC;
}

OAL_STATIC osal_u32 wal_ccpriv_del_vap_send_event(hmac_cfg_del_vap_param_stru *vap_info, oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru *wdev;
    osal_s32 ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8   p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    vap_info->net_dev = net_dev;
    wdev = net_dev->ieee80211_ptr;
#ifdef _PRE_WLAN_FEATURE_P2P
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_del_vap_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    vap_info->add_vap.p2p_mode = p2p_mode;
#endif

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_DEL_VAP,
        (osal_u8 *)vap_info, OAL_SIZEOF(hmac_cfg_del_vap_param_stru));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_del_vap_etc::return err code %d}", ret);
        wifi_printf("{uapi_ccpriv_del_vap_etc::return err code %d}\r\n", ret);
        /* 去注册 */
        oal_net_unregister_netdev(net_dev);
        oal_mem_free(wdev, OAL_TRUE);
        return (osal_u32)ret;
    }
    /* 去注册 */
    oal_net_unregister_netdev(net_dev);
    oal_mem_free(wdev, OAL_TRUE);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除vap
 输入参数  : net_dev : net_device
             pc_param    : 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年5月13日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 uapi_ccpriv_del_vap_etc(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap;
    hmac_cfg_del_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    if (OAL_UNLIKELY((net_dev == OAL_PTR_NULL) || (param == OAL_PTR_NULL))) {
        // 访问网络接口的模块可能不止一个,需要上层保证可靠删除
        oam_warning_log2(0, OAM_SF_ANY, "{uapi_ccpriv_del_vap_etc::null %p %p!}", (uintptr_t)net_dev, (uintptr_t)param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(NULL == hmac_vap)) {
        oam_error_log0(0, OAM_SF_CFG, "{uapi_ccpriv_del_vap_etc::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (WLAN_VAP_MODE_CONFIG == hmac_vap->vap_mode) { /* 配置vap不走命令删除接口 */
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_del_vap_etc::invalid parameters, mac vap mode: %d}",
                         hmac_vap->vap_mode);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 设备在up状态不允许删除，必须先down */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((hmac_vap->vap_state != MAC_VAP_STATE_INIT) && (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE))
#else
    if (hmac_vap->vap_state != MAC_VAP_STATE_INIT)
#endif
    {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_del_vap_etc:vap state[%d],down vap}", hmac_vap->vap_state);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    fastip_detach_wifi(hmac_vap->fastip_idx);
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */

    /* 删除vap对应的proc文件 */
#ifdef LINUX_VERSION_CODE
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34)
    wal_del_vap_proc_file(net_dev);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59)
    wal_del_vap_proc_file_sec(net_dev);
#endif
#endif

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    // 删除vap 时需要将参数赋值。
    ret = wal_ccpriv_del_vap_send_event(&vap_info, net_dev);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_destroy_all_vap_etc
 功能描述  : 卸载前删除所有vap
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月28日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void  wal_destroy_all_vap_etc(osal_void)
{
#if defined(_PRE_TEST_MODE_UT) && defined(_PRE_TEST_MODE) && (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)

    osal_u8               vap_id;
    oal_net_device_stru    *net_dev;
    osal_s8                ac_param[10] = {0};
    oam_warning_log0(0, OAM_SF_ANY, "wal_destroy_all_vap_etc start.");

    /* 删除业务vap，双芯片id从2开始，增加编译宏表示板级业务vap起始id 后续业务vap的处理，采用此宏  */
    for (vap_id = 1; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        net_dev = hmac_vap_get_net_device_etc(vap_id);
        if (net_dev != OAL_PTR_NULL) {
            oal_net_close_dev(net_dev);
            uapi_ccpriv_del_vap_etc(net_dev, ac_param);
            frw_event_process_all_event_etc(0);
        }
    }
#endif
    return;
}

/*****************************************************************************
 函 数 名  : wal_main_exit_etc
 功能描述  : WAL模块卸载
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 模块卸载返回值，成功或失败原因
 调用函数  : 无
 被调函数  : 无

 修改历史      :
  1.日    期   : 2012年9月18日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void  wal_main_exit_etc(osal_void)
{
    plat_register_reboot(OSAL_NULL, WLAN_REBOOT_NOTIFY);
#ifdef _PRE_WLAN_FEATURE_DFR
    wal_dfx_exit();
#endif
#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
    /* debug sysfs */
    wal_sysfs_entry_exit();
#endif
#endif
    /* down掉所有的vap */
    wal_destroy_all_vap_etc();

    /* 卸载每个device硬件设备对应的wiphy */
    wal_cfg80211_exit_etc();

    wal_event_fsm_exit_etc();
    /* 卸载成功时，将初始化状态置为HMAC初始化成功 */
    frw_set_init_state_etc(FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC);

    /* 卸载时屏蔽ccpriv入口 */
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    uapi_set_plat_ccpriv_opts(NULL, NULL);
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    /* DTSxxxxxx 删除cfg80211 删除网络设备工作队列 */
#endif

    oal_wake_lock_exit(&g_st_wal_wakelock_etc);
    frw_timer_clean_timer(OAM_MODULE_ID_WAL);
}

oal_module_symbol(wal_main_init_etc);
oal_module_symbol(wal_main_exit_etc);

oal_module_license("GPL");

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

