// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_PMU_CMU_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_PMU_CMU_CTL_RB_H__
#define __C_UNION_DEFINE_PMU_CMU_CTL_RB_H__

/* Define the union u_sysldo_gp_reg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sysldo_gp_reg : 8; /* [7..0] */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sysldo_gp_reg;

/* Define the union u_pmu_rsv */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_resev1 : 16; /* [15..0] */
        osal_u32 pmu_resev2 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_rsv;

/* Define the union u_pmu_efuse_bg_trim */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_pmu_ref2_bg_trim2_man : 12; /* [11..0] */
        osal_u32 rg_pmu_ref2_bg_trim2_sel : 1; /* [12] */
        osal_u32 rg_pmu_ref1_bg_trim1_man : 12; /* [24..13] */
        osal_u32 rg_pmu_ref1_bg_trim1_sel : 1; /* [25] */
        osal_u32 reserved_0 : 6; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_efuse_bg_trim;

/* Define the union u_pmu_efuse_rc */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_pmu_rc_32k_lp_res_trim_man : 6; /* [5..0] */
        // Comment of field: rg_pmu_rc_32k_lp_res_trim_sel
        osal_u32 rg_pmu_rc_32k_lp_res_trim_sel : 1; /* [6] */
        osal_u32 rg_pmu_rc_32k_res_trim_man : 6; /* [12..7] */
        osal_u32 pmu_rc_32k_reserve : 6; /* [18..13] */
        osal_u32 pmu_rc_32k_en : 1; /* [19] */
        osal_u32 pmu_rc_32k_lp_reserve : 6; /* [25..20] */
        osal_u32 pmu_rc_32k_lp_en : 1; /* [26] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_efuse_rc;

/* Define the union u_pmu_efuse_new_ldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_pmu_intldo_trim_man : 4; /* [3..0] */
        osal_u32 rg_pmu_intldo_trim_sel : 1; /* [4] */
        osal_u32 rg_pmu_phyldo_trim_man : 4; /* [8..5] */
        osal_u32 rg_pmu_phyldo_trim_sel : 1; /* [9] */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_efuse_new_ldo;

/* Define the union u_pmu_cldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_sysldo_sel : 2; /* [1..0] */
        osal_u32 pmu_sysldo_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_sysldo_ocp_set : 1; /* [3] */
        osal_u32 pmu_sysldo_vset : 4; /* [7..4] */
        osal_u32 pmu_sysldo_ictr : 3; /* [10..8] */
        osal_u32 pmu_sysldo_sw : 1; /* [11] */
        osal_u32 pmu_sysldo_hiz : 1; /* [12] */
        osal_u32 pmu_cldo_sel : 2; /* [14..13] */
        osal_u32 pmu_cldo_ocp_bypass : 1;  /* [15] */
        osal_u32 pmu_cldo_ocp_set : 1; /* [16] */
        osal_u32 pmu_cldo_vset : 4; /* [20..17] */
        osal_u32 pmu_cldo_ictr : 3; /* [23..21] */
        osal_u32 pmu_cldo_sw : 1; /* [24] */
        osal_u32 pmu_cldo_hiz : 1; /* [25] */
        osal_u32 dbg_pmu_cldo_en_delay : 1; /* [26] */
        osal_u32 dbg_pmu_cldo_en : 1; /* [27] */
        osal_u32 dbg_pmu_cldo_en_sel : 1; /* [28] */
        osal_u32 reserved_0 : 3; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cldo;

/* Define the union u_pmu_ioldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_ioldo_sel : 4; /* [3..0] */
        osal_u32 pmu_ioldo_vset : 4; /* [7..4] */
        osal_u32 pmu_ioldo_ictr : 3; /* [10..8] */
        osal_u32 reserved_0 : 21; /* [31..11] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_ioldo;

/* Define the union u_pmu_intldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_intldo_sel : 2; /* [1..0] */
        osal_u32 pmu_intldo_vset : 4; /* [5..2] */
        osal_u32 pmu_intldo_ictr : 3; /* [8..6] */
        osal_u32 pmu_intldo_sw : 1; /* [9] */
        osal_u32 pmu_intldo_hiz : 1; /* [10] */
        osal_u32 dbg_pmu_intldo_en_delay : 1; /* [11] */
        osal_u32 dbg_pmu_intldo_en : 1; /* [12] */
        osal_u32 dbg_pmu_intldo_en_sel : 1; /* [13] */
        osal_u32 reserved_0 : 18; /* [31..14] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_intldo;

/* Define the union u_pmu_xldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_xldo_sel : 2; /* [1..0] */
        osal_u32 pmu_xldo_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_xldo_ocp_set : 1; /* [3] */
        osal_u32 pmu_xldo_vset : 4; /* [7..4] */
        osal_u32 pmu_xldo_ictr : 3; /* [10..8] */
        osal_u32 pmu_xldo_sw : 1; /* [11] */
        osal_u32 pmu_xldo_hiz : 1; /* [12] */
        osal_u32 dbg_pmu_xldo_en_delay : 1; /* [13] */
        osal_u32 dbg_pmu_xldo_en : 1; /* [14] */
        osal_u32 dbg_pmu_xldo_en_sel : 1; /* [15] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_xldo;

/* Define the union u_pmu_cmuldo_1p8 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cmuldo_1p8_sel : 2; /* [1..0] */
        osal_u32 pmu_cmuldo_1p8_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_cmuldo_1p8_ocp_set : 1; /* [3] */
        osal_u32 pmu_cmuldo_1p8_vset : 4; /* [7..4] */
        osal_u32 pmu_cmuldo_1p8_ictr : 3; /* [10..8] */
        osal_u32 pmu_cmuldo_1p8_sw : 1; /* [11] */
        osal_u32 pmu_cmuldo_1p8_hiz : 1; /* [12] */
        osal_u32 dbg_pmu_cmuldo_1p8_en_delay : 1; /* [13] */
        osal_u32 dbg_pmu_cmuldo_1p8_en : 1; /* [14] */
        osal_u32 dbg_pmu_cmuldo_1p8_en_sel : 1; /* [15] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cmuldo_1p8;

/* Define the union u_pmu_cmuldo0_0p9 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cmuldo0_0p9_sel : 2; /* [1..0] */
        osal_u32 pmu_cmuldo0_0p9_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_cmuldo0_0p9_ocp_set : 1; /* [3] */
        osal_u32 pmu_cmuldo0_0p9_vset : 4; /* [7..4] */
        osal_u32 pmu_cmuldo0_0p9_ictr : 3; /* [10..8] */
        osal_u32 pmu_cmuldo0_0p9_sw : 1; /* [11] */
        osal_u32 pmu_cmuldo0_0p9_hiz : 1; /* [12] */
        osal_u32 dbg_pmu_cmuldo0_0p9_en_delay : 1; /* [13] */
        osal_u32 dbg_pmu_cmuldo0_0p9_en : 1; /* [14] */
        osal_u32 dbg_pmu_cmuldo0_0p9_en_sel : 1; /* [15] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cmuldo0_0p9;

/* Define the union u_pmu_cmuldo1_0p9 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cmuldo1_0p9_sel : 2; /* [1..0] */
        osal_u32 pmu_cmuldo1_0p9_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_cmuldo1_0p9_ocp_set : 1; /* [3] */
        osal_u32 pmu_cmuldo1_0p9_vset : 4; /* [7..4] */
        osal_u32 pmu_cmuldo1_0p9_ictr : 3; /* [10..8] */
        osal_u32 pmu_cmuldo1_0p9_sw : 1; /* [11] */
        osal_u32 pmu_cmuldo1_0p9_hiz : 1; /* [12] */
        osal_u32 dbg_pmu_cmuldo1_0p9_en_delay : 1; /* [13] */
        osal_u32 dbg_pmu_cmuldo1_0p9_en : 1; /* [14] */
        osal_u32 dbg_pmu_cmuldo1_0p9_en_sel : 1; /* [15] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_cmuldo1_0p9;

/* Define the union u_pmu_txldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_txldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_txldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_txldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_txldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_txldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_txldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_txldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_txldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_txldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_txldo_1p0;

/* Define the union u_pmu_rxldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_rxldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_rxldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_rxldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_rxldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_rxldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_rxldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_rxldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_rxldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_rxldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_rxldo_1p0;

/* Define the union u_pmu_lnaldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_lnaldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_lnaldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_lnaldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_lnaldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_lnaldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_lnaldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_lnaldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_lnaldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_lnaldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_lnaldo_1p0;

/* Define the union u_pmu_vcoldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_vcoldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_vcoldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_vcoldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_vcoldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_vcoldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_vcoldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_vcoldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_vcoldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_vcoldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_vcoldo_1p0;

/* Define the union u_pmu_abbldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_abbldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_abbldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_abbldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_abbldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_abbldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_abbldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_abbldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_abbldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_abbldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_abbldo_1p0;

/* Define the union u_pmu_pllldo_1p0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_pllldo_1p0_sel : 2; /* [1..0] */
        osal_u32 pmu_pllldo_1p0_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_pllldo_1p0_ocp_set : 1; /* [3] */
        osal_u32 pmu_pllldo_1p0_vset : 4; /* [7..4] */
        osal_u32 pmu_pllldo_1p0_ictr : 3; /* [10..8] */
        osal_u32 pmu_pllldo_1p0_sw : 1; /* [11] */
        osal_u32 pmu_pllldo_1p0_hiz : 1; /* [12] */
        osal_u32 pmu_pllldo_1p0_en_delay : 1; /* [13] */
        osal_u32 pmu_pllldo_1p0_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_pllldo_1p0;

/* Define the union u_dbg_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 dbg_pmu_ref1_vbg_en : 1; /* [0] */
        osal_u32 dbg_pmu_ref1_buff_en : 1; /* [1] */
        osal_u32 dbg_pmu_ref1_ibg_en : 1; /* [2] */
        osal_u32 dbg_pmu_ref2_buff_en : 1; /* [3] */
        osal_u32 dbg_pmu_ref2_bg_en : 1; /* [4] */
        osal_u32 dbg_pmu_ref2_ibg_en : 1; /* [5] */
        osal_u32 dbg_pmu_ref2_itune_en : 1; /* [6] */
        osal_u32 dbg_pmu_ref2_ipoly_en : 1; /* [7] */
        osal_u32 dbg_pmu_ref2_uvlo_en : 1; /* [8] */
        osal_u32 dbg_pmu_ref2_ovp_en : 1; /* [9] */
        osal_u32 dbg_cldo_to_sysldo_iso_en : 1; /* [10] */
        osal_u32 dbg_cmu2dbb_iso_en : 1; /* [11] */
        osal_u32 dbg_pmu_ref1_vbg_en_sel : 1; /* [12] */
        osal_u32 dbg_pmu_ref1_buff_en_sel : 1; /* [13] */
        osal_u32 dbg_pmu_ref1_ibg_en_sel : 1; /* [14] */
        osal_u32 dbg_pmu_ref2_buff_en_sel : 1; /* [15] */
        osal_u32 dbg_pmu_ref2_bg_en_sel : 1; /* [16] */
        osal_u32 dbg_pmu_ref2_ibg_en_sel : 1; /* [17] */
        osal_u32 dbg_pmu_ref2_itune_en_sel : 1; /* [18] */
        osal_u32 dbg_pmu_ref2_ipoly_en_sel : 1; /* [19] */
        osal_u32 dbg_pmu_ref2_uvlo_en_sel : 1; /* [20] */
        osal_u32 dbg_pmu_ref2_ovp_en_sel : 1; /* [21] */
        osal_u32 dbg_cldo_to_sysldo_iso_en_sel : 1; /* [22] */
        osal_u32 dbg_cmu2dbb_iso_en_sel : 1; /* [23] */
        osal_u32 reserved_0 : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_dbg_ctl;

/* Define the union u_pmu_test_volt_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_volt_test_sel : 5; /* [4..0] */
        osal_u32 pmu_test_volt_en : 1; /* [5] */
        osal_u32 reserved_0 : 26; /* [31..6] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_test_volt_en;

/* Define the union u_pmu_sig */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 16; /* [15..0] */
        osal_u32 pmu_uvlo_th : 2; /* [17..16] */
        osal_u32 pmu_ibias_trim_en : 1; /* [18] */
        osal_u32 pmu_ref2_bg_vset : 4; /* [22..19] */
        osal_u32 pmu_sw_tsensor_en : 1; /* [23] */
        osal_u32 reserved_1 : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_sig;

/* Define the union u_pmu_dft_sw_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_dft_sw_en : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_dft_sw_en;

/* Define the union u_pmu_other_trim1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_pmu_rxldo_1p0_trim_man : 4; /* [3..0] */
        osal_u32 rg_pmu_lnaldo_1p0_trim_man : 4; /* [7..4] */
        osal_u32 rg_pmu_vcoldo_1p0_trim_man : 4; /* [11..8] */
        osal_u32 rg_pmu_abbldo_1p0_trim_man : 4; /* [15..12] */
        osal_u32 rg_pmu_pllldo_1p0_trim_man : 4; /* [19..16] */
        osal_u32 rg_pmu_xldo_trim_man : 4; /* [23..20] */
        osal_u32 rg_pmu_rxldo_1p0_trim_sel : 1; /* [24] */
        osal_u32 rg_pmu_lnaldo_1p0_trim_sel : 1; /* [25] */
        osal_u32 rg_pmu_vcoldo_1p0_trim_sel : 1; /* [26] */
        osal_u32 rg_pmu_abbldo_1p0_trim_sel : 1; /* [27] */
        osal_u32 rg_pmu_pllldo_1p0_trim_sel : 1; /* [28] */
        osal_u32 rg_pmu_xldo_trim_sel : 1; /* [29] */
        osal_u32 reserved_0 : 2; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_other_trim1;

/* Define the union u_pmu_other_trim2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_pmu_cmuldo_1p8_trim_man : 4; /* [3..0] */
        osal_u32 rg_pmu_cmuldo1_0p9_trim_man : 4; /* [7..4] */
        osal_u32 rg_pmu_cmuldo0_0p9_trim_man : 4; /* [11..8] */
        osal_u32 rg_pmu_txldo_1p0_trim_man : 4; /* [15..12] */
        osal_u32 rg_pmu_txldo_1p8_trim_man : 4; /* [19..16] */
        osal_u32 rg_pmu_cmuldo_1p8_trim_sel : 1; /* [20] */
        osal_u32 rg_pmu_cmuldo1_0p9_trim_sel : 1; /* [21] */
        osal_u32 rg_pmu_cmuldo0_0p9_trim_sel : 1; /* [22] */
        osal_u32 rg_pmu_txldo_1p0_trim_sel : 1; /* [23] */
        osal_u32 rg_pmu_txldo_1p8_trim_sel : 1; /* [24] */
        osal_u32 reserved_0 : 7; /* [31..25] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_other_trim2;

/* Define the union u_pmu_txldo_1p8 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_txldo_1p8_sel : 2; /* [1..0] */
        osal_u32 pmu_txldo_1p8_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_txldo_1p8_ocp_set : 1; /* [3] */
        osal_u32 pmu_txldo_1p8_vset : 4; /* [7..4] */
        osal_u32 pmu_txldo_1p8_ictr : 3; /* [10..8] */
        osal_u32 pmu_txldo_1p8_sw : 1; /* [11] */
        osal_u32 pmu_txldo_1p8_hiz : 1; /* [12] */
        osal_u32 pmu_txldo_1p8_en_delay : 1; /* [13] */
        osal_u32 pmu_txldo_1p8_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_txldo_1p8;

