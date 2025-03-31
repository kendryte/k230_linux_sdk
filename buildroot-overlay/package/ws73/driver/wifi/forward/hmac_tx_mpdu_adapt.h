/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Create: 2022-05-09
 */
#ifndef __HMAC_TX_MPDU_ADAPT_H__
#define __HMAC_TX_MPDU_ADAPT_H__

#include "hal_common_ops.h"
#include "mac_vap_ext.h"
#include "frw_msg_rom.h"
#include "dmac_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    hmac_vap_stru *hmac_vap;
    hmac_user_stru *user;
    mac_tx_ctl_stru *tx_ctl;
} hmac_tx_data_stat_stru;


/*****************************************************************************
 函 数 名  : hmac_tx_tid_lock_bh
 功能描述  : 使用锁保护所有tid队列资源
*****************************************************************************/
static INLINE__ osal_void hmac_tx_tid_lock_bh(hmac_device_stru *hmac_device)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    osal_spin_lock_bh(&hmac_device->lock);
#else
    unref_param(hmac_device);
    osal_adapt_kthread_lock();
#endif
}

/*****************************************************************************
 函 数 名  : hmac_tx_tid_unlock_bh
 功能描述  : 释放保护所有tid队列资源的锁
*****************************************************************************/
static INLINE__ osal_void hmac_tx_tid_unlock_bh(hmac_device_stru *hmac_device)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    osal_spin_unlock_bh(&hmac_device->lock);
#else
    unref_param(hmac_device);
    osal_adapt_kthread_unlock();
#endif
}

static INLINE__ osal_void hmac_tx_excp_free_netbuf(oal_netbuf_stru *net_buffer)
{
    oal_netbuf_stru *netbuf = net_buffer;
    oal_netbuf_stru *buf_next = OSAL_NULL;

    while (netbuf != OSAL_NULL) {
        buf_next = oal_get_netbuf_next(netbuf);
        oal_netbuf_free(netbuf);
        netbuf = buf_next;
    }
}

#ifdef _PRE_WLAN_FEATURE_ROAM
/*****************************************************************************
 函 数 名  : hmac_tx_enable_in_roaming
 功能描述  : 判断漫游状态下，是否可以发送帧
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 hmac_tx_enable_in_roaming(const hal_to_dmac_device_stru *hal_device,
    const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        if (hal_device->current_chan_number != hmac_vap->channel.chan_number) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_tx_is_tx_force
 功能描述  : 判断是否常发模式
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 hmac_tx_is_tx_force(const hmac_vap_stru *hmac_vap)
{
    /* 常发下返回状态OK */
    if ((hmac_vap->al_tx_flag == EXT_SWITCH_ON) ||
        (hmac_vap->al_rx_flag == EXT_SWITCH_ON)) {
        return OSAL_TRUE;
    }
#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态时，强制发送该数据帧 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        return OSAL_TRUE;
    }
#endif

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt_get_txop_para
 功能描述  : 设置管理帧txop 参数
*****************************************************************************/
static INLINE__ osal_void hmac_tx_mgmt_get_txop_para(hmac_vap_stru *hmac_vap,
    hal_tx_txop_alg_stru **txop_alg, const mac_tx_ctl_stru *tx_ctl)
{
    if (tx_ctl->ismcast == OSAL_TRUE) {
        /* 获取组播、广播管理帧 发送参数 */
        *txop_alg = &(hmac_vap->tx_mgmt_bmcast[hmac_vap->channel.band]);
    } else {
        /* 获取单播 管理帧 发送参数 */
        *txop_alg = &(hmac_vap->tx_mgmt_ucast[hmac_vap->channel.band]);
    }
}

osal_u32 hmac_tx_update_protection_txop_alg(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    hal_tx_txop_alg_stru *txop_alg, osal_u8 do_default_cfg);
osal_u32 hmac_tx_process_data(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);

osal_void hmac_tx_send_mpdu(hmac_vap_stru *hmac_vap, hcc_msg_type type,
    oal_netbuf_stru *net_buffer, osal_u8 mpdu_num);
osal_void hmac_tx_data_send(hmac_tx_data_stat_stru *tx_data, oal_netbuf_head_stru *buff_head);
osal_void hmac_tx_fill_first_cb_info(mac_tx_ctl_stru *tx_ctl, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
