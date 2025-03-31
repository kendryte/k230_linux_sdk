/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hook function for wifi promiscuous.
 */

/* 混杂模式在linux上编译不过，先屏蔽，后续特性责任人做适配 */
#include "hmac_promisc.h"
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "soc_zdiag.h"
#include "hal_mac_reg.h"
#else
#include "mac_resource_ext.h"
#endif
#include "hmac_sta_pm.h"
#include "oam_ext_if.h"
#include "oal_net.h"
#include "mac_frame.h"
#include "mac_device_ext.h"
#include "dmac_ext_if_device.h"
#include "hmac_config.h"
#include "msg_psm_rom.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_feature_interface.h"
#include "hmac_hook.h"
#include "hmac_rx_data.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_PROMISC_C

#define WLAN_PROCMIS_ZDIAG_ID       0x7580
#define WLAN_PROMISC_MODE_NUM       4

static wlan_promisc_mode_stru g_promisc_st = {
    .promis_count = {{{0}}}
};

static osal_u8 g_custom_en = 0;
static osal_u8 g_promisc_switch = 0;
static osal_u8 g_promisc_mode = 0;

/* 函数功能: 清零混杂模式帧计数 */
OAL_STATIC osal_void hmac_promisc_clear_counts(void)
{
    memset_s(g_promisc_st.promis_count, sizeof(g_promisc_st.promis_count), 0, sizeof(g_promisc_st.promis_count));
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_void hmac_promis_send_custom_frame(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    const dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    const mac_rx_ctl_stru *rx_info = OSAL_NULL;

    if (netbuf == OSAL_NULL) {
        return;
    }

    if (g_custom_en == 0) {
        return;
    }

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf); /* 获取帧头信息 */
    rx_info = (const mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    hmac_rx_send_report_frame_msg(hmac_vap, rx_info, rx_ctl->rx_statistic.rssi_dbm, WLAN_MSG_H2W_UPLOAD_PROMIS);
    return;
}
#endif

OAL_STATIC osal_void hmac_show_promisc_res_by_mode(osal_u8 bit_num, osal_u8 frame_type,
    osal_u8 subtype_num, osal_u8 frame_mode)
{
    osal_u8 i;
    osal_u8 promisc_mode = g_promisc_mode;
    osal_u8 type = (frame_type == WLAN_MANAGEMENT) ? WLAN_FC0_TYPE_MGT : WLAN_FC0_TYPE_DATA;

    if (((promisc_mode >> bit_num) & 0x1) != OSAL_FALSE) {
        for (i = 0; i < subtype_num; i++) {
            if (g_promisc_st.promis_count[frame_type][i][frame_mode] != 0) {
                oam_warning_log3(0, 0, "type[%d], sub_type[%d], count[%d]\r\n",
                    type, i, g_promisc_st.promis_count[frame_type][i][frame_mode]);
            }
        }
    }
}

OAL_STATIC osal_void hmac_show_80211_frame_promisc_statistic(osal_void)
{
    osal_u8 i;
    wlan_promisc_query_stru promisc_query_module[WLAN_PROMISC_MODE_NUM] = {
        {0, WLAN_DATA_BASICTYPE, WLAN_DATA_SUBTYPE_MGMT, 1}, /* bit0 多播、广播数据帧打开 */
        {1, WLAN_DATA_BASICTYPE, WLAN_MGMT_SUBTYPE_BUTT, 0}, /* bit1 单播数据帧打开 */
        {2, WLAN_MANAGEMENT, WLAN_MGMT_SUBTYPE_BUTT, 1}, /* bit2 多播、广播管理帧打开 */
        {3, WLAN_MANAGEMENT, WLAN_MGMT_SUBTYPE_BUTT, 0}, /* bit3 单播管理帧打开 */
    };

    for (i = 0; i < osal_array_size(promisc_query_module); i++) {
        hmac_show_promisc_res_by_mode(promisc_query_module[i].bit_num, promisc_query_module[i].frame_type,
            promisc_query_module[i].subtype_num, promisc_query_module[i].frame_mode);
    }
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static osal_void hmac_promis_set_netbuf_netdev_etc(oal_netbuf_stru *netbuf)
{
    osal_u8 vap_idx;
    hmac_vap_stru *mac_vap_temp = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, 0, "{hmac_80211_frame_promis_proc::hmac_device null.}");
        return;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        mac_vap_temp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(mac_vap_temp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            return;
        }

        if (mac_vap_temp->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            netbuf->dev = mac_vap_temp->net_device;
            return;
        }
    }

    return;
}
#endif

/* 组播 广播数据帧的vap类型是配置vap, netbuf的dev需要改为station的, 其他类型报文待确认 */
static osal_u32 hmac_promisc_send_data_to_lan(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u8 type, osal_u8 is_multicast)
{
    unref_param(hmac_vap);
    unref_param(netbuf);
    unref_param(type);
    unref_param(is_multicast);

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if ((hmac_config_sta_rcv_probe_req_is_on() == OSAL_TRUE) && (type == WLAN_DATA_BASICTYPE) && (is_multicast != 0)) {
        hmac_promis_set_netbuf_netdev_etc(netbuf);
        hmac_rx_transmit_msdu_to_lan_promis(hmac_vap, netbuf);
        return OAL_SUCC;
    }
#endif
    return OAL_CONTINUE;
}

