/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_main.c
 * 生成日期   : 2012年9月18日
 * 功能描述   : HMAC模块初始化与卸载
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_main.h"
#include "oal_ext_if.h"
#include "oam_struct.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "mac_regdomain.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_fsm.h"
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_11i.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_chan_mgmt.h"
#include "hmac_dfs.h"
#include "hmac_m2u.h"
#include "hmac_sdp.h"
#include "hmac_data_acq.h"
#include "hmac_rx_filter.h"
#include "hmac_config.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_dfx.h"
#include "frw_hmac.h"
#include "soc_osal.h"
#if defined(_PRE_PRODUCT_ID_HOST)
#include "hmac_cali_mgmt.h"
#endif
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
#ifndef WIFI_LITE_EXTREME
#include "oal_kernel_file.h"
#endif
#endif
#if defined(_PRE_PRODUCT_ID_HOST)
#include "hcc_host_if.h"
#endif
#include "oal_netbuf_ext.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_resource.h"
#include "hmac_mgmt_classifier.h"
#include "plat_pm_wlan.h"
#include "hmac_wow.h"
#include "hmac_dhcp_offload.h"
#include "hmac_arp_offload.h"
#include "hmac_rekey_offload.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#include "diag_log_common.h"
#include "common_dft.h"
#include "hmac_config.h"
#include "alg_main.h"
#include "hmac_auto_adjust_freq.h"
#include "wlan_msg.h"
#include "hmac_rx_data_event.h"

#include "hmac_mgmt_classifier.h"
#include "hmac_mgmt_sta.h"
#include "hmac_beacon.h"
#include "hmac_chan_mgmt.h"

#include "hmac_feature_interface.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ba.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
#include "alg_schedule_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "hmac_single_proxysta.h"
#include "hmac_sample_daq.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hmac_sample_daq_phy.h"
#include "diag_sample_data.h"
#endif
#include "hmac_psm_ap.h"
#include "hmac_psm_sta.h"
#include "hmac_alg_notify.h"
#include "hmac_alg_config.h"
#include "frw_util_notifier.h"
#include "hmac_sr_sta.h"
#include "hmac_hook.h"
#include "msg_blockack_rom.h"
#include "hmac_csa_ap.h"
#include "hmac_csa_sta.h"
#include "hmac_apf.h"
#include "hmac_slp.h"
#include "hmac_latency_stat.h"
#include "hmac_frag.h"
#include "hmac_bsrp_nfrp.h"
#include "hmac_mbo.h"
#include "hmac_11k.h"
#include "hmac_ccpriv.h"
#include "hmac_wps.h"
#include "hmac_csi.h"
#include "hmac_blacklist.h"
#include "hmac_wapi.h"
#include "hmac_radar_sensor.h"
#include "hmac_promisc.h"
#include "hmac_roam_if.h"
#include "hmac_wmm.h"
#include "hmac_11v.h"
#include "hmac_sta_pm.h"
#include "hmac_uapsd_sta.h"
#include "hmac_ant_sel.h"
#include "hmac_psd.h"
#include "hmac_11r.h"
#include "hmac_twt.h"
#include "hmac_dnb_sta.h"
#include "hmac_uapsd.h"
#include "hmac_achba.h"
#include "hmac_mfg.h"
#include "hmac_al_tx_rx.h"
#include "hmac_tcp_opt.h"
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "hmac_obss_ap.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MAIN_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/* hmac模块板子的全局控制变量 */
mac_board_stru g_st_hmac_board_etc;

oal_wakelock_stru g_st_hmac_wakelock_etc;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
hmac_rxdata_thread_stru     g_st_rxdata_thread_etc;
#endif

