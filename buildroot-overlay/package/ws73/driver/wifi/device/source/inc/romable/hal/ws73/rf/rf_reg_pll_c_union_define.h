/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: ws73 rf reg interface and struct adaption
 * Date: 2022-11-12
*/

#ifndef __RF_REG_PLL_C_UNION_DEFINE_H__
#define __RF_REG_PLL_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 xmoOGxx_              : 1;
        osal_u32 xGxxOwsROo_           : 1;
        osal_u32 xuoowSiFWwCWo_        : 1;
        osal_u32 xGxxO4l2Ox4OCdOmo_    : 1;
        osal_u32 xuoow6LwWhwFBg6oKwFBqV_ : 1;
        osal_u32 xWhwFBgw6oKw_WCW_qW_  : 1;
        osal_u32 xWhwFBgw6oKwFBqD_     : 1;
        osal_u32 xmoO4l2OCxdO4lPZ_     : 1;
        osal_u32 xmoO4l2OCxd_          : 1;
        osal_u32 x6oKwNi_6Wwih_        : 1;
        osal_u32 xGxxO4l2OCd2Omo_      : 1;
        osal_u32 xWhwuQGCWwCoBu_       : 1;
        osal_u32 xmoOGxxOx4_           : 1;
        osal_u32 xmoOPCvOCR_           : 1;
        osal_u32 xmoOzzC_              : 1;
        osal_u32 xWhwGG6w6L_           : 1;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll1_union;

typedef union {
    struct {
        osal_u32 xtwzCOoOIsy_ : 7;
        osal_u32 xBhLwh_    : 9;
        osal_u32            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll2_union;

typedef union {
    struct {
        osal_u32 xtwzCOoOxsy_ : 16;
        osal_u32            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll3_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_         : 3;
        osal_u32 xFwq6iwGLWSuwWh_ : 1;
        osal_u32 x4OPCvOyStOxvOmo_ : 1;
        osal_u32 x4OPCvOyStOGxxOmo_ : 1;
        osal_u32 xFwq6iwWh_       : 1;
        osal_u32 x4OylzsOmo_      : 1;
        osal_u32 x4Oo4lPOmo_      : 1;
        osal_u32 x4OCIGOmo_       : 1;
        osal_u32 x4OsQoCOmo_      : 1;
        osal_u32 x4O2IOmo_        : 1;
        osal_u32 xFwCuFwWh_       : 1;
        osal_u32 xFwFL6wWh_       : 1;
        osal_u32 xFw_WNjdNwWh_    : 1;
        osal_u32 testmode         : 1;
        osal_u32                  : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll4_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_            : 5;
        osal_u32 xFwq6iwLWCLwWh_     : 1;
        osal_u32 xFwgSwq_WNwLWCLwWh_ : 1;
        osal_u32 xFwCuFwLWCLwWh_     : 1;
        osal_u32 xFw6SuwWhwq6SuwLWCL_ : 1;
        osal_u32 xFwouNwWhwqLdhWwCWL_ : 1;
        osal_u32 x4OxGtOmoOPRSomORmsR_ : 1;
        osal_u32 x4OzRmIGORmsROmo_   : 1;
        osal_u32 x4ORmsRIS8Osmx_     : 3;
        osal_u32 x4OmoORmsRIS8_      : 1;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll5_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_               : 5;
        osal_u32 x4OwmtyStOwmsmwPm_     : 2;
        osal_u32 x4OwmtyStO4vSyxmwOmo_  : 1;
        osal_u32 xFw_WNjdNwFidjoW_wFWoGt_ : 4;
        osal_u32 xFw_WNjdNwL_BSwidLjdN_ : 2;
        osal_u32 xFw_WNjdNwL_BSwBhjdN_  : 2;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll6_union;

typedef union {
    struct {
        osal_u32 x4O4RCOyStOsq_ : 4;
        osal_u32 x4O4RCOwOsmx_ : 5;
        osal_u32 x4O4RCOwmsmwPm_ : 1;
        osal_u32 x4O4RCOyQGzss_ : 1;
        osal_u32 xFwFL6w6NBP_  : 5;
        osal_u32               : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll7_union;

typedef union {
    struct {
        osal_u32 x4OsG4OwOsxvGm_        : 4;
        osal_u32 xFwCuFwoFwWh_          : 2;
        osal_u32 xFwFL6wCth6wFotwCWo_   : 3;
        osal_u32 x4O4RCOsQoCOwOsmx_     : 3;
        osal_u32 x4O4RCOsQoCOwsRO4xQOsmx_ : 2;
        osal_u32 xFwFL6wCth6w6oKwBhq_   : 1;
        osal_u32 x4O4RCOsQoCOwsROmo_    : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll8_union;

typedef union {
    struct {
        osal_u32 x4OsG4OCZ_       : 3;
        osal_u32 xFwCuFw_V_       : 3;
        osal_u32 xFwCuFw6CV_      : 3;
        osal_u32 x4OsG4OCsr_      : 3;
        osal_u32 x_WCqwy_         : 1;
        osal_u32 x4OsG4Om42mOCd4lP_ : 1;
        osal_u32 xFwCuFwWFgWw6K_WN_ : 1;
        osal_u32 x4OsG4Ox4OIv4m_  : 1;
        osal_u32                  : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll9_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_       : 2;
        osal_u32 xFwgSwq_WNwCWo_ : 3;
        osal_u32 x4O2IOsmx_     : 3;
        osal_u32 x4O2IOlGRzR_   : 3;
        osal_u32 xFwgSwBjg_     : 3;
        osal_u32 x4OsG4Ox4O4mxzQ_ : 2;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll10_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_       : 6;
        osal_u32 x4OCGOlCG_     : 2;
        osal_u32 x4OCGOmo_      : 2;
        osal_u32 xFwuNFwNi_6WwFh_ : 1;
        osal_u32 x4OGt4OtvwCmOSG_ : 1;
        osal_u32 xFwuNFwWhwFh_  : 1;
        osal_u32 x4OGt4OmoOSG_  : 1;
        osal_u32 x4OGt4Omo_     : 2;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll11_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 5;
        osal_u32 x4OxGtO4zCOPsmR_        : 3;
        osal_u32 xFwouNwFG6wq_WNwCWowuLGL_ : 1;
        osal_u32 x4OxGtOzIGOlRwlI_       : 1;
        osal_u32 x4OxGtOmoOzIG_          : 2;
        osal_u32 x4OxGtOmoOsqtxx_        : 2;
        osal_u32 x4OxGtOmoOtxx_          : 2;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll12_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_ : 2;
        osal_u32 x4OxGtOwi_ : 2;
        osal_u32 xFwouNw_0_ : 2;
        osal_u32 x4OxGtOCK_ : 2;
        osal_u32 xFwouNw6k_ : 2;
        osal_u32 x4OxGtOCr_ : 2;
        osal_u32 xwmsPOr_ : 1;
        osal_u32 xFwouNw6y_ : 3;
        osal_u32          : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll13_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_         : 6;
        osal_u32 x4OCIGOGwmOCAzw2m_ : 1;
        osal_u32 x4OCIGOCzxOl_    : 3;
        osal_u32 x4OCIGOCzxOC_    : 3;
        osal_u32 x4OCoROsmxOm42m_ : 1;
        osal_u32 x4OCoROmo_       : 2;
        osal_u32                  : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll14_union;

typedef union {
    struct {
        osal_u32 xFwhFBqwuTwCWo_     : 8;
        osal_u32 xFwhFBqw6oKjdNwL_BS_ : 2;
        osal_u32 xFwhFBqw6oKVuNFwL_BS_ : 2;
        osal_u32 x4Oo4lPOCxdZ4sIORwlI_ : 2;
        osal_u32 xFwhFBqw6hLwFBqwCWo_ : 2;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll15_union;

typedef union {
    struct {
        osal_u32 xFwq6iwLGBowBhFw6L_BS_ : 4;
        osal_u32 xFwq6iwBjBGCwBuLGLwCWo_ : 4;
        osal_u32 x4OPCvOlylzsOly2Osmx_ : 4;
        osal_u32 x4OPCvOlylzsOwCORwlI_ : 2;
        osal_u32 x4OPCvOlylzsOtzsRvo_  : 2;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll16_union;

typedef union {
    struct {
        osal_u32 x4OPCvOCROylzsOsmx_    : 4;
        osal_u32 x4OPCvOCROylzsOmo_     : 1;
        osal_u32 xFwq6iwqG_wjBGCwWh_    : 1;
        osal_u32 x4OPCvOlylzsOyQGzssOmo_ : 1;
        osal_u32 xFwq6iwBjBGCwWh_       : 1;
        osal_u32 x4OPCvO2IOwRwlI_       : 3;
        osal_u32 xFwq6iwgSw6iduoWwjtuGCC_ : 1;
        osal_u32 x4OPCvOyStOxvORwlI_    : 2;
        osal_u32 x4OPCvOyStOGxxORwlI_   : 2;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll17_union;

typedef union {
    struct {
        osal_u32 xFwq6iwqG_wKBwL_BS_   : 3;
        osal_u32 xFwq6iwqG_wKuwL_BS_   : 3;
        osal_u32 xFwq6iwjdNwFLtw6towGFb_ : 5;
        osal_u32 xFwq6iwqG_wjBGCwCWo_  : 3;
        osal_u32 x4OPCvOPzwOylzsOtzsRvo_ : 2;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll18_union;

typedef union {
    struct {
        osal_u32 x4OPCvOwmsmwPm_             : 3;
        osal_u32 x4OPCvOylzsOzxqzQsOvo_      : 1;
        osal_u32 xFwq6iwGLWSuwFG6wqCWL_      : 3;
        osal_u32 x4OPCvOzRmIGOGwmxvz4OIv4mOmo_ : 1;
        osal_u32 xFwq6iwGLWSuwuLGLwCoiuW_    : 2;
        osal_u32 x4OPCvOzRmIGOGRzROIv4m_     : 1;
        osal_u32 x4OPCvOzRmIGO2o4Omo_        : 1;
        osal_u32 x4OPCvOzRmIGOwCORwlI_       : 2;
        osal_u32 x4OPCvOzRmIGOtzsRvo_        : 2;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll19_union;

typedef union {
    struct {
        osal_u32 reserve_ana1 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll20_union;

typedef union {
    struct {
        osal_u32 reserve_ana2 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll21_union;

typedef union {
    struct {
        osal_u32 reserve_ana3 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll22_union;

typedef union {
    struct {
        osal_u32 reserve_ana4 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll23_union;

typedef union {
    struct {
        osal_u32 reserve_ana5 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll24_union;

typedef union {
    struct {
        osal_u32 reserve_dig1 : 16;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll25_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_   : 4;
        osal_u32 tphaseslip : 6;
        osal_u32 twarmup    : 6;
        osal_u32            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll26_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_       : 7;
        osal_u32 xtwm6OoIywOzP2_ : 3;
        osal_u32 xIzoSzxOzP2OwSo_ : 1;
        osal_u32 xNCSwCLiuwCLGLW_ : 4;
        osal_u32 xNCSwCLiuwWh_  : 1;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll27_union;

typedef union {
    struct {
        osal_u32 xPCvOCRtOylROsmx_ : 3;
        osal_u32 xGG6w6LNwjBLwCWo_ : 3;
        osal_u32 xLoFw_dh_       : 5;
        osal_u32 xoFw6hLwLQ_     : 4;
        osal_u32 xSGhdGowoFw_dh_ : 1;
        osal_u32                 : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll28_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 4;
        osal_u32 xuoow6LwSGhdGowjGhFwq6i0_ : 11;
        osal_u32 xuoow6LwSGhdGowq6i0_    : 1;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll29_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 4;
        osal_u32 xGxxOCROIzoSzxOyzo4OPCvZ_ : 11;
        osal_u32 xGxxOCROIzoSzxOPCvZ_    : 1;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll30_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 2;
        osal_u32 xuoow6LwNBhWwGqgwLBSW__ : 4;
        osal_u32 xGxxOCROCvzwsmOzP2ORlImw_ : 4;
        osal_u32 xCROyzo4OvttsmR_        : 3;
        osal_u32 x6Lwuio_                : 1;
        osal_u32 xuoow6LwWhwNBhFwjGhFy_  : 1;
        osal_u32 xuoow6L6wjBLwCWo_       : 1;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll31_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 1;
        osal_u32 xGxxOCROqzlRORlImwOCRCZ_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6L6D_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6L6U_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6L60X_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6L6kV_ : 3;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll32_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 1;
        osal_u32 xGxxOCROqzlRORlImwOCRtZ_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6LND_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6LNU_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6LN0X_ : 3;
        osal_u32 xuoow6LwTGBLwLBSW_w6L60_ : 3;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll33_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                  : 1;
        osal_u32 x6Lw6L_owq6iw6L_          : 2;
        osal_u32 xuoow6LwCTGBLwLBSW_wq6iw6L_ : 3;
        osal_u32 xmoOCROsqzlROPCvOCR_      : 1;
        osal_u32 xCROCRwxOzzCOCR_          : 2;
        osal_u32 xGxxOCROsqzlRORlImwOzzCOCR_ : 3;
        osal_u32 xWhw6LwCTGBLwGG6w6L_      : 1;
        osal_u32 xuoow6LwTGBLwLBSW_w6LN0_  : 3;
        osal_u32                           : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll34_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_           : 3;
        osal_u32 xszIGxlo2OqzlRORlIm_ : 5;
        osal_u32 xRsmRRxmOtxxOCG_   : 8;
        osal_u32                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll35_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_          : 2;
        osal_u32 xNqw6oKw6L_o_     : 2;
        osal_u32 x4OGAzsmOsxlGOsRmG_ : 2;
        osal_u32 xFwuQGCWwCoBuwuio_ : 1;
        osal_u32 x4OsmxOIzsAOvw4mw_ : 1;
        osal_u32 x4Os4IvO4mxzQ_    : 2;
        osal_u32 xLdhWwBhLwhFBq_   : 2;
        osal_u32 xWhwBhLwFBLQW__   : 1;
        osal_u32 xWhwFCSwBhL_      : 1;
        osal_u32 xCFSwWh_          : 2;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll36_union;

typedef union {
    struct {
        osal_u32 xFwCBghwqCGSuwFWoGt_     : 2;
        osal_u32 xFL6w6iFWwCFSwSdowuiCLwWh_ : 2;
        osal_u32 xFL6w6iFWwCFSwSdowu_WwWh_ : 2;
        osal_u32 xCxmzwO4RCO2zloOCzx_     : 1;
        osal_u32 xCxmzwOCdwmtO4CC_        : 1;
        osal_u32 xwmxvz4O4RCO2zloOCzx_    : 1;
        osal_u32 xwmxvz4OCdwmtO4CC_       : 1;
        osal_u32 xmoOPvsOCzx_             : 2;
        osal_u32 xFL6wg6wWh_              : 2;
        osal_u32 xCdwmtO4CCOmo_           : 2;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll37_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_               : 4;
        osal_u32 xmoOCzxOzxxOvoCm_      : 1;
        osal_u32 xFwFL6w6iFWw6GowFWoGt_ : 3;
        osal_u32 xFwFL6w6iFWwFWoGt_     : 2;
        osal_u32 x4O4RCOylRs_           : 2;
        osal_u32 xtxlGOPvsOCzxOGvxzwlRQ_ : 1;
        osal_u32 xtxlGO4RCO2zloOGvxzwlRQ_ : 1;
        osal_u32 xNoBuwFL6w6Go_         : 1;
        osal_u32 xtxlGOCdwmtO4CCOCzx_   : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll38_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_              : 8;
        osal_u32 xCt2OzozO4zRzOxvCd_   : 1;
        osal_u32 xCt2O4RCO2zloOCzxOxvCd_ : 1;
        osal_u32 xCt2OCdwmtO4CCOxvCd_  : 1;
        osal_u32 xxstwO4lRAmwO2zlo_    : 4;
        osal_u32 xWhwoCN_wFBLQW__      : 1;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll39_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                : 4;
        osal_u32 xzxGAzOxIsOCdwmt4CCOxvq_ : 4;
        osal_u32 xGouQGwoSCw6K_WNF66wQBgQ_ : 4;
        osal_u32 xwmsPOr_                : 2;
        osal_u32 xF66w6GowGouQGw6ihL_io_ : 2;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll40_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xGouQGwoSCwFL6wg6woiT_ : 4;
        osal_u32 xzxGAzOxIsO4RCO2COAl2A_ : 4;
        osal_u32 xwmsPOr_              : 2;
        osal_u32 x4RCOCzxOzxGAzOCvoRwvx_ : 2;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll41_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xGouQGwqiCw6GowoiT_   : 4;
        osal_u32 xzxGAzOPvsOCzxOAl2A_  : 4;
        osal_u32 xwmsPOr_              : 2;
        osal_u32 xqiCw6GowGouQGw6ihL_io_ : 2;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll42_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_            : 1;
        osal_u32 x4Owm2OCdwmtO4CCOIsy_ : 15;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll43_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_            : 2;
        osal_u32 x4Owm2O4RCO2zloOCzx_ : 10;
        osal_u32 x4Owm2OCdwmtO4CCOxsy_ : 4;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll44_union;

typedef union {
    struct {
        osal_u32 xSGhdGow6K_WNwF66_         : 1;
        osal_u32 xFw_Wgw6K_WNwF66wSCjwSGhdGo_ : 15;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll45_union;

typedef union {
    struct {
        osal_u32 xSGhdGowFL6wgGBhw6Go_      : 1;
        osal_u32 x_WCqwy_                   : 1;
        osal_u32 xFw_WgwFL6wgGBhw6GowSGhdGo_ : 10;
        osal_u32 xFw_Wgw6K_WNwF66woCjwSGhdGo_ : 4;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll46_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFw_WgwFL6w6iFWwSGhdGo_ : 11;
        osal_u32 xSGhdGowFL6w6iFW_     : 1;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll47_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_ : 10;
        osal_u32 xC_GSwCSu_ : 6;
        osal_u32          : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll48_union;

typedef union {
    struct {
        osal_u32 xqzlRORlImOzxGAzOxIsO4RCO2COAl2A_ : 6;
        osal_u32 xTGBLwLBSWwGouQGwoSCw6K_WNF66wQBgQ_ : 6;
        osal_u32 xTGBLwLBSWwF66w6GowNB_CL_         : 4;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll49_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_                     : 5;
        osal_u32 xGxxOx4OqzlRORlIm_           : 5;
        osal_u32 xqzlRORlImOzxGAzOPvsOCzxOAl2A_ : 6;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll50_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_             : 8;
        osal_u32 xqzlRORlImOy2OCzxOvSR_ : 8;
        osal_u32                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll51_union;

typedef union {
    struct {
        osal_u32 x6LwjGhFw_j_ : 11;
        osal_u32 xx4OxvCdOCoR_ : 4;
        osal_u32 xx4O4l2O4vom_ : 1;
        osal_u32             : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll52_union;

typedef union {
    struct {
        osal_u32 xtsIOsRzRmOtlozx_    : 4;
        osal_u32 x_WCqwy_             : 1;
        osal_u32 xjgw6Gow_WGFtwNoGg_  : 1;
        osal_u32 x6oiCWw_WGFtwCSuwNoGg_ : 1;
        osal_u32 xCxvsmOwmz4QOtxxOtxz2_ : 1;
        osal_u32 xGAzsmOwmz4QOtxz2_   : 1;
        osal_u32 xPCvOCROmo4Otxz2_    : 1;
        osal_u32 xGG6w6LwWhFwNoGg_    : 1;
        osal_u32 xqzwIOwmz4QOtxz2_    : 1;
        osal_u32 x6LwWhFwNoGg_        : 1;
        osal_u32 x6LNwFihWwNoGg_      : 1;
        osal_u32 x6L6wFihWwNoGg_      : 1;
        osal_u32 xuoowoi6KwBhFB6GLi__ : 1;
        osal_u32                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll53_union;

typedef union {
    struct {
        osal_u32 xN_WRwGqg_ : 15;
        osal_u32 xGqgwWhF_ : 1;
        osal_u32          : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll54_union;

typedef union {
    struct {
        osal_u32 xN_WRwGqgwGFF_ : 5;
        osal_u32 x_WCqwy_     : 11;
        osal_u32              : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll55_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_               : 1;
        osal_u32 x4Owm2OCdwmtO4CCOIsyOwy_ : 15;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll56_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_               : 2;
        osal_u32 x4Owm2O4RCO2zloOCzxOwy_ : 10;
        osal_u32 x4Owm2OCdwmtO4CCOxsyOwy_ : 4;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll57_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_          : 5;
        osal_u32 x4Owm2O4RCOCv4mOwy_ : 11;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll58_union;

typedef union {
    struct {
        osal_u32 x_WCqwy_   : 6;
        osal_u32 xCFSiw_j_  : 9;
        osal_u32 xFWoLGwqw_j_ : 1;
        osal_u32            : 16;
    } bits;
    osal_u16 u16;
} u_rf_pll59_union;

#endif
