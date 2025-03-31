// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  cldo_ctl_reg_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __CLDO_CTL_REG_REG_OFFSET_H__
#define __CLDO_CTL_REG_REG_OFFSET_H__
#ifdef BUILD_UT
#include "wifi_ut_stub.h"
#endif

#ifndef BUILD_UT
/* CLDO_CTL_REG Base address of Module's Register */
#define CLDO_CTL_REG_BASE (0x40017400)
#else
#define CLDO_CTL_REG_BASE (osal_u32)g_cldo_ctl_addr
#endif
/* **************************************************************************** */
/*                      HH503 CLDO_CTL_REG Registers' Definitions                            */
/* **************************************************************************** */

#define CLDO_CTL_REG_SYS_INT_REG (CLDO_CTL_REG_BASE + 0x0)
#define CLDO_CTL_REG_SOFT_INT_EN_REG (CLDO_CTL_REG_BASE + 0x4)   /* 中断控制寄存器 */
#define CLDO_CTL_REG_SOFT_INT_SET_REG (CLDO_CTL_REG_BASE + 0x8)  /* 中断控制寄存器 */
#define CLDO_CTL_REG_SOFT_INT_CLR_REG (CLDO_CTL_REG_BASE + 0xC)  /* 中断控制寄存器 */
#define CLDO_CTL_REG_SOFT_INT_STS_REG (CLDO_CTL_REG_BASE + 0x10) /* 中断控制寄存器 */
#define CLDO_CTL_REG_MAC_PKT_BIT_MODE_REG (CLDO_CTL_REG_BASE + 0x18)
#define CLDO_CTL_REG_PKT_RAM_CFG_REG (CLDO_CTL_REG_BASE + 0x1C)      /* PKT ram 读配置 */
#define CLDO_CTL_REG_ICM_PRIORITY_REG (CLDO_CTL_REG_BASE + 0x20)     /* 总线优先级配置 */
#define CLDO_CTL_REG_ICM_PRIORITY2_REG    (CLDO_CTL_REG_BASE + 0x24)
#define CLDO_CTL_REG_SOFT_RST_MAC_BUS_REG (CLDO_CTL_REG_BASE + 0x28)
#define CLDO_CTL_REG_BT_BRG_CFG_REG (CLDO_CTL_REG_BASE + 0x2C)
#define CLDO_CTL_REG_INT_EN_CTL_REG (CLDO_CTL_REG_BASE + 0x34)   /* 中断使能控制寄存器 */
#define CLDO_CTL_REG_INT_MASK_CTL_REG (CLDO_CTL_REG_BASE + 0x38) /* 中断屏蔽控制寄存器 */
#define CLDO_CTL_REG_INT_CLR_CTL_REG (CLDO_CTL_REG_BASE + 0x3C)  /* 中断清除寄存器 */
#define CLDO_CTL_REG_INT_STS_REG (CLDO_CTL_REG_BASE + 0x40)      /* 中断状态查询寄存器 */
#define CLDO_CTL_REG_SDIO_ID_REG (CLDO_CTL_REG_BASE + 0x44)      /* SDIO ID 配置 */
#define CLDO_CTL_REG_ICM_CG_PKT_RAM_FIFO_CG_CFG_REG       (CLDO_CTL_REG_BASE + 0x100)
#define CLDO_CTL_REG_CORE_STAT_MODE_REG (CLDO_CTL_REG_BASE + 0x104)    /* cpu ｍｏｄｅ 指示 */
#define CLDO_CTL_REG_SDIO_DEBUG_SIGNAL_REG (CLDO_CTL_REG_BASE + 0x108) /* SDIO的debug观测信号 */
#define CLDO_CTL_REG_TCM_ROM_CFG_REG (CLDO_CTL_REG_BASE + 0x200)       /* PTCM_ROM控制寄存器 */
#define CLDO_CTL_REG_MONITOR_MAC_ADDR0_H_REG (CLDO_CTL_REG_BASE + 0x300)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR0_L_REG (CLDO_CTL_REG_BASE + 0x304)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR1_H_REG (CLDO_CTL_REG_BASE + 0x308)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR1_L_REG (CLDO_CTL_REG_BASE + 0x30C)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR2_H_REG (CLDO_CTL_REG_BASE + 0x310)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR2_L_REG (CLDO_CTL_REG_BASE + 0x314)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR3_H_REG (CLDO_CTL_REG_BASE + 0x318)
#define CLDO_CTL_REG_MONITOR_MAC_ADDR3_L_REG (CLDO_CTL_REG_BASE + 0x31C)
#define CLDO_CTL_REG_MAC_MONITOR_REG (CLDO_CTL_REG_BASE + 0x320)
#define CLDO_CTL_REG_MONITOR_REC_ADDR_REG (CLDO_CTL_REG_BASE + 0x324)
#define CLDO_CTL_REG_MAC_REC_WR_REG (CLDO_CTL_REG_BASE + 0x328)
#define CLDO_CTL_REG_MAC_MONITOR_INIT_STST_REG (CLDO_CTL_REG_BASE + 0x32C)
#define CLDO_CTL_REG_FPGA_VERSION_REG (CLDO_CTL_REG_BASE + 0x340)
#define CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG (CLDO_CTL_REG_BASE + 0x350)    /* cpu mem 共享ram cken配置 */
#define CLDO_CTL_REG_BANK_RAM0_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x354) /* bank ram0 START_ADDR配置 */
#define CLDO_CTL_REG_BANK_RAM1_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x358) /* bank ram1 START_ADDR配置 */
#define CLDO_CTL_REG_BANK_RAM2_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x35C) /* bank ram2 START_ADDR配置 */
#define CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x360) /* bank ram3 START_ADDR配置 */
#define CLDO_CTL_REG_BANK_RAM4_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x364) /* bank ram4 START_ADDR配置 */
#define CLDO_CTL_REG_BANK_RAM5_START_ADDR_REG \
    (CLDO_CTL_REG_BASE + 0x368) /* bank ram5 START_ADDR配置(此mem固定给PKT_RAM) */
#define CLDO_CTL_REG_SHARE_RAM_SW_CONFLICT_CLR_REG (CLDO_CTL_REG_BASE + 0x36C)
#define CLDO_CTL_REG_SHARE_RAM_SW_CONFLICT_ADDR_REG (CLDO_CTL_REG_BASE + 0x370)
#define CLDO_CTL_REG_SHARE_RAM_SW_CONFLICT_START_ADDR_REG (CLDO_CTL_REG_BASE + 0x374)
#define CLDO_CTL_REG_SHARE_RAM_SW_CONFLICT_INFO_REG (CLDO_CTL_REG_BASE + 0x378)
#define CLDO_CTL_REG_TP_RAM_TMOD_0P9_REG (CLDO_CTL_REG_BASE + 0x37C) /* 0.9V下双端口ram调速值 */
#define CLDO_CTL_REG_SP_RAM_TMOD_HIGH_0P9_REG                 \
    (CLDO_CTL_REG_BASE + 0x380) /* 0.9V下单端口RAM调速值,高位 \
                                        */
#define CLDO_CTL_REG_SP_RAM_TMOD_LOW_0P9_REG                                                                 \
    (CLDO_CTL_REG_BASE + 0x384)                                               /* 0.9V下单端口RAM调速值，低位 \
                                                                                      */
#define CLDO_CTL_REG_BANK_RAM0_USE_CFG_REG (CLDO_CTL_REG_BASE + 0x388) /* bank ram0 USE配置 */
#define CLDO_CTL_REG_BANK_RAM1_USE_CFG_REG (CLDO_CTL_REG_BASE + 0x38C) /* bank ram1 USE配置 */
#define CLDO_CTL_REG_BANK_RAM2_USE_CFG_REG (CLDO_CTL_REG_BASE + 0x390) /* bank ram2 USE配置 */
#define CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG (CLDO_CTL_REG_BASE + 0x394) /* bank ram3 USE配置 */
#define CLDO_CTL_REG_BANK_RAM4_USE_CFG_REG (CLDO_CTL_REG_BASE + 0x398) /* bank ram4 USE配置 */
#define CLDO_CTL_REG_BANK_RAM5_USE_CFG_REG \
    (CLDO_CTL_REG_BASE + 0x39C) /* bank ram5 USE配置(此mem固定给PKT_RAM) */

#endif // __CLDO_CTL_REG_REG_OFFSET_H__
