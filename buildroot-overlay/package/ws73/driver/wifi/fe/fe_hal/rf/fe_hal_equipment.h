/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: 产线涉及到的hal层接口
 */
#ifndef FE_HAL_EQUIPMENT_H
#define FE_HAL_EQUIPMENT_H
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_u32 fe_efuse_get_all_power_cali(osal_u8 *value, osal_u8 len);
osal_u32 fe_efuse_get_all_rssi_comp(osal_u8 *value, osal_u8 len);
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // FE_HAL_EQUIPMENT_H