/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize power 头文件
 * Create: 2022-11-26
 */

#ifndef _FE_RF_CUSTOMIZE_POWER_H_
#define _FE_RF_CUSTOMIZE_POWER_H_

#include "osal_types.h"
#include "mac_vap_ext.h"
#include "power_ctrl_spec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define FE_CUSTOMIZE_MAX_CHIP_POWER_LEN 1 // 芯片最大功率数据长度

// 法规功率数据长度
#define FE_CUSTOMIZE_LIMIT_POWER_MODE 4 // 四种模式 11b 11g 20M 40M
#define FE_CUSTOMIZE_LIMIT_POWER_LEN 56
#define FE_CUSTOMIZE_LIMIT_MAX_POWER 0xff
// 比吸收率功率数据长度
#define FE_CUSTOMIZE_SAR_POWER_LEN 3
#define FE_CUSTOMIZE_CTA_COEF_FLAG_LEN 1
#define FE_CUSTOMIZE_SAR_MAX_POWER 0xff
typedef struct {
    osal_u8 chip_max_power;
    osal_u8 target_power[FE_CUSTOMIZE_TARGET_POWER_LEN];
    osal_u8 limit_power[FE_CUSTOMIZE_LIMIT_POWER_LEN];
    osal_u8 sar_power[FE_CUSTOMIZE_SAR_POWER_LEN];
    osal_u8 special_cta_coef_flag; /* 无委认证-特殊滤波系数标志位 */
    osal_u8 rsv[2];
} fe_custom_power_2g_stru;

typedef struct {
    fe_custom_power_2g_stru power_2g;
} fe_custom_power_stru;

osal_void fe_custom_set_chip_max_power(osal_u8 band, osal_u8 *power, osal_u16 len);
osal_u8 fe_custom_get_chip_max_power(osal_u8 band);
osal_void fe_custom_set_target_power(osal_u8 band, osal_u8 rate, osal_u8 power);
osal_void fe_custom_config_all_target_power(osal_u8 band, osal_u8 *power, osal_u16 len);
osal_u8 fe_custom_get_target_power(osal_u8 band, osal_u8 rate_idx);
osal_void fe_custom_set_limit_power(osal_u8 band, osal_u8 *power, osal_u16 len);
osal_u8 fe_custom_get_limit_power(osal_u8 band, osal_u8 rate_idx, osal_u8 ch_idx);
osal_void fe_custom_set_sar_power(osal_u8 band, osal_u8 *power, osal_u16 len);
osal_u8 fe_custom_get_sar_power(osal_u8 band, osal_u8 level);
osal_void fe_custom_set_cta_coef_flag(osal_u8 band, osal_u8 *value, osal_u16 len);
osal_u8 fe_custom_get_cta_coef_flag(osal_u8 band);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#endif  // _FE_RF_CUSTOMIZE_POWER_H_
