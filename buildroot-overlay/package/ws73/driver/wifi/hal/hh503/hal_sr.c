/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Interface of power settings for hal layer.
 * Create: 2023-1-17
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_sr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_SR_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_SR
osal_void hh503_get_sr_info(osal_u8 param)
{
    osal_u32 val;
    osal_u32 reg1, reg2, reg3, reg4;
    /* 获取SR使能 */
    
    if (param & BIT0) {
        val = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        oam_warning_log3(0, OAM_SF_ANY, "{OBSS_PD = %d,  NON_SRG = %d,  SRG = %d!}",
            val & BIT0, (val & BIT1) >> 1, (val & BIT2) >> 0x2);
    } else if (param & BIT1) { /* 获取SR的pd_lv和tx_power */
        val = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4); /* non_srg的pd_lv */
        oam_warning_log4(0, OAM_SF_ANY,
            "{NON_SRG PD_LV = 0x%x, PD_SR_TX_POWER = 0x%x, SRG PD_LV = 0x%x, SRG TX_POWER = 0x%x!}",
            val & HH503_MAC_CFG_OBSS_PD_LV_MASK,
            (val & HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_MASK) >> HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_OFFSET,
            (val & HH503_MAC_CFG_SRG_OBSS_PD_LV_MASK)>> HH503_MAC_CFG_SRG_OBSS_PD_LV_OFFSET,
            val >> HH503_MAC_CFG_OBSS_PD_SRG_TX_POWER_OFFSET);
    } else if (param & BIT2) { /* 获取帧复用数目 */
        reg1 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x148);
        reg2 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x14C);
        reg3 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x150);
        reg4 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x160);
        val = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x164);
        oam_warning_log4(0, OAM_SF_ANY,
            "NON_SRG_62_68_CNT = %d, NON_SRG_68_74_CNT = %d, NON_SRG_74_78_CNT = %d, NON_SRG_78_82_CNT = %d",
            reg3 >> HH503_MAC_RPT_NON_SRG_62_68_CNT_OFFSET, reg3 & HH503_MAC_RPT_NON_SRG_68_74_CNT_MASK,
            reg4 >> HH503_MAC_RPT_NON_SRG_74_78_CNT_OFFSET, reg4 & HH503_MAC_RPT_NON_SRG_78_82_CNT_MASK);
        oam_warning_log4(0, OAM_SF_ANY,
            "SRG_62_68_CNT = %d, SRG_68_74_CNT = %d, SRG_74_78_CNT = %d, SRG_78_82_CNT = %d",
            reg1 >> HH503_MAC_RPT_SRG_62_68_CNT_OFFSET, reg1 & HH503_MAC_RPT_SRG_68_74_CNT_MASK,
            reg2 >> HH503_MAC_RPT_SRG_74_78_CNT_OFFSET, reg2 & HH503_MAC_RPT_SRG_78_82_CNT_MASK);
        oam_warning_log2(0, OAM_SF_ANY, "TX_SUCCESS_NUM = %d, TX_NUM = %d",
            val & HH503_MAC_RPT_OBSS_PD_TX_SUCCESS_NUM_MASK, val >> HH503_MAC_RPT_OBSS_PD_TX_NUM_OFFSET);
    }
}

osal_void hh503_set_psr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs)
{
    osal_u32 reg1;

    reg1 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x168);

    hmac_sr_sta_vfs->rpt_psr_sr_tx_success_num = (reg1 >> 0) & 0xFFFF;
    hmac_sr_sta_vfs->rpt_psr_sr_tx_num = (reg1 >> 16) & 0xFFFF; /* 16:右移16位 */
}

osal_void hh503_set_sr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs)
{
    osal_u32 reg1;

    reg1 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x164);

    hmac_sr_sta_vfs->rpt_obss_pd_tx_success_num = (reg1 >> 0) & 0xFFFF;
    hmac_sr_sta_vfs->rpt_obss_pd_tx_num = (reg1 >> 16) & 0xFFFF; /* 16:右移16位 */
}

osal_void hh503_set_sr_statistics_get(dmac_sr_sta_srg_stru *hmac_sr_sta_srg,
    dmac_sr_sta_non_srg_stru *hmac_sr_sta_non_srg)
{
    osal_u32 reg1, reg2, reg3, reg4;

    reg1 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x148);
    reg2 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x14C);
    reg3 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x150);
    reg4 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x160);

    hmac_sr_sta_srg->rpt_srg_78_82_cnt = (reg2 >> 0) & 0xFFFF;
    hmac_sr_sta_srg->rpt_srg_74_78_cnt = (reg2 >> 16) & 0xFFFF; /* 16:右移16位 */
    hmac_sr_sta_srg->rpt_srg_68_74_cnt = (reg1 >> 0) & 0xFFFF;
    hmac_sr_sta_srg->rpt_srg_62_68_cnt = (reg1 >> 16) & 0xFFFF; /* 16:右移16位 */

    hmac_sr_sta_non_srg->rpt_non_srg_78_82_cnt = (reg4 >> 0) & 0xFFFF;
    hmac_sr_sta_non_srg->rpt_non_srg_74_78_cnt = (reg4 >> 16) & 0xFFFF; /* 16:右移16位 */
    hmac_sr_sta_non_srg->rpt_non_srg_68_74_cnt = (reg3 >> 0) & 0xFFFF;
    hmac_sr_sta_non_srg->rpt_non_srg_62_68_cnt = (reg3 >> 16) & 0xFFFF; /* 16:右移16位 */
}

osal_void hh503_set_sr_vfs_end(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x1000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x800);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_psr_statistics_end(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x2000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x8000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_statistics_end(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x200);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x80);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_vfs_start(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x1000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x800;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_psr_statistics_start(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x2000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x8000;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_statistics_start(osal_void)
{
    osal_u32 regval;
    osal_u8 pd_en_offset = 1;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x80);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x200;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB0);
    regval = regval | (pd_en_offset << HH503_MAC_CFG_OBSS_PD_EN_OFFSET_OFFSET);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB0, regval);
}

osal_void hh503_set_sr_statistics_continue()
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x200;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_psr_statistics_continue()
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x8000;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_statistics_stop()
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x200);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_psr_statistics_stop()
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x8000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_vfs_clear(osal_void)
{
    osal_u32 regval;

    // start
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x800);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    // clear
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x1000;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_psr_statistics_clear(osal_void)
{
    osal_u32 regval;

    // start
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x8000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x2000;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

osal_void hh503_set_sr_statistics_clear(osal_void)
{
    osal_u32 regval;

    // start
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval & (~0x200);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    // clear
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = regval | 0x80;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);
}

/* 还未考虑psr_enable = 0, trig_enable = 0的情况 */
osal_void hh503_set_psr_ctrl(oal_bool_enum_uint8 flag)
{
    osal_u32 regval;

    if (flag == OSAL_TRUE) {
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA0);
        regval |= 0x2f; /* bit[3:0] = 1 bit[5] = 1 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA0, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval |= 0x80; /* bit[7] = 1 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
        regval |= 0xf; /* bit[3:0] = 1 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

        // trig_enable
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval |= 0x400; /* BIT10 = 1 */

        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);
    } else {
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA0);
        regval &= 0xffffffd0; /* bit[3:0] = 0 bit[5] = 0 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA0, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval &= 0xffffff7f; /* bit[7] = 0 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
        regval &= 0xfffffff0; /* bit[3:0] = 0 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

        // trig_enable
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval &= 0xfffffbff; /* BIT10 = 0 */

        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);
    }

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    regval = (regval & (~0xFFFF0000)) | (0xFF000000);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x100);
    regval = 0x000C1044;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x100, regval);
}

osal_void hh503_set_psrt_htc(osal_void)
{
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xC8, 0x11B);
}

osal_void hh503_set_psr_offset(osal_s32 offset)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA0);
    regval = (regval & (~0xFF00)) | (((osal_u32)offset << 0x8) & 0xFF00);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA0, regval);
}

osal_void hh503_set_srg_pd_etc(osal_u32 pd, osal_u32 pw)
{
    osal_u32 regval;
    u_spatial_reuse_ctrl1 sr_ctrl1;

    sr_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    sr_ctrl1.bits.x6NgwC_gwuG_LBGowjCCBFwWh_ = 1;
    sr_ctrl1.bits.xCt2Osw2OyssOCvxvwOmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, sr_ctrl1.u32);
    sr_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    sr_ctrl1.bits.xCt2Osw2OmxmImoROPx4_ = 1;
    sr_ctrl1.bits.x6NgwC_gwg_iduwBFwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, sr_ctrl1.u32);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4);
    /* HH503_MAC_OBSS_PD_POWER [15:8] */
    regval = (regval & (~HH503_MAC_CFG_SRG_OBSS_PD_LV_MASK)) |
        ((pd << HH503_MAC_CFG_SRG_OBSS_PD_LV_OFFSET) & HH503_MAC_CFG_SRG_OBSS_PD_LV_MASK);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB4, regval);

    /* ctrl1+0x12c[31:24] */
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4);
    regval = (regval & (~HH503_MAC_CFG_OBSS_PD_SRG_TX_POWER_MASK)) | (pw << HH503_MAC_CFG_OBSS_PD_SRG_TX_POWER_OFFSET);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB4, regval);
}

osal_void hh503_set_non_pd(osal_u32 pd, osal_u32 pw)
{
    osal_u32 regval;
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    regval = regval | HH503_MAC_CFG_OBSS_PD_BASE_CHK_EN_MASK;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4);
    regval = (regval & (~HH503_MAC_CFG_OBSS_PD_LV_MASK)) | pd;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB4, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4);
    regval = (regval & (~HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_MASK)) |
        ((pw << HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_OFFSET) & HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_MASK);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB4, regval);
}

osal_void hh503_set_srg_ctrl(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    regval |= HH503_MAC_CFG_OBSS_PD_EN_SEL_MASK; /* ctrl1+0x114[9] */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
    /* ctrl1+0x11c[6],ctrl1+0x11c[5:4],ctrl1+0x11c[3] */
    regval |= (HH503_MAC_CFG_OBSS_PD_BASED_TX_RATE_RANK_EN_MASK |
        HH503_MAC_CFG_OBSS_PD_BASED_TX_RATE_RANK_SEL_MASK | HH503_MAC_CFG_SR_DISALLOW_NAV_EN_MASK);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

    /* ctrl1+0x134[15:0] */
    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xFC);
    regval = (regval & (~0xFFFFFFFF)) | (0x000C1044);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xFC, regval);
}

osal_void hh503_set_srg_partial_bssid_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high)
{
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xC0, bitmap_low);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xC4, bitmap_high);
}

osal_void hh503_set_srg_bss_color_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high)
{
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB8, bitmap_low);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xBC, bitmap_high);
}

osal_void hh503_set_srg_ele_off(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    regval = regval & (~HH503_MAC_CFG_SRG_ELEMENT_VLD_MASK);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);
}

/*****************************************************************************
 函 数 名  : hh503_color_rpt_clr
 功能描述  : 冲突获取寄存器清零
*****************************************************************************/
osal_void hh503_color_rpt_clr(osal_void)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA8);
    regval = regval | (HH503_MAC_CFG_BSS_COLOR_RPT_CLR_MASK);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA8, regval);
}

/*****************************************************************************
 函 数 名  : hh503_color_area_get
 功能描述  : 获取到冲突信息
*****************************************************************************/
osal_void hh503_color_area_get(mac_color_area_stru *color_area)
{
    color_area->bss_color_bitmap_h = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x154);
    color_area->bss_color_bitmap_l = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x158);
}

osal_void hh503_set_sr_ctrl(oal_bool_enum_uint8 flag)
{
    osal_u32 regval;

    if (flag == OSAL_FALSE) {
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval = regval & (~(HH503_MAC_CFG_OBSS_PD_SPR_EN_MASK | HH503_MAC_CFG_OBSS_PD_BASE_CHK_EN_MASK |
            HH503_MAC_CFG_SRG_PPDU_CHK_EN_MASK | HH503_MAC_CFG_SRG_BSS_COLOR_EN_MASK |
            HH503_MAC_CFG_SRG_PARTIAL_BSSID_EN_MASK));
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xB4);
        regval &= (~(HH503_MAC_CFG_OBSS_PD_SR_TX_POWER_MASK | HH503_MAC_CFG_OBSS_PD_SRG_TX_POWER_MASK));
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xB4, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA0);
        regval &= (~(HH503_MAC_CFG_PSR_PWR_SW_CALC_MASK | HH503_MAC_CFG_PSR_SR_PWR_OFFSET_MASK));
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA0, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA4);
        regval &= (~(HH503_MAC_CFG_OBSS_PD_RPT_END_MASK | HH503_MAC_CFG_OBSS_PD_RPT_START_MASK |
            HH503_MAC_CFG_PSR_SR_RPT_CLR_MASK | HH503_MAC_CFG_PSR_SR_RPT_START_MASK));
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA4, regval);

        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
        regval &= (~(HH503_MAC_CFG_OBSS_PD_SR_TRIG_DIS_MASK | HH503_MAC_CFG_BSS_COLOR_DISALLOW_MASK));
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);

        return;
    }

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    regval = regval | HH503_MAC_CFG_OBSS_PD_SPR_EN_MASK;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);
}
#endif

