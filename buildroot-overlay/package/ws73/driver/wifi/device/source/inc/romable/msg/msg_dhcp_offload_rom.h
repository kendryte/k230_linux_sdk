/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_DHCP_OFFLOAD_ROM_H
#define MSG_DHCP_OFFLOAD_ROM_H
 
#include "mac_frame_common_rom.h"
 
typedef struct {
    osal_u8 cipher_key_type;
    osal_u8 cipher_protocol_type;
    osal_u8 cipher_key_id;
    osal_u8 ra_lut_index;
} mac_cipher_info_stru;
 
typedef struct {
    dhcp_record_frame dhcp_request;
    mac_ieee80211_frame_stru dhcp_request_header;
    osal_u32 lease_time;
    osal_u32 request_ip;
    osal_u32 server_ip;
    osal_u32 dhcp_offload_timeout;
    osal_u32 dhcp_renew_retry;
    mac_cipher_info_stru cipher_info;
} mac_dhcp_offload_param_sync;
 
#endif