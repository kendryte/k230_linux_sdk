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

/*****************************************************************************
  FPGA基地址定义
*****************************************************************************/
#define HH503_RF_W_C0_REG_BASE_ADDR            0x40040800
#define HH503_RF_W_C1_REG_BASE_ADDR            0x40042800

#define HH503_RF_W_PLL_C0_REG_BASE_ADDR        0x40041000
#define HH503_RF_W_PLL_C1_REG_BASE_ADDR        0x40043000

/* 临时存放RF ABB寄存器,RF 寄存器确认后删除 */
#ifdef BUILD_UT
#define HH503_RF_W_C0_CTL_BASE (osal_u32)g_soc_w_c0_reg_base_addr
#define HH503_RF_W_C1_CTL_BASE (osal_u32)g_soc_w_c1_reg_base_addr
#else
#define HH503_RF_W_C0_CTL_BASE 0x40040000
#define HH503_RF_W_C1_CTL_BASE 0x40042000
#endif
#define HH503_RF_W_C0_CTL_WL_TRX_MAN_C0_REG (HH503_RF_W_C0_CTL_BASE + 0x110)
#define HH503_RF_W_C0_CTL_WL_TRX_SEL_C0_REG (HH503_RF_W_C0_CTL_BASE + 0x114)
#define HH503_RF_W_C0_CTL_BASE_0x204 (HH503_RF_W_C0_CTL_BASE + 0x204)
#define HH503_RF_W_C0_CTL_BASE_0x200 (HH503_RF_W_C0_CTL_BASE + 0x200)
#define HH503_RF_W_C0_CTL_BASE_0x220 (HH503_RF_W_C0_CTL_BASE + 0x220)
#define HH503_RF_W_C0_CTL_BASE_0x218 (HH503_RF_W_C0_CTL_BASE + 0x218)
#define HH503_RF_W_C0_CTL_BASE_0x210 (HH503_RF_W_C0_CTL_BASE + 0x210)
#define HH503_RF_W_C0_CTL_BASE_0x230 (HH503_RF_W_C0_CTL_BASE + 0x230)

#define HH503_RF_W_C0_CTL_WL_TRX_MAN_C1_REG (HH503_RF_W_C1_CTL_BASE + 0x110)
#define HH503_RF_W_C0_CTL_WL_TRX_SEL_C1_REG (HH503_RF_W_C1_CTL_BASE + 0x114)
#define HH503_RF_W_C1_CTL_BASE_0x204 (HH503_RF_W_C1_CTL_BASE + 0x204)
#define HH503_RF_W_C1_CTL_BASE_0x200 (HH503_RF_W_C1_CTL_BASE + 0x200)
#define HH503_RF_W_C1_CTL_BASE_0x220 (HH503_RF_W_C1_CTL_BASE + 0x220)
#define HH503_RF_W_C1_CTL_BASE_0x218 (HH503_RF_W_C1_CTL_BASE + 0x218)
#define HH503_RF_W_C1_CTL_BASE_0x210 (HH503_RF_W_C1_CTL_BASE + 0x210)
#define HH503_RF_W_C1_CTL_BASE_0x230 (HH503_RF_W_C1_CTL_BASE + 0x230)

/*****************************************************************************
  2 共用宏定义
*****************************************************************************/
/* wifi校准buf长度 */
#define RF_CALI_DATA_BUF_LEN              (0x6cd8)
#define RF_SINGLE_CHAN_CALI_DATA_BUF_LEN  (RF_CALI_DATA_BUF_LEN >> 1)
#define HH503_TEMPERATURE_THRESH         50

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_TOP_TST_SW2                      30
#endif /* end of _PRE_WLAN_03_MPW_RF */

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_PLL_LD_REG34           34
#endif

#define HH503_RF_LPF_GAIN_DEFAULT_VAL    0x4688
#define HH503_RF_DAC_GAIN_DEFAULT_VAL    0x24

/* PLL使能默认值配置 */

#define HH503_RF_2G_CHANNEL_NUM        13

#define HH503_CALI_CHN_INDEX_2422        0
#define HH503_CALI_CHN_INDEX_2447        1
#define HH503_CALI_CHN_INDEX_2472        2

