/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: HAL Layer Specifications.
 * Create: 2022-2-17
 */

#ifndef __HAL_COMMON_OPS_DEVICE_ROM_H__
#define __HAL_COMMON_OPS_DEVICE_ROM_H__

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "hal_commom_ops_type_rom.h"
#include "wlan_spec_type_rom.h"
#include "hal_mac_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    2 宏定义
*****************************************************************************/
typedef enum {
    HAL_VAP_VAP0       = 0,              /* STA固定使用hal vap0 */
    HAL_VAP_VAP1       = 1,              /* P2P固定使用hal vap1 */
    HAL_VAP_VAP2       = 2,              /* AP固定使用hal vap2 */
    HAL_MAX_VAP_NUM
} hal_vap_id_enum;
typedef osal_u8 hal_vap_id_enum_uint8;
#ifdef _PRE_WLAN_DFT_STAT
#define OAM_PHY_STAT_NODE_ENABLED_MAX_NUM 4
#endif

#define HAL_POW_11B_RATE_NUM                4               /* 11b速率数目 */
#define HAL_POW_11G_RATE_NUM                8               /* 11g速率数目 */
#define HAL_POW_11A_RATE_NUM                8               /* 11a速率数目 */

#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
#ifdef _PRE_WLAN_FEATURE_1024QAM
#define HAL_POW_11AC_20M_NUM                12              /* 11n_11ac_2g速率数目 */
#else
#define HAL_POW_11AC_20M_NUM                10              /* 11n_11ac_2g速率数目 */
#endif
#else
#define HAL_POW_11AC_20M_NUM                9               /* 11n_11ac_2g速率数目 */
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
#define HAL_POW_11AC_40M_NUM                13              /* 11n_11ac_2g速率数目 */
#define HAL_POW_11AC_80M_NUM                12              /* 11n_11ac_2g速率数目 */
#else
#define HAL_POW_11AC_40M_NUM                11              /* 11n_11ac_2g速率数目 */
#define HAL_POW_11AC_80M_NUM                10              /* 11n_11ac_2g速率数目 */
#endif

#define HAL_POW_11AX_20M_NUM                12               /* 11ax 速率数目 */
#define HAL_POW_11AX_40M_NUM                12               /* 11ax 速率数目 */
#define HAL_POW_11AX_80M_NUM                12               /* 11ax 速率数目 */

#define HAL_POW_11AX_ER_SU_106_NUM          1                /* 11ax ER_SU_106 tone速率数目 */
#define HAL_POW_11AX_ER_SU_242_NUM          3                /* 11ax ER_SU_242 tone速率数目 */

/* rate-tpccode table中速率个数 */
#define HAL_POW_RATE_POW_CODE_TABLE_LEN     (HAL_POW_11B_RATE_NUM + HAL_POW_11G_RATE_NUM + HAL_POW_11AC_20M_NUM + \
    HAL_POW_11AC_40M_NUM + HAL_POW_11AC_80M_NUM + HAL_POW_11AX_20M_NUM + HAL_POW_11AX_40M_NUM + \
    HAL_POW_11AX_80M_NUM + HAL_POW_11AX_ER_SU_106_NUM + HAL_POW_11AX_ER_SU_242_NUM)

/* rate-tpccode table中速率个数 */
#define HAL_POW_RATE_POW_CODE_TABLE_2G_LEN  (HAL_POW_11B_RATE_NUM + HAL_POW_11G_RATE_NUM + HAL_POW_11AC_20M_NUM + \
    HAL_POW_11AC_40M_NUM + HAL_POW_11AC_80M_NUM + HAL_POW_11AX_20M_NUM + HAL_POW_11AX_40M_NUM + \
    HAL_POW_11AX_80M_NUM  + HAL_POW_11AX_ER_SU_106_NUM + HAL_POW_11AX_ER_SU_242_NUM)

/* 为了保证idx从0开始，个数中包含11B速率个数 */
/* rate-tpccode table中速率个数 */
#define HAL_POW_RATE_POW_CODE_TABLE_5G_LEN  (HAL_POW_11B_RATE_NUM + HAL_POW_11G_RATE_NUM + HAL_POW_11AC_20M_NUM + \
    HAL_POW_11AC_40M_NUM + HAL_POW_11AC_80M_NUM + HAL_POW_11AX_20M_NUM + HAL_POW_11AX_40M_NUM + \
    HAL_POW_11AX_80M_NUM + HAL_POW_11AX_ER_SU_106_NUM + HAL_POW_11AX_ER_SU_242_NUM)

#define HAL_POW_CUSTOM_24G_11B_RATE_NUM            2    /* 定制化11b速率数目 */
#define HAL_POW_CUSTOM_11G_11A_RATE_NUM            5    /* 定制化11g/11a速率数目 */
#define HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM         5    /* 定制化HT20_VHT20速率数目 */
#define HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM     6
#define HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM      6
#define HAL_POW_CUSTOM_5G_VHT80_RATE_NUM           5
/* 定制化全部速率 */
#define HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM         2
#define HAL_POW_CUSTOM_MCS10_11_RATE_NUM           1

#define HAL_POW_CUSTOM_24G_HE20_RATE_NUM           3

#define HAL_POW_CUSTOM_HE_ER_SU_RATE_NUM           4

#define HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM     5    /* 定制化DPD速率数目 */
#define HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM     5

#define HH503_DYN_CALI_5G_SECTION                  2            /* 5G TX Power分 high & low power校准 */

/* 定制化相关宏 */
/* NVRAM中存储的各协议速率最大发射功率参数的个数 From:24G_11b_1M To:5G_VHT80_MCS7 */
#define NUM_OF_NV_NORMAL_MAX_TXPOWER   (HAL_POW_CUSTOM_24G_11B_RATE_NUM + \
                                        HAL_POW_CUSTOM_11G_11A_RATE_NUM + HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM + \
                                        HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM + HAL_POW_CUSTOM_11G_11A_RATE_NUM + \
                                        HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM + HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM + \
                                        HAL_POW_CUSTOM_5G_VHT80_RATE_NUM)

#define NUM_OF_NV_MAX_TXPOWER          (NUM_OF_NV_NORMAL_MAX_TXPOWER + HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM  + \
                                        HAL_POW_CUSTOM_MCS10_11_RATE_NUM + HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM + \
                                        HAL_POW_CUSTOM_MCS10_11_RATE_NUM + HAL_POW_CUSTOM_MCS10_11_RATE_NUM + \
                                        HAL_POW_CUSTOM_24G_HE20_RATE_NUM * 5 + HAL_POW_CUSTOM_HE_ER_SU_RATE_NUM * 2)

#define NUM_OF_NV_DPD_MAX_TXPOWER      (HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM + HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM)
#define NUM_OF_NV_24G_11G_6M_POWER_IDX     (2)
#define NUM_OF_NV_24G_20M_MCS0_POWER_IDX   (7)
#define NUM_OF_24G_11G_6M_RATE_IDX         (4)
#define NUM_OF_24G_20M_MCS0_RATE_IDX       (12)

#define HAL_CUS_NUM_FCC_2G_PRO                   3   /* 定制化2g FCC 11B+OFDM_20M+OFDM_40M */
#define HAL_CUS_NUM_OF_SAR_PARAMS                8   /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define HAL_NUM_5G_20M_SIDE_BAND                 6   /* 定制化5g边带数 */
#define HAL_NUM_5G_40M_SIDE_BAND                 6
#define HAL_NUM_5G_80M_SIDE_BAND                 5

#define HAL_POW_PRECISION_SHIFT         10          /* TPC算法中功率的精度 */
#define HAL_POW_PA_LUT_NUM              4          /* 筛选使用的PA档位数目 */

#define HAL_POW_UPC_RF_LUT_NUM          256        /* UPC在RF中的最大档位数目 */

#define HAL_POW_UPC_LUT_NUM             2     /* 筛选使用的UPC档位数目(03 pilot upc code 修订为1bit) */

#define HAL_POW_PA_BASE_IDX                 2           /* 基准PA Index */

#define HAL_POW_2G_LPF_BASE_IDX             2           /* 2G基准LPF Index */
#define HAL_POW_2G_DAC_BASE_IDX             0           /* 2G基准DAC Index */
#define HAL_POW_5G_LPF_BASE_IDX             3           /* 5G基准LPF Index */
#define HAL_POW_5G_DAC_BASE_IDX             2           /* 5G基准DAC Index */

#define HAL_UPC_DATA_REG_NUM                1
#define HAL_POW_UPC_LOW_START_IDX           0        /* UPC低档位的起始索引 */
#define HAL_POW_CFR_BASE_IDX                0        /* 基准cfr_index Index */
#define HAL_POW_DPD_TPC_BASE_IDX            0        /* 基准dpd_tpc_lv Index */
#define HAL_DBB_SCALING_FOR_MAX_TXPWR_BASE  204      /* 2.4g6M 基准功率差值 delt_pwr = 20*ln(256)单位0.01 db */
#define HAL_POW_DELTA_DBB_SCAL_BASE_IDX     HAL_DBB_SCALING_FOR_MAX_TXPWR_BASE  /* 基准delta_dbb_scaling Index */

#define HAL_POW_2G_1MBPS_RATE_POW_IDX       0               /* 2G 1Mbps对应的功率表索引 */
#define HAL_POW_5G_6MBPS_RATE_POW_IDX       4               /* 5G 6Mbps对应的功率表索引 */
/*****************************************************************************
  3 枚举
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* sw preempt机制下蓝牙业务状态，a2dp|transfer  page|inquiry 或者  both */
typedef enum {
    HAL_BTCOEX_PS_STATUE_ACL       = 1,   /* only a2dp|数传 BIT0 */
    HAL_BTCOEX_PS_STATUE_PAGE_INQ  = 2,   /* only  page|inquiry BIT1 */
    HAL_BTCOEX_PS_STATUE_PAGE_ACL  = 3,   /* both a2dp|数传 and page|inquiry BIT0|BIT1 */
    HAL_BTCOEX_PS_STATUE_LDAC      = 4,   /* only ldac BIT2 */
    HAL_BTCOEX_PS_STATUE_LDAC_ACL  = 5,   /* ldac and a2dp|数传 BIT2|BIT0 */
    HAL_BTCOEX_PS_STATUE_LDAC_PAGE = 6,   /* ldac and page|inquiry BIT2|BIT1 */
    HAL_BTCOEX_PS_STATUE_TRIPLE    = 7,   /* ldac and page|inquiry and a2dp|数传 BIT2|BIT1|BIT0 */

    HAL_BTCOEX_PS_STATUE_BUTT
} hal_btcoex_ps_status_enum;
typedef osal_u8 hal_btcoex_ps_status_enum_uint8;

typedef enum {
    HAL_BTCOEX_HW_POWSAVE_NOFRAME   = 0,
    HAL_BTCOEX_HW_POWSAVE_SELFCTS   = 1,
    HAL_BTCOEX_HW_POWSAVE_NULLDATA  = 2,
    HAL_BTCOEX_HW_POWSAVE_QOSNULL   = 3,

    HAL_BTCOEX_HW_POWSAVE_BUTT
} hal_coex_hw_preempt_mode_enum;
typedef osal_u8 hal_coex_hw_preempt_mode_enum_uint8;

