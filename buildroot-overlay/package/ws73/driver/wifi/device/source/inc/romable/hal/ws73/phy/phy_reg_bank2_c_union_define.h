// ******************************************************************************
// Copyright : Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK2_C_UNION_DEFINE_H__
#define __PHY_REG_BANK2_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 x6NgwhdSwF6wiNNCWLwuKL_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_num_dc_offset_pkt;

typedef union {
    struct {
        osal_u32 x6NgwhdSwF6wiNNCWLwuBh_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_num_dc_offset_pin;

typedef union {
    struct {
        osal_u32 x6NgwhdSwBh6iSBhg_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_num_incoming;

typedef union {
    struct {
        osal_u32 x6NgwCL_ihgwCBghGowLQ_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_strong_signal_th;

typedef union {
    struct {
        osal_u32 x6NgwBh6iSBhgwCBghGowLQ_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_incoming_signal_th;

typedef union {
    struct {
        osal_u32 xCt2OoSIOloCvIlo2ORA_ : 7;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_num_incoming_th;

typedef union {
    struct {
        osal_u32 xCt2OszRSOysQoCORA_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2O4OszRSOysQoCORA_ : 10;
        osal_u32 reserved_1 : 10;
    } bits;

    osal_u32 u32;
} u_satu_bsync_th;

typedef union {
    struct {
        osal_u32 x6NgwCGLdwGCth6wLQ_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwFwCGLdwGCth6wLQ_ : 10;
        osal_u32 reserved_1 : 10;
    } bits;

    osal_u32 u32;
} u_satu_async_th;

typedef union {
    struct {
        osal_u32 xCt2OoSIOszRSOysQoC_ : 2;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2O4OoSIOszRSOysQoC_ : 2;
        osal_u32 reserved_1 : 26;
    } bits;

    osal_u32 u32;
} u_num_satu_bsync;

typedef union {
    struct {
        osal_u32 x6NgwhdSwCGLdwGCth6_ : 2;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwFwhdSwCGLdwGCth6_ : 2;
        osal_u32 reserved_1 : 26;
    } bits;

    osal_u32 u32;
} u_num_satu_async;

typedef union {
    struct {
        osal_u32 xCt2OoSIOCwvszRSOysQoCORA_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2O4OoSIOCwvszRSOysQoCORA_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 reserved_2 : 7;
        osal_u32 reserved_3 : 9;
    } bits;

    osal_u32 u32;
} u_num_crosatu_bsync_th;

typedef union {
    struct {
        osal_u32 x6NgwhdSw6_iCGLdwGCth6wLQ_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwFwhdSw6_iCGLdwGCth6wLQ_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 reserved_2 : 7;
        osal_u32 reserved_3 : 9;
    } bits;

    osal_u32 u32;
} u_num_crosatu_async_th;

typedef union {
    struct {
        osal_u32 xCt2OoSIOGvqmwsSIOGdR_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_num_powersum_pkt;

typedef union {
    struct {
        osal_u32 xCt2OoSIOGvqmwsSIOysQoC_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_num_powersum_bsync;

typedef union {
    struct {
        osal_u32 x6NgwhdSwuiTW_CdSwGCth6_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_num_powersum_async;

typedef union {
    struct {
        osal_u32 xCt2OxvqOwmxOz4COGlo_ : 9;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_cfg_low_rel_adc_pin;

typedef union {
    struct {
        osal_u32 xCt2Oz4CORzw2mR_ : 9;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2O4l2lRzxO2zloORzw2mR_ : 9;
        osal_u32 reserved_1 : 11;
    } bits;

    osal_u32 u32;
} u_adc_target;

typedef union {
    struct {
        osal_u32 xCt2OCvzwsmORmxOmww_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_coarse_tel_err;

typedef union {
    struct {
        osal_u32 xCt2OIz8OCvzwsmOoSI_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_cfg_max_coarse_num;

typedef union {
    struct {
        osal_u32 x6NgwNBhWwLiw6iG_CWwLWowW___ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_fine_to_coarse_tel_err;

typedef union {
    struct {
        osal_u32 x6NgwNBhWwLWowW___ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_fine_tel_err;

typedef union {
    struct {
        osal_u32 x6NgwSGPwNBhWwhdS_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_cfg_max_fine_num;

typedef union {
    struct {
        osal_u32 x6NgwL_G6KwGjiqWwW__wjCth6_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_track_above_err_bsync;

typedef union {
    struct {
        osal_u32 xCt2ORwzCdOzyvPmOmwwOzsQoC_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_track_above_err_async;

typedef union {
    struct {
        osal_u32 x6NgwL_G6KwjWoiTwW__wjCth6_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_track_below_err_bsync;

typedef union {
    struct {
        osal_u32 xCt2ORwzCdOymxvqOmwwOzsQoC_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_track_below_err_async;

typedef union {
    struct {
        osal_u32 x6NgwLG_gWLwGjiqWwW__wjCth6_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_target_above_err_bsync;

typedef union {
    struct {
        osal_u32 xCt2ORzw2mROzyvPmOmwwOzsQoC_ : 8;
        osal_u32 x6NgwLG_gWLwGjiqWwW__wLQ_ : 8;
        osal_u32 x6NgwLG_gWLwGjiqWwW__wLQ0_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_cfg_target_above_err_async;

typedef union {
    struct {
        osal_u32 x6NgwLG_gWLwjWoiTwW__wjCth6_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_target_below_err_bsync;

typedef union {
    struct {
        osal_u32 xCt2ORzw2mROymxvqOmwwOzsQoC_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_target_below_err_async;

typedef union {
    struct {
        osal_u32 xCt2OoSIOwltsOqzlROszIG_ : 11;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwTGBLwFNWwF6w_FtwjtuGCC_ : 1;
        osal_u32 xCt2OqzlROvoxlomO4COyQGzss_ : 1;
        osal_u32 reserved_1 : 18;
    } bits;

    osal_u32 u32;
} u_num_rifs_wait_samp;

typedef union {
    struct {
        osal_u32 xCt2Oz2COGdRO4mROqloOloOsQoCOwmxlzyxmO4mROmo_ : 1;
        osal_u32 x6NgwGg6wuKLwFWLwTBhwBhwCth6w_WoBGjoWwFWLwL_G6KwuiTW_wCWo_ : 2;
        osal_u32 x6NgwGg6wuKLwFWLwTBhwBhwCth6w_WoBGjoWwFWLwuiTW_wFBNNwGjiqWwLQ_ : 8;
        osal_u32 x6NgwGg6wuKLwFWLwTBhwBhwCth6w_WoBGjoWwFWLwuiTW_wFBNNwjWoiTwLQ_ : 8;
        osal_u32 x6NgwGg6wuKLwFWLwTBhwBhwCth6w_WoBGjoWwFWLwjWNi_Ww_CCBwCWo_ : 1;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_agc_pkt_det_win_in_sync_reliable_det;

typedef union {
    struct {
        osal_u32 xCt2Oz2COGloORwzCdOwsslOsmx_ : 1;
        osal_u32 xCt2Oz2COtlwsROwsslOwmxlzyxmO4mROmo_ : 1;
        osal_u32 xCt2Oz2COtlwsROwsslOwmxlzyxmO4mROz4COGloORA_ : 9;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2O4l2lRzxOszRO4mROIv4m_ : 2;
        osal_u32 x6NgwFBgBLGowCGLwjtuGCCwSiFW_ : 1;
        osal_u32 reserved_1 : 17;
    } bits;

    osal_u32 u32;
} u_agc_first_rssi_reliable_det;

typedef union {
    struct {
        osal_u32 x6NgwGg6woi6KwLBSWwWhwjtwdhoi6KwjtuGCC_ : 1;
        osal_u32 xCt2Oz2COxvCdORlImOCoROyQOSoxvCd_ : 7;
        osal_u32 x6NgwGg6woi6KwLBSWw6hLwjtwCGL_ : 7;
        osal_u32 xCt2Oz2COGdRO4mROxvo2Omo_ : 1;
        osal_u32 xCt2Oz2CO4l2OGdRZORAwmsA_ : 5;
        osal_u32 x6NgwGg6wNBhWwuT_WCLwWh_ : 1;
        osal_u32 x6NgwGg6wVGg6wLQ_WCQwQB_ : 6;
        osal_u32 xCt2Oz2COSoxvCdnOCoRORA_ : 3;
        osal_u32 x6NgwGg6wFGg6wduwdhoi6KwWh_ : 1;
    } bits;

    osal_u32 u32;
} u_agc_op1;

typedef union {
    struct {
        osal_u32 xCt2Oz2CORzw2mROzyvPmOmwwORAOAl2AmsR_ : 7;
        osal_u32 xCt2Oz2CO4lsSoxvCdOiKOGwlZ3wsslORAOr_ : 8;
        osal_u32 xCt2Oz2COGdROsROP2zOsmx_ : 2;
        osal_u32 x6NgwGg6wuKLwCLwqgGw6iFW_ : 6;
        osal_u32 x6NgwGg6whiL6Qwu_BVySwqgG6iFW_ : 1;
        osal_u32 xCt2Oz2COovRCAOP2zORA_ : 6;
        osal_u32 xCt2Oz2COtzsROxvCdOmo_ : 1;
        osal_u32 x6NgwGg6wFGg6wduwBhwoi6KwWh_ : 1;
    } bits;

    osal_u32 u32;
} u_agc_op2;

typedef union {
    struct {
        osal_u32 x6Ngw66GwiqW_oGuwFWLwCWo_ : 1;
        osal_u32 xCt2OCCzOvPmwxzGOGvqrOwmt_ : 2;
        osal_u32 x6Ngw66GwiqW_oGuwuiTVw_WN_ : 4;
        osal_u32 x6Ngw66GwiqW_oGuwuiTkw_WN_ : 6;
        osal_u32 x6Ngw66GwiqW_oGuwuiTwLQ_ : 8;
        osal_u32 xCt2OCCzOvPmwxzGOsRzORA_ : 4;
        osal_u32 xCt2OCCzOvPmwxzGOsRzORAr_ : 4;
        osal_u32 reserved_0 : 3;
    } bits;

    osal_u32 u32;
} u_cca_overlap_det;

typedef union {
    struct {
        osal_u32 xCt2OCCzOvPmwxzGOGvqKOwmt_ : 6;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_cca_overlap_det1;

typedef union {
    struct {
        osal_u32 xCt2OloROxozOsmRORlIm_ : 8;
        osal_u32 xCt2OCvm8OsmRORlIm_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_int_lna_set_time;

typedef union {
    struct {
        osal_u32 x6NgwTuw_PwG6LBqWwiuLBih_ : 2;
        osal_u32 xCt2OqGOszRSwzRm4OvGRlvo_ : 2;
        osal_u32 xCt2OCvm8OsxozOyROxvCdOyQGzss_ : 1;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_coexslnacfgregister;

typedef union {
    struct {
        osal_u32 xCt2OP2zOsmRORlIm_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_vga_set_time;

typedef union {
    struct {
        osal_u32 xCt2O4mtzSxROtxz2_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cfg_default_flag;

typedef union {
    struct {
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2O4mtzSxROxozCv4mOlolR_ : 4;
        osal_u32 xCt2O4mtzSxROm8xozOlolR_ : 4;
        osal_u32 reserved_1 : 20;
    } bits;

    osal_u32 u32;
} u_default_lnacode;

typedef union {
    struct {
        osal_u32 x6NgwG66dSwTBhwjNwoi6K_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_accum_win_bf_sync;

typedef union {
    struct {
        osal_u32 x6NgwCoBFWwTBhwjNwoi6K_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_slide_win_bf_sync;

typedef union {
    struct {
        osal_u32 xCt2OzCCSIOqloOtvwOrry4z2C_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_accum_win_for_initialbw;

typedef union {
    struct {
        osal_u32 xCt2Osxl4mOqloOtvwOrry4z2C_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_slide_win_for_initialbw;

typedef union {
    struct {
        osal_u32 x6NgwG66dSwTBhwL_G6KBhg_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_accum_win_tracking;

typedef union {
    struct {
        osal_u32 x6NgwCoBFWwTBhwL_G6KBhg_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_slide_win_tracking;

typedef union {
    struct {
        osal_u32 xCt2OGdRO4mROqloORAr_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pkt_det_win_th1;

typedef union {
    struct {
        osal_u32 x6NgwuKLwFWLwTBhwLQV_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pkt_det_win_th2;

typedef union {
    struct {
        osal_u32 xCt2Ow8OGdROmo4OqzlRORlIm_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pkt_end_wait;

typedef union {
    struct {
        osal_u32 xCt2O4COvttsmROmsROIloOP2z_ : 6;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2O4COvttsmROmsROIloO42zlo_ : 5;
        osal_u32 reserved_1 : 3;
        osal_u32 x6NgwF6wWCLwSBhwqgG_ : 6;
        osal_u32 reserved_2 : 10;
    } bits;

    osal_u32 u32;
} u_dc_offset_est_th;

typedef union {
    struct {
        osal_u32 x6NgwCBhgoWwLihWwWCLwSBhwqgG_ : 6;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_single_tone_est_th;

typedef union {
    struct {
        osal_u32 x6NgwF6w6iSuwjtuGCC_ : 1;
        osal_u32 x6Ngw_WCWLwFNW66GwjtuGCC_ : 1;
        osal_u32 x6NgwuwhwCGLwFWLwWh_ : 1;
        osal_u32 x6NgwFBgBLGowCGLwFWLwWh_ : 1;
        osal_u32 x6NgwuT_6Go6wF6wjtuGCC_ : 1;
        osal_u32 x6NgwN_WW1WwGg6wjtuGCC_ : 1;
        osal_u32 xCt2O4COmoOyQOCzxlOyQGzss_ : 1;
        osal_u32 x6NgwohGwGFbwNBhWwWh_ : 1;
        osal_u32 xCt2OszRO4mROz4COsmx_ : 1;
        osal_u32 xCt2OszRO4mROsROsmx_ : 1;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_agc_enable_or_bypass;

typedef union {
    struct {
        osal_u32 xCt2O4mtzSxRORwzosORlIm_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cfg_default_trans_time;

typedef union {
    struct {
        osal_u32 x6NgwuBhw6jTVywSBhwLQ_ : 8;
        osal_u32 x6NgwuBhw6jTVywSBhw0LQ_ : 8;
        osal_u32 xCt2OGqwOSoxCdOSsmORA_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_pin_cbw20_min;

typedef union {
    struct {
        osal_u32 xCt2O4l2lRzxOsmRORlIm_ : 6;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_dagc_set_time;

typedef union {
    struct {
        osal_u32 x6NgwSGPwFBgBLGowGg6_ : 5;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2O4z2COrryOylROxlIlR_ : 4;
        osal_u32 reserved_1 : 20;
    } bits;

    osal_u32 u32;
} u_max_digital_agc;

typedef union {
    struct {
        osal_u32 xCt2OGwlZ3OrryORzw2mROxv_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 x6Ngwu_BVywiNFSwLG_gWLwoi_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2OGwlK3Ovt4IORzw2mROxv_ : 7;
        osal_u32 reserved_2 : 9;
    } bits;

    osal_u32 u32;
} u_dagc_target_lo;

typedef union {
    struct {
        osal_u32 xCt2OGwlZ3OrryORzw2mROAl_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 x6Ngwu_BVywiNFSwLG_gWLwQB_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2OGwlK3Ovt4IORzw2mROAl_ : 7;
        osal_u32 reserved_2 : 9;
    } bits;

    osal_u32 u32;
} u_dagc_target_hi;

typedef union {
    struct {
        osal_u32 x6NgwFNWw_CCBwCTwuiTW__ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_dfe_rssi_switch_power;

typedef union {
    struct {
        osal_u32 xCt2OGqwO4lttOxvqORA_ : 8;
        osal_u32 x6NgwuT_wFBNNwQBgQwLQ_ : 8;
        osal_u32 xCt2OGqwO4lttOztOxvqORA_ : 8;
        osal_u32 x6NgwuT_wFBNNwGNwQBgQwLQ_ : 8;
    } bits;

    osal_u32 u32;
} u_pwr_diff_high_low_th;

typedef union {
    struct {
        osal_u32 xCt2Ow8l6OtlxRmwO4yIORA_ : 8;
        osal_u32 reserved_0 : 2;
        osal_u32 reserved_1 : 2;
        osal_u32 x6NgwuBhw6jTVywdCWwCWo_ : 1;
        osal_u32 reserved_2 : 3;
        osal_u32 x6NgwhiBCWwNoii_w6hLwLQ_ : 12;
        osal_u32 reserved_3 : 4;
    } bits;

    osal_u32 u32;
} u_agc_track_ant_sel;

typedef union {
    struct {
        osal_u32 x6Ngwu_BVywFBNNwoiTwLQ_ : 8;
        osal_u32 xCt2OGwlZ3O4lttOAl2AORA_ : 8;
        osal_u32 xCt2OGwlZ3O4lttOztOAl2AORA_ : 8;
        osal_u32 x6Ngwu_BVywFBNNwGNwoiTwLQ_ : 8;
    } bits;

    osal_u32 u32;
} u_pri20_diff_high_low_th;

typedef union {
    struct {
        osal_u32 xCt2Oz2CO4COwGROmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_dc_rpt_en;

typedef union {
    struct {
        osal_u32 x6NgwF6wWCLwSBhwqgGwCth6_ : 6;
        osal_u32 xCt2O4COmsROIloOP2zOqml2AR_ : 6;
        osal_u32 x6NgwF6wWCLwSBhwFgGBh_ : 5;
        osal_u32 x6NgwF6wWCLwSBhwFgGBhwCth6_ : 5;
        osal_u32 xCt2O4COmsROIloO42zloOqml2AR_ : 5;
        osal_u32 reserved_0 : 5;
    } bits;

    osal_u32 u32;
} u_dc_offset_enable;

typedef union {
    struct {
        osal_u32 x6NgwL_G6KwNBoLW_wCWo_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_track_filter;

typedef union {
    struct {
        osal_u32 x6Ngw66GwjtuGCC_ : 1;
        osal_u32 x6NgwBhBLwjTwjtuGCC_ : 1;
        osal_u32 xCt2OlolROCCzOPzxSm_ : 1;
        osal_u32 x6NgwGg6w66GwjdCtwjtuGCC_ : 1;
        osal_u32 x6NgwGg6w66GwLBSWwidL_ : 10;
        osal_u32 reserved_0 : 18;
    } bits;

    osal_u32 u32;
} u_cca_bypass;

typedef union {
    struct {
        osal_u32 x6NgwLBSWiqLwNBhWwCWLwCL_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cfg_timeovt_fine_set_st;

typedef union {
    struct {
        osal_u32 x6NgwQWwWPLWhFwLBSW_ : 16;
        osal_u32 xCt2OtlolsAO4mxzQOrYSs_ : 16;
    } bits;

    osal_u32 u32;
} u_htstf_agc3;

typedef union {
    struct {
        osal_u32 xCt2Oz2COsQoCOtxz2OrryOCxwOyGs_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cfg_agc_sync_flag_11b_clr_bps;

typedef union {
    struct {
        osal_u32 x6NgwGg6w6d__wDCLwiqW_LBSWwWh_ : 1;
        osal_u32 reserved_0 : 15;
        osal_u32 x6NgwGg6w6d__wDCLwhdS_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_agc_curr_4st_overtime_en;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 reserved_1 : 2;
        osal_u32 x6NgwqgGw6iFWwSiFW_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 x6NgwgGBhw6QGhgWwVhFGg6wWh_ : 1;
        osal_u32 reserved_3 : 2;
        osal_u32 xCt2OwtxGtO4yOsmx_ : 4;
        osal_u32 reserved_4 : 8;
        osal_u32 reserved_5 : 6;
        osal_u32 xCt2Oz4COGloOwsslOsmx_ : 2;
        osal_u32 xCt2OARsRtOGwmPOmo_ : 1;
        osal_u32 reserved_6 : 1;
        osal_u32 reserved_7 : 1;
        osal_u32 reserved_8 : 1;
    } bits;

    osal_u32 u32;
} u_cfg_agc_ctrl_add0;

typedef union {
    struct {
        osal_u32 x6NgwCGLdw6Cth6wLQ_ : 10;
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwFwCGLdw6Cth6wLQ_ : 10;
        osal_u32 xCt2Oz2CO4COtvwO4tmOw4QOyQGzss_ : 1;
        osal_u32 x6NgwGg6wdhoi6KwFBgwLG_gWLwWh_ : 1;
        osal_u32 x6NgwGg6wdhoi6Kw_BNCwCGLwWh_ : 1;
        osal_u32 x6Ngw6iG_CWwNBhWwGFbdCL_ : 1;
        osal_u32 reserved_1 : 4;
        osal_u32 xCt2OxozOCv4mOlolROmo_ : 1;
        osal_u32 reserved_2 : 1;
    } bits;

    osal_u32 u32;
} u_cfg_agc_ctrl_add1;

typedef union {
    struct {
        osal_u32 x6NgwhdSw6_iCGLdw6Cth6wLQ_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwFwhdSw6_iCGLdw6Cth6wLQ_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 x6NgwhdSwCGLdw6Cth6_ : 2;
        osal_u32 x6NgwFwhdSwCGLdw6Cth6_ : 2;
        osal_u32 reserved_2 : 12;
    } bits;

    osal_u32 u32;
} u_cfg_agc_ctrl_add2;

typedef union {
    struct {
        osal_u32 x6NgwGPwgBwFjSwGFbdCL_ : 6;
        osal_u32 xCt2O2lOCvwwOZGvqCvIG_ : 1;
        osal_u32 xCt2Oz8O2lOl6OsAltR_ : 3;
        osal_u32 x6Ngw66GwWFwCWo_ : 1;
        osal_u32 x6Ngw66GwSBFuKLwFWLwCWo_ : 1;
        osal_u32 x6NgwuW_VywLQ_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 x6NgwGPwSBFuKLwFWLwWh_ : 1;
        osal_u32 xCt2OtwmmpmOCCzOyGs_ : 1;
        osal_u32 reserved_1 : 2;
    } bits;

    osal_u32 u32;
} u_cca_mid_packet_det_cfg;

typedef union {
    struct {
        osal_u32 xCt2O2lCvwwzysO4yIORA_ : 8;
        osal_u32 x6NgwgB6i__GjCwLQ_ : 6;
        osal_u32 xCt2Oz8OwsslO2lCvwwO4lttORA_ : 6;
        osal_u32 x6NgwGPw6d__NhLwFBNNwLQ_ : 5;
        osal_u32 reserved_0 : 7;
    } bits;

    osal_u32 u32;
} u_cca_mid_packet_det_th;

typedef union {
    struct {
        osal_u32 xwGROwsslO3zoR_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_rpt_rssi_2ant;

typedef union {
    struct {
        osal_u32 xCt2OzoROCA2Osmx_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cfg_ant_chg_sel;

typedef union {
    struct {
        osal_u32 xCt2OovoOAROwsslOsmx_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_non_ht_rssi_sel;

typedef union {
    struct {
        osal_u32 x6NgwuT_wF6w_WSiqGo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_pwr_dc_removal;

typedef union {
    struct {
        osal_u32 x6NgwLQwjT_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_th_bw;

typedef union {
    struct {
        osal_u32 x6NgwNBoLW_wFot_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cca_fil_dly;

typedef union {
    struct {
        osal_u32 xCt2Om4OAQsRORA_ : 3;
        osal_u32 xCt2Ol4xmOGqwORA_ : 8;
        osal_u32 xCt2Om4OIlCwvOqzPmORA_ : 8;
        osal_u32 reserved_0 : 13;
    } bits;

    osal_u32 u32;
} u_ed_hyst_th;

typedef union {
    struct {
        osal_u32 x6NgwWFwQBgQwDyLQ_ : 8;
        osal_u32 xCt2Om4OAl2AOZ3RA_ : 8;
        osal_u32 x6Ngwu_BVywNBhWwLBSBhgwSG_gBh_ : 8;
        osal_u32 reserved_0 : 8;
    } bits;

    osal_u32 u32;
} u_ed_high_2040th;

typedef union {
    struct {
        osal_u32 xCt2Om4OxvqORAO4sss_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_ed_low_th_dsss;

typedef union {
    struct {
        osal_u32 x6NgwWFwoiTwLQwiNFS_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_ed_low_th_ofdm;

typedef union {
    struct {
        osal_u32 xR8OrryOovRCAOmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6Ngwu_BVywdh_WoBGjoWwLQ_ : 8;
        osal_u32 reserved_1 : 22;
    } bits;

    osal_u32 u32;
} u_eco_cfg;

typedef union {
    struct {
        osal_u32 x6NgwWFwoiTwLQwVygN_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_ed_low_th_20htgf;

typedef union {
    struct {
        osal_u32 xCt2Om4OxvqORAOK32t_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_ed_low_th_4080htgf;

typedef union {
    struct {
        osal_u32 x6NgwWFwoiTwLQwqQL_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_ed_low_th_vht;

typedef union {
    struct {
        osal_u32 x6NgwN_WWwG66dSwTBh_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_free_accum_win;

typedef union {
    struct {
        osal_u32 x6Ngw66GwiNFSwCth6wFWoGt_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cca_ofdm_header_delay;

typedef union {
    struct {
        osal_u32 x6NgwQLCLNwLQ_WCQioF_ : 9;
        osal_u32 x6NgwQLCLNwGg6wWh_ : 1;
        osal_u32 xCt2OCCzOxmoOtxz2Omo_ : 1;
        osal_u32 x6NgwQLCLNwTBh_ : 2;
        osal_u32 reserved_0 : 3;
        osal_u32 x6NgwQLCLNwG6LdGowLQ_ : 10;
        osal_u32 xCt2O4lsORqvOzoRO2zloOCAzo2mOyGs_ : 1;
        osal_u32 xCt2Oz2COZo4z2CO4z2COylROSGOmo_ : 1;
        osal_u32 xCt2Oz2COAmOISOZo4z2CO4z2COylROSGOIv4m_ : 2;
        osal_u32 reserved_1 : 2;
    } bits;

    osal_u32 u32;
} u_htstf_agc1;

typedef union {
    struct {
        osal_u32 x6Ngwu_WwFWoGtw00uxdC_ : 16;
        osal_u32 x6NgwFWoGtw0VdC_ : 16;
    } bits;

    osal_u32 u32;
} u_htstf_agc2;

typedef union {
    struct {
        osal_u32 x6NgwLPwFBgBLGowC6GoWwqGo_ : 9;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2OR8O4l2lRzxOsCzxmOsmx_ : 1;
        osal_u32 reserved_1 : 12;
        osal_u32 reserved_2 : 7;
    } bits;

    osal_u32 u32;
} u_digital_scale2;

typedef union {
    struct {
        osal_u32 x6NgwuiTW_yw_WNwy6Q_ : 8;
        osal_u32 xCt2OGvqmwrOwmtO3CA_ : 8;
        osal_u32 x6NgwuCFwiNFSw6GowLBSW_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_power0_ref_0ch;

typedef union {
    struct {
        osal_u32 xCt2ORGCORzyxmOzCCmssOz44w_ : 8;
        osal_u32 reserved_0 : 16;
        osal_u32 x6NgwBhFB_W6Lw_TwSiFW_ : 1;
        osal_u32 reserved_1 : 3;
        osal_u32 xCt2ORGCOwzIOGmwlv4_ : 1;
        osal_u32 reserved_2 : 3;
    } bits;

    osal_u32 u32;
} u_tpc_ram_access_reg1;

typedef union {
    struct {
        osal_u32 x6NgwLu6wLGjoWwT_wFGLG_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_tpc_ram_access_reg2;

typedef union {
    struct {
        osal_u32 xwGRORGCORzyxmOw4O4zRz_ : 26;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_tpc_ram_access_reg3;

typedef union {
    struct {
        osal_u32 x6NgwLu6wjidhFG_twV_ : 8;
        osal_u32 xCt2ORGCOyvSo4zwQOr_ : 8;
        osal_u32 x6NgwLu6wjidhFG_twy_ : 8;
        osal_u32 xCt2O4yyOsCzxmOsAz4vqOmwsSr3Y_ : 8;
    } bits;

    osal_u32 u32;
} u_tpc_boundary_threshold1;

typedef union {
    struct {
        osal_u32 xCt2ORGCOyvSo4zwQOY_ : 8;
        osal_u32 x6NgwLu6wjidhFG_twx_ : 8;
        osal_u32 xCt2ORGCOyvSo4zwQOK_ : 8;
        osal_u32 x6NgwLu6wjidhFG_twk_ : 8;
    } bits;

    osal_u32 u32;
} u_tpc_boundary_threshold2;

typedef union {
    struct {
        osal_u32 x6NgwFjjwC6GoWwCQGFiTwk_ : 8;
        osal_u32 x6NgwFjjwC6GoWwCQGFiTwV_ : 8;
        osal_u32 xCt2O4yyOsCzxmOsAz4vqOr_ : 8;
        osal_u32 x6NgwFjjwC6GoWwCQGFiTwy_ : 8;
    } bits;

    osal_u32 u32;
} u_dbb_scale_shadow1;

typedef union {
    struct {
        osal_u32 x6NgwFjjwC6GoWwCQGFiTwp_ : 8;
        osal_u32 xCt2O4yyOsCzxmOsAz4vqOY_ : 8;
        osal_u32 x6NgwFjjwC6GoWwCQGFiTwx_ : 8;
        osal_u32 xCt2O4yyOsCzxmOsAz4vqOK_ : 8;
    } bits;

    osal_u32 u32;
} u_dbb_scale_shadow2;

typedef union {
    struct {
        osal_u32 x6NgwGF6wBRwCTBL6Q_ : 1;
        osal_u32 x6NgwGF6wBhLW_uioGLWFwjtuGCC_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cfg_adc_iq_switch;

typedef union {
    struct {
        osal_u32 x6Ngwhiu_BwiNFSwFWLwjtuGCC_ : 1;
        osal_u32 reserved_0 : 7;
        osal_u32 x6Ngwhiu_BwiNFSwFWLwLQ_ : 5;
        osal_u32 reserved_1 : 3;
        osal_u32 xCt2OovGwlOvt4IO4mROqlo_ : 3;
        osal_u32 x6Ngw66GwjTwjtuGCC_ : 1;
        osal_u32 xCt2OovGwlOvt4IO4mROtlwORA_ : 5;
        osal_u32 x6Ngwhiu_BwiNFSwFWLwNB_wTBh_ : 3;
        osal_u32 reserved_2 : 4;
    } bits;

    osal_u32 u32;
} u_cfg_nopri_ofdm_det;

typedef union {
    struct {
        osal_u32 xCt2OxmoORCdOsRzyxmORA_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_len_tck_stable_th;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozOCA2GR4yI3ORA_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm0_th;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohGw6QguLFjS0wLQ_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm1_th;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozOCA2GR4yIZORA_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm2_th;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozOCA2GR4yIiORA_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm3_th;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohGw6QguLFjSDwLQ_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm4_th;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozOCA2GR4yIaORA_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm5_th;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohGw6QguLFjSXwLQ_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna_chgptdbm6_th;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxoz3O2zlo_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna0_cfg;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohG0wgGBh_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna1_cfg;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozZO2zlo_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna2_cfg;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxoziO2zlo_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna3_cfg;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohGDwgGBh_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna4_cfg;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxozaO2zlo_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna5_cfg;

typedef union {
    struct {
        osal_u32 x6Ngw6QywBhLohGXwgGBh_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna6_cfg;

typedef union {
    struct {
        osal_u32 xCt2OCA3OloRxoz9O2zlo_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_ch0_intlna7_cfg;

typedef union {
    struct {
        osal_u32 x6NgwBhBLwGg6woi6KwLBSWC_ : 13;
        osal_u32 x6Ngwdhoi6KpwCth6wBhdCWwCWo_ : 1;
        osal_u32 reserved_0 : 18;
    } bits;

    osal_u32 u32;
} u_agc_optimize_ctrl;

typedef union {
    struct {
        osal_u32 xGg6woi6KwuiC_ : 1;
        osal_u32 xxvOGloOSoxvCd_ : 1;
        osal_u32 xAlOGloOSoxvCd_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_agc_lock_unlock_rpt;

typedef union {
    struct {
        osal_u32 x_NwLPwWhwy6Q_ : 1;
        osal_u32 x_Nw_PwWhwy6Q_ : 1;
        osal_u32 xGAQOR8OmoO3CA_ : 1;
        osal_u32 xGAQOw8OmoO3CA_ : 1;
        osal_u32 xwtOR8OmoOrCA_ : 1;
        osal_u32 xwtOw8OmoOrCA_ : 1;
        osal_u32 xuQtwLPwWhw06Q_ : 1;
        osal_u32 xuQtw_PwWhw06Q_ : 1;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_rpt_trx_en;

typedef union {
    struct {
        osal_u32 xwGRO4COvttsmRO36_ : 10;
        osal_u32 xwGRO4COvttsmRO3l_ : 10;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_adc_dc_offset_0ch;

typedef union {
    struct {
        osal_u32 x_uLwF6wiNNCWLw0R_ : 10;
        osal_u32 x_uLwF6wiNNCWLw0B_ : 10;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_adc_dc_offset_1ch;

typedef union {
    struct {
        osal_u32 xz4COGloO3CA_ : 9;
        osal_u32 reserved_0 : 3;
        osal_u32 xxozOCv4mOvSRO3CA_ : 4;
        osal_u32 xP2zOCv4mOvSRO3CA_ : 6;
        osal_u32 reserved_1 : 2;
        osal_u32 x4l2lRzxOCv4mOvSRO3CA_ : 5;
        osal_u32 reserved_2 : 3;
    } bits;

    osal_u32 u32;
} u_ch0_adc_pin_code_rpt;

typedef union {
    struct {
        osal_u32 xGg6woi6KwLBSWCw_uL_ : 13;
        osal_u32 reserved_0 : 19;
    } bits;

    osal_u32 u32;
} u_agc_lock_time_rpt;

typedef union {
    struct {
        osal_u32 xCt2O4CvttsmROwz4zwORA_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_radar_agc_dc_th;

typedef union {
    struct {
        osal_u32 x6NgwFBCdhoi6Kw66Gw_NgGBhwLQ_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_disunlock_cca_rfgain;

typedef union {
    struct {
        osal_u32 xCt2O4lsSoxvCdOrZOGwlZ3wsslORA_ : 8;
        osal_u32 xCt2O4lsSoxvCdOiKOGwlZ3wsslORA_ : 8;
        osal_u32 xCt2O4lsSoxvCdOY9OGwlZ3wsslORA_ : 8;
        osal_u32 xCt2O4lsSoxvCdOncOGwlZ3wsslORA_ : 8;
    } bits;

    osal_u32 u32;
} u_disunlockj_pri20rssi_th;

typedef union {
    struct {
        osal_u32 xCt2O4l2lRzxO2zloOylRsOrry_ : 4;
        osal_u32 x6NgwFBgBLGowgGBhwjBLCwiNFSVyS_ : 3;
        osal_u32 xCt2O4l2lRzxO2zloOylRsOvt4IK3I_ : 3;
        osal_u32 xCt2O4l2lRzxO2zloOylRsOvt4In3I_ : 3;
        osal_u32 x6Ngw0CLwQLCLNwNBhBCQwCWo_ : 1;
        osal_u32 reserved_0 : 18;
    } bits;

    osal_u32 u32;
} u_digital_gain_bits;

typedef union {
    struct {
        osal_u32 xCt2O3CAOIloOxozOCv4m_ : 4;
        osal_u32 x6Ngwy6QwSGPwohGw6iFW_ : 4;
        osal_u32 reserved_0 : 8;
        osal_u32 xCt2O3CAOIloOP2zOCv4m_ : 6;
        osal_u32 reserved_1 : 2;
        osal_u32 x6Ngwy6QwSGPwqgGw6iFW_ : 6;
        osal_u32 reserved_2 : 2;
    } bits;

    osal_u32 u32;
} u_agc_ch0_debug_reg1;

typedef union {
    struct {
        osal_u32 xCt2O3CAOIz8O4l2lRzxOz2C_ : 5;
        osal_u32 x6Ngwy6QwFWNGdoLwohGw6iFW_ : 4;
        osal_u32 x6Ngwy6QwFWNGdoLwqgGw6iFW_ : 6;
        osal_u32 x6Ngwy6QwFWNGdoLwFBgBLGowgGBh_ : 5;
        osal_u32 x6Ngwy6QwGF6wLG_gWL_ : 9;
        osal_u32 reserved_0 : 3;
    } bits;

    osal_u32 u32;
} u_agc_ch0_debug_reg2;

typedef union {
    struct {
        osal_u32 xCt2O3CAOszROz2COsRmGOsmC_ : 9;
        osal_u32 xCt2O3CAOszROz2COsRmG_ : 9;
        osal_u32 reserved_0 : 9;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_agc_ch0_debug_reg4;

typedef union {
    struct {
        osal_u32 x6Ngw_NwuuGw6iFWwodL0_ : 16;
        osal_u32 xCt2OwtOGGzOCv4mOxSR3_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut0;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSRi_ : 16;
        osal_u32 xCt2OwtOGGzOCv4mOxSRZ_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut1;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSRa_ : 16;
        osal_u32 x6Ngw_NwuuGw6iFWwodLD_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut2;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSR9_ : 16;
        osal_u32 x6Ngw_NwuuGw6iFWwodLX_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut3;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSRc_ : 16;
        osal_u32 x6Ngw_NwuuGw6iFWwodLU_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut4;

typedef union {
    struct {
        osal_u32 x6Ngw_NwuuGw6iFWwodL00_ : 16;
        osal_u32 xCt2OwtOGGzOCv4mOxSRr3_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut5;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSRri_ : 16;
        osal_u32 xCt2OwtOGGzOCv4mOxSRrZ_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut6;

typedef union {
    struct {
        osal_u32 xCt2OwtOGGzOCv4mOxSRra_ : 16;
        osal_u32 x6Ngw_NwuuGw6iFWwodL0D_ : 16;
    } bits;

    osal_u32 u32;
} u_rf_ppa_code_lut7;

typedef union {
    struct {
        osal_u32 xCt2OwtOGzOCv4mOxSRi_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRZ_ : 8;
        osal_u32 x6Ngw_NwuGw6iFWwodL0_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSR3_ : 8;
    } bits;

    osal_u32 u32;
} u_rf_pa_code_lut0;

typedef union {
    struct {
        osal_u32 xCt2OwtOGzOCv4mOxSR9_ : 8;
        osal_u32 x6Ngw_NwuGw6iFWwodLX_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRa_ : 8;
        osal_u32 x6Ngw_NwuGw6iFWwodLD_ : 8;
    } bits;

    osal_u32 u32;
} u_rf_pa_code_lut1;

typedef union {
    struct {
        osal_u32 x6Ngw_NwuGw6iFWwodL00_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRr3_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRc_ : 8;
        osal_u32 x6Ngw_NwuGw6iFWwodLU_ : 8;
    } bits;

    osal_u32 u32;
} u_rf_pa_code_lut2;

typedef union {
    struct {
        osal_u32 xCt2OwtOGzOCv4mOxSRra_ : 8;
        osal_u32 x6Ngw_NwuGw6iFWwodL0D_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRri_ : 8;
        osal_u32 xCt2OwtOGzOCv4mOxSRrZ_ : 8;
    } bits;

    osal_u32 u32;
} u_rf_pa_code_lut3;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xwGROsmCZ3OCCz_ : 1;
        osal_u32 x_uLwu_BVyw66G_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xwGROsmCZ3OCCzOsRz_ : 1;
        osal_u32 x_uLwu_BVyw66GwCLG_ : 1;
        osal_u32 reserved_3 : 9;
        osal_u32 xz4COGloOqyO3CA_ : 9;
        osal_u32 reserved_4 : 6;
    } bits;

    osal_u32 u32;
} u_cca_det;

typedef union {
    struct {
        osal_u32 xwGROGwlZ3Ol4xmOGqw_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pri20_idle_pwr;

typedef union {
    struct {
        osal_u32 x_uLwu_BDywBFoWwuT__ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_pri40_idle_pwr;

typedef union {
    struct {
        osal_u32 xwGRO4yIOZ3Gwl_ : 8;
        osal_u32 x_uLwFjSwVyCW6_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rpt_dbm_cca;

typedef union {
    struct {
        osal_u32 xwGROwsslOIqv_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_rpt_rssi_mwo;

typedef union {
    struct {
        osal_u32 x6NgwQWCBgjwW__wLQ_ : 8;
        osal_u32 x6NgwQWCBgjwW__wjtuGCC_ : 1;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_hesigb_err_reg;

typedef union {
    struct {
        osal_u32 xwsslOtlomORlIlo2OzoR3_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 x_CCBwQLwCLNwGhLy_ : 8;
        osal_u32 reserved_1 : 8;
    } bits;

    osal_u32 u32;
} u_rpt_rssi_fine_timing_ht_stf;

typedef union {
    struct {
        osal_u32 xCt2OCCzOIv4mOloOSsm_ : 1;
        osal_u32 xCt2O4lsRl2OqltlOyQGzss_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cca_mode_in_use;

typedef union {
    struct {
        osal_u32 xCt2OGwlZ3OsRORA_ : 8;
        osal_u32 x6NgwCW6VywCLwLQ_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cca_ed_hyst_th;

typedef union {
    struct {
        osal_u32 x6Ngwu_BVywWFwjBS_wLQ_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OsmCZ3Om4OylIwORA_ : 3;
        osal_u32 reserved_1 : 25;
    } bits;

    osal_u32 u32;
} u_cca_ed_hyst_step;

typedef union {
    struct {
        osal_u32 xwGROyzo4IiOl4xmOGqw_ : 8;
        osal_u32 x_uLwjGhFS0wBFoWwuT__ : 8;
        osal_u32 x_uLwjGhF0wBFoWwuT__ : 8;
        osal_u32 xwGROyzo4iOl4xmOGqw_ : 8;
    } bits;

    osal_u32 u32;
} u_per20_idle_pwr;

typedef union {
    struct {
        osal_u32 x6Ngw66Gw_WGoLBSWw_CCBwCWo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cca_realtime_rssi_sel;

typedef union {
    struct {
        osal_u32 xCt2OzClO4mROmo_ : 1;
        osal_u32 xCt2OzClO4mROsRzwRORlIm_ : 1;
        osal_u32 xCt2OxGtOzRRmoOZ3I_ : 6;
        osal_u32 x6NgwouNwGLLWhwDyS_ : 6;
        osal_u32 xCt2OzClO4mROoSIORA_ : 7;
        osal_u32 x6NgwG6BwGPwjTDySwjuC_ : 1;
        osal_u32 x6NgwG6BwGjhi_SGow_uLwWh_ : 1;
        osal_u32 reserved_0 : 9;
    } bits;

    osal_u32 u32;
} u_aci_det_0;

typedef union {
    struct {
        osal_u32 x6NgwG6BwFBNNwLQ_ : 8;
        osal_u32 x6NgwG6BwhiBCWwVLQ_ : 8;
        osal_u32 xCt2OzClOovlsmOrRA_ : 8;
        osal_u32 xCt2OIloOGwlOyq_ : 8;
    } bits;

    osal_u32 u32;
} u_aci_det_1;

typedef union {
    struct {
        osal_u32 xCt2OzClO4mROtwIOoSI_ : 7;
        osal_u32 x6NgwTBLQwG6BwLQ_ : 7;
        osal_u32 xCt2OqlRAvSROzClORA_ : 7;
        osal_u32 reserved_0 : 11;
    } bits;

    osal_u32 u32;
} u_aci_det_2;

typedef union {
    struct {
        osal_u32 x_uLwuiTwG6B_ : 8;
        osal_u32 xwGROGvqOzClOloRw_ : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_aci_det_3;

typedef union {
    struct {
        osal_u32 x6NgwGg6wBh6iSBhgwGdLiw6gwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_agc_auto_cg_en;

#endif
