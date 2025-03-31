/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: the head file for hmac rx data msg process
 * Create: 2022-5-10
 */


#ifndef __HMAC_RX_DATA_EVENT_H__
#define __HMAC_RX_DATA_EVENT_H__

#include "mac_vap_ext.h"
#include "frw_msg_rom.h"
#include "oal_net.h"
#include "dmac_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_RX_DATA_H

osal_s32 hmac_rx_data_event_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_rx_process_data_msg(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_rx_process_data_msg_process(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_rx_data_event.h */
