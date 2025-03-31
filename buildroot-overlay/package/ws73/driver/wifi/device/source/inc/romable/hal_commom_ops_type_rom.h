/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header files shared by hal_commom_ops.h and hal_common_ops_rom.h.
 * Create: 2021-09-18
 */

#ifndef __HAL_COMMOM_OPS_TYPE_ROM_H__
#define __HAL_COMMOM_OPS_TYPE_ROM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    枚举定义
*****************************************************************************/
typedef enum {
    HAL_TX_QUEUE_BE        = 0,        /* 尽力而为业务 */
    HAL_TX_QUEUE_BK        = 1,        /* 背景业务 */
    HAL_TX_QUEUE_VI        = 2,        /* 视频业务 */
    HAL_TX_QUEUE_VO        = 3,        /* 语音业务 */
    HAL_TX_QUEUE_HI        = 4,        /* 高优先级队列(管理帧/控制帧用此队列) */
    HAL_TX_QUEUE_MC        = 5,        /* 组播帧发送队列 */
    HAL_TX_QUEUE_BUTT
} hal_tx_queue_type_enum;
typedef osal_u8 hal_tx_queue_type_enum_uint8;

typedef enum {
    HAL_BTCOEX_SW_POWSAVE_IDLE       = 0,
    HAL_BTCOEX_SW_POWSAVE_WORK       = 1,
    HAL_BTCOEX_SW_POWSAVE_TIMEOUT    = 2,
    HAL_BTCOEX_SW_POWSAVE_SCAN       = 3,
    HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN = 4,
    HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT  = 5,
    HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT = 6,
    HAL_BTCOEX_SW_POWSAVE_SCAN_END   = 7,
    HAL_BTCOEX_SW_POWSAVE_PSM_START  = 8,
    HAL_BTCOEX_SW_POWSAVE_PSM_END    = 9,
    HAL_BTCOEX_SW_POWSAVE_PSM_STOP   = 10,

    HAL_BTCOEX_SW_POWSAVE_BUTT
} hal_coex_sw_preempt_type;
typedef osal_u8 hal_coex_sw_preempt_type_uint8;

typedef enum {
    HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE   = 0,
    HAL_BTCOEX_SW_POWSAVE_SUB_IDLE     = 1,
    HAL_BTCOEX_SW_POWSAVE_SUB_SCAN     = 2,
    HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT  = 3,
    HAL_BTCOEX_SW_POWSAVE_SUB_PSM_FORBIT  = 4, /* 低功耗唤醒时,连续出现多次ps=1状态,要禁止psm 做状态判断, 直到soc中断来更新 */

    HAL_BTCOEX_SW_POWSAVE_SUB_BUTT
} hal_coex_sw_preempt_subtype_enum;
typedef osal_u8 hal_coex_sw_preempt_subtype_uint8;

/*****************************************************************************
    枚举定义
*****************************************************************************/
typedef enum {
    HAL_FCS_PROTECT_TYPE_NONE      = 0,    /* NONE        */
    HAL_FCS_PROTECT_TYPE_SELF_CTS,         /* SELF CTS    */
    HAL_FCS_PROTECT_TYPE_NULL_DATA,        /* NULL DATA   */

    HAL_FCS_PROTECT_TYPE_BUTT
} hal_fcs_protect_type_enum;
typedef osal_u8 hal_fcs_protect_type_enum_uint8;

typedef enum {
    HAL_FCS_SERVICE_TYPE_DBAC      = 0,    /* DBAC业务    */
    HAL_FCS_SERVICE_TYPE_SCAN,             /* 扫描业务    */
    HAL_FCS_SERVICE_TYPE_M2S,              /* m2s切换业务 */
    HAL_FCS_SERVICE_TYPE_BTCOEX_NORMAL,    /* btcoex共存业务 */
    HAL_FCS_SERVICE_TYPE_BTCOEX_LDAC,      /* btcoex共存业务 */

    HAL_FCS_PROTECT_NOTIFY_BUTT
} hal_fcs_service_type_enum;
typedef osal_u8 hal_fcs_service_type_enum_uint8;

