/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: MAC/PHY initialization and adapt interface implementation.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_mac.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hal_soc_reg.h"
#endif
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "wlan_mib_hcm.h"
#include "hal_soc.h"
#include "hal_power.h"
#include "hal_mac_reg.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_chip.h"
#include "hal_phy.h"
#include "hal_rf.h"
#include "hal_reset.h"
#include "hmac_sdp.h"
#include "hmac_feature_interface.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hal_coex_reg.h"
#endif
#include "hh503_phy_reg.h"
#include "hal_csi.h"
#include "hal_tbtt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_MAC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HAL_MAX_TRLR_TEST_WORD_CNT 5 /* mac上报trailer个数 */

#define HH503_MAC_FREQ_TABLE_LEN 7

const osal_u32 g_datarate_reg_table[] = {
    HH503_MAC_CTRL0_BANK_BASE_0xD8, /* 配置prop  DATARATE */
    HH503_MAC_CTRL0_BANK_BASE_0xDC,
    HH503_MAC_CTRL0_BANK_BASE_0xE0,
    HH503_MAC_CTRL0_BANK_BASE_0xE4,
    HH503_MAC_CTRL0_BANK_BASE_0xF4,   /* 配置MAC CF END DATARATE */
    HH503_MAC_CTRL0_BANK_BASE_0xEC,     /* 配置MAC RSP DATARATE */
    HH503_MAC_CTRL0_BANK_BASE_0x110, /* 配置MAC RSP CTS DATARATE */
    HH503_MAC_CTRL0_BANK_BASE_0xFC,   /* update BAR datarate */
    HH503_MAC_CTRL0_BANK_BASE_0x100,
    HH503_MAC_CTRL0_BANK_BASE_0x104,
    HH503_MAC_CTRL0_BANK_BASE_0x108,
    HH503_MAC_CTRL1_BANK_BASE_0x14,              /* 配置ONE PACKET DATARATE */
    HH503_MAC_CTRL1_BANK_BASE_0x58,           // vht 和 he date rate共用
    HH503_MAC_CTRL1_BANK_BASE_0x80, /* 配置COEX DATARATE */
    // HH503_MAC_TX_FOREVER_DATA_RATE,  /* 配置长发 DATARATE */
    HH503_MAC_CTRL2_BANK_BASE_0x20, /* 配置BEACON DATARATE */
    HH503_MAC_CTRL2_BANK_BASE_0x48 /* 配置BEACON DATARATE */
};

osal_u32 g_mac_freq_reg_val[][HH503_MAC_FREQ_TABLE_LEN] = {
    {0x28, 0x4, 0, 0x2, 0x418, 1}, /* MAC工作频率40MHz */
    {0x50, 0x8, 0, 0x2, 0x414, 1}, /* MAC工作频率80MHz */
    {0xA0, 0x10, 0, 0x2, 0x412, 1}, /* MAC工作频率160M */
    {0x140, 0x20, 0, 0x2, 0x411, 1}, /* MAC的工作频率320M */
};

oal_bool_enum_uint8 g_dyn_bypass_extlna_pm_flag = OSAL_TRUE;  /* 根据吞吐判断是否为低功耗测试场景 */

osal_void hh503_set_tx_qos_seq_num(osal_u8 lut_index, osal_u8 tid, osal_u32 val_write);

#ifdef _PRE_WLAN_FEATURE_DAQ
osal_void hal_pkt_ram_sample_deinit(osal_void)
{
    oam_info_log0(0, OAM_SF_CFG, "hal_pkt_ram_sample_deinit done!");
}

osal_void hal_sample_daq_prepare_data(osal_void)
{
    oam_info_log0(0, OAM_SF_CFG, "hal_sample_daq_prepare_data done!");
}

osal_u32 hal_sample_daq_get_data(osal_u32 diag_read_addr)
{
    return hal_reg_read(diag_read_addr + HAL_MAC_DIAG_DAQ_OFFSET);
}

oal_bool_enum_uint8 hal_sample_daq_done(osal_void)
{
    osal_u32 diag_end_en;

    diag_end_en = hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_REG);
    if ((diag_end_en & 0x1) != 0) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

osal_void hal_show_mac_daq_reg_cfg_info(osal_void)
{
    oam_warning_log4(0, OAM_SF_CFG, "{mac_daq_reg_cfg::clk[0x%x] ctl1[0x%x] reuser1[0x%x] diag_en[0x%x]}",
        hal_reg_read(HH503_MAC_WLMAC_CTRL_BASE_0x60), hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C),
        hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x19C), hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78));
}

osal_void hal_show_daq_done_reg_info(osal_void)
{
    oam_warning_log3(0, OAM_SF_CFG,
        "{daq_done_reg_info::sample_done_reg[%x] sample_done_addr_reg[%x] fifo_status_reg[%x]}",
        hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_REG), hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_ADDR_REG),
        hal_reg_read(DIAG_CTL_RB_MONITOR_FIFO_STATUS_REG));
}
#endif

