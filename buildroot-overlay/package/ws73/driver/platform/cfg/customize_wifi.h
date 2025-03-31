/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Description: header file.
 * Author: CompanyName
 * Create: 2023-01-01
 */

#ifndef __CUSTOMIZE_WIFI_H__
#define __CUSTOMIZE_WIFI_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "customize.h"
#include "oal_types.h"
#include "ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PRODUCT_ID_MP0X_HOST)
// #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define CUS_TAG_INI                     0x11
#define CUS_TAG_NV                      0x13
#define CUS_TAG_PRIV_INI                0x14

#define CUS_PARAMS_LEN_MAX              (104)   /* 定制项最大长度 */
#define WLAN_CMU_XO_TRIM_NUM            2    /* 频偏寄存器校正码值个数 */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 定制化 INI CONFIG ID */
typedef enum {
    /* 性能 */
    WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT,
    WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT,
    /* 平台低功耗 */
    WLAN_CFG_INIT_DEVICE_PLT_PM_ENABLE, // device侧平台低功耗使能
    WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_IDX, // device唤醒host的GPIO号
    WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_LEVEL, // device唤醒host的GPIO电平
    WLAN_PLAT_CFG_INIT_BUTT,
} plat_cfg_init;
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    char   *name;
    int     case_entry;
} plat_cfg_cmd;

extern osal_u32 ini_get_cmu_xo_trim(osal_void);
extern osal_void hwifi_plat_pm_device_init_custom_param(osal_void);
extern osal_void hwifi_config_cmu_xo_trim_temp_comp(osal_void);
// #endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#endif // #if defined(_PRE_PRODUCT_ID_MP0X_HOST)


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif // customize_wifi.h