/* 函数功能: 混杂模式帧类型统计信息串口打印 */
OAL_STATIC osal_u32 hmac_80211_frame_promis_proc(hmac_vap_stru *hmac_vap,
    const mac_ieee80211_frame_stru *frame_hdr, const mac_rx_ctl_stru *rx_info, osal_s8 rssi, oal_netbuf_stru *netbuf)
{
    osal_u8 is_multicast, type, sub_type, filter, frame_mode;

    /* 混杂模式只处理RX方向的 802.11报文(包含beacon) */
    is_multicast = (frame_hdr->frame_control.to_ds == 0) ?
        ether_is_multicast(frame_hdr->address1) : ether_is_multicast(frame_hdr->address3);
    frame_mode = (is_multicast == 0) ? 0 : 1; /* 0:表示单播，1:表示广播 */
    type = frame_hdr->frame_control.type;
    sub_type = frame_hdr->frame_control.sub_type;
    filter = (osal_u8)g_promisc_mode;

    if (type == WLAN_CONTROL) { /* 控制帧处理 */
        return OAL_CONTINUE;
    } else if (type == WLAN_MANAGEMENT) { /* 管理帧处理 */
        g_promisc_st.promis_count[type][sub_type][frame_mode]++;
    } else { /* 数据帧处理 */
        g_promisc_st.promis_count[type][sub_type][frame_mode]++;
    }

    /* bit0 多播、广播数据帧没有打开 */
    if (((filter & 0x1) == OSAL_FALSE) && ((type == WLAN_DATA_BASICTYPE) && (is_multicast != 0))) {
        return OAL_CONTINUE;
    }
    /* bit1 单播数据帧没有打开 */
    if ((((filter >> 1) & 0x1) == OSAL_FALSE) && ((type == WLAN_DATA_BASICTYPE) && (is_multicast == 0))) {
        return OAL_CONTINUE;
    }
    /* bit2 多播、广播管理帧没有打开 */
    if ((((filter >> 2) & 0x1) == OSAL_FALSE) && ((type == WLAN_MANAGEMENT) && (is_multicast != 0))) {
        return OAL_CONTINUE;
    }
    /* bit3 单播管理帧没有打开 */
    if ((((filter >> 3) & 0x1) == OSAL_FALSE) && ((type == WLAN_MANAGEMENT) && (is_multicast == 0))) {
        return OAL_CONTINUE;
    }

    /* 格式转换后 发给内核 */
    if (hmac_promisc_send_data_to_lan(hmac_vap, netbuf, type, is_multicast) != OAL_CONTINUE) {
        return OAL_SUCC;
    }

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    hmac_rx_send_report_frame_msg(hmac_vap, rx_info, rssi, WLAN_MSG_H2W_UPLOAD_PROMIS);
#endif
    return OAL_CONTINUE;
}

OAL_STATIC osal_void hmac_80211_frame_promis_data2diag(const mac_ieee80211_frame_stru *wlan_frame, osal_u16 frame_len)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_WS73
    diag_option option = DIAG_OPTION_INIT_VAL;
#else
    diag_option_t option = DIAG_OPTION_INIT_VAL;
#endif

    oam_warning_log1(0, 0, "{dmac_80211_frame_promis_data2diag::frame_len[%d].}", frame_len);
#ifndef _PRE_LITEOS_SDK_
    uapi_zdiag_report_packet(WLAN_PROCMIS_ZDIAG_ID, &option, (td_u8 *)wlan_frame, (td_u16)frame_len, TD_FALSE);
#endif
#endif
}

/*****************************************************************************
 功能描述  : 混杂模式的处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_frame_promis_proc(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctl;
    const mac_rx_ctl_stru *rx_info = OSAL_NULL;
    const mac_ieee80211_frame_stru *frame_hdr;
    osal_u32 ret = OAL_SUCC;

    if (g_promisc_switch == 0) {
        return OAL_CONTINUE;
    }

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb_const(*netbuf); /* 获取帧头信息 */
    rx_info = (const mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(*netbuf);

    ret = hmac_80211_frame_promis_proc(hmac_vap, frame_hdr, rx_info, rx_ctl->rx_statistic.rssi_dbm, (*netbuf));
    hmac_80211_frame_promis_data2diag(frame_hdr, rx_ctl->rx_info.frame_len);

    return ret;
}

/*****************************************************************************
 功能描述  : 混杂模式的处理
*****************************************************************************/
OAL_STATIC osal_void hmac_promisc_set_monitor_hal_reg(osal_u8 promis_switch)
{
    hal_rx_filter_set_rx_flt_en(promis_switch);
}

OAL_STATIC osal_u32 hmac_sta_rx_deauth_req_proc_for_promisc(hmac_vap_stru *hmac_vap, const osal_u8 *mac_hdr)
{
    osal_u8 *da = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    mac_rx_get_da((mac_ieee80211_frame_stru *)mac_hdr, &da);

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req_proc_for_promisc::hmac_device is null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 混杂模式下，判断关联之后如果收到了AP发给其他STA的去关联帧，丢弃 */
    if ((g_promisc_switch != 0) &&
        (memcmp(da, hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id, WLAN_MAC_ADDR_LEN) != 0)) {
        oam_warning_log1(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req_proc_for_promisc::rx deauth for other STA}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

hmac_netbuf_hook_stru g_promisc_data_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_frame_promis_proc,
};

hmac_netbuf_hook_stru g_promisc_mgmt_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_frame_promis_proc,
};

OAL_STATIC osal_void hmac_promis_feature_hook_register(osal_void)
{
    /* 注册对外接口 */
    (osal_void)hmac_register_netbuf_hook(&g_promisc_data_netbuf_hook);
    (osal_void)hmac_register_netbuf_hook(&g_promisc_mgmt_netbuf_hook);
    hmac_feature_hook_register(HMAC_FHOOK_PROMIS_RX_DEAUTH_REQ_PROC, hmac_sta_rx_deauth_req_proc_for_promisc);
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    hmac_feature_hook_register(HMAC_FHOOK_PROMIS_SEND_CUSTOM_FRAME, hmac_promis_send_custom_frame);
#endif
    return;
}

OAL_STATIC osal_void hmac_promis_feature_hook_unregister(osal_void)
{
    /* 去注册对外接口 */
    (osal_void)hmac_unregister_netbuf_hook(&g_promisc_data_netbuf_hook);
    (osal_void)hmac_unregister_netbuf_hook(&g_promisc_mgmt_netbuf_hook);
    hmac_feature_hook_unregister(HMAC_FHOOK_PROMIS_RX_DEAUTH_REQ_PROC);
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    hmac_feature_hook_unregister(HMAC_FHOOK_PROMIS_SEND_CUSTOM_FRAME);
#endif
    return;
}

/*****************************************************************************
 功能描述  : 配置芯片是否接收其他BSS网络内发的包
 修改历史      :
  1.日    期   : 2016年3月14日
    修改内容   : 新生成函数
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_monitor_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_ps_open_stru ps_open;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u8 value = msg->data[0];
    osal_u8 promisc_siwtch = msg->data[1];

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_monitor_switch::hal_dev null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (promisc_siwtch == 2) { /* 2：混杂统计打印 */
        hmac_show_80211_frame_promisc_statistic();
        return OAL_SUCC;
    }

    g_custom_en = 0;
    /* 有其他过滤方式时，custom_en:4 不生效 */
    if ((((value >> 4) & 0x1) == OSAL_TRUE) && (promisc_siwtch == 0)) {
        g_custom_en = 1;
    }
    g_promisc_switch = promisc_siwtch;
    g_promisc_mode = value & 0xF;
    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_monitor_switch::promisc_switch[%d], promisc_cfg[%d]",
        g_promisc_switch, g_promisc_mode);

    hal_device->promis_switch = promisc_siwtch;
    if (promisc_siwtch == 1) {
        /* 混杂开启时注册 */
        wifi_printf("hmac_config_set_monitor_switch:: siwtch[%d] mode[%d]\r\n", promisc_siwtch, value);
        hmac_promis_feature_hook_register();
    } else {
        /* 混杂关闭时注册 */
        hmac_promis_feature_hook_unregister();
    }
    hmac_hal_device_sync(hal_device);
    hmac_promisc_clear_counts(); /* 混杂模式统计数组清零 */
    ps_open.pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_MONITOR; /* monitor模式低功耗控制位 */

    if (hal_device->promis_switch == 0) { /* 关闭报文上报 */
        ps_open.pm_enable = MAC_STA_PM_SWITCH_ON; /* 开启低功耗 */
    } else {
        ps_open.pm_enable = MAC_STA_PM_SWITCH_OFF; /* 关闭低功耗 */
    }
    oam_warning_log2(0, OAM_SF_CFG, "Setting Promisc mode%d! Power Manager mode%d\r\n", g_promisc_mode,
        ps_open.pm_enable);
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) { /* 须先配置低功耗模式,再配置寄存器 */
        hmac_set_sta_pm_on_cb(hmac_vap, &ps_open);
    }

    hmac_promisc_set_monitor_hal_reg(g_promisc_mode);

    return OAL_SUCC;
}

osal_u32 hmac_promis_init(osal_void)
{
    /* 消息注册 */
    /* W2H */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_MONITOR_EN, hmac_config_set_monitor_switch);
    return OAL_SUCC;
}

osal_void hmac_promis_deinit(osal_void)
{
    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_MONITOR_EN);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
