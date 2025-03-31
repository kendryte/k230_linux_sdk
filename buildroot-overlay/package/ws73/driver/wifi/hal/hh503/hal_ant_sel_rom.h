/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Header file of HAL ANT SEL external public interfaces .
 * Create: 2023-3-8
 */

#ifndef HAL_ANT_SEL_H
#define HAL_ANT_SEL_H

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void hal_set_ant_sel_phy_enable(osal_u8 phy_enable);
osal_void hal_set_ant_sel_phy_cfg_man(osal_u8 cfg_man);
osal_u32 hal_get_ant_sel_phy_enable(osal_void);
osal_u32 hal_get_ant_sel_phy_cfg_man(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif