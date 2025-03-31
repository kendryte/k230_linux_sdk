/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_soc.h.
 * Create: 2020-7-3
 */

#ifndef __HAL_SOC_H__
#define __HAL_SOC_H__

#include "osal_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HH503_SSI0_MST_RB_BASE_ADDR            0x40080000

typedef enum {
    WITP_SOC_BANK_0 = 0,
    WITP_SOC_RF_W_C0_CTL_1,
    WITP_SOC_RF_W_C1_CTL_2,
    WITP_SOC_W_CTL_3,
    WITP_SOC_COEX_CTL_4,

    WITP_SOC_BANK_BUTT
} hh503_soc_bank_idx_enum;

#ifdef BOARD_ASIC_WIFI
#define HH503_SOC_PHY_WLMAC_CGR 0x4001F128
typedef union {
    struct {
        osal_u32 xCiNLw_CLwT6udwSWSw6Lowh_ : 1;
        osal_u32 reserved                : 1;
        osal_u32 xSG6wSGBhwFBqwhdS_      : 6;
        osal_u32 xSG6wSGBhw6oKWh_        : 1;
        osal_u32 xTLiuwFBqwWh_           : 1;
        osal_u32 reserved_0              : 22;
    } bits;

    osal_u32 u32;
} u_phy_wlmac_cgr;
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
