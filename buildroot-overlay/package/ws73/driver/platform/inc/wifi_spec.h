/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: wifi spec header file
 * Author: Huanghe
 * Create: 2020-10-13
 */

#ifndef __WIFI_SPEC_H__
#define __WIFI_SPEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"

#define WLAN_CHIP_MAX_NUM_PER_BOARD         1   /* 每个board支持chip的最大个数，总数不会超过8个 */

/* 每个chip支持hal device的最大个数，总数不会超过8个,03应该为2,目前硬件不支持 */
#define WLAN_DEVICE_MAX_NUM_PER_CHIP            1

#define WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP    1                     /* 每个chip支持业务device的最大个数 */

/* 整个BOARD支持的最大的业务device数目 */
#define WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC  (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)

/* 整个BOARD支持的最大的hal device数目 */
#define WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC    (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP)

#define WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE      2 /* 业务AP的规格，将之前的WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE修改 */

#define WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE      1 /* 配置VAP个数,一个业务device一个 */
#define WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE     3 /* 业务VAP个数(原本应该用AP个数+STA个数（这个表示芯片最大支持能力 */

#define WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT    (WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE + WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE)

/* 整个BOARD支持的最大的VAP数目 */
#define WLAN_VAP_SUPPORT_MAX_NUM_LIMIT   (WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT)

#define WLAN_CHIP_MAX_NUM_PER_BOARD         1                       /* 每个board支持chip的最大个数，总数不会超过8个 */

/* 每个chip支持hal device的最大个数，总数不会超过8个,03应该为2,目前硬件不支持 */
#define WLAN_DEVICE_MAX_NUM_PER_CHIP            1

#define WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP    1                     /* 每个chip支持业务device的最大个数 */

/* 整个BOARD支持的最大的hal device数目 */
#define WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC    (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP)

/* board组播用户数 */
#define WLAN_MULTI_USER_MAX_NUM_LIMIT      (WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT)

/* 整个BOARD支持的最大业务VAP的数目 */
#define WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT (WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * \
                        (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT - WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE))

#define WLAN_ASSOC_USER_MAX_NUM                    16  /* 关联用户的最大个数 */

/* 活跃用户的最大个数 */
#define WLAN_ACTIVE_USER_MAX_NUM                   16
#define WLAN_ASOC_USER_MAX_NUM_LIMIT       (WLAN_ASSOC_USER_MAX_NUM * WLAN_CHIP_MAX_NUM_PER_BOARD)

/* board最大用户数 = 最大关联用户数 + 组播用户个数 */
#define WLAN_USER_MAX_USER_LIMIT           (WLAN_ASOC_USER_MAX_NUM_LIMIT + WLAN_MULTI_USER_MAX_NUM_LIMIT)

#define WLAN_MEM_MAX_SUBPOOL_NUM            8             /* 内存池中最大子内存池个数 */

#define WLAN_CHIP_VERSION_WF51           0x11510101

#define WLAN_MEM_NETBUF_SIZE2         1600
#define WLAN_LARGE_NETBUF_SIZE        WLAN_MEM_NETBUF_SIZE2   /* NETBUF内存池长帧的长度，统一用这个宏 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifndef __WIFI_SPEC_H__ */

