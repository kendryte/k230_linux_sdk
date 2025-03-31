/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file of WLAN product specification macro definition.
 * Create: 2022-11-26
 */

#ifndef __WLAN_SPEC_HH503_H__
#define __WLAN_SPEC_HH503_H__

#include "wlan_spec_hh503_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  0.1.2 热点入网功能
*****************************************************************************/
/* 作为P2P GO 允许关联最大用户数 */
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC 4

/*****************************************************************************
  2 宏定义，分类和DR保持一致
*****************************************************************************/
/*****************************************************************************
  2.1 基础协议/定义物理层协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.1.1 扫描侧STA 功能
*****************************************************************************/
#define WLAN_SCAN_REQ_MAX_BSSID                 2
#define WLAN_SCAN_REQ_MAX_SSID                  8

#define WLAN_PROBE_DELAY_TIME                   10
#define WLAN_MAX_SCAN_BSS_PER_CH                8

#define WLAN_DEFAULT_FG_SCAN_COUNT_PER_CHANNEL         2       /* 前景扫描每信道扫描次数 */
#define WLAN_DEFAULT_BG_SCAN_COUNT_PER_CHANNEL         1       /* 背景扫描每信道扫描次数 */
#define WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL  1       /* 每次信道扫描发送probe req帧的次数 */

#define WLAN_DEFAULT_MAX_TIME_PER_SCAN                 (3 * 1500)  /* 扫描的默认的最大执行时间，超过此时间，做超时处理 */

#define WLAN_DEFAULT_ACTIVE_SCAN_TIME           20
#define WLAN_DEFAULT_PASSIVE_SCAN_TIME          60

#define WLAN_LONG_ACTIVE_SCAN_TIME              40             /* 指定SSID扫描个数超过3个时,1次扫描超时时间为40ms */

/*****************************************************************************
  2.1.1 STA入网功能
*****************************************************************************/
#define WLAN_AUTH_TIMEOUT                       500
#define WLAN_ASSOC_TIMEOUT                      600
#define WLAN_JOIN_START_TIMEOUT                 10000

/*****************************************************************************
  2.1.2 热点入网功能
*****************************************************************************/
/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration.
 */
#define WLAN_AID_MAX                        2007
#define WLAN_AID_DEFAULT                    128

/* 活跃定时器触发周期 */
#define WLAN_USER_ACTIVE_TRIGGER_TIME           1000
/* 老化定时器触发周期 */
#define WLAN_USER_AGING_TRIGGER_TIME            5000
/* 单位ms */
#define WLAN_USER_ACTIVE_TO_INACTIVE_TIME       5000

#define WLAN_AP_KEEPALIVE_INTERVAL              (55 * 1000)     /* ap发送keepalive null帧间隔 */
#define WLAN_GO_KEEPALIVE_INTERVAL              (25 * 1000)     /* P2P GO发送keepalive null帧间隔  */

/*****************************************************************************
  2.1.3 STA断网功能
*****************************************************************************/
#define WLAN_LINKLOSS_OFFSET_11H                5  /* 切信道时的延迟 */

/*****************************************************************************
  2.1.6 保护模式功能
*****************************************************************************/
#define WLAN_RTS_DEFAULT                    512
#define WLAN_RTS_MIN                        1

/*****************************************************************************
  2.1.7 分片功能
*****************************************************************************/
/* Fragmentation limits */
/* default frag threshold */
#define WLAN_FRAG_THRESHOLD_DEFAULT         2346 /* 默认配置为最大分片长度，减少分片 */
/* min frag threshold */
#define WLAN_FRAG_THRESHOLD_MIN             256  /* 为了保证分片数小于16: (1472(下发最大长度)/16)+36(数据帧最大帧头) = 128 */
/* max frag threshold */
#define WLAN_FRAG_THRESHOLD_MAX             2346
/*****************************************************************************
  2.1.14 数据速率功能
*****************************************************************************/
/* 速率相关参数,记录支持的速率 */
#define WLAN_SUPP_RATES                         8

#define HAL_TX_RATE_NUM_1                  1
#define HAL_TX_RATE_NUM_2                  2
#define HAL_TX_RATE_NUM_3                  3

/* 每级速率最大的发送次数 */
#define HAL_MAX_TX_COUNT                   7
/*****************************************************************************
  2.2 其他协议/定义MAC 层协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.2.8 国家码功能
*****************************************************************************/
/* 管制类最大个数 */
#define WLAN_MAX_RC_NUM                         20
/* 管制类位图字数 */
#define WLAN_RC_BMAP_WORDS                      2

/*****************************************************************************
  2.2.9 WMM功能
*****************************************************************************/
/* EDCA参数 */
/* STA所用WLAN_EDCA_XXX参数同WLAN_QEDCA_XXX */
#define WLAN_QEDCA_TABLE_INDEX_MIN           1
#define WLAN_QEDCA_TABLE_INDEX_MAX           4
#define WLAN_QEDCA_TABLE_CWMIN_MIN           0
#define WLAN_QEDCA_TABLE_CWMIN_MAX           10
#define WLAN_QEDCA_TABLE_CWMAX_MIN           0
#define WLAN_QEDCA_TABLE_CWMAX_MAX           10
#define WLAN_QEDCA_TABLE_AIFSN_MIN           0
#define WLAN_QEDCA_TABLE_AIFSN_MAX           15
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN      0
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX      65535
#define WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX   500

/* 存储硬件接收上报的描述符链表数目(ping pong使用) */
#define HAL_HW_RX_DSCR_LIST_NUM         2

/*****************************************************************************
  2.2.10 协议节能STA侧功能
*****************************************************************************/
/* DTIM Period参数 */
/* beacon interval的倍数 */
#define WLAN_DTIM_PERIOD_MAX                255
#define WLAN_DTIM_PERIOD_MIN                1

/*****************************************************************************
  2.3 校准类别的spec
*****************************************************************************/
/*****************************************************************************
  2.4 安全协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.4.7 PMF STA功能
*****************************************************************************/
/* SA Query流程间隔时间,老化时间的三分之一 */
#define WLAN_SA_QUERY_RETRY_TIME                201

/* SA Query流程超时时间,小于老化时间 */
#define WLAN_SA_QUERY_MAXIMUM_TIME              1000

/*****************************************************************************
  2.4.9 WPA功能
*****************************************************************************/
/* 加密相关的宏定义 */
/*****************************************************************************
  2.5 性能类别的spec
*****************************************************************************/
/*****************************************************************************
  2.5.1 块确认功能
*****************************************************************************/
#define WLAN_ADDBA_TIMEOUT                      500
#define WLAN_TX_PROT_TIMEOUT                    6000

/* 支持的建立rx ba 的最大个数 */
#define WLAN_MAX_RX_BA                      16

/* 支持的建立tx ba 的最大个数 */
#define WLAN_MAX_TX_BA                      16

/*****************************************************************************
  2.5.2 AMPDU功能
*****************************************************************************/
#define WLAN_AMPDU_RX_HE_BUFFER_SIZE    64 /* HE AMPDU接收端接收缓冲区的buffer size的大小 */
#define WLAN_AMPDU_RX_BUFFER_SIZE       64  /* AMPDU接收端接收缓冲区的buffer size的大小 */
#define WLAN_AMPDU_RX_BA_LUT_WSIZE      64  /* AMPDU接收端用于填写BA RX LUT表的win size,
                                               要求大于等于WLAN_AMPDU_RX_BUFFER_SIZE */
#define WLAN_AMPDU_TX_MAX_NUM_ROM       16
#define MAC_RX_BA_LUT_BMAP_LEN             ((HAL_MAX_RX_BA_LUT_SIZE + 7) >> 3)

/*****************************************************************************
  2.5.3 AMSDU功能
*****************************************************************************/
#define AMSDU_ENABLE_ALL_TID                0xFF
/* amsdu下子msdu的最大长度 */
#define WLAN_MSDU_MAX_LEN                   256
#define WLAN_AMSDU_FRAME_MAX_LEN            7935
#define HAL_NORMAL_RX_MIN_BUFFS            24

/*****************************************************************************
  2.5.8 自动调频
*****************************************************************************/
/* 未建立聚合时pps门限 */
#define NO_BA_PPS_VALUE_0              (0)
#define NO_BA_PPS_VALUE_1              (1000)
#define NO_BA_PPS_VALUE_2              (2500)
#define NO_BA_PPS_VALUE_3              (4000)
/* mate7规格 */
/* pps门限       CPU主频下限     DDR主频下限 */
/* mate7 pps门限 */
#define PPS_VALUE_0              (0)
#define PPS_VALUE_1              (1100)       /* 40M up limit */
#define PPS_VALUE_2              (2200)      /* 80M up limit */
#define PPS_VALUE_3              (14000)     /* 160M up limit */
/* mate7 CPU主频下限 */
#define CPU_MIN_FREQ_VALUE_0              (403200)
#define CPU_MIN_FREQ_VALUE_1              (604800)
#define CPU_MIN_FREQ_VALUE_2              (806400)
#define CPU_MIN_FREQ_VALUE_3              (1305600)
/* mate7 DDR主频下限 */
#define DDR_MIN_FREQ_VALUE_0              (0)
#define DDR_MIN_FREQ_VALUE_1              (3456)
#define DDR_MIN_FREQ_VALUE_2              (6403)
#define DDR_MIN_FREQ_VALUE_3              (9216)

/*****************************************************************************
  2.5.9 DEVICE 接收中断最大个数
*****************************************************************************/
#define WLAN_RX_INTERRUPT_MAX_NUM_PER_DEVICE  16          /* 一个device最多一次处理16个rx中断 */

/*****************************************************************************
  2.6 算法类别的spec
*****************************************************************************/
/*****************************************************************************
  2.6.1 Autorate
*****************************************************************************/
#define WLAN_AUTORATE_VO_AGGR_SUPPORT       0
/* Autorate 最小聚合时间索引 */
#define WLAN_AUTORATE_MIN_AGGR_TIME_IDX     3
/* 每个速率等级的平均重传次数 */
#define ALG_AUTORATE_AVG_RATE_RETRY_NUM     3

/*****************************************************************************
  2.6.2 intf det
*****************************************************************************/
/* 同频干扰信息统计的模式 */
#define WLAN_INTF_DET_COCH_MODE       1

/*****************************************************************************
  2.6.3 算法工作模式区分ASIC和FPGA
*****************************************************************************/
#define WLAN_CCA_OPT_WORK_MODE       WLAN_CCA_OPT_ENABLE
#define WLAN_EDCA_OPT_MODE_STA       WLAN_EDCA_OPT_STA_ENABLE
#define WLAN_EDCA_OPT_MODE_AP        WLAN_EDCA_OPT_AP_EN_DISABLE

#define WLAN_ANTI_INTF_WORK_MODE     WLAN_ANTI_INTF_EN_PROBE

/*****************************************************************************
  2.6.6 TXBF功能
*****************************************************************************/
/*****************************************************************************
  2.6.7 dbac
*****************************************************************************/
#define CFG_DBAC_TIMER_IDX                  0

/*****************************************************************************
  2.6.8 schedule
*****************************************************************************/
#define WLAN_TX_QUEUE_UAPSD_DEPTH    5 /* 芯片省成本，BK预处理不同，最多5个就满 */

/*****************************************************************************
  2.6.15 TPC功能
*****************************************************************************/
/* TPC步进DB数 */
#define WLAN_TPC_STEP                       3
#define WLAN_MAX_TXPOWER                    30

/*****************************************************************************
  2.6.22 STA P2P异频调度
*****************************************************************************/
/*  虚假队列个数，用于切离一个信道时，将原信道上放到硬件队列里的帧保存起来
当前只有2个场景: DBAC 与 背景扫描 DBAC占用2个队列，编号0 1; 背景扫描占用一个，编号2 */
#define HAL_TX_FAKE_QUEUE_NUM               3
#define HAL_TX_FAKE_QUEUE_BGSCAN_ID         2

/*****************************************************************************
  2.8 架构形态类别的spec
*****************************************************************************/
/*****************************************************************************
  2.8.1 芯片适配规格
*****************************************************************************/

/* 芯片无效动态功率 */
#define WLAN_DYN_POW_INVALID        250

/* HAL DEV0是否支持11MC */
#define WLAN_HAL0_11MC_IS_EN        OAL_FALSE

/* HAL DEV0是否support 2.4g dpd */
#define WLAN_HAL0_DPD_2G_IS_EN      OAL_FALSE

/* HAL DEV0是否support 5g dpd */
#define WLAN_HAL0_DPD_5G_IS_EN      OAL_FALSE

/*****************************************************************************
  2.8.3 低成本约束
*****************************************************************************/
/* 接收描述符做ping pong处理 */
#define HAL_HW_MAX_RX_DSCR_LIST_IDX        1
/* 用于存储接收完成中断最大个数 */
#define HAL_HW_RX_ISR_INFO_MAX_NUMS       (WLAN_MEM_NORMAL_RX_MAX_CNT + WLAN_MEM_SMALL_RX_MAX_CNT)
#define HAL_DOWM_PART_RX_TRACK_MEM         200

/*****************************************************************************
  RX描述符动态调整
*****************************************************************************/
#define WLAN_PKT_MEM_PKT_OPT_LIMIT   2000
#define WLAN_PKT_MEM_PKT_RESET_LIMIT 500
#define WLAN_PKT_MEM_OPT_TIME_MS     1000
#define WLAN_PKT_MEM_OPT_MIN_PKT_LEN HAL_RX_SMALL_FRAME_LEN

/*****************************************************************************
  2.8.7 特性默认开启关闭定义
*****************************************************************************/
#define WLAN_FEATURE_AMPDU_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_AMSDU_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_DSSS_CCK_IS_OPEN           OAL_FALSE
#define WLAN_FEATURE_UAPSD_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_WPA_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_TXBF_IS_OPEN               OAL_TRUE

/*****************************************************************************
  2.9 WiFi应用类别的spec
*****************************************************************************/
/*****************************************************************************
  2.9.4 P2P特性
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_P2P
#define WLAN_MAX_SERVICE_P2P_DEV_NUM          1
#define WLAN_MAX_SERVICE_P2P_GOCLIENT_NUM     1
#define WLAN_MAX_SERVICE_CFG_VAP_NUM          1
#endif

/*****************************************************************************
  2.9.12 私有安全增强
*****************************************************************************/
#define WLAN_BLACKLIST_MAX     (32)

/*****************************************************************************
  2.10 MAC FRAME特性
*****************************************************************************/
/*****************************************************************************
  2.10.1 ht cap info
*****************************************************************************/
#define HT_GREEN_FILED_DEFAULT_VALUE            0
#define HT_TX_STBC_DEFAULT_VALUE                0
#define HT_BFEE_NTX_SUPP_ANTA_NUM              (4)     /* 11n支持的bfer发送sounding时的最大天线个数 */
/*****************************************************************************
  2.10.2 vht cap info
*****************************************************************************/
#define VHT_BFEE_NTX_SUPP_STS_CAP              (4)     /* 协议中表示最大接收NDP Nsts个数 */

#define WLAN_CLOSE_DISTANCE_RSSI       (-15)             /* 关联前距离判断门限-25dBm */

#define WLAN_NORMAL_DISTANCE_RSSI_UP   (-42)             /* 信号强度小于-42dBm时，才认为非超近距离 */

#define WLAN_NEAR_DISTANCE_IMPROVE_RSSI_UP      (-40)    /* improve 1*1问题规避,要求近距离判断上门限为-44dBm */
#define WLAN_NEAR_DISTANCE_IMPROVE_RSSI_DOWN    (-48)    /* improve 1*1问题规避,要求近距离判断下门限为-50dBm */

#define WLAN_PHY_EXTLNA_CHGPTDBM_TH_VAL_20_40M  (-25)    /* 方向性攻关，修改20M和40M切换门限为-25dBm */
#define WLAN_PHY_EXTLNA_CHGPTDBM_TH_VAL_UP_80M  (-25)    /* 方向性攻关，修改80M和160M切换门限为-25dBm */

/*****************************************************************************
  2.12 COEX FEATURE
*****************************************************************************/
#define BTCOEX_RX_COUNT_LIMIT               (128)
#define BTCOEX_RX_STATISTICS_TIME           (3000)    // 双链接建议保持3s，目前测试出来最多是1s多
#define BTCOEX_PRI_DURATION_TIME            (30) // ms

#define BT_POSTPREEMPT_MAX_TIMES            (15)
#define BT_PREEMPT_MAX_TIMES                (1)

#define BLE_PREEMPT_TIMEOUT_US              (10)

#define BTCOEX_BLACKLIST_BA_SIZE_LIMIT       0x0002

#define BTCOEX_BT_SCO_DURATION
#define BTCOEX_BT_DATATRANS_DURATION
#define BTCOEX_BT_A2DP_DURATION


#define BTCOEX_PHY_TXRX_ALL_EN              (0x0000000F)
#define BTCOEX_BT2WIFI_RF_STABLE_TIME_US    (50)

#define BT_WLAN_COEX_UNAVAIL_PAYLOAD_THRES  (8)
#define BT_WLAN_COEX_SMALL_PKT_THRES        (200)
#define BT_WLAN_COEX_SMALL_FIFO_THRES       (1023)

#define OCCUPIED_TIMES                      (3)
#define OCCUPIED_INTERVAL                   (60)

#define COEX_LINKLOSS_OCCUP_TIMES           (15)
#define COEX_LINKLOSS_OCCUP_PERIOD          (20000)

#define CALI_DATA_REUPLOAD_MASK             (0x2)
#define CALI_INTVL_MASK                     (0xe0)
#define CALI_INTVL_OFFSET                   (5)

#define WLAN_SCAN_REQ_MAX_BSS              2        /* 一次可以扫描的BSS个数，PROBE REQ帧中最大可携带的BSSID SSID数量 */

/* 单位ms */
#define WLAN_AP_USER_AGING_TIME                 (300 * 1000)    /* AP 用户老化时间 300S */
#define WLAN_P2PGO_USER_AGING_TIME              (60 * 1000)     /* GO 用户老化时间 60S */

/*****************************************************************************
  22 STA AP规格
*****************************************************************************/
#define WLAN_MESHAP_ASSOC_USER_MAX_NUM       6          /* MESHAP关联的最大用户个数:6 */
#define WLAN_SOFTAP_ASSOC_USER_MAX_NUM       6          /* AP关联的最大用户个数:2 */
#define WLAN_AP_NUM_PER_DEVICE               1          /* AP的规格 WR11H: 1个AP */
#define WLAN_STA_NUM_PER_DEVICE              2          /* STA的规格 WR11H: 2个STA */
#define WLAN_SERVICE_VAP_NUM_PER_DEVICE  (WLAN_AP_NUM_PER_DEVICE + WLAN_STA_NUM_PER_DEVICE)  /* AP+STA */
#define WLAN_SERVICE_VAP_START_ID            1          /* 单芯片下，每个board的业务vap id从1开始 */
#define WLAN_CFG_VAP_ID                      0          /* 单device下 配置vap的ID为0 */
#define WLAN_CFG_VAP_NAME                    "Featureid0"   /* 单device下 配置vap的名称 */

#define WLAN_LARGE_PAYLOAD_SIZE             1500    /* 允许发送的最大帧数据长度 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifndef __WLAN_SPEC_HH503_H__ */

