/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: descriptor process.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_common_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 获取自动调频门限初始值
*****************************************************************************/
hmac_pps_value_stru hh503_get_auto_adjust_freq_pps(osal_void)
{
    hmac_pps_value_stru pps_value;

    pps_value.pps_value_0 = PPS_VALUE_0;
    pps_value.pps_value_1 = PPS_VALUE_1;
    pps_value.pps_value_2 = PPS_VALUE_2;
    return pps_value;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
