/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  Timer_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_TIMER_H__
#define __C_UNION_DEFINE_TIMER_H__

/* Define the union u_timerx_control */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: oneshot
        /*
         * 选择计数模式为单次计数模式还是周期计数模式。
         * 0：周期计数模式或自由运行模式；
         * 1：单次计数模式。
         */
        osal_u32 oneshot : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 1; /* [1] */
        // Comment of field: timerpre
        /*
         * 该字段用于设置Timer的预分频因子。
         * 00：不经过预分频，时钟频率除以1；
         * 01：4级预分频，将Timer时钟频率除以16；
         * 10：8级预分频，将Timer时钟频率除以256；
         * 11：保留。
         */
        osal_u32 timerpre : 2; /* [3..2] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 1; /* [4] */
        // Comment of field: intenable
        /*
         * TIMERx_RIS中断屏蔽。
         * 0：屏蔽该中断；
         * 1：不屏蔽该中断。
         */
        osal_u32 intenable : 1; /* [5] */
        // Comment of field: timermode
        /*
         * 定时器的计数模式。
         * 0：自由运行模式；
         * 1：周期模式。
         */
        osal_u32 timermode : 1; /* [6] */
        // Comment of field: timeren
        /*
         * 定时器使能。
         * 0：Timer禁止；
         * 1：Timer使能。
         */
        osal_u32 timeren : 1; /* [7] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_timerx_control;

/* Define the union u_timerx_ris */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: timerxris
        /*
         * Timerx的原始中断状态。
         * 0：未产生中断；
         * 1：已产生中断。
         */
        osal_u32 timerxris : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。写入无效，读时返回0。
         */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_timerx_ris;

/* Define the union u_timerx_mis */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: timerxmis
        /*
         * 屏蔽后的Timerx的中断状态。
         * 0：中断无效；
         * 1：中断有效。
         */
        osal_u32 timerxmis : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_timerx_mis;

#endif /* __TIMER_C_UNION_DEFINE_H__ */
