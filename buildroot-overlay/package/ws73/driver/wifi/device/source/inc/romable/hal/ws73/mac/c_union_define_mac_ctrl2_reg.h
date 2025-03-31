/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_CTRL2_REG_H__
#define __C_UNION_DEFINE_MAC_CTRL2_REG_H__

typedef union {
    struct {
        osal_u32 x6NgwqGuywjCCBFwQ_ : 16;
        osal_u32 x6NgwqGuywSG6GFF_wQ_ : 16;
    } bits;

    osal_u32 u32;
} u_vap0_macaddr_bssid_h;

typedef union {
    struct {
        osal_u32 xCt2OPzG3Ow8OyCoORlImvSROCAdOmo_ : 1;
        osal_u32 xCt2OPzG3OGsOIv4mOmo_ : 1;
        osal_u32 x6NgwqGuyw_Pwu_jw_CuwCth6wWh_ : 1;
        osal_u32 x6NgwqGuyw_Pwj6hwCth6wWh_ : 1;
        osal_u32 x6NgwqGuywoBCLWhwBhL_wWh_ : 1;
        osal_u32 xCt2OPzG3O4RlIOloRwOmo_ : 1;
        osal_u32 x6NgwqGuywj6hwuW_BiFwCth6wFBC_ : 1;
        osal_u32 x6NgwqGuywLPwj6hwCdCuWhF_ : 1;
        osal_u32 xCt2OPzG3OyCoOtzlxOR8OyCO6Omo_ : 1;
        osal_u32 x6NgwqGuyw_PwNBoCwFBC6wCth6wWh_ : 1;
        osal_u32 xCt2OPzG3Ow8ORlIOsQoCOmo_ : 1;
        osal_u32 x6NgwqGuywCth6wj6hwFB_W6L_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap0_tsf_ctrl1;

typedef union {
    struct {
        osal_u32 xCt2OPzG3OvttsmRORyRROPzx_ : 16;
        osal_u32 x6NgwqGuywoBCLWhwBhLW_qGo_ : 8;
        osal_u32 x6NgwqGuywFLBSwuW_BiF_ : 8;
    } bits;

    osal_u32 u32;
} u_vap0_tsf_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwqGuywj6hwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap0_bcn_pkt_ptr;

typedef union {
    struct {
        osal_u32 xCt2OPzG3OyCoOxmo_ : 16;
        osal_u32 xCt2OPzG3OyCoORlIOGvs_ : 16;
    } bits;

    osal_u32 u32;
} u_vap0_bcn_tx_length;

typedef union {
    struct {
        osal_u32 xCt2OPzG3OyCoOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap0_bcn_phy_tx_mode;

typedef union {
    struct {
        osal_u32 x6NgwqGuywj6hwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_vap0_bcn_tx_data_rate;

typedef union {
    struct {
        osal_u32 xCt2OPzG3OGzwRlzxOzl4_ : 9;
        osal_u32 xCt2OPzG3OyssOCvxvw_ : 6;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwqGuywGBFwqGo_ : 12;
        osal_u32 xCt2OPzG3OGzwRlzxOyssOCvxvw_ : 4;
    } bits;

    osal_u32 u32;
} u_vap0_aid_val;

typedef union {
    struct {
        osal_u32 xCt2OPzGrOyssl4OA_ : 16;
        osal_u32 xCt2OPzGrOIzCz44wOA_ : 16;
    } bits;

    osal_u32 u32;
} u_vap1_macaddr_bssid_h;

typedef union {
    struct {
        osal_u32 x6NgwqGu0w_Pwj6hwLBSWidLw6QKwWh_ : 1;
        osal_u32 x6NgwqGu0wuCwSiFWwWh_ : 1;
        osal_u32 xCt2OPzGrOw8OGwyOwsGOsQoCOmo_ : 1;
        osal_u32 xCt2OPzGrOw8OyCoOsQoCOmo_ : 1;
        osal_u32 xCt2OPzGrOxlsRmoOloRwOmo_ : 1;
        osal_u32 x6NgwqGu0wFLBSwBhL_wWh_ : 1;
        osal_u32 xCt2OPzGrOyCoOGmwlv4OsQoCO4ls_ : 1;
        osal_u32 xCt2OPzGrOR8OyCoOsSsGmo4_ : 1;
        osal_u32 x6NgwqGu0wj6hwNGBowLPwj6wRwWh_ : 1;
        osal_u32 xCt2OPzGrOw8OtlxsO4lsCOsQoCOmo_ : 1;
        osal_u32 x6NgwqGu0w_PwLBSwCth6wWh_ : 1;
        osal_u32 xCt2OPzGrOsQoCOyCoO4lwmCR_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap1_tsf_ctrl1;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wiNNCWLwLjLLwqGo_ : 16;
        osal_u32 xCt2OPzGrOxlsRmoOloRmwPzx_ : 8;
        osal_u32 xCt2OPzGrO4RlIOGmwlv4_ : 8;
    } bits;

    osal_u32 u32;
} u_vap1_tsf_ctrl2;

typedef union {
    struct {
        osal_u32 xCt2OPzGrOyCoOz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap1_bcn_pkt_ptr;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wj6hwoWh_ : 16;
        osal_u32 x6NgwqGu0wj6hwLBSwuiC_ : 16;
    } bits;

    osal_u32 u32;
} u_vap1_bcn_tx_length;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wj6hwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap1_bcn_phy_tx_mode;

typedef union {
    struct {
        osal_u32 xCt2OPzGrOyCoO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_vap1_bcn_tx_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwqGu0wuG_LBGowGBF_ : 9;
        osal_u32 x6NgwqGu0wjCCw6ioi__ : 6;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OPzGrOzl4OPzx_ : 12;
        osal_u32 x6NgwqGu0wuG_LBGowjCCw6ioi__ : 4;
    } bits;

    osal_u32 u32;
} u_vap1_aid_val;

typedef union {
    struct {
        osal_u32 x6NgwqGuwLCNwGFbdCLwiNNCWLwqGo_ : 16;
        osal_u32 xCt2ORstOGSxsmOmo_ : 1;
        osal_u32 xCt2ORstOGSxsmOsmx_ : 2;
        osal_u32 xCt2ORstOqvwdORlImwOSsOmo_ : 1;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap_tsf_adjust_offset;

typedef union {
    struct {
        osal_u32 x6NgwqGuwLCNwGFbdCLwWh_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 x6NgwC6_wLCNwqGuwBhFWP_ : 2;
        osal_u32 x6NgwFCLwLCNwqGuwBhFWP_ : 2;
        osal_u32 reserved_1 : 24;
    } bits;

    osal_u32 u32;
} u_vap_tsf_adjust_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwLCNwudoCWwLBSWCLGSuwSGCK_ : 16;
        osal_u32 xCt2ORstOGSxsmORlImsRzIG_ : 16;
    } bits;

    osal_u32 u32;
} u_tsf_pulse_timestamp;

typedef union {
    struct {
        osal_u32 x6NgwLPwj6hwLBSWidLwqGo_ : 8;
        osal_u32 x6NgwLPwjWG6ihwSBCCwSGPwhdS_ : 8;
        osal_u32 x6Ngw_Pwj6hwLBSWidLwqGo_ : 8;
        osal_u32 xCt2OR8OyCoOARCOmo_ : 1;
        osal_u32 reserved_0 : 7;
    } bits;

    osal_u32 u32;
} u_beacon_timeout_val;

typedef union {
    struct {
        osal_u32 x6NgwL_GhCwjCCBFwQ_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_vap0_trans_bssid_h;

typedef union {
    struct {
        osal_u32 xCt2ORwl2ORwsOGz44lo2ORlImOmwwOCxw_ : 1;
        osal_u32 xCt2ORwl2O4lwmCROtwzImOzl4OCAmCdOmo_ : 1;
        osal_u32 xCt2Ow8ORwl2OloRwOmo_ : 1;
        osal_u32 xCt2OARGOGvxlCQOCAmCdOmo_ : 1;
        osal_u32 x6Ngwhi_SGowuioB6tw6QW6KwWh_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwoWhw6QKwWh_ : 1;
        osal_u32 xCt2ORwl2OwmsGOSGxlodO4sCwOmo_ : 1;
        osal_u32 xCt2ORwl2OtlwsROSsmwOlotvOIzRCAOmo_ : 1;
        osal_u32 xCt2ORwl2OGz44lo2ORlImOCAmCdORAw_ : 8;
        osal_u32 xCt2Ow8ORwl2O2lOxRtORQGm_ : 4;
        osal_u32 xCt2ORwl2OovoO4lwmCROtwzImOzl4OCAmCdOmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOloRwOz44OsSCCmss_ : 1;
        osal_u32 xCt2ORwl2OwmsGOCsOCAdOmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOz44OGs4SOCoROmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOovROsSGGvwROCAd_ : 1;
        osal_u32 xCt2ORwl2OwmsGOvyssOoyOwSOsSGGvwR_ : 1;
        osal_u32 xCt2Ow8OGs4SOsRzRSsOCxwOyQGzss_ : 1;
        osal_u32 x6NgwL_BgwL_Cw_WCuwduQwBhCW_LwWh_ : 1;
        osal_u32 xCt2ORwl2ORwsOwmsGOCRwxl4OwmsmwPm4OPzx_ : 4;
    } bits;

    osal_u32 u32;
} u_trig_resp_common1;

typedef union {
    struct {
        osal_u32 x6NgwL_Bgw_WCuwUy_dwLPwFBCGjoW_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwjTw_dwSGL6Qw6QKwWh_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwjTwLQ_WoWqWo_ : 2;
        osal_u32 xCt2ORwl2OwmsGOICsORAwmxmPmx_ : 4;
        osal_u32 xCt2ORwl2OwmsGOossORAwmxmPmx_ : 3;
        osal_u32 x6NgwL_Bgw_WCuwF6SwLQ_WoWqWo_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwFiuuoW_wLQ_WoWqWo_ : 1;
        osal_u32 xCt2Om4CzORwl2OloRwOmo_ : 1;
        osal_u32 x6Ngw_WCuwL_CwBhL_wWh_ : 1;
        osal_u32 xCt2OwmsGORwl2OloRwOmo_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwuG_GSwCWLwWh_ : 12;
        osal_u32 xCt2OwmsGOzIG4SOIIssOPzx_ : 3;
        osal_u32 xCt2OovORwl2OqlRAOARGOwmsGOyQGzss_ : 1;
    } bits;

    osal_u32 u32;
} u_trig_resp_common2;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwRiChdoowCWRhdSwqGo_ : 12;
        osal_u32 x6Ngw_WCuwRiChdoowCWRhdSwSiFW_ : 1;
        osal_u32 xCt2OR8O6vsoSxxOzCdGvxlCQOsmROmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOwS3OovOCAdOsmCCAOCCzOmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOCzxCOtCsmwwOwmsmROmo_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwuQtwuQGCWwBh6__ : 15;
        osal_u32 x6NgwL_Bgw_WCuwuQtwuQGCWwBh6_wWh_ : 1;
    } bits;

    osal_u32 u32;
} u_trig_resp_common3;

typedef union {
    struct {
        osal_u32 x6NgwSdw_LCw_WCuwuQtwSiFWwWh_ : 1;
        osal_u32 xCt2OotwGOwmsGOGAQOIv4mOmo_ : 1;
        osal_u32 xCt2OyzslCOwmsGOGAQOIv4mOmo_ : 2;
        osal_u32 xCt2Oy6wGOwmsGOGAQOIv4mOmo_ : 2;
        osal_u32 xCt2OISOyzwOwmsGOGAQOIv4mOmo_ : 2;
        osal_u32 xCt2OytwGOwmsGOGAQOIv4mOmo_ : 2;
        osal_u32 x6NgwjC_uw_WCuwuQtwSiFWwWh_ : 2;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_trig_resp_txvctr;

typedef union {
    struct {
        osal_u32 xCt2OotwGORwl2OwmsGOyQGzss_ : 1;
        osal_u32 xCt2Oy6wGORwl2OwmsGOyQGzss_ : 1;
        osal_u32 x6NgwjC_uwL_Bgw_WCuwjtuGCC_ : 1;
        osal_u32 x6NgwSdw_LCwL_Bgw_WCuwjtuGCC_ : 1;
        osal_u32 xCt2OISOyzwORwl2OwmsGOyQGzss_ : 1;
        osal_u32 xCt2OytwGORwl2OwmsGOyQGzss_ : 1;
        osal_u32 xCt2OyzslCORwl2OwmsGOyQGzss_ : 1;
        osal_u32 x6NgwL_Cw_WCuwjtuGCC_ : 1;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_trig_resp_bypass;

typedef union {
    struct {
        osal_u32 x6NgwjGCB6w_WCuwGSuFdwWh_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwjR_uwRiCw_WCuwjGwWh_ : 1;
        osal_u32 xCt2OyswGO6vsOwmsGOyzOmo_ : 1;
        osal_u32 x6NgwjGCB6wRiCw_WCuwjGwWh_ : 1;
        osal_u32 x6NgwL_CwhiRiCw_WCuwjGwWh_ : 1;
        osal_u32 xCt2ORwsO6vsOwmsGOyzOmo_ : 1;
        osal_u32 x6NgwjR_uw_WCuwRiCwhdoowWh_ : 1;
        osal_u32 xCt2OyswGOwmsGO6vsOoSxxOmo_ : 1;
        osal_u32 x6NgwjGCB6w_WCuwRiCwhdoowWh_ : 1;
        osal_u32 xCt2ORwsOwmsGO6vsOoSxxOmo_ : 1;
        osal_u32 x6NgwL_CwSgSLw_WCuwRiCwhdoowWh_ : 1;
        osal_u32 x6NgwL_CwRiCw_WCuwRiCwhdoowWh_ : 1;
        osal_u32 x6NgwL_Cwhiw_WCuwWh_ : 1;
        osal_u32 xCt2OyzslCOovOwmsGOmo_ : 1;
        osal_u32 xCt2Oslo2xmO6vsOwmsGOyzOmo_ : 1;
        osal_u32 x6Ngwj6wRiCwjBLSGuwCWG_6QwWh_ : 1;
        osal_u32 xCt2OICO6vsOylRIzGOsmzwCAOmo_ : 1;
        osal_u32 xCt2OCAdOw8OICOIyzOmo_ : 1;
        osal_u32 x6NgwCwSuFdw6QKw_PwF6wSjGwWh_ : 1;
        osal_u32 reserved_1 : 11;
    } bits;

    osal_u32 u32;
} u_trig_resp_qosnull_ba_en;

typedef union {
    struct {
        osal_u32 x6NgwL_Bgw_WCuwCuGLBGow_WdCW_ : 16;
        osal_u32 xCt2ORwl2OwmsGOAmOsl2zOwmsmwPm4_ : 9;
        osal_u32 x6NgwL_Bgw_WCuwuWwFd__ : 2;
        osal_u32 reserved_0 : 5;
    } bits;

    osal_u32 u32;
} u_trig_resp_pedur_sigareserv_spreuse;

typedef union {
    struct {
        osal_u32 x6NgwSdwjG_w_WCuwFGLGw_GLWwWh_ : 10;
        osal_u32 x6NgwjN_uw_WCuwFGLGw_GLWwWh_ : 10;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_trig_resp_datarate_en1;

typedef union {
    struct {
        osal_u32 x6NgwjR_uw_WCuwFGLGw_GLWwWh_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OyswGOwmsGO4zRzOwzRmOmo_ : 10;
        osal_u32 x6NgwjGCB6w_WCuwFGLGw_GLWwWh_ : 10;
    } bits;

    osal_u32 u32;
} u_trig_resp_datarate_en2;

typedef union {
    struct {
        osal_u32 xCt2ORwl2OwmsGORwl24SwOxmssOIv4m_ : 1;
        osal_u32 xCt2ORwl2OwmsGO4SwOm4CzOIv4m_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwSSCNwqoF_ : 1;
        osal_u32 xCt2ORwl2OwmsGOzCOxGwlOmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOzCOAGwlOmo_ : 1;
        osal_u32 xCt2ORwl2OwmsGOzCOCAORAwm_ : 3;
        osal_u32 xCt2ORwl2OwmsGOCAdORl4Omo_ : 1;
        osal_u32 x6NgwjGCB6w_WCuwFWC6wBhCW_LwQL6wWh_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwG6wQBuwWh_ : 1;
        osal_u32 xCt2ORwl2OwmsGOzCOvw4mw_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwLPwRiChdoowWhV_ : 1;
        osal_u32 xCt2ORwl2OwmsGOR8O6vsoSxxOmor_ : 1;
        osal_u32 x6NgwL_Bgw_WCuwLPwRiChdoowWhk_ : 1;
        osal_u32 xCt2OyzslCORwl2OwmsGOIv4m_ : 1;
        osal_u32 x6NgwL_Bgw_WCuw6L_oN_Sw6QKwSiFW_ : 2;
        osal_u32 x6NgwjGCB6w_WCuwFWC6woWhw6QKwWh_ : 1;
        osal_u32 x6NgwjGCB6w_WCuwDRiChdoowWh_ : 1;
        osal_u32 x6NgwjGCB6w_WCuwi_BgBhGowWh0_ : 1;
        osal_u32 xCt2OyzslCOwmsGOvwl2lozxOmoZ_ : 1;
        osal_u32 xCt2OyzslCOwmsGOvwl2lozxOmoi_ : 1;
        osal_u32 xCt2OAmOyzslCRwl2OwmsGOlosmwROARCOmo_ : 1;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_trig_resp_basic;

typedef union {
    struct {
        osal_u32 x6NgwL_Bgw_WCuwFd_wGFbwqGo_ : 16;
        osal_u32 xCt2OyzslCO6SmSmOslpm_ : 8;
        osal_u32 x6NgwjGCB6wLBF_ : 4;
        osal_u32 reserved_0 : 4;
    } bits;

    osal_u32 u32;
} u_basic_trig_htc_qos;

typedef union {
    struct {
        osal_u32 xCt2OyzslCOwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_basic_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwjGCB6w_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_basic_resp_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwL_CwRdWdWwCB1W_ : 8;
        osal_u32 xCt2ORwsORl4_ : 4;
        osal_u32 xCt2ORwsOwmsGOyqORAwmxmPmx_ : 2;
        osal_u32 x6NgwL_Cw_WCuwS6CwLQ_WoWqWo_ : 2;
        osal_u32 xCt2ORwsOwmsGO4CIORAwmxmPmx_ : 1;
        osal_u32 x6NgwL_Cw_WCuwhiLwCduui_Lw6QK_ : 1;
        osal_u32 x6NgwL_Cw_WCuwijCCwhjw_dwCduui_L_ : 1;
        osal_u32 xCt2ORwsOwmsGOxmoOCAdOmo_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2ORwsOwmsGOzGO4mtzSxROGmO4Sw_ : 2;
        osal_u32 x6NgwQWwL_Cw_WCuwBhCW_LwQL6wWh_ : 1;
        osal_u32 x6NgwL_Cw_WCuwBhL_wGFFwCd66WCC_ : 1;
        osal_u32 xCt2Ow8OAmORwsOloRwOmo_ : 1;
        osal_u32 x6Ngw_PwQWwL_CwWh_ : 1;
        osal_u32 reserved_1 : 3;
    } bits;

    osal_u32 u32;
} u_trig_resp_trs1;

typedef union {
    struct {
        osal_u32 xCt2ORwsOwmsGOsGzRlzxOwmSsm_ : 16;
        osal_u32 x6NgwL_Cw_WCuwQWwCBgGw_WCW_qWF_ : 9;
        osal_u32 reserved_0 : 7;
    } bits;

    osal_u32 u32;
} u_trig_resp_trs2;

typedef union {
    struct {
        osal_u32 x6NgwLjwCidhFBhgwjN_uwLBSWidLwqGo_ : 10;
        osal_u32 xCt2ORyOsvSo4lo2OqzlROytwGORlImvSROCAdOmo_ : 1;
        osal_u32 xCt2OISOsvSo4lo2OwmsGO4myS2Omo_ : 1;
        osal_u32 xCt2OISOsvSo4lo2OCAdORzOmo_ : 1;
        osal_u32 x6Ngw_PwCidhFBhgwuWW_wBhFWPwLBSBhgwFWjdgwWh_ : 1;
        osal_u32 xCt2OPAROISOsvSo4lo2OqzlROywGORlImvSROCAdOmo_ : 1;
        osal_u32 xCt2OAmOytwGOwmsGOlosmwROARCOmo_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_trig_resp_bfrp;

typedef union {
    struct {
        osal_u32 xCt2OytwGOwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_bfrp_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwjN_uw_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_bfrp_resp_phy_mode;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOISwRsOovROsSGGvwROCAdOxmPmx_ : 2;
        osal_u32 xCt2OwmsGOISwRsOvyssOoyOwSOCAdOmo_ : 1;
        osal_u32 xCt2OISOwRsOwmsGOCCzOCAdOmo_ : 1;
        osal_u32 xCt2OISOwRsOwmsGOyqOCAdOmo_ : 1;
        osal_u32 x6NgwSdw_LCw_WCuwjTwSiFW_ : 1;
        osal_u32 x6NgwSdw_LCwdowjTw6QKwWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwSdw_LCwu_BVySwuiC_ : 4;
        osal_u32 reserved_1 : 20;
    } bits;

    osal_u32 u32;
} u_trig_resp_murts;

typedef union {
    struct {
        osal_u32 xCt2OISOwRsOwmsGOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_mu_rts_resp_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwSdw_LCw_WCuwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_mu_rts_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwSdoLBwCLGwG6KwGBF_ : 12;
        osal_u32 x6NgwLBFwBhNiw_WCW_qWF_ : 4;
        osal_u32 x6NgwSdwCChwLBFwhdS_ : 4;
        osal_u32 x6NgwBSSFtwjG_w6QW6KwWh_ : 1;
        osal_u32 xCt2ORyOCoROyzwOz44Omo_ : 1;
        osal_u32 x6Ngw_WCuwQWwCBhgoWwLBFwjGwLtuW_ : 1;
        osal_u32 x6NgwCwSuFdwhihwGSuFdw_Pw6jGwWh_ : 1;
        osal_u32 x6NgwCwSuFdwBhwGSuFdw_PwG6KwWh_ : 1;
        osal_u32 x6NgwQLwCBhgoWwSuFdwBhGSuFdwNFwWh_ : 1;
        osal_u32 xCt2OISOyzwOyzwOzywOCAdOmo_ : 1;
        osal_u32 xCt2OrsROR8OwmRwQOsmROmo_ : 1;
        osal_u32 xCt2OyzwOzCdOGvxlCQOwmsGOyQGzss_ : 1;
        osal_u32 xCt2OR8Oslo2xmOIG4SOloOsmsslvoOsmROmvtOmo_ : 1;
        osal_u32 x6Ngw_PwFB_W6LwSCLGwjGwGBFwdhSGL6QwWh_ : 1;
        osal_u32 x6Ngw_PwSjGw6QKwLBFwWh_ : 1;
    } bits;

    osal_u32 u32;
} u_trig_resp_mubar_he_ba_ctrl;

typedef union {
    struct {
        osal_u32 xCt2OISOyzwOwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_mu_bar_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwSdwjG_w_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_mu_bar_resp_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwhN_uw_WCuwCuGLBGow_WdCW_ : 16;
        osal_u32 x6NgwhN_uwNWWFjG6KwCLGLdC_ : 1;
        osal_u32 xCt2OotwGOtmm4yzCdORQGmO4ls_ : 1;
        osal_u32 xCt2OotwGOzOtzCRvw_ : 2;
        osal_u32 xCt2OotwGO4vGGxmw_ : 1;
        osal_u32 xCt2OotwGOx4GCOm8RwzOsQI_ : 1;
        osal_u32 xCt2OotwGOGmO4lszIyl2SlRQ_ : 1;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_trig_resp_ndrp1;

typedef union {
    struct {
        osal_u32 xCt2OotwGOwmsGOwSOzxxvCzRlvo_ : 16;
        osal_u32 x6NgwhN_uw_WCuwFGLGw_GLWwWh_ : 6;
        osal_u32 reserved_0 : 10;
    } bits;

    osal_u32 u32;
} u_trig_resp_ndrp2;

typedef union {
    struct {
        osal_u32 xCt2OotwGOwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_nfrp_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwhN_uw_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_nfrp_resp_phy_mode;

typedef union {
    struct {
        osal_u32 xCt2Oy6wO6SmSmOslpm_ : 8;
        osal_u32 x6NgwjR_wLBF_ : 4;
        osal_u32 xCt2OyswGOwmsGO6SmSmOslpmOzCOmo_ : 1;
        osal_u32 xCt2OyswGOwmsGOzIG4SOmo_ : 1;
        osal_u32 xCt2OyswGOwmsGOslo2xmORl4Omo_ : 1;
        osal_u32 xCt2OAmOy6wGOwmsGOlosmwROARCOmo_ : 1;
        osal_u32 x6NgwjR_uw_WCuwQL6wNoFwqGowWh_ : 1;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_trig_resp_bsrp_bqrp;

typedef union {
    struct {
        osal_u32 xCt2Oy6wGOwmsGO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_bqrp_resp_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwjR_uw_WCuwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_bqrp_resp_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwjC_uw_WCuwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_bsrp_resp_data_rate;

typedef union {
    struct {
        osal_u32 xCt2OyswGOwmsGOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_bsrp_resp_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwjC_wRdWdWwCB1W_ : 8;
        osal_u32 xCt2OyswORl4_ : 4;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_queue_size;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwSiFW_ : 2;
        osal_u32 xCt2OPzGZORstOmo_ : 1;
        osal_u32 xCt2OPzGZOI2IROyCO4mCwOmo_ : 1;
        osal_u32 x6NgwqGuVw_PwFGLGwLGw6QKwWh_ : 1;
        osal_u32 xCt2OloRwOGZGOsmx_ : 1;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_vap2_mode_set0;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwQWwSGL_BPwjdNNwGFF__ : 20;
        osal_u32 x6NgwLPjNwQWwjdNNVwoWh_ : 10;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_vap2_mode_set1;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwjCCBFwQ_ : 16;
        osal_u32 x6NgwqGuVwSG6GFF_wQ_ : 16;
    } bits;

    osal_u32 u32;
} u_vap2_macaddr_bssid_h;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOw8OyCoORlImvSROCAdOmo_ : 1;
        osal_u32 xCt2OPzGZOGsOIv4mOmo_ : 1;
        osal_u32 x6NgwqGuVw_Pwu_jw_CuwCth6wWh_ : 1;
        osal_u32 x6NgwqGuVw_Pwj6hwCth6wWh_ : 1;
        osal_u32 x6NgwqGuVwoBCLWhwBhL_wWh_ : 1;
        osal_u32 xCt2OPzGZO4RlIOloRwOmo_ : 1;
        osal_u32 x6NgwqGuVwj6hwuW_BiFwCth6wFBC_ : 1;
        osal_u32 x6NgwqGuVwLPwj6hwCdCuWhF_ : 1;
        osal_u32 xCt2OPzGZOyCoOtzlxOR8OyCO6Omo_ : 1;
        osal_u32 x6NgwqGuVw_PwNBoCwFBC6wCth6wWh_ : 1;
        osal_u32 xCt2OPzGZOw8ORlIOsQoCOmo_ : 1;
        osal_u32 x6NgwqGuVwCth6wj6hwFB_W6L_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap2_tsf_ctrl1;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOvttsmRORyRROPzx_ : 16;
        osal_u32 x6NgwqGuVwoBCLWhwBhLW_qGo_ : 8;
        osal_u32 x6NgwqGuVwFLBSwuW_BiF_ : 8;
    } bits;

    osal_u32 u32;
} u_vap2_tsf_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwj6hwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap2_bcn_pkt_ptr;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOyCoOxmo_ : 16;
        osal_u32 xCt2OPzGZOyCoORlIOGvs_ : 16;
    } bits;

    osal_u32 u32;
} u_vap2_bcn_tx_length;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOyCoOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap2_bcn_phy_tx_mode;

typedef union {
    struct {
        osal_u32 x6NgwqGuVwj6hwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_vap2_bcn_tx_data_rate;

typedef union {
    struct {
        osal_u32 xCt2OPzGZOGzwRlzxOzl4_ : 9;
        osal_u32 xCt2OPzGZOyssOCvxvw_ : 6;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwqGuVwGBFwqGo_ : 12;
        osal_u32 xCt2OPzGZOGzwRlzxOyssOCvxvw_ : 4;
    } bits;

    osal_u32 u32;
} u_vap2_aid_val;

typedef union {
    struct {
        osal_u32 x6NgwqGukwSiFW_ : 2;
        osal_u32 xCt2OPzGiORstOmo_ : 1;
        osal_u32 xCt2OPzGiOI2IROyCO4mCwOmo_ : 1;
        osal_u32 x6NgwqGukw_PwFGLGwLGw6QKwWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwBhL_wLjLLwCWo_ : 2;
        osal_u32 reserved_1 : 24;
    } bits;

    osal_u32 u32;
} u_vap3_mode_set0;

typedef union {
    struct {
        osal_u32 x6NgwqGukwQWwSGL_BPwjdNNwGFF__ : 20;
        osal_u32 x6NgwLPjNwQWwjdNNkwoWh_ : 10;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_vap3_mode_set1;

typedef union {
    struct {
        osal_u32 x6NgwqGukwjCCBFwQ_ : 16;
        osal_u32 x6NgwqGukwSG6GFF_wQ_ : 16;
    } bits;

    osal_u32 u32;
} u_vap3_macaddr_bssid_h;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOw8OyCoORlImvSROCAdOmo_ : 1;
        osal_u32 xCt2OPzGiOGsOIv4mOmo_ : 1;
        osal_u32 x6NgwqGukw_Pwu_jw_CuwCth6wWh_ : 1;
        osal_u32 x6NgwqGukw_Pwj6hwCth6wWh_ : 1;
        osal_u32 x6NgwqGukwoBCLWhwBhL_wWh_ : 1;
        osal_u32 xCt2OPzGiO4RlIOloRwOmo_ : 1;
        osal_u32 x6NgwqGukwj6hwuW_BiFwCth6wFBC_ : 1;
        osal_u32 x6NgwqGukwLPwj6hwCdCuWhF_ : 1;
        osal_u32 xCt2OPzGiOyCoOtzlxOR8OyCO6Omo_ : 1;
        osal_u32 x6NgwqGukw_PwNBoCwFBC6wCth6wWh_ : 1;
        osal_u32 xCt2OPzGiOw8ORlIOsQoCOmo_ : 1;
        osal_u32 x6NgwqGukwCth6wj6hwFB_W6L_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap3_tsf_ctrl1;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOvttsmRORyRROPzx_ : 16;
        osal_u32 x6NgwqGukwoBCLWhwBhLW_qGo_ : 8;
        osal_u32 x6NgwqGukwFLBSwuW_BiF_ : 8;
    } bits;

    osal_u32 u32;
} u_vap3_tsf_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwqGukwj6hwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap3_bcn_pkt_ptr;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOyCoOxmo_ : 16;
        osal_u32 xCt2OPzGiOyCoORlIOGvs_ : 16;
    } bits;

    osal_u32 u32;
} u_vap3_bcn_tx_length;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOyCoOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vap3_bcn_phy_tx_mode;

typedef union {
    struct {
        osal_u32 x6NgwqGukwj6hwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_vap3_bcn_tx_data_rate;

typedef union {
    struct {
        osal_u32 xCt2OPzGiOGzwRlzxOzl4_ : 9;
        osal_u32 xCt2OPzGiOyssOCvxvw_ : 6;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwqGukwGBFwqGo_ : 12;
        osal_u32 xCt2OPzGiOGzwRlzxOyssOCvxvw_ : 4;
    } bits;

    osal_u32 u32;
} u_vap3_aid_val;

typedef union {
    struct {
        osal_u32 x6NgwLBSWCLGSuw_GhgW_ : 16;
        osal_u32 xCt2Ow8ORlImOCzxOIv4m_ : 1;
        osal_u32 xCt2Ow8ORlImOCzxOsmCvo4Omo_ : 1;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2Ow8ORlImOCzxOz4WOPzx_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_time_cal_mode;

#endif
