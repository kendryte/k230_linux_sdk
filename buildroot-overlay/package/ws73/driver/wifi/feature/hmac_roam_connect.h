/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_connect.h
 * 生成日期   : 2015年3月18日
 * 功能描述   : hmac_roam_connect.c 的头文件
 */


#ifndef __HMAC_ROAM_CONNECT_H__
#define __HMAC_ROAM_CONNECT_H__

#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_CONNECT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ROAM_JOIN_TIME_MAX        (1 * 1000)       /* JOIN超时时间 单位ms */
#define ROAM_AUTH_TIME_MAX        (300)            /* AUTH超时时间 单位ms */
#define ROAM_ASSOC_TIME_MAX       (300)            /* ASSOC超时时间 单位ms */
#define ROAM_HANDSHAKE_TIME_MAX   (7 * 1000)       /* 握手超时时间 单位ms */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 漫游connect状态机状态 */
typedef enum {
    ROAM_CONNECT_STATE_INIT               = 0,
    ROAM_CONNECT_STATE_FAIL               = ROAM_CONNECT_STATE_INIT,
    ROAM_CONNECT_STATE_WAIT_JOIN          = 1,
    ROAM_CONNECT_STATE_WAIT_FT_COMP       = 2,
    ROAM_CONNECT_STATE_WAIT_AUTH_COMP     = 3,
    ROAM_CONNECT_STATE_WAIT_ASSOC_COMP    = 4,
    ROAM_CONNECT_STATE_HANDSHAKING        = 5,
    ROAM_CONNECT_STATE_UP                 = 6,

    ROAM_CONNECT_STATE_BUTT
} roam_connect_state_enum;
typedef osal_u8  roam_connect_state_enum_uint8;

/* 漫游connect状态机事件类型 */
typedef enum {
    ROAM_CONNECT_FSM_EVENT_START          = 0,
    ROAM_CONNECT_FSM_EVENT_MGMT_RX        = 1,
    ROAM_CONNECT_FSM_EVENT_KEY_DONE       = 2,
    ROAM_CONNECT_FSM_EVENT_TIMEOUT        = 3,
    ROAM_CONNECT_FSM_EVENT_FT_OVER_DS     = 4,
    ROAM_CONNECT_FSM_EVENT_TYPE_BUTT
} roam_connect_fsm_event_type_enum;

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_roam_connect_set_join_reg_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_roam_connect_start_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr);
osal_u32 hmac_roam_connect_stop_etc(hmac_vap_stru *hmac_vap);
osal_u32 hmac_roam_connect_rx_mgmt_etc(oal_netbuf_stru *netbuf, hmac_vap_stru *hmac_vap);
osal_void hmac_roam_connect_fsm_init_etc(osal_void);
osal_void hmac_roam_connect_key_done_etc(hmac_vap_stru *hmac_vap);
osal_void hmac_roam_connect_complete_etc(hmac_vap_stru *hmac_vap, osal_u32 result);
osal_void hmac_roam_connect_fsm_deinit_etc(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_roam_connect.h */
