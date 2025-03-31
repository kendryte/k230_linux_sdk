/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_psd
 * Author:
 * Create: 2022-08-15
 */
#include "hmac_psd.h"
#include "hmac_config.h"
#ifndef _PRE_WLAN_FEATURE_WS73
#include "diag.h"
#else
#include "soc_zdiag.h"
#endif
#include "wal_common.h"
#include "hmac_feature_interface.h"
#include "hal_psd.h"
#include "soc_ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PSD_C
static hmac_psd_enable_timer g_psd_enable_timer = {0};
/* psd定时器处理函数 */
static osal_u32 hmac_psd_enable_callback(osal_void *hmac_vap_arg);
static osal_u8 g_psd_switch = 0;

typedef osal_void (*psd_enable_callback)(osal_u16 cmd_id, osal_void* cmd_param, osal_u16 cmd_param_size,
    DIAG_OPTION *option);
osal_void uapi_zdiag_psd_enable_register(psd_enable_callback cb);
osal_void uapi_zdiag_psd_enable_unregister(void);
#ifndef _PRE_WLAN_FEATURE_WS73
errcode_t uapi_diag_report_packet(uint16_t cmd_id, DIAG_OPTION *option, const uint8_t *packet, uint16_t packet_size,
    bool sync);
#else
ext_errno zdiag_report_packet(td_u16 cmd_id, DIAG_OPTION *option, TD_CONST td_u8 *packet, td_u16 packet_size,
    td_bool sync);
#endif

osal_u8 hmac_psd_get_switch(void)
{
    return g_psd_switch;
}

static osal_void hmac_psd_enable(hmac_psd_enable_stru *psd_enable_param, hmac_vap_stru *hmac_vap)
{
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));

    g_psd_switch = OSAL_TRUE;
    /* 设置中心信道(默认7, 80M采样率, 覆盖1~13信道) */
    msg_info.data = (osal_u8 *)&psd_enable_param->channel;
    msg_info.data_len = sizeof(psd_enable_param->channel);
    hmac_config_set_freq_etc(hmac_vap, &msg_info);
    /* 先打开mac跟phy */
    hal_enable_machw_phy_and_pa(hmac_vap->hal_device);
    /* psd寄存器使能 */
    hal_psd_enable_etc();
    /* 创建定时器, 定期获取PSD数据 */
    if (g_psd_enable_timer.psd_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_psd_enable_timer.psd_timer);
    }
    g_psd_enable_timer.sampling_count = 0;
    /* 采样时间单位为min 转成ms, 乘以60 * 1000; 周期单位为ms */
    g_psd_enable_timer.max_sampling_count = psd_enable_param->duration * 60 * 1000 / psd_enable_param->cycle;
    frw_create_timer_entry(&g_psd_enable_timer.psd_timer, hmac_psd_enable_callback, psd_enable_param->cycle, hmac_vap,
        OSAL_TRUE);
    wifi_printf("psd enable\r\n");
}

static osal_void hmac_psd_disable(hmac_psd_enable_stru *psd_enable_param, hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
    unref_param(psd_enable_param);

    /* psd寄存器去使能 */
    hal_psd_disable_etc();
    /* 销毁定时器 */
    frw_destroy_timer_entry(&g_psd_enable_timer.psd_timer);
    g_psd_enable_timer.sampling_count = 0;
    g_psd_enable_timer.max_sampling_count = 0;

    /* 最后关闭mac跟phy */
    hal_disable_machw_phy_and_pa();
    g_psd_switch = OSAL_FALSE;
    wifi_printf("psd disable\r\n");
}
/* psd使能时的定时处理函数 */
static osal_u32 hmac_psd_enable_callback(osal_void *hmac_vap_arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)hmac_vap_arg;
    frw_msg psd_msg;
    DIAG_OPTION option = {0};
    osal_u32 psd_data = OSAL_FALSE;
    (osal_void)memset_s(&psd_msg, sizeof(frw_msg), 0, sizeof(frw_msg));
    g_psd_enable_timer.sampling_count++;
    /* 采样次数已够 关闭psd */
    if (g_psd_enable_timer.sampling_count > g_psd_enable_timer.max_sampling_count) {
        wifi_printf("reach max_sampling_count[%d], disable psd\r\n", g_psd_enable_timer.max_sampling_count);
        hmac_psd_disable(OSAL_NULL, hmac_vap);
        /* 给hso发消息终止测量 */
#ifndef _PRE_WLAN_FEATURE_WS73
        uapi_diag_report_packet(DIAG_CMD_ID_PSD_ENABLE, &option, (osal_u8 *)&psd_data, sizeof(psd_data), TD_TRUE);
#else
        zdiag_report_packet(DIAG_CMD_ID_PSD_ENABLE, &option, (osal_u8 *)&psd_data, sizeof(psd_data), TD_TRUE);
#endif
        return OAL_SUCC;
    }
    /* 发消息给dmac取数据 */
    frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_GET_PSD_DATA, &psd_msg, OSAL_TRUE);
    return OAL_SUCC;
}

osal_s32 hmac_config_psd_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_psd_enable_stru *psd_enable_param = (hmac_psd_enable_stru *)msg->data;
    frw_msg psd_enable_msg = {0};
    osal_s32 ret;
    (osal_void)memset_s(&psd_enable_msg, sizeof(frw_msg), 0, sizeof(frw_msg));

    oam_warning_log3(0, OAM_SF_ANY, "hmac_config_psd_enable enable[%d] cyc[%d] dur[%d]",
        psd_enable_param->enable, psd_enable_param->cycle, psd_enable_param->duration);

    psd_enable_msg.data = (osal_u8 *)&psd_enable_param->enable;
    psd_enable_msg.data_len = sizeof(psd_enable_param->enable);
    if (psd_enable_param->enable == OSAL_TRUE) {
        hmac_psd_enable(psd_enable_param, hmac_vap);
    } else {
        hmac_psd_disable(psd_enable_param, hmac_vap);
    }
    /* 发消息通知dmac */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_PSD_ENABLE, &psd_enable_msg, OSAL_TRUE);
    return ret;
}

/* 处理dmac报上来的psd数据,交给hso解析 */
osal_s32 hmac_config_psd_data_report(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s8 *psd_data = (osal_s8 *)msg->data;
    DIAG_OPTION option = {0};
    osal_s32 ret = 0;

    unref_param(hmac_vap);
    unref_param(ret);
#ifndef _PRE_WLAN_FEATURE_WS73
    uapi_diag_report_packet(DIAG_CMD_ID_PSD_REPORT, &option, (osal_u8 *)psd_data, PSD_DATA_LEN, TD_TRUE);
#else
    zdiag_report_packet(DIAG_CMD_ID_PSD_REPORT, &option, (osal_u8 *)psd_data, PSD_DATA_LEN, TD_TRUE);
#endif
    /* los在上报给HSO后还要上报给Service API */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_RX_PSD, FRW_POST_PRI_LOW, hmac_vap->vap_id, msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_config_psd_data_report err ret[%d]", ret);
        return ret;
    }
#endif
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_psd_diag_enable_callback(osal_u16 cmd_id, osal_void* cmd_param, osal_u16 cmd_param_size,
    DIAG_OPTION *option)
{
    hmac_psd_enable_stru *psd_enable_param = (hmac_psd_enable_stru *)cmd_param;
    oal_net_device_stru *net_dev;
    osal_s32 ret;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    static osal_u8 first_get_ifname = 1;
    osal_s8 ifname[OAL_IF_NAME_SIZE] = {0};
    static osal_s8 name[OAL_IF_NAME_SIZE] = {0};
#endif

    unref_param(option);
    unref_param(cmd_id);
    unref_param(cmd_param_size);
    wifi_printf("hmac_psd_diag_enable_callback enable[%d] ch[%d] bw[%d] cyc[%d] dur[%d]\r\n",
        psd_enable_param->enable, psd_enable_param->channel, psd_enable_param->band, psd_enable_param->cycle,
        psd_enable_param->duration);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (first_get_ifname == 1) {
        first_get_ifname = 0;
        (void)memset_s(ifname, sizeof(ifname), 0, sizeof(ifname));
        (void)memset_s(name, sizeof(name), 0, sizeof(name));
#ifdef _PRE_WLAN_NO_SUPPORT_INI
        ret = strcpy_s(ifname, OAL_IF_NAME_SIZE, "wlan0");
#else
        ret = get_cust_conf_string_etc(INI_MODU_WIFI, "wlan_ifname0", ifname, sizeof(ifname));
#endif
        if ((ret != INI_SUCC) || (osal_strlen(ifname) == 0) ||
            (memcpy_s(name, sizeof(name), ifname, osal_strlen(ifname)) != EOK)) {
            oam_warning_log2(0, 0, "{hmac_psd_diag_enable_callback:find ifname0 fail %d %d}", ret, osal_strlen(ifname));
            (void)memset_s(name, sizeof(name), 0, sizeof(name));
            (void)memcpy_s(name, sizeof(name), PSD_DEV_NAME, osal_strlen(PSD_DEV_NAME));
        }
    }
    net_dev = oal_dev_get_by_name(name);
#else
    net_dev = oal_get_netdev_by_name(PSD_DEV_NAME);
#endif
    if (net_dev == OSAL_NULL) {
        oam_warning_log0(0, 0, "hmac_psd_diag_enable_callback::net_dev null");
        return;
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);
#endif
    /* 此处为临时方案,待设置信道卡死问题解决后改为直调 */
    ret = wal_async_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_PSD_ENABLE,
        (osal_u8 *)psd_enable_param, sizeof(hmac_psd_enable_stru), FRW_POST_PRI_LOW);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_psd_enable_callback::return err code %d!}", ret);
        return;
    }
}

osal_u32 hmac_psd_init(osal_void)
{
    /* 消息注册 */
    /* W2H */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_PSD_ENABLE, hmac_config_psd_enable);
    /* H2D */
    /* D2H */
    frw_msg_hook_register(WLAN_MSG_D2H_PSD_DATA_REPORT, hmac_config_psd_data_report);
    /* 对外接口注册 */
    uapi_zdiag_psd_enable_register(hmac_psd_diag_enable_callback);
    hmac_feature_hook_register(HMAC_FHOOK_PSD_GET_SWITCH, hmac_psd_get_switch);
    return OAL_SUCC;
}

osal_void hmac_psd_deinit(osal_void)
{
    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_D2H_PSD_DATA_REPORT);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_PSD_ENABLE);
    /* 对外接口去注册 */
    uapi_zdiag_psd_enable_unregister();
    hmac_feature_hook_unregister(HMAC_FHOOK_PSD_GET_SWITCH);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif