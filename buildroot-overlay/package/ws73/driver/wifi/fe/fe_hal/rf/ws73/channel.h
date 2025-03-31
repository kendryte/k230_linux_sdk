/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: channel config header for ABB 03 mpw RF.
 */

#ifndef __RF_CHANNEL_H__
#define __RF_CHANNEL_H__

#include "fe_rf_dev_attach.h"
#include "hal_common_ops_rom.h"
#include "cali_base_def.h"
#include "fe_hal_rf_reg_if.h"

// rf d_top_rf_band_local[1:0] 定义 00: LowBand 01, MidBand,10 high band
#define HH503_CALI_CHN_BAND_INFO_2422    0
#define HH503_CALI_CHN_BAND_INFO_2447    1
#define HH503_CALI_CHN_BAND_INFO_2472    2
// 校准三个频点的信道索引
#define CALI_2G_BAND_0_CHANNEL_INDEX 0
#define CALI_2G_BAND_1_CHANNEL_INDEX 6
#define CALI_2G_BAND_2_CHANNEL_INDEX 12

#define OFFSET_PLL_TOP_EN_REG1  1
#define OFFSET_PLL_INT_N_REG2   2
#define OFFSET_PLL_FRAC_MSB_REG2    2
#define OFFSET_PLL_FRAC_LSB_REG3    3
#define OFFSET_PLL_TESTMODE_REG4    4
#define OFFSET_PLL_TEST_REG5    5
#define OFFSET_PLL_REFBUF_REG6  6
#define OFFSET_PLL_DTC_REG7 7
#define OFFSET_PLL_DTC_SPD_REG8 8
#define OFFSET_PLL_SPD_REG9 9
#define OFFSET_PLL_GM_SPD_REG10 10
#define OFFSET_PLL_PFD_CP_REG11 11
#define OFFSET_PLL_LPF_REG12    12
#define OFFSET_PLL_LPF_REG13    13
#define OFFSET_PLL_CNT_CMP_REG14    14
#define OFFSET_PLL_NDIV_REG15   15
#define OFFSET_PLL_VCO_REG16    16
#define OFFSET_PLL_VCO_REG17    17
#define OFFSET_PLL_VCO_REG18    18
#define OFFSET_PLL_VCO_REG19    19
#define OFFSET_PLL_RESERVE_REG20    20
#define OFFSET_PLL_RESERVE_REG21    21
#define OFFSET_PLL_RESERVE_REG22    22
#define OFFSET_PLL_RESERVE_REG23    23
#define OFFSET_PLL_RESERVE_REG24    24
#define OFFSET_PLL_RESERVE_REG25    25
#define OFFSET_PLL_WAITTIME_REG26   26
#define OFFSET_PLL_FSM_DEBUG_REG27  27
#define OFFSET_PLL_LD_CT_REG28  28
#define OFFSET_PLL_CT_REG29 29
#define OFFSET_PLL_CT_REG30 30
#define OFFSET_PLL_CT_REG31 31
#define OFFSET_PLL_CT_REG32 32
#define OFFSET_PLL_CT_REG33 33
#define OFFSET_PLL_CT_REG34 34
#define OFFSET_PLL_WAITTIME_REG35   35
#define OFFSET_PLL_DSM_REG36    36
#define OFFSET_PLL_DSM_REG37    37
#define OFFSET_PLL_DSM_REG38    38
#define OFFSET_PLL_DSM_REG39    39
#define OFFSET_PLL_DSM_REG40    40
#define OFFSET_PLL_DSM_REG41    41
#define OFFSET_PLL_DSM_REG42    42
#define OFFSET_PLL_DSM_REG43    43
#define OFFSET_PLL_DSM_REG44    44
#define OFFSET_PLL_DSM_REG45    45
#define OFFSET_PLL_DSM_REG46    46
#define OFFSET_PLL_DSM_REG47    47
#define OFFSET_PLL_SRAM_REG48   48
#define OFFSET_PLL_WAITTIME_REG49   49
#define OFFSET_PLL_WAITTIME_REG50   50
#define OFFSET_PLL_WAITTIME_REG51   51
#define OFFSET_PLL_LD_VCO_REG52 52
#define OFFSET_PLL_LD_CT_CTRL_FSM_REG53 53
#define OFFSET_PLL_CNT_REG54    54
#define OFFSET_PLL_CNT_REG55    55
#define OFFSET_PLL_SDM_REG56    56
#define OFFSET_PLL_SDM_REG57    57
#define OFFSET_PLL_SDM_REG58    58
#define OFFSET_PLL_SDM_REG59    59

/* PLL配置结构 */
typedef struct {
    osal_u16 div_int;      /* 分频整数部分 */
    osal_u16 div_frac;     /* 分频小数部分  */
} rf_pll_div_cfg_stru;

#define HAL_PLL_LOCK_TIME           50
#define HAL_PLL_LOCK_TIME_FIRST     200
#define HAL_RF_PLL_2G_40M_OFFSET    2

osal_void fe_hal_rf_update_band_info(hal_rf_dev *rf_dev, osal_u8 freq, osal_u8 band_info);
rf_pll_div_cfg_stru fe_hal_rf_calc_pll_div_param(osal_u16 pll_idx);
osal_void fe_hal_rf_cfg_pll_div(hal_rf_dev *rf_dev, osal_u16 div_int, osal_u16 div_frac);
osal_void fe_hal_rf_lock_pll(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 wait_lock_time, osal_u8 locked_now);
osal_void fe_hal_rf_pre_cfg_pll(hal_rf_dev *rf_dev);
osal_void fe_hal_rf_set_center_freq(hal_rf_dev *rf_dev, hal_rf_chan_info *rf_chan);
#ifndef _PRE_WLAN_ATE
osal_u32 fe_hal_rf_pll_temp_update_timer_handle(osal_void *arg);
#endif
#endif
