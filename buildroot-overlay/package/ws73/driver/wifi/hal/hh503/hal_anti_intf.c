/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Implementation for MAC / PHY initialization and adapt interface.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_anti_intf.h"
#include "hh503_phy_reg.h"
#include "hal_mac_reg.h"
#include "hal_ext_if.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_ANTI_INTF_C

/*****************************************************************************
  2 函数定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
/*****************************************************************************
功能描述  : 抗干扰nav免疫设置接收duration最大值
*****************************************************************************/
osal_void hal_set_nav_max_duration(osal_u16 bss_dur, osal_u32 obss_dur)
{
    osal_u32 val;

    /* 本BSS门限值 */
    val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x64);
    val &= oal_mask_inverse(0x10, 16); /* 左移16 */
    val |= ((bss_dur << 16) & oal_mask(0x10, 16)); /* 左移16 */

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x64, val);

    /* OBSS门限值 */
    val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x68);
    val &= oal_mask_inverse(0x10, 0);
    val |= (obss_dur & oal_mask(0x10, 0));

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x68, val);
}

/*****************************************************************************
 函 数 名  : hal_get_nav_protect_times
 功能描述  : CCA优化特性下获取CCA门限寄存器
*****************************************************************************/
osal_void hal_get_nav_protect_times(osal_u32 *times)
{
    *times = (osal_u32)(hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xE8) >> 16); /* 右移16 */
}

/*****************************************************************************
函 数 名  : hal_set_agc_unlock_min_th
功能描述  : 抗干扰下设置agc 最小的tx、rx失锁门限
*****************************************************************************/
osal_void hal_set_agc_unlock_min_th(osal_s8 tx_reg_val, osal_s8 rx_reg_val)
{
    osal_u8 tx_val = (osal_u8)tx_reg_val;
    osal_u8 rx_val = (osal_u8)rx_reg_val;
    osal_u16 reg_val = oal_make_word16(rx_val, tx_val);
    osal_u32 val;

    val = hal_reg_read(HH503_PHY_BANK2_BASE_0xE4);
    val &= oal_mask_inverse(0x10, 0);
    val |= (reg_val & oal_mask(0x10, 0));

    hal_reg_write(HH503_PHY_BANK2_BASE_0xE4, val);
}

/*****************************************************************************
功能描述  : 设置干扰免疫使能和阈值寄存器
*****************************************************************************/
osal_void hal_set_weak_intf_rssi_th(osal_s32 reg_val)
{
    u_rx_maxlenfilt_weak_rssi rssi;

    rssi.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x50);
    /* 使能寄存器 */
    rssi.bits.xCt2OovO4lwmCROqmzdOwsslO4wvGOmo_ = 1;
    rssi.bits.xCt2OovO4lwmCROrryO4wvGOmo_ = 1;
    rssi.bits.x6NgwhiwFB_W6Lw00GgwF_iuwWh_ = 1;
    /* 门限寄存器 */
    rssi.bits.xCt2OqmzdOwsslOPzx_ = reg_val;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x50, rssi.u32);
}

/*****************************************************************************
 函 数 名  : hal_get_cca_reg_th
 功能描述  : CCA优化特性下获取CCA门限寄存器
*****************************************************************************/
osal_void hal_get_cca_reg_th(osal_s8 *ac_reg_val, osal_u32 reg_num)
{
    if (reg_num < 3) { /* 3:数组大小 */
        return;
    }
    ac_reg_val[0] = (osal_s8)(hal_reg_read(HH503_PHY_BANK2_BASE_0x174) & 0xFF);
    ac_reg_val[1] = (osal_s8)(hal_reg_read(HH503_PHY_BANK2_BASE_0x178) & 0xFF);
    ac_reg_val[2] = (osal_s8)(hal_reg_read(HH503_PHY_BANK2_BASE_0x188) & 0xFF); /* 2：VHT对应门限 */
}
#endif /* end of _PRE_WLAN_FEATURE_ANTI_INTERF */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif