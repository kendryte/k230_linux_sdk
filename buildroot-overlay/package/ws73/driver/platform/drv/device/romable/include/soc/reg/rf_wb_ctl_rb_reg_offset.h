// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  rf_wb_ctl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __RF_WB_CTL_RB_REG_OFFSET_H__
#define __RF_WB_CTL_RB_REG_OFFSET_H__

/* RF_WB_CTL_RB Base address of Module's Register */
#define RF_WB_CTL_RB_BASE (0x40014000)


/* **************************************************************************** */
/*           HH503 RF_WB_CTL_RB Registers' Definitions                          */
/* **************************************************************************** */

#define RF_WB_CTL_RB_SYS_CTL_ID_REG              (RF_WB_CTL_RB_BASE + 0x0)  /* SYS CTL ID寄存器 */
#define RF_WB_CTL_RB_RF_WL_REG_CLK_CTL_REG       (RF_WB_CTL_RB_BASE + 0x10) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_RF_WL_REG_SOFT_RSTN_REG     (RF_WB_CTL_RB_BASE + 0x14) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_RF_WL_REG_CMD_SOFT_RSTN_REG (RF_WB_CTL_RB_BASE + 0x18) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_RF_WL_REG_CMD_FIFO_STS_REG  (RF_WB_CTL_RB_BASE + 0x1C) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_RF_WL_CBB_DIS_REG           (RF_WB_CTL_RB_BASE + 0x20) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_RF_BT_CBB_DIS_REG           (RF_WB_CTL_RB_BASE + 0x24) /* RF REG控制寄存器 */
#define RF_WB_CTL_RB_ABB_ADC_CTRL_REG            (RF_WB_CTL_RB_BASE + 0x30) /* ABB_ADC控制寄存器 */
#define RF_WB_CTL_RB_ABB_DAC_CTRL_REG            (RF_WB_CTL_RB_BASE + 0x34) /* ABB_DAC控制寄存器 */
#define RF_WB_CTL_RB_ABB_EN_CTRL_REG             (RF_WB_CTL_RB_BASE + 0x38) /* ABB控制寄存器 */
#define RF_WB_CTL_RB_CFG_RF_MAN_EN_CTRL_REG      (RF_WB_CTL_RB_BASE + 0x40) /* rf端口控制软控使能寄存器 */
#define RF_WB_CTL_RB_CFG_TRXEN_CTRL_REG          (RF_WB_CTL_RB_BASE + 0x44) /* RF端口使能信号软控数据寄存器 */
#define RF_WB_CTL_RB_CFG_DCOC_IQ_CTRL_REG        (RF_WB_CTL_RB_BASE + 0x48) /* DCOC控制寄存器 */
#define RF_WB_CTL_RB_CFG_PA_IDX_CTRL_REG         (RF_WB_CTL_RB_BASE + 0x4C) /* IDX控制寄存器 */
#define RF_WB_CTL_RB_CFG_LPF_CTRL_REG            (RF_WB_CTL_RB_BASE + 0x50) /* GAIN控制寄存器 */
#define RF_WB_CTL_RB_CFG_TEMP_CTRL_REG           (RF_WB_CTL_RB_BASE + 0x54) /* TEMP控制寄存器 */
#define RF_WB_CTL_RB_CFG_RF_DIAG_MUX_REG         (RF_WB_CTL_RB_BASE + 0x58) /* 维测控制寄存器 */
#define RF_WB_CTL_RB_CFG_WL_ADC_MUX_REG          (RF_WB_CTL_RB_BASE + 0x5C) /* WLADC时钟动态切换控制寄存器 */
#define RF_WB_CTL_RB_CFG_PPA_CODE_REG            (RF_WB_CTL_RB_BASE + 0x60) /* 动态功率校准配置寄存器 */

#endif // __RF_WB_CTL_RB_REG_OFFSET_H__
