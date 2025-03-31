// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_USB2_CRG_CFG_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_USB2_CRG_CFG_RB_H__
#define __C_UNION_DEFINE_USB2_CRG_CFG_RB_H__

/* define the union u_usb2_ctrl_crg_cfg */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 usb2_reset_req : 1;     /* [0]  */
        osal_u32 reserved_0 : 3;         /* [3..1]  */
        osal_u32 usb2_bus_cken : 1;      /* [4]  */
        osal_u32 reserved_1 : 1;         /* [5]  */
        osal_u32 reserved_2 : 2;         /* [7..6]  */
        osal_u32 usb2_utmi_cken : 1;     /* [8]  */
        osal_u32 reserved_3 : 7;         /* [15..9]  */
        osal_u32 usb2_freeclk_cksel : 1; /* [16]  */
        osal_u32 reserved_4 : 15;        /* [31..17]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_usb2_ctrl_crg_cfg;

/* define the union u_usb2_phy_crg_cfg0 */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 usb2_phy_rst_req : 1;       /* [0]  */
        osal_u32 usb2_phy_utmi_rst_req : 1;  /* [1]  */
        osal_u32 usb2_phy_apb_rst_req : 1;   /* [2]  */
        osal_u32 reserved_0 : 1;             /* [3]  */
        osal_u32 usb2_phy_ref_xtal_cken : 1; /* [4]  */
        osal_u32 usb2_phy_ref_pll_cken : 1;  /* [5]  */
        osal_u32 usb2_480m_cken : 1;         /* [6]  */
        osal_u32 reserved_1 : 25;            /* [31..7]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_usb2_phy_crg_cfg0;

/* define the union u_usb2_crg_cfg0 */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 usb_turbo_reset_req : 1; /* [0]  */
        osal_u32 usb_turbo_cken : 1;      /* [1]  */
        osal_u32 clk_tcxo_ini_cksel : 1;  /* [2]  */
        osal_u32 clk_ref_xtal_cksel : 1;  /* [3]  */
        osal_u32 reserved_0 : 28;         /* [31..4]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_usb2_crg_cfg0;

/* Define the union u_usb2_crg_div_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 usb2_refclk_pll_load_div_en : 1; /* [0] */
        osal_u32 usb2_refclk_pll_div : 5; /* [5..1] */
        osal_u32 reserved_0 : 26; /* [31..6] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_usb2_crg_div_cfg;

#endif /* __C_UNION_DEFINE_USB2_CRG_CFG_RB_H__ */