#ifdef _PRE_WIFI_DEBUG
osal_void hal_show_linkloss_reg_info(osal_void)
{
    oam_warning_log4(0, OAM_SF_DFT,
        "hal_show_linkloss_reg_info::mac rx_cnt[0x%x], rx_queue[0x%x], intr[0x%x], err_intr[0x%x]",
        hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x48), hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x10),
        hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54), hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x58));

    oam_warning_log4(0, OAM_SF_DFT,
        "hal_show_linkloss_reg_info::mac vector word0[0x%x], word1[0x%x], word2[0x%x], word3[0x%x]",
        hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x170), hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x174),
        hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x178), hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x17C));

    oam_warning_log4(0, OAM_SF_DFT,
        "hal_show_linkloss_reg_info::mac vector word4[0x%x], word5[0x%x], word6[0x%x], word7[0x%x]",
        hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x180), hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x184),
        hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x188), hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x18C));

    oam_warning_log2(0, OAM_SF_DFT, "hal_show_linkloss_reg_info::mac err_intr2[0x%x], phy 11b frame cnt[0x%x]",
        hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x5C), hal_reg_read(HH503_PHY_BANK1_BASE_0x1DC));
}
#endif

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
/*****************************************************************************
 功能描述  :初始化硬件聚合相关寄存器
*****************************************************************************/
osal_void hh503_set_hw_en_reg_cfg(osal_u8 hw_en)
{
    u_ht_vht_ctrl vht;
    u_ampdu_ctrl ampdu;

    vht.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC); // ctrl0+0xc [1]
    vht.bits.x6NgwLPiuw_WSwLBSWw6QKwWh_ = hw_en;
    /* 硬件聚合 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC, vht.u32);

    ampdu.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18); // ctrl0+0xc [1]
    ampdu.bits.x6NgwQTw6Go6wGSuFdwoWhwWh_ = hw_en;
    ampdu.bits.x6NgwQTwuC_wjBLSGuwWh_ = hw_en;
    ampdu.bits.xCt2OAqOR8OyzwOmo_ = hw_en;
    ampdu.bits.x6NgwQTw_WL_twuG_LBGowGSuFdwWh_ = hw_en;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18, ampdu.u32);
}
#endif

/*****************************************************************************
 功能描述  : 设置响应帧带宽,
*****************************************************************************/
osal_void hh503_set_dup_rts_cts_mode(osal_void)
{
    u_pa_control1 pa_ctrl;
    u_tx_resp_ctrl resp;
    /* 使能dup rts */
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.xCt2OR8OwRsO4SGOmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);

    /* 使能dup cts, 根据 */

    /* 当前使能non ht和vht的dup rsp帧回复,MAC根据对端带宽回复 */
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.x6Ngw_PwQLw_WCuwFduwWh_ = 1;
    resp.bits.x6Ngw_PwhihwQLw_WCuwFduwWh_ = 1;
    /* HT、HE后期根据需求决定是否需要打开,HT打开增加40M模式抗干扰能力 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}

/*****************************************************************************
 功能描述  : hh503_mac_reg_init
*****************************************************************************/
osal_void hh503_mac_reg_init(osal_void)
{
    u_pa_control1 pa_ctrl;
    u_pa_control2 pa_ctrl2;
    u_ht_vht_ctrl vht;
    u_tx_resp_ctrl resp;
    u_rx_maxlenfilt_weak_rssi rssi;
    u_phytxrxplcp_dly dly;
    u_rx_desc_test_ctrl desc;
    u_sounding_ctrl2 sounding_ctrl2;
    osal_u32 value;
    u_rx_buffer_length len;
    hal_reg32_cfg_stru machw_reg_tbl[] = {
        {HH503_MAC_CTRL0_BANK_BASE_0x84,             0x8449},
        /* 配置BAR移窗约束 HWPSIRT-2022-04416 bar为0， data为4095 */
        {HH503_MAC_CTRL0_BANK_BASE_0x130, 0x0000FFF},
        /* 限制MAC访问 DTCM 0x20060000~0x20080000,pkt ram */
        /* 设置接收帧过滤 */
        {HH503_MAC_CTRL0_BANK_BASE_0x48, HH503_MAC_CFG_INIT},
        {HH503_MAC_CTRL2_BANK_BASE_0x1C, HH503_PHY_TX_MODE_DEFAULT},
        {HH503_MAC_CTRL2_BANK_BASE_0x44, HH503_PHY_TX_MODE_DEFAULT},
        {HH503_MAC_CTRL2_BANK_BASE_0x13C, HH503_PHY_TX_MODE_DEFAULT}
    };

    hal_reg_write_tbl(machw_reg_tbl, sizeof(machw_reg_tbl) / sizeof(hal_reg32_cfg_stru));
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x34, OAL_PKTRAM_ADDR_BEGIN & 0xFFFFF);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x38, OAL_PKTRAM_ADDR_END & 0xFFFFF);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x40, HAL_DSCR_BASE_ADDR);
    rssi.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x50);
    rssi.bits.x6Ngw_PwSuFdwSGPwoWh_ = HAL_RX_FRAME_MAX_LEN;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x50, rssi.u32); // {HH503_MAC_RX_MAXLENFILT, HAL_RX_FRAME_MAX_LEN}

    len.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x44);
    /* 右移2位 */
    len.bits.xCt2Ow8OovwIOySttOxmo_ = HAL_RX_FRAME_LEN >> 2; // 逻辑约束单位是word，而不是原来的byte 右移2
    len.bits.x6Ngw_PwQBu_BwjdNNwoWh_ = HAL_RX_MGMT_FRAME_LEN >> 2; // 逻辑约束单位是word，而不是原来的byte 右移2
    len.bits.x6Ngw_PwCSGoowjdNNwoWh_ = HAL_RX_SMALL_FRAME_LEN >> 2; // 逻辑约束单位是word，而不是原来的byte 右移2
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x44, len.u32);

    dly.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x78);
    dly.u32 &= (~(HH503_MAC_CFG_VHT_OFDM_RXPLCP_DELAY_MASK | HH503_MAC_CFG_HT_OFDM_RXPLCP_DELAY_MASK
        | HH503_MAC_CFG_NON_HT_OFDM_RXPLCP_DELAY_MASK | HH503_MAC_CFG_DSSS_RXPLCP_DELAY_MASK));
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x78, dly.u32 | 0x3C5); // {HH503_MAC_PHYRXPLCP_DELAY,              0xCCC5},

    value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x7C);
    value &= (~(HH503_MAC_CFG_OFDM_RX2TX_TURNAROUND_TIME_MASK | HH503_MAC_CFG_DSSS_RX2TX_TURNAROUND_TIME_MASK));
    /* {HH503_MAC_PHYRXTX_TURNAROUND_TIME,      0x1B0010}, 0x1B0010 去掉[15:8]=0x1b10 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x7C,
        value | (0x1819 << HH503_MAC_CFG_OFDM_RX2TX_TURNAROUND_TIME_OFFSET));

    sounding_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x48);
    sounding_ctrl2.bits.x6NgwSGL_BPw6Go6wLBSWidLwqGodW_ = 0x32;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x48, sounding_ctrl2.u32);

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.xCt2Ow8OloRwOSolR_ = 4; // 4个报文上报一个中断
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32); // {HH503_MAC_RX_INT_UNIT_CTRL, 0x00000008}

    hh503_set_dup_rts_cts_mode();
    vht.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC);
    vht.bits.xCt2Ow8OAlGwlOloOovwIOySttOmo_ = 0;
    vht.bits.xCt2Ow8OsIzxxOloOovwIOySttOmo_ = 0;
    vht.bits.x6NgwLPiuw_LCwSiFW_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC, vht.u32);
    /* 过滤tx suspend中断,bit12 */
    hh503_mask_interrupt(12);
    /* 屏蔽error2中断中NAV保护生效中断上报 寄存器整体替换 ctrl0+0x78 */
    hh503_mask_mac_error2_init_status(HH503_MAC_RPT_ERR_BSS_NAV_PROT_MASK | HH503_MAC_RPT_ERR_OBSS_NAV_PROT_MASK);

    /* 将LUT表清零 */
    hh503_ce_clear_all();

    /* 用于双天线RSSI上报,作为MIMO/SISO场景切换的输入判决 */
    /* RX DSCR维测信息rpt_test_0word上报RX VECTOR WORD0, START */
    desc.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x134);
    desc.bits.xCt2Ow8O4msCORmsRO3qvw4Osmx_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x134, desc.u32);
    /* RX DSCR维测信息rpt_test_0word上报RX VECTOR WORD0, END */

    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.x6NgwuQtwBhNiwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);

    /* MAC收到TXBF 2*2sounding时,反馈2*1矩阵 */
    // hal_reg_write(HH503_MAC_TXBF_NC_MAX, 1); //寄存器已删除

    /* 动态带宽选择，默认选择rx hdr里的freq bandwidth mode，防止cts回复大于phy当前工作带宽,mpw2
     * 80M存在问题 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x8C,
        (hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x8C) | HH503_MAC_CFG_RX_DYN_BW_SEL_MASK)); // ctrl1+0x104[3]

    /* 芯片有bug, MAC BUS 、 TX 、 RX clock gating常开, 无需再配置;Pilot 解决 */

    /* 接收帧回复响应帧需要检查rx trailer里的status的使能 */
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.x6Ngw_Pw6QKwL_o_wCLGLdCwFBC_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}
#ifdef BOARD_ASIC_WIFI
/*****************************************************************************
 功能描述  : 更新mac freq
*****************************************************************************/
osal_void hh503_set_mac_freq(hal_clk_freq_enum_uint8 mac_clk_freq)
{
    u_phy_wlmac_cgr phy_wlmac_cgr;
    u_num_pa_clks_us num_pa_clks_us;

    phy_wlmac_cgr.u32 = hal_reg_read(HH503_SOC_PHY_WLMAC_CGR);
    num_pa_clks_us.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x20);

    phy_wlmac_cgr.bits.xTLiuwFBqwWh_ = 1;
    phy_wlmac_cgr.bits.xSG6wSGBhw6oKWh_ = 1;
    if (mac_clk_freq == HAL_CLK_160M_FREQ) {
        phy_wlmac_cgr.bits.xSG6wSGBhwFBqwhdS_ = 1; /* 1: 代表2分频，源头为320M时钟，分频后为160M时钟 */

        num_pa_clks_us.bits.xCt2OoSIOGzOCxdsO4mClOSs_ = 0x10;
        num_pa_clks_us.bits.x6NgwhdSwuGw6oKCwihWwdC_ = 0xA0;
    } else if (mac_clk_freq == HAL_CLK_80M_FREQ) {
        phy_wlmac_cgr.bits.xSG6wSGBhwFBqwhdS_ = 2; /* 2: 代表4分频，源头为320M时钟，分频后为80M时钟 */

        num_pa_clks_us.bits.xCt2OoSIOGzOCxdsO4mClOSs_ = 0x8;
        num_pa_clks_us.bits.x6NgwhdSwuGw6oKCwihWwdC_ = 0x50;
    }

    hal_reg_write(HH503_SOC_PHY_WLMAC_CGR, phy_wlmac_cgr.u32);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x20, num_pa_clks_us.u32);
}

