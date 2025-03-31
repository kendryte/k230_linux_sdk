// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_EFUSE_CTRL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_EFUSE_CTRL_RB_H__
#define __C_UNION_DEFINE_EFUSE_CTRL_RB_H__

/* Define the union u_efuse_byte_write */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_wr_data : 8; /* [7..0] */
        osal_u32 byte_write_sel : 1; /* [8] */
        osal_u32 reserved_0 : 23; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_byte_write;

/* Define the union u_efuse_avdd_ctl_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_avdd_ctl_sel : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_avdd_ctl_sel;

/* Define the union u_efuse_avdd_man */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_avdd_man : 16; /* [15..0] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_avdd_man;

/* Define the union u_efuse_mode_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_mode_en : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;    /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_mode_en;

/* Define the union u_efuse_wr_addr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_wr_addr : 10; /* [9..0]  */
        osal_u32 reserved_0 : 22;    /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_wr_addr;

/* Define the union u_efuse_ctrl_st */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_pgm_st : 1;    /* [0]  */
        osal_u32 efuse_rd_st : 1;     /* [1]  */
        osal_u32 efuse_boot_st : 1;   /* [2]  */
        osal_u32 efuse_ctrl_busy : 1; /* [3]  */
        osal_u32 efuse_pgm_err : 1;   /* [4]  */
        osal_u32 efuse_mode_err : 1; /* [5] */
        osal_u32 efuse_conflict_err : 1; /* [6] */
        osal_u32 efuse_err_clr : 1; /* [7] */
        osal_u32 reserved_0 : 24; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_ctrl_st;

/* Define the union u_efuse_rdata */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_rdata : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_rdata;

/* Define the union u_efuse_time0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cyc_pgm_h : 10; /* [9..0] */
        osal_u32 reserved_0 : 22; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_time0;

/* Define the union u_efuse_time1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cyc_pgm_sp_rd : 4; /* [3..0] */
        osal_u32 cyc_pgm_sp_a_front : 4; /* [7..4] */
        osal_u32 cyc_pgm_sp_a : 4; /* [11..8] */
        osal_u32 cyc_pgm_hp_pgm : 4; /* [15..12] */
        osal_u32 cyc_rd_sr_a_front : 4; /* [19..16] */
        osal_u32 cyc_pgm_hp_rd : 4; /* [23..20] */
        osal_u32 reserved_0 : 8; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_time1;

/* Define the union u_efuse_time2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cyc_rd_sr_a : 4;         /* [3..0]  */
        osal_u32 cyc_rd_h : 8;            /* [11..4]  */
        osal_u32 cyc_rd_hr : 4;           /* [15..12]  */
        osal_u32 reserved_0 : 16;         /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_time2;

/* Define the union u_efuse_time3 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32    cyc_pgm_hp_pg_avdd : 16; /* [15..0] */
        osal_u32 cyc_pgm_sp_pg_avdd : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_time3;

/* Define the union u_efuse_lock_stat1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 efuse_lock_stat1 : 16; /* [15..0] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_efuse_lock_stat1;

/* Define the union u_cmu */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmu_xo_trim_coarse : 4; /* [3..0] */
        osal_u32 cmu_xo_trim_fine : 8; /* [11..4] */
        osal_u32 cmu_trim_reserved : 4; /* [15..12] */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cmu;

/* Define the union u_pmu_trim0 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pmu_rc_32k_lp_res_trim : 6; /* [5..0] */
        osal_u32 pmu_ref_bg_trim1 : 12; /* [17..6] */
        osal_u32 pmu_ref_bg_trim2 : 12; /* [29..18] */
        osal_u32 reserved_0 : 2; /* [31..30] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_trim0;

/* Define the union u_pmu_trim1 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 xldo_trim : 4; /* [3..0] */
        osal_u32 pllldo_1p0_trim : 4; /* [7..4] */
        osal_u32 abbldo_1p0_trim : 4; /* [11..8] */
        osal_u32 vcoldo_1p0_trim : 4; /* [15..12] */
        osal_u32 lnaldo_1p0_trim : 4; /* [19..16] */
        osal_u32 rxldo_1p0_trim : 4; /* [23..20] */
        osal_u32 txldo_1p8_trim : 4; /* [27..24] */
        osal_u32 txldo_1p0_trim : 4; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_trim1;

/* Define the union u_pmu_trim2 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cmuldo0_0p9_trim : 4; /* [3..0] */
        osal_u32 cmuldo1_0p9_trim : 4; /* [7..4] */
        osal_u32 cmuldo_1p8_trim : 4; /* [11..8] */
        osal_u32 intldo_trim : 4; /* [15..12] */
        osal_u32 phyldo_trim : 4; /* [19..16] */
        osal_u32 reserved_0 : 12; /* [31..20] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pmu_trim2;

/* Define the union u_wifi6_sw */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wifi6_sw : 2; /* [1..0] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_wifi6_sw;

/* Define the union u_gle_sw */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 gle_sw : 2; /* [1..0] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_gle_sw;

/* Define the union u_poc_fuse */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 poc_fuse_ms1c : 1; /* [0] */
        osal_u32 poc_fuse_sel : 1; /* [1] */
        osal_u32 reserved_0 : 30; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_poc_fuse;

typedef union {
    struct {
        osal_u32 type_id : 4; /* [3..0] */
        osal_u32 reserved_0 : 28; /* [31..4] */
    } bits;
    osal_u32 u32;
} u_type_id;
typedef union {
    struct {
        osal_u32 dbg_bypass : 1; /* [0] */
        osal_u32 reserved_0 : 31; /* [31..1] */
    } bits;
    osal_u32 u32;
} u_dbg_bypass;
#endif /* __C_UNION_DEFINE_EFUSE_CTRL_RB_H__ */
