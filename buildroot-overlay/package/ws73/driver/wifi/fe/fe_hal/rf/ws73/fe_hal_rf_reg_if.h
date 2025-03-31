/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf寄存器定义
 * Date: 2022-11-26
*/


#ifndef _FE_HAL_RF_REG_IF_H_
#define _FE_HAL_RF_REG_IF_H_

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 配置寄存器的类型 */
typedef enum {
    RF_REG = 0,
    PLL_REG = 1,
    SOC_REG = 2,

    REG_BUTT
} fe_hal_rf_reg_type_enum;
typedef osal_u8 fe_hal_rf_reg_type_enum_uint8;

// band 枚举
typedef enum {
    CALI_RF_2G_BAND_SEL_1 = 1,  // band 枚举从1开始
    CALI_RF_2G_BAND_SEL_2,
    CALI_RF_2G_BAND_SEL_3,

    CALI_RF_2G_BAND_SEL_BUTT
} fe_hal_rf_2g_band_sel_enum;
typedef osal_u8 fe_hal_rf_2g_band_sel_enum_uint8;
// ctune val index 枚举
typedef enum {
    CALI_RF_CTUNE_VAL_INDEX_0,
    CALI_RF_CTUNE_VAL_INDEX_1,
    CALI_RF_CTUNE_VAL_INDEX_2,
    CALI_RF_CTUNE_VAL_INDEX_3,
    CALI_RF_CTUNE_VAL_INDEX_4,
    CALI_RF_CTUNE_VAL_INDEX_5,
    CALI_RF_CTUNE_VAL_INDEX_6,
    CALI_RF_CTUNE_VAL_INDEX_7,

    CALI_RF_CTUNE_VAL_INDEX_BUTT
} fe_hal_rf_ctune_val_index_enum;
typedef osal_u8 fe_hal_rf_ctune_val_index_enum_uint8;

/*  WL TRX REG OFFSET */
#define OFFSET_WB_LOCAL_CTRL_EN_REG0                      0
#define OFFSET_WB_LOCAL_CTRL_TX_REG1                      1
#define OFFSET_WB_LOCAL_CTRL_GAIN_REG2                    2
#define OFFSET_WB_LOCAL_CTRL_GAIN_REG3                    3
#define OFFSET_WB_LOCAL_CTRL_DCOC_REG4                    4
#define OFFSET_WB_LOCAL_CTRL_TEMP_REG5                    5
#define OFFSET_WB_MODE_SEL_REG6                           6
#define OFFSET_WB_MODE_SEL_REG7                           7
#define OFFSET_WB_RF_LO_TX_ENABLE_REG8                    8
#define OFFSET_WB_RF_RX_ENABLE_REG9                       9
#define OFFSET_WB_RF_ABB_ENABLE_REG10                     10
#define OFFSET_WB_RF_LO_TX_TOP_TEST_REG11                 11
#define OFFSET_WB_RF_RX_TOP_TEST_REG12                    12
#define OFFSET_WB_RF_ABB_TEST_REG13                       13
#define OFFSET_WB_ADC_CTRL_REG14                          14
#define OFFSET_WB_ADC_CTRL_REG15                          15
#define OFFSET_WB_DAC_CTRL_REG16                          16
#define OFFSET_WB_DAC_CTRL_REG17                          17
#define OFFSET_WB_TEST_MUX_SWITCH_REG18                   18
#define OFFSET_WB_TEST_MUX_SWITCH_REG19                   19
#define OFFSET_WL_RF_TRX_RC_CODE_REG20                    20
#define OFFSET_WL_RF_TRX_RC_CODE_REG21                    21
#define OFFSET_BT_RF_TRX_RC_CODE_REG22                    22
#define OFFSET_BT_RF_TRX_RC_CODE_REG23                    23
#define OFFSET_WB_RF_TX_RC_CODE_REG24                     24
#define OFFSET_WB_RF_TRX_IQCAL_CONTROL_REG25              25
#define OFFSET_WB_RF_TRX_SWAP_REG26                       26
#define OFFSET_WB_RFABB_LPF_BW_MAP_REG27                  27
#define OFFSET_WB_FB_EN_LUT_REG28                         28
#define OFFSET_WB_FB_EN_LUT_REG29                         29
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG30                  30
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG31                  31
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG32                  32
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG33                  33
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG34                  34
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG35                  35
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG36                  36
#define OFFSET_WB_FB_GAIN_LUT_CTRL_REG37                  37
#define OFFSET_WB_RF_LOGEN_2G_TRIM_REG38                  38
#define OFFSET_D_WB_RF_LOGEN_SSB_TUNE_REG39               39
#define OFFSET_D_WB_RF_LOGEN_SSB_IFB_GAIN_TRIM_REG40      40
#define OFFSET_WB_RF_RX_SETUP_REG41                       41
#define OFFSET_WB_RF_RX_MANUAL_REG42                      42
#define OFFSET_WB_RF_RX_MANUAL_REG43                      43
#define OFFSET_WB_RF_RX_MANUAL_REG44                      44
#define OFFSET_WB_RF_RX_LNA_MUX_REG45                     45
#define OFFSET_WB_RF_RX_LNA_MUX_REG46                     46
#define OFFSET_WB_RF_RX_LNA_MUX_REG47                     47
#define OFFSET_WB_RF_RX_LNA_MUX_REG48                     48
#define OFFSET_WB_RF_RX_LNA_MUX_REG49                     49
#define OFFSET_WB_RF_RX_LNA_MUX_REG50                     50
#define OFFSET_WB_RF_RX_LNA_MUX_REG51                     51
#define OFFSET_WB_RF_RX_LNA_MUX_REG52                     52
#define OFFSET_WB_RF_RX_LNA_MUX_REG53                     53
#define OFFSET_WB_RF_RX_LNA_MUX_REG54                     54
#define OFFSET_WB_RF_RX_LNA_MUX_REG55                     55
#define OFFSET_WB_RF_RX_LNA_MUX_REG56                     56
#define OFFSET_WB_RF_RX_LNA_MUX_REG57                     57
#define OFFSET_WB_RF_RX_LNA_MUX_REG58                     58
#define OFFSET_WB_RF_RX_LNA_MUX_REG59                     59
#define OFFSET_WB_RF_RX_LNA_MUX_REG60                     60
#define OFFSET_WB_RF_RX_LNA_MUX_REG61                     61
#define OFFSET_WB_RF_RX_LNA_MUX_REG62                     62
#define OFFSET_WB_RF_RX_LNA_BIAS_CONTROL_REG63            63
#define OFFSET_WB_RF_RX_LNA_BIAS_CONTROL_REG64            64
#define OFFSET_WB_RF_RX_MIX_TIA_MUX_REG65                 65
#define OFFSET_WB_RF_RX_MIX_TIA_MUX_REG66                 66
#define OFFSET_WB_RF_RX_MIX_TIA_MUX_REG67                 67
#define OFFSET_WB_RF_RX_MIX_TIA_MUX_REG68                 68
#define OFFSET_WB_RF_RX_GAIN_LUT_REG69                    69
#define OFFSET_WB_RF_RX_GAIN_LUT_REG70                    70
#define OFFSET_WB_RF_RX_GAIN_LUT_REG71                    71
#define OFFSET_WB_RF_RX_GAIN_LUT_REG72                    72
#define OFFSET_WB_RF_RX_GAIN_LUT_REG73                    73
#define OFFSET_WB_RF_RX_GAIN_LUT_REG74                    74
#define OFFSET_WB_RF_RX_GAIN_LUT_REG75                    75
#define OFFSET_WB_RF_RX_GAIN_LUT_REG76                    76
#define OFFSET_WB_TX_RF_CONFIG_REG77                      77
#define OFFSET_WB_TX_RF_CALIBRATION_REG78                 78
#define OFFSET_WB_TX_TRIM_BIAS_MODE_REG79                 79
#define OFFSET_WB_TX_PA_TRIM_MANUAL_REG80                 80
#define OFFSET_WB_TX_PA_TRIM_MANUAL_REG81                 81
#define OFFSET_WB_TX_PA_TRIM_MANUAL_REG82                 82
#define OFFSET_WB_TX_TRIM_BIAS_MANUAL_REG83               83
#define OFFSET_TIA_VCM_REG84                              84
#define OFFSET_TIA_IBG_REG85                              85
#define OFFSET_TIA_CC_REG86                               86
#define OFFSET_TIA_RZ_REG87                               87
#define OFFSET_TIA_CMFB_R_REG88                           88
#define OFFSET_TIA_ITRIM_REG89                            89
#define OFFSET_LPF_VCM_REG90                              90
#define OFFSET_LPF_VCM_REG91                              91
#define OFFSET_LPF_IBG_REG92                              92
#define OFFSET_LPF_IBG_REG93                              93
#define OFFSET_LPF_RIPCAL_REG94                           94
#define OFFSET_LPF_RIPCAL_REG95                           95
#define OFFSET_LPF_RIPCAL_REG96                           96
#define OFFSET_LPF_CC_REG97                               97
#define OFFSET_LPF_RZ_REG98                               98
#define OFFSET_LPF_CMFB_R_REG99                           99
#define OFFSET_LPF_ITRIM_REG100                           100
#define OFFSET_VGA_VCM_REG101                             101
#define OFFSET_VGA_IBG_REG102                             102
#define OFFSET_VGA_ITRIM_REG103                           103
#define OFFSET_VGA_CC_LUT_REG104                          104
#define OFFSET_VGA_CC_LUT_REG105                          105
#define OFFSET_VGA_RZ_LUT_REG106                          106
#define OFFSET_VGA_CMFB_R_LUT_REG107                      107
#define OFFSET_VGA_CMFB_R_LUT_REG108                      108
#define OFFSET_DCOC_REG109                                109
#define OFFSET_TX_DCCAL_REG109                            109
#define OFFSET_PDBUF_CTRL_REG109                          109
#define OFFSET_WL_TX_PDET2G_CTRL_REG110                   110
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG111               111
#define OFFSET_WB_RF_LOGEN_SSB_VCTRL_REG111               111
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG112               112
#define OFFSET_D_WB_RF_TX_PA_VMID_LUT_REG112              112
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG113               113
#define OFFSET_D_WB_RF_TX_PPA_VCAS_SEL_LUT_REG113         113
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG114               114
#define OFFSET_D_WB_RF_TX_PPA_VBC_SEL_LUT_REG114          114
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG115                  115
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG116                  116
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG117                  117
#define OFFSET_D_WB_RF_TX_PPA_VBA_SEL_LUT_REG117          117
#define OFFSET_D_WB_RF_TX_PPA_VBC_SEL_LUT_REG118          118
#define OFFSET_D_WB_RF_TX_PA_ADB_VB_SEL_LUT_REG118        118
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG119               119
#define OFFSET_WB_RF_LOGEN_SSB_VCTRL_REG119               119
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG120               120
#define OFFSET_D_WB_RF_TX_PA_VMID_LUT_REG120              120
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG121               121
#define OFFSET_D_WB_RF_TX_PPA_VCAS_SEL_LUT_REG121         121
#define OFFSET_WB_TX_PA_ADB_IOUT_LUT_REG122               122
#define OFFSET_D_WB_RF_TX_PPA_VBC_SEL_LUT_REG122          122
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG123                  123
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG124                  124
#define OFFSET_WB_RF_TX_PA_VB_LUT_REG125                  125
#define OFFSET_D_WB_RF_TX_PPA_VBA_SEL_LUT_REG125          125
#define OFFSET_D_WB_RF_TX_PPA_VBC_SEL_LUT_REG126          126
#define OFFSET_D_WB_RF_TX_PA_ADB_VB_SEL_LUT_REG126        126
#define OFFSET_WL_TX_PDET2G_CTRL_REG127                   127
#define OFFSET_WL_TX_PDET2G_CTRL_REG128                   128
#define OFFSET_WB_LINECTRL_REG129                         129
#define OFFSET_WB_LINECTRL_REG130                         130
#define OFFSET_WB_LINECTRL_REG131                         131

#define CALI_RF_RC_CODE_BITS        6
#define CALI_RF_CAP_BANK_BW_I_OFST  BIT_OFFSET_0
#define CALI_RF_CAP_BANK_BW_Q_OFST  BIT_OFFSET_8
#define CALI_RF_RC_END_OFST         BIT_OFFSET_15
#define CALI_RF_RC_TUNE_END_BITS    1
#define CALI_RF_2G_UPC_PPA_BITS    5

/* RC/R/C校准完成标志 */
#define CALI_RF_RC_TUNE_COMP 1
#define CALI_RF_RC_TUNE_INIT 0

#define CALI_RF_PA_2G_CSW_MAX_VAL 32
#define CALI_RF_PA_2G_LCTUNE_MAX_VAL 32
#define CALI_RF_TX_IQ_LVL_USED_NUM 2
#define CALI_RF_RX_IQ_LVL_USED_NUM 2
#define CALI_RF_TX_DC_LVL_USED_NUM 2
#define CALI_RF_IQ_PHASE_MIN_VALUE 350
#define CALI_RF_IQ_PHASE_THR_VALUE (1 << 0x9)
#define CALI_RF_IQ_PHASE_RECT_VALUE (1 << 0xA)  // 90度对应数值
#define CALI_IQ_PHASE_SHIFT_TONE_FREQ   2500   // IQ校准单音方案计算相角时使用的单音频率

// rx_dc用到的一些rf相关宏
#define CALI_RF_RX_DC_FIX_BIT (8) /* rx dc vga 最小增益值 定点化放大位数 */
#define CALI_RF_RX_DC_DEFAULT_ANALOG_COMP (0x8080) /* rx dc 默认的模拟补偿值 */
#define CALI_RF_RX_DC_VGA_LVL_USED_NUM (2) /* rx dc补偿值vga档位数目 */
#define CALI_RF_RX_DC_VGA_MAX_GAIN (4057) /* rx dc vga 最大增益值 定点化放大 */
#define CALI_RF_RX_DC_VGA_MIN_GAIN (128) /* rx dc vga 最小增益值 定点化放大 */

// tx_dc用到的一些rf相关宏
#define CALI_RF_TX_DC_DC3_VALUE    0x100    /* DC3=(64mv/512mv)*2048=256 */
#define CALI_20M_BANDWIDTH_ADJUST         129             /* ACI刷新20M的RC CODE缩放值 */

#define HH503_RF_FREQ_2_CHANNEL_NUM  14  /* 2g支持信道数目 */
#define HH503_RF_FREQ_5_CHANNEL_NUM  29  /* 5g支持信道数目 */

#define CALI_RF_ADDR_SHIFT_BITS           2

#define HH503_WL_PLL1_RFLDO789_ENABLE      0xFF00
// 校准异常保护相关宏定义
#define HAL_2G_TX_PWR_VAL_MAX       (0x1F3F)                       /* ppa code限定最大值 */
#define HAL_2G_TX_PWR_VAL_MIN       (0x1101)                       /* ppa code限定最小值 */
#define HAL_2G_TX_PWR_VAL_DEFAULT   (0x1C32)                       /* ppa code超过限定值时的赋值 */
// 模拟校准搜索初始值
#define HAL_TXPWR_SEARCH_UPC_MAX    (0x1F3F)
#define HAL_TXPWR_SEARCH_UPC_MIN    (0x1101)
#define HAL_TXPWR_SEARCH_UPC_NORMAL (0x1C32)
#define HAL_TXPWR_SEARCH_UPC_TH     (0x1F)
#define HAL_TXPWR_SEARCH_UPC_STEP   (0x05)

osal_void hal_soc_cfg_ppa_code_gm_man(osal_u16 gm_code);
osal_void hal_soc_cfg_ppa_code_unit_man(osal_u16 ppa_code_unit);
osal_void hal_soc_cfg_ppa_code_man(osal_u16 ppa_code);
osal_void hal_soc_cfg_dac_index_man(osal_u16 dac_index);
osal_void hal_soc_cfg_pa_code_man(osal_u16 pa_code);
osal_u32 hal_rf_get_soc_reg_base(osal_u8 rf_id);
osal_void hal_rf_set_pll2_reg(osal_u16 div_int);
osal_void hal_rf_set_pll1_reg(osal_u16 pll_top);
osal_u8 hal_rf_get_fb_gain_lna_lut(osal_u8 fb_gain_local);
osal_void hal_rf_set_abb78_d_wb_rf_tx_ppa_cap_sw(osal_u16 x4OqyOwtOR8OGGzOCzGOsq_);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
