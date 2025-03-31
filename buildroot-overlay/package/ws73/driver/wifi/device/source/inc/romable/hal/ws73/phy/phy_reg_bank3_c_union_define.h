// ******************************************************************************
// Copyright : Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK3_C_UNION_DEFINE_H__
#define __PHY_REG_BANK3_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 xCt2OR8O4zRzOsmx_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cal_tx_data_sel;

typedef union {
    struct {
        osal_u32 x6NgwLPwF6wWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc_en;

typedef union {
    struct {
        osal_u32 x6NgwLPwF6wBqGodW_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_cal_tx_ivalue;

typedef union {
    struct {
        osal_u32 x6NgwLPwF6wRqGodW_ : 12;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_cal_tx_qvalue;

typedef union {
    struct {
        osal_u32 cfg_tone_en : 1;
        osal_u32 x6NgwLihWwGSu_ : 3;
        osal_u32 xCt2ORvomOIzsd_ : 2;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLihWwSiFWwdCW__ : 8;
        osal_u32 x6NgwLihWwCBgh_ : 1;
        osal_u32 iq_cali_delay : 12;
        osal_u32 reserved_1 : 1;
    } bits;

    osal_u32 u32;
} u_cal_tone_gen1;

typedef union {
    struct {
        osal_u32 x6NgwFdGowLihWwWh_ : 1;
        osal_u32 xCt2ORvomZOzIG_ : 3;
        osal_u32 x6NgwLihWVwSGCK_ : 2;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2ORvomZOIv4mOSsmw_ : 8;
        osal_u32 xCt2ORvomZOsl2o_ : 1;
        osal_u32 reserved_1 : 13;
    } bits;

    osal_u32 u32;
} u_cal_tone_gen2;

typedef union {
    struct {
        osal_u32 xCt2OzCCSIOmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cal_accum_en;
typedef union {
    struct {
        osal_u32 xCt2OR8ORvomOGoOloRPx_      : 16;
        osal_u32 xCt2Ow8ORvomOGoOloRPx_      : 16;
    } bits;

    osal_u32 u32;
} u_cal_tone_gen_ctrl2;
typedef union {
    struct {
        osal_u32 xwGROzCCSIOwmz4Q_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cal_accum_rpt_ready;

typedef union {
    struct {
        osal_u32 x6GoywG66dSwB_ : 31;
        osal_u32 xwGROzCCSIOlOwmz4Q_ : 1;
    } bits;

    osal_u32 u32;
} u_cal_accum_i;

typedef union {
    struct {
        osal_u32 x6GoywG66dSwR_ : 31;
        osal_u32 xwGROzCCSIO6Owmz4Q_ : 1;
    } bits;

    osal_u32 u32;
} u_cal_accum_q;

typedef union {
    struct {
        osal_u32 xCt2ORw8Ol6OCzxlOmo_ : 1;
        osal_u32 xCt2Ol6OsQIOoSI_ : 4;
        osal_u32 reserved_0 : 3;
        osal_u32 x6NgwL_PwjTwSiFW_ : 2;
        osal_u32 reserved_1 : 1;
        osal_u32 x6NgwL_PwBRwNNLwCQBNLwWh_ : 1;
        osal_u32 xwGROw8OttRO4vom_ : 1;
        osal_u32 xCt2ORw8Ol6OCzxlOIS8_ : 1;
        osal_u32 reserved_2 : 18;
    } bits;

    osal_u32 u32;
} u_cal_trx_iq_ctrl1;
typedef union {
    struct {
        osal_u32 xCt2OR8ORvomOGAzsmOCRwx_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2OR8ORvomOCxdOGAzsm_ : 2;
        osal_u32 reserved_1 : 2;
        osal_u32 x6NgwL_PwBRwSiFW_ : 1;
        osal_u32 reserved_2 : 23;
    } bits;

    osal_u32 u32;
} u_cal_tone_gen_ctrl1;

typedef union {
    struct {
        osal_u32 x6NgwqoFwN_WRwCWo_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 x6Ngw6GoBwBRwNNLwCB1W_ : 2;
        osal_u32 reserved_1 : 26;
    } bits;

    osal_u32 u32;
} u_cal_fft_sa_ctrl1;

typedef union {
    struct {
        osal_u32 x6Ngw6GoBw_PwGF6wCWo_ : 2;
        osal_u32 x6NgwCGwuQGCWwCWo_ : 2;
        osal_u32 xCt2OCzxlOwsslOz4CO2zlo_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 x6NgwFNWwoiiuw6QiiCW_ : 1;
        osal_u32 reserved_1 : 23;
    } bits;

    osal_u32 u32;
} u_cal_fft_sa_ctrl2;

typedef union {
    struct {
        osal_u32 index0 : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_index_0;

typedef union {
    struct {
        osal_u32 index1 : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_index_1;

typedef union {
    struct {
        osal_u32 index2 : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_index_2;

typedef union {
    struct {
        osal_u32 index3 : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_index_3;

typedef union {
    struct {
        osal_u32 xwGROttROl483O46_ : 16;
        osal_u32 xwGROttROl483O4l_ : 16;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_0;

typedef union {
    struct {
        osal_u32 x_uLwNNLwBFP0wFR_ : 16;
        osal_u32 x_uLwNNLwBFP0wFB_ : 16;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_1;

typedef union {
    struct {
        osal_u32 xwGROttROl48ZO46_ : 16;
        osal_u32 xwGROttROl48ZO4l_ : 16;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_2;

typedef union {
    struct {
        osal_u32 xwGROttROl48iO46_ : 16;
        osal_u32 xwGROttROl48iO4l_ : 16;
    } bits;

    osal_u32 u32;
} u_cal_trxiq_fft_3;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OCvIGOyQGzss_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2OR8Ol6OCvIGOsmxOIzo_ : 1;
        osal_u32 reserved_1 : 27;
    } bits;

    osal_u32 u32;
} u_cal_txiq_comp_ctrl;

typedef union {
    struct {
        osal_u32 x6NgwLu6wg_iduwLQ_WC0w6Qy_ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2ORGCO2wvSGORAwmsZOCA3_ : 7;
        osal_u32 reserved_1 : 1;
        osal_u32 xCt2ORGCO2wvSGORAwmsiOCA3_ : 7;
        osal_u32 reserved_2 : 9;
    } bits;

    osal_u32 u32;
} u_cal_txiq_comp_thres;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyywBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyyw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef0_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef1_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef2_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef3_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef4_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6QwuiCwjWLGwBSGg_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6QwuiCwjWLGw_WGo_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iq_pos_beta_comp;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOom2OymRzOlIz2_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOom2OymRzOwmzx_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iq_neg_beta_comp;

typedef union {
    struct {
        osal_u32 x6Ngw_PwBRwSBCSGL6QwuiCwjWLGwBSGg_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6Ngw_PwBRwSBCSGL6QwuiCwjWLGw_WGo_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_rx_iq_pos_beta_comp;

typedef union {
    struct {
        osal_u32 xCt2Ow8Ol6OIlsIzRCAOom2OymRzOlIz2_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ow8Ol6OIlsIzRCAOom2OymRzOwmzx_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_rx_iq_neg_beta_comp;
typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt33OsmRrOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt33OsmRrOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef0_set1_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNy0wCWL0wBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNy0wCWL0w_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef1_set1_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3ZOsmRrOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3ZOsmRrOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef2_set1_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3iOsmRrOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3iOsmRrOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef3_set1_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyDwCWL0wBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyDwCWL0w_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef4_set1_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyywCWLVwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyywCWLVw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef0_set2_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOsmRZOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOsmRZOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef1_set2_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVwCWLVwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVwCWLVw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef2_set2_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykwCWLVwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykwCWLVw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef3_set2_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOsmRZOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOsmRZOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef4_set2_ch0;

typedef union {
    struct {
        osal_u32 xCt2Ow8Ol6OCvIGOyQGzss_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2Ow8Ol6OCvIGOsmxOIzo_ : 1;
        osal_u32 reserved_1 : 27;
    } bits;

    osal_u32 u32;
} u_cal_cfg9;

typedef union {
    struct {
        osal_u32 x6Ngw_PBRwqgGwCWLwLQ0w6Qy_ : 6;
        osal_u32 reserved_0 : 2;
        osal_u32 x6Ngw_PBRwohGwCWLwLQ0w6Qy_ : 6;
        osal_u32 reserved_1 : 2;
        osal_u32 xCt2Ow8l6OIS8OIv4m_ : 1;
        osal_u32 reserved_2 : 15;
    } bits;

    osal_u32 u32;
} u_rxiq_vga_set_th;

typedef union {
    struct {
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt33OlIz2O3_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt33OwmzxO3_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_coef0_ch0;

typedef union {
    struct {
        osal_u32 x6NgwBRwSBCSGL6Qw6iWNy0wBSGgwy_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6NgwBRwSBCSGL6Qw6iWNy0w_WGowy_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_coef1_ch0;

typedef union {
    struct {
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt3ZOlIz2O3_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt3ZOwmzxO3_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_coef2_ch0;

typedef union {
    struct {
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt3iOlIz2O3_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ol6OIlsIzRCAOCvmt3iOwmzxO3_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_coef3_ch0;

typedef union {
    struct {
        osal_u32 x6NgwBRwSBCSGL6Qw6iWNyDwBSGgwy_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6NgwBRwSBCSGL6Qw6iWNyDw_WGowy_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_coef4_ch0;

typedef union {
    struct {
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNyywBSGgwy_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNyyw_WGowy_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_set1_coef0_ch0;

typedef union {
    struct {
        osal_u32 xCt2Ol6OIlsIzRCAOsmRrOCvmt3rOlIz2O3_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ol6OIlsIzRCAOsmRrOCvmt3rOwmzxO3_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_set1_coef1_ch0;

typedef union {
    struct {
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNyVwBSGgwy_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNyVw_WGowy_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_set1_coef2_ch0;

typedef union {
    struct {
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNykwBSGgwy_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 x6NgwBRwSBCSGL6QwCWL0w6iWNykw_WGowy_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_set1_coef3_ch0;

typedef union {
    struct {
        osal_u32 xCt2Ol6OIlsIzRCAOsmRrOCvmt3KOlIz2O3_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ol6OIlsIzRCAOsmRrOCvmt3KOwmzxO3_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_iqmis_cls_set1_coef4_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8O4COCvIGOyQGzss_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xCt2OR8O4COCvIGOsmxOIzo_ : 1;
        osal_u32 reserved_1 : 27;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc_comp_bypass;

typedef union {
    struct {
        osal_u32 x6NgwLPywF6yw6iSuwR_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPywF6yw6iSuwB_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc0_comp;

typedef union {
    struct {
        osal_u32 xCt2OR83O4CrOCvIGO6_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR83O4CrOCvIGOl_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc1_comp;

typedef union {
    struct {
        osal_u32 x6NgwLPywF6Vw6iSuwR_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPywF6Vw6iSuwB_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc2_comp;

typedef union {
    struct {
        osal_u32 xCt2OR8OGqwOCvIGOyQGzss_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cal_tx_pwr_comp_bypass;

typedef union {
    struct {
        osal_u32 xCt2OGz3OCzxOPzx_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cal_tx_pa_cal_val;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4COCvIGOyQGzss_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cal_rx_dc_comp_bypass;

typedef union {
    struct {
        osal_u32 xCt2Ow83O4COCvIGO6_ : 11;
        osal_u32 reserved_0 : 5;
        osal_u32 xCt2Ow83O4COCvIGOl_ : 11;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_cal_rx_dc_comp_value;

typedef union {
    struct {
        osal_u32 xwGRO4CO4yyOPzxSmOSG4zRmOtxz2_ : 28;
        osal_u32 reserved_0 : 4;
    } bits;

    osal_u32 u32;
} u_lna_dc_dbbcomp_58;

typedef union {
    struct {
        osal_u32 xFthwLPwuiTW_w6GoB_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_dyn_tx_power_cali;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWy_ : 16;
        osal_u32 xCt2OGxxOqwOz44w3_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_0;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmr_ : 16;
        osal_u32 x6NgwuoowT_wGFF_0_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_1;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWV_ : 16;
        osal_u32 xCt2OGxxOqwOz44wZ_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_2;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWk_ : 16;
        osal_u32 xCt2OGxxOqwOz44wi_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_3;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmK_ : 16;
        osal_u32 x6NgwuoowT_wGFF_D_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_4;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWx_ : 16;
        osal_u32 xCt2OGxxOqwOz44wa_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_5;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmY_ : 16;
        osal_u32 x6NgwuoowT_wGFF_X_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_6;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWp_ : 16;
        osal_u32 xCt2OGxxOqwOz44w9_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_7;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmn_ : 16;
        osal_u32 x6NgwuoowT_wGFF_U_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_8;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodWc_ : 16;
        osal_u32 xCt2OGxxOqwOz44wc_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_9;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodW0y_ : 16;
        osal_u32 xCt2OGxxOqwOz44wr3_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_10;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmrr_ : 16;
        osal_u32 x6NgwuoowT_wGFF_00_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_11;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodW0V_ : 16;
        osal_u32 xCt2OGxxOqwOz44wrZ_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_12;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodW0k_ : 16;
        osal_u32 xCt2OGxxOqwOz44wri_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_13;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwOPzxSmrK_ : 16;
        osal_u32 x6NgwuoowT_wGFF_0D_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_14;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wqGodW0x_ : 16;
        osal_u32 xCt2OGxxOqwOz44wra_ : 10;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_pll_wr_addr_value_15;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwO4mxzQi_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQZ_ : 8;
        osal_u32 x6NgwuoowT_wFWoGt0_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQ3_ : 8;
    } bits;

    osal_u32 u32;
} u_pll_wr_delay_set0;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwO4mxzQ9_ : 8;
        osal_u32 x6NgwuoowT_wFWoGtX_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQa_ : 8;
        osal_u32 x6NgwuoowT_wFWoGtD_ : 8;
    } bits;

    osal_u32 u32;
} u_pll_wr_delay_set1;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_wFWoGt00_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQr3_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQc_ : 8;
        osal_u32 x6NgwuoowT_wFWoGtU_ : 8;
    } bits;

    osal_u32 u32;
} u_pll_wr_delay_set2;

typedef union {
    struct {
        osal_u32 xCt2OGxxOqwO4mxzQra_ : 8;
        osal_u32 x6NgwuoowT_wFWoGt0D_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQri_ : 8;
        osal_u32 xCt2OGxxOqwO4mxzQrZ_ : 8;
    } bits;

    osal_u32 u32;
} u_pll_wr_delay_set3;

typedef union {
    struct {
        osal_u32 x6NgwuoowT_whdS_ : 4;
        osal_u32 xCt2OGxxOqwOmo_ : 1;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_pll_wr_num;

typedef union {
    struct {
        osal_u32 x_uLwLPwuiTW_woWqWo_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_dyn_tx_power_rpt;

typedef union {
    struct {
        osal_u32 xGvqmwOzCCSIO4mxzQ_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_tx_power_accum_delay;

typedef union {
    struct {
        osal_u32 xLPwuiTW_w_uLwiqW__ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_tx_power_rpt_over;

typedef union {
    struct {
        osal_u32 xWPLwBhwGF6w6oi6K_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_ext_in_adc_clock;

typedef union {
    struct {
        osal_u32 x_WgwLPwNBNiwWh_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x_Wgw_PwNBNiwWh_ : 1;
        osal_u32 reserved_1 : 29;
    } bits;

    osal_u32 u32;
} u_fifoctrl;

typedef union {
    struct {
        osal_u32 x_PwNBNiw_FwoGLWh6t_ : 4;
        osal_u32 xLPwNBNiw_FwoGLWh6t_ : 4;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_fifo_latency;

typedef union {
    struct {
        osal_u32 reserved_0 : 6;
        osal_u32 xR8OtltvOmIGRQO3CA_ : 1;
        osal_u32 xR8OtltvOzxIvsROtSxxO3CA_ : 1;
        osal_u32 xR8OtltvOtSxxO3CA_ : 1;
        osal_u32 xw8OtltvOmIGRQO3CA_ : 1;
        osal_u32 xw8OtltvOzxIvsROtSxxO3CA_ : 1;
        osal_u32 xw8OtltvOtSxxO3CA_ : 1;
        osal_u32 reserved_1 : 20;
    } bits;

    osal_u32 u32;
} u_fifo_status;

typedef union {
    struct {
        osal_u32 reserved_0 : 6;
        osal_u32 xR8OtltvOmIGRQOzxIOom2O3CA_ : 1;
        osal_u32 xR8OtltvOzxIvsROtSxxOzxIOom2O3CA_ : 1;
        osal_u32 xR8OtltvOtSxxOzxIOom2O3CA_ : 1;
        osal_u32 xw8OtltvOmIGRQOzxIOom2O3CA_ : 1;
        osal_u32 xw8OtltvOzxIvsROtSxxOzxIOom2O3CA_ : 1;
        osal_u32 xw8OtltvOtSxxOzxIOom2O3CA_ : 1;
        osal_u32 reserved_1 : 6;
        osal_u32 xqxzoO2moOtltvOSo4mwtxvq_ : 1;
        osal_u32 xqxzoO2moOtltvOvPmwtxvq_ : 1;
        osal_u32 xqxzoOszIGxmOtltvOvPmwtxvq_ : 1;
        osal_u32 xqxzoOszIGxmOtltvOmIGRQ_ : 1;
        osal_u32 reserved_2 : 10;
    } bits;

    osal_u32 u32;
} u_fifo_alarm;

typedef union {
    struct {
        osal_u32 xLPuiCLwy6iWNN_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_txpost_0coeff;

typedef union {
    struct {
        osal_u32 xR8GvsROrCvmtt_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_txpost_1coeff;

typedef union {
    struct {
        osal_u32 intxpostfiltbypassn : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_intxpostfiltbypassn;

typedef union {
    struct {
        osal_u32 xF6wFjjwN0wBB__ : 2;
        osal_u32 x4CO4yyORwzCdlo2OoSIOSxlom_ : 5;
        osal_u32 x4CO4yyORwzCdlo2OvoxQOtxz2_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xvoxlomO4CORwzCdlo2Omozyxm_ : 1;
        osal_u32 x4COCvIGOwtOwm2OSG4zRmOyQGzss_ : 1;
        osal_u32 xF6w6iSuw_Nw_WgwohGwduFGLWwjtuGCC_ : 1;
        osal_u32 reserved_1 : 6;
        osal_u32 reserved_2 : 14;
    } bits;

    osal_u32 u32;
} u_online_cfg0;

typedef union {
    struct {
        osal_u32 x6NgwuVSw_Pw_CLwFot_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg_gating_0;

typedef union {
    struct {
        osal_u32 xCt2OR84lPOmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cfg_txdiv;

typedef union {
    struct {
        osal_u32 xCt2Ow84lPOmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_cfg_rxdiv;

typedef union {
    struct {
        osal_u32 xCt2Ow84lPOsRzwRO4xQ_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2OzyyOCzxlO4xQ_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_rxdiv_start_over;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OGzOIv4mOrr2OrZI_ : 3;
        osal_u32 x6NgwuGwSiFWw00gw0US_ : 3;
        osal_u32 xCt2OGzOIv4mOrr2OZKI_ : 3;
        osal_u32 xCt2OGzOIv4mOrr2OiYI_ : 3;
        osal_u32 x6NgwuGwSiFWw00gwDUS_ : 3;
        osal_u32 xCt2OGzOIv4mOrr2OaKI_ : 3;
        osal_u32 x6NgwuGwSiFWw00hwyS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroOrICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hwVS6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00hwkS6CwVFDg_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_0_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwuGwSiFWw00hDySwkVS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCO3ICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6w0S6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCOZICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrryOrI_ : 3;
        osal_u32 x6NgwuGwSiFWw00jwVS_ : 3;
        osal_u32 xCt2OGzOIv4mOrryOa4aI_ : 3;
        osal_u32 xCt2OGzOIv4mOrryOrrI_ : 3;
        osal_u32 x6NgwuGwSiFWw00gwXS_ : 3;
        osal_u32 xCt2OGzOIv4mOrr2OcI_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_1_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OGzOIv4mOrroOYICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hwpS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroK3IO3ICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hDySw0S6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroK3IOZICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrroK3IOiICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hDySwDS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroK3IOaICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hDySwXS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroK3IO9ICsOZ4K2_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_2_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwuGwSiFWw00GPDySwyS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8K3IOrICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPDySwVS6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPDySwkS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8K3IOKICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPDySwxS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8K3IOYICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPDySwpS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrroOKICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00hwxS6CwVFDg_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_3_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 x6NgwuGwSiFWw00G6DySwxS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCK3IOYICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IO3ICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPVySw0S6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IOZICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IOiICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPVySwDS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IOaICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPVySwXS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IO9ICsOZ4K2_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_4_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OGzOIv4mOrrzCO9ICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6wUS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCOcICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCOr3ICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6w00S6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySwyS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCK3IOrICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySwVS6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySwkS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCK3IOKICsOZ4K2_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_5_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 x6NgwuGwSiFWwWh_ : 1;
        osal_u32 xCt2OGzOIv4mOPzx_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySwpS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCK3IOnICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySwcS6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6DySw0yS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCK3IOrrICsOZ4K2_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCOiICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6wDS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrzCOaICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00G6wXS6CwVFDg_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_6_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 20;
        osal_u32 xCt2OGzOIv4mOrrz8K3IOnICsOZ4K2_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPDySwcS6CwVFDg_ : 3;
        osal_u32 x6NgwuGwSiFWw00GPVySwUS6CwVFDg_ : 3;
        osal_u32 xCt2OGzOIv4mOrrz8Z3IOcICsOZ4K2_ : 3;
    } bits;

    osal_u32 u32;
} u_pa_mode_7_code;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4CvCOxoz3OP2zrO6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxoz3OP2zrOl_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGywqgGywR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGywqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set1;

typedef union {
    struct {
        osal_u32 x6Ngw_PwF6i6wohG0wqgG0wR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohG0wqgG0wB_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozrOP2z3O6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozrOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set2;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4CvCOxozZOP2zrO6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozZOP2zrOl_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGVwqgGywR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGVwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set3;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4CvCOxoziOP2zrO6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxoziOP2zrOl_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGkwqgGywR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGkwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set4;

typedef union {
    struct {
        osal_u32 x6Ngw_PwF6i6wohGDwqgG0wR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGDwqgG0wB_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozKOP2z3O6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozKOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set5;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4CvCOxozaOP2zrO6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozaOP2zrOl_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGxwqgGywR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGxwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set6;

typedef union {
    struct {
        osal_u32 x6Ngw_PwF6i6wohGXwqgG0wR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGXwqgG0wB_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozYOP2z3O6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxozYOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set7;

typedef union {
    struct {
        osal_u32 xCt2Ow8O4CvCOxoz9OP2zrO6_ : 8;
        osal_u32 xCt2Ow8O4CvCOxoz9OP2zrOl_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGpwqgGywR_ : 8;
        osal_u32 x6Ngw_PwF6i6wohGpwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rx_dcoc_comp_value_set8;

typedef union {
    struct {
        osal_u32 xwGROw84CO4mxRzO4CvCOxoz3OP2zrO6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxoz3OP2zrOl_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGywqgGywR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGywqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set0;

typedef union {
    struct {
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohG0wqgG0wR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohG0wqgG0wB_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozrOP2z3O6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozrOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set1;

typedef union {
    struct {
        osal_u32 xwGROw84CO4mxRzO4CvCOxozZOP2zrO6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozZOP2zrOl_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGVwqgGywR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGVwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set2;

typedef union {
    struct {
        osal_u32 xwGROw84CO4mxRzO4CvCOxoziOP2zrO6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxoziOP2zrOl_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGkwqgGywR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGkwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set3;

typedef union {
    struct {
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGDwqgG0wR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGDwqgG0wB_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozKOP2z3O6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozKOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set4;

typedef union {
    struct {
        osal_u32 xwGROw84CO4mxRzO4CvCOxozaOP2zrO6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozaOP2zrOl_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGxwqgGywR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGxwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set5;

typedef union {
    struct {
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGXwqgG0wR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGXwqgG0wB_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozYOP2z3O6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxozYOP2z3Ol_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set6;

typedef union {
    struct {
        osal_u32 xwGROw84CO4mxRzO4CvCOxoz9OP2zrO6_ : 8;
        osal_u32 xwGROw84CO4mxRzO4CvCOxoz9OP2zrOl_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGpwqgGywR_ : 8;
        osal_u32 x_uLw_PF6wFWoLGwF6i6wohGpwqgGywB_ : 8;
    } bits;

    osal_u32 u32;
} u_rxdc_delta_dcoc_value_set7;

typedef union {
    struct {
        osal_u32 xCt2OvoxlomOw84COmo_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 x6Ngw_PF6wFWoLGwF6i6wWh_ : 1;
        osal_u32 reserved_1 : 3;
        osal_u32 xCt2Ow84COyl24COCxwOwzIOmo_ : 1;
        osal_u32 reserved_2 : 3;
        osal_u32 xCt2OvoxlomOw84COlolR_ : 1;
        osal_u32 reserved_3 : 19;
    } bits;

    osal_u32 u32;
} u_cfg_online_rxdc_config0;

typedef union {
    struct {
        osal_u32 xCt2Ow84CO4mxRzO4CvCOsRmGOl_ : 8;
        osal_u32 xCt2Ow84CO4mxRzO4CvCOsRmGO6_ : 8;
        osal_u32 xCt2Ow84COyl24COyxvCdOIlooSIOylRs_ : 4;
        osal_u32 x6Ngw_PF6wFWoLGwF6i6wjoi6KwSBhhdSwjBLC_ : 4;
        osal_u32 x6Ngw_PF6wGg6wVhFwTGBLwLBSW_ : 8;
    } bits;

    osal_u32 u32;
} u_cfg_online_rxdc_config1;

typedef union {
    struct {
        osal_u32 x6Ngw_PF6wFWoLGwF6i6wduFwoBS_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 x6Ngw_PF6wFWoLGwF6i6wduFwLQ__ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_online_rxdc_config2;

typedef union {
    struct {
        osal_u32 xCt2Ow84CORwzCdlo2OIz8OyxvCds_ : 12;
        osal_u32 x6Ngw_PF6wL_G6KBhgwTBhwjBLC_ : 4;
        osal_u32 xCt2Ow84COPCvIGiOyl24CORAw_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_online_rxdc_config3;

typedef union {
    struct {
        osal_u32 x6NgwihoBhWw_PF6w_GSw_FwGFF__ : 7;
        osal_u32 reserved_0 : 1;
        osal_u32 xCt2OvoxlomOw84COwzIOw4Omo_ : 1;
        osal_u32 reserved_1 : 23;
    } bits;

    osal_u32 u32;
} u_cfg_online_rxdc_ram_rd_config;

typedef union {
    struct {
        osal_u32 xwGROw84CORwzCdlo2OwzIOw4OyxvCdOoSI_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xwGROw84CORwzCdlo2OyxvCdOoSI_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_rpt_rxdc_tracking_block_num;

typedef union {
    struct {
        osal_u32 xwGROw84CORwzCdlo2O6_ : 16;
        osal_u32 xwGROw84CORwzCdlo2Ol_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rxdc_tracking_result;

typedef union {
    struct {
        osal_u32 xwGROw84CORwzCdlo2OwzIOw4O6_ : 16;
        osal_u32 xwGROw84CORwzCdlo2OwzIOw4Ol_ : 16;
    } bits;

    osal_u32 u32;
} u_rpt_rxdc_tracking_ram_rdresult;

typedef union {
    struct {
        osal_u32 xwtOR8O4vqoORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xwtOR8OSGORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_tx_up_down_time;

typedef union {
    struct {
        osal_u32 xuQtwLPwFiThwLBSW_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xuQtwLPwduwLBSW_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_phy_tx_up_down_time;

typedef union {
    struct {
        osal_u32 x_NwWhw6L_owCWo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_rf_ctrl;

typedef union {
    struct {
        osal_u32 x_NwLPwWhwGFF__ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 x_Nw_PwWhwGFF__ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_en_addr;

typedef union {
    struct {
        osal_u32 xuGywywFGLG_ : 3;
        osal_u32 xGz3OrO4zRz_ : 3;
        osal_u32 xuGywVwFGLG_ : 3;
        osal_u32 xuGywkwFGLG_ : 3;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_pa_gain_code;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xSGC3OrO4zRz_ : 8;
        osal_u32 xdu6ywywFGLG_ : 8;
    } bits;

    osal_u32 u32;
} u_upc0_00_03_data;

typedef union {
    struct {
        osal_u32 xxGtO3O4zRz_ : 3;
        osal_u32 xouNw0wFGLG_ : 3;
        osal_u32 xxGtOZO4zRz_ : 3;
        osal_u32 xxGtOiO4zRz_ : 3;
        osal_u32 xouNwDwFGLG_ : 3;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_lpf_data;

typedef union {
    struct {
        osal_u32 xFG6w6oKwWPLwUyS_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_dac_clk_ext_cfg;

typedef union {
    struct {
        osal_u32 xGF6w6oKwgLwCWo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_adc_clk_gt_en_sel;

typedef union {
    struct {
        osal_u32 xwtOw8O4vqoORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xwtOw8OSGORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_rx_up_down_time;

typedef union {
    struct {
        osal_u32 xuQtw_PwFiThwLBSW_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xuQtw_PwduwLBSW_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_phy_rx_up_down_time;

typedef union {
    struct {
        osal_u32 xwtOR8O4vqoOIz8O4xQ_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xwtOw8O4vqoOIz8O4xQ_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_pa_dly;

typedef union {
    struct {
        osal_u32 xLu6wouNwFot_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xLu6wdu6wFot_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_reg_wr_delay_1;

typedef union {
    struct {
        osal_u32 xLu6wFG6wFot_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xLu6wuGwFot_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_reg_wr_delay_2;

typedef union {
    struct {
        osal_u32 xRGCOGzOIv4mO4xQ_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2OR84lPOsRzwRO4xQ_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_reg_wr_delay_3;

typedef union {
    struct {
        osal_u32 x6NgwSGPwLu6wFot_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xLu6wuGwjBGCw_WgwT_wFot_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_reg_wr_delay_4;

typedef union {
    struct {
        osal_u32 reserved_0 : 4;
        osal_u32 xRGCOwm2OGqwOxmPmx_ : 8;
        osal_u32 xLu6wuT_w6L_o_ : 1;
        osal_u32 xRGCOyQGzss_ : 1;
        osal_u32 xGzOIv4mOyQGzss_ : 1;
        osal_u32 reserved_1 : 17;
    } bits;

    osal_u32 u32;
} u_tpc_ctrl;

typedef union {
    struct {
        osal_u32 xSG6wCQdLwCoWWu_ : 2;
        osal_u32 x4zCOCRwx_ : 1;
        osal_u32 xR8ORmsROIv4m_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_rf_if_ctrl;

typedef union {
    struct {
        osal_u32 xwmsmwPOrOqwO4xQ_ : 10;
        osal_u32 xCt2OwmsmwPOrOqwOmo_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 x_WCW_qw0wT_wGFF__ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_1_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 x_WCW_qw0wT_wqGodWwy6Q_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_1_reg_2;

typedef union {
    struct {
        osal_u32 x_WCW_qwVwT_wFot_ : 10;
        osal_u32 x6Ngw_WCW_qwVwT_wWh_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 xwmsmwPOZOqwOz44w_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_2_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xwmsmwPOZOqwOPzxSmO3CA_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_2_reg_2;

typedef union {
    struct {
        osal_u32 x_WCW_qwkwT_wFot_ : 10;
        osal_u32 x6Ngw_WCW_qwkwT_wWh_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 xwmsmwPOiOqwOz44w_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_3_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xwmsmwPOiOqwOPzxSmO3CA_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_3_reg_2;

typedef union {
    struct {
        osal_u32 xwmsmwPOKOqwO4xQ_ : 10;
        osal_u32 xCt2OwmsmwPOKOqwOmo_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 x_WCW_qwDwT_wGFF__ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_4_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 x_WCW_qwDwT_wqGodWwy6Q_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_4_reg_2;

typedef union {
    struct {
        osal_u32 x_WCW_qwxwT_wFot_ : 10;
        osal_u32 x6Ngw_WCW_qwxwT_wWh_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 xwmsmwPOaOqwOz44w_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_5_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xwmsmwPOaOqwOPzxSmO3CA_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_5_reg_2;

typedef union {
    struct {
        osal_u32 xwmsmwPOYOqwO4xQ_ : 10;
        osal_u32 xCt2OwmsmwPOYOqwOmo_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 x_WCW_qwXwT_wGFF__ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_6_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 x_WCW_qwXwT_wqGodWwy6Q_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_6_reg_2;

typedef union {
    struct {
        osal_u32 x_WCW_qwpwT_wFot_ : 10;
        osal_u32 x6Ngw_WCW_qwpwT_wWh_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 xwmsmwPO9OqwOz44w_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_7_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 xwmsmwPO9OqwOPzxSmO3CA_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_7_reg_2;

typedef union {
    struct {
        osal_u32 xwmsmwPOnOqwO4xQ_ : 10;
        osal_u32 xCt2OwmsmwPOnOqwOmo_ : 1;
        osal_u32 reserved_0 : 5;
        osal_u32 x_WCW_qwUwT_wGFF__ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_8_reg_1;

typedef union {
    struct {
        osal_u32 reserved_0 : 16;
        osal_u32 x_WCW_qwUwT_wqGodWwy6Q_ : 16;
    } bits;

    osal_u32 u32;
} u_cfg_reserv_8_reg_2;

typedef union {
    struct {
        osal_u32 xRGCOGzOylzsOz44w_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_pa_bias_addr;

typedef union {
    struct {
        osal_u32 xGzOylzsO3O4zRz_ : 16;
        osal_u32 xuGwjBGCw0wFGLG_ : 16;
    } bits;

    osal_u32 u32;
} u_pa_bias;

typedef union {
    struct {
        osal_u32 xCt2OCvm8OIv4mOsmxmCROyGs_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 reserved_3 : 12;
        osal_u32 reserved_4 : 16;
    } bits;

    osal_u32 u32;
} u_cfg_coex_input_bps;

typedef union {
    struct {
        osal_u32 xCt2Oz4COsRzo4yQOCvm8OyQGzss_ : 1;
        osal_u32 xCt2OCvm8OwsROGAQOw8OmoOyQGzss_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_cfg_adc_standby_coex_bypass;

typedef union {
    struct {
        osal_u32 xCt2OwtOqwOloRmwPzx_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_rf_wr_interval;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyywCWLkwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyywCWLkw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef0_set3_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOsmRiOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3rOsmRiOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef1_set3_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVwCWLkwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNyVwCWLkw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;
    osal_u32 u32;
} u_tx_iqmis_cls_coef2_set3_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykwCWLkwBSGgwy_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPwBRwSBCSGL6Qw6iWNykwCWLkw_WGowy_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef3_set3_ch0;

typedef union {
    struct {
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOsmRiOlIz2O3_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OR8Ol6OIlsIzRCAOCvmt3KOsmRiOwmzxO3_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_tx_iqmis_cls_coef4_set3_ch0;

typedef union {
    struct {
        osal_u32 x6NgwLPywF6kw6iSuwR_ : 12;
        osal_u32 reserved_0 : 4;
        osal_u32 x6NgwLPywF6kw6iSuwB_ : 12;
        osal_u32 reserved_1 : 4;
    } bits;

    osal_u32 u32;
} u_cal_tx_dc3_comp;

typedef union {
    struct {
        osal_u32 xCt2OwtOxGtOyqOIv4m_ : 2;
        osal_u32 xCt2OwtOxGtOIv4mOmo_ : 1;
        osal_u32 xCt2Oz2CO4zRzOloOCAvvsm_ : 1;
        osal_u32 xCt2OtyOGqwOCzxCOGw4_ : 4;
        osal_u32 reserved_0 : 11;
        osal_u32 xCt2O4QoGqwOlZ6ZOCxsxvvGOmo_ : 1;
        osal_u32 xCt2Oz4COvw2ZCvIGOmo_ : 1;
        osal_u32 xCt2O4zCOCvIGZvw2Omo_ : 1;
        osal_u32 x6Ngw6iWPwGF6wFGLGwLBWywWh_ : 1;
        osal_u32 reserved_1 : 9;
    } bits;

    osal_u32 u32;
} u_cali_new_add_reg;

typedef union {
    struct {
        osal_u32 xwGROGqwOzCCSIOCoR_ : 21;
        osal_u32 reserved_0 : 11;
    } bits;

    osal_u32 u32;
} u_cali_new_add_reg_3;

typedef union {
    struct {
        osal_u32 xCt2O4zCOwsRoO4vqoORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2O4zCOwsRoOSGORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_tx_line_ctrl_up_down_time;

typedef union {
    struct {
        osal_u32 xCt2O4zCOsRzo4yQOmoOSGORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2O4zCOsRzo4yQOmoO4vqoORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_tx_line_ctrl_up_down_time1;

typedef union {
    struct {
        osal_u32 xCt2OlGzOmoO4vqoORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2OlGzOmoOSGORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_tx_line_ctrl_up_down_time2;

typedef union {
    struct {
        osal_u32 xCt2Oz4COwsRoO4vqoORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2Oz4COwsRoOSGORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_rx_line_ctrl_up_down_time;

typedef union {
    struct {
        osal_u32 xCt2Oz4COsRzo4yQOmoOSGORlIm_ : 10;
        osal_u32 reserved_0 : 6;
        osal_u32 xCt2Oz4COsRzo4yQOmoO4vqoORlIm_ : 10;
        osal_u32 reserved_1 : 6;
    } bits;

    osal_u32 u32;
} u_rf_rx_line_ctrl_up_down_time1;

typedef union {
    struct {
        osal_u32 x6Ngw6iWPwGF6w_CLhw6L_owduwLBSW_ : 10;
        osal_u32 x6Ngw6iWPwGF6w_CLhw6L_owFiThwLBSW_ : 10;
        osal_u32 x6Ngw6iWPwGF6wCLGhFjtw6L_owFiThwLBSW_ : 10;
        osal_u32 xCt2OqltlOyROz4COwsRoOCRwxOyQGzss_ : 1;
        osal_u32 xCt2OqltlOyROz4COsRzo4yQOCRwxOyQGzss_ : 1;
    } bits;

    osal_u32 u32;
} u_rf_rx_line_ctrl_coex;

typedef union {
    struct {
        osal_u32 x6NgwNjwuQGCWw_iLcywWh_ : 1;
        osal_u32 xCt2Ow84CO4CvCOPCvIGOP2zOCv4mORAw_ : 6;
        osal_u32 x6NgwoBhW6L_owouNwSiFW_ : 1;
        osal_u32 xCt2OxlomCRwxOxGtOIv4mOIzo_ : 1;
        osal_u32 xCt2OxlomCRwxOxGtOyq_ : 2;
        osal_u32 x6NgwoBhW6L_owouNwjTwSGh_ : 1;
        osal_u32 x6NgwoBhW6L_owNjwWh_ : 2;
        osal_u32 xCt2OxlomCRwxOtyOmoOIzo_ : 1;
        osal_u32 xCt2Ow84CO4CvCOl6Om8CAzo2m_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_rf_line_ctrl_reg;

#endif
