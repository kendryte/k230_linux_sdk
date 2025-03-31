// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_RTC_REGS.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_RTC_REGS_H__
#define __C_UNION_DEFINE_RTC_REGS_H__

/* Define the union u_rtc_irq_time_msbs */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_irq_time_msbs : 16; /* [15..0] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_irq_time_msbs;

/* Define the union u_rtc_irq_clr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_irq_clr : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_irq_clr;

/* Define the union u_rtc_irq_mask_out */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_irq_mask_out : 1; /* [0] */
        osal_u32 reserved_0 : 31;      /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_irq_mask_out;

/* Define the union u_rtc_irq_int_raw_in */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_irq_int_raw_in : 1; /* [0] */
        osal_u32 reserved_0 : 31;        /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_irq_int_raw_in;

/* Define the union u_rtc_irq_int_mask_in */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_irq_int_mask_in : 1; /* [0] */
        osal_u32 reserved_0 : 31;         /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_irq_int_mask_in;

/* Define the union u_rtc_time_out_h */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rtc_time_out_h : 16; /* [15..0] */
        osal_u32 reserved_0 : 16;     /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rtc_time_out_h;

#endif /* __C_UNION_DEFINE_RTC_REGS_H__ */
