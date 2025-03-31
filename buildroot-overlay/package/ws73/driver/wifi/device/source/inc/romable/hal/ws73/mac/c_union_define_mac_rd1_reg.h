/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_RD1_REG_H__
#define __C_UNION_DEFINE_MAC_RD1_REG_H__

typedef union {
    struct {
        osal_u32 xSihw6Ww6L_owNCS_ : 4;
        osal_u32 xSihwGSCFdwNCS_ : 4;
        osal_u32 xSihwLPwSuFdwNCS_ : 4;
        osal_u32 xIvoOR8OGs4SOtsI_ : 4;
        osal_u32 xSihwLPwjd_CLwNCS_ : 4;
        osal_u32 xSihwLPwQFowNCS_ : 4;
        osal_u32 xSihwFd_wNCS_ : 4;
        osal_u32 xIvoOwmsGOyzOtsI_ : 4;
    } bits;

    osal_u32 u32;
} u_fsm_mon1_st;

typedef union {
    struct {
        osal_u32 xSihwLPwFSGBNwNCS_ : 4;
        osal_u32 xIvoOCvIGO4IzOtsI_ : 4;
        osal_u32 xSihw_PwQFowNCS_ : 4;
        osal_u32 xIvoOw8OCvoRwvxOtsI_ : 2;
        osal_u32 xSihw_PwuowFWC6wNCS_ : 2;
        osal_u32 xSihw_PwFSGBNwNCS_ : 4;
        osal_u32 xSihw_PwFWC6wNCS_ : 4;
        osal_u32 xSihw_PwSuFdwNCS_ : 4;
        osal_u32 xIvoOw8OGs4SOtsI_ : 4;
    } bits;

    osal_u32 u32;
} u_fsm_mon2_st;

typedef union {
    struct {
        osal_u32 xIvoOyCoOtsI_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 xSihwFjG6wNCS_ : 4;
        osal_u32 xIvoOsltsOtsI_ : 4;
        osal_u32 xSihw6iWPwNCS_ : 4;
        osal_u32 xIvoOCAOzCCOtsI_ : 4;
        osal_u32 xSihwWF6GwNCS_ : 4;
        osal_u32 xSihwuLw6L_owNCS_ : 4;
        osal_u32 xIvoOxSROyIOtsI_ : 4;
    } bits;

    osal_u32 u32;
} u_fsm_mon3_st;

typedef union {
    struct {
        osal_u32 xwCKOvGOySttmw_ : 4;
        osal_u32 xwCKOlGOySttmw_ : 4;
        osal_u32 xGWCwiuwjdNNW__ : 4;
        osal_u32 xGWCwBuwjdNNW__ : 4;
        osal_u32 x6WwBiwNBNi_ : 4;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ce_fifo_status;

typedef union {
    struct {
        osal_u32 xIvoOwqCAOtsI_ : 8;
        osal_u32 xIvoOCvIGOwGROtsI_ : 4;
        osal_u32 xSihwLPwRwuowNCS_ : 4;
        osal_u32 xSihwuQtwLPwNCS_ : 4;
        osal_u32 xSihwuQtw_PwNCS_ : 4;
        osal_u32 xSihwLPwRwNCS_ : 4;
        osal_u32 xSihw_PwRwNCS_ : 4;
    } bits;

    osal_u32 u32;
} u_fsm_mon4_st;

typedef union {
    struct {
        osal_u32 xySsOwqCAOtltv_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_bus_fifo_status;

typedef union {
    struct {
        osal_u32 xySsOwqCAOwm6Oxmo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_bus_rwch_req_cnt;

typedef union {
    struct {
        osal_u32 xR8OovwIzxOGwlOIG4SOCoR_ : 16;
        osal_u32 xR8OAlOGwlOIG4SOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_hi_norm_mpdu_cnt;

typedef union {
    struct {
        osal_u32 xR8OIG4SOlozIG4SOCvSoR_ : 16;
        osal_u32 xLPwGSuFdw6idhL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_mpdu_inampdu_count;

typedef union {
    struct {
        osal_u32 xR8OymzCvoOCvSoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_bcn_count;

typedef union {
    struct {
        osal_u32 xovwIO6OwmRwQOCvSoR_ : 16;
        osal_u32 xQBu_BwRw_WL_tw6idhL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_hipri_normal_retry_cnt;

typedef union {
    struct {
        osal_u32 x_PwGSuFdw6idhL_ : 16;
        osal_u32 x_uLw_PwuCFdwFWC6wW__w6hL_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_rpt_rx_ampdu_count;

typedef union {
    struct {
        osal_u32 x_PwFduwSuFdw6hL_ : 16;
        osal_u32 x_PwFWoBSBLwNGBow6idhL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_dup_mpdu_cnt;

typedef union {
    struct {
        osal_u32 xw8OymzCvoOCoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_bcn_cnt;

typedef union {
    struct {
        osal_u32 x_PwuQtwW__wSG6wuGCCWFw6hL_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_phy_err_mac_passed_cnt;

typedef union {
    struct {
        osal_u32 x_PwuQtwCQi_LW_wW__w6hL_ : 16;
        osal_u32 x_PwuQtwoihgW_wW__w6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_phy_shorter_err_cnt;

typedef union {
    struct {
        osal_u32 x_PwNBoLW_WFw6hL_ : 16;
        osal_u32 x4lz2Ow8O4wvGORQGm_ : 6;
        osal_u32 reserved_0 : 10;
    } bits;

    osal_u32 u32;
} u_rpt_rx_filtered_cnt;

typedef union {
    struct {
        osal_u32 x_uLwWPuW6LwGSuFdwoWh_ : 20;
        osal_u32 xR8OIloOsSyOIG4SOxmo_ : 11;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_rpt_expect_len_of_ampdu;

typedef union {
    struct {
        osal_u32 x_uLwjGwhiLwNidhFwGFF_wSCj_ : 16;
        osal_u32 x_uLwjGwhiLwNidhFwLBF_ : 4;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_rpt_ba_not_found_info1;

typedef union {
    struct {
        osal_u32 xwGROzIG4SOwmRwQOCoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_ampdu_retry_cnt;

typedef union {
    struct {
        osal_u32 xCCIGOwmGxzQOtzlxOCoR_ : 16;
        osal_u32 xLKBuw_WuoGtwNGBow6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_tkip_ccmp_rep_fail_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_Pw66SuwSB6wNGBow6hL_ : 16;
        osal_u32 xwGROw8ORdlGOIlCOtzlxOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_tkip_ccmp_mic_fail_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OylGOwmGxzQOtzlxOCoR_ : 16;
        osal_u32 xwGROw8OylGOIlCOtzlxOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_bip_mic_replay_fail_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwKWtwCWG_6QwNGBow6hL_ : 16;
        osal_u32 xwGROw8OlCPOtzlxOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_icv_key_search_fail_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OtCsOtzlxOCoR_ : 16;
        osal_u32 xwGROw8OqzGlOIlCOtzlxOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_fcsfail_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OlIOyzOCoR_ : 16;
        osal_u32 x_uLw_PwFWoGtwjGw6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_ba_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OyssOovoO4lwmCRO4zRzOCoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_non_direct_data_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwijCCwd6wFGLGw6hL_ : 16;
        osal_u32 xwGROw8OvyssOICyCO4zRzOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_obss_data_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwG6Kw6hL_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rx_ack_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_Pw6LCw6hL_ : 16;
        osal_u32 xwGROw8OwRsOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_rtscts_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OISOIlIvOGs4SOqlRAOtzlxOCoR_ : 16;
        osal_u32 xwGROw8OISOIlIvOGs4SOqlRAOsSCCmssOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mu_mimo_psdu_success_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8Ovt4IzOGs4SOqlRAOtzlxOCoR_ : 16;
        osal_u32 xwGROw8Ovt4IzOGs4SOqlRAOsSCCmssOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_ofdma_psdu_success_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwjNwuCFdwTBLQwNGBow6hL_ : 16;
        osal_u32 x_uLw_PwjNwuCFdwTBLQwCd66WCCw6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_bf_psdu_success_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwNCSwLBSWidLw6hL_ : 8;
        osal_u32 xwGROw8OA4xORlImvSROCoR_ : 8;
        osal_u32 x_uLwLPwNCSwLBSWidLw6hL_ : 8;
        osal_u32 xwGROR8OA4xORlImvSROCoR_ : 8;
    } bits;

    osal_u32 u32;
} u_rpt_fsm_timeout_cnt;

typedef union {
    struct {
        osal_u32 xwGROR8OloRwOCoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_tx_intr_cnt;

typedef union {
    struct {
        osal_u32 xwGROw8OovwIOloRwOCoR_ : 16;
        osal_u32 x_uLw_PwQBu_BwBhL_w6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rx_intr_cnt;

typedef union {
    struct {
        osal_u32 xwGROIzCOzSRvOwsROCoR_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_mac_auto_rst_cnt;

typedef union {
    struct {
        osal_u32 x_uLw66Gwu_BwVySw6hL_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_cca_pri_20m_cnt;

typedef union {
    struct {
        osal_u32 xwGROCCzOsmCOZ3IOCoR_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_cca_sec_20m_cnt;

typedef union {
    struct {
        osal_u32 x_uLwWhLW_wLPiuwuCw6hL_ : 16;
        osal_u32 xwGROR8vGOGsOmo_ : 1;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_rpt_enter_txop_ps_param1;

typedef union {
    struct {
        osal_u32 xwGROR8vGOGsORlImOCoR_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_rpt_enter_txop_ps_param2;

typedef union {
    struct {
        osal_u32 xwGROCvm8OzyvwRO4vomOCoR_ : 16;
        osal_u32 xwGROCvm8OzyvwROsRzwROCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_coex_1;

typedef union {
    struct {
        osal_u32 x_uLw6iWPwu_WWSuLw6hL_ : 16;
        osal_u32 xwGROCvm8OzyvwROmo4OCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_coex_2;

typedef union {
    struct {
        osal_u32 x_uLw6iWPwuiCLwu_WWSuLwNGBow6hL_ : 16;
        osal_u32 x_uLw6iWPwuiCLwu_WWSuLw6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_coex_3;

typedef union {
    struct {
        osal_u32 x_uLwjG6KiNNwLBSWidLwhGqwLBSW_w6hL_ : 16;
        osal_u32 x_uLwjG6KiNNwLBSWidLwu_BwVySw66GwNoGg_ : 1;
        osal_u32 xwGROyzCdvttORlImvSROsmCOZ3IOCCzOtxz2_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_rpt_backoff_timeout_info;

typedef union {
    struct {
        osal_u32 x_uLwLPwN_GSWwLBSWw6hL_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_tx_frame_time_cnt;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLwuQtwLPwuCFdw6hL_ : 16;
        osal_u32 x_uLwSihwCLwuQtwLPwuCFdwoWh_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_phy_tx;

typedef union {
    struct {
        osal_u32 xwGROIvoOsROR8OtsIOGs4SOCoR_ : 16;
        osal_u32 xwGROIvoOsROR8OtsIORlImwOCoR_ : 6;
        osal_u32 reserved_0 : 10;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st1_tx_fsm;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLwLPwNCSwSuFdw6hL_ : 16;
        osal_u32 x_uLwSihwCLwLPwNCSwSuFdwoWh_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st2_tx_fsm;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLwuQtw_PwuCFdw6hL_ : 16;
        osal_u32 x_uLwSihwCLwuQtw_PwuCFdwoWh_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_phy_rx;

typedef union {
    struct {
        osal_u32 xwGROIvoOsROw8OtsIOGs4SOCoR_ : 16;
        osal_u32 xwGROIvoOsROw8OtsIORlImwOCoR_ : 6;
        osal_u32 reserved_0 : 10;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st1_rx_fsm;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLw_PwNCSwSuFdw6hL_ : 16;
        osal_u32 x_uLwSihwCLw_PwNCSwSuFdwoWh_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st2_rx_fsm;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLw_PwuWW_wBhFWP_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 x_uLwSihwCLw_PwqGuwBhFWP_ : 3;
        osal_u32 reserved_1 : 1;
        osal_u32 x_uLwSihwCLwLPwuWW_wBhFWP_ : 3;
        osal_u32 reserved_2 : 1;
        osal_u32 x_uLwSihwCLwLPwqGuwBhFWP_ : 2;
        osal_u32 reserved_3 : 2;
        osal_u32 x_uLwSihwCLw_Pwj6wNoGg_ : 1;
        osal_u32 x_uLwSihwCLwLPwj6wNoGg_ : 1;
        osal_u32 xwGROIvoOsROw8OzIG4SOmo_ : 1;
        osal_u32 xwGROIvoOsROR8OzIG4SOmo_ : 1;
        osal_u32 x_uLwSihwCLw6WwKWtwBF_ : 4;
        osal_u32 xwGROIvoOsROCmOClGAmwORQGm_ : 4;
        osal_u32 xwGROIvoOsROCmOmoCwOGwvCOmo_ : 4;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_ce_param;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLwLPwCWRwhdS_ : 12;
        osal_u32 xwGROIvoOsROR8Oyq_ : 4;
        osal_u32 xwGROIvoOsROR8O4zRzOwzRm_ : 8;
        osal_u32 x_uLwSihwCLwLPwQWwNoGg_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xwGROIvoOsROR8OtwzImORQGm_ : 6;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_tx_param;

typedef union {
    struct {
        osal_u32 x_uLwSihwCLw_PwCWRwhdS_ : 12;
        osal_u32 xwGROIvoOsROw8Oyq_ : 4;
        osal_u32 xwGROIvoOsROw8O4zRzOwzRm_ : 8;
        osal_u32 x_uLwSihwCLw_PwQWwNoGg_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xwGROIvoOsROw8OtwzImORQGm_ : 6;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_rx_param;

typedef union {
    struct {
        osal_u32 xwGROR8OwRsOtzlxOCoR_ : 16;
        osal_u32 xwGROR8OwRsOGzssOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_rts_cnt;

typedef union {
    struct {
        osal_u32 x_uLwLPwjG_w6hL_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_tx_bar_rx_mubar_cnt;

typedef union {
    struct {
        osal_u32 xwGROR8OtvwmPmwOtwzImOCoR_ : 16;
        osal_u32 xwGROR8OtvwmPmwOCSwwOsR_ : 4;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_rpt_tx_forever_mon;

typedef union {
    struct {
        osal_u32 xwGROw8O4lwmCROtwzImORlImOCoR_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_rpt_rx_direct_time_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwhihwFB_W6LwN_GSWwijCCwLBSWw6hL_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_rpt_rx_non_direct_obss_time_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwhihwFB_W6LwSgSLwLBSWw6hL_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_rpt_rx_nondirect_mgmt_time_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_PwhihwFB_W6LwFGLGwLBSWw6hL_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_rpt_rx_nondirect_data_time_cnt;

typedef union {
    struct {
        osal_u32 x_uLw_Pw66GwgGu_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rx_cca;

typedef union {
    struct {
        osal_u32 x_uLwGhLBwBhLNwLBSWVw6hL_ : 16;
        osal_u32 xwGROzoRlOloRtORlImrOCoR_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_anti_intf_period_cnt;

typedef union {
    struct {
        osal_u32 x_uLwSihwQwBuwCdjw6hL_ : 8;
        osal_u32 xwGROIvoOGAlOGslOCoR_ : 8;
        osal_u32 x_uLwSihwQVqwhLP_ : 4;
        osal_u32 x_uLwSihwQVqwh_P_ : 4;
        osal_u32 reserved_0 : 1;
        osal_u32 xwGROIvoOCxdOytOmo_ : 1;
        osal_u32 xwGROIvoOAZPOCzxCOlo2Omo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 x_uLwSihwQVqwhgwSiFW_ : 4;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_txbf1;

typedef union {
    struct {
        osal_u32 x_uLwSihw_PwuCFdwCLGLdC_ : 3;
        osal_u32 xwGROIvoOw8OGmmwOlo4m8Omww_ : 1;
        osal_u32 xwGROIvoOw8OR8ytOsvSo4lo2OIv4m_ : 2;
        osal_u32 reserved_0 : 1;
        osal_u32 xwGROIvoOPAROtmm4yzCdORQGm_ : 1;
        osal_u32 x_uLwSihwQWwNWWFjG6KwLtuW_ : 1;
        osal_u32 x_uLwSihw6iSu_wu_i6wWh_ : 1;
        osal_u32 x_uLwSihwLPjNw_uLwCWo_ : 2;
        osal_u32 xwGROIvoOR8ytOzyvwR_ : 1;
        osal_u32 xwGROIvoOR8ytOIzRwl8Ow4Q_ : 1;
        osal_u32 reserved_1 : 18;
    } bits;

    osal_u32 u32;
} u_rpt_mon_st_txbf2;

typedef union {
    struct {
        osal_u32 xwGROIvoOqzlROwmsGORlIm_ : 16;
        osal_u32 xwGROIvoO4yzCOqzlRORlIm_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_one_pkt1;

typedef union {
    struct {
        osal_u32 xwGRO4yzCOR8OCoR_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xwGROvomOGdROR8OCoR_ : 8;
        osal_u32 reserved_1 : 8;
    } bits;

    osal_u32 u32;
} u_rpt_mon_one_pkt2;

typedef union {
    struct {
        osal_u32 x_uLwFjG6wLPwBhL_w6hL_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 x_uLwFjG6wLPwLBSWwBhNi_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_mon_one_pkt3;

typedef union {
    struct {
        osal_u32 x_uLwSihwjdCw_TwSGPwLBSW_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_bus_rw_max_time;

typedef union {
    struct {
        osal_u32 rpt_srg_68_74_cnt : 16;
        osal_u32 rpt_srg_62_68_cnt : 16;
    } bits;

    osal_u32 u32;
} u_srg_62_74_cnt;

typedef union {
    struct {
        osal_u32 rpt_srg_78_82_cnt : 16;
        osal_u32 rpt_srg_74_78_cnt : 16;
    } bits;

    osal_u32 u32;
} u_srg_74_82_cnt;

typedef union {
    struct {
        osal_u32 rpt_non_srg_68_74_cnt : 16;
        osal_u32 rpt_non_srg_62_68_cnt : 16;
    } bits;

    osal_u32 u32;
} u_non_srg_62_74_cnt;

typedef union {
    struct {
        osal_u32 xwGROGswOswOR8OtzlxOoSI_ : 8;
        osal_u32 x_uLwuC_wC_wWhwhdS_ : 8;
        osal_u32 xwGROvyssOG4OytOw8O4vomOoSI_ : 8;
        osal_u32 xwGROvyssOG4OmoOoSI_ : 8;
    } bits;

    osal_u32 u32;
} u_obss_pd_en_num;

typedef union {
    struct {
        osal_u32 rpt_non_srg_78_82_cnt : 16;
        osal_u32 rpt_non_srg_74_78_cnt : 16;
    } bits;

    osal_u32 u32;
} u_non_srg_74_82_cnt;

typedef union {
    struct {
        osal_u32 rpt_obss_pd_tx_success_num : 16;
        osal_u32 rpt_obss_pd_tx_num : 16;
    } bits;

    osal_u32 u32;
} u_obss_pd_tx_num;

typedef union {
    struct {
        osal_u32 rpt_psr_sr_tx_success_num : 16;
        osal_u32 rpt_psr_sr_tx_num : 16;
    } bits;

    osal_u32 u32;
} u_psr_sr_tx_num;

typedef union {
    struct {
        osal_u32 xwGROR8OwmsGOzCdOCoR_ : 16;
        osal_u32 x_uLwLPw_WCuwjGw6hL_ : 16;
    } bits;

    osal_u32 u32;
} u_rssi_max_min;

typedef union {
    struct {
        osal_u32 xwGROw8OPAROsvSo4lo2OISOrsROovIzRCAOCoR_ : 8;
        osal_u32 x_uLw_PwqQLwCidhFBhgwSdw0CLwSGL6Qw6hL_ : 8;
        osal_u32 x_uLw_PwqQLwCidhFBhgwFB_W6Lw6hL_ : 8;
        osal_u32 x_uLw_PwQLwCidhFBhgw6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_sounding_cnt1;

typedef union {
    struct {
        osal_u32 x_uLwqQLwCidhFBhgwSdw0CLwSGL6Qw_WCuw6hL_ : 8;
        osal_u32 x_uLwqQLwCidhFBhgwFB_W6Lw_WCuw6hL_ : 8;
        osal_u32 x_uLw_PwQWwCidhFBhgwLjw6hL_ : 8;
        osal_u32 x_uLw_PwQWwCidhFBhgwhihwLjw6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_sounding_cnt2;

typedef union {
    struct {
        osal_u32 x_uLwQWwCidhFBhgwhihwLjw_WCuw6hL_ : 8;
        osal_u32 xwGROPAROsvSo4lo2OISOrsROovIzRCAOwmsGOCoR_ : 8;
        osal_u32 x_uLwCidhFBhgw_PwqGuwBhFWP_ : 3;
        osal_u32 reserved_0 : 2;
        osal_u32 x_uLwCidhFBhgwu_iLi6io_ : 2;
        osal_u32 reserved_1 : 9;
    } bits;

    osal_u32 u32;
} u_rx_sounding_cnt3;

typedef union {
    struct {
        osal_u32 xwGROsvSo4lo2OqzlROAORlImw_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x_uLwCidhFBhgwTGBLwqwLBSW__ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_rpt_sounding_wait_timer;

typedef union {
    struct {
        osal_u32 xwGROsvSo4lo2OIzCO4mxRzOsowOCoR_ : 8;
        osal_u32 xwGROsvSo4lo2OGAQO4mxRzOsowOCoR_ : 8;
        osal_u32 xwGROsvSo4lo2OIzCOGAlOGslOCoR_ : 8;
        osal_u32 xwGROsvSo4lo2OGAQOGAlOGslOCoR_ : 8;
    } bits;

    osal_u32 u32;
} u_sounding_phi_psi_delta_snr;

typedef union {
    struct {
        osal_u32 x_uLw6CBwCTwjoKwW__w6hL_ : 8;
        osal_u32 x_uLw6CBwLBSWidLwW__w6hL_ : 8;
        osal_u32 xwGROCslOAORlIlo2OmwwOCoR_ : 8;
        osal_u32 x_uLw6CBwQwhdSwW__w6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rpt_csi_err_cnt;

typedef union {
    struct {
        osal_u32 rpt_csi_succ_cnt : 16;
        osal_u32 x_uLw6CBwjoKwhdSw6hL_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 xwGROCslOtlxROCoR_ : 8;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_rpt_csi_cnt;

typedef union {
    struct {
        osal_u32 xRyOIvolRvwOw8ORyOtzlxOCoR_ : 8;
        osal_u32 xLjwSihBLi_w_PwLjwCd66w6hL_ : 8;
        osal_u32 xLjwSihBLi_wuG_GSwNWWFjG6Kw6hL_ : 8;
        osal_u32 xLjwSihBLi_w_PwL_BgwGBFwSGL6Qw6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rpt_tb_monitor1;

typedef union {
    struct {
        osal_u32 xRyOIvolRvwOGzwzIOmwwlotv_ : 4;
        osal_u32 xwGROAmORyOSGAORIG_ : 8;
        osal_u32 x_uLwQWwLjwLPwuiTW__ : 8;
        osal_u32 x_uLwLjwLPwLu6wqGo_ : 8;
        osal_u32 reserved_0 : 4;
    } bits;

    osal_u32 u32;
} u_rpt_tb_monitor3;

typedef union {
    struct {
        osal_u32 x_uLw6o_wSdwWF6GwLBSW_w6hL_ : 8;
        osal_u32 xwGROmoRmwOISOm4CzOCoR_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_ac_mu_edca_param;

typedef union {
    struct {
        osal_u32 xPzGZORyRROCSwwOCoR_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap2_tbtt_timer_status;

typedef union {
    struct {
        osal_u32 x_uLwqGuVw_Pwj6hwuW_BiF_ : 16;
        osal_u32 xPzGZOxlsRmoOCoR_ : 8;
        osal_u32 xqGuVwFLBSw6d__w6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rpt_vap2_rx_bcn_param;

typedef union {
    struct {
        osal_u32 xPzGiORyRROCSwwOCoR_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_vap3_tbtt_timer_status;

typedef union {
    struct {
        osal_u32 x_uLwqGukw_Pwj6hwuW_BiF_ : 16;
        osal_u32 xPzGiOxlsRmoOCoR_ : 8;
        osal_u32 xqGukwFLBSw6d__w6hL_ : 8;
    } bits;

    osal_u32 u32;
} u_rpt_vap3_rx_bcn_param;

typedef union {
    struct {
        osal_u32 xwGROCslOAOoSI_ : 8;
        osal_u32 xwGROCslOAOoSIORAmwvOPzx_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_csi_h_num;

#endif
