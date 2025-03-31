// ******************************************************************************
// Copyright : Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK1_C_UNION_DEFINE_H__
#define __PHY_REG_BANK1_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 xCt2Otwm6Oyzo4_ : 1;
        osal_u32 xCt2Orrz8Omo_ : 1;
        osal_u32 xCt2ORwl2OGSoCOyQGzss_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_freq_band_ax_en;

typedef union {
    struct {
        osal_u32 x6NgwuQtwjTwSiFW_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_phy_bw_mode;

typedef union {
    struct {
        osal_u32 x6Ngwu_BVySw6QhwhdS_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pri20m_chn_num;

typedef union {
    struct {
        osal_u32 x6NgwCW6VySwiNNCWL_ : 5;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_sec20m_offset;

typedef union {
    struct {
        osal_u32 x6NgwGF6wNC_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_adc_fs;

typedef union {
    struct {
        osal_u32 x6NgwFG6wNC_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_dac_fs;

typedef union {
    struct {
        osal_u32 xCt2OvomOzoROsmx_ : 2;
        osal_u32 x6NgwihWwGhLwCWowSGh_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_one_ant_sel;

typedef union {
    struct {
        osal_u32 xCt2Ow8zoROsmxOGdR4mR_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_rx_antsel_ctrl;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2Ow8OGs4OC2OyGs_ : 1;
        osal_u32 xCt2OtvCOC2OyGs_ : 1;
        osal_u32 xCt2Ow8OCAoO4mCOPRyOC2OyGs_ : 1;
        osal_u32 x6Ngw_PywC6iw6gwjuC_ : 1;
        osal_u32 x6Ngw_PwBRw6iSuw6gwjuC_ : 1;
        osal_u32 x6Ngw_PywBB_w6gwjuC_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 x6NgwLPywC6iw6gwjuC_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xCt2OR83Ol6OIlsIzRCAOC2OyGs_ : 1;
        osal_u32 reserved_3 : 8;
        osal_u32 xCt2OrrysSyyxvCdOC2OyGs_ : 1;
        osal_u32 xCt2OyqOCzxlOC2OyGs_ : 1;
        osal_u32 x6Ngw_PwG66dSw6gwjuC_ : 1;
        osal_u32 xCt2Ow8O4tROC2OyGs_ : 1;
        osal_u32 x6Ngw6QhwCSiiLQw6gwjuC_ : 1;
        osal_u32 x6Ngw6QhwFF6w6gwjuC_ : 1;
        osal_u32 xCt2OCAoOlIGPOC2OyGs_ : 1;
        osal_u32 xCt2OCAoOAxGOxsOC2OyGs_ : 1;
        osal_u32 xCt2OCAoOmsROC2OyGs_ : 1;
        osal_u32 xCt2Ox4GCOlRmwROC2OyGs_ : 1;
        osal_u32 x6NgwhihQLwFduw6gwjuC_ : 1;
        osal_u32 x6NgwNB_w0Pw6gwjuC_ : 1;
        osal_u32 x6NgwLPwCQBNLw6gwjuC_ : 1;
    } bits;

    osal_u32 u32;
} u_cfg_23stage_cg_bps;

typedef union {
    struct {
        osal_u32 xCt2O4QoOttROslpmOmo_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwGdLiw6gwFWjdg_ : 1;
        osal_u32 xCt2OzSRvOC2O4lszyxm_ : 1;
        osal_u32 x6NgwGdLiw_CLwFBCGjoW_ : 1;
        osal_u32 xCt2OsIGsOvGRlIlpmOyGs_ : 1;
        osal_u32 xCt2O4vRyOPAROyGs_ : 1;
        osal_u32 x6Ngw00jwjoi6KBhg_ : 1;
        osal_u32 x6NgwSGCKwGg6wdhoi6KwjtuGCC_ : 1;
        osal_u32 reserved_1 : 6;
        osal_u32 x6NgwSWSwGdLiwoCwjuC_ : 1;
        osal_u32 reserved_2 : 3;
        osal_u32 xCt2Oxsl2OmwwOwGROCRwx_ : 1;
        osal_u32 xCt2OAROwsPOCAdOmo_ : 1;
        osal_u32 x6NgwqQLw_Cqw6QKwWh_ : 1;
        osal_u32 x6NgwqQLwhFuwW__w6QW6KwWh_ : 1;
        osal_u32 xCt2OPAROCCzOxmoOSG4Omo_ : 1;
        osal_u32 xCt2OPARO4mROmo_ : 1;
        osal_u32 x6Ngw00G6wWPuw_doW0wjuC_ : 1;
        osal_u32 xCt2OrrzCOm8GOwSxmZOyGs_ : 1;
        osal_u32 xCt2Oxm2zOytOmo_ : 1;
        osal_u32 x6Ngw00G6woWhwWPuwWh_ : 1;
        osal_u32 x6Ngw66GwiNFSwuKLwFWLwCWo_ : 1;
        osal_u32 x6Ngw66GwiNFSwuKLwFWLwhG_wjuC_ : 1;
    } bits;

    osal_u32 u32;
} u_phy_ctrl0;

typedef union {
    struct {
        osal_u32 xCt2OsmxtOGwvROmo_ : 1;
        osal_u32 x6NgwCWoNwu_iLw_WCWLwWh_ : 1;
        osal_u32 x6Ngw66GwiqLw_WCWLwWh_ : 1;
        osal_u32 xCt2OIz8OGG4SOxmo_ : 6;
        osal_u32 xCt2OrryO4lszyxm_ : 1;
        osal_u32 x6NgwiNFSwFBCGjoW_ : 1;
        osal_u32 xCt2Ow8OZ8OC2OyGs_ : 1;
        osal_u32 x6NgwLPw_PwVPw6gwjuC_ : 1;
        osal_u32 x6Ngw_GFG_wVPw6gwjuC_ : 1;
        osal_u32 x6NgwFiLjwUyw6gwjuC_ : 1;
        osal_u32 x6NgwFNWwDPw6gwjuC_ : 1;
        osal_u32 x6NgwjNw6gwjuC_ : 1;
        osal_u32 xCt2OIlIvO4mROC2OyGs_ : 1;
        osal_u32 x6NgwFW6iFW_w6gwjuC_ : 1;
        osal_u32 x6Ngw_Pw6L_ow6gwjuC_ : 1;
        osal_u32 xCt2Ox4GCOK8OC2OyGs_ : 1;
        osal_u32 x6NgwoFu6wVPw6gwjuC_ : 1;
        osal_u32 x6NgwLPwu_WGSjoWw6gwjuC_ : 1;
        osal_u32 x6NgwqBLW_jBw6gwjuC_ : 1;
        osal_u32 x6NgwCth6wUyw6gwjuC_ : 1;
        osal_u32 xCt2O4G4OC2OyGs_ : 1;
        osal_u32 x6Ngw_GFG_wUyw6gwjuC_ : 1;
        osal_u32 x6NgwNLSw6gwjuC_ : 1;
        osal_u32 x6NgwLPwFNWwVFiSw6gwjuC_ : 1;
        osal_u32 x6NgwuVSw6gwjuC_ : 1;
        osal_u32 x6Ngw6N_w6gwjuC_ : 1;
        osal_u32 x6Ngw00jwNi_6WwWhGjoW_ : 1;
    } bits;

    osal_u32 u32;
} u_phy_ctrl1;

typedef union {
    struct {
        osal_u32 x6NgwqQLwQF_w6L_o_ : 2;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwCBgjwW__wWh_ : 1;
        osal_u32 x6NgwFW6wFtSwN_WRwjuC_ : 1;
        osal_u32 xCt2O4vRyOloO4mCv4lo2OwsROyGs_ : 1;
        osal_u32 xCt2OsmGOGdRO4mRORlImvSR_ : 8;
        osal_u32 xCt2OPAROISOyCCOm8Rwz_ : 1;
        osal_u32 reserved_1 : 16;
    } bits;

    osal_u32 u32;
} u_phy_rx_ctrl;

typedef union {
    struct {
        osal_u32 xCt2Orrz8OsRzOl4_ : 11;
        osal_u32 x6NgwGPw_Cqw6QKwWh_ : 1;
        osal_u32 x6Ngw00GPwg_iduwCLGwBF_ : 11;
        osal_u32 xCt2Orrz8OISxRlOyssOmo_ : 1;
        osal_u32 x6Ngw00GPwCLGBFwu_Bi_BLt_ : 3;
        osal_u32 reserved_0 : 5;
    } bits;

    osal_u32 u32;
} u_phy_rx_11ax_ctrl0;

typedef union {
    struct {
        osal_u32 x6NgwSBSiwFWLwFWoGt_ : 9;
        osal_u32 xCt2OCmO4mxzQ_ : 6;
        osal_u32 x6NgwSiFwFWLwFWoGt_ : 8;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_rx_module_delay;

typedef union {
    struct {
        osal_u32 reserved_0 : 9;
        osal_u32 xCt2OaRAOIv4O4mRO4xQ_ : 8;
        osal_u32 xCt2OAmOK8OsIvvRAO4mxzQ_ : 8;
        osal_u32 x6NgwQWwFWoGtwiNFShdS_ : 6;
        osal_u32 reserved_1 : 1;
    } bits;

    osal_u32 u32;
} u_cfg_rx_buffer_delay_ctl;

typedef union {
    struct {
        osal_u32 x6NgwNBhWwLBSBhgwFWoGt_ : 13;
        osal_u32 xCt2OtlomORlIlo2Osmx_ : 1;
        osal_u32 reserved_0 : 18;
    } bits;

    osal_u32 u32;
} u_fine_timing_delay;

typedef union {
    struct {
        osal_u32 xCt2Oxm2zOz4WO4mxzQ_ : 9;
        osal_u32 xCt2Oxm2zOZsQIOz4WO4mxzQ_ : 10;
        osal_u32 reserved_0 : 13;
    } bits;

    osal_u32 u32;
} u_lega_adj_delay;

typedef union {
    struct {
        osal_u32 x6Ngw_PwiNFSwuo6uwFot_ : 11;
        osal_u32 xCt2ORwl2O4mxRO4xQ_ : 7;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_rx_ofdm_plcp_dly;

typedef union {
    struct {
        osal_u32 x6NgwoFu6woGCLwLBSWy_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 x6NgwoFu6woGCLwLBSWV_ : 8;
        osal_u32 x6NgwoFu6woGCLwLBSWk_ : 8;
    } bits;

    osal_u32 u32;
} u_ldpc_itert_num1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xCt2Ox4GCOxzsRORlImY_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_ldpc_itert_num2;

typedef union {
    struct {
        osal_u32 xCt2OxzsROlRmwROSsmzyxm_ : 1;
        osal_u32 xCt2O4CIOx4GCOICs3Osmx_ : 1;
        osal_u32 x6NgwoFu6woGCLwhdSw6Go6_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_ldpc_itert_usemode;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 x6Ngw00GPwCLj6wCWo_ : 1;
        osal_u32 x6NgwL_o_wFihWw6hLwSGP_ : 8;
        osal_u32 reserved_1 : 21;
    } bits;

    osal_u32 u32;
} u_ldpc_mem_wdth;

typedef union {
    struct {
        osal_u32 xCt2Ow8OrryOz4WO4xQ_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_rx_11b_adjust;

typedef union {
    struct {
        osal_u32 xCt2OR8OsmwPlCmOtx4_ : 1;
        osal_u32 x6NgwWhw6ihLwLP_ : 1;
        osal_u32 xCt2OCvoROR8OGzRRmwo_ : 2;
        osal_u32 xCt2OCvoROR8OtwI_ : 1;
        osal_u32 xCt2OR8OGwmzIyxmOvoxQ_ : 1;
        osal_u32 xCt2OzslCOIv4m_ : 1;
        osal_u32 x6NgwRwFthwgGBhwjtuGCC_ : 1;
        osal_u32 reserved_0 : 20;
        osal_u32 xCt2O4G4OvttxlomOtwm6OsAltR_ : 4;
    } bits;

    osal_u32 u32;
} u_tx_control;

typedef union {
    struct {
        osal_u32 x6Ngw6ihLwLPwgGu_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cfg_cont_tx_gap;

typedef union {
    struct {
        osal_u32 xCt2OR8ORlImOvSRORAw_ : 16;
        osal_u32 xCt2OR8w4QORlImvSROmoy_ : 1;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_time_out_thr;

typedef union {
    struct {
        osal_u32 xCt2OSGORlImOsmx_ : 1;
        osal_u32 xCt2OwtOsmRRxmORlIm_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OrryOR8O4xQ_ : 8;
        osal_u32 reserved_1 : 11;
    } bits;

    osal_u32 u32;
} u_sifs_ctrl;

typedef union {
    struct {
        osal_u32 reserved_0 : 8;
        osal_u32 xCt2OR8O4xQOZ3IORlImOAm_ : 8;
        osal_u32 x6NgwLPwFotwDySwLBSWwQW_ : 8;
        osal_u32 reserved_1 : 8;
    } bits;

    osal_u32 u32;
} u_tx_time_1dly;

typedef union {
    struct {
        osal_u32 x6NgwLPwFotwVySwLBSW_ : 8;
        osal_u32 xCt2OR8O4xQOK3IORlIm_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 xCt2OR8O4xQOrryORlIm_ : 8;
    } bits;

    osal_u32 u32;
} u_tx_dfe_0dly;

typedef union {
    struct {
        osal_u32 xRlImOCs43OsAltROx_ : 7;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_timin_csd0_shift_l;

typedef union {
    struct {
        osal_u32 xtwm6O3OCs4_ : 4;
        osal_u32 xN_WRw0w6CF_ : 4;
        osal_u32 xtwm6OZOCs4_ : 4;
        osal_u32 xtwm6OiOCs4_ : 4;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_freq_0_csd;

typedef union {
    struct {
        osal_u32 xN_WRwDw6CF_ : 4;
        osal_u32 xtwm6OaOCs4_ : 4;
        osal_u32 xN_WRwXw6CF_ : 4;
        osal_u32 xtwm6O9OCs4_ : 4;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_freq_1_csd;

typedef union {
    struct {
        osal_u32 xGlxvROytOmo_ : 1;
        osal_u32 x6NgwjNwQwLWCL_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_pilot_bf_en;

typedef union {
    struct {
        osal_u32 xowoLNwSGPw6ud_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_l_ltf_max_cpu;

typedef union {
    struct {
        osal_u32 xNoGLwxwVyw6iWN_ : 5;
        osal_u32 xtxzROKOZ3OCvmt_ : 5;
        osal_u32 xNoGLwkwVyw6iWN_ : 5;
        osal_u32 xNoGLwVwVyw6iWN_ : 5;
        osal_u32 xtxzROrOZ3OCvmt_ : 5;
        osal_u32 xNoGLwywVyw6iWN_ : 5;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_flatness_0_20_coef;

typedef union {
    struct {
        osal_u32 xNoGLwpwVyw6iWN_ : 5;
        osal_u32 xtxzROYOZ3OCvmt_ : 5;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_flatness_1_20_coef;

typedef union {
    struct {
        osal_u32 xtxzROaOK3OCvmt_ : 5;
        osal_u32 xNoGLwDwDyw6iWN_ : 5;
        osal_u32 xtxzROiOK3OCvmt_ : 5;
        osal_u32 xtxzROZOK3OCvmt_ : 5;
        osal_u32 xNoGLw0wDyw6iWN_ : 5;
        osal_u32 xtxzRO3OK3OCvmt_ : 5;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_flatness_0_40_coef;

typedef union {
    struct {
        osal_u32 xNoGLw00wDyw6iWN_ : 5;
        osal_u32 xtxzROr3OK3OCvmt_ : 5;
        osal_u32 xtxzROcOK3OCvmt_ : 5;
        osal_u32 xNoGLwUwDyw6iWN_ : 5;
        osal_u32 xtxzRO9OK3OCvmt_ : 5;
        osal_u32 xNoGLwXwDyw6iWN_ : 5;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_flatness_1_40_coef;

typedef union {
    struct {
        osal_u32 xtxzROraOK3OCvmt_ : 5;
        osal_u32 xNoGLw0DwDyw6iWN_ : 5;
        osal_u32 xtxzROriOK3OCvmt_ : 5;
        osal_u32 xtxzROrZOK3OCvmt_ : 5;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_flatness_2_40_coef;

typedef union {
    struct {
        osal_u32 xwGROvSROsQIOCoR_ : 20;
        osal_u32 xwGROR8OyzsmOsRzRm_ : 3;
        osal_u32 xwGROvSRO4zRzOtxz2_ : 1;
        osal_u32 x_uLwSG6wBNwLPwCLGLW_ : 2;
        osal_u32 x_uLwLPwiNFSwNoGg_ : 1;
        osal_u32 xwGROR8rryO4zRzOmoOo_ : 1;
        osal_u32 xwGROIzCOR8OmoOo_ : 1;
        osal_u32 xCt2OR8ORmsROIv4m_ : 3;
    } bits;

    osal_u32 u32;
} u_tx_debug_info;

typedef union {
    struct {
        osal_u32 x6Ngw_PC6iwuQGCWwiNNCWL_ : 10;
        osal_u32 x6Ngw_PC6iwuQGCWwSGh_ : 10;
        osal_u32 xCt2Ow8sCvOGAzsmOsmx_ : 1;
        osal_u32 xCt2OR8sCvOGAzsmOsmx_ : 1;
        osal_u32 x6NgwC6iw6GL6QwNLwjuC_ : 1;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_trx_sco_cfg;

typedef union {
    struct {
        osal_u32 xCt2ORw8OCxdOtwm6_ : 2;
        osal_u32 xCt2ORw8OCxdOtwm6Osmx_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_trx_clk_freq;

typedef union {
    struct {
        osal_u32 x_uLwBhL_w_GFG_wFWL_ : 1;
        osal_u32 x_uLwBhL_wSB6_iwTGqWwFWL_ : 1;
        osal_u32 xwGROloRwOwtOR84lPOCzxlO4mR_ : 1;
        osal_u32 xwGROloRwOwtOw84lPOCzxlO4mR_ : 1;
        osal_u32 xwGROloRwOR8w4QORlImOvSR_ : 1;
        osal_u32 xwGROloRwOGs4O4mR_ : 1;
        osal_u32 xwGROqlRAOzClO4mR_ : 1;
        osal_u32 x_uLwTBLQidLwG6BwFWL_ : 1;
        osal_u32 x_uLwFWoLGwF6i6wjBgwFWL_ : 1;
        osal_u32 x_uLw_PF6wihoBhWwjBgwFWL_ : 1;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_phy_intr_rpt;

typedef union {
    struct {
        osal_u32 xCt2O4xOISOIlIvOw8OyQGzss_ : 1;
        osal_u32 x6NgwCLG0wCLGVwg_iduwu_Bi_ : 1;
        osal_u32 x6Ngw_PwLjwWhGjoW_ : 1;
        osal_u32 reserved_0 : 13;
        osal_u32 xwGROISO4mROmwwO2wvSGOsRsOvPmwOCzGz_ : 8;
        osal_u32 x_uLwSdwFWLwW__wg_iduwCWG_6Q_ : 8;
    } bits;

    osal_u32 u32;
} u_dl_mu_mimo_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OR8OsxGOmoz_ : 1;
        osal_u32 xCt2Ow8OsxGOmoz_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_wl_slp_position_ctrl;

typedef union {
    struct {
        osal_u32 xwGROCvzwsmOGAzsmOmsR_ : 10;
        osal_u32 x_uLwNBhWwuQGCWwWCL_ : 10;
        osal_u32 xwGROwmtlomOGAzsm_ : 12;
    } bits;

    osal_u32 u32;
} u_rpt_phase_est;

typedef union {
    struct {
        osal_u32 xwGROsowOmsR_ : 9;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_rpt_snr_est;

typedef union {
    struct {
        osal_u32 xN6CwW__i__ : 1;
        osal_u32 xwGROw8tltvOtSxxOmww_ : 1;
        osal_u32 xwGROR8tltvOtSxxOmww_ : 1;
        osal_u32 xwGROR84zRzOm8Cmm4Omww_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_macif_fifo_err_rpt;

typedef union {
    struct {
        osal_u32 xwGROxsl2OGzwlRQOmww_ : 1;
        osal_u32 xwGROxsl2OwmsPOmww_ : 1;
        osal_u32 x_uLwoCBgw_GLWwW___ : 1;
        osal_u32 xwGROARsl2OCwCOmww_ : 1;
        osal_u32 xwGROsvSo4lo2OxmoOmww_ : 1;
        osal_u32 xwGROsvSo4lo2OomssOmww_ : 1;
        osal_u32 xwGROICsiZOmww_ : 1;
        osal_u32 xwGROARO2tOmww_ : 1;
        osal_u32 x_uLwQLwBoowS6CwW___ : 1;
        osal_u32 xwGROAROlxxOwmsPOmww_ : 1;
        osal_u32 x_uLwQLwBoowCLj6wW___ : 1;
        osal_u32 x_uLwqQLwBoowSdwW___ : 1;
        osal_u32 x_uLwqQLwBoow_WCqwW___ : 1;
        osal_u32 xwGROPAROlxxOs2lOmww_ : 1;
        osal_u32 xwGROPAROlxxOICsOmww_ : 1;
        osal_u32 x_uLwqQLCBgGw6_6wW___ : 1;
        osal_u32 xwGROPARsl2yOCwCOmww_ : 1;
        osal_u32 xwGROo4GOmww_ : 1;
        osal_u32 xwGROx2Oyq_ : 1;
        osal_u32 x_uLwogwhCC_ : 1;
        osal_u32 xwGROx2OosRsOomss_ : 1;
        osal_u32 xwGROISO4mR_ : 1;
        osal_u32 x_uLwogwS6C_ : 1;
        osal_u32 x_uLwihWwQouwCidhFBhg_ : 1;
        osal_u32 xwGROvt4IOloROyQOrry_ : 1;
        osal_u32 xwGROmzwxQOCzwwlmwOxvsm_ : 1;
        osal_u32 x_uLw_Pwuo6uwiqL_ : 1;
        osal_u32 xFiLjwW__wFWL_ : 1;
        osal_u32 x_uLwFWwBhLoqwjoi6K_ : 1;
        osal_u32 x_uLwC6iwNBNiwNdoo_ : 1;
        osal_u32 xSG6VuQtwLPwF_tdu_ : 1;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_phy_err_rpt;

typedef union {
    struct {
        osal_u32 xGzCdmRO4mR_ : 1;
        osal_u32 x6iG_CWwLBSBhgwFWL_ : 1;
        osal_u32 xtlomORlIlo2O4mR_ : 1;
        osal_u32 x4vRyOzC6O4mR_ : 1;
        osal_u32 x4vRyOGzCdmRO4mR_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x_uLwL_o_wiqL_ : 1;
        osal_u32 xwGROCCzOvPR_ : 1;
        osal_u32 xwGROtwmmpmOvPR_ : 1;
        osal_u32 xwGROsmCOsQoCO4mR_ : 1;
        osal_u32 x_uLwWG_otwdhoi6K_ : 1;
        osal_u32 xwGROwsPOwSOzxxvCO4mR_ : 1;
        osal_u32 x_uLwGPwSdoLBwdCW_wNoFwSGL6Q_ : 1;
        osal_u32 x_uLwhiSGL6QwdCW_wNoFwNoGg_ : 1;
        osal_u32 xwGROAmOmwOsSOlxxO4mR_ : 1;
        osal_u32 x_uLwQWwF6SwFWL_ : 1;
        osal_u32 xwGROAmxRtOr8O4mR_ : 1;
        osal_u32 x_uLwjWGS6QGhgWwywFWL_ : 1;
        osal_u32 x_uLw_oCBgw_WCqwW___ : 1;
        osal_u32 xwGROwxsl2OwzRmOmww_ : 1;
        osal_u32 x_uLw_oCBgwuG_BLtwW___ : 1;
        osal_u32 x_uLwQWCBgjwdC_wNoFw6_6wW___ : 1;
        osal_u32 x_uLwQWCBgjw6iSwNoFw6_6wW___ : 1;
        osal_u32 xwGROAmsl2zOAmISOlxxOAmxRtOsQI_ : 1;
        osal_u32 xwGROAmsl2zOAmISOlxxOsl2yICs_ : 1;
        osal_u32 x_uLwQWCBgGwBoow_WCqwW___ : 1;
        osal_u32 xwGROAmsl2zOCwCOmww_ : 1;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_phy_warn_rpt;

typedef union {
    struct {
        osal_u32 xGvqmwO4mROmww_ : 1;
        osal_u32 xCvzwsmOtwm6Omww_ : 1;
        osal_u32 xNBhWwN_WRwW___ : 1;
        osal_u32 xoLNwuWGKwG6wW___ : 1;
        osal_u32 xRlIlo2OsQoCOvPR_ : 1;
        osal_u32 x6iG_CWwLBSBhgwiqL_ : 1;
        osal_u32 xwGROsQoCOx2O4C_ : 1;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_sync_err_rpt;

typedef union {
    struct {
        osal_u32 x_uLw_PNBNiwNdoowCL_ : 1;
        osal_u32 x_uLw_PNBNiwWSuLtwCL_ : 1;
        osal_u32 x_uLwLPNBNiwNdoowCL_ : 1;
        osal_u32 x_uLwLPNBNiwWSuLtwCL_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_macif_fifo_state_rpt;

typedef union {
    struct {
        osal_u32 xGzCdmRO4mROsR_ : 6;
        osal_u32 xz2COxvCd_ : 1;
        osal_u32 xsQoCOtwmmpmOz2C_ : 1;
        osal_u32 x6d_w66GwCLGLW_ : 2;
        osal_u32 reserved_0 : 8;
        osal_u32 xloRxPOGzo2OtSxx_ : 1;
        osal_u32 xloRxPOGlo2OtSxx_ : 1;
        osal_u32 xCL_WGSwuGhgwNdoo_ : 1;
        osal_u32 xCL_WGSwuBhgwNdoo_ : 1;
        osal_u32 xz2COCSwwOsR_ : 3;
        osal_u32 x_uLwC6iwNBNiwWSuLt_ : 1;
        osal_u32 reserved_1 : 2;
        osal_u32 xwGROx4GCOltltvOmIGRQ_ : 1;
        osal_u32 xwGROx4GCOltltvOtSxx_ : 1;
        osal_u32 xwGROsmCOsQoCOtxz2_ : 1;
        osal_u32 reserved_2 : 1;
    } bits;

    osal_u32 u32;
} u_phy_state_rpt0;

typedef union {
    struct {
        osal_u32 xttROsQIOCoR_ : 15;
        osal_u32 xwGROAROyqOmww_ : 1;
        osal_u32 xwGROovoOAROyqOmww_ : 1;
        osal_u32 xwGROR8OCAOsmxOmww_ : 1;
        osal_u32 xwGROyqO2ROGIOmww_ : 1;
        osal_u32 xwGRO4vRyOa2Omww_ : 1;
        osal_u32 x_uLwjTwqW6Li_wW___ : 1;
        osal_u32 xwGROtmCCOPmCRvwOmww_ : 1;
        osal_u32 xwGROR8OPmCRvwOmww_ : 1;
        osal_u32 xwGROR8OrryOw4QyOsR_ : 1;
        osal_u32 x_uLwLPwCWhFwiqW_wCL_ : 1;
        osal_u32 x_uLwLPwWhw_NVuQtwCL_ : 1;
        osal_u32 xwGROvSRrryR8tsICSwwsRzRm_ : 3;
        osal_u32 reserved_0 : 3;
    } bits;

    osal_u32 u32;
} u_phy_state_rpt1;

typedef union {
    struct {
        osal_u32 xtwzImOxmoOCoR_ : 20;
        osal_u32 xFiLjwBhwFW6iFBhg_ : 1;
        osal_u32 xiNFSwBhwFW6iFBhg_ : 1;
        osal_u32 xvt4IOloROyQOrry_ : 1;
        osal_u32 xmzwxQOCzwwlmwOxvsm_ : 1;
        osal_u32 xGxCGOvPROtxz2_ : 1;
        osal_u32 x_PNBNiwNdoowNoGg_ : 1;
        osal_u32 xFWwBhLoqwjoi6KwNoGg_ : 1;
        osal_u32 xiNFSwQWGFW_wW__i__ : 1;
        osal_u32 xiNFSwdhCduui_LwSiFW_ : 1;
        osal_u32 x_Pw6L_owoCBgw_Ft_ : 1;
        osal_u32 xw8OCRwxOARsl2Ow4Q_ : 1;
        osal_u32 xFNWwCWhFwFGLGwNoGg_ : 1;
    } bits;

    osal_u32 u32;
} u_phy_state_rpt2;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 xR8w8O4tmOr4vIO3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 x4G4OR8O3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xR8O4tmOZ4vIO3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_3 : 1;
        osal_u32 x6N_wLPwy6Qw_CLw_WRwh_ : 1;
        osal_u32 xCtwOR8OrCAOwsROwm6Oo_ : 1;
        osal_u32 xCzxlOR8O3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_4 : 3;
        osal_u32 x4vRyOKKOwsROwm6Oo_ : 1;
        osal_u32 reserved_5 : 3;
        osal_u32 x_Pw6L_ow_CLw_WRwh_ : 1;
        osal_u32 xSG6BNw_Pw_CLw_WRwh_ : 1;
        osal_u32 xFW6iFW_w_CLw_WRwh_ : 1;
        osal_u32 reserved_6 : 1;
        osal_u32 xqBLW_jBw_CLw_WRwh_ : 1;
        osal_u32 x66GwTBFwy6Qw_WRwh_ : 1;
        osal_u32 xCCzOz8O3CAOwm6Oo_ : 1;
        osal_u32 xCCzOovGwlOZ3O4mRO3CAOwm6Oo_ : 1;
        osal_u32 x66Gwhiu_BwDywFWLwy6Qw_WRwh_ : 1;
        osal_u32 x66Gwhiu_BwUywFWLwy6Qw_WRwh_ : 1;
        osal_u32 xGZIOw8OwsROwm6Oo_ : 1;
        osal_u32 x_Pw_CLw_WRwh_ : 1;
        osal_u32 xLPwFNWw0FiSwLPC6iwy6Qw_CLw_WRwh_ : 1;
        osal_u32 xR8O4tmOr4vIOR8sCvOrCAOwsROwm6Oo_ : 1;
        osal_u32 reserved_7 : 1;
    } bits;

    osal_u32 u32;
} u_phy_state_rpt3;

typedef union {
    struct {
        osal_u32 xCth6wUyw_CLw_WRwh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xFiLjwUyw_CLw_WRwh_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 xR8OrY3OwsROwm6Oo_ : 1;
        osal_u32 x4tmOovRCAO3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xCAoOmsROwsROwm6Oo_ : 1;
        osal_u32 reserved_3 : 2;
        osal_u32 xjNw_CLw_WRwh_ : 1;
        osal_u32 reserved_4 : 2;
        osal_u32 xuQtwLPw_CLw_WRwh_ : 1;
        osal_u32 reserved_5 : 1;
        osal_u32 xSG6BNwLPw_CLw_WRwh_ : 1;
        osal_u32 xLP_PwNNLw_CLw_WRwh_ : 1;
        osal_u32 xsQoCO4zRzOGzRAOwsROwm6Oo_ : 1;
        osal_u32 xR8w8OttROZ8OwsROwm6Oo_ : 1;
        osal_u32 xw8O4tmOytxvCdO3CAOwsROwm6Oo_ : 1;
        osal_u32 reserved_6 : 1;
        osal_u32 x_PwFNWwGNoi6Kwy6Qw_CLw_WRwh_ : 1;
        osal_u32 reserved_7 : 3;
        osal_u32 x_Pw6GoBw_CLw_WRwh_ : 1;
        osal_u32 reserved_8 : 1;
        osal_u32 xz2COwsROwm6Oo_ : 1;
        osal_u32 xNLSw_CLw_WRwh_ : 1;
        osal_u32 x66Gwy6Qw_CLw_WRwh_ : 1;
        osal_u32 reserved_9 : 1;
        osal_u32 xLPwFNWw0FiSwy6Qw_CLw_WRwh_ : 1;
    } bits;

    osal_u32 u32;
} u_phy_state_rpt4;

typedef union {
    struct {
        osal_u32 xCt2OtwIOvdOmwwOsRzOmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_sta_en;

typedef union {
    struct {
        osal_u32 x4vRyOvdOtwIOCxw_ : 1;
        osal_u32 xQLwiKwN_Sw6o__ : 1;
        osal_u32 xPAROvdOtwIOCxw_ : 1;
        osal_u32 xoWgGwiKwN_Sw6o__ : 1;
        osal_u32 xFiLjwW__wN_Sw6o__ : 1;
        osal_u32 xAROmwwOtwIOCxw_ : 1;
        osal_u32 xqQLwW__wN_Sw6o__ : 1;
        osal_u32 xxm2zOmwwOtwIOCxw_ : 1;
        osal_u32 xsRz3OwGROCxw_ : 1;
        osal_u32 xCLG0w_uLw6o__ : 1;
        osal_u32 xsRzZOwGROCxw_ : 1;
        osal_u32 xsRziOwGROCxw_ : 1;
        osal_u32 xSdwhCLCw_uLw6o__ : 1;
        osal_u32 xsQoCOvdOtwIOCxw_ : 1;
        osal_u32 xCLGDw_uLw6o__ : 1;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_stat_clr;

typedef union {
    struct {
        osal_u32 x6NgwCLGywWh_ : 16;
        osal_u32 xCt2OsRzrOmo_ : 16;
    } bits;

    osal_u32 u32;
} u_phy_sta_01_en;

typedef union {
    struct {
        osal_u32 x6NgwCLGVwWh_ : 16;
        osal_u32 x6NgwCLGkwWh_ : 16;
    } bits;

    osal_u32 u32;
} u_phy_sta_23_en;

typedef union {
    struct {
        osal_u32 xGAQOsRzKOwGR_ : 16;
        osal_u32 xCt2OsRzKOmo_ : 1;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_phy_sta_4_en_and_rpt;

typedef union {
    struct {
        osal_u32 x6NgwLWCLwuBhwCWo_ : 4;
        osal_u32 x6NgwLWCLwuBhwWh_ : 1;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_test_pin_sel;

typedef union {
    struct {
        osal_u32 xwGROqxOGAQOPmwO4zQ_ : 8;
        osal_u32 xwGROqxOGAQOPmwOIvoRA_ : 4;
        osal_u32 xwGROqxOGAQOPmwOQmzw_ : 5;
        osal_u32 x_uLwTowuQtwqW_whdS_ : 7;
        osal_u32 x_uLwTowuQtwu_ibwBhNi_ : 8;
    } bits;

    osal_u32 u32;
} u_wl_phy_version;

typedef union {
    struct {
        osal_u32 xCt2OISOosRsOwGR_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_mumimo_nsts_rpt;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2O4myS2OszIGxmOmo_ : 1;
        osal_u32 cfg_debug_sample_start_mode : 6;
        osal_u32 cfg_debug_sample_end_mode : 6;
        osal_u32 x6NgwFWjdgwFGSuoWwFGLGwLQ_ : 9;
        osal_u32 reserved_1 : 2;
        osal_u32 x6NgwuQtwFGLGwCGSuoWw6gwjuC_ : 1;
        osal_u32 x6NgwuQtwFGLGwCGSuoWw6oKwWh_ : 1;
        osal_u32 x6NgwWqWhLw_uLw6gwjuC_ : 1;
        osal_u32 reserved_2 : 3;
    } bits;

    osal_u32 u32;
} u_sample_cfg_0;

typedef union {
    struct {
        osal_u32 xCt2OtwzImO4zRzOxmo2RA_ : 24;
        osal_u32 x6NgwCGSuoWwS6C_ : 6;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_sample_cfg_1;

typedef union {
    struct {
        osal_u32 x6NgwCGSuw6oKwCWo_ : 2;
        osal_u32 x6NgwFWjdgwCGSuoWwjtwoWh_ : 1;
        osal_u32 xCt2O2moOCxdOsmx_ : 1;
        osal_u32 xCt2ORw8OszIGxmOyQOGvs_ : 1;
        osal_u32 cfg_debug_sample_delay : 8;
        osal_u32 cfg_with_sample_delay_en : 1;
        osal_u32 cfg_with_mac_info_en : 1;
        osal_u32 cfg_with_mac_tsf_en : 1;
        osal_u32 xCt2OszIGxmOGwvRvCvx_ : 3;
        osal_u32 reserved_0 : 13;
    } bits;

    osal_u32 u32;
} u_sample_cfg_2;

typedef union {
    struct {
        osal_u32 x6Ngw_CCBwWhFwLQV_ : 8;
        osal_u32 xCt2OwsslOmo4ORAr_ : 8;
        osal_u32 x6Ngw_CCBwCLG_LwLQV_ : 8;
        osal_u32 xCt2OwsslOsRzwRORAr_ : 8;
    } bits;

    osal_u32 u32;
} u_sample_cfg_3;

typedef union {
    struct {
        osal_u32 xCt2OCCzOvPROCoROA4wOCxwOmo_ : 1;
        osal_u32 xCt2OCCzOvPROCoROGdROmo4OCxwOmo_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cca_ovt_cfg;

typedef union {
    struct {
        osal_u32 x6NgwWqWhLw_uLwWh_ : 1;
        osal_u32 x6NgwWqWhLw_uLwu_W6BCBihwCWo_ : 3;
        osal_u32 xCt2OmPmoROwGROoSIOmo_ : 17;
        osal_u32 reserved_0 : 11;
    } bits;

    osal_u32 u32;
} u_event_rpt_cfg;
#endif
