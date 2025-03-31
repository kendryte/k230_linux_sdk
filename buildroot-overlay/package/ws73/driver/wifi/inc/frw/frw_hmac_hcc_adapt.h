/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc service adapt layer.
 * Author: Huanghe
 * Create: 2021-09-01
 */

#ifndef __FRW_HMAC_HCC_ADAPT_H
#define __FRW_HMAC_HCC_ADAPT_H

#include "osal_types.h"
#include "oal_net.h"
#include "frw_ext_if.h"
#include "frw_msg_rom.h"

osal_void hmac_frw_hcc_extend_hdr_init(osal_u16 data_len,
    struct frw_hcc_extend_hdr *hdr, osal_u8 vap_id, osal_u16 msg_id, osal_u8 msg_type);
osal_void *frw_alloc_hcc_cfg_data(osal_u32 size);
osal_void frw_free_hcc_cfg_data(osal_u8 *data);
osal_s32 frw_send_msg_adapt(osal_u16 msg_id, frw_msg *cfg, osal_u8 vap_id, osal_u32 msg_seq, osal_bool sync);
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
osal_u32 frw_start_hcc_service(hcc_service_type service_type, hcc_adapt_priv_rx_process rx_process);
osal_u32 frw_stop_hcc_service(hcc_service_type service_type);
osal_u32 frw_tx_wifi_netbuf_by_hcc(oal_netbuf_stru *netbuf, osal_u16 payload_len, osal_u8 data_type);
#else
osal_u32 frw_start_hcc_service(osal_u8 service_type, osal_void *rx_process);
osal_u32 frw_stop_hcc_service(osal_u8 service_type);
#endif

osal_void hcc_hmac_wifi_msg_register(osal_u8 msg_id, osal_void *callback);
osal_void hcc_hmac_config_bus_ini(osal_u8 *param);
#endif /* end of frw_hmac_hcc_adapt.h */
