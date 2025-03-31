 /*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: schedule algorithm
 */

#ifndef __ALG_SCHEDULE_IF_H__
#define __ALG_SCHEDULE_IF_H__

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
#include "msg_alg_rom.h"
#include "frw_msg_rom.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/******************************************************************************
  3 枚举定义
******************************************************************************/

/******************************************************************************
  4 STRUCT定义
******************************************************************************/

/******************************************************************************
  5 外部函数声明
******************************************************************************/
/* 业务调度对外接口函数 */
osal_s32 alg_schedule_init(osal_void);
osal_void alg_schedule_exit(osal_void);
osal_s32 alg_schedule_user_info_sync(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 alg_schedule_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_SCHEDULE */
#endif /* end of alg_schedule_if.h */
