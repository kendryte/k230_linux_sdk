/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_VLD_REG_H__
#define __C_UNION_DEFINE_MAC_VLD_REG_H__

typedef union {
    struct {
        osal_u32 xCt2OPzG3OyCoOGmwlv4_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_vap0_bcn_period;

typedef union {
    struct {
        osal_u32 xCt2OPzG3ORyRROqOPzx_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap0_tbtt_timer_update;

typedef union {
    struct {
        osal_u32 x6NgwqGuywFLBSwTwqGo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap0_dtim_count_update;

typedef union {
    struct {
        osal_u32 xCt2OPzG3OxlsRmoOqOPzx_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap0_listen_interval_timer_update;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wj6hwuW_BiF_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_vap1_bcn_period;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wLjLLwTwqGo_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap1_tbtt_timer_update;

typedef union {
    struct {
        osal_u32 xCt2OPzGrO4RlIOqOPzx_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap1_dtim_count_update;

typedef union {
    struct {
        osal_u32 x6NgwqGu0woBCLWhwTwqGo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap1_listen_interval_timer_update;

typedef union {
    struct {
        osal_u32 xCt2OR8OyCO6Oqz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_bc_q_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 xCt2OzCOymO6Oqz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ac_be_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 x6NgwG6wjKwRwTGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ac_bk_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 x6NgwG6wqBwRwTGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ac_vi_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 x6NgwG6wqiwRwTGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ac_vo_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 xCt2OR8OAlGwlO6Oqz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_hi_pri_q_first_frm_ptr_update;

typedef union {
    struct {
        osal_u32 xCt2Ow8OAlGwlO6Oqz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_rx_hibufaddr_update;

typedef union {
    struct {
        osal_u32 x6Ngw_Pwhi_SwRwTGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_rx_bufaddr_update;

typedef union {
    struct {
        osal_u32 xCt2Ow8OsIzxxO6Oqz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_rx_small_buff_addr_update;

typedef union {
    struct {
        osal_u32 x6NgwG6wu_jCwCWWFwqGo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_prbs_seedval;

typedef union {
    struct {
        osal_u32 xCt2OCvm8OGwlvwlRQOGmwlv4_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_coex_priority_period;

typedef union {
    struct {
        osal_u32 xCt2OCvm8OvCCSGlm4OGmwlv4_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_coex_occupied_period;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwjLwVyFjSwCLw6o__ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_coex_bt_20dbm_st_clr;

typedef union {
    struct {
        osal_u32 x6Ngw6QwCLGLwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_ch_statistic_control;

typedef union {
    struct {
        osal_u32 x6NgwNLSwCGSuoWwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_ftm_sample_en;

typedef union {
    struct {
        osal_u32 xCt2O4lz2rOmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2O4lz2KOmo_ : 1;
        osal_u32 xCt2O4lz2nOmo_ : 1;
        osal_u32 reserved_1 : 28;
    } bits;

    osal_u32 u32;
} u_diag_en;

typedef union {
    struct {
        osal_u32 xCt2OzCOPvOISOm4CzORlImwOCxwOmo_ : 1;
        osal_u32 xCt2OzCOPlOISOm4CzORlImwOCxwOmo_ : 1;
        osal_u32 x6NgwG6wjWwSdwWF6GwLBSW_w6o_wWh_ : 1;
        osal_u32 xCt2OzCOydOISOm4CzORlImwOCxwOmo_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_ac_mu_edca_timer_clr_en;

typedef union {
    struct {
        osal_u32 x6NgwGhLBwBhLNwLBSW_wWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_anti_intf_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwSdwWF6GwCTwL_Bg_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_mu_edca_sw_trig;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOyCoOGmwlv4_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_vap2_bcn_period;

typedef union {
    struct {
        osal_u32 xCt2OPzGZORyRROqOPzx_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap2_tbtt_timer_update;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwFLBSwTwqGo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap2_dtim_count_update;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOxlsRmoOqOPzx_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap2_listen_interval_timer_update;

typedef union {
    struct {
        osal_u32 xCt2OCvSoROGmwlv4_ : 6;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_count_period;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOyCoOGmwlv4_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_vap3_bcn_period;

typedef union {
    struct {
        osal_u32 xCt2OPzGiORyRROqOPzx_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap3_tbtt_timer_update;

typedef union {
    struct {
        osal_u32 x6NgwqGukwFLBSwTwqGo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap3_dtim_count_update;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOxlsRmoOqOPzx_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vap3_listen_interval_timer_update;

#endif
