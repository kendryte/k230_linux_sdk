/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: vap / user common info for host and device
 */

#ifndef __ALG_COMMON_MACROS_ROM_H__
#define __ALG_COMMON_MACROS_ROM_H__

#include "td_type.h"
#include "alg_autorate_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * 是否支持VHT协议的宏，针对一些只有VHT支持的场景下使用
 * #define _PRE_WLAN_SUPPORT_VHT
 *
 * 串口打印宏，仅在宏开启时串口打印
 * #define _PRE_WLAN_ALG_UART_PRINT
 *
 * 资源获取宏，多芯片场景下使用
 * #define _PRE_ALG_RESOURCE_ACQUIRE_CHECK
 */
/******************************************************************************
  1.1 aggr宏定义
******************************************************************************/
#define ALG_AGGR_DEFAULT_AGING_TIME_MS     10000 /* 默认per统计老化的时间门限(单位:ms) */
#define ALG_AGGR_PROBE_INTVL_NUM           16    /* 聚合时间探测对应的间隔包数目 */
#define ALG_AGGR_STAT_NUM                  16    /* 聚合时间探测的统计delta PER的包数目 */
#define ALG_AGGR_STAT_SHIFT                4     /* 聚合时间探测的统计delta PER的平滑移位值 */
#define ALG_AGGR_INTF_STAT_SHIFT           2     /* 叠频场景, 减少delta PER平滑移位值，加快判决 */
#define ALG_AGGR_INTF_DOWN_STAT_NUM        4     /* 干扰场景聚合时间向下探测的统计delta PER的包数目 */
#define ALG_AGGR_INTF_UP_STAT_NUM          32    /* 干扰场景聚合时间向上探测的统计delta PER的包数目 */
#define ALG_AGGR_INTF_DOWN_UPDATE_STAT_NUM 3     /* 干扰场景聚合时间向下探测的开始更新state num数目 */
#define ALG_AGGR_INTF_UP_UPDATE_STAT_NUM   8     /* 干扰场景聚合时间向上探测的开始更新state num数目 */

#define ALG_AGGR_TIME_LIST_NUM             6          /* 聚合传输时间列表的元素个数 */
#define ALG_AGGR_INIT_AVG_STAT_NUM         4     /* 聚合初始per统计的包数目 */
#define ALG_AGGR_MAX_PROBE_WAIT_COUNT      3     /* 等待探测结果的最大发送完成中断次数(避免中断丢失导致状态异常) */
#define ALG_AGGR_NON_PROBE_PKT_NUM         6     /* 最优速率发生变化时，不进行包探测的个数 */
#define ALG_AGGR_DBAC_DEFAULT_TIME_IDX     (ALG_AGGR_TIME_LIST_NUM - 1)     /* DBAC聚合传输时间对应的数组下标(4ms) */

/******************************************************************************
  1.2 DBAC 宏定义
******************************************************************************/
#define CFG_DBAC_TBTT_OFFSET                10000     /* 使用定时器延时后 OFFSET为 0us */
#define CFG_DBAC_PROTECT_TIME_OUT_MIN       4096    /* us */
#define DBAC_ONE_PACKET_SW_WAIT_TIME 500
#define DBAC_VIP_PROTECT_THRESHOLD 1
#define CFG_DBAC_TIMER_OFFSET               10     /* 右移10位 */
#define CFG_DBAC_DESIRED_TBTT_ADJUST        0     /* us */
#define CFG_DBAC_START_NOA_OFFSET 0              /* us */
#define CFG_DBAC_TSF_THRESHOLD  20               /* us */
#define CFG_DBAC_RESV_NETBUF_THRESHOLD 10        /* 预留10个netbuf */
#define CFG_DBAC_PROTECT_GC_SLOT_PERIOD 1

/*****************************************************************************
  1.3 COMMON 宏定义
*****************************************************************************/
#define ALG_LOCK_TIMES_TH 2048
#define ALG_LOCK_TIMESTAMP_TH 300000
/*****************************************************************************
  1.4 RTS 宏定义
*****************************************************************************/
#define ALG_RTS_TOLERANT_RTS_ALL_FAIL_CNT   6      /* 可容忍的RTS All fail次数 */

#define ALG_RTS_PER_MULTIPLE                1024   /* PER扩大的倍数，默认扩大1024倍 */
#define ALG_RTS_LOSS_RATIO_MAX_NUM          250    /* 首包错误率最大门限，大于该门限清零 */
#define ALG_RTS_LOSS_RATIO_RTS_RETRY_NUM    2      /* 首包错误率统计重传次数门限，小于该重传次数时，保持1024 */
#define ALG_RTS_LOSS_RATIO_STAT_RETRY_NUM   4      /* 首包错误率统计重传次数门限，小于该重传次数时，统计值减半 */

#define ALG_RTS_PER_STAT_FAR_RSSI_THR      (-82)   /* 更新PER时判定当前是否为强信号的门限 */
#define ALG_RTS_PER_STAT_NORMAL_RSSI_THR   (-60)   /* 更新PER时判定当前是否为强信号的门限 */

#define ALG_RTS_FIRST_PKT_STAT_SHIFT       6       /* 首包错误率的默认统计除2位移值(只针对最优速率) */
#define ALG_RTS_FIRST_PKT_STAT_INTVL       32      /* 默认首包错误率的统计间隔包数目 */
#define ALG_RTS_LEGACY_1ST_LOSS_RATIO_TH   65      /* LEGACY帧的默认首包错误率(单位:千分数) */
#define ALG_RTS_HT_VHT_1ST_LOSS_RATIO_TH   130     /* HT/VHT帧的默认首包错误率(单位:千分数) */

#define ALG_RTS_OPEN_RTS_RSSI_HIGH_THR     (-70)   /* RSSI小于该门限时，打开RTS */
#define ALG_RTS_OPEN_RTS_RSSI_LOW_THR      (-85)   /* RSSI大于该门限时，打开RTS */

#define TXOP_DURATION_RTS_THRESHOLD_MAX    1023    /* dot11TxopRTSThreshold的最大值 */
#define TXOP_DURATION_RTS_THRESHOLD_ON     1       /* dot11TxopRTSThreshold是否开启 */
#define ALG_RTS_KBPS_TO_BPUS               1000    /* kbps转换到bpus */

#define ALG_RTS_DURATION_MULTIPLE          0       /* 与rts空口相加，与ppdu空口比较，判断ppdu空口是否需要rts保护 */
#define ALG_RTS_DURATION_OPEN              1       /* 是否开启用ppdu空口开销决策是否开关rts */
#define ALG_RTS_RSSI_OPEN                  0       /* 是否启用弱信号场景打开RTS的功能 */
#define ALG_RTS_RATE_DESCEND               0       /* RTS速率是否需要主动降档 */

#define ALG_RTS_11B_RATE_NUM  4    /* RTS 11B速率个数 */
#define ALG_RTS_11G_RATE_NUM  8    /* RTS 11G速率个数 */
#define ALG_RTS_11N_RATE_NUM  6    /* RTS 11N速率个数 */
#define ALG_RTS_11AX_RATE_NUM 6    /* RTS 11AX速率个数 */

/******************************************************************************
  1.5 TPC 宏定义
******************************************************************************/
#define HMAC_WLAN_NEAR_DISTANCE_RSSI        (-20)             /* 默认近距离信号门限-20dBm */
#define HMAC_WLAN_FAR_DISTANCE_RSSI         (-73)             /* 默认远距离信号门限-73dBm */
#define HMAC_WLAN_NORMAL_DISTANCE_RSSI_UP   (-42)             /* 信号强度小于-42dBm时，才认为非超近距离 */
#define HMAC_ALG_TPC_RATE_DOWN_GAP 2                          /* 速率下降的间隙 */
#define HMAC_ALG_TPC_RATE_UP_GAP 2                            /* 速率上升的间隙 */
#define HMAC_ALG_TPC_RATE_DOWN_WAIT_COUNT 32                  /* 速率下降等待次数(避免速率突变导致超近距离误判) */

/*****************************************************************************
  1.6 OTHER 宏定义
*****************************************************************************/
#define ALG_PER_PERCENT_MULTIPLE     1024  /* 百分比扩大倍数 */
#define ALG_PER_MULTIPLE_SHIFT       10    /* 百分比扩大倍数的移位值 */
#define ALG_FRAME_LEN                1544  /* 典型包长 */
#define ALG_MAX_USER_NUM_BUTT        8    /* 用户id最大7，对应数组长度 */
#define ALG_MAGIC_NUM  0xdeadbeef

#define alg_is_null_ptr1(ptr1) \
    (NULL == (ptr1))
#define alg_is_null_ptr2(ptr1, ptr2) \
    ((NULL == (ptr1)) || (NULL == (ptr2)))
