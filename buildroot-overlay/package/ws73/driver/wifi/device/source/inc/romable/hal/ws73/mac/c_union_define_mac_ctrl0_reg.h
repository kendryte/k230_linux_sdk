/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_CTRL0_REG_H__
#define __C_UNION_DEFINE_MAC_CTRL0_REG_H__

typedef union {
    struct {
        osal_u32 x6NgwTowCLG_LduwWh_ : 1;
        osal_u32 xCt2OAlGwlOw8O6Omo_ : 1;
        osal_u32 xCt2OwmsGOwzRmORmsR_ : 1;
        osal_u32 x6NgwWF6GwWh_ : 1;
        osal_u32 x6NgwFihwLwLid6QwSuFd_ : 1;
        osal_u32 xCt2OwRsOwzRmOCzxCOzROyzslCOwzRm_ : 1;
        osal_u32 x6Ngw_WCuwGLwjGCB6w_GLWwWh_ : 1;
        osal_u32 xCt2OwRsOwzRmOsmx_ : 2;
        osal_u32 xCt2OR8OtvwmPmwOmo_ : 1;
        osal_u32 x6NgwoBNWLBSWw6QKwWh_ : 1;
        osal_u32 xCt2OCvm8OzyvwROmo_ : 1;
        osal_u32 x6NgwLPwCdCuWhF_ : 1;
        osal_u32 x6Ngw_PwFduw6QKwWh_ : 1;
        osal_u32 x6Ngw_LCwNGBowLPwuCFdwWh_ : 1;
        osal_u32 xCt2OR8OwRsO4SGOmo_ : 1;
        osal_u32 x6Ngwu_iLi6iowqW_CBih_ : 2;
        osal_u32 xCt2OGAQOIz8Oyq_ : 2;
        osal_u32 x6NgwuQtwSGPwhCC_ : 3;
        osal_u32 xCt2OAmOGG4SOCAmCdOmo_ : 1;
        osal_u32 xCt2OGZGOGsOmo_ : 1;
        osal_u32 xCt2O4mOzIs4SOmo_ : 1;
        osal_u32 x6NgwLPw6NwWhFwWh_ : 1;
        osal_u32 x6NgwuQtwBhNiwWh_ : 1;
        osal_u32 x6NgwihWwuKLwWh_ : 1;
        osal_u32 reserved_0 : 3;
    } bits;

    osal_u32 u32;
} u_pa_control1;

typedef union {
    struct {
        osal_u32 x6NgwqGuywSiFW_ : 2;
        osal_u32 xCt2OPzGrOIv4m_ : 2;
        osal_u32 x6NgwGhLwCWowFBq_ : 2;
        osal_u32 xCt2OAmOsvSo4lo2Omo_ : 1;
        osal_u32 xCt2OAROsvSo4lo2Omo_ : 1;
        osal_u32 x6NgwqQLwCidhFBhgwWh_ : 1;
        osal_u32 x6NgwCidhFBhgwFWoGtw_WCuw_uLwWh_ : 1;
        osal_u32 xCt2OR8vGOGsOmo_ : 1;
        osal_u32 x6NgwLPiuwuCw06ihF_ : 1;
        osal_u32 xCt2OR8vGOGsOZCvo4_ : 1;
        osal_u32 xCt2OPzG3ORstOmo_ : 1;
        osal_u32 x6NgwqGu0wLCNwWh_ : 1;
        osal_u32 x6NgwSdoLBwjCCBFwqGuwCWo_ : 2;
        osal_u32 xCt2OotwGOISxRlOyssl4Omo_ : 1;
        osal_u32 xCt2OCRwxOISxRlOyssl4Omo_ : 1;
        osal_u32 xCt2OISxRlOyssl4Omo_ : 1;
        osal_u32 x6NgwSdoLBwjCCBFw6QKwjCCBFwWh_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2Ow8OloRwOSolR_ : 4;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_pa_control2;

typedef union {
    struct {
        osal_u32 xCt2OzIG4SOwmGxzQO4mRmCROmo_ : 1;
        osal_u32 xCt2OsGGOzIs4SOmo_ : 1;
        osal_u32 x6NgwLKBuwSB6wWh_ : 1;
        osal_u32 xCt2OwmGxzQO4mRmCROmo_ : 1;
        osal_u32 xCt2O4voROloCOGoOwmRwQ_ : 1;
        osal_u32 x6Ngw00hwLKBuwjtuGCCwWh_ : 1;
        osal_u32 xCt2OrrzCORdlGOyQGzssOmo_ : 1;
        osal_u32 x6NgwuWW_wGFF_wW__w6WwjtuGCCwWh_ : 1;
        osal_u32 xCt2OPzG3OI2IROyCO4mCwOmo_ : 1;
        osal_u32 x6NgwqGu0wSgSLwj6wFW6_wWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OCmOwsRORlIm_ : 6;
        osal_u32 xCt2O4zRzOmoCwOmo_ : 1;
        osal_u32 x6NgwFGLGwFW6_wWh_ : 1;
        osal_u32 x6NgwuSNwWh6_wWh_ : 1;
        osal_u32 x6NgwuSNwFW6wWh_ : 1;
        osal_u32 reserved_1 : 11;
    } bits;

    osal_u32 u32;
} u_ce_control;

typedef union {
    struct {
        osal_u32 xCt2OR8vGOCAdOCCzOzvoOmo_ : 1;
        osal_u32 x6NgwLPiuw_WSwLBSWw6QKwWh_ : 1;
        osal_u32 xCt2Ow8OAlGwlOloOovwIOySttOmo_ : 1;
        osal_u32 xCt2Ow8OsIzxxOloOovwIOySttOmo_ : 1;
        osal_u32 x6NgwLPiuw_LCwSiFW_ : 2;
        osal_u32 xCt2Ow8OSolO4mCwOI2IROwmsvxPmOmo_ : 1;
        osal_u32 x6Ngw_PwDGFF_wqGuwBhFWP_ : 3;
        osal_u32 xCt2OR8OGs4SOCoROz44OwRsOIv4m_ : 1;
        osal_u32 xCt2Ow8O4lwmCROtwzImOozPOSG4zRm_ : 1;
        osal_u32 xCt2OR8vGOozPORlImwOmo_ : 1;
        osal_u32 x6NgwhihwQLw_GLWw6Go6wSiFW_ : 1;
        osal_u32 xCt2Ow8OwmsGOCAdO4lwmCROIv4m_ : 1;
        osal_u32 x6NgwLPwj6hwBhwLPiuwWh_ : 1;
        osal_u32 xCt2OR8OyCoOltsOsmx_ : 1;
        osal_u32 x6Ngw_Pw_WCuw6QKwLGwWh_ : 1;
        osal_u32 x6Ngw_Pwj6wFduw6QKwFBC_ : 1;
        osal_u32 x6NgwGSuFdwShgwQBu_BwFBC_ : 1;
        osal_u32 x6Ngw_WCW_qWFwNi_wW6i_ : 6;
        osal_u32 x6NgwWF6GwLPw6NWhFwFd_w1W_iwWh_ : 1;
        osal_u32 x6NgwLPw6NWhFwGFF_VwCWLwSiFW_ : 1;
        osal_u32 xCt2O6SmSmOyzCdvttORlImvSRO4myS2Omo_ : 1;
        osal_u32 xCt2OR8OtlwsROtzlxOCtmo4Omo_ : 1;
        osal_u32 xCt2OR8OCtmo4OIv4m_ : 1;
        osal_u32 xCt2Ow8OwmsGOCAdOlIIm4lzRmOwmsGOmo_ : 1;
    } bits;

    osal_u32 u32;
} u_ht_vht_ctrl;

typedef union {
    struct {
        osal_u32 x6Ngw_Pw00GPwWh_ : 1;
        osal_u32 x6Ngw_PwQWw_iSwWh_ : 1;
        osal_u32 xCt2Ow8OAmOo4GzOGzwROwmsGOmo_ : 1;
        osal_u32 x6NgwSG6wL_PwjTDySwWh_ : 1;
        osal_u32 x6NgwQWwL_BgwBhCW_LwQL6wWh_ : 1;
        osal_u32 xCt2OAmOR8vGO4SwzRlvoO4lszyxm_ : 1;
        osal_u32 xCt2OAmOyswGOlosmwROARCOmo_ : 1;
        osal_u32 xCt2OAmORwl2OCsOCAdOmo_ : 1;
        osal_u32 x6NgwLjwLPiuwFd_GLBihwFBCGjoW_ : 1;
        osal_u32 xCt2OyqZ3OrsROl48_ : 1;
        osal_u32 x6NgwjTDyw0CLwBFP_ : 1;
        osal_u32 x6NgwjTDywVhFwBFP_ : 1;
        osal_u32 xCt2OAmOwvIOR8OyqOossOCAdOmo_ : 1;
        osal_u32 xCt2Ow8OAmOwvIO4lttOoss_ : 3;
        osal_u32 x6Ngw_PwQWw_iSwFBNNwjT_ : 2;
        osal_u32 xCt2Ow8OAmOwvIO4lttOIv4m_ : 1;
        osal_u32 reserved_0 : 13;
    } bits;

    osal_u32 u32;
} u_he_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwqGuyw_PwFGLGwLGw6QKwWh_ : 1;
        osal_u32 xCt2OPzGrOw8O4zRzORzOCAdOmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OszImOyssOCAdOwzOmo_ : 1;
        osal_u32 xCt2OszImOyssOCAdORzOmo_ : 1;
        osal_u32 x6NgwCGSWwjCCw6QKwjCCw6ioi_wWh_ : 1;
        osal_u32 xCt2OszImOyssOCAdOGzwRlzxOyssl4Omo_ : 1;
        osal_u32 xCt2OszImOyssOCAdOGzwRlzxOCvxvwOmo_ : 1;
        osal_u32 xCt2OszImOyssOCAdOtCsOmwwOGzwRlzxOCvxvwOmo_ : 1;
        osal_u32 xCt2OszImOyssOCAdOtCsOmwwOGzwRlzxOyssl4Omo_ : 1;
        osal_u32 x6NgwCGSWwjCCw6QKwN6CwW__wjCCw6ioi_wWh_ : 1;
        osal_u32 x6NgwQWwL_BgwhGqwWh_ : 1;
        osal_u32 xCt2OszImOyssOCAdOrrz8Oo4GOmo_ : 1;
        osal_u32 x6NgwCGSWwjCCw6QKwN6CwW__wWh_ : 1;
        osal_u32 x6NgwoCBgwhGqwduFGLWwWh_ : 1;
        osal_u32 x6Ngw_LCwLBSW_widL_ : 1;
        osal_u32 xCt2OR8vGOAvx4mwOxvo2ozPOyQGzssZ_ : 1;
        osal_u32 x6NgwLPiuwQioFW_woihghGqwjtuGCC0_ : 1;
        osal_u32 x6NgwLPiuwQioFW_w6QKwWh_ : 1;
        osal_u32 xCt2O6vsOGvxxOozPOszImOyssl4Omo_ : 1;
        osal_u32 x6Ngw_WCWLwjiLQwhGqwWh_ : 1;
        osal_u32 xCt2OAmORwsOozPOmo_ : 1;
        osal_u32 x6NgwRLuwRdBWLwCdCuWhFwWh_ : 1;
        osal_u32 x6NgwRLuwWh_ : 1;
        osal_u32 x6NgwRLuwLPiuwoBSBLwiNN_ : 1;
        osal_u32 reserved_1 : 7;
    } bits;

    osal_u32 u32;
} u_intra_bss_nav_qtp_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwQTw6Go6wGSuFdwoWhwWh_ : 1;
        osal_u32 x6NgwQTwuC_wjBLSGuwWh_ : 1;
        osal_u32 xCt2OovOsmsslvoO4voROwmsGOyzOmo_ : 1;
        osal_u32 xCt2OAqOR8OyzwOmo_ : 1;
        osal_u32 x6NgwQTwjG_wSGPwWhF_ : 1;
        osal_u32 x6NgwQTw_WL_twuG_LBGowGSuFdwWh_ : 1;
        osal_u32 x6NgwQTwLPwjG_w_GLWwCWo_ : 2;
        osal_u32 x6NgwQTwLPwjG_wSGPwhdS_ : 5;
        osal_u32 reserved_0 : 19;
    } bits;

    osal_u32 u32;
} u_ampdu_ctrl;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuw_GLWwCWL_ : 12;
        osal_u32 x6Ngw_WCuw6LCwWh_ : 1;
        osal_u32 xCt2O4zRzOwmsGOzCdOmo_ : 1;
        osal_u32 xCt2OI2IROwmsGOzCdOmo_ : 1;
        osal_u32 x6Ngw6L_ow_WCuwG6KwWh_ : 1;
        osal_u32 xCt2OwRsOwmsGOCAdOozPOmo_ : 1;
        osal_u32 x6Ngw_Pw6QKwL_o_wCLGLdCwFBC_ : 1;
        osal_u32 x6Ngw_WCuw_GLWw6Go6wSiFW_ : 1;
        osal_u32 x6Ngw_PwQLw_WCuwFduwWh_ : 1;
        osal_u32 x6Ngw_PwhihwQLw_WCuwFduwWh_ : 1;
        osal_u32 xCt2Ow8OPAROwmsGO4SGOmo_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OSsmwOwmsGOCRwx_ : 8;
    } bits;

    osal_u32 u32;
} u_tx_resp_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OoSIOGzOCxdsO4mClOSs_ : 6;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwhdSwuGw6oKCwihWwdC_ : 9;
        osal_u32 reserved_1 : 15;
    } bits;

    osal_u32 u32;
} u_num_pa_clks_us;

typedef union {
    struct {
        osal_u32 x6NgwhGqw_WCWL_ : 1;
        osal_u32 x6NgwLPwqiwRw_WCWL_ : 1;
        osal_u32 x6NgwLPwqBwRw_WCWL_ : 1;
        osal_u32 xCt2OR8OymO6OwmsmR_ : 1;
        osal_u32 x6NgwLPwjKwRw_WCWL_ : 1;
        osal_u32 x6NgwLPwQBu_BwRw_WCWL_ : 1;
        osal_u32 x6NgwLPwj6wRw_WCWL_ : 1;
        osal_u32 xCt2OzSRvOwsROqAmoORlImOvSR_ : 1;
        osal_u32 xCt2OzSRvOwsROqAmoOySsOmww_ : 1;
        osal_u32 x6NgwGdLiw_CLwuQtwTQWhw_PwLBSWidL_ : 1;
        osal_u32 x6NgwGdLiw_CLwuQtwTQWhwLPwLBSWidL_ : 1;
        osal_u32 xCt2OzSRvOwsROGAQOqAmoOySsOmww_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_mac_rst;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 x6Ngw6oKwLPjNwBNwGihwWh_ : 1;
        osal_u32 x6Ngw6oKw66SuwGihwWh_ : 1;
        osal_u32 xCt2OCxdOylGOzvoOmo_ : 1;
        osal_u32 x6NgwCoGqWwGdLiw6gw_WSiqWwFWoGt_ : 8;
        osal_u32 xCt2OzSRvOC2OwmIvPmO4mxzQ_ : 10;
        osal_u32 xCt2OCxdOsIKOzvoOmo_ : 1;
        osal_u32 reserved_1 : 9;
    } bits;

    osal_u32 u32;
} u_auto_cg_ctrl_remove_dleay;

typedef union {
    struct {
        osal_u32 x6NgwLPwLBSWidLwqGo_ : 6;
        osal_u32 reserved_0 : 2;
        osal_u32 x6Ngw_PwLBSWidLwqGo_ : 6;
        osal_u32 reserved_1 : 18;
    } bits;

    osal_u32 u32;
} u_txrx_time_out_value;

