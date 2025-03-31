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
#include "hal_rf.h"
#include "hal_soc.h"
#include "hal_mac.h"
#include "hal_power.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "oal_ext_if.h"
#include "hal_chip.h"
#include "hal_mac_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_PHY_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#define HMAC_DFT_PHY_STAT_NODE_0_BASE 0  /* phy统计节点0的基准bit，寄存器PHY_STA_01_EN的bit0 */
#define HMAC_DFT_PHY_STAT_NODE_1_BASE 16 /* phy统计节点1的基准bit，寄存器PHY_STA_01_EN的bit16 */
#define HMAC_DFT_PHY_STAT_NODE_2_BASE 0  /* phy统计节点2的基准bit，寄存器PHY_STA_23_EN的bit0 */
#define HMAC_DFT_PHY_STAT_NODE_3_BASE 16 /* phy统计节点3的基准bit，寄存器PHY_STA_23_EN的bit16 */
#endif

#define HH503_DATARATES_PHY_80211G_NUM      12
#define HH503_PHY_RX_SNR_REF_NUMS           13           /* SNR的参考个数 */
#define HH503_PHY_RX_SNR_EXP_MULTIP         12           /* SNR扩大倍数,结果扩大两倍 */

const mac_data_rate_stru g_hh503_rates_11g[HH503_DATARATES_PHY_80211G_NUM] = {
    { 0x02, 0x00, 1, { 0 } },  /* 1 Mbps   */
    { 0x04, 0x01, 2, { 0 } },  /* 2 Mbps   */
    { 0x0B, 0x02, 5, { 0 } },  /* 5.5 Mbps */
    { 0x0C, 0x0B, 6, { 0 } },  /* 6 Mbps   */
    { 0x12, 0x0F, 9, { 0 } },  /* 9 Mbps   */
    { 0x16, 0x03, 11, { 0 } }, /* 11 Mbps  */
    { 0x18, 0x0A, 12, { 0 } }, /* 12 Mbps  */
    { 0x24, 0x0E, 18, { 0 } }, /* 18 Mbps  */
    { 0x30, 0x09, 24, { 0 } }, /* 24 Mbps  */
    { 0x48, 0x0D, 36, { 0 } }, /* 36 Mbps  */
    { 0x60, 0x08, 48, { 0 } }, /* 48 Mbps  */
    { 0x6C, 0x0C, 54, { 0 } }  /* 54 Mbps  */
};

/* phy 带宽枚举对应phy寄存器的值 */
const hh503_phy_bandwidth_stru g_phy_bw_sec_tbl[WLAN_BAND_WIDTH_BUTT] = {
    /* 带宽枚举                                 带宽 次20偏移 次40偏移 次80偏移 */
    {WLAN_BAND_WIDTH_20M,                   0,      0,      0,      0},
    {WLAN_BAND_WIDTH_40PLUS,                1,      4,      0,      0},
    {WLAN_BAND_WIDTH_40MINUS,               1,      -4,     0,      0},
    {WLAN_BAND_WIDTH_80PLUSPLUS,            2,      4,      10,     0},
    {WLAN_BAND_WIDTH_80PLUSMINUS,           2,      4,      -6,     0},
    {WLAN_BAND_WIDTH_80MINUSPLUS,           2,      -4,     6,      0},
    {WLAN_BAND_WIDTH_80MINUSMINUS,          2,      -4,    -10,     0},
    {WLAN_BAND_WIDTH_5M,                    8,      0,      0,      0},
    {WLAN_BAND_WIDTH_10M,                   12,     0,      0,      0}
};

hal_reg32_cfg_stru g_flatness_reg_table[] = {};

#ifdef BOARD_ASIC_WIFI
/* ASIC PHY 寄存器 初始化值 */
hal_reg32_cfg_stru g_phy_init_cfg[] = {
    {HH503_PHY_BANK3_BASE_0x2B0,                    0x0006},
    {HH503_PHY_BANK3_BASE_0x264,           0x2d0000},
    {HH503_PHY_BANK3_BASE_0x288,           0x150000},
    {HH503_PHY_BANK1_BASE_0x98,                 0x70089},
    {HH503_PHY_BANK3_BASE_0x368, 0x00700000},
    {HH503_PHY_BANK4_BASE_0x26C,                      0x0c010000}, /* 上行tb时需要配置0x0c010000 针对聚合帧长较长时出现载波频偏大的问题 */
    {HH503_PHY_BANK5_BASE_0xB8,                 0x644060},
    {HH503_PHY_BANK1_BASE_0x9C,                  0x769400},
    {HH503_PHY_BANK1_BASE_0xA0,                   0x6c007694},
    {HH503_PHY_BANK1_BASE_0x74,              0x81},
    {HH503_PHY_BANK2_BASE_0x100,             0x11080},
    {HH503_PHY_BANK2_BASE_0x1D0,       0x3d8},
    {HH503_PHY_BANK2_BASE_0x1D4,       0x3c0},
    {HH503_PHY_BANK2_BASE_0x1D8,       0x3a8},
    {HH503_PHY_BANK2_BASE_0x1DC,       0x390},
    {HH503_PHY_BANK2_BASE_0x1E0,       0x378},
    {HH503_PHY_BANK2_BASE_0x1E4,       0x360},
    {HH503_PHY_BANK2_BASE_0x1E8,       0x348},
    {HH503_PHY_BANK2_BASE_0x1EC,               0x3f8},
    {HH503_PHY_BANK2_BASE_0x1F0,               0x10},
    {HH503_PHY_BANK2_BASE_0x1F4,               0x28},
    {HH503_PHY_BANK2_BASE_0x1F8,               0x40},
    {HH503_PHY_BANK2_BASE_0x1FC,               0x58},
    {HH503_PHY_BANK2_BASE_0x200,               0x70},
    {HH503_PHY_BANK2_BASE_0x204,               0x88},
    {HH503_PHY_BANK2_BASE_0x208,               0xa0},
    {HH503_PHY_BANK2_BASE_0x248,                0x123e0070},
    {HH503_PHY_BANK1_BASE_0x24,                     0xc1100301},
    {HH503_PHY_BANK1_BASE_0xD4,            0x1294A108},
    {HH503_PHY_BANK1_BASE_0xDC,            0x1294A508},
    {HH503_PHY_BANK1_BASE_0xE0,            0x10842108},
    {HH503_PHY_BANK1_BASE_0xE4,            0x42108},
    {HH503_PHY_BANK5_BASE_0x30,         0x00506870},
    {HH503_PHY_BANK5_BASE_0x50,          0x3FFFFFFF},
    {HH503_PHY_BANK5_BASE_0x54,          0x3FFFF},
    {HH503_PHY_BANK5_BASE_0x58,          0x0},
    {HH503_PHY_BANK5_BASE_0x174,                   0x2000000},
    {HH503_PHY_BANK5_BASE_0x17C,          0x0},
    {HH503_PHY_BANK5_BASE_0x18C,                 0xb},
    {HH503_PHY_BANK5_BASE_0x1E0,                       0x61D09},
    {HH503_PHY_BANK2_BASE_0x1A0,                0x2d0c4cc},   // RSSI 偏高 降低2dB配置
    {HH503_PHY_BANK5_BASE_0x1B4,                 0x72212E88},
    {HH503_PHY_BANK5_BASE_0x1B8,                 0x3F6C5302},
    {HH503_PHY_BANK5_BASE_0x1BC,                 0x252EB8},
    {HH503_PHY_BANK5_BASE_0x1C0,                 0x67F89302},
    {HH503_PHY_BANK5_BASE_0x1C4,                 0x8487},
    {HH503_PHY_BANK5_BASE_0x1C8,                 0x9102},
    {HH503_PHY_BANK5_BASE_0x1A0,                0x28},
    {HH503_PHY_BANK5_BASE_0x1A4,                0x28081020},
    {HH503_PHY_BANK5_BASE_0x2B4,             0x2193204},
    {HH503_PHY_BANK5_BASE_0x3D4,             0x258},
    {HH503_PHY_BANK5_BASE_0x37C,                    0x306},
    {HH503_PHY_BANK5_BASE_0x380,                    0x509},
    {HH503_PHY_BANK5_BASE_0x384,                    0x808},
    {HH503_PHY_BANK5_BASE_0x388,                    0x709},
    {HH503_PHY_BANK5_BASE_0x38C,                    0xB0C},
    {HH503_PHY_BANK5_BASE_0x390,                    0xA12},
    {HH503_PHY_BANK5_BASE_0x394,                    0xD16},
    {HH503_PHY_BANK5_BASE_0x398,                    0x1697},
    {HH503_PHY_BANK5_BASE_0x39C,                    0x1E32},
    {HH503_PHY_BANK5_BASE_0x3A0,                   0x20A3},
    {HH503_PHY_BANK5_BASE_0x3B4,                 0x3884805},
    {HH503_PHY_BANK5_BASE_0x3B8,                 0xC942332},
    {HH503_PHY_BANK5_BASE_0x3C0,              0x1C230F0A},
    {HH503_PHY_BANK5_BASE_0x3C4,              0x6E645046},
    {HH503_PHY_BANK5_BASE_0x3C8,              0xAA8C},

     /* 配置默认频偏系数 */
    {HH503_PHY_BANK5_BASE_0x190,          0x1F7},
    {HH503_PHY_BANK1_BASE_0xC,                0x0},
    {HH503_PHY_BANK1_BASE_0x28, 0x20000147}
};
#else
/* FPGA PHY 寄存器 初始化值 */
hal_reg32_cfg_stru g_phy_init_cfg[] = {
    {HH503_PHY0_CTL_BASE_0x54,       0x0},
    {HH503_PHY0_CTL_BASE_0x5C,       0x100},
    {HH503_PHY0_CTL_BASE_0x90,         0x1000},
    {HH503_PHY0_CTL_BASE_0x98,          0xd},
    {HH503_PHY_BANK3_BASE_0x368, 0x320000},
    {HH503_PHY_BANK3_BASE_0x2B0,                   0x0006},
    {HH503_PHY_BANK1_BASE_0x8C,                   0x40},
    {HH503_PHY_BANK1_BASE_0xC,                0x0},
    {HH503_PHY_BANK1_BASE_0x74,             0xf884}, /* FPGA支持1X LTF的发送，SIFS误差减少到正负0.1us SIFS TIME要满足协议spec */
    {HH503_PHY_BANK1_BASE_0x6C,            0x2fc},
    {HH503_PHY_BANK2_BASE_0x190,        0xac},
    {HH503_PHY_BANK3_BASE_0xB4,                     0x0},
    {HH503_PHY_BANK2_BASE_0x19C,               0x1080},
    {HH503_PHY_BANK1_BASE_0x64,              0x3f7eff},
    {HH503_PHY_BANK2_BASE_0x130, 0x3a00a00},   // rssi 精度调整 考虑10dB增益
    {HH503_PHY_BANK2_BASE_0x1D0,      0x1ff},
    {HH503_PHY_BANK2_BASE_0x1D4,      0x3a4},
    {HH503_PHY_BANK2_BASE_0x1D8,      0x38c},
    {HH503_PHY_BANK2_BASE_0x1DC,      0x378},
    {HH503_PHY_BANK2_BASE_0x1E0,      0x364},
    {HH503_PHY_BANK2_BASE_0x1E4,      0x34c},
    {HH503_PHY_BANK2_BASE_0x1E8,      0x344},
    {HH503_PHY_BANK1_BASE_0x68,      0x088cc400},
    {HH503_PHY_BANK1_BASE_0x24,                    0xc1100301},
    {HH503_PHY_BANK5_BASE_0x174,                  0x2000000},
    {HH503_PHY_BANK5_BASE_0x18C,                0xb},
    {HH503_PHY_BANK3_BASE_0x248,      0xffff0010},
    {HH503_PHY_BANK3_BASE_0x24C,      0xffffcfff},
    {HH503_PHY_BANK3_BASE_0x240,      0x101},
    {HH503_PHY_BANK1_BASE_0x2C,                  0x820},
    {HH503_PHY_BANK5_BASE_0x88,                 0xa000002},
    {HH503_PHY_BANK3_BASE_0x264,          0x130006},
    {HH503_PHY_BANK3_BASE_0x288,          0x150000},
    {HH503_PHY_BANK1_BASE_0x98,                0x700b6},
    {HH503_PHY_BANK4_BASE_0x26C,                     0x0c011000},   /* 上行tb时需要配置0x0c010000 针对聚合帧长较长时出现载波频偏大的问题 */
    {HH503_PHY_BANK1_BASE_0xA0,                  0xff00ffff},
    {HH503_PHY_BANK2_BASE_0x1EC,              0x3e0},
    {HH503_PHY_BANK2_BASE_0x1F0,              0x3f4},
    {HH503_PHY_BANK2_BASE_0x1F4,              0xc},
    {HH503_PHY_BANK2_BASE_0x1F8,              0x24},
    {HH503_PHY_BANK2_BASE_0x1FC,              0x3c},
    {HH503_PHY_BANK2_BASE_0x200,              0x54},
    {HH503_PHY_BANK2_BASE_0x204,              0x6c},
    {HH503_PHY_BANK2_BASE_0x208,              0x84},
    {HH503_PHY_BANK2_BASE_0x248,           0x1a3b00f0},
    {HH503_PHY_BANK2_BASE_0x18,           0x190140},
    {HH503_PHY_BANK2_BASE_0x28,           0x100a},
    {HH503_PHY_BANK2_BASE_0x50,           0xc},
    {HH503_PHY_BANK2_BASE_0x58,   0x10},
    {HH503_PHY_BANK2_BASE_0x5C,   0x10},
    {HH503_PHY_BANK2_BASE_0x68,   0x20},
    {HH503_PHY_BANK2_BASE_0x6C,   0x40420},
    {HH503_PHY_BANK2_BASE_0x9C,   0x4848},
    {HH503_PHY_BANK2_BASE_0xA4,   0x40},
    {HH503_PHY_BANK2_BASE_0xB4,   0x1},
    {HH503_PHY_BANK2_BASE_0xC4,   0x3},
    {HH503_PHY_BANK2_BASE_0xE4,   0xb09595},
    {HH503_PHY_BANK2_BASE_0x100,   0xf107f},
    {HH503_PHY_BANK2_BASE_0x10C,   0xed4},
    {HH503_PHY_BANK2_BASE_0x128,   0xc70001},
    {HH503_PHY_BANK5_BASE_0x17C,   0x0},
    {HH503_PHY_BANK5_BASE_0x184,   0x20},
    {HH503_PHY_BANK5_BASE_0x188,   0x3},
    {HH503_PHY_BANK2_BASE_0x158,   0x3},
    {HH503_PHY_BANK2_BASE_0x168,   0x3},
    {HH503_PHY_BANK2_BASE_0x250,   0x17078},
    {HH503_PHY_BANK2_BASE_0x1A0,   0x2d0bccc},
    {HH503_PHY_BANK2_BASE_0x150,                0x13a3a000},
    {HH503_PHY_BANK2_BASE_0x194, 0x8181e18},
    {HH503_PHY_BANK5_BASE_0x1B4, 0x70212E88},
    {HH503_PHY_BANK5_BASE_0x1B8, 0x3F6C5302},
    {HH503_PHY_BANK5_BASE_0x1BC, 0x252CB8},
    {HH503_PHY_BANK5_BASE_0x1C0, 0x67F89302},
    {HH503_PHY_BANK5_BASE_0x1C4, 0x8487},
    {HH503_PHY_BANK5_BASE_0x1C8, 0x9102},
    {HH503_PHY_BANK5_BASE_0x1A0, 0x28},
    {HH503_PHY_BANK5_BASE_0x278, 0x6200C11},
    {HH503_PHY_BANK5_BASE_0x1E0, 0x61D08},
    {HH503_PHY_BANK5_BASE_0x2F4, 0x4C908},
    {HH503_PHY_BANK5_BASE_0x2B4, 0x2193204},
    {HH503_PHY_BANK2_BASE_0x238, 0xa6a3a6a3},
    {HH503_PHY_BANK1_BASE_0x9C, 0xffff00},
    {HH503_PHY_BANK5_BASE_0xB8, 0x644060},
    {HH503_PHY_BANK5_BASE_0x37C, 0x306},
    {HH503_PHY_BANK5_BASE_0x380, 0x509},
    {HH503_PHY_BANK5_BASE_0x384, 0x808},
    {HH503_PHY_BANK5_BASE_0x388, 0x709},
    {HH503_PHY_BANK5_BASE_0x38C, 0xB0C},
    {HH503_PHY_BANK5_BASE_0x390, 0xA12},
    {HH503_PHY_BANK5_BASE_0x394, 0xD16},
    {HH503_PHY_BANK5_BASE_0x398, 0x1697},
    {HH503_PHY_BANK5_BASE_0x39C, 0x1E32},
    {HH503_PHY_BANK5_BASE_0x3A0, 0x20A3},
    {HH503_PHY_BANK5_BASE_0x3B4, 0x3884805},
    {HH503_PHY_BANK5_BASE_0x3B8, 0xC942332},
    {HH503_PHY_BANK5_BASE_0x3C0, 0x1C230F0A},
    {HH503_PHY_BANK5_BASE_0x3C4, 0x6E645046},
    {HH503_PHY_BANK5_BASE_0x3C8, 0xAA8C},
    {HH503_PHY_BANK1_BASE_0x28, 0x20000147}
};
#endif


/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  :   ASIC PHY初始化
*****************************************************************************/
osal_void hal_initialize_phy(osal_void)
{
    /* CFG FLATNESS */
    hal_reg_write_tbl(g_flatness_reg_table, sizeof(g_flatness_reg_table) / sizeof(hal_reg32_cfg_stru));
    wifi_printf("===%s===%d===\r\n", __FUNCTION__, __LINE__);
    hal_reg_write_tbl(g_phy_init_cfg, sizeof(g_phy_init_cfg) / sizeof(hal_reg32_cfg_stru));
}

/*****************************************************************************
功能描述  : 设置phy max bw三个功能参数
*****************************************************************************/
osal_void hh503_set_phy_max_bw_field(hal_to_dmac_device_stru *hal_device, osal_u32 data,
    hal_phy_max_bw_sect_enmu_uint8 sect)
{
    u_pa_control1 pa_ctrl;
    wlan_channel_bandwidth_enum_uint8 bandwidth;

    /* 读取当前值 */
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);

    switch (sect) {
        case HAL_PHY_MAX_BW_SECT_MAX_BANDWIDTH:
            // 配置最大phy带宽
            bandwidth = (wlan_channel_bandwidth_enum_uint8)data;
            pa_ctrl.bits.xCt2OGAQOIz8Oyq_ = g_phy_bw_sec_tbl[bandwidth].bandwidth;
            break;

        case HAL_PHY_MAX_BW_SECT_MAX_NSS:
            // 配置最大nss
            pa_ctrl.bits.x6NgwuQtwSGPwhCC_ = (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_DOUBLE) ? 1 : 0;
            break;

        default:
            oam_error_log1(0, OAM_SF_ANY, "{hh503_set_phy_max_bw_field:: Error! sect[%d].}", sect);
            return;
    }

    /* 刷新值后重新写入寄存器 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}
/*****************************************************************************
 函 数 名  : hh503_set_sec_bandwidth_offset
 功能描述  : 设置次信道带宽
 调用函数  : hh503_set_rf_bw_cb hh503_set_rf_bw
*****************************************************************************/
osal_void hh503_set_sec_bandwidth_offset(osal_u8 bandwidth)
{
    if (bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) {
        oam_error_log0(0, OAM_SF_RX, "{hh503_set_sec_bandwidth_offset::not support 80M.}");
        bandwidth = WLAN_BAND_WIDTH_20M;
    }

    // 次信道配置
    if (g_phy_bw_sec_tbl[bandwidth].sec20_offset != 0) {
        hal_reg_write(HH503_PHY_BANK1_BASE_0xC, (osal_u8)g_phy_bw_sec_tbl[bandwidth].sec20_offset);
    }
}

osal_void hh503_set_phy_channel_num(osal_u8 channel_num)
{
    unref_param(channel_num);
}
osal_void hh503_set_phy_channel_freq_factor(osal_u32 freq_factor)
{
    hal_reg_write(HH503_PHY_BANK5_BASE_0x190, freq_factor);
}

/*****************************************************************************
 函 数 名  : hh503_set_ana_dbb_ch_sel
 功能描述  : 配置天线和phy通道对应关系
*****************************************************************************/
osal_void hh503_set_ana_dbb_ch_sel(const hal_to_dmac_device_stru *hal_device)
{
    unref_param(hal_device);
}

/*****************************************************************************
函 数 名  : hal_get_ch_measurement_result_ram
功能描述  : 读取PHY的信道测量结果寄存器
*****************************************************************************/
osal_void hal_get_ch_measurement_result_ram(hal_ch_statics_irq_event_stru *ch_statics)
{
    osal_u32 value;

    /* 2. 保存信道功率测量结果 */
    value = hal_reg_read(HH503_PHY_BANK2_BASE_0x18C);
    ch_statics->phy_ch_estimate_time_ms = (osal_u8)(value & 0xF);

    value = hal_reg_read(HH503_PHY_BANK2_BASE_0x2D8);
    ch_statics->pri20_idle_power = (osal_s8)(value & 0xFF);

    value = hal_reg_read(HH503_PHY_BANK2_BASE_0x2DC);
    ch_statics->pri40_idle_power = (osal_s8)(value & 0xFF);
}

osal_void hh503_set_phy_aid(osal_u16 aid)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_PHY_BANK1_BASE_0x30);
    regval &= (~HH503_PHY_CFG_11AX_STA_ID_MASK);
    regval |= ((osal_u32)aid << HH503_PHY_CFG_11AX_STA_ID_OFFSET);
    hal_reg_write(HH503_PHY_BANK1_BASE_0x30, regval);
}

/*****************************************************************************
 函 数 名  : hal_phy_rx_get_snr_info
 功能描述  : SNR数据转换，73已经ROM化，在收包时已经做了SNR转换，此处无需转换
*****************************************************************************/
osal_void hal_phy_rx_get_snr_info(hal_to_dmac_device_stru *hal_device, uint8_t phy_proto,
    uint16_t origin_data, int8_t *snr)
{
    unref_param(hal_device);
    unref_param(phy_proto);

    *snr = (int8_t)origin_data;
}

#ifdef _PRE_WLAN_FEATURE_FTM
/*****************************************************************************
 功能描述  : hh503_get_ftm_cali_rx_time
*****************************************************************************/
osal_void hh503_get_ftm_cali_rx_time(osal_u32 *ftm_cali_rx_time)
{
    *ftm_cali_rx_time = hal_reg_read(HH503_PHY_FTM_CALI_RX_TIME);
}

/*****************************************************************************
 功能描述  : hh503_get_ftm_cali_rx_intp_time
*****************************************************************************/
osal_void hh503_get_ftm_cali_rx_intp_time(osal_u32 *ftm_cali_rx_time)
{
    *ftm_cali_rx_time = hal_reg_read(HH503_PHY_FTM_CALI_INTP_TIME);
}

/*****************************************************************************
 功能描述  : hal_get_ftm_cali_tx_time
*****************************************************************************/
osal_void hal_get_ftm_cali_tx_time(osal_u32 *ftm_cali_tx_time)
{
    *ftm_cali_tx_time = hal_reg_read(HH503_PHY_FTM_CALI_TX_TIME);
}

/*****************************************************************************
功能描述  : hal_set_ftm_m2s_phy
*****************************************************************************/
osal_void hal_set_ftm_m2s_phy(oal_bool_enum_uint8 is_mimo, osal_u8 tx_chain_selection)
{
    /* siso_c0 */
    if ((tx_chain_selection == 1) && (is_mimo == 0)) {
        hal_reg_write(HH503_PHY_BANK3_BASE_0x1B0, 0x1);
        hal_reg_write(HH503_PHY_FTM_CFG,
                      (hal_reg_read(HH503_PHY_FTM_CFG) & (~HH503_PHY_CFG_FTM_CALI_CH_SEL_MASK)));
    /* siso_c1 */
    } else if ((tx_chain_selection == 0x2) && (is_mimo == 0)) {
        hal_reg_write(HH503_PHY_BANK3_BASE_0x1B0, 0x2);
        hal_reg_write(HH503_PHY_FTM_CFG,
                      (hal_reg_read(HH503_PHY_FTM_CFG) | HH503_PHY_CFG_FTM_CALI_CH_SEL_MASK));
    /* mimo_c0 */
    } else if ((tx_chain_selection == 1) && (is_mimo == 1)) {
        hal_reg_write(HH503_PHY_BANK3_BASE_0x1B0, 0xf);
        hal_reg_write(HH503_PHY_FTM_CFG,
                      (hal_reg_read(HH503_PHY_FTM_CFG) & (~HH503_PHY_CFG_FTM_CALI_CH_SEL_MASK)));
    /* mimo_c1 */
    } else if ((tx_chain_selection == 0x2) && (is_mimo == 1)) {
        hal_reg_write(HH503_PHY_BANK3_BASE_0x1B0, 0xf);
        hal_reg_write(HH503_PHY_FTM_CFG,
                      (hal_reg_read(HH503_PHY_FTM_CFG) | HH503_PHY_CFG_FTM_CALI_CH_SEL_MASK));
    } else {
        return;
    }
}

/*****************************************************************************
功能描述  : hal_get_ftm_rtp_reg
*****************************************************************************/
osal_void hal_get_ftm_rtp_reg(osal_u32 *reg0, osal_u32 *reg1, osal_u32 *reg2, osal_u32 *reg3, osal_u32 *reg4)
{
    *reg0 = hal_reg_read(HH503_PHY_FTM_RPT_REG0);
    *reg1 = hal_reg_read(HH503_PHY_FTM_RPT_REG1);
    *reg2 = hal_reg_read(HH503_PHY_FTM_RPT_REG2);
    *reg3 = hal_reg_read(HH503_PHY_FTM_RPT_REG3);
    *reg4 = hal_reg_read(HH503_PHY_FTM_RPT_REG4);
}

/*****************************************************************************
 功能描述  : 获取FTM插值开关状态
*****************************************************************************/
osal_void hal_get_ftm_intp_status(osal_u32 *ftm_status)
{
    // 17表示同步新特新寄存器使能bit位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG0, 17, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM同步信息特性开关状态
*****************************************************************************/
osal_void hal_get_ftm_new_status(osal_u32 *ftm_status)
{
    // 状态有效位为1位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG0, 0, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM GPIO 脉冲开关状态
*****************************************************************************/
osal_void hal_get_ftm_pulse_enable_status(osal_u32 *ftm_status)
{
    // 3为时钟同步GPIO脉冲使能位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG0, 3, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM 计数器开关状态
*****************************************************************************/
osal_void hal_get_ftm_cnt_status(osal_u32 *ftm_status)
{
    // 13为FTM计数器寄存器使能位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG4, 13, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM晶振偏差更新使能状态
*****************************************************************************/
osal_void hal_get_ftm_crystal_oscillator_offset_update_status(osal_u32 *ftm_status)
{
    // 12为晶振偏差更新寄存器使能位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG4, 12, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM时钟偏差更新使能状态
*****************************************************************************/
osal_void hal_get_ftm_clock_offset_update_status(osal_u32 *ftm_status)
{
    // 17为时钟偏差更新寄存器使能位
    *ftm_status = hal_reg_read_bit(HH503_PHY_FTM_TIME_SYNC_REG6, 17, 1);
}

/*****************************************************************************
 功能描述  : 获取FTM时钟频率
*****************************************************************************/
osal_void hal_get_ftm_phy_bw_mode(osal_u32 *ftm_status)
{
    osal_u32 value;
    osal_u32 ax_enable_flag;
    osal_u32 bw_mode;
    // PHY时钟频率  1:160M 2:320M 3:640M
    // 2表示ax开关的状态，16表示带宽种类数
    osal_u32 mode[2][16] = {
        {1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
        {2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}
    };

    value = hal_reg_read(HH503_PHY_BANK1_BASE_0x0);
    ax_enable_flag = (value & (1 << 1)) == 0 ? 0 : 1;
    bw_mode = hal_reg_read(HH503_PHY_BANK1_BASE_0x4) & 0xF;

    *ftm_status = mode[ax_enable_flag][bw_mode];
}

/*****************************************************************************
 功能描述  : 获取FTM 插值上报频率
*****************************************************************************/
osal_void hal_get_ftm_intp_freq(osal_u32 *ftm_status)
{
    osal_u32 value;

    // ADC采样频率  0:80M 1:160M 2:320M 3:640M
    value = hal_reg_read(HH503_PHY_BANK1_BASE_0x10) & 0x3;
    // ADC 0:80M采样频率对应插值上报频率960M1 1: 60M采样频率对应插值上报频率1920M
    *ftm_status = (value == 0) ? 0 : 1;
}

/*****************************************************************************
 功能描述  : 获取FTM脉冲时刻
*****************************************************************************/
osal_u64 hal_get_ftm_pulse_offset(osal_void)
{
    osal_u32 value;

    value = hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG1);
    // 先屏蔽低16位，然后再左移16位，最后再或上低32位
    return (((osal_u64)hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG3) & 0xffff0000) << 0x10) | ((osal_u64)value);
}

/*****************************************************************************
 功能描述  : 获取FTM 脉冲周期
*****************************************************************************/
osal_u64 hal_get_ftm_pulse_freq(osal_void)
{
    osal_u32 value;

    value = hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG2);
    // 高位左移32位，最后再或上低位32位
    return (((osal_u64)hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG3) & 0xffff) << 0x20) | ((osal_u64)value);
}

/*****************************************************************************
 功能描述  : 获取FTM晶振偏差
*****************************************************************************/
osal_u32 hal_get_ftm_crystal_oscillator_offset(osal_void)
{
    osal_u32 value;

    value = hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG4) & 0xfff;
    return value;
}

/*****************************************************************************
 功能描述  : 获取FTM时钟偏差
*****************************************************************************/
osal_u64 hal_get_ftm_clock_offset(osal_void)
{
    osal_u32 value;

    value = hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG5);
    // 高位左移32位，最后再或上低位32位
    return (((osal_u64)hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG6) & 0xffff) << 0x20) | ((osal_u64)value);
}

/*****************************************************************************
 功能描述  : 设置FTM插值开关状态
*****************************************************************************/
osal_void hh503_set_ftm_intp_status(osal_u32 ftm_status)
{
    // 17为FTM插值寄存器使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG0, 17, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM同步信息特性开关状态
*****************************************************************************/
osal_void hh503_set_ftm_new_status(osal_u32 ftm_status)
{
    // 0为FTM同步特性使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG0, 0, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM计数步长复位开关
*****************************************************************************/
osal_void hal_set_ftm_step_count_status(osal_u32 ftm_status)
{
    // 1为FTM计数步长复位使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG0, 1, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM GPIO 脉冲开关状态
*****************************************************************************/
osal_void hh503_set_ftm_pulse_enable_status(osal_u32 ftm_status)
{
    // 3为GPIO寄存器使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG0, 3, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM 计数器开关状态
*****************************************************************************/
osal_void hal_set_ftm_cnt_status(osal_u32 ftm_status)
{
    // 13为FTM计数器寄存器使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG4, 13, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM晶振偏差更新使能状态
*****************************************************************************/
osal_void hal_set_ftm_crystal_oscillator_offset_update_status(osal_u32 ftm_status)
{
    // 12为晶振寄存器使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG4, 12, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM时钟偏差更新使能状态
*****************************************************************************/
osal_void hh503_set_ftm_clock_offset_update_status(osal_u32 ftm_status)
{
    // 17为时钟偏差更新寄存器使能位
    hal_reg_write_bits(HH503_PHY_FTM_TIME_SYNC_REG6, 17, 1, ftm_status);
}

/*****************************************************************************
 功能描述  : 设置FTM晶振偏差
*****************************************************************************/
osal_void hal_set_ftm_crystal_oscillator_offset(osal_u32 ftm_status)
{
    osal_u32 value;

    // 先读取寄存器中的值，然后屏蔽低12位， 再将新的12位值写入
    value = (hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG4) & 0xfffff000) | (ftm_status & 0xfff);
    hal_reg_write(HH503_PHY_FTM_TIME_SYNC_REG4, value);
}

/*****************************************************************************
 功能描述  : 设置FTM配置状态
*****************************************************************************/
osal_void hal_set_ftm_clock_offset(osal_u64 ftm_status)
{
    osal_u32 high;
    osal_u32 low;
    osal_u32 value;

    high = (osal_u32)(ftm_status >> 0x20) & 0x1ffff;
    low = (osal_u32)(ftm_status);

    // 先将低32位写入
    hal_reg_write(HH503_PHY_FTM_TIME_SYNC_REG5, low);
    // 从高16位的寄存器中读出数据，然后屏蔽掉高17位，最后将高17位写入
    value = ((hal_reg_read(HH503_PHY_FTM_TIME_SYNC_REG6) & 0xfffe0000) | high);
    hal_reg_write(HH503_PHY_FTM_TIME_SYNC_REG6, value);
}

/*****************************************************************************
 功能描述  : 晶振偏差饱和处理
*****************************************************************************/
osal_s64 hal_sat_crystal_oscillator_offset(osal_s64 clock_offset, osal_s64 delta)
{
    osal_s64 clock_time;
    if (delta == 0) {
        return 0;
    }
    // 时钟累积偏差扩大32756倍，提高精度
    clock_time = (clock_offset << 0x15) / (osal_s64)delta;
    // 晶振偏差做饱和处理 0xb为晶振偏差偏移量
    if (clock_time > ((osal_s64)1 << 0xb) - 1) {
        clock_time = ((osal_s64)1 << 0xb) - 1;
    } else if (clock_time < (1 - (osal_s64)(1 << 0xb))) {
        clock_time = (1 - ((osal_s64)1 << 0xb));
    }
    return clock_time;
}

/*****************************************************************************
 功能描述  : hal_get_rx_digital_scaling_bypass
*****************************************************************************/
osal_void hal_get_rx_digital_scaling_bypass(osal_u32 *bypass)
{
    unref_param(bypass);
}
/*****************************************************************************
 功能描述  : hal_get_rx_lpf1_bypass
*****************************************************************************/
osal_void hal_get_rx_lpf1_bypass(osal_u32 *bypass)
{
    unref_param(bypass);
}
/*****************************************************************************
 功能描述  : hal_get_rxiq_filter_force_bypass
*****************************************************************************/
osal_void hal_get_rxiq_filter_force_bypass(osal_u32 *bypass)
{
    unref_param(bypass);
}


osal_void hh503_device_enable_ftm(osal_void)
{
}

osal_void hh503_device_disable_ftm(osal_void)
{
}
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
 函 数 名  : hh503_radar_config_reg_bw
 功能描述  : hh503_radar_config_reg_bw
*****************************************************************************/
osal_void hh503_radar_config_reg_bw(hal_dfs_radar_type_enum_uint8 radar_type,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    if (radar_type == HAL_DFS_RADAR_TYPE_ETSI) {
        if (bandwidth <= WLAN_BAND_WIDTH_40MINUS) {
            hal_reg_write(HH503_PHY_CFG_RADAR_REG5, 0x4001f3);
            hal_reg_write(HH503_PHY_RADARCONTROLREGISTER9, 0x21464140);
        } else {
            hal_reg_write(HH503_PHY_CFG_RADAR_REG5, 0x4001ff);
            hal_reg_write(HH503_PHY_RADARCONTROLREGISTER9, 0x21464150);
        }
    }
}

/*****************************************************************************
 函 数 名  : hh503_radar_enable_chirp_det
 功能描述  : hh503_radar_enable_chirp_det
*****************************************************************************/
osal_void hh503_radar_enable_chirp_det(oal_bool_enum_uint8 chirp_det)
{
    if (chirp_det == 1) {
        hal_reg_write(HH503_PHY_RADARCONTROLREGISTER10,
                      (hal_reg_read(HH503_PHY_RADARCONTROLREGISTER10)) & 0xFFFFFFFE);
    } else {
        hal_reg_write(HH503_PHY_RADARCONTROLREGISTER10,
                      (hal_reg_read(HH503_PHY_RADARCONTROLREGISTER10)) | 0x1);
    }
}

/*****************************************************************************
函 数 名  : hh503_enable_radar_det
功能描述  : 设置硬件RADAR检测的使能位状态
*****************************************************************************/
osal_void hh503_enable_radar_det_cb(osal_u8 enable)
{
    unref_param(enable);
}
#endif

/*****************************************************************************
 功能描述  : 获取11g 速率表
*****************************************************************************/
osal_void hh503_get_rate_80211g_table(const mac_data_rate_stru **rate)
{
    *rate = g_hh503_rates_11g;
}

/*****************************************************************************
 功能描述  : 获取11g 速率个数
*****************************************************************************/
osal_void hh503_get_rate_80211g_num(osal_u32 *rate_num)
{
    *rate_num = oal_array_size(g_hh503_rates_11g);
}

/*****************************************************************************
函 数 名  : hal_get_ch_measurement_result
功能描述  : 读取PHY的信道测量结果寄存器
*****************************************************************************/
osal_void hal_get_ch_measurement_result(hal_ch_statics_irq_event_stru *ch_statics)
{
    unref_param(ch_statics);
}

/*****************************************************************************
功能描述  : 获取phy关键统计信息
*****************************************************************************/
osal_void hh503_get_phy_statistics_data(hal_mac_key_statis_info_stru *mac_key_statis)
{
    unref_param(mac_key_statis);
}

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
/*****************************************************************************
功能描述  : 特殊干扰场景底噪规避
*****************************************************************************/
osal_void hal_phy_do_sgl_tone_notch_coef(osal_u32 notch_filter_coef1, osal_u32 notch_filter_coef2,
    osal_u32 sgl_tone_0_2_car_and_en, osal_u32 sgl_tone_3_car)
{
    hal_reg_write(HH503_PHY_BANK5_BASE_0x1D0, notch_filter_coef1);
    hal_reg_write(HH503_PHY_BANK5_BASE_0x1D4, notch_filter_coef2);
    hal_reg_write(HH503_PHY_BANK5_BASE_0x354, sgl_tone_0_2_car_and_en);
    hal_reg_write(HH503_PHY_BANK5_BASE_0x358, sgl_tone_3_car);
}

/*****************************************************************************
功能描述  : 特殊干扰场景底噪规避
*****************************************************************************/
osal_void hal_phy_do_sgl_tone_notch_weight(osal_u32 sgl_tone_0_7_weight, osal_u8 notch_ch)
{
    hal_reg_write(HH503_PHY_BANK5_BASE_0x360, sgl_tone_0_7_weight);
    hal_reg_write(HH503_PHY_BANK2_BASE_0xD8, (notch_ch == 0) ? 0x5 : 0x8);
}
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
/*******************************************************************************
 函 数 名  : hh503_radar_config_reg_rom_cb
*****************************************************************************/
static osal_void hh503_radar_config_reg_rom_cb(hal_dfs_radar_type_enum_uint8 radar_type)
{
    unref_param(radar_type);
}

static osal_void hh503_radar_config_reg_fcc(osal_void)
{
}

static osal_void hh503_radar_config_reg_etsi(osal_void)
{
}

static osal_void hh503_radar_config_reg_mkk(osal_void)
{
}

static osal_void hh503_radar_config_reg_korea(osal_void)
{
}

/*****************************************************************************
 函 数 名  : hh503_radar_config_reg_ext
 函 数 功 能 : DFS雷达检测配置
*****************************************************************************/
osal_void hh503_radar_config_reg_ext(hal_dfs_radar_type_enum_uint8 radar_type)
{
    switch (radar_type) {
        case HAL_DFS_RADAR_TYPE_FCC:
            hh503_radar_config_reg_fcc();
            break;

        case HAL_DFS_RADAR_TYPE_ETSI:
            hh503_radar_config_reg_etsi();
            break;

        case HAL_DFS_RADAR_TYPE_MKK:
            hh503_radar_config_reg_mkk();
            break;

        case HAL_DFS_RADAR_TYPE_KOREA:
            hh503_radar_config_reg_korea();
            break;

        default:
            break;
    }

    hh503_radar_config_reg_rom_cb(radar_type);
}

/*****************************************************************************
 函 数 功 能 : DFS雷达检测配置
*****************************************************************************/
osal_void hh503_radar_config_reg(hal_dfs_radar_type_enum_uint8 radar_type)
{
    hh503_radar_config_reg_ext(radar_type);
}
#endif
#ifdef BOARD_FPGA_WIFI  // 仅FPGA使用
#define CLOCK_REG_NUM 2

osal_void hh503_set_phy_gain(osal_void)
{
}

/*****************************************************************************
 函 数 名  : hh503_phy_set_cfg_pa_mode_code
 功能描述  : 设置PA_MODE_0_CODE PA_MODE_1_CODE
*****************************************************************************/
osal_void hh503_phy_set_cfg_pa_mode_code(osal_void)
{
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
