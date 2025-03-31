/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: header file of hmac_tx_mgmt.c
 * Create: 2022-5-16
 */

#ifndef __HMAC_TX_MGMT_H__
#define __HMAC_TX_MGMT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"
#include "oal_netbuf_ext.h"
#include "hal_common_ops.h"
#include "oal_netbuf_data.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_u32 hmac_tx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt, osal_u16 len, osal_bool data_ready);
osal_void hmac_clear_hw_fifo_tx_queue(hal_to_dmac_device_stru *hal_device);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tx_mgmt.h */
