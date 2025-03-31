/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Hal PM.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_pm.h"
#include "osal_common.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_mac.h"
#include "frw_timer.h"
#ifdef CONFIRM_URANUS_CS
#include "pm_extern.h"
#include "soc_lowpower.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WIFI_HOST_HAL_PM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
hal_pm_ctrl_stru  g_pm_ctl;
osal_u16  g_ext_inner_offset_diff  = HAL_PM_DEF_EXT_TSF_OFFSET - HAL_PM_DEF_TBTT_OFFSET;
/* 需要参考92提供公共头文件，当前定义在hal_irq_rom.c文件，无法引用，此处先定义，后续整改 */
#define HH503_MAC_RPT_INTR_BCN_NO_FRM_LEN  1
#define HH503_MAC_RPT_INTR_BCN_NO_FRM_OFFSET   29
#define HH503_MAC_RPT_INTR_BCN_NO_FRM_MASK 0x20000000

/*****************************************************************************
 功能描述 : 设置内置和外置的tbtt offset
*****************************************************************************/
osal_void hh503_pm_set_tbtt_offset(hal_to_dmac_vap_stru *hal_vap, osal_u16 adjust_val)
{
    osal_u16 tbtt_offset;
    osal_u8 bcn_rf_chain;

    bcn_rf_chain = hal_vap->pm_info.bcn_rf_chain;

    if (bcn_rf_chain == WLAN_PHY_CHAIN_DOUBLE) {
        tbtt_offset = hal_vap->pm_info.inner_tbtt_offset_mimo;
    } else {
        tbtt_offset = hal_vap->pm_info.inner_tbtt_offset_siso;
    }

    hal_set_tbtt_offset(hal_vap, tbtt_offset + adjust_val);
}

/*****************************************************************************
 功能描述 : 设置beacon接收通道
*****************************************************************************/
osal_void hh503_pm_set_bcn_rf_chain(hal_to_dmac_vap_stru *hal_vap, osal_u8 bcn_rf_chain)
{
    /* 更新bcn rx chain */
    hal_vap->pm_info.bcn_rf_chain = bcn_rf_chain;

    /* 更新tbtt offset */
    hh503_pm_set_tbtt_offset(hal_vap, 0);
}

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
/*****************************************************************************
 功能描述 : probe内存分配和初始化
*****************************************************************************/
osal_void hh503_tbtt_offset_probe_init(hal_to_dmac_vap_stru *hal_vap)
{
    if (hal_vap->pm_info.offset_probe == OSAL_NULL) {
        hal_vap->pm_info.offset_probe = (hal_tbtt_offset_probe_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(hal_tbtt_offset_probe_stru), OAL_TRUE);
        if (hal_vap->pm_info.offset_probe == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hh503_tbtt_offset_probe_init fail:vap[%d]}", hal_vap->vap_id);
            return;
        }
    }
    (osal_void)memset_s(hal_vap->pm_info.offset_probe, sizeof(hal_tbtt_offset_probe_stru),
        0, sizeof(hal_tbtt_offset_probe_stru));
    hal_vap->pm_info.offset_probe->offset_siso_base = hal_vap->pm_info.inner_tbtt_offset_siso;
    hal_vap->pm_info.offset_probe->offset_mimo_base = hal_vap->pm_info.inner_tbtt_offset_mimo;
}

/*****************************************************************************
 功能描述 : probe内存释放
*****************************************************************************/
osal_void hh503_tbtt_offset_probe_destroy(hal_to_dmac_vap_stru *hal_vap)
{
    if (hal_vap->pm_info.offset_probe != OSAL_NULL) {
        oal_mem_free(hal_vap->pm_info.offset_probe, OAL_TRUE);
        hal_vap->pm_info.offset_probe = OSAL_NULL;
    }
}

#endif

/*****************************************************************************
 函 数 名  : hh503_init_pm_info
 功能描述  : 初始化pm_info结构体
*****************************************************************************/
osal_void hh503_init_pm_info(hal_to_dmac_vap_stru *hal_vap)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_vap->pm_info.inner_tbtt_offset_mimo = HAL_PM_DEF_TBTT_OFFSET;
    hal_vap->pm_info.inner_tbtt_offset_siso = HAL_PM_DEF_TBTT_OFFSET;

    hal_device = hal_chip_get_hal_device();
    hh503_pm_set_bcn_rf_chain(hal_vap, hal_device->cfg_cap_info->rf_chain);

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
    hh503_tbtt_offset_probe_init(hal_vap);
#endif
}

/*****************************************************************************
 功能描述  : 低功耗投票
*****************************************************************************/
osal_void hh503_pm_vote2platform(hal_pm_mode_enum_uint8 pm_mode)
{
    /* asic开发平台低功耗实现 */
    unref_param(pm_mode);
}

/*****************************************************************************
 功能描述  : 向平台去注册service id
*****************************************************************************/
osal_void hh503_pm_servid_unregister(osal_void)
{
#ifdef CONFIRM_URANUS_CS
    uapi_pm_unvote_against_sleep(PM_DEEP_SLEEP, PM_ID_WIFI);
    if (uapi_pm_register_wakeup_callback(WCPU_WKUP_TYPE_EXT0_TSF, OSAL_NULL) !=  OAL_SUCC) {
        oam_error_log0(0, 0, "hh503_pm_servid_unregister::unregister wakeup cb failed.");
    }
#endif
    g_pm_ctl.state = HAL_PM_DEEPSLEEP;
}

/*****************************************************************************
功能描述  : 设置内置tbtt提前量
*****************************************************************************/
osal_void hh503_set_tbtt_offset(const hal_to_dmac_vap_stru *hal_vap, osal_u16 offset)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 value;
    osal_u32 addr[] = {HH503_MAC_CTRL2_BANK_BASE_0x10, HH503_MAC_CTRL2_BANK_BASE_0x38, HH503_MAC_CTRL2_BANK_BASE_0x130};

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_warning_log2(0, 0, "hh503_set_psm_tbtt_offset::vap[%d] id[%d] invalid.}", hal_vap->vap_mode, vap_id);
        return;
    }

    /* 入参的单位为us */
    value = hal_reg_read(addr[vap_id]);
    value &= ~(HH503_MAC_CFG_VAP0_OFFSET_TBTT_VAL_MASK);
    hal_reg_write(addr[vap_id], value | offset);
}

/*****************************************************************************
功能描述  : 设置hh503_set_psm_beacon_period寄存器
*****************************************************************************/
osal_void hh503_set_psm_beacon_period(const hal_to_dmac_vap_stru *hal_vap, osal_u32 beacon_period)
{
    osal_u8 vap_id = hal_vap->vap_id;

    osal_u32 addr[] = {
        HH503_MAC_VLD_BANK_BASE_0x8, HH503_MAC_VLD_BANK_BASE_0x20, HH503_MAC_VLD_BANK_BASE_0x90
    };

    osal_u32 update_addr[] = {
        HH503_MAC_VLD_BANK_BASE_0xC, HH503_MAC_VLD_BANK_BASE_0x24, HH503_MAC_VLD_BANK_BASE_0x94
    };

    if ((hal_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) || (vap_id >= HAL_MAX_VAP_NUM)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_set_psm_beacon_period::vap mode [%d]!= WLAN_VAP_MODE_BSS_STA,or hal vap id=%d >= max vap=%d]}",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    hal_reg_write(addr[vap_id], (osal_u32)beacon_period);
    hal_reg_write(update_addr[vap_id], ((osal_u32)beacon_period << 0xa));

    oam_warning_log2(0, OAM_SF_CFG, "{hh503_set_psm_beacon_period::after config,hal vap id[%d] beacon period[%d]}",
        vap_id, (osal_u32)beacon_period);
}

/*****************************************************************************
功能描述  : 根据用户uc_lut_index去使能peer_sta_ps_ctrl
*****************************************************************************/
osal_void hh503_tx_disable_resp_ps_bit_ctrl(osal_u8 lut_index)
{
    u_ps_ctrl ps;

    ps.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x128);
    ps.bits.xCt2OwmsGOGsOylROCRwx_  &= ~(BIT0 << lut_index);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x128, ps.u32);
}

