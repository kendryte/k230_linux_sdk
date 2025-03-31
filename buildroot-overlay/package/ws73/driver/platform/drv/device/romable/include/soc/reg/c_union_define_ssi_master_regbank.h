/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  SSI_MASTER_REGBANK_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_SSI_MASTER_REGBANK_H__
#define __C_UNION_DEFINE_SSI_MASTER_REGBANK_H__

/* Define the union u_ahb_size_16_32 */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: ahb_size_16_32
        /*
         * ahb总线操作位宽的配置寄存器
         * 0:16bit
         * 1:32bit
         */
        osal_u32 ahb_size_16_32 : 1; /* [0] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 15; /* [15..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_ahb_size_16_32;

/* Define the union u_sm_status */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: ahb_hready
        /*
         * AHB SLAVE的hready为高时，寄存hready
         */
        osal_u32 ahb_hready : 1; /* [0] */
        // Comment of field: ahb_hresp
        /*
         * AHB SLAVE的hready为高时，寄存hresp
         */
        osal_u32 ahb_hresp : 2; /* [2..1] */
        // Comment of field: ahb_hresp_err
        /*
         * 读写AHB发生错误时，记录AHB SLAVE反馈信号hresp
         */
        osal_u32 ahb_hresp_err : 2; /* [4..3] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 11; /* [15..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_sm_status;

/* Define the union u_sm_clear */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: clear
        /*
         * 该寄存器被写1时清零sm_status、sm_err_haddr和sm_err_laddr以及check_st
         */
        osal_u32 clear : 1; /* [0] */
        osal_u32 reserved_0 : 15; /* [15..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_sm_clear;

/* Define the union u_aon_clksel */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: ssi_aon_clksel
        /*
         * 0：AON选择TCXO时钟；
         * 1：AON选择SSI时钟
         */
        osal_u32 ssi_aon_clksel : 1; /* [0] */
        osal_u32 reserved_0 : 15; /* [15..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_aon_clksel;

/* Define the union u_sel_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: ssi_rst_sel
        /*
         * 0：由硬件控制
         * 1：由SSI控制
         */
        osal_u32 ssi_rst_sel : 1; /* [0] */
        // Comment of field: ssi_rst_pmu_sel
        /*
         * 0：由硬件控制
         * 1：由SSI控制
         */
        osal_u32 ssi_rst_pmu_sel : 1; /* [1] */
        // Comment of field: ssi_rst_aon_lgc_sel
        /*
         * 0：由硬件控制
         * 1：由SSI控制
         */
        osal_u32 ssi_rst_aon_lgc_sel : 1; /* [2] */
        // Comment of field: ssi_rst_aon_crg_sel
        /*
         * 0：由硬件控制
         * 1：由SSI控制
         */
        osal_u32 ssi_rst_aon_crg_sel : 1; /* [3] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 4; /* [7..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16 : 8;
} u_sel_ctrl;

/* Define the union u_ssi_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: ssi_rst_n
        /*
         * 0：复位
         * 1：解复位
         */
        osal_u32 ssi_rst_n : 1; /* [0] */
        // Comment of field: ssi_rst_pmu_n
        /*
         * 0：复位
         * 1：解复位
         */
        osal_u32 ssi_rst_pmu_n : 1; /* [1] */
        // Comment of field: ssi_rst_aon_lgc_n
        /*
         * 0：复位
         * 1：解复位
         */
        osal_u32 ssi_rst_aon_lgc_n : 1; /* [2] */
        // Comment of field: ssi_rst_aon_crg_n
        /*
         * 0：复位
         * 1：解复位
         */
        osal_u32 ssi_rst_aon_crg_n : 1; /* [3] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 4; /* [7..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16 : 8;
} u_ssi_ctrl;

/* Define the union u_ssi_diag */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 ssi_hbusreq : 1; /* [0] */
        osal_u32 ssi_hgrant : 1; /* [1] */
        // Comment of field: reserved_0
        /*
         * 保留
         */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_ssi_diag;

#endif /* __SSI_MASTER_REGBANK_C_UNION_DEFINE_H__ */
