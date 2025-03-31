/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_psd.c
 * Create: 2023-01-16
 */
#ifdef _PRE_WLAN_DFT_STAT
#include "hal_common_ops.h"
#include "hal_mac.h"
#include "hal_phy.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_DFT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
功能描述  : 获取mac rx关键统计信息
*****************************************************************************/
osal_void hh503_get_mac_rx_statistics_data(hal_mac_rx_mpdu_statis_info_stru *mac_rx_statis)
{
    osal_u32 int_save = frw_osal_irq_lock();
    u_rpt_rx_ampdu_count rpt_rx_ampdu_count;
    u_rpt_rx_filtered_cnt x_PwNBoLW_WFw6hL_;
    rpt_rx_ampdu_count.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x38);
    mac_rx_statis->rx_ampdu_cnt = rpt_rx_ampdu_count.bits.x_PwGSuFdw6idhL_;
    mac_rx_statis->rx_suc_mpdu_inampdu_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x13C);
    mac_rx_statis->rx_err_mpdu_inampdu_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x140);
    mac_rx_statis->rx_suc_mpdu_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x48);
    mac_rx_statis->rx_err_mpdu_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x4C);

    x_PwNBoLW_WFw6hL_.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x5C);
    mac_rx_statis->rx_filter_mpdu_cnt = x_PwNBoLW_WFw6hL_.bits.x_PwNBoLW_WFw6hL_;
    frw_osal_irq_restore(int_save);

    return;
}

/*****************************************************************************
功能描述  : 获取mac tx关键统计信息
*****************************************************************************/
osal_void hh503_get_mac_tx_statistics_data(hal_mac_tx_mpdu_statis_info_stru *mac_tx_statis)
{
    osal_u32 int_save = frw_osal_irq_lock();
    osal_u32 high_normal_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x24);
    osal_u32 tx_ampdu_cnt = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x28);

    mac_tx_statis->tx_high_priority_mpdu_cnt =
        (high_normal_cnt & HH503_MAC_RPT_TX_HI_MPDU_CNT_MASK) >> HH503_MAC_RPT_TX_HI_MPDU_CNT_OFFSET;
    mac_tx_statis->tx_normal_priority_mpdu_cnt = high_normal_cnt & HH503_MAC_RPT_TX_NORMAL_MPDU_CNT_MASK;

    mac_tx_statis->tx_ampdu_cnt =
        (tx_ampdu_cnt & HH503_MAC_RPT_TX_AMPDU_COUNT_MASK) >> HH503_MAC_RPT_TX_AMPDU_COUNT_OFFSET;
    mac_tx_statis->tx_ampdu_inmpdu_cnt = tx_ampdu_cnt & HH503_MAC_RPT_TX_MPDU_INAMPDU_COUNT_MASK;
    mac_tx_statis->tx_complete_cnt                  = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0xA8);
    frw_osal_irq_restore(int_save);
    return;
}

/*****************************************************************************
 功能描述  : 清除MAC统计信息
*****************************************************************************/
osal_void hh503_hw_stat_clear(osal_void)
{
    osal_u32 int_save = frw_osal_irq_lock();
    /* RX 寄存器电平拉高 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 21, 1, 1); /* 21bit表示清除RX AMPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 20, 1, 1); /* 20bit表示清除RX AMPDU中FCS正确的mpdu统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 19, 1, 1); /* 19bit表示清除RX AMPDU中FCS错误的mpdu统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 15, 1, 1); /* 15bit表示清除RX MPDU FCS正确统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 14, 1, 1); /* 14bit表示清除RX MPDU FCS错误统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 22, 1, 1); /* 22bit表示清除RX 过滤统计 */

    /* TX 寄存器电平拉高 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 9, 1, 1); /* 9bit表示清除TX 高优先级MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 8, 1, 1); /* 8bit表示清除TX 普通优先级MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 6, 1, 1); /* 6bit表示清除TX AMPDU中MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 7, 1, 1); /* 7bit表示清除TX AMPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x140, 4, 1, 1); /* 4bit表示清除TX COMPLETE统计 */

    /* RX 寄存器电平拉低 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 21, 1, 0); /* 21bit表示清除RX AMPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 20, 1, 0); /* 20bit表示清除RX AMPDU中FCS正确的mpdu统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 19, 1, 0); /* 19bit表示清除RX AMPDU中FCS错误的mpdu统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 15, 1, 0); /* 15bit表示清除RX MPDU FCS正确统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 14, 1, 0); /* 14bit表示清除RX MPDU FCS错误统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 22, 1, 0); /* 22bit表示清除RX 过滤统计 */

    /* TX 寄存器电平拉低 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 9, 1, 0); /* 9bit表示清除TX 高优先级MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 8, 1, 0); /* 8bit表示清除TX 普通优先级MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 6, 1, 0); /* 6bit表示清除TX AMPDU中MPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x13C, 7, 1, 0); /* 7bit表示清除TX AMPDU统计 */
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0x140, 4, 1, 0); /* 4bit表示清除TX COMPLETE统计 */

    frw_osal_irq_restore(int_save);
}

/*****************************************************************************
功能描述  : 读取CTRL0寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_ctrl0(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_CTRL0_BANK_BASE; (reg_addr < HH503_MAC_CTRL0_BANK_BASE + HH503_MAC_CTRL0_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取CTRL1寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_ctrl1(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_CTRL1_BANK_BASE; (reg_addr < HH503_MAC_CTRL1_BANK_BASE + HH503_MAC_CTRL1_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取CTRL2寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_ctrl2(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_CTRL2_BANK_BASE; (reg_addr < HH503_MAC_CTRL2_BANK_BASE + HH503_MAC_CTRL2_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取RD0寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_rd0(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_RD0_BANK_BASE; (reg_addr < HH503_MAC_RD0_BANK_BASE_0x20) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
    for (reg_addr = HH503_MAC_RD0_BANK_BASE_0x34; (*len <= HAL_DFT_REG_OTA_LEN) &&
        (reg_addr < HH503_MAC_RD0_BANK_BASE + HH503_MAC_RD0_OFFEST_BUTT); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取RD1寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_rd1(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_RD1_BANK_BASE; (reg_addr < HH503_MAC_RD1_BANK_BASE + HH503_MAC_RD1_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取LUT0寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_lut0(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_LUT0_BANK_BASE; (reg_addr < HH503_MAC_LUT0_BANK_BASE + HH503_MAC_LUT0_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 读取wlmac寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_mac_wlmac(osal_u32 *machw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_MAC_WLMAC_CTRL_BASE; (reg_addr <= HH503_MAC_WLMAC_CTRL_BASE_0x110) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
    for (reg_addr = HH503_MAC_WLMAC_CTRL_BASE_0x800;
        (reg_addr < HH503_MAC_WLMAC_CTRL_BASE + HH503_MAC_WLMAC_CTRL_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4, *len += 0x4) {
        *machw_stat = hal_reg_read(reg_addr);
        machw_stat = machw_stat + 1;
    }
}

/*****************************************************************************
功能描述  : 维测读取硬件寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_machw_stat_info(osal_u32 *machw_stat, osal_u8 bank_select, osal_u32 *len)
{
    switch (bank_select) {
        case MAC_CTRL0_BANK:
            hh503_dft_get_mac_ctrl0(machw_stat, len);
            break;
        case MAC_CTRL1_BANK:
            hh503_dft_get_mac_ctrl1(machw_stat, len);
            break;
        case MAC_CTRL2_BANK:
            hh503_dft_get_mac_ctrl2(machw_stat, len);
            break;
        /* mac RD0中有7个寄存器不能读取，否则会报错 */
        case MAC_RD0_BANK:
            hh503_dft_get_mac_rd0(machw_stat, len);
            break;
        case MAC_RD1_BANK:
            hh503_dft_get_mac_rd1(machw_stat, len);
            break;
        case MAC_LUT0_BANK:
            hh503_dft_get_mac_lut0(machw_stat, len);
            break;
        case MAC_WLMAC_CTRL:
            hh503_dft_get_mac_wlmac(machw_stat, len);
            break;

        default:
            break;
    }
}

/*****************************************************************************
功能描述  : 获取MAC寄存器
*****************************************************************************/
osal_void hh503_dft_force_report_mac_reg_state(oal_netbuf_stru *netbuff)
{
    osal_u8 reg_select_idx;
    osal_u32 data_len;

    osal_u32 *report_reg_data = (osal_u32 *)oal_netbuf_data(netbuff);
    for (reg_select_idx = MAC_CTRL0_BANK; reg_select_idx < WITP_MAC_BANK_BUTT; reg_select_idx++) {
        /* 将统计值清零 */
        (osal_void)memset_s(report_reg_data, HAL_DFT_REG_OTA_LEN, 0, HAL_DFT_REG_OTA_LEN);
        /* 从MAC寄存器获取统计值 */
        hh503_dft_get_machw_stat_info(report_reg_data, reg_select_idx, &data_len);
    }
}

/*****************************************************************************
功能描述  : 错误发生时，打印重要mac寄存器的值
*****************************************************************************/
osal_void hh503_dft_print_machw_stat(osal_void)/////////////////暂时未改，确认应该打印哪些寄存器后再改
{
}

/*****************************************************************************
功能描述  : 读取PHY_BANK1寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank1(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK1_BASE; (reg_addr < HH503_PHY_BANK1_BASE + HH503_PHY_BANK1_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 读取PHY_BANK2寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank2(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK2_BASE; (reg_addr < HH503_PHY_BANK2_BASE + HH503_PHY_BANK2_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 读取PHY_BANK3寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank3(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK3_BASE; (reg_addr < HH503_PHY_BANK3_BASE + HH503_PHY_BANK3_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 读取PHY_BANK4寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank4(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK4_BASE; (reg_addr < HH503_PHY_BANK4_BASE + HH503_PHY_BANK4_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 读取PHY_BANK5寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank5(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK5_BASE; (reg_addr < HH503_PHY_BANK5_BASE + HH503_PHY_BANK5_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 读取PHY_BANK6寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_bank6(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY_BANK6_BASE; (reg_addr < HH503_PHY_BANK6_BASE + HH503_PHY_BANK6_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}


/*****************************************************************************
功能描述  : 读取PHY0_CTRL寄存器上报的值
*****************************************************************************/
osal_void hh503_dft_get_phy_phy0_ctrl(osal_u32 *phyhw_stat, osal_u32 *len)
{
    osal_u32 reg_addr;
    *len = 0;

    for (reg_addr = HH503_PHY0_CTL_BASE; (reg_addr < HH503_PHY0_CTL_BASE + HH503_PHY0_CTL_OFFEST_BUTT) &&
        (*len <= HAL_DFT_REG_OTA_LEN); reg_addr += 0x4) {
        *phyhw_stat = hal_reg_read(reg_addr);
        phyhw_stat += 1;
        *len += 0x4;
    }
}

/*****************************************************************************
功能描述  : 获取phy寄存器的值
*****************************************************************************/
osal_void hh503_dft_get_phyhw_stat_info(osal_u32 *phyhw_stat, osal_u8 bank_select, osal_u32 *len)
{
    switch (bank_select) {
        case WITP_PHY_BANK_0:
            break;
        case WITP_PHY_BANK_1:
            hh503_dft_get_phy_bank1(phyhw_stat, len);
            break;
        case WITP_PHY_BANK_2:
            hh503_dft_get_phy_bank2(phyhw_stat, len);
            break;
        case WITP_PHY_BANK_3:
            hh503_dft_get_phy_bank3(phyhw_stat, len);
            break;
        case WITP_PHY_BANK_4:
            hh503_dft_get_phy_bank4(phyhw_stat, len);
            break;
        case WITP_PHY_BANK_5:
            hh503_dft_get_phy_bank5(phyhw_stat, len);
            break;
        case WITP_PHY_BANK_6:
            hh503_dft_get_phy_bank6(phyhw_stat, len);
            break;
        case WITP_PHY_PHY0_CTRL:
            hh503_dft_get_phy_phy0_ctrl(phyhw_stat, len);
            break;
        default:
            break;
    }
}

/*****************************************************************************
函 数 名  : hh503_dft_set_phy_stat_node
功能描述  : 设置phy观测点
*****************************************************************************/
osal_void hh503_dft_set_phy_stat_node(oam_stats_phy_node_idx_stru *phy_node_idx)
{
    unref_param(phy_node_idx);
}

/*****************************************************************************
功能描述  :错误发生时，打印重要phy寄存器的值
*****************************************************************************/
osal_void hh503_dft_print_phyhw_stat(osal_void)
{
}

/*****************************************************************************
功能描述  : 获取所有寄存器的状态
*****************************************************************************/
osal_void hh503_dft_force_report_all_reg_state(hal_to_dmac_device_stru *hal_device)
{
    oal_netbuf_stru *netbuff = (oal_netbuf_stru *)oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, HAL_DFT_REG_OTA_LEN,
        OAL_NETBUF_PRIORITY_MID);
    if (netbuff == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_DFT, "{hh503_dft_report_all_reg_state::alloc netbuff failed! size[%d].}",
            HAL_DFT_REG_OTA_LEN);
        return;
    }

    /* 上报MAC寄存器的值 */
    if (hal_device->reg_info_flag & BIT2) {
        hh503_dft_force_report_mac_reg_state(netbuff);
    }
    oal_netbuf_free(netbuff);
}

/*****************************************************************************
功能描述  : 获取所有寄存器的状态
*****************************************************************************/
osal_void hh503_dft_report_all_reg_state_rom_cb(hal_to_dmac_device_stru *hal_device)
{
    hh503_dft_force_report_all_reg_state(hal_device);
}

/*****************************************************************************
功能描述  : 获取所有寄存器的状态
*****************************************************************************/
osal_void hh503_dft_report_all_reg_state(hal_to_dmac_device_stru *hal_device)
{
    hh503_dft_report_all_reg_state_rom_cb(hal_device);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
