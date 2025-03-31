/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Executive file of MAIN element structure and its corresponding enumeration.
 * Create: 2020-6-18
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_OAL_FSM_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifndef _PRE_WLAN_FEATURE_WS63  /* fsm重复代码，63上不编译 */

/*****************************************************************************
 函 数 名  : oal_fsm_create
 功能描述  : 状态机创建接口
*****************************************************************************/
osal_u32  oal_fsm_create(const oal_fsm_create_stru *fsm_stru,
                         oal_fsm_stru *oal_fsm,                 /* oal状态机内容 */
                         osal_u8 init_state,                     /* 初始状态 */
                         const oal_fsm_state_info *p_state_info,    /* 状态机实例指针 */
                         osal_u8 num_states)                      /* 本状态机的状态个数 */
{
    osal_u32      loop;

    if (oal_fsm == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{oal_fsm_create:oal_fsm is NULL }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (num_states > OAL_FSM_MAX_STATES) {
        oam_error_log1(0, OAM_SF_CFG, "{oal_fsm_create:state number [%d] too big. }", num_states);
        return OAL_FAIL;
    }

    /* 检查状态信息顺序是否和状态定义匹配 */
    for (loop = 0; loop < num_states; loop++) {
        if ((p_state_info[loop].state >= OAL_FSM_MAX_STATES) || (p_state_info[loop].state != loop)) {
            /* OAM日志中不能使用%s */
            oam_error_log2(0, OAM_SF_CFG, "{oal_fsm_create::entry %d has invalid state %d }",
                loop, p_state_info[loop].state);
            return OAL_FAIL;
        }
    }

    (osal_void)memset_s(oal_fsm, sizeof(oal_fsm_stru), 0, sizeof(oal_fsm_stru));
    oal_fsm->cur_state  = init_state;
    oal_fsm->prev_state = init_state;
    oal_fsm->p_state_info  = p_state_info;
    oal_fsm->num_states = num_states;
    oal_fsm->p_oshandler   = fsm_stru->oshandle;
    oal_fsm->p_ctx         = fsm_stru->context;
    oal_fsm->last_event = OAL_FSM_EVENT_NONE;

    /* strncpy fsm name */
    loop = 0;
    while ((fsm_stru->name[loop] != '\0') && (loop < OAL_FSM_MAX_NAME - 1)) {
        oal_fsm->name[loop] = fsm_stru->name[loop];
        loop++;
    }
    if (loop < OAL_FSM_MAX_NAME) {
        oal_fsm->name[loop] = '\0';
    }

    /* 启动状态机 */
    if (oal_fsm->p_state_info[oal_fsm->cur_state].oal_fsm_entry) {
        oal_fsm->p_state_info[oal_fsm->cur_state].oal_fsm_entry(oal_fsm->p_ctx);
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : mac_fsm_trans_to
 功能描述  : 状态机的destroy接口
*****************************************************************************/
osal_u32 oal_fsm_trans_to_state(oal_fsm_stru* p_fsm, osal_u8 state)
{
    osal_u8                    cur_state = p_fsm->cur_state;

    if ((state == OAL_FSM_STATE_NONE) || (state >= OAL_FSM_MAX_STATES) || (state >= p_fsm->num_states)) {
        /* OAM日志中不能使用%s */
        oam_error_log2(0, OAM_SF_PWR, "FSM:trans to state %d needs to be a valid state cur_state=%d",
            state, cur_state);
        return OAL_FAIL;
    }

    if (state == cur_state) {
        /* OAM日志中不能使用%s */
        oam_warning_log2(0, OAM_SF_PWR, "FSM :trans to state %d EQUAL to current state %d,nothing to do",
            state, cur_state);
        return OAL_SUCC;
    }

    /* 调用前一状态的退出函数 */
    if (p_fsm->p_state_info[p_fsm->cur_state].oal_fsm_exit) {
        p_fsm->p_state_info[p_fsm->cur_state].oal_fsm_exit(p_fsm->p_ctx);
    }

    /* 调用本状态的进入函数 */
    if (p_fsm->p_state_info[state].oal_fsm_entry) {
        p_fsm->p_state_info[state].oal_fsm_entry(p_fsm->p_ctx);
    }

    p_fsm->prev_state = cur_state;
    p_fsm->cur_state  = state;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : oal_fsm_event_dispatch
 功能描述  : 状态机的事件处理接口
*****************************************************************************/
osal_u32 oal_fsm_event_dispatch(oal_fsm_stru* p_fsm, osal_u16 event,
    osal_u16 event_data_len, osal_void *p_event_data)
{
    osal_u32 event_handled = OAL_FALSE;

    if (p_fsm == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_PWR, "oal_fsm_event_dispatch:p_fsm = OAL_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((p_fsm->cur_state != OAL_FSM_STATE_NONE) && (p_fsm->cur_state < p_fsm->num_states)) {
        p_fsm->last_event = event;
        event_handled = (*p_fsm->p_state_info[p_fsm->cur_state].oal_fsm_event)(p_fsm->p_ctx,
            event, event_data_len, p_event_data);
    }
    if (event_handled == OAL_FAIL) {
        /* OAM日志中不能使用%s */
        oam_error_log2(0, OAM_SF_PWR, "FSM :oal_fsm_event_dispatch:event[%d] did not handled in state %d",
            event, p_fsm->p_state_info[p_fsm->cur_state].state);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

