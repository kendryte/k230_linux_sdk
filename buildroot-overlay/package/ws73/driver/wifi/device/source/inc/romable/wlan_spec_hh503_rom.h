/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of WLAN product specification macro definition.
 * Create: 2020-10-18
 */

#ifndef __WLAN_SPEC_HH503_ROM_H__
#define __WLAN_SPEC_HH503_ROM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* AP keepalive参数,单位ms */
#define WLAN_AP_KEEPALIVE_TRIGGER_TIME          (15 * 1000)     /* keepalive定时器触发周期 */
#define DMAC_WLAN_AP_KEEPALIVE_TRIGGER_TIME          (2 * 1000)       /* device侧 keepalive定时器触发周期 */

/* STA keepalive参数,单位ms */
#define WLAN_STA_KEEPALIVE_TIME                 (25*1000) /* wlan0发送keepalive null帧间隔, 25s */
#define WLAN_CL_KEEPALIVE_TIME                  (20*1000) /* P2P CL发送keepalive null帧间隔,避免CL被GO pvb唤醒,P2P cl 20s */

/* Beacon Interval参数 */
/* max beacon interval, ms */
#define WLAN_BEACON_INTVAL_MAX              3500
/* min beacon interval */
#define WLAN_BEACON_INTVAL_MIN              40
/* min beacon interval */
#define WLAN_BEACON_INTVAL_DEFAULT          100
/* AP IDLE状态下beacon interval值 */
#define WLAN_BEACON_INTVAL_IDLE             1000
#define WLAN_RTS_MAX                        2346
/* 用于记录03支持的速率最大个数 */
#define WLAN_MAX_SUPP_RATES                     12

/* 每个用户支持的最大速率集个数 */
#define HAL_TX_RATE_MAX_NUM                4
#define HAL_TX_RATE_NUM_0                  0
/*****************************************************************************
  2.2 其他协议/定义MAC 层协议类别的spec
*****************************************************************************/
/* TID个数放入平台SPEC,默认的数据类型业务的TID */
#define WLAN_TID_FOR_DATA                   0

/* 接收队列的个数 与HAL_RX_DSCR_QUEUE_ID_BUTT相等 */
#define HAL_RX_QUEUE_NUM                3
/* 发送队列的个数 */
#define HAL_TX_QUEUE_NUM                6

/*****************************************************************************
  2.2.10 协议节能STA侧功能
*****************************************************************************/
/* PSM特性规格 */
/* default DTIM period */
#define WLAN_DTIM_DEFAULT                   3

/*****************************************************************************
  2.3 校准类别的spec
*****************************************************************************/
/*****************************************************************************
  2.4 安全协议类别的spec
*****************************************************************************/
/*****************************************************************************
  2.4.9 WPA功能
*****************************************************************************/
/* 加密相关的宏定义 */
/* 硬件MAC 最多等待32us， 软件等待40us */
#define HAL_CE_LUT_UPDATE_TIMEOUT          4

/*****************************************************************************
  2.5 性能类别的spec
*****************************************************************************/
#define WLAN_AMPDU_TX_MAX_NUM           16  /* AMPDU发送端最大聚合子MPDU个数 */
#define WLAN_AMPDU_TX_MAX_NUM_CUST      14  /* AMPDU发送端最大聚合子MPDU个数 定制化配置 */
#define WLAN_AMPDU_TX_MAX_BUF_SIZE      64  /* 发送端的buffer size */
#define WLAN_AMPDU_TX_SCHD_STRATEGY     2   /* 软件聚合逻辑，最大聚合设置为窗口大小的一半 */

/* MAC RX BA_LUT表共32行 */
#define HAL_MAX_RX_BA_LUT_SIZE                32
/* MAC TX BA_LUT表共32行 */
#define HAL_MAX_TX_BA_LUT_SIZE                32
#define MAC_TX_BA_LUT_BMAP_LEN             ((HAL_MAX_TX_BA_LUT_SIZE + 7) >> 3)

/*****************************************************************************
  2.5.3 AMSDU功能
*****************************************************************************/

/* >= WLAN_AMSDU_MAX_NUM/2  */
#define WLAN_DSCR_SUBTABEL_MAX_NUM          1

/*****************************************************************************
  2.5.6 小包优化
*****************************************************************************/
/* 管理帧长度  */
#define HAL_RX_MGMT_FRAME_LEN              WLAN_MGMT_NETBUF_SIZE
/* 短包长度 */
/* 短包队列会造成乱序问题,先关掉 */
#define HAL_RX_SMALL_FRAME_LEN             WLAN_SHORT_NETBUF_SIZE

/* 长包长度 */
/* 80211帧最大长度:软件最大为1600，流20字节的余量，防止硬件操作越界 */
#define HAL_RX_FRAME_LEN               WLAN_LARGE_NETBUF_SIZE
#define HAL_RX_FRAME_MAX_LEN           8000

#define SMALL_IRQ_CON_TH                2    /* 连续N个中断后开始检查硬件队列资源剩余量，避免硬件断流 */
#define NORMAL_IRQ_CON_TH               2
#define SMALL_RX_DSCR_CON_TH            48   /* rx持续性门限值:中断超过门限后检查可用rx描述符,并统一补充到该门限值 */
#define NORMAL_RX_DSCR_CON_TH           24

#define WLAN_TPC_WORK_MODE           WLAN_TPC_WORK_MODE_ENABLE

/*****************************************************************************
  2.6.6 TXBF功能
*****************************************************************************/
#define WLAN_PROTECT_MAX_LEN                36      /* 保护帧预留的最大长度 */
#define WLAN_ONEPACK_PKT_LEN                24      /* ONEPACKET发送的NULL DATA帧长度 */
#define WLAN_COEX_PKT_LEN                   32      /* coex发送的NULL DATA帧长度 */

/*****************************************************************************
  2.8 架构形态类别的spec
*****************************************************************************/
/*****************************************************************************
  2.8.1 芯片适配规格
*****************************************************************************/

/* 2.4G 芯片动态功率调整范围 */
#define WLAN_2G_DYN_POW_UPPER_RANGE    30
#define WLAN_2G_DYN_POW_LOWER_RANGE    100
#define WLAN_2G_DYN_POW_RANGE_MIN      100

/* 5G 芯片动态功率动态调整范围 */
#define WLAN_5G_DYN_POW_RANGE_MIN      50
#define WLAN_5G_DYN_POW_UPPER_RANGE    20

/*****************************************************************************
  2.8.2 HAL Device0芯片适配规格
*****************************************************************************/
/* HAL DEV0支持的空间流数 */
#if (WLAN_SINGLE_NSS == WLAN_MAX_NSS_NUM)
#define WLAN_HAL0_NSS_NUM           WLAN_SINGLE_NSS
#elif (WLAN_DOUBLE_NSS == WLAN_MAX_NSS_NUM)
#define WLAN_HAL0_NSS_NUM           WLAN_DOUBLE_NSS
#endif

/* HAL DEV0支持的最大带宽 FPGA只支持80M */
#if defined(_PRE_WLAN_FEATURE_WIDTH_MODE)
#if defined(_PRE_MAX_WIDTH_80M) && (_PRE_WLAN_FEATURE_WIDTH_MODE == _PRE_MAX_WIDTH_80M)
#define WLAN_HAL0_BW_MAX_WIDTH      WLAN_BW_CAP_40M
#elif defined(_PRE_MAX_WIDTH_40M) && (_PRE_WLAN_FEATURE_WIDTH_MODE == _PRE_MAX_WIDTH_40M)
#define WLAN_HAL0_BW_MAX_WIDTH      WLAN_BW_CAP_40M
#elif defined(_PRE_MAX_WIDTH_20M) && (_PRE_WLAN_FEATURE_WIDTH_MODE == _PRE_MAX_WIDTH_20M)
#define WLAN_HAL0_BW_MAX_WIDTH      WLAN_BW_CAP_20M
#endif
#endif

/* HAL DEV0 支持SOUNDING功能 */
#define WLAN_HAL0_11N_SOUNDING      OAL_TRUE

/* HAL DEV0 支持Green Field功能 */
#define WLAN_HAL0_GREEN_FIELD       OAL_TRUE

/* HAL DEV0是否支持窄带 */
#define WLAN_HAL0_NB_IS_EN          OAL_FALSE

/* HAL DEV0是否支持1024QAM */
#define WLAN_HAL0_1024QAM_IS_EN     OAL_FALSE

/* HAL DEV0的SU_BFEE能力 */
#define WLAN_HAL0_SU_BFEE_NUM       4

/* HAL DEV0的通道选择 */
#if (WLAN_SINGLE_NSS == WLAN_MAX_NSS_NUM)
#define WLAN_HAL0_PHY_CHAIN_SEL      WLAN_PHY_CHAIN_ZERO
#elif (WLAN_DOUBLE_NSS == WLAN_MAX_NSS_NUM)
#define WLAN_HAL0_PHY_CHAIN_SEL      WLAN_PHY_CHAIN_DOUBLE
#endif

/* HAL DEV0的需要用单天线发送11b等帧时的TX通道选择 */
#define WLAN_HAL0_SNGL_TX_CHAIN_SEL WLAN_TX_CHAIN_ZERO

/* HAL DEV0的RF通道选择 */
#define WLAN_HAL0_RF_CHAIN_SEL      WLAN_RF_CHAIN_DOUBLE

/* HAL DEV0是否support tx stbc, su/mu txbfer */
#if (WLAN_SINGLE_NSS == WLAN_MAX_NSS_NUM)
#define WLAN_HAL0_TX_STBC_IS_EN     OAL_FALSE
#define WLAN_HAL0_SU_BFER_IS_EN     OAL_FALSE
#define WLAN_HAL0_MU_BFER_IS_EN     OAL_FALSE
#elif (WLAN_DOUBLE_NSS == WLAN_MAX_NSS_NUM)
/* 当前double nss下的能力以ASIC定义，FPGA可通过私有定制化文件ini来覆盖刷新 */
#define WLAN_HAL0_TX_STBC_IS_EN     OAL_TRUE
#define WLAN_HAL0_SU_BFER_IS_EN     OAL_TRUE
#define WLAN_HAL0_MU_BFER_IS_EN     OAL_FALSE
#endif

/* HAL DEV0是否support rx stbc, su/mu txbfee */
/* 当前能力以ASIC的能力来定义，FPGA可通过私有定制化文件ini来覆盖刷新 */
#define WLAN_HAL0_RX_STBC_IS_EN     OAL_TRUE
#define WLAN_HAL0_SU_BFEE_IS_EN     OAL_TRUE
#define WLAN_HAL0_MU_BFEE_IS_EN     OAL_FALSE
#define WLAN_HAL0_11N_TXBF_IS_EN    OAL_FALSE
#define WLAN_HAL0_CONTROL_FRM_TX_DOUBLE_CHAIN_FLAG    OAL_FALSE

#define WLAN_HAL0_LDPC_IS_EN        OAL_FALSE
/* 11ax开关 */
#ifdef _PRE_WLAN_FEATURE_11AX
#define WLAN_HAL0_11AX_IS_EN        OAL_TRUE
#else
#define WLAN_HAL0_11AX_IS_EN        OAL_FALSE
#endif
#define WLAN_HAL0_DPD_IS_EN         OAL_TRUE

#define WLAN_HAL0_RADAR_DETECTOR_IS_EN      OAL_TRUE
/* HAL DEV0支持TXOP PS */
#define WLAN_HAL0_TXOPPS_IS_EN        OAL_TRUE

/*****************************************************************************
  2.8.2 STA AP规格
*****************************************************************************/
/* 通道 */
#define WLAN_RF_0   0
#define WLAN_RF_1   1
/*****************************************************************************
  2.10.3 RSSI
*****************************************************************************/
#define WLAN_NEAR_DISTANCE_RSSI        (-20)             /* 默认近距离信号门限-35dBm */
#define WLAN_NEAR_DISTANCE_ADJUST_RSSI (15)             /* 近距离默认校准15dB */
#define WLAN_FAR_DISTANCE_RSSI         (-73)             /* 默认远距离信号门限-73dBm */
#define WLAN_NORMAL_DISTANCE_RSSI_DOWN (-66)             /* 信号强度大于-66dBm时，才认为是非超远距离 */
#define WLAN_FIX_MAX_POWER_RSSI        (-55)             /* 固定最大功率信号门限 */
#define WLAN_RSSI_ADJUST_TH            (-88)             /* 软件上报RSSI的调整阈值, 低于改阈值rssi-=2 */

/*****************************************************************************
  2.10.4 TXBF cap
*****************************************************************************/
#define OAL_TXBFER_USER_NUM                     2       /* txbfer用户数量:STA & P2PCL AP不支持 */
/* buffer size = 2*(4+6)/2*250/8(80M) = 312bytes,另外为snr值预留10byte,预留部分内存到336bytes */
#define WLAN_TXBFER_BUFF_SIZE                   336
#define WALN_TXBFER_PKT_LEN                     (WLAN_TXBFER_BUFF_SIZE * OAL_TXBFER_USER_NUM)
#define WLAN_BEACON_PKT_LEN 768
#define BT_POSTPREEMPT_TIMEOUT_US           (150)
#define BT_ABORT_RETRY_TIMES_MAX            (1)
#define BT_PREEMPT_TIMEOUT_US               (50)
#define BTCOEX_BT_DEFAULT_DURATION          (0xFF)
#define OCCUPIED_PERIOD                     (60000)

/*****************************************************************************
  2.13 Calibration FEATURE spec
*****************************************************************************/
/* 校准数据上传下发MASK */
#define CALI_DATA_REFRESH_MASK              (0x1)
#define CALI_POWER_LVL_DBG_MASK             (0x4)

#define WLAN_M2S_BLACKLIST_MAX_NUM             (16)    /* m2s下发黑名单用户数，需要和上层保持一致 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifndef __WLAN_SPEC_HH503_ROM_H__ */

