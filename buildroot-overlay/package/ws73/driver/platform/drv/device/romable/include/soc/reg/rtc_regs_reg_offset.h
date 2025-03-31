// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  rtc_regs_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __RTC_REGS_REG_OFFSET_H__
#define __RTC_REGS_REG_OFFSET_H__

/* RTC_REGS Base address of Module's Register */
#define RTC_REGS_BASE (0x40001000)

/* **************************************************************************** */
/*                      HH503 RTC_REGS Registers' Definitions                            */
/* **************************************************************************** */

#define RTC_REGS_RTC_IRQ_TIME_LSBS_REG (RTC_REGS_BASE + 0x0)    /* RTC[31:0]配置寄存器。 */
#define RTC_REGS_RTC_IRQ_TIME_MSBS_REG (RTC_REGS_BASE + 0x4)    /* RTC[47:32]配置寄存器。 */
#define RTC_REGS_RTC_IRQ_CLR_REG (RTC_REGS_BASE + 0x8)          /* RTC 中断清零寄存器。 */
#define RTC_REGS_RTC_IRQ_MASK_OUT_REG (RTC_REGS_BASE + 0xC)     /* 中断屏蔽控制寄存器 */
#define RTC_REGS_RTC_IRQ_INT_RAW_IN_REG (RTC_REGS_BASE + 0x10)  /* 原始中断寄存器 */
#define RTC_REGS_RTC_IRQ_INT_MASK_IN_REG (RTC_REGS_BASE + 0x14) /* 屏蔽后中断寄存器 */
#define RTC_REGS_RTC_TIME_OUT_H_REG (RTC_REGS_BASE + 0x18)      /* 高32位计数值 */
#define RTC_REGS_RTC_TIME_OUT_L_REG (RTC_REGS_BASE + 0x1C)      /* 低32位计数值 */

#endif // __RTC_REGS_REG_OFFSET_H__
