/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_ant_sel head
 * Create: 2023-3-6
 */

#ifndef HMAC_ANT_SEL_H
#define HMAC_ANT_SEL_H

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    HMAC_ANT_SEL_CONTROL_FROM_PHY = 0,
    HMAC_ANT_SEL_CONTROL_FROM_MAC,
    HMAC_ANT_SEL_CONTROL_FROM_BUTT
} hmac_phy_ant_sel_param_enum;

static osal_u32 hmac_ant_sel_init_weakref(osal_void) __attribute__ ((weakref("hmac_ant_sel_init"), used));
static osal_void hmac_ant_sel_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_ant_sel_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_auto_adjust_freq_rom.h */
