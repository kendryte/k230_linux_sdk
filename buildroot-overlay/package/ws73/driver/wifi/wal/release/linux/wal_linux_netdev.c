/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal net dev api.
 * Create: 2021-07-19
 */

#include <net/addrconf.h>
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include "oal_net.h"
#include "oam_struct.h"
#include "mac_resource_ext.h"
#include "hmac_config.h"
#include "mac_vap_ext.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "hmac_ext_if.h"

#include "wal_main.h"
#include "wal_linux_util.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_android_cmd.h"
#include "wal_linux_ioctl.h"
#if ((defined(_PRE_PRODUCT_ID_HIMPXX_DEV)) || (defined(_PRE_PRODUCT_ID_HOST)))
#include "hmac_cali_mgmt.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_netbuf.h"
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "hcc_host_if.h"
#include "plat_cali.h"
#endif
#if ((!defined(_PRE_PRODUCT_ID_HIMPXX_DEV)) && (!defined(_PRE_PRODUCT_ID_HOST)))
#include "wal_linux_witp_cmd.h"
#endif
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#include "hmac_vap.h"
#endif
#include "wlan_msg.h"
#include "hmac_dfr.h"
#include "hmac_roam_alg.h"
#include "frw_util_notifier.h"
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#endif
#include "wal_linux_netdev.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_NETDEV_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define NETDEV_WAIT_WIFI_INSMOD_TIMEOUT 20000
osal_s8 g_wlan_name[NETDEV_MAC_INDEX_BUTT][OAL_IF_NAME_SIZE]; /* 接口名称字符串数组 */
osal_u8 g_wlan_mac[NETDEV_MAC_INDEX_BUTT][WLAN_MAC_ADDR_LEN];
oal_completion g_insmod_ready; /* insmod wifi ko完成标志，未完成时不允许启动netdev */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* 每次上下电由配置vap完成的定制化只配置一次，wlan p2p iface不再重复配置 */
OAL_STATIC osal_u8 g_uc_cfg_once_flag = OAL_FALSE;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
wal_ap_config_stru g_st_ap_config_info = {0};    /* AP配置信息,需要在vap 创建后下发的 */

osal_s32 wal_ccpriv_inetaddr_notifier_call_etc(struct notifier_block *this, osal_ulong event, osal_void *ptr);

OAL_STATIC struct notifier_block wal_ccpriv_notifier = {
    .notifier_call = wal_ccpriv_inetaddr_notifier_call_etc
};

osal_s32 wal_ccpriv_inet6addr_notifier_call_etc(struct notifier_block *this, osal_ulong event, osal_void *ptr);

OAL_STATIC struct notifier_block wal_ccpriv_notifier_ipv6 = {
    .notifier_call = wal_ccpriv_inet6addr_notifier_call_etc
};

STATIC osal_s32 wal_deinit_wlan_vap_etc(oal_net_device_stru *net_dev);
OAL_STATIC osal_s32 wal_config_mac_filter(oal_net_device_stru *net_dev, osal_s8 *command);

/* 获取从ini配置项读取到的net dev name */
osal_s8 *wal_get_wlan_name_config(osal_u8 netdev_index)
{
    if (netdev_index >= NETDEV_MAC_INDEX_BUTT) {
        return OSAL_NULL;
    }
    return g_wlan_name[netdev_index];
}

/* 从ini配置项读取到的net dev ifname 写入全局变量失败时, 设置缺省name */
osal_void wal_set_wlan_name_default(osal_u8 netdev_index)
{
    osal_u32 len;
    osal_s8 wlan_name_default[NETDEV_MAC_INDEX_BUTT][OAL_IF_NAME_SIZE] = {
        "wlan0",
#ifdef CONTROLLER_CUSTOMIZATION
        "ap0",
#else
        "wlan1",
#endif
        "p2p0"};

    if (netdev_index >= NETDEV_MAC_INDEX_BUTT) {
        return;
    }

    len = sizeof(g_wlan_name[netdev_index]);
    memset_s(g_wlan_name[netdev_index], len, 0, len);
    (void)memcpy_s(g_wlan_name[netdev_index], len,
        wlan_name_default[netdev_index], osal_strlen(wlan_name_default[netdev_index]));

    return;
}

/* 从ini配置项读取到的net dev name 写入全局变量中 */
osal_void wal_set_wlan_name_config(osal_u8 netdev_index, osal_s8 *name, osal_u8 len)
{
    osal_u32 name_size;

    if (netdev_index >= NETDEV_MAC_INDEX_BUTT) {
        return;
    }

    name_size = sizeof(g_wlan_name[netdev_index]);
    (void)memset_s(g_wlan_name[netdev_index], name_size, 0, name_size);

    if (memcpy_s(g_wlan_name[netdev_index], sizeof(g_wlan_name[netdev_index]), name, len) != EOK) {
        oam_warning_log2(0, 0, "{wal_set_wlan_name_config::memcpy fail, index %d, len %d!}", netdev_index, len);

        wal_set_wlan_name_default(netdev_index);
    }

#ifdef CONTROLLER_CUSTOMIZATION
    /* 如果不是默认值wlan1,说明用户做了配置,则不修改为ap0 */
    if ((netdev_index == NETDEV_AP_MAC_INDEX) &&
        (osal_strcmp("wlan1", (const osal_s8 *)g_wlan_name[netdev_index]) == 0)) {
        (void)memset_s(g_wlan_name[netdev_index], name_size, 0, name_size);
        (void)memcpy_s(g_wlan_name[netdev_index], sizeof(g_wlan_name[netdev_index]), "ap0", osal_strlen("ap0"));
    }
#endif

    wifi_printf("{wal_set_wlan_name_config::g_wlan_name[%d]=%s}\r\n", netdev_index, g_wlan_name[netdev_index]);

    return;
}

wal_ap_config_stru *wal_get_ap_config_info(osal_void)
{
    return &g_st_ap_config_info;
}

osal_s32  wal_cfg_vap_h2d_event_etc(oal_net_device_stru *net_dev)
{
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_wireless_dev_stru      *wdev;
    mac_wiphy_priv_stru        *wiphy_priv;
    hmac_vap_stru              *cfg_hmac_vap;
    hmac_device_stru           *hmac_device;
    osal_s32                   l_ret;
    mac_cfg_vap_h2d_stru mac_cfg_info = {0};

    wdev = OAL_NETDEVICE_WDEV(net_dev);
    if (wdev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::wdev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_device  = wiphy_priv->hmac_device;
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::hmac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    if (cfg_hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::mac_res_get_hmac_vap fail.vap_id[%u]}",
                         hmac_device->cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (cfg_hmac_vap->net_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg_vap_h2d_event_etc::cfg_net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    抛事件到wal层处理
    ***************************************************************************/
    mac_cfg_info.net_dev = cfg_hmac_vap->net_device;
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_hmac_vap->net_device), WLAN_MSG_W2H_CFG_CFG_VAP_H2D,
        (osal_u8 *)&mac_cfg_info, OAL_SIZEOF(mac_cfg_vap_h2d_stru));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_cfg_vap_h2d_event_etc::return err code %d!}",
                         wal_util_get_vap_id(net_dev), l_ret);
        return l_ret;
    }

#endif

    return OAL_SUCC;
}

#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*****************************************************************************
 函 数 名  : wal_host_dev_config
 功能描述  : 02 host device_sruc配置接口，目前用于上下电流程
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年11月24日
    修改内容   : 新生成函数
*****************************************************************************/
OAL_STATIC osal_s32  wal_host_dev_config(oal_net_device_stru *net_dev, osal_u16 wid)
{
    oal_wireless_dev_stru      *wdev;
    mac_wiphy_priv_stru        *wiphy_priv;
    hmac_vap_stru              *cfg_hmac_vap;
    hmac_device_stru           *hmac_device;
    oal_net_device_stru        *cfg_net_dev;

    osal_s32                   l_ret;

    wdev = OAL_NETDEVICE_WDEV(net_dev);
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::wdev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::hmac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_device = wiphy_priv->hmac_device;
    if (hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::hmac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    if (cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::cfg_hmac_vap is null vap_id:%d!}",
                         hmac_device->cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_net_dev = cfg_hmac_vap->net_device;
    if (cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::cfg_net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), wid, (osal_u8 *)0, 0);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::wal_sync_post2hmac_no_rsp return err code %d!}", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_host_dev_init_etc
 功能描述  : 02 host device_sruc的初始化接口，目前用于上下电流程
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年11月24日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_host_dev_init_etc(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_MSG_W2H_CFG_HOST_DEV_INIT);
}

/*****************************************************************************
 函 数 名  : wal_host_dev_init_etc
 功能描述  : 02 host device_sruc的去初始化接口，目前用于上下电流程
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年11月26日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_host_dev_exit(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_MSG_W2H_CFG_HOST_DEV_EXIT);
}
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

osal_s32 hwifi_config_init_get_ini_info(oal_net_device_stru *net_dev, oal_wireless_dev_stru **wdev,
    hmac_device_stru **hmac_device, oal_net_device_stru **cfg_net_dev)
{
    mac_wiphy_priv_stru *wiphy_priv = OSAL_NULL;
    hmac_vap_stru       *hmac_vap = OSAL_NULL;

    if (net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::net_dev is null!}");
        return -OAL_EINVAL;
    }

    *wdev = OAL_NETDEVICE_WDEV(net_dev);
    if (*wdev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::wdev is null!}");
        return -OAL_EFAUL;
    }

    wiphy_priv  = (mac_wiphy_priv_stru *)oal_wiphy_priv((*wdev)->wiphy);
    if (wiphy_priv == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::wiphy_priv is null!}");
        return -OAL_EFAUL;
    }
    *hmac_device  = wiphy_priv->hmac_device;
    if (*hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::hmac_device is null!}");
        return -OAL_EFAUL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap((*hmac_device)->cfg_vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::cfg_hmac_vap is null, vap_id:%d!}",
                         (*hmac_device)->cfg_vap_id);
        return -OAL_EFAUL;
    }

    *cfg_net_dev = hmac_vap->net_device;
    if (*cfg_net_dev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_etc::cfg_net_dev is null!}");
        return -OAL_EFAUL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hwifi_config_init_force_etc
 功能描述  : 初始化定制化信息
 输入参数  : 无
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年12月1日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void hwifi_config_init_force_etc(osal_void)
{
    /* 重新上电时置为FALSE */
    g_uc_cfg_once_flag = OAL_FALSE;
}

#endif

osal_void wal_set_ap_power_flag(osal_u8 flag)
{
    g_st_ap_config_info.l_ap_power_flag = flag;
}

OAL_STATIC osal_s32 wal_netdev_open_pre_opt(oal_net_device_stru *net_dev, osal_u8 entry_flag)
{
#if !defined(_PRE_PRODUCT_ID_HOST)
    hmac_vap_stru     *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;
#endif

#if defined(_PRE_PRODUCT_ID_HOST)
    /* 在Android部分版本中，sta与softap共用一个netdev，当Softap开启时如果直接打开Sta，Android framework层会先下发一个网卡up的动作 */
    /* 此时不能重复走初始化流程，本次初始化需要直接return出去，但是如果当前是DFR过程中或者VAP类型不为ap则不需要 */
    if ((g_st_ap_config_info.l_ap_power_flag == OAL_TRUE) && (hmac_dfr_get_reset_process_flag() == OSAL_FALSE) &&
        (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_netdev_open_etc::power state is on,in ap mode, start vap later.}");

        /* 此变量临时用一次，防止 Android framework层在模式切换前下发网卡up动作 */
        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
        oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */
        return OAL_SUCC;
    }

    if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION)
#ifdef _PRE_WLAN_FEATURE_P2P
        || (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)
#endif
    ) { /* 信道跟随--取消name判断 */
        if (wal_init_wlan_vap_etc(net_dev) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
    } else if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_netdev_open_etc::ap mode,no need to start vap.!}");
        oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */
        if (wal_init_wlan_vap_etc(net_dev) != OAL_SUCC) {
            return -OAL_EFAIL;
        }
        return OAL_SUCC;
    }
#else
    if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) && (entry_flag == OAL_TRUE)) {
        hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
        if (OAL_UNLIKELY(hmac_vap == NULL)) {
            oam_error_log0(0, OAM_SF_CFG, "{_wal_netdev_open::can't get mac vap from netdevice priv data!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
        if (OAL_UNLIKELY(hmac_device == NULL)) {
            oam_error_log0(0, OAM_SF_CFG, "{_wal_netdev_open::hmac device is NULL!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (!hmac_device->start_via_priv) {
            OAL_NETDEVICE_FLAGS(net_dev) |= OAL_IFF_RUNNING;
            return OAL_SUCC;
        }
    }
#endif

    return OAL_CONTINUE;
}

OAL_STATIC osal_s32 wal_netdev_send_open_msg(oal_net_device_stru *net_dev)
{
    osal_s32 ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_wireless_dev_stru *wdev = OSAL_NULL;
    wlan_p2p_mode_enum_uint8 p2p_mode;
#endif
    mac_cfg_start_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

#ifdef _PRE_WLAN_FEATURE_P2P
    wdev = net_dev->ieee80211_ptr;
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_netdev_send_open_msg:wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}");
        return -OAL_EINVAL;
    }
    vap_info.p2p_mode = p2p_mode;
#endif
    vap_info.mgmt_rate_init_flag = OAL_TRUE;

    vap_info.ifindex = net_dev->ifindex;

    vap_info.net_dev = net_dev;

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_START_VAP,
        (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (OAL_UNLIKELY(OAL_SUCC != ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_send_open_msg::START_VAP return err code %d!}", ret);
        wifi_printf("{wal_netdev_send_open_msg::START_VAP return err code %d!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 打开wlan及相关的错误处理
*****************************************************************************/
static osal_s32  wlan_open_proc(oal_net_device_stru *net_dev)
{
    osal_s32 l_ret;

    l_ret = wlan_open();
    if (l_ret == OAL_FAIL) {
        oam_error_log0(0, OAM_SF_ANY, "{wlan_open_proc::wlan_open Fail!}");
        return -OAL_EFAIL;
    } else if (l_ret != OAL_ERR_CODE_ALREADY_OPEN) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        /* 重新上电时置为FALSE */
        hwifi_config_init_force_etc();
#endif

        /* 上电host device_stru初始化 */
        l_ret = wal_host_dev_init_etc(net_dev);
        if (l_ret != OAL_SUCC) {
            wifi_printf("wlan_open_proc::wal_host_dev_init_etc FAIL %d \r\n", l_ret);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

/* 起ap时,校验 1) 是否用p2p0启动 2) 是否已存在p2p */
OAL_STATIC osal_s32 wal_netdev_open_ap_pre_check(oal_net_device_stru *net_dev)
{
    hmac_vap_stru *hmac_vap_tmp = OAL_PTR_NULL;
    osal_u8 vap_idx;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    if ((osal_strcmp((const osal_s8 *)net_dev->name, wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX))) == 0) {
        wifi_printf("{wal_netdev_open_ap_pre_check::not support p2p0 as softap!}\r\n");
        oam_warning_log0(0, OAM_SF_ANY, "{wal_netdev_open_ap_pre_check::not support p2p0 as softap!}");
        return -OAL_EFAUL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(0);
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            continue;
        }
        /* vap的net_dev未处于UP状态，不应该影响其他vap的启动 */
        if (!(OAL_NETDEVICE_FLAGS(hmac_vap_tmp->net_device) & OAL_IFF_UP)) {
            continue;
        }
        if (hmac_vap_tmp->p2p_mode == WLAN_P2P_DEV_MODE ||
            hmac_vap_tmp->p2p_mode == WLAN_P2P_GO_MODE || hmac_vap_tmp->p2p_mode == WLAN_P2P_CL_MODE) {
            return -OAL_EFAUL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_netdev_open_etc
 功能描述  : 启用VAP
 输入参数  : net_dev: net_device
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月11日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  _wal_netdev_open(oal_net_device_stru *net_dev, osal_u8 entry_flag)
{
    osal_s32               l_ret;
    hmac_vap_stru *hmac_vap_tmp = OAL_PTR_NULL;
    osal_u8 vap_idx;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    if (OAL_UNLIKELY(net_dev == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_netdev_open_etc::net_dev is null ptr!}");
        return -OAL_EFAUL;
    }

    wifi_printf("wal_netdev_open_etc,dev_name is:%s\n", net_dev->name);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_open_etc::iftype:%d.!}", net_dev->ieee80211_ptr->iftype);
    
    /* 起ap时,校验 1) 是否用p2p0启动 2) 是否已存在p2p */
    if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) && (wal_netdev_open_ap_pre_check(net_dev) != OAL_SUCC)) {
        return -OAL_EFAUL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(0);
    /* 如果当前启动的是p2p,则判断是否存在ap */
    if ((osal_strcmp((const osal_s8 *)net_dev->name, wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX))) == 0) {
        for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
            hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
            if (hmac_vap_tmp == OAL_PTR_NULL) {
                continue;
            }

            /* vap的net_dev未处于UP状态，不应该影响其他vap的启动 */
            if (!(OAL_NETDEVICE_FLAGS(hmac_vap_tmp->net_device) & OAL_IFF_UP)) {
                continue;
            }

            if (hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_AP) {
                return -OAL_EFAUL;
            }
        }
    }

#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    l_ret = wlan_open_proc(net_dev);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    /* 配置vap的创建,函数内通过标志位判断是否需要下发device */
    l_ret = wal_cfg_vap_h2d_event_etc(net_dev);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_cfg_vap_h2d_event_etc FAIL %d.", l_ret);
        return -OAL_EFAIL;
    }
    oam_warning_log0(0, OAM_SF_ANY, "wal_cfg_vap_h2d_event_etc succ.");
#endif

    l_ret = wal_netdev_open_pre_opt(net_dev, entry_flag);
    if (l_ret != OAL_CONTINUE) {
        return l_ret;
    }

    l_ret = wal_netdev_send_open_msg(net_dev) != OAL_SUCC;
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    OAL_NETDEVICE_FLAGS(net_dev) |= OAL_IFF_RUNNING;
    oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */
    return OAL_SUCC;
}

STATIC osal_s32  wal_netdev_open_ext(oal_net_device_stru *net_dev)
{
    wal_record_wifi_external_log(WLAN_WIFI_NETDEV_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* 如果是DFR流程中, 仅在AP/STA恢复时允许内核open */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (wal_dfr_get_status() != WAL_DFR_STA_AP_RECOVERY)) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_netdev_open_ext::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAUL;
    }
#endif
    return  wal_netdev_open_etc(net_dev, OAL_TRUE);
}

osal_void wal_insmod_ko_begin(osal_void)
{
    OAL_INIT_COMPLETION(&g_insmod_ready);
    wifi_printf("insmod wifi ko beginning!\r\n");
    return;
}

osal_s32 wal_insmod_ko_finish(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    OAL_COMPLETE(&g_insmod_ready);
    wifi_printf("insmod wifi ko finished!\r\n");
    return OAL_SUCC;
}

osal_s32  wal_netdev_open_etc(oal_net_device_stru *net_dev, osal_u8 entry_flag)
{
    osal_s32 ret;
#if defined (_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    osal_u8 *dev_name = OSAL_NULL;
    wal_cfgvendor_radio_stat_stru *cfgvendor_radio_stat = wal_cfg80211_get_radio_stat();
#endif
    static osal_u32 first_open_flag = OSAL_TRUE;
    if (first_open_flag == OSAL_TRUE) {
        if (oal_wait_for_completion_timeout((&g_insmod_ready),
            (osal_u32)oal_msecs_to_jiffies(NETDEV_WAIT_WIFI_INSMOD_TIMEOUT)) == 0) {
            oam_warning_log1(0, OAM_SF_ANY, "wait wifi ko ready timeout... %d ms ", HOST_WAIT_BOTTOM_WIFI_TIMEOUT);
        }
        first_open_flag = OSAL_FALSE;
    }

    wal_wake_lock();
    ret = _wal_netdev_open(net_dev, entry_flag);
    wal_wake_unlock();

#if defined (_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    /* 记录wlan0 开的时间 */
    dev_name = wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX);
    if ((dev_name != OSAL_NULL) && (osal_adapt_strncmp(dev_name, net_dev->name, osal_strlen(dev_name)) == 0)) {
        cfgvendor_radio_stat->ull_wifi_on_time_stamp = OAL_TIME_JIFFY;
    }
#endif
    return ret;
}

STATIC osal_s32 hwifi_wal_netdev_stop_send_vap_down_event(oal_net_device_stru *net_dev)
{
    osal_s32                   ret;

    ret = wal_down_vap(net_dev, wal_util_get_vap_id(net_dev));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_stop_etc::wal_down_vap return err code %d!}", ret);
        wifi_printf("{wal_netdev_stop_etc::wal_down_vap return err code %d!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

#if defined(_PRE_PRODUCT_ID_HOST)
/*****************************************************************************
 函 数 名  : wal_get_macdev_by_netdev
 功能描述  : 通过device id设置netdev的名称device id 0 ->wlan0 p2p0, device id 1->wlan1 p2p1
 输入参数  : net_dev: net_device
 输出参数  : 无
 返 回 值  : hmac_device_stru
 修改历史      :
  1.日    期   : 2016年07月23日
*****************************************************************************/
STATIC hmac_device_stru *wal_get_macdev_by_netdev(oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru    *wdev;
    mac_wiphy_priv_stru      *wiphy_priv;

    wdev = OAL_NETDEVICE_WDEV(net_dev);
    if (wdev == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_macdev_by_netdev::wdev is null!}");
        return OAL_PTR_NULL;
    }
    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wdev->wiphy));
    return wiphy_priv->hmac_device;
}

STATIC osal_s32 hwifi_wal_netdev_stop_vap_down(oal_net_device_stru *net_dev)
{
    osal_s32 ret;
    hmac_device_stru *hmac_device;

    /* P2P关联成为GO时，打印错误.修改wal_net_dev_stop 为需要等待hmac 响应 */
    hmac_device = wal_get_macdev_by_netdev(net_dev);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_deinit_wlan_vap_etc::wal_get_macdev_by_netdev FAIL");
        return -OAL_EFAIL;
    }

    /* 删除VAP */
    if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION)
#ifdef _PRE_WLAN_FEATURE_P2P
        || (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE)
#endif
    ) {
        ret = wal_deinit_wlan_vap_etc(net_dev);
        if (ret != OAL_SUCC) {
            return ret;
        }
        return OAL_SUCC;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : wal_netdev_stop_etc
 功能描述  : 停用vap
 输入参数  : net_dev: net_device
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年5月13日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  wal_netdev_stop(oal_net_device_stru *net_dev)
{
    osal_s32                   l_ret;
    hmac_vap_stru               *hmac_vap;

    if (OAL_UNLIKELY(net_dev == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_netdev_stop_etc::net_dev is null ptr!}");
        return -OAL_EFAUL;
    }

    /* stop the netdev's queues */
    oal_net_tx_stop_all_queues(net_dev); /* 停止发送队列 */
    wal_force_scan_complete_etc(net_dev, OAL_TRUE);

#if defined(_PRE_PRODUCT_ID_HOST)
    oam_warning_log1(0, OAM_SF_ANY, "{wal_netdev_stop::iftype:%d.!}", net_dev->ieee80211_ptr->iftype);

    /* AP模式下,在模式切换时down和删除 vap */
    if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
        l_ret = wal_netdev_stop_ap_etc(net_dev);
        return l_ret;
    }
#endif

    /* 如果netdev下mac vap已经释放，则直接返回成功 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_netdev_stop:: mac vap of netdevice is down!iftype:[%d]}",
                         net_dev->ieee80211_ptr->iftype);
        return OAL_SUCC;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_INIT) {
        oam_warning_log3(0, OAM_SF_ANY, "{wal_netdev_stop::vap is already down!iftype[%d] vap mode[%d] p2p mode[%d]!}",
                         net_dev->ieee80211_ptr->iftype, hmac_vap->vap_mode, hmac_vap->p2p_mode);
        if (hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE) {
            /* p2p client设备down掉成为p2p device，需要继续执行deinit操作，不能直接返回 */
            return OAL_SUCC;
        }
    }

    l_ret = hwifi_wal_netdev_stop_send_vap_down_event(net_dev);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

#if defined(_PRE_PRODUCT_ID_HOST)
    return hwifi_wal_netdev_stop_vap_down(net_dev);
#else
    return OAL_SUCC;
#endif
}

#if defined(_PRE_PRODUCT_ID_HOST)
/*****************************************************************************
 函 数 名  : wal_set_power
 功能描述  : CMD_SET_POWER_ON命令
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2017年7月25日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  wal_set_power_on(oal_net_device_stru *net_dev, osal_s32 power_flag)
{
    osal_s32    l_ret = 0;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_power_on::set power flag:%d}", power_flag);

    /* ap上下电，配置VAP */
    if (power_flag == 0) { /* 下电 */
        /* 下电host device_stru去初始化 */
        wal_host_dev_exit(net_dev);

        wal_wake_lock();
        wlan_close();
        wal_wake_unlock();

        g_st_ap_config_info.l_ap_power_flag = OAL_FALSE;
    } else if (power_flag == 1) { /* 上电 */
        g_st_ap_config_info.l_ap_power_flag = OAL_TRUE;

        wal_wake_lock();
        l_ret = wlan_open();
        wal_wake_unlock();
        if (l_ret == OAL_FAIL) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_set_power_on::wlan_pm_open_etc Fail!}");
            return -OAL_EFAIL;
        } else if (l_ret != OAL_ERR_CODE_ALREADY_OPEN) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* 重新上电时置为FALSE */
            hwifi_config_init_force_etc();
#endif
            /* 重新上电场景，下发配置VAP */
            l_ret = wal_cfg_vap_h2d_event_etc(net_dev);
            if (l_ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }
        }

        /* 上电host device_stru初始化 */
        l_ret = wal_host_dev_init_etc(net_dev);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "wal_set_power_on FAIL %d.", l_ret);
            return -OAL_EFAIL;
        }
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_power_on::pupower_flag:%d error.}", power_flag);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_netdev_stop_ap_etc
 功能描述  : netdev_stop流程中针对ap的流程
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2017年7月19日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_netdev_stop_ap_etc(oal_net_device_stru *net_dev)
{
    osal_s32          l_ret;

    if (net_dev->ieee80211_ptr->iftype != NL80211_IFTYPE_AP) {
        return OAL_SUCC;
    }

    /* 结束扫描,以防在20/40M扫描过程中关闭AP */
    wal_force_scan_complete_etc(net_dev, OAL_TRUE);

    /* AP关闭切换到STA模式,删除相关vap */
    /* 解决vap状态与netdevice状态不一致，无法删除vap的问题，VAP删除后，上报成功 */
    if (wal_stop_vap_etc(net_dev) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::wal_stop_vap_etc enter a error.}");
    }
    l_ret = wal_deinit_wlan_vap_etc(net_dev);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::wal_deinit_wlan_vap_etc enter a error.}");
        return l_ret;
    }

    /* Del aput后需要切换netdev iftype状态到station */
    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_STATION;
    oam_warning_log0(0, OAM_SF_CFG, "{wal_netdev_stop_ap_etc::set iftype to station(2).}");

    return OAL_SUCC;
}
#endif

