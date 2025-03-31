// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  usb2_crg_cfg_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __USB2_CRG_CFG_RB_REG_OFFSET_H__
#define __USB2_CRG_CFG_RB_REG_OFFSET_H__

/* USB2_CRG_CFG_RB Base address of Module's Register */
#define USB2_CRG_CFG_RB_BASE (0x4000a000)

/* **************************************************************************** */
/*                      HH503 USB2_CRG_CFG_RB Registers' Definitions                            */
/* **************************************************************************** */

#define USB2_CRG_CFG_RB_USB2_CTRL_CRG_CFG_REG \
    (USB2_CRG_CFG_RB_BASE + 0x800) /* USB2 CTRL时钟及软复位控制寄存器。 */
#define USB2_CRG_CFG_RB_USB2_PHY_CRG_CFG0_REG \
    (USB2_CRG_CFG_RB_BASE + 0x804) /* USB2 PHY 时钟及软复位控制寄存器。 */
#define USB2_CRG_CFG_RB_USB2_CRG_CFG0_REG \
    (USB2_CRG_CFG_RB_BASE + 0x808) /* USB2 PHY 时钟及软复位控制寄存器。 */
#define USB2_CRG_CFG_RB_USB2_CRG_DIV_CFG_REG \
    (USB2_CRG_CFG_RB_BASE + 0x80C) /* USB2 PHY PCLK分频配置 */

#endif // __USB2_CRG_CFG_RB_REG_OFFSET_H__
