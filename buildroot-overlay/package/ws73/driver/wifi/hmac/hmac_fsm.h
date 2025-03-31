/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_fsm.h
 * 生成日期   : 2013年6月18日
 * 功能描述   : hmac_fsm.c 的头文件
 */

#ifndef __HMAC_FSM_H__
#define __HMAC_FSM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "hmac_rx_filter.h"
#include "hmac_config.h"

#if defined (WIN32)
#include "plat_pm_wlan.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FSM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
typedef osal_u32(*hmac_fsm_func)(hmac_vap_stru *hmac_vap, osal_void *p_param);

extern osal_u32 hmac_config_sta_update_rates_etc(hmac_vap_stru *hmac_vap,
    mac_cfg_mode_param_stru *cfg_mode, mac_bss_dscr_stru *bss_dscr);

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 状态机输入类型枚举 */
typedef enum {
    /* AP STA共有的输入类型 */
    HMAC_FSM_INPUT_RX_MGMT,
    HMAC_FSM_INPUT_RX_DATA,
    HMAC_FSM_INPUT_TX_DATA,
    HMAC_FSM_INPUT_TIMER0_OUT,
    HMAC_FSM_INPUT_LISTEN_REQ,  /* listen请求 */
    HMAC_FSM_INPUT_LISTEN_TIMEOUT,
    HMAC_FSM_INPUT_MISC,        /* TBTT 异常 等其他输入类型 */

    /* AP 独有的输入类型 */
    HMAC_FSM_INPUT_START_REQ,

    /* STA独有的输入类型 */
    HMAC_FSM_INPUT_SCAN_REQ,
    HMAC_FSM_INPUT_JOIN_REQ,
    HMAC_FSM_INPUT_AUTH_REQ,
    HMAC_FSM_INPUT_ASOC_REQ,
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    HMAC_FSM_INPUT_SCHED_SCAN_REQ,      /* PNO调度扫描请求 */
#endif

    HMAC_FSM_INPUT_TYPE_BUTT
} hmac_fsm_input_type_enum;
typedef osal_u8 hmac_fsm_input_type_enum_uint8;

#define HMAC_FSM_AP_INPUT_TYPE_BUTT     (HMAC_FSM_INPUT_MISC + 1)
#define HMAC_FSM_STA_INPUT_TYPE_BUTT    HMAC_FSM_INPUT_TYPE_BUTT
#define HMAC_SWITCH_STA_PSM_PERIOD      120000                  // staut打开低功耗超时的时间120s
#define HMAC_SWITCH_STA_PSM_MAX_CNT     (HMAC_SWITCH_STA_PSM_PERIOD / WLAN_SLEEP_TIMER_PERIOD)

/* MISC输入类型的子类型定义 */
typedef enum {
    HMAC_MISC_TBTT,
    HMAC_MISC_ERROR,
    HMAC_MISC_RADAR,

    HMAC_MISC_BUTT
} hmac_misc_input_enum;
typedef osal_u8 hmac_misc_input_enum_uint8;

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
/* MISC输入类型，入参结构体定义 */
typedef struct {
    hmac_misc_input_enum_uint8  type;
    osal_u8                   auc_resv[3];
    osal_void                   *p_data;
} hmac_misc_input_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_void  hmac_fsm_init_etc(osal_void);
extern osal_u32  hmac_fsm_call_func_ap_etc(hmac_vap_stru *hmac_vap,
    hmac_fsm_input_type_enum_uint8 input, osal_void *p_param);
extern osal_u32  hmac_fsm_call_func_sta_etc(hmac_vap_stru *hmac_vap,
    hmac_fsm_input_type_enum_uint8 input, osal_void *p_param);
extern osal_void  hmac_fsm_change_state_etc(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 vap_state);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_fsm.h */
