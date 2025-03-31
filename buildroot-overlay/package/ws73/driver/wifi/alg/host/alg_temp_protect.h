/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: header file of temperature protection algorithm
 */

#ifndef __ALG_TEMP_PROTECT_H__
#define __ALG_TEMP_PROTECT_H__

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "frw_ext_if.h"
#include "hmac_alg_if.h"
#include "alg_transplant.h"
#include "msg_alg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_TEMP_PROTECT_SAFE_TH (90)            /* 降占空比的恢复门限 90 */
#define ALG_TEMP_PROTECT_OVER_TH (105)           /* 降占空比的恢复门限 105 */
#define ALG_TEMP_PROTECT_PA_OFF_TH (125)         /* 关PA的触发门限  125 */
#define ALG_TEMP_PROTECT_PA_ON_TH (105)          /* 开PA的触发门限  105 */
#define ALG_TEMP_PROTECT_DUTY_CYC_FULL (1000)    /* 调整占空比流程中的合理占空比门限 */
#define ALG_TEMP_PROTECT_DUTY_CYC_MAX (500)      /* 调整占空比流程中的合理占空比门限 */
#define ALG_TEMP_PROTECT_DUTY_CYC_MIN (100)      /* 调整占空比流程中的合理占空比门限 */
#define ALG_TEMP_PROTECT_DUTY_CYC_STEP (100)     /* 调整占空比的步长 */

#define ALG_TEMP_PROTECT_TIMER_CYCLE (1000)       /* 过温状态下查询温度定时器的间隔时间 MS */
#define ALG_TEMP_PROTECT_TIMER_CYCLE_IDLE (10000) /* IDLE状态下定时器间隔周期10s */

#define ALG_TEMP_PROTECT_MAX_AGGR_SIZE (16)       /* 最大聚合个数 考虑硬件聚合最大64  */
#define ALG_TEMP_PROTECT_MIN_AGGR_SIZE (1)        /* 最小聚合个数 */
#define ALG_TEMP_PROTECT_AGGR_SIZE_STEP (2)       /* 调整聚合个数的步长 */
#define ALG_TEMP_PROTECT_TIMES_TH_SAFE (10)       /* SAFE状态下状态机需要等待的循环次数 */
#define ALG_TEMP_PROTECT_TIMES_TH_OVER (3)        /* OVER状态下状态机需要等待的循环次数 */
#define ALG_TEMP_PROTECT_TIMES_TH_PA_OFF (1)      /* PA-OFF状态下状态机需要等待的循环次数 */
#define ALG_TEMP_PROTECT_TIMES_TH_INIT (1)        /* INIT状态下状态机需要等待的循环次数 */

#define ALG_TEMP_PROTECT_CUSTOM_TH_ADJUST (20)    /* 特殊芯片模式下的温度门限调整量 */

#define HMAC_TEMP_PROTECT_ENABLE           (OSAL_TRUE)
#define HMAC_TEMP_PROTECT_REDUCE_ENABLE    (OSAL_TRUE)
#define HMAC_TEMP_PROTECT_RECOVER_TH       (90)
#define HMAC_TEMP_PROTECT_PA_OFF_TH        (125)
/* 维测用宏定义 */
#define HMAC_TEMP_PROTECT_TSENSOR_MAX        (140)
#define HMAC_TEMP_PROTECT_TSENSOR_MIN        (-40)
#define HMAC_TEMP_PROTECT_INVALID_TEMP       (255)

/******************************************************************************
  3 枚举定义
******************************************************************************/
/* temperature protection状态机状态枚举 */
typedef enum {
    ALG_TEMP_PROTECT_STATE_INIT = 0,   /* 初始运行状态 */
    ALG_TEMP_PROTECT_STATE_SAFE = 1,   /* 温度小于90的状态 */
    ALG_TEMP_PROTECT_STATE_NORMAL = 2, /* 温度小于105的状态 */
    ALG_TEMP_PROTECT_STATE_OVER = 3,   /* 温度大于105的状态 */
    ALG_TEMP_PROTECT_STATE_PA_OFF = 4, /* 温度大于125的状态 */

    ALG_TEMP_PROTECT_BUTT_STATE /* 错误状态 */
} alg_temp_protect_state;
typedef osal_u8 alg_temp_protect_sta_enum_uint8;

/* 时间枚举 */
typedef enum {
    ALG_TEMP_PROTECT_EVENT_TIMER = 0,
    ALG_TEMP_PROTECT_EVENT_DEVICE_IDLE = 1,

    ALG_TEMP_PROTECT_EVENT_BUTT
} alg_temp_protect_event_enum;
typedef osal_u16 alg_temp_protect_event_enum_uint16;

#define ALG_TEMP_PROTECT_RECORD_AMOUNT 20      /* 温度保护功能 */

/******************************************************************************
  7 STRUCT定义
******************************************************************************/
typedef struct {
    osal_u8   temp_pro_enable : 1; /* 过温保护的使能开关 */
    osal_u8   temp_pro_reduce_pwr_enable : 1; /* 过温保护过程中是否需要降低功率 */
    osal_u8   rsv : 6;
    osal_s8   temp_pro_safe_th; /* 过温保护的恢复安全水线 */
    osal_s16  temp_pro_pa_off_th; /* 过温保护的PA_OFF水线 */
}hmac_temp_pri_custom_stru;

typedef struct _alg_temp_protection_fsm {
    oal_fsm_stru oal_fsm; /* 过温保护状态机 */
    oal_bool_enum_uint8 is_fsm_attached;
    osal_u8 resv[3]; /* 3:数组大小 */
} alg_temp_protect_fsm_stru;

/* 过温保护历史信息记录结构体 */
typedef struct {
    osal_s16 ave_temp;                          /* 平均温度 */
    osal_u8 state;                              /* 当前状态 */
    osal_u8 temp_pro_aggr_size;                 /* 聚合度 */
    osal_u32 pre_set_duty_cyc;                  /* 预设占空比 */
    osal_u8 schedule_reduce;                  /* 减少一次调度调度开关 */
    osal_u8 reduce_pwr_enable;                  /* 功率开关 */
    oal_bool_enum_uint8 is_pa_off;              /* PA OFF 状态记录 */
    osal_u8 times_th;                           /* 抛事件的间隔次数 */
    osal_u32 cur_duty_cyc;                      /* 当前占空比 */
} temp_stat_stru;

/* 过温保护识别对应的私有信息结构体 */
typedef struct {
    osal_u8 times_set;                     /* 命令设置的间隔次数 */
    osal_u8 times;                         /* 定时器的次数 */
    osal_u8 times_th;                      /* 不同状态下抛事件的间隔次数 */

    osal_u8 reduce_pwr_enable;             /* 减小功率的使能开关 */
    osal_u8 schedule_reduce;               /* 减小device侧缓存队列调度 */
    oal_bool_enum_uint8 is_pa_off;         /* pa_off 记录 */

    osal_u8 temp_pro_aggr_size;
    osal_s16 ave_temp;                     /* 单周期内温度的平均值 */
    osal_u16 over_th_set;                  /* 命令设置的OVER门限 */
    osal_u32 pre_set_duty_cyc;             /* 预设占空比，千分之几为单位，初始化500 */
    osal_s32 accumulate_temp;              /* 温度累加值 */

    /* 状态机 */
    alg_temp_protect_fsm_stru temp_pro_fsm;
    frw_timeout_stru temp_protect_timer;   /* 温度检测定时器 */

    /* 维测 */
    oal_bool_enum_uint8 temp_debug_flag;    /* 温度保护算法维测开关 */
    osal_u8 temp_debug_temperature;         /* 温度保护算法维测设定温度 */
    osal_u8 record_idx;                     /* 温度历史记录idx */
    temp_stat_stru stat_record[ALG_TEMP_PROTECT_RECORD_AMOUNT];    /* 统计信息记录 记录20条历史信息 */
} alg_temp_protect_stru;
/******************************************************************************
  4 全局变量声明
******************************************************************************/

/******************************************************************************
  5 函数声明
******************************************************************************/
osal_void alg_temp_protect_init(osal_void);
osal_void alg_temp_protect_exit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_TEMP_PRO */
#endif /* end of alg_temp_protection.h */