/*****************************************************************************
功能描述  : 设置mac  aid寄存器
*****************************************************************************/
osal_void hh503_set_bss_color(const hal_to_dmac_vap_stru *hal_vap, osal_u8 bss_color)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 vap_aid_val_reg[HAL_MAX_VAP_NUM] = {
        HH503_MAC_CTRL2_BANK_BASE_0x24, HH503_MAC_CTRL2_BANK_BASE_0x4C, HH503_MAC_CTRL2_BANK_BASE_0x144
    };
    osal_u32 regval;

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_set_bss_color::hal vap id=%d > max vap=%d]}", vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    regval = hal_reg_read(vap_aid_val_reg[vap_id]);

    regval &= (~HH503_MAC_CFG_VAP0_BSS_COLOR_MASK);
    regval |= (((osal_u32)(bss_color & 0x3F) << HH503_MAC_CFG_VAP0_BSS_COLOR_OFFSET));

    hal_reg_write(vap_aid_val_reg[vap_id], regval);
}

/*****************************************************************************
功能描述  : 设置mac  aid寄存器
*****************************************************************************/
osal_void hh503_set_partial_bss_color(const hal_to_dmac_vap_stru *hal_vap, osal_u8 partial_bss_color)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 vap_aid_val_reg[HAL_MAX_VAP_NUM] = {
        HH503_MAC_CTRL2_BANK_BASE_0x24, HH503_MAC_CTRL2_BANK_BASE_0x4C, HH503_MAC_CTRL2_BANK_BASE_0x144
    };
    osal_u32 regval;

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_set_partial_bss_color::hal vap id=%d > max vap=%d]}", vap_id,
                       HAL_MAX_VAP_NUM);
        return;
    }

    regval = hal_reg_read(vap_aid_val_reg[vap_id]);

    regval &= (~HH503_MAC_CFG_VAP0_PARTIAL_BSS_COLOR_MASK);
    regval |= (((osal_u32)(partial_bss_color & 0x0F) << HH503_MAC_CFG_VAP0_PARTIAL_BSS_COLOR_OFFSET));

    hal_reg_write(vap_aid_val_reg[vap_id], regval);
}

osal_void hh503_set_bss_color_enable(oal_bool_enum_uint8 flag)
{
    osal_u32 regval;
    osal_u32 enable = 0;
    osal_u32 sr_enable = 0x800; /* 0x800000: 取 HH503_MAC_CFG_PSR_SR_RPT_EN 寄存器第24位值 */

    if (flag == OSAL_TRUE) {
        enable = 0x20; /* 0x20: 取 HH503_MAC_INTRA_BSS_NAV_QTP_CTRL bit5 */
        sr_enable = 0;
    }

    regval = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14);
    /* 0x20: 取 HH503_MAC_INTRA_BSS_NAV_QTP_CTRL bit5 */
    regval = (regval & (~0x20)) | (enable & 0x20);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14, regval);

    regval = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14);
    /* 0x400: 取 HH503_MAC_INTRA_BSS_NAV_QTP_CTRL bit10 */
    regval = (regval & (~0x400)) | (enable & 0x400);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14, regval);

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x9C);
    /* 0x800: 取 HH503_MAC_SPATIAL_REUSE_CTRL1 bit11 */
    regval = (regval & (~0x800)) | (sr_enable & 0x800);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x9C, regval);
}

/*****************************************************************************
 函 数 名  : hh503_color_rpt_en
 功能描述  : 冲突获取使能
*****************************************************************************/
osal_void hh503_color_rpt_en(oal_bool_enum_uint8 flag)
{
    osal_u32 regval;

    regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA8);
    regval = regval & (~HH503_MAC_CFG_BSS_COLOR_RPT_CLR_MASK);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA8, regval);

    if (flag) {
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA8);
        regval = regval | (HH503_MAC_CFG_BSS_COLOR_RPT_EN_MASK);
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA8, regval);
    } else {
        regval = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xA8);
        regval = regval & (~HH503_MAC_CFG_BSS_COLOR_RPT_EN_MASK);

        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xA8, regval);
    }
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
