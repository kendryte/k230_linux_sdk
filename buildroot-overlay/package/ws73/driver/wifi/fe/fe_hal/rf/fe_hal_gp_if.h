/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: 校准用到的gp接口
 */
#ifndef _FE_HAL_GP_IF_H_
#define _FE_HAL_GP_IF_H_

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_u32 fe_hal_gp_get_ppa_lut_lock_state(osal_void);
osal_void fe_hal_gp_set_ppa_lut_lock_state(osal_u32 val);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif