/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Interface of power settings for hal layer.
 * Create: 2023-1-17
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_tbtt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
 
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_TBTT_C
 
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
 功能描述  : 调整tsf的基准
*****************************************************************************/
osal_void hal_tsf_pulse_adjust(osal_void)
{
    u_vap_tsf_adjust_offset tsf_ctrl2;
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
    tsf_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x50);

    if (pa_ctrl2.bits.xCt2OPzG3ORstOmo_ == 1) {
        tsf_ctrl2.bits.xCt2ORstOGSxsmOsmx_ = HH503_MAC_CFG_TSF_PULSE_SEL_VAP0;
    } else if (pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ == 1) {
        tsf_ctrl2.bits.xCt2ORstOGSxsmOsmx_ = HH503_MAC_CFG_TSF_PULSE_SEL_VAP1;
    } else if (vap2_mode_set0.bits.xCt2OPzGZORstOmo_ == 1) {
        tsf_ctrl2.bits.xCt2ORstOGSxsmOsmx_ = HH503_MAC_CFG_TSF_PULSE_SEL_VAP2;
    }
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x50, tsf_ctrl2.u32);
}

/*****************************************************************************
 功能描述 : 打开vap的tsf
*****************************************************************************/
osal_void hh503_vap_ap_start_tsf(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u8 vap_id = hal_vap->vap_id;
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    /* tsf控制寄存器 */
    osal_u32 tsf_ctl_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0xC, HH503_MAC_CTRL2_BANK_BASE_0x34, HH503_MAC_CTRL2_BANK_BASE_0x12C
    };

    if ((vap_id >= HAL_MAX_VAP_NUM) || (hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) ||
        (vap_id >= sizeof(tsf_ctl_reg) / sizeof(tsf_ctl_reg[0]))) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_vap_ap_start_tsf::vap mode [%d]!= WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d!",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    /* 1.1 启动vap的tsf，tsf status和tbtt status都会清0 */
    if (vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.xCt2OPzGZORstOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }

    /* 定位TSF被关闭的临时维测代码 */
    oam_warning_log2(0, OAM_SF_ASSOC,
        "{hh503_enable_ap_tsf_tbtt::VAP(hal id%d)TSF ENABLE, tsf reg val[0x%08x]}", vap_id,
        hal_reg_read(tsf_ctl_reg[vap_id]));
}

/*****************************************************************************
 函 数 名  : hh503_vap_tsf_get_32bit
*****************************************************************************/
osal_void hh503_vap_tsf_get_32bit(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_lo)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 addr[] = {
        HH503_MAC_RD0_BANK_BASE_0x5C, HH503_MAC_RD0_BANK_BASE_0x68,
        HH503_MAC_RD1_BANK_BASE_0x1AC
    };

    *tsf_lo = hal_reg_read(addr[vap_id]);
}

/*****************************************************************************
 功能描述 : 获取vap的timestamp的64位状态值,单位us
*****************************************************************************/
osal_void hh503_vap_tsf_get_64bit(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_high, osal_u32 *tsf_lo)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 tsf_lo_val1;
    osal_u32 tsf_lows[] = {
        HH503_MAC_RD0_BANK_BASE_0x5C, HH503_MAC_RD0_BANK_BASE_0x68
    };

    osal_u32 tsf_highs[] = {
        HH503_MAC_RD0_BANK_BASE_0x58, HH503_MAC_RD0_BANK_BASE_0x64
    };

    do {
        tsf_lo_val1 = hal_reg_read(tsf_lows[vap_id]);
        *tsf_high = hal_reg_read(tsf_highs[vap_id]);
        *tsf_lo = hal_reg_read(tsf_lows[vap_id]);
    } while (*tsf_lo < tsf_lo_val1);
}

/*****************************************************************************
功能描述  : 使能sta tbtt中断,低功耗 只支持3个VAP
*****************************************************************************/
osal_void hh503_enable_sta_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u32 value;
    osal_u32 reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0xC, HH503_MAC_CTRL2_BANK_BASE_0x34, HH503_MAC_CTRL2_BANK_BASE_0x12C
    };
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;
    hmac_vap_stru *hmac_vap;

    if (hal_vap->vap_id >= HAL_MAX_VAP_NUM) {
        oam_warning_log1(0, 0, "{hh503_enable_sta_tsf_tbtt::vap id[%d] invalid.}", hal_vap->vap_id);
        return;
    }

    hmac_vap =(hmac_vap_stru *)mac_res_get_hmac_vap(hal_vap->mac_vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, 0, "{hh503_enable_sta_tsf_tbtt::dmac vap is null.}");
        return;
    }

    /* 开启TSF使能,beacon超时,probe rsp同步,beacon同步,LISTEN CONT,DTIM CNT */
    if ((mac_mib_get_powermanagementmode(hmac_vap)) == WLAN_MIB_PWR_MGMT_MODE_PWRSAVE) {
        /* 节能态开启DTIM和LISTEN使能 */
        value = hal_reg_read(reg[hal_vap->vap_id]);
        value &= 0xfffffe00;
        value |= 0x3f; /* bit6-8=0, 0-5 = 1 */
    } else {
         /* 非节能态使用BEACON周期触发TBTT中断 */
        value = hal_reg_read(reg[hal_vap->vap_id]);
        value &= 0xfffffe00;
        value |= 0x3b; /* bit2 6-8=0, 0,1,3,4,5 = 1 */
    }
    hal_reg_write(reg[hal_vap->vap_id], value);

    if (hal_vap->vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (hal_vap->vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.xCt2OPzGZORstOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}

/*****************************************************************************
 功能描述 : 关闭ap模式下vap的tsf
*****************************************************************************/
osal_void hh503_vap_set_machw_tsf_disable(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u8 vap_id = hal_vap->vap_id;
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    if ((vap_id >= HAL_MAX_VAP_NUM) || (hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_vap_set_machw_tsf_disable::vap mode [%d] != WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d!",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    if (vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.xCt2OPzGZORstOmo_ = 0;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}


/*****************************************************************************
功能描述  : 关闭sta tbtt中断低功耗
*****************************************************************************/
osal_void hh503_disable_sta_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 value;
    osal_u32 tsf_ctl_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0xC, HH503_MAC_CTRL2_BANK_BASE_0x34, HH503_MAC_CTRL2_BANK_BASE_0x12C
    };
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_warning_log1(0, 0, "{hh503_disable_sta_tsf_tbtt::vap id[%d] invalid}", vap_id);
        return;
    }
    /* 恢复至默认值0x78即可         */
    value = hal_reg_read(tsf_ctl_reg[vap_id]);
    value &= 0xfffffe00;
    value |= 0x3c; /* 对应寄存器的bit0，1，6-8 = 0， 2-5 = 1 */
    hal_reg_write(tsf_ctl_reg[vap_id], value);

    if (vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzG3ORstOmo_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGu0wLCNwWh_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.xCt2OPzGZORstOmo_ = 0;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}

/*****************************************************************************
功能描述  : 打开vap tbtt中断
*****************************************************************************/
osal_void hh503_enable_tsf_tbtt(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 dbac_enable)
{
    if (hal_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hh503_enable_tsf_tbtt::hal_vap null}");
        return;
    }
    unref_param(dbac_enable);
    if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hh503_enable_sta_tsf_tbtt(hal_vap);
    } else if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hh503_vap_ap_start_tsf(hal_vap);
    } else {
        oam_error_log3(0, 0, "vap_id[%d] {hh503_enable_tsf_tbtt::hal vap[%d],wrong vap mode[%d]}",
            hal_vap->mac_vap_id, hal_vap->vap_id, hal_vap->vap_mode);
    }
    hal_tsf_pulse_adjust();
}

/*****************************************************************************
功能描述  : 关闭vap tbtt中断
*****************************************************************************/
osal_void hh503_disable_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap)
{
    if (hal_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_disable_tsf_tbtt::hal_vap null}");
        return;
    }

    if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hh503_disable_sta_tsf_tbtt(hal_vap);
    } else if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hh503_vap_set_machw_tsf_disable(hal_vap);
    } else {
        oam_error_log2(0, OAM_SF_ANY, "{hh503_disable_tsf_tbtt::hal vap[%d],wrong vap mode[%d]}",
            hal_vap->vap_id, hal_vap->vap_mode);
    }
    hal_tsf_pulse_adjust();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif