/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  TSENSOR_REGS_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_TSENSOR_REGS_H__
#define __C_UNION_DEFINE_TSENSOR_REGS_H__

/* Define the union u_ts_start */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_start
        /*
         * 软件配置开始采样信号脉冲信号。
         */
        osal_u32 tsensor_start : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_start;

/* Define the union u_ts_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_enable
        /*
         * tsensor使能。
         * 1：有效；
         * 0：禁止。
         */
        osal_u32 tsensor_enable : 1; /* [0] */
        osal_u32 tsensor_mode : 2; /* [2..1] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_cfg;

/* Define the union u_ts_man_clr */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_man_clr
        /*
         * 输出的data和rdy信号的清除信号（不计算单点16次平均值）。
         */
        osal_u32 tsensor_man_clr : 1; /* [0] */
        // Comment of field: tsensor_rdy_man
        /*
         * 当前rdy信号上报（针对的是不计算平均值的情况）。
         */
        osal_u32 tsensor_rdy_man : 1; /* [1] */
        // Comment of field: tsensor_data_man
        /*
         * 当前温度计算结果上报（单点的温度）。
         */
        osal_u32 tsensor_data_man : 10; /* [11..2] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 20; /* [31..12] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_man_clr;

/* Define the union u_ts_auto_clr */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_auto_clr
        /*
         * 输出的data和rdy信号的清除信号（计算单点16次平均值）。
         */
        osal_u32 tsensor_auto_clr : 1; /* [0] */
        // Comment of field: tsensor_rdy_auto
        /*
         * 当前rdy信号上报（针对的是16点计算平均值的情况）。
         */
        osal_u32 tsensor_rdy_auto : 1; /* [1] */
        // Comment of field: tsensor_data_auto
        /*
         * 当前温度计算结果上报（16点平均值计算结果）。
         */
        osal_u32 tsensor_data_auto : 10; /* [11..2] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 20; /* [31..12] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_auto_clr;

/* Define the union u_tsensor_c0_ctrl1 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: temp_calib
        /*
         * 0：选择开启校准算法（默认）
         * 1.：不开启校准算法；
         */
        osal_u32 temp_calib : 1; /* [0] */
        // Comment of field: temp_ct_sel
        /*
         * 对于输入1M精准时钟情况下：
         * 00-----0.512ms；
         * 01-----0.256ms；
         * 10-----1.024ms；
         * 11-----2.048ms
         */
        osal_u32 temp_ct_sel : 2; /* [2..1] */
        // Comment of field: temp_vcalib1_en
        /*
         * 校准预留寄存器
         */
        osal_u32 temp_vcalib1_en : 1; /* [3] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_tsensor_c0_ctrl1;

/* Define the union u_ts_temp_high_limit */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_high_limit
        /*
         * 当统计温度值大于该门限值时产生中断。
         */
        osal_u32 tsensor_temp_high_limit : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_high_limit;

/* Define the union u_ts_temp_low_limit */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_low_limit
        /*
         * 当统计温度值小于该门限值时产生中断。
         */
        osal_u32 tsensor_temp_low_limit : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_low_limit;

/* Define the union u_ts_overtemp_thresh */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_overtemp_thresh
        /*
         * 超高温度阈值。
         */
        osal_u32 tsensor_overtemp_thresh : 10; /* [9..0] */
        // Comment of field: tsensor_overtemp_thresh_en
        /*
         * 超高温度阈值信号的使能信号。
         */
        osal_u32 tsensor_overtemp_thresh_en : 1; /* [10] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 21; /* [31..11] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_overtemp_thresh;

/* Define the union u_ts_int_en */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_done_int_en
        /*
         * 采样完成中断信号。
         */
        osal_u32 tsensor_done_int_en : 1; /* [0] */
        // Comment of field: tsensor_out_thresh_int_en
        /*
         * 大于高温阈值或者小于低温阈值中断使能。
         */
        osal_u32 tsensor_out_thresh_int_en : 1; /* [1] */
        // Comment of field: tsensor_overtemp_int_en
        /*
         * 超高温中断产生使能信号。
         */
        osal_u32 tsensor_overtemp_int_en : 1; /* [2] */
        // Comment of field: tsensor_ml0_int_en
        /*
         * TSensor 0档温度中断上报使能。
         */
        osal_u32 tsensor_ml0_int_en : 1; /* [3] */
        // Comment of field: tsensor_ml1_int_en
        /*
         * TSensor 1档温度中断上报使能。
         */
        osal_u32 tsensor_ml1_int_en : 1; /* [4] */
        // Comment of field: tsensor_ml2_int_en
        /*
         * TSensor 2档温度中断上报使能。
         */
        osal_u32 tsensor_ml2_int_en : 1; /* [5] */
        // Comment of field: tsensor_ml3_int_en
        /*
         * TSensor 3档温度中断上报使能。
         */
        osal_u32 tsensor_ml3_int_en : 1; /* [6] */
        // Comment of field: tsensor_ml4_int_en
        /*
         * TSensor 4档温度中断上报使能。
         */
        osal_u32 tsensor_ml4_int_en : 1; /* [7] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_int_en;

/* Define the union u_ts_int_clr */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_int_clr
        /*
         * 中断清除信号。
         */
        osal_u32 tsensor_int_clr : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_int_clr;

/* Define the union u_ts_int_sts */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_done_int_sts
        /*
         * Tsensor一次采样完成中断上报。
         */
        osal_u32 tsensor_done_int_sts : 1; /* [0] */
        // Comment of field: tsensor_out_thresh_int_sts
        /*
         * 温度统计越过阈值中断上报。
         */
        osal_u32 tsensor_out_thresh_int_sts : 1; /* [1] */
        // Comment of field: tsensor_overtemp_int_sts
        /*
         * 过温中断上报。
         */
        osal_u32 tsensor_overtemp_int_sts : 1; /* [2] */
        // Comment of field: tsensor_ml0_int_sts
        /*
         * TSensor 0档温度中断上报。
         */
        osal_u32 tsensor_ml0_int_sts : 1; /* [3] */
        // Comment of field: tsensor_ml1_int_sts
        /*
         * TSensor 1档温度中断上报。
         */
        osal_u32 tsensor_ml1_int_sts : 1; /* [4] */
        // Comment of field: tsensor_ml2_int_sts
        /*
         * TSensor 2档温度中断上报。
         */
        osal_u32 tsensor_ml2_int_sts : 1; /* [5] */
        // Comment of field: tsensor_ml3_int_sts
        /*
         * TSensor 3档温度中断上报。
         */
        osal_u32 tsensor_ml3_int_sts : 1; /* [6] */
        // Comment of field: tsensor_ml4_int_sts
        /*
         * TSensor 4档温度中断上报。
         */
        osal_u32 tsensor_ml4_int_sts : 1; /* [7] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_int_sts;

/* Define the union u_ts_auto_refresh_period */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 tsensor_auto_refresh_period : 16; /* [15..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_auto_refresh_period;

/* Define the union u_ts_auto_refresh_en */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_auto_refresh_enable
        /*
         * 周期采样使能，高电平有效。
         */
        osal_u32 tsensor_auto_refresh_enable : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_auto_refresh_en;

/* Define the union u_ts_overtemp_prt_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_overtemp_prt_mode
        /*
         * 过温保护模式。
         * 1：man；
         * 0：auto。
         */
        osal_u32 tsensor_overtemp_prt_mode : 1; /* [0] */
        // Comment of field: tsensor_overtemp_prt_man
        /*
         * 过温保护寄存器配置。
         */
        osal_u32 tsensor_overtemp_prt_man : 1; /* [1] */
        // Comment of field: tsensor_overtemp_prt_polar
        /*
         * 过温保护auto取反，高电平有效
         */
        osal_u32 tsensor_overtemp_prt_polar : 1; /* [2] */
        // Comment of field: tsensor_overtemp_prt_sts
        /*
         * 过温保护状态
         */
        osal_u32 tsensor_overtemp_prt_sts : 1; /* [3] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_overtemp_prt_cfg;

/* Define the union u_ts_temp_ml0 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_ml0
        /*
         * TSensor 多档温度配置档位 temp0。
         */
        osal_u32 tsensor_temp_ml0 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_ml0;

/* Define the union u_ts_temp_ml1 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_ml1
        /*
         * TSensor 多档温度配置档位 temp1。
         */
        osal_u32 tsensor_temp_ml1 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_ml1;

/* Define the union u_ts_temp_ml2 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_ml2
        /*
         * TSensor 多档温度配置档位 temp2。
         */
        osal_u32 tsensor_temp_ml2 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_ml2;

/* Define the union u_ts_temp_ml3 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_temp_ml3
        /*
         * TSensor 多档温度配置档位 temp3。
         */
        osal_u32 tsensor_temp_ml3 : 10; /* [9..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_temp_ml3;

/* Define the union u_ts_efuse_cali */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: tsensor_cali_delta_value
        /*
         * 校准使用的10bit数据，默认为0
         */
        osal_u32 tsensor_cali_delta_value : 10; /* [9..0] */
        // Comment of field: tsensor_cali_delta_value_sign_bit
        /*
         * Tsensor校准温度符号位：默认为0
         * 1：输出值（tsensor_data_man和tsensor_data_auto）减去tsensor_cali_delta_value
         * 0：输出值（tsensor_data_man和tsensor_data_auto）加上tsensor_cali_delta_value
         */
        osal_u32 tsensor_cali_delta_value_sign_bit : 1; /* [10] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 21; /* [31..11] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ts_efuse_cali;

#endif /* __TSENSOR_REGS_C_UNION_DEFINE_H__ */
