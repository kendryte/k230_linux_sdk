/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_data_acq.c
 * 生成日期   : 2014年4月25日
 * 功能描述   : hmac数据采集功能
 */


#ifdef _PRE_WLAN_FEATURE_DAQ

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_data_acq.h"
#include "wlan_spec.h"
#include "mac_resource_ext.h"
#include "mac_device_ext.h"
#include "hmac_config.h"
#include "hmac_vap.h"
#include "hmac_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DATA_ACQ_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u8 g_uc_data_acq_used = OAL_FALSE;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_data_acq_init
 功能描述  : 数据采集功能Hmac初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC
*****************************************************************************/
osal_void  hmac_data_acq_init(osal_void)
{
    g_uc_data_acq_used = OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_data_acq_exit
 功能描述  : 数据采集功能Hmac退出
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC
*****************************************************************************/
osal_void  hmac_data_acq_exit(osal_void)
{
    g_uc_data_acq_used = OAL_FALSE;
}

osal_u8 hmac_get_data_acq_used(osal_void)
{
    return g_uc_data_acq_used;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /* end of _PRE_WLAN_FEATURE_DAQ */

