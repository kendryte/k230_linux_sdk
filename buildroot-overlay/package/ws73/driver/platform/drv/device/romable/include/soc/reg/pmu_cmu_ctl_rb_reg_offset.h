// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  pmu_cmu_ctl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __PMU_CMU_CTL_RB_REG_OFFSET_H__
#define __PMU_CMU_CTL_RB_REG_OFFSET_H__

/* PMU_CMU_CTL_RB Base address of Module's Register */
#define PMU_CMU_CTL_RB_BASE (0x4001A000)

/* **************************************************************************** */
/*            HH503 PMU_CMU_CTL_RB Registers' Definitions                       */
/* **************************************************************************** */

#define PMU_CMU_CTL_RB_SYSLDO_GP_REG_REG         (PMU_CMU_CTL_RB_BASE + 0x80)
#define PMU_CMU_CTL_RB_PMU_RSV_REG               (PMU_CMU_CTL_RB_BASE + 0x100)
#define PMU_CMU_CTL_RB_PMU_EFUSE_BG_TRIM_REG     (PMU_CMU_CTL_RB_BASE + 0x104)
#define PMU_CMU_CTL_RB_PMU_EFUSE_RC_REG          (PMU_CMU_CTL_RB_BASE + 0x108)
#define PMU_CMU_CTL_RB_PMU_EFUSE_NEW_LDO_REG     (PMU_CMU_CTL_RB_BASE + 0x10C)
#define PMU_CMU_CTL_RB_PMU_CLDO_REG              (PMU_CMU_CTL_RB_BASE + 0x114)
#define PMU_CMU_CTL_RB_PMU_IOLDO_REG             (PMU_CMU_CTL_RB_BASE + 0x118)
#define PMU_CMU_CTL_RB_PMU_INTLDO_REG            (PMU_CMU_CTL_RB_BASE + 0x11C)
#define PMU_CMU_CTL_RB_PMU_XLDO_REG              (PMU_CMU_CTL_RB_BASE + 0x120)
#define PMU_CMU_CTL_RB_PMU_CMULDO_1P8_REG        (PMU_CMU_CTL_RB_BASE + 0x124)
#define PMU_CMU_CTL_RB_PMU_CMULDO0_0P9_REG       (PMU_CMU_CTL_RB_BASE + 0x128)
#define PMU_CMU_CTL_RB_PMU_CMULDO1_0P9_REG       (PMU_CMU_CTL_RB_BASE + 0x12C)
#define PMU_CMU_CTL_RB_PMU_TXLDO_1P0_REG         (PMU_CMU_CTL_RB_BASE + 0x130)
#define PMU_CMU_CTL_RB_PMU_RXLDO_1P0_REG         (PMU_CMU_CTL_RB_BASE + 0x134)
#define PMU_CMU_CTL_RB_PMU_LNALDO_1P0_REG        (PMU_CMU_CTL_RB_BASE + 0x138)
#define PMU_CMU_CTL_RB_PMU_VCOLDO_1P0_REG        (PMU_CMU_CTL_RB_BASE + 0x140)
#define PMU_CMU_CTL_RB_PMU_ABBLDO_1P0_REG        (PMU_CMU_CTL_RB_BASE + 0x144)
#define PMU_CMU_CTL_RB_PMU_PLLLDO_1P0_REG        (PMU_CMU_CTL_RB_BASE + 0x148)
#define PMU_CMU_CTL_RB_DBG_CTL_REG               (PMU_CMU_CTL_RB_BASE + 0x14C)
#define PMU_CMU_CTL_RB_PMU_TEST_VOLT_EN_REG      (PMU_CMU_CTL_RB_BASE + 0x150)
#define PMU_CMU_CTL_RB_PMU_SIG_REG               (PMU_CMU_CTL_RB_BASE + 0x154)
#define PMU_CMU_CTL_RB_PMU_DFT_SW_EN_REG         (PMU_CMU_CTL_RB_BASE + 0x15C)
#define PMU_CMU_CTL_RB_PMU_OTHER_TRIM1_REG       (PMU_CMU_CTL_RB_BASE + 0x160)
#define PMU_CMU_CTL_RB_PMU_OTHER_TRIM2_REG       (PMU_CMU_CTL_RB_BASE + 0x164)
#define PMU_CMU_CTL_RB_PMU_TXLDO_1P8_REG         (PMU_CMU_CTL_RB_BASE + 0x168)
#define PMU_CMU_CTL_RB_PMU_PHY_LDO_REG           (PMU_CMU_CTL_RB_BASE + 0x16C)
#define PMU_CMU_CTL_RB_EXCEP_CFG_RG_REG          (PMU_CMU_CTL_RB_BASE + 0x18C)
#define PMU_CMU_CTL_RB_EXCEP_CLR_RG_REG          (PMU_CMU_CTL_RB_BASE + 0x190)
#define PMU_CMU_CTL_RB_EXCEP_OCP_RO_RG_STICK_REG (PMU_CMU_CTL_RB_BASE + 0x194) /* OCP异常观测 */
#define PMU_CMU_CTL_RB_EXCEP_OCP_RO_RG_REG       (PMU_CMU_CTL_RB_BASE + 0x198) /* OCP异常观测 */
#define PMU_CMU_CTL_RB_EXCEP_RO_RG_REG           (PMU_CMU_CTL_RB_BASE + 0x19C) /* OVP和UVLO和pll_unlock异常观测 */
#define PMU_CMU_CTL_RB_CUR_STS_REG               (PMU_CMU_CTL_RB_BASE + 0x200)
#define PMU_CMU_CTL_RB_OVP_PD_COUNT_REG          (PMU_CMU_CTL_RB_BASE + 0x204)
#define PMU_CMU_CTL_RB_OCP_PD_COUNT_REG          (PMU_CMU_CTL_RB_BASE + 0x208)
#define PMU_CMU_CTL_RB_ISO_SIG_REG               (PMU_CMU_CTL_RB_BASE + 0x20C)
#define PMU_CMU_CTL_RB_RF_CONTROL_REG            (PMU_CMU_CTL_RB_BASE + 0x220)
#define PMU_CMU_CTL_RB_POC_COEX_REG              (PMU_CMU_CTL_RB_BASE + 0x224)
#define PMU_CMU_CTL_RB_PMU_TIME1_REG             (PMU_CMU_CTL_RB_BASE + 0x240)
#define PMU_CMU_CTL_RB_PMU_TIME2_REG             (PMU_CMU_CTL_RB_BASE + 0x244)
#define PMU_CMU_CTL_RB_PMU_TIME3_REG             (PMU_CMU_CTL_RB_BASE + 0x248)
#define PMU_CMU_CTL_RB_PMU_TIME4_REG             (PMU_CMU_CTL_RB_BASE + 0x24C)
#define PMU_CMU_CTL_RB_PMU_TIME5_REG             (PMU_CMU_CTL_RB_BASE + 0x250)
#define PMU_CMU_CTL_RB_PMU_TIME6_REG             (PMU_CMU_CTL_RB_BASE + 0x254)

#endif // __PMU_CMU_CTL_RB_REG_OFFSET_H__
