/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 累加器相关公共处理
 * Create: 2022-09-15
 */

#ifndef __CALI_ACCUM_DATA_H__
#define __CALI_ACCUM_DATA_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 接收累加通路定义 */
#define HH503_CALI_ACCUM_INDEX_I 0x01
#define HH503_CALI_ACCUM_INDEX_Q 0x02

/* 校准算法的累加参数配置 */
#define HAL_CALI_ACCUM_TIMES_MIN_SHIFT 8    /* 最小累加值 2^8 对应配置寄存器cfg_fb_pwr_calc_prd:0 */
/* 维测累加上报打印 */
#define HH503_CALI_ACCUM_TIMES_DEBUG_SHIFT 8
#define HH503_CALI_ACCUM_TIMES_DEBUG_CONFIG (HH503_CALI_ACCUM_TIMES_DEBUG_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)
/* rx dc 累加上报参考值 配合代码中定点化运算 */
#define HH503_CALI_RXDC_ACCUM_TIMES_SHIFT 8
#define HH503_CALI_RXDC_ACCUM_TIMES_CONFIG (HH503_CALI_RXDC_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)
#define HH503_CALI_TXDC_ACCUM_TIMES_SHIFT 9
#define HH503_CALI_TXDC_ACCUM_TIMES_CONFIG (HH503_CALI_TXDC_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)

#define HH503_CALI_TX_POWER_ACCUM_TIMES_SHIFT 8
#define HH503_CALI_TX_POWER_ACCUM_TIMES_CONFIG (HH503_CALI_TX_POWER_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)

#define HAL_CALI_RC_ACCUM_TIMES_SHIFT 8
#define HAL_CALI_RC_ACCUM_TIMES_CONFIG (HAL_CALI_RC_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)

#define HAL_CALI_LOGEN_ACCUM_TIMES_SHIFT  8
#define HAL_CALI_LOGEN_ACCUM_TIMES_CONFIG (HAL_CALI_LOGEN_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)
#define HAL_CALI_LOGEN_ACCUM_DELAY        0

#define HH503_CALI_ACCUM_LEN_AGC 31
#define HH503_CALI_ACCUM_TIMES_AGC 0
/* 超时宏定义 */
#define HH503_CALI_ACCUM_WAIT_TIMEOUT 1000

/* 硬件常发累加参数 */
#define HAL_CALI_HW_ALTX_ACCUM_DELAY        (0x00200)
#define HAL_CALI_HW_ALTX_ACCUM_TIMES_SHIFT  (0xB)
#define HAL_CALI_HW_ALTX_ACCUM_TIMES_CONFIG (HAL_CALI_HW_ALTX_ACCUM_TIMES_SHIFT - HAL_CALI_ACCUM_TIMES_MIN_SHIFT)

/* 累加器配置参数 */
typedef struct {
    osal_u8 accum_times;       /* 累加次数 范围[0,12],表示2^8-2^20点 */
    osal_u8 accum_times_shift; /* 累加次数偏移 */
    osal_u16 rsv;
    osal_u32 accum_delay;      /* 累加器延时值 */
} cali_accum_para_stru;

osal_void cali_print_accum_data(const hal_device_stru *device);
osal_void cali_report_one_accum(const hal_device_stru *device, cali_accum_para_stru *acc_param);
osal_void cali_sum_accum_data(const hal_device_stru *device, osal_s32 *acc_i, osal_s32 *acc_q);
osal_s32 cali_get_acc_val_i(osal_void);
osal_s32 cali_get_acc_val_q(osal_void);
osal_void cali_logen_get_accum_para(cali_accum_para_stru *accum_param);
osal_void cali_txpwr_get_accum_para(cali_accum_para_stru *accum_param);
osal_void cali_rc_get_accum_para(cali_accum_para_stru *accum_param);
osal_void cali_hw_al_tx_report_one_accum(const hal_device_stru *device, cali_accum_para_stru *acc_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
