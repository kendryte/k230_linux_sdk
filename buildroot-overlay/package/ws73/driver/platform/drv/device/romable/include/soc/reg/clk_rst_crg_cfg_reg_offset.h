// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  clk_rst_crg_cfg_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __CLK_RST_CRG_CFG_REG_OFFSET_H__
#define __CLK_RST_CRG_CFG_REG_OFFSET_H__

/* CLK_RST_CRG_CFG Base address of Module's Register */
#define CLK_RST_CRG_CFG_BASE (0x4001f000)

/* **************************************************************************** */
/*                      HH503 CLK_RST_CRG_CFG Registers' Definitions                            */
/* **************************************************************************** */

#define CLK_RST_CRG_CFG_GP_REG0_REG (CLK_RST_CRG_CFG_BASE + 0x10)              /* 通用寄存器 */
#define CLK_RST_CRG_CFG_GP_REG1_REG (CLK_RST_CRG_CFG_BASE + 0x14)              /* 通用寄存器 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_CKEN_CTL_REG (CLK_RST_CRG_CFG_BASE + 0x100) /* 时钟门控控制寄存器 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG \
    (CLK_RST_CRG_CFG_BASE + 0x104) /* 时钟选择信号控制寄存器 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_OPT_DIV_CTL0_REG \
    (CLK_RST_CRG_CFG_BASE + 0x108) /* 时钟扣脉冲分频控制寄存器 */
#define CLK_RST_CRG_CFG_CLK_SEL_STS_REG (CLK_RST_CRG_CFG_BASE + 0x114)         /* 时钟选择状态 */
#define CLK_RST_CRG_CFG_GLB_RST_CRG_CTL0_REG (CLK_RST_CRG_CFG_BASE + 0x118)    /* 全局复位控制寄存器 */
#define CLK_RST_CRG_CFG_GLB_RST_CRG_CTL1_REG (CLK_RST_CRG_CFG_BASE + 0x11C)    /* 全局复位控制寄存器 */
#define CLK_RST_CRG_CFG_SYSLDO_SOFT_RST_CTL_REG (CLK_RST_CRG_CFG_BASE + 0x120) /* 软复位控制寄存器 */
#define CLK_RST_CRG_CFG_CPU_POR_PLUSE_RSTSYNC_REG \
    (CLK_RST_CRG_CFG_BASE + 0x124)                                             /* CPU复位脉冲展宽寄存器 */
#define CLK_RST_CRG_CFG_PHY_WLMAC_CRG_REG (CLK_RST_CRG_CFG_BASE + 0x128) /* PHY_WLMAC CRG信号需求 */
#define CLK_RST_CRG_CFG_PMU_CMU_REG (CLK_RST_CRG_CFG_BASE + 0x12C)       /* 需求新增 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_CKEN_CTL1_REG (CLK_RST_CRG_CFG_BASE + 0x130) /* 时钟门控控制寄存器 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL0_REG  (CLK_RST_CRG_CFG_BASE + 0x140) /* 时钟分频控制寄存器0 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL1_REG  (CLK_RST_CRG_CFG_BASE + 0x144) /* 时钟分频控制寄存器1 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL2_REG  (CLK_RST_CRG_CFG_BASE + 0x148) /* 时钟分频控制寄存器2 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL3_REG  (CLK_RST_CRG_CFG_BASE + 0x14C) /* 时钟分频控制寄存器3 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL4_REG  (CLK_RST_CRG_CFG_BASE + 0x150) /* 时钟分频控制寄存器4 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL5_REG  (CLK_RST_CRG_CFG_BASE + 0x154) /* 时钟分频控制寄存器5 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL6_REG  (CLK_RST_CRG_CFG_BASE + 0x158) /* 时钟分频控制寄存器6 */
#define CLK_RST_CRG_CFG_SYSLDO_CRG_DIV_CTL7_REG  (CLK_RST_CRG_CFG_BASE + 0x15C) /* 时钟分频控制寄存器7 */
#define CLK_RST_CRG_CFG_UART_P2P_BRIDGE_BYPASS_REG  (CLK_RST_CRG_CFG_BASE + 0x160)

#endif // __CLK_RST_CRG_CFG_REG_OFFSET_H__
