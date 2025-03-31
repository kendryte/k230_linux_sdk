/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: autorate common
 */

#ifndef __ALG_AUTORATE_COMMON_ROM_H__
#define __ALG_AUTORATE_COMMON_ROM_H__

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  1 宏定义
*****************************************************************************/
#define ALG_AUTOARTE_EACH_RATE_MIN_TX_COUNT 3     /* 每个速率等级的最小重传次数 */

#define ALG_AR_INITIAL_STAGE_PKT_CNT_THRD    3    /* 初始阶段发包个数门限 */
#define ALG_AUTORATE_CONSECUTIVE_ALL_FAIL_DEFAULT_THRD  5  /* 连续所有rank发送失败的PPDU包默认容忍个数门限 */
#define ALG_AUTORATE_COLLISION_TOLERANT_PPDU_CNT_THRD  3   /* 容忍疑似碰撞的PPDU个数阈值 */
#define ALG_AUTORATE_COLLISION_INSTANT_PER_THRD        150 /* 疑似碰撞 instant PER门限值 */
#define ALG_AUTORATE_RATE_AGING_TIME_MS         5000       /* 速率老化的时间门限(单位:ms) */
#define ALG_AUTORATE_PROBE_AGING_TIME_MS        1500       /* 探测老化时间门限 */
#define ALG_AUTORATE_SHORT_STAT_SHIFT           3          /* 默认短期统计的除2位移值 */
#define ALG_AUTORATE_LONG_STAT_SHIFT            4          /* 默认长期统计的除2位移值 */
#define ALG_AUTORATE_NON_AGGR_SMOOTH_SCALE      1   /* 非聚合的统计量平滑缩放比例(N表示非聚合相对于聚合, 平滑窗口乘2^N) */
#define ALG_AUTORATE_DELTA_GOODPUT_RATIO        200        /* 默认goodput突变的比例(%) */
#define ALG_AUTORATE_DELTA_BAD_GOODPUT_RATIO    150        /* 默认goodput突变不好的比例(%) */
#define ALG_AUTORATE_MCS_UP_RESERVE_RATE_BETTER_PER_THRD 250 /* phase1 向上探测预留速率判别变优的PER门限 */
#define ALG_AR_PROBE_DIFF_NSS_GAIN_TH         50            /* 不同nss之间探测时，增益千分比超过门限才会切换 */
#define ALG_AR_PROBE_SAME_NSS_GI_GAIN_TH      15           /* 相同nss GI，不同mcs之间探测时，增益千分比超过门限才会切换 */
#define ALG_AR_GI_GOODPUT_GAIN_TH             50            /* GI切换时, 增益阈值 */
#define ALG_AR_PROBE_OTHER_NSS_GAIN_BEYOND_BEST_NSS_TH 20   /* OTHER_NSS 需要优于 BEST_NSS 千分比才可切换 */
#define ALG_AUTORATE_DESCEND_PROTOCOL_PER_TH    1024        /* 降协议PER门限值 */
#define ALG_AUTORATE_DESCEND_PROTOCOL_PER_TH_INTRF_MODE 300 /* 抗干扰模式下降协议PER门限值 */
#define ALG_AUTORATE_11B_ASCEND_PROTOCOL_PER_TH 300         /* 11b升协议PER门限值 */
#define ALG_AUTORATE_11A_ASCEND_PROTOCOL_PER_TH 100         /* 11a升协议PER门限值 */
#define ALG_AUTORATE_11B_DESCEND_PROTOCOL_RSSI_TH    (-70)  /* 降11b协议，rssi门限 */
#define ALG_AUTORATE_INTF_ADJ_MIN_TH 100                    /* 邻频、叠频干扰繁忙度最小门限 */
#define ALG_AUTORATE_INTF_ADJ_MAX_TH 500                    /* 邻频、叠频干扰繁忙度最大门限 */
#define ALG_AUTORATE_DESCEND_BW_GDPT_BETTER_THRD 50         /* 自动带宽特性降带宽探测速率较最优速率gdpt优千分数阈值 */
#define ALG_AUTORATE_ASCEND_BW_GDPT_BETTER_THRD  30         /* 自动带宽特性升带宽探测速率较最优速率gdpt优千分数阈值 */
#define ALG_AUTORATE_DESCEND_BW_EVENT_TRIG_PER_THRD 500     /* 事件触发降带宽探测PER门限 */
#define ALG_AUTORATE_DESCEND_BW_MAX_EVENT_TRIG_PKTCNT_THRD   5  /* 事件触发降带宽探测计数阈值，超过去使能事件触发降带宽 */
#define ALG_AUTORATE_ASCEND_BW_RATE_KEEPCNT_THRD    3       /* 升带宽速率较优保持计数门限 */
#define ALG_AUTORATE_DESCEND_BW_RATE_KEEPCNT_THRD   3       /* 降带宽速率较优保持计数门限 */
#define ALG_AUTORATE_BW_MIN_PROBE_INTVL_PKTNUM      16      /* 带宽最小探测包间隔门限 */
#define ALG_AUTORATE_MIN_PROBE_INTVL_PKTNUM             8   /* 默认最小探测间隔包数目 */
#define ALG_AUTORATE_MAX_PROBE_INTVL_PKTNUM             128 /* 默认最大探测间隔包数目 */
#define ALG_AUTORATE_PROBE_INTVL_KEEP_TIMES             32  /* 默认探测间隔保持的次数 */
#define ALG_AR_INITIAL_STAGE_RSSI_TOLERANT              10  /* 根据rssi估计用户最优速率，rssi回退值, \
                                                               FPGA上板实测 -51db最高mcs6能通, backoff 15db */
#define ALG_AUTORATE_PROBE_CROSS_BW_MAX_MCS_DIFF        3   /* 自动带宽探测mcs最大差值 */
#define ALG_AUTORATE_PROBE_UP_SHORT_GI_STOP_PROBE_PER_THRD 600 /* 升mcs探测short gi时，首个探测点per大于门限，停止探测 */
#define ALG_AR_PROBE_DOWN_CONTINUE_PER_TH               1000 /* 超过该阈值会继续向下探测 */
#define ALG_AUTORATE_PROBE_DOWN_PER_TH                  500 /* 立即向下探测的PER门限(单位:千分数) */
#define ALG_AR_CROSS_NSS_NEED_ADJUST_RATE_CNT 5
#define ALG_AR_RATE_BAD_THRD 700
#define ALG_AUTORATE_MCS_UP_RESERVE_RATE_BETTER_PER_THRD 250 /* mcs 向上探测预留速率判别变优的PER门限 */
#define ALG_AUTORATE_MCS_UP_RESERVE_RATE_BETTER_PROBE_THRD 32 /* mcs 向上探测预留速率判别变优下的探测门限上限 */
#define ALG_AUTORATE_MAX_PROBE_WAIT_COUNT       100 /* 等待探测结果的最大发送完成中断次数(避免中断丢失导致状态异常) */
#define ALG_AUTORATE_PROBE_MIN_MPDU_NUM         4   /* mcs探测速率发送最小mpdu数 */

/* SMOOTH PER */
#define ALG_AUTORATE_NROMAL_PKT_INIT_PER_LIMIT 200  /* 非聚合包 PER为0 或 老化后 首次更新PER上限限制值 */
#define ALG_AUTORATE_AGGR_PKT_INIT_PER_LIMIT   100  /* 聚合包 PER为0 或 老化后 聚合个数不多于2 首次更新PER上限限制值 */
#define ALG_AUTORATE_AGGR_MIN_MPDU_CNT_THRD    2    /* 聚合模式下 MPDU 数下限 */
#define ALG_AR_SMALL_AMPDU_SHORT_PER_SMOOTH    3    /* 聚合个数小于门限(2)的short per 平滑系数 */
#define ALG_AR_SMALL_AMPDU_LONG_PER_SMOOTH     4    /* 聚合个数小于门限(2)的long per 平滑系数 */
#define ALG_AR_SMOOTH_PER_OPT_TH               500  /* 采用固定参数方式平滑per，short per需要达到的阈值 */
#define ALG_AR_SMOOTH_PER_OPT_DIFF_TH          100  /* 采用固定参数方式平滑per，instant per比short per大的阈值 */
#define ALG_AR_SMOOTH_PER_OPT_SHORT_CHANGE     100  /* 采用固定参数方式平滑per，short per的平滑值 */
#define ALG_AR_SMOOTH_PER_OPT_LONG_CHANGE      50   /* 采用固定参数方式平滑per，long per的平滑值 */
#define ALG_AR_SMOOTH_PER_BAD_TH               1000 /* 平滑之后的per超过该阈值会直接使用1024 */
#define ALG_AUTORATE_PROBE_MCS_LARGE_AGGR_SMOOTH_PARAM 2  /* mcs高聚合探测帧per平滑系数 */
#define ALG_AUTORATE_PROBE_MCS_SMALL_AGGR_SMOOTH_PARAM 3  /* mcs少探测帧per平滑系数 */
#define ALG_AUTORATE_PROBE_BW_PER_SMOOTH_PARAM         2  /* bw探测帧per平滑系数 */
#define ALG_AUTORATE_PROBE_PROTOCOL_PER_SMOOTH_PARAM   1  /* protocol探测帧per平滑系数 */
#define ALG_AR_PROBE_PER_BETTER_TH              200  /* 探测时per变好的阈值 */
#define ALG_AUTORATE_PROBE_SMOOTH_RECORD_PER_TH 50   /* 探测过程中, 平滑rate_record的PER门限值(千分数) */
#define ALG_AR_RSSI_SUDDEN_PER_TH               200  /* rssi突降5db以上，固定参数平滑short per */
#define ALG_AR_RSSI_SUDDEN_TH                   5    /* rssi突降门限 */

/*****************************************************************************
  2 枚举定义
*****************************************************************************/
typedef enum {
    ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_WITH_PRE_PROBE,
    ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_WITH_EXISTING_RATE_SET_INFO,
    ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_TX_INSTANT_RESULT,
    ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_WITH_SNR_PER_CURVE,

    ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_BUTT
} alg_autorate_probe_per_init_strategy_enum;
typedef osal_u8 alg_autorate_probe_per_init_strategy_enum_uint8;
/*****************************************************************************
  3 STRUCT定义
*****************************************************************************/
/* 每个rank的固定速率信息 */
typedef struct {
    osal_u32   bit_protocol_mode : 3;    /* wlan_phy_protocol_enum */
    osal_u32   bit_freq_bw : 3;          /* hal_channel_assemble_enum */
    osal_u32   bit_preamble : 1;         /* wlan_phy_preamble_type */
    osal_u32   bit_fec_coding : 1;       /* wlan_channel_code_enum */
    osal_u32   bit_gi_type : 2;          /* wlan_phy_he_gi_type_enum_uint8 */
    osal_u32   bit_ltf_type : 2;         /* wlan_phy_he_ltf_type_enum */
    osal_u32   bit_tx_count : 3;         /* 发送次数 */
    osal_u32   bit_ppdu_dcm : 1;         /* wlan_phy_dcm_status */
    osal_u32   bit_nss_rate : 6;         /* wlan_phy_rate_enum */
    osal_u32   bit_resv : 10;
} alg_autorate_tx_fix_rate_rank_stru;

/* 统计信息结构体 */
typedef struct {
    osal_u16   short_per;                          /* 该速率短期平均的PER(千分数) */
    osal_u16   long_per;                           /* 该速率长期平均的PER(千分数) */
    osal_u32   rate_last_used_time_ms;             /* rate级别最后的使用的时间 */
    osal_u32   theory_goodput;                     /* 对应带宽下该速率的理论goodput */
} alg_autorate_rate_stat_info_stru;

/* 固定速率结构体 */
typedef struct {
    alg_autorate_tx_fix_rate_rank_stru tx_rate[HAL_TX_RATE_MAX_NUM];
    alg_autorate_rate_stat_info_stru rate_stat[HAL_TX_RATE_MAX_NUM];
} alg_autorate_fix_rate_stru;

/* 速率自适应算法 DEV级别 结构体 */
typedef struct {
    /* 功能开关 */
    osal_u8 enable;                     /* 1：算法使能  , 0：算法不使能 */
    osal_u8 enable_cross_protocol;      /* 是否使能动态协议策略 */
    osal_u8 enable_auto_bw;             /* 是否使能自动带宽 */
    osal_u8 enable_event_trig_descend_bw; /* 是否使能事件触发降带宽开关 */
    osal_u8 fix_rate_mode;              /* 1：固定速率  , 0：自动速率 */
    osal_u8 initial_stage_pkt_cnt_thrd; /* 初始阶段发包个数门限 */
    osal_u8 enable_ar_log;              /* ar_stat log record enable */

    /* 速率统计相关 */
    osal_u8   short_stat_shift;             /* 短期统计的平滑因子 */
    osal_u8   long_stat_shift;              /* 长期统计的平滑因子 */
    osal_u8   non_aggr_smooth_scale;        /* 非聚合统计量平滑缩放比例 */

    /* 辅20忙识别相关，用于事件触发降带宽 */
    osal_u16  intf_adj_min_th;              /* 邻频、叠频干扰繁忙度最小阈值 */
    osal_u16  intf_adj_max_th;              /* 邻频、叠频干扰繁忙度最大阈值 */
    osal_u8   sec20_is_busy;                /* 辅20忙 */

    /* 碰撞检测相关 */
    osal_u8    max_probe_wait_cnt;        /* 等待探测结果的最大发送完成中断次数(避免中断丢失导致状态异常) */
    osal_u8    consecutive_all_fail_th;   /* 连续所有rank发送失败的PPDU包容忍个数 */
    osal_u8    collision_tolerant_ppdu_cnt_thrd; /* 容忍疑似碰撞的PPDU个数阈值, 越大越稳定, 但反应随之减缓 */
    osal_u16   collision_instant_per_th; /* 疑似碰撞 instant PER门限值 */

    /* 探测相关 */
    alg_autorate_probe_per_init_strategy_enum_uint8 en_probe_per_init_strategy; /* 探测速率初始PER赋值策略 */
    osal_u8   mcs_up_resv_rate_better_probe_thrd;  /* mcs 向上探测预留速率判别变优下的探测门限上限 */
    osal_u8   max_probe_intvl_pktnum;           /* 最大探测间隔包数目,后需要用宏定义替代，不再定义结构体成员 */
    osal_u8   min_probe_intvl_pktnum;      /* 最小探测间隔包数目,后需要用宏定义替代，不再定义结构体成员 */

    osal_u8   probe_intvl_keep_times;      /* 默认探测间隔保持的次数 */
    osal_u8     rssi_sudden_th;             /* rssi突降门限 */
    osal_u16    rssi_sudden_per_th;        /* rssi突降5db以上，固定参数平滑short per */

    osal_u16    probe_down_per_th;              /* 立即向下探测的PER门限(单位:千分数) */
    osal_u16    sudden_good_delta_gdpt_ratio;   /* 事件触发向上探测 gdpt 变化比例 */
    osal_u16    sudden_bad_delta_gdpt_ratio;    /* 事件触发向下探测 gdpt 变化比例 */
    osal_u16    continue_probe_down_per_th;     /* 超过该阈值会继续向下探测 */

    osal_u16    cross_gi_probe_stop_per_th;     /* 升mcs探测short gi时，首个探测点per大于门限，停止探测 */
    osal_u16    normal_pkt_init_per_limit;      /* 非聚合包 PER为0 或 老化后 首次更新PER上限限制值 */
    osal_u16    aggr_pkt_init_per_limit;        /* 聚合包 PER为0 或 老化后 聚合个数不多于2 首次更新PER上限限制值 */
    osal_u16    smooth_per_opt_th;              /* 采用固定参数方式平滑per，short per需要达到的阈值 */

    osal_u16    smooth_per_opt_diff_th;         /* 采用固定参数方式平滑per，instant per比short per大的阈值 */
    osal_u16    smooth_per_opt_th_short_change; /* 采用固定参数方式平滑per，short per的平滑值 */
    osal_u16    smooth_per_opt_th_long_change;  /* 采用固定参数方式平滑per，long per的平滑值 */
    osal_u16    smooth_per_bad_th;              /* 平滑之后的per超过该阈值会直接使用1024 */

    osal_u16    probe_per_better_th;            /* 探测时per变好的阈值 */
    osal_u16    mcs_up_reserve_rate_better_per_thrd; /* phase1 向上探测预留速率判别变优的PER门限 */
    osal_u16    probe_best_update_same_nss_gi_th;    /* 相同nss gi之间探测时切换mcs的增益阈值(千分比) */
    osal_u16    probe_best_update_diff_nss_th;  /* 不同nss之间探测时切换mcs的增益阈值(千分比) */

    osal_u16    probe_best_update_other_nss_gain_th; /* OTHER_NSS 需要优于 BEST_NSS 千分比才可切换 */
    osal_u16    probe_best_update_gi_th;         /* 相同nss之间探测时切换gi的增益阈值(千分比) */
    osal_u16    probe_smooth_record_per_th;     /* 探测过程中, 平滑rate_record的PER门限值(千分数) */
    osal_u16   descend_protocol_per_th;         /* 降协议PER门限值 */

    osal_u16   ar_11b_ascend_protocol_per_th;   /* 11b升协议PER门限值 */
    osal_u16   ar_11a_ascend_protocol_per_th;   /* 11a升协议PER门限值 */

    hal_alg_intf_det_mode_enum_uint8    adjch_intf_type; /* 邻叠频干扰状态 */
    osal_s8     descend_protocol_11b_rssi_th;   /* 11b降协议RSSI门限 */
    osal_u8     aggr_min_mpdu_cnt_thrd;         /* 聚合模式下 MPDU 数下限 */
    osal_u8     small_ampdu_short_per_smooth;   /* 聚合个数小于门限(2)的short per 平滑系数 */
    osal_u8     small_ampdu_long_per_smooth;    /* 聚合个数小于门限(2)的long per 平滑系数 */
    osal_u8     probe_mcs_large_aggr_smooth;    /* mcs高聚合探测帧per平滑系数 */
    osal_u8     probe_mcs_small_aggr_smooth;    /* mcs少探测帧per平滑系数 */
    osal_u8     probe_bw_per_smooth;            /* bw探测帧per平滑系数 */

    osal_u8    probe_protocol_per_smooth;      /* protocol探测帧per平滑系数 */
    osal_u8    initial_stage_rssi_tolerant;    /* 根据rssi估计用户最优速率，rssi回退值 */
    osal_u8    descend_bw_event_trig_cnt_thrd;    /* 事件触发降带宽探测计数阈值，超过阈值去使能事件触发降带宽 */
    osal_u8    ascend_bw_rate_keep_cnt_thrd;      /* 升带宽速率较优保持计数门限 */

    osal_u8    descend_bw_rate_keep_cnt_thrd;     /* 降带宽速率较优保持计数门限 */
    osal_u8    bw_probe_pktcnt_min_thrd;          /* 带宽最小探测包间隔门限 */
    osal_u16   descend_bw_gdpt_better_thrd;       /* 自动带宽特性降带宽探测速率较最优速率gdpt优千分数阈值 */
    osal_u16   ascend_bw_gdpt_better_thrd;        /* 自动带宽特性升带宽探测速率较最优速率gdpt优千分数阈值 */
    osal_u16   descend_bw_event_trig_per_thrd;    /* 事件触发降带宽探测PER门限 */
    osal_u16   probe_min_mpdu_num;                /* mcs探测速率发送最小mpdu数 */
    osal_u16   rev;
    /* 速率统计相关 */
    osal_u32  rate_aging_time_ms;               /* 速率老化时间(单位:ms) */
    osal_u32  stat_aging_time;                     /* 探测老化时间门限 */

    alg_autorate_tx_fix_rate_rank_stru slp_frame_rate[HAL_TX_RATE_MAX_NUM]; /* slp帧类型使用的速率 */

    /* 固定速率统计参数 */
    alg_autorate_fix_rate_stru  fix_rate;
} alg_autorate_dev_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_autorate_common.h */
