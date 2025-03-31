/*
 * Copyright (c) CompanyNameMagicTag 2015-2023. All rights reserved.
 * 文 件 名   : hmac_device.c
 * 生成日期   : 2015年1月31日
 * 功能描述   : hmac device对应操作函数实现的源文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_device.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "mac_regdomain.h"
#include "mac_vap_ext.h"

#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_rx_filter.h"

#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_STAT
#include "hmac_stat.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#include "hmac_data_acq.h"
#include "hmac_rx_filter.h"

#include "hmac_config.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_rx_data_feature.h"
#include "hmac_dfx.h"
#include "hmac_protection.h"
#include "frw_util_notifier.h"

#include "hmac_btcoex.h"
#include "hmac_feature_interface.h"
#include "hmac_feature_dft.h"

#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt_struc.h"
#include "hmac_tcp_opt.h"
#endif

#include "plat_pm_wlan.h"

#include "hmac_feature_main.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "mac_device_ext.h"
#endif

#include "hmac_fcs.h"
#include "hmac_alg_notify.h"
#include "hmac_scan.h"
#include "hmac_beacon.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#include "hmac_dfr.h"
#include "hmac_feature_interface.h"
#include "hmac_wapi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DEVICE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_module_license("GPL");
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_u32 hmac_device_init_hmac_device(osal_u8 dev_id);
OAL_STATIC osal_u32 hmac_cfg_vap_get_devname(const hmac_device_stru *hmac_device,
    osal_s8 *netdev_name, osal_u8 name_len);
OAL_STATIC osal_u32 hmac_device_exit_etc_handle(hmac_device_stru *hmac_device);

osal_s32 hmac_config_reset_mac_phy(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);

    hal_reset_phy_machw(HAL_RESET_HW_TYPE_MAC_PHY, HAL_RESET_MAC_ALL, OSAL_FALSE, OSAL_FALSE);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_device_exit_etc
 功能描述  : 去初始化hmac device级别参数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_device_exit_etc(hmac_device_stru *hmac_device)
{
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_exit_etc::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 扫描模块去初始化 */
    hmac_scan_exit_etc(hmac_device);

    /* 亮暗屏去注册 */

#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&hmac_device->early_suspend);
#endif
    osal_adapt_wait_destroy(&hmac_device->netif_change_event);
    return hmac_device_exit_etc_handle(hmac_device);
}

OSAL_STATIC osal_void hmac_device_feature_exit(hmac_device_stru *hmac_device)
{
#ifdef _PRE_WLAN_DFT_STAT
    hmac_dft_vap_stat_deinit(0);
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    /* DFS特性退出，释放内存 */
    hmac_dfs_deinit(hmac_device);
#endif

    /* hmac_device特性指针数组内存释放 */
    hmac_device_feature_deinit(hmac_device);
}

OAL_STATIC osal_u32 hmac_device_exit_etc_handle(hmac_device_stru *hmac_device)
{
    osal_s32                    l_return;
    hmac_vap_stru                *hmac_vap;
    hmac_cfg_del_vap_param_stru   down_vap;
    osal_u8                     vap_idx;

    osal_u32                    ul_ret;

    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    /* 由于配置vap初始化在HMAC做，所以配置VAP卸载也在HMAC做 */
    vap_idx = hmac_device->cfg_vap_id;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_idx);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_exit_etc_handle::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    msg_info.data = (osal_u8 *)&down_vap;
    msg_info.data_len = OAL_SIZEOF(hmac_cfg_del_vap_param_stru);
    l_return = hmac_config_del_vap_etc(hmac_vap, &msg_info);
    if (l_return != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_device_exit_etc_handle::del_vap failed[%d].}",
            hmac_vap->vap_id, l_return);
        return (osal_u32)l_return;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        /* 获取最右边一位为1的位数，此值即为vap的数组下标 */
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_device_exit_etc_handle::vap failed vap_idx[%u].}", vap_idx);
            continue;
        }
        l_return  = hmac_vap_destroy_etc(hmac_vap, &msg_info);
        if (l_return != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d]{hmac_device_exit_etc_handle::hmac_vap_destroy_etc failed[%d].}",
                hmac_vap->vap_id, l_return);
            return (osal_u32)l_return;
        }
        hmac_device->vap_id[vap_idx] = 0;
    }

    /* 释放公共结构体 以及 对应衍生特性 */
    ul_ret = mac_device_exit_etc(hmac_device);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_device_exit_etc_handle::p_device_destroy_fun failed[%d].}", ul_ret);
        return ul_ret;
    }
    /* 特性退出 */
    hmac_device_feature_exit(hmac_device);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_cfg_vap_init
 功能描述  : 配置VAP初始化
 输入参数  : dev_id: 设备id
 返 回 值  : 错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_cfg_vap_init(hmac_device_stru *hmac_device)
{
    osal_s8             ac_vap_netdev_name[MAC_NET_DEVICE_NAME_LENGTH];
    osal_u32            ul_ret;
    hmac_vap_stru       *hmac_vap = OAL_PTR_NULL;
    osal_u8             vap_id;

    /* 初始化流程中，只初始化配置vap，其他vap需要通过配置添加 */
    ul_ret = mac_res_alloc_hmac_vap(&vap_id);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_cfg_vap_init::mac_res_alloc_hmac_vap failed[%d].}",
            hmac_device->cfg_vap_id, ul_ret);
        return ul_ret;
    }

    hmac_device->cfg_vap_id = vap_id;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_cfg_vap_init::hmac_vap null.}", hmac_device->cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 把hmac_vap_stru结构体初始化赋值为0 */
    memset_s(hmac_vap, OAL_SIZEOF(hmac_vap_stru), 0, OAL_SIZEOF(hmac_vap_stru));

    {
        hmac_cfg_add_vap_param_stru  param = {0};       /* 构造配置VAP参数结构体 */
        param.add_vap.vap_mode = WLAN_VAP_MODE_CONFIG;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        param.add_vap.is_11ac2g_enable = OAL_FALSE;
        param.add_vap.disable_capab_2ght40 = OAL_FALSE;
#endif
        ul_ret = hmac_vap_init_etc(hmac_vap,
                                   hmac_device->chip_id,
                                   hmac_device->device_id,
                                   hmac_device->cfg_vap_id,
                                   &param);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_cfg_vap_init::hmac_vap_init_etc failed[%d].}",
                hmac_device->cfg_vap_id, ul_ret);
            return ul_ret;
        }
    }

    ul_ret = hmac_cfg_vap_get_devname(hmac_device, ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH);
    if (ul_ret != OAL_CONTINUE) {
        return ul_ret;
    }

    ul_ret = hmac_vap_creat_netdev_etc(hmac_vap, ac_vap_netdev_name, MAC_NET_DEVICE_NAME_LENGTH,
        (osal_s8 *)(hmac_device->hw_addr));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_cfg_vap_init::hmac_vap_creat_netdev_etc failed[%d].}",
            hmac_device->cfg_vap_id, ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_cfg_vap_get_devname(const hmac_device_stru *hmac_device,
    osal_s8 *netdev_name, osal_u8 name_len)
{
    (osal_void)snprintf_s((osal_char *)netdev_name, name_len, name_len, "Featureid%u", hmac_device->chip_id);

    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : device结构以及对应特性退出
*****************************************************************************/
OSAL_STATIC osal_void hmac_device_exit(hmac_device_stru *hmac_device)
{
    osal_void *fhook = NULL;
    hal_to_dmac_device_stru *hal_device = hmac_device->hal_device[0];
    if (hal_device != OSAL_NULL) {
#ifdef _PRE_WLAN_FEATURE_M2S
        /* m2s 状态机注销 */
        hmac_m2s_fsm_detach(hal_device);
#endif

        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_DEVICE_EXIT_DESTROY_TIMER);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_device_exit_destroy_timer_cb)fhook)(hal_device);
        }

        hal_device->tbtt_update_beacon_func = OSAL_NULL;

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
        /* 删除动态校准通道切换定时器 */
#endif
    }

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    osal_spin_lock_destroy(&hmac_device->lock);
#endif

    /* 取消keepalive定时器 */
    if (hmac_device->keepalive_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_device->keepalive_timer));
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (hmac_device->obss_scan_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&hmac_device->obss_scan_timer);
    }
#endif

    (osal_void)memset_s(hmac_device->hal_device, sizeof(hmac_device->hal_device[0]) * WLAN_DEVICE_MAX_NUM_PER_CHIP,
        0, sizeof(hmac_device->hal_device[0]) * WLAN_DEVICE_MAX_NUM_PER_CHIP);
#ifdef _PRE_WLAN_FEATURE_STAT
    /* 吞吐统计定时器退出 */
    hmac_throughput_timer_deinit();
#endif
}

/*****************************************************************************
 功能描述  : 删除board
*****************************************************************************/
osal_void hmac_board_exit(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    hmac_device_exit(hmac_device);
}

/*****************************************************************************
 函 数 名  : hmac_cfg_vap_init_etc
 功能描述  : dmac_cfg_vap_init_event需要移植到此处.dmac初始化完成后，需要增量和hmac的握手
*****************************************************************************/
static osal_u32 hmac_cfg_vap_init_etc(hmac_device_stru *hmac_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    hmac_device->cfg_vap_id = 0;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    hmac_vap->hal_device = hmac_device->hal_device[HAL_DEVICE_ID_MASTER]; // cfg vap都默认挂在自己的主路上
    hmac_vap->hal_vap = OSAL_NULL;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_register_hal_dev_cb
 功能描述  : 注册hal device回调函数
*****************************************************************************/
static osal_void hmac_register_hal_dev_cb(hal_to_dmac_device_stru *hal_device)
{
    hal_device->hal_dev_fsm.hal_device_fsm_cb.hmac_scan_one_channel_start = hmac_scan_one_channel_start;

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 注册del bitmap后检查系统vap状态，根据btcoex状态，看是否需要切换到mimo */
    get_hal_device_m2s_mgr(hal_device)->hal_device_m2s_mgr_cb.m2s_back_to_mimo_check = hmac_m2s_mgr_back_to_mimo_check;
#endif
}

/*****************************************************************************
 功能描述  : dmac device添加hal device
*****************************************************************************/
static osal_void hmac_device_add_hal_device(hmac_device_stru *hmac_device)
{
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    hal_device->tbtt_update_beacon_func = hmac_irq_tbtt_ap_isr;

    hmac_register_hal_dev_cb(hal_device);
    hmac_device->hal_device[0] = hal_device;
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_fsm_attach(hal_device);
#endif
}

OSAL_STATIC osal_u32 hmac_device_init_ext(hmac_device_stru *hmac_device,
    const hal_to_dmac_device_stru *hal_device)
{
    const mac_data_rate_stru *data_rate = OSAL_NULL;
    osal_u32 rate_num = 0;
    osal_u32 idx;
    osal_u32 ret;
    /* 初始化DEVICE下的速率集 */
    hal_get_rate_80211g_table(hal_device, &data_rate);
    if (data_rate == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init::data_rate null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_get_rate_80211g_num(hal_device, &rate_num);

    for (idx = 0; idx < rate_num; idx++) {
        (osal_void)memcpy_s(&(hmac_device->mac_rates_11g[idx]), sizeof(mac_data_rate_stru),
            &data_rate[idx], sizeof(mac_data_rate_stru));
    }

    /* 初始化DMAC SCANNER */
    hmac_scan_init(hmac_device);

    ret = hmac_cfg_vap_init_etc(hmac_device);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init::hmac_cfg_vap_init failed[%d].}", ret);
        return OAL_FAIL;
    }

    // rssi limit 初始化, 默认关闭
    hmac_device->rssi_limit.rssi_limit_enable_flag = OSAL_FALSE;
    hmac_device->rssi_limit.rssi = WAL_HIPRIV_RSSI_DEFAULT_THRESHOLD;
    hmac_device->rssi_limit.rssi_delta = HMAC_RSSI_LIMIT_DELTA;
    hmac_device->arp_offload_switch = OSAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_STAT
    /* 吞吐统计定时器启动 */
    hmac_throughput_timer_init();
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : device的初始化函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_device_init(osal_void)
{
    osal_u32 loop;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    /* 结构体初始化 */
    hmac_device_add_hal_device(hmac_device);
    /* 使用master进行下面的初始化 */
    hal_device = hmac_device->hal_device[HAL_DEVICE_ID_MASTER];
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    osal_spin_lock_init(&hmac_device->lock);
#endif

    hmac_device->total_mpdu_num = 0;
    hmac_device->psm_mpdu_num = 0;

    for (loop = 0; loop < WLAN_WME_AC_BUTT; loop++) {
        hmac_device->aus_ac_mpdu_num[loop] = 0;
    }

    for (loop = WLAN_WME_AC_BE; loop < WLAN_WME_AC_BUTT; loop++) {
        hmac_device->aus_ac_mpdu_num[loop] = 0;
    }

    /* 初始化TXOP特性相关值 */
    hmac_device->txop_enable = OSAL_FALSE;

    /* 从eeprom或flash获得MAC地址 */
    hal_get_hw_addr(hmac_device->hw_addr);

    return hmac_device_init_ext(hmac_device, hal_device);
}

/*****************************************************************************
 函 数 名  : hmac_board_init
 功能描述  : 在hmac和dmac中调用时，分别直接对全局变量赋值
*****************************************************************************/
osal_u32 hmac_board_init(osal_void)
{
    osal_u32 ret;

    ret = hmac_device_init();
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_board_init::hmac_device_init failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_device_get_another_h2d_dev
 功能描述  : 获取另外一路hal device指针,注意这里是获取dmac device
                             所能使用的hal device, 不是chip下的
*****************************************************************************/
hal_to_dmac_device_stru *hmac_device_get_another_h2d_dev(const hmac_device_stru *hmac_device,
    const hal_to_dmac_device_stru *ori_hal_dev)
{
    osal_u8 hal_device_idx;

    for (hal_device_idx = 0; hal_device_idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; hal_device_idx++) {
        if (hmac_device->hal_device[hal_device_idx] != ori_hal_dev) {
            return hmac_device->hal_device[hal_device_idx];
        }
    }

    return OSAL_NULL;
}

/*****************************************************************************
 功能描述  : 检查fake_q,期望为空,调用dbac接口检查,不为空上报第一个帧
*****************************************************************************/
osal_u8 hmac_device_check_fake_queues_empty(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_device_is_fake_queues_empty::hmac_vap null, vap id is %d.}",
                hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (hmac_vap_fake_queue_empty_assert(hmac_vap, THIS_FILE_ID) != OSAL_TRUE) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_do_suspend_action_etc
 功能描述  : 暗屏
 输入参数  : hmac device
 返 回 值  : 无
*****************************************************************************/
osal_void  hmac_do_suspend_action_etc(hmac_device_stru *hmac_device, osal_u8 uc_in_suspend)
{
    osal_s32                ret;
    hmac_vap_stru            *cfg_mac_vap;
    mac_cfg_suspend_stru     suspend;
    osal_u32               is_wlan_poweron = OAL_FALSE;
    frw_msg                  msg_info;
    osal_bool wapi = OSAL_TRUE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_DISABLE);

    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_do_suspend_action_etc dev null.}");
        return;
    }
    if (fhook != OSAL_NULL) {
        wapi = ((hmac_wapi_disable_cb)fhook)(hmac_device->wapi);
    }
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    hmac_device->uc_in_suspend   = uc_in_suspend;
    cfg_mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->cfg_vap_id);
    if (cfg_mac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap[%d] hmac_do_suspend_action_etc:hmac_vap null", hmac_device->cfg_vap_id);
        return;
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
    is_wlan_poweron = wlan_pm_is_poweron_etc();
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LITEOS)
    is_wlan_poweron = OSAL_TRUE; /* liteos版本 平台接口暂不支持 */
#endif
    /* 开了host低功耗并且device已上电才需要将亮暗屏状态同步到device */
    if (is_wlan_poweron == OSAL_TRUE) {
        suspend.in_suspend = uc_in_suspend;
        suspend.arpoffload_switch = OAL_FALSE;
        if ((uc_in_suspend == OAL_TRUE) && wapi) {
            suspend.arpoffload_switch = OAL_TRUE;
        }
        hmac_wake_lock();
        /***************************************************************************
            抛事件到DMAC层, 同步屏幕最新状态到DMAC
        ***************************************************************************/
        msg_info.data = (osal_u8 *)&suspend;
        msg_info.data_len = OAL_SIZEOF(mac_cfg_suspend_stru);
        ret = hmac_config_suspend_action_sync(cfg_mac_vap, &msg_info);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_suspend_action::hmac_config_send_event_etc failed[%d]}", ret);
        }
        if (ret == OAL_SUCC) {
            frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_SUSPEND_CHANGE, &uc_in_suspend);
        }
        hmac_wake_unlock();
    }
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/*****************************************************************************
 函 数 名  : hmac_early_suspend
 功能描述  : 屏暗处理
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功或失败原因
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
osal_void hmac_early_suspend(struct early_suspend *early_sup)
{
    hmac_device_stru    *hmac_device;

    hmac_device = osal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action_etc(hmac_device, OAL_TRUE);
}

/*****************************************************************************
 函 数 名  : hmac_late_resume
 功能描述  : 屏亮处理
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功或失败原因
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
osal_void hmac_late_resume(struct early_suspend *early_sup)
{
    hmac_device_stru    *hmac_device;

    hmac_device = osal_container_of(early_sup, hmac_device_stru, early_suspend);
    hmac_do_suspend_action_etc(hmac_device, OAL_FALSE);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_device_init_etc
 功能描述  : 初始化hmac device级别参数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_device_init_etc(osal_void)
{
    osal_u8            dev_id;
    hmac_device_stru     *hmac_device = OAL_PTR_NULL;
    osal_u32           ul_ret;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    osal_u32           loop;
#endif

    /* 申请公共mac device结构体 */
    ul_ret = mac_res_alloc_hmac_dev_etc(&dev_id);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init_etc::mac_res_alloc_dmac_dev_etc failed[%d].}", ul_ret);

        return OAL_FAIL;
    }

    /* 获取mac device结构体指针 */
    hmac_device = hmac_res_get_mac_dev_etc(dev_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init_etc::device null.}");
        mac_res_free_dev_etc(dev_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_device_init_etc(hmac_device, dev_id);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init_etc::mac_device_init_etc failed[%d]}", ul_ret);

        mac_res_free_dev_etc(dev_id);
        return ul_ret;
    }

    ul_ret = hmac_device_init_hmac_device(dev_id);
    if (ul_ret != OAL_CONTINUE) {
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    for (loop = 0; loop < MAC_MAX_SUPP_CHANNEL; loop++) {
        hmac_device->ap_channel_list[loop].num_networks = 0;
        hmac_device->ap_channel_list[loop].ch_type      = MAC_CH_TYPE_NONE;
    }
#endif

    /* 配置vap初始化 */
    ul_ret = hmac_cfg_vap_init(hmac_device);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_cfg_vap_init failed[%d].}", ul_ret);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_init(hmac_device);
#endif

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_device_init_hmac_device(osal_u8 dev_id)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    /* 申请hmac device资源 */
    if (osal_unlikely(hmac_res_alloc_mac_dev_etc(dev_id) != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init_hmac_device::hmac_res_alloc_mac_dev_etc failed.}");
        return OAL_FAIL;
    }

    /* 获取hmac device，并进行相关参数赋值 */
    hmac_device = hmac_res_get_mac_dev_etc(dev_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init_hmac_device::hmac_device[%] null!}", dev_id);
        hmac_res_free_mac_dev_etc(dev_id); /* 前方申请成功过资源，若获取不到理应删掉 */
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
    hmac_device->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20;    /* 20 */
    hmac_device->early_suspend.suspend = hmac_early_suspend;
    hmac_device->early_suspend.resume  = hmac_late_resume;
    register_early_suspend(&hmac_device->early_suspend);
#endif

    /* 扫描模块初始化 */
    hmac_scan_init_etc(hmac_device);

    osal_wait_init(&hmac_device->netif_change_event);

#ifdef _PRE_WLAN_TCP_OPT
    hmac_device->sys_tcp_tx_ack_opt_enable = DEFAULT_TX_TCP_ACK_OPT_ENABLE;
    hmac_device->sys_tcp_rx_ack_opt_enable = DEFAULT_RX_TCP_ACK_OPT_ENABLE;
#endif

    return OAL_CONTINUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
