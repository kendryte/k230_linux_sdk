/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_mfg
 * Create: 2023-05-20
 */

#ifndef HMAC_MFG_H
#define HMAC_MFG_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "wlan_types.h"
#include "hmac_feature_interface.h"
#include "hal_mfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WLAN_CFG_MAX_PARAM_NUM 10 /* 产测保存数据最大长度 */

#define WLAN_CFG_MIN_RSSI_OFFSET (-16) /* rssi偏移最小值 */
#define WLAN_CFG_MAX_RSSI_OFFSET 15 /* rssi偏移最大值 */
#define WLAN_CFG_MIN_CHANNEL_NUM 1
#define WLAN_CFG_MAX_CHANNEL_NUM 14

#define WLAN_CFG_ONE_POWER_PARAM_NUM 8       /* 功率校准参数个数 */
#define WLAN_CFG_ONE_CURVE_FACTOR_NUM 3      /* 单条曲线放大系数个数 */
#define WLAN_CFG_CURVE_FACTOR_NUM 6          /* 放大系数个数 */
#define WLAN_CFG_POWER_PARAM_NUM 9           /* 功率偏移值个数 */
#define WLAN_CFG_MAX_POWER_PARAM_NUM 9       /* 曲线系数个数 */
#define WLAN_CFG_TES_POWER_NUM 2             /* 设置目标功率个数 */
#define WLAN_CFG_MAX_TAR_POWER 230           /* 设置目标功率最大值 */
#define WLAN_CFG_MIN_TAR_POWER 100           /* 设置目标功率最小值 */
#define WLAN_CFG_MAX_CALI_POWER 300          /* 设置实际功率最大值 */
#define WLAN_CFG_MIN_CALI_POWER 0            /* 设置实际功率最小值 */
#define WLAN_CFG_CURVE_FACTOR_MIN 0          /* 放大系数最小值 */
#define WLAN_CFG_CURVE_FACTOR_MAX 31         /* 放大系数最大值 */
#define WLAN_CFG_CONSTANT_OFFSET_MIN (-32758)  /* 系数c的偏移值最小值 */
#define WLAN_CFG_CONSTANT_OFFSET_MAX 32767   /* 系数c的偏移值最大值 */
#define WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN 0 /* 设置高功率段系数c的偏移值 */
#define WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN 1  /* 设置低功率段系数c的偏移值 */

#define WLAN_CFG_COARSE_MIN_NUM 0
#define WLAN_CFG_COARSE_MAX_NUM 15
#define WLAN_CFG_FINE_MIN_NUM 0
#define WLAN_CFG_FINE_MAX_NUM 127
#define WLAN_CFG_TEMPER_MIN_LEVEL (-40)      /* 最低温度温度 */
#define WLAN_CFG_TEMPER_MAX_LEVEL 120        /* 最高温度 */
#define WLAN_CFG_CONSTANT_OFFSET_NUM 3

#define SIZE_8_BITS 8
#define RSSI_OFFSET_SIZE 3
#define TEST_PARAM_NUM 3

typedef enum {
    MAC_CURVE_FACTOR_DEFAULT,
    MAC_HIGH_CURVE_FACTOR,
    MAC_LOW_CURVE_FACTOR,
    MAC_CURVE_FACTOR_BUTT
} mfg_power_curve_factor;

typedef struct {
    osal_s32 param[WLAN_CFG_MAX_PARAM_NUM];
    osal_s32 output_param[WLAN_CFG_MAX_PARAM_NUM];
    osal_u32 output_param_num;
} mfg_param_stru;

/*****************************************************************************
  2 函数声明
*****************************************************************************/

static osal_u32 hmac_mfg_init_weakref(osal_void) __attribute__ ((weakref("hmac_mfg_init"), used));
static osal_void hmac_mfg_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_mfg_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mfg.h */
