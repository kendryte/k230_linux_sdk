/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_psd.c
 * Create: 2023-01-16
 */
#include "hal_common_ops.h"
#include "hal_phy.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_PSD_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_PSD
osal_void hal_psd_disable(osal_void)
{
    hal_reg_write_bits(HH503_PHY0_CTL_BASE_0xD8, 0, 1, 0x0);
    hal_reg_write_bits(HH503_PHY_BANK4_BASE_0x26C, 1, 1, 0x0);
    /* FTM时钟还原 */
    hal_reg_write_bits(HH503_PHY0_CTL_BASE_0x20, 0x9, 1, 0);
    return;
}

osal_void hal_psd_enable(osal_void)
{
    hal_reg_write_bits(HH503_PHY_BANK4_BASE_0x270, 0, 8, 0xae); /* bits: 8 -82dBm */
    hal_reg_write_bits(HH503_PHY0_CTL_BASE_0xD8, 0, 1, 0x1); /* PHY REG BANK1~6寄存器时钟强制开启Bypass寄存器
                                                     正常工作模式下REG BANK时钟自动gating，
                                                     使用WC寄存器上报时需要强制开启时钟 */
    hal_reg_write_bits(HH503_PHY0_CTL_BASE_0x20, 0x9, 1, 1); // FTM与PSD时钟复用
    hal_reg_write_bits(HH503_PHY_BANK4_BASE_0x26C, 1, 1, 0x1); /* 1:psd使能 */
}

osal_void hal_psd_disable_etc(osal_void)
{
    hal_psd_disable();
    /* FTM与PSD时钟复用, 需还原FTM PHY的时钟门控控制, bit:16 */
    hal_reg_write_bits(HH503_PHY0_CTL_BASE + 0x40, 16, 1, 0);
    return;
}

osal_void hal_psd_enable_etc(osal_void)
{
    hal_psd_enable();
    /* FTM与PSD时钟复用, 需使能FTM PHY的时钟门控控制, bit:16 */
    hal_reg_write_bits(HH503_PHY0_CTL_BASE + 0x40, 16, 1, 1);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif