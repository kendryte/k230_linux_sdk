/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_reset.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_RESET_H__
#define __HAL_RESET_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hh503_phy_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_RESET_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* MAC每个bank所需保存的长度 */
#define PS_MAC_CTRL0_REG_LENGTH         HH503_MAC_CTRL0_OFFEST_BUTT
#define PS_MAC_CTRL1_REG_LENGTH         HH503_MAC_CTRL1_OFFEST_BUTT
#define PS_MAC_CTRL2_REG_LENGTH         HH503_MAC_CTRL2_OFFEST_BUTT

/* wlmac后面2个地址与前面的相隔太远 单独保存 */
#define PS_MAC_WLMAC_CTRL_REG_LENGTH    (HH503_MAC_WLMAC_CTRL_BASE_0x94 - HH503_MAC_WLMAC_CTRL_BASE + 4)

/* 0x400c0800 ~ 0x400c0b80(HH503_PHY_CFG_INTR_MASK) 长度0x384 */
#define PS_PHY_BANK1_REG_LENGTH         (HH503_PHY_BANK1_BASE_0x68 - HH503_PHY_BANK1_BASE + 0x4)
/* 0x400c0c00 ~ 0x400c0fb0(HH503_PHY_PHY_RX_11AX_CTRL0) 长度0x3b4 */
#define PS_PHY_BANK2_REG_LENGTH         (HH503_PHY_BANK1_BASE_0x30 - HH503_PHY_BANK2_BASE + 0x4)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_reset.h */
