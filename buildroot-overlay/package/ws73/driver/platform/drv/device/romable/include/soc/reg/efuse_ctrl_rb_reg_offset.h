// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  efuse_ctrl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __EFUSE_CTRL_RB_REG_OFFSET_H__
#define __EFUSE_CTRL_RB_REG_OFFSET_H__

/* EFUSE_CTRL_RB Base address of Module's Register */
#define EFUSE_CTRL_RB_BASE (0x40018000)

/* **************************************************************************** */
/*          HH503 EFUSE_CTRL_RB Registers' Definitions                          */
/* **************************************************************************** */

#define EFUSE_CTRL_RB_EFUSE_BYTE_WRITE_REG   (EFUSE_CTRL_RB_BASE + 0x4) /* EFUSE byte写使能与数据 */
#define EFUSE_CTRL_RB_EFUSE_AVDD_CTL_SEL_REG (EFUSE_CTRL_RB_BASE + 0x8) /* EFUSE AVDD电源开启软控寄存器 */
#define EFUSE_CTRL_RB_EFUSE_AVDD_MAN_REG     (EFUSE_CTRL_RB_BASE + 0xC) /* EFUSE AVDD电源开启软控寄存器 */
#define EFUSE_CTRL_RB_EFUSE_MODE_EN_REG    (EFUSE_CTRL_RB_BASE + 0x10) /* 编程使能寄存器。 */
#define EFUSE_CTRL_RB_EFUSE_WR_ADDR_REG    (EFUSE_CTRL_RB_BASE + 0x14) /* 编程地址寄存器。 */
#define EFUSE_CTRL_RB_EFUSE_CTRL_ST_REG    (EFUSE_CTRL_RB_BASE + 0x18) /* 状态指示寄存器。 */
#define EFUSE_CTRL_RB_EFUSE_RDATA_REG      (EFUSE_CTRL_RB_BASE + 0x1C) /* 读数据寄存器。 */
#define EFUSE_CTRL_RB_EFUSE_TIME0_REG      (EFUSE_CTRL_RB_BASE + 0x20) /* EFUSE时间参数寄存器0 */
#define EFUSE_CTRL_RB_EFUSE_TIME1_REG      (EFUSE_CTRL_RB_BASE + 0x24) /* EFUSE时间参数寄存器1 */
#define EFUSE_CTRL_RB_EFUSE_TIME2_REG      (EFUSE_CTRL_RB_BASE + 0x28) /* EFUSE时间参数寄存器2 */
#define EFUSE_CTRL_RB_EFUSE_TIME3_REG      (EFUSE_CTRL_RB_BASE + 0x2C) /* EFUSE时间参数寄存器3。 */
#define EFUSE_CTRL_RB_EFUSE_LOCK_STAT0_REG (EFUSE_CTRL_RB_BASE + 0x30) /* EFUSE锁定状态上报寄存器0 */
#define EFUSE_CTRL_RB_EFUSE_LOCK_STAT1_REG (EFUSE_CTRL_RB_BASE + 0x34) /* EFUSE锁定状态上报寄存器1 */
#define EFUSE_CTRL_RB_CMU_REG              (EFUSE_CTRL_RB_BASE + 0x40) /* EFUSE中CMU_TRIM */
#define EFUSE_CTRL_RB_PMU_TRIM0_REG        (EFUSE_CTRL_RB_BASE + 0x44) /* EFUSE中PMU_TRIM0 */
#define EFUSE_CTRL_RB_PMU_TRIM1_REG        (EFUSE_CTRL_RB_BASE + 0x48) /* EFUSE中PMU_TRIM1 */
#define EFUSE_CTRL_RB_PMU_TRIM2_REG        (EFUSE_CTRL_RB_BASE + 0x4C) /* EFUSE中PMU_TRIM2 */
#define EFUSE_CTRL_RB_WIFI6_SW_REG         (EFUSE_CTRL_RB_BASE + 0x50) /* EFUSE中WIFI6开关 */
#define EFUSE_CTRL_RB_GLE_SW_REG           (EFUSE_CTRL_RB_BASE + 0x54) /* EFUSE中GLE开关 */
#define EFUSE_CTRL_RB_POC_FUSE_REG         (EFUSE_CTRL_RB_BASE + 0x58) /* EFUSE中POC指示信号 */
#define EFUSE_CTRL_RB_TYPE_ID_REG          (EFUSE_CTRL_RB_BASE + 0x5C) /* EFUSE中TYPE_ID */
#define EFUSE_CTRL_RB_DBG_BYPASS_REG       (EFUSE_CTRL_RB_BASE + 0x60) /* EFUSE新增功能bypass信号 */

#endif // __EFUSE_CTRL_RB_REG_OFFSET_H__
