
/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: hal alg func.
 * Create: 2022-1-12
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_alg_cca.h"
#include "hal_ext_if.h"
#include "wlan_spec.h"
#include "hal_mac_reg.h"
#include "mac_frame.h"
#include "hmac_ext_if.h"
#include "hal_mac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_ALG_CCA_C

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
/*****************************************************************************
 功能描述  :  11abg使能txop check cca
*****************************************************************************/
osal_void hh503_set_txop_check_cca(osal_u8 txop_check_cca)
{
    u_ht_vht_ctrl vht;
    vht.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC);
    vht.bits.xCt2OR8vGOCAdOCCzOzvoOmo_ = txop_check_cca;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC, vht.u32);
}

/*****************************************************************************
 函 数 名  : hh503_enable_sync_error_counter
 功能描述  : CCA优化特性下使能sync error统计寄存器
*****************************************************************************/
osal_void hh503_enable_sync_error_counter(osal_s32 enable_cnt_reg_val)
{
    osal_u32 val = hal_reg_read(HH503_PHY_BANK1_BASE_0x1BC);
    unref_param(enable_cnt_reg_val);

    /* 节点3表示OFDM同步检测错误次数 */
    val &= (~HH503_PHY_CFG_STA0_EN_MASK);
    val |= BIT3;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x1BC, val);
}

/*****************************************************************************
 函 数 名  : hh503_get_sync_error_cnt
 功能描述  : CCA优化特性下获取sync error的统计计数值
*****************************************************************************/
osal_void hh503_get_sync_error_cnt(osal_u32 *reg_val)
{
    osal_u32 val = hal_reg_read(HH503_PHY_BANK1_BASE_0x1BC);

    reg_val[0] = 0;

    /* 若sta0的统计节点不是节点3，则无效 */
    val &= HH503_PHY_CFG_STA0_EN_MASK;
    if (val != BIT3) {
        return;
    }

    reg_val[0] = hal_reg_read(HH503_PHY_BANK1_BASE_0x1CC);
}

/*****************************************************************************
 函 数 名  : hh503_set_sync_err_counter_clear
 功能描述  : CCA优化特性下清空sync error统计寄存器
*****************************************************************************/
osal_void hh503_set_sync_err_counter_clear(osal_void)
{
    osal_u32 val = hal_reg_read(HH503_PHY_BANK1_BASE_0x1B8);
    val = val | BIT8;

    /* 清空sta0的计数 */
    hal_reg_write(HH503_PHY_BANK1_BASE_0x1B8, val);
}

/*****************************************************************************
 函 数 名  : hh503_set_ed_high_th
 功能描述  : CCA优化下设置cca的20/40M检测门限
*****************************************************************************/
osal_void hh503_set_ed_high_th(osal_s8 ed_high_20_reg_val, osal_s8 ed_high_40_reg_val,
    oal_bool_enum_uint8 is_default_th)
{
    osal_u32 val;

    unref_param(is_default_th);
    // cca 20/40
    val = hal_reg_read(HH503_PHY_BANK2_BASE_0x170);
    val = (val & 0xFFFFFF00) | ((osal_u32)ed_high_40_reg_val & 0xFF); // bit0~7, 40 ed th
    val = (val & 0xFFFF00FF) | (((osal_u32)ed_high_20_reg_val << 8) & 0xFF00); // bit 8~15 20 ed th
    hal_reg_write(HH503_PHY_BANK2_BASE_0x170, val);
}

/*****************************************************************************
 函 数 名  : hh503_set_cca_prot_th
 功能描述  : CCA优化下设置cca的20/40M检测门限
*****************************************************************************/
osal_void hh503_set_cca_prot_th(osal_s8 sd_cca_20th_dsss_val, osal_s8 sd_cca_20th_ofdm_val)
{
    osal_u32 val;

    val = hal_reg_read(HH503_PHY_BANK2_BASE_0x174);
    val = (val & 0xFFFFFF00) | ((osal_u32)sd_cca_20th_dsss_val & 0xFF);
    hal_reg_write(HH503_PHY_BANK2_BASE_0x174, val);

    val = hal_reg_read(HH503_PHY_BANK2_BASE_0x178);
    val = (val & 0xFFFFFF00) | ((osal_u32)sd_cca_20th_ofdm_val & 0xFF);
    hal_reg_write(HH503_PHY_BANK2_BASE_0x178, val);
}

#endif /* _PRE_WLAN_FEATURE_CCA_OPT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