#define alg_is_null_ptr3(ptr1, ptr2, ptr3) \
    ((NULL == (ptr1)) || (NULL == (ptr2)) || (NULL == (ptr3)))
#define alg_is_null_ptr4(ptr1, ptr2, ptr3, ptr4) \
    ((NULL == (ptr1)) || (NULL == (ptr2)) || (NULL == (ptr3)) || (NULL == (ptr4)))
#define alg_is_null_ptr5(ptr1, ptr2, ptr3, ptr4, ptr5) \
    ((NULL == (ptr1)) || (NULL == (ptr2)) || (NULL == (ptr3)) ||  \
    (NULL == (ptr4)) || (NULL == (ptr5)))

#define alg_delta_count32(_ul_start, _ul_end) \
                ((osal_u32)((_ul_start) < (_ul_end) ? ((_ul_end) - (_ul_start)) : \
                ((((osal_u32)0XFFFFFFFF) - (_ul_start)) + (_ul_end) + 1)))
/*****************************************************************************
  2 枚举
*****************************************************************************/
/* RTS速率挡位枚举 */
typedef enum {
    ALG_RTS_LONG_11B_1M = 0,       /* 0: RTS速率为11b1M, long preamble */
    ALG_RTS_11B_5M5     = 1,       /* 1: RTS速率为11b5.5M */
    ALG_RTS_11B_11M     = 2,       /* 2: RTS速率为11b11M */
    ALG_RTS_11G_6M      = 3,       /* 3: RTS速率为11g6M */
    ALG_RTS_11G_12M     = 4,       /* 4: RTS速率为11g12M */
    ALG_RTS_11G_24M     = 5,       /* 5: RTS速率为11g24M */

    ALG_RTS_RATE_BUTT
} alg_rts_rate_enum;
typedef osal_u8 alg_rts_rate_enum_uint8;

typedef enum {
    CH_BW_CHG_TYPE_MOVE_WORK,
    CH_BW_CHG_TYPE_SCAN,
    CH_BW_CHG_TYPE_REFRESH, /* 用于TPC刷新code表 */

    CH_BW_CHG_TYPE_BUTT
} mac_alg_channel_bw_chg_type;
typedef osal_u8 mac_alg_channel_bw_chg_type_uint8;

typedef enum {
    /* 业务调度算法配置参数,请添加到对应的START和END之间 */
    MAC_ALG_CFG_SCHEDULE_START,

    MAC_ALG_CFG_SCHEDULE_SCH_CYCLE_MS,
    MAC_ALG_CFG_SCHEDULE_SCH_METHOD,
    MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE,
    MAC_ALG_CFG_SCHEDULE_STAT_LOG,
    MAC_ALG_CFG_SCHEDULE_HOST_END,

    MAC_ALG_CFG_SCHEDULE_DMAC_START,

    MAC_ALG_CFG_SCHEDULE_TX_TIME_METHOD,
    MAC_ALG_CFG_SCHEDULE_TX_TIME_DEBUG,
    MAC_ALG_CFG_SCHEDULE_DMAC_STAT_LOG,

    MAC_ALG_CFG_SCHEDULE_END,

    /* AUTORATE算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_AUTORATE_START = 50,
    MAC_ALG_CFG_AUTORATE_DMAC_START,
    MAC_ALG_CFG_AUTORATE_RATE_MODE,
    MAC_ALG_CFG_AUTORATE_FIX_RATE,
    MAC_ALG_CFG_AUTORATE_DEBUG_LOG,
    MAC_ALG_CFG_AUTORATE_AUTO_BW,
    MAC_ALG_CFG_AUTORATE_EVENT_TRIG_DESCEND_BW,
    MAC_ALG_CFG_AUTORATE_AUTO_PROTOCOL,
    MAC_ALG_CFG_AUTORATE_IS_ENABLE,

    MAC_ALG_CFG_AUTORATE_END,

    /* AUTOAGGR算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_AGGR_START,

    MAC_ALG_CFG_AGGR_ENABLE,
    MAC_ALG_CFG_AGGR_MODE,
    MAC_ALG_CFG_AGGR_FIXED_TIME_IDX,
    MAC_ALG_CFG_AGGR_MAX_AGGR_NUM,
    MAC_ALG_CFG_AGGR_MIN_TIME_IDX,
    MAC_ALG_CFG_AGGR_DBG_LOG,
    MAC_ALG_CFG_AGGR_SERIAL_LOG,
    MAC_ALG_CFG_AGGR_VO_EN,
    MAC_ALG_CFG_AGGR_PROBE_INTVL_NUM,
    MAC_ALG_CFG_AGGR_NON_PROBE_PCK_NUM,
    MAC_ALG_CFG_AGGR_STAT_SHIFT,

    MAC_ALG_CFG_AGGR_END,

    /* SMARTANT算法配置参数， 请添加到对应的START和END之间 */
    MAC_ALG_CFG_SMARTANT_START,

    MAC_ALG_CFG_SMARTANT_ENABLE,
    MAC_ALG_CFG_SMARTANT_CERTAIN_ANT,
    MAC_ALG_CFG_SMARTANT_TRAINING_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_CHANGE_ANT,
    MAC_ALG_CFG_SMARTANT_START_TRAIN,
    MAC_ALG_CFG_SMARTANT_SET_TRAINING_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_SET_LEAST_PACKET_NUMBER,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_INTERVAL,
    MAC_ALG_CFG_SMARTANT_SET_USER_CHANGE_INTERVAL,
    MAC_ALG_CFG_SMARTANT_SET_PERIOD_MAX_FACTOR,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_FREQ,
    MAC_ALG_CFG_SMARTANT_SET_ANT_CHANGE_THRESHOLD,

    MAC_ALG_CFG_SMARTANT_END,
    /* TXBF算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_TXBF_START,
    MAC_ALG_CFG_TXBF_MASTER_SWITCH,
    MAC_ALG_CFG_TXBF_TXMODE_ENABLE,
    MAC_ALG_CFG_TXBF_11N_BFEE_ENABLE,
    MAC_ALG_CFG_TXBF_2G_BFER_ENABLE,
    MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE,
    MAC_ALG_CFG_TXBF_FIX_MODE,
    MAC_ALG_CFG_TXBF_FIX_SOUNDING,
    MAC_ALG_CFG_TXBF_PROBE_INT,
    MAC_ALG_CFG_TXBF_REMOVE_WORST,
    MAC_ALG_CFG_TXBF_STABLE_NUM,
    MAC_ALG_CFG_TXBF_PROBE_COUNT,
    MAC_ALG_CFG_TXBF_GET,
    MAC_ALG_CFG_TXBF_END,
/* TXBF LOG配置参数，请添加到对应的START和END之间 */
    /* temp protect算法配置 */
    MAC_ALG_CFG_TEMP_PROTECT_START,
    MAC_ALG_CFG_TEMP_PROTECT_GET,
    MAC_ALG_CFG_TEMP_PROTECT_SAFE_TH_SET,
    MAC_ALG_CFG_TEMP_PROTECT_OVER_TH_SET,
    MAC_ALG_CFG_TEMP_PROTECT_PAOFF_TH_SET,
    MAC_ALG_CFG_TEMP_PROTECT_TIMES,
    MAC_ALG_CFG_TEMP_PROTECT_DEBUG,
    MAC_ALG_CFG_TEMP_PROTECT_TEMP_SET,
    MAC_ALG_CFG_TEMP_PROTECT_END,
    /* 抗干扰算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_ANTI_INTF_START,

    MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE,
    MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME,
    MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE,
    MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM,
    MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM,
    MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN,
    MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH,
    MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH,
    MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE,
    MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENQUIRY,

    MAC_ALG_CFG_ANTI_INTF_END,

    /* 干扰检测算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_INTF_DET_START,

    MAC_ALG_CFG_INTF_DET_CYCLE,
    MAC_ALG_CFG_INTF_DET_MODE,
    MAC_ALG_CFG_INTF_DET_DEBUG,
    MAC_ALG_CFG_INTF_DET_COCH_THR_STA,
    MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA,
    MAC_ALG_CFG_INTF_DET_COCH_THR_UDP,
    MAC_ALG_CFG_INTF_DET_COCH_THR_TCP,
    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC,
    MAC_ALG_CFG_INTF_DET_ADJRATIO_THR,
    MAC_ALG_CFG_INTF_DET_SYNC_THR,
    MAC_ALG_CFG_INTF_DET_AVE_RSSI,
    MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH,
    MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH,
    MAC_ALG_CFG_INTF_DET_GET_INTF_TYPE,
    MAC_ALG_CFG_INTF_DET_SET_PK_MODE,

    MAC_ALG_CFG_INTF_DET_END,

    /* EDCA优化算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_EDCA_OPT_START,

    MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE,
    MAC_ALG_CFG_EDCA_OPT_STA_EN,
    MAC_ALG_CFG_TXOP_LIMIT_STA_EN,
    MAC_ALG_CFG_EDCA_OPT_STA_WEIGHT,
    MAC_ALG_CFG_EDCA_OPT_PK_DEBUG_MODE,
    MAC_ALG_CFG_EDCA_OPT_INTF_DEBUG_MODE,
    MAC_ALG_CFG_EDCA_DET_COLLISION_TH,
    MAC_ALG_CFG_EDCA_NEG_DET_NONPROBE_TH,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_OPT,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_DBG,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_NO_INTF,
    MAC_ALG_CFG_EDCA_ONE_BE_TCP_TH_INTF,

    MAC_ALG_CFG_EDCA_OPT_END,

    /* CCA优化算法配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_CCA_OPT_START,

    MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE,
    MAC_ALG_CFG_CCA_OPT_DEBUG_MODE,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_ED20,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_ED40,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_DSSS,
    MAC_ALG_CFG_CCA_OPT_SET_CCA_OFDM,
    MAC_ALG_CFG_CCA_OPT_LOG,

    MAC_ALG_CFG_CCA_OPT_END,

    /* 算法日志配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_INTF_DET_LOG_START,

    MAC_ALG_CFG_INTF_DET_STAT_LOG_START,
    MAC_ALG_CFG_INTF_DET_STAT_LOG_WRITE,

    MAC_ALG_CFG_INTF_DET_LOG_END,

    /* TPC算法配置参数, 请添加到对应的START和END之间 */
    MAC_ALG_CFG_TPC_START,

    MAC_ALG_CFG_TPC_MODE,
    MAC_ALG_CFG_TPC_DEBUG,
    MAC_ALG_CFG_TPC_LOG,
    MAC_ALG_CFG_TPC_QUERY,
    MAC_ALG_CFG_TPC_OVER_TMP_TH,
    MAC_ALG_CFG_TPC_DPD_ENABLE_RATE,
    MAC_ALG_CFG_TPC_TARGET_RATE_11B,
    MAC_ALG_CFG_TPC_TARGET_RATE_11AG,
    MAC_ALG_CFG_TPC_TARGET_RATE_HT20,
    MAC_ALG_CFG_TPC_FIX_PWR_CODE,
    MAC_ALG_CFG_TPC_TX_PWR_OFFSET_2G,
    MAC_ALG_CFG_TPC_RF_LIMIT_OFFSET_2G,

    MAC_ALG_CFG_TPC_END,

    /* TPC算法日志配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_TPC_LOG_START,

    MAC_ALG_CFG_TPC_STAT_LOG_START,
    MAC_ALG_CFG_TPC_STAT_LOG_WRITE,
    MAC_ALG_CFG_TPC_PER_PKT_LOG_START,
    MAC_ALG_CFG_TPC_PER_PKT_LOG_WRITE,
    MAC_ALG_CFG_TPC_GET_FRAME_POW,
    MAC_ALG_CFG_TPC_RESET_STAT,
    MAC_ALG_CFG_TPC_RESET_PKT,

    MAC_ALG_CFG_TPC_LOG_END,

    /* 多用户流量控制算法配置参数 */
    MAC_ALG_CFG_TRAFFIC_CTL_START,
    MAC_ALG_CFG_TRAFFIC_CTL_ENABLE,
    MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT,
    MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG,
    MAC_ALG_CFG_TRAFFIC_CTL_RX_ENABLE,
    MAC_ALG_CFG_TRAFFIC_CTL_TX_ENABLE,

    MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_PKT_LOW_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_PKT_HIGH_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_PKT_BUSY_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_MIN_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_LOW_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_HIGH_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_BUSY_THRESHOLD,

    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_RX_RESTORE_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_LOW_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_HIGH_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_BUSY_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_MIN_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_LOW_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_HIGH_THRESHOLD,
    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_BUSY_THRESHOLD,

    MAC_ALG_CFG_TRAFFIC_CTL_TX_WINDOW_SIZE,
    MAC_ALG_CFG_TRAFFIC_CTL_TX_NETBUF_RESV_SIZE,
    MAC_ALG_CFG_TRAFFIC_CTL_TX_MAX_FLOWCTL_CNT,
    MAC_ALG_CFG_TRAFFIC_CTL_INFO_DUMP,
    MAC_ALG_CFG_TRAFFIC_CTL_INFO_CLEAR,

    MAC_ALG_CFG_TRAFFIC_CTL_END,

    /* MWO DET算法日志配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_MWO_DET_START,

    MAC_ALG_CFG_MWO_DET_ENABLE,
    MAC_ALG_CFG_MWO_DET_END_RSSI_TH,
    MAC_ALG_CFG_MWO_DET_START_RSSI_TH,
    MAC_ALG_CFG_MWO_DET_DEBUG,

    MAC_ALG_CFG_MWO_DET_END,

    /* RTS算法使能，速率配置，rts模式选择配置参数，请添加到对应的START和END之间 */
    MAC_ALG_CFG_RTS_START,

    MAC_ALG_CFG_RTS_ENABLE,
    MAC_ALG_CFG_RTS_MODE,
    MAC_ALG_CFG_RTS_DEBUG,
    MAC_ALG_CFG_RTS_RATE,
    MAC_ALG_CFG_GET_RTS_MODE,
    MAC_ALG_CFG_RTS_MIB,
    MAC_ALG_CFG_RTS_HOST,

    MAC_ALG_CFG_RTS_END,

    MAC_ALG_CFG_DBAC_START,
    MAC_ALG_CFG_DBAC_LOG_ON,
    MAC_ALG_CFG_DBAC_LOG_OFF,
    MAC_ALG_CFG_DBAC_FIX_RATIO,
    MAC_ALG_CFG_DBAC_STATS_INFO,
    MAC_ALG_CFG_DBAC_RUN_INFO,
    MAC_ALG_CFG_DBAC_STATS_CLEAR,
    MAC_ALG_CFG_DBAC_END,

    MAC_ALG_CFG_GLA_START,
    MAC_ALG_CFG_GLA_USUAL_SWITCH,
    MAC_ALG_CFG_GET_GLA_USUAL_SWITCH,
    MAC_ALG_CFG_GLA_OPTIONAL_SWITCH,
    MAC_ALG_CFG_GET_GLA_OPTIONAL_SWITCH,
    MAC_ALG_CFG_GLA_END,
    // 预留
    MAC_ALG_CFG_AR_PARAMETER_DEBUG,
    MAC_ALG_CFG_RESV2,
    MAC_ALG_CFG_RESV3,
    MAC_ALG_CFG_RESV4,
    MAC_ALG_CFG_RESV5,
    MAC_ALG_CFG_RESV6,
    MAC_ALG_CFG_RESV7,
    MAC_ALG_CFG_RESV8,
    MAC_ALG_CFG_RESV9,
    MAC_ALG_CFG_RESV10,
    MAC_ALG_CFG_RESV11,
    MAC_ALG_CFG_RESV12,
    MAC_ALG_CFG_RESV13,
    MAC_ALG_CFG_RESV14,
    MAC_ALG_CFG_RESV15,
    MAC_ALG_CFG_RESV16,

    MAC_ALG_CFG_BUTT
} mac_alg_cfg_enum;
typedef osal_u16 mac_alg_cfg_enum_uint16;

/* 距离状态通知钩子类型 */
typedef enum {
    ALG_DISTANCE_NOTIFY_START = 0,
    ALG_DISTANCE_NOTIFY_DMAC_MODULE = ALG_DISTANCE_NOTIFY_START,
    ALG_TPC_BFEE_REPORT_POW_UPDATE_NOTIFY,

    ALG_DISTANCE_NOTIFY_BUTT
} alg_distance_notify_enum;
typedef osal_u8 alg_distance_notify_enum_uint8;

/* 参数同步通知钩子类型 */
typedef enum {
    ALG_PARAM_SYNC_START = 0,
    ALG_PARAM_SYNC_TPC = ALG_PARAM_SYNC_START,
    ALG_PARAM_SYNC_TRAFFIC_CTL,
    ALG_PARAM_SYNC_ANTI_INTF,
    ALG_PARAM_SYNC_DBAC,
    ALG_PARAM_SYNC_RTS,
    ALG_PARAM_SYNC_COMMON,
    ALG_PARAM_SYNC_AGGR,
    ALG_PARAM_SYNC_AUTORATE,
    ALG_PARAM_SYNC_TX_TB,
    ALG_PARAM_SYNC_SCH,
    ALG_PARAM_SYNC_GLA,
    ALG_PARAM_SYNC_NOTIFY_BUTT
} alg_param_sync_notify_enum;
typedef osal_u8 alg_param_sync_notify_enum_uint8;

/* 用户统计RSSI类别 */
typedef enum {
    ALG_TPC_RSSI_ACK = 0,
    ALG_TPC_RSSI_DATA = 1,
    ALG_TPC_RSSI_MANAG = 2,

    ALG_TPC_RSSI_BUTT
} alg_tpc_rssi_mode_enum;
typedef osal_u8 alg_tpc_rssi_mode_enum_uint8;

/* 同频干扰状态通知钩子类型 */
typedef enum {
    ALG_CO_INTF_NOTIFY_START = 0,
    ALG_CO_INTF_NOTIFY_DMAC_MODULE = ALG_CO_INTF_NOTIFY_START,
    ALG_CO_INTF_NOTIFY_AGGR = 1,
    ALG_CO_INTF_NOTIFY_RTS = 2,
    ALG_CO_INTF_NOTIFY_TPC = 3,

    ALG_CO_INTF_NOTIFY_BUTT
} alg_co_intf_notify_enum;
typedef osal_u8 alg_co_intf_notify_enum_uint8;

typedef enum {
    ALG_CFG_INT8   = 0,
    ALG_CFG_UINT8  = 1,
    ALG_CFG_UINT16 = 2,
    ALG_CFG_UINT32 = 3,
    ALG_CFG_UINT64 = 4,
    ALG_CFG_TYPE_BUTT
} alg_cfg_param_type_enum;
typedef osal_u8 alg_cfg_param_type_enum_uint8;
typedef enum {
    ALG_MEM_AUTORATE = 0,
    ALG_MEM_AGGR,
    ALG_MEM_TPC,
    ALG_MEM_RTS,
    ALG_MEM_CCA,
    ALG_MEM_DBAC,
    ALG_MEM_EDCA,
    ALG_MEM_INTF_DET,
    ALG_MEM_SCHEDULE,
    ALG_MEM_SMARTANT,
    ALG_MEM_TXBF,
    ALG_MEM_TEMP_PROT,
    ALG_MEM_ANTI_INTF,
    ALG_MEM_COMMON,
    ALG_MEM_BUTT
} alg_mem_enum;
typedef osal_u8 alg_mem_enum_uint8;

typedef enum {
    ALG_INTF_DET_COCH = 0,        /* 同频干扰检测 */
    ALG_INTF_DET_ADJCH = 1,       /* 邻频干扰检测 */
    ALG_INTF_DET_TX_TIME = 2,     /* 发送时间 */

    ALG_INTF_DET_TYPE_BUTT
} alg_intf_det_type_enum;
typedef osal_u8 alg_intf_det_type_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/* 与hal_alg_intf_det_mode_enum_uint8的区别 */
typedef enum {
    ALG_INTF_DET_STATE_NOINTF = 0, /* 无干扰状态 */
    ALG_INTF_DET_STATE_DETINTF,     /* 检测到干扰状态 */
    ALG_INTF_DET_STATE_PKADJ,      /* PK参数调整状态 */
    ALG_INTF_DET_STATE_PK,         /* PK状态 */

    ALG_INTF_DET_STATE_BUTT
} alg_intf_det_state_enum;
typedef osal_u8 alg_intf_det_state_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
typedef enum {
    MAC_ALG_INTRF_MODE_11B_SWITCH,
    MAC_ALG_INTRF_MODE_CCA_SWITCH,
    MAC_ALG_INTRF_MODE_EDCA_SWITCH,
    MAC_ALG_INTRF_MODE_11N_SWITCH,
    MAC_ALG_INTRF_MODE_NO_11B_SWITCH,
    MAC_ALG_INTRF_MODE_LONG_RANGE_INTRF_SWITCH,

    MAC_ALG_INTRF_MODE_CFG_BUTT
} mac_alg_intrf_mode_cfg_enum;
typedef osal_u8 mac_alg_intrf_mode_cfg_enum_uint8;
#endif

/* GLA开关枚举类型 常驻/可选 */
typedef enum {
    ALG_GLA_USUAL_SWITCH,
    ALG_GLA_OPTIONAL_SWITCH,

    ALG_GLA_SWITCH_TYPE_BUTT
} alg_gla_switch_type_enum;
typedef osal_u8 alg_gla_switch_type_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_common_macros_rom.h */
