// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK4_C_UNION_DEFINE_H__
#define __PHY_REG_BANK4_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 x6Ngw66Gw00jwQWGFW_wFWoGt_ : 5;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_cca_11b_header_delay;

typedef union {
    struct {
        osal_u32 xCt2OR8OrryOK3IOCvmtr_ : 10;
        osal_u32 reserved_0 : 7;
        osal_u32 x6NgwLPw00jwDySw6iWNy_ : 10;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_elevenb_40m_coef0_1;

typedef union {
    struct {
        osal_u32 x6NgwLPw00jwDySw6iWNk_ : 10;
        osal_u32 reserved_0 : 7;
        osal_u32 x6NgwLPw00jwDySw6iWNV_ : 10;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_elevenb_40m_coef2_3;

typedef union {
    struct {
        osal_u32 x6NgwLPw00jwDySw6iWNx_ : 10;
        osal_u32 reserved_0 : 7;
        osal_u32 xCt2OR8OrryOK3IOCvmtK_ : 10;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_elevenb_40m_coef4_5;

typedef union {
    struct {
        osal_u32 x6NgwLPw00jwDySw6iWNp_ : 10;
        osal_u32 reserved_0 : 7;
        osal_u32 xCt2OR8OrryOK3IOCvmtY_ : 10;
        osal_u32 reserved_1 : 5;
    } bits;

    osal_u32 u32;
} u_elevenb_40m_coef6_7;

typedef union {
    struct {
        osal_u32 xCt2OR8OrryOK3IOCvmtn_ : 10;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_elevenb_40m_coef8;

typedef union {
    struct {
        osal_u32 x4mxzQORlImOrry_ : 12;
        osal_u32 xG6RwSdoL_ : 4;
        osal_u32 x6NgwG6Rw6iG_CWwCLwLBSW_ : 6;
        osal_u32 xCt2OvomzoRO4mROIwCOmoy_ : 1;
        osal_u32 x6Ngw_WCWLwGoo_ : 1;
        osal_u32 xCt2OSoxvCdm4_ : 1;
        osal_u32 xCt2OGvqO2zloOIwCOyQGzss_ : 1;
        osal_u32 x6Ngw00jwjidhFG_twgGBh_ : 6;
    } bits;

    osal_u32 u32;
} u_delay_time_11b;

typedef union {
    struct {
        osal_u32 x6NgwNB_hPw00jwCWo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_11b;

typedef union {
    struct {
        osal_u32 txcntrlout : 8;
        osal_u32 txlengthhighout : 1;
        osal_u32 rxcntrlout : 8;
        osal_u32 x6NgwLP00joi6KWFdhoi6KWF_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_cfg1_11b;

typedef union {
    struct {
        osal_u32 rxstatusin : 8;
        osal_u32 rxstatusstrbn : 1;
        osal_u32 x_uLwidL00j_PSGBhNCS_ : 4;
        osal_u32 x_uLwidL00jQF_6QK_NCS_ : 4;
        osal_u32 x_uLwidL00j_PFWSiFNCS_ : 3;
        osal_u32 xwGROvSRrryloRmwGRlImCvww_ : 6;
        osal_u32 reserved_0 : 6;
    } bits;

    osal_u32 u32;
} u_cfg2_11b;

typedef union {
    struct {
        osal_u32 rxlengthin : 12;
        osal_u32 xwGROvSRrryoCvsmm4_ : 16;
        osal_u32 reserved_0 : 4;
    } bits;

    osal_u32 u32;
} u_cfg3_11b;

typedef union {
    struct {
        osal_u32 acqthreshout : 8;
        osal_u32 acqtimeoutvla : 8;
        osal_u32 snrrakeupperthout : 8;
        osal_u32 snrrakelowerthout : 8;
    } bits;

    osal_u32 u32;
} u_cfg4_11b;

typedef union {
    struct {
        osal_u32 mpathrakethout : 8;
        osal_u32 tmcntrl1out : 8;
        osal_u32 tmcntrl2out : 8;
        osal_u32 csbarkerth11out : 8;
    } bits;

    osal_u32 u32;
} u_cfg5_11b;

typedef union {
    struct {
        osal_u32 csbarkerth12out : 8;
        osal_u32 csbarkerth13out : 8;
        osal_u32 csbarkerth14out : 8;
        osal_u32 csbarkerth15out : 8;
    } bits;

    osal_u32 u32;
} u_cfg6_11b;

typedef union {
    struct {
        osal_u32 csbarkerth16out : 8;
        osal_u32 csbarkerth21out : 8;
        osal_u32 csbarkerth22out : 8;
        osal_u32 csbarkerth23out : 8;
    } bits;

    osal_u32 u32;
} u_cfg7_11b;

typedef union {
    struct {
        osal_u32 csbarkerth24out : 8;
        osal_u32 csbarkerth25out : 8;
        osal_u32 csbarkerth26out : 8;
        osal_u32 csbarkerth27out : 8;
    } bits;

    osal_u32 u32;
} u_cfg8_11b;

typedef union {
    struct {
        osal_u32 rakeestdelayout : 8;
        osal_u32 antiphasehangupreg : 8;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cfg9_11b;

typedef union {
    struct {
        osal_u32 rakemultipathcountreg : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_cfg10_11b;

typedef union {
    struct {
        osal_u32 outtxprefiltcoeff0 : 10;
        osal_u32 outtxprefiltcoeff1 : 10;
        osal_u32 outtxprefiltcoeff2 : 10;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_cfg11_11b;

typedef union {
    struct {
        osal_u32 outtxprefiltcoeff3 : 10;
        osal_u32 outtxprefiltcoeff4 : 10;
        osal_u32 outtxprefiltcoeff5 : 10;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_cfg12_11b;

typedef union {
    struct {
        osal_u32 x_PQuNwV6iWN_ : 10;
        osal_u32 xw8AGtOCvmt_ : 10;
        osal_u32 outtxprefiltcoeff6 : 10;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_cfg13_11b;

typedef union {
    struct {
        osal_u32 x6NgwS_6w00jwWhj_ : 1;
        osal_u32 x6Ngw00jw_PwoWhgLQwSGP_ : 12;
        osal_u32 reserved_0 : 19;
    } bits;

    osal_u32 u32;
} u_cfg14_11b;

typedef union {
    struct {
        osal_u32 xCt2Oz2CO2zloORAOtvwOIwC_ : 8;
        osal_u32 xCt2OrryOst4OIwCO4ls_ : 1;
        osal_u32 x6NgwCBghGowuiTwLQ_F_ : 8;
        osal_u32 xCt2OzoR3OzC6OwmsmR_ : 1;
        osal_u32 x6NgwGhL0wG6Rw_WCWL_ : 1;
        osal_u32 xCt2OCA3OGvqOsmx_ : 2;
        osal_u32 xCt2OCvwwORAw_ : 4;
        osal_u32 reserved_0 : 7;
    } bits;

    osal_u32 u32;
} u_cfg15_11b;

typedef union {
    struct {
        osal_u32 xCt2OrryOwmsmwPm4Owm2_ : 9;
        osal_u32 reserved_0 : 23;
    } bits;

    osal_u32 u32;
} u_cfg16_11b;

typedef union {
    struct {
        osal_u32 x6Ngw00jwDuGLQWRdw6hL_ : 11;
        osal_u32 x6Ngw00jwCoB6WwqGodW_ : 5;
        osal_u32 xCt2OrryOm6SOSG4zRmOCoR_ : 4;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_11b_set0;

typedef union {
    struct {
        osal_u32 xCt2OrryOloRtOIS3_ : 8;
        osal_u32 x6Ngw00jwBhLNwSd0_ : 8;
        osal_u32 xCt2OrryOloRtOISZ_ : 8;
        osal_u32 xCt2OrryOloRtOISi_ : 8;
    } bits;

    osal_u32 u32;
} u_11b_set1;

typedef union {
    struct {
        osal_u32 xCt2OrryOovoloRtOIS3_ : 8;
        osal_u32 x6Ngw00jwhihBhLNwSd0_ : 8;
        osal_u32 xCt2OrryOovoloRtOISZ_ : 8;
        osal_u32 xCt2OrryOovoloRtOISi_ : 8;
    } bits;

    osal_u32 u32;
} u_11b_set2;

typedef union {
    struct {
        osal_u32 xCt2OrryOsRmGz4WOoSI3_ : 4;
        osal_u32 x6Ngw00jwCLWuGFbwhdS0_ : 4;
        osal_u32 xCt2OrryOsRmGz4WOoSIZ_ : 4;
        osal_u32 xCt2OrryOsRmGz4WOoSIi_ : 4;
        osal_u32 xCt2OrryOz4zGsRmGOmozyxm_ : 1;
        osal_u32 x6Ngw00jw6oKoi6KwWhGjoW_ : 1;
        osal_u32 x6Ngw00jwN_WRwCQBNLwqGodW_ : 11;
        osal_u32 reserved_0 : 3;
    } bits;

    osal_u32 u32;
} u_11b_set3;

typedef union {
    struct {
        osal_u32 x6Ngw00jwuoowu_WGSjoWw6iWNy_ : 18;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_11b_set4;

typedef union {
    struct {
        osal_u32 xCt2OrryOGxxOGwmzIyxmOCvmtr_ : 15;
        osal_u32 reserved_0 : 2;
        osal_u32 x6Ngw00jwuoowu_WGSjoWw6iWNV_ : 15;
    } bits;

    osal_u32 u32;
} u_11b_set5;

typedef union {
    struct {
        osal_u32 xCt2OrryOGxxO4zRzOCvmt3_ : 18;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_11b_set6;

typedef union {
    struct {
        osal_u32 x6Ngw00jwuoowFGLGw6iWN0_ : 15;
        osal_u32 reserved_0 : 2;
        osal_u32 xCt2OrryOGxxO4zRzOCvmtZ_ : 15;
    } bits;

    osal_u32 u32;
} u_11b_set7;

typedef union {
    struct {
        osal_u32 x6Ngw00jwFoowjG_KW_w6iWNy_ : 17;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OrryO4xxOyzwdmwOCvmtr_ : 11;
    } bits;

    osal_u32 u32;
} u_11b_set8;

typedef union {
    struct {
        osal_u32 x6Ngw00jwFoow66Kw6iWNy_ : 17;
        osal_u32 reserved_0 : 4;
        osal_u32 xCt2OrryO4xxOCCdOCvmtr_ : 11;
    } bits;

    osal_u32 u32;
} u_11b_set9;

typedef union {
    struct {
        osal_u32 x6NgwLWFwSiFWwCWo_ : 1;
        osal_u32 xCt2OmzwxQOCCzOsmx_ : 1;
        osal_u32 reserved_0 : 15;
        osal_u32 xCt2OzC6OsRzwRORlIm_ : 7;
        osal_u32 xCt2OzC6O4xQ_ : 8;
    } bits;

    osal_u32 u32;
} u_11b_set10;

typedef union {
    struct {
        osal_u32 x6NgwWRdwFot_ : 9;
        osal_u32 x6NgwFWSiFwFot_ : 9;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_11b_set11;

typedef union {
    struct {
        osal_u32 x6Ngw_PouNkw6iWNwCWo_ : 1;
        osal_u32 x6NgwBhLNwNB_wWh_ : 1;
        osal_u32 xCt2Otxm8OtlwOmo_ : 1;
        osal_u32 x6NgwN_WRwCQBNLwjtuGCC_ : 1;
        osal_u32 xCt2OloRtOtxz2_ : 1;
        osal_u32 xCt2Om6SOyQGzss_ : 1;
        osal_u32 x6NgwFW6BCBihwSWLQiF_ : 1;
        osal_u32 xCt2OtvwCmORzGoSIOmo_ : 1;
        osal_u32 x6NgwNNwLGuwWh_ : 24;
    } bits;

    osal_u32 u32;
} u_11b_set12;

typedef union {
    struct {
        osal_u32 xCt2OtyORzGOmo_ : 9;
        osal_u32 xCt2OGxxOqzlRORlIm_ : 10;
        osal_u32 x6Ngwh6iwCLG_LwLBSW_ : 9;
        osal_u32 x6NgwduFGLWLWFwhdS_ : 4;
    } bits;

    osal_u32 u32;
} u_11b_set13;

typedef union {
    struct {
        osal_u32 x6Ngw6iWNwLQy_ : 8;
        osal_u32 xCt2OCvmtORAr_ : 8;
        osal_u32 x6Ngw6iWNwLQV_ : 8;
        osal_u32 xCt2OrryOISOsRmG_ : 3;
        osal_u32 xCt2OrryOISO4zRz_ : 5;
    } bits;

    osal_u32 u32;
} u_11b_set14;

typedef union {
    struct {
        osal_u32 xCt2Otl8OISOmo_ : 1;
        osal_u32 xCt2OISOSG4zRmOszIGxm_ : 10;
        osal_u32 xCt2OISOSG4zRmOszIGxmOloRt_ : 10;
        osal_u32 x6NgwSdoLBuGLQwhdS_ : 4;
        osal_u32 x6NgwSdoLBuGLQwFWLwhdS_ : 4;
        osal_u32 x6Ngwuj66w6QKw6L_o_ : 1;
        osal_u32 reserved_0 : 2;
    } bits;

    osal_u32 u32;
} u_11b_set15;

typedef union {
    struct {
        osal_u32 x_uLwuCFw_CCBwGqg_ : 8;
        osal_u32 xwGROP2zOCv4mOtvwOGs4_ : 6;
        osal_u32 xwGROxozOCv4mOtvwOGs4_ : 4;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_psd_cfg0;

typedef union {
    struct {
        osal_u32 xwGROGs4OvSR_ : 8;
        osal_u32 xwGROGs4OwsslOIz8_ : 8;
        osal_u32 xwGROGs4OwsslOIz8Olo4m8_ : 10;
        osal_u32 xwGROGs4Ovt4IOsl2_ : 1;
        osal_u32 x_uLwuCFw00jwCBg_ : 1;
        osal_u32 x_uLwuCFwjT_ : 3;
        osal_u32 x_uLwuCFwidLwqoF_ : 1;
    } bits;

    osal_u32 u32;
} u_psd_cfg1;

typedef union {
    struct {
        osal_u32 x6NgwduwNNLwWh_ : 1;
        osal_u32 xCt2Ow8OGs4Omo_ : 1;
        osal_u32 x6NgwuCFwWhL_twhdSwBh_ : 10;
        osal_u32 x6NgwLPwC6iwjtuGCC_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6Ngw_PwuCFwTBNBwTi_KwWh_ : 1;
        osal_u32 xCt2Ow8OGs4OttROszIG_ : 3;
        osal_u32 reserved_1 : 2;
        osal_u32 xCt2Ow8OGs4Ovt4IO4mROmo_ : 1;
        osal_u32 x6Ngw_PwuCFw00jwFWLwWh_ : 1;
        osal_u32 xCt2Ow8OGs4OoyO4mROmo_ : 1;
        osal_u32 xwGROoyOsl2oO4mR_ : 1;
        osal_u32 x6NgwuCFwNBhWwLBSBhgwFotwCWo_ : 2;
        osal_u32 xCt2OGs4OCxwOql4RA_ : 2;
        osal_u32 x6Ngw_PwuCFw_WCWL_ : 1;
        osal_u32 reserved_2 : 3;
    } bits;

    osal_u32 u32;
} u_psd_cfg2;

typedef union {
    struct {
        osal_u32 xCt2Ow8OGs4OyvRRvIOovlsm_ : 8;
        osal_u32 x6Ngw_PwuCFw_CCBwhj_ : 8;
        osal_u32 x6Ngw_PwuCFwLQWwuiTW_whj_ : 7;
        osal_u32 x6Ngw_PwuCFwLQWwhdSwhj_ : 9;
    } bits;

    osal_u32 u32;
} u_psd_cfg3;

typedef union {
    struct {
        osal_u32 xCt2O4vRyOsQoCOvdORlIm_ : 11;
        osal_u32 x6NgwuCFwFiLjw6GowLBSW_ : 11;
        osal_u32 xCt2Ovt4IOsQoCOvdORlIm_ : 10;
    } bits;

    osal_u32 u32;
} u_psd_cfg4;

typedef union {
    struct {
        osal_u32 xCt2OIqvORlImOCoRORA_ : 20;
        osal_u32 reserved_0 : 12;
    } bits;

    osal_u32 u32;
} u_mwo_det_time_th;

typedef union {
    struct {
        osal_u32 x6NgwSTiwCLG_Lw_CCBwLQ_ : 8;
        osal_u32 x6NgwSTiwWhFw_CCBwLQ_ : 8;
        osal_u32 x6NgwSTiwFWLwWh_ : 1;
        osal_u32 x6NgwSTiwuT_wCWo_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_mwo_det_rssi_th;

#endif
