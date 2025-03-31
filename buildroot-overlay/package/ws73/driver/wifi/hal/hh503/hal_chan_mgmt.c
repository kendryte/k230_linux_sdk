/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: phy func.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_phy.h"
#include "hal_phy_reg_field.h"
#include "hal_soc.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_rf.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "oal_ext_if.h"
#include "hal_chip.h"
#include "hal_mac.h"
#include "hal_power.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CHAN_MGMT_C

osal_void hh503_chan_measure_start(hal_chan_measure_stru *meas)
{
    /* 配置统计周期,15ms */
    u_ch_load_stat_period ch_load_stat_period;
    ch_load_stat_period.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x90);
    ch_load_stat_period.bits.x6Ngw6QwoiGFwCLGLwuW_BiF_ = meas->statics_period;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x90, ch_load_stat_period.u32);
    /* 配置DBB PHY统计周期 */
    hal_reg_write(HH503_PHY_BANK2_BASE_0x18C, (osal_u32)meas->meas_period);

    /* 信道统计使能寄存器不是常态值，软件不用存储其配置 */
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x70, OSAL_TRUE);
}

/*****************************************************************************
 功能描述  : 根据tx描述符更新beacon_rate寄存器域
*****************************************************************************/
osal_void hal_chan_update_beacon_rate(hh503_tx_phy_rate_stru *beacon_rate, const hal_tx_ctrl_desc_rate_stru *tx_dscr)
{
    if (!beacon_rate || !tx_dscr) {
        return;
    }

    beacon_rate->bits.xLPwqW6Li_wQWwoLNwLtuW_ = tx_dscr->nss_rate.phy_mode.ltf_type;
    beacon_rate->bits.xLPwqW6Li_wGhLwCWo_ = tx_dscr->tx_chain_sel;
    beacon_rate->bits.xLPwqW6Li_wF6S_ = tx_dscr->nss_rate.phy_mode.he_dcm;
    beacon_rate->bits.xLPwqW6Li_wS6Cw_GLW_ = tx_dscr->nss_rate.legacy_rate.legacy_rate;
    beacon_rate->bits.xR8OPmCRvwOGwvRvCvxOIv4m_ = tx_dscr->protocol_mode;

    /* ignore beacon_rate.bits.tx_vector_lsig_txop=hmac_vap->tx_mgmt_bmcast[channel_param->band].phy_info.lsig_txop */
    beacon_rate->bits.xLPwqW6Li_wu_WGSjoW_ = tx_dscr->nss_rate.phy_mode.preamble_mode;
    beacon_rate->bits.xLPwqW6Li_wgBwLtuW_ = tx_dscr->nss_rate.phy_mode.gi_type;
}
