/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header file of edca option algorithm
 */

#ifndef __ALG_EDCA_OPT_H__
#define __ALG_EDCA_OPT_H__

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_transplant.h"
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#include "alg_intf_det.h"
#endif
#include "alg_common_rom.h"
#include "msg_alg_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_EDCA_DET_THR_NUM 2          /* 统计周期内最优速率的goodput */
/******************************************************************************
  3 枚举定义
******************************************************************************/
typedef enum {
    EDCA_PK_DEBUG_MODE_OFF = 0,
    EDCA_PK_DEBUG_ADJ_INIT = 1,
    EDCA_PK_DEBUG_ADJ_AIFSN = 2,
    EDCA_PK_DEBUG_ADJ_CW = 3,
    EDCA_PK_DEBUG_ADJ_TXOP = 4,
    EDCA_PK_DEBUG_COMPLETE = 5,
    EDCA_PK_DEBUG_MODE_BUTT
} hmac_edca_pk_debug_mode_enum;
typedef osal_u8 hmac_edca_pk_debug_mode_enum_uint8;

typedef enum {
    EDCA_INTF_DEBUG_MODE_OFF = 0,
    EDCA_INTF_DEBUG_GOODPUT_NEG_ON = 1, /* goodput 负增益 */
    EDCA_INTF_DEBUG_GOODPUT_NEG_OFF = 2, /* goodput 无负增益 */
    EDCA_INTF_DEBUG_PK_RECOVER_ON = 3, /* 干扰繁忙度负增益,重置参数 */
    EDCA_INTF_DEBUG_PK_RECOVER_OFF = 4, /* 干扰繁忙度无负增益，调整参数 */
    EDCA_INTF_DEBUG_HAS_COCH_ON = 5, /* 模拟同频干扰 */
    EDCA_INTF_DEBUG_HAS_ADJCH_ON = 6, /* 模拟邻频干扰 */
    EDCA_INTF_DEBUG_HAS_COCH_OFF = 7, /* 模拟无干扰 */
    EDCA_INTF_DEBUG_MODE_BUTT
} hmac_edca_intf_debug_mode_enum;
typedef osal_u8 hmac_edca_intf_debug_mode_enum_uint8;
/* 指明当前哪个干扰检测算法在进行负增益检测 */
typedef enum {
    ALG_EDCA_DET_PROBE_OFF = 0,   /* 负增益检测关闭 */
    ALG_EDCA_DET_PROBE_COCH = 1,  /* 同频干扰probe */
    ALG_EDCA_DET_PROBE_ADJCH = 2, /* 邻频干扰probe */
    ALG_EDCA_DET_PROBE_TXOP = 4,  /* txop limit probe */

    ALG_EDCA_DET_PROBE_BUTT
} alg_edca_det_probe_enum;
typedef osal_u8 alg_edca_det_probe_uint8;

typedef enum {
    ALG_EDCA_DET_PROBE_STAT_OFF = 0,         /* 非探测状态 */
    ALG_EDCA_DET_PROBE_STAT_DEFAULT_THR = 1, /* 保持默认CCA门限 */
    ALG_EDCA_DET_PROBE_STAT_BEGIN = 2,       /* 开始探测 */
    ALG_EDCA_DET_PROBE_STAT_FINISH = 3,      /* 结束探测 */

    ALG_EDCA_DET_PROBE_STAT_BUTT,
} alg_edca_det_probe_stat_enum;
typedef osal_u8 alg_edca_det_probe_stat_uint8;
/******************************************************************************
  4 STRUCT定义
******************************************************************************/
/* 负增益检测相关 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef struct {
    alg_edca_det_probe_uint8 probe_flag;               /* 当前持有负增益探测标记 */
    alg_edca_det_probe_uint8 last_probe_flag;          /* 上次持有负增益探测标记,注意为非OFF */
    alg_edca_det_probe_stat_uint8 intf_det_probe_stat; /* 探测状态 */
    osal_u8 neg_nonprobe_thr;                            /* 出现负增益后延迟探测计数门限 */

    osal_u8 collision_ratio_th;            /* 碰撞率判断门限 */
    osal_u8 goodput_loss_th;               /* goodput损失门限 */
    osal_u8 intvl_time_thr;                /* 统计间隔时间计数阈值 */
    oal_bool_enum_uint8 thrpt_loss_stat; /* 当前是否处于负增益 */
} alg_neg_det_probe_info_stru;

typedef struct {
    osal_u8 neg_nonprobe_cnt; /* 出现负增益后延迟探测计数 */
    osal_u8 intvl_time_cnt;   /* 统计间隔时间计数 */
    osal_u8 probe_finish[2];  /* 2:数组大小 开启/关闭 探测完成标志 */
} alg_neg_det_probe_stat_stru;

typedef struct {
    /* 统计计数 */
    osal_u32 tx_cnt;        /* 统计周期内总发包个数计数 */
    osal_u32 collision_cnt; /* 统计周期内碰撞个数计数 */
    osal_u32 pkt_short_goodput_sum;

    /* 统计结果 */
    osal_u32 avg_goodput[ALG_EDCA_DET_THR_NUM];     /* 统计周期内最优速率的goodput */
    osal_u16 collision_ratio[ALG_EDCA_DET_THR_NUM]; /* 统计周期内碰撞率 */
} alg_neg_det_ac_info_stru;

#endif
/* device下抗干扰特性对应的信息结构体 */
typedef struct {
    /* edca优化特性开关参数 */
    oal_bool_enum_uint8 edca_opt_en_ap;    /* ap模式下edca优化使能 */
    oal_bool_enum_uint8 edca_opt_en_sta;   /* sta模式下edca优化使能 */
    oal_bool_enum_uint8 txop_limit_en_sta; /* sta模式下txop limit优化使能 */
    hmac_edca_pk_debug_mode_enum_uint8 pk_debug_mode; /* pk debug mode en:仅用于本地调试 */
    hmac_edca_intf_debug_mode_enum_uint8 intf_debug_mode; /* intf debug mode en:仅用于本地调试 */
    /* (edca优化)相关参数 */
    osal_u8 edca_opt_weight_sta; /* sta模式下edca参数加权系数 */
    osal_u8 edca_reg_optimized;  /* edca参数寄存器是否被优化过 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    osal_u8 txop_optimized; /* 是否优化了TXOP */
#else
    osal_u8 reserv[1];
#endif
    oal_bool_enum_uint8 txop_flag;      /* txop limit开启标记:0未打开; 1:打开 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    osal_u8 thrpt_loss_det_en;              /* 负增益检测开关 */
    osal_u8 thrpt_loss_det_debug;           /* 负增益检测日志开关 */
    osal_u8 rsv2[1]; /* 保留1字节对齐 */
    alg_neg_det_probe_info_stru negdet_probe_info; /* 负增益探测信息结构体 */
    alg_neg_det_probe_stat_stru negdet_probe_stat; /* 负增益探测状态信息 */

    alg_neg_det_ac_info_stru txac_info_bebk; /* ac信息 bebk */
    alg_neg_det_ac_info_stru txac_info_vi;   /* ac信息 vi */
    alg_neg_det_ac_info_stru txac_info_vo;   /* ac信息 vo */
    alg_neg_det_ac_info_stru *txac_info;     /* ac信息  */
#else
    osal_u8 rsv3[3];
#endif
} alg_edca_opt_stru;
/******************************************************************************
  5 函数声明
******************************************************************************/
osal_s32 alg_edca_opt_init(osal_void);
osal_void alg_edca_opt_exit(osal_void);
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
osal_void alg_edca_param_reset(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt);
osal_void alg_edca_intrf_mode_process(hmac_vap_stru *hmac_vap, osal_u8 edca_switch);
osal_u32 alg_edca_intrf_mode_process_assoc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
#endif
osal_void alg_edca_print_cw_gla_info(osal_u8 ac_type, osal_u8 cwmax, osal_u8 cwmin);
osal_void alg_edca_print_aifsn_all_gla_info(osal_u8 be, osal_u8 bk, osal_u8 vi, osal_u8 vo);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_EDCA_OPT */
#endif /* end of alg_edca_opt.h */