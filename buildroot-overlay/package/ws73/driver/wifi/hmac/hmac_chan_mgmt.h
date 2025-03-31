/*
 * Copyright (c) CompanyNameMagicTag Co., Ltd. 2014-2024. All rights reserved.
 * 文 件 名   : hmac_chan_mgmt_ap.h
 * 生成日期   : 2014年2月22日
 * 功能描述   : hmac_chan_mgmt.c 的头文件
 */

#ifndef __HMAC_CHAN_MGMT_H__
#define __HMAC_CHAN_MGMT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "mac_regdomain.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MGMT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_CHANNEL_SWITCH_COUNT 10
#define HMAC_CHANNEL_NEED_CHECK_CH12 12
#define HMAC_CHANNEL_NEED_CHECK_CH13 13
#define HMAC_BW_40M_MAX_CHAN_IDX_CH12_DISALLOWED 7
#define HMAC_BW_40M_MAX_CHAN_IDX_CH12_ALLOWED 9
#define HMAC_BW_40M_MIN_CHAN_IDX 5

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

typedef enum {
    MAC_CHNL_AV_CHK_NOT_REQ  = 0,   /* 不需要进行信道扫描 */
    MAC_CHNL_AV_CHK_IN_PROG  = 1,   /* 正在进行信道扫描 */
    MAC_CHNL_AV_CHK_COMPLETE = 2,   /* 信道扫描已完成 */

    MAC_CHNL_AV_CHK_BUTT,
} mac_chnl_av_chk_enum;
typedef osal_u8 mac_chnl_av_chk_enum_uint8;


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u8     idx;     /* 信道索引号 */
    osal_u16    freq;    /* 信道频点 */
    osal_u8     auc_resv;
} hmac_dfs_channel_info_stru;

/* 频率信道索引映射关系 */

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_chan_attempt_new_chan(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void hmac_chan_select_real_channel(const hmac_device_stru *hmac_device, mac_channel_stru *channel,
    osal_u8 dst_chan_num);
osal_void hmac_chan_select_channel_mac(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void hmac_chan_disable_machw_tx(const hmac_vap_stru *hmac_vap);
osal_void hmac_chan_enable_machw_tx(const hmac_vap_stru *hmac_vap);
osal_void hmac_switch_complete_notify(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 check_cac);
osal_void hmac_chan_update_user_bandwidth(hmac_vap_stru *hmac_vap);
osal_void  hmac_chan_dfs_update_alg_anti_intf_switch(const hal_to_dmac_device_stru *hal_device,
    const mac_channel_stru *channel);
osal_u32  hmac_mgmt_switch_channel(hal_to_dmac_device_stru *hal_device, mac_channel_stru *channel,
    osal_bool clear_fifo);

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
mac_freq_channel_map_stru get_ast_freq_map_2g_elem(td_u32 index);
#endif
extern osal_void  hmac_chan_multi_select_channel_mac_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern osal_u32  hmac_start_bss_in_available_channel_etc(hmac_vap_stru *hmac_vap);
extern osal_u32  hmac_chan_restart_network_after_switch_etc(hmac_vap_stru *hmac_vap);
extern osal_void  hmac_chan_multi_switch_to_new_channel_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern osal_void hmac_chan_sync_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth, oal_bool_enum_uint8 switch_immediately);
#ifdef _PRE_WLAN_FEATURE_DBAC
extern osal_s32 hmac_dbac_status_notify_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_u32  hmac_chan_start_bss_etc(hmac_vap_stru *hmac_vap, mac_channel_stru *channel,
    wlan_protocol_enum_uint8 protocol);
#if defined(_PRE_PRODUCT_ID_HOST)
extern osal_u32 hmac_check_ap_channel_follow_sta(const hmac_vap_stru *check_mac_vap,
    mac_channel_stru *set_mac_ch);
#endif

/*****************************************************************************
  11 inline函数定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_chan_scan_availability
 输出参数  : 无
    备注:该函数仅02使用，03使用时需要将mac_dfs_get_dfs_enable替换为mac_vap_get_dfs_enable
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hmac_chan_scan_availability(
    hmac_device_stru *hmac_device,
    mac_ap_ch_info_stru    *channel_info)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    if (OAL_FALSE == mac_dfs_get_dfs_enable(hmac_device)) {
        return OAL_TRUE;
    }

    if ((MAC_CHAN_NOT_SUPPORT != channel_info->ch_status) &&
        (MAC_CHAN_BLOCK_DUE_TO_RADAR != channel_info->ch_status)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
#else
    unref_param(hmac_device);
    unref_param(channel_info);
    return OAL_TRUE;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_chan_get_bw_40m_max_chan_idx
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u8  hmac_chan_get_bw_40m_max_chan_idx(void)
{
    /* 当前国家码支持12, 13信道 */
    if ((hmac_is_channel_num_valid_etc(WLAN_BAND_2G, HMAC_CHANNEL_NEED_CHECK_CH12) == OAL_SUCC) &&
        (hmac_is_channel_num_valid_etc(WLAN_BAND_2G, HMAC_CHANNEL_NEED_CHECK_CH13) == OAL_SUCC)) {
        return HMAC_BW_40M_MAX_CHAN_IDX_CH12_ALLOWED;
    }
    return HMAC_BW_40M_MAX_CHAN_IDX_CH12_DISALLOWED;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_chan_mgmt.h */