typedef enum {
    HAL_FCS_PROTECT_COEX_PRI_NORMAL   = 0,    /* b00 */
    HAL_FCS_PROTECT_COEX_PRI_PRIORITY = 1,    /* b01 */
    HAL_FCS_PROTECT_COEX_PRI_OCCUPIED = 2,    /* b10 */

    HAL_FCS_PROTECT_COEX_PRI_BUTT
} hal_fcs_protect_coex_pri_enum;
typedef osal_u8 hal_fcs_protect_coex_pri_enum_uint8;

typedef enum {
    HAL_FCS_PROTECT_CNT_1 = 1,    /* 1 */
    HAL_FCS_PROTECT_CNT_2 = 2,    /* 2 */
    HAL_FCS_PROTECT_CNT_3 = 3,    /* 3 */
    HAL_FCS_PROTECT_CNT_6 = 6,    /* 10 */
    HAL_FCS_PROTECT_CNT_10 = 10,    /* 10 */
    HAL_FCS_PROTECT_CNT_20 = 20,    /* 20 */

    HAL_FCS_PROTECT_CNT_BUTT
} hal_fcs_protect_cnt_enum;
typedef osal_u8 hal_fcs_protect_cnt_enum_uint8;

/*****************************************************************************
    描述符相关枚举定义
*****************************************************************************/
typedef enum {
    HAL_DFS_RADAR_TYPE_NULL  = 0,
    HAL_DFS_RADAR_TYPE_FCC   = 1,
    HAL_DFS_RADAR_TYPE_ETSI  = 2,
    HAL_DFS_RADAR_TYPE_MKK   = 3,
    HAL_DFS_RADAR_TYPE_KOREA = 4,

    HAL_DFS_RADAR_TYPE_BUTT
} hal_dfs_radar_type_enum;
typedef osal_u8 hal_dfs_radar_type_enum_uint8;

/* 接收描述符队列号 */
typedef enum {
    HAL_RX_DSCR_NORMAL_QUEUE = 0,
    HAL_RX_DSCR_HIGH_PRI_QUEUE,
    HAL_RX_DSCR_SMALL_QUEUE,
    HAL_RX_DSCR_QUEUE_ID_BUTT
} hal_rx_dscr_queue_id_enum;
typedef osal_u8 hal_rx_dscr_queue_id_enum_uint8;

/*****************************************************************************
    中断相关枚举定义
*****************************************************************************/
/* 因为mac error和dmac misc优先级一致，03将high prio做实时事件队列来处理，mac error并入dmac misc */
/* 实时事件中断类型 */
typedef enum {
    HAL_EVENT_DMAC_HIGH_PRIO_BTCOEX_PS,        /* BTCOEX ps中断, 因为rom化，目前只能放置一个 */
    HAL_EVENT_DMAC_HIGH_PRIO_BTCOEX_LDAC,      /* BTCOEX LDAC中断 */

    HAL_EVENT_DMAC_HIGH_PRIO_SUB_TYPE_BUTT
} hal_event_dmac_high_prio_sub_type_enum;
typedef osal_u8 hal_event_dmac_high_prio_sub_type_enum_uint8;

/*****************************************************************************
    复位相关枚举定义
*****************************************************************************/
/* 复位事件子类型枚举 */
typedef enum {
    HAL_RESET_HW_TYPE_ALL = 0,
    HAL_RESET_HW_TYPE_PHY,
    HAL_RESET_HW_TYPE_MAC,
    HAL_RESET_HW_TYPE_MAC_PHY,

    HAL_RESET_HW_TYPE_BUTT
} hal_reset_hw_type_enum;
typedef osal_u8 hal_reset_hw_type_enum_uint8;

typedef enum {
    HAL_ALG_ISR_NOTIFY_DBAC,
    HAL_ALG_ISR_NOTIFY_MWO_DET,
    HAL_ALG_ISR_NOTIFY_ANTI_INTF,

    HAL_ALG_ISR_NOTIFY_BUTT,
} hal_alg_noify_enum;
typedef osal_u8 hal_alg_noify_enum_uint8;

typedef enum {
    HAL_ISR_TYPE_TBTT,
    HAL_ISR_TYPE_ONE_PKT,
    HAL_ISR_TYPE_MWO_DET,
    HAL_ISR_TYPE_NOA_START,
    HAL_ISR_TYPE_NOA_END,

    HAL_ISR_TYPE_BUTT,
} hal_isr_type_enum;
typedef osal_u8 hal_isr_type_enum_uint8;

/* 性能测试相关枚举 */
typedef enum {
    HAL_ALWAYS_TX_DISABLE,         /* 禁用常发 */
    HAL_ALWAYS_TX_RF,              /* 保留给RF测试广播报文 */
    HAL_ALWAYS_TX_AMPDU_ENABLE,    /* 使能AMPDU聚合包常发 */
    HAL_ALWAYS_TX_MPDU,            /* 使能非聚合包常发 */
    HAL_ALWAYS_TX_DC,              /* 使能DC常发,用于CE认证测频偏 */
    HAL_ALWAYS_TX_BUTT
} hal_device_always_tx_state_enum;
typedef osal_u8 hal_device_always_tx_enum_uint8;

typedef enum {
    HAL_ALWAYS_RX_DISABLE,         /* 禁用常收 */
    HAL_ALWAYS_RX_RESERVED,        /* 保留给RF测试广播报文 */
    HAL_ALWAYS_RX_AMPDU_ENABLE,    /* 使能AMPDU聚合包常收 */
    HAL_ALWAYS_RX_ENABLE,          /* 使能非聚合包常收 */
    HAL_ALWAYS_RX_BUTT
} hal_device_always_rx_state_enum;
typedef osal_u8 hal_device_always_rx_enum_uint8;

/*****************************************************************************
    加密相关枚举定义
*****************************************************************************/
/****3.6.1  芯片密钥类型定义 ************************************************/
typedef enum {
    HAL_KEY_TYPE_TX_GTK              = 0,
    HAL_KEY_TYPE_PTK                 = 1,
    HAL_KEY_TYPE_RX_GTK              = 2,
    HAL_KEY_TYPE_RX_GTK2             = 3,       /* 02使用，03和51不使用 */
    HAL_KEY_TYPE_BUTT
} hal_cipher_key_type_enum;
typedef osal_u8 hal_cipher_key_type_enum_uint8;

/*****************************************************************************
    对外部保留的设备级接口列表，建议外部不做直接调用，而是调用对应的内联函数
*****************************************************************************/

typedef enum {
    HAL_DFR_TIMER_STEP_1 = 0,
    HAL_DFR_TIMER_STEP_2 = 1,
} hal_dfr_timer_step_enum;
typedef osal_u8 hal_dfr_timer_step_enum_uint8;

typedef struct {
    osal_u32          tkipccmp_rep_fail_cnt;    /* 重放攻击检测计数TKIP + CCMP */
    osal_u32          tx_mpdu_cnt;              /* 发送计数非ampdu高优先级 + 普通优先级 + ampdu中mpdu */
    osal_u32          rx_passed_mpdu_cnt;       /* 属于AMPDU MPDU的FCS正确的MPDU数量 */
    osal_u32          rx_failed_mpdu_cnt;       /* 属于AMPDU MPDU的FCS错误的MPDU数量 */
    osal_u32          rx_tkipccmp_mic_fail_cnt; /* kip mic + ccmp mic fail的帧数 */
    osal_u32          key_search_fail_cnt;      /* 接收key serach fail的帧数 */
    osal_u32          phy_rx_dotb_ok_frm_cnt;       /* PHY接收dotb ok的帧个数 */
    osal_u32          phy_rx_htvht_ok_frm_cnt;      /* PHY接收vht ht ok的帧个数 */
    osal_u32          phy_rx_lega_ok_frm_cnt;       /* PHY接收legace ok的帧个数 */
    osal_u32          phy_rx_dotb_err_frm_cnt;      /* PHY接收dotb err的帧个数 */
    osal_u32          phy_rx_htvht_err_frm_cnt;     /* PHY接收vht ht err的帧个数 */
    osal_u32          phy_rx_lega_err_frm_cnt;      /* PHY接收legace err的帧个数 */
} hal_mac_key_statis_info_stru;

typedef struct {
    osal_u32          rx_ampdu_cnt;                 /* hw接收apmdu报文数量 */
    osal_u32          rx_suc_mpdu_inampdu_cnt;      /* hw接收apmdu中mpdu fcs正确报文数量 */
    osal_u32          rx_err_mpdu_inampdu_cnt;      /* hw接收apmdu中mpdu fcs错误报文数量 */
    osal_u32          rx_suc_mpdu_cnt;              /* hw接收mpdu fcs正确报文数量 */
    osal_u32          rx_err_mpdu_cnt;              /* hw接收mpdu fcs错误报文数量 */
    osal_u32          rx_filter_mpdu_cnt;
} hal_mac_rx_mpdu_statis_info_stru;
typedef struct {
    osal_u32          tx_high_priority_mpdu_cnt;    /* hw中高优先级队列中MPDU发送数量 */
    osal_u32          tx_normal_priority_mpdu_cnt;  /* hw中普通优先级队列中MPDU发送数量 */
    osal_u32          tx_ampdu_inmpdu_cnt;          /* hw中发送的AMPDU中MPDU数量 */
    osal_u32          tx_ampdu_cnt;                 /* hw中发送的AMPDU数量 */
    osal_u32          tx_complete_cnt;              /* hw中发送完成中断数量 */
} hal_mac_tx_mpdu_statis_info_stru;

/* 扫描状态，通过判断当前扫描的状态，判断多个扫描请求的处理策略以及上报扫描结果的策略 */
typedef enum {
    MAC_SCAN_STATE_IDLE,
    MAC_SCAN_STATE_RUNNING,

    MAC_SCAN_STATE_BUTT
} mac_scan_state_enum;
typedef osal_u8 mac_scan_state_enum_uint8;

/* 扫描状态，打印tx dscr控制 */
typedef enum {
    HAL_SCAN_TX_DSCR_DBG_CTL_INIT,
    HAL_SCAN_TX_DSCR_DBG_CTL_PRINT_PERMIT,
    HAL_SCAN_TX_DSCR_DBG_CTL_PRINTED,
    HAL_SCAN_TX_DSCR_DBG_CTL_BUTT
} hal_scan_tx_dscr_dbg_ctl_enum;
typedef osal_u8 hal_scan_tx_dscr_dbg_ctl_enum_uint8;

/* 干扰检测模块检测到的邻频叠频干扰模式改变时回调函数枚举定义 */
typedef enum {
    HAL_ALG_INTF_DET_ADJINTF_NO        = 0,    /* 无干扰 */
    HAL_ALG_INTF_DET_ADJINTF_MEDIUM    = 1,    /* 中等强度干扰 */
    HAL_ALG_INTF_DET_ADJINTF_STRONG    = 2,    /* 强干扰 */
    HAL_ALG_INTF_DET_ADJINTF_CERTIFY   = 3,    /* 认证干扰 */
    HAL_ALG_INTF_DET_STATE_PKADJ       = 4,    /* PK参数调整状态 */
    HAL_ALG_INTF_DET_STATE_PK          = 5,    /* PK状态 */
    HAL_ALG_INTF_DET_ADJINTF_BUTT
} hal_alg_intf_det_mode_enum;
typedef osal_u8 hal_alg_intf_det_mode_enum_uint8;

