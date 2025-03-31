/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: hal alg func.
 * Create: 2022-1-12
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_alg_txbf.h"
#include "hal_ext_if.h"
#include "wlan_spec.h"
#include "hal_mac_reg.h"
#include "mac_frame.h"
#include "hmac_ext_if.h"
#include "hal_mac.h"
#include "hal_power.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_ALG_TXBF_WS73_C

/*****************************************************************************
  3 函数实现
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_TXBF
/*****************************************************************************
功能描述  : 设置action no ack的速率和功率
*****************************************************************************/
osal_void hal_set_bf_rate(osal_u8 rate)
{
    osal_u32 tx_data_rate;
    tx_data_rate = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x58);

    ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xLPwqW6Li_wS6Cw_GLW_ = rate;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x58, tx_data_rate);
}

/*****************************************************************************
功能描述  : 设置he buff addr,vht sounding使用
*****************************************************************************/
osal_void hh503_set_txbf_he_buff_addr(osal_u32 addr, osal_u16 buffer_len)
{
    u_sounding_ctrl5 sounding_ctrl5;
    u_vap2_mode_set1 vap2_mode_set1;
    u_vap0_he_matrix_buff_addr vap0_addr;
    u_vap1_he_matrix_buff_addr vap1_addr;

    /* matrix buff */
    buffer_len /= WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE;
    /* vap0 */
    addr &= HH503_MAC_ADDR_MASK;
    vap0_addr.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x5C);
    vap0_addr.bits.x6NgwqGuywQWwSGL_BPwjdNNwGFF__ = addr;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x5C, vap0_addr.u32);
    sounding_ctrl5.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x4C);
    sounding_ctrl5.bits.x6NgwLPjNwQWwjdNNywoWh_ = buffer_len;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x4C, sounding_ctrl5.u32);
    /* vap1 */
    addr += (osal_u32)buffer_len;
    addr &= HH503_MAC_ADDR_MASK;
    vap1_addr.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x60);
    vap1_addr.bits.xCt2OPzGrOAmOIzRwl8OySttOz44w_ = addr;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x60, vap1_addr.u32);
    sounding_ctrl5.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x4C);
    sounding_ctrl5.bits.xCt2OR8ytOAmOySttrOxmo_ = buffer_len;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x4C, sounding_ctrl5.u32);
    /* vap2 */
    addr += (osal_u32)buffer_len;
    addr &= HH503_MAC_ADDR_MASK;
    vap2_mode_set1.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x11C);
    vap2_mode_set1.bits.x6NgwqGuVwQWwSGL_BPwjdNNwGFF__ = addr;
    vap2_mode_set1.bits.x6NgwLPjNwQWwjdNNVwoWh_ = buffer_len;
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x11C, vap2_mode_set1.u32);
}

/*****************************************************************************
功能描述  : 设置bfee的使能参数
*****************************************************************************/
osal_void hh503_set_bfee_sounding_en(osal_u8 bfee_enable)
{
    u_pa_control2 pa_ctrl2;

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.xCt2OAmOsvSo4lo2Omo_ = bfee_enable; /* HE BFEE sounding使能 */

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
}

/*****************************************************************************
功能描述  : 设置bfee grouping codebook
*****************************************************************************/
osal_void hh503_set_bfee_grouping_codebook(osal_u8 codebook, osal_u8 min_group)
{
    /*       HT codebook information(2 bits)       */
    /*    set to 0 for 1 bit for Ψ,3 bits for Φ    */
    /*    set to 1 for 2 bit for Ψ,4 bits for Φ    */
    /*    set to 2 for 3 bit for Ψ,5 bits for Φ    */
    /*    set to 3 for 4 bit for Ψ,6 bits for Φ    */

    /*       VHT codebook information(1 bit)        */
    /* If Feedback Type is SU:                        */
    /*    set to 0 for 2 bits for Ψ,4 bits for Φ   */
    /*    set to 1 for 4 bits for Ψ,6 bits for Φ   */
    /* If Feedback Type is MU:                       */
    /*    set to 0 for 5 bits for Ψ,7 bits for Φ   */
    /*    set to 1 for 7 bits for Ψ,9 bits for Φ   */
    u_sounding_ctrl1 sounding_ctrl1;
    u_sounding_ctrl2 sounding_ctrl2;

    sounding_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x44);
    sounding_ctrl1.bits.x6NgwQWwCidhFBhgwuG_GSwFBNNwWh_ = 0;   /* 表示该codebook由NDPA决定，而非beamformee自身决定 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x44, sounding_ctrl1.u32);

    /* he 模式下 */
    sounding_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x48);
    sounding_ctrl2.bits.x6NgwhihwLjwCidhFBhgw_uLw6iFWjiiK_ = codebook;
    sounding_ctrl2.bits.xCt2OovoORyOsvSo4lo2OwGROo2_ = min_group; /* 0x0 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x48, sounding_ctrl2.u32);
}

#endif /* end of _PRE_WLAN_FEATURE_TXBF */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
