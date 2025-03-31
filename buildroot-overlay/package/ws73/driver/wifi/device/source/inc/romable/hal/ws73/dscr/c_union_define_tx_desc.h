// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __C_UNION_DEFINE_TX_DESC_H__
#define __C_UNION_DEFINE_TX_DESC_H__

typedef union {
    struct {
        osal_u32    xR8O4msCOloRwOsm6_    : 4   ;
        osal_u32    xR8O4msCOloRtOR8OCoR_ : 6   ;
        osal_u32    xLPwFWC6woGCLwG6KwjT_ : 2   ;
        osal_u32    xR8O4msCOxzsROoss_    : 1   ;
        osal_u32    xLPwFWC6woGCLwjGhFwTBFLQ_ : 3   ;
        osal_u32    xR8O4msCOxzsROzCdOwssl_ : 8   ;
        osal_u32    xLPwFWC6woGCLwLjwNoGg_ : 2   ;
        osal_u32    tx_desc_rate_rank     : 2   ;
        osal_u32    xLPwFWC6wCLGLdC_      : 4   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word0;

typedef union {
    struct {
        osal_u32    xLPwFWC6w_LCwNGBoWFw6idhL_ : 5   ;
        osal_u32    xLPwFWC6wFGLGwCWhFw6idhL_ : 5   ;
        osal_u32    xR8O4msCOR8ORyOCvSoR_ : 5   ;
        osal_u32    xR8O4msCOsmo4Otxz2_   : 1   ;
        osal_u32    reserved_0            : 4   ;
        osal_u32    xLPwFWC6wjGwCCh_      : 12  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word1;

typedef union {
    struct {
        osal_u32 xR8O4msCOyzOylRIzGOqvw4r_ : 32  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word2;

typedef union {
    struct {
        osal_u32 xLPwFWC6wjGwjBLSGuwTi_Fy_ : 32  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word3;

typedef union {
    struct {
        osal_u32    xLPwFWC6wLPwuhwSCj_   : 16  ;
        osal_u32    xR8O4msCOR8OGoOtx4OPx4_ : 1   ;
        osal_u32    xLPwFWC6wCWRwhdSwNoFwqoF_ : 1   ;
        osal_u32    xR8O4msCOAqOwmRwQOtxz2_ : 1   ;
        osal_u32    reserved_0            : 1   ;
        osal_u32    xR8O4msCOsm6OoSIOtx4_ : 12  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word4;

typedef union {
    struct {
        osal_u32 xLPwFWC6wLPwuhwoCj_    : 32  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word5;

typedef union {
    struct {
        osal_u32    xLPwFWC6whWPLwFWC6wGFF__ : 20  ;
        osal_u32    reserved_0            : 12  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word6;

typedef union {
    struct {
        osal_u32    xLPwFWC6wCdjwSCFdywjdNNwGFF__ : 20  ;
        osal_u32    xLPwFWC6wCdjwSCFdywoWh_ : 12  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word7;

typedef union {
    struct {
        osal_u32    xR8O4msCOsSyOIs4SrOySttOz44w_ : 20  ;
        osal_u32    xR8O4msCOsSyOIs4SrOxmo_ : 12  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word8;

typedef union {
    struct {
        osal_u32    xR8O4msCOIzCOA4wOySttOz44w_ : 20  ;
        osal_u32    xR8O4msCOtRIOCzxlOmo_ : 1   ;
        osal_u32    xR8O4msCOtRIOtxz2_    : 1   ;
        osal_u32    xLPwFWC6wGhLwCWo_     : 2   ;
        osal_u32    reserved_0            : 8   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word9;

typedef union {
    struct {
        osal_u32    reserved_0            : 1   ;
        osal_u32    xLPwFWC6wLCNwLBSWCLGSu_ : 15  ;
        osal_u32    xR8O4msCOIzCOA4wOxmo_ : 6   ;
        osal_u32    reserved_1            : 1   ;
        osal_u32    xLPwFWC6wCdjwSCFdwhdS_ : 1   ;
        osal_u32    xLPwFWC6w_WL_twNoGgwQTwjtuGCC_ : 1   ;
        osal_u32    xR8O4msCO4SwzRlvoOAqOyQGzss_ : 1   ;
        osal_u32    xLPwFWC6wCWRw6L_owQTwjtuGCC_ : 1   ;
        osal_u32    xLPwFWC6wLBSWCLGSuwQTwjtuGCC_ : 1   ;
        osal_u32    xLPwFWC6wLPwGFFjGwCChwjtuGCC_ : 1   ;
        osal_u32    xLPwFWC6wLPwuhwQTwjtuGCC_ : 1   ;
        osal_u32    xR8O4msCOR8OtCsO2moOmwwOmo_ : 1   ;
        osal_u32    xR8O4msCOsvtRqzwmOwmRwQ_ : 1   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word10;

typedef union {
    struct {
        osal_u32    xR8O4msCOIG4SOoSI_    : 6   ;
        osal_u32    xLPwFWC6wLPwqGuwBhFWP_ : 2   ;
        osal_u32    xLPwFWC6wLPwGSuFdwCWCCBihwBhFWP_ : 4   ;
        osal_u32    xLPwFWC6wGhLBwBhLNw0LQ__ : 2   ;
        osal_u32    xR8O4msCOzoRlOloRtO3RAw_ : 2   ;
        osal_u32    xLPwFWC6wGhLBwBhLNwWh_ : 1   ;
        osal_u32    xLPwFWC6wQWwSGPwuWwNoF_ : 2   ;
        osal_u32    xLPwFWC6wuFWLwWh_     : 1   ;
        osal_u32    xLPwFWC6w6BuQW_wLtuW_ : 4   ;
        osal_u32    xR8O4msCOClGAmwOdmQOl4_ : 3   ;
        osal_u32    xLPwFWC6w_GwodLwBhFWP_ : 3   ;
        osal_u32    xR8O4msCOSGxlodOtxz2_ : 1   ;
        osal_u32    xLPwFWC6wGSuFdwWhGjoW_ : 1   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word11;

typedef union {
    struct {
        osal_u32    xLPwFWC6wuG_LBGowGBF_ : 9   ;
        osal_u32    xR8O4msCO2wvSGOl4OyssOCvxvw_ : 6   ;
        osal_u32    xR8O4msCOGwmzIyxmOyvvsROtxz2_ : 1   ;
        osal_u32    xR8O4msCOCAOyzo4ql4RAOloOovoOAROm8lsR_ : 1   ;
        osal_u32    xLPwFWC6wFthwjGhFTBFLQwBhwhihwQLwWPBCL_ : 1   ;
        osal_u32    xR8O4msCO4QoOyzo4ql4RAOloOovoOAR_ : 1   ;
        osal_u32    xLPwFWC6wLPiuwuCwhiLwGooiTWF_ : 1   ;
        osal_u32    xLPwFWC6woCBgwLPiu_   : 1   ;
        osal_u32    xR8O4msCOxvo2OozPOmozyxm_ : 1   ;
        osal_u32    xR8O4msCOIloOIG4SOxmo_ : 10  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word12;

typedef union {
    struct {
        osal_u32    xLPwFWC6wGSuFdwoWh_   : 16  ;
        osal_u32    xLPwFWC6wNW6w6iFBhg_  : 1   ;
        osal_u32    xR8O4msCOsIvvRAlo2_   : 1   ;
        osal_u32    xLPwFWC6wQWwoLNwLtuW_ : 2   ;
        osal_u32    xLPwFWC6wLPw6idhLk_   : 3   ;
        osal_u32    xLPwFWC6wLPw6idhLV_   : 3   ;
        osal_u32    xR8O4msCOR8OCvSoRr_   : 3   ;
        osal_u32    xLPwFWC6wLPw6idhLy_   : 3   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word13;

typedef union {
    struct {
        osal_u32    xLPwFWC6whCCwS6Cw_GLWy_ : 4   ;
        osal_u32    xR8O4msCOtwm6Oyq3_    : 2   ;
        osal_u32    xLPwFWC6wu_iLi6iowSiFWy_ : 3   ;
        osal_u32    xLPwFWC6wQWwF6Sy_     : 1   ;
        osal_u32    xR8O4msCOGwmzIyxm3_   : 1   ;
        osal_u32    xR8O4msCO2lORQGm3_    : 2   ;
        osal_u32    xR8O4msCOwRsOCRsOGwvRmCROIv4m3_ : 2   ;
        osal_u32    reserved_0            : 1   ;
        osal_u32    xR8O4msCORGCOCA3O3_   : 7   ;
        osal_u32    xR8O4msCO4yyOsCzxlo2Ol48O3_ : 5   ;
        osal_u32    xR8O4msCORGCO4G4OxPO3_ : 2   ;
        osal_u32    xLPwFWC6w6N_wBFPwy_   : 2   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word14;

typedef union {
    struct {
        osal_u32    xR8O4msCOossOICsOwzRmr_ : 4   ;
        osal_u32    xLPwFWC6wN_WRwjT0_    : 2   ;
        osal_u32    xR8O4msCOGwvRvCvxOIv4mr_ : 3   ;
        osal_u32    xR8O4msCOAmO4CIr_     : 1   ;
        osal_u32    xLPwFWC6wu_WGSjoW0_   : 1   ;
        osal_u32    xLPwFWC6wgBwLtuW0_    : 2   ;
        osal_u32    xLPwFWC6w_LCw6LCwu_iLW6LwSiFW0_ : 2   ;
        osal_u32    reserved_0            : 1   ;
        osal_u32    xLPwFWC6wLu6w6Qyw0_   : 7   ;
        osal_u32    xLPwFWC6wFjjwC6GoBhgwBFPw0_ : 5   ;
        osal_u32    xLPwFWC6wLu6wFuFwoqw0_ : 2   ;
        osal_u32    xR8O4msCOCtwOl48Or_   : 2   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word15;

typedef union {
    struct {
        osal_u32    xLPwFWC6whCCwS6Cw_GLWV_ : 4   ;
        osal_u32    xR8O4msCOtwm6OyqZ_    : 2   ;
        osal_u32    xLPwFWC6wu_iLi6iowSiFWV_ : 3   ;
        osal_u32    xLPwFWC6wQWwF6SV_     : 1   ;
        osal_u32    xR8O4msCOGwmzIyxmZ_   : 1   ;
        osal_u32    xR8O4msCO2lORQGmZ_    : 2   ;
        osal_u32    xR8O4msCOwRsOCRsOGwvRmCROIv4mZ_ : 2   ;
        osal_u32    reserved_0            : 1   ;
        osal_u32    xR8O4msCORGCOCA3OZ_   : 7   ;
        osal_u32    xR8O4msCO4yyOsCzxlo2Ol48OZ_ : 5   ;
        osal_u32    xR8O4msCORGCO4G4OxPOZ_ : 2   ;
        osal_u32    xLPwFWC6w6N_wBFPwV_   : 2   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word16;

typedef union {
    struct {
        osal_u32    xLPwFWC6whCCwS6Cw_GLWk_ : 4   ;
        osal_u32    xR8O4msCOtwm6Oyqi_    : 2   ;
        osal_u32    xLPwFWC6wu_iLi6iowSiFWk_ : 3   ;
        osal_u32    xLPwFWC6wQWwF6Sk_     : 1   ;
        osal_u32    xR8O4msCOGwmzIyxmi_   : 1   ;
        osal_u32    xR8O4msCO2lORQGmi_    : 2   ;
        osal_u32    xR8O4msCOwRsOCRsOGwvRmCROIv4mi_ : 2   ;
        osal_u32    reserved_0            : 1   ;
        osal_u32    xR8O4msCORGCOCA3Oi_   : 7   ;
        osal_u32    xR8O4msCO4yyOsCzxlo2Ol48Oi_ : 5   ;
        osal_u32    xR8O4msCORGCO4G4OxPOi_ : 2   ;
        osal_u32    xLPwFWC6w6N_wBFPwk_   : 2   ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word17;

typedef union {
    struct {
        osal_u32    xLPwFWC6wQL6w6ihL_io_  : 32  ;
    } bits;

    osal_u32    u32;
} u_tx_desc_word18;

#endif