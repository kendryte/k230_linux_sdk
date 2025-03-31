/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: the head file for hmac rx data filter
 * Create: 2022-5-11
 */

#ifndef __HMAC_RX_DATA_FILTER_H__
#define __HMAC_RX_DATA_FILTER_H__

#include "mac_vap_ext.h"
#include "oal_net.h"
#include "dmac_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void hmac_rx_data_user_is_null(hmac_vap_stru *hmac_vap, mac_ieee80211_frame_stru *frame_hdr);
dmac_rx_frame_ctrl_enum_uint8 hmac_rx_deal_frame_filter(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

