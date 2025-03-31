// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_USB_TURBO_REG.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_USB_TURBO_REG_H__
#define __C_UNION_DEFINE_USB_TURBO_REG_H__

/* Define the union u_usb_turbo_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_usb_turbo_on : 1;          /* [0]  */
        osal_u32 rg_usb_turbo_err_rpt_en : 1;  /* [1]  */
        osal_u32 reserved_0 : 2;               /* [3..2]  */
        osal_u32 rg_int_trig_det_mode_sel : 1; /* [4]  */
        osal_u32 rg_int_vld_chk_en : 1;        /* [5]  */
        osal_u32 rg_int_clr_en : 1;            /* [6]  */
        osal_u32 rg_int_clr_det_mode_sel : 1;  /* [7]  */
        osal_u32 rg_usb_dev_mps : 11;          /* [18..8]  */
        osal_u32 reserved_1 : 13;              /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_cfg;

/* Define the union u_usb_turbo_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_ce_usb_dev_access_fail : 1;          /* [0]  */
        osal_u32 rg_ih_usb_dev_access_fail : 1;          /* [1]  */
        osal_u32 rg_dd_stage_check_fail : 1;             /* [2]  */
        osal_u32 rg_target_reg_addr_is_out_of_range : 1; /* [3]  */
        osal_u32 rg_int_usb_dev_is_fall_abnormal : 1;    /* [4]  */
        osal_u32 reserved_0 : 11;                        /* [15..5]  */
        osal_u32 rg_ih_gevntcount_invalid_cnt : 6;       /* [21..16]  */
        osal_u32 reserved_1 : 10;                        /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_sts;

/* Define the union u_usb_turbo_gevntcount_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_usb_dev_gevntcount_val : 16;    /* [15..0]  */
        osal_u32 rg_usb_dev_gevntcount_offset : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_gevntcount_cfg;

/* Define the union u_usb_turbo_cmd_setup_addr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_cmd_setup_addr : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;        /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_cmd_setup_addr;

/* Define the union u_usb_turbo_cmd_datain_addr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_cmd_datain_addr : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;         /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_cmd_datain_addr;

/* Define the union u_usb_turbo_cmd_dataout_addr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_cmd_dataout_addr : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;          /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb_turbo_cmd_dataout_addr;

#endif /* __C_UNION_DEFINE_USB_TURBO_REG_H__ */
