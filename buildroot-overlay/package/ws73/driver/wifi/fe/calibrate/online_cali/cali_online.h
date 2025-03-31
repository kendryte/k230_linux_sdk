/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: cali_online.c 的头文件
 * Date: 2022-10-20
 */

#ifndef __CALI_ONLINE_H__
#define __CALI_ONLINE_H__
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "cali_online_tx_pwr.h"
#include "cali_online_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ONLINE_CALI_CUS_FB_LVL_NUM                 (2)     /* 功率校准区分高低两个档位 */
#define ONLINE_CALI_INIT_ENTRY_DELAY_TIMER_PERIOD  (200) /* INIT状态进入延迟 */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/* ONLINE TX CALI状态机状态枚举 */
typedef enum {
    ONLINE_CALI_STATE_INIT    = 0,  /* 初始运行状态 */
    ONLINE_CALI_STATE_DYN_PWR = 1,  /* 动态功率状态 */

    ONLINE_CALI_STATE_BUTT          /* 最大状态 */
} online_tx_cali_state;

/* ONLINE TX CALI状态机事件枚举 */
typedef enum {
    ONLINE_CALI_EVENT_TX_NOTIFY         = 0,        /* 发送时online校准处理事件 */
    ONLINE_CALI_EVENT_TX_COMPLETE       = 1,        /* 发送完成时online校准处理事件 */

    ONLINE_CALI_EVENT_DYN_PWR_COMPLETE  = 2,        /* 动态功率校准完成处理事件 */

    ONLINE_CALI_EVENT_CHANNEL_CHANGE    = 7,        /* 信道改变时处理事件 */
    ONLINE_CALI_EVENT_BW_CHANGE         = 8,        /* 带宽改变时处理事件 */
    ONLINE_CALI_EVENT_EQUIPE_CHANGE     = 9,        /* 装备校准时处理事件 */

    ONLINE_CALI_EVENT_BUTT
} online_cali_event_tpye;
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
    oal_fsm_stru st_oal_fsm;
    oal_bool_enum_uint8 en_is_fsm_attached;
    osal_u8 auc_resv[3]; /* 3代表3个字节 */
} online_cali_tx_fsm_info_stru;
typedef struct {
    osal_u8 uc_chip_id;
    osal_u8 uc_device_id;
    osal_u8 online_cali_mask[WLAN_CALI_BAND_BUTT];
    osal_u8 rsv;
    frw_timeout_stru dyn_cali_init_entry_delay_timer; /* 初始化进入的定时器 */
    online_cali_tx_fsm_info_stru st_online_tx_cali_fsm;
    online_cali_dyn_stru dyn_cali_val; /* 动态校准结构体 */
} online_cali_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 online_cali_trans_to_next_state(online_cali_stru *rf_cali);
online_cali_stru *online_cali_get_alg_rf_online_cali_ctx(osal_void);
osal_u32 online_rf_cali_fsm_trans_to_state(online_cali_tx_fsm_info_stru *online_cali_fsm, osal_u8 state);
osal_u32 online_rf_cali_handle_event(online_cali_stru *rf_cali, osal_u16 type,
    osal_u16 event_data_len, osal_u8 *event_data);
osal_void online_set_cali_id(online_cali_stru *alg_rf_cali, osal_u8 chip_id, osal_u8 dev_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_ONLINE_TX_PWR_H__