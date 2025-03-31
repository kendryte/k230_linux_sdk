/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file for rom_cb.c.
 */

#ifndef __ROM_CB_H__
#define __ROM_CB_H__

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
 2 枚举定义
*****************************************************************************/
typedef enum {
    ROM_CB_START = 0,
    ROM_CB_PLAT_DFR_FOR_PM = ROM_CB_START,
    HCC_TX_DATA,
    HCC_TX_BT_DATA,
    HCC_RX_ENQ,
    HCC_RX_PROC,
    HCC_RX_PROC_TIME,
    PM_FSM_SVC_VOTE,
    PM_FSM_PLT_VOTE,
    PM_CRG_SWITCH_FREQ,
    PM_CTRL_CHECK_HOST_SLEEP,
    PM_CTRL_AWAKE_HOST,
    ROM_CB_DBG_MSG_EN,
    ROM_CB_END
}rom_cb_enum;

/*****************************************************************************
  3 函数声明
*****************************************************************************/
osal_void rom_cb_register(rom_cb_enum id, osal_void *func_cb);
osal_void *rom_cb_get(rom_cb_enum func_id);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif

