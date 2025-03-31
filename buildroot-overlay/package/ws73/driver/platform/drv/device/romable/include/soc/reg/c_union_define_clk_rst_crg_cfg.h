// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_CLK_RST_CRG_CFG.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_CLK_RST_CRG_CFG_H__
#define __C_UNION_DEFINE_CLK_RST_CRG_CFG_H__

/* Define the union u_soc_top_crg_gp_reg0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 gp_reg0 : 16;    /* [15..0]  */
        osal_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soc_top_crg_gp_reg0;

/* Define the union u_soc_top_crg_gp_reg1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 gp_reg1 : 16;    /* [15..0]  */
        osal_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soc_top_crg_gp_reg1;

/* Define the union u_sysldo_crg_cken_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bgle_ahb_cken : 1;       /* [0]  */
        osal_u32 reserved_0 : 2;          /* [2..1] */
        osal_u32 uart_apb_cken : 1;       /* [3]  */
        osal_u32 rf_ctl_apb_clken : 1;    /* [4]  */
        osal_u32 diag_ctl_cken : 1;       /* [5]  */
        osal_u32 wdt_cken : 1;            /* [6]  */
        osal_u32 gpio_apb_cken : 1;       /* [7]  */
        osal_u32 lower_power_cken : 1;    /* [8]  */
        osal_u32 cs_bus_cken : 1;         /* [9]  */
        osal_u32 dma_cken         : 1;    /* [10] */
        osal_u32 monitor_wr_cken : 1;     /* [11]  */
        osal_u32 coex_cken : 1;           /* [12]  */
        osal_u32 ref_1m_cken : 1;         /* [13]  */
        osal_u32 reserved_1 : 1;          /* [14]  */
        osal_u32 mac_wifi_320m_cken : 1;  /* [15]  */
        osal_u32 uart_glp_cken : 1;       /* [16]  */
        osal_u32 wdt_tcxo_cken : 1;       /* [17]  */
        osal_u32 timer_cken : 1;          /* [18]  */
        osal_u32 efuse_ssi_tcxo_cken : 1; /* [19]  */
        osal_u32 hpm_cken : 1;            /* [20]  */
        osal_u32 uart_cken : 1;           /* [21]  */
        osal_u32 tsensor_cken : 1;        /* [22]  */
        osal_u32 usb2_apb_cken : 1;       /* [23]  */
        osal_u32 glb_sdio_cken : 1;       /* [24]  */
        osal_u32 sdio_cken : 1;           /* [25]  */
        osal_u32 cali_ref_cken : 1;       /* [26]  */
        osal_u32 sdio_tcxo_cken : 1;      /* [27]  */
        osal_u32 coex_apb_cken : 1;       /* [28]  */
        osal_u32 rf_wb_tcxo_cken : 1;     /* [29]  */
        osal_u32 ioconfig_apb_cken : 1;   /* [30]  */
        osal_u32 rtc_cken : 1;            /* [31]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_cken_ctl;

/* Define the union u_sysldo_crg_cksel_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_bus_cksel : 1;           /* [0]  */
        osal_u32 clk_uart_cksel : 1;          /* [1]  */
        osal_u32 clk_mac_wifi_320m_cksel : 1; /* [2]  */
        osal_u32 clk_monitor_wr_sel : 2;      /* [4..3]  */
        osal_u32 clk_rf_wb_160m_cksel : 1;    /* [5] */
        osal_u32 clk_uart_glp_cksel    : 1;   /* [6] */
        osal_u32 reserved_0            : 25;  /* [31..7] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_cksel_ctl;

/* Define the union u_sysldo_crg_opt_div_ctl0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 1; /* [0] */
        osal_u32 clk_bus_load_div_en : 1; /* [1] */
        osal_u32 reserved_1 : 2; /* [3..2] */
        osal_u32 clk_bus_div1_num : 4;  /* [7..4]  */
        osal_u32 clk_bus_div2_num : 4;  /* [11..8]  */
        osal_u32 reserved_2 : 4; /* [15..12] */
        osal_u32 reserved_3 : 1; /* [16] */
        osal_u32 clk_apb_load_div_en : 1; /* [17] */
        osal_u32 reserved_4 : 2; /* [19..18] */
        osal_u32 clk_apb_div1_num : 3;  /* [22..20]  */
        osal_u32 clk_apb_div2_num : 3;  /* [25..23]  */
        osal_u32 trng_ring_en : 1; /* [26] */
        osal_u32 reserved_5 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_opt_div_ctl0;

/* Define the union u_clk_sel_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_uart_clka_sts : 1; /* [0] */
        osal_u32 clk_uart_clkb_sts : 1; /* [1] */
        osal_u32 clk_uart_glp_clka_sts : 1; /* [2] */
        osal_u32 clk_uart_glp_clkb_sts : 1; /* [3] */
        osal_u32 clk_240m_bus_clka_sts : 1; /* [4] */
        osal_u32 clk_240m_bus_clkb_sts : 1; /* [5] */
        osal_u32 reserved_0 : 2; /* [7..6] */
        osal_u32 clk_rf_wb_160m_clka_sts : 1; /* [8] */
        osal_u32 clk_rf_wb_160m_clkb_sts : 1; /* [9] */
        osal_u32 reserved_1 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_clk_sel_sts;

