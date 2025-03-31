/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of WLAN product specification macro definition.
 * Create: 2020-10-18
 */

#ifndef WLAN_SPEC_ROM_H
#define WLAN_SPEC_ROM_H

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "wlan_spec_hh503_rom.h"
#include "wlan_spec_type_rom.h"
#include "wlan_phy_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  1.1.1 版本spec
*****************************************************************************/
#define WLAN_CHIP_VERSION_WF51V100H           0x11510101

#define WLAN_CHIP_VERSION_WF51V100L           0x11510102

/*****************************************************************************
  1.1.2 多Core对应spec
*****************************************************************************/
/* WiFi对应系统CORE的数量 为1 */
#define WLAN_FRW_MAX_NUM_CORES          1

/*****************************************************************************
  2 WLAN 宏定义
*****************************************************************************/
/*****************************************************************************
  2.1 WLAN芯片对应的spec
*****************************************************************************/
/* 每个board支持chip的最大个数，总数不会超过8个 */
#define WLAN_CHIP_MAX_NUM_PER_BOARD         1

/* 每个chip支持hal device的最大个数，总数不会超过8个 */
#define WLAN_DEVICE_MAX_NUM_PER_CHIP            1

#define WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP    1 /* 每个chip支持业务device的最大个数 */

/* 整个BOARD支持的最大的hal device数目 */
#define WLAN_DEVICE_SUPPORT_MAX_NUM_SPEC    \
    (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP)

/*  todo wg. 以下两个宏含义一致 HMAC和DMAC需要归一 */
/* 整个BOARD支持的最大的业务device数目 */
#define WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC \
    (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)

#define WLAN_SERVICE_DEVICE_MAX_NUM             1   /* 业务device数量,单device与上面宏合并 */

/* 业务AP规格 */
#define WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE      2
/* 业务STA规格 */
#define WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE     3
/* 业务ap sta共存时vap数目 */
#define WLAN_AP_STA_COEXIST_VAP_NUM             0

/* 由于最大业务VAP个数需要按照设备同时创建的最大规格初始化，目前按照最大的规格设置即可 */
#define WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE      1   /* 配置VAP个数,一个业务device一个 */
/* 业务VAP数目（4个）：STA 1个+P2P 1个+ SOFT AP 1个+1个预留扩展 */
#define WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE     3

/* 每个device支持vap的最大个数=最大业务VAP数目+配置VAP数量 */

/* 4个业务VAP + 1个配置vap */
#define WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT    \
    (WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE + WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE)

/* 整个BOARD支持的最大的VAP数目 */
#define WLAN_VAP_SUPPORT_MAX_NUM_LIMIT    \
    (WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT)

#define WLAN_VAP_NUM_PER_BOARD    WLAN_VAP_SUPPORT_MAX_NUM_LIMIT

/* 整个BOARD支持的最大业务VAP的数目 */
#define WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT    \
(WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * (WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT - WLAN_CONFIG_VAP_MAX_NUM_PER_DEVICE))

/* 以下两个成员可以定制化: WLAN_ASSOC_USER_MAX_NUM对应g_us_assoc_max_user，WLAN_ACTIVE_USER_MAX_NUM对应g_us_active_max_user */
/* 关联用户的最大个数 */
#define WLAN_ASSOC_USER_MAX_NUM                   8
#define WLAN_VAP_USER_HASH_MAX_VALUE         (WLAN_ASSOC_USER_MAX_NUM * 2)       /* 2为扩展因子 */
#define WLAN_DTCM_RAM_ASSOC_USER_MAX_NUM          8 /* DTCM_RAM中定义的关联用户最大个数，为总用户规格16的一半 */

/* 不可用的RA LUT IDX */
#define WLAN_INVALID_RA_LUT_IDX             WLAN_ACTIVE_USER_MAX_NUM

/* 以下三个用户规格，表示软件支持最大规格; 与之对应的是MAC_RES_XXX(大于等于RES_XXX)，为定制化得到，对应WLAN_ASSOC_USER_MAX_NUM为g_us_assoc_max_user */
/* 使用规则:这里三个用于oal oam hal初始化成员，或者数组下标，与之对应mac res的单播和组播，以及整board user个数会封装成函数供业务层代码调用 */
/* board最大关联用户数 = 1个CHIP支持的最大关联用户数 * board上面的CHIP数目 */
#define WLAN_ASOC_USER_MAX_NUM_LIMIT       (WLAN_ASSOC_USER_MAX_NUM * WLAN_CHIP_MAX_NUM_PER_BOARD)

/* board组播用户数 */
#define WLAN_MULTI_USER_MAX_NUM_LIMIT      (WLAN_SERVICE_VAP_SUPPORT_MAX_NUM_LIMIT)

/* board最大用户数 = 最大关联用户数 + 组播用户个数 */
#define WLAN_USER_MAX_USER_LIMIT           (WLAN_ASOC_USER_MAX_NUM_LIMIT + WLAN_MULTI_USER_MAX_NUM_LIMIT)

/*****************************************************************************
  2.2 WLAN协议对应的spec
*****************************************************************************/
/* 速率功率表各协议的档位数量 */
#define WLAN_POW_11B_RATE_NUM  4  /* 11b速率数目 */
#define WLAN_POW_LEGACY_RATE_NUM 8  /* 11a/g速率数目 */
#define WLAN_POW_NONLEGACY_20M_NUM  12 /* 11n_11ac_11ax_20M速率数目最大值 */
#define WLAN_POW_NONLEGACY_40M_NUM  13 /* 11n_11ac_11ax_40M速率数目最大值+mcs32 */
// 速率功率表的表长
#define WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN (WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM + \
    WLAN_POW_NONLEGACY_20M_NUM + WLAN_POW_NONLEGACY_40M_NUM)
// 速率功率表的协议速率偏移
/* 11b 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_11B_OFFSET 0
/* 11a/g 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_LEGACY_OFFSET WLAN_POW_RATE_POW_CODE_11B_OFFSET
/* 11n_11ac_11ax_20M 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_NONLEGACY_20M_OFFSET (WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM)
/* 11n_11ac_11ax_40M 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_NONLEGACY_40M_OFFSET \
        (WLAN_POW_RATE_POW_CODE_NONLEGACY_20M_OFFSET + WLAN_POW_NONLEGACY_20M_NUM)
/* 11n_11ac_11ax_80M 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_NONLEGACY_80M_OFFSET WLAN_POW_RATE_POW_CODE_INVALID_OFFSET
/* 11n_11ac_11ax_160M 速率索引偏移 */
#define WLAN_POW_RATE_POW_CODE_NONLEGACY_160M_OFFSET WLAN_POW_RATE_POW_CODE_INVALID_OFFSET
/*****************************************************************************
  2.3 oam相关的spec
*****************************************************************************/
#define WLAN_OAM_FILE_PATH      "\\home\\oam.log"    /* LINUX和裸系统下,LOG文件默认的保存位置 */

/*****************************************************************************
  2.4 mem对应的spec
*****************************************************************************/
/*****************************************************************************
  2.4.1 内存池规格
*****************************************************************************/
/* 3個VAP，每個VAP提供472字節長度的存放地址 */
#define WLAN_TXBFEE_BUFF_SIZE_PER_VAP           472
#define WLAN_TXBFEE_BUFF_SIZE                   (WLAN_TXBFEE_BUFF_SIZE_PER_VAP * WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE)
#define WLAN_MEM_MAX_USERS_NUM              4             /* 共享同一块内存的最大用户数 */
/*****************************************************************************
  2.4.2 共享描述符内存池配置信息
*****************************************************************************/
#define WLAN_MAX_NETBUF_SIZE        (WLAN_LARGE_NETBUF_SIZE + OAL_MAX_MAC_HDR_LEN)  /* 最大帧长:帧头+payload */
/* 整个device所有TID的最大MPDU数量限制
    需要重新定义，以支持调度逻辑
*/
#define WLAN_TID_MPDU_NUM_BIT               9
#define WLAN_TID_MPDU_NUM_LIMIT             (1 << WLAN_TID_MPDU_NUM_BIT)

// WLAN_MEM_NETBUF_CNT1由_PRE_WLAN_FEATURE_11K特性解耦
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define WLAN_MEM_NETBUF_CNT1                16
#else
#define WLAN_MEM_NETBUF_CNT1                192     /* 接收数据帧是AMSDU，其中的每个MSDU对应一个克隆netbuf */
#endif
#define WLAN_MEM_SHARED_RX_DSCR_SIZE        68
#define WLAN_MEM_NETBUF_SIZE2               1600
#define WLAN_LARGE_NETBUF_SIZE_TMP          1200
#define WLAN_HMEM_EVENT_SIZE2               528       /* 注意: 事件内存长度包括4字节IPC头长度 */
#define WLAN_HMEM_LOCAL_SIZE2               144

#define WLAN_WPS_IE_MAX_SIZE                (WLAN_HMEM_EVENT_SIZE2 - 128)

/*****************************************************************************
  2.4.10 RF通道数规格已放入WLAN_SPEC
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DAQ
#define WLAN_MEM_MAC_TEST_INFO_SIZE          64 * 1024
#endif


/*****************************************************************************
  2.4.11 TCP ACK优化
*****************************************************************************/
/* tcp ack过滤功能不再默认打开，根据流量调整 */
#define DEFAULT_TX_TCP_ACK_OPT_ENABLE (OAL_FALSE)
#define DEFAULT_RX_TCP_ACK_OPT_ENABLE (OAL_FALSE)
#define DEFAULT_TX_TCP_ACK_THRESHOLD (1) /* 丢弃发送ack 的门限 */
#define DEFAULT_RX_TCP_ACK_THRESHOLD (1) /* 丢弃接收ack 的门限 */
/*****************************************************************************
  2.5 frw相关的spec
*****************************************************************************/
/*****************************************************************************
  2.6 alg相关的spec
*****************************************************************************/
#define ALG_TRAFFIC_CTL_ENABLE_DEFAULT                OSAL_TRUE
#define ALG_TRAFFIC_CTL_RX_ENABLE_DEFAULT             OSAL_TRUE
#define ALG_TRAFFIC_CTL_TX_ENABLE_DEFAULT             OSAL_TRUE

/* 接收方向大包队列阈值 */
#define ALG_TRAFFIC_CTL_RX_PKT_LOW_THRES              2  /* > 5Mbps */
#define ALG_TRAFFIC_CTL_RX_PKT_HIGH_THRES             84  /* > 10Mbps */
#define ALG_TRAFFIC_CTL_RX_PKT_BUSY_THRES             168 /* > 20Mbps */
#define ALG_TRAFFIC_CTL_NETBUF_WINDOW_SIZE            2  /* 回滞区间 */
#define ALG_TRAFFIC_CTL_NETBUF_RESERVED_SIZE          0  /* TX方向预留内存设置 */

#define ALG_TRAFFIC_CTL_RX_DSCR_DEFAULT_THRES         WLAN_MEM_NORMAL_RX_MAX_CNT
#define ALG_TRAFFIC_CTL_RX_DSCR_MIN_THRES             2  /* 调低阈值，至少满足一次中断收包所需要的描述符个数即可 */
#define ALG_TRAFFIC_CTL_RX_DSCR_LOW_THRES             12
#define ALG_TRAFFIC_CTL_RX_DSCR_HIGH_THRES            14
#define ALG_TRAFFIC_CTL_RX_DSCR_BUSY_THRES            16
#define ALG_TRAFFIC_CTL_RX_DSCR_BUSY_THRES_LEGACY     16

/* 接收方向小包队列阈值 */
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_LOW_THRES      8
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_HIGH_THRES     72
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_BUSY_THRES     100

#define ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_DEFAULT_THRES WLAN_MEM_SMALL_RX_MAX_CNT
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_MIN_THRES     14  /* 需要至少满足一次中断收包所需要的描述符个数 */
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_LOW_THRES     16
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_HIGH_THRES    18
#define ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_BUSY_THRES    22
/******************************************************************************
    事件队列配置信息表
    注意: 每个队列所能容纳的最大事件个数必须是2的整数次幂
*******************************************************************************/
/* host event cfg table */
#define WLAN_ACTIVE_USER_MAX_NUM                  8    /* 活跃用户的最大个数 */

/* 活跃用户索引位图长度 */
#define WLAN_ACTIVE_USER_IDX_BMAP_LEN      ((WLAN_ACTIVE_USER_MAX_NUM + 7)>> 3)

/*****************************************************************************
  2.8.2 STA AP规格
*****************************************************************************/
/* 通道数 */
#define WLAN_RF_CHANNEL_NUMS   1
/* RF PLL个数 */
#define WLAN_RF_PLL_NUMS   1

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of file */

