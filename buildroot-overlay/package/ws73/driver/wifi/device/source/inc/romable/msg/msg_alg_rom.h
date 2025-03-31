 /*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: msg algorithm
 * Create: 2020-7-6
 */
#ifndef MSG_ALG_ROM_H
#define MSG_ALG_ROM_H

#include "osal_types.h"

/* 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_enum_uint16 alg_cfg; /* 配置命令枚举 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_u8 is_negtive; /* 配置参数值是否为负 */
    osal_u8 resv[1];    /* 字节对齐 */
#else
    osal_u8 resv[2]; /* 字节对齐 */
#endif
    osal_u32 value; /* 配置参数值 */
} mac_ioctl_alg_param_stru;

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
/* 干扰场景优化配置 */
typedef struct {
    mac_alg_intrf_mode_cfg_enum_uint8 alg_intrf_mode_cfg; /* 配置命令枚举 */
    osal_u8 resv;                                         /* 字节对齐 */
    osal_u16 value;                                       /* 配置参数值 */
} mac_ioctl_alg_intrf_mode_param_stru;
typedef struct {
    osal_u8 enable; /* 配置命令枚举 */
    osal_u8 resv;                                         /* 字节对齐 */
    osal_u16 value;                                       /* 配置参数值 */
} mac_ioctl_alg_intrf_mode_todmac_stru;
#endif

#endif