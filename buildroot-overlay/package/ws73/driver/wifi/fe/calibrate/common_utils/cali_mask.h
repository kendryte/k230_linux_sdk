/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准mask相关
 * Create: 2022-10-19
 */

#ifndef __CALI_MASK_H__
#define __CALI_MASK_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"
#include "cali_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define CALI_DATA_MASK_ONLINE_CALI 0x10
extern osal_u32 g_cali_mask[WLAN_CALI_BAND_BUTT];
osal_void cali_set_cali_mask(const osal_u8 *param);
static inline osal_u32 cali_get_cali_mask(wlan_channel_band_enum_uint8 band)
{
    if (band >= WLAN_CALI_BAND_BUTT) {
        return 0;
    }
    return g_cali_mask[band];
}
osal_void cali_set_cali_data_mask(const osal_u8 *param);
osal_u8 cali_get_cali_data_mask(osal_void);
osal_void cali_set_cali_done_flag(osal_u8 is_done);
osal_u8 cali_get_cali_done_flag(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
