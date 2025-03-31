/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: msg.
 * Create: 2021-11-29
 */

#ifndef MSG_TWT_ROM_H
#define MSG_TWT_ROM_H

#ifdef _PRE_WLAN_FEATURE_TWT
typedef struct {
    osal_u8 twt_session_status;
    osal_u16 user_idx;
    osal_u8 twt_ps_pause;
} mac_d2hd_twt_cfg_stru;

typedef struct {
    osal_u32 cfg_type;
    mac_d2hd_twt_cfg_stru twt_cfg;
} mac_d2hd_twt_sync_info_stru;

/* 修改参数 */
typedef struct {
    mac_twt_update_source_enum_uint8 update_source;
    osal_u16 user_idx;
    osal_u8 resv;
    mac_cfg_twt_stru twt_cfg;
} mac_ctx_update_twt_stru;
#endif
#endif