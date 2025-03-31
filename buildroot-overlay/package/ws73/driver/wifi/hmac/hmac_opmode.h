/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * 文 件 名   : hmac_opmode.h
 * 生成日期   : 2016年9月1日
 * 功能描述   : hmac_opmode.c 的头文件
 */
#ifndef __HMAC_OPMODE_H__
#define __HMAC_OPMODE_H__

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hmac_ext_if.h"
#include "oam_ext_if.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "mac_user_ext.h"
#include "mac_ie.h"
#include "hmac_vap.h"
#include "hmac_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_OPMODE_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


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
typedef osal_void (*encap_opmode_action)(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf, osal_u16 *len,
    wlan_nss_enum_uint8 nss);

typedef struct {
    encap_opmode_action encap_opmode_action_cb;
} hmac_opmode_cb;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_check_opmode_notify_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    osal_u8 *payload_offset, osal_u32 msg_len, hmac_user_stru *hmac_user);
extern osal_u32  hmac_mgmt_rx_opmode_notify_frame_etc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *pst_netbuf);
extern osal_s32 hmac_config_set_opmode_notify_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_void hmac_mgmt_encap_opmode_notify_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u16 *len,
    oal_bool_enum_uint8 bool_code, wlan_nss_enum_uint8 nss);
osal_u32 hmac_mgmt_send_opmode_notify_action(hmac_vap_stru *hmac_vap, wlan_nss_enum_uint8 nss,
    oal_bool_enum_uint8 bool_code);
osal_u32 hmac_rx_beacon_check_opmode_notify(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 msg_len,
    hmac_user_stru *hmac_user);
osal_u32 hmac_mgmt_rx_opmode_notify_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf);
osal_u32 hmac_ie_proc_opmode_notify(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    mac_opmode_notify_stru *opmode_notify);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of _PRE_WLAN_FEATURE_OPMODE_NOTIFY */

#endif /* end of hmac_opmode.h */
