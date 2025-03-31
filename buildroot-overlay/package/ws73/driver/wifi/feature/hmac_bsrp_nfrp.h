/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac bsrp nfrp
 * Create: 2023-2-17
 */

#ifndef HMAC_BSRP_NFRP_H
#define HMAC_BSRP_NFRP_H

#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define BSRP_CALC_TIMEOUT  200 /* 200ms */
#define SF0_MIN_SIZE (0)
#define SF1_MIN_SIZE (1024)
#define SF2_MIN_SIZE (17408)
#define SF3_MIN_SIZE (148480)
#define SF3_MAX_SIZE (2147328)
#define SF0_SIZE_UNITS (16)
#define SF1_SIZE_UNITS (256)
#define SF2_SIZE_UNITS (2048)
#define SF3_SIZE_UNITS (32768)
#define UNSCALED_VALUE0 (0)
#define UNSCALED_VALUE_MAX (62)
#define UNSCALED_VALUE_UNKNOWN (63)
#define SCALING_FACTOR0 (0)
#define SCALING_FACTOR1 (1)
#define SCALING_FACTOR2 (2)
#define SCALING_FACTOR3 (3)
#define SF_OFFSET (6)

/* TID调试相关的结构体 */
typedef struct {
    osal_u8     tid;
    osal_u8     resv[3];
    osal_u32    buffer_size;
} mac_tid_dbg_stru;

typedef struct {
    frw_timeout_stru buffer_status_timer;
    osal_u32 ac_queue_size;
    osal_u8 sta_vap_id;
    osal_u8 resv[3]; /* 保留3字节对齐 */
    /* 保留2字节对齐 */
    osal_u32 tid_dbg_size[WLAN_TID_MAX_NUM];
} mac_bsrp_info_stru;

typedef struct {
    osal_u8 res_req_buf_threshold_exponent;
    osal_u8 ndp_status;
    osal_u8 res[2];
} mac_nfrp_info_stru;

typedef osal_void (*hmac_nfrp_sta_update_param_cb)(osal_u8 *payload, osal_u16 msg_len, hmac_vap_stru *hmac_vap,
    osal_u8 frame_sub_type, hmac_user_stru *hmac_user);

static osal_u32 hmac_bsrp_nfrp_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_bsrp_nfrp_init"), used));
static osal_void hmac_bsrp_nfrp_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_bsrp_nfrp_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
