// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __C_UNION_DEFINE_RX_DESC_H__
#define __C_UNION_DEFINE_RX_DESC_H__

typedef union {
    struct {
        osal_u32    x_PwFWC6whWPLwFWC6wGFF__ : 20  ;
        osal_u32    reserved_0            : 12  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word0;

typedef union {
    struct {
        osal_u32    xw8O4msCOA4wOySttOz44w_ : 20  ;
        osal_u32    reserved_0            : 12  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word1;

typedef union {
    struct {
        osal_u32    x_PwFWC6wuGtoiGFwjdNNwGFF__ : 20  ;
        osal_u32    reserved_0            : 12  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word2;

typedef union {
    struct {
        osal_u32    xw8O4msCOPzGOlo4m8_   : 3   ;
        osal_u32    reserved_0            : 2   ;
        osal_u32    x_PwFWC6w_WCuihCWwNoGg_ : 1   ;
        osal_u32    xw8O4msCOw8Ovt4IzOtxz2_ : 1   ;
        osal_u32    xw8O4msCOAmOtxz2_     : 1   ;
        osal_u32    xw8O4msCOwssl_        : 8   ;
        osal_u32    xw8O4msCOossOICsOwzRm_ : 6   ;
        osal_u32    xw8O4msCOGwvRvCvxOIv4m_ : 2   ;
        osal_u32    x_PwFWC6w6BuQW_wLtuW_ : 4   ;
        osal_u32    x_PwFWC6wCLGLdC_      : 4   ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word3;

typedef union {
    struct {
        osal_u32    x_PwFWC6wN_GSWwoWh_   : 16  ;
        osal_u32    x_PwFWC6wWPLWhFWFwCuGLBGowCL_WGSC_ : 2   ;
        osal_u32    xw8O4msCOsIvvRAlo2_   : 1   ;
        osal_u32    xw8O4msCOtwm6Oyzo4ql4RAOIv4m_ : 4   ;
        osal_u32    x_PwFWC6wu_WGSjoW_    : 1   ;
        osal_u32    x_PwFWC6w6QGhhWow6iFW_ : 1   ;
        osal_u32    xw8O4msCOsRyC_        : 2   ;
        osal_u32    x_PwFWC6wgBwLtuW_     : 2   ;
        osal_u32    x_PwFWC6wGSuFd_       : 1   ;
        osal_u32    x_PwFWC6wCidhFBhgwSiFW_ : 2   ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word4;

typedef union {
    struct {
        osal_u32    xw8O4msCOCSwwOySttOsSyOIs4SOoSI_ : 8   ;
        osal_u32    x_PwFWC6w_W6qwNoGgC_  : 4   ;
        osal_u32    xw8O4msCOCSwwOIG4SOySttmwsOoSI_ : 4   ;
        osal_u32    xw8O4msCOIzCOA4wOxmo_ : 6   ;
        osal_u32    x_PwFWC6w_PwjNwNoGg_  : 1   ;
        osal_u32    xw8O4msCOw8OISOIlIv_  : 1   ;
        osal_u32    xw8O4msCOw8OsSyOIG4SOlo4m8_ : 8   ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word5;

typedef union {
    struct {
        osal_u32    x_PwFWC6wLCNwLBSWCLGSu_ : 24  ;
        osal_u32    x_PwFWC6wQWwoLNwLtuW_ : 2   ;
        osal_u32    x_PwFWC6wF6S_         : 1   ;
        osal_u32    xw8O4msCOloRwOsm6_    : 4   ;
        osal_u32    x_PwFWC6woGCLwSuFdwNoGg_ : 1   ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word6;

typedef union {
    struct {
        osal_u32    xwGROsowOzoR3_        : 8   ;
        osal_u32    xwGROzPmwOmPIOzoR3_   : 12  ;
        osal_u32    x_Pw6Wwuh_            : 8   ;
        osal_u32    xw8O4msCOGmmwOlo4m8_  : 4   ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word7;

typedef union {
    struct {
        osal_u32 x_uLwLWCLwyTi_F_       : 32  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word8;

typedef union {
    struct {
        osal_u32 xwGRORmsROrqvw4_       : 32  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word9;

typedef union {
    struct {
        osal_u32 x_uLwLWCLwVTi_F_       : 32  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word10;

typedef union {
    struct {
        osal_u32 x_uLwLWCLwkTi_F_       : 32  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word11;

typedef union {
    struct {
        osal_u32 xwGRORmsROKqvw4_       : 32  ;
    } bits;

    osal_u32    u32;
} u_rx_desc_word12;

#endif