/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file of HAL power save public interfaces .
 * Create: 2020-8-8
 */
#ifndef __HAL_POWER_SAVE_H__
#define __HAL_POWER_SAVE_H__

#include "hal_common_ops.h"

/*****************************************************************************
 * 1 其他头文件包含
 *****************************************************************************/
/*****************************************************************************
 * STA ps feature
 *****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXOPPS
/*****************************************************************************
 * 功能描述  : 设置PA_TXOP_PS_CTRL_REG寄存器中使能字段
 *****************************************************************************/
static  INLINE__ osal_void hal_set_txop_ps_enable(hal_to_dmac_device_stru *hal_device, osal_u8 on_off)
{
    hal_public_hook_func(_set_txop_ps_enable)(hal_device, on_off);
}

/*****************************************************************************
 * 功能描述  : 设置PA_TXOP_PS_CTRL_REG寄存器中condition1使能
 *****************************************************************************/
static  INLINE__ osal_void hal_set_txop_ps_condition1(osal_u8 on_off)
{
    hal_public_hook_func(_set_txop_ps_condition1)(on_off);
}

/*****************************************************************************
 * 功能描述  : 设置PA_TXOP_PS_CTRL_REG寄存器中conditon2使能字段
 *****************************************************************************/
static  INLINE__ osal_void hal_set_txop_ps_condition2(osal_u8 on_off)
{
    hal_public_hook_func(_set_txop_ps_condition2)(on_off);
}

/*****************************************************************************
 * 功能描述  : 设置mac partial aid寄存器
 *****************************************************************************/
static  INLINE__ osal_void hal_set_txop_ps_partial_aid(const hal_to_dmac_vap_stru *hal_vap, osal_u32 partial_aid)
{
    hal_public_hook_func(_set_txop_ps_partial_aid)(hal_vap, partial_aid);
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
 * 功能描述  : 设置PA_SMPS_CTRL_REG寄存器中SMPS的模式
 *****************************************************************************/
static  INLINE__ osal_void hal_set_smps_mode(osal_u8 smps_mode)
{
    hal_public_hook_func(_set_smps_mode)(smps_mode);
}
#endif
/*****************************************************************************
 功能描述  : 低功耗唤醒后清除rx中断,防止rx中断里保留上一次睡前的值造成rx new
*****************************************************************************/
static  INLINE__ osal_void hal_psm_clear_mac_rx_isr(osal_void)
{
    hal_public_hook_func(_psm_clear_mac_rx_isr)();
}

#ifndef _PRE_WLAN_FEATURE_WS63
/*****************************************************************************
 * 功能描述  : 设置mac aid寄存器
 *****************************************************************************/
static  INLINE__ osal_void hal_set_mac_aid(const hal_to_dmac_vap_stru *hal_vap, osal_u16 aid)
{
    hal_public_hook_func(_set_mac_aid)(hal_vap, aid);
}

/*****************************************************************************
 * 功能描述  : 设置PSM寄存器中tbtt offset的值
 *****************************************************************************/
static  INLINE__ osal_void hal_set_tbtt_offset(const hal_to_dmac_vap_stru *hal_vap, osal_u16 offset)
{
    hal_public_hook_func(_set_tbtt_offset)(hal_vap, offset);
}

/*****************************************************************************
 * 功能描述  : 设置PSM寄存器中ext tbtt offset的值
 *****************************************************************************/
static  INLINE__ osal_void hal_set_psm_ext_tbtt_offset(const hal_to_dmac_vap_stru *hal_vap, osal_u16 offset)
{
    hal_public_hook_func(_set_psm_ext_tbtt_offset)(hal_vap, offset);
}

/*****************************************************************************
 * 功能描述  : 设置beacon period寄存器
 *****************************************************************************/
static  INLINE__ osal_void hal_vap_set_psm_beacon_period(const hal_to_dmac_vap_stru *hal_vap,
    osal_u32 beacon_period)
{
    hal_public_hook_func(_set_psm_beacon_period)(hal_vap, beacon_period);
}

/*****************************************************************************
 * 功能描述  : 设置beacon等待的超时值
 *****************************************************************************/
static  INLINE__ osal_void hal_set_beacon_timeout(osal_u16 value)
{
    hal_public_hook_func(_set_beacon_timeout)(value);
}

/*****************************************************************************
 * AP ps feature
 *****************************************************************************/
/*****************************************************************************
 * 功能描述  : 使能sta ps ctrl寄存器
 *****************************************************************************/
static  INLINE__ osal_void hal_tx_enable_peer_sta_ps_ctrl(osal_u8 lut_index)
{
    hal_public_hook_func(_tx_enable_peer_sta_ps_ctrl)(lut_index);
}

/*****************************************************************************
 * 功能描述  : 去使能sta ps ctrl寄存器
 *****************************************************************************/
static  INLINE__ osal_void hal_tx_disable_peer_sta_ps_ctrl(osal_u8 lut_index)
{
    hal_public_hook_func(_tx_disable_peer_sta_ps_ctrl)(lut_index);
}

/*****************************************************************************
 * 功能描述  : 禁能resp_ps_bit_ctrl
 *****************************************************************************/
static  INLINE__ osal_void hal_tx_disable_resp_ps_bit_ctrl(osal_u8 lut_index)
{
    hal_public_hook_func(_tx_disable_resp_ps_bit_ctrl)(lut_index);
}
#endif
#ifdef _PRE_WLAN_FEATURE_WUR_TX
static  INLINE__ osal_void hal_vap_send_wur_beacon_pkt(wur_bcn_reg_param_stru *wur_bcn_param)
{
    hal_public_hook_func(_vap_send_wur_beacon_pkt)(wur_bcn_param);
}

static  INLINE__ osal_void hal_vap_set_wur_twbtt(osal_u16 wur_offset_twbtt, osal_u8 twbtt_en)
{
    hal_public_hook_func(_vap_set_wur_twbtt)(wur_offset_twbtt, twbtt_en);
}

static  INLINE__ osal_void hal_vap_set_wur_beacon_en(osal_u8 wur_bcn_data_rate, osal_u8 wur_bcn_vap_en)
{
    hal_public_hook_func(_vap_set_wur_beacon_en)(wur_bcn_data_rate, wur_bcn_vap_en);
}

static  INLINE__ osal_void hal_write_wur_beacon(osal_u16 id)
{
    hal_public_hook_func(_write_wur_beacon)(id);
}

static  INLINE__ osal_void hal_write_wur_fl_wake(wur_wakeup_reg_param_stru *wur_wakeup_param)
{
    hal_public_hook_func(_write_wur_fl_wake)(wur_wakeup_param);
}

static  INLINE__ osal_void hal_write_wur_short_wake(osal_u16 wur_id)
{
    hal_public_hook_func(_write_wur_short_wake)(wur_id);
}

static  INLINE__ osal_void hal_vap_set_wur(const hal_to_dmac_vap_stru *hal_vap, wur_reg_param_stru *wur_param,
    osal_u8 count)
{
    hal_public_hook_func(_vap_set_wur)(hal_vap, wur_param, count);
}
#endif

#endif /* end of hal_power_save.h */

