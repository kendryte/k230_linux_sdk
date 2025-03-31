/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy寄存器操作接口
 * Date: 2022-10-22
 */

#ifndef __FE_HAL_PHY_REG_IF_DEVICE_H__
#define __FE_HAL_PHY_REG_IF_DEVICE_H__

#include "osal_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* 数字补偿使能定义 */
#define HH503_DIGITAL_COMP_BYPASS 1 /* 数字补偿旁路 */
#define HH503_DIGITAL_COMP_EN 0     /* 数字补偿使能 */

/* 单音信号频率枚举值定义 */
typedef enum {
    HAL_CALI_TONE_FREQ_1P25MHZ = 4, /* 1.25MHz单频 */
    HAL_CALI_TONE_FREQ_2P5MHZ  = 8, /* 2.5MHz单频 */
    HAL_CALI_TONE_FREQ_5MHZ  = 16, /* 5MHz单频 */
    HAL_CALI_TONE_FREQ_7P5MHZ  = 24, /* 7.5MHz单频 */
    HAL_CALI_TONE_FREQ_10MHZ  = 32, /* 10MHz单频 */
    HAL_CALI_TONE_FREQ_12P5MHZ = 40, /* 12.5MHz单频 */
    HAL_CALI_TONE_FREQ_20MHZ  = 64, /* 20MHz单频 */
    HAL_CALI_TONE_FREQ_17P5MHZ = 56, /* 17.5MHz单频 */
    HAL_CALI_TONE_FREQ_22P5MHZ = 72, /* 22.5MHz单频 */
    HAL_CALI_TONE_FREQ_40MHZ  = 128, /* 40MHz单频 */
    HAL_CALI_TONE_FREQ_BUTT
} hal_cali_single_tone_freq_enum;
typedef osal_u8 cali_single_tone_freq_enum_uint8;

/* 单音信号屏蔽选择枚举值定义 */
typedef enum {
    HAL_CALI_TONE_MASK_I_Q, /* 00：输出I路，Q路测试信号给DAC */
    HAL_CALI_TONE_MASK_I,   /* 01：输出I路，屏蔽Q路测试信号给DAC   */
    HAL_CALI_TONE_MASK_Q,   /* 10：输出Q路，屏蔽I路测试信号给DAC  */

    HAL_CALI_TONE_MASK_BUTT /* 11: 无效状态，同00 */
} hal_cali_single_tone_mask_enum;
typedef osal_u8 cali_single_tone_mask_enum_uint8;

/* 单音信号幅度档位枚举值定义 */
typedef enum {
    /* 暂时只支持前7个幅度档位 */
    HAL_CALI_TONE_AMP_NEG21DBFS,  /* 0: -21dbfs */
    HAL_CALI_TONE_AMP_NEG18DBFS, /* 1: -18dbfs */
    HAL_CALI_TONE_AMP_NEG15DBFS, /* 2: -15dbfs */
    HAL_CALI_TONE_AMP_NEG12DBFS, /* 3: -12dbfs */
    HAL_CALI_TONE_AMP_NEG9DBFS, /* 4: -9dbfs */
    HAL_CALI_TONE_AMP_NEG6DBFS, /* 5: -6dbfs */
    HAL_CALI_TONE_AMP_NEG3DBFS, /* 6: -3dbfs */
    HAL_CALI_TONE_AMP_0DBFS, /* 7: 0dbfs */
    HAL_CALI_TONE_AMP_BUTT
} hal_cali_single_tone_amp_enum;
typedef osal_u8 cali_single_tone_amp_enum_uint8;

typedef enum {
    FE_HAL_PHY_AL_TX_BW_20,            // BW20
    FE_HAL_PHY_AL_TX_BW_SU_ER_106,     // SU ER 106 tone
    FE_HAL_PHY_AL_TX_BW_40 = 4,        // BW40
    FE_HAL_PHY_AL_TX_BW_20_DUP_40 = 5, // BW_20 – Duplicate Legacy in 40MHz
    FE_HAL_PHY_AL_TX_BW_80 = 8,        // BW80
    FE_HAL_PHY_AL_TX_BW_20_DUP_80 = 9  // BW_20 – Duplicate Legacy in 80MHz
} fe_hal_phy_al_tx_bw_enum;
typedef osal_u8 fe_hal_phy_al_tx_bw_enum_uint8;
// 二级映射
osal_void hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0(osal_u32 xCt2OwtOGGzOCv4mOxSR3_);
osal_u32 hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0(osal_void);
osal_void hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1(osal_u32 x6Ngw_NwuuGw6iFWwodL0_);
osal_u32 hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1(osal_void);
osal_void hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2(osal_u32 xCt2OwtOGGzOCv4mOxSRZ_);
osal_u32 hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2(osal_void);
osal_void hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3(osal_u32 xCt2OwtOGGzOCv4mOxSRi_);
osal_u32 hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3(osal_void);
osal_void hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4(osal_u32 x6Ngw_NwuuGw6iFWwodLD_);
osal_u32 hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4(osal_void);
osal_void hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5(osal_u32 xCt2OwtOGGzOCv4mOxSRa_);
osal_u32 hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5(osal_void);
osal_void hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6(osal_u32 x6Ngw_NwuuGw6iFWwodLX_);
osal_u32 hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6(osal_void);
osal_void hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7(osal_u32 xCt2OwtOGGzOCv4mOxSR9_);
osal_u32 hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7(osal_void);
osal_void hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8(osal_u32 x6Ngw_NwuuGw6iFWwodLU_);
osal_u32 hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8(osal_void);
osal_void hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9(osal_u32 xCt2OwtOGGzOCv4mOxSRc_);
osal_u32 hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9(osal_void);
osal_void hal_set_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10(osal_u32 xCt2OwtOGGzOCv4mOxSRr3_);
osal_u32 hal_get_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_PHY_REG_IF_H__