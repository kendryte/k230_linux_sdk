// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_CMU_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_CMU_CTL_RB_H__
#define __C_UNION_DEFINE_CMU_CTL_RB_H__

/* Define the union u_dbg_cmu_xo_pd */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 dbg_cmu_xo_pd : 1; /* [0] */
        osal_u32 dbg_cmu_xo_pd_sel : 1; /* [1] */
        osal_u32 dbg_clk_xo_ini_pd : 1; /* [2] */
        osal_u32 dbg_clk_xo_ini_pd_sel : 1; /* [3] */
        osal_u32 dbg_cmu_clkout_pd : 1; /* [4] */
        osal_u32 dbg_cmu_clkout_pd_sel : 1; /* [5] */
        osal_u32 clk_xo_rf_pd : 1; /* [6] */
        osal_u32 reserved_0 : 25; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_dbg_cmu_xo_pd;

/* Define the union u_cmu_xo_sig */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_cmu_xo_trim_fine : 8; /* [7..0] */
        osal_u32 rg_cmu_xo_trim_coarse : 4; /* [11..8] */
        osal_u32 rg_cmu_xo_trim_rev : 4; /* [15..12] */
        osal_u32 rg_cmu_xo_trim_fine_sel : 1; /* [16] */
        osal_u32 rg_cmu_xo_trim_coarse_sel : 1; /* [17] */
        osal_u32 rg_cmu_xo_trim_rev_sel : 1; /* [18] */
        osal_u32 reserved_0 : 13; /* [31..19] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_xo_sig;

/* Define the union u_cmu_fnpll_sig */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_postdiv2 : 3; /* [2..0] */
        osal_u32 cmu_postdiv1 : 3; /* [5..3] */
        osal_u32 cmu_prediv : 6; /* [11..6] */
        osal_u32 cmu_fout2xpd : 1; /* [12] */
        osal_u32 cmu_fout4phasepd : 1; /* [13] */
        osal_u32 cmu_foutpostdivpd : 1; /* [14] */
        osal_u32 cmu_fnpll_pd : 1; /* [15] */
        osal_u32 cmu_fnpll_bypass : 1; /* [16] */
        osal_u32 cmu_fnpll_mode : 1; /* [17] */
        osal_u32 reserved_0 : 14; /* [31..18] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_fnpll_sig;

/* Define the union u_cmu_fbdiv */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_fbdiv : 12; /* [11..0] */
        osal_u32 reserved_0 : 20; /* [31..12] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_fbdiv;

/* Define the union u_cmu_frac */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_frac : 24; /* [23..0] */
        osal_u32 reserved_0 : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_frac;

/* Define the union u_cmu_clk_wf_rx */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_wf_rx_rstn_sync : 1; /* [0] */
        osal_u32 rg_cmu_div_wf_rx_rstn : 1; /* [1] */
        osal_u32 cmu_div_wf_rx_cfg : 2; /* [3..2] */
        osal_u32 rg_cmu_div_wf_rx_rstn_sel : 1; /* [4] */
        osal_u32 reserved_0 : 27; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_wf_rx;

/* Define the union u_cmu_clk_bt_rx */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_bt_rx_rstn_sync : 1; /* [0] */
        osal_u32 rg_cmu_div_bt_rx_rstn : 1; /* [1] */
        osal_u32 cmu_div_bt_rx_cfg : 2; /* [3..2] */
        osal_u32 rg_cmu_div_bt_rx_rstn_sel : 1; /* [4] */
        osal_u32 reserved_0 : 27; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_bt_rx;

/* Define the union u_cmu_clk_wf_tx */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_wf_tx_rstn_sync : 1; /* [0] */
        osal_u32 rg_cmu_div_wf_tx_rstn : 1; /* [1] */
        osal_u32 cmu_div_wf_tx_cfg : 2; /* [3..2] */
        osal_u32 rg_cmu_div_wf_tx_rstn_sel : 1; /* [4] */
        osal_u32 reserved_0 : 27; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_wf_tx;

/* Define the union u_cmu_clk_bt_tx */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_bt_tx_rstn_sync : 1; /* [0] */
        osal_u32 rg_cmu_div_bt_tx_rstn : 1; /* [1] */
        osal_u32 cmu_div_bt_tx_cfg : 2; /* [3..2] */
        osal_u32 rg_cmu_div_bt_tx_rstn_sel : 1; /* [4] */
        osal_u32 reserved_0 : 27; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_bt_tx;

/* Define the union u_cmu_clk_320m_wdbb */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_wdbb_320_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_wdbb_320_rstn : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_320m_wdbb;

/* Define the union u_cmu_clk_480m_wdbb */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_wdbb_480_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_wdbb_480_rstn : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_480m_wdbb;

/* Define the union u_cmu_clk_480m_usbdft */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_usbdft_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_usbdft_rstn : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_480m_usbdft;

/* Define the union u_cmu_clk_24m_usb */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_usb_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_usb_rstn : 1; /* [1] */
        osal_u32 cmu_div_usb_cfg : 2; /* [3..2] */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clk_24m_usb;

/* Define the union u_cmu_clkout0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_clkout0_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_clkout0_rstn : 1; /* [1] */
        osal_u32 cmu_div_clkout0_cfg : 1; /* [2] */
        osal_u32 reserved_0 : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clkout0;

/* Define the union u_cmu_clkout1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_div_clkout1_rstn_sync : 1; /* [0] */
        osal_u32 cmu_div_clkout1_rstn : 1; /* [1] */
        osal_u32 cmu_div_clkout1_cfg : 1; /* [2] */
        osal_u32 reserved_0 : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu_clkout1;

/* Define the union u_coex_pta_switch_cmu_rf_man */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 dbg_coex_pta_switch_cmu_man : 1; /* [0] */
        osal_u32 dbg_coex_pta_switch_rf_man : 1; /* [1] */
        osal_u32 dbg_coex_pta_switch_cmu_sel : 1; /* [2] */
        osal_u32 dbg_coex_pta_switch_rf_sel : 1; /* [3] */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_coex_pta_switch_cmu_rf_man;

/* Define the union u_pmu_cmu_rb_rst_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_rb_rst_sel : 1; /* [0] */
        osal_u32 pmu_rb_rst_sel_0 : 1; /* [1] */
        osal_u32 pmu_rb_rst_sel_1 : 1; /* [2] */
        osal_u32 reserved_0 : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cmu_rb_rst_sel;

#endif /* __C_UNION_DEFINE_CMU_CTL_RB_H__ */
