// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  glb_ctl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __GLB_CTL_RB_REG_OFFSET_H__
#define __GLB_CTL_RB_REG_OFFSET_H__

/* GLB_CTL_RB Base address of Module's Register */
#define GLB_CTL_RB_BASE (0x40019000)

/* **************************************************************************** */
/*                      HH503 GLB_CTL_RB Registers' Definitions                            */
/* **************************************************************************** */

#define GLB_CTL_RB_HW_CTL_REG (GLB_CTL_RB_BASE + 0x0)          /* 硬件控制字寄存器 */
#define GLB_CTL_RB_CORE_RESET_PC_REG (GLB_CTL_RB_BASE + 0x100) /* CPU_PC值 */
#define GLB_CTL_RB_CORE_IRF_X1_REG (GLB_CTL_RB_BASE + 0x104)   /* CPU_LR值 */
#define GLB_CTL_RB_CORE_IRF_X2_REG (GLB_CTL_RB_BASE + 0x108)   /* CPU_SP值 */
#define GLB_CTL_RB_CORE_RA_WR_EN_REG (GLB_CTL_RB_BASE + 0x10C)
#define GLB_CTL_RB_CPU_DFX_BITS_LOAD_REG (GLB_CTL_RB_BASE + 0x110)
#define GLB_CTL_RB_CPU_DFX_BITS_CLR_REG (GLB_CTL_RB_BASE + 0x114)
#define GLB_CTL_RB_GLB_CLK_FORCE_ON_REG (GLB_CTL_RB_BASE + 0x118)
#define GLB_CTL_RB_GLB_CTL_RB_PWR_ON_LABLE_REG_REG (GLB_CTL_RB_BASE + 0x204)
#define GLB_CTL_RB_WIFI_VENDER_ID_REG (GLB_CTL_RB_BASE + 0x208)
#define GLB_CTL_RB_WIFI_PRODUCT_ID_REG (GLB_CTL_RB_BASE + 0x20C)
#define GLB_CTL_RB_RST_RECORDER_REG (GLB_CTL_RB_BASE + 0x220)
#define GLB_CTL_RB_RST_RECORD_CLR_REG (GLB_CTL_RB_BASE + 0x224)
#define GLB_CTL_RB_WDT_RST_COUNTER_REG (GLB_CTL_RB_BASE + 0x228)
#define GLB_CTL_RB_WDT_RST_COUNTER_CLR_REG (GLB_CTL_RB_BASE + 0x22C)
#define GLB_CTL_RB_UART1_RX_WAKE_BASE_FUNC_REG (GLB_CTL_RB_BASE + 0x230)
#define GLB_CTL_RB_UART1_RX_WAKE_INT_REG (GLB_CTL_RB_BASE + 0x234)
#define GLB_CTL_RB_UART1_RX_WAKE_INT_CLR_REG (GLB_CTL_RB_BASE + 0x238)
#define GLB_CTL_RB_UART1_RX_WAKE_INT_EN_REG (GLB_CTL_RB_BASE + 0x23C)
#define GLB_CTL_RB_GLB_CTL_WDT_SOFT_REQ_REG (GLB_CTL_RB_BASE + 0x300)
#define GLB_CTL_RB_CFG_DFX_REG (GLB_CTL_RB_BASE + 0x304)
#define GLB_CTL_RB_CALI_32K_TCXO_CTL_REG (GLB_CTL_RB_BASE + 0x308)    /* 32K校TCXO时钟校准寄存器 */
#define GLB_CTL_RB_CALI_32K_TCXO_CNT_REG (GLB_CTL_RB_BASE + 0x30C)    /* 32K校TCXO时钟校准寄存器 */
#define GLB_CTL_RB_CALI_32K_TCXO_RESULT_REG (GLB_CTL_RB_BASE + 0x310) /* 32K校TCXO时钟校准寄存器 */
#define GLB_CTL_RB_GP_REG0_REG (GLB_CTL_RB_BASE + 0x380)
#define GLB_CTL_RB_GP_REG1_REG (GLB_CTL_RB_BASE + 0x384)
#define GLB_CTL_RB_GP_REG2_REG (GLB_CTL_RB_BASE + 0x388)
#define GLB_CTL_RB_GP_REG3_REG (GLB_CTL_RB_BASE + 0x38C)
#define GLB_CTL_RB_GP_REG4_REG (GLB_CTL_RB_BASE + 0x390)
#define GLB_CTL_RB_GP_REG5_REG (GLB_CTL_RB_BASE + 0x394)
#define GLB_CTL_RB_GP_REG6_REG (GLB_CTL_RB_BASE + 0x398)
#define GLB_CTL_RB_GP_REG7_REG (GLB_CTL_RB_BASE + 0x39C)
#define GLB_CTL_RB_GP_REG8_REG (GLB_CTL_RB_BASE + 0x400)
#define GLB_CTL_RB_GP_REG9_REG (GLB_CTL_RB_BASE + 0x404)
#define GLB_CTL_RB_GP_REG10_REG (GLB_CTL_RB_BASE + 0x408)
#define GLB_CTL_RB_GP_REG11_REG (GLB_CTL_RB_BASE + 0x40C)
#define GLB_CTL_RB_GP_REG12_REG (GLB_CTL_RB_BASE + 0x410)
#define GLB_CTL_RB_GP_REG13_REG (GLB_CTL_RB_BASE + 0x414)
#define GLB_CTL_RB_GP_REG14_REG (GLB_CTL_RB_BASE + 0x418)
#define GLB_CTL_RB_GP_REG15_REG (GLB_CTL_RB_BASE + 0x41C)
#define GLB_CTL_RB_SDIO_INT_REQ_EN_REG (GLB_CTL_RB_BASE + 0x420)

#endif // __GLB_CTL_RB_REG_OFFSET_H__
