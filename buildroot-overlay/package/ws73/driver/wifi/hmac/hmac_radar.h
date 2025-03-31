/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmaradar
 * Create: 2020-7-5
 */

#ifndef __HMAC_RADAR_H__
#define __HMAC_RADAR_H__

#if defined(_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined(_PRE_WLAN_FEATURE_DFS_ENABLE)
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "mac_device_ext.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined(_PRE_WLAN_FEATURE_DFS_ENABLE)
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_RADAR_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define STAGGER_MAX_DURATION 100          /* STAGGER类型雷达最大宽度 */
#define STAGGER_PULSE_MARGIN 4            /* STAGGER类型雷达相同脉冲间隔误差 */
#define RADAR_PULSE_MARGIN_ETSI 4         /* ETSI脉冲间隔最小误差 */
#define RADAR_PULSE_MARGIN_FCC 3          /* FCC脉冲间隔最小误差 */
#define RADAR_PULSE_MARGIN_FCC_LOW_TYPE 3 /* FCC TYPE0~TYPE2脉冲间隔最小误差 */

#define RADAR_PULSE_DURATION_MARGIN 20          /* 脉冲宽度最小误差 */
#define RADAR_PULSE_POWER_MARGIN 20             /* 脉冲功率最小误差 */
#define RADAR_PULSE_DURATION_MARGIN_ERROR 100   /* 脉冲宽度最大误差 */
#define RADAR_FCC_CHIRP_PULSE_DURATION_MARGIN 5 /* 脉冲宽度最小误差 */

#define MAX_RADAR_NORMAL_PULSE_ANA_CNT 5            /* 非chirp雷达脉冲信息分析最大个数 */
#define MAX_RADAR_NORMAL_PULSE_ANA_CNT_ETSI_TYPE3 8 /* ETSI type3雷达脉冲信息分析最大个数 */
#define EXTRA_PULSE_CNT 3                           /* buf中额外查询的脉冲个数 */

#define MAX_RADAR_STAGGER_NUM 3        /* STAGGER类型脉冲最大个数 */
#define MIN_RADAR_STAGGER_DURATION 9   /* STAGGER类型脉冲最小宽度 */
#define MEAN_RADAR_STAGGER_DURATION 80 /* STAGGER类型脉冲最大宽度 */

#define MIN_RADAR_NORMAL_DURATION 9     /* normal类型脉冲最小宽度 */
#define MIN_RADAR_NORMAL_DURATION_MKK 8 /* normal类型脉冲最小宽度 */

#define MIN_RADAR_NORMAL_DURATION_ETSI_TYPE3 180 /* ETSI TYPE3 最小宽度 */
#define MAX_RADAR_NORMAL_DURATION_FCC_TYPE2 110  /* FCC TYPE2  最大宽度 */
#define MIN_RADAR_NORMAL_DURATION_FCC_TYPE4 40   /* FCC TYPE4  最小宽度 */

#define MIN_RADAR_PULSE_PRI 148 /* 雷达脉冲最小间距(us) */
#define MIN_ETSI_PULSE_PRI 248  /* ETSI雷达脉冲最小间距(us) */

#define MIN_ETSI_CHIRP_PRI 245   /* ETSI chirp雷达最小脉冲间隔(us) */
#define MIN_FCC_CHIRP_PRI 990    /* FCC chirp雷达最小脉冲间隔(us) */
#define MIN_MKK_CHIRP_PRI 990    /* MKK chirp雷达最小脉冲间隔(us) */
#define MIN_ETSI_CHIRP_PRI_NUM 4 /* ETSI chirp雷达最小脉冲间隔个数 */
#define MIN_FCC_CHIRP_PRI_NUM 1  /* FCC chirp雷达最小脉冲间隔个数 */
#define MIN_MKK_CHIRP_PRI_NUM 1  /* MKK chirp雷达最小脉冲间隔个数 */

#define MIN_RADAR_PULSE_POWER 394              /* 脉冲power最小值 */
#define MIN_RADAR_PULSE_POWER_FCC_TYPE0 390    /* FCC type0脉冲power最小值 */
#define MIN_RADAR_PULSE_POWER_ETSI_STAGGER 394 /* ETSI Stagger脉冲power最小值 */

#define RADAR_NORMAL_PULSE_TYPE 0 /* 非chirp雷达脉冲类型 */
#define RADAR_CHIRP_PULSE_TYPE 1  /* chirp雷达脉冲类型 */

#define MAX_PULSE_TIMES 4 /* 脉冲间隔之间最大倍数 */

#define MAX_STAGGER_PULSE_TIMES 4          /* stagger脉冲间隔相差的最大倍数关系 */
#define MIN_FCC_TOW_TIMES_INT_PRI 100      // 200    /* FCC chirp雷达两次中断的最小时间间隔(ms) */
#define MAX_FCC_TOW_TIMES_INT_PRI 8000     /* FCC chirp雷达两次中断的最大时间间隔(ms) */
#define MAX_FCC_CHIRP_PULSE_CNT_IN_600US 5 /* FCC chirp雷达一个中断600ms内最大脉冲个数 */
#define MAX_FCC_CHIRP_EQ_DURATION_NUM 3    /* FCC chirp连续相同duration的最大个数 */

#define RADAR_PULSE_NO_PERIOD 0     /* 脉冲不具备周期性 */
#define RADAR_PULSE_NORMAL_PERIOD 1 /* 脉冲具备周期性 */
#define RADAR_PULSE_BIG_PERIOD 2    /* 脉冲间隔相差较大 */
#define RADAR_PULSE_ONE_DIFF_PRI 3  /* 等间隔脉冲中出现一个不等间隔 */

#define RADAR_ETSI_PPS_MARGIN 2
#define RADAR_ETSI_TYPE5_MIN_PPS_DIFF (20 - RADAR_ETSI_PPS_MARGIN)  /* ETSI type5不同PRI之间最小PPS偏差度 */
#define RADAR_ETSI_TYPE5_MAX_PPS_DIFF (50 + RADAR_ETSI_PPS_MARGIN)  /* ETSI type5不同PRI之间最大PPS偏差度 */
#define RADAR_ETSI_TYPE6_MIN_PPS_DIFF (80 - RADAR_ETSI_PPS_MARGIN)  /* ETSI type6不同PRI之间最小PPS偏差度 */
#define RADAR_ETSI_TYPE6_MAX_PSS_DIFF (400 + RADAR_ETSI_PPS_MARGIN) /* ETSI type6不同PRI之间最大PPS偏差度 */

#define RADAR_ONE_SEC_IN_US 1000000

#define TWO_TIMES_STAGGER_PULSE_MARGIN (2 * STAGGER_PULSE_MARGIN)
#define THREE_TIMES_STAGGER_PULSE_MARGIN (3 * STAGGER_PULSE_MARGIN)
#define FOUR_TIMES_STAGGER_PULSE_MARGIN (4 * STAGGER_PULSE_MARGIN)

/* chirp crazy repot */
#define MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_FCC 100
#define MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_ETSI 40

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* stagger模式周期检测类型 */
typedef enum {
    HMAC_RADAR_STAGGER_PERIOD_PRI_EQUAL, /* 存在相同的PRI，且PRI符合范围要求 */
    HMAC_RADAR_STAGGER_PERIOD_PRI_ERR,   /* 存在相同的PRI，但PRI不符合范围要求 */
    HMAC_RADAR_STAGGER_PERIOD_NOT_DEC,   /* 不存在相同的PRI，无法确定周期性，需做进一步检查 */

    HMAC_RADAR_STAGGER_PERIOD_BUTT
} hmac_radar_stagger_period_enum;

typedef osal_u8 hmac_radar_stagger_period_enum_uint8;

/* stagger模式radar类型 */
typedef enum {
    HMAC_RADAR_STAGGER_TYPE_INVALID,
    HMAC_RADAR_STAGGER_TYPE5,
    HMAC_RADAR_STAGGER_TYPE6,

    HMAC_RADAR_STAGGER_TYPE_BUTT
} hmac_radar_stagger_type_enum;

typedef osal_u8 hmac_radar_stagger_type_enum_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 脉冲信息分析结果 */
typedef struct {
    osal_u32 min_pri;                  /* 最小脉冲间隔 */
    osal_u16 min_duration;             /* 最小脉冲宽度 */
    osal_u16 max_power;                /* 脉冲power最大值 */
    osal_u32 pri[MAX_RADAR_PULSE_NUM]; /* 所有脉冲间隔 */

    osal_u8 pri_cnt;     /* 脉冲间隔个数 */
    osal_u8 stagger_cnt; /* stagger脉冲数目 */
    osal_u8 begin;       /* 分析脉冲信息的起始index */
    osal_u8 resv1[1];    /* 保留位 */

    osal_u16 avrg_power;    /* 平均脉冲功率 */
    osal_u16 duration_diff; /* 最小脉冲宽度 */
    osal_u32 extra_pri;     /* stagger type6额外读取一个pri */
    osal_u32 pri_diff;      /* 最小脉冲间隔 */

    osal_u16 power_diff;    /* 脉冲power最大值 */
    osal_u16 avrg_duration; /* 平均脉冲间隔 */
} hmac_radar_pulse_analysis_result_stru;

typedef struct {
    osal_u8 stagger_cnt;
    osal_u16 min_duration;
    osal_u16 max_duration;
    osal_u32 min_pri;
    osal_u32 max_pri;
    osal_u16 max_power;
    osal_u16 min_power;
    osal_u8 index;
    osal_u32 sum_power;
    osal_u32 sum_duration;
} hmac_radar_pulse_analysis_info;

/*****************************************************************************
  8 内联函数
*****************************************************************************/
static INLINE__ osal_u8 check_radar_etsi_type1_pri(osal_u32 val)
{
    return ((val >= 998 && val <= 5002) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type2_pri(osal_u32 val)
{
    return ((val >= 623 && val <= 5002) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type3_pri(osal_u32 val)
{
    return ((val >= 248 && val <= 500) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type5_pri(osal_u32 val)
{
    return ((val >= 2500 && val <= 3333) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type6_pri(osal_u32 val)
{
    return ((val >= 833 && val <= 2500) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_stagger_pri(osal_u32 val)
{
    return ((val >= 833 && val <= 3333) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type6_duration_diff(osal_u16 val)
{
    return ((val <= 47) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_short_pulse(osal_u16 val)
{
    return ((val >= 10 && val <= 30) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type0_pri(osal_u32 val)
{
    return ((val >= 1426 && val <= 1430) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type3_pri(osal_u32 val)
{
    return ((val >= 198 && val <= 502) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type4_pri(osal_u32 val)
{
    return ((val >= 198 && val <= 502) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type4_pri_small(osal_u32 val)
{
    return (((val >= 233 && val <= 330) || (val >= 336 && val <= 502)) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type5_pri(osal_u32 val)
{
    return ((val >= 998 && val <= (2002 * 2)) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type6_pri(osal_u32 val)
{
    return ((val >= 331 && val <= 335) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type2_pri_small(osal_u32 val)
{
    return ((val >= 148 && val <= 198) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_chirp_total_cnt(osal_u8 val)
{
    return ((val >= 3 && val <= 15) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type4_duration(osal_u16 val)
{
    return ((val >= 5 && val <= 220) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_chirp_pow_diff(osal_u16 val)
{
    return ((val <= 65) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u32 two_times(osal_u32 val)
{
    return (val << 1);
}

static INLINE__ osal_u8 check_radar_etsi_type2_hw(hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num)
{
    return ((radar_type_num == 2) && (radar_type == HAL_DFS_RADAR_TYPE_ETSI));
}

static INLINE__ osal_u8 check_radar_etsi_type3_hw(hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num)
{
    return ((radar_type_num == 3) && (radar_type == HAL_DFS_RADAR_TYPE_ETSI));
}

static INLINE__ osal_u8 check_radar_fcc_type3_hw(hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num)
{
    return ((radar_type_num == 3) && (radar_type == HAL_DFS_RADAR_TYPE_FCC));
}

static INLINE__ osal_u8 check_radar_fcc_type4_hw(hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num)
{
    return ((radar_type_num == 4) && (radar_type == HAL_DFS_RADAR_TYPE_FCC));
}

static INLINE__ osal_u8 check_radar_etsi_type23_or_fcc_type34_hw(
    hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num)
{
    return (check_radar_etsi_type2_hw(radar_type, radar_type_num) ||
        check_radar_etsi_type3_hw(radar_type, radar_type_num) ||
        check_radar_fcc_type3_hw(radar_type, radar_type_num) ||
        check_radar_fcc_type4_hw(radar_type, radar_type_num));
}

static INLINE__ osal_u8 check_radar_etsi_type2_irq_num(osal_u8 num_a, osal_u8 num_b)
{
    return ((num_a >= 1 && num_a <= 3 && (0 == num_b || (num_b >= 1 && num_b <= 8))) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_etsi_type3_irq_num(osal_u8 num_a, osal_u8 num_b)
{
    return ((num_a >= 1 && num_a <= 5 && (0 == num_b || (num_b >= 1 && num_b <= 8))) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type4_irq_num(osal_u8 num_a, osal_u8 num_b)
{
    return ((num_a >= 1 && num_a <= 3 && (0 == num_b || (num_b >= 1 && num_b <= 4))) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 check_radar_fcc_type3_irq_num(osal_u8 num_a, osal_u8 num_b)
{
    return ((num_a >= 1 && num_a <= 4 && (0 == num_b || (num_b >= 1 && num_b <= 4))) ? OSAL_TRUE : OSAL_FALSE);
}

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_filter(hmac_device_stru *hmac_device, hal_radar_det_event_stru *radar_det_info);
oal_bool_enum_uint8 hmac_radar_crazy_report_det_timer(hal_to_dmac_device_stru *hal_device);

#endif /* _PRE_WLAN_FEATURE_DFS_OPTIMIZE */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_radar.h */