osal_void hal_update_mac_freq(hal_to_dmac_device_stru *hal_device, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hal_clk_freq_enum_uint8 hal_clk_freq = HAL_CLK_80M_FREQ;

    if (hal_device->ax_vap_cnt != 0) {
        hal_clk_freq = HAL_CLK_160M_FREQ;
    } else if (((bandwidth >= WLAN_BAND_WIDTH_40PLUS) && (bandwidth <= WLAN_BAND_WIDTH_40MINUS))) {
        hal_clk_freq = HAL_CLK_160M_FREQ;
    } else if (bandwidth == WLAN_BAND_WIDTH_20M) {
        hal_clk_freq = HAL_CLK_80M_FREQ;
    }

    hh503_set_mac_freq(hal_clk_freq);
}
#else
/*****************************************************************************
 功能描述  : 更新mac freq
*****************************************************************************/
osal_void hh503_set_mac_freq(hal_clk_freq_enum_uint8 mac_clk_freq)
{
    osal_u8 index;
    osal_u8 freq_index = 0;
    osal_u16 w_ctl_val = 0x10;
    osal_u32 value;
    /*
    WL0、WL1 MAC的主时钟分频系数，当输入为640M时
    6'h1:1分频
    6'h2:2分频
    …
    6'h15:15分频
    6'h0:64分频
    */
    const osal_u32 mac_freq_reg[4] = {
        HH503_MAC_WLMAC_CTRL_BASE_0x110, HH503_MAC_WLMAC_CTRL_BASE_0x90,
        HH503_MAC_WLMAC_CTRL_BASE_0x94, HH503_MAC_WLMAC_CTRL_BASE_0x110
    };

    if (mac_clk_freq == HAL_CLK_40M_FREQ) {
        /* SOC WL0 MAC的主控频率40M,16分频 */
        w_ctl_val = 0x10;
        freq_index = 0;
    } else if (mac_clk_freq == HAL_CLK_80M_FREQ) {
        /* SOC WL0 MAC的主控频率80M,8分频 */
        w_ctl_val = 0x8;
        freq_index = 1;
    } else if (mac_clk_freq == HAL_CLK_160M_FREQ) {
        /* SOC WL0 MAC的主控频率160M,4分频 */
        w_ctl_val = 0x4;
        freq_index = 0x2;
    } else {
        oam_error_log1(0, OAM_SF_ANY, "{hh503_device_set_mac_freq::wrong clk freq[%d]!!!", mac_clk_freq);
        return;
    }

    value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x20);
    value &= (~HH503_MAC_CFG_NUM_PA_CLKS_ONE_US_MASK);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x20,
        (value | (g_mac_freq_reg_val[freq_index][0] << HH503_MAC_CFG_NUM_PA_CLKS_ONE_US_OFFSET)));

    value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x20);
    value &= (~HH503_MAC_CFG_NUM_PA_CLKS_DECI_US_MASK);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x20, (value | g_mac_freq_reg_val[freq_index][1]));

    for (index = 0; index < HH503_MAC_FREQ_TABLE_LEN - 3; index++) {    /* 7 - 3 = 4 */
        hal_reg_write(mac_freq_reg[index], g_mac_freq_reg_val[freq_index][index + 2]); /* 偏移2元素 */
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
osal_void hh503_get_reg_he_rom_nss(osal_u32 *peer_rom_nss)
{
    osal_u32 reg_val;

    reg_val = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x100);
    *peer_rom_nss = (reg_val & HH503_MAC_ROM_INFO_NSS_MASK) >> HH503_MAC_ROM_INFO_NSS_OFFSET;
}
#endif

/*****************************************************************************
功能描述  : 命令设置HH503_PA_SMPS_CTRL_REG寄存器中帧过滤值
*****************************************************************************/
osal_void hh503_set_rx_filter_reg(osal_u32 rx_filter_command)
{
    osal_u32 rx_filter_val = HH503_MAC_CFG_INIT;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_SDP_IS_INIT);

    if (fhook != OSAL_NULL && ((hmac_sdp_is_init_cb)fhook)() == OAL_TRUE) {
        rx_filter_command  &= ~WLAN_BCMC_MGMT_OTHER_BSS_FLT;
    }

    /* BIP重放攻击过滤 */
    if (rx_filter_command & WLAN_BIP_REPLAY_FAIL_FLT) {
        rx_filter_val |= HH503_MAC_CFG_BIP_REPLAY_FAIL_FLT_EN_MASK;
    } else {
        rx_filter_val &= ~HH503_MAC_CFG_BIP_REPLAY_FAIL_FLT_EN_MASK;
    }
    /* CCMP重放攻击过滤 */
    if (rx_filter_command & WLAN_CCMP_REPLAY_FAIL_FLT) {
        rx_filter_val |= HH503_MAC_CFG_CCMP_REPLAY_FAIL_FLT_EN_MASK;
    } else {
        rx_filter_val &= ~HH503_MAC_CFG_CCMP_REPLAY_FAIL_FLT_EN_MASK;
    }
    /* direct控制帧过滤 */
    if (rx_filter_command & WLAN_OTHER_CTRL_FRAME_FLT) {
        rx_filter_val |= HH503_MAC_CFG_OTHER_CTRL_FRAME_FLT_EN_MASK;
    } else {
        rx_filter_val &= ~HH503_MAC_CFG_OTHER_CTRL_FRAME_FLT_EN_MASK;
    }
    /* 其他BSS网络的组播管理帧过滤 */
    if (rx_filter_command & WLAN_BCMC_MGMT_OTHER_BSS_FLT) {
        rx_filter_val |= HH503_MAC_CFG_BCMC_MGMT_OTHER_BSS_FLT_EN_MASK;
    } else {
        rx_filter_val &= ~HH503_MAC_CFG_BCMC_MGMT_OTHER_BSS_FLT_EN_MASK;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{hh503_set_rx_filter_reg::rx_filter_val is 0x%08x}", rx_filter_val);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x48, rx_filter_val);
}

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/*****************************************************************************
 功能描述  : 设置vap发送WUR BEACON帧的周期，地址，长度，phy_tx_mode，tx_data_rate
*****************************************************************************/
osal_void hh503_vap_send_wur_beacon_pkt(const wur_bcn_reg_param_stru *wur_bcn_param)
{
    unref_param(wur_bcn_param);
}

/*****************************************************************************
 功能描述  : 使能TWBTT的VAP TSF功能, 并且校准出TWBTT的偏移
*****************************************************************************/
osal_void hh503_vap_set_wur_twbtt(osal_u16 wur_offset_twbtt, osal_u8 twbtt_en)
{
    unref_param(wur_offset_twbtt);
    unref_param(twbtt_en);
}

/*****************************************************************************
 功能描述  : 设置vap在TWBTT发送WUR BEACON帧使能，配置tbtt，tsf，速率等
*****************************************************************************/
osal_void hh503_vap_set_wur_beacon_en(osal_u8 wur_bcn_data_rate, osal_u8 wur_bcn_vap_en)
{
    unref_param(wur_bcn_data_rate);
    unref_param(wur_bcn_vap_en);
}

/*****************************************************************************
 功能描述  : 发送WUR BEACON帧时，把帧内容写入指定明文寄存器
*****************************************************************************/
osal_void hh503_write_wur_beacon(osal_u16 id)
{
    unref_param(id);
}

/*****************************************************************************
 功能描述  : 发送WUR FL WAKE UP帧时，把帧内容写入指定明文寄存器
*****************************************************************************/
osal_void hh503_write_wur_fl_wake(const wur_wakeup_reg_param_stru *wur_wakeup_param)
{
    unref_param(wur_wakeup_param);
}

/*****************************************************************************
 功能描述  : 发送WUR SHORT WAKE UP帧时，把帧内容写入指定明文寄存器
*****************************************************************************/
osal_void hh503_write_wur_short_wake(osal_u16 wur_id)
{
    unref_param(wur_id);
}

/*****************************************************************************
 功能描述  : 使能WUR周期
*****************************************************************************/
osal_void hh503_vap_set_wur(const hal_to_dmac_vap_stru *hal_vap, const wur_reg_param_stru *wur_param,
    osal_u8 count)
{
    unref_param(hal_vap);
    unref_param(wur_param);
    unref_param(count);
}
#endif

/*****************************************************************************
 功能描述  : pa_ctrl2寄存器初始化
 目的：减少 hal_initialize_machw()函数代码行
*****************************************************************************/
static osal_void hh503_mac_pa_ctrl2_init(osal_void)
{
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 0;
    pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);

    vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
    vap2_mode_set0.bits.xCt2OPzGZORstOmo_ = 0;
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);

    hal_tsf_pulse_adjust();
}

/*****************************************************************************
 功能描述  : hh503_initialize_machw
*****************************************************************************/
osal_void hal_initialize_machw(osal_void)
{
    u_he_ctrl he;
    u_trig_resp_common1 trig_resp_common1;
    osal_u32 value;
    u_peer_addr_lut_config peer_addr_lut_config;
    u_trig_resp_basic trig_resp_basic;
    u_intra_bss_nav_qtp_ctrl intra_bss_nav_qtp_ctrl;

    hh503_set_mac_freq(HAL_CLK_80M_FREQ);
#ifdef BOARD_ASIC_WIFI
    /* 初始化加密时钟 */
    hal_reg_write(HH503_MAC_WLMAC_CTRL_BASE_0x110,  0x1);
    hal_reg_write(HH503_MAC_WLMAC_CTRL_BASE_0x90,  0x1);
#endif

    hh503_mac_reg_init();

    hh503_mac_pa_ctrl2_init();
    hh503_mac_ce_ctr_init();
    intra_bss_nav_qtp_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14);
    intra_bss_nav_qtp_ctrl.bits.x6Ngw_WCWLwjiLQwhGqwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14, intra_bss_nav_qtp_ctrl.u32);

    /* 控制帧反馈LUT表清空 */
    /* BIT2-3:   0:read 1:Write 2:delete 3:clear all */
    /* BIT0:     Will trigger LUT operation */
    peer_addr_lut_config.u32 = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x8);
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 1;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = 6;  /* 6:0110 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8,  peer_addr_lut_config.u32);

    trig_resp_common1.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x68);
    trig_resp_common1.bits.xCt2ORwl2OwmsGOz44OGs4SOCoROmo_ = 1;
    trig_resp_common1.bits.x6NgwL_BgwL_Cw_WCuwduQwBhCW_LwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x68, trig_resp_common1.u32);

    value = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xF8);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xF8, (value & (~HH503_MAC_TRIG_RESP_TPC_POWERMODE_MASK)));

    /* 响应bqrp时，qosnull帧的htc域的BQR信息由逻辑填写还是寄存器配置：0：逻辑填写； 1：软件寄存器配置 */
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xEC,
        (hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xEC) & (~HH503_MAC_CFG_BQRP_RESP_HTC_FLD_VAL_EN_MASK)));

    /* 以qos null帧响应basic trigger，是否插入+HTC域寄存器使能。0：不插入(默认)；1：插入 */
    value = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x94);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x94, (value | HH503_MAC_CFG_HE_BASICTRIG_RESP_INSERT_HTC_EN_MASK));

    /* 以描述符响应basic trigger帧时，硬件是否将管理帧/数据帧插入htc域。
       0：硬件不插入htc（默认）；1：硬件插入htc */
    value = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x94);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x94, (value | HH503_MAC_CFG_BASIC_RESP_DESC_INSERT_HTC_EN_MASK));

    /* 接收非聚合basic trigger帧，遍历队列的最大个数门限值,设为5 */
    trig_resp_basic.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x94);
    trig_resp_basic.bits.xCt2ORwl2OwmsGOzCOCAORAwm_ = 5; // 5,遍历TID队列的最大个数
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x94, trig_resp_basic.u32);

    /* BSRP回复的qos null是否携带HTC使能 0：不使能 1：使能 */
    he.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x10);
    he.bits.xCt2OAmOyswGOlosmwROARCOmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x10, he.u32);

    /* cfg_trig_resp_ampdu_ba_bypass配1：接收basic + 组播或广播的数据帧且需确认的数据帧时，不响应ba tb ppdu,避免tb的mac err过多 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1CC, 0x10);

    /* bfrp、trs、bqrp trigger回复的qos null是否携带HTC使能 0：不使能 1：使能 */
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xEC,
        (hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xEC) | HH503_MAC_CFG_HE_BQRP_RESP_INSERT_HTC_EN_MASK));
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xA8,
        (hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xA8) | HH503_MAC_CFG_HE_TRS_RESP_INSERT_HTC_EN_MASK));
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xB4,
        (hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xB4) | HH503_MAC_CFG_HE_BFRP_RESP_INSERT_HTC_EN_MASK));
}

osal_u32 hh503_wait_mac_done_timeout(osal_u32 reg_addr, osal_u32 bitmask, osal_u32 reg_busy_value,
    osal_u32 wait_10us_cnt)
{
    osal_u32 delay = 0;
    while ((hal_reg_read(reg_addr) & bitmask) == reg_busy_value) {
        /* 最多等待 (HAL_CE_LUT_UPDATE_TIMEOUT * 10 ) us */
        if (++delay > wait_10us_cnt) {
            oam_warning_log1(0, OAM_SF_ANY, "{hh503_wait_mac_done_timeout::lut done timeout value[%d]!}\r\n",
                hal_reg_read(reg_addr));

            return delay;
        }

        osal_udelay(10); /* 延时10us */
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 获取hal_device指针
*****************************************************************************/
hal_device_stru *hh503_chip_get_device(osal_void)
{
    return &(hal_get_chip_stru()->device);
}

/*****************************************************************************
 功能描述 : 关闭数据/管理/控制帧回响应帧
*****************************************************************************/
osal_void hh503_disable_machw_ack_trans(osal_void)
{
    u_tx_resp_ctrl resp;
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.xCt2O4zRzOwmsGOzCdOmo_ = 0;
    resp.bits.xCt2OI2IROwmsGOzCdOmo_ = 0;
    resp.bits.x6Ngw6L_ow_WCuwG6KwWh_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}

/*****************************************************************************
 功能描述 : 关闭收rts回cts
*****************************************************************************/
osal_void hh503_disable_machw_cts_trans(osal_void)
{
    u_tx_resp_ctrl resp;
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.x6Ngw_WCuw6LCwWh_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}

/*****************************************************************************
 功能描述 : 打开收rts回cts
*****************************************************************************/
osal_void hh503_enable_machw_cts_trans(osal_void)
{
    u_tx_resp_ctrl resp;
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.x6Ngw_WCuw6LCwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}

/*****************************************************************************
 功能描述 : 清除所有TX seq lut表项
*****************************************************************************/
osal_void hh503_initialize_machw_common(osal_void)
{
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x10, (BIT0 | BIT1 | BIT2 | BIT3));
}

/*****************************************************************************
 功能描述  : 根据信道号获取频偏系数， 具体对应公式由phy提供
*****************************************************************************/
osal_void hh503_get_freq_factor(osal_u8 freq_band, osal_u8 channel_num, osal_u32 *freq_factor)
{
    osal_u32 frequency;

    if (freq_band == WLAN_BAND_2G) {
        frequency = (channel_num == 14) ? 2484 : (2407 + channel_num * 5); /* 信道编号14，频率2484、2407，乘以5 */
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
        if (channel_num == 15) { // 15信道
            frequency = 2512; // 15信道对应频率为2512
        }
        if (channel_num == 16) { // 16信道
            frequency = 2384; // 16信道对应频率为2384
        }
#endif
    } else {
        /* japan 184~196, 4.9G 信道编号184、196，频率4000、5000 */
        osal_u32 band_base = (channel_num >= 184 && channel_num <= 196) ? 4000 : 5000;
        frequency = band_base + channel_num * 5; /* 乘以5 */
    }

    /* 计算公式为round(20*1024*128/freq, 0), 考虑到代码实现以及freq大多时候为奇数，公式修改到如下形式 */
    *freq_factor = (5242880 + frequency) / (2 * frequency); /* 20*1024*128*2 结果为 5242880 */
}

/*****************************************************************************
 功能描述 : 清空硬件发送缓冲区FIFO,广播/高优先级/BK/BE/VI/VO队列
*****************************************************************************/
osal_void hh503_clear_hw_fifo(hal_to_dmac_device_stru *hal_device_base)
{
    osal_u32 value;
    value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x24);
    value &= 0xffffff80;
    /* 写1再写0，复位 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x24, value |
        HH503_MAC_CFG_TX_BC_Q_RESET_MASK | HH503_MAC_CFG_TX_HIPRI_Q_RESET_MASK | HH503_MAC_CFG_TX_BK_Q_RESET_MASK |
        HH503_MAC_CFG_TX_BE_Q_RESET_MASK | HH503_MAC_CFG_TX_VI_Q_RESET_MASK | HH503_MAC_CFG_TX_VO_Q_RESET_MASK);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x24, value);

    hal_device_base->clear_fifo_st = OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : 屏蔽指定中断
*****************************************************************************/
osal_void hh503_mask_interrupt(osal_u32 offset)
{
    osal_u32 value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (value | (1U << offset)));
}

/*****************************************************************************
 功能描述  : 读中断状态寄存器
*****************************************************************************/
osal_u32 hh503_get_mac_int_status(osal_void)
{
    return hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x4);
}

/*****************************************************************************
 功能描述  : 清中断状态寄存器
*****************************************************************************/
osal_void hh503_clear_mac_int_status(osal_u32 status)
{
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x54, status);
}
#ifdef _PRE_WLAN_DFR_STAT
/*****************************************************************************
 功能描述  : 清error中断状态寄存器
*****************************************************************************/
osal_void hh503_clear_mac_error_int_status(const hal_error_state_stru *status)
{
    if (status == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hh503_clear_mac_error_int_status::status is NULL.}\r\n");
        return;
    }
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x58, status->error1_val.u32);
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x5C, status->error2_val.u32);
}
#endif

/*****************************************************************************
功能描述  : 设置RX BCN TIMEOUT
*****************************************************************************/
osal_void hh503_set_beacon_timeout(osal_u16 value)
{
    osal_u32 reg_value;
    reg_value = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x5C);
    reg_value &= ~(HH503_MAC_CFG_RX_BCN_TIMEOUT_VAL_MASK);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x5C, reg_value | (value << HH503_MAC_CFG_RX_BCN_TIMEOUT_VAL_OFFSET));
}

