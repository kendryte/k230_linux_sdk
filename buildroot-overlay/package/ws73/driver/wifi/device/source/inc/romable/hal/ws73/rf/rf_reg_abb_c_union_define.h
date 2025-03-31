/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: ws73 rf reg interface and struct adaption
 * Date: 2022-11-12
*/

#ifndef __RF_REG_ABB_C_UNION_DEFINE_H__
#define __RF_REG_ABB_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 x4ORvGOwtOyzo4OxvCzx_ : 2;
        osal_u32 x_WCqwy_              : 1;
        osal_u32 xFwTjwSiFWwCWowoi6Go_ : 1;
        osal_u32 x4OqyOxGtOyqOxvCzx_   : 2;
        osal_u32 xwmsPOr_              : 2;
        osal_u32 xFwTjwouNwSiFWwoi6Go_ : 1;
        osal_u32 xFwTjwLPwWhwoi6Go_    : 1;
        osal_u32 x4OqyOlGzOmoOxvCzx_   : 1;
        osal_u32 xFwTjw_PwWhwoi6Go_    : 1;
        osal_u32 xFwTjwNjwWhwoi6Go_    : 2;
        osal_u32 x4OqyOtyOwCCzxOmo_    : 1;
        osal_u32 x4OqyOwtzyyOR8l6CzxOmo_ : 1;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb0_union;

typedef union {
    struct {
        osal_u32 x4OqyOGzOylzsOCRwxOxvCzx_ : 3;
        osal_u32 x_WCqwy_                : 1;
        osal_u32 xFwTjwouNwgGBhwoi6Go_   : 3;
        osal_u32 xwmsPOr_                : 9;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb1_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyO2zloOxvCzx_ : 3;
        osal_u32 x_WCqwy_            : 1;
        osal_u32 xFwTjwNjwuQGCWwoi6Go_ : 1;
        osal_u32 xwmsPOr_            : 3;
        osal_u32 x4OqyOP2zO2zloOxvCzx_ : 5;
        osal_u32 resv_2              : 3;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb2_union;

typedef union {
    struct {
        osal_u32 x4OqyOxozO2zloOxvCzx_ : 3;
        osal_u32 x_WCqwy_            : 13;
        osal_u32                     : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb3_union;

typedef union {
    struct {
        osal_u32 x4OqyO4CvCOlOxvCzx_ : 8;
        osal_u32 x4OqyO4CvCO6OxvCzx_ : 8;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb4_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtORmIGOCv4mOIzGOxvCzx_ : 1;
        osal_u32 x_WCqwy_                    : 3;
        osal_u32 xFwTjwiqW_wLWSuwu_Lwoi6Go_  : 1;
        osal_u32 xwmsPOr_                    : 11;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb5_union;

typedef union {
    struct {
        osal_u32 xFwLiuw_NwjGhFwSiFW_   : 1;
        osal_u32 x4OqyOIv4mOsmxOIv4m_   : 1;
        osal_u32 xFwTjwouNwjTwSiFW_     : 1;
        osal_u32 x_WCqwy_               : 1;
        osal_u32 x4OqyOxGtOIv4mOCRwxOyQ_ : 1;
        osal_u32 x4OqyOxGtOIv4mOIv4m_   : 1;
        osal_u32 x4OqyOR8OmoOIv4m_      : 1;
        osal_u32 xFwTjwBuGwWhwSiFW_     : 1;
        osal_u32 x4OqyOw8OmoOIv4m_      : 1;
        osal_u32 x4OqyOtyOmoOIv4m_      : 1;
        osal_u32 xwmsPOr_               : 2;
        osal_u32 xFwTjwuGwjBGCw6L_owSiFW_ : 1;
        osal_u32 x4OqyOxGtO2zloOIv4m_   : 1;
        osal_u32 xFwTjwNjwgGBhwSiFW_    : 1;
        osal_u32 x4OqyOtyOGAzsmOIv4m_   : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb6_union;

typedef union {
    struct {
        osal_u32 xFwTjwqgGwgGBhwSiFW_    : 1;
        osal_u32 xFwTjwohGwgGBhwSiFW_    : 1;
        osal_u32 x_WCqwy_                : 2;
        osal_u32 xFwTjwF6i6wBwSiFW_      : 1;
        osal_u32 xFwTjwF6i6wRwSiFW_      : 1;
        osal_u32 x4OqyORmIGOCv4mOIv4m_   : 1;
        osal_u32 x4OqyOvPmwORmIGOGwROIv4m_ : 1;
        osal_u32 xwmsPOr_                : 8;
        osal_u32                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb7_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOxv2moOssyOmo_          : 1;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZR8Omo_  : 1;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZw8Omo_  : 1;
        osal_u32 xFwTjw_NwoigWhwCCjwqFWLwWh_     : 1;
        osal_u32 xFwTjw_NwoigWhwoiThiBCWwWhwLiwoC_ : 1;
        osal_u32 xFwTjw_NwLPwBjgwuGwWh_          : 1;
        osal_u32 xFwTjw_NwLPwBjgwuuGwWh_         : 1;
        osal_u32 x4OqyOwtOR8OlGRzROGzOmo_        : 1;
        osal_u32 x4OqyOwtOR8OlGRzROGGzOmo_       : 1;
        osal_u32 x4OqyOwtOR8OGzOmo_              : 1;
        osal_u32 x4OqyOwtOR8OGzOylzsOmo_         : 1;
        osal_u32 x4OqyOwtOR8OGGzOmo_             : 1;
        osal_u32 x4OqyOwtOR8OGGzOylzsOmo_        : 1;
        osal_u32 x4OqyOwtOR8OSGCOmo_             : 1;
        osal_u32 xFwTjw_NwLPwuGwGFjwWh_          : 1;
        osal_u32 xFwTjw_NwLPwoiFBqwWh_           : 1;
        osal_u32                                 : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb8_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwWh_           : 1;
        osal_u32 x4OqyOwtOw8OxozOyStOmo_       : 1;
        osal_u32 xFwTjw_Nw_PwohGwCTwGiwWhwLiwoC_ : 1;
        osal_u32 x4OqyOwtOw8OylzsOzvOmo_       : 1;
        osal_u32 xFwTjw_NwL_PwL_PCTwWh_        : 1;
        osal_u32 xFwTjw_NwL_PwBR6GowWh_        : 1;
        osal_u32 xFwTjw_Nw_PwoiFBqwWh_         : 1;
        osal_u32 x4OqyOwtOw8OxvylzsOmo_        : 1;
        osal_u32 xFwTjw_Nw_PwLBGwWh_           : 1;
        osal_u32 xFwTjw_NwL_PwBR6GowBjBGCwWh_  : 1;
        osal_u32 x4OqyOwtORw8Ol6CzxOyStOmo_    : 1;
        osal_u32 x_WCqwy_                      : 1;
        osal_u32 x4OqxOGzOP4mROmo_             : 1;
        osal_u32 x4OqxOGGzOP4mROmo_            : 1;
        osal_u32 xwmsPOr_                      : 2;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb9_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwGjjwjBGCwWh_     : 1;
        osal_u32 xFwTjw_NGjjwLPwNBoLW_wWh_    : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxOlOmo_ : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxO6Omo_ : 1;
        osal_u32 xFwTjw_NGjjw_PwNBoLW_wWh_    : 1;
        osal_u32 xFwTjw_NGjjw_PwqgGwBwWh_     : 1;
        osal_u32 xFwTjw_NGjjw_PwqgGwRwWh_     : 1;
        osal_u32 x4OqyOwtzyyOw8O4CvCOmo_      : 1;
        osal_u32 x4OqyOwtzyyOw8Ol6CzxOmo_     : 1;
        osal_u32 xFwTjw_NGjjwLPwLBGwWh_       : 1;
        osal_u32 x4OqyOwtzyyOG4yStOmo_        : 1;
        osal_u32 x_WCqwy_                     : 5;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb10_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOxv2moOssyORmsR_       : 1;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZR8ORmsR_ : 1;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZw8ORmsR_ : 1;
        osal_u32 xFwTjw_NwoigWhwCCjwqFWLwLWCL_  : 1;
        osal_u32 x_WCqwy_                       : 1;
        osal_u32 xFwTjw_NwLPwBjgwuGwLWCL_       : 1;
        osal_u32 xFwTjw_NwLPwBjgwuuGwLWCL_      : 1;
        osal_u32 x4OqyOwtOR8OlGRzROGzORmsR_     : 1;
        osal_u32 x4OqyOwtOR8OlGRzROGGzORmsR_    : 1;
        osal_u32 x4OqyOwtOR8OGzORmsR_           : 1;
        osal_u32 x4OqyOwtOR8OGzOylzsORmsR_      : 1;
        osal_u32 x4OqyOwtOR8OGGzORmsR_          : 1;
        osal_u32 x4OqyOwtOR8OGGzOylzsORmsR_     : 1;
        osal_u32 x4OqyOwtOR8OSGCORmsR_          : 1;
        osal_u32 xFwTjw_NwLPwuGwGFjwLWCL_       : 1;
        osal_u32 xFwTjw_NwLPwoiFBqwLWCL_        : 1;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb11_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwLWCL_        : 1;
        osal_u32 x4OqyOwtOw8OxozOyStORmsR_    : 1;
        osal_u32 x_WCqwy_                     : 1;
        osal_u32 x4OqyOwtOw8OylzsORmsR_       : 1;
        osal_u32 xFwTjw_NwL_PwL_PCTwLWCL_     : 1;
        osal_u32 xFwTjw_NwL_PwBR6GowLWCL_     : 1;
        osal_u32 xFwTjw_Nw_PwoiFBqwLWCL_      : 1;
        osal_u32 x4OqyOwtOw8OxvylzsORmsR_     : 1;
        osal_u32 xFwTjw_Nw_PwLBGwLWCL_        : 1;
        osal_u32 xFwTjw_NwL_PwBR6GowBjBGCwLWCL_ : 1;
        osal_u32 x4OqyOwtORw8Ol6CzxOyStORmsR_ : 1;
        osal_u32 xwmsPOr_                     : 1;
        osal_u32 x4OqxOGzOP4mRORmsROmo_       : 1;
        osal_u32 x4OqxOGGzOP4mRORmsROmo_      : 1;
        osal_u32 resv_2                       : 2;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb12_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwGjjwjBGCwLWCL_     : 1;
        osal_u32 xFwTjw_NGjjwLPwNBoLW_wLWCL_    : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxOlORmsR_ : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxO6ORmsR_ : 1;
        osal_u32 xFwTjw_NGjjw_PwNBoLW_wLWCL_    : 1;
        osal_u32 xFwTjw_NGjjw_PwqgGwBwLWCL_     : 1;
        osal_u32 xFwTjw_NGjjw_PwqgGwRwLWCL_     : 1;
        osal_u32 x4OqyOwtzyyOw8O4CvCORmsR_      : 1;
        osal_u32 xFwTjw_NGjjwLPwLBGwLWCL_       : 1;
        osal_u32 x4OqyOwtzyyOG4yStORmsR_        : 1;
        osal_u32 x_WCqwy_                       : 6;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb13_union;

typedef union {
    struct {
        osal_u32 xFwTjwGF6w6L_oy_ : 16;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb14_union;

typedef union {
    struct {
        osal_u32 x4OqyOz4COCRwxr_ : 16;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb15_union;

typedef union {
    struct {
        osal_u32 xFwTjwFG6w6L_oy_ : 16;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb16_union;

typedef union {
    struct {
        osal_u32 x4OqyO4zCOCRwxr_ : 16;
        osal_u32                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb17_union;

typedef union {
    struct {
        osal_u32 xFwTjwLCLwCTw6L_o0_ : 16;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb18_union;

typedef union {
    struct {
        osal_u32 x4OqyORsROsqOCRwxZ_ : 16;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb19_union;

typedef union {
    struct {
        osal_u32 x4OqxOR8OwCOCv4mOl_ : 6;
        osal_u32 x_WCqwy_          : 2;
        osal_u32 x4OqxOR8OwCOCv4mO6_ : 6;
        osal_u32 xwmsPOr_          : 2;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb20_union;

typedef union {
    struct {
        osal_u32 x4OqxOw8OwCOCv4mOl_ : 6;
        osal_u32 x_WCqwy_          : 2;
        osal_u32 x4OqxOw8OwCOCv4mO6_ : 6;
        osal_u32 xwmsPOr_          : 2;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb21_union;

typedef union {
    struct {
        osal_u32 xFwjLwLPw_6w6iFWwB_ : 6;
        osal_u32 x_WCqwy_          : 2;
        osal_u32 xFwjLwLPw_6w6iFWwR_ : 6;
        osal_u32 xwmsPOr_          : 2;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb22_union;

typedef union {
    struct {
        osal_u32 xFwjLw_Pw_6w6iFWwB_ : 6;
        osal_u32 x_WCqwy_          : 2;
        osal_u32 xFwjLw_Pw_6w6iFWwR_ : 6;
        osal_u32 xwmsPOr_          : 2;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb23_union;

typedef union {
    struct {
        osal_u32 x4OqxOwtOw8OCOCv4m_ : 3;
        osal_u32 x_WCqwy_          : 1;
        osal_u32 xFwTow_Nw_Pw_w6iFW_ : 3;
        osal_u32 xwmsPOr_          : 1;
        osal_u32 xFwjLw_Nw_Pw6w6iFW_ : 3;
        osal_u32 resv_2            : 1;
        osal_u32 x4OyROwtOw8OwOCv4m_ : 3;
        osal_u32 x_WCqwk_          : 1;
        osal_u32                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb24_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwL_PwBR6Gow_66iFWwLiwoC_ : 4;
        osal_u32 xFwTjw_NwL_PwBR6Gow6C6iFWwLiwoC_ : 4;
        osal_u32 x4OqyOwtORw8Ol6CzxOCGCv4mORvOxs_ : 4;
        osal_u32 x_WCqwy_                       : 4;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb25_union;

typedef union {
    struct {
        osal_u32 xFwLiuw6L_owL_PCTwCTGuwWh_           : 1;
        osal_u32 x_WCqwy_                             : 3;
        osal_u32 x4OqyOwtzyyOw8O4CvCOsqzGORvOxs_      : 1;
        osal_u32 x4OqyOwtzyyOw8O4CvCOl6OsqzGORvOxs_   : 1;
        osal_u32 xwmsPOr_                             : 2;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxOsqzGOlORvOxs_ : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxOsqzGO6ORvOxs_ : 1;
        osal_u32 resv_2                               : 6;
        osal_u32                                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb26_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOR8OxGtOyq_               : 2;
        osal_u32 xFwTjw_NGjjwLPwouNwjTw00jwCWo_       : 2;
        osal_u32 xFwTjw_NGjjwLPwouNwjTwBjBGCw00jwqGodW_ : 3;
        osal_u32 x_WCqwy_                             : 9;
        osal_u32                                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb27_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyOw8wtOmoOxSR_  : 4;
        osal_u32 x4OqyOtyOGzO4mROmoOxSR_ : 4;
        osal_u32 x4OqyOtyOGGzO4mROmoOxSR_ : 4;
        osal_u32 x_WCqwy_               : 4;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb28_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyOGzOmoOxSR_  : 4;
        osal_u32 x4OqyOtyOR8wtOmoOxSR_ : 4;
        osal_u32 x4OqyOtyORw8sqOmoOxSR_ : 4;
        osal_u32 x4OqyOtyOl6CzxOmoOxSR_ : 4;
        osal_u32                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb29_union;

typedef union {
    struct {
        osal_u32 xFwTjwNjwgGBhywohGwodL_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFwTjwNjwgGBhywqgGwodL_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb30_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyO2zlorOxozOxSR_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 x4OqyOtyO2zlorOP2zOxSR_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb31_union;

typedef union {
    struct {
        osal_u32 xFwTjwNjwgGBhVwohGwodL_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFwTjwNjwgGBhVwqgGwodL_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb32_union;

typedef union {
    struct {
        osal_u32 xFwTjwNjwgGBhkwohGwodL_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFwTjwNjwgGBhkwqgGwodL_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb33_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyO2zloKOxozOxSR_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 x4OqyOtyO2zloKOP2zOxSR_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb34_union;

typedef union {
    struct {
        osal_u32 xFwTjwNjwgGBhxwohGwodL_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFwTjwNjwgGBhxwqgGwodL_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb35_union;

typedef union {
    struct {
        osal_u32 x4OqyOtyO2zloYOxozOxSR_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 x4OqyOtyO2zloYOP2zOxSR_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb36_union;

typedef union {
    struct {
        osal_u32 xFwTjwNjwgGBhpwohGwodL_ : 3;
        osal_u32 x_WCqwy_              : 4;
        osal_u32 xFwTjwNjwgGBhpwqgGwodL_ : 5;
        osal_u32 xwmsPOr_              : 4;
        osal_u32                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb37_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwoigWhwCCjwFBqUg_      : 4;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZR8ORwlI_ : 4;
        osal_u32 x4OqyOwtOxv2moOxvyStOxvZw8ORwlI_ : 4;
        osal_u32 xFwTjw_NwoigWhwCCjwq6L_o0_     : 4;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb38_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOxv2moOssyORSomOy3OxSR_ : 4;
        osal_u32 xFwTjw_NwoigWhwCCjwLdhWwj0wodL_ : 4;
        osal_u32 x4OqyOwtOxv2moOssyORSomOyZOxSR_ : 4;
        osal_u32 x_WCqwy_                      : 4;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb39_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwoigWhwCCjwBNjwLPwodL_ : 4;
        osal_u32 xFwTjw_NwoigWhwCCjwBNjw_PwodL_ : 4;
        osal_u32 x4OqyOwtOxv2moOssyO2zloOR8ORwlI_ : 3;
        osal_u32 x_WCqwy_                       : 1;
        osal_u32 x4OqyOwtOxv2moOssyO2zloOw8ORwlI_ : 3;
        osal_u32 x4OqyOwtOxv2moOssyOlCRwxOsmx_  : 1;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb40_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8Oyzo4smx_          : 2;
        osal_u32 x_WCqwy_                      : 2;
        osal_u32 x4OqyOwtOw8OxozO2zloOsmxOxvCzx_ : 3;
        osal_u32 xwmsPOr_                      : 1;
        osal_u32 x4OqyOwtOw8ORlzO2zloOsmxOxvCzx_ : 3;
        osal_u32 resv_2                        : 1;
        osal_u32 xFwTjw_Nw_PwjGhFCWowGdLi_     : 1;
        osal_u32 x_WCqwk_                      : 3;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb41_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwGdLi_                 : 1;
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwoi6Go_      : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwoi6Go_      : 1;
        osal_u32 xFwTjw_Nw_PwohGwogwWhwoi6Go_      : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxvCzx_    : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOxvCzx_     : 3;
        osal_u32 x4OqyOwtOw8ORlzOwtyOwRwlIOxvCzx_  : 3;
        osal_u32 x_WCqwy_                          : 1;
        osal_u32 x4OqyOwtORw8Ol6CzxOyStOlRwlIORvOxs_ : 3;
        osal_u32 xwmsPOr_                          : 1;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb42_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwhdShGwoi6Go_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxvCzx_ : 4;
        osal_u32 x_WCqwy_                    : 8;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb43_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOwzRROxvCzx_    : 6;
        osal_u32 x_WCqwy_                       : 2;
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxvCzx_ : 6;
        osal_u32 xwmsPOr_                       : 2;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb44_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwodLy_ : 1;
        osal_u32 x4OqyOwtOw8OxozOA2OmoOxSRr_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwodLV_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwodLk_ : 1;
        osal_u32 x4OqyOwtOw8OxozOA2OmoOxSRK_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwodLx_ : 1;
        osal_u32 x4OqyOwtOw8OxozOA2OmoOxSRY_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwQgwWhwodLp_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwodLy_ : 1;
        osal_u32 x4OqyOwtOw8OxozOI2OmoOxSRr_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwodLV_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwodLk_ : 1;
        osal_u32 x4OqyOwtOw8OxozOI2OmoOxSRK_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwodLx_ : 1;
        osal_u32 x4OqyOwtOw8OxozOI2OmoOxSRY_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwSgwWhwodLp_ : 1;
        osal_u32                           : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb45_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwogwWhwodLy_ : 1;
        osal_u32 x4OqyOwtOw8OxozOx2OmoOxSRr_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwogwWhwodLV_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwogwWhwodLk_ : 1;
        osal_u32 x4OqyOwtOw8OxozOx2OmoOxSRK_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwogwWhwodLx_ : 1;
        osal_u32 x4OqyOwtOw8OxozOx2OmoOxSRY_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwogwWhwodLp_ : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxSR3_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwCL0jwWhwodL0_ : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxSRZ_ : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxSRi_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwCL0jwWhwodLD_ : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxSRa_ : 1;
        osal_u32 xFwTjw_Nw_PwohGwCL0jwWhwodLX_ : 1;
        osal_u32 x4OqyOwtOw8OxozOsRryOmoOxSR9_ : 1;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb46_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOwzRROxSR3_ : 6;
        osal_u32 x_WCqwy_                 : 2;
        osal_u32 xFwTjw_Nw_PwohGw_GLLwodL0_ : 6;
        osal_u32 xwmsPOr_                 : 2;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb47_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOwzRROxSRZ_ : 6;
        osal_u32 x_WCqwy_                 : 2;
        osal_u32 x4OqyOwtOw8OxozOwzRROxSRi_ : 6;
        osal_u32 xwmsPOr_                 : 2;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb48_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGw_GLLwodLD_ : 6;
        osal_u32 x_WCqwy_                 : 2;
        osal_u32 x4OqyOwtOw8OxozOwzRROxSRa_ : 6;
        osal_u32 xwmsPOr_                 : 2;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb49_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGw_GLLwodLX_ : 6;
        osal_u32 x_WCqwy_                 : 2;
        osal_u32 x4OqyOwtOw8OxozOwzRROxSR9_ : 6;
        osal_u32 xwmsPOr_                 : 2;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb50_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxSR3_ : 6;
        osal_u32 x_WCqwy_                      : 2;
        osal_u32 xFwTjw_Nw_PwohGwGLLCwL_BSwodL0_ : 6;
        osal_u32 xwmsPOr_                      : 2;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb51_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxSRZ_ : 6;
        osal_u32 x_WCqwy_                      : 2;
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxSRi_ : 6;
        osal_u32 xwmsPOr_                      : 2;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb52_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwGLLCwL_BSwodLD_ : 6;
        osal_u32 x_WCqwy_                      : 2;
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxSRa_ : 6;
        osal_u32 xwmsPOr_                      : 2;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb53_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwGLLCwL_BSwodLX_ : 6;
        osal_u32 x_WCqwy_                      : 2;
        osal_u32 x4OqyOwtOw8OxozOzRRsORwlIOxSR9_ : 6;
        osal_u32 xwmsPOr_                      : 2;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb54_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwhdShGwodLy_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozOxSRr_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGwodLV_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGwodLk_ : 4;
        osal_u32                           : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb55_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOoSIozOxSRK_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGwodLx_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozOxSRY_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGwodLp_ : 4;
        osal_u32                           : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb56_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxSR3_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGVwodL0_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxSRZ_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxSRi_ : 4;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb57_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwhdShGVwodLD_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxSRa_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShGVwodLX_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIozZOxSR9_ : 4;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb58_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjyw6x_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOy3OCK_ : 3;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjyw6k_ : 3;
        osal_u32 resv_2                      : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjyw6V_ : 3;
        osal_u32 x_WCqwk_                    : 1;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb59_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOICRSomOy3OCr_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOyrOCa_ : 3;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwj0w6D_ : 3;
        osal_u32 resv_2                      : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOyrOCi_ : 3;
        osal_u32 x_WCqwk_                    : 1;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb60_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxozOICRSomOyrOCZ_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwj0w60_ : 3;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjVw6x_ : 3;
        osal_u32 resv_2                      : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOyZOCK_ : 3;
        osal_u32 x_WCqwk_                    : 1;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb61_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjVw6k_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 xFwTjw_Nw_PwohGwS6LdhWwjVw6V_ : 3;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 x4OqyOwtOw8OxozOICRSomOyZOCr_ : 3;
        osal_u32 resv_2                      : 5;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb62_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwhdShwLiwoC_  : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIoZORvOxs_ : 4;
        osal_u32 xFwTjw_Nw_PwohGwhdShu6wLiwoC_ : 4;
        osal_u32 x4OqyOwtOw8OxozOoSIoGCZORvOxs_ : 4;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb63_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwohGwhdSh6wLiwoC_ : 5;
        osal_u32 x_WCqwy_                    : 3;
        osal_u32 x4OqyOwtOw8OxozOoSIoCZORvOxs_ : 5;
        osal_u32 xwmsPOr_                    : 3;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb64_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8OxvylzsORwlIORvOxs_       : 4;
        osal_u32 xFwTjw_Nw_PwLBGw6SNjw_L_BSwLiwoC_    : 3;
        osal_u32 x_WCqwy_                             : 1;
        osal_u32 xFwTjw_Nw_PwLBGwjBGCwBL_BSwLiwoC_    : 4;
        osal_u32 x4OqyOwtOw8ORlzOylzsOlRwlIOCItyORvOxs_ : 4;
        osal_u32                                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb65_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8ORlzOPwmtORwlIORvOxs_ : 4;
        osal_u32 xFwTjw_Nw_PwLBGwiuwBL_BSwLiwoC_ : 2;
        osal_u32 x_WCqwy_                       : 2;
        osal_u32 xFwTjw_Nw_PwLBGwiuw6w6GowLiwoC_ : 4;
        osal_u32 x4OqyOwtOw8ORlzOvGOwOCzxORvOxs_ : 2;
        osal_u32 xwmsPOr_                       : 2;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb66_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOw8ORlzOwtyOwRwlIOwa_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 xFwTjw_Nw_PwLBGw_Njw_L_BSw_D_ : 3;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 x4OqyOwtOw8ORlzOwtyOwRwlIOwi_ : 3;
        osal_u32 resv_2                      : 1;
        osal_u32 x4OqyOwtOw8ORlzOwtyOwRwlIOwZ_ : 3;
        osal_u32 x_WCqwk_                    : 1;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb67_union;

typedef union {
    struct {
        osal_u32 xFwTjw_Nw_PwLBGw_Njw_L_BSw_0_ : 3;
        osal_u32 x_WCqwy_                    : 13;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb68_union;

typedef union {
    struct {
        osal_u32 xFwTow_Nw_PwohGwgGBhwodLy_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 x4OqxOwtOw8OxozO2zloOxSRr_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 xFwTow_Nw_PwohGwgGBhwodLV_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 xFwTow_Nw_PwohGwgGBhwodLk_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb69_union;

typedef union {
    struct {
        osal_u32 x4OqxOwtOw8OxozO2zloOxSRK_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 xFwTow_Nw_PwohGwgGBhwodLx_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 x4OqxOwtOw8OxozO2zloOxSRY_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 xFwTow_Nw_PwohGwgGBhwodLp_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb70_union;

typedef union {
    struct {
        osal_u32 xFwTow_Nw_PwLBGwgGBhwodLy_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 x4OqxOwtOw8ORlzO2zloOxSRr_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 xFwTow_Nw_PwLBGwgGBhwodLV_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 xFwTow_Nw_PwLBGwgGBhwodLk_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb71_union;

typedef union {
    struct {
        osal_u32 x4OqxOwtOw8ORlzO2zloOxSRK_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 xFwTow_Nw_PwLBGwgGBhwodLx_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 x4OqxOwtOw8ORlzO2zloOxSRY_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 xFwTow_Nw_PwLBGwgGBhwodLp_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb72_union;

typedef union {
    struct {
        osal_u32 x4OyROwtOw8OxozO2zloOxSR3_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 xFwjLw_Nw_PwohGwgGBhwodL0_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 x4OyROwtOw8OxozO2zloOxSRZ_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 x4OyROwtOw8OxozO2zloOxSRi_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb73_union;

typedef union {
    struct {
        osal_u32 xFwjLw_Nw_PwohGwgGBhwodLD_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 x4OyROwtOw8OxozO2zloOxSRa_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 xFwjLw_Nw_PwohGwgGBhwodLX_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 x4OyROwtOw8OxozO2zloOxSR9_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb74_union;

typedef union {
    struct {
        osal_u32 x4OyROwtOw8ORlzO2zloOxSR3_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 xFwjLw_Nw_PwLBGwgGBhwodL0_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 x4OyROwtOw8ORlzO2zloOxSRZ_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 x4OyROwtOw8ORlzO2zloOxSRi_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb75_union;

typedef union {
    struct {
        osal_u32 xFwjLw_Nw_PwLBGwgGBhwodLD_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 x4OyROwtOw8ORlzO2zloOxSRa_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 xFwjLw_Nw_PwLBGwgGBhwodLX_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 x4OyROwtOw8ORlzO2zloOxSR9_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb76_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OlGRzROGzOsmx_ : 2;
        osal_u32 x_WCqwy_                  : 2;
        osal_u32 x4OqyOwtOR8OlGRzROGGzOsmx_ : 2;
        osal_u32 xwmsPOr_                  : 2;
        osal_u32 xFwTjw_NwLPwuGwGFjwGLLwCWo_ : 3;
        osal_u32 resv_2                    : 1;
        osal_u32 x4OqyOwtOR8OGzOPRvGOsmx_  : 4;
        osal_u32                           : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb77_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOz4yOPRAOsmxOmtSsm_ : 4;
        osal_u32 x_WCqwy_                        : 4;
        osal_u32 x4OqyOwtOR8OGGzOCzGOsq_         : 5;
        osal_u32 xwmsPOr_                        : 3;
        osal_u32                                 : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb78_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowGdLi_ : 1;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowGdLi_     : 1;
        osal_u32 x4OqyOwtOR8OGzOPIl4OsmxOzSRv_   : 1;
        osal_u32 xFwTjw_NwLPwuuGwq6GCwCWowGdLi_  : 1;
        osal_u32 x4OqyOwtOR8OGzOz4yOPyOsmxOzSRv_ : 1;
        osal_u32 xFwTjw_NwLPwuuGwqjGwCWowGdLi_   : 1;
        osal_u32 xFwTjw_NwLPwuuGwqj6wCWowGdLi_   : 1;
        osal_u32 xFwTjw_NwLPwdu6wqjwCWowGdLi_    : 1;
        osal_u32 x4OqyOwtOR8OGzOzIGIOPtl8OzSRv_  : 1;
        osal_u32 x_WCqwy_                        : 7;
        osal_u32                                 : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb79_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowSGhdGo_ : 6;
        osal_u32 x_WCqwy_                          : 2;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowSGhdGo_     : 5;
        osal_u32 xwmsPOr_                          : 3;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb80_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOPIl4OsmxOIzoSzx_ : 4;
        osal_u32 xFwTjw_NwLPwuuGwq6GCwCWowSGhdGo_ : 4;
        osal_u32 x4OqyOwtOR8OGzOz4yOPyOsmxOIzoSzx_ : 6;
        osal_u32 x_WCqwy_                        : 2;
        osal_u32                                 : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb81_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuuGwqjGwCWowSGhdGo_ : 5;
        osal_u32 x_WCqwy_                      : 3;
        osal_u32 xFwTjw_NwLPwuuGwqj6wCWowSGhdGo_ : 5;
        osal_u32 xwmsPOr_                      : 3;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb82_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwdu6wqjwCWowSGhdGo_ : 4;
        osal_u32 x4OqyOwtOR8OGzOzIGIOPtl8OIzoSzx_ : 5;
        osal_u32 x_WCqwy_                       : 7;
        osal_u32                                : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb83_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwLBGwq6SwL_BS_    : 4;
        osal_u32 x4OqyOwtzyyORlzOPCIORwlIOw8l6_ : 4;
        osal_u32 x_WCqwy_                     : 8;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb84_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyORlzOly2ORwlIOr3I_ : 4;
        osal_u32 xFwTjw_NGjjwLBGwBjgwL_BSwVyS_ : 4;
        osal_u32 x4OqyOwtzyyORlzOly2ORwlIOw8l6_ : 4;
        osal_u32 x_WCqwy_                     : 4;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb85_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyORlzOCC_    : 3;
        osal_u32 x_WCqwy_               : 1;
        osal_u32 xFwTjw_NGjjwLBGw66w_PBR_ : 3;
        osal_u32 xwmsPOr_               : 9;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb86_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwLBGw_1_    : 2;
        osal_u32 x_WCqwy_               : 2;
        osal_u32 x4OqyOwtzyyORlzOwpOw8l6_ : 2;
        osal_u32 xwmsPOr_               : 10;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb87_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyORlzOCItyOw_    : 3;
        osal_u32 x_WCqwy_                   : 1;
        osal_u32 xFwTjw_NGjjwLBGw6SNjw_w_PBR_ : 3;
        osal_u32 xwmsPOr_                   : 9;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb88_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyORlzOvGrOlRwlI_    : 1;
        osal_u32 x_WCqwy_                      : 3;
        osal_u32 xFwTjw_NGjjwLBGwiu0wBL_BSw_PBR_ : 1;
        osal_u32 xwmsPOr_                      : 11;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb89_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOtlxRmwOPCIORwlIOw8OqxOr3I_ : 4;
        osal_u32 xFwTjw_NGjjwNBoLW_wq6SwL_BSw_PwTowVyS_ : 4;
        osal_u32 x4OqyOwtzyyOtlxRmwOPCIORwlIOw8OyROZGaI_ : 4;
        osal_u32 xFwTjw_NGjjwNBoLW_wq6SwL_BSw_PwjLwxS_ : 4;
        osal_u32                                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb90_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwNBoLW_wq6SwL_BSwLPw0yS_ : 4;
        osal_u32 x4OqyOwtzyyOtlxRmwOPCIORwlIOR8OZ3I_ : 4;
        osal_u32 x_WCqwy_                          : 8;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb91_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOtlxRmwOly2ORwlIOw8OqxOr3I_ : 4;
        osal_u32 xFwTjw_NGjjwNBoLW_wBjgwL_BSw_PwTowVyS_ : 4;
        osal_u32 x4OqyOwtzyyOtlxRmwOly2ORwlIOw8OyROZGaI_ : 4;
        osal_u32 xFwTjw_NGjjwNBoLW_wBjgwL_BSw_PwjLwxS_ : 4;
        osal_u32                                       : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb92_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwNBoLW_wBjgwL_BSwLPw0yS_ : 4;
        osal_u32 x4OqyOwtzyyOtlxRmwOly2ORwlIOR8OZ3I_ : 4;
        osal_u32 x_WCqwy_                          : 8;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb93_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOtlxRmwOwlGCzxORwlIOw8OqxOr3I_ : 5;
        osal_u32 x_WCqwy_                                : 3;
        osal_u32 xFwTjw_NGjjwNBoLW_w_Bu6GowL_BSw_PwTowVyS_ : 5;
        osal_u32 xwmsPOr_                                : 3;
        osal_u32                                         : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb94_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOtlxRmwOwlGCzxORwlIOw8OyROZGaI_ : 5;
        osal_u32 x_WCqwy_                                 : 3;
        osal_u32 xFwTjw_NGjjwNBoLW_w_Bu6GowL_BSw_PwjLwxS_ : 5;
        osal_u32 xwmsPOr_                                 : 3;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb95_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwNBoLW_w_Bu6GowL_BSwLPw0yS_ : 5;
        osal_u32 x_WCqwy_                             : 3;
        osal_u32 x4OqyOwtzyyOtlxRmwOwlGCzxORwlIOR8OZ3I_ : 5;
        osal_u32 xwmsPOr_                             : 3;
        osal_u32                                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb96_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOw8OxGtOvGrOCC_ : 3;
        osal_u32 x_WCqwy_                 : 1;
        osal_u32 xFwTjw_NGjjw_PwouNwiuVw66_ : 3;
        osal_u32 xwmsPOr_                 : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtOvGrOCC_ : 3;
        osal_u32 resv_2                   : 1;
        osal_u32 xFwTjw_NGjjwLPwouNwiuVw66_ : 3;
        osal_u32 x_WCqwk_                 : 1;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb97_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjw_PwouNwiu0w_1_ : 2;
        osal_u32 x4OqyOwtzyyOw8OxGtOvGZOwp_ : 2;
        osal_u32 xFwTjw_NGjjwLPwouNwiu0w_1_ : 2;
        osal_u32 x4OqyOwtzyyOR8OxGtOvGZOwp_ : 2;
        osal_u32 x_WCqwy_                 : 8;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb98_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOw8OxGtOvGrOCItyOw_ : 3;
        osal_u32 x_WCqwy_                     : 1;
        osal_u32 xFwTjw_NGjjw_PwouNwiuVw6SNjw__ : 3;
        osal_u32 xwmsPOr_                     : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtOvGrOCItyOw_ : 3;
        osal_u32 resv_2                       : 1;
        osal_u32 xFwTjw_NGjjwLPwouNwiuVw6SNjw__ : 3;
        osal_u32 x_WCqwk_                     : 1;
        osal_u32                              : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb99_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOw8OxGtOvGrOlRwlI_     : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtOvGrOlRwlI_     : 1;
        osal_u32 x_WCqwy_                          : 2;
        osal_u32 xFwTjw_NGjjw_PwouNwiuVwBL_BSwLiwoC_ : 2;
        osal_u32 xFwTjw_NGjjwLPwouNwiuVwBL_BSwLiwoC_ : 2;
        osal_u32 xwmsPOr_                          : 8;
        osal_u32                                   : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb100_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwqgGwq6SwLiwoC_ : 4;
        osal_u32 x_WCqwy_                 : 12;
        osal_u32                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb101_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwqgGwBjgwL_BSwLiwoC_ : 4;
        osal_u32 x_WCqwy_                      : 12;
        osal_u32                               : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb102_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOP2zOlRwlIORvOxs_ : 1;
        osal_u32 x_WCqwy_                   : 15;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb103_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOP2zOCCOxSR3_ : 3;
        osal_u32 x_WCqwy_               : 1;
        osal_u32 xFwTjw_NGjjwqgGw66wodL0_ : 3;
        osal_u32 xwmsPOr_               : 1;
        osal_u32 x4OqyOwtzyyOP2zOCCOxSRZ_ : 3;
        osal_u32 resv_2                 : 1;
        osal_u32 x4OqyOwtzyyOP2zOCCOxSRi_ : 3;
        osal_u32 x_WCqwk_               : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb104_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwqgGw66wodLD_ : 3;
        osal_u32 x_WCqwy_               : 1;
        osal_u32 x4OqyOwtzyyOP2zOCCOxSRa_ : 3;
        osal_u32 xwmsPOr_               : 1;
        osal_u32 xFwTjw_NGjjwqgGw66wodLX_ : 3;
        osal_u32 resv_2                 : 1;
        osal_u32 x4OqyOwtzyyOP2zOCCOxSR9_ : 3;
        osal_u32 x_WCqwk_               : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb105_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwqgGw_1wodLy_ : 2;
        osal_u32 x4OqyOwtzyyOP2zOwpOxSRr_ : 2;
        osal_u32 xFwTjw_NGjjwqgGw_1wodLV_ : 2;
        osal_u32 xFwTjw_NGjjwqgGw_1wodLk_ : 2;
        osal_u32 x4OqyOwtzyyOP2zOwpOxSRK_ : 2;
        osal_u32 xFwTjw_NGjjwqgGw_1wodLx_ : 2;
        osal_u32 x4OqyOwtzyyOP2zOwpOxSRY_ : 2;
        osal_u32 xFwTjw_NGjjwqgGw_1wodLp_ : 2;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb106_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtzyyOP2zOCItyOwOxSR3_ : 3;
        osal_u32 x_WCqwy_                   : 1;
        osal_u32 xFwTjw_NGjjwqgGw6SNjw_wodL0_ : 3;
        osal_u32 xwmsPOr_                   : 1;
        osal_u32 x4OqyOwtzyyOP2zOCItyOwOxSRZ_ : 3;
        osal_u32 resv_2                     : 1;
        osal_u32 x4OqyOwtzyyOP2zOCItyOwOxSRi_ : 3;
        osal_u32 x_WCqwk_                   : 1;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb107_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwqgGw6SNjw_wodLD_ : 3;
        osal_u32 x_WCqwy_                   : 1;
        osal_u32 x4OqyOwtzyyOP2zOCItyOwOxSRa_ : 3;
        osal_u32 xwmsPOr_                   : 1;
        osal_u32 xFwTjw_NGjjwqgGw6SNjw_wodLX_ : 3;
        osal_u32 resv_2                     : 1;
        osal_u32 x4OqyOwtzyyOP2zOCItyOwOxSR9_ : 3;
        osal_u32 x_WCqwk_                   : 1;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb108_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NGjjwF6i6wBuiotwL_BSwLiwoC_   : 2;
        osal_u32 x4OqyOwtzyyO4CvCOCvzwsmRwlIORvOxs_   : 2;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxORwlIOlORvOxs_ : 3;
        osal_u32 x_WCqwy_                             : 1;
        osal_u32 x4OqyOwtzyyOR8OxGtO4CCzxORwlIO6ORvOxs_ : 3;
        osal_u32 xwmsPOr_                             : 1;
        osal_u32 xFwTjw_NGjjwuFjdNwq6SwLiwoC_         : 3;
        osal_u32 resv_2                               : 1;
        osal_u32                                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb109_union;

typedef union {
    struct {
        osal_u32 x4OqxOGzOP4mROsAltROylzsOmoORvOxs_ : 1;
        osal_u32 x4OqxOGzOP4mROPCIOZ2ORvOxs_      : 3;
        osal_u32 xFwTowuGwqFWLw_WCwgGBhwVgwLiwoC_ : 3;
        osal_u32 x4OqxOGzOP4mROwmsOsAltROZ2ORvOxs_ : 3;
        osal_u32 x4OqxOGGzOP4mROPCIOZ2ORvOxs_     : 3;
        osal_u32 x_WCqwy_                         : 3;
        osal_u32                                  : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb110_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyyyyyywodL_ : 6;
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO33333rOxSR_ : 6;
        osal_u32 xFwTjw_NwoigWhwCCjwq6L_oVwLywodL_        : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb111_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO3333r3OxSR_ : 6;
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyyyy00wodL_ : 6;
        osal_u32 x4OqyOwtOR8OGzOPIl4OsmxORO333OxSR_       : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb112_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO333r33OxSR_ : 6;
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyyy0y0wodL_ : 6;
        osal_u32 xFwTjw_NwLPwuuGwq6GCwCWowLwyyywodL_      : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb113_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyyy00ywodL_ : 6;
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO333rrrOxSR_ : 6;
        osal_u32 xFwTjw_NwLPwdu6wqjwCWowLwyyywodL_        : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb114_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyyyyyywodL_ : 5;
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO33333rOxSR_ : 5;
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO3333r3OxSR_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb115_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyyyy00wodL_ : 5;
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO333r33OxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyyy0y0wodL_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb116_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyyy00ywodL_ : 5;
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO333rrrOxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuuGwqjGwCWowLwyyywodL_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb117_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuuGwqj6wCWowLwyyywodL_ : 5;
        osal_u32 x4OqyOwtOR8OGzOzIGIOPtl8ORO333OxSR_ : 5;
        osal_u32 x4OqyOwtOR8OGzOz4yOPyOsmxORO333OxSR_ : 6;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb118_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO33r333OxSR_ : 6;
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0yy0wodL_ : 6;
        osal_u32 x4OqyOwtOxv2moOssyOPCRwxZORrOxSR_        : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb119_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0y0ywodL_ : 6;
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO33r3rrOxSR_ : 6;
        osal_u32 xFwTjw_NwLPwuGwqSBFwCWowLwyy0wodL_       : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb120_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyy00yywodL_ : 6;
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO33rr3rOxSR_ : 6;
        osal_u32 x4OqyOwtOR8OGGzOPCzsOsmxORO33rOxSR_      : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb121_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOz4yOlvSROsmxORyO33rrr3OxSR_ : 6;
        osal_u32 xFwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0000wodL_ : 6;
        osal_u32 x4OqyOwtOR8OSGCOPyOsmxORO33rOxSR_        : 4;
        osal_u32                                          : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb122_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO33r333OxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyy0yy0wodL_ : 5;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyy0y0ywodL_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb123_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO33r3rrOxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyy00yywodL_ : 5;
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO33rr3rOxSR_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb124_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGzOPyOsmxORyO33rrr3OxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuGwqjwCWowLjwyy0000wodL_ : 5;
        osal_u32 x4OqyOwtOR8OGGzOPyzOsmxORO33rOxSR_ : 5;
        osal_u32 x_WCqwy_                           : 1;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb125_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtOR8OGGzOPyCOsmxORO33rOxSR_ : 5;
        osal_u32 xFwTjw_NwLPwuGwGSuSwqNBPwLwyy0wodL_ : 5;
        osal_u32 xFwTjw_NwLPwuGwGFjwqjwCWowLwyy0wodL_ : 6;
        osal_u32                                    : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb126_union;

typedef union {
    struct {
        osal_u32 x4OqyOwtORmIGOCv4mOyzodOsmx_ : 1;
        osal_u32 x_WCqwy_                   : 15;
        osal_u32                            : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb127_union;

typedef union {
    struct {
        osal_u32 xFwTjw_NwjGhFwuQt_ : 2;
        osal_u32 x_WCqwy_         : 14;
        osal_u32                  : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb128_union;

typedef union {
    struct {
        osal_u32 x4OqyOxlomCRwxOGzOylzsOCRwx_ : 3;
        osal_u32 x_WCqwy_                    : 1;
        osal_u32 xFwTjwoBhW6L_owiqW_wLWSuwu_L_ : 1;
        osal_u32 x4OqyOxlomCRwxO4tROl446OIv4m_ : 1;
        osal_u32 xFwTjwoBhW6L_owSiFWwCWo_    : 1;
        osal_u32 xFwTjwoBhW6L_owouNwSiFW_    : 1;
        osal_u32 xFwTjwoBhW6L_owLPwWh_       : 1;
        osal_u32 x4OqyOxlomCRwxOlGzOmo_      : 1;
        osal_u32 xwmsPOr_                    : 1;
        osal_u32 xFwTjwoBhW6L_ow_PwWh_       : 1;
        osal_u32 xFwTjwoBhW6L_owNjwWh_       : 2;
        osal_u32 x4OqyOxlomCRwxOxGtOyq_      : 2;
        osal_u32                             : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb129_union;

typedef union {
    struct {
        osal_u32 x4OqyOxlomCRwxOxozO2zlo_ : 3;
        osal_u32 x_WCqwy_               : 1;
        osal_u32 xFwTjwoBhW6L_owouNwgGBh_ : 3;
        osal_u32 x4OqyOxlomCRwxOP2zO2zlo_ : 5;
        osal_u32 x4OqyOxlomCRwxOtyO2zlo_ : 3;
        osal_u32 xFwTjwoBhW6L_owNjwuQGCW_ : 1;
        osal_u32                        : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb130_union;

typedef union {
    struct {
        osal_u32 x4OqyOxlomCRwxO4CvCOl_ : 8;
        osal_u32 x4OqyOxlomCRwxO4CvCO6_ : 8;
        osal_u32                      : 16;
    } bits;
    osal_u16 u16;
} u_rf_abb131_union;

#endif