osal_s32 wal_netdev_stop_ext(oal_net_device_stru *net_dev)
{
    wal_record_wifi_external_log(WLAN_WIFI_NETDEV_OPS, __func__);
#ifdef _PRE_WLAN_FEATURE_DFR
    if ((plat_is_device_in_recovery() == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_netdev_stop_ext::dfr ing... status[%d]", wal_dfr_get_status());
        return -OAL_EFAUL;
    }
#endif
    return wal_netdev_stop_etc(net_dev);
}

osal_s32  wal_netdev_stop_etc(oal_net_device_stru *net_dev)
{
    osal_s32 ret;
    wal_record_wifi_external_log(WLAN_WIFI_NETDEV_OPS, __func__);
    wal_wake_lock();
    ret = wal_netdev_stop(net_dev);
    wal_wake_unlock();

    if (ret == OAL_SUCC) {
        OAL_NETDEVICE_FLAGS(net_dev) &= ~OAL_IFF_RUNNING;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : wal_netdev_get_stats
 功能描述  : 获取统计信息
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月1日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_net_device_stats_stru  *wal_netdev_get_stats(oal_net_device_stru *net_dev)
{
    oal_net_device_stats_stru  *stats = &(net_dev->stats);
#ifdef _PRE_PRODUCT_ID_HOST
    hmac_vap_stru               *hmac_vap;
    oam_stat_info_stru         *oam_stat;

    oam_vap_stat_info_stru     *oam_vap_stat;

    hmac_vap  = (hmac_vap_stru *)net_dev->ml_priv;
    oam_stat = OAM_STAT_GET_STAT_ALL();

    if (hmac_vap == NULL) {
        return stats;
    }

    if (hmac_vap->vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_error_log1(0, OAM_SF_ANY, "wal_netdev_get_stats error vap id %u", hmac_vap->vap_id);
        return stats;
    }

    oam_vap_stat = &(oam_stat->vap_stat_info[hmac_vap->vap_id]);

    /* 更新统计信息到net_device */
    stats->rx_packets   = oam_vap_stat->rx_pkt_to_lan;
    stats->rx_bytes     = oam_vap_stat->rx_bytes_to_lan;
    stats->rx_errors    = oam_vap_stat->rx_defrag_process_dropped;
    stats->rx_dropped   = oam_vap_stat->rx_no_buff_dropped + oam_vap_stat->rx_da_check_dropped;

    stats->tx_packets   = oam_vap_stat->tx_pkt_num_from_lan;
    stats->tx_bytes     = oam_vap_stat->tx_bytes_from_lan;
    stats->tx_errors    = oam_vap_stat->tx_abnormal_msdu_dropped + oam_vap_stat->tx_security_check_faild;
    stats->tx_dropped   = 0;
#endif
    return stats;
}

osal_u32 wlan_set_mac(osal_u8 *mac_addr, osal_u8 len, oal_net_device_stru *net_dev)
{
    netdev_mac_index_enum netdev_type;
    osal_u32 ret;

    /* AP名字有定制，但是wlan0 p2p0是固定的，所以这里默认匹配为AP的index */
    netdev_type = NETDEV_AP_MAC_INDEX;
    if (osal_strcmp(wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX), (const osal_s8 *)net_dev->name) == 0) {
        netdev_type = NETDEV_STA_MAC_INDEX;
    }
    if (osal_strcmp(wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX), (const osal_s8 *)net_dev->name) == 0) {
        netdev_type = NETDEV_P2P_MAC_INDEX;
    }
    if (len != WLAN_MAC_ADDR_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "wlan_set_mac mac len %u wrong", len);
        return OAL_FAIL;
    }

    ret = memcpy_s(g_wlan_mac[netdev_type], WLAN_MAC_ADDR_LEN, mac_addr, len);
    if (ret != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "wlan_set_mac memcpy_s failed ");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_u32 wlan_get_mac(osal_u8 *mac_addr, osal_u8 len, oal_net_device_stru *net_dev)
{
    netdev_mac_index_enum netdev_type;
    osal_u32 ret;

    /* AP名字有定制，但是wlan0 p2p0是固定的，所以这里默认匹配为AP的index */
    netdev_type = NETDEV_AP_MAC_INDEX;
    if (osal_strcmp(wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX), (const osal_s8 *)net_dev->name) == 0) {
        netdev_type = NETDEV_STA_MAC_INDEX;
    }
    if (osal_strcmp(wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX), (const osal_s8 *)net_dev->name) == 0) {
        netdev_type = NETDEV_P2P_MAC_INDEX;
    }
    if (len != WLAN_MAC_ADDR_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "wlan_get_mac mac len %u wrong", len);
        return OAL_FAIL;
    }
    ret = memcpy_s(mac_addr, len, g_wlan_mac[netdev_type], WLAN_MAC_ADDR_LEN);
    memcpy(&net_dev->dev_addr_shadow[0],g_wlan_mac[netdev_type], len);
    if (ret != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "wlan_get_mac memcpy_s failed ");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_void wlan_get_addr_from_plat(osal_void)
{
    osal_u32 ret;
    oal_net_device_stru *net_dev = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OSAL_NULL) {
        wifi_printf("wlan_get_addr_from_plat get hmac_device failed\r\n");
        return;
    }

    ret = get_dev_addr(g_wlan_mac[NETDEV_STA_MAC_INDEX], WLAN_MAC_ADDR_LEN, NL80211_IFTYPE_STATION);
    if (ret != OAL_SUCC) {
        wifi_printf("wlan_get_addr_from_plat: get sta mac failed:%d\r\n", ret);
        return;
    }

    ret = get_dev_addr(g_wlan_mac[NETDEV_AP_MAC_INDEX], WLAN_MAC_ADDR_LEN, NL80211_IFTYPE_AP);
    if (ret != OAL_SUCC) {
        wifi_printf("wlan_get_addr_from_plat: get ap mac failed:%d\r\n", ret);
        return;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    ret = get_dev_addr(g_wlan_mac[NETDEV_P2P_MAC_INDEX], WLAN_MAC_ADDR_LEN, NL80211_IFTYPE_P2P_DEVICE);
    if (ret != OAL_SUCC) {
        wifi_printf("wlan_get_addr_from_plat: get p2p mac failed:%d\r\n", ret);
        return;
    }
#endif
    net_dev = hmac_device->st_p2p_info.primary_net_device;
    if ((net_dev != OSAL_NULL) && (wlan_get_mac(net_dev->dev_addr, WLAN_MAC_ADDR_LEN, net_dev) != OAL_SUCC)) {
        wifi_printf("wlan_get_addr_from_plat: set [%s] mac failed\r\n", net_dev->name);
        return;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    net_dev = hmac_device->st_p2p_info.p2p_net_device;
    if ((net_dev != OSAL_NULL) && (wlan_get_mac(net_dev->dev_addr, WLAN_MAC_ADDR_LEN, net_dev) != OAL_SUCC)) {
        wifi_printf("wlan_get_addr_from_plat: set [%s] mac failed\r\n", net_dev->name);
        return;
    }
#endif
    net_dev = hmac_device->st_p2p_info.second_net_device;
    if ((net_dev != OSAL_NULL) && (wlan_get_mac(net_dev->dev_addr, WLAN_MAC_ADDR_LEN, net_dev) != OAL_SUCC)) {
        wifi_printf("wlan_get_addr_from_plat: set [%s] mac failed\r\n", net_dev->name);
        return;
    }
}

/*****************************************************************************
 函 数 名  : wal_netdev_set_mac_addr
 功能描述  : 设置mac地址
 输入参数  : dev: 网络设备
             p_addr : 地址
 输出参数  : 无

 修改历史      :
  1.日    期   : 2012年12月24日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  _wal_netdev_set_mac_addr(oal_net_device_stru *net_dev, void *p_addr)
{
    oal_sockaddr_stru *mac_addr = OAL_PTR_NULL;
    osal_u32 ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == p_addr))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::null ptr net_dev(%p) addr(%p)}",
            (uintptr_t)net_dev, (uintptr_t)p_addr);
        return -OAL_EFAUL;
    }

    if (oal_netif_running(net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::cannot set address; device running!}");
        return -OAL_EBUSY;
    }

    mac_addr = (oal_sockaddr_stru *)p_addr;

    if (ETHER_IS_MULTICAST(mac_addr->sa_data)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_netdev_set_mac_addr::can not set group/broadcast addr!}");
        return -OAL_EINVAL;
    }

    oal_set_mac_addr((osal_u8 *)(net_dev->dev_addr), (osal_u8 *)(mac_addr->sa_data));
    ret = wlan_set_mac(net_dev->dev_addr, WLAN_MAC_ADDR_LEN, net_dev);

    return (osal_s32)ret;
}

OAL_STATIC osal_s32  wal_netdev_set_mac_addr(oal_net_device_stru *net_dev, void *p_addr)
{
    osal_s32 ret;
    wal_record_wifi_external_log(WLAN_WIFI_NETDEV_OPS, __func__);
    wal_wake_lock();
    printk("wal_netdev_set_mac_addr\r\n");
    ret = _wal_netdev_set_mac_addr(net_dev, p_addr);
    wal_wake_unlock();

    return ret;
}

/*****************************************************************************
 函 数 名  : wal_net_device_ioctl
 功能描述  : net device的ioctl函数
 输入参数  : net device指针
 输出参数  : 无
 返 回 值  : 统计结果指针

 修改历史      :
  1.日    期   : 2013年11月27日
    修改内容   : 新增函数

*****************************************************************************/
STATIC osal_s32 wal_net_device_ioctl(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr, osal_s32 ul_cmd)
{
    osal_s32                           l_ret   = 0;

    wal_record_wifi_external_log(WLAN_WIFI_NETDEV_OPS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == ifr)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_net_device_ioctl::dev %p, ifr %p!}",
                       (uintptr_t)net_dev, (uintptr_t)ifr);
        return -OAL_EFAUL;
    }

    if (OAL_PTR_NULL == ifr->ifr_data) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_net_device_ioctl::ifr->ifr_data is NULL, ul_cmd[0x%x]!}", ul_cmd);
        return -OAL_EFAUL;
    }

    /* wpa_supplicant 通过ioctl 下发命令 */
    if (WAL_SIOCDEVPRIVATE + 1 == ul_cmd) {
#ifndef _PRE_WLAN_ANDROID_CROP
        l_ret = uapi_android_priv_cmd_etc(net_dev, ifr, ul_cmd);
#endif
#ifdef _PRE_WLAN_WS86_FPGA
        l_ret = 0;
#endif
        return l_ret;
#if ((!defined(_PRE_PRODUCT_ID_HIMPXX_DEV)) && (!defined(_PRE_PRODUCT_ID_HOST)))
    /* 51ioctl入口重构隔离 */
    /* 51 通过ioctl下发命令 */
    } else if (WAL_SIOCDEVPRIVATE == ul_cmd) {

        if (!capable(CAP_NET_ADMIN)) {
            return -EPERM;
        }

        return OAL_SUCC;
    /* 51ioctl入口重构隔离 */
#endif
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_net_device_ioctl::unrecognised cmd[0x%x]!}", ul_cmd);
        return OAL_SUCC;
    }
}

#if defined(_PRE_PRODUCT_ID_HOST)
/*****************************************************************************
 函 数 名  : wal_start_vap_etc
 输入参数  : oal_net_device_stru *net_dev
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_u32

 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_start_vap_etc(oal_net_device_stru *net_dev)
{
    osal_s32 l_ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode;
    oal_wireless_dev_stru *wdev = OSAL_NULL;
#endif
    mac_cfg_start_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    if (OAL_UNLIKELY(OAL_PTR_NULL == net_dev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap_etc::net_dev is null ptr!}");
        return -OAL_EFAUL;
    }

    wifi_printf("wal_start_vap_etc,dev_name is:%s\n", net_dev->name);
#ifdef _PRE_WLAN_FEATURE_P2P
    wdev = net_dev->ieee80211_ptr;
    if (wdev == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_start_vap_etc:: wdev null");
        return -OAL_EINVAL;
    }
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (WLAN_P2P_BUTT == p2p_mode) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_start_vap_etc::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}");
        return -OAL_EINVAL;
    }
    vap_info.p2p_mode = p2p_mode;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap_etc::p2p_mode:%d}\r\n", p2p_mode);
#endif
    vap_info.mgmt_rate_init_flag = OAL_TRUE;

    vap_info.net_dev = net_dev;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_START_VAP,
        (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_start_vap_etc::wal_sync_post2hmac_no_rsp return err code %d!}", l_ret);
        wifi_printf("{wal_start_vap_etc::wal_sync_post2hmac_no_rsp return err code %d!}\r\n", l_ret);
        return l_ret;
    }
    /* AP模式,启动VAP后,启动发送队列 */
    oal_net_tx_wake_all_queues(net_dev); /* 启动发送队列 */

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_stop_vap_etc
 功能描述  : 停用vap
 输入参数  : oal_net_device_stru: net_device
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2015年5月25日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_stop_vap_etc(oal_net_device_stru *net_dev)
{
    osal_s32 l_ret;
    l_ret = wal_down_vap(net_dev, wal_util_get_vap_id(net_dev));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_stop_vap_etc::wal_sync_post2hmac_no_rsp return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 初始化wlan0,p2p0的vap - 抛事件到wal层处理
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_P2P
osal_s32 wal_send_init_wlan_event(oal_net_device_stru *net_dev, wlan_vap_mode_enum_uint8 vap_mode,
    hmac_vap_stru *cfg_mac_vap, hmac_vap_stru *cfg_hmac_vap, wlan_p2p_mode_enum_uint8 p2p_mode)
#else
osal_s32 wal_send_init_wlan_event(oal_net_device_stru *net_dev, wlan_vap_mode_enum_uint8 vap_mode,
    hmac_vap_stru *cfg_mac_vap, hmac_vap_stru *cfg_hmac_vap)
#endif
{
    osal_s32 ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    ret = wal_add_vap(net_dev, cfg_hmac_vap->vap_id, vap_mode, p2p_mode);
#else
    ret = wal_add_vap(net_dev, cfg_hmac_vap->vap_id, vap_mode, WLAN_LEGACY_VAP_MODE);
#endif
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_send_init_wlan_event::return err code %d!}", ret);
        return -OAL_EFAIL;
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (wal_set_random_mac_to_mib_etc(net_dev) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_send_init_wlan_event::wal_set_random_mac_to_mib_etc fail!}");
        return -OAL_EFAUL;
    }
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置最大用户数和mac地址过滤模式
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_P2P
OAL_STATIC osal_s32 wal_set_max_user_mac_filter(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev,
    hmac_device_stru *hmac_device, wlan_p2p_mode_enum_uint8 p2p_mode)
#else
OAL_STATIC osal_s32 wal_set_max_user_mac_filter(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev)
#endif
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_send_init_wlan_event::net_dev->ml_priv is null ptr.}");
        return -OAL_EINVAL;
    }

    if (wdev->iftype == NL80211_IFTYPE_AP) {
        /* AP模式初始化，初始化配置最大用户数和mac地址过滤模式 */
        if (g_st_ap_config_info.ap_max_user > 0) {
            wal_set_ap_max_user(net_dev, g_st_ap_config_info.ap_max_user);
        }

        if (osal_strlen((const osal_s8 *)g_st_ap_config_info.ac_ap_mac_filter_mode) > 0) {
            wal_config_mac_filter(net_dev, g_st_ap_config_info.ac_ap_mac_filter_mode);
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 初始化wlan0,p2p0的vap
*****************************************************************************/
osal_s32 wal_init_wlan_vap_ext(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev, hmac_device_stru *hmac_device,
    hmac_vap_stru *cfg_mac_vap, hmac_vap_stru *cfg_hmac_vap)
{
    osal_s32 ret;
    wlan_vap_mode_enum_uint8 vap_mode;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif
    /* 仅用于WIFI和AP打开时创建VAP */
    if (wdev->iftype == NL80211_IFTYPE_STATION
#ifdef _PRE_WLAN_FEATURE_P2P
        || wdev->iftype == NL80211_IFTYPE_P2P_DEVICE
#endif
    ) {
        vap_mode = WLAN_VAP_MODE_BSS_STA;
#ifdef _PRE_WLAN_FEATURE_P2P
        if (osal_strcmp((wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX)), (const osal_s8 *)net_dev->name) == 0) {
            p2p_mode = WLAN_P2P_DEV_MODE;
        }
#endif
#ifdef CONTROLLER_CUSTOMIZATION
        if (osal_strcmp((wal_get_wlan_name_config(NETDEV_AP_MAC_INDEX)), (const osal_s8 *)net_dev->name) == 0) {
            vap_mode = WLAN_VAP_MODE_BSS_AP;
        }
#endif
    } else if (wdev->iftype == NL80211_IFTYPE_AP) {
        vap_mode = WLAN_VAP_MODE_BSS_AP;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_ext::net_dev is not wlan or p2p!}");
        return OAL_SUCC;
    }
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

#ifdef _PRE_WLAN_FEATURE_P2P
    oam_warning_log2(0, OAM_SF_ANY, "{wal_init_wlan_vap_ext::vap_mode:%d,p2p_mode:%d}", vap_mode, p2p_mode);
    ret = wal_send_init_wlan_event(net_dev, vap_mode, cfg_mac_vap, cfg_hmac_vap, p2p_mode);
#else
    ret = wal_send_init_wlan_event(net_dev, vap_mode, cfg_mac_vap, cfg_hmac_vap);
#endif
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_vap_ext::wal_send_init_wlan_event fail, err code[%d]!}", ret);
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    ret = wal_set_max_user_mac_filter(net_dev, wdev, hmac_device, p2p_mode);
#else
    ret = wal_set_max_user_mac_filter(net_dev, wdev);
#endif
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_vap_ext::wal_set_max_user_mac_filter err code[%d]!}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : wal_init_wlan_vap_etc
 功能描述  : 初始化wlan0,p2p0的vap
 输入参数  : oal_net_device_stru *cfg_net_dev
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_init_wlan_vap_etc(oal_net_device_stru *net_dev)
{
    hmac_vap_stru               *hmac_vap = OSAL_NULL;
    oal_wireless_dev_stru       *wdev = OSAL_NULL;
    hmac_vap_stru               *cfg_hmac_vap = OSAL_NULL;
    hmac_device_stru            *hmac_device = OSAL_NULL;

    if (net_dev == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::net_dev is null!}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap != OSAL_NULL) {
        if (MAC_VAP_STATE_BUTT != hmac_vap->vap_state) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::mac_vap already exists}");
            return OAL_SUCC;
        }
        /* netdev下的vap已经被删除，需要重新创建和挂载 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc: vap already free, create again! set ml_priv to null}");
        net_dev->ml_priv = OSAL_NULL;
    }

    wdev = OAL_NETDEVICE_WDEV(net_dev);
    if (wdev == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::wdev is null!}");
        return -OAL_EFAUL;
    }

    hmac_device  = ((mac_wiphy_priv_stru *)oal_wiphy_priv(wdev->wiphy))->hmac_device;
    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::mac_device is null!}");
        return -OAL_EFAUL;
    }

    cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    if (cfg_hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::cfg_hmac_vap is null!}");
        return -OAL_EFAUL;
    }
    /* 仅用于WIFI和AP打开时创建VAP */
    if (wal_init_wlan_vap_ext(net_dev, wdev, hmac_device, cfg_hmac_vap, cfg_hmac_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_vap_etc::wal_init_wlan_vap_ext fail!}");
        return -OAL_EFAUL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 仅用于WIFI和AP关闭时删除VAP - 发送删除事件
 被调函数  : wal_deinit_wlan_vap_etc(oal_net_device_stru *net_dev);
*****************************************************************************/
OAL_STATIC osal_s32 wal_send_deinit_wlan_vap_event(oal_net_device_stru *net_dev, hmac_cfg_del_vap_param_stru *vap_info,
    hmac_vap_stru *hmac_vap)
{
    osal_s32                    ret;
    osal_s32                    l_del_vap_flag = OAL_TRUE;

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_DEL_VAP,
        (osal_u8 *)vap_info, OAL_SIZEOF(hmac_cfg_del_vap_param_stru));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {wal_send_deinit_wlan_vap_event::return error code %d}", hmac_vap->vap_id, ret);
        wifi_printf("vap_id[%d] {wal_send_deinit_wlan_vap_event::return error code %d}", hmac_vap->vap_id, ret);
        if (-OAL_ENOMEM == ret || -OAL_EFAIL == ret) {
            /* wid had't processed */
            l_del_vap_flag = OAL_FALSE;
        }
    }

    if (l_del_vap_flag == OAL_TRUE) {
        net_dev->ml_priv = NULL;
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d]{wal_send_deinit_wlan_vap_event:ret %d, set ml_priv to NULL}",
            hmac_vap->vap_id, ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : wal_deinit_wlan_vap_etc
 功能描述  : 仅用于WIFI和AP关闭时删除VAP
 输入参数  : oal_net_device_stru *net_dev
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年1月3日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_s32 wal_deinit_wlan_vap_etc(oal_net_device_stru *net_dev)
{
    hmac_vap_stru               *hmac_vap = OSAL_NULL;
    hmac_device_stru            *hmac_device = OSAL_NULL;
    osal_s32                    ret;
    osal_s8                     p2p_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    wlan_p2p_mode_enum_uint8    p2p_mode = WLAN_LEGACY_VAP_MODE;
    hmac_cfg_del_vap_param_stru vap_info;

    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    if (OAL_UNLIKELY(net_dev == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::del_vap_param null ptr !}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc::mac_vap is already null}");
        return OAL_SUCC;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_deinit_wlan_vap_etc::hmac_res_get_mac_dev_etc[%d] FAIL",
            hmac_vap->device_id);
        return -OAL_EFAIL;
    }

    ret = snprintf_s(p2p_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH,
        wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX));
    if (ret < 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_deinit_wlan_vap_etc:: p2p_dev_name ret_len %d}", ret);
    }

    if (0 == osal_strcmp((const osal_s8 *)p2p_netdev_name, (const osal_s8 *)net_dev->name)) {
        p2p_mode = WLAN_P2P_DEV_MODE;
    }
    oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d]{wal_deinit_wlan_vap_etc:p2p_mode:%d}", hmac_vap->vap_id, p2p_mode);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    // 删除vap 时需要将参数赋值。
    vap_info.net_dev = net_dev;
    vap_info.add_vap.p2p_mode = p2p_mode;

    ret = wal_send_deinit_wlan_vap_event(net_dev, &vap_info, hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_deinit_wlan_vap_etc::wal_send_deinit_wlan_vap_event fail.");
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : wal_set_mac_addr
 功能描述  : 初始化mac地址
 输入参数  : 无
 输出参数  : osal_u8 *buf
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月25日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_set_mac_addr(oal_net_device_stru *net_dev)
{
    oal_wireless_dev_stru        *wdev;
    mac_wiphy_priv_stru          *wiphy_priv;
    hmac_device_stru             *hmac_device;
#ifndef _PRE_WLAN_FEATURE_P2P
    osal_u8                     primary_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };    /* MAC地址 */
    osal_s8                      hwlan_netdev_name[MAC_NET_DEVICE_NAME_LENGTH] = { 0 };
    osal_s32 ret_len;
#endif

    wdev = OAL_NETDEVICE_WDEV(net_dev);
    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wdev->wiphy));
    hmac_device = wiphy_priv->hmac_device;

#ifdef _PRE_WLAN_FEATURE_P2P
    if (wlan_get_mac((osal_u8 *)(net_dev->dev_addr), WLAN_MAC_ADDR_LEN, net_dev) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_mac_addr::wal_set_p2p_mac_addr fail.}");
        return OAL_FAIL;
    }
#else
    /* 信道跟随--add 增加hwlan name判断 */
    oal_random_ether_addr(primary_mac_addr, sizeof(primary_mac_addr));
    ret_len = snprintf_s(hwlan_netdev_name, MAC_NET_DEVICE_NAME_LENGTH, MAC_NET_DEVICE_NAME_LENGTH, "hwlan%d",
        hmac_device->device_id);
    if (0 == (osal_strcmp(hwlan_netdev_name, net_dev->name))) {
        primary_mac_addr[0] |= 0x02;
        primary_mac_addr[5] += 1; /* mac地址第5位 */
    } else {
        primary_mac_addr[0] &= (~0x02);
        primary_mac_addr[1] = 0x11;
        primary_mac_addr[2] = 0x02; /* mac地址第2位 */
    }
    oal_set_mac_addr((osal_u8 *)OAL_NETDEVICE_MAC_ADDR(net_dev), primary_mac_addr);
#endif

    return OAL_SUCC;
}

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* 不支持ethtool_ops 相关函数操作,需要显示定义空的结构体，否则将采用内核默认的ethtool_ops会导致空指针异常 */
oal_ethtool_ops_stru g_st_wal_ethtool_ops_etc = { 0 };
#endif

/*****************************************************************************
 功能描述  : 初始化wlan0和p2p0设备 - 初始化net_device
*****************************************************************************/
OAL_STATIC osal_void wal_init_wlan_init_netdev(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev,
    enum nl80211_iftype type, oal_wiphy_stru *wiphy)
{
    (osal_void)memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));

#ifdef _PRE_WLAN_FEATURE_GSO
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev::[GSO] add NETIF_F_SG}");
    net_dev->features |= NETIF_F_SG;
    net_dev->hw_features |= NETIF_F_SG;
#endif
    /* 对netdevice进行赋值 */
#if defined (_PRE_WLAN_WIRELESS_EXT) && defined(CONFIG_WIRELESS_EXT)
    net_dev->wireless_handlers             = &g_st_iw_handler_def_etc;
#endif
    net_dev->netdev_ops                    = &g_st_wal_net_dev_ops_etc;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    net_dev->ethtool_ops                   = &g_st_wal_ethtool_ops_etc;
#endif
    OAL_NETDEVICE_DESTRUCTOR(net_dev)      = oal_net_free_netdev;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
    oal_netdevice_master(net_dev)          = OSAL_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(net_dev)         = OSAL_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(net_dev)  = 5; /* watchdog_timeo配置为5 */
    OAL_NETDEVICE_WDEV(net_dev)            = wdev;
    oal_netdevice_qdisc(net_dev, OSAL_NULL);

    wdev->netdev = net_dev;
    wdev->iftype = type;
    wdev->wiphy = wiphy;
}

/*****************************************************************************
 功能描述  : 初始化wlan0和p2p0设备
*****************************************************************************/
OAL_STATIC osal_u32 wal_init_wlan_netdev_ext(char *dev_name, enum nl80211_iftype type, oal_wiphy_stru *wiphy)
{
    oal_net_device_stru        *net_dev = OSAL_NULL;
    oal_wireless_dev_stru      *wdev = OSAL_NULL;
    mac_wiphy_priv_stru        *wiphy_priv;
    osal_u32                   ret;

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = oal_dev_get_by_name(dev_name);
    if (net_dev != OSAL_NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_create_netdev::the net_device already exists!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(0, dev_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
#else
    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev(0, dev_name, oal_ether_setup);
#endif

    if (OAL_UNLIKELY(net_dev == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_create_netdev::oal_net_alloc_netdev return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wdev = (oal_wireless_dev_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(wdev == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_init_wlan_create_netdev::alloc mem, wdev is null ptr!}");
        oal_net_free_netdev(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    wal_init_wlan_init_netdev(net_dev, wdev, type, wiphy);

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wiphy));

    /* 信道跟随--add wlan name 判断 */
    if (type == NL80211_IFTYPE_STATION) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        wiphy_priv->hmac_device->st_p2p_info.primary_net_device = net_dev;
    } else if (type == NL80211_IFTYPE_AP) {
        /* 信道跟随--add hwlan name 判断 */
        wiphy_priv->hmac_device->st_p2p_info.second_net_device = net_dev;
    
#ifdef _PRE_WLAN_FEATURE_P2P
    }else if (type == NL80211_IFTYPE_P2P_DEVICE) {
        wiphy_priv->hmac_device->st_p2p_info.p2p_net_device = net_dev;
#endif
    }
    OAL_NETDEVICE_FLAGS(net_dev) &= ~OAL_IFF_RUNNING;   /* 将net device的flag设为down */
    wal_set_mac_addr(net_dev);

    /* 注册net_device */
    ret = oal_net_register_netdev(net_dev);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_register_netdev::oal_net_register_netdev err code %d}", ret);
        oal_mem_free(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 初始化wlan0和p2p0设备
 输入参数  : oal_wiphy_stru *wiphy
                         : char *dev_name
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月25日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_init_wlan_netdev_etc(oal_wiphy_stru *wiphy, enum nl80211_iftype type)
{
    hmac_device_stru *hmac_device = OSAL_NULL;
    mac_wiphy_priv_stru *wiphy_priv = OSAL_NULL;
    osal_s8 dev_name[MAC_NET_DEVICE_NAME_LENGTH] = {0};
    osal_s32 ret;
    osal_u32 return_code;

    if (wiphy == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::wiphy is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)(oal_wiphy_priv(wiphy));
    if (wiphy_priv == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::wiphy_priv is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = wiphy_priv->hmac_device;
    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::type is %d}", type);

    /* 信道跟随--添加hwlan name判断 */
    if (type == NL80211_IFTYPE_STATION) {
        ret = sprintf_s(dev_name, MAC_NET_DEVICE_NAME_LENGTH, wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX));
    } else if (type == NL80211_IFTYPE_AP) {
        ret = sprintf_s(dev_name, MAC_NET_DEVICE_NAME_LENGTH, wal_get_wlan_name_config(NETDEV_AP_MAC_INDEX));
#ifdef _PRE_WLAN_FEATURE_P2P
    } else if (type == NL80211_IFTYPE_P2P_DEVICE) {
        ret = sprintf_s(dev_name, MAC_NET_DEVICE_NAME_LENGTH, wal_get_wlan_name_config(NETDEV_P2P_MAC_INDEX));
#endif
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::type %d error}", type);
        return OAL_SUCC;
    }
    if (ret < 0) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_init_wlan_netdev_etc::sprintf_s error, type %d ret %d}", type, ret);
    }

    return_code = wal_init_wlan_netdev_ext(dev_name, type, wiphy);
    if (return_code != OAL_SUCC) {
        oam_warning_log2(0, 0, "wal_init_wlan_netdev_etc:wal_init_wlan_netdev_ext err %d type %d", return_code, type);
        return return_code;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_setup_ap_etc
 功能描述  : wal侧创建ap的总接口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年12月2日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_setup_ap_etc(oal_net_device_stru *net_dev)
{
    osal_s32 l_ret;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    /* 起ap时,校验 1) 是否用p2p0启动 2) 是否已存在p2p */
    if (wal_netdev_open_ap_pre_check(net_dev) != OAL_SUCC) {
        return -OAL_EFAUL;
    }

#if defined(_PRE_PRODUCT_ID_HOST)
    l_ret = wal_set_power_on(net_dev, OAL_TRUE);
    if (OAL_SUCC != l_ret) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_setup_ap_etc::wal_set_power_on fail [%d]!}", l_ret);
        return l_ret;
    }
#endif /* #if _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap) {
        if (hmac_vap->vap_state != MAC_VAP_STATE_INIT) {
            /* 切换到AP前如果网络设备处于UP状态，需要先down wlan0网络设备 */
            wal_netdev_stop_etc(net_dev);
        }
    }

    net_dev->ieee80211_ptr->iftype = NL80211_IFTYPE_AP;

    l_ret = wlan_open_proc(net_dev);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    l_ret = wal_init_wlan_vap_etc(net_dev);
    return l_ret;
}

#endif
/*****************************************************************************
 功能描述  : set ap mac filter mode to hmac - set filter
 被调函数  : wal_config_mac_filter(oal_net_device_stru *net_dev,
                                  osal_s8 *command)
*****************************************************************************/
OAL_STATIC osal_s32 wal_config_send_mac_filter_event(oal_net_device_stru *net_dev, osal_u32 mac_cnt,
    osal_s8 *pc_parse_command, osal_u32 off_set)
{
    osal_u32                  i;
    osal_u32                  ret_code;
    osal_s8                    ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN] = {0};
    osal_s32                   ret;
    osal_u16 msg_id;
    mac_blacklist_stru blacklist;
    (osal_void)memset_s(&blacklist, OAL_SIZEOF(blacklist), 0, OAL_SIZEOF(blacklist));

    for (i = 0; i < mac_cnt; i++) {
        pc_parse_command += off_set;
        ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC=", &off_set,
            WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
        if (ret_code != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_config_send_mac_filter_event::wal_get_parameter_from_cmd err [%u]!}",
                             ret_code);
            return -OAL_EINVAL;
        }
        /* 5.1  检查参数是否符合MAC长度 */
        if (osal_strlen((const osal_s8 *)ac_parsed_command) != WLAN_MAC_ADDR_LEN * 2) { /* osal_u8 1位对应2位mac地址字符 */
            oam_warning_log0(0, OAM_SF_ANY, "{wal_config_send_mac_filter_event::invalid MAC format}");
            return -OAL_EINVAL;
        }
        /* 6. 添加过滤设备 */
        oal_strtoaddr(ac_parsed_command, blacklist.auc_mac_addr); /* 将字符 ac_name 转换成数组 mac_add[6] */
        if (i == (mac_cnt - 1)) {
            /* 等所有的mac地址都添加完成后，才进行关联用户确认，是否需要删除 */
            msg_id = WLAN_MSG_W2H_CFG_ADD_BLACK_LIST;
        } else {
            msg_id = WLAN_MSG_W2H_CFG_ADD_BLACK_LIST_ONLY;
        }

        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), msg_id,
            (osal_u8 *)&blacklist, OAL_SIZEOF(blacklist));
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_config_send_mac_filter_event::wal_sync_post2hmac_no_rsp err %d!}", ret);
            return ret;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_config_mac_filter
 功能描述  : set ap mac filter mode to hmac
 输入参数  : oal_net_device_stru *net_dev
                         : osal_s8 *command
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年8月18日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_config_mac_filter(oal_net_device_stru *net_dev, osal_s8 *command)
{
    osal_s8                    ac_parsed_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    osal_s8                   *pc_parse_command = OSAL_NULL;
    osal_u32                  mac_mode;
    osal_u32                  mac_cnt;
    osal_u32                  ret_code;
    osal_u32                  off_set;
    osal_s32                   ret;

    if (command == OSAL_NULL) {
        return -OAL_EINVAL;
    }
    pc_parse_command = command;

    /* 解析MAC_MODE */
    ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_MODE=", &off_set,
        WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_config_mac_filter::wal_get_parameter_from_cmd return err_code %u.}", ret_code);
        return -OAL_EINVAL;
    }
    mac_mode = (osal_u32)oal_atoi(ac_parsed_command);
    if (mac_mode > 2) { /* 检查参数是否合法 0,1,2 */
        oam_warning_log4(0, OAM_SF_ANY, "{wal_config_mac_filter::invalid MAC_MODE[%c%c%c%c]!}",
                         (osal_u8)ac_parsed_command[0],
                         (osal_u8)ac_parsed_command[1],
                         (osal_u8)ac_parsed_command[2],   /* mac mode第2位 */
                         (osal_u8)ac_parsed_command[3]);  /* mac mode第3位 */
        return -OAL_EINVAL;
    }

    /* 设置过滤模式 */
    ret_code = wal_post2hmac_uint32_data(net_dev, (osal_s8 *)ac_parsed_command, WLAN_MSG_W2H_CFG_BLACKLIST_MODE);
    if (ret_code != OAL_SUCC) {
        return (osal_s32)ret_code;
    }
    /* 解析MAC_CNT */
    pc_parse_command += off_set;
    ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parsed_command, "MAC_CNT=", &off_set,
        WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_config_mac_filter::wal_get_parameter_from_cmd return err_code [%u]!}",
                         ret_code);
        return -OAL_EINVAL;
    }
    mac_cnt = (osal_u32)oal_atoi(ac_parsed_command);

    ret = wal_config_send_mac_filter_event(net_dev, mac_cnt, pc_parse_command, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_config_mac_filter:: wal_config_send_mac_filter_event err %d!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_QUE_CLASSIFY
/*****************************************************************************
 函 数 名  : wal_netdev_select_queue_etc
 功能描述  : kernel给skb选择合适的tx subqueue;
 输入参数  :
 输出参数  : 无
 返 回 值  : 无

 修改历史      :
  1.日    期   : 2015年3月17日
    修改内容   : 新生成函数

*****************************************************************************/
/*
 * 因内核版本而异 struct net_device_ops里回调函数ndo_select_queue格式不同
 * linux < 3.13
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb);
 * linux >= 3.13 && linux < 3.14
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb,
 *                         void *accel_priv);
 * linux >= 3.14 && linux < 4.19
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb,
 *                         void *accel_priv, select_queue_fallback_t fallback);
 * linux >= 4.19 && linux < 5.2
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb,
 *                         struct net_device *sb_dev,
 *                         select_queue_fallback_t fallback);
 * linux >= 5.2
 * u16 (*ndo_select_queue)(struct net_device *dev, struct sk_buff *skb,
 *                         struct net_device *sb_dev);
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0))
STATIC osal_u16  wal_netdev_select_queue_etc(oal_net_device_stru *dev, oal_netbuf_stru *buf)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
STATIC osal_u16  wal_netdev_select_queue_etc(oal_net_device_stru *dev, oal_netbuf_stru *buf, void *accel_priv)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
STATIC osal_u16  wal_netdev_select_queue_etc(oal_net_device_stru *dev, oal_netbuf_stru *buf, void *accel_priv,
                                        select_queue_fallback_t fallback)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5,2,0))
STATIC osal_u16  wal_netdev_select_queue_etc(oal_net_device_stru *dev, oal_netbuf_stru *buf,
                                        oal_net_device_stru *sb_dev, select_queue_fallback_t fallback)
#else
STATIC osal_u16 wal_netdev_select_queue_etc(oal_net_device_stru*dev, oal_netbuf_stru *buf,
                                        oal_net_device_stru *sb_dev)
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)) */
{
    return oal_netbuf_select_queue_etc(buf);
}

#endif /* endif of _PRE_WLAN_FEATURE_QUE_CLASSIFY */

/*****************************************************************************
  net_device上挂接的net_device_ops函数
*****************************************************************************/

oal_net_device_ops_stru g_st_wal_net_dev_ops_etc = {
    .ndo_get_stats          = wal_netdev_get_stats,
    .ndo_open               = wal_netdev_open_ext,
    .ndo_stop               = wal_netdev_stop_ext,
    .ndo_start_xmit         = wal_bridge_vap_xmit_etc,
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0))
    .ndo_set_multicast_list = OAL_PTR_NULL,
#endif

    .ndo_do_ioctl           = wal_net_device_ioctl,
    .ndo_change_mtu         = oal_net_device_change_mtu,
    .ndo_init               = oal_net_device_init,

#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    .ndo_select_queue       = wal_netdev_select_queue_etc,
#endif

    .ndo_set_mac_address    = wal_netdev_set_mac_addr
};

/*****************************************************************************
 函 数 名  : wal_bridge_vap_xmit_etc
 功能描述  : 挂接到VAP对应net_device结构体下的发送函数
 输入参数  : buf: SKB结构体,其中data指针指向以太网头
             dev: net_device结构体
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年11月6日
    修改内容   : 新生成函数

*****************************************************************************/
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)) && \
        defined(_PRE_WLAN_MP13_TCP_SMALL_QUEUE_BUGFIX)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
#ifdef KERNEL_4_9_ADAP
static osal_u8 g_sk_pacing_shift = 8;
#endif
#endif
#endif

#ifdef CONTROLLER_CUSTOMIZATION
netdev_tx_t wal_bridge_vap_xmit_etc(struct sk_buff *buf, struct net_device *dev)
#else
osal_s32 wal_bridge_vap_xmit_etc(oal_netbuf_stru *buf, oal_net_device_stru *dev)
#endif
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)) && \
        defined(_PRE_WLAN_MP13_TCP_SMALL_QUEUE_BUGFIX)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
#ifdef KERNEL_4_9_ADAP
    sk_pacing_shift_update(buf->sk, g_sk_pacing_shift);
#endif
#endif
#endif
    if (OAL_UNLIKELY(dev == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{wal_bridge_vap_xmit_etc::dev = OAL_PTR_NULL!}\r\n");
        oal_netbuf_free(buf);
#ifdef CONTROLLER_CUSTOMIZATION
        return (netdev_tx_t)OAL_NETDEV_TX_OK;
#else
        return OAL_NETDEV_TX_OK;
#endif
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        oal_netbuf_free(buf);
#ifdef CONTROLLER_CUSTOMIZATION
        return (netdev_tx_t)OAL_NETDEV_TX_OK;
#else
        return OAL_NETDEV_TX_OK;
#endif
    }
#endif
#ifdef _PRE_WLAN_FEATURE_GSO
    if (skb_linearize(buf)) {
        oam_warning_log0(0, OAM_SF_RX, "{wal_bridge_vap_xmit::[GSO] failed at skb_linearize");
        dev_kfree_skb_any(buf);
#ifdef CONTROLLER_CUSTOMIZATION
        return (netdev_tx_t)OAL_NETDEV_TX_OK;
#else
        return OAL_NETDEV_TX_OK;
#endif
    }
#endif

#ifdef _PRE_WIFI_DEBUG
    if (hal_tx_data_type_debug_on() == OSAL_TRUE) {
        wal_print_tx_data_type((mac_ether_header_stru *)((osal_u8 *)oal_netbuf_payload(buf)));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_WFA_CODE
    if (wal_get_data_type_from_8023_etc((osal_u8 *)oal_netbuf_payload(buf),
        MAC_NETBUFF_PAYLOAD_ETH) == MAC_DATA_ICMP_OTH) {
        oal_netbuf_free(buf);
        wifi_printf("[TX]wal_bridge_vap_xmit_etc discard PKT_TRACE_ICMP_OTH\r\n");
#ifdef CONTROLLER_CUSTOMIZATION
        return (netdev_tx_t)OAL_NETDEV_TX_OK;
#else
        return OAL_NETDEV_TX_OK;
#endif
    }
#endif

    frw_host_post_data(FRW_NETBUF_W2H_DATA_FRAME, wal_util_get_vap_id(dev), buf);
#ifdef CONTROLLER_CUSTOMIZATION
        return (netdev_tx_t)OAL_NETDEV_TX_OK;
#else
        return OAL_NETDEV_TX_OK;
#endif
}

/*****************************************************************************
 功能描述  : 注册网络接口的通知链
 输入参数  : osal_void
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月21日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_ccpriv_register_inetaddr_notifier_etc(osal_void)
{
    if (0 == register_inetaddr_notifier(&wal_ccpriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_ccpriv_register_inetaddr_notifier_etc::register inetaddr notifier failed.}");
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 注销网络接口的通知链
 输入参数  : osal_void
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月21日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_ccpriv_unregister_inetaddr_notifier_etc(osal_void)
{
    if (0 == unregister_inetaddr_notifier(&wal_ccpriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR,
                   "{wal_ccpriv_unregister_inetaddr_notifier_etc::hmac_unregister inetaddr notifier failed.}");
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 注册IPv6网络接口的通知链
 输入参数  : osal_void
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年6月15日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_ccpriv_register_inet6addr_notifier_etc(osal_void)
{
    if (0 == register_inet6addr_notifier(&wal_ccpriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_ccpriv_register_inet6addr_notifier_etc::register inetaddr6 notifier failed.}");
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 注销IPv6网络接口的通知链
 输入参数  : osal_void
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年6月15日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_ccpriv_unregister_inet6addr_notifier_etc(osal_void)
{
    if (0 == unregister_inet6addr_notifier(&wal_ccpriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR,
                   "{wal_ccpriv_unregister_inet6addr_notifier_etc::hmac_unregister inetaddr6 notifier failed.}");
    return OAL_FAIL;
}

/*****************************************************************************
内核网络设备通知链回调参数检查函数
*****************************************************************************/
static osal_u32 wal_ccpriv_inetaddr_notifier_check(osal_ulong event, struct in_ifaddr *ifa)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    if (OAL_UNLIKELY(ifa == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_ccpriv_inetaddr_notifier_call_etc::ifa is NULL.}");
        return OAL_FAIL;
    }
    if (OAL_UNLIKELY(ifa->ifa_dev->dev == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_ccpriv_inet6addr_notifier_call_etc::ifa->idev->dev is NULL.}");
        return OAL_FAIL;
    }

    /* Filter notifications meant for non-self-owned devices */
    if (ifa->ifa_dev->dev->netdev_ops != &g_st_wal_net_dev_ops_etc) {
        return OAL_FAIL;
    }

    hmac_vap = (hmac_vap_stru *)(ifa->ifa_dev->dev->ml_priv);
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_PWR,
                         "{wal_ccpriv_inetaddr_notifier_call_etc::Get mac vap failed, when %d"
                         "(UP:1 DOWN:2 UNKNOWN:others) ipv4 address.}", event);
        return OAL_FAIL;
    }

    if (ipv4_is_linklocal_169(ifa->ifa_address)) {
        oam_warning_log_alter(0, OAM_SF_PWR, 5, /* 打印5个参数 */
            "vap_id[%d] {wal_ccpriv_inetaddr_notifier_call_etc::Invalid IPv4[%d.%d.%d.*], MASK[0x%08X].}",
            hmac_vap->vap_id,
            ((osal_u8 *) & (ifa->ifa_address))[0], /* 0表示IP地址第0字节 */
            ((osal_u8 *) & (ifa->ifa_address))[1], /* 1表示IP地址第1字节 */
            ((osal_u8 *) & (ifa->ifa_address))[2], /* 2表示IP地址第2字节 */
            ifa->ifa_mask);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_void wal_ccpriv_inetaddr_notifier_up(hmac_vap_stru *hmac_vap, struct in_ifaddr *ifa)
{
    dmac_ip_addr_config_stru ip_addr;
    osal_u32 ret;
    osal_u32 ip_state;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (ifa == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ccpriv_inetaddr_notifier_up::hmac_vap or ifa is NULL}");
        return;
    }
    oam_warning_log_alter(0, OAM_SF_PWR, 5, /* 打印5个参数 */
        "vap_id[%d] {wal_ccpriv_inetaddr_notifier_up::Up IPv4[%d.%d.%d.*], MASK[0x%08X].}",
        hmac_vap->vap_id,
        ((osal_u8 *) & (ifa->ifa_address))[0], /* IP地址第0字节 */
        ((osal_u8 *) & (ifa->ifa_address))[1], /* IP地址第1字节 */
        ((osal_u8 *) & (ifa->ifa_address))[2], /* IP地址第2字节 */
        ifa->ifa_mask);
    ip_addr.type = DMAC_CONFIG_IPV4;
    ip_addr.oper = DMAC_IP_ADDR_ADD;
    memcpy_s(&ip_addr.ip_addr, OAL_IPV4_ADDR_SIZE, &(ifa->ifa_address), OAL_IPV4_ADDR_SIZE);
    memcpy_s(&ip_addr.mask_addr, OAL_IPV4_ADDR_SIZE, &(ifa->ifa_mask), OAL_IPV4_ADDR_SIZE);
    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_SET_IPADDR, (osal_u8 *)&ip_addr,
        sizeof(dmac_ip_addr_config_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "wal_ccpriv_inetaddr_notifier_up::post2hmac ret %d", ret);
    }

    if (WLAN_VAP_MODE_BSS_STA != hmac_vap->vap_mode) {
        return;
    }
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP, &(ifa->ifa_address));

    /* 获取到IP地址的时候通知漫游计时 */
    ip_state = WPAS_CONNECT_STATE_IPADDR_OBTAINED;
    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_SET_IP_STATE,
        (osal_u8 *)&ip_state, sizeof(osal_u32));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "wal_ccpriv_inetaddr_notifier_up::ip state post2hmac ret %d", ret);
    }
}

osal_void wal_ccpriv_inetaddr_notifier_down(hmac_vap_stru *hmac_vap, struct in_ifaddr *ifa)
{
    dmac_ip_addr_config_stru ip_addr;
    osal_u32 ret;
    osal_u32 ip_state;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (ifa == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ccpriv_inetaddr_notifier_down::hmac_vap or ifa is NULL}");
        return;
    }
    oam_warning_log_alter(0, OAM_SF_PWR, 5, /* 打印5个参数 */
        "vap_id[%d] {wal_ccpriv_inetaddr_notifier_down::Up IPv4[%d.%d.%d.*], MASK[0x%08X].}",
        hmac_vap->vap_id,
        ((osal_u8 *) & (ifa->ifa_address))[0], /* IP地址第0字节 */
        ((osal_u8 *) & (ifa->ifa_address))[1], /* IP地址第1字节 */
        ((osal_u8 *) & (ifa->ifa_address))[2], /* IP地址第2字节 */
        ifa->ifa_mask);
    ip_addr.type = DMAC_CONFIG_IPV4;
    ip_addr.oper = DMAC_IP_ADDR_DEL;
    memcpy_s(&ip_addr.ip_addr, OAL_IPV4_ADDR_SIZE, &(ifa->ifa_address), OAL_IPV4_ADDR_SIZE);
    memcpy_s(&ip_addr.mask_addr, OAL_IPV4_ADDR_SIZE, &(ifa->ifa_mask), OAL_IPV4_ADDR_SIZE);
    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_SET_IPADDR, (osal_u8 *)&ip_addr,
        sizeof(dmac_ip_addr_config_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "wal_ccpriv_inetaddr_notifier_down::post2hmac ret %d", ret);
    }

    /* 获取到IP地址的时候通知漫游计时 */
    ip_state = WPAS_CONNECT_STATE_IPADDR_REMOVED;
    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_SET_IP_STATE,
        (osal_u8 *)&ip_state, sizeof(osal_u32));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "wal_ccpriv_inetaddr_notifier_down::ip state post2hmac ret %d", ret);
    }
}

/*****************************************************************************
内核网络设备通知链回调函数
*****************************************************************************/
osal_s32 wal_ccpriv_inetaddr_notifier_call_etc(struct notifier_block *this, osal_ulong event, osal_void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    if (wal_ccpriv_inetaddr_notifier_check(event, ifa) != OAL_SUCC) {
        return NOTIFY_DONE;
    }

    hmac_vap = (hmac_vap_stru *)(ifa->ifa_dev->dev->ml_priv);
    wal_wake_lock();

    switch (event) {
        case NETDEV_UP:
            wal_ccpriv_inetaddr_notifier_up(hmac_vap, ifa);
            break;
        case NETDEV_DOWN:
            wal_ccpriv_inetaddr_notifier_down(hmac_vap, ifa);
            break;
        default: {
            oam_error_log2(0, OAM_SF_PWR,
                           "vap_id[%d] {wal_ccpriv_inetaddr_notifier_call_etc::Unknown notifier event[%d].}",
                           hmac_vap->vap_id, event);
            break;
        }
    }
    wal_wake_unlock();
    return NOTIFY_DONE;
}

/*****************************************************************************
内核网络设备通知链回调参数检查ipv6函数
*****************************************************************************/
static osal_u32 wal_ccpriv_inet6addr_notifier_check(struct inet6_ifaddr *ifa, osal_u8 *vap_id, osal_ulong event)
{
    hmac_vap_stru *hmac_vap;

    if (OAL_UNLIKELY(NULL == ifa) || OAL_UNLIKELY(NULL == ifa->idev->dev)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_ccpriv_inet6addr_notifier_call_etc::param is NULL.}");
        return OAL_FAIL;
    }

    /* Filter notifications meant for non-self-owned devices */
    if (ifa->idev->dev->netdev_ops != &g_st_wal_net_dev_ops_etc) {
        return OAL_FAIL;
    }

    hmac_vap = (hmac_vap_stru *)(ifa->idev->dev->ml_priv);
    if (OAL_UNLIKELY(NULL == hmac_vap)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_ccpriv_inet6addr_notifier_call_etc::Get mac vap failed, \
                when %d(UP:1 DOWN:2 UNKNOWN:others) ipv6 address.}", event);
        return OAL_FAIL;
    }

    if (event != NETDEV_UP && event != NETDEV_DOWN) {
        oam_error_log2(0, OAM_SF_PWR, "vap[%d] {wal_ccpriv_inet6addr_notifier_call_etc::Unknown event[%u].}",
            hmac_vap->vap_id, event);
        return OAL_FAIL;
    }

    *vap_id = hmac_vap->vap_id;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : IPv6通知链回调函数
 输入参数  : struct notifier_block *this
             osal_ulong event
             osal_void *ptr
 输出参数  : 无
 返 回 值  : osal_s32

 修改历史      :
  1.日    期   : 2015年6月15日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_ccpriv_inet6addr_notifier_call_etc(struct notifier_block *this, osal_ulong event, osal_void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct inet6_ifaddr *ifa = (struct inet6_ifaddr *)ptr;
    dmac_ip_addr_config_stru ip_addr;
    osal_u8 vap_id;
    osal_u32 ret;

    if (wal_ccpriv_inet6addr_notifier_check(ifa, &vap_id, event) != OAL_SUCC) {
        return NOTIFY_DONE;
    }

#if defined(__UAPI_DEF_IN6_ADDR_ALT) && (__UAPI_DEF_IN6_ADDR_ALT)
    oam_warning_log4(0, OAM_SF_PWR,
        "{wal_ccpriv_inet6addr_notifier_call_etc::event %d IPv6[%04x:%04x:%02xXX:XXXX:XXXX:XXXX:XXXX:XXXX].}", event,
        oal_net2host_short((ifa->addr.s6_addr16)[0]), oal_net2host_short((ifa->addr.s6_addr16)[1]),
        ((oal_net2host_short((ifa->addr.s6_addr16)[2]) >> 8) & 0xFF));
#endif

    ip_addr.type = DMAC_CONFIG_IPV6;
    if (memcpy_s(&ip_addr.ip_addr, OAL_IPV6_ADDR_SIZE, &(ifa->addr), OAL_IPV6_ADDR_SIZE) != EOK) {
        oam_warning_log0(0, OAM_SF_PWR, "wal_ccpriv_inet6addr_notifier_call_etc::memcpy_s ip_addr fail");
    }
    if (memcpy_s(&ip_addr.mask_addr, OAL_IPV6_ADDR_SIZE, &(ifa->addr), OAL_IPV6_ADDR_SIZE) != EOK) {
        oam_warning_log0(0, OAM_SF_PWR, "wal_ccpriv_inet6addr_notifier_call_etc::memcpy_s mask_addr fail");
    }
    if (event == NETDEV_UP) {
        ip_addr.oper = DMAC_IP_ADDR_ADD;
    } else if (event == NETDEV_DOWN) {
        ip_addr.oper = DMAC_IP_ADDR_DEL;
    }

    ret = wal_async_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_SET_IPADDR, (osal_u8 *)&ip_addr,
        OAL_SIZEOF(dmac_ip_addr_config_stru), FRW_POST_PRI_LOW);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "wal_ccpriv_inet6addr_notifier_call_etc::post2hmac ret %d", ret);
    }

    return NOTIFY_DONE;
}

oal_module_symbol(wal_bridge_vap_xmit_etc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
