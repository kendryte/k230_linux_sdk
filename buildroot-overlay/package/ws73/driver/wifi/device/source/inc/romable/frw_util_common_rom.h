/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf enum type head file.
 * Create: 2022-07-29
 */

#ifndef FRW_UTIL_COMMON_ROM_H
#define FRW_UTIL_COMMON_ROM_H

#include "osal_types.h"
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/* 32位寄存器最大长度 */
#define EXT_TIME_US_MAX_LEN (0xFFFFFFFF - 1)
#define USEC_TO_MSEC        1000                 /* s -> ms -> us 转换 */

typedef struct {
    osal_slong i_sec;
    osal_slong i_usec;
} osal_time_us_stru;

/* 寄存器反转模块运行时间计算 */
#define osal_time_calc_runtime(_start, _end) (((EXT_TIME_US_MAX_LEN) - (_start)) + (_end))

/* 获取从_ul_start到_ul_end的时间差 */
#define osal_get_runtime(_ul_start, _ul_end) \
    (((_ul_start) > (_ul_end)) ? (osal_time_calc_runtime((_ul_start), (_ul_end))) : ((_ul_end) - (_ul_start)))

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif /* end of frw_util_common_rom.h */