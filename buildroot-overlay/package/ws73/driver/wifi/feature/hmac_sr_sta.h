/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description:
 * Date: 2020-07-07
 */

#ifndef HMAC_SR_STA_H
#define HMAC_SR_STA_H

#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SRG_GEARS_ZERO (-81)
#define SRG_GEARS_ONE (-77)
#define SRG_GEARS_TWO (-73)
#define SRG_GEARS_THREE (-67)
#define NON_SRG_GEARS_ZERO (-81)
#define NON_SRG_GEARS_ONE (-77)
#define NON_SRG_GEARS_TWO (-73)
#define NON_SRG_GEARS_THREE (-67)
typedef struct {
    osal_s32 pd_lv_non;
    osal_s32 pd_lv;
    osal_s32 pd_max_non;
    osal_s32 pd_min;
    osal_s32 pd_max;
    osal_s32 tx_non;
    osal_s32 tx;
} mac_vap_he_sr_pd;

typedef struct {
    osal_u8 enable_flag;
    osal_u8 offset;
    osal_u8 resv[2];
    osal_u8 srg_bss_color_bitmap[8];     /* 8 */
    osal_u8 srg_partial_bssid_bitmap[8]; /* 8 */
    osal_s32 srg_pd;
    osal_s32 non_srg_pd;
    mac_vap_he_sr_pd sr_pd_info;
    mac_frame_he_spatial_reuse_parameter_set_ie_stru sr_ie_info;
} mac_vap_he_sr_config_info_stru;

typedef struct _mac_sta_sr_handler {
    osal_u8 enable_flag : 1;
    osal_u8 enable_flag_psr : 1;
    osal_u8 enable_sr_vfs : 1;
    osal_u8 enable_hpv : 1;

    osal_u8 vfs_state : 1;
    osal_u8 vfs_state1 : 1;
    osal_u8 resv : 2;

    osal_u8 resv1[3]; /* 3BYTE保留字段 */

    frw_timeout_stru sr_obss_timer;
    frw_timeout_stru psr_timer;
    frw_timeout_stru sr_vfs_timer;
    osal_u32 sr_obss_timeout;
    osal_u32 sr_psr_timeout;
    osal_u32 sr_vfs_timeout;
} mac_sta_sr_handler;

typedef struct {
    mac_sta_sr_handler sr_handler;
    mac_vap_he_sr_config_info_stru he_sr_config_info;
} hmac_sta_sr_stru;

typedef osal_void (*hmac_sta_up_process_spatial_reuse_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 frame_len, osal_bool is_need_enable);

static osal_u32 hmac_sta_sr_init_weakref(osal_void) __attribute__ ((weakref("hmac_sta_sr_init"), used));
static osal_void hmac_sta_sr_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_sta_sr_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
