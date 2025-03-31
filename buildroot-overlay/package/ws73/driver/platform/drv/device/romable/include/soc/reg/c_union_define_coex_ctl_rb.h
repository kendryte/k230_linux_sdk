// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_COEX_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_COEX_CTL_RB_H__
#define __C_UNION_DEFINE_COEX_CTL_RB_H__

/* Define the union u_coex_soft_diag */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_soft_diag : 5; /* [4..0] */
        osal_u32 reserved_0 : 11; /* [15..5] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_coex_soft_diag;

/* Define the union u_cfg_coex_diag_sel0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_diag_bit0_sel : 5; /* [4..0] */
        osal_u32 cfg_coex_diag_bit1_sel : 5; /* [9..5] */
        osal_u32 cfg_coex_diag_bit2_sel : 5; /* [14..10] */
        osal_u32 reserved_0 : 1; /* [15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_coex_diag_sel0;

/* Define the union u_cfg_coex_diag_sel1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_diag_bit3_sel : 5; /* [4..0] */
        osal_u32 cfg_coex_diag_bit4_sel : 5; /* [9..5] */
        osal_u32 cfg_coex_diag_bit5_sel : 5; /* [14..10] */
        osal_u32 reserved_0 : 1; /* [15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_coex_diag_sel1;

/* Define the union u_cfg_coex_diag_sel2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_diag_bit6_sel : 5; /* [4..0] */
        osal_u32 cfg_coex_diag_bit7_sel : 5; /* [9..5] */
        osal_u32 reserved_0 : 6; /* [15..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_coex_diag_sel2;

/* Define the union u_int_en_wl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_int_en_r4 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;       /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_en_wl;

/* Define the union u_int_en_bt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_bt_int_en_m3 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;       /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_en_bt;

/* Define the union u_int_set_wl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_int_set_r4 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_set_wl;

/* Define the union u_int_set_bt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_bt_int_set_m3 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_set_bt;

/* Define the union u_int_sts_wl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_bt_int_sts_r4 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_sts_wl;

/* Define the union u_int_sts_bt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_int_sts_m3 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_sts_bt;

/* Define the union u_int_clr_wl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_bt_int_clr_r4 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_clr_wl;

/* Define the union u_int_clr_bt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_int_clr_m3 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;        /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_clr_bt;

/* Define the union u_int_mask_wl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_int_mask_r4 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;         /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_mask_wl;

/* Define the union u_int_mask_bt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_bt_int_mask_m3 : 1; /* [0]  */
        osal_u32 reserved_0 : 15;         /* [15..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_int_mask_bt;

/* Define the union u_wifi_resume_int */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wifi_resume_int_clr : 1; /* [0] */
        osal_u32 coex_wifi_resume_int_mask : 1; /* [1] */
        osal_u32 coex_wifi_resume_raw_int : 1; /* [2] */
        osal_u32 coex_wifi_resume_int : 1; /* [3] */
        osal_u32 reserved_0 : 12; /* [15..4] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_wifi_resume_int;

/* Define the union u_soft_rst */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wl_bt_soft_rst_n : 1; /* [0]  */
        osal_u32 wl_soft_rst_n : 1;    /* [1]  */
        osal_u32 bt_soft_rst_n : 1;    /* [2]  */
        osal_u32 reserved_0 : 13;      /* [15..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_soft_rst;

/* Define the union u_cfg_force_priority */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_force_bsle_prio_higher : 1; /* [0] */
        osal_u32 reserved_0 : 15; /* [15..1] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u16;
} u_cfg_force_priority;

/* Define the union u_cfg_wlan_abort_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wp_bt_rx_abort_en : 1; /* [0] */
        osal_u32 cfg_wl_tx_abort_mode : 1; /* [1] */
        osal_u32 cfg_wl_rx_abort_mode : 1; /* [2] */
        osal_u32 cfg_wl_tx_abort_cnt_en : 1; /* [3] */
        osal_u32 cfg_wl_rx_abort_cnt_en : 1; /* [4] */
        osal_u32 cfg_rf_prot_en : 1; /* [5] */
        osal_u32 reserved_0 : 10; /* [15..6] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_wlan_abort_en;

/* Define the union u_cfg_switch_rf_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_pta_switch_rf_cnt : 8; /* [7..0] */
        osal_u32 reserved_0 : 8; /* [15..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_switch_rf_cnt;

/* Define the union u_cfg_rf_w2b_prot_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_rf_w2b_prot_cnt : 13; /* [12..0] */
        osal_u32 reserved_0 : 3; /* [15..13] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_rf_w2b_prot_cnt;

/* Define the union u_cfg_rf_b2w_prot_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_rf_b2w_prot_cnt : 13; /* [12..0] */
        osal_u32 reserved_0 : 3; /* [15..13] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_rf_b2w_prot_cnt;

/* Define the union u_coex_pta0_curr_state */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_pta0_curr_state : 3; /* [2..0] */
        osal_u32 reserved_0 : 5; /* [7..3] */
        osal_u32 coex_pta0_wl0_selected : 1; /* [8] */
        osal_u32 coex_pta0_wl0_rx_abort : 1; /* [9] */
        osal_u32 coex_pta0_wl0_tx_abort : 1; /* [10] */
        osal_u32 reserved_1 : 1; /* [11] */
        osal_u32 coex_pta0_btdm_selected : 1; /* [12] */
        osal_u32 coex_pta0_btdm_denied : 1; /* [13] */
        osal_u32 reserved_2 : 2; /* [15..14] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_coex_pta0_curr_state;

/* Define the union u_rpt_coex_result */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 coex_wl_rx_abort : 1; /* [0] */
        osal_u32 coex_wl_tx_abort : 1; /* [1] */
        osal_u32 coex_wl_selected : 1; /* [2] */
        osal_u32 reserved_0 : 1; /* [3] */
        osal_u32 coex_pta_switch_rf : 1; /* [4] */
        osal_u32 reserved_1 : 3; /* [7..5] */
        osal_u32 coex_pta_btdm_denied : 1; /* [8] */
        osal_u32 coex_pta_btdm_selected : 1; /* [9] */
        osal_u32 reserved_2 : 6; /* [15..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_rpt_coex_result;

/* Define the union u_cfg_coex_pta_wl_rx_abort_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_pta_wl_rx_abort_sel : 1; /* [0] */
        osal_u32 cfg_coex_pta_wl_tx_abort_sel : 1; /* [1] */
        osal_u32 cfg_coex_pta_wl_selected_sel : 1; /* [2] */
        osal_u32 reserved_0 : 1; /* [3] */
        osal_u32 cfg_coex_pta_switch_rf_sel : 1; /* [4] */
        osal_u32 reserved_1 : 3; /* [7..5] */
        osal_u32 cfg_pta_btdm_denied_sel : 1; /* [8] */
        osal_u32 cfg_pta_btdm_selected_sel : 1; /* [9] */
        osal_u32 reserved_2 : 6; /* [15..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_coex_pta_wl_rx_abort_sel;

/* Define the union u_cfg_coex_pta_wl_rx_abort_man */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_pta_wl_rx_abort_man : 1; /* [0] */
        osal_u32 cfg_coex_pta_wl_tx_abort_man : 1; /* [1] */
        osal_u32 cfg_coex_pta_wl_selected_man : 1; /* [2] */
        osal_u32 reserved_0 : 1; /* [3] */
        osal_u32 cfg_coex_pta_switch_rf_man : 1; /* [4] */
        osal_u32 reserved_1 : 3; /* [7..5] */
        osal_u32 cfg_pta_btdm_denied_man : 1; /* [8] */
        osal_u32 cfg_pta_btdm_selected_man : 1; /* [9] */
        osal_u32 reserved_2 : 6; /* [15..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_coex_pta_wl_rx_abort_man;

/* Define the union u_cfg_wl_bt_byp */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wl_rx_status_byp : 1; /* [0] */
        osal_u32 wl_tx_status_byp : 1; /* [1] */
        osal_u32 wp_rx_active_byp : 1; /* [2] */
        osal_u32 reserved_0 : 1; /* [3] */
        osal_u32 btdm_status_byp : 1; /* [4] */
        osal_u32 btdm_priority_vld_byp : 1; /* [5] */
        osal_u32 reserved_1 : 10; /* [15..6] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_wl_bt_byp;

/* Define the union u_cfg_ant_sw_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_ant0_sw_sel : 1; /* [0] */
        osal_u32 cfg_ant1_sw_sel : 1; /* [1] */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_ant_sw_sel;

/* Define the union u_cfg_ant_sw_man */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_ant0_sw_man : 1; /* [0] */
        osal_u32 cfg_ant1_sw_man : 1; /* [1] */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_ant_sw_man;

/* Define the union u_cfg_wifi_bgle_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_coex_ant_sel : 1; /* [0] */
        osal_u32 cfg_coex_ant_wifi_bgle_sel : 1; /* [1] */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_wifi_bgle_sel;

/* Define the union u_cfg_bgle_antctl_polar */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_bgle_ant0_polar : 1; /* [0]  */
        osal_u32 cfg_bgle_ant1_polar : 1; /* [1]  */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_cfg_bgle_antctl_polar;

/* Define the union u_rpt_ant_sw_result */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 ant0_sw : 1;      /* [0]  */
        osal_u32 ant1_sw : 1;      /* [1]  */
        osal_u32 reserved_0 : 14; /* [15..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u16;
} u_rpt_ant_sw_result;

#endif /* __c_union_define_coex_ctl_rb_h__ */