#define CALI_RF_2G_UPC_PPA_BITS         5
#define HH503_RF_2G_UPC_PPA_ENABLE_BITS  2

#define HH503_RF_TEMP_STS_RSV                  0x7

#define HH503_RF_TEMP_INVALID                   (-100)   /* 温度非法值 */

#define HH503_RF_2G_UPC_PPA_2422_OFST            BIT_OFFSET_0
#define HH503_RF_2G_UPC_PPA_2447_OFST            BIT_OFFSET_5
#define HH503_RF_2G_UPC_PPA_2472_OFST            BIT_OFFSET_10

#define HH503_RF_2G_PPA_UPC_BITS     5
#define HH503_RF_BAND_INFO_BITS      3
#define HH503_RF_BAND_WIDTH_BITS     1

#define HH503_RF_2G_UPC_BITS         8
#define HH503_RF_2G_UPC_GC_OFST      BIT_OFFSET_0
#define HH503_RF_BAND_INFO_OFST      BIT_OFFSET_0
#define HH503_RF_BAND_WIDTH_OFST     BIT_OFFSET_0
#define HH503_RF_RX_PPF_OFST         BIT_OFFSET_12

/* WL TRX REG OFFSET */
#define HH503_WL_TRX_ENABLE_REG0               0
#define HH503_WL_TRX_ENABLE_REG1               1
#define HH503_WL_TRX_ENABLE_REG2               2
#define HH503_WL_TRX_ENABLE_REG3               3
#define HH503_WL_RX_MODULES_EN                 4
#define HH503_WL_TX_MODULES_EN                 5
#define HH503_LOGEN_EN                         6
#define HH503_WL_CALI_MODULES_EN_REG7          7
#define HH503_WL_CALI_MODULES_EN_REG8          8
#define HH503_WL_TRX_ENABLE_REG9               9
#define HH503_WL_RX_AGC_CTRL                   10
#define HH503_WL_TX_GAIN_CTRL                  11
#define HH503_WL_BAND_SEL                      12
#define HH503_WL_TEMP_CODE                     13
#define HH503_WL_C_R_CODE                      14
#define HH503_WL_RX_RC_CODE                    15
#define HH503_WL_TX_RC_CODE                    16
#define HH503_WL_RX_RC_CODE_VGA_REG17          17
#define HH503_WL_RX_RC_CODE_VGA_REG18          18
#define HH503_WL_LPF_BW                        19
#define HH503_WL_RX_DCOC                       20
#define HH503_WL_RX_TST_MODE                   21
#define HH503_WL_TX_TST_MODE                   22
#define HH503_WL_TST_SW1                       25
#define HH503_WL_TST_SW2_REG26                 26
#define HH503_WL_TST_SW2_REG27                 27
#define HH503_WL_TST_BUF                       28
#define HH503_TOP_TST_SW1                      29
#define HH503_TOP_TST_SW2                      30
#define HH503_WL_TOP_RESERVED                  31
#define HH503_TX2G_PPA_VC_CTL                  32
#define HH503_TX2G_PPA_VC_SW                   33
#define HH503_TX2G_PA_GAINCTL                  35
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG36    36
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG37    37
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG38    38
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG39    39
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG40    40
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG41    41
#define HH503_WL_TX_UPC5_PPAGAIN_CTRL_REG42    42
#define HH503_WL_TX_UPC5_LC_TANK_REG43         43
#define HH503_WL_TX_UPC5_LC_TANK_REG44         44
#define HH503_WL_TX_UPC5_LC_TANK_REG45         45
#define HH503_WL_TX_UPC5_LC_TANK_REG46         46
#define HH503_TX2G_CURRENT_CAL_REG47           47
#define HH503_TX2G_CURRENT_CAL_REG48           48
#define HH503_2G_LNA_AUTO_CONTROL              50
#define HH503_D_WL_LNA2G_CTUNE_LUT_REG67       67
#define HH503_D_WL_LNA2G_CTUNE_LUT_REG68       68
#define HH503_RX2G_LO_DIV_EN_CTRL_REG89        89
#define HH503_WL_RX_DIVIDER_REG90              90
#define HH503_RX2G_LO_DIV_LODIV_CAL_O_REG91    91
#define HH503_5G_LNA_AUTO_CONTROL              96
#define HH503_D_WL_LNA5G_CTUNE_LUT1_2          114
#define HH503_D_WL_LNA5G_CTUNE_LUT3_4          115
#define HH503_D_WL_LNA5G_CTUNE_LUT5_6          116
#define HH503_D_WL_LNA5G_CTUNE_LUT7_8          117
#define HH503_D_WL_LNA5G_CTUNE_LUT9_10         118
#define HH503_D_WL_LNA5G_CTUNE_LUT11_12        119
#define HH503_D_WL_LNA5G_CTUNE_LUT13_14        120
#define HH503_D_WL_LNA5G_CTUNE_LUT15_16        121
#define HH503_D_WL_LNA5G_CTUNE_LUT17_18        122
#define HH503_D_WL_LNA5G_CTUNE_LUT19_20        123
#define HH503_D_WL_LPF_CMAIN                   144
#define HH503_LPF_RX_IBG_TRIM1                 146
#define HH503_LPF_TX_IBG_TRIM                  147
#define HH503_LPF_OP3_IBG_TRIM                 148
#define HH503_LPF_OP3_IBG_TRIM_REG149          149
#define HH503_LPF_RX_IBG_DOUBLE1               150
#define HH503_LPF_RX_IBG_DOUBLE2               151
#define HH503_LPF_TX_QTUNE                     157
#define HH503_VGA_ITRIM                        162
#define HH503_VGA_ITRIM_DOUBLE                 164
#define HH503_VGA_RZ_TT2                       170
#define HH503_VGA_CFB_CTRL_NORMAL_TT2          176
#define HH503_VGA_CC_CTRL_NORMAL_TT4           187
#define HH503_VGA_CC_CTRL_NORMAL_TT5           188
#define HH503_PDET_REG                         225
#define HH503_VGA_BW_RP                        226
#define HH503_WL_VGA_ITRIM                     229
#define HH503_TX2G_BIAS_CURRENT_EN             233
#define HH503_TX2G_PA_GATE_VCTL_REG236         236
#define HH503_TX2G_PA_GATE_VCTL_REG237         237
#define HH503_TX2G_PA_GATE_VCTL_REG238         238
#define HH503_TX2G_PA_GATE_VCTL_REG239         239
#define HH503_TX2G_PA_GATE_VCTL_REG240         240
#define HH503_TX2G_PA_GATE_VCTL_REG241         241
#define HH503_TX2G_PA_GATE_VCTL_REG242         242
#define HH503_TX2G_PA_GATE_VCTL_REG243         243
#define HH503_TX2G_PA_GATE_VCTL_REG244         244
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG253  253
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG254  254
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG255  255
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG256  256
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG257  257
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG258  258
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG259  259
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG260  260
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG261  261
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG262  262
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG263  263
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG264  264
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG265  265
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG266  266
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG267  267
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG268  268
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG269  269
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG270  270
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG271  271
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG272  272
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG273  273
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG274  274
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG275  275
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG276  276
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG277   277
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG278   278
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG279   279
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG280   280
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG281   281
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG282   282
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG283   283
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG284   284
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG286   286
#define HH503_TX2G_LO_DIV_LODIV_CAL_O_REG287   287
#define HH503_TX2G_PA_CTL_REG289               289
#define HH503_TX2G_PA_CTL_REG290               290
#define HH503_TX2G_PA_CTL_REG291               291
#define HH503_TX2G_PA_CTL_REG292               292
#define HH503_TX2G_PA_CTL_REG293               293
#define HH503_TX2G_PA_CURRENT_TRIM             294
#define HH503_TX2G_PA_CURRENT_TRIM_B           295
#define HH503_TX2G_RESERVE_CTRL_1              296
#define HH503_TX2G_RESERVE_CTRL_2              297
#define HH503_TX2G_RESERVE_CTRL_3              298
#define HH503_TX2G_RESERVE_CTRL_4              299
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_1         300
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_2         301
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_3         302
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_4         303
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_5         304
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_6         305
#define HH503_TX5G_UPC_MIX_GAIN_CTRL_7         306
#define HH503_WL_TX_PDET2G_CTRL_REG340         340
#define HH503_WL_TX_PDET2G_CTRL_REG341         341
#define HH503_WL_TX_PDET2G_CTRL_REG342         342
#define HH503_LOGEN_EN_REG349                  349
#define HH503_LOGEN_EN_REG358                  358
#define HH503_LOGEN_EN_REG359                  359
#define HH503_LOGEN_EN_REG360                  360
#define HH503_LOGEN_REG364                     364
#define HH503_LOGEN_REG365                     365
#define HH503_LOGEN_REG366                     366
#define HH503_LOGEN_REG367                     367
#define HH503_LOGEN_REG368                     368
#define HH503_LOGEN_REG369                     369
#define HH503_LOGEN_REG370                     370
#define HH503_LOGEN_REG371                     371
#define HH503_LOGEN_REG372                     372
#define HH503_LOGEN_REG373                     373
#define HH503_LOGEN_REG374                     374
#define HH503_LOGEN_REG375                     375
#define HH503_LOGEN_REG376                     376
#define HH503_LOGEN_REG377                     377
#define HH503_LOGEN_REG378                     378
#define HH503_LOGEN_REG379                     379
#define HH503_LOGEN_REG380                     380
#define HH503_LOGEN_REG381                     381
#define HH503_ADC_WL_REG0                      396
#define HH503_ADC_WL_REG1                      397
#define HH503_ADC_WL_REG2                      398
#define HH503_ADC_WL_REG3                      399
#define HH503_ADC_WL_REG4                      400
#define HH503_ADC_WL_REG5                      401
#define HH503_ADC_WL_REG6                      402
#define HH503_ADC_WL_REG7                      403
#define HH503_ADC_WL_REG8                      404
#define HH503_DAC_WL_REG0                      405
#define HH503_DAC_WL_REG1                      406
#define HH503_DAC_WL_REG2                      407
#define HH503_DAC_WL_REG3                      408
#define HH503_ADC_WL_RO_REG0                   409
/* end of WL TRX REG OFFSET */

#define CALI_RF_RC_CODE_BITS        7
#define CALI_RF_CAP_BANK_BW_I_OFST  BIT_OFFSET_0
#define CALI_RF_CAP_BANK_BW_Q_OFST  BIT_OFFSET_8
#define CALI_RF_RC_END_OFST         BIT_OFFSET_15
#define CALI_RF_RC_TUNE_END_BITS    1

/* RC/R/C校准完成标志 */
#define CALI_RF_RC_TUNE_COMP 1
#define CALI_RF_RC_TUNE_INIT 0

#define CALI_RF_PA_2G_CSW_MAX_VAL 32
#define CALI_RF_PA_2G_LCTUNE_MAX_VAL 32
#define CALI_RF_TX_IQ_LVL_USED_NUM 1
#define CALI_RF_RX_IQ_LVL_USED_NUM 1
#define CALI_RF_TX_DC_LVL_USED_NUM 4
#define CALI_RF_IQ_PHASE_MIN_VALUE 350
#define CALI_RF_IQ_PHASE_THR_VALUE (1 << 0x9)
#define CALI_RF_IQ_PHASE_RECT_VALUE (1 << 0xA)  // 90度对应数值
#define CALI_IQ_PHASE_SHIFT_TONE_FREQ   2500   // IQ校准单音方案计算相角时使用的单音频率

// rx_dc用到的一些rf相关宏
#define CALI_RF_RX_DC_FIX_BIT (8) /* rx dc vga 最小增益值 定点化放大位数 */
#define CALI_RF_RX_DC_DEFAULT_ANALOG_COMP (0x8080) /* rx dc 默认的模拟补偿值 */
#define CALI_RF_RX_DC_VGA_LVL_USED_NUM (1) /* rx dc补偿值vga档位数目 */
#define CALI_RF_RX_DC_VGA_MAX_GAIN (5108) /* rx dc vga 最大增益值 定点化放大 */
#define CALI_RF_RX_DC_VGA_MIN_GAIN (144) /* rx dc vga 最小增益值 定点化放大 */

// tx_dc用到的一些rf相关宏
#define CALI_RF_TX_DC_DC3_VALUE    0x100    /* DC3=(64mv/512mv)*2048=256 */
#define CALI_20M_BANDWIDTH_ADJUST         129             /* ACI刷新20M的RC CODE缩放值 */

#define HH503_RF_FREQ_2_CHANNEL_NUM  14  /* 2g支持信道数目 */
#define HH503_RF_FREQ_5_CHANNEL_NUM  29  /* 5g支持信道数目 */

#define CALI_RF_ADDR_SHIFT_BITS           2

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_WL_PLL1_RFLDO789_ENABLE      0xFF00

#define HH503_RF_TCXO_FREQ           HH503_RF_TCXO_384MHZ
#define HH503_RF_TCXO_25MHZ          1
#define HH503_RF_TCXO_40MHZ          2
#define HH503_RF_TCXO_384MHZ         3
#if (HH503_RF_TCXO_FREQ == HH503_RF_TCXO_384MHZ)
/* 2G 信道PLL频率定义 */
#define HH503_RF_INTDIV_VAL_CH1              0x53C0
#define HH503_RF_FRACDIV_VAL_CH1             0x0000

#define HH503_RF_INTDIV_VAL_CH2              0x53EC
#define HH503_RF_FRACDIV_VAL_CH2             0x71C7

#define HH503_RF_INTDIV_VAL_CH3              0x5418
#define HH503_RF_FRACDIV_VAL_CH3             0xE38E

#define HH503_RF_INTDIV_VAL_CH4              0x5445
#define HH503_RF_FRACDIV_VAL_CH4             0x5555

#define HH503_RF_INTDIV_VAL_CH5              0x5471
#define HH503_RF_FRACDIV_VAL_CH5             0xC71C

#define HH503_RF_INTDIV_VAL_CH6              0x549E
#define HH503_RF_FRACDIV_VAL_CH6             0x38E3

#define HH503_RF_INTDIV_VAL_CH7              0x54CA
#define HH503_RF_FRACDIV_VAL_CH7             0xAAAA

#define HH503_RF_INTDIV_VAL_CH8              0x54F7
#define HH503_RF_FRACDIV_VAL_CH8             0x1C71

#define HH503_RF_INTDIV_VAL_CH9              0x5523
#define HH503_RF_FRACDIV_VAL_CH9             0x8E38

#define HH503_RF_INTDIV_VAL_CH10             0x5550
#define HH503_RF_FRACDIV_VAL_CH10            0x0000

#define HH503_RF_INTDIV_VAL_CH11             0x557C
#define HH503_RF_FRACDIV_VAL_CH11            0x71C7

#define HH503_RF_INTDIV_VAL_CH12             0x55A8
#define HH503_RF_FRACDIV_VAL_CH12            0xE38E

#define HH503_RF_INTDIV_VAL_CH13             0x55D5
#define HH503_RF_FRACDIV_VAL_CH13            0x5555

#define HH503_RF_INTDIV_VAL_CH14             0x5640
#define HH503_RF_FRACDIV_VAL_CH14            0x0000
#endif  /* #if (HH503_RF_TCXO_FREQ == HH503_RF_TCXO_40MHZ) */

#endif

// 校准异常保护相关宏定义
#define HAL_2G_TX_PWR_VAL_MAX       (0xa0)                       /* ppa code限定最大值 */
#define HAL_2G_TX_PWR_VAL_MIN       (0x28)                       /* ppa code限定最小值 */
#define HAL_2G_TX_PWR_VAL_DEFAULT   (0x50)                       /* ppa code超过限定值时的赋值 */
// 模拟校准搜索初始值
#define HAL_TXPWR_SEARCH_UPC_MAX    (0xc8)
#define HAL_TXPWR_SEARCH_UPC_MIN    (0x00)
#define HAL_TXPWR_SEARCH_UPC_NORMAL (0x50)
#define HAL_TXPWR_SEARCH_UPC_TH     (0x7F)
#define HAL_TXPWR_SEARCH_UPC_STEP   (0x05)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
