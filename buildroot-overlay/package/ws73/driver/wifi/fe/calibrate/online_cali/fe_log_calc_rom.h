/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 前端log计算的查找表
 * Create: 2022-10-20
 */

#ifndef __FE_LOG_CALC_ROM_H__
#define __FE_LOG_CALC_ROM_H__

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32 fe_calc_20log(osal_s32 input_val);
// 得到20log计算表的元素个数
osal_u32 fe_20log_table_len(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif