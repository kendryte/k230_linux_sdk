/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize相关
 * Create: 2022-11-26
 */

#ifndef _FE_RF_CUSTOMIZE_H_
#define _FE_RF_CUSTOMIZE_H_

#include "cali_outer_hal_if.h"
#include "cali_outer_os_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
// rf相关的定制化参数结构体
typedef struct {
    /* cca ed thr */
    hal_cfg_custom_cca_stru cfg_cus_cca;
} fe_rf_custom_stru;
#endif

// 获取定制化数据中的cca门限默认值
osal_s8 fe_rf_get_customized_cca_ed(osal_u32 cca_ed_type, wlan_channel_band_enum_uint8 band);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
