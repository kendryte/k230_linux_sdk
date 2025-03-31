/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: memory config
 * Create: 2021-11-22
 */
#ifndef __OAL_MEM_POOL_H__
#define __OAL_MEM_POOL_H__

#ifdef BUILD_UT
#include "wifi_ut_stub.h"
#endif

#ifdef BUILD_UT
#else
#ifdef _PRE_HIGH_SPEED_MODE
#define OAL_PKTRAM_ADDR_END             0x664000    // 144k
#else
#define OAL_PKTRAM_ADDR_END             0x658000    // 96k
#endif
#define OAL_PKTRAM_ADDR_BEGIN           0x640000
#endif
#define OAL_PKTRAM_CALI_RSV             (64 * 1024) // 64: RAM2 + RAM3

/* 预先申请全局变量长度, 73本地内存使用osal接口申请 */
#define G_OAL_MEM_TOTAL_SIZE            0
#define USE_OAL_MEM_TOTAL_ADDR          1

/* 各池起始地址 */
#define OAL_MEM_ADDR_OAM_SDT            ((uintptr_t)NULL)        /* 不分配空间 */
#define OAL_MEM_ADDR_TXBFEB_PKT         ((uintptr_t)NULL)
#define OAL_MEM_ADDR_TXBFEE_BUFF        (OAL_PKTRAM_ADDR_BEGIN)  /* pktram */
#define OAL_MEM_ADDR_PROTECT            (OAL_MEM_ADDR_TXBFEE_BUFF + WLAN_TXBFEE_BUFF_SIZE)
#define OAL_MEM_ADDR_COEX               (OAL_MEM_ADDR_PROTECT + WLAN_PROTECT_MAX_LEN)
#define OAL_MEM_ADDR_BEACON_PKT         (OAL_MEM_ADDR_COEX + WLAN_COEX_PKT_LEN)
#define OAL_MEM_ADDR_NETBUF             (OAL_MEM_ADDR_BEACON_PKT + WLAN_BEACON_PKT_LEN)
#define OAL_MEM_ADDR_TX_DSCR            ((uintptr_t)NULL)
#define OAL_MEM_ADDR_LOCAL              ((uintptr_t)NULL) /* 全局变量 */
#define OAL_MEM_ADDR_MIB                ((uintptr_t)NULL)

#define OAL_NETBUF_HIGH_PRIORITY_COUNT  5

/* 描述符个数的宏定义: (SMALL + NORMAL + HI RX-Q) <= WLAN_MEM_RX_DSCR_CNT */
#define WLAN_MEM_RX_DSCR_SIZE           64           /* 等于接收描述符结构体,13 * 4 + 12 = 64 */
#define WLAN_MEM_TX_DSCR_SIZE           92           /* 等于发送描述符结构体 19 * 4 + 16 = 92 */

#define WLAN_MEM_RX_DSCR_CNT            0            /* 包括小包，大包，管理帧 */
#define WLAN_MEM_TX_DSCR_CNT            0
#define WLAN_MEM_SMALL_RX_MAX_CNT       8
#define WLAN_MEM_NORMAL_RX_MAX_CNT      12
#define WLAN_MEM_HIGH_RX_MAX_CNT        8

/* 本地变量内存池配置信息70k */
#define WLAN_MEM_LOCAL_SIZE1            12
#define WLAN_MEM_LOCAL_CNT1             128             /* TID 和HAL VAP */

#define WLAN_MEM_LOCAL_SIZE2            64
#define WLAN_MEM_LOCAL_CNT2             176             /* 200(杂用) */

#define WLAN_MEM_LOCAL_SIZE3            264
#define WLAN_MEM_LOCAL_CNT3             25

#define WLAN_MEM_LOCAL_SIZE4            512             /* 长度为128的事件队列用 */
#define WLAN_MEM_LOCAL_CNT4             4

#define WLAN_MEM_LOCAL_SIZE5            608             /* PNO扫描使用  hash桶初始化使用1 */
#define WLAN_MEM_LOCAL_CNT5             8               /* wps ie 增加1片, 实测SoftAp场景又有6次申请该内存，共8 */

#define WLAN_MEM_LOCAL_SIZE6            1024
#define WLAN_MEM_LOCAL_CNT6             0               /* dmac_reset_lut_init 使用1 */

#define WLAN_MEM_LOCAL_SIZE7            1528            /* dmac_user_stru 使用 */
#define WLAN_MEM_LOCAL_CNT7             ((DMAC_WLAN_DTCM_RAM_ASSOC_USER_MAX_NUM) + (DMAC_WLAN_MULTI_USER_MAX_NUM))

#define WLAN_MEM_LOCAL_SIZE8            4100          /* alg 使用、 msg初始化分配内存使用，msg id rom化由3260--->3800 */
#define WLAN_MEM_LOCAL_CNT8             DMAC_WLAN_DTCM_RAM_ASSOC_USER_MAX_NUM

/* MIB内存池 */
#define WLAN_MEM_MIB_SIZE1              512             /* mib结构体大小 */
#define WLAN_MEM_MIB_CNT1               0               /* 配置VAP没有MIB DMAC_WLAN_SERVICE_VAP_SUPPORT_MAX_NUM */

/* netbuf内存池 */
#define WLAN_SHORT_NETBUF_SIZE          180             /* 短帧长度 payload:180 */
#define WLAN_SMGMT_NETBUF_SIZE          400             /* 短管理帧长度 payload:400 */
#define WLAN_MGMT_NETBUF_SIZE           768             /* 管理帧长度 payload:768 */
#define WLAN_LARGE_NETBUF_SIZE          1600            /* 长帧长度 payload:1600 */
#define WLAN_SHORT_NETBUF_CNT           22              /* 短帧netbufpayload个数 */
#define WLAN_SMGMT_NETBUF_CNT           0               /* 短管理帧netbufpayload个数 */
#define WLAN_MGMT_NETBUF_CNT            12              /* 管理帧netbufpayload个数 */
#ifdef _PRE_HIGH_SPEED_MODE
#define WLAN_LARGE_NETBUF_CNT           72              /* 长帧netbufpayload个数 */
#else
#define WLAN_LARGE_NETBUF_CNT           43              /* 长帧netbufpayload个数 */
#endif

#endif /* end of oal_mem_pool.h */
