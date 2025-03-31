/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Function related to wifi aware(NAN) feature.
 * Create: 2021-04-16
 */

#ifndef __WAL_CONFIG_SDP_H__
#define __WAL_CONFIG_SDP_H__

#include "hmac_ext_if.h"
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32 wal_config_sdp_init(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_start_publish(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_start_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_cancel_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_cancel_publish(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_send_data(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_retry_times(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_beacon_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_config_sdp_test(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_sdp_process_rx_data(hmac_vap_stru *hmac_vap, frw_msg *msg);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of wal_config_sdp.h */