/* Define the union u_glb_rst_crg_ctl0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wdt_rst_all_chip_mask : 1; /* [0]  */
        osal_u32 reserved_0 : 3;            /* [3..1]  */
        osal_u32 chip_rst_req : 1;          /* [4]  */
        osal_u32 reserved_1 : 3;            /* [7..5]  */
        osal_u32 reserved_2 : 24;           /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_glb_rst_crg_ctl0;

/* Define the union u_glb_rst_crg_ctl1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 8; /* [7..0] */
        osal_u32 sdio_delay_time_rst_logic : 10; /* [17..8] */
        osal_u32 reserved_1 : 2; /* [19..18] */
        osal_u32 sdio_delay_time_rst_crg : 10; /* [29..20] */
        osal_u32 reserved_2 : 2; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_glb_rst_crg_ctl1;

/* Define the union u_sysldo_soft_rst_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_rst_cs_jtag_n : 1;     /* [0]  */
        osal_u32 soft_rst_pmu_cmu_n : 1;     /* [1]  */
        osal_u32 soft_rst_ioconfig_n : 1;    /* [2]  */
        osal_u32 soft_rst_gpio_n : 1;        /* [3]  */
        osal_u32 soft_rst_cpu_por_pluse : 1; /* [4]  */
        osal_u32 soft_rst_bt_crg_glb_n : 1;  /* [5]  */
        osal_u32 soft_rst_lower_power_n : 1; /* [6]  */
        osal_u32 soft_rst_wifi_n : 1;        /* [7]  */
        osal_u32 soft_rst_efuse_n : 1;       /* [8]  */
        osal_u32 soft_rst_tsensor_n : 1;     /* [9]  */
        osal_u32 soft_rst_aoncrgrst_ctrl : 1;    /* [10]  */
        osal_u32 soft_rst_trng_n : 1;        /* [11]  */
        osal_u32 soft_rst_wdt_n : 1;         /* [12]  */
        osal_u32 soft_rst_timer_n : 1;       /* [13]  */
        osal_u32 soft_rst_uart_n : 1;        /* [14]  */
        osal_u32 soft_rst_uart_glp_n : 1;    /* [15]  */
        osal_u32 soft_rst_hpm_n : 1;         /* [16]  */
        osal_u32 soft_rst_rf_wb_n : 1;       /* [17]  */
        osal_u32 soft_rst_coex_n : 1;        /* [18]  */
        osal_u32 soft_rst_cpu_trace_n : 1;   /* [19]  */
        osal_u32 xCiNLw_CLwSihBLi_wh_ : 1;     /* [20]  */
        osal_u32 soft_rst_diag_n : 1;        /* [21]  */
        osal_u32 soft_rst_pkt_ram_n : 1;     /* [22]  */
        osal_u32 soft_rst_bt_sub_n : 1;      /* [23]  */
        osal_u32 soft_rst_sd_n : 1;          /* [24]  */
        osal_u32 soft_rst_rtc_n : 1;         /* [25]  */
        osal_u32 soft_rst_dma_n   : 1;       /* [26] */
        osal_u32 reserved_0       : 5;       /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_soft_rst_ctl;

/* Define the union u_cpu_por_pluse_rstsync */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cpu_por_pulse_width : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;         /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cpu_por_pluse_rstsync;

/* Define the union u_phy_wlmac_crg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 xCiNLw_CLwT6udwSWSw6Lowh_ : 1; /* [0]  */
        osal_u32 reserved_0 : 1;              /* [1]  */
        osal_u32 xSG6wSGBhwFBqwhdS_ : 6;        /* [7..2]  */
        osal_u32 xSG6wSGBhw6oKWh_ : 1;          /* [8]  */
        osal_u32 xTLiuwFBqwWh_ : 1;             /* [9]  */
        osal_u32 reserved_1 : 22;             /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_phy_wlmac_crg;

/* Define the union u_pmu_cmu */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cmu_ctrl_cken : 1; /* [0]  */
        osal_u32 reserved_0 : 31;       /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cmu;

/* Define the union u_sysldo_crg_cken_ctl1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_hadm_ck_sync_cken : 1;     /* [0]  */
        osal_u32 reserved_0 : 1; /* [1]  */
        osal_u32 bgle_32m_div1_cken : 1;        /* [2]  */
        osal_u32 bgle_32m_div0_cken : 1;        /* [3]  */
        osal_u32 reserved_1 : 28;               /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_cken_ctl1;

/* Define the union u_sysldo_crg_div_ctl0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_sdio_occ_div_en : 1; /* [0] */
        osal_u32 clk_sdio_occ_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 clk_sdio_occ_div_num : 4; /* [7..4] */
        osal_u32 reserved_1 : 8; /* [15..8] */
        osal_u32 clk_uart_div_en : 1; /* [16] */
        osal_u32 clk_uart_load_div_en : 1; /* [17] */
        osal_u32 reserved_2 : 2; /* [19..18] */
        osal_u32 clk_uart_div1_num : 5; /* [24..20] */
        osal_u32 clk_uart_div2_num : 5; /* [29..25] */
        osal_u32 reserved_3 : 2; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl0;

/* Define the union u_sysldo_crg_div_ctl1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_timer_tcxo_div_en : 1; /* [0] */
        osal_u32 clk_timer_tcxo_load_div_en : 1; /* [1] */
        osal_u32 clk_timer_tcxo_div1_num : 7; /* [8..2] */
        osal_u32 clk_timer_tcxo_div2_num : 7; /* [15..9] */
        osal_u32 clk_wdt_tcxo_div_en : 1; /* [16] */
        osal_u32 clk_wdt_tcxo_load_div_en : 1; /* [17] */
        osal_u32 clk_wdt_tcxo_div1_num : 7; /* [24..18] */
        osal_u32 clk_wdt_tcxo_div2_num : 7; /* [31..25] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl1;

/* Define the union u_sysldo_crg_div_ctl2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_ref_tcxo_div_en : 1; /* [0] */
        osal_u32 clk_ref_tcxo_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 clk_ref_tcxo_div1_num : 6; /* [9..4] */
        osal_u32 clk_ref_tcxo_div2_num : 6; /* [15..10] */
        osal_u32 clk_hpm1m_tcxo_div_en : 1; /* [16] */
        osal_u32 clk_hpm1m_tcxo_load_div_en : 1; /* [17] */
        osal_u32 reserved_1 : 2; /* [19..18] */
        osal_u32 clk_hpm1m_tcxo_div1_num : 6; /* [25..20] */
        osal_u32 clk_hpm1m_tcxo_div2_num : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl2;

/* Define the union u_sysldo_crg_div_ctl3 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_rtc1m_tcxo_div_en : 1; /* [0] */
        osal_u32 clk_rtc1m_tcxo_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 clk_rtc1m_tcxo_div1_num : 6; /* [9..4] */
        osal_u32 clk_rtc1m_tcxo_div2_num : 6; /* [15..10] */
        osal_u32 clk_tsensor1m_tcxo_div_en : 1; /* [16] */
        osal_u32 clk_tsensor1m_tcxo_load_div_en : 1; /* [17] */
        osal_u32 reserved_1 : 2; /* [19..18] */
        osal_u32 clk_tsensor1m_tcxo_div1_num : 6; /* [25..20] */
        osal_u32 clk_tsensor1m_tcxo_div2_num : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl3;

/* Define the union u_sysldo_crg_div_ctl4 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_trng_div_en : 1; /* [0] */
        osal_u32 clk_trng_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 clk_trng_div1_num : 3; /* [6..4] */
        osal_u32 reserved_1 : 1; /* [7] */
        osal_u32 clk_trng_div2_num : 3; /* [10..8] */
        osal_u32 reserved_2 : 5; /* [15..11] */
        osal_u32 clk_rf_wb_div_en : 1; /* [16] */
        osal_u32 clk_rf_wb_load_div_en : 1; /* [17] */
        osal_u32 reserved_3 : 2; /* [19..18] */
        osal_u32 clk_rf_wb_div1_num : 2; /* [21..20] */
        osal_u32 clk_rf_wb_div2_num : 2; /* [23..22] */
        osal_u32 reserved_4 : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl4;

/* Define the union u_sysldo_crg_div_ctl5 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 clk_lower_power_div_en : 1; /* [0] */
        osal_u32 clk_lower_power_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 clk_lower_power_div1_num : 12; /* [15..4] */
        osal_u32 clk_lower_power_div2_num : 12; /* [27..16] */
        osal_u32 reserved_1 : 4; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl5;

/* Define the union u_sysldo_crg_div_ctl6 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 1; /* [0] */
        osal_u32 cldo_hpm_80m_load_div_en : 1; /* [1] */
        osal_u32 reserved_1 : 2; /* [3..2] */
        osal_u32 cldo_hpm_80m_div_num : 6; /* [9..4] */
        osal_u32 clk_uart_glp_div_en   : 1; /* [10] */
        osal_u32 clk_uart_glp_load_div_en : 1; /* [11] */
        osal_u32 clk_uart_glp_div1_num : 4; /* [15..12] */
        osal_u32 clk_uart_glp_div2_num : 4; /* [19..16] */
        osal_u32 reserved_2            : 12; /* [31..20] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl6;

/* Define the union u_sysldo_crg_div_ctl7 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bgle_32m_div0_div_en : 1; /* [0] */
        osal_u32 bgle_32m_div0_load_div_en : 1; /* [1] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 bgle_32m_div0_div_num : 4; /* [7..4] */
        osal_u32 reserved_1 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_crg_div_ctl7;
 
typedef union {
    struct {
        osal_u32    uart0_p2p_bypass      : 1; /* [0] */
        osal_u32    uart1_p2p_bypass      : 1; /* [1] */
        osal_u32    reserved_0            : 30; /* [31..2] */
    } bits;
    osal_u32    u32;
} u_uart_p2p_bridg_bypass;

#endif /* __C_UNION_DEFINE_CLK_RST_CRG_CFG_H__ */