/*****************************************************************************
功能描述  : 根据用户uc_lut_index使能peer_sta_ps_ctrl
*****************************************************************************/
osal_void hh503_tx_enable_peer_sta_ps_ctrl(const osal_u8 lut_index)
{
    u_ps_ctrl ps;

    ps.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x128);
    ps.bits.xCt2OGmmwOsRzOGsOCRwx_ |= (BIT0 << lut_index);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x128, ps.u32);
}

/*****************************************************************************
功能描述  : 根据用户uc_lut_index去使能peer_sta_ps_ctrl
*****************************************************************************/
osal_void hh503_tx_disable_peer_sta_ps_ctrl(const osal_u8 lut_index)
{
    u_ps_ctrl ps;

    ps.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x128);
    ps.bits.xCt2OGmmwOsRzOGsOCRwx_ &= ~(BIT0 << lut_index);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x128, ps.u32);
}

#ifdef _PRE_WLAN_FEATURE_TXOPPS
/*****************************************************************************
函 数 名  : hh503_txop_ps_enable
功能描述  : 设置HH503_PA_TXOP_PS_CTRL_REG寄存器中使能字段
*****************************************************************************/
osal_void hh503_set_txop_ps_enable(hal_to_dmac_device_stru *hal_device, osal_u8 on_off)
{
    u_pa_control2 pa_ctrl2;

    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);

    if (hal_device->cfg_cap_info->txopps_is_supp == OSAL_TRUE) {
        /* 使能位寄存器BIT0 */
        pa_ctrl2.bits.xCt2OR8vGOGsOmo_ = 0;
        if (on_off != 0) {
            pa_ctrl2.bits.xCt2OR8vGOGsOmo_ = 1;
        }
    } else {
        /* 关闭位寄存器BIT0 */
        pa_ctrl2.bits.xCt2OR8vGOGsOmo_ = 0;
    }

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
}

/*****************************************************************************
功能描述  : 设置HH503_PA_TXOP_PS_CTRL_REG寄存器中condition1使能
*****************************************************************************/
osal_void hh503_set_txop_ps_condition1(osal_u8 on_off)
{
    u_pa_control2 pa_ctrl2;

    /* conditon1为寄存器BIT1 */
    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.x6NgwLPiuwuCw06ihF_ = 0;
    if (on_off != 0) {
        pa_ctrl2.bits.x6NgwLPiuwuCw06ihF_ = 1;
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
}

/*****************************************************************************
功能描述  : 设置HH503_PA_TXOP_PS_CTRL_REG寄存器中conditon2使能字段
*****************************************************************************/
osal_void hh503_set_txop_ps_condition2(osal_u8 on_off)
{
    u_pa_control2 pa_ctrl2;

    /* conditon2为寄存器BIT2 */
    pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
    pa_ctrl2.bits.xCt2OR8vGOGsOZCvo4_ = 0;
    if (on_off != 0) {
        pa_ctrl2.bits.xCt2OR8vGOGsOZCvo4_ = 1;
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
}

/*****************************************************************************
功能描述  : 设置mac partial aid寄存器
*****************************************************************************/
osal_void hh503_set_txop_ps_partial_aid(const hal_to_dmac_vap_stru *hal_vap, osal_u32 partial_aid)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 regval;

    osal_u32 vap_aid_val_reg[HAL_MAX_VAP_NUM] = {
        HH503_MAC_CTRL2_BANK_BASE_0x24, HH503_MAC_CTRL2_BANK_BASE_0x4C, HH503_MAC_CTRL2_BANK_BASE_0x144
    };

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_set_txop_ps_partial_aid::hal vap id=%d > max vap=%d]}", vap_id,
            HAL_MAX_VAP_NUM);
        return;
    }
    regval = hal_reg_read(vap_aid_val_reg[vap_id]) & (~HH503_MAC_CFG_VAP0_PARTIAL_AID_MASK);

    hal_reg_write(vap_aid_val_reg[vap_id], regval | partial_aid);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
