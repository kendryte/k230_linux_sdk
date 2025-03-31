/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  tsensor_regs_reg_offset.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:05 Create file
 */

#ifndef __TSENSOR_REGS_REG_OFFSET_H__
#define __TSENSOR_REGS_REG_OFFSET_H__

/* TSENSOR_REGS Base address of Module's Register */
#define TSENSOR_REGS_BASE                       (0x40018800)

/******************************************************************************/
/*                       TSENSOR_REGS Registers' Definitions                            */
/******************************************************************************/

#define TSENSOR_REGS_TS_START_REG               (TSENSOR_REGS_BASE + 0x0)  /* tsensor开始采样配置寄存器。 */
#define TSENSOR_REGS_TS_CFG_REG                 (TSENSOR_REGS_BASE + 0x4)  /* tsensor配置寄存器。 */
#define TSENSOR_REGS_TS_MAN_CLR_REG             (TSENSOR_REGS_BASE + 0x8)  /* tsensor man清除寄存器。 */
#define TSENSOR_REGS_TS_AUTO_CLR_REG            (TSENSOR_REGS_BASE + 0xC)  /* tsensor auto清除寄存器。 */
#define TSENSOR_REGS_TSENSOR_C0_CTRL1_REG       (TSENSOR_REGS_BASE + 0x10) /* TSENSOR_C0控制寄存器1 */
#define TSENSOR_REGS_TS_TEMP_HIGH_LIMIT_REG     (TSENSOR_REGS_BASE + 0x14) /* tsensor高温温度门限配置寄存器。 */
#define TSENSOR_REGS_TS_TEMP_LOW_LIMIT_REG      (TSENSOR_REGS_BASE + 0x18) /* tsensor低温温度门限配置寄存器。 */
#define TSENSOR_REGS_TS_OVERTEMP_THRESH_REG     (TSENSOR_REGS_BASE + 0x1C) /* tsensor超高温度阈值配置寄存器。 */
#define TSENSOR_REGS_TS_INT_EN_REG              (TSENSOR_REGS_BASE + 0x20) /* tsensor中断使能寄存器。 */
#define TSENSOR_REGS_TS_INT_CLR_REG             (TSENSOR_REGS_BASE + 0x24) /* tsensor中断清除寄存器。 */
#define TSENSOR_REGS_TS_INT_STS_REG             (TSENSOR_REGS_BASE + 0x28) /* tsensor中断上报寄存器。 */
#define TSENSOR_REGS_TS_AUTO_REFRESH_PERIOD_REG (TSENSOR_REGS_BASE + 0x2C) /* tsensor周期采样间隔配置寄存器。 */
#define TSENSOR_REGS_TS_AUTO_REFRESH_EN_REG     (TSENSOR_REGS_BASE + 0x30) /* 周期采样使能寄存器。 */
#define TSENSOR_REGS_TS_OVERTEMP_PRT_CFG_REG    (TSENSOR_REGS_BASE + 0x34) /* tsensor过温保护配置寄存器。 */
#define TSENSOR_REGS_TS_TEMP_ML0_REG            (TSENSOR_REGS_BASE + 0x38) /* tsensor温度配置寄存器。 */
#define TSENSOR_REGS_TS_TEMP_ML1_REG            (TSENSOR_REGS_BASE + 0x3C) /* tsensor温度配置寄存器。 */
#define TSENSOR_REGS_TS_TEMP_ML2_REG            (TSENSOR_REGS_BASE + 0x40) /* tsensor温度配置寄存器。 */
#define TSENSOR_REGS_TS_TEMP_ML3_REG            (TSENSOR_REGS_BASE + 0x44) /* tsensor温度配置寄存器。 */
#define TSENSOR_REGS_TS_EFUSE_CALI_REG          (TSENSOR_REGS_BASE + 0x48) /* tsensor温度校准差值寄存器 */

#endif // __TSENSOR_REGS_REG_OFFSET_H__
