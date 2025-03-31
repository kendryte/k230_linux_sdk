/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for oal_fsm
 * Create: 2020-08-18
 */
#ifndef __OAL_FSM_H__
#define __OAL_FSM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define OAL_FSM_MAX_NAME    32  /* 状态机名最大长度 */
#define OAL_FSM_MAX_STATES 100
#define OAL_FSM_MAX_EVENTS 100
#define OAL_FSM_STATE_NONE 255 /* invalid state */
#define OAL_FSM_EVENT_NONE 255 /* invalid event */

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 状态信息结构定义 */
typedef struct __oal_fsm_state_info {
    osal_u32          state;                   /* 状态ID */
    const osal_char      *name;                   /* 状态名 */
    osal_void (*oal_fsm_entry)(osal_void *p_ctx);  /* 进入本状态的处理回调函数指针 */
    osal_void (*oal_fsm_exit)(osal_void *p_ctx);   /* 退出本状态的处理回调函数指针 */
                                         /* 本状态下的事件处理回调函数指针 */
    osal_u32 (*oal_fsm_event)(osal_void *p_ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data);
} oal_fsm_state_info;

/* 状态机结构定义 */
typedef struct  __oal_fsm {
    osal_u8  name[OAL_FSM_MAX_NAME];              /* 状态机名字 */
    osal_u8  cur_state;                           /* 当前状态 */
    osal_u8  prev_state;                          /* 前一状态，发出状态切换事件的状态 */
    osal_u8  num_states;                          /* 状态机的状态个数 */
    osal_u8  rsv[1];
    const oal_fsm_state_info *p_state_info;
    osal_void   *p_ctx;                                 /* 上下文，指向状态机实例拥有者 */
    osal_void   *p_oshandler;                           /* owner指针，指向VAP或者device,由具体的状态机决定 */
    osal_u16  last_event;                          /* 最后处理的事件 */
    osal_u8  rsv1[2];                          /* 保留2字节对齐 */
} oal_fsm_stru;

typedef struct {
    osal_void *oshandle;  /* 状态机owner的指针，对低功耗状态机，指向VAP结构 */
    osal_u8   *name;      /* 状态机的名字 */
    osal_void *context;   /* 状态机context */
} oal_fsm_create_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32  oal_fsm_create(const oal_fsm_create_stru *fsm_stru,
    oal_fsm_stru    *oal_fsm,                   /* oal状态机内容 */
    osal_u8           init_state,                  /* 初始状态 */
    const oal_fsm_state_info *p_state_info,         /* 状态机实例指针 */
    osal_u8                    num_states);        /* 本状态机的状态个数 */

osal_u32 oal_fsm_trans_to_state(oal_fsm_stru *p_fsm, osal_u8 state);

osal_u32 oal_fsm_event_dispatch(oal_fsm_stru *p_fsm, osal_u16 event,
    osal_u16 event_data_len, osal_void *p_event_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