/* Define the union u_pmu_phy_ldo */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_phyldo_sel : 2; /* [1..0] */
        osal_u32 pmu_phyldo_ocp_bypass : 1; /* [2] */
        osal_u32 pmu_phyldo_ocp_set : 1; /* [3] */
        osal_u32 pmu_phyldo_vset : 4; /* [7..4] */
        osal_u32 pmu_phyldo_ictr : 3; /* [10..8] */
        osal_u32 pmu_phyldo_sw : 1; /* [11] */
        osal_u32 pmu_phyldo_hiz : 1; /* [12] */
        osal_u32 pmu_phyldo_en_delay : 1; /* [13] */
        osal_u32 pmu_phyldo_en : 1; /* [14] */
        osal_u32 reserved_0 : 17; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_phy_ldo;

/* Define the union u_excep_cfg_rg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 unlock_grm_int_en : 1; /* [0] */
        osal_u32 unlock_raw_int_en : 1; /* [1] */
        osal_u32 pmu_uvlo_grm_int_en : 1; /* [2] */
        osal_u32 pmu_uvlo_raw_int_en : 1; /* [3] */
        osal_u32 pmu_ocp_grm_int_en : 1; /* [4] */
        osal_u32 pmu_ocp_raw_int_en : 1; /* [5] */
        osal_u32 pmu_ovp_grm_int_en : 1; /* [6] */
        osal_u32 pmu_ovp_raw_int_en : 1; /* [7] */
        osal_u32 cmu_unlock_grm_time : 6; /* [13..8] */
        osal_u32 pmu_1ms_grm_time_cfg : 6; /* [19..14] */
        osal_u32 reserved_0 : 12; /* [31..20] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_excep_cfg_rg;

/* Define the union u_excep_clr_rg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 unlock_grm_1stick_clr : 1; /* [0] */
        osal_u32 unlock_raw_1stick_clr : 1; /* [1] */
        osal_u32 pmu_uvlo_grm_1stick_clr : 1; /* [2] */
        osal_u32 pmu_uvlo_raw_1stick_clr : 1; /* [3] */
        osal_u32 pmu_ocp_grm_1stick_clr : 1; /* [4] */
        osal_u32 pmu_ocp_raw_1stick_clr : 1; /* [5] */
        osal_u32 pmu_ovp_grm_1stick_clr : 1; /* [6] */
        osal_u32 pmu_ovp_raw_1stick_clr : 1; /* [7] */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_excep_clr_rg;

/* Define the union u_excep_ocp_ro_rg_stick */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cldo_ocp_raw_1stick : 1; /* [0] */
        osal_u32 pmu_cldo_ocp_grm_1stick : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_excep_ocp_ro_rg_stick;

/* Define the union u_excep_ocp_ro_rg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cldo_ocp : 1; /* [0] */
        osal_u32 pmu_cldo_ocp_grm : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_excep_ocp_ro_rg;

/* Define the union u_excep_ro_rg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 unlock_grm_1stick : 1; /* [0] */
        osal_u32 unlock_grm : 1; /* [1] */
        osal_u32 unlock_raw_1stick : 1; /* [2] */
        osal_u32 unlock : 1; /* [3] */
        osal_u32 pmu_ovp_grm_1stick : 1; /* [4] */
        osal_u32 pmu_ovp_grm : 1; /* [5] */
        osal_u32 pmu_ovp_raw_1stick : 1; /* [6] */
        osal_u32 pmu_ovp : 1; /* [7] */
        osal_u32 pmu_uvlo_grm_1stick : 1; /* [8] */
        osal_u32 pmu_uvlo_grm : 1; /* [9] */
        osal_u32 pmu_uvlo_raw_1stick : 1; /* [10] */
        osal_u32 pmu_uvlo : 1; /* [11] */
        osal_u32 cmu_fnpll_lock_grm : 1; /* [12] */
        osal_u32 cmu_fnpll_lock : 1; /* [13] */
        osal_u32 reserved_0 : 18; /* [31..14] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_excep_ro_rg;

/* Define the union u_cur_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cur_sts_sync : 2; /* [1..0] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cur_sts;

/* Define the union u_ovp_pd_count */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 ovp_pd_time : 8; /* [7..0] */
        osal_u32 ovp_pd_count_en : 1; /* [8] */
        osal_u32 reserved_0 : 23; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ovp_pd_count;

/* Define the union u_ocp_pd_count */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 ocp_pd_time : 8; /* [7..0] */
        osal_u32 ocp_pd_count_en : 1; /* [8] */
        osal_u32 reserved_0 : 23; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_ocp_pd_count;

/* Define the union u_iso_sig */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 d_wb_linectrl_dac_iso : 1; /* [0] */
        osal_u32 d_wb_linectrl_adc_iso : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_iso_sig;

/* Define the union u_rf_control */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 d_wb_test0_gpio_en : 1; /* [0] */
        osal_u32 d_wb_test1_gpio_en : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rf_control;

/* Define the union u_poc_coex */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rg_poc_value : 1; /* [0] */
        osal_u32 rg_poc_sel : 1; /* [1] */
        osal_u32 dbg_pwron_cmu_drive_en : 1; /* [2] */
        osal_u32 dbg_pwron_cmu_drive_en_sel : 1; /* [3] */
        osal_u32 pmu_vdd3318_flag : 1; /* [4] */
        osal_u32 dbg_pmu_ack : 1; /* [5] */
        osal_u32 dbg_pmu_ack_sel : 1; /* [6] */
        osal_u32 dbg_efuse_boot_en : 1; /* [7] */
        osal_u32 dbg_efuse_boot_en_sel : 1; /* [8] */
        osal_u32 reserved_0 : 23; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_poc_coex;

/* Define the union u_pmu_time1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cldo_en_pu_time : 9; /* [8..0] */
        osal_u32 pmu_ref1_ibg_en_pu_time : 9; /* [17..9] */
        osal_u32 pmu_ref1_vbg_en_pu_time : 9; /* [26..18] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time1;

/* Define the union u_pmu_time2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_intldo_en_delay_pu_time : 9; /* [8..0] */
        osal_u32 pmu_intldo_en_pu_time : 9; /* [17..9] */
        osal_u32 pmu_cldo_en_delay_pu_time : 9; /* [26..18] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time2;

/* Define the union u_pmu_time3 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_ref2_itune_en_pu_time : 9; /* [8..0] */
        osal_u32 pmu_ref2_uvlo_en_pu_time : 9; /* [17..9] */
        osal_u32 pmu_ref2_ibg_en_pu_time : 9; /* [26..18] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time3;

/* Define the union u_pmu_time4 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_cmuldo1_0p9_en_delay_pu_time : 9; /* [8..0] */
        osal_u32 pmu_cmuldo0_0p9_en_delay_pu_time : 9; /* [17..9] */
        osal_u32 pmu_cmuldo0_0p9_en_pu_time : 9; /* [26..18] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time4;

/* Define the union u_pmu_time5 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu2dbb_iso_en_pd_time : 9; /* [8..0] */
        osal_u32 pmu_xldo_en_delay_pu_time : 9; /* [17..9] */
        osal_u32 reserved_0 : 14; /* [31..18] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time5;

/* Define the union u_pmu_time6 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cldo_rst_pu_time : 9; /* [8..0] */
        osal_u32 xo_clk_gt_pu_time : 9; /* [17..9] */
        osal_u32 cmu_xo_pd_pd_time : 9; /* [26..18] */
        osal_u32 reserved_0 : 5; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_time6;

#endif /* __C_UNION_DEFINE_PMU_CMU_CTL_RB_H__ */