typedef enum {
    HAL_BTCOEX_WIFI_STATE_ON                     = 0,
    HAL_BTCOEX_WIFI_STATE_JOINING                = 1,
    HAL_BTCOEX_WIFI_STATE_SCAN                   = 2,
    HAL_BTCOEX_WIFI_STATE_CONN                   = 3, /* STA/AP connect */
    HAL_BTCOEX_WIFI_STATE_P2P_SCAN               = 4,
    HAL_BTCOEX_WIFI_STATE_P2P_CONN               = 5,
    HAL_BTCOEX_WIFI_STATE_PS_STOP                = 6,
    HAL_BTCOEX_WIFI_STATE_SLEEP                  = 7,

    HAL_BTCOEX_WIFI_STATE_WORK_MODE              = 8, /* bit0-3 STA/AP/GO/GC */
    HAL_BTCOEX_WIFI_STATE_DIFF_CHANNEL           = 9,
    HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11BGN       = 10,
    HAL_BTCOEX_WIFI_STATE_PROTOCOL_2G11AX        = 11,
    HAL_BTCOEX_WIFI_STATE_CHAN_BITMAP            = 12,

    /* 92 only */
    HAL_BTCOEX_WIFI_STATE_BAND                   = 13, /* 0-2G,1=5G */
    HAL_BTCOEX_WIFI_STATE_CHAN_NUM               = 14,
    HAL_BTCOEX_WIFI_STATE_BAND_WIDTH             = 15,
    HAL_BTCOEX_WIFI_STATE_AP_MODE                = 16,
    HAL_BTCOEX_WIFI_STATE_AUTH_MODE              = 17,
    HAL_BTCOEX_WIFI_STATE_C1_SISO                = 18,

    /* 73 only */
    HAL_BTCOEX_WIFI_BUSY_STATUS                  = 19,

    HAL_BTCOEX_STATE_BUTT
} hal_btcoex_wifi_state_enum;
typedef osal_u8 hal_btcoex_wifi_state_id_enum_uint8;

typedef enum {
    HAL_BTCOEX_PS_OFF,
    HAL_BTCOEX_PS_ON,
    HAL_BTCOEX_PS_BUTT
} hal_btcoex_ps_enum;
typedef osal_u8 hal_btcoex_ps_enum_uint8;
#endif

#ifdef _PRE_WLAN_DFT_STAT
/* phy统计节点设置，保存节点idx的结构 */
typedef struct {
    osal_u8 node_idx[OAM_PHY_STAT_NODE_ENABLED_MAX_NUM];
} oam_stats_phy_node_idx_stru;
#endif
/* HAL_DEVICE_WORK_STATE子状态 */
typedef enum {
    HAL_DEVICE_WORK_SUB_STATE_ACTIVE         = 0,              /* active子状态 */
    HAL_DEVICE_WORK_SUB_STATE_AWAKE          = 1,              /* awake子状态 */
    HAL_DEVICE_WORK_SUB_STATE_DOZE           = 2,              /* doze子状态 */
    HAL_DEVICE_WORK_SUB_STATE_INIT           = 3,              /* init子状态 */
    HAL_DEVICE_WORK_SUB_STATE_BUTT
} hal_device_work_sub_state_info;
typedef osal_u8 hal_work_sub_state_uint8;
#define HAL_WORK_SUB_STATE_NUM      (HAL_DEVICE_WORK_SUB_STATE_INIT - HAL_DEVICE_WORK_SUB_STATE_ACTIVE)

typedef enum {
    HAL_PHY_MAX_BW_SECT_MAX_BANDWIDTH = 0,
    HAL_PHY_MAX_BW_SECT_MAX_NSS       = 1,
    HAL_PHY_MAX_BW_SECT_SINGLE_CH_SEL = 2,

    HAL_PHY_MAX_BW_SECT_BUTT
} hal_phy_max_bw_sect_enum;
typedef osal_u8 hal_phy_max_bw_sect_enmu_uint8;

/*****************************************************************************
    3.1 队列相关枚举定义
*****************************************************************************/

#define hal_ac_to_q_num(_ac) ( \
                              ((_ac) == WLAN_WME_AC_VO) ? HAL_TX_QUEUE_VO : \
                              ((_ac) == WLAN_WME_AC_VI) ? HAL_TX_QUEUE_VI : \
                              ((_ac) == WLAN_WME_AC_BK) ? HAL_TX_QUEUE_BK : \
                              ((_ac) == WLAN_WME_AC_BE) ? HAL_TX_QUEUE_BE : \
                              ((_ac) == WLAN_WME_AC_MGMT) ? HAL_TX_QUEUE_HI : \
                              ((_ac) == WLAN_WME_AC_PSM) ? HAL_TX_QUEUE_MC : \
                              HAL_TX_QUEUE_BK)

#define hal_q_num_to_ac(_q) ( \
                             ((_q) == HAL_TX_QUEUE_VO) ? WLAN_WME_AC_VO : \
                             ((_q) == HAL_TX_QUEUE_VI) ? WLAN_WME_AC_VI : \
                             ((_q) == HAL_TX_QUEUE_BK) ? WLAN_WME_AC_BK : \
                             ((_q) == HAL_TX_QUEUE_BE) ? WLAN_WME_AC_BE : \
                             ((_q) == HAL_TX_QUEUE_HI) ? WLAN_WME_AC_MGMT : \
                             ((_q) == HAL_TX_QUEUE_MC) ? WLAN_WME_AC_PSM : \
                             WLAN_WME_AC_BE)

/*****************************************************************************
  3.3 描述符相关枚举定义
*****************************************************************************/
typedef enum {
    HAL_TX_RATE_RANK_0 = 0,
    HAL_TX_RATE_RANK_1,
    HAL_TX_RATE_RANK_2,
    HAL_TX_RATE_RANK_3,

    HAL_TX_RATE_RANK_BUTT
} hal_tx_rate_rank_enum;
typedef osal_u8 hal_tx_rate_rank_enum_uint8;

typedef enum {
    HAL_RX_NEW                    = 0x0,
    HAL_RX_SUCCESS                = 0x1,
    HAL_RX_DUP_DETECTED           = 0x2,
    HAL_RX_FCS_ERROR              = 0x3,
    HAL_RX_KEY_SEARCH_FAILURE     = 0x4,
    HAL_RX_CCMP_MIC_FAILURE       = 0x5,
    HAL_RX_ICV_FAILURE            = 0x6,
    HAL_RX_TKIP_REPLAY_FAILURE    = 0x7,
    HAL_RX_CCMP_REPLAY_FAILURE    = 0x8,
    HAL_RX_TKIP_MIC_FAILURE       = 0x9,
    HAL_RX_BIP_MIC_FAILURE        = 0xA,
    HAL_RX_BIP_REPLAY_FAILURE     = 0xB,
    HAL_RX_MUTI_KEY_SEARCH_FAILURE = 0xC,     /* 组播广播 */
    HAL_RX_WAPI_MIC_FAILURE       = 0xD
} hal_rx_status_enum;
typedef osal_u8 hal_rx_status_enum_uint8;

typedef enum {
    HAL_TX_INVALID = 0,     /* 无效 */
    HAL_TX_SUCC,            /* 成功 */
    HAL_TX_FAIL,            /* 发送失败（超过重传限制：接收响应帧超时） */
    HAL_TX_TIMEOUT,         /* lifetime超时（没法送出去） */
    HAL_TX_RTS_FAIL,        /* RTS 发送失败（超出重传限制：接收cts超时） */
    HAL_TX_NOT_COMPRASS_BA, /* 收到的BA是非压缩块确认 */
    HAL_TX_TID_MISMATCH,    /* 收到的BA中TID与发送时填写在描述符中的TID不一致 */
    HAL_TX_KEY_SEARCH_FAIL, /* Key search failed */
    HAL_TX_AMPDU_MISMATCH,  /* 描述符异常 */
    HAL_TX_PENDING,         /* 02:没有中断均为pending;03:发送过程中为pending */
    HAL_TX_FAIL_RESV,       /* resv */
    HAL_TX_FAIL_BW_TOO_BIG, /* 带宽超过PHY的最大工作带宽或流数超过最大天线数， 软件回收该帧 */
    HAL_TX_FAIL_ABORT,                  /* 发送失败（因为abort） */
    HAL_TX_FAIL_STATEMACHINE_PHY_ERROR, /* MAC发送该帧异常结束（状态机超时、phy提前结束等原因） */
    HAL_TX_SOFT_PSM_BACK,               /* 软件节能回退 */
    HAL_TX_AMPDU_BITMAP_MISMATCH       /* 硬件解析bitmap，当前mpdu未被确认 */
} hal_tx_dscr_status_enum;
typedef osal_u8 hal_tx_status_enum_uint8;

/* 接收描述符队列状态 */
typedef enum {
    HAL_DSCR_QUEUE_INVALID  = 0,
    HAL_DSCR_QUEUE_VALID,
    HAL_DSCR_QUEUE_SUSPENDED,
    HAL_DSCR_QUEUE_BUSY,
    HAL_DSCR_QUEUE_IDLE,
    HAL_DSCR_QUEUE_STATUS_BUTT
} hal_dscr_queue_status_enum;
typedef osal_u8 hal_dscr_queue_status_enum_uint8;

typedef enum {
    HAL_POW_SET_TYPE_INIT           = 0,
    HAL_POW_SET_TYPE_REFRESH        = 1,
    HAL_POW_SET_TYPE_MAG_LVL_CHANGE = 2,
    HAL_POW_SET_TYPE_CTL_LVL_CHANGE = 3,

    HAL_POW_GEN_TYPE_BUTT
} hal_pow_set_type_enum;
typedef osal_u8 hal_pow_set_type_enum_uint8;

typedef osal_u8 hal_coex_sw_irq_type_enum_uint8;

/*****************************************************************************
    3.4 中断相关枚举定义
*****************************************************************************/

/* 3.4.2  MAC错误中断类型 (枚举值与错误中断状态寄存器的位一一对应!) */
typedef enum {
    /* 描述符参数配置异常,包括AMPDU长度配置不匹配,AMPDU中MPDU长度超长,sub msdu num错误 */
    HAL_MAC_ERROR_PARA_CFG_ERR                  = 0,
    HAL_MAC_ERROR_TX_VECTOR_ERR                 = 1,        /* 发送vector中参数错误 */
    HAL_MAC_ERROR_BA_ENTRY_NOT_FOUND            = 2,        /* 未找到BA会话表项异常0 */
    HAL_MAC_ERROR_PHY_TRLR_TIME_OUT             = 3,        /* PHY_RX_TRAILER超时 */
    HAL_MAC_ERROR_PHY_RX_FIFO_OVERRUN           = 4,        /* PHY_RX_FIFO满写异常 */
    HAL_MAC_ERROR_TX_DATAFLOW_BREAK             = 5,        /* 发送帧数据断流 */
    HAL_MAC_ERROR_RX_FSM_ST_TIMEOUT             = 6,        /* RX_FSM状态机超时 */
    HAL_MAC_ERROR_TX_FSM_ST_TIMEOUT             = 7,        /* TX_FSM状态机超时 */
    HAL_MAC_ERROR_RX_HANDLER_ST_TIMEOUT         = 8,        /* RX_HANDLER状态机超时 */
    HAL_MAC_ERROR_TX_HANDLER_ST_TIMEOUT         = 9,        /* TX_HANDLER状态机超时 */
    HAL_MAC_ERROR_TX_INTR_FIFO_OVERRUN          = 10,       /* TX 中断FIFO满写 */
    HAL_MAC_ERROR_RX_INTR_FIFO_OVERRUN          = 11,       /* RX中断 FIFO满写 */
    HAL_MAC_ERROR_HIRX_INTR_FIFO_OVERRUN        = 12,       /* HIRX中断FIFO满写 */

    /* 接收到普通优先级帧但此时RX BUFFER指针为空 */
    HAL_MAC_ERROR_UNEXPECTED_RX_Q_EMPTY         = 13,

    /* 接收到高优先级帧但此时HI RX BUFFER指针为空 */
    HAL_MAC_ERROR_UNEXPECTED_HIRX_Q_EMPTY       = 14,
    HAL_MAC_ERROR_BUS_RLEN_ERR                  = 15,       /* 总线读请求长度为0异常 */
    HAL_MAC_ERROR_BUS_RADDR_ERR                 = 16,       /* 总线读请求地址无效异常 */
    HAL_MAC_ERROR_BUS_WLEN_ERR                  = 17,       /* 总线写请求长度为0异常 */
    HAL_MAC_ERROR_BUS_WADDR_ERR                 = 18,       /* 总线写请求地址无效异常 */
    HAL_MAC_ERROR_TX_ACBK_Q_OVERRUN             = 19,       /* tx acbk队列fifo满写 */
    HAL_MAC_ERROR_TX_ACBE_Q_OVERRUN             = 20,       /* tx acbe队列fifo满写 */
    HAL_MAC_ERROR_TX_ACVI_Q_OVERRUN             = 21,       /* tx acvi队列fifo满写 */
    HAL_MAC_ERROR_TX_ACVO_Q_OVERRUN             = 22,       /* tx acv0队列fifo满写 */
    HAL_MAC_ERROR_TX_HIPRI_Q_OVERRUN            = 23,       /* tx hipri队列fifo满写 */
    HAL_MAC_ERROR_MATRIX_CALC_TIMEOUT           = 24,       /* matrix计算超时 */
    HAL_MAC_ERROR_CCA_TIME_OUT                  = 25,       /* cca超时 */
    HAL_MAC_ERROR_TB_INTE_ERR                   = 26,       /* 响应TB  */
    HAL_MAC_ERROR_BEACON_MISS                   = 27,       /* 连续发送beacon失败 */
    HAL_MAC_ERROR_INTR_FIFO_UNEXPECTED_READ     = 28,       /* interrupt fifo空读异常 */
    HAL_MAC_ERROR_UNEXPECTED_RX_DESC_ADDR       = 29,       /* rx desc地址错误异常 */

    /* mac没有处理完前一帧,phy又上报了一帧异常 */
    HAL_MAC_ERROR_RX_OVERLAP_ERR                = 30,
    HAL_MAC_ERROR_RESERVED_31                   = 31,       /* 保留位 */
    HAL_MAC_ERROR_TX_ACBE_BACKOFF_TIMEOUT       = 32,       /* 发送BE队列退避超时 */
    HAL_MAC_ERROR_TX_ACBK_BACKOFF_TIMEOUT       = 33,       /* 发送BK队列退避超时 */
    HAL_MAC_ERROR_TX_ACVI_BACKOFF_TIMEOUT       = 34,       /* 发送VI队列退避超时 */
    HAL_MAC_ERROR_TX_ACVO_BACKOFF_TIMEOUT       = 35,       /* 发送VO队列退避超时 */
    HAL_MAC_ERROR_TX_HIPRI_BACKOFF_TIMEOUT      = 36,       /* 发送高优先级队列退避超时 */

    /* 接收普通队列的小包，但是小包队列指针为空 */
    HAL_MAC_ERROR_RX_SMALL_Q_EMPTY              = 37,
    HAL_MAC_ERROR_PARA_CFG_2ERR                 = 38,       /* 发送描述符中AMPDU中MPDU长度过长 */

    /* 发送描述符中11a，11b，11g发送时，mpdu配置长度超过4095 */
    HAL_MAC_ERROR_PARA_CFG_3ERR                 = 39,
    HAL_MAC_ERROR_EDCA_ST_TIMEOUT               = 40,       /* CH_ACC_EDCA_CTRL状态机超时 */

    /* 发送描述符中11a/b/g发送时，打开了ampdu使能 */
    HAL_MAC_ERROR_PARA_CFG_4ERR                 = 41,

    /* TX广播帧队列fifo满写错误，最后一次写的地址丢弃 */
    HAL_MAC_ERROR_TX_BC_Q_OVERRUN               = 42,

    /* 接收到本bss的帧，duration很大，nav保护起作用 */
    HAL_MAC_ERROR_BSS_NAV_PORT                  = 43,

    /* 接收到其他obss的帧，duration很大，nav保护起作用 */
    HAL_MAC_ERROR_OBSS_NAV_PORT                 = 44,
    HAL_MAC_ERROR_BUS_RW_TIMEOUT                = 45,       /* 读写访问超时 */
    HAL_MAC_ERROR_TX_DATA_FIFO_OVERRUN          = 46,       /* 发送通路fifo存在写溢出 */
    HAL_MAC_ERROR_RX_DATA_FIFO_OVERRUN          = 47,       /* 接收送通路fifo存在写溢出 */
    HAL_MAC_ERROR_TXBF_FIFO_OVERRUN             = 48,       /* TXBF通路FIFO满写异常告警 */
    HAL_MAC_ERROR_TYPE_BUTT
} hal_mac_error_type_enum;
typedef osal_u8 hal_mac_error_type_enum_uint8;

/* DMAC MISC 子事件枚举定义 */
typedef enum {
    HAL_EVENT_DMAC_MISC_CH_STATICS_COMP,    /* 信道统计/测量完成中断 */
    HAL_EVENT_DMAC_MISC_RADAR_DETECTED,     /* 检测到雷达信号 */
    HAL_EVENT_DMAC_MISC_DFS_AUTH_CAC,       /* DFS认证CAC测试 */
    HAL_EVENT_DMAC_MISC_DBAC,               /* DBAC */
    HAL_EVENT_DMAC_MISC_MWO_DET,            /* 微波炉识别中断 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    HAL_EVENT_DMAC_BT_A2DP,
    HAL_EVENT_DMAC_BT_LDAC,
    HAL_EVENT_DMAC_BT_SCO,
    HAL_EVENT_DMAC_BT_TRANSFER,
    HAL_EVENT_DMAC_BT_PAGE_SCAN,
    HAL_EVENT_DMAC_BT_INQUIRY,
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    HAL_EVENT_DMAC_P2P_NOA_ABSENT_START,
    HAL_EVENT_DMAC_P2P_NOA_ABSENT_END,
    HAL_EVENT_DMAC_P2P_CTWINDOW_END,
#endif
    HAL_EVENT_DMAC_BEACON_TIMEOUT,          /* 等待beacon帧超时 */
    HAL_EVENT_DMAC_CALI_TO_HMAC,            /* 校准数据从dmac抛到hmac */

#ifdef _PRE_WLAN_ONLINE_DPD
    HAL_EVENT_DMAC_DPD_TO_HMAC,
#endif
#ifdef _PRE_WLAN_RF_AUTOCALI
    HAL_EVENT_DMAC_AUTOCALI_TO_HMAC,        /* 自动化校准数据从dmac抛到hmac */
#endif
    HAL_EVENT_DMAC_MISC_WOW_WAKE,

    HAL_EVENT_DMAC_MISC_GREEN_AP,           /* Green ap timer */

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    HAL_EVENT_DMAC_MISC_BCN_NO_FRM,          /* 硬件解析无缓存帧中断 */
#endif

    HAL_EVENT_ERROR_IRQ_MAC_ERROR,          /* MAC错误中断时间 */
    HAL_EVENT_ERROR_IRQ_SOC_ERROR,          /* SOC错误中断事件 */
    HAL_EVENT_DMAC_MISC_CHR,                /* dmac处理上报的chr事件 */

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    HAL_EVENT_DMAC_BT_ASSOC_AP_CHECK,
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
    HAL_EVENT_DMAC_TWT_SP_START,
    HAL_EVENT_DMAC_TWT_SP_END,
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    HAL_EVENT_DMAC_WUR_SP_START,
    HAL_EVENT_DMAC_WUR_SP_END,
#endif

    HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT
} hal_dmac_misc_sub_type_enum;
typedef osal_u8  hal_dmac_misc_sub_type_enum_uint8;

/*****************************************************************************
    3.5 复位相关枚举定义
*****************************************************************************/
/****3.5.1  复位MAC子模块定义 **********************************************/
typedef enum {
    HAL_RESET_MAC_ALL = 0,
    HAL_RESET_MAC_LOGIC,
    HAL_RESET_MAC_BUTT
} hal_reset_mac_submod_enum;
typedef osal_u8 hal_reset_mac_submod_enum_uint8;

typedef enum {
    HAL_PM_WORK,
    HAL_PM_DEEPSLEEP,

    HAL_PM_MODE_BUTT
} hal_pm_mode_enum;
typedef osal_u8 hal_pm_mode_enum_uint8;

typedef enum {
    WLAN_PHY_RATE_1M                = 0,    /* 0000 */
    WLAN_PHY_RATE_2M                = 1,    /* 0001 */
    WLAN_PHY_RATE_5HALF_M           = 2,    /* 0010 */
    WLAN_PHY_RATE_11M               = 3,    /* 0011 */

    WLAN_PHY_RATE_48M               = 8,    /* 1000 */
    WLAN_PHY_RATE_24M               = 9,    /* 1001 */
    WLAN_PHY_RATE_12M               = 10,   /* 1010 */
    WLAN_PHY_RATE_6M                = 11,   /* 1011 */

    WLAN_PHY_RATE_54M               = 12,   /* 1100 */
    WLAN_PHY_RATE_36M               = 13,   /* 1101 */
    WLAN_PHY_RATE_18M               = 14,   /* 1110 */
    WLAN_PHY_RATE_9M                = 15,   /* 1111 */

    WLAN_PHY_RATE_BUTT
} wlan_phy_rate_enum;

typedef enum {
    HAL_VAP_STATE_INIT               = 0,
    HAL_VAP_STATE_CONNECT            = 1,       /* sta独有 */
    HAL_VAP_STATE_UP                 = 2,       /* VAP UP */
    HAL_VAP_STATE_PAUSE              = 3,       /* pause , for ap &sta */

    HAL_VAP_STATE_BUTT
} hal_vap_state_enum;
typedef osal_u8  hal_vap_state_enum_uint8;

/*****************************************************************************
    3.6 加密相关枚举定义
*****************************************************************************/
/****3.6.2  芯片加密算法类型对应芯片中的值 **********************************/
typedef enum {
    HAL_WEP40                      = 0,
    HAL_TKIP                       = 1,
    HAL_CCMP                       = 2,
    HAL_NO_ENCRYP                  = 3,
    HAL_WEP104                     = 4,
    HAL_BIP                        = 5,
    HAL_GCMP                       = 6,
    HAL_GCMP_256                   = 7,
    HAL_CCMP_256                   = 8,
    HAL_BIP_256                    = 9,
    HAL_CIPER_PROTOCOL_TYPE_BUTT
} hal_cipher_protocol_type_enum;
typedef osal_u8 hal_cipher_protocol_type_enum_uint8;

/****3.6.3  芯片填写加密寄存器CE_LUT_CONFIG AP/STA **************************/
typedef enum {
    HAL_AUTH_KEY = 0,      /* 表明该设备为认证者 */
    HAL_SUPP_KEY = 1,      /* 表明该设备为申请者 */

    HAL_KEY_ORIGIN_BUTT,
} hal_key_origin_enum;
typedef osal_u8 hal_key_origin_enum_uint8;

typedef enum {
    HAL_DYN_CALI_PDET_ADJUST_INIT = 0,
    HAL_DYN_CALI_PDET_ADJUST_ASCEND,       /* while real_pdet < expect_pdet */
    HAL_DYN_CALI_PDET_ADJUST_DECLINE,      /* while real_pdet > expect_pdet */
    HAL_DYN_CALI_PDET_ADJUST_VARIED,
    HAL_DYN_CALI_PDET_ADJUST_BUTT,
} hal_dyn_cali_adj_type_enum;
typedef osal_u8 hal_dyn_cali_adj_type_enum_uint8;

/*****************************************************************************
  STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u32 bss_color_bitmap_h;
    osal_u32 bss_color_bitmap_l;
    osal_u32 tsf_l;
    osal_u32 tsf_h;
} mac_color_area_stru;

typedef struct {
    osal_u8 mac_rate; /* MAC对应速率 */
    osal_u8 phy_rate; /* PHY对应速率 */
    osal_u8 mbps;     /* 速率 */
    osal_u8 auc_resv[1];
} mac_data_rate_stru;

/* NVRAM 参数结构体 FCC认证 非FCC */
typedef struct {
    osal_u8       max_txpower;     /* 最大发送功率 */
} hal_cfg_custom_nvram_params_stru;

typedef struct hal_pwr_fit_para_stru {
    osal_s16 pow_par2;   /* 二次项系数 */
    osal_s16 pow_par1;   /* 一次 */
    osal_s16 pow_par0;   /* 常数项 */
} hal_pwr_fit_para_stru;

/* FCC边带功率定制项 */
typedef struct {
    osal_u8 fcc_txpwr_limit_params_26ru_5g[HAL_NUM_5G_20M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_52ru_5g[HAL_NUM_5G_20M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_106ru_5g[HAL_NUM_5G_20M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_20m_5g[HAL_NUM_5G_20M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_40m_5g[HAL_NUM_5G_40M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_80m_5g[HAL_NUM_5G_80M_SIDE_BAND];
    osal_u8 fcc_txpwr_limit_params_2g[WLAN_2G_SUB_BAND_NUM + WLAN_2G_SUB_BAND_NUM][HAL_CUS_NUM_FCC_2G_PRO];
} hal_cfg_custom_fcc_txpwr_limit_stru;

typedef struct {
    osal_char                   delta_cca_ed_high_20th_2g;
    osal_char                   delta_cca_ed_high_40th_2g;
    osal_char                   delta_cca_ed_high_20th_5g;
    osal_char                   delta_cca_ed_high_40th_5g;
} hal_cfg_custom_cca_stru;

typedef struct {
    oal_bool_enum_uint8     mac_in_one_pkt_mode  : 1;
    oal_bool_enum_uint8     self_cts_success     : 1;
    oal_bool_enum_uint8     null_data_success    : 1;
    oal_bool_enum_uint8     null_data_timeout    : 1;
    osal_u8                 resv                 : 4;
} hal_one_packet_status_stru;

typedef struct {
    osal_u8     pn_tid;          /* tid,0~7, 对rx pn lut有效 */
    osal_u8     pn_peer_idx;     /* 对端peer索引,0~31 */
    osal_u8     pn_key_type;     /* 单播管理帧  0x4：组播/广播管理帧 others：保留 */
    osal_u8     all_tid;         /* 0:仅配置TID,1:所有TID 对rx pn lut有效 */
    osal_u32    pn_msb;          /* pn值的高32位,写操作时做入参，读操作时做返回值 */
    osal_u32    pn_lsb;          /* pn值的低32位，写操作时做入参，读操作时做返回值 */
} hal_pn_lut_cfg_stru;

/*****************************************************************************
  7.0 寄存器配置结构
*****************************************************************************/
struct witp_reg16_cfg {
    osal_u16    addr;
    osal_u16    val;
};
typedef struct witp_reg16_cfg witp_reg16_cfg_stru;

/*****************************************************************************
  7.3 对外部发送提供接口所用数据结构
*****************************************************************************/
/*****************************************************************************
  结构名  : hal_channel_matrix_dsc_stru
  结构说明: 矩阵信息结构体
*****************************************************************************/
typedef struct {
    /* (第10 23行) */
    osal_u8          codebook              : 2;
    osal_u8          grouping              : 2;
    osal_u8          row_num               : 4;

    osal_u8          column_num            : 4;

    /* 在Tx 描述符中不用填写该字段;发送完成中断后，将有无信道矩阵信息存储在此 */
    osal_u8          response_flag         : 1;
    osal_u8          reserve1              : 3;

    osal_u16         channel_matrix_length;      /* 信道矩阵的总字节(Byte)数 */
    osal_u32         steering_matrix;            /* txbf需要使用的矩阵地址,填写发送描述符时候使用 */
} hal_channel_matrix_dsc_stru;


typedef struct {
    osal_u32 delta_dbb_scaling0  : 10;
    osal_u32 tpc_ch1_0           : 8;
    osal_u32 dpd_tpc_lv_ch1_0    : 2;
    osal_u32 tpc_ch0_0           : 8;
    osal_u32 dpd_tpc_lv_ch0_0    : 2;
    osal_u32 cfr_idx0            : 2;

    osal_u32 delta_dbb_scaling1  : 10;
    osal_u32 tpc_ch1_1           : 8;
    osal_u32 dpd_tpc_lv_ch1_1    : 2;
    osal_u32 tpc_ch0_1           : 8;
    osal_u32 dpd_tpc_lv_ch0_1    : 2;
    osal_u32 cfr_idx1            : 2;

    osal_u32 delta_dbb_scaling2  : 10;
    osal_u32 tpc_ch1_2           : 8;
    osal_u32 dpd_tpc_lv_ch1_2    : 2;
    osal_u32 tpc_ch0_2           : 8;
    osal_u32 dpd_tpc_lv_ch0_2    : 2;
    osal_u32 cfr_idx2            : 2;

    osal_u32 delta_dbb_scaling3  : 10;
    osal_u32 tpc_ch1_3           : 8;
    osal_u32 dpd_tpc_lv_ch1_3    : 2;
    osal_u32 tpc_ch0_3           : 8;
    osal_u32 dpd_tpc_lv_ch0_3    : 2;
    osal_u32 cfr_idx3            : 2;
}hal_tx_txop_tpc_stru;

typedef struct {
    osal_u16 delta_dbb_scaling : 5;
    osal_u16 tpc_ch0           : 7;
    osal_u16 dpd_tpc_lv_ch0    : 2;
    osal_u16 cfr_idx           : 2;
}hal_tx_dscr_tpc_stru;

typedef struct {
    wlan_tx_ack_policy_enum_uint8           ack_policy;     /* ACK 策略 */
    osal_u8                               tid_no;        /* 通信标识符 */
    osal_u8                               qos_enable;    /* 是否开启QoS */
    osal_u8                               nonqos_seq_bypass; /* 非qos数据帧是否bypass */
} hal_wmm_txop_params_stru;

/* 第12 17行 */
typedef struct {
    osal_u16                              tsf_timestamp;
    osal_u8                               mac_hdr_len;
    osal_u8                               num_sub_msdu;
} hal_tx_mpdu_mac_hdr_params_stru;

typedef struct {
    osal_u8                               ra_lut_index;
    osal_u8                               tx_vap_index;
    osal_u8                               auc_resv[2];    /* 保留2字节对齐 */
} hal_tx_ppdu_addr_index_params_stru;

typedef struct {
    osal_u32                              msdu_addr0;
    osal_u16                              msdu0_len;
    osal_u16                              msdu1_len;
    osal_u32                              msdu_addr1;
    osal_u16                              msdu2_len;
    osal_u32                              msdu_addr2;
    osal_u16                              msdu3_len;
    osal_u32                              msdu_addr3;
} hal_tx_msdu_address_params;

typedef struct {
    /* 由安全特性更新 */
    wlan_security_txop_params_stru           security;         /* 第16行 MAC TX MODE 2 */

    /* groupid和partial_aid */
    wlan_groupid_partial_aid_stru            groupid_partial_aid;  /* 第12和15行部分 */
} hal_tx_txop_feature_stru;
/*****************************************************************************
  结构名  : hal_tx_txop_alg_stru
  结构说明: DMAC模块TXOP发送控制结构
*****************************************************************************/
/* 描述符第15行结构体 */
typedef struct {
    osal_u8 beam_change : 1;
    osal_u8 doppler : 1;
    osal_u8 reserved : 1;
    osal_u8 ftm_cali_en : 1;

    osal_u8 anti_intf_1thr : 2;
    osal_u8 anti_intf_0thr : 2;
    osal_u8 anti_intf_en : 1;
    osal_u8 ch_bandwidth_in_non_ht_exist : 1;
    osal_u8 dyn_bandwidth_in_non_ht_exist : 1;
    osal_u8 dyn_bandwidth_in_non_ht : 1;

    osal_u16 he_max_pe_fld : 2;                          /* WLAN_PHY_MAX_PE_FLD_VALUE_BUTT */
    osal_u16 dpd_enable : 1; /* 标识DPD enable or disable */
    osal_u16 pdet_en : 2;
    osal_u16 uplink_flag : 1;                             /* wlan_phy_uplink_flag_enum */
    osal_u16 txop_ps_not_allowed : 1;
    osal_u16 lsig_txop : 1;     /* L-SIG TXOP的保护 */
    osal_u16 long_nav_enable : 1;
    osal_u16 ampdu_enable : 1; /* 当前帧是AMPDU的标识 */
} hal_tx_phy_mode_one_stru;

typedef union {
    osal_u16 val;
    struct {
        osal_u16 he_mcs : 4;
        osal_u16 nss_mode : 2;                            /* wlan_nss_enum_uint8 */
        osal_u16 others : 10; /* 速率无关的后面10位 */
    } he_nss_mcs;
    struct {
        osal_u16 vht_mcs : 4;
        osal_u16 nss_mode : 2;
        osal_u16 others : 10;
    } vht_nss_mcs; /* 11ac的速率集定义 */
    struct {
        osal_u16 ht_mcs : 6;
        osal_u16 others : 10;
    } ht_rate; /* 11n的速率集定义 */
    struct {
        osal_u16 legacy_rate : 4;
        osal_u16 reserved1 : 2;
        osal_u16 others : 10;
    } legacy_rate; /* 11a/b/g的速率集定义 */
    struct {
        osal_u16 rate : 6; /* 上面定义的速率结构体位域 */
        osal_u16 freq_bw : 4;                             /* hal_channel_assemble_enum */
        osal_u16 gi_type : 2;                             /* wlan_phy_he_gi_type_enum/wlan_phy_he_gi_type_enum */
        osal_u16 ltf_type : 2;                            /* wlan_phy_he_ltf_type_enum */
        osal_u16 preamble_mode : 1;                       /* wlan_phy_preamble_type */
        osal_u16 he_dcm : 1;                              /* wlan_phy_dcm_status */
    } phy_mode;
} nss_rate_union;

/*  word20-23 */
typedef struct {
    nss_rate_union nss_rate;
    osal_u8 protocol_mode : 3;                            /* wlan_phy_protocol_enum */
    osal_u8 fec_coding : 1;                               /* wlan_channel_code_enum */
    osal_u8 tx_chain_sel : 2;                             /* wlan_phy_chain_sel_enum */
    osal_u8 txbf_mode : 1;
    osal_u8 stbc_mode : 1;
    osal_u8 smoothing : 1;
    osal_u8 preamble_boost_flag : 1;
    osal_u8 rts_cts_protect_mode : 2;                     /* wlan_phy_rts_cts_protect_mode_enum */
    osal_u8 tx_count : 3;        /* old 传输次数 */
    osal_u8 reserved : 1; /* 短保护间隔 */
} hal_tx_ctrl_desc_rate_stru;

#ifdef _PRE_WLAN_PLAT_URANUS
typedef struct {
    /* TX POWER (?11?) */
    osal_u8                               dpd_enable                 : 1;

    /* TX POWER (?13?) */
    osal_u8                               upc_gain_level0            : 1;
    osal_u8                               upc_gain_level1            : 1;
    osal_u8                               upc_gain_level2            : 1;
    osal_u8                               upc_gain_level3            : 1;

    /* TX POWER (?15~17?) */
    osal_u32                              pa_gain_level0              : 2;
    osal_u32                              pa_gain_level1              : 2;
    osal_u32                              pa_gain_level2              : 2;
    osal_u32                              pa_gain_level3              : 2;
    osal_u32                              dac_gain_level0             : 2;
    osal_u32                              dac_gain_level1             : 2;
    osal_u32                              dac_gain_level2             : 2;
    osal_u32                              dac_gain_level3             : 2;
    osal_u32                              lpf_gain_level0             : 3;
    osal_u32                              lpf_gain_level1             : 3;
    osal_u32                              lpf_gain_level2             : 3;
    osal_u32                              lpf_gain_level3             : 3;
    osal_u32                              reserved8                   : 4;

    osal_u32                              delta_dbb_scaling0       : 10;
    osal_u32                              delta_dbb_scaling1       : 10;
    osal_u32                              delta_dbb_scaling2       : 10;
    osal_u32                              delta_dbb_scaling3       : 10;

    osal_u32                              dpd_tpc_lv0              : 2;
    osal_u32                              dpd_tpc_lv1              : 2;
    osal_u32                              dpd_tpc_lv2              : 2;
    osal_u32                              dpd_tpc_lv3              : 2;

    /* TX POWER (?18?) */
    osal_u32                              cfr_idx0                 : 2;
    osal_u32                              cfr_idx1                 : 2;
    osal_u32                              cfr_idx2                 : 2;
    osal_u32                              cfr_idx3                 : 2;
} hal_tx_txop_tx_power_stru;
#endif

/*****************************************************************************
  结构名  : hal_rate_pow_code_gain_table_stru
  结构说明: HAL模块POW Code表结构
*****************************************************************************/
typedef struct {
    osal_u32 tpc_code_level[HAL_POW_LEVEL_NUM];
#ifdef _PRE_WLAN_PLAT_URANUS
    osal_u32 pow_code_level[HAL_POW_LEVEL_NUM];
    osal_u16 pow_gain_level[HAL_POW_LEVEL_NUM];
#endif
}hal_rate_pow_code_gain_table_stru;

/*****************************************************************************
  结构名  : hal_vap_pow_info_stru
  结构说明: HAL模块VAP级别功率结构
*****************************************************************************/
typedef struct {
    hal_rate_pow_code_gain_table_stru   *rate_pow_table;  /* 速率-TPC code&TPC gain对应表 */
    osal_u16                           reg_pow;
    oal_bool_enum_uint8                debug_flag;     /* rate_pow table调试开关 */
    osal_u8                            resv;
    osal_u32                           beacon_tpc_code;
} hal_vap_pow_info_stru;

/*****************************************************************************
  结构名  : hal_tx_ppdu_feature_stru
  结构说明: DMAC模块PPDU发送控制结构
*****************************************************************************/
typedef struct {
    /* TX VAP index 和 RA LUT Index */
    hal_tx_ppdu_addr_index_params_stru  ppdu_addr_index;

    osal_u32                          ampdu_length;         /* 不包括null data的ampdu总长度 */
    osal_u16                          min_mpdu_length;      /* 根据速率查表得到的ampdu最小mpdu的长度 */
    osal_u16                          tsf;

    osal_u8                           ampdu_enable : 1, /* 是否使能AMPDU */
                                    rifs_enable : 1, /* rifs模式下发送时，MPDU链最后是否挂一个BAR帧 */
                                    retry_flag_hw_bypass : 1,
                                    duration_hw_bypass : 1,
                                    seq_ctl_hw_bypass : 1,
                                    timestamp_hw_bypass : 1,
                                    addba_ssn_hw_bypass : 1,
                                    tx_pn_hw_bypass : 1;
    osal_u8                           mpdu_num;             /* ampdu中mpdu的个数 */
    osal_u8                           tx_ampdu_session_index;
    osal_u8                           ax_bss_color;
} hal_tx_ppdu_feature_stru;

typedef struct {
    osal_u8 mpdu_enable;
    osal_u8 mpdu_num;
    osal_u8 sub_msdu_num;
    osal_u8   mac_hdr_len;
    osal_u8 msdu0_len;
    osal_u8 msdu1_len;
    osal_u8 msdu2_len;
    osal_u8 msdu3_len;
    osal_u32 *next_dscr_addr;
    osal_u32 *msdu0_addr;
    osal_u32 *msdu1_addr;
    osal_u32 *msdu2_addr;
    osal_u32 *msdu3_addr;
    osal_u32 *mac_hdr_addr;
} hal_tx_addr_info;

/*****************************************************************************
  结构名  : hal_security_key_stru
  结构说明: DMAC模块安全密钥配置结构体
*****************************************************************************/
typedef struct {
    osal_u8                           key_id;
    wlan_cipher_key_type_enum_uint8     key_type;
    osal_u8                           lut_idx;
    wlan_ciper_protocol_type_enum_uint8 cipher_type;
    oal_bool_enum_uint8                 update_key;
    wlan_key_origin_enum_uint8          key_origin;
    osal_u8                           reserve[2];        /* 保留2字节对齐 */
    osal_u8                           *cipher_key;
    osal_u8                           *mic_key;
} hal_security_key_stru;

/*****************************************************************************
  结构名  : hal_tx_dscr_rate_stru
  结构说明: DMAC模块tx描述符速率结构体
*****************************************************************************/
typedef struct {
    osal_u8                           rate;        /* 保留2字节对齐 */
    osal_u8                           nss : 2;
    osal_u8                           count : 3;
    osal_u8                           resv : 3;
    osal_u8                           auc_resv[2];
} hal_tx_dscr_rate_stru;

/*****************************************************************************
  7.4 基准VAP和Device结构
*****************************************************************************/
typedef struct {
    osal_u16      offset_siso_base;   /* siso下提前量基准值 */
    osal_u16      offset_mimo_base;   /* mimo下提前量基准值 */
    osal_u8       state;          /* 探索状态 */
    osal_u8       tbtt_cnt;       /* tbtt数量 */
    osal_u8       bcn_rx_cnt;     /* bcn接收数,会先乘以100用u16定义 */
    osal_u8       index;          /* 探索步进数 */
    osal_u8       best_index;     /* 最优探索步进数 */
    osal_u8       accum_index;    /* 累积探索步进数 */
    osal_u8       best_rx_ratio;  /* 最优接收率 */
    osal_u8       weak_rx_cnt;    /* 探索完成后连续接收小于门限数 */
} hal_tbtt_offset_probe_stru;

#define TBTT_OFFSET_PROBE_STEP_US       30
#define TBTT_OFFSET_PROBE_MAX           20      /* 最多增加30乘20共600us */

#define TBTT_OFFSET_UP_PROBE_STEP       2       /* up probe */
#define TBTT_OFFSET_DOWN_PROBE_STEP     1
#define TBTT_OFFSET_PROBE_ACCETP_DIF    3
#define TBTT_OFFSET_PROBE_DONE_RATION   95
#define TBTT_OFFSET_PROBE_WEAK_RATION   70
#define TBTT_OFFSET_PROBE_RETRY_CNT     5

#define TBTT_OFFSET_PROBE_CALC_PERIOD   100    /* beacon接收率计算周期 */

/* state define */
#define TBTT_OFFSET_PROBE_STATE_INIT    0
#define TBTT_OFFSET_PROBE_STATE_START   1
#define TBTT_OFFSET_PROBE_STATE_UP_DONE 2
#define TBTT_OFFSET_PROBE_STATE_END     3

typedef struct {
    osal_u16                           inner_tbtt_offset_siso;
    osal_u16                           inner_tbtt_offset_mimo;

/* 唤醒后收beacon的通道，在hal device状态机awake子状态时生效 */
    osal_u8                            bcn_rf_chain;
#ifdef _PRE_PM_TBTT_OFFSET_PROBE
    hal_tbtt_offset_probe_stru       *offset_probe;
#endif
    osal_u8                            _rom[4];            /* 4字节储存rom地址 */
} hal_pm_info_stru;

typedef struct tag_hal_to_dmac_vap_stru {
    osal_u8                    vap_id : 4;                 /* VAP ID */
    osal_u8                    mac_vap_id : 4;             /* 保存mac vap id */
    wlan_vap_mode_enum_uint8 vap_mode : 4;               /* VAP工作模式 */
    wlan_p2p_mode_enum_uint8 p2p_mode : 4;               /* P2P */

    hal_vap_state_enum_uint8 hal_vap_state;              /* hal vap state状态 */
    hal_pm_info_stru         pm_info;
    /* ROM化后资源扩展指针 */
    osal_void                  *_rom;
} hal_to_dmac_vap_stru;

/*****************************************************************************
  7.5 对外部接收提供接口所用数据结构
*****************************************************************************/
#pragma pack(push, 1)
typedef struct {
    /* byte 0 */
    osal_s8            rssi_dbm;

    /* byte 1 */
    union {
        struct {
            osal_u8  he_mcs        : 4;
            osal_u8  nss_mode      : 2;                    /* wlan_nss_enum_uint8 */
            osal_u8  protocol_mode : 2;
        } he_nss_mcs;
        struct {
            osal_u8   vht_mcs       : 4;
            osal_u8   nss_mode      : 2;
            osal_u8   protocol_mode : 2;
        } vht_nss_mcs;                                   /* 11ac的速率集定义 */
        struct {
            osal_u8   ht_mcs        : 6;
            osal_u8   protocol_mode : 2;
        } ht_rate;                                       /* 11n的速率集定义 */
        struct {
            osal_u8   legacy_rate   : 4;
            osal_u8   reserved1     : 2;
            osal_u8   protocol_mode : 2;
        } legacy_rate;                                   /* 11a/b/g的速率集定义 */
    } nss_rate;

    /* byte 2-3 */
    osal_s8           snr_ant0;                          /* ant0 SNR */
    osal_s8           snr_ant1;                          /* ant1 SNR */

    /* byte 4-5 */
    osal_s8           ant0_rssi;        /* ANT0上报当前帧RSSI */
    osal_s8           ant1_rssi;        /* ANT1上报当前帧RSSI */

    /* byte 6-7 */
    osal_s16             phase_incr;             /* 报文频偏信息 */
} hal_rx_statistic_stru;
#pragma pack(pop)

/* dmac_pkt_captures使用,tx rx均会使用 */
typedef struct {
    osal_u8        ant_rssi_sw; /* 通过ANT RSSI切换使能。bit0:管理帧切换使能 bit1:数据帧切换使能 */
    oal_bool_enum_uint8 log_print;
    osal_u8        auc_resv[2];          /* 保留2字节对齐 */

    osal_s16        ant0_rssi_smth;   /* 平滑处理后历史RSSI */
    osal_s16        ant1_rssi_smth;

    osal_u16       rssi_high_cnt;
    osal_u16       rssi_high_cnt_th;

    osal_u16       rssi_low_cnt;
    osal_u16       rssi_low_cnt_th;
} hal_rx_ant_rssi_stru;

typedef struct {
    oal_bool_enum_uint8 ant_rssi_sw;     /* 通过ANT RSSI切换使能 */
    oal_bool_enum_uint8 log_print;
    osal_u8           rssi_th;         /* RSSI高门限 */
    oal_bool_enum_uint8 reserv[1];

    osal_s16        ant0_rssi_smth;   /* 平滑处理后历史RSSI */
    osal_s16        ant1_rssi_smth;
} hal_rx_ant_rssi_mgmt_stru;

/* ant_detect结构体 */
typedef struct {
    hal_rx_ant_rssi_stru rx_rssi;
    osal_void *cb;

    osal_u8   tbtt_cnt;    /* 当前统计的tbtt中断数 */
    osal_u8   tbtt_cnt_th; /* tbtt中断门限值 */
    osal_u8   mimo_tbtt_cnt;      /* 当前统计的tbtt中断数 */
    osal_u8   mimo_tbtt_open_th;  /* tbtt中断开启探测门限值 */

    osal_u8   mimo_tbtt_close_th; /* tbtt中断关闭探测门限值 */
    osal_char ori_min_th;
    osal_char cur_min_th;
    osal_u8   diff_th;

    osal_u8 miso_hold : 1; /* 是否保持在MISO状态 */
    osal_u8 mimo_hold : 1; /* 是否保持在MIMO状态 */
    osal_u8 resv0     : 6;
    osal_u8 resv1[3]; /* 3byte保留字段 */
} hal_rssi_stru;

/* 裸系统下需要传输给HMAC模块的信息 */
/* hal_rx_ctl_stru结构的修改要考虑 */
/* 最大为8字节 */
/* 1字节对齐 */
#pragma pack(push, 1)
struct mac_rx_ctl {
    /* byte 0 */
    osal_u8                   vap_id            : 5;
    osal_u8                   amsdu_enable      : 1;   /* 是否为amsdu帧,每个skb标记 */
    osal_u8                   is_first_buffer   : 1;   /* 当前skb是否为amsdu的首个skb */
    osal_u8                   is_fragmented     : 1;

    /* byte 1 */
    osal_u8                   msdu_in_buffer;          /* 每个skb包含的msdu数,amsdu用,每帧标记 */

    /* byte 2 */
    osal_u8                   ta_user_idx       : 5;
    osal_u8                   tid               : 2;
    osal_u8                   is_key_frame      : 1;

    /* byte 3 */
    osal_u8                   mac_header_len    : 6;   /* mac header帧头长度 */
    osal_u8                   is_beacon         : 1;
    osal_u8                   is_last_buffer    : 1;
    /* byte 4-5 */
    osal_u16                  frame_len;                /* 帧头与帧体的总长度,AMSDU非首帧不填 */

    /* byte 6 */
    osal_u8                   mac_vap_id        : 4;   /* 业务侧vap id号 */
    osal_u8                   buff_nums         : 4;   /* 每个MPDU占用的SKB数,AMSDU帧占多个 */
    /* byte 7 */
    osal_u8                   channel_number;          /* 接收帧的信道 */

    /* byte 8 ~ byte 13 */
    osal_u16                  ftm_frame         : 1;    /* 是否为FTM帧 */
    osal_u16                  stream_id         : 3;  /* 标记匹配的五元组流id */
    osal_u16                  xwGROzPmwOmPIOzoR3_ : 12; /* evm */
    osal_u32                  latency_timestamp_us : 24; /* 时延耗时统计时间戳，单位1us精度 */
    osal_u32                  latency_index        : 8;  /* 时延耗时统计报文标识 */

    /* byte 14 ~ byte 15 */
    osal_u16                  da_user_idx;                 /* 目的地址用户索引 */
    /* byte 16 ~ byte 19 */
    osal_u32                  pul_mac_hdr_start_addr;     /* 对应的帧的帧头地址,虚拟地址 */
};
typedef struct mac_rx_ctl  hal_rx_ctl_stru;
#pragma pack(pop)

/* 对DMAC SCAN 模块提供的硬件MAC/PHY信道测量结果结构体 */
typedef struct {
    /* MAC信道统计 */
    osal_u32  ch_stats_time_us;
    osal_u32  pri20_free_time_us;
    osal_u32  pri40_free_time_us;
    osal_u32  pri80_free_time_us;
    osal_u32  sec20_free_time_us;
    osal_u32  ch_rx_time_us;
    osal_u32  ch_tx_time_us;

    /* PHY信道测量 */
    osal_u8   phy_ch_estimate_time_ms;
    osal_s8   pri20_idle_power;
    osal_s8   pri40_idle_power;
    osal_s8   pri80_idle_power;
} hal_ch_statics_irq_event_stru;

typedef struct {
    osal_u16 statics_period;
    osal_u8  meas_period;
    osal_u8 rsv;
} hal_chan_measure_stru;

/*
 * 裸系统下针对接收，提供读取接口
 * frame_len长度
 * 802.11帧头长度(mac_hdr_len)
*/
#pragma pack(push, 1)

typedef struct {
    /* byte 0 */
    osal_u8   cipher_protocol_type  : 4;      /* 接收帧加密类型 */
    osal_u8   dscr_status           : 4;      /* 接收状态 */

    /* byte 1 */
    osal_u8   channel_code          : 1;
    osal_u8   stbc                  : 2;
    osal_u8   gi                    : 2;
    osal_u8   ampdu                 : 1;
    osal_u8   sounding_mode         : 2;

    /* byte 2 */
    osal_u8   ext_spatial_streams   : 2;
    osal_u8   smoothing             : 1;
    osal_u8   freq_bandwidth_mode   : 4;
    osal_u8   preabmle              : 1;

    /* byte 3 */
    osal_u8   rsp_flag              : 1;
    osal_u8   reserved2             : 1;
    osal_u8   he_flag               : 1;
    osal_u8   last_mpdu_flag        : 1;
    osal_u8   he_ltf_type           : 2;
    osal_u8   dcm                   : 1;
    osal_u8   reserved3             : 1;
} hal_rx_status_stru;
#pragma pack(pop)

/*****************************************************************************
    7.6 对外部保留的VAP级接口列表，建议外部不做直接调用，而是调用对应的内联函数
*****************************************************************************/
typedef struct {
    hal_to_dmac_vap_stru    vap_base;
} hal_vap_stru;

typedef struct {
    osal_u8  lut_index;
    osal_u8  peer_lut_index;
    osal_u8  tid;
    osal_u8  mmss;

    osal_u16 win_size;
    osal_u8  rsv[2];             /* 保留2字节对齐 */

    osal_u16 ssn;
    osal_u16 seq_num;

    osal_u32 bitmap_lsb;
    osal_u32 bitmap_msb;
} hal_ba_para_stru;

typedef struct {
    osal_u8 aggr_tx_on;
    osal_u8 snd_type;
    osal_u8 resv[2];
} mac_cfg_ampdu_tx_on_param_stru;

/* 2g/5g rf定制化对应得到phy+rf chain能力定制化 */
typedef struct {
    wlan_nss_enum_uint8                     nss_num_2g;              /* 2G Nss 空间流个数 */
    wlan_nss_enum_uint8                     nss_num_5g;              /* 5G Nss 空间流个数 */

    /* rf通道1/2/3(双通道),解决方案和芯片都不支持交叉 */
    osal_u8                               support_rf_chain_2g;

    /* rf通道1/2/3(双通道),解决方案和芯片都不支持交叉 */
    osal_u8                               support_rf_chain_5g;
} hal_cfg_rf_custom_cap_info_stru;

typedef struct tag_hal_to_dmac_chip_stru {
} hal_to_dmac_chip_stru;

#ifdef _PRE_WLAN_PLAT_URANUS
/* Define the union u_err_intr_stat */
typedef union {
    /* Define the struct  bits */
    struct {
        osal_u32 x_uLwW__wuG_Gw6NgwW___ : 1;              /* [0]  */
        osal_u32 x_uLwW__wLPwqW6Li_wW___ : 1;             /* [1]  */
        osal_u32 x_uLwW__wjGwWhL_twhiLwNidhF_ : 1;        /* [2]  */
        osal_u32 x_uLwW__wuQtwL_o_wLBSWwidL_ : 1;         /* [3]  */
        osal_u32 x_uLwW__wuQtw_PwNBNiwiqW__dh_ : 1;       /* [4]  */
        osal_u32 x_uLwW__wLPwFGLGNoiTwj_WGK_ : 1;         /* [5]  */
        osal_u32 xwGROmwwOw8OtsIOsRORlImvSR_ : 1;         /* [6]  */
        osal_u32 xwGROmwwOR8OtsIOsRORlImvSR_ : 1;         /* [7]  */
        osal_u32 xwGROmwwOw8OAzo4xmwOsRORlImvSR_ : 1;     /* [8]  */
        osal_u32 xwGROmwwOR8OAzo4xmwOsRORlImvSR_ : 1;     /* [9]  */
        osal_u32 xwGROmwwOR8OloRwOtltvOvPmwwSo_ : 1;      /* [10]  */
        osal_u32 xwGROmwwOw8OloRwOtltvOvPmwwSo_ : 1;      /* [11]  */
        osal_u32 xwGROmwwOAlw8OloRwOtltvOvPmwwSo_ : 1;    /* [12]  */
        osal_u32 x_uLwW__wdhWPuW6LWFw_PwRwWSuLt_ : 1;     /* [13]  */
        osal_u32 x_uLwW__wdhWPuW6LWFwQB_PwRwWSuLt_ : 1;   /* [14]  */
        osal_u32 x_uLwW__wjdCw_oWhwW___ : 1;              /* [15]  */
        osal_u32 xwGROmwwOySsOwz44wOmww_ : 1;             /* [16]  */
        osal_u32 x_uLwW__wjdCwToWhwW___ : 1;              /* [17]  */
        osal_u32 xwGROmwwOySsOqz44wOmww_ : 1;             /* [18]  */
        osal_u32 xwGROmwwOR8OzCydO6OvPmwwSo_ : 1;         /* [19]  */
        osal_u32 x_uLwW__wLPwG6jWwRwiqW__dh_ : 1;         /* [20]  */
        osal_u32 xwGROmwwOR8OzCPlO6OvPmwwSo_ : 1;         /* [21]  */
        osal_u32 xwGROmwwOR8OzCPvO6OvPmwwSo_ : 1;         /* [22]  */
        osal_u32 xwGROmwwOR8OAlGwlO6OvPmwwSo_ : 1;        /* [23]  */
        osal_u32 xwGROmwwOIzRwl8OCzxCORlImOvSR_ : 1;      /* [24]  */
        osal_u32 xwGROmwwOCCzORlImOvSR_ : 1;              /* [25]  */
        osal_u32 xwGRORyOloRwOmww_ : 1;                   /* [26]  */
        osal_u32 xwGROmwwOymzCvoOIlss_ : 1;               /* [27]  */
        osal_u32 x_uLwW__wBhL_wNBNiwdhWPuW6LWFw_WGF_ : 1; /* [28]  */
        osal_u32 xwGROmwwOSom8GmCRm4Ow8O4msCOz44w_ : 1;   /* [29]  */
        osal_u32 xwGROmwwOw8OvPmwxzGOmww_ : 1;            /* [30]  */
        osal_u32 reserved_0 : 1;                        /* [31]  */
    } bits;
    /* Define an unsigned member */
    osal_u32 u32;
} u_err_intr_stat;

/* Define the union u_err2_intr_status */
typedef union {
    /* Define the struct  bits */
    struct {
        osal_u32 xwGROmwwOR8OzCymOyzCdvttORlImvSR_ : 1;  /* [0]  */
        osal_u32 x_uLwW__wLPwG6jKwjG6KiNNwLBSWidL_ : 1;  /* [1]  */
        osal_u32 x_uLwW__wLPwG6qBwjG6KiNNwLBSWidL_ : 1;  /* [2]  */
        osal_u32 x_uLwW__wLPwG6qiwjG6KiNNwLBSWidL_ : 1;  /* [3]  */
        osal_u32 x_uLwW__wLPwQBu_BwjG6KiNNwLBSWidL_ : 1; /* [4]  */
        osal_u32 x_uLwW__w_PwCSGoowRwWSuLt_ : 1;         /* [5]  */
        osal_u32 xwGROmwwOGzwzOCt2OZmww_ : 1;            /* [6]  */
        osal_u32 xwGROmwwOGzwzOCt2Oimww_ : 1;            /* [7]  */
        osal_u32 x_uLwW__wWF6GwCLwLBSWidL_ : 1;          /* [8]  */
        osal_u32 x_uLwW__wuG_Gw6NgwDW___ : 1;            /* [9]  */
        osal_u32 xwGROmwwOR8OyCO6OvPmwwSo_ : 1;          /* [10]  */
        osal_u32 xwGROmwwOyssOozPOGwvR_ : 1;             /* [11]  */
        osal_u32 xwGROmwwOvyssOozPOGwvR_ : 1;            /* [12]  */
        osal_u32 xwGROmwwOySsOwqORlImvSR_ : 1;           /* [13]  */
        osal_u32 x_uLwLPwFGLGwNBNiwiqW__dh_ : 1;         /* [14]  */
        osal_u32 x_uLw_PwFGLGwNBNiwiqW__dh_ : 1;         /* [15]  */
        osal_u32 x_uLwLPjNwNBNiwiqW__dh_ : 1;            /* [16]  */
        osal_u32 reserved_0 : 15;                      /* [31..17]  */
    } bits;
    /* Define an unsigned member */
    osal_u32 u32;
} u_err2_intr_status;
#endif

typedef struct {
    u_err_intr_stat                      error1_val; /* 错误1中断状态 */
    u_err2_intr_status                      error2_val; /* 错误2中断状态 */
} hal_error_state_stru;

/* 会影响目标vdet值的参数集合 */
typedef union {                                                 /* todo */
    struct {
        wlan_channel_band_enum_uint8    freq         : 4;
        wlan_bw_cap_enum_uint8          band_width   : 4;
        osal_u8                         channel;
        osal_u8                         pdet_chain   : 4;
        wlan_mod_enum_uint8             mod          : 4;
        osal_u8                         tx_pow;
    } rf_core_para;
    osal_u32 para;
} hal_dyn_cali_record_union;

typedef struct {
    hal_dyn_cali_record_union           record_para;
    wlan_phy_protocol_enum_uint8        cur_protocol;
    osal_s16                            real_pdet;
    osal_u8                             upc_gain_idx;
    osal_u8                             rate_idx;
    oal_bool_enum_uint8                 flag;
    osal_u8                             rsv[2];          /* 保留2字节对齐 */
} hal_pdet_info_stru;

typedef struct {
    hal_dyn_cali_record_union          record_para;
    osal_s16                           real_pdet;
    osal_s16                           exp_pdet;
    osal_u8                            auc_resv[4];
} hal_dyn_cali_usr_record_stru;

/* 扫描状态，通过判断当前扫描的状态，判断多个扫描请求的处
    理策略以及上报扫描结果的策略 */
typedef enum {
    HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT,
    HAL_SCAN_PASUE_TYPE_SWITCH_BACK,

    HAL_SCAN_PASUE_TYPE_BUTT
} hal_scan_pasue_enum;
typedef osal_u8 hal_scan_pause_type_enum_uint8;

typedef struct {
    osal_u8                    num_channels_2g;
    osal_u8                    num_channels_5g;
    osal_u8                    max_scan_count_per_channel;   /* 每个信道的扫描次数 */
    osal_u8                    resv;
    osal_u8                    scan_channel_interval;   /* 间隔n个信道，切回工作信道工作一段时间 */
    wlan_scan_mode_enum_uint8    scan_mode;
    wlan_channel_band_enum_uint8 scan_band;                   /* 扫描的频段,支持fast scan使用 */
    osal_u8                    auc_resv[1];

    /* 扫描在某一信道停留此时间后，扫描结束, ms，必须配置为MAC负载统计周期的整数倍 */
    osal_u16                   scan_time;
    osal_u16                   work_time_on_home_channel;   /* 背景扫描时，返回工作信道工作的时间 */
} hal_scan_info_stru;

/* 配置相关信息(包含定制化) */
typedef struct {
    osal_u8 nss_num                       : 3; /* Nss 空间流个数 */
    osal_u8 wlan_bw_max                   : 4;
    osal_u8 ctrl_frm_tx_double_chain_flag : 1;
    osal_u8 phy_chain;   /* phy通道1/2/3 */
    osal_u8 single_tx_chain;
    /* 管理帧采用单通道发送时选择的通道(单通道时要配置和uc_phy_chain一致),
       或者用于配置phy接收通道寄存器 */
    osal_u8 rf_chain;             /* rf通道1/2/3(双通道),解决方案和芯片都不支持交叉 */

    osal_u8    tx_stbc_is_supp  : 1,     /* 是否支持最少2x1 STBC发送 */
             rx_stbc_is_supp  : 1,     /* 是否支持stbc接收,支持2个空间流 */
             su_bfmer_is_supp : 1,     /* 是否支持单用户beamformer */
             su_bfmee_is_supp : 1,     /* 是否支持单用户beamformee */
             mu_bfmer_is_supp : 1,     /* 是否支持多用户beamformer */
             mu_bfmee_is_supp : 1,     /* 是否支持多用户beamformee */
             is_supp_11ax     : 1,
             radar_detector_is_supp : 1;
    osal_u8    dpd_is_supp      : 1,
             sounding_11n     : 1,
             green_field      : 1,
             txopps_is_supp   : 1,      /* 是否使能TXOP PS */
             is_supp_1024qam  : 1,      /* 支持1024QAM速率 */
             nb_is_supp       : 1,
             is_supp_txbf_11n : 1,      /* 是否支持11n txbf */
             ldpc_is_supp     : 1;      /* 是否支持ldpc */
    osal_u8    su_bfee_num;
    osal_u8    phy2dscr_chain; /* 和uc_phy_chain对应，这里是配置发送描述符 */
} hal_cfg_cap_info_stru;

/* hal device alg结构体定义 */
typedef enum {
    HAL_ALG_DEVICE_STRU_ID_SCHEDULE,
    HAL_ALG_DEVICE_STRU_ID_AUTORATE,
    HAL_ALG_DEVICE_STRU_ID_AGGR,
    HAL_ALG_DEVICE_STRU_ID_RTS,
    HAL_ALG_DEVICE_STRU_ID_SMARTANT,
    HAL_ALG_DEVICE_STRU_ID_DBAC,
    HAL_ALG_DEVICE_STRU_ID_TXBF,
    HAL_ALG_DEVICE_STRU_ID_ANTI_INTF,
    HAL_ALG_DEVICE_STRU_ID_MWO_DET,
    HAL_ALG_DEVICE_STRU_ID_TPC,        // TPC结构体
    HAL_ALG_DEVICE_STRU_ID_EDCA_OPT,
    HAL_ALG_DEVICE_STRU_ID_CCA_OPT,
    HAL_ALG_DEVICE_STRU_ID_INTF_DET,
    HAL_ALG_DEVICE_STRU_ID_TEMP_PROTECT,
    HAL_ALG_DEVICE_STRU_ID_BUTT
} hal_alg_device_stru_id_enum;
typedef osal_u8 hal_alg_device_stru_id_enum_uint8;

/* 挂在各个hal device上的数据结构 */
typedef struct {
    osal_void       *alg_info[HAL_ALG_DEVICE_STRU_ID_BUTT];
} hal_alg_device_stru;

/* 设备距离、干扰状态等信息 */
typedef struct {
    hal_alg_user_distance_enum_uint8           alg_distance_stat;    /* 距离状态  */
    hal_alg_intf_det_mode_enum_uint8           adj_intf_state;       /* 邻频干扰状态 */
    oal_bool_enum_uint8                        co_intf_state;        /* 同频干扰状态 */
    osal_u8                                    reserv;
} hal_alg_stat_info_stru;

typedef struct {
    osal_u8 rate_max_tx_pow_ctrl_table[HAL_POW_RATE_POW_CODE_TABLE_LEN][WLAN_BAND_BUTT];
    osal_u8 rev[2];             /* 保留2字节对齐 */
} hal_cfg_rate_max_tx_pow_stru;

typedef struct {
    osal_s16 as_pow_level_table[HAL_POW_LEVEL_NUM - 1][WLAN_BAND_BUTT];
} hal_cfg_tpc_lvl_vs_gain_stru;

/* DMAC_VAP发射功率TX功率控制相关的参数结构体 */
typedef struct {
    /* 1) MAX_Power@rate */
    hal_cfg_rate_max_tx_pow_stru     *max_tx_pow_per_rate_ctrl;

    osal_u8                            rsv_0;                     /* 对齐hal_cfg_rate_max_tx_pow_stru中的h8 */
    osal_u8                            tx_power;
    /* PAPR=5.5时的功率偏差值 */
    osal_u8                            pwr_compens_val_6m;
    osal_u8                            pwr_compens_val_mcs0;
    /* 3) LPF Index@rate */
    osal_u8                           *lpf_idx_per_rate_ctrl_2g;
    osal_u8                           *lpf_idx_per_rate_ctrl_5g;
    /* 4) TPC档位 vs Target Gain表 */
    hal_cfg_tpc_lvl_vs_gain_stru  *tpc_lvl_vs_gain_ctrl;
    hal_rate_pow_code_gain_table_stru      *rate_pow_table_2g;
    hal_rate_pow_code_gain_table_stru      *rate_pow_table_5g;
    osal_void                              *_rom;
} hal_device_tx_pwr_ctrl_stru;
typedef struct {
    osal_u16 rpt_obss_pd_tx_num;
    osal_u16 rpt_obss_pd_tx_success_num;
    osal_u16 rpt_psr_sr_tx_num;
    osal_u16 rpt_psr_sr_tx_success_num;
} dmac_sr_sta_vfs_stru;

typedef struct {
    osal_u16 rpt_non_srg_62_68_cnt;
    osal_u16 rpt_non_srg_68_74_cnt;
    osal_u16 rpt_non_srg_74_78_cnt;
    osal_u16 rpt_non_srg_78_82_cnt;
} dmac_sr_sta_non_srg_stru;

typedef struct {
    osal_u16 rpt_srg_62_68_cnt;
    osal_u16 rpt_srg_68_74_cnt;
    osal_u16 rpt_srg_74_78_cnt;
    osal_u16 rpt_srg_78_82_cnt;
} dmac_sr_sta_srg_stru;

#ifdef _PRE_WLAN_FEATURE_TWT
typedef struct {
    osal_u32 duration;
    osal_u32 interval;
    osal_u64 start_tsf;
} twt_reg_param_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/* WUR周期配置参数 */
typedef struct {
    osal_u32 duration;
    osal_u32 interval;
    osal_u64 start_tsf;
} wur_reg_param_stru;

/* WUR BEACON帧发送参数 */
typedef struct {
    osal_u16 wur_bcn_period;
    osal_u16 rsvd;
    osal_u32 pkt_ptr;
    osal_u32 pkt_len;
    osal_u32 phy_tx_mode;
    osal_u32 tx_data_rate;
} wur_bcn_reg_param_stru;

/* WUR WAKE UP帧发送参数 */
typedef struct {
    osal_u16 id : 12,
             counter : 4;
    osal_u16 group_buffer : 1,
             rsvd1 : 15;
} wur_wakeup_reg_param_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct {
    hal_coex_sw_preempt_mode_stru       sw_preempt_mode;
    hal_coex_sw_preempt_type_uint8      sw_preempt_type;
    hal_coex_sw_preempt_subtype_uint8   sw_preempt_subtype;
    hal_fcs_protect_coex_pri_enum_uint8 protect_coex_pri;      /* one pkt帧发送优先级 */
    osal_u16                          timeout_ms;        /* ps超时时间，page扫描190slot 音乐和数传30slot */
    osal_u8 last_acl_status : 1, /* 保存上一次acl状态 */
            ps_stop         : 1, /* 特定业务下，不需要开启ps，通知蓝牙不要发送ps中断 */
            ps_pause        : 1, /* 特定业务下，需要暂停ps，不影响ps中断处理，防止和wifi特定业务冲突 */
            coex_pri_forbit : 1, /* coex pri控制开关，ldac下需要关闭该功能 */
            resv0           : 4;
    osal_u8   resv;
    osal_u32                          ps_cur_time;           /* 用于ps中断上下半部执行时间统计 */
    osal_atomic                       ps_event_num;          /* ps中断event数目 */
    osal_u32                          ps_on_cnt;             /* DFX统计ps on切换次数 */
    osal_u32                          ps_off_cnt;            /* DFX统计ps off切换次数 */
    osal_u32                          ps_timeout_cnt;        /* DFX统计ps超时切换次数 */
} hal_device_btcoex_sw_preempt_stru;
#endif

/* 接收端描述符分配算法进行吞吐量统计的结构体 */
typedef struct {
    /* 记录一个周期(100ms)内接收队列接收帧的数量 */
    osal_u16                      rx_event_pkts_sum;

    /* 记录一个周期(100ms)内接收队列产生isr info数量 */
    osal_u16                      rx_event_cnt;

    /* 记录一个周期(100ms)内接收队列挂载的资源个数的峰值 */
    osal_u16                      rx_max_dscr_used_cnt;

    /* 记录一个周期(100ms)内接收队列接收帧的数量的平滑值 */
    osal_u16                      rx_dscr_smooth_event_pkts;

    /* 记录一个周期(100ms)内接收队列挂载的资源个数的峰值的平滑值 */
    osal_u16                      rx_dscr_smooth_avr_dscr;

    /* 记录一个周期(100ms)内接收描述符上报Q_EMPTY数量 */
    osal_u16                      rx_dscr_q_empty;
} alg_rx_dscr_ctl_alg_info_stru;

/* 接收端描述符分配算法进行吞吐量统计的结构体 */
typedef struct {
    alg_rx_dscr_ctl_alg_info_stru  *rx_dscr_ctl_alg_info;
    osal_u16          pending_isr[HAL_RX_DSCR_QUEUE_ID_BUTT]; /* 记录pinding在乒乓队列里的isr info数量 */
    osal_u16          pending_pkt[HAL_RX_DSCR_QUEUE_ID_BUTT]; /* 记录pinding在乒乓队列里的帧数量 */
} alg_rx_dscr_ctl_device_info_stru;

typedef struct {
    /* MAC统计 */
    osal_u32 rx_direct_time;
    osal_u32 rx_nondir_time;
    osal_u32 tx_time;
} hal_ch_mac_statics_stru;
typedef struct {
    /* 干扰相关统计 */
    osal_u16 duty_cyc_ratio_20; /* 20M干扰繁忙度 */
    osal_u16 duty_cyc_ratio_40; /* 40M干扰繁忙度 */
    osal_u16 duty_cyc_ratio_80; /* 80M干扰繁忙度 */
    osal_u16 sync_err_ratio;   /* 同步错误率 */
    osal_u32 rx_time;          /* rx总时间 */
    osal_u32 tx_time;          /* tx总时间 */
    osal_u32 free_time;        /* 空闲时间 */
    osal_u32 abort_time_us;    /* 被打断时间，包括共存和扫描 */
} hal_ch_intf_statics_stru;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* coex siso业务管理结构体 */
typedef struct {
    osal_u8   m2s_on_6slot         : 1,        /* 6slot申请siso */
            ldac_m2s_on          : 1,        /* ldac申请siso */
            m2s_resv             : 6;
} hal_coex_m2s_mode_bitmap_stru;

/* coex mimo业务管理结构体 */
typedef struct {
    osal_u8   s2m_on_6slot         : 1,        /* sco申请回mimo */
            ldac_s2m_on          : 1,        /* ldac申请回mimo */
            a2dp_s2m_on          : 1,        /* a2dp申请回mimo */
            s2m_resv             : 5;
} hal_coex_s2m_mode_bitmap_stru;
#endif

typedef struct {
    osal_s16                           vdet_val;
    osal_u8                           ppa_working;  /* 0:PA；1:PPA */
    osal_u8                           rsv;
    hal_dyn_cali_record_union           record_para;
} hal_dyn_cali_pa_ppa_asjust_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
/* HAL模块和DMAC模块共用的FTM TIME RX结构体 */
typedef struct {
    osal_u8       dialog_token;
    osal_u8       resv[3];
    osal_u64      t2;
    osal_u64      t3;
} hal_wlan_ftm_t2t3_rx_event_stru;

typedef enum {
    DMAC_TX_FTM_FRAME = 0,
    DMAC_TX_FTM_REQ_FRAME = 1,
    DMAC_TX_OTHER_FRAME,
} dmac_tx_ftm_frame_enum;
typedef osal_u8 dmac_tx_ftm_frame_enum_uint8;

typedef struct {
    osal_u8 frame_type;
    osal_u8 dialog_token_ack;
    osal_u8 address[WLAN_MAC_ADDR_LEN];
    osal_u64 t1;
    osal_u64 t4;
} dmac_tx_ftm_frame_stru;
#endif

typedef struct {
    osal_u8                     p2p_noa_status; /* 0: 表示noa 定时器停止，1: 表示noa 定时器正在工作 */
    osal_u8                     auc_resv[3];    /* 预留3字节对齐 */
} hal_p2p_pm_event_stru;

typedef struct {
    osal_u8        bw;     /* 值0代表20M, 值1代表40M, 值2代表80M, 值3代表160M */
    osal_u8        nss;    /* 值0代表单流, 值1代表双流 */
    osal_u8        vap_id;
} hal_to_dmac_he_rom_update_stru;

typedef struct {
    osal_u8 lut_index; /* BA会话LUT session index */
    osal_u8 tid; /* tid num */
    osal_u16 seq_no; /* 第一个未确认的MPDU的序列号 */
    osal_u8 win_size; /* Block_Ack会话的buffer size大小 */
    osal_u8 mmss; /* Min AMPDU Start Spacing */
    wlan_protocol_enum_uint8 user_protocol_mode;
    wlan_protocol_enum_uint8 sta_protocol_mode;
    osal_u8 ba_32bit_flag;
    osal_u8 resv[3]; /* 3BYTE保留字段 */
} machw_tx_ba_params_stru;

typedef struct {
    osal_u8 lut_index;
    osal_u8 tid;
    oal_bool_enum_uint8 qos_flag;
} machw_tx_sequence_stru;

/* 寄存器地址和值配置数据结构 */
typedef struct {
    osal_u32 address;
    osal_u32 value;
} hal_reg32_cfg_stru;

typedef struct {
    osal_u32 address;
    osal_u16 value;
} hal_reg16_cfg_stru;

typedef union {
    osal_u64  trig_para_value;
    struct {
        osal_u32  aid  : 12;                /* STA的关联ID */
        osal_u32  mac_vap_id : 4;
        osal_u32  he_ltf_num : 4;
        osal_u32  doppler : 1;
        osal_u32  midamble_period : 1;
        osal_u32  resv : 10;
        osal_u16  psdu_tx_time;             /* AP允许TB PPDU发送时常,单位us */
        osal_u16  bw             : 2;       /* 上行带宽,0:20M,1:40M,2:80M,3:80+80或160M */
        osal_u16  ltf_gi_type    : 2;       /* AP指定gi */
        osal_u16  ru_type        : 4;       /* AP指定ru size */
        osal_u16  mcs            : 4;       /* AP指定mcs */
        osal_u16  nss            : 4;       /* AP指定nss */
    }basic_trig_para;
} hal_tx_ul_ofdma_para_stru;

typedef struct {
    osal_u64 trigger_type : 4,
           length : 12,
           more_tf : 1,
           cs_required : 1,
           bw : 2,
           gi_and_ltf_type : 2,
           mu_mimo_ltf_mode : 1,
           num_of_he_ltf_syms_and_midamble_period : 3,
           stbc : 1,
           ldpc_extra_symbol_segment : 1,
           ap_tx_power : 6,
           pre_fec_padding_factor : 2,
           pe_disambiguity : 1,
           spatial_reuse : 16,
           doppler : 1,
           he_sig_a2_resv : 9,
           resv : 1;
} hal_trig_common_info_field_stru;

typedef struct {
    osal_u64 aid12 : 12,
           ru_ch_offset : 1, /* 对20MHz,40MHz and 80MHz PPDUs,置0;对80+80 MHz and 160MHz PPDUs,置1 */
           ru_allocation : 7,
           fec_coding_type : 1,
           mcs : 4,
           dcm : 1,
           ss_allocation : 3,
           nss : 3,
           target_rssi : 7,
           resv : 25;
} hal_trig_user_info_field_stru;

typedef struct {
    osal_u8 mu_spacing_factor : 2;
    osal_u8 tid_aggr_limit : 3;
    osal_u8 res1 : 1;
    osal_u8 preferred_ac : 2;
    osal_u8 resv[3];
} hal_trig_user_trig_depend_info_stru;

typedef enum {
    HAL_RF_TRX_TYPE_0,    /* WF0 */
    HAL_RF_TRX_TYPE_1,    /* WF1 */
    HAL_RF_TRX_TYPE_2T2R, /* MIMO双发双收 */
    HAL_RF_TRX_TYPE_1T2R, /* MIMO单发双收 */
    HAL_RF_TRX_TYPE_2T1R, /* MIMO双发单收 */
    HAL_RF_TRX_TYPE_BUTT
} hal_rf_trx_type_enum;  /* 不能调整顺序或增加类型,如果需要,要同步修改hh503_rf_set_stb_mode函数 */
typedef osal_u8 hal_rf_trx_type_enum_uint8;

typedef struct {
    osal_u32  lpf_gain         : 4;
    osal_u32  mod_pga_unit     : 6;
    osal_u32  mod_gm_unit      : 6;
    osal_u32  mod_slice_tpc    : 2;
    osal_u32  pa_unit          : 4;
    osal_u32  pa_slice_idx     : 4;
    osal_u32  resv0            : 6;
} hal_pow_phy_tpc_value_stru;

/* 校准维测命令参数结构体 */
typedef struct {
    osal_u8 cali_mode;  /* 需查询的校准项idx */
    osal_u8 chnl;       /* 物理通道 */
    osal_u8 band;       /* 0:2G 1:5G */
    osal_u8 bw;         /* 带宽 */
    osal_u16 freq;      /* 信道 */
} hal_cali_info_dump_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
