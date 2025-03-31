/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_BA_MANAGE_ROM_H
#define MSG_BA_MANAGE_ROM_H

#include "osal_types.h"

typedef osal_u8 mac_ba_conn_status_enum_uint8;

typedef struct {
    osal_u16 baw_size;
    osal_u16 ba_timeout;
    osal_u16 mac_user_idx;
    osal_u8 tidno;
    oal_bool_enum_uint8 is_ba;
    mac_ba_conn_status_enum_uint8 ba_conn_status;
    mac_back_variant_enum_uint8 back_var;
    osal_u8 dialog_token;
    osal_u8 ba_policy;
} mac_mgmt_tx_addba_req_ba_sync;

typedef struct {
    osal_u16 assoc_id;
    osal_u16 baw_start;
    osal_u16 baw_size;
    osal_u8 tidno;
    mac_ba_conn_status_enum_uint8 ba_conn_status;
    osal_u8 lut_index;
    mac_ba_policy_enum_uint8 ba_policy;
    wlan_protocol_enum_uint8 protocol;
    osal_u8 protocol_mode : 4;
    osal_u8 ba_32bit_flag : 4;
} mac_mgmt_tx_addba_rsp_ba_sync;

typedef struct {
    osal_u16 assoc_id;
    osal_u16 baw_size;
    osal_u16 ba_timeout;
    osal_u8 tidno;
    osal_u8 status;
    osal_u8 dialog_token;
    osal_u8 ba_policy;
    osal_u8 protocol_mode;
    osal_u8 max_rx_ampdu_factor;
    osal_u8 max_ampdu_len_exp;
    osal_u8 min_mpdu_start_spacing;
    oal_bool_enum_uint8 amsdu_supp;
} mac_mgmt_rx_addba_rsp_ba_sync;

typedef struct {
    osal_u16 assoc_id;
    osal_u8 tidno;
    osal_u8 initiator : 4;
    osal_u8 is_tx : 4;
} mac_mgmt_delba_ba_sync;
#endif