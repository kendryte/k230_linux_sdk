// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_RF_WB_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_RF_WB_CTL_RB_H__
#define __C_UNION_DEFINE_RF_WB_CTL_RB_H__

/* Define the union u_rf_wl_reg_clk_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rf_wl_reg_clken_man : 1; /* [0]  */
        osal_u32 rf_wl_reg_cg_mod : 1;    /* [1]  */
        osal_u32 reserved_0 : 30;         /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_wl_reg_clk_ctl;

/* Define the union u_rf_wl_reg_soft_rstn */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rf_wl_reg_soft_rstn : 1; /* [0]  */
        osal_u32 reserved_0 : 31;         /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_wl_reg_soft_rstn;

/* Define the union u_rf_wl_reg_cmd_soft_rstn */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_rst_rf_cmd_fifo_n : 1; /* [0]  */
        osal_u32 cfg_wl_cmd_fifo_man_en : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_wl_reg_cmd_soft_rstn;

/* Define the union u_rf_wl_reg_cmd_fifo_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rf_cmd_fifo_empty : 1;    /* [0]  */
        osal_u32 rf_cmd_fifo_full : 1;     /* [1]  */
        osal_u32 rf_cmd_fifo_full_stk : 1; /* [2]  */
        osal_u32 reserved_0 : 29;          /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_wl_reg_cmd_fifo_sts;

/* Define the union u_rf_wl_cbb_dis */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cbb_wl_rf_reg_wr_dis : 1; /* [0] */
        osal_u32 cbb_wl_rf_reg_rd_dis : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_wl_cbb_dis;

/* Define the union u_rf_bt_cbb_dis */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cbb_bt_rf0_reg_wr_dis : 1; /* [0] */
        osal_u32 cbb_bt_rf0_reg_rd_dis : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_bt_cbb_dis;

/* Define the union u_abb_adc_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_wlan_adc_iq_swap_en : 1; /* [0] */
        osal_u32 cfg_wlan_adc_msb_en : 1; /* [1] */
        osal_u32 cfg_wlan_adc_cken : 1; /* [2] */
        osal_u32 soft_wlan_adc_rst_n : 1; /* [3] */
        osal_u32 cfg_bgle_adc_iq_swap_en : 1; /* [4] */
        osal_u32 cfg_bgle_adc_msb_en : 1; /* [5] */
        osal_u32 cfg_bgle_adc_cken : 1; /* [6] */
        osal_u32 soft_bgle_adc_rst_n : 1; /* [7] */
        osal_u32 cfg_adc_inv_en : 1; /* [8] */
        osal_u32 reserved_0 : 23; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_abb_adc_ctrl;

/* Define the union u_abb_dac_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_dac_iq_swap_en : 1; /* [0] */
        osal_u32 cfg_dac_msb_en : 1; /* [1] */
        osal_u32 cfg_dac_inv_en : 1; /* [2] */
        osal_u32 cfg_dac_cken : 1; /* [3] */
        osal_u32 soft_dac_rst_n : 1; /* [4] */
        osal_u32 wifi_ble_dac_sw_man : 1;    /* [5]  */
        osal_u32 wifi_ble_dac_sw_man_en : 1; /* [6]  */
        osal_u32 reserved_0 : 25; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_abb_dac_ctrl;

/* Define the union u_abb_en_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_wb_adc_en_i : 1; /* [0] */
        osal_u32 cfg_wb_adc_en_q : 1; /* [1] */
        osal_u32 cfg_wb_adc_rstn : 1; /* [2] */
        osal_u32 cfg_wb_adc_man_en : 1; /* [3] */
        osal_u32 cfg_wb_dac_en_i : 1; /* [4] */
        osal_u32 cfg_wb_dac_en_q : 1; /* [5] */
        osal_u32 cfg_wb_dac_rstn : 1; /* [6] */
        osal_u32 cfg_wb_dac_man_en : 1; /* [7] */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_abb_en_ctrl;

/* Define the union u_cfg_rf_man_en_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_rx_en_man_en : 1; /* [0] */
        osal_u32 cfg_tx_en_man_en : 1; /* [1] */
        osal_u32 cfg_adc_standby_man_en : 1; /* [2] */
        osal_u32 cfg_dac_standby_man_en : 1; /* [3] */
        osal_u32 cfg_fb_en_man_en : 1; /* [4] */
        osal_u32 cfg_fb_phase_man_en : 1; /* [5] */
        osal_u32 cfg_ipa_en_man_en : 1; /* [6] */
        osal_u32 cfg_over_temp_prt_man_en : 1; /* [7] */
        osal_u32 cfg_mode_sel_man_en : 1; /* [8] */
        osal_u32 cfg_dcoc_i_man_en : 1; /* [9] */
        osal_u32 cfg_dcoc_q_man_en : 1; /* [10] */
        osal_u32 cfg_dac_index_man_en : 1; /* [11] */
        osal_u32 cfg_ppa_code_man_en : 1; /* [12] */
        osal_u32 cfg_pa_code_man_en : 1; /* [13] */
        osal_u32 cfg_pa_bias_ctrl_man_en : 1; /* [14] */
        osal_u32 cfg_lpf_mode_man_en : 1; /* [15] */
        osal_u32 cfg_lpf_bw_man_en : 1; /* [16] */
        osal_u32 cfg_lpf_gain_man_en : 1; /* [17] */
        osal_u32 cfg_lna_gain_man_en : 1; /* [18] */
        osal_u32 cfg_fb_gain_man_en : 1; /* [19] */
        osal_u32 cfg_vga_gain_man_en : 1; /* [20] */
        osal_u32 cfg_temp_code_man_en : 1; /* [21] */
        osal_u32 reserved_0 : 10; /* [31..22] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_rf_man_en_ctrl;

/* Define the union u_cfg_trxen_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rx_en_man : 1; /* [0] */
        osal_u32 tx_en_man : 1; /* [1] */
        osal_u32 adc_standby_man : 1; /* [2] */
        osal_u32 dac_standby_man : 1; /* [3] */
        osal_u32 ipa_en_man : 1; /* [4] */
        osal_u32 over_temp_prt_man : 1; /* [5] */
        osal_u32 mode_sel_man : 1; /* [6] */
        osal_u32 fb_phase_man : 1; /* [7] */
        osal_u32 fb_en_man : 2; /* [9..8] */
        osal_u32 fb_gain_man : 3; /* [12..10] */
        osal_u32 reserved_0 : 19; /* [31..13] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_trxen_ctrl;

/* Define the union u_cfg_dcoc_iq_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 dcoc_i_man : 8; /* [7..0] */
        osal_u32 dcoc_q_man : 8; /* [15..8] */
        osal_u32 cfg_dcoc_swap_en : 1; /* [16] */
        osal_u32 cfg_dcoc_msb_en : 1; /* [17] */
        osal_u32 reserved_0 : 14; /* [31..18] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_dcoc_iq_ctrl;

/* Define the union u_cfg_pa_idx_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 dac_index_man : 2; /* [1..0] */
        osal_u32 reserved_0 : 2; /* [3..2] */
        osal_u32 pa_bias_ctrl_man : 3; /* [6..4] */
        osal_u32 reserved_1 : 25; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_pa_idx_ctrl;

/* Define the union u_cfg_lpf_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 lpf_mode_man : 1; /* [0] */
        osal_u32 lpf_bw_man : 2; /* [2..1] */
        osal_u32 cfg_lpf_mode_inv_en : 1; /* [3] */
        osal_u32 lpf_gain_man : 3; /* [6..4] */
        osal_u32 reserved_0 : 1; /* [7] */
        osal_u32 lna_gain_man : 3; /* [10..8] */
        osal_u32 vga_gain_man : 5; /* [15..11] */
        osal_u32 reserved_1 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_lpf_ctrl;

/* Define the union u_cfg_temp_ctrl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 temp_code_man : 4; /* [3..0] */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_temp_ctrl;

/* Define the union u_cfg_rf_diag_mux */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_rf_diag_mux : 2; /* [1..0] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_rf_diag_mux;

/* Define the union u_cfg_wl_adc_mux */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_wladc_dmux_man : 1; /* [0] */
        osal_u32 cfg_wladc_dmux_man_en : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_wl_adc_mux;

/* Define the union u_cfg_ppa_code */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 ppa_code_man : 16; /* [15..0] */
        osal_u32 pa_code_man : 8; /* [23..16] */
        osal_u32 reserved_0 : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_ppa_code;
#endif /* __C_UNION_DEFINE_RF_WB_CTL_RB_H__ */
