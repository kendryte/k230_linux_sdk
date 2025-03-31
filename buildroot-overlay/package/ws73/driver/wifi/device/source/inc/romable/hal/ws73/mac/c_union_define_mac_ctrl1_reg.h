/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_CTRL1_REG_H__
#define __C_UNION_DEFINE_MAC_CTRL1_REG_H__

typedef union {
    struct {
        osal_u32 x6NgwihWwuKLwCWo_ : 2;
        osal_u32 xCt2OvomOGdROCvm8OGwlOIv4m_ : 2;
        osal_u32 xCt2OvomOGdROR8OCoR_ : 8;
        osal_u32 x6NgwihWwuKLwLBSWidLwFBC_ : 1;
        osal_u32 cfg_one_pkt_tx_peer_index : 3;
        osal_u32 cfg_one_pkt_tx_vap_index : 2;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_one_pkt_ctrl1;

typedef union {
    struct {
        osal_u32 xCt2O4yzCOR8OloRwOCxw_ : 1;
        osal_u32 x6NgwFjG6wLPw6hLw6o__ : 1;
        osal_u32 x6NgwihWwuKLwBhL_wSiFW_ : 1;
        osal_u32 x6NgwihWwuKLwRiCwFd_wCWLwSiFW_ : 1;
        osal_u32 xCt2OvomOGdRO6vsO4SwOPzx_ : 16;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_one_pkt_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwihWwuKLwFd_wqGo_ : 16;
        osal_u32 xCt2OvomOGdROtwzImORlImvSR_ : 16;
    } bits;

    osal_u32 u32;
} u_one_pkt_ctrl3;

typedef union {
    struct {
        osal_u32 x6NgwihWwuKLwjdNNwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_one_pkt_buf_addr;

typedef union {
    struct {
        osal_u32 x6NgwihWwuKLwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_one_pkt_phy_tx_mode;

typedef union {
    struct {
        osal_u32 xCt2OvomOGdRO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_one_pkt_datarate;

typedef union {
    struct {
        osal_u32 xCt2OGZGOzGGOGsOmo_ : 1;
        osal_u32 xCt2OGZGOzysmoROsSsGmo4Omo_ : 1;
        osal_u32 xCt2OGZGOPzGOsmx_ : 2;
        osal_u32 x6NgwLTLwCLG_LwLBSWwFWjdgwWh_ : 1;
        osal_u32 x6NgwLTLwLPiuwoBSBLwiNN_ : 1;
        osal_u32 xCt2OR8O6SmSmOIzsdOovsSsGmo4OPx4_ : 1;
        osal_u32 xCt2OGZGOsSsGmo4OIv4m_ : 1;
        osal_u32 xCt2OR8O6SmSmOIzsd_ : 6;
        osal_u32 reserved_0 : 18;
    } bits;

    osal_u32 u32;
} u_p2p_ctrl1;

typedef union {
    struct {
        osal_u32 xCt2OGZGOovzOvGmw_ : 2;
        osal_u32 x6NgwLTLwWh_ : 1;
        osal_u32 xCt2ORqROqzdmOR8Omo_ : 1;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OGZGOovzOCvSoR_ : 8;
        osal_u32 reserved_1 : 16;
    } bits;

    osal_u32 u32;
} u_p2p_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwuVuw6LwTBhFiTwiNNCWLwqGo_ : 16;
        osal_u32 x6NgwuVuwj6hwLBSWidLwqGo_ : 8;
        osal_u32 xCt2OGZGOCROqlo4vqOPzx_ : 8;
    } bits;

    osal_u32 u32;
} u_p2p_ctrl3;

typedef union {
    struct {
        osal_u32 xCt2OGZGOovzOsRzwROvttsmROPzx_ : 16;
        osal_u32 xCt2OGZGOloRwOvttsmROPzx_ : 16;
    } bits;

    osal_u32 u32;
} u_p2p_offset_val;

typedef union {
    struct {
        osal_u32 xCt2ORwl2OwmsGORqROIz8OCoR_ : 2;
        osal_u32 x6NgwL_Bgw_WCuwQTwLTLwWh_ : 1;
        osal_u32 x6NgwQTwLPwuCwuioowWh_ : 1;
        osal_u32 xCt2ORqROwmsGO6vsoSxxOzCdGvxlCQ_ : 2;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_twt_resp_basic_trig;

typedef union {
    struct {
        osal_u32 x6NgwBhL_GwuCwQWwSdwWh_ : 1;
        osal_u32 xCt2OloRwzOGsOPAROISOmo_ : 1;
        osal_u32 x6NgwBhL_GwuCwGSuFdwhiLwFB_W6LwWh_ : 1;
        osal_u32 xCt2OloRwzOGsOPAROSGOmo_ : 1;
        osal_u32 x6NgwBhL_GwuCwQWwduwWh_ : 1;
        osal_u32 xCt2OloRwzOGG4SOGsOmo_ : 1;
        osal_u32 xCt2OloRwzOGG4SOGsOCxvsmOGAQOmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OloRwzOGsOoSIOCxdsO4xQ_ : 10;
        osal_u32 reserved_1 : 14;
    } bits;

    osal_u32 u32;
} u_intra_ppdu_power_save_en;

typedef union {
    struct {
        osal_u32 x6NgwBhL_GwuuFdwuCwLBSWwiNNCWL_ : 16;
        osal_u32 x6NgwBhL_GwuuFdwuCwLBSWwoBSBL_ : 16;
    } bits;

    osal_u32 u32;
} u_he_intra_ppdu_ps_time_offset;

typedef union {
    struct {
        osal_u32 x6NgwQLwSGL_BPwjdNNW_wCLWu_ : 10;
        osal_u32 x6NgwhFuwCidhFBhgw_WCuwjTwNiooiTwhFuwWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwQLwSGL_BPwjdNNW_whdS_ : 4;
        osal_u32 xCt2Ow8OPAROo4GOIz8ORlIm_ : 8;
        osal_u32 x6NgwCidhFBhgwBSSWFBGLWw_WCuwWh_ : 1;
        osal_u32 x6NgwQWwCidhFBhgwuG_GSwFBNNwWh_ : 1;
        osal_u32 x6NgwQWwhFuwLPiuwFd_GLBihwCWLwdhCuW6BNBWFwWh_ : 1;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_sounding_ctrl1;

typedef union {
    struct {
        osal_u32 x6NgwqQLwSGL_BPw6Go6wyg_iduBhg_ : 4;
        osal_u32 xCt2OAROIzRwl8OCzxCO32wvSGlo2_ : 4;
        osal_u32 x6NgwqQLwSGL_BPw6Go6wy6iFWjiiK_ : 2;
        osal_u32 xCt2OAROIzRwl8OCzxCO3Cv4myvvd_ : 2;
        osal_u32 x6NgwSGL_BPw6Go6wLBSWidLwqGodW_ : 8;
        osal_u32 x6NgwhihwLjwCidhFBhgw_uLw6iFWjiiK_ : 1;
        osal_u32 xCt2OovoORyOsvSo4lo2OwGROo2_ : 1;
        osal_u32 reserved_0 : 10;
    } bits;

    osal_u32 u32;
} u_sounding_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwLPjNwQWwjdNNywoWh_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OR8ytOAmOySttrOxmo_ : 10;
        osal_u32 reserved_1 : 10;
    } bits;

    osal_u32 u32;
} u_sounding_ctrl5;

typedef union {
    struct {
        osal_u32 xCt2OAROIzRwl8OySttmwOGvloRmw_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_ht_matrix_buffer_pointer;

typedef union {
    struct {
        osal_u32 x6NgwqQLwjNwuQtwSiFWw6_iu_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_vht_bf_tx_phy_mode;

typedef union {
    struct {
        osal_u32 x6NgwQWwjNwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_he_bf_data_rate;

typedef union {
    struct {
        osal_u32 x6NgwqGuywQWwSGL_BPwjdNNwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap0_he_matrix_buff_addr;

typedef union {
    struct {
        osal_u32 xCt2OPzGrOAmOIzRwl8OySttOz44w_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_vap1_he_matrix_buff_addr;

typedef union {
    struct {
        osal_u32 xCt2OzoRlOloRtORlImZOPzx_ : 16;
        osal_u32 x6NgwGhLBwBhLNwLBSW0wqGo_ : 16;
    } bits;

    osal_u32 u32;
} u_anti_intf1;

typedef union {
    struct {
        osal_u32 xCt2OzoRlOloRtOR8OwRsO4mCwOwzodOmo_ : 1;
        osal_u32 x6NgwGhLBwBhLNwyLQ_wFWjdgwWh_ : 1;
        osal_u32 x6NgwGhLBwBhLNwyLQ_wFWjdgwWh0_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2OzoRlOloRtORAw3OrPzx_ : 8;
        osal_u32 x6NgwGhLBwBhLNwLQ_ywVqGo_ : 8;
        osal_u32 x6NgwGhLBwBhLNwLQ_ywkqGo_ : 8;
    } bits;

    osal_u32 u32;
} u_anti_intf2;

typedef union {
    struct {
        osal_u32 xCt2OzoRlOloRtORAwrO3Pzx_ : 8;
        osal_u32 x6NgwGhLBwBhLNwLQ_0w0qGo_ : 8;
        osal_u32 xCt2OzoRlOloRtORAwrOZPzx_ : 8;
        osal_u32 xCt2OzoRlOloRtORAwrOiPzx_ : 8;
    } bits;

    osal_u32 u32;
} u_anti_intf3;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwu_Bi_BLtwSiFW_ : 2;
        osal_u32 xCt2OCvm8OsqOR8OsRzRSsOtxz2_ : 1;
        osal_u32 xCt2OCvm8OsqOw8OsRzRSsOtxz2_ : 1;
        osal_u32 x6Ngw6iWPwQTw_Pwu_Bi_BLtwFBC_ : 1;
        osal_u32 xCt2OCvm8OsqOw8OzyvwROIv4m_ : 1;
        osal_u32 x6Ngw6iWPwQTw_Pw_WCuwjGwu_BwSiFW_ : 2;
        osal_u32 xCt2OCvm8OAqOw8OwmsGOCRsOGwlOIv4m_ : 2;
        osal_u32 x6Ngw6iWPwQTw_Pw_WCuwiLQW_wu_BwSiFW_ : 2;
        osal_u32 x6Ngw6iWPwLPwGji_Lwu_WWSuLwLtuW_ : 2;
        osal_u32 x6Ngw6iWPwQTwLPw_WCuwjGwu_BwSiFW_ : 2;
        osal_u32 xCt2OCvm8OAqOR8OwmsGOCRsOGwlOIv4m_ : 2;
        osal_u32 x6Ngw6iWPwQTwLPw_WCuwiLQW_wu_BwSiFW_ : 2;
        osal_u32 x6Ngw6iWPwCTw_PwGji_Lw_WR_ : 1;
        osal_u32 xCt2OCvm8OsqOR8OzyvwROIv4m_ : 1;
        osal_u32 x6Ngw6iWPwCTwLPwGji_Lw_WR_ : 1;
        osal_u32 xCt2OCvm8Ow8OR8OzyvwROGwmmIGROmo_ : 1;
        osal_u32 xCt2OCvm8OwtOsRzyxmORlIm_ : 8;
    } bits;

    osal_u32 u32;
} u_coex_ctrl1;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwSGPw6hL_ : 5;
        osal_u32 x6Ngw6iWPw_WCuwFBCGjoW_ : 1;
        osal_u32 xCt2OCvm8OqzlROCCzO4mRO4lszyxm_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OCvm8OGwmmIGROR8OCoR_ : 2;
        osal_u32 xCt2OCvm8OGvsROGwmmIGROR8Omo_ : 1;
        osal_u32 xCt2OCvm8OR8OzyvwROGvsROGwmmIRO4ls_ : 1;
        osal_u32 xCt2OCvm8OGvsROGwmmIGROR8OCoR_ : 4;
        osal_u32 xCt2OCvm8OR8OzyvwROwmsGOmo_ : 1;
        osal_u32 x6Ngw6iWPwLPwu_WWSuLwGihwWh_ : 1;
        osal_u32 xCt2OCvm8OR8OyCoOwm6Omo_ : 1;
        osal_u32 x6Ngw6iWPwLPwQBu_BwRw_WRwWh_ : 1;
        osal_u32 xCt2OCvm8OR8OovwIO6Owm6Omo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 cfg_coex_tx_vap_index : 2;
        osal_u32 cfg_coex_tx_qos_null_tid : 4;
        osal_u32 cfg_coex_tx_peer_index : 3;
        osal_u32 reserved_2 : 1;
    } bits;

    osal_u32 u32;
} u_coex_ctrl2;

typedef union {
    struct {
        osal_u32 xCt2OCvm8OCtOmo4OR8ORlIm_ : 16;
        osal_u32 x6Ngw6iWPwGji_LwCWoNw6LCwFd_GLBih_ : 16;
    } bits;

    osal_u32 u32;
} u_coex_ctrl3;

typedef union {
    struct {
        osal_u32 xCt2OCvm8OzyvwROsmxtOCRsOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_coex_ctrl4;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwGji_LwCWoNw6LCwFGLGw_GLWw6_iu_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_coex_ctrl5;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwuiCLwu_WWSuLwLBSWidLwqGo_ : 16;
        osal_u32 xCt2OCvm8OyROzyvwRORlImvSROPzx_ : 16;
    } bits;

    osal_u32 u32;
} u_coex_ctrl6;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwGji_LwhdoowjdNNwGFF__ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_coex_ctrl7;

typedef union {
    struct {
        osal_u32 xCt2Ow8OsRzOwRsOCAdOsmCOCAOmo_ : 1;
        osal_u32 xCt2Ow8OwRsOwmsGOCRsOIv4m_ : 1;
        osal_u32 x6Ngw_PwFthw_LCw6QKwCW6w6QwWh_ : 1;
        osal_u32 xCt2Ow8O4QoOyqOsmx_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_dynamic_bw_ctrl;

typedef union {
    struct {
        osal_u32 x6Ngw6QwoiGFwCLGLwuW_BiF_ : 16;
        osal_u32 x6Ngw6QwCLGLwLPwGFbwqGo_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_ch_load_stat_period;

typedef union {
    struct {
        osal_u32 x6NgwNLSwWh_ : 1;
        osal_u32 xCt2OtRIO4lzxv2ORvdmoOCAdOmo_ : 1;
        osal_u32 xCt2OtRIOsm6OovOCAdOmo_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_ftm_ctrl;

typedef union {
    struct {
        osal_u32 xCt2Ow8OAmOARCOloRwOmo_ : 15;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_rx_he_htc_ctrl;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OvyssOG4OyzsmOCAdOmo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2Osw2OmxmImoROPx4_ : 1;
        osal_u32 xCt2Osw2OyssOCvxvwOmo_ : 1;
        osal_u32 x6NgwC_gwuG_LBGowjCCBFwWh_ : 1;
        osal_u32 x6NgwC_gwg_iduwBFwWh_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 x6NgwhihwC_gw6LCwjtuGCC_ : 1;
        osal_u32 reserved_3 : 1;
        osal_u32 xCt2OvyssOG4OswORwl2O4ls_ : 1;
        osal_u32 reserved_4 : 1;
        osal_u32 x6NgwijCCwuFw6L_owN_SwjtuGCC_ : 1;
        osal_u32 xCt2OvyssOG4OI2IROtwIOyQGzss_ : 1;
        osal_u32 xCt2OvyssOG4OCAdOtCsOmo_ : 1;
        osal_u32 x6NgwijCCwuFwFBCwoiTwuT__ : 1;
        osal_u32 x6NgwijCCwuFwjtuGCCwN_SwoWh_ : 8;
        osal_u32 xCt2OvyssOG4OyQGzssOsAvwROtwI_ : 1;
        osal_u32 x6NgwijCCwuFw_WCWLwuQtw_PwWh_ : 1;
        osal_u32 reserved_5 : 3;
        osal_u32 reserved_6 : 3;
    } bits;

    osal_u32 u32;
} u_spatial_reuse_ctrl1;

typedef union {
    struct {
        osal_u32 x6NgwuC_wjGCWFwC_wWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OGswOGswROGG4SOwmsGOmo_ : 1;
        osal_u32 xCt2OGswOGswROGG4SOR8Omo_ : 1;
        osal_u32 xCt2OGswOyzsmOIv4m_ : 1;
        osal_u32 xCt2OGswOsmCOZ3IOmo_ : 1;
        osal_u32 x6NgwuC_wjGCWFwBhCW_LwQL6wu_iSiFW_ : 1;
        osal_u32 xCt2OGswOGqwOsqOCzxC_ : 1;
        osal_u32 x6NgwuC_wC_wuT_wiNNCWL_ : 8;
        osal_u32 xCt2OGswOwmsmROGAQOw8Omo_ : 1;
        osal_u32 x6NgwC_wWhwNi_wLWCL_ : 1;
        osal_u32 reserved_1 : 3;
        osal_u32 reserved_2 : 3;
        osal_u32 x6NgwC_uwFBCGooiTwqGo_ : 4;
        osal_u32 reserved_3 : 4;
    } bits;

    osal_u32 u32;
} u_spatial_reuse_ctrl2;

typedef union {
    struct {
        osal_u32 x6NgwuC_wjGCWFwC_wLPw_GLWw_GhKwWh_ : 1;
        osal_u32 x6NgwuC_wjGCWFwC_wLPw_GLWw_GhKwCWo_ : 2;
        osal_u32 xCt2OvyssOG4Oyzsm4OR8OwzRmOwzodOmo_ : 1;
        osal_u32 xCt2OvyssOG4Oyzsm4OR8OwzRmOwzodOsmx_ : 2;
        osal_u32 xCt2OswO4lszxxvqOozPOmo_ : 1;
        osal_u32 xCt2OvyssOG4OwGROCxw_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwijCCwuFw_uLwCLG_L_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2OvyssOG4OR8OwGROsRzwR_ : 1;
        osal_u32 x6NgwijCCwuFwLPw_uLw6o__ : 1;
        osal_u32 x6NgwuC_wC_w_uLw6o__ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xCt2OGswOswOwGROsRzwR_ : 1;
        osal_u32 xCt2OGswOyzsm4OswOGswROGqw_ : 8;
        osal_u32 xCt2OGswOyzsm4OswOGqwOxPO4mtzSxR_ : 8;
    } bits;

    osal_u32 u32;
} u_spatial_reuse_ctrl3;

typedef union {
    struct {
        osal_u32 x6NgwijCCw6hLw_CCBw_GhKy_ : 8;
        osal_u32 xCt2OyssOCvxvwOCAdO4zRzOtwI_ : 1;
        osal_u32 xCt2OyssOCvxvwOwGROmo_ : 1;
        osal_u32 x6NgwjCCw6ioi_w_uLw6o__ : 1;
        osal_u32 xCt2OyssOCvxvwOCAdOmo_ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_spatial_reuse_ctrl4;

typedef union {
    struct {
        osal_u32 xCt2OvyssOCoROwsslOwzodK_ : 8;
        osal_u32 x6NgwijCCw6hLw_CCBw_GhKk_ : 8;
        osal_u32 x6NgwijCCw6hLw_CCBw_GhKV_ : 8;
        osal_u32 xCt2OvyssOCoROwsslOwzodr_ : 8;
    } bits;

    osal_u32 u32;
} u_obss_cnt_rssi_rank;

typedef union {
    struct {
        osal_u32 x6NgwuC_wC_w_Pw_WCWLwhdS_ : 8;
        osal_u32 xCt2OvyssOG4Ow8OwmsmROoSI_ : 8;
        osal_u32 x6NgwuC_wC_wWhwiNNCWL_ : 8;
        osal_u32 xCt2OvyssOG4OmoOvttsmR_ : 8;
    } bits;

    osal_u32 u32;
} u_spatial_reuse_offset;

typedef union {
    struct {
        osal_u32 xCt2OvyssOG4OxP_ : 8;
        osal_u32 xCt2Osw2OvyssOG4OxP_ : 8;
        osal_u32 x6NgwijCCwuFwC_wLPwuiTW__ : 8;
        osal_u32 x6NgwijCCwuFwC_gwLPwuiTW__ : 8;
    } bits;

    osal_u32 u32;
} u_obss_pd_power;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwFCCCwLPwuiTW_woq_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_dsss_power_lv;

typedef union {
    struct {
        osal_u32 xCt2OwmsGO4sssOR8OGvqmwOxPOIlo_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_dsss_power_lv_min;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwFCCCwLPwuiTW_woqwSGP_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_dsss_power_lv_max;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwiNFSwVDSwLPwuiTW_woq_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_ofdm_24m_power_lv;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOvt4IOZKIOR8OGvqmwOxPOIlo_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_ofdm_24m_power_lv_min;

typedef union {
    struct {
        osal_u32 x6Ngw_WCuwiNFSwVDSwLPwuiTW_woqwSGP_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_resp_ofdm_24m_power_lv_max;

typedef union {
    struct {
        osal_u32 xCt2OwmsGOGvqmwOxPOIz8Owssl_ : 8;
        osal_u32 x6Ngw_WCuwuiTW_woqwSBhw_CCB_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_resp_power_lv_sel;

typedef union {
    struct {
        osal_u32 x6NgwQWwLjwS6CwLPwuiTW_wy_ : 30;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_he_tb_mcs_tx_power0;

typedef union {
    struct {
        osal_u32 xCt2OAmORyOICsOR8OGvqmwOr_ : 30;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_he_tb_mcs_tx_power1;

typedef union {
    struct {
        osal_u32 xCt2ORGCOlolRlzxOGvqmw_ : 8;
        osal_u32 x6NgwL_Bgw_CCBwhi_wCWLwqGo_ : 9;
        osal_u32 xCt2ORwl2OwsslOovwOsmROmo_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2ORyOwSO4mxRzOGvqmw_ : 12;
    } bits;

    osal_u32 u32;
} u_he_tb_mcs_tx_power2;

typedef union {
    struct {
        osal_u32 xCt2OAmORyOwSOR8OGvqmwOIz8OZKZ_ : 6;
        osal_u32 x6NgwQWwLjw_dwLPwuiTW_wSGPw0yX_ : 6;
        osal_u32 xCt2OAmORyOwSOR8OGvqmwOIz8OaZ_ : 6;
        osal_u32 x6NgwQWwLjw_dwLPwuiTW_wSGPwVX_ : 6;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_he_tb_ru_tx_power_max;

typedef union {
    struct {
        osal_u32 xCt2OytwGOwmsGORGCOGvqmwOIv4m_ : 1;
        osal_u32 xCt2OISOyzwOwmsGORGCOGvqmwOIv4m_ : 1;
        osal_u32 xCt2OotwGOwmsGORGCOGvqmwOIv4m_ : 1;
        osal_u32 x6NgwjC_uw_WCuwLu6wuiTW_wSiFW_ : 1;
        osal_u32 xCt2Oy6wGOwmsGORGCOGvqmwOIv4m_ : 1;
        osal_u32 xCt2OyzslCOwmsGORGCOGvqmwOIv4m_ : 1;
        osal_u32 x6NgwL_Cw_WCuwLu6wuiTW_wSiFW_ : 1;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_trig_resp_tpc_power_mode;

typedef union {
    struct {
        osal_u32 x6NgwC_uLw_WCuwuFwqGo_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_srpt_resp_pd_val;

typedef union {
    struct {
        osal_u32 x6NgwC_uLw_WCuwC_uwqGo_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_srpt_resp_srp_val;

typedef union {
    struct {
        osal_u32 x6Ngw6idhLw_CCBw_GhK0_ : 8;
        osal_u32 xCt2OCvSoROwsslOwzod3_ : 8;
        osal_u32 xCt2OCvSoROCCzOsmx_ : 1;
        osal_u32 xCt2OCvSoROR8O4zRzOsmx_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_cfg_cnt_param;

typedef union {
    struct {
        osal_u32 x6NgwTGuBwuhwVSCj_ : 16;
        osal_u32 xCt2Ow8OqzGlOPx4OGmmwOlo4m8_ : 8;
        osal_u32 xCt2Ow8OqzGlOPx4OPzGOlo4m8_ : 4;
        osal_u32 reserved_0 : 4;
    } bits;

    osal_u32 u32;
} u_wapi_pn_2msb;

typedef union {
    struct {
        osal_u32 xCt2OmRAZOsozGOA4wOIsyr_ : 16;
        osal_u32 x6NgwWLQVwChGuwQF_wSCjy_ : 16;
    } bits;

    osal_u32 u32;
} u_eth2_snap_hdr_code1;

typedef union {
    struct {
        osal_u32 xCt2OmRAZOxmo2RAOsmROIv4m_ : 1;
        osal_u32 xCt2OmRAOR8OzIs4SOsmROIv4m_ : 1;
        osal_u32 xCt2OmRAOSolCzsROz44wrOsmROIv4m_ : 1;
        osal_u32 xCt2OmRAOz44wZOsmROIv4m_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_eth_tx_rx_set;

typedef union {
    struct {
        osal_u32 xCt2OPAROytO4zRzOwzRmOCwvG_ : 23;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_vht_bf_data_rate;

typedef union {
    struct {
        osal_u32 xCt2OAmOytOGAQOIv4mOCwvG_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_he_bf_tx_phy_mode;

typedef union {
    struct {
        osal_u32 x6Ngw_LCwuiTW_whWG_wqGowLu6w6Q_ : 8;
        osal_u32 xCt2OwRsOGvqmwOomzwOPzxO4G4ORGCOxP_ : 2;
        osal_u32 x6Ngw_LCwuiTW_whWG_wqGow6N_wBFP_ : 2;
        osal_u32 reserved_0 : 4;
        osal_u32 x6Ngw_LCwLu6w_GLWwCWLwSiFW_ : 1;
        osal_u32 reserved_1 : 15;
    } bits;

    osal_u32 u32;
} u_rts_autorate1;

typedef union {
    struct {
        osal_u32 xCt2OwRsOGvqmwOovwIzxOPzxORGCOCA_ : 8;
        osal_u32 x6Ngw_LCwuiTW_whi_SGowqGowFuFwLu6woq_ : 2;
        osal_u32 xCt2OwRsOGvqmwOovwIzxOPzxOCtwOl48_ : 2;
        osal_u32 reserved_0 : 4;
        osal_u32 x6Ngw_LCwuiTW_wNG_wqGowLu6w6Q_ : 8;
        osal_u32 xCt2OwRsOGvqmwOtzwOPzxO4G4ORGCOxP_ : 2;
        osal_u32 x6Ngw_LCwuiTW_wNG_wqGow6N_wBFP_ : 2;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_rts_autorate2;

typedef union {
    struct {
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoywS6C_ : 4;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx3Oyq_ : 2;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx3OGwvRvCvx_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx3OGwmzIyxm_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxrOICs_ : 4;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGo0wjT_ : 2;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGo0wu_iLi6io_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGo0wu_WGSjoW_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoVwS6C_ : 4;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxZOyq_ : 2;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxZOGwvRvCvx_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxZOGwmzIyxm_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGokwS6C_ : 4;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxiOyq_ : 2;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxiOGwvRvCvx_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxiOGwmzIyxm_ : 1;
    } bits;

    osal_u32 u32;
} u_rts_autorate3;

typedef union {
    struct {
        osal_u32 xCt2OwRsOwzRmOwzodOPzxKOICs_ : 4;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoDwjT_ : 2;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoDwu_iLi6io_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoDwu_WGSjoW_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoxwS6C_ : 4;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxaOyq_ : 2;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxaOGwvRvCvx_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxaOGwmzIyxm_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzxYOICs_ : 4;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoXwjT_ : 2;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoXwu_iLi6io_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGoXwu_WGSjoW_ : 1;
        osal_u32 x6Ngw_LCw_GLWw_GhKwqGopwS6C_ : 4;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx9Oyq_ : 2;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx9OGwvRvCvx_ : 1;
        osal_u32 xCt2OwRsOwzRmOwzodOPzx9OGwmzIyxm_ : 1;
    } bits;

    osal_u32 u32;
} u_rts_autorate4;

typedef union {
    struct {
        osal_u32 x6Ngw_LCwuiTW_woiTwqGowLu6w6Q_ : 8;
        osal_u32 xCt2OwRsOGvqmwOxvqOPzxO4G4ORGCOxP_ : 2;
        osal_u32 x6Ngw_LCwuiTW_woiTwqGow6N_wBFP_ : 2;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OwRsOGvqmwOAl2AOPzxORGCOCA_ : 8;
        osal_u32 x6Ngw_LCwuiTW_wQBgQwqGowFuFwLu6woq_ : 2;
        osal_u32 xCt2OwRsOGvqmwOAl2AOPzxOCtwOl48_ : 2;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_rts_autorate;

#endif
