// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_GLB_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_GLB_CTL_RB_H__
#define __C_UNION_DEFINE_GLB_CTL_RB_H__

/* define the union u_hw_ctl */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 refclk_freq_status : 1; /* [0] */
        osal_u32 clk24m_coex : 1; /* [1]  */
        osal_u32 type_id : 2; /* [3..2] */
        osal_u32 debug_mode : 2; /* [5..4]  */
        osal_u32 reserved_0 : 26; /* [31..6]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_hw_ctl;

/* Define the union u_core_ra_wr_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 core_ra_wr_en : 1; /* [0] */
        osal_u32 reserved_0 : 31;   /* [31..1]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_core_ra_wr_en;

/* define the union u_cpu_dfx_bits_load */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 cpu_dfx_bits_load : 1; /* [0]  */
        osal_u32 reserved_0 : 31;       /* [31..1]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_cpu_dfx_bits_load;

/* define the union u_cpu_dfx_bits_clr */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 cpu_dfx_bits_clr : 1; /* [0]  */
        osal_u32 reserved_0 : 31;      /* [31..1]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_cpu_dfx_bits_clr;

/* define the union u_glb_clk_force_on */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 glb_clk_force_on : 1; /* [0]  */
        osal_u32 mem_clk_force_on : 1; /* [1]  */
        osal_u32 reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_glb_clk_force_on;

/* define the union u_hh503_glb_ctl_rb_pwr_on_lable_reg */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 hh503_glb_ctl_rb_pwr_on_lable_reg : 16; /* [15..0] */
        osal_u32 reserved_0 : 16;                        /* [31..16]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_hh503_glb_ctl_rb_pwr_on_lable_reg;

/* Define the union u_rst_recorder */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 rst_recorder : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28;  /* [31..4]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_rst_recorder;

/* define the union u_rst_record_clr */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 rst_record_clr : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28;    /* [31..4]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_rst_record_clr;

/* define the union u_wdt_rst_counter */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 wdt_rst_count : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;   /* [31..8]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_wdt_rst_counter;

/* define the union u_wdt_rst_counter_clr */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 wdt_rst_count_clr : 1; /* [0]  */
        osal_u32 reserved_0 : 31;       /* [31..1]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_wdt_rst_counter_clr;

/* Define the union u_uart1_rx_wake_base_func */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 uart1_rx_wake_func_en : 1; /* [0] */
        osal_u32 uart1_rx_wake_edge_sel : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_uart1_rx_wake_base_func;

/* Define the union u_uart1_rx_wake_int */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 uart1_rx_wake_int : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_uart1_rx_wake_int;

/* Define the union u_uart1_rx_wake_int_clr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 uart1_rx_wake_int_clr : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_uart1_rx_wake_int_clr;

/* Define the union u_uart1_rx_wake_int_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 uart1_rx_wake_int_en : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_uart1_rx_wake_int_en;

/* Define the union u_glb_ctl_wdt_soft_req */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 glb_ctl_wdt_soft_req : 1; /* [0]  */
        osal_u32 reserved_0 : 31;          /* [31..1]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_glb_ctl_wdt_soft_req;

/* define the union u_cfg_dfx */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 cfg_dfx_mux : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28; /* [31..4]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_cfg_dfx;

/* define the union u_cali_32k_tcxo_ctl */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 cali_32k_tcxo_en : 1;             /* [0]  */
        osal_u32 cali_32k_tcxo_in_process_sts : 1; /* [1]  */
        osal_u32 cali_32k_tcxo_done_sts : 1;       /* [2]  */
        osal_u32 reserved_0 : 29;                  /* [31..3]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_cali_32k_tcxo_ctl;

/* Define the union u_gp_reg14 */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 gp_reg14 : 31;     /* [30..0]  */
        osal_u32 sdio_int_req0 : 1; /* [31]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_gp_reg14;

/* define the union u_gp_reg15 */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 gp_reg15 : 31;     /* [30..0]  */
        osal_u32 sdio_int_req1 : 1; /* [31]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_gp_reg15;

/* define the union u_sdio_int_req_en */
typedef union {
    /* define the struct bits */
    struct {
        osal_u32 sdio_int_req0_en : 1; /* [0]  */
        osal_u32 sdio_int_req1_en : 1; /* [1]  */
        osal_u32 reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* define an unsigned member */
    osal_u32 u32;
} u_sdio_int_req_en;

#endif /* __c_union_define_glb_ctl_rb_h__ */