typedef union {
    struct {
        osal_u32 xCt2Ow8OwmsGORlImvSROPzx_ : 16;
        osal_u32 xCt2OGAQOw8OmwwOCoROmo_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OGAQOw8OCoROCxw_ : 2;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rx_resp_timeout_val_phy_rx_cnt;

typedef union {
    struct {
        osal_u32 xCt2OySsOIloOz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_bus_min_addr;

typedef union {
    struct {
        osal_u32 x6NgwjdCwSGPwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_bus_max_addr;

typedef union {
    struct {
        osal_u32 x6NgwjdNNwjGCWwGFF__ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_bus_max_2addr;

typedef union {
    struct {
        osal_u32 xCt2Ow8OovwIOySttOxmo_ : 12;
        osal_u32 x6Ngw_PwQBu_BwjdNNwoWh_ : 12;
        osal_u32 x6Ngw_PwCSGoowjdNNwoWh_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_buffer_length;

typedef union {
    struct {
        osal_u32 xCt2OIvolRvwOIv4mOmo_ : 1;
        osal_u32 xCt2ORzOm6OxvCzxOz44wOtxROmo_ : 1;
        osal_u32 xCt2OyCICOtxROmo_ : 1;
        osal_u32 x6Ngwj6S6wFGLGwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 x6Ngwj6S6wSgSLwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 xCt2OymzCvoOtxROmo_ : 1;
        osal_u32 xCt2OzRlIOtxROmo_ : 1;
        osal_u32 x6NgwBSSWFBGLWwjGwNoLwWh_ : 1;
        osal_u32 xCt2O4mxzQm4OyzOtxROmo_ : 1;
        osal_u32 x6Ngw6NwWhFwNoLwWh_ : 1;
        osal_u32 xCt2OvRAmwOCRwxOtwzImOtxROmo_ : 1;
        osal_u32 x6NgwhihwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 x6NgwiLQW_wjCCwFCLwGFF_wj6wNoLwWh_ : 1;
        osal_u32 x6NgwhihwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 x6NgwhihwFB_W6LwFGLGwNoLwWh_ : 1;
        osal_u32 xCt2OovoO4lwmCROCRwxOtxROmo_ : 1;
        osal_u32 x6NgwhihwFB_W6LwSgSLwNoLwWh_ : 1;
        osal_u32 xCt2OyCOdmQOsmzwCAOtzlxOtxROmo_ : 1;
        osal_u32 x6NgwBhqGoBFwN_GSWwLtuWwNoLwWh_ : 1;
        osal_u32 x6NgwLiiwoihgwSuFdwNoLwWh_ : 1;
        osal_u32 x6NgwFduwN_GSWwNoLwWh_ : 1;
        osal_u32 xCt2OtCsOtzlxOtxROmo_ : 1;
        osal_u32 x6NgwKWtwCWG_6QwNGBowNoLwWh_ : 1;
        osal_u32 x6Ngw66SuwSB6wNGBowNoLwWh_ : 1;
        osal_u32 xCt2OlCPOtzlxOtxROmo_ : 1;
        osal_u32 xCt2ORdlGOwmGxzQOtzlxOtxROmo_ : 1;
        osal_u32 x6Ngw66Suw_WuoGtwNGBowNoLwWh_ : 1;
        osal_u32 xCt2ORdlGOIlCOtzlxOtxROmo_ : 1;
        osal_u32 x6NgwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 x6NgwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ : 1;
        osal_u32 xCt2OylGOIlCOtzlxOtxROmo_ : 1;
        osal_u32 xCt2OylGOwmGxzQOtzlxOtxROmo_ : 1;
    } bits;

    osal_u32 u32;
} u_rx_framefilt1;

typedef union {
    struct {
        osal_u32 xCt2ORwl22mwORQGmOovoOIzRCAOtxROmo_ : 8;
        osal_u32 x6NgwL_BggW_wLtuWwNoLwWh_ : 8;
        osal_u32 x6NgwhFuGwhihwGBFwSGL6QwNoLwWh_ : 1;
        osal_u32 x6NgwhFuGwGBFwSGL6QwNoLwWh_ : 1;
        osal_u32 xCt2OvRAmwOyssOo4GzOtxROmo_ : 1;
        osal_u32 x6NgwhihwFB_W6LwhFuGwNoLwWh_ : 1;
        osal_u32 x6NgwFB_W6LwhFuGwNoLwWh_ : 1;
        osal_u32 x6Ngwj6wG6LBihwhiwG6KwNoLwWh_ : 1;
        osal_u32 xCt2Oq4sOovoO4lwmCROtxROmo_ : 1;
        osal_u32 xCt2Oq4sOyCICOtxROmo_ : 1;
        osal_u32 x6NgwTFCwNoLwWh_ : 1;
        osal_u32 xCt2OvRAmwOyssOymzCvoOtxROmo_ : 1;
        osal_u32 x6NgwiLQW_wjCCwL_BggW_wNoLwWh_ : 1;
        osal_u32 xCt2OszImOyssORwl22mwOtxROmo_ : 1;
        osal_u32 x6Ngw_PwSdoLBwjCCwFGLGwNoLwWh_ : 1;
        osal_u32 x6Ngw_PwSdoLBwjCCwSgSLwNoLwWh_ : 1;
        osal_u32 x6Ngw_PwFidjoWwSG6wSgSLwNoLwWh_ : 1;
        osal_u32 xCt2OqzGlOIlCOtzlxOtxROmo_ : 1;
    } bits;

    osal_u32 u32;
} u_rx_framefilt2;

typedef union {
    struct {
        osal_u32 x6Ngw_PwSuFdwSGPwoWh_ : 16;
        osal_u32 xCt2OqmzdOwsslOPzx_ : 8;
        osal_u32 x6NgwhiwFB_W6Lw00GgwF_iuwWh_ : 1;
        osal_u32 xCt2OovO4lwmCROrryO4wvGOmo_ : 1;
        osal_u32 xCt2OovO4lwmCROqmzdOwsslO4wvGOmo_ : 1;
        osal_u32 reserved_0 : 5;
    } bits;

    osal_u32 u32;
} u_rx_maxlenfilt_weak_rssi;

typedef union {
    struct {
        osal_u32 xCt2O4SwzRlvoOz4WSsROPzx_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_duration_adjust_val;

typedef union {
    struct {
        osal_u32 xCt2O4mtzSxRO4SwzRlvoOPzx_ : 16;
        osal_u32 xCt2OIz8O4SwzRlvoOPzx_ : 16;
    } bits;

    osal_u32 u32;
} u_max_duration_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwijCCwSGPwFd_GLBihwqGo_ : 16;
        osal_u32 x6NgwijCCwFWNGdoLwFd_GLBihwqGo_ : 16;
    } bits;

    osal_u32 u32;
} u_obss_max_duration_value;

typedef union {
    struct {
        osal_u32 x6NgwWBNCwG6KwLPLBSWwiNFS_ : 6;
        osal_u32 xCt2OmltsOzCdOR8RlImOrry_ : 9;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OsxvRORlIm_ : 5;
        osal_u32 xCt2OGwlOZ3IOGltsOozPOmo_ : 1;
        osal_u32 xCt2O4QoOyqOGltsO4myS2Omo_ : 1;
        osal_u32 xCt2OsmCOCAOGltsOsxvROoSI_ : 2;
        osal_u32 reserved_1 : 7;
    } bits;

    osal_u32 u32;
} u_slot_sifs_time;

typedef union {
    struct {
        osal_u32 x6NgwFCCCwCBNCwLBSWwFW6BwdC_ : 8;
        osal_u32 xCt2Ovt4IOsltsORlImO4mClOSs_ : 8;
        osal_u32 x6NgwFCCCw66KwCBNCwLBSW_ : 5;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2Ovt4IOsltsORlIm_ : 5;
        osal_u32 reserved_1 : 3;
    } bits;

    osal_u32 u32;
} u_sifs_time;

typedef union {
    struct {
        osal_u32 xCt2OySsOwqORlImOvSROPzx_ : 8;
        osal_u32 x6NgwjdCw_TwLBSWwidLw6QKwWh_ : 1;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_bus_rw_time_out_val;

typedef union {
    struct {
        osal_u32 x6NgwFCCCw_Puo6uwFWoGt_ : 4;
        osal_u32 xCt2Ovt4IOw8GxCGO4mxzQ_ : 4;
        osal_u32 xCt2Ovt4IOR8GxCGO4mxzQ_ : 4;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_phytxrxplcp_dly;

typedef union {
    struct {
        osal_u32 xCt2OGwlOZ3IOCCzO4mxzQ_ : 8;
        osal_u32 x6NgwCW6wVySw66GwFWoGt_ : 8;
        osal_u32 xCt2Ovt4IOw8ZR8ORSwozwvSo4ORlIm_ : 8;
        osal_u32 x6NgwFCCCw_PVLPwLd_hG_idhFwLBSW_ : 8;
    } bits;

    osal_u32 u32;
} u_phyrxtx_turnaround_time_ccadelay;

typedef union {
    struct {
        osal_u32 xCt2Ovt4IOZ3IOR8GxCGOz4WSsROPzx_ : 8;
        osal_u32 xCt2OqxOsqlRCAOqzlRORlIm_ : 8;
        osal_u32 x6Ngw_PwL_GBoW_wLBSWidLwqGo_ : 8;
        osal_u32 xCt2O4sssZvt4IOw8ZR8ORSwozwvSo4ORlIm_ : 8;
    } bits;

    osal_u32 u32;
} u_tx_rx_wait_val;

typedef union {
    struct {
        osal_u32 x6NgwFCCCw_Puo6uwFWoGtwFW6BwdC_ : 8;
        osal_u32 xCt2Ovt4IOw8GxCGO4mxzQO4mClOSs_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_phyrxplcp_delay2;

typedef union {
    struct {
        osal_u32 xCt2O4sssOw8OsRzwRO4mxzQOxvo2_ : 8;
        osal_u32 xCt2O4sssOw8OsRzwRO4mxzQOsAvwR_ : 8;
        osal_u32 xCt2Ovt4IOw8OsRzwRO4mxzQ_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_phyrxstartdelay;

typedef union {
    struct {
        osal_u32 xCt2Ow8Ovt4IOwmsGOqzlRORlIm_ : 16;
        osal_u32 x6Ngw_Pw66GwFCCCw_WCuwTGBLwLBSW_ : 8;
        osal_u32 xCt2Ow8OCCzOvt4IOwmsGOqzlRORlIm_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_ofdm_resp_wait_time;

typedef union {
    struct {
        osal_u32 x6Ngw_PwowFCCCw_WCuwTGBLwLBSW_ : 16;
        osal_u32 xCt2Ow8OsO4sssOwmsGOqzlRORlIm_ : 16;
    } bits;

    osal_u32 u32;
} u_rx_dsss_resp_wait_time;

typedef union {
    struct {
        osal_u32 xCt2OzCOymOzltsOsxvROoSI_ : 4;
        osal_u32 x6NgwG6wjKwGBNCwCoiLwhdS_ : 4;
        osal_u32 x6NgwG6wqBwGBNCwCoiLwhdS_ : 4;
        osal_u32 x6NgwG6wqiwGBNCwCoiLwhdS_ : 4;
        osal_u32 xCt2OISOymOzltsOsxvROoSI_ : 4;
        osal_u32 x6NgwSdwjKwGBNCwCoiLwhdS_ : 4;
        osal_u32 x6NgwSdwqBwGBNCwCoiLwhdS_ : 4;
        osal_u32 x6NgwSdwqiwGBNCwCoiLwhdS_ : 4;
    } bits;

    osal_u32 u32;
} u_aifsn;

typedef union {
    struct {
        osal_u32 x6NgwG6wqiw6TSBh_ : 4;
        osal_u32 xCt2OzCOPvOCqIz8_ : 4;
        osal_u32 x6NgwG6wqBw6TSBh_ : 4;
        osal_u32 xCt2OzCOPlOCqIz8_ : 4;
        osal_u32 x6NgwG6wjKw6TSBh_ : 4;
        osal_u32 xCt2OzCOydOCqIz8_ : 4;
        osal_u32 xCt2OzCOymOCqIlo_ : 4;
        osal_u32 x6NgwG6wjWw6TSGP_ : 4;
    } bits;

    osal_u32 u32;
} u_cwminmaxac;

typedef union {
    struct {
        osal_u32 xCt2OISOymOCqIlo_ : 4;
        osal_u32 x6NgwSdwjWw6TSGP_ : 4;
        osal_u32 x6NgwSdwjKw6TSBh_ : 4;
        osal_u32 xCt2OISOydOCqIz8_ : 4;
        osal_u32 x6NgwSdwqBw6TSBh_ : 4;
        osal_u32 xCt2OISOPlOCqIz8_ : 4;
        osal_u32 x6NgwSdwqiw6TSBh_ : 4;
        osal_u32 xCt2OISOPvOCqIz8_ : 4;
    } bits;

    osal_u32 u32;
} u_mu_edca_cw;

typedef union {
    struct {
        osal_u32 xCt2OyzCdvttOGwmOzO4vomOsxvROoSI_ : 5;
        osal_u32 xCt2OzCOGwysOzx2OIv4m_ : 3;
        osal_u32 xCt2OyzCdvttOwzo4vIOIv4m_ : 1;
        osal_u32 x6NgwLPw6QKwGg6wWh_ : 1;
        osal_u32 x6NgwjG6KiNNwCQdLFiThwWh_ : 1;
        osal_u32 x6NgwjG6KiNNw_WCLG_LwWh_ : 1;
        osal_u32 x6NgwWBNCwGNLW_wN6CwW___ : 1;
        osal_u32 xCt2OmltsOztRmwOGAQOmww_ : 1;
        osal_u32 xCt2OvoxQOyzsmOCCzOmo_ : 1;
        osal_u32 x6NgwLPw6QKwCW6w6QwWh_ : 1;
        osal_u32 xCt2OyzCdvttO4mxRzOPzx_ : 5;
        osal_u32 x6Ngw6TwFidjoWwLBSBhgwFWjdg_ : 1;
        osal_u32 xCt2OR8OsSsGmo4OsRvGOyzCdvttOmo_ : 1;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_wl_backoff_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwjG6KiNNwLBSWidLwqGo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_backoff_timeout_val;

typedef union {
    struct {
        osal_u32 x6NgwG6jWwu_jCwSBhwqGo_ : 8;
        osal_u32 xCt2OzCymOGwysOIz8OPzx_ : 8;
        osal_u32 xCt2OzCydOGwysOIloOPzx_ : 8;
        osal_u32 x6NgwG6jKwu_jCwSGPwqGo_ : 8;
    } bits;

    osal_u32 u32;
} u_acbe_bk_backoff_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OzCPlOGwysOIloOPzx_ : 8;
        osal_u32 x6NgwG6qBwu_jCwSGPwqGo_ : 8;
        osal_u32 xCt2OzCPvOGwysOIloOPzx_ : 8;
        osal_u32 x6NgwG6qiwu_jCwSGPwqGo_ : 8;
    } bits;

    osal_u32 u32;
} u_acvi_vo_backoff_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OzCOymOR8vGOxlIlR_ : 16;
        osal_u32 x6NgwG6wjKwLPiuwoBSBL_ : 16;
    } bits;

    osal_u32 u32;
} u_edca_txoplimit_acbkbe;

typedef union {
    struct {
        osal_u32 x6NgwG6wqBwLPiuwoBSBL_ : 16;
        osal_u32 x6NgwG6wqiwLPiuwoBSBL_ : 16;
    } bits;

    osal_u32 u32;
} u_edca_txoplimit_acvivo;

typedef union {
    struct {
        osal_u32 x6NgwQBu_BwLPiuwoBSBL_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_edca_txoplimit_hi_pri;

typedef union {
    struct {
        osal_u32 xCt2OzCOymOxltmRlImOPzx_ : 16;
        osal_u32 x6NgwG6wjKwoBNWLBSWwqGo_ : 16;
    } bits;

    osal_u32 u32;
} u_acbkbe_edca_lifetimelmt;

typedef union {
    struct {
        osal_u32 x6NgwG6wqBwoBNWLBSWwqGo_ : 16;
        osal_u32 x6NgwG6wqiwoBNWLBSWwqGo_ : 16;
    } bits;

    osal_u32 u32;
} u_acvivo_edca_lifetimelmt;

typedef union {
    struct {
        osal_u32 x6NgwQBu_BwoBNWLBSWwqGo_ : 16;
        osal_u32 x6Ngwj6woBNWLBSWwqGo_ : 16;
    } bits;

    osal_u32 u32;
} u_hi_pri_lifetime_val;

typedef union {
    struct {
        osal_u32 xCt2OCxwOISOm4CzORlImwOCoROCxw_ : 1;
        osal_u32 x6NgwWhLW_wSdwWF6Gw6hLw6o__ : 1;
        osal_u32 x6NgwRdBLwSdwWF6GwjG6KiNNw_WCLG_LwWh_ : 1;
        osal_u32 x6NgwWhLW_wSdwWF6GwjG6KiNNw_WCLG_LwWh_ : 1;
        osal_u32 xCt2OISOm4CzORyOsSCCOsmx_ : 1;
        osal_u32 x6NgwG6wqiwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ : 1;
        osal_u32 x6NgwG6wqBwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ : 1;
        osal_u32 xCt2OzCOymOISOm4CzO4lsOCAzoomxOzCCmssOmo_ : 1;
        osal_u32 x6NgwG6wjKwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ : 1;
        osal_u32 xCt2OISOm4CzOtSoCOmo_ : 1;
        osal_u32 xCt2OISOm4CzOwmsRzwROloOISOGmwlv4Omo_ : 1;
        osal_u32 xCt2OISOm4CzOzxxOzCOsqlRCAOmo_ : 1;
        osal_u32 x6NgwSdwWF6GwLBSW_wiNNCWL_ : 8;
        osal_u32 x6NgwSdwWF6Gw_uLwCWo_ : 2;
        osal_u32 xCt2OISOm4CzORyOtzlxOmoRmwOmo_ : 1;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_mu_edca_func_en;

typedef union {
    struct {
        osal_u32 x6NgwG6wqiwSdwWF6GwLBSW__ : 8;
        osal_u32 x6NgwG6wqBwSdwWF6GwLBSW__ : 8;
        osal_u32 xCt2OzCOymOISOm4CzORlImw_ : 8;
        osal_u32 x6NgwG6wjKwSdwWF6GwLBSW__ : 8;
    } bits;

    osal_u32 u32;
} u_ac_mu_edca_timer;

typedef union {
    struct {
        osal_u32 xCt2OR8vGOCtOmo4OxlIlRORlIm_ : 16;
        osal_u32 xCt2OR8vGO4voROR8OxmtRORlIm_ : 16;
    } bits;

    osal_u32 u32;
} u_txop_cf_end_limit_time;

typedef union {
    struct {
        osal_u32 xCt2OGwvROGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_prot_phy_tx_mode;

typedef union {
    struct {
        osal_u32 xCt2OGwvR3O4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_prot_datarate;

typedef union {
    struct {
        osal_u32 x6Ngwu_iL0wFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_prot1_datarate;

typedef union {
    struct {
        osal_u32 xCt2OGwvRZO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_prot2_datarate;

typedef union {
    struct {
        osal_u32 xCt2OGwvRiO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_prot3_datarate;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_phy_tx_mode;

typedef union {
    struct {
        osal_u32 xCt2OwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_resp_datarate;

typedef union {
    struct {
        osal_u32 x6Ngw6NwWhFwuQtwLPwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_cf_end_phy_tx_mode;

typedef union {
    struct {
        osal_u32 x6Ngw6NwWhFwFGLG_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_cf_end_datarate;

typedef union {
    struct {
        osal_u32 x6NgwLPwjG_wuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_tx_bar_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwLPwjG_ywFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_tx_bar_datarate;

typedef union {
    struct {
        osal_u32 xCt2OR8OyzwrO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_tx_bar1_datarate;

typedef union {
    struct {
        osal_u32 x6NgwLPwjG_VwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_tx_bar2_datarate;

typedef union {
    struct {
        osal_u32 x6NgwLPwjG_kwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_tx_bar3_datarate;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuw6LCwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_cts_phy_mode;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOCRsO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_resp_cts_datarate;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOwmtmwmoCmOwzRmOrry_ : 16;
        osal_u32 x6Ngw6QKwLPiuwNB_CLwoihghGqwWh_ : 1;
        osal_u32 xCt2OdmQtzlxOxmoOCvwwmCRlvoOmo_ : 1;
        osal_u32 xCt2OsqlRCAOR8OyCoOzxxvqOyQGzss_ : 1;
        osal_u32 x6NgwLPwGjhi_SGow_WL_twCWLwWh_ : 1;
        osal_u32 xCt2OwRsOtzlxOR8OGs4SOwmRwQOsmROmo_ : 1;
        osal_u32 xCt2OR8OzyvwROsRzRSsOIzRCAOyQGzss_ : 1;
        osal_u32 x6NgwLPwu_iLi6iowCWLwqGo_ : 2;
        osal_u32 x6NgwLPwu_iLi6iowCWLwqoF_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 x6Ngw6Go6wGGFwQoWhwTBLQwQL6_ : 1;
        osal_u32 x6Ngw6Go6wGGFwQoWhwiNNCWL_ : 4;
    } bits;

    osal_u32 u32;
} u_tx_param_val_pilot1;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOR8Om8RO4SwOvttsmR_ : 8;
        osal_u32 x6NgwLPiuwFd_GLBihwLQ_W_ : 10;
        osal_u32 xCt2Orrz8OsRzOPzGOlo4m83_ : 2;
        osal_u32 x6NgwL_Bgw_WCuw0oLNwSiFW_ : 1;
        osal_u32 xCt2OAqOR8OSGxlodO4mtzSxROPzx_ : 1;
        osal_u32 xCt2OwmsGOR8Om8ROmo_ : 1;
        osal_u32 x6Ngw_WCuwLPw0yXWPLwWh_ : 1;
        osal_u32 x6Ngw_WCuwLPwhiWPLwWh_ : 1;
        osal_u32 xCt2OwmsGOR8Om8RO4CIOsmx_ : 1;
        osal_u32 x6Ngw_WCuwLPw0yXWPLwF6SwCWo_ : 1;
        osal_u32 x6Ngw_WCuwLPwhiWPLwF6SwCWo_ : 1;
        osal_u32 x6Ngw_WCuwLPwQWwSGPuWwqGo_ : 3;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_tx_param_val_pilot2;

typedef union {
    struct {
        osal_u32 x6NgwjGwLPLBSWwGFbwqGo_ : 16;
        osal_u32 x6NgwjGwLPLBSWw6Go6wSiFW_ : 1;
        osal_u32 reserved_0 : 7;
        osal_u32 x6NgwFCCCwCQi_Lwu_WGSjoWwiNNCWL_ : 8;
    } bits;

    osal_u32 u32;
} u_ba_txtime_adj_val;

typedef union {
    struct {
        osal_u32 xCt2OGmmwOsRzOGsOCRwx_ : 8;
        osal_u32 xCt2OwmsGOGsOylROCRwx_ : 8;
        osal_u32 x6Ngwdhu_iLW6LWFwFB_W6LwFGLGwNoLwWh_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_ps_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OGwlOZ3IOCCzOCAdORlImvSROmo_ : 1;
        osal_u32 x6NgwCW6wVySw66Gw6QKwLBSWidLwWh_ : 1;
        osal_u32 reserved_0 : 14;
        osal_u32 xCt2OCCzORlImvSROPzx_ : 16;
    } bits;

    osal_u32 u32;
} u_cca_time_out_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwFGLGwTBhwCQBNLwoBSBLwqGo_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OyzwOqloOsAltROxlIlROPzx_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_bitmap_shift_limit;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4msCORmsRO3qvw4Osmx_ : 4;
        osal_u32 x6Ngw_PwFWC6wLWCLw0Ti_FwCWo_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2Ow8O4msCORmsROZqvw4Osmx_ : 3;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2Ow8O4msCORmsROiqvw4Osmx_ : 3;
        osal_u32 reserved_2 : 1;
        osal_u32 x6Ngw_PwFWC6wLWCLwDTi_FwCWo_ : 3;
        osal_u32 reserved_3 : 13;
    } bits;

    osal_u32 u32;
} u_rx_desc_test_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OGwlOZ3IOCCzOyQGzssOmo_ : 1;
        osal_u32 x6Ngwu_BwVySw66GwjtuGCCwqGo_ : 1;
        osal_u32 x6NgwCW6wVySw66GwjtuGCCwWh_ : 1;
        osal_u32 xCt2OsmCOZ3IOCCzOyQGzssOPzx_ : 1;
        osal_u32 x6NgwRiCw6NG6Kw6NuioowjGwiuW_wjtuGCCwWh_ : 1;
        osal_u32 x6NgwRiCw6NuioowjGwiuW_wjtuGCCwWh_ : 1;
        osal_u32 xCt2O6vsOoSxxOyzOvGmwOyQGzssOmo_ : 1;
        osal_u32 xCt2O6vs4zRzOCtzCdOCtGvxxOyzOvGmwOyQGzssOmo_ : 1;
        osal_u32 xCt2O6vs4zRzOCtGvxxOyzOvGmwOyQGzssOmo_ : 1;
        osal_u32 xCt2O6vs4zRzOCtzCdOyzOvGmwOyQGzssOmo_ : 1;
        osal_u32 x6NgwhGqwjtuGCCwWh_ : 1;
        osal_u32 xCt2OozPOyQGzssOPzx_ : 1;
        osal_u32 x6NgwLPwW__wN6CwWh_ : 1;
        osal_u32 x6NgwuQtw_PwCGSuoWwFWoBwW__wWh_ : 1;
        osal_u32 x6NgwuQtw_PwCGSuoWwN6CwW__wWh_ : 1;
        osal_u32 x6Ngwj6hwLBSwQGqWwN_SwBhL_wWh_ : 1;
        osal_u32 xCt2O2moOw8OtwIOmo_ : 1;
        osal_u32 xCt2OyCoORlIOtCsOCAdOmo_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_bypass_sample_control;

typedef union {
    struct {
        osal_u32 x6Ngw6CBwdC_ywN_SwLtuW_ : 3;
        osal_u32 xCt2OCslOSswrOtwIORQGm_ : 3;
        osal_u32 x6Ngw6CBwdC_VwN_SwLtuW_ : 3;
        osal_u32 x6Ngw6CBwdC_kwN_SwLtuW_ : 3;
        osal_u32 cfg_csi_en : 4;
        osal_u32 xCt2OCslOz44wOIzRCAOyQGzss_ : 4;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_csi_en;

typedef union {
    struct {
        osal_u32 x6Ngw6CBwdC_ywN_SwCdjLtuW_ : 4;
        osal_u32 xCt2OCslOSswrOtwIOsSyRQGm_ : 4;
        osal_u32 x6Ngw6CBwdC_VwN_SwCdjLtuW_ : 4;
        osal_u32 x6Ngw6CBwdC_kwN_SwCdjLtuW_ : 4;
        osal_u32 x6Ngw6CBwN_SwCdjLtuWwSGL6QwWh_ : 4;
        osal_u32 xCt2OCslOSswOIzRCAORzOwzOsmx_ : 4;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_csi_user_frm_subtype;

typedef union {
    struct {
        osal_u32 xCt2OCslOSsw3OGG4SOtvwIzR_ : 6;
        osal_u32 x6Ngw6CBwdC_0wuuFdwNi_SGL_ : 6;
        osal_u32 xCt2OCslOSswZOGG4SOtvwIzR_ : 6;
        osal_u32 xCt2OCslOSswiOGG4SOtvwIzR_ : 6;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_csi_ppdu_format;

typedef union {
    struct {
        osal_u32 x6Ngw6CBwdC_0wCGSuoWwuW_BiF_ : 12;
        osal_u32 xCt2OCslOSsw3OszIGxmOGmwlv4_ : 12;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_csi_usr01_sample_period;

typedef union {
    struct {
        osal_u32 xCt2OCslOSswiOszIGxmOGmwlv4_ : 12;
        osal_u32 xCt2OCslOSswZOszIGxmOGmwlv4_ : 12;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_csi_usr23_sample_period;

typedef union {
    struct {
        osal_u32 xCt2OCslOwGROCoROCxw_ : 5;
        osal_u32 xCt2OCslOsqOySttOlo4m8_ : 3;
        osal_u32 xCt2OCslORlImvSRORAw_ : 5;
        osal_u32 x6Ngw6CBwjdNNwhdS_ : 3;
        osal_u32 x6Ngw6CBwjdNNwCLWu_ : 10;
        osal_u32 xCt2OCslOtCsOmwwO4wvGOmo_ : 1;
        osal_u32 xCt2OCslO4lwmCROwmsGOCRwxOtxROmo_ : 1;
        osal_u32 xCt2OCslOAmO2wvSGlo2_ : 1;
        osal_u32 x6Ngw6CBwhihwQLwjTwSBCSGL6Qw6WhLW_wN_WR_ : 1;
        osal_u32 xCt2OCslOovoOAROyqOIlsIzRCAOCmoRmwOtwm6OyQGzssOmo_ : 1;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_csi_buff_set;

typedef union {
    struct {
        osal_u32 xCt2OCslOySttOyzsmOz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_csi_buff_base_addr;

typedef union {
    struct {
        osal_u32 x6NgwoiTwoGLWh6twjG6KiNNwWh_ : 1;
        osal_u32 xCt2OxvqOxzRmoCQOmo_ : 1;
        osal_u32 xCt2OxvqOxzRmoCQOyzCdvttOmoOztOR8mo4_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_low_latency_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwLjwLPoBhKwLW_SBhGLWwhdS_ : 6;
        osal_u32 xCt2ORyOR8xlodORmwIlozRmOmo_ : 1;
        osal_u32 xCt2ORyOm4CzOAqOzIG4SOmo_ : 1;
        osal_u32 x6Ngw6dLwLPoBhKwCdjwSuFdwoWh_ : 14;
        osal_u32 xCt2OCSROR8xlodOIv4m_ : 1;
        osal_u32 xCt2Om4CzORwl2Oyzsm4OR8OAqOyzwOmo_ : 1;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_tb_txlink_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwCTwjtuGCCwWF6GwCBNCwLBSWwFW6B_ : 8;
        osal_u32 xCt2Om4CzOsltsOsqOyQGzssOmo_ : 1;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_edca_sifs_sw_bypass;

typedef union {
    struct {
        osal_u32 x6NgwFidjoWwSG6wGFF_wQ_ : 16;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2O4vSyxmOIzCOmo_ : 1;
        osal_u32 reserved_1 : 14;
    } bits;

    osal_u32 u32;
} u_double_mac_en1;

typedef union {
    struct {
        osal_u32 xCt2OR8O6SmSmOzxOmIGRQORAwm_ : 8;
        osal_u32 xCt2OR8O6SmSmOzxOmIGRQOCAdOoz44wOyQGzss_ : 1;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_tx_q_empty_param;

typedef union {
    struct {
        osal_u32 x6NgwFBGgwL_BgwSiFWwSdP_ : 5;
        osal_u32 xCt2OGdROySttmwOsmx_ : 3;
        osal_u32 xCt2O4lz2ORstOsmx_ : 2;
        osal_u32 x6Ngw_PwFBGgwWh_ : 1;
        osal_u32 x6NgwLPwFBGgwWh_ : 1;
        osal_u32 xCt2O4lz2OCyyOmo_ : 1;
        osal_u32 xCt2O4lz2Ol4xmOCoROozPOyQGzssOmo_ : 1;
        osal_u32 xCt2O4lz2ORstOSolROsmx_ : 2;
        osal_u32 x6Ngw_PwN_SwLtuWwSGL6QwWh_ : 1;
        osal_u32 x6NgwLPwN_SwLtuWwSGL6QwWh_ : 1;
        osal_u32 xCt2Ow8Oz44wZOIzRCAOmo_ : 1;
        osal_u32 x6Ngw_PwGFF_0wSGL6QwWh_ : 1;
        osal_u32 xCt2OR8Oz44wZOIzRCAOmo_ : 1;
        osal_u32 x6NgwLPwGFF_0wSGL6QwWh_ : 1;
        osal_u32 x6NgwhiwidLudLwWh_ : 1;
        osal_u32 x6NgwhiwidLudLwCTwL_Bg_ : 1;
        osal_u32 x6NgwFBGgwBFoWw6hLwNCSwjtuGCCwWh_ : 1;
        osal_u32 xCt2O4lz2OIzCOzGyOIv4mOmo_ : 1;
        osal_u32 xCt2O4lz2OwsslOIv4mOmo_ : 1;
        osal_u32 x6NgwFBGgwqGowCWo_ : 1;
        osal_u32 xCt2O4lz2nOov4mOsmx_ : 4;
    } bits;

    osal_u32 u32;
} u_diag_control1;

typedef union {
    struct {
        osal_u32 xCt2O4lz2ORwl2OIv4mOwsslORAw_ : 8;
        osal_u32 x6NgwFBGgwL_BgwSiFWwuuFdwNi_SGL_ : 3;
        osal_u32 x6NgwFBGgwL_BgwSiFWwjTwWh_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWwjT_ : 3;
        osal_u32 xCt2O4lz2ORwl2OIv4mOw8OAmOARCOmo_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWw_Pwu_iLi6iowqW_CBihwWh_ : 1;
        osal_u32 xCt2O4lz2ORwl2OIv4mOw8OzCdGvxlCQOmo_ : 1;
        osal_u32 xCt2O4lz2ORwl2OIv4mOw8ORwl2ORQGmOmo_ : 1;
        osal_u32 xCt2O4lz2ORwl2OIv4mOtwIORQGmOmo_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWwFotw6hL_ : 8;
        osal_u32 x6NgwFBGgwL_BgwSiFWwuuFdwNi_SGLwWh_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWw_PwGSCFd_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWw_PwGSCFdwWh_ : 1;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_diag_control2;

typedef union {
    struct {
        osal_u32 xCt2O4lz2ORwl2OIv4mOSom8GmCRm4OGz44w3_ : 11;
        osal_u32 reserved_0 : 3;
        osal_u32 x6NgwFBGgwL_BgwSiFWwdhWPuW6LWFwuGFF_0_ : 11;
        osal_u32 reserved_1 : 3;
        osal_u32 xCt2O4lz2ORwl2OIv4mOz44wrOmo_ : 1;
        osal_u32 xCt2O4lz2ORwl2OIv4mOw8Oz44wZOmo_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWwuS_ : 1;
        osal_u32 x6NgwFBGgwL_BgwSiFWwuSwWh_ : 1;
    } bits;

    osal_u32 u32;
} u_diag_control3;

typedef union {
    struct {
        osal_u32 x6NgwLjwSihBLi_wWh_ : 1;
        osal_u32 x6NgwLjwSihBLi_wNB_CLwdCW_wBhNiwSGL6QwWh_ : 1;
        osal_u32 x6NgwLjwSihBLi_wCLGLB6w6o__ : 1;
        osal_u32 x6NgwLjwSihBLi_wuG_GSw6QKwjtuGCC_ : 1;
        osal_u32 xCt2ORyOIvolRvwOzl4OPzx_ : 12;
        osal_u32 x6NgwLPwL_BgwWPuW6Lw_WCuwLtuW_ : 6;
        osal_u32 x6NgwLPwL_BgwWPuW6Lw_WCuwWh_ : 1;
        osal_u32 x6NgwLjwSihBLi_wN6Cw6QKwjtuGCC_ : 1;
        osal_u32 xCt2ORyOIvolRvwOotwGOwSOzxxvCzRlvo_ : 7;
        osal_u32 reserved_0 : 1;
    } bits;

    osal_u32 u32;
} u_tb_monitor_en;

typedef union {
    struct {
        osal_u32 x6Ngw_uLwLPwFGLGwN_SLtuWwCWo_ : 2;
        osal_u32 x6Ngw_uLwLPwFGLGwqGuwCWo_ : 2;
        osal_u32 xCt2OwGROR8O4zRzOsmx_ : 4;
        osal_u32 xCt2OwGROR8O4zRzOGwvRvCvxOsmx_ : 3;
        osal_u32 xCt2OwGROR8O4zRzOPzGOIv4m_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_cfg_tb_monitor_sel;

#endif