/* 算法使用的用户距离状态 */
typedef enum {
    HAL_ALG_USER_DISTANCE_NEAR       = 0,
    HAL_ALG_USER_DISTANCE_NORMAL     = 1,
    HAL_ALG_USER_DISTANCE_FAR        = 2,

    HAL_ALG_USER_DISTANCE_BUTT
} hal_alg_user_distance_enum;
typedef osal_u8 hal_alg_user_distance_enum_uint8;

typedef union {
    struct {
        osal_u32 netbuf_reserve  : 8;     /* 调度：需要预留的大包个数 */
        osal_u32 free_large_cnt  : 8;     /* 调度：实际空闲的大包个数 */
        osal_u32 free_short_cnt  : 6;     /* 调度：实际空闲的小包个数 */
        osal_u32 free_mgmt_cnt   : 6;     /* 调度：实际空闲的管理帧个数 */
        osal_u32 sdp_chnl_switch_off : 1; /* SDP：判断wifi aware是否切离STA 工作信道 */
        osal_u32 slp_tx_ctrl : 1;         /* slp: 判断slp状态 */
        osal_u32 resv        : 2;
    } bits;

    osal_u32 u32;
} hal_gp_reg_0_u;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/* ps mode管理结构体 */
typedef struct {
    osal_u8   ps_on                 : 1,        /* ps软件机制: 0=关闭, 1=打开 */
              delba_on              : 1,        /* 删减聚合逻辑: 0=关闭, 1=打开 */
              reply_cts             : 1,        /* 是否回复CTS， 0=不回复， 1=回复 */
              rsp_frame_ps          : 1,        /* resp帧节能位是否设置 0=不设置， 1=设置 */
              btcoex_wl0_tx_slv_on  : 1,         /* btcoex下wl0 tx slv配置开关 */
              bit_resv              : 3;
} hal_coex_sw_preempt_mode_stru;
#endif

typedef struct {
    osal_u8 rf_id;
    wlan_channel_band_enum_uint8 band;
    osal_u8 chan_idx;
    wlan_channel_bandwidth_enum_uint8 bandwidth;
}hal_rf_chan_info;

/* MIMO/SISO切换业务bitmap */
typedef enum {
    HAL_M2S_MODE_BITMAP_6SLOT = BIT0,         /* 6slot */
    HAL_M2S_MODE_BITMAP_LDAC  = BIT1,         /* ldac */
    HAL_M2S_MODE_BITMAP_MSS   = BIT2,         /* mss */
    HAL_M2S_MODE_BITMAP_2GAP  = BIT3,         /* 2g ap切siso */
    HAL_M2S_MODE_BITMAP_5GAP  = BIT4,         /* 5g ap切siso */

    HAL_M2S_MODE_BITMAP_BUTT                 /* 最大状态 */
} hal_m2s_mode_bitmap;
typedef osal_u8 hal_m2s_mode_bitmap_uint8;
/* TPC档位设置 */
#define HAL_POW_LEVEL_NUM               6                       /* 算法总档位数目 */
#define HAL_POW_ADJUST_LEVEL_NUM        (HAL_POW_LEVEL_NUM - 1)   /* 算法支持可调整的档位数目 */
#define HAL_POW_MAX_POW_LEVEL           0                       /* 算法的最大功率档位 */
#define HAL_POW_MIN_POW_LEVEL           (HAL_POW_LEVEL_NUM - 2)         /* 算法的最小功率档位 */
#define HAL_POW_LEVEL_DOWN_LIMIT        (HAL_POW_ADJUST_LEVEL_NUM - 1)    /* 功率衰减的最小档位 */
#define HAL_POW_LEVEL_UP_LIMIT          (HAL_POW_MAX_POW_LEVEL)           /* 功率回升的最大档位 */
#define HAL_POW_RF_LIMIT_POW_LEVEL      (HAL_POW_LEVEL_NUM - 1)    /* 极远距离的功率档位标识 */
#define HAL_POW_INVALID_POW_LEVEL        0xff        /* 无效功率档位 */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_commom_ops_type.h */