/*****************************************************************************
功能描述  : 设置MAC的信道统计时间长度寄存器
*****************************************************************************/
osal_void hh503_get_ch_statics_result(hal_ch_statics_irq_event_stru *ch_statics)
{
    osal_u32 value;
    u_ch_load_stat_period ch_load_stat_period;

    /* 1. 保存信道统计结果 */
    ch_load_stat_period.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x90);
    ch_statics->ch_stats_time_us = ch_load_stat_period.bits.x6Ngw6QwoiGFwCLGLwuW_BiF_;

    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x88) & HH503_MAC_PRIMARY_20M_IDLE_CNT_MASK;
    ch_statics->pri20_free_time_us = value;

    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x8C) & HH503_MAC_PRIMARY_40M_IDLE_CNT_MASK;
    ch_statics->pri40_free_time_us = value;

    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x90) & HH503_MAC_RPT_SEC_20M_IDLE_CNT_MASK;
    ch_statics->sec20_free_time_us = value;

    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x98) & HH503_MAC_RX_PROGRESS_CNT_MASK;
    ch_statics->ch_rx_time_us = value;

    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x94) & HH503_MAC_TX_PROGRESS_CNT_MASK;
    ch_statics->ch_tx_time_us = value;
}

#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
函 数 名  : hh503_enable_radar_det
功能描述  : 设置硬件RADAR检测的使能位状态
*****************************************************************************/
osal_void hh503_enable_radar_det(osal_u8 enable)
{
    hh503_enable_radar_det_cb(enable);
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
功能描述  : 设置HH503_PA_SMPS_CTRL_REG寄存器中SMPS的模式
*****************************************************************************/
osal_void hh503_set_smps_mode(osal_u8 smps_mode)
{
    unref_param(smps_mode);
}
#endif

/*****************************************************************************
 函 数 名  : hh503_vap_get_gtk_rx_lut_idx
 功能描述  : hal vap获取组播秘钥对应lut idx
*****************************************************************************/
osal_void hh503_vap_get_gtk_rx_lut_idx(const hal_to_dmac_vap_stru *hal_vap, osal_u8 *lut_idx)
{
    /* 调用函数出保证了hal vap指针非空，不需要额外判断空指针 */
    *lut_idx = hal_vap->vap_id;
}

/*****************************************************************************
 功能描述  : 撤销对指定中断的屏蔽
*****************************************************************************/
osal_void hh503_unmask_interrupt(osal_u32 offset)
{
    osal_u32 value = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x54);

    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x54, (value | (1U << offset)));

    value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (value & ~(1U << offset)));
}

/*****************************************************************************
功能描述  : 配置slottime类型  0:short 1:long
*****************************************************************************/
osal_void hh503_cfg_slottime_type(osal_u32 slottime_type)
{
#define LONG_SLOT_TIME       20
#define SHORT_SLOT_TIME      9
#define SLOTTIME_TYPE_LONG   1
#define SLOTTIME_TYPE_SHORT  0
    u_slot_sifs_time sifs;
    osal_u8 time = LONG_SLOT_TIME;
    if (slottime_type == SLOTTIME_TYPE_SHORT) {
        time = SHORT_SLOT_TIME;
    }
    sifs.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x6C);
        // ctrl0+0x88[29]
    sifs.bits.xCt2OsxvRORlIm_ = time;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x6C, sifs.u32);
}

/*****************************************************************************
 功能描述  : PA和PHY是否使能
*****************************************************************************/
oal_bool_enum_uint8 hh503_is_machw_enabled(osal_void)
{
    u_pa_control1 pa_ctrl;
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    return pa_ctrl.bits.x6NgwTowCLG_LduwWh_;
}

/*****************************************************************************
 功能描述 : 关闭mac和phy的PA
*****************************************************************************/
osal_void hh503_disable_machw_phy_and_pa(osal_void)
{
    u_pa_control1 pa_ctrl;

#ifdef _PRE_WLAN_FEATURE_CSI
    /* 如果CSI是打开状态，则不关闭PA */
    if (hal_csi_is_open()) {
        return;
    }
#endif

    /* dis PA之前需要先关闭BT */
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.xCt2OCvm8OzyvwROmo_ = 0;
    pa_ctrl.bits.x6NgwTowCLG_LduwWh_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}

/*****************************************************************************
 功能描述 : 打开mac和phy的PA
*****************************************************************************/
osal_void hh503_enable_machw_phy_and_pa(const hal_to_dmac_device_stru *hal_device)
{
    u_pa_control1 pa_ctrl;

    /* 查询芯片温度过高时，延迟打开MAC PA */
    if (hal_device->is_temp_protect_pa_off) {
        return;
    }

    /* en PA之后再打开BT */
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.xCt2OCvm8OzyvwROmo_ = 1;
    pa_ctrl.bits.x6NgwTowCLG_LduwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}

/*****************************************************************************
 功能描述 : 打开或关闭mac和phy的PA
*****************************************************************************/
osal_void hh503_recover_machw_phy_and_pa(hal_to_dmac_device_stru *hal_device_base)
{
    if (hal_device_base->is_mac_pa_enabled) {
        hh503_enable_machw_phy_and_pa(hal_device_base);
    } else {
        hh503_disable_machw_phy_and_pa();
    }
}

/*****************************************************************************
 功能描述 : 打开或关闭mac和phy的PA
*****************************************************************************/
osal_u8 hal_save_machw_phy_pa_status(hal_to_dmac_device_stru *hal_device_base)
{
    u_pa_control1 pa_ctrl;

    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    hal_device_base->is_mac_pa_enabled = pa_ctrl.bits.x6NgwTowCLG_LduwWh_;

    return hal_device_base->is_mac_pa_enabled;
}

/*****************************************************************************
 函 数 名  : hh503_show_irq_info
 功能描述  : 上报或者清零各类中断次数信息
*****************************************************************************/
osal_void hh503_show_irq_info(hal_to_dmac_device_stru *hal_device, osal_u8 param)
{
    hal_device_stru *device = (hal_device_stru *)hal_device;
    osal_u16 irq_info_len = (osal_u16)sizeof(hal_irq_device_stats_stru);

    if (param == OSAL_FALSE) {
        (osal_void)memset_s(&device->irq_stats, irq_info_len, 0, irq_info_len);
    }

    oam_info_log_alter(0, OAM_SF_P2P,
        "{hh503_show_irq_info::ct_end[%d], absent_start[%d], noa_absent_end[%d], absent_start[%d], absent_end[%d]}",
        0x5, device->irq_stats.p2p_stat.ct_window_end_count,
        device->irq_stats.p2p_stat.noa_absent_start_count,
        device->irq_stats.p2p_stat.noa_absent_end_count,
        device->irq_stats.p2p_stat.absent_start_count, device->irq_stats.p2p_stat.absent_end_count);
}

/*****************************************************************************
 功能描述  : 清除中断统计信息
*****************************************************************************/
osal_void hh503_clear_irq_stat(hal_to_dmac_device_stru *hal_device_base)
{
    hal_device_stru *hal_device = (hal_device_stru *)hal_device_base;
    (osal_void)memset_s(&(hal_device->irq_stats.p2p_stat), sizeof(hal_p2p_ps_stat_stru), 0x0,
        sizeof(hal_p2p_ps_stat_stru));
}

/*****************************************************************************
功能描述  : 设置常发退避时间
*****************************************************************************/
osal_void hh503_set_mac_backoff_delay(osal_u32 back_off_time)
{
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0xA8, 0, 16, back_off_time); // 0: offset 16: bits
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0xA8, 16, 16, back_off_time);
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0xAC, 0, 16, back_off_time); // 0: offset 16: bits
    hal_reg_write_bits(HH503_MAC_CTRL0_BANK_BASE_0xAC, 16, 16, back_off_time);
}

/*****************************************************************************
 功能描述  : 设置11ax使能
*****************************************************************************/
osal_void hh503_set_dev_support_11ax(hal_to_dmac_device_stru *hal_device, osal_u8 reg_value)
{
    u_he_ctrl he;
    if (hal_device == OAL_PTR_NULL) {
        return;
    }

    hal_device->current_11ax_working = reg_value;

    /* HE CTRL */
    he.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x10);
    he.bits.x6Ngw_Pw00GPwWh_ = reg_value;
    /* 如果需要支持反馈BSR则必须支持HTC域(协议10.9),当前先禁止 */
    he.bits.x6NgwQWwL_BgwBhCW_LwQL6wWh_ = 1;
    he.bits.xCt2OAmOyswGOlosmwROARCOmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x10, he.u32);
    oam_warning_log0(0, OAM_SF_RX, "{hh503_set_dev_support_11ax:: hal dev enable.}");
}
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
/*****************************************************************************
函 数 名  : hh503_rf_test_disable_al_tx
功能描述  : 禁用常发
*****************************************************************************/
osal_void hh503_rf_test_disable_al_tx(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 vap0_tsf_value, mac_bypass_value, int_mask;
    osal_u32 reg_value;
    u_pa_control1 pa_ctrl;
    u_pa_control2 pa_ctrl2;
    if (hal_device == OSAL_NULL) {
        return;
    }

    /* 停止常发时先suspend，将TX描述符的NEXT清空后，使能中断，再resume */
    hh503_set_machw_tx_suspend();

    /* mac复位 */
    hh503_reset_phy_machw(HAL_RESET_HW_TYPE_MAC, HAL_RESET_MAC_LOGIC, OSAL_FALSE, OSAL_FALSE);

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    hal_tsf_pulse_adjust();

    reg_value = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xC);
    reg_value &= ~(0x1ff);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xC, reg_value);

    /* 使能beacon */
    vap0_tsf_value = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xC);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xC, (vap0_tsf_value & (~HH503_MAC_CFG_VAP0_TX_BCN_SUSPEND_MASK)));

    /* 打开所有TBTT中断 */
    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & (~HH503_MAC_RPT_INTR_VAP0_TBTT_MASK)));

    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & (~HH503_MAC_RPT_INTR_VAP1_TBTT_MASK)));

    /* NAV open */
    mac_bypass_value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x138);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x138, (mac_bypass_value & (~HH503_MAC_CFG_NAV_BYPASS_EN_MASK)));

    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    /* 允许硬件修改软件配置的MPDU参数 */
    pa_ctrl.bits.x6NgwFihwLwLid6QwSuFd_ = 0;

    /* lifetime check enable */
    pa_ctrl.bits.x6NgwoBNWLBSWw6QKwWh_ = 1;
    /* 使能edca模式下cancel txop时发送cf end帧 */
    pa_ctrl.bits.x6NgwLPw6NwWhFwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
    /* 使能发送完成中断 */
    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & ~(HH503_MAC_RPT_INTR_TX_COMPLETE_MASK)));

    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & ~(HH503_MAC_RPT_INTR_ERROR_MASK)));

    /* 使能管理帧接收完成中断 */
    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & ~(HH503_MAC_RPT_INTR_RX_HIPRI_COMPLETE_MASK)));

    int_mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x54, (int_mask & ~(HH503_MAC_RPT_INTR_RX_NORM_COMPLETE_MASK)));

    hal_set_machw_tx_resume();
}
#endif

/*****************************************************************************
函 数 名  : hh503_get_dieid
功能描述  : 获取dieid
*****************************************************************************/
osal_void hh503_get_dieid(hal_to_dmac_device_stru *hal_device, osal_u32 *dieid, osal_u32 *length)
{
    unref_param(hal_device);
    unref_param(dieid);
    unref_param(length);
    return;
}

/*****************************************************************************
 功能描述  : 获取bssid的低32位和高16位，用于设置bssid寄存器
*****************************************************************************/
osal_void hh503_get_bssid_lsb_msb(const osal_u8 *bssid, osal_u32 *bssid_lsb, osal_u16 *bssid_msb)
{
    osal_u32 bssid_lsb_tmp;
    osal_u16 bssid_msb_tmp;

    bssid_msb_tmp = bssid[0];
    bssid_msb_tmp = (bssid_msb_tmp << 0x8) | bssid[0x1]; /* 左移8位 */

    bssid_lsb_tmp = bssid[0x2];                          /* bssid第2个字节 */
    bssid_lsb_tmp = (bssid_lsb_tmp << 0x8) | bssid[0x3]; /* 左移8位，bssid第3个字节 */
    bssid_lsb_tmp = (bssid_lsb_tmp << 0x8) | bssid[0x4]; /* 左移8位，bssid第4个字节 */
    bssid_lsb_tmp = (bssid_lsb_tmp << 0x8) | bssid[0x5]; /* 左移8位，bssid第5个字节 */

    *bssid_lsb = bssid_lsb_tmp;
    *bssid_msb = bssid_msb_tmp;
}

/*****************************************************************************
 功能描述  : 设置multi bssid相关寄存器
*****************************************************************************/
osal_void hal_vap_set_multi_bssid(osal_u8 *trans_bssid, osal_u8 maxbssid_indicator)
{
    osal_u32 reg;
    osal_u32 bssid_lsb = 0; /* BSSID低32位 */
    osal_u16 bssid_msb = 0; /* BSSID高16位 */
    u_pa_control2 pa_ctrl2;

    unref_param(maxbssid_indicator);

    hh503_get_bssid_lsb_msb(trans_bssid, &bssid_lsb, &bssid_msb);

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    /* 默认取使能multi bssid的check bssid功能 */
    pa_ctrl2.bits.x6NgwSdoLBwjCCBFw6QKwjCCBFwWh_ = 0;
    /* VAP0支持multi bssid使能 */
    pa_ctrl2.bits.xCt2OISxRlOyssl4Omo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    reg = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x60);
    reg = (reg & ~(HH503_MAC_CFG_VAP0_TRANS_BSSID_H_MASK)) |
        (bssid_msb << HH503_MAC_CFG_VAP0_TRANS_BSSID_H_OFFSET);    /* transmitted bssid的高16bit */
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x60, reg);

    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x64, bssid_lsb);    /* transmitted bssid的低3216bit */

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    /* 默认关闭控制帧multi bssid检测 */
    pa_ctrl2.bits.xCt2OCRwxOISxRlOyssl4Omo_ = 0;
    /* 即收到NFRP Trigger时,不受ctrl_multi_bssid_en控制,直接进行判断 */
    pa_ctrl2.bits.xCt2OotwGOISxRlOyssl4Omo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
}

/********************************************************
功能描述  : 设置11ax ROM解析使能
*******************************************************/
osal_void hh503_set_he_rom_en(oal_bool_enum_uint8 rx_om)
{
    osal_u32 value_htc_type;
    u_he_ctrl he;

    value_htc_type = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x98);
    he.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x10);
    he.bits.x6Ngw_PwQWw_iSwWh_ = rx_om;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x10, he.u32);

    if (rx_om == OSAL_TRUE) {
        value_htc_type |= HH503_MAC_CFG_RX_HE_ROM_HTC_TYPE_EN_MASK;
    } else {
        value_htc_type &= (~HH503_MAC_CFG_RX_HE_ROM_HTC_TYPE_EN_MASK);
    }

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x98, value_htc_type);
}
#ifdef _PRE_WLAN_FEATURE_BSRP
osal_void hh503_set_queue_size_in_ac(osal_u32 queue_size, osal_u8 vap_id, osal_u16 ac_order)
{
    osal_u32 ac_queue_tid;
    unref_param(vap_id);

    if (vap_id >= BSRP_VAP_MAX) {
        oam_error_log1(0, OAM_SF_CFG, "{hh503_set_ac_queue_size failed, vap = %d}\r\n", vap_id);
        return;
    }

    ac_queue_tid = ac_order << BIT_OFFSET_16;
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x108, queue_size);
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x110, ac_queue_tid);
}

/*****************************************************************************
 功能描述  : 设置ndp的反馈矩阵状态
*****************************************************************************/
osal_void hh503_set_ndp_status(osal_u8 status)
{
    osal_u32 val;

    val = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0xDC);
    if (status == 0) {
        val &= (~BIT16);
    } else {
        val |= BIT16;
    }
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0xDC, val);
    return;
}
#endif

/*****************************************************************************
 功能描述  : 挂起PA(Protocol accelerator)
*****************************************************************************/
osal_void hh503_set_machw_tx_suspend(osal_void)
{
    u_pa_control1 pa_ctrl;
    if (hh503_is_machw_enabled() == OSAL_FALSE) {
        return;
    }

    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.x6NgwLPwCdCuWhF_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);

    if (hh503_wait_mac_done_timeout(HH503_MAC_CTRL0_BANK_BASE_0x0, HH503_MAC_CFG_RTS_RATE_CALC_AT_BASIC_RATE_MASK, 0,
        HH503_TX_SUSPEND_UPDATE_TIMEOUT) != OAL_SUCC) {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if (hal_btcoex_btble_status()->bt_status.bt_status.bt_on == OSAL_TRUE) {
            oam_warning_log0(0, OAM_SF_TX,
                "{hh503_set_machw_tx_suspend::wait suspend status failed in btcoex scene.}");
        } else
#endif

        oam_error_log0(0, OAM_SF_TX, "{hh503_set_machw_tx_suspend::wait suspend status failed.}");
    }
}

/*****************************************************************************
 功能描述  : 恢复PA(Protocol accelerator)
*****************************************************************************/
osal_void hal_set_machw_tx_resume(osal_void)
{
    u_pa_control1 pa_ctrl;
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.x6NgwLPwCdCuWhF_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}

/*****************************************************************************
 功能描述  : 设置非qos帧seq num
*****************************************************************************/
osal_void hh503_set_tx_nonqos_seq_num(osal_u8 vap_index, osal_u32 val_write)
{
    osal_u32 reg_val;
    osal_u32 temp = 0;

    reg_val = (vap_index & 0x7) << 8; /* vap_index:8-10bit */
    reg_val &= (~BIT7); /* BIT7 non qos */
    reg_val |= BIT2; /* BIT2:写操作 */
    reg_val |= BIT0; /* BIT0:操作使能 */
    reg_val |= BIT1; /* TX */

    while ((hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x10) & BIT0)) {
        osal_udelay(10); /* 延时10us */

        /* 等待20ms */
        if (++temp > HH503_PA_LUT_UPDATE_TIMEOUT) {
            oam_error_log0(0, OAM_SF_CFG, "{hh503_set_tx_nonqos_seq_num::failed.}\r\n");
            return;
        }
    }

    /* 写数据寄存器 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0xC, val_write);
    /* 写操作命令 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x10, reg_val);
}

/*****************************************************************************
 功能描述  : 设置qos帧seq num
*****************************************************************************/
osal_void hh503_set_tx_qos_seq_num(osal_u8 lut_index, osal_u8 tid, osal_u32 val_write)
{
    osal_u32 reg_val;
    osal_u32 temp = 0;

    reg_val = (lut_index & 0x7) << 8; /* lut_index:8-10bit */
    reg_val |= (tid & 0x7) << 4; /* tid:4-6bit */
    reg_val |= BIT7; /* BIT7: qos */
    reg_val |= BIT2; /* BIT2:写操作 */
    reg_val |= BIT0; /* BIT0:操作使能 */
    reg_val |= BIT1; /* TX */
    while ((hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x10) & BIT0)) {
        osal_udelay(10); /* 延时10us */

        /* 等待20ms */
        if (++temp > HH503_PA_LUT_UPDATE_TIMEOUT) {
            oam_error_log0(0, OAM_SF_CFG, "{hh503_set_tx_qos_seq_num::failed.}\r\n");
            return;
        }
    }

    /* 写数据寄存器 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0xC, val_write);
    /* 写操作命令 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x10, reg_val);
}

/*****************************************************************************
 功能描述  : 填写sequence num
*****************************************************************************/
osal_void hh503_set_tx_sequence_num(machw_tx_sequence_stru tx_seq, osal_u32 val_write, osal_u8 vap_index)
{
    if (tx_seq.qos_flag) {
        hh503_set_tx_qos_seq_num(tx_seq.lut_index, tx_seq.tid, val_write);
    } else {
        hh503_set_tx_nonqos_seq_num(vap_index, val_write);
    }
}

/*****************************************************************************
 功能描述 : 打开数据/管理/控制帧回响应帧
*****************************************************************************/
osal_void hh503_enable_machw_ack_trans(osal_void)
{
    u_tx_resp_ctrl resp;
    resp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1C);
    resp.bits.xCt2O4zRzOwmsGOzCdOmo_ = 1;
    resp.bits.xCt2OI2IROwmsGOzCdOmo_ = 1;
    resp.bits.x6Ngw6L_ow_WCuwG6KwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1C, resp.u32);
}

/*****************************************************************************
 功能描述  : 写eifs_time 寄存器
*****************************************************************************/
osal_void hh503_config_eifs_time(wlan_protocol_enum_uint8 protocol)
{
    unref_param(protocol);
}

/*****************************************************************************
功能描述  : 填写sta bssid寄存器
*****************************************************************************/
osal_void hh503_set_sta_bssid(const hal_to_dmac_vap_stru *hal_vap, const osal_u8 *bssid)
{
    osal_u16 bssid_msb; /* MAC地址高16位 */
    osal_u32 bssid_lsb; /* MAC地址低32位 */
    osal_u32 value;
    osal_u8 vap_id;
    osal_u32 bssid_h_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x4, HH503_MAC_CTRL2_BANK_BASE_0x2C, HH503_MAC_CTRL2_BANK_BASE_0x124
    };

    osal_u32 bssid_l_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x8, HH503_MAC_CTRL2_BANK_BASE_0x30, HH503_MAC_CTRL2_BANK_BASE_0x128
    };

    if (hal_vap == OSAL_NULL) {
        return;
    }

    vap_id = hal_vap->vap_id;
    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_set_sta_bssid::vap mode [%d]!= WLAN_VAP_MODE_BSS_STA,or hal vap id=%d >= max vap=%d]}",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    bssid_msb = bssid[0];
    bssid_msb = (bssid_msb << 8) | bssid[1]; /* 左移8位 */

    bssid_lsb = bssid[2]; /* bssid第2个字节 */
    bssid_lsb = (bssid_lsb << 8) | bssid[3]; /* 左移8位，bssid第3个字节 */
    bssid_lsb = (bssid_lsb << 8) | bssid[4]; /* 左移8位，bssid第4个字节 */
    bssid_lsb = (bssid_lsb << 8) | bssid[5]; /* 左移8位，bssid第5个字节 */

    value = hal_reg_read(bssid_h_reg[vap_id]);
    value &= ~HH503_MAC_CFG_VAP0_BSSID_H_MASK;
    hal_reg_write(bssid_h_reg[vap_id], value | bssid_msb);
    hal_reg_write(bssid_l_reg[vap_id], bssid_lsb);
}

/*****************************************************************************
功能描述  : 不使能beacon filter
*****************************************************************************/
osal_void hh503_disable_beacon_filter(osal_void)
{
    osal_u32 value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x48);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x48, (value & ~BIT5));
}

/*****************************************************************************
功能描述  : 使能non frame filter
*****************************************************************************/
osal_void hh503_enable_non_frame_filter(osal_void)
{
    osal_u32 value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x48);

    /* 将其他BSS的广播帧过滤掉 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x48,
        (value | HH503_MAC_CFG_BCMC_DATA_OTHER_BSS_FLT_EN_MASK | HH503_MAC_CFG_BCMC_MGMT_OTHER_BSS_FLT_EN_MASK));
}
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
/*****************************************************************************
 功能描述  : 低功耗不下电唤醒后清除rx中断,防止rx中断里保留上一次睡前的值造成rx new
*****************************************************************************/
osal_void hh503_psm_clear_mac_rx_isr(osal_void)
{
    osal_u32 state;
    osal_u32 mask;
    osal_u32 clear_rx_mask;

    /* Clear rx isr */
    clear_rx_mask = HH503_MAC_RPT_INTR_RX_NORM_COMPLETE_MASK | HH503_MAC_RPT_INTR_RX_HIPRI_COMPLETE_MASK;

    do {
        /* 处理MAC中断 */
        state = hh503_get_mac_int_status(); /* 读取MAC中断状态寄存器 */
        mask = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x54);
        state &= ~mask;
        state &= clear_rx_mask;
        /* 清中断状态 */
        hh503_clear_mac_int_status(state);
    } while (state != 0);
}
#endif
/*****************************************************************************
函 数 名  : hh503_get_hw_addr
功能描述  : 从eeprom或flash获取硬件MAC地址
*****************************************************************************/
osal_void hh503_get_hw_addr(osal_u8 *addr)
{
    addr[0] = 0x06; /* 第0个数组元素 */
    addr[1] = 0x05; /* 第1个数组元素 */
    addr[2] = 0x04; /* 第2个数组元素 */
    addr[3] = 0x03; /* 第3个数组元素 */
    addr[4] = 0x02; /* 第4个数组元素 */
    addr[5] = 0x01; /* 第5个数组元素 */
}

/*****************************************************************************
函 数 名  : hh503_set_mac_aid
功能描述  : 设置mac  aid寄存器
*****************************************************************************/
osal_void hh503_set_mac_aid(const hal_to_dmac_vap_stru *hal_vap, osal_u16 aid)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 regval;
    osal_u32 vap_aid_val_reg[HAL_MAX_VAP_NUM] = {
        HH503_MAC_CTRL2_BANK_BASE_0x24, HH503_MAC_CTRL2_BANK_BASE_0x4C, HH503_MAC_CTRL2_BANK_BASE_0x144
    };

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_set_mac_aid::hal vap id=%d > max vap=%d]}", vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    regval = hal_reg_read(vap_aid_val_reg[vap_id]) & (~HH503_MAC_CFG_VAP0_AID_VAL_MASK);
    hal_reg_write(vap_aid_val_reg[vap_id], regval | ((osal_u32)aid << HH503_MAC_CFG_VAP0_AID_VAL_OFFSET));
}

#ifdef _PRE_WLAN_DFT_STAT
#define STRING_MODULE_MAX_SIZE 4
#define STRING_DESC_MAX_SIZE 24

typedef struct {
    osal_uchar module[STRING_MODULE_MAX_SIZE];
    osal_uchar desc[STRING_DESC_MAX_SIZE];
    osal_u32   reg_addr;
} stats_reg_stru;

static osal_void hal_dump_reg(osal_u32 base, osal_u32 size)
{
    osal_u32 addr;

    wifi_printf("\r\n [0x%x : 0x%x]\r\n", base, base + size - 0x4);
    for (addr = base; addr < base + size; addr += 0x4) {
        wifi_printf("addr:0x%x offset:0x%x value:0x%x\r\n", addr, addr - base, hal_reg_read(addr));
    }
}

static osal_void hal_dump_reg_table(stats_reg_stru *reg_table, const osal_u8 table_size)
{
    osal_u8 index;

    wifi_printf("-----STATS DUMP START-----\r\n");
    for (index = 0; index < table_size; index++) {
        wifi_printf("[%s][%s]: 0x%x \r\n", reg_table[index].module, reg_table[index].desc,
            hal_reg_read(reg_table[index].reg_addr));
    }
    wifi_printf("-----STATS DUMP END-----\r\n");
}

osal_void hal_dump_phy_reg(osal_void)
{
    wifi_printf("\r\n -----PHY REG DUMP START-----\r\n");
    hal_dump_reg(HH503_PHY_BANK1_BASE, HH503_PHY_BANK1_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY_BANK2_BASE, HH503_PHY_BANK2_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY_BANK3_BASE, HH503_PHY_BANK3_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY_BANK4_BASE, HH503_PHY_BANK4_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY_BANK5_BASE, HH503_PHY_BANK5_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY_BANK6_BASE, HH503_PHY_BANK6_OFFEST_BUTT);
    hal_dump_reg(HH503_PHY0_CTL_BASE, HH503_PHY0_CTL_OFFEST_BUTT);
    wifi_printf("\r\n -----PHY REG DUMP END-----\r\n");
    return;
}

osal_void hal_dump_mac_reg(osal_void)
{
    wifi_printf("\r\n -----MAC REG DUMP START-----\r\n");
    hal_dump_reg(HH503_MAC_CTRL0_BANK_BASE, HH503_MAC_CTRL0_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_CTRL1_BANK_BASE, HH503_MAC_CTRL1_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_CTRL2_BANK_BASE, HH503_MAC_CTRL2_OFFEST_BUTT);

    hal_dump_reg(HH503_MAC_VLD_BANK_BASE, HH503_MAC_VLD_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_RD0_BANK_BASE, HH503_MAC_RD0_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_RD1_BANK_BASE, HH503_MAC_RD1_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_LUT0_BANK_BASE, HH503_MAC_LUT0_OFFEST_BUTT);
    hal_dump_reg(HH503_MAC_WLMAC_CTRL_BASE, HH503_MAC_WLMAC_CTRL_OFFEST_BUTT);
    wifi_printf("\r\n -----MAC REG DUMP END-----\r\n");
    return;
}

osal_void hal_dump_count(osal_void)
{
    stats_reg_stru reg_table[] = {
        /* MAC Tx stats */
        {"MAC", "TX_HI_PRI_MPDU_CNT     ", HH503_MAC_RD1_BANK_BASE_0x24},
        {"MAC", "TX_NORMAL_PRI_MPDU_CNT ", HH503_MAC_RD1_BANK_BASE_0x28},
        {"MAC", "TX_BCN_COUNT           ", HH503_MAC_RD1_BANK_BASE_0x30},
        {"MAC", "TX_AMPDU_RETRY_CNT     ", HH503_MAC_RD1_BANK_BASE_0x6C},
        {"MAC", "TX_HI_PRI_RETRY_CNT    ", HH503_MAC_RD1_BANK_BASE_0x34},
        {"MAC", "TX_INTR_CNT            ", HH503_MAC_RD1_BANK_BASE_0xA8},
        /* MAC Rx stats */
        {"MAC", "RX_PASSED_MPDU_CNT     ", HH503_MAC_RD1_BANK_BASE_0x38},
        {"MAC", "RX_FAILED_MPDU_CNT     ", HH503_MAC_RD1_BANK_BASE_0x140},
        {"MAC", "RX_BCN_CNT             ", HH503_MAC_RD1_BANK_BASE_0x50},
        {"MAC", "RX_FILTERED_CNT        ", HH503_MAC_RD1_BANK_BASE_0x5C},
        {"MAC", "RX_INTR_CNT            ", HH503_MAC_RD1_BANK_BASE_0xAC},
        /* PHY Rx Stats */
        {"PHY", "RX_11B_OK_CNT          ", HH503_PHY_BANK1_BASE_0x1DC},
        {"PHY", "RX_11N_OK_CNT          ", HH503_PHY_BANK1_BASE_0x1E0},
        {"PHY", "RX_11AX_OK_CNT         ", HH503_PHY_BANK1_BASE_0x1E4},
        {"PHY", "RX_11G_OK_CNT          ", HH503_PHY_BANK1_BASE_0x1E8},
        {"PHY", "RX_11B_ERR_CNT         ", HH503_PHY_BANK1_BASE_0x1EC},
        {"PHY", "RX_11N_ERR_CNT         ", HH503_PHY_BANK1_BASE_0x1F0},
        {"PHY", "RX_11AX_ERR_CNT        ", HH503_PHY_BANK1_BASE_0x1F4},
        {"PHY", "RX_11G_ERR_CNT         ", HH503_PHY_BANK1_BASE_0x1F8}
    };

    hal_dump_reg_table(reg_table, sizeof(reg_table) / sizeof(stats_reg_stru));
}

static osal_void hal_dump_cfg(osal_void)
{
    stats_reg_stru reg_table[] = {
        /* MAC Cfg stats */
        {"MAC", "PA_CONTROL1            ", HH503_MAC_CTRL0_BANK_BASE_0x0},
        {"MAC", "PA_CONTROL2            ", HH503_MAC_CTRL0_BANK_BASE_0x4},
        {"MAC", "CE_CTRL                ", HH503_MAC_CTRL0_BANK_BASE_0x8},
        {"MAC", "HT_VHT_CTRL            ", HH503_MAC_CTRL0_BANK_BASE_0xC},
        {"MAC", "HE_CTRL                ", HH503_MAC_CTRL0_BANK_BASE_0x10},
        {"MAC", "AMPDU_CTRL             ", HH503_MAC_CTRL0_BANK_BASE_0x18},
        {"MAC", "HIBUFADDR_STATUS       ", HH503_MAC_RD0_BANK_BASE_0x10},
        {"MAC", "NORMALADDR_STATUS      ", HH503_MAC_RD0_BANK_BASE_0x14},
        {"MAC", "SMALLADDR_STATUS       ", HH503_MAC_RD0_BANK_BASE_0x18},
        {"MAC", "BE_ADDRESS_STATUS      ", HH503_MAC_RD0_BANK_BASE_0x34},
        {"MAC", "BK_ADDRESS_STATUS      ", HH503_MAC_RD0_BANK_BASE_0x38},
        {"MAC", "VI_ADDRESS_STATUS      ", HH503_MAC_RD0_BANK_BASE_0x3C},
        {"MAC", "VI_ADDRESS_STATUS      ", HH503_MAC_RD0_BANK_BASE_0x40},
        {"MAC", "HI_PRI_ADDRESS_STATUS  ", HH503_MAC_RD0_BANK_BASE_0x44},
        {"MAC", "TX_Q_STATUS            ", HH503_MAC_RD0_BANK_BASE_0x50},
        {"MAC", "INTERRUPT_MASK         ", HH503_MAC_CTRL0_BANK_BASE_0x54},
        {"MAC", "P2P_CTRL1              ", HH503_MAC_CTRL1_BANK_BASE_0x18},
    };

    hal_dump_reg_table(reg_table, sizeof(reg_table) / sizeof(stats_reg_stru));
}

osal_void hal_dump_stats(osal_void)
{
    hal_dump_cfg();
    hal_dump_count();
}
#endif

/*****************************************************************************
 函 数 名  : hh503_set_bcn_timeout_multi_q_enable
 功能描述  : beacon帧发送超时是否需要继续发送组播队列
*****************************************************************************/
osal_void hh503_set_bcn_timeout_multi_q_enable(const hal_to_dmac_vap_stru *hal_vap, osal_u8 enable)
{
    osal_u32 addr[] = {
        (osal_u32)HH503_MAC_CTRL2_BANK_BASE_0xC,
        (osal_u32)HH503_MAC_CTRL2_BANK_BASE_0x34,
        (osal_u32)HH503_MAC_CTRL2_BANK_BASE_0x12C
    };

    osal_u32 value[] = {
        (osal_u32)HH503_MAC_CFG_VAP0_BCN_FAIL_TX_BC_Q_EN_MASK,
        (osal_u32)HH503_MAC_CFG_VAP1_BCN_FAIL_TX_BC_Q_EN_MASK,
        (osal_u32)HH503_MAC_CFG_VAP2_BCN_FAIL_TX_BC_Q_EN_MASK
    };

    osal_u32 regval = hal_reg_read(addr[hal_vap->vap_id]);
    if (enable == OSAL_TRUE) {
        regval |= value[hal_vap->vap_id];
    } else {
        regval &= ~value[hal_vap->vap_id];
    }
    hal_reg_write(addr[hal_vap->vap_id], regval);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
