/*
 * Copyright (c) CompanyNameMagicTag. 2013-2024. All rights reserved.
 * 文 件 名   : hmac_encap_frame_ap.h
 * 生成日期   : 2013年6月28日
 * 功能描述   : hmac_encap_frame_ap.c 的头文件
 */

#ifndef __HMAC_ENCAP_FRAME_AP_H__
#define __HMAC_ENCAP_FRAME_AP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_user.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
typedef enum {
    /* 加密方式为open */
    HMAC_AP_AUTH_SEQ1_OPEN_ANY  = 0,
    /* 加密方式为wep,处理重传帧 */
    HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND,
    /* 加密方式为wep,处理非重传帧 */
    HMAC_AP_AUTH_SEQ1_WEP_RESEND,
    /* 加密方式为open */
    HMAC_AP_AUTH_SEQ3_OPEN_ANY,
    /* 加密方式为WEP,assoc状态为auth comlete */
    HMAC_AP_AUTH_SEQ3_WEP_COMPLETE,
    /* 加密方式为WEP,assoc状态为assoc */
    HMAC_AP_AUTH_SEQ3_WEP_ASSOC,
    /* 什么也不做 */
    HMAC_AP_AUTH_DUMMY,

    HMAC_AP_AUTH_BUTT
} hmac_ap_auth_process_code_enum;
typedef osal_u8 hmac_ap_auth_process_code_enum_uint8;

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
typedef struct tag_hmac_auth_rsp_param_stru {
    /* 收到auth request是否为重传帧 */
    osal_u8                               auth_resend;
    /* 记录是否为wep */
    oal_bool_enum_uint8                     is_wep_allowed;
    /* 记录认证的类型 */
    osal_u16                              auth_type;
    /* 记录函数处理前，user的关联状态 */
    mac_user_asoc_state_enum_uint8          user_asoc_state;
    osal_u8                               pad[3];
} hmac_auth_rsp_param_stru;

typedef hmac_ap_auth_process_code_enum_uint8(*hmac_auth_rsp_fun)(
    hmac_vap_stru *hmac_vap, \
    const hmac_auth_rsp_param_stru *auth_rsp_param, \
    osal_u8 *code, \
    hmac_user_stru *hmac_user);

typedef struct tag_hmac_auth_rsp_handle_stru {
    hmac_auth_rsp_param_stru auth_rsp_param;
    hmac_auth_rsp_fun        auth_rsp_fun;
} hmac_auth_rsp_handle_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/

osal_u16  hmac_encap_auth_rsp_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_rsp,
    oal_netbuf_stru *auth_req, osal_u8 *chtxt);

osal_u32 hmac_mgmt_encap_asoc_rsp_ap_etc(hmac_vap_stru *hmac_vap, osal_u8 *sta_addr, hmac_user_stru *hmac_user,
    mac_status_code_enum_uint16 status_code, osal_u8 *asoc_rsp);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_encap_frame_ap.h */
