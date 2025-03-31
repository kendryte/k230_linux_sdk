// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  coex_ctl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __COEX_CTL_RB_REG_OFFSET_H__
#define __COEX_CTL_RB_REG_OFFSET_H__

/* COEX_CTL_RB Base address of Module's Register */

#ifdef BUILD_UT
#include "wifi_ut_stub.h"
#endif
#ifndef BUILD_UT
/* COEX_CTL_RB Base address of Module's Register */
#define HH503_COEX_CTL_RB_BASE                 (0x40017000)
#else
#define HH503_COEX_CTL_RB_BASE                 (osal_u32)g_coex_ctl_rb_addr
#endif

/* **************************************************************************** */
/*                COEX_CTL_RB Registers' Definitions                            */
/* **************************************************************************** */
#define HH503_COEX_CTL_RB_COEX_GP_REG0_REG                 (HH503_COEX_CTL_RB_BASE + 0x10) /* 共存通用寄存器0 */
#define HH503_COEX_CTL_RB_COEX_GP_REG1_REG                 (HH503_COEX_CTL_RB_BASE + 0x14) /* 共存通用寄存器1 */
#define HH503_COEX_CTL_RB_COEX_SOFT_DIAG_REG               (HH503_COEX_CTL_RB_BASE + 0x20) /* 共存中维测信号复用模式选择寄存器 */
#define HH503_COEX_CTL_RB_CFG_COEX_DIAG_SEL0_REG           (HH503_COEX_CTL_RB_BASE + 0x30) /* 维测管脚观测配置寄存器0 */
#define HH503_COEX_CTL_RB_CFG_COEX_DIAG_SEL1_REG           (HH503_COEX_CTL_RB_BASE + 0x34) /* 维测管脚观测配置寄存器1 */
#define HH503_COEX_CTL_RB_CFG_COEX_DIAG_SEL2_REG           (HH503_COEX_CTL_RB_BASE + 0x38) /* 维测管脚观测配置寄存器2 */
#define HH503_COEX_CTL_RB_INT_EN_WL_REG                    (HH503_COEX_CTL_RB_BASE + 0x50) /* WLAN中断使能寄存器 */
#define HH503_COEX_CTL_RB_INT_EN_BT_REG                    (HH503_COEX_CTL_RB_BASE + 0x54) /* BGLE中断使能寄存器 */
#define HH503_COEX_CTL_RB_INT_SET_WL_REG                   (HH503_COEX_CTL_RB_BASE + 0x58) /* WLAN软中断寄存器 */
#define HH503_COEX_CTL_RB_INT_SET_BT_REG                   (HH503_COEX_CTL_RB_BASE + 0x5C) /* BGLE软中断寄存器 */
#define HH503_COEX_CTL_RB_INT_STS_WL_REG                   (HH503_COEX_CTL_RB_BASE + 0x60) /* WLAN中断状态寄存器 */
#define HH503_COEX_CTL_RB_INT_STS_BT_REG                   (HH503_COEX_CTL_RB_BASE + 0x64) /* BGLE中断状态寄存器 */
#define HH503_COEX_CTL_RB_INT_CLR_WL_REG                   (HH503_COEX_CTL_RB_BASE + 0x68) /* WLAN中断状态清零寄存器 */
#define HH503_COEX_CTL_RB_INT_CLR_BT_REG                   (HH503_COEX_CTL_RB_BASE + 0x6C) /* BGLE中断状态清零寄存器 */
#define HH503_COEX_CTL_RB_INT_MASK_WL_REG                  (HH503_COEX_CTL_RB_BASE + 0x70) /* WLAN中断可屏蔽寄存器 */
#define HH503_COEX_CTL_RB_INT_MASK_BT_REG                  (HH503_COEX_CTL_RB_BASE + 0x74) /* BGLE中断可屏蔽寄存器 */
#define HH503_COEX_CTL_RB_WIFI_RESUME_INT_REG              (HH503_COEX_CTL_RB_BASE + 0x78) /* WLAN射频恢复中断配置寄存器 */
#define HH503_COEX_CTL_RB_SOFT_RST_REG                     (HH503_COEX_CTL_RB_BASE + 0x80) /* 共存状态机软复位寄存器 */
#define HH503_COEX_CTL_RB_CFG_FORCE_PRIORITY_REG           (HH503_COEX_CTL_RB_BASE + 0x88) /* 强制设定优先级寄存器 */
#define HH503_COEX_CTL_RB_CFG_WLAN_ABORT_EN_REG            (HH503_COEX_CTL_RB_BASE + 0x8C) /* WLAN ABORT配置寄存器 */
#define HH503_COEX_CTL_RB_CFG_SWITCH_RF_CNT_REG            (HH503_COEX_CTL_RB_BASE + 0x90) /* RF切换开关延时配置寄存器 */
#define HH503_COEX_CTL_RB_CFG_RF_W2B_PROT_CNT_REG          (HH503_COEX_CTL_RB_BASE + 0x94) /* RF W2B切换保护时间寄存器 */
#define HH503_COEX_CTL_RB_CFG_RF_B2W_PROT_CNT_REG          (HH503_COEX_CTL_RB_BASE + 0x98) /* RF B2W切换保护时间寄存器 */
#define HH503_COEX_CTL_RB_CFG_WL_TX_ABORT_CNT_REG          (HH503_COEX_CTL_RB_BASE + 0x9C) /* WLAN TX_ABORT信号计数寄存器 */
#define HH503_COEX_CTL_RB_CFG_WL_RX_ABORT_CNT_REG          (HH503_COEX_CTL_RB_BASE + 0xA0) /* WLAN RX_ABORT信号计数寄存器 */
#define HH503_COEX_CTL_RB_COEX_PTA0_CURR_STATE_REG         (HH503_COEX_CTL_RB_BASE + 0xA4) /* PTA0状态查询寄存器 */
#define HH503_COEX_CTL_RB_RPT_COEX_RESULT_REG              (HH503_COEX_CTL_RB_BASE + 0xA8) /* COEX仲裁结果查询寄存器 */
#define HH503_COEX_CTL_RB_CFG_COEX_PTA_WL_RX_ABORT_SEL_REG (HH503_COEX_CTL_RB_BASE + 0xB0) /* COEX输出仲裁信号软件旁路功能 */
#define HH503_COEX_CTL_RB_CFG_COEX_PTA_WL_RX_ABORT_MAN_REG (HH503_COEX_CTL_RB_BASE + 0xB4) /* COEX输出仲裁信号软件旁路配置值 */
#define HH503_COEX_CTL_RB_CFG_WL_BT_BYP_REG                (HH503_COEX_CTL_RB_BASE + 0xB8) /* 申请信号BYPASS寄存器 */
#define HH503_COEX_CTL_RB_CFG_ANT_SW_SEL_REG               (HH503_COEX_CTL_RB_BASE + 0xC0) /* 天线分集控制软件旁路功能 */
#define HH503_COEX_CTL_RB_CFG_ANT_SW_MAN_REG               (HH503_COEX_CTL_RB_BASE + 0xC4) /* 天线分集控制软件旁路配置值 */
#define HH503_COEX_CTL_RB_CFG_WIFI_BGLE_SEL_REG            (HH503_COEX_CTL_RB_BASE + 0xC8) /* 天线分集逻辑配置功能寄存器 */
#define HH503_COEX_CTL_RB_CFG_BGLE_ANTCTL_POLAR_REG        (HH503_COEX_CTL_RB_BASE + 0xCC) /* 天线分集逻辑配置功能寄存器 */
#define HH503_COEX_CTL_RB_RPT_ANT_SW_RESULT_REG            (HH503_COEX_CTL_RB_BASE + 0xD0) /* ANT_SW查询寄存器 */

#endif // __COEX_CTL_RB_REG_OFFSET_H__
