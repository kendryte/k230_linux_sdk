/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal_linux_dfx
 * Create: 2022-09-23
 */
#include "wal_linux_dfx.h"
#include "wal_main.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hmac_ext_if.h"
#include "wlan_spec.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ccpriv.h"
#include "oal_cfg80211.h"
#include "hmac_vap.h"
#include "wlan_msg.h"
#include "wal_linux_netdev.h"
#include "hmac_dfr.h"
#include "wal_linux_scan.h"
#include "hal_chip.h"
#include "alg_main.h"
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_DFX_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_DFR
static osal_void wal_process_p2p_excp_etc(hmac_vap_stru *hmac_vap);
hmac_vap_stru *g_go_gc_hmac_vap = OSAL_NULL;
hmac_vap_stru *g_p2p_change_hmac_vap = OSAL_NULL;

static osal_void wal_dfr_set_status(wal_dfr_status_enum dfr_status)
{
    wifi_printf("dfr change status[%d] to [%d]\r\n", g_dfr_recovery_info.dfr_status, dfr_status);
    g_dfr_recovery_info.dfr_status = dfr_status;
}

wal_dfr_status_enum wal_dfr_get_status(osal_void)
{
    return g_dfr_recovery_info.dfr_status;
}

/* 记录被DFR流程中P2P恢复前被拦截的change动作 */
osal_void wal_dfr_save_p2p_change_record(hmac_vap_stru *hmac_vap)
{
    if ((hmac_vap == OSAL_NULL) || (wal_dfr_get_status() > WAL_DFR_P2P_RECOVERY)) {
        return;
    }
    if (is_legacy_vap(hmac_vap)) {
        return;
    }
    g_p2p_change_hmac_vap = hmac_vap;
    osal_printk("wal_dfr_save_p2p_change_record vap\r\n");
}

static osal_void wal_dfr_recovery_ap_etc(oal_net_device_stru *netdev)
{
    osal_s32 ret;
    hmac_vap_stru *hmac_vap = netdev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        return;
    }
    // 1. 设置信道
    wal_set_channel_info(hmac_vap->vap_id, g_dfr_ap_info.channel_info.channel, g_dfr_ap_info.channel_info.band,
        g_dfr_ap_info.channel_info.en_bandwidth);
    // 2. 设置ssid等信息
    wal_set_ssid(netdev, g_dfr_ap_info.ssid_info.ac_ssid, g_dfr_ap_info.ssid_info.ssid_len);
    // 2.2 设置auth mode信息
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_AUTH_MODE,
        (osal_u8 *)&g_dfr_ap_info.auth_algs, OAL_SIZEOF(g_dfr_ap_info.auth_algs));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_dfr_recovery_ap_etc::fail to send auth_tpye cfg msg, error[%d]}", ret);
        return;
    }

    // 3. 设置beacon信息
    wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON,
        (osal_u8 *)&g_dfr_ap_info.beacon_param, OAL_SIZEOF(g_dfr_ap_info.beacon_param));
    // 4. 启动vap
    ret = wal_start_vap_etc(netdev);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_recovery_ap_etc::failed to start ap, error[%u]}", ret);
    }
    /* set gtk */
    osal_msleep(WAL_DFR_AP_WAIT_START_TIME_MS); // 100ms休眠，给启动vap时间，避免add key提前运行
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_ADD_KEY,
        (osal_u8 *)&g_dfr_ap_info.payload_params, OAL_SIZEOF(mac_addkey_param_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_recovery_ap_etc::failed to set gtk err code [%u]!}", ret);
    }
    /* set igtk */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_ADD_KEY,
        (osal_u8 *)&g_dfr_ap_info.igtk_key_params, OAL_SIZEOF(mac_addkey_param_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_recovery_ap_etc::failed to set igtk err code [%u]!}", ret);
    }
    return;
}

/* 恢复AP */
static osal_s32 wal_dfr_recovery_ap(oal_net_device_stru *netdev)
{
    osal_s32 ret = OAL_SUCC;

    // 创建vap
    ret = wal_setup_ap_etc(netdev);

    // 恢复softap的配置
    wal_dfr_recovery_ap_etc(netdev);

    // netdev running态
    oal_net_device_open(netdev);

    wal_set_ap_power_flag(OSAL_FALSE);

    return ret;
}

static osal_void wal_dfr_open_dev(oal_net_device_stru *netdev)
{
    rtnl_lock();
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
    dev_open(netdev);
#else
    dev_open(netdev, NULL);
#endif
    rtnl_unlock();
}

static osal_u32 wal_dfr_recovery_all_vap(void)
{
    oal_wireless_dev_stru *wireless_dev = NULL;
    oal_net_device_stru *netdev = NULL;
    osal_s32 ret_event = OAL_SUCC;
    for (; (g_dfr_recovery_info.netdev_num > 0 && g_dfr_recovery_info.netdev_num < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
        g_dfr_recovery_info.netdev_num--) {
        netdev = g_dfr_recovery_info.dfr_netdev[g_dfr_recovery_info.netdev_num - 1];
        wifi_printf("[%s][%d]iftype[%d] netdevnum[%d]\r\n", __FUNCTION__, __LINE__,
            netdev->ieee80211_ptr->iftype, g_dfr_recovery_info.netdev_num);
        if (netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
            // 恢复业务AP
            wal_dfr_recovery_ap(netdev);
        } else if (netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION
#ifdef _PRE_WLAN_FEATURE_P2P
            || netdev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_DEVICE
#endif
        ) {
            // 拉起netdev
            wal_netdev_open_etc(netdev, OAL_FALSE);
            // 置为up状态 (go/gc 恢复为dev后默认为DOWN,要调用此函数up起来)
            wal_dfr_open_dev(netdev);
        } else {
            wireless_dev = OAL_NETDEVICE_WDEV(netdev);
            /* 去注册netdev */
            oal_net_unregister_netdev(netdev);
            oal_mem_free(wireless_dev, OAL_TRUE);
            continue;
        }
    }
    return ret_event;
}
/* DFR收尾操作 */
static osal_void wal_dfr_end(void)
{
    g_dfr_recovery_info.bit_device_reset_process_flag = OSAL_FALSE;
    g_dfr_recovery_info.bit_ready_to_recovery_flag = OSAL_FALSE;
    g_p2p_change_hmac_vap = OSAL_NULL;
    g_go_gc_hmac_vap = OSAL_NULL;
    g_dfr_recovery_info.netdev_num = 0;
    // 通知平台wlan自愈完毕
    wlan_set_dfr_recovery_flag(OSAL_FALSE);
    wal_dfr_set_status(WAL_DFR_RECOVERY_INIT);
    wifi_printf("[%s][%d]dfr recovery cnt[%d]\r\n", __FUNCTION__, __LINE__, g_dfr_recovery_info.dfr_num);
    oam_warning_log1(0, OAM_SF_ANY, "wal_dfr_recovery_env dfr recovery cnt[%d]!}", g_dfr_recovery_info.dfr_num);
}

/* 开始恢复vap */
static osal_u32 wal_dfr_recovery_env(void)
{
    int32_t ret = OAL_SUCC;
    /* 这里处理GO/GC */
    if (g_go_gc_hmac_vap != OSAL_NULL) {
        wal_process_p2p_excp_etc(g_go_gc_hmac_vap);
    }
    wal_dfr_set_status(WAL_DFR_STA_AP_RECOVERY);
    /* 恢复vap */
    ret = wal_dfr_recovery_all_vap();
    wal_dfr_set_status(WAL_DFR_FINISH_RECOVERY);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_dfr_recovery_env recovery vap fail ret[%d]", ret);
    }
    wal_dfr_end();
    return ret;
}

/*
 * 函 数 名  : wal_dfr_destroy_vap
 * 功能描述  : dfr流程中删除vap
 */
static osal_s32 wal_dfr_destroy_vap(hmac_vap_stru *hmac_vap, oal_net_device_stru *netdev)
{
    osal_s32 ret;
    osal_u32 destroy_enable = 0; /* invalid data */

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_DESTROY_VAP,
        (osal_u8 *)&destroy_enable, OAL_SIZEOF(osal_u32));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_dfr_destroy_vap::return err code [%u]!}", ret);
        return OAL_FAIL;
    }

    // 清空hmac_vap, 之后恢复时会重新创建
    // 但softap hmac_vap的配置需要保存，在上层不感知情况下，自我配置生效
    netdev->ml_priv = OSAL_NULL;
    oam_warning_log0(0, OAM_SF_DFR, "{wal_dfr_destroy_vap::finish!}");
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_dfr_kick_all_user
 * 功能描述  : 剔除vap下面的所有用户, 广播地址（配置vap除外）
 */
static osal_void wal_dfr_kick_all_user(hmac_vap_stru *hmac_vap, osal_u16 reason)
{
    osal_s32 ret;
    mac_cfg_kick_user_param_stru kick_user_param = {0};
    /* 设置配置命令参数 */
    oal_set_mac_addr(kick_user_param.mac_addr, BROADCAST_MACADDR);

    /* 填写去关联reason code */
    kick_user_param.reason_code = reason;
    if (hmac_vap->net_device == OSAL_NULL) {
        wifi_printf("{wal_dfr_kick_all_user::pst_net_device is null!}");
        return;
    }

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_KICK_USER,
        (osal_u8 *)&kick_user_param, OAL_SIZEOF(mac_cfg_kick_user_param_stru));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        wifi_printf("{wal_dfr_kick_all_user::return err code [%u]!}", ret);
        return;
    }

    return;
}
/*
 * 函 数 名  : hmac_process_sta_excp
 * 功能描述  : dfr恢复前的异常处理
 */
static osal_u32 wal_process_excp_vap(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_dev;
    hmac_dev = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_dev == OSAL_NULL) {
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    wifi_printf("[%s][%d]vap mode[%d]p2p mode[%d]\r\n", __FUNCTION__, __LINE__, hmac_vap->vap_mode, hmac_vap->p2p_mode);
    // go/gc 要转为device状态,单独处理
    if (is_p2p_go(hmac_vap) || is_p2p_cl(hmac_vap)) {
        return OAL_SUCC;
    }
    /* 关联状态下上报关联失败，删除用户 */
    wal_dfr_kick_all_user(hmac_vap, MAC_UNSPEC_REASON);
    osal_msleep(WAL_DFR_WAIT_KICK_USER_MS);
    if (is_ap(hmac_vap)) {
        return OAL_SUCC;
    }
    /* 删除扫描信息列表，停止扫描 */
    if (hmac_dev->scan_mgmt.scan_record_mgmt.vap_id == hmac_vap->vap_id) {
        hmac_dev->scan_mgmt.scan_record_mgmt.p_fn_cb = OSAL_NULL;
        hmac_dev->scan_mgmt.is_scanning = OSAL_FALSE;
    }

    return OAL_SUCC;
}

/* 主动触发P2P CHANGE, 将GO/GC转为DEV */
static osal_void wal_dfr_p2p_change(oal_net_device_stru *net_dev)
{
    hmac_cfg_del_vap_param_stru del_vap_param = {0};
    hmac_cfg_add_vap_param_stru add_vap_param = {0};

    add_vap_param.add_vap.vap_mode = WLAN_VAP_MODE_BSS_STA;
    add_vap_param.add_vap.p2p_mode = WLAN_P2P_DEV_MODE;

    wal_cfg80211_change_virtual_intf(net_dev, &del_vap_param, &add_vap_param, NL80211_IFTYPE_STATION);
}

static osal_void wal_process_p2p_excp_etc(hmac_vap_stru *hmac_vap)
{
    osal_u8 vap_idx, time;
    oal_net_device_stru *net_dev = OSAL_NULL;
    hmac_vap_stru *hmac_vap_tmp = OSAL_NULL;
    oal_net_device_stru *net_dev_tmp = OSAL_NULL;
    /* 只恢复GO/GC */
    if (is_legacy_vap(hmac_vap)) {
        wifi_printf("wal_process_p2p_excp_etc recovery p2p mode err vap mode[%d]\r\n", hmac_vap->vap_mode);
        return;
    }
    if (is_p2p_dev(hmac_vap)) {
        return;
    }
    net_dev = hmac_vap->net_device;
    if (net_dev == OSAL_NULL) {
        return;
    }
    wal_dfr_set_status(WAL_DFR_P2P_RECOVERY);
    wifi_printf("[%s][%d] start p2p recovery p2p mode[%d]\r\n", __FUNCTION__, __LINE__, hmac_vap->p2p_mode);
    /*  go/gc做p2p down时,会触发change流程
        DFR时,如果在P2P恢复前,由外部下发了ifconfig p2p down,虽然驱动会拦截动作,但kernel/WPA仍然认为已做过change;
        从而导致P2P恢复时做close/deauth操作不会触发上层下发change
        故在此处检测,如果P2P恢复前拦截过上层change,则在此处主动change恢复
    */
    if (g_p2p_change_hmac_vap == g_go_gc_hmac_vap) {
        wifi_printf("[%s][%d] change record detected active change\r\n", __FUNCTION__, __LINE__);
        wal_dfr_p2p_change(net_dev);
        return;
    }
    if (is_p2p_go(hmac_vap)) {
        /* go触发down操作,会立刻触发wpa supplicant P2P_GROUP_REMOVAL_UNAVAILABLE错误, 触发change流程转为p2p dev */
        oal_net_close_dev(net_dev);
    } else {
        /* gc上报错误码3,触发wpa_supplicant P2P_GROUP_REMOVAL_GO_ENDING_SESSION错误,立刻调change流程转为p2p dev */
        wal_dfr_kick_all_user(hmac_vap, MAC_DEAUTH_LV_SS);
    }
    /* 等待go/gc变为dev(命令由supplicant下发,最大等20s) */
    for (time = 0; time < WAL_P2P_CHANGE_TIME_MAX; time++) {
        for (vap_idx = 0; vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_idx++) {
            hmac_vap_tmp = (hmac_vap_stru *)hmac_get_vap(vap_idx);
            if (hmac_vap_tmp == OSAL_NULL) {
                continue;
            }
            net_dev_tmp = hmac_vap_tmp->net_device;
            if (net_dev_tmp == OSAL_NULL) {
                continue;
            }
            if (is_p2p_dev(hmac_vap_tmp)) {
                wifi_printf("[%s][%d]change to P2P device\r\n", __FUNCTION__, __LINE__);
                return;
            }
        }
        wifi_printf("[%s][%d]waiting..p2p mode[%d]change to P2P dev\r\n", __FUNCTION__, __LINE__, hmac_vap->p2p_mode);
        osal_msleep_uninterruptible(WAL_P2P_CHANGE_WAIT_MS);
    }
    /* 超时强行change */
    wifi_printf("[%s][%d]wait change time out; force change\r\n", __FUNCTION__, __LINE__);
    wal_dfr_p2p_change(net_dev);
}

osal_void wal_dfr_excp_init(osal_void)
{
    g_dfr_recovery_info.bit_device_reset_process_flag = OSAL_TRUE;
    g_dfr_recovery_info.dfr_num++;
    (osal_void)memset_s((osal_u8 *)(g_dfr_recovery_info.dfr_netdev),
        sizeof(g_dfr_recovery_info.dfr_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT, 0,
        sizeof(g_dfr_recovery_info.dfr_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
    set_wlan_flag_close();
}

/* 函 数 名  : wal_dfr_excp_rx
 * 功能描述  : device异常接收总入口，目前用于device挂死，SDIO读写失败等异常
 */
osal_u32 wal_dfr_excp_rx(osal_u8 device_id)
{
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_net_device_stru *netdev = OSAL_NULL;
    osal_s32 ret;

    g_go_gc_hmac_vap = OSAL_NULL;
    wal_dfr_set_status(WAL_DFR_START_RECOVERY);
    wal_dfr_excp_init();

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
        /* 记录netdev */
        g_dfr_recovery_info.dfr_netdev[g_dfr_recovery_info.netdev_num] = netdev;
        g_dfr_recovery_info.netdev_num++;
        /* 进行vap异常处理,随后删除vap */
        wal_process_excp_vap(hmac_vap);
        // 删除netdev
        wal_force_scan_complete_etc(netdev, OSAL_TRUE);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
        wal_stop_sched_scan_etc(netdev);
#endif
        // 共端口方案,GO\GC要先转变成netdev, 此处单独记录go/gc的hmac vap放到后面处理
        if (is_p2p_go(hmac_vap) || is_p2p_cl(hmac_vap)) {
            g_go_gc_hmac_vap = hmac_vap;
            continue;
        }
        // 标识netdev关闭
        oal_net_device_close(netdev);
        // 删除vap
        ret = wal_dfr_destroy_vap(hmac_vap, netdev);
        if (ret != OAL_SUCC) {
            g_dfr_recovery_info.netdev_num--;
            continue;
        }
    }
    /* 置标志位,标识异常处理结束,开始恢复 */
    g_dfr_recovery_info.bit_ready_to_recovery_flag = OSAL_TRUE;

    return OAL_SUCC;
}

osal_void wal_dfr_init_param(osal_void)
{
    (osal_void)memset_s((osal_u8 *)&g_dfr_recovery_info, sizeof(hmac_dfr_info_stru), 0, sizeof(hmac_dfr_info_stru));
    (osal_void)memset_s((osal_u8 *)&g_dfr_ap_info, sizeof(hmac_ap_dfr_recovery_info), 0,
        sizeof(hmac_ap_dfr_recovery_info));
    g_dfr_recovery_info.bit_device_reset_process_flag = OAL_FALSE;
    g_dfr_recovery_info.bit_ready_to_recovery_flag = OAL_FALSE;
}

/* 自愈命令入口 */
osal_u32 uapi_ccpriv_dfr_start(oal_net_device_stru *net_dev, osal_s8 *param)
{
    hmac_device_stru *mac_device = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_test_dfr_start::hmac_vap is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_test_dfr_start::mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 开始业务清理, 此处自愈动作已拆分到业务恢复中 */
    wal_dfr_excp_rx(mac_device->device_id);
    return OAL_SUCC;
}

/* dfr自愈过程中启动wlan时对各个模块重新初始化 */
osal_void wal_dfr_main_reinit_later(osal_void)
{
    // 先集体exit
    alg_hmac_main_exit();
    hmac_board_exit();
    hal_chip_exit();
    // 在集体init, 与hmac_main_init_later流程保持一致
    hal_chip_init();
    hmac_board_init();
    alg_hmac_main_init();
}

/* DFR 业务清理 */
static osal_void dfr_rsv_service_clean(osal_void)
{
    oal_net_device_stru *net_dev;

    net_dev = oal_dev_get_by_name("Featureid0");
    if (net_dev == OSAL_NULL) {
        oam_warning_log0(0, 0, "dfr_rsv_start::net_dev null");
        return;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    uapi_ccpriv_dfr_start(net_dev, OSAL_NULL);
}

/* DFR 清理后业务恢复 */
static osal_u32 dfr_rsv_service_recover(osal_void)
{
    // 异常处理完毕，开始恢复
    return wal_dfr_recovery_env();
}

static osal_void dfr_rsv_start(osal_void)
{
    dfr_rsv_service_clean();
    dfr_rsv_service_recover();
}

osal_u32 wal_dfx_init(void)
{
    wal_dfr_init_param();
    plat_wifi_exception_rst_register_etc(dfr_rsv_start);
    pm_wifi_suspend_cb_host_register(dfr_rsv_service_clean);
    pm_wifi_resume_cb_host_register(dfr_rsv_service_recover);
    return OAL_SUCC;
}

osal_void wal_dfx_exit(osal_void)
{
    pm_wifi_suspend_cb_host_register(NULL);
    pm_wifi_resume_cb_host_register(NULL);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
