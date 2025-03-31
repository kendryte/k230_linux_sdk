/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf寄存器定义
*/


#ifndef _RF_REG_SOC_OFFSET_H_
#define _RF_REG_SOC_OFFSET_H_

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HH503_RF_WB_CTL_RB_BASE_ADDR           0x40014000

/* RF_WB_CTL_RB */
#define HH503_CFG_RF_MAN_EN_CTRL         (HH503_RF_WB_CTL_RB_BASE_ADDR + 0x40)  /* rf端口控制软控使能寄存器 */
#define HH503_CFG_PA_IDX_CTRL            (HH503_RF_WB_CTL_RB_BASE_ADDR + 0x4c)
#define HH503_CFG_PA_PPA_CODE            (HH503_RF_WB_CTL_RB_BASE_ADDR + 0x60)  /* 动态功率校准配置寄存器 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
