/*
 * Copyright (c) CompanyNameMagicTag. 2022-2024. All rights reserved.
 * 文 件 名   : hmac_mbo.h
 * 作    者   : CompanyName
 * 创建日期 : 2022年09月24日
 */

#ifndef __HMAC_MBO_H__
#define __HMAC_MBO_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_11v.h"
#include "hmac_device.h"
#include "hmac_11k.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MBO_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAX_STA_MBO_IE_LEN 9
#define MAC_GLOBAL_OPERATING_CLASS_115_CH_NUMS  4 /* global operating class 115对应的信道集个数 */
#define HMAC_11V_MBO_RE_ASSOC_DALAY_TIME_S_TO_MS 1000

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    MAC_GLOBAL_OPERATING_CLASS_NUMBER81 = 81,
    MAC_GLOBAL_OPERATING_CLASS_NUMBER115 = 115,

    MAC_GLOBAL_OPERATING_CLASS_BUFF,
} mac_global_operaing_class_enum;

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
    osal_u8 mbo_cell_capa; /* MBO STA移动数据能力 */
    osal_u8 mbo_assoc_disallowed_test_switch;
    osal_u8 disable_connect; /* 是否发送re-assoc req帧给对应BSS标志位 */
    osal_u8 resv0;
    osal_u32 btm_req_received_time; /* 收到BTM Req帧的时间 */
    osal_u32 reassoc_delay_time; /* Neighbor元素中获得的re-assoc delay time ms */
    osal_u8 reassoc_delay_bss_mac_addr[WLAN_MAC_ADDR_LEN]; /* 有re-assoc delay的BSS地址 */
    osal_u8 resv1[2];
} mac_mbo_para_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 inline函数定义
*****************************************************************************/

/*****************************************************************************
  11 函数声明
*****************************************************************************/
typedef osal_void (*hmac_mbo_assoc_add_mbo_and_soc_ie_cb)(hmac_vap_stru *hmac_vap,
    osal_u8 *buffer, osal_u8 *ie_len, hmac_scanned_bss_info  *scaned_bss);
typedef osal_void (*hmac_handle_ie_specific_mbo_cb)(osal_u8 *ie_data,
    hmac_neighbor_bss_info_stru *bss_list_alloc, osal_u8 bss_list_index);
typedef osal_u32 (*hmac_mbo_check_is_assoc_or_re_assoc_allowed_cb)(hmac_vap_stru *hmac_vap,
    mac_conn_param_stru *connect_param, mac_bss_dscr_stru *bss_dscr);
typedef osal_void (*hmac_scan_update_bss_assoc_disallowed_attr_cb)(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body, osal_u16 frame_len);
typedef osal_void (*hmac_set_bss_re_assoc_delay_params_cb)(const hmac_bsst_req_info_stru *bsst_req_info,
    const hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_roam);
typedef osal_void (*hmac_roam_attach_mbo_ie_auth_cb)(hmac_vap_stru *hmac_vap,
    osal_u8 *mgmt_frame, osal_u16 *auth_len, mac_bss_dscr_stru *bss_dscr);
typedef osal_void (*hmac_vap_mbo_update_rrm_scan_params_cb)(const mac_bcn_req_stru *bcn_req,
    mac_scan_req_stru *scan_req);
typedef osal_void (*hmac_mbo_modify_chan_param_cb)(osal_u8 optclass,
    wlan_channel_band_enum_uint8 *chan_band, osal_u8 *chan_count);
typedef osal_u32 (*hmac_mbo_check_neighbor_subie_len_cb)(osal_u8 *ie_data, osal_u8 sub_ie_len);
typedef osal_void (*hmac_mbo_scan_add_mbo_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *app_ie, osal_u8 *ie_len);
typedef osal_void (*hmac_remove_mbo_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *ies, osal_s32 *ie_len);

static osal_u32 hmac_mbo_sta_init_weakref(osal_void) __attribute__ ((weakref("hmac_mbo_sta_init"), used));
static osal_void hmac_mbo_sta_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_mbo_sta_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mob.h */