OAL_STATIC osal_s32 hmac_voice_aggr_event(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
osal_u8  g_uc_print_data_wakeup_etc = OAL_FALSE;
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
osal_void hmac_wake_lock(osal_void)
{
    oal_wake_lock(&g_st_hmac_wakelock_etc);
}

osal_void hmac_wake_unlock(osal_void)
{
    oal_wake_unlock(&g_st_hmac_wakelock_etc);
}

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
osal_void hmac_print_data_wakeup_en_etc(oal_bool_enum_uint8 en)
{
    g_uc_print_data_wakeup_etc = en;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_event_fsm_action_subtable_register_new
 功能描述  : hmac模块事件执行子表的注册函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC
*****************************************************************************/
OAL_STATIC osal_void hmac_event_fsm_action_subtable_register_new(osal_void)
{
    frw_msg_hook_register(WLAN_MSG_D2H_C_ALG_INFO_SYN, hmac_syn_info_event);
    frw_msg_hook_register(WLAN_MSG_D2H_C_VOICE_AGGR, hmac_voice_aggr_event);

    frw_msg_hook_register(WLAN_MSG_D2H_C_DEL_BA, hmac_del_ba_event);
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    frw_msg_hook_register(WLAN_MSG_D2H_ALG_SCHEDULE_STAT_INFO, alg_schedule_user_info_sync);
#endif
#ifdef _PRE_WLAN_FEATURE_DBAC
    frw_msg_hook_register(WLAN_MSG_D2H_C_DBAC, hmac_dbac_status_notify_etc);
#endif
    frw_msg_hook_register(WLAN_MSG_H2H_RX, hmac_rx_process_data_msg_process);
}

static osal_void hmac_netbuf_d2h_process_register(osal_void)
{
#ifdef _PRE_WLAN_FEATURE_FTM
    frw_netbuf_hook_register(FRW_NETBUF_D2H_TX_FTM_ACK, hmac_process_ftm_ack_complete);
#endif

    /* device -> dmac ko */
    frw_netbuf_hook_register(FRW_NETBUF_D2H_DMAC_DATA_FRAME, hmac_rx_data_event_adapt);
    frw_netbuf_hook_register(FRW_NETBUF_D2H_DMAC_MGNT_FRAME, hmac_rx_mgmt_event_adapt);
}

static osal_void hmac_netbuf_w2h_process_register(osal_void)
{
#ifndef _PRE_BSLE_GATEWAY
    frw_netbuf_hook_register(FRW_NETBUF_W2H_DATA_FRAME, hmac_bridge_vap_xmit_etc);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_event_fsm_register_etc
 功能描述  : hmac模块事件及其处理函数的注册函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC
*****************************************************************************/
osal_u32  hmac_event_fsm_register_etc(osal_void)
{
    hmac_netbuf_w2h_process_register();
    hmac_netbuf_d2h_process_register();

    hmac_event_fsm_action_subtable_register_new();
    return OAL_SUCC;
}

static osal_void hmac_event_fsm_action_subtable_register(osal_void)
{
    static const msg_hook_array_stru msg_hook_array[] = {
#ifdef _PRE_WLAN_FEATURE_DFS
        {WLAN_MSG_D2H_RADAR_DETECTED, hmac_dfs_radar_detect_event},
#endif
        {WLAN_MSG_H2H_C_DISASOC, hmac_mgmt_send_disasoc_event},
        {WLAN_MSG_H2H_C_MISC_DISASOC, hmac_handle_disasoc_misc_msg},
#ifdef _PRE_WLAN_FEATURE_P2P
        {WLAN_MSG_D2H_C_P2P_CT_WINDOW_INFO, hmac_p2p_opps_info_event},
#endif
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        {WLAN_MSG_D2H_WUR_SP_START, hmac_wur_sp_start_event},
        {WLAN_MSG_D2H_WUR_SP_END, hmac_wur_sp_end_event},
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
        /* 注册DMAC模块TX_COMP事件子表 */
        {WLAN_MSG_D2H_AL_TX, hmac_al_tx_complete_event_handler},
#endif
#ifdef _PRE_WLAN_FEATURE_OM
        /* 11ax rom feature debug */
        {WLAN_MSG_D2H_CRX_RPT_HE_ROM, hmac_omi_rx_he_rom_event},
#endif
        /* 注册DMAC模块MISC事件字表 */
        {WLAN_MSG_D2H_CH_STATICS_COMP, hmac_scan_channel_statistics_complete},
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        {WLAN_MSG_D2H_BCN_NO_FRM, hmac_bcn_no_frm_event_hander},
#endif
        /* 注册DMAC模块TBTT事件字表 */
        {WLAN_MSG_D2H_TBTT, hmac_tbtt_event_handler},
        /* 注册DMAC模块CRX/DRX事件子表 */
        {WLAN_MSG_H2H_CRX_RX, hmac_rx_cut_process_mgmt_ctrl_event},

#ifdef _PRE_WLAN_ONLINE_DPD
        {WLAN_MSG_D2H_DPD_TO_HMAC, hmac_dpd_to_hmac},
#endif
        {WLAN_MSG_D2H_CALI_TO_HMAC, hmac_cali_to_hmac},
    };

    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
}

/*****************************************************************************
 功能描述  : dmac模块事件及其处理函数的注册函数
*****************************************************************************/
osal_void hmac_event_fsm_register(osal_void)
{
    /* 注册所有事件的执行函数子表 */
    hmac_event_fsm_action_subtable_register();
}

/*****************************************************************************
 函 数 名  : hmac_param_check_etc
 功能描述  : Check the struct and var used in hmac
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC means ok, others failed!
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_param_check_etc(osal_void)
{
    /* netbuf's cb size! */
    osal_u32 netbuf_cb_size = (osal_u32)OAL_NETBUF_CB_SIZE();
    if (netbuf_cb_size < (osal_u32)OAL_SIZEOF(mac_tx_ctl_stru)) {
        oam_info_log2(0, OAM_SF_ANY, "hmac_param_check_etc::mac_tx_ctl_stru size[%u] large then netbuf cb max size[%u]",
            netbuf_cb_size, (osal_u32)OAL_SIZEOF(mac_tx_ctl_stru));
        return OAL_EFAIL;
    }

    if (netbuf_cb_size < (osal_u32)OAL_SIZEOF(mac_rx_ctl_stru)) {
        oam_info_log2(0, OAM_SF_ANY, "hmac_param_check_etc::mac_rx_ctl_stru size[%u] large then netbuf cb max size[%u]",
            netbuf_cb_size, (osal_u32)OAL_SIZEOF(mac_rx_ctl_stru));
        return OAL_EFAIL;
    }
    return OAL_SUCC;
}

#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
/* debug sysfs */
OAL_STATIC oal_kobject *g_conn_syfs_hmac_object = OAL_PTR_NULL;
OAL_STATIC osal_s32 hmac_print_vap_stat(osal_void *data, char *buf, osal_s32 buf_len)
{
    osal_s32 ret = 0;
    osal_u8               vap_id;
    oal_net_device_stru    *net_device = OAL_PTR_NULL;
    hmac_vap_stru          *hmac_vap = OAL_PTR_NULL;
    for (vap_id = 0; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        osal_s32 i;
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
        if (hmac_vap == OAL_PTR_NULL) {
            continue;
        }
        ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "vap %2u info:\n", vap_id);
        ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret,
                                     "vap_state %2u,protocol:%2u,user nums:%2u,init:%u\n",
                                     hmac_vap->vap_state,
                                     hmac_vap->protocol,
                                     hmac_vap->user_nums,
                                     hmac_vap->init_flag);
        net_device = hmac_vap->net_device;

        if (net_device == OAL_PTR_NULL) {
            ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
            continue;
        }

        ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "net name:%s\n", netdev_name(net_device));
        ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret,
                                     "tx [%u]queues info,state [bit0:DRV_OFF],[bit1:STACK_OFF],[bit2:FROZEN]\n",
                                     net_device->num_tx_queues);
        for (i = 0; i < net_device->num_tx_queues; i++) {
            struct netdev_queue *txq = netdev_get_tx_queue(net_device, i);
            if (txq != OAL_PTR_NULL && txq->state) {
                ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret,
                                             "net queue[%2d]'s state:0x%lx\n", i, txq->state);
            }
        }
        ret +=  (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
    }
    return ret;
}

OAL_STATIC ssize_t hmac_get_vap_stat(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (dev == OAL_PTR_NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (buf == OAL_PTR_NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == OAL_PTR_NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += hmac_print_vap_stat(NULL, buf, PAGE_SIZE - ret);
    return ret;
}
OAL_STATIC DEVICE_ATTR(vap_info, S_IRUGO, hmac_get_vap_stat, NULL);

OAL_STATIC ssize_t  hmac_get_adapt_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (attr == OAL_PTR_NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (buf == OAL_PTR_NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == OAL_PTR_NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    return ret;
}

osal_s32 hmac_wakelock_info_print_etc(char *buf, osal_s32 buf_len)
{
    osal_s32 ret = 0;

#ifdef CONFIG_PRINTK
    if (g_st_hmac_wakelock_etc.locked_addr) {
        ret += (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "wakelocked\n");
    }
#endif

    ret += (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret,
                                "hold %lu locks\n", g_st_hmac_wakelock_etc.lock_count);

    return ret;
}

OAL_STATIC ssize_t  hmac_get_wakelock_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (dev == OAL_PTR_NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (buf == OAL_PTR_NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == OAL_PTR_NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += hmac_wakelock_info_print_etc(buf, PAGE_SIZE - ret);

    return ret;
}

OAL_STATIC DEVICE_ATTR(adapt_info, S_IRUGO, hmac_get_adapt_info, NULL);
OAL_STATIC DEVICE_ATTR(wakelock, S_IRUGO, hmac_get_wakelock_info, NULL);

OAL_STATIC ssize_t hmac_show_roam_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    osal_s32               ret = 0;
    osal_u8               vap_id;
    osal_u8               roming_now  = 0;
    hmac_vap_stru          *hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = OSAL_NULL;
    osal_void *fhook_wapi = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_UPDATE_ROAM);

    if (buf == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{buf is NULL.}");
        return ret;
    }

    if ((dev == OAL_PTR_NULL) || (attr == OAL_PTR_NULL)) {
        ret += (osal_s32)snprintf_s(buf, PAGE_SIZE, PAGE_SIZE, "roam_status=0\n");
        return ret;
    }

    for (vap_id = 0; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
        if ((hmac_vap == OAL_PTR_NULL) || (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) ||
            (hmac_vap->vap_state == MAC_VAP_STATE_BUTT)) {
            continue;
        }

        if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
            roming_now = 1;
            break;
        }

        if (fhook_wapi != OSAL_NULL) {
            ret = ((hmac_wapi_update_roam_cb)fhook_wapi)(hmac_vap, &roming_now);
            if (ret == OAL_PTR_NULL) {
                continue;
            } else if (ret == OAL_FAIL) {
                break;
            }
        }

        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_BA_CHECK);
        if ((hmac_user != OAL_PTR_NULL) && (fhook != OSAL_NULL) &&
            ((hmac_btcoex_check_by_ba_size_etc_cb)fhook)(hmac_user) == OAL_TRUE) {
            roming_now = 1;
        }
    }
    /* 先出一个版本强制关闭arp探测，测试下效果 */
    ret += (osal_s32)snprintf_s(buf, PAGE_SIZE, PAGE_SIZE, "roam_status=%1u\n", roming_now);
    return ret;
}

OAL_STATIC DEVICE_ATTR(roam_status, S_IRUGO, hmac_show_roam_status, NULL);

OAL_STATIC ssize_t hmac_set_rxthread_enable(struct device *dev, struct device_attribute *attr, const char *buf,
    size_t count)
{
    osal_u32 val;

    if (buf == OAL_PTR_NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == OAL_PTR_NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == OAL_PTR_NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%u", &val) != 1)) {
        wifi_printf("set value one char!\n");
        return -OAL_EINVAL;
    }

    g_st_rxdata_thread_etc.rxthread_enable = (oal_bool_enum_uint8)val;

    return count;
}
OAL_STATIC ssize_t hmac_get_rxthread_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == OAL_PTR_NULL) {
        wifi_printf("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == OAL_PTR_NULL) {
        wifi_printf("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == OAL_PTR_NULL) {
        wifi_printf("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret += (osal_s32)snprintf_s(buf, PAGE_SIZE, PAGE_SIZE,
        "rxthread_enable=%u\nrxthread_queue_len=%u\nrxthread_pkt_loss=%u\n",
        g_st_rxdata_thread_etc.rxthread_enable, oal_netbuf_list_len(&g_st_rxdata_thread_etc.rxdata_netbuf_head),
        g_st_rxdata_thread_etc.pkt_loss_cnt);

    return ret;
}
OAL_STATIC DEVICE_ATTR(rxdata_info, S_IRUGO | S_IWUSR, hmac_get_rxthread_info, hmac_set_rxthread_enable);

OAL_STATIC struct attribute *hmac_sysfs_entries[] = {
    &dev_attr_vap_info.attr,
    &dev_attr_adapt_info.attr,
    &dev_attr_wakelock.attr,
    &dev_attr_roam_status.attr,
    &dev_attr_rxdata_info.attr,
    NULL
};

OAL_STATIC struct attribute_group hmac_attribute_group = {
    .name = "vap",
    .attrs = hmac_sysfs_entries,
};

OAL_STATIC osal_s32 hmac_sysfs_entry_init(osal_void)
{
    osal_s32       ret;
    oal_kobject     *root_object = OAL_PTR_NULL;
    root_object = oal_get_sysfs_root_object_etc();
    if (root_object == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::get sysfs root object failed!}");
        return -OAL_EFAIL;
    }

    g_conn_syfs_hmac_object = kobject_create_and_add("hmac", root_object);
    if (g_conn_syfs_hmac_object == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::create hmac object failed!}");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(g_conn_syfs_hmac_object, &hmac_attribute_group);
    if (ret) {
        kobject_put(g_conn_syfs_hmac_object);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sysfs_entry_init::sysfs create group failed!}");
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_sysfs_entry_exit(osal_void)
{
    if (g_conn_syfs_hmac_object) {
        sysfs_remove_group(g_conn_syfs_hmac_object, &hmac_attribute_group);
        kobject_put(g_conn_syfs_hmac_object);
    }
    return OAL_SUCC;
}
#endif
#endif

OSAL_STATIC osal_void hmac_register_pm_callback_etc(osal_void)
{
    struct wifi_srv_callback_handler *wifi_srv_handler;

    wifi_srv_handler = wlan_pm_get_wifi_srv_handler_etc();
    if (wifi_srv_handler != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
        wifi_srv_handler->p_data_wkup_print_en_func = hmac_print_data_wakeup_en_etc;
#endif
    } else {
        wifi_printf("hmac_register_pm_callback_etc:wlan_pm_get_wifi_srv_handler_etc is null\n");
    }
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_bool_enum_uint8 hmac_get_rxthread_enable_etc(osal_void)
{
    return g_st_rxdata_thread_etc.rxthread_enable;
}

osal_void hmac_rxdata_sched_etc(osal_void)
{
    up(&g_st_rxdata_thread_etc.rxdata_sema);
    return;
}
osal_void hmac_rxdata_netbuf_enqueue_etc(oal_netbuf_stru  *pst_netbuf)
{
    /* 1000表示rxdata_netbuf_head最大长度 */
    if (oal_netbuf_list_len(&g_st_rxdata_thread_etc.rxdata_netbuf_head) > 1000) {
        oal_netbuf_free(pst_netbuf);
        g_st_rxdata_thread_etc.pkt_loss_cnt++;
        return;
    }

    osal_spin_lock(&g_st_rxdata_thread_etc.lock);
    oal_netbuf_list_tail_nolock(&g_st_rxdata_thread_etc.rxdata_netbuf_head, pst_netbuf);
    osal_spin_unlock(&g_st_rxdata_thread_etc.lock);
}

OAL_STATIC osal_s32 hmac_rxdata_thread(osal_void *p_data)
{
    oal_netbuf_stru    *pst_netbuf = OAL_PTR_NULL;

    osal_kthread_set_priority(g_st_rxdata_thread_etc.rxdata_thread, 97); /* 97表示调度优先级 */
    osal_kthread_set_affinity(g_st_rxdata_thread_etc.rxdata_thread, OSAL_CPU_1);
    allow_signal(SIGTERM);
    while (OAL_LIKELY(!down_interruptible(&g_st_rxdata_thread_etc.rxdata_sema))) {
        if (osal_kthread_should_stop()) {
            break;
        }

        osal_spin_lock(&g_st_rxdata_thread_etc.lock);
        pst_netbuf = oal_netbuf_delist_nolock(&g_st_rxdata_thread_etc.rxdata_netbuf_head);
        osal_spin_unlock(&g_st_rxdata_thread_etc.lock);
        if (pst_netbuf != OAL_PTR_NULL) {
            oal_netif_rx(pst_netbuf);
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_uapi_thread_init(osal_void)
{
    osal_wait_init(&g_st_rxdata_thread_etc.rxdata_wq);
    oal_netbuf_list_head_init(&g_st_rxdata_thread_etc.rxdata_netbuf_head);
    osal_spin_lock_init(&g_st_rxdata_thread_etc.lock);
    g_st_rxdata_thread_etc.rxthread_enable = OAL_TRUE;
    g_st_rxdata_thread_etc.pkt_loss_cnt    = 0;

    sema_init(&g_st_rxdata_thread_etc.rxdata_sema, 0);
    g_st_rxdata_thread_etc.rxdata_thread = osal_kthread_create(hmac_rxdata_thread, OSAL_NULL, "wifi_rxdata", 0x800);

#ifndef WIN32
    if (OAL_PTR_NULL == g_st_rxdata_thread_etc.rxdata_thread) {
        wifi_printf("wifi_rxdata thread create failed!\n");
        return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_uapi_thread_exit(osal_void)
{
    osal_adapt_wait_destroy(&g_st_rxdata_thread_etc.rxdata_wq);
    if (g_st_rxdata_thread_etc.rxdata_thread != OAL_PTR_NULL) {
#ifdef CONFIG_NO_SUPPORT_OSAL
        send_sig(SIGTERM, g_st_rxdata_thread_etc.rxdata_thread, 1);
#else
        send_sig(SIGTERM, (struct task_struct *)((g_st_rxdata_thread_etc.rxdata_thread)->task), 1);
#endif
        osal_kthread_destroy(g_st_rxdata_thread_etc.rxdata_thread, OSAL_TRUE);
        g_st_rxdata_thread_etc.rxdata_thread = OAL_PTR_NULL;
    }
    osal_spin_lock_destroy(&g_st_rxdata_thread_etc.lock);
}
#endif

DEFINE_GET_BUILD_VERSION_FUNC(hmac);

static osal_s32 hmac_event_init(frw_init_enum_uint16  init_state)
{
    osal_u32 ul_return;

    /* 如果初始化状态处于配置VAP成功前的状态，表明此次为HMAC第一次初始化，即重加载或启动初始化 */
    if (init_state < FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC) {
        /* 调用状态机初始化接口 */
        hmac_fsm_init_etc();

        /* 事件注册 */
        hmac_event_fsm_register_etc();

        ul_return = hmac_device_init_etc();
        if (osal_unlikely(ul_return != OAL_SUCC)) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_chip_init::hmac_device_init_etc failed[%d].}", ul_return);
            return OAL_FAIL;
        }

        frw_set_init_state_etc(FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC);

        /* 启动成功后，输出打印 */
    } else {
        /* 如果初始化状态为配置VAP成功后的状态，则表明本次为配置触发的初始化，
           需要遍历所有业务VAP，并检查其状态；如未初始化，则需要初始化其相关内容
           如支持特性接口挂接 */
    }
    return OAL_SUCC;
}

static osal_void hmac_misc_init(osal_void)
{
#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
    osal_s32 ul_return;
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_DAQ
    hmac_data_acq_init();
    /* wifi数采上报HSO：初始化 */
    diag_sample_data_register(ZDIAG_WLAN_SAMPLE_CB, (diag_sample_func_cb)hmac_sample_wlan_data_enable);
    diag_sample_data_register(ZDIAG_SOC_SAMPLE_CB, (diag_sample_func_cb)hmac_sample_soc_data_enable);
    diag_sample_data_register(ZDIAG_WLAN_PHY_SAMPLE_CB, (diag_sample_func_cb)hmac_sample_wlan_phy_data_enable);
#endif

#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
    ul_return = hmac_sysfs_entry_init();
    if (osal_unlikely(ul_return != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_main_init_etc::hmac_sysfs_entry_init failed[%d].}", ul_return);
    }
#endif
#endif

    hmac_register_pm_callback_etc();
}

osal_s32 hmac_main_init_later(osal_void)
{
    osal_s32 ret = OAL_SUCC;
    osal_u32 ret_board;
    ret = hal_hmac_main_init();
    if (ret != OAL_SUCC) {
#ifdef _PRE_WLAN_FEATURE_WS63 // 回退冲突
        hal_main_exit();
#else
        hal_hmac_main_exit();
#endif
        wifi_printf("hal_hmac_main_init: ret %d\r\n", ret);
        return ret;
    }

    ret_board = hmac_board_init();
    if (ret_board != OAL_SUCC) {
        wifi_printf("hmac_board_init fail: %d\r\n", ret_board);
        return (osal_s32)ret_board;
    }

#ifndef _PRE_BSLE_GATEWAY
    alg_hmac_main_init();
#endif

    return ret;
}

#ifdef CONTROLLER_CUSTOMIZATION
osal_u32 hmac_alg_config_init(osal_void);
osal_u32 hmac_sta_sr_init(osal_void);
osal_u32 hmac_single_proxysta_init(osal_void);
osal_u32 hmac_csa_sta_init(osal_void);
osal_u32 hmac_csa_ap_init(osal_void);
osal_u32 hmac_sdp_msg_init(osal_void);
osal_u32 hmac_m2u_init(osal_void);
osal_u32 hmac_apf_init(osal_void);
osal_u32 hmac_slp_init(osal_void);
osal_u32 hmac_radar_sensor_init(osal_void);
osal_u32 hmac_latency_stat_init(osal_void);
osal_u32 hmac_frag_init(osal_void);
osal_u32 hmac_auto_adjust_freq_init(osal_void);
osal_u32 hmac_wapi_init(osal_void);
osal_u32 hmac_bsrp_nfrp_init(osal_void);
osal_u32 hmac_btcoex_init(osal_void);
osal_u32 hmac_mbo_sta_init(osal_void);
osal_u32 hmac_11k_init(osal_void);
osal_u32 hmac_tx_amsdu_init(osal_void);
osal_u32 hmac_wow_init(osal_void);
osal_u32 hmac_dhcp_offload_init(osal_void);
osal_u32 hmac_arp_offload_init(osal_void);
osal_u32 hmac_rekey_offload_init(osal_void);
osal_u32 hmac_csi_init(osal_void);
osal_u32 hmac_blacklist_register_init(osal_void);
osal_u32 hmac_dfx_init(osal_void);
osal_u32 hmac_promis_init(osal_void);
osal_u32 hmac_wps_init(osal_void);
osal_u32 hmac_roam_sta_init(osal_void);
osal_u32 hmac_11v_init(osal_void);
osal_u32 hmac_sta_pm_init(osal_void);
osal_u32 hmac_uapsd_sta_init(osal_void);
osal_u32 hmac_psd_init(osal_void);
osal_u32 hmac_ant_sel_init(osal_void);
osal_u32 hmac_11r_init(osal_void);
osal_u32 hmac_sta_dnb_init(osal_void);
osal_u32 hmac_twt_sta_init(osal_void);
osal_u32 hmac_uapsd_ap_init(osal_void);
osal_u32 hmac_achba_init(osal_void);
osal_u32 hmac_mfg_init(osal_void);
osal_u32 hmac_sample_daq_init(osal_void);
osal_u32 hmac_tcp_opt_init(osal_void);
osal_u32 hmac_sta_obss_init(osal_void);
osal_u32 hmac_ap_obss_init(osal_void);
osal_void hmac_ccpriv_deinit(osal_void);
osal_void hmac_wmm_deinit(osal_void);
osal_void hmac_alg_config_deinit(osal_void);
osal_void hmac_sta_sr_deinit(osal_void);
osal_void hmac_single_proxysta_deinit(osal_void);
osal_void hmac_csa_sta_deinit(osal_void);
osal_void hmac_csa_ap_deinit(osal_void);
osal_void hmac_sdp_msg_deinit(osal_void);
osal_void hmac_m2u_deinit(osal_void);
osal_void hmac_apf_deinit(osal_void);
osal_void hmac_slp_deinit(osal_void);
osal_void hmac_radar_sensor_deinit(osal_void);
osal_void hmac_latency_stat_deinit(osal_void);
osal_void hmac_frag_deinit(osal_void);
osal_void hmac_auto_adjust_freq_deinit(osal_void);
osal_void hmac_wapi_deinit(osal_void);
osal_void hmac_bsrp_nfrp_deinit(osal_void);
osal_void hmac_btcoex_deinit(osal_void);
osal_void hmac_mbo_sta_deinit(osal_void);
osal_void hmac_11k_deinit(osal_void);
osal_void hmac_tx_amsdu_deinit(osal_void);
osal_void hmac_wow_deinit(osal_void);
osal_void hmac_dhcp_offload_deinit(osal_void);
osal_void hmac_arp_offload_deinit(osal_void);
osal_void hmac_rekey_offload_deinit(osal_void);
osal_void hmac_csi_deinit(osal_void);
osal_void hmac_blacklist_register_deinit(osal_void);
osal_void hmac_dfx_deinit(osal_void);
osal_void hmac_promis_deinit(osal_void);
osal_void hmac_wps_deinit(osal_void);
osal_void hmac_roam_sta_deinit(osal_void);
osal_void hmac_11v_deinit(osal_void);
osal_void hmac_sta_pm_deinit(osal_void);
osal_void hmac_psd_deinit(osal_void);
osal_void hmac_uapsd_sta_deinit(osal_void);
osal_void hmac_ant_sel_deinit(osal_void);
osal_void hmac_11r_deinit(osal_void);
osal_void hmac_sta_dnb_deinit(osal_void);
osal_void hmac_twt_sta_deinit(osal_void);
osal_void hmac_uapsd_ap_deinit(osal_void);
osal_void hmac_achba_deinit(osal_void);
osal_void hmac_mfg_deinit(osal_void);
osal_void hmac_sample_daq_deinit(osal_void);
osal_void hmac_tcp_opt_deinit(osal_void);
osal_void hmac_sta_obss_deinit(osal_void);
osal_void hmac_ap_obss_deinit(osal_void);
#endif

  /* 后续需要各个模块自己注册初始化函数 */
OSAL_STATIC osal_void frw_wifi_module_exit(osal_void)
{
    osal_u32 i = 0;

#ifdef CONTROLLER_CUSTOMIZATION
    frw_wifi_module_func_exit module_exit_func[] = {
        hmac_ccpriv_deinit,        hmac_wmm_deinit,             hmac_alg_config_deinit,
        hmac_sta_sr_deinit,        hmac_single_proxysta_deinit, hmac_csa_sta_deinit,
        hmac_csa_ap_deinit,        hmac_sdp_msg_deinit,         hmac_m2u_deinit,
        hmac_apf_deinit,           hmac_slp_deinit,             hmac_11v_deinit,
        hmac_latency_stat_deinit,  hmac_11r_deinit,             hmac_auto_adjust_freq_deinit,
        hmac_bsrp_nfrp_deinit,     hmac_frag_deinit,
        hmac_mbo_sta_deinit,       hmac_11k_deinit,             hmac_tx_amsdu_deinit,
        hmac_wow_deinit,           hmac_dhcp_offload_deinit,    hmac_arp_offload_deinit,
        hmac_rekey_offload_deinit, hmac_csi_deinit,             hmac_blacklist_register_deinit,
        hmac_dfx_deinit,           hmac_promis_deinit,          hmac_wps_deinit,
        hmac_roam_sta_deinit,      hmac_sta_pm_deinit,
        hmac_psd_deinit,           hmac_uapsd_sta_deinit,       hmac_ant_sel_deinit,
        hmac_sta_dnb_deinit,       hmac_twt_sta_deinit,
        hmac_uapsd_ap_deinit,      hmac_mfg_deinit,
        hmac_sample_daq_deinit,    hmac_tcp_opt_deinit,         hmac_sta_obss_deinit,
        hmac_btcoex_deinit,
    };
#else
    frw_wifi_module_func_exit module_exit_func[] = {
        hmac_ccpriv_deinit,                hmac_wmm_deinit,                     hmac_alg_config_deinit_weakref,
        hmac_sta_sr_deinit_weakref,        hmac_single_proxysta_deinit_weakref, hmac_csa_sta_deinit_weakref,
        hmac_csa_ap_deinit_weakref,        hmac_sdp_deinit_weakref,             hmac_m2u_deinit_weakref,
        hmac_apf_deinit_weakref,           hmac_slp_deinit_weakref,             hmac_radar_sensor_deinit_weakref,
        hmac_latency_stat_deinit_weakref,  hmac_frag_deinit_weakref,            hmac_auto_freq_deinit_weakref,
        hmac_wapi_deinit_weakref,          hmac_bsrp_nfrp_deinit_weakref,       hmac_btcoex_deinit_weakref,
        hmac_mbo_sta_deinit_weakref,       hmac_11k_deinit_weakref,             hmac_tx_amsdu_deinit_weakref,
        hmac_wow_deinit_weakref,           hmac_dhcp_offload_deinit_weakref,    hmac_arp_offload_deinit_weakref,
        hmac_rekey_offload_deinit_weakref, hmac_csi_deinit_weakref,             hmac_blacklist_deinit_weakref,
        hmac_dfx_deinit_weakref,           hmac_promis_deinit_weakref,          hmac_wps_deinit_weakref,
        hmac_roam_sta_deinit_weakref,      hmac_11v_deinit_weakref,             hmac_sta_pm_deinit_weakref,
        hmac_psd_deinit_weakref,           hmac_uapsd_sta_deinit_weakref,       hmac_ant_sel_deinit_weakref,
        hmac_11r_deinit_weakref,           hmac_sta_dnb_deinit_weakref,         hmac_twt_sta_deinit_weakref,
        hmac_uapsd_ap_deinit_weakref,      hmac_achba_deinit_weakref,           hmac_mfg_deinit_weakref,
        hmac_sample_daq_deinit_weakref,    hmac_tcp_opt_deinit_weakref,         hmac_sta_obss_deinit_weakref,
        hmac_ap_obss_deinit_weakref
    };
#endif

    for (i = 0; i < sizeof(module_exit_func) / sizeof(module_exit_func[0]); i++) {
        if (module_exit_func[i] == OSAL_NULL) {
            continue;
        }

        module_exit_func[i]();
    }
}

  /* 后续需要各个模块自己注册初始化函数 */
OSAL_STATIC osal_void frw_wifi_module_init(osal_void)
{
    osal_u32 i = 0;

#ifdef CONTROLLER_CUSTOMIZATION
    frw_wifi_module_func_init module_init_func[] = {
        hmac_ccpriv_init,        hmac_wmm_init,             hmac_alg_config_init,
        hmac_sta_sr_init,        hmac_single_proxysta_init, hmac_csa_sta_init,
        hmac_csa_ap_init,        hmac_sdp_msg_init,         hmac_m2u_init,
        hmac_apf_init,           hmac_slp_init,             hmac_11v_init,
        hmac_latency_stat_init,  hmac_11r_init,             hmac_auto_adjust_freq_init,
        hmac_bsrp_nfrp_init,     hmac_frag_init,
        hmac_mbo_sta_init,       hmac_11k_init,             hmac_tx_amsdu_init,
        hmac_wow_init,           hmac_dhcp_offload_init,    hmac_arp_offload_init,
        hmac_rekey_offload_init, hmac_csi_init,             hmac_blacklist_register_init,
        hmac_dfx_init,           hmac_promis_init,          hmac_wps_init,
        hmac_roam_sta_init,      hmac_sta_pm_init,
        hmac_uapsd_sta_init,     hmac_psd_init,             hmac_ant_sel_init,
        hmac_sta_dnb_init,       hmac_twt_sta_init,
        hmac_uapsd_ap_init,      hmac_mfg_init,
        hmac_sample_daq_init,    hmac_tcp_opt_init,         hmac_sta_obss_init,
        hmac_btcoex_init,
    };
#else
    frw_wifi_module_func_init module_init_func[] = {
        hmac_ccpriv_init,                hmac_wmm_init,                     hmac_alg_config_init_weakref,
        hmac_sta_sr_init_weakref,        hmac_single_proxysta_init_weakref, hmac_csa_sta_init_weakref,
        hmac_csa_ap_init_weakref,        hmac_sdp_init_weakref,             hmac_m2u_init_weakref,
        hmac_apf_init_weakref,           hmac_slp_init_weakref,             hmac_radar_sensor_init_weakref,
        hmac_latency_stat_init_weakref,  hmac_frag_init_weakref,            hmac_auto_freq_init_weakref,
        hmac_wapi_init_weakref,          hmac_bsrp_nfrp_init_weakref,       hmac_btcoex_init_weakref,
        hmac_mbo_sta_init_weakref,       hmac_11k_init_weakref,             hmac_tx_amsdu_init_weakref,
        hmac_wow_init_weakref,           hmac_dhcp_offload_init_weakref,    hmac_arp_offload_init_weakref,
        hmac_rekey_offload_init_weakref, hmac_csi_init_weakref,             hmac_blacklist_init_weakref,
        hmac_dfx_init_weakref,           hmac_promis_init_weakref,          hmac_wps_init_weakref,
        hmac_roam_sta_init_weakref,      hmac_11v_init_weakref,             hmac_sta_pm_init_weakref,
        hmac_uapsd_sta_init_weakref,     hmac_psd_init_weakref,             hmac_ant_sel_init_weakref,
        hmac_11r_init_weakref,           hmac_sta_dnb_init_weakref,         hmac_twt_sta_init_weakref,
        hmac_uapsd_ap_init_weakref,      hmac_achba_init_weakref,           hmac_mfg_init_weakref,
        hmac_sample_daq_init_weakref,    hmac_tcp_opt_init_weakref,         hmac_sta_obss_init_weakref,
        hmac_ap_obss_init_weakref
    };
#endif

    for (i = 0; i < sizeof(module_init_func) / sizeof(module_init_func[0]); i++) {
        if (module_init_func[i] == OSAL_NULL) {
            continue;
        }

        if (module_init_func[i]() != OSAL_SUCCESS) {
            wifi_printf("module_init_failed[%p]!\r\n\r\n", module_init_func[i]);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_main_init_etc
 功能描述  : HMAC模块初始化总入口，包含HMAC模块内部所有特性的初始化。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 初始化返回值，成功或失败原因
*****************************************************************************/
osal_s32 hmac_main_init_etc(osal_void)
{
    osal_s32            ul_return;
    frw_init_enum_uint16 init_state = frw_get_init_state_etc();
    oal_wake_lock_init(&g_st_hmac_wakelock_etc, "wlan_hmac_wakelock");

    /* 为了解各模块的启动时间，增加时间戳打印 */
    if (hmac_param_check_etc() != OAL_SUCC) {
        wifi_printf("hmac_main_init_etc:hmac_param_check_etc failed!\n");
        return -OAL_EFAIL;
    }

    if (osal_unlikely((init_state == FRW_INIT_STATE_BUTT) || (init_state < FRW_INIT_STATE_FRW_SUCC))) {
        wifi_printf("hmac_main_init_etc:init_state is error %d\n", init_state);
        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ul_return = hmac_uapi_thread_init();
    if (ul_return != OAL_SUCC) {
        frw_timer_delete_all_timer();
        wifi_printf("hmac_main_init_etc: hmac_uapi_thread_init failed\n");

        return -OAL_EFAIL;
    }
#endif

    ul_return = (osal_s32)mac_res_init_etc();
    if (ul_return != OAL_SUCC) {
        wifi_printf("hmac_main_init_etc: mac_res_init_etc return err code %d\n", ul_return);
        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* hmac资源初始化 */
    hmac_res_init_etc();

    /* 如果初始化状态处于配置VAP成功前的状态，表明此次为HMAC第一次初始化，即重加载或启动初始化 */
    ul_return = hmac_event_init(init_state);
    if (ul_return != OAL_SUCC) {
        frw_timer_delete_all_timer();
        hmac_res_exit();
        return (osal_s32)ul_return;
    }

    hmac_hook_init();
    frw_util_notifier_init();
    frw_wifi_module_init();

    hmac_misc_init();
    hmac_cfg_init();
    hmac_cfg_ccpriv_init();
    h2d_cfg_init();
    hmac_event_fsm_register();

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_main_exit_etc
 功能描述  : HMAC模块卸载
*****************************************************************************/
osal_void hmac_main_exit_etc(osal_void)
{
    osal_u32 ul_return;
    hmac_device_stru *hmac_device = OSAL_NULL;

#if defined(_PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT)
#ifndef WIFI_LITE_EXTREME
    ul_return = hmac_sysfs_entry_exit();
    if (osal_unlikely(ul_return != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_main_exit_etc::hmac_sysfs_entry_exit failed[%d].}", ul_return);
    }
#endif
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_uapi_thread_exit();
#endif

    hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 待挪动位置 释放资源 */
    hmac_res_free_mac_dev_etc(0);

    ul_return = hmac_device_exit_etc(hmac_device);
    if (ul_return != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_device_exit_etc failed[%d].}", ul_return);
        return;
    }

    hmac_hook_exit();
    hmac_cfg_ccpriv_exit();
    frw_wifi_module_exit();
    frw_util_notifier_exit();

#ifdef _PRE_WLAN_FEATURE_DAQ
    hmac_data_acq_exit();
#endif

    hmac_res_exit(); /* 需要判空优化 */

    frw_set_init_state_etc(FRW_INIT_STATE_FRW_SUCC);

#ifndef CONFIG_NO_SUPPORT_INI
    /* 动态cali申请的内存也需要释放 */
    hmac_init_cali_data_buffer_exit();
#endif
    oal_wake_lock_exit(&g_st_hmac_wakelock_etc);

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    frw_timer_clean_timer(OAM_MODULE_ID_HMAC);
#endif
}

#ifdef _PRE_WLAN_ONLINE_DPD
/*****************************************************************************
 函 数 名  : hmac_sdt_up_reg_val_etc
 功能描述  : hmac接收dmac抛回来的寄存器的值，然后交给wal，wal上报SDT
*****************************************************************************/
osal_s32 hmac_sdt_up_dpd_data(oal_netbuf_stru *netbuf)
{
    osal_u16 payload_len;
    osal_u8 *payload = OAL_PTR_NULL;
    osal_s8 *pc_print_buff = OAL_PTR_NULL;

    if (netbuf == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sdt_up_dpd_data::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    payload  = (osal_u8 *)OAL_NETBUF_DATA(netbuf);
    payload_len = osal_strlen(payload);

    oam_error_log1(0, 0, "hmac dpd payload len %d", payload_len);

    pc_print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (OAL_PTR_NULL == pc_print_buff) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sdt_up_dpd_data::pc_print_buff null.}");
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    if (memcpy_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, payload, payload_len) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_sdt_up_dpd_data data::memcpy err.}");
    }
    pc_print_buff[payload_len] = '\0';
    dft_report_params_etc(pc_print_buff, osal_strlen(pc_print_buff), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    oal_mem_free(pc_print_buff, OAL_TRUE);
    oal_netbuf_free(netbuf);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_del_ba_event
*****************************************************************************/
osal_s32 hmac_del_ba_event(hmac_vap_stru *vap, frw_msg *msg)
{
    osal_u8       tid;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_action_mgmt_args_stru action_args;   /* 用于填写ACTION帧的参数 */
    hmac_tid_stru  *hmac_tid = OAL_PTR_NULL;
    osal_s32      ul_ret;
    mac_ctx_event_stru *del_ba_event = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = vap;

    if (osal_unlikely(msg == OAL_PTR_NULL) || osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{del_ba_event::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    del_ba_event = (mac_ctx_event_stru *)msg->data;
    hmac_user = mac_res_get_hmac_user_etc(del_ba_event->user_index);
    if (hmac_user == OAL_PTR_NULL) {
        /* dmac抛事件到hmac侧删除ba，此时host侧可能已经删除用户了，此时属于正常，直接返回即可 */
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_del_ba_event::hmac_user[%d] null.}", del_ba_event->user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap  = mac_res_get_hmac_vap(del_ba_event->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_del_ba_event::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user_set_cur_protocol_mode_etc(hmac_user, del_ba_event->cur_protocol);
    ul_ret = (osal_s32)hmac_config_user_info_syn_etc(hmac_vap, hmac_user);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
        hmac_tid  = &hmac_user->tx_tid_queue[tid];

        if (hmac_tid->ba_tx_info.ba_status == HMAC_BA_INIT) {
            oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_del_ba_event:the tx hdl is not exist", hmac_vap->vap_id);
            continue;
        }

        action_args.category = MAC_ACTION_CATEGORY_BA;
        action_args.action   = MAC_BA_ACTION_DELBA;
        action_args.arg1     = tid;       /* 该数据帧对应的TID号 */
        action_args.arg2     = MAC_ORIGINATOR_DELBA;           /* ADDBA_REQ中，buffer_size的默认大小 */
        action_args.arg3     = MAC_UNSPEC_REASON;              /* BA会话的确认策略 */
        action_args.arg5    = hmac_user->user_mac_addr;   /* ba会话对应的user */

        /* 删除BA会话 */
        ul_ret = (osal_s32)hmac_mgmt_tx_action_etc(hmac_vap,  hmac_user, &action_args);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_del_ba_event::tx_action_etc failed.}", hmac_vap->vap_id);
            continue;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_syn_info_event
 功能描述  : 处理协议模式同步事件
*****************************************************************************/
osal_s32 hmac_syn_info_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    hmac_user_stru *hmac_user = OSAL_NULL;
    dmac_to_hmac_syn_info_event_stru *syn_info_event = OSAL_NULL;
    if (osal_unlikely(msg == OSAL_NULL) || osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_syn_info_event::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    syn_info_event = (dmac_to_hmac_syn_info_event_stru *)msg->data;
    hmac_user = mac_res_get_hmac_user_etc(syn_info_event->user_index);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user_set_cur_protocol_mode_etc(hmac_user, syn_info_event->cur_protocol);
    hmac_user_set_cur_bandwidth_etc(hmac_user, syn_info_event->cur_bandwidth);
    ret = hmac_config_user_info_syn_etc(hmac_vap, hmac_user);

    return (osal_s32)ret;
}

/*****************************************************************************
 函 数 名  : hmac_voice_aggr_event
 功能描述  : 处理Voice聚合同步事件
 输入参数  : event_mem
*****************************************************************************/
OAL_STATIC osal_s32 hmac_voice_aggr_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_to_hmac_voice_aggr_event_stru *voice_aggr_event = OAL_PTR_NULL;

    if (osal_unlikely(msg->data == OAL_PTR_NULL) || osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_voice_aggr_event::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    voice_aggr_event = (dmac_to_hmac_voice_aggr_event_stru *)msg->data;
    hmac_vap->voice_aggr = voice_aggr_event->voice_aggr;
    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_module_license("GPL");
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
