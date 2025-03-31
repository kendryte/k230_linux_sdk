/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  timer_reg_offset.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:05 Create file
 */

#ifndef __TIMER_REG_OFFSET_H__
#define __TIMER_REG_OFFSET_H__

/* Timer Base address of Module's Register */
#define TIMER_BASE                       (0x40004000)

/******************************************************************************/
/*                       Timer Registers' Definitions                         */
/******************************************************************************/

#define TIMER_TIMERX_LOAD_0_REG    (TIMER_BASE + 0x0)
#define TIMER_TIMERX_LOAD_1_REG    (TIMER_BASE + 0x100)
#define TIMER_TIMERX_LOAD_2_REG    (TIMER_BASE + 0x200)
#define TIMER_TIMERX_LOAD_3_REG    (TIMER_BASE + 0x300)
#define TIMER_TIMERX_VALUE_0_REG   (TIMER_BASE + 0x4)   /* 当前计数值寄存器。用于给出正在递减的计数器的当前值。 */
#define TIMER_TIMERX_VALUE_1_REG   (TIMER_BASE + 0x104) /* 当前计数值寄存器。用于给出正在递减的计数器的当前值。 */
#define TIMER_TIMERX_VALUE_2_REG   (TIMER_BASE + 0x204) /* 当前计数值寄存器。用于给出正在递减的计数器的当前值。 */
#define TIMER_TIMERX_VALUE_3_REG   (TIMER_BASE + 0x304) /* 当前计数值寄存器。用于给出正在递减的计数器的当前值。 */
#define TIMER_TIMERX_CONTROL_0_REG (TIMER_BASE + 0x8)
#define TIMER_TIMERX_CONTROL_1_REG (TIMER_BASE + 0x108)
#define TIMER_TIMERX_CONTROL_2_REG (TIMER_BASE + 0x208)
#define TIMER_TIMERX_CONTROL_3_REG (TIMER_BASE + 0x308)
#define TIMER_TIMERX_INTCLR_0_REG  (TIMER_BASE + 0xC)
#define TIMER_TIMERX_INTCLR_1_REG  (TIMER_BASE + 0x10C)
#define TIMER_TIMERX_INTCLR_2_REG  (TIMER_BASE + 0x20C)
#define TIMER_TIMERX_INTCLR_3_REG  (TIMER_BASE + 0x30C)
#define TIMER_TIMERX_RIS_0_REG     (TIMER_BASE + 0x10)  /* 原始中断寄存器。 */
#define TIMER_TIMERX_RIS_1_REG     (TIMER_BASE + 0x110) /* 原始中断寄存器。 */
#define TIMER_TIMERX_RIS_2_REG     (TIMER_BASE + 0x210) /* 原始中断寄存器。 */
#define TIMER_TIMERX_RIS_3_REG     (TIMER_BASE + 0x310) /* 原始中断寄存器。 */
#define TIMER_TIMERX_MIS_0_REG     (TIMER_BASE + 0x14)  /* 屏蔽后中断寄存器。 */
#define TIMER_TIMERX_MIS_1_REG     (TIMER_BASE + 0x114) /* 屏蔽后中断寄存器。 */
#define TIMER_TIMERX_MIS_2_REG     (TIMER_BASE + 0x214) /* 屏蔽后中断寄存器。 */
#define TIMER_TIMERX_MIS_3_REG     (TIMER_BASE + 0x314) /* 屏蔽后中断寄存器。 */
#define TIMER_TIMERX_BGLOAD_0_REG  (TIMER_BASE + 0x18)
#define TIMER_TIMERX_BGLOAD_1_REG  (TIMER_BASE + 0x118)
#define TIMER_TIMERX_BGLOAD_2_REG  (TIMER_BASE + 0x218)
#define TIMER_TIMERX_BGLOAD_3_REG  (TIMER_BASE + 0x318)

#endif // __TIMER_REG_OFFSET_H__
