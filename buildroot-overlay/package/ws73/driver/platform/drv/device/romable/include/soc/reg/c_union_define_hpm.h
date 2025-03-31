/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  HPM_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_HPM_H__
#define __C_UNION_DEFINE_HPM_H__

/* Define the union u_peri_pmc22 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm0_div
        /*
         * HPM0时钟分频比配置。对应([hpm0_div]+1)分频。
         * 支持2~64分频。
         */
        osal_u32 hpm0_div : 6; /* [5..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [7..6] */
        // Comment of field: hpm0_shift
        /*
         * HPM0原始码字右移位数。
         */
        osal_u32 hpm0_shift : 2; /* [9..8] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [11..10] */
        // Comment of field: hpm0_offset
        /*
         * HPM0原始码字右移后的偏移量。
         */
        osal_u32 hpm0_offset : 10; /* [21..12] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 2; /* [23..22] */
        // Comment of field: hpm0_en
        /*
         * 使能一次HPM0测量过程
         * 0：开始一次流程之前，此值需保持为0；
         * 1：开始一次调频流程。
         */
        osal_u32 hpm0_en : 1; /* [24] */
        // Comment of field: hpm0_bypass
        /*
         * HPM0单次使能信号。
         * 1：HPM的启动信号由hpm0_en决定；
         * 0：HPM的启动信号由hpm0_monitor_en决定。
         */
        osal_u32 hpm0_bypass : 1; /* [25] */
        // Comment of field: hpm0_monitor_en
        /*
         * 循环监控HPM0的使能。
         * 0：禁止循环监控HPM；
         * 1：使能循环监控HPM。
         */
        osal_u32 hpm0_monitor_en : 1; /* [26] */
        // Comment of field: hpm0_rst_req
        /*
         * HPM0复位请求，高有效
         * 0:HPM模块复位请求不使能；
         * 1:使能HPM模块复位。
         */
        osal_u32 hpm0_rst_req : 1; /* [27] */
        osal_u32 reserved_3 : 4; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc22;

/* Define the union u_peri_pmc23 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm0_pc_record0
        /*
         * HPM0原始码型。
         */
        osal_u32 hpm0_pc_record0 : 10; /* [9..0] */
        // Comment of field: hpm0_pc_valid
        /*
         * HPM0输出有效指示
         */
        osal_u32 hpm0_pc_valid : 1; /* [10] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 1; /* [11] */
        // Comment of field: hpm0_pc_record1
        /*
         * HPM0原始码型。
         */
        osal_u32 hpm0_pc_record1 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm0_low_warning
        /*
         * HPM0的原始码字低于下门限的Warning。
         */
        osal_u32 hpm0_low_warning : 1; /* [24] */
        // Comment of field: hpm0_up_warning
        /*
         * HPM0的原始码字超过上门限的Warning。
         */
        osal_u32 hpm0_up_warning : 1; /* [25] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc23;

/* Define the union u_peri_pmc24 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm0_pc_record2
        /*
         * HPM0原始码型
         */
        osal_u32 hpm0_pc_record2 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm0_pc_record3
        /*
         * HPM0原始码型
         */
        osal_u32 hpm0_pc_record3 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm0_rcc
        /*
         * HPM0输出有RCC码
         */
        osal_u32 hpm0_rcc : 5; /* [28..24] */
        osal_u32 reserved_2 : 3; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc24;

/* Define the union u_peri_pmc25 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm0_uplimit
        /*
         * hpm0原始码型上限。
         */
        osal_u32 hpm0_uplimit : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm0_lowlimit
        /*
         * hpm0原始码型下限。
         */
        osal_u32 hpm0_lowlimit : 11; /* [22..12] */
        // Comment of field: reserved_1
        /*
         * 保留位。
         */
        osal_u32 reserved_1 : 1; /* [23] */
        osal_u32 hpm0_monitor_period : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc25;

/* Define the union u_peri_pmc26 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm1_div
        /*
         * HPM1时钟分频比配置。对应([hpm1_div]+1)分频。
         * 支持2~64分频。
         */
        osal_u32 hpm1_div : 6; /* [5..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [7..6] */
        // Comment of field: hpm1_shift
        /*
         * HPM1原始码字右移位数。
         */
        osal_u32 hpm1_shift : 2; /* [9..8] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [11..10] */
        // Comment of field: hpm1_offset
        /*
         * HPM1原始码字右移后的偏移量。
         */
        osal_u32 hpm1_offset : 10; /* [21..12] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 2; /* [23..22] */
        // Comment of field: hpm1_en
        /*
         * 使能一次HPM1测量过程
         * 0：开始一次流程之前，此值需保持为0；
         * 1：开始一次调频流程。
         */
        osal_u32 hpm1_en : 1; /* [24] */
        // Comment of field: hpm1_bypass
        /*
         * HPM1单次使能信号。
         * 1：HPM的启动信号由hpm1_en决定；
         * 0：HPM的启动信号由hpm1_monitor_en决定。
         */
        osal_u32 hpm1_bypass : 1; /* [25] */
        // Comment of field: hpm1_monitor_en
        /*
         * 循环监控HPM1的使能。
         * 0：禁止循环监控HPM；
         * 1：使能循环监控HPM。
         */
        osal_u32 hpm1_monitor_en : 1; /* [26] */
        // Comment of field: hpm1_rst_req
        /*
         * HPM1复位请求，高有效
         * 0:HPM模块复位请求不使能；
         * 1:使能HPM模块复位。
         */
        osal_u32 hpm1_rst_req : 1; /* [27] */
        osal_u32 reserved_3 : 4; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc26;

/* Define the union u_peri_pmc27 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm1_pc_record0
        /*
         * HPM1原始码型。
         */
        osal_u32 hpm1_pc_record0 : 10; /* [9..0] */
        // Comment of field: hpm1_pc_valid
        /*
         * HPM1输出有效指示
         */
        osal_u32 hpm1_pc_valid : 1; /* [10] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 1; /* [11] */
        // Comment of field: hpm1_pc_record1
        /*
         * HPM1原始码型。
         */
        osal_u32 hpm1_pc_record1 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm1_low_warning
        /*
         * HPM1的原始码字低于下门限的Warning。
         */
        osal_u32 hpm1_low_warning : 1; /* [24] */
        // Comment of field: hpm1_up_warning
        /*
         * HPM1的原始码字超过上门限的Warning。
         */
        osal_u32 hpm1_up_warning : 1; /* [25] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc27;

/* Define the union u_peri_pmc28 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm1_pc_record2
        /*
         * HPM1原始码型
         */
        osal_u32 hpm1_pc_record2 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm1_pc_record3
        /*
         * HPM1原始码型
         */
        osal_u32 hpm1_pc_record3 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm1_rcc
        /*
         * HPM1输出有RCC码
         */
        osal_u32 hpm1_rcc : 5; /* [28..24] */
        osal_u32 reserved_2 : 3; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc28;

/* Define the union u_peri_pmc29 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm1_uplimit
        /*
         * hpm1原始码型上限。
         */
        osal_u32 hpm1_uplimit : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm1_lowlimit
        /*
         * hpm1原始码型下限。
         */
        osal_u32 hpm1_lowlimit : 11; /* [22..12] */
        // Comment of field: reserved_1
        /*
         * 保留位。
         */
        osal_u32 reserved_1 : 1; /* [23] */
        osal_u32 hpm1_monitor_period : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc29;

/* Define the union u_peri_pmc30 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm2_div
        /*
         * HPM2时钟分频比配置。对应([hpm2_div]+1)分频。
         * 支持2~64分频。
         */
        osal_u32 hpm2_div : 6; /* [5..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [7..6] */
        // Comment of field: hpm2_shift
        /*
         * HPM2原始码字右移位数。
         */
        osal_u32 hpm2_shift : 2; /* [9..8] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [11..10] */
        // Comment of field: hpm2_offset
        /*
         * HPM2原始码字右移后的偏移量。
         */
        osal_u32 hpm2_offset : 10; /* [21..12] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 2; /* [23..22] */
        // Comment of field: hpm2_en
        /*
         * 使能一次HPM2测量过程
         * 0：开始一次流程之前，此值需保持为0；
         * 1：开始一次调频流程。
         */
        osal_u32 hpm2_en : 1; /* [24] */
        // Comment of field: hpm2_bypass
        /*
         * HPM2单次使能信号。
         * 1：HPM的启动信号由hpm2_en决定；
         * 0：HPM的启动信号由hpm2_monitor_en决定。
         */
        osal_u32 hpm2_bypass : 1; /* [25] */
        // Comment of field: hpm2_monitor_en
        /*
         * 循环监控HPM2的使能。
         * 0：禁止循环监控HPM；
         * 1：使能循环监控HPM。
         */
        osal_u32 hpm2_monitor_en : 1; /* [26] */
        // Comment of field: hpm2_rst_req
        /*
         * HPM2复位请求，高有效
         * 0:HPM模块复位请求不使能；
         * 1:使能HPM模块复位。
         */
        osal_u32 hpm2_rst_req : 1; /* [27] */
        osal_u32 reserved_3 : 4; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc30;

/* Define the union u_peri_pmc31 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm2_pc_record0
        /*
         * HPM2原始码型。
         */
        osal_u32 hpm2_pc_record0 : 10; /* [9..0] */
        // Comment of field: hpm2_pc_valid
        /*
         * HPM2输出有效指示
         */
        osal_u32 hpm2_pc_valid : 1; /* [10] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 1; /* [11] */
        // Comment of field: hpm2_pc_record1
        /*
         * HPM2原始码型。
         */
        osal_u32 hpm2_pc_record1 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm2_low_warning
        /*
         * HPM2的原始码字低于下门限的Warning。
         */
        osal_u32 hpm2_low_warning : 1; /* [24] */
        // Comment of field: hpm2_up_warning
        /*
         * HPM2的原始码字超过上门限的Warning。
         */
        osal_u32 hpm2_up_warning : 1; /* [25] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc31;

/* Define the union u_peri_pmc32 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm2_pc_record2
        /*
         * HPM2原始码型
         */
        osal_u32 hpm2_pc_record2 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm2_pc_record3
        /*
         * HPM2原始码型
         */
        osal_u32 hpm2_pc_record3 : 10; /* [21..12] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 2; /* [23..22] */
        // Comment of field: hpm2_rcc
        /*
         * HPM2输出有RCC码
         */
        osal_u32 hpm2_rcc : 5; /* [28..24] */
        osal_u32 reserved_2 : 3; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc32;

/* Define the union u_peri_pmc33 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm2_uplimit
        /*
         * hpm2原始码型上限。
         */
        osal_u32 hpm2_uplimit : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 2; /* [11..10] */
        // Comment of field: hpm2_lowlimit
        /*
         * hpm2原始码型下限。
         */
        osal_u32 hpm2_lowlimit : 11; /* [22..12] */
        // Comment of field: reserved_1
        /*
         * 保留位。
         */
        osal_u32 reserved_1 : 1; /* [23] */
        osal_u32 hpm2_monitor_period : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc33;

/* Define the union u_peri_pmc34 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm0_monitor_period_fine
        /*
         * 对HPM0循环监控周期进一步微调，最小间隔精确到2ms/256。
         */
        osal_u32 hpm0_monitor_period_fine : 8; /* [7..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc34;

/* Define the union u_peri_pmc35 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm1_monitor_period_fine
        /*
         * 对HPM1循环监控周期进一步微调，最小间隔精确到2ms/256。
         */
        osal_u32 hpm1_monitor_period_fine : 8; /* [7..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc35;

/* Define the union u_peri_pmc36 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: hpm2_monitor_period_fine
        /*
         * 对HPM2循环监控周期进一步微调，最小间隔精确到2ms/256。
         */
        osal_u32 hpm2_monitor_period_fine : 8; /* [7..0] */
        // Comment of field: reserved_0
        /*
         * 保留位。
         */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_peri_pmc36;

#endif /* __HPM_C_UNION_DEFINE_H__ */
