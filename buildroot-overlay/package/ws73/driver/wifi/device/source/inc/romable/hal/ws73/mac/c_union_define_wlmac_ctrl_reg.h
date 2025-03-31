/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_WLMAC_CTRL_REG_H__
#define __C_UNION_DEFINE_WLMAC_CTRL_REG_H__

typedef union {
    struct {
        osal_u32 xCiNLw_CLwToywSG6wGihwh_ : 1;
        osal_u32 xsvtROwsROqx3OIzCOCRwxOo_ : 1;
        osal_u32 xsvtROwsROqx3OIzCOySsOo_ : 1;
        osal_u32 xCiNLw_CLwToywSG6wCoGqWwh_ : 1;
        osal_u32 xCiNLw_CLwToywSG6wLPwh_ : 1;
        osal_u32 xCiNLw_CLwToywSG6w_Pwh_ : 1;
        osal_u32 xCiNLw_CLwToyw6WwBNwh_ : 1;
        osal_u32 xsvtROwsROqx3OCCIGOo_ : 1;
        osal_u32 xCiNLw_CLwToywLKBuwh_ : 1;
        osal_u32 xCiNLw_CLwToywjNwh_ : 1;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_reset0;

typedef union {
    struct {
        osal_u32 xqx3OIzCOzvoOq4ROwsROsmx_ : 1;
        osal_u32 xToywSG6w6L_owTFLw_CLwCWo_ : 1;
        osal_u32 xToywSG6wjdCwTFLw_CLwCWo_ : 1;
        osal_u32 xqx3OIzCOsxzPmOq4ROwsROsmx_ : 1;
        osal_u32 xqx3OIzCOR8Oq4ROwsROsmx_ : 1;
        osal_u32 xqx3OIzCOw8Oq4ROwsROsmx_ : 1;
        osal_u32 xqx3OIzCOCmOltOq4ROwsROsmx_ : 1;
        osal_u32 xToywSG6w66SuwTFLw_CLwCWo_ : 1;
        osal_u32 xqx3OIzCORdlGOq4ROwsROsmx_ : 1;
        osal_u32 xqx3OIzCOytOq4ROwsROsmx_ : 1;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_wcbb_wdt_sel0;

typedef union {
    struct {
        osal_u32 xToywGihw6oKWh_ : 1;
        osal_u32 xqx3OCRwxOCxdmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 reserved_3 : 5;
        osal_u32 xIzCOCxdmo_ : 1;
        osal_u32 reserved_4 : 21;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_clken0;

typedef union {
    struct {
        osal_u32 xToyw6oKwSG6wjdCwWhwSGh_ : 1;
        osal_u32 xqx3OCxdOIzCOsxzPmOmoOIzo_ : 1;
        osal_u32 xqx3OCxdOIzCOR8OmoOIzo_ : 1;
        osal_u32 xqx3OCxdOIzCOw8OmoOIzo_ : 1;
        osal_u32 xqx3OCxdOCmOltOmoOIzo_ : 1;
        osal_u32 xToyw6oKw66SuwWhwSGh_ : 1;
        osal_u32 xqx3OCxdOytOmoOIzo_ : 1;
        osal_u32 xqx3OCxdORdlGOmoOIzo_ : 1;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_clken1;

typedef union {
    struct {
        osal_u32 xqx3OCxdOIzCOySsOmoOIv4_ : 1;
        osal_u32 xToyw6oKwSG6wCoGqWwWhwSiF_ : 1;
        osal_u32 xToyw6oKwSG6wLPwWhwSiF_ : 1;
        osal_u32 xToyw6oKwSG6w_PwWhwSiF_ : 1;
        osal_u32 xToyw6oKw6WwBNwWhwSiF_ : 1;
        osal_u32 xqx3OCxdOCCIGOmoOIv4_ : 1;
        osal_u32 xToyw6oKwjNwWhwSiF_ : 1;
        osal_u32 xToyw6oKwLKBuwWhwSiF_ : 1;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_clken_sel0;

typedef union {
    struct {
        osal_u32 xSG6wkVywFBqwhdS_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_0;

typedef union {
    struct {
        osal_u32 reserved_0 : 4;
        osal_u32 reserved_1 : 4;
        osal_u32 reserved_2 : 4;
        osal_u32 reserved_3 : 20;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_1;

typedef union {
    struct {
        osal_u32 xqxIzCO4lPOmo_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_crg_div_en;

typedef union {
    struct {
        osal_u32 reserved_0 : 8;
        osal_u32 reserved_1 : 7;
        osal_u32 reserved_2 : 1;
        osal_u32 reserved_3 : 6;
        osal_u32 reserved_4 : 2;
        osal_u32 reserved_5 : 6;
        osal_u32 reserved_6 : 2;
    } bits;

    osal_u32 u32;
} u_mac_mem_ctrl;

#endif
