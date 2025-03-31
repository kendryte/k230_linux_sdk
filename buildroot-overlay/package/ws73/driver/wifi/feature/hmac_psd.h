/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_psd
 * Author:
 * Create: 2022-08-15
 */

#ifndef __HMAC_PSD_H__
#define __HMAC_PSD_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hmac_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PSD_H

#define DIAG_CMD_ID_PSD_ENABLE 0x71D0
#define DIAG_CMD_ID_PSD_REPORT 0x71D1
#define PSD_DATA_LEN 256
#define PSD_DEV_NAME "wlan0"
#define PSD_CENTER_CHAN 7 /* psd 中心信道固定7 */

#ifndef _PRE_WLAN_FEATURE_WS73
#define DIAG_OPTION diag_option_t
#else
#define DIAG_OPTION diag_option
#endif
/* HSO PSD消息 */
typedef struct {
    osal_bool enable;
    osal_u8 channel;
    osal_u8 band;
    osal_u32 cycle;
    osal_u16 duration;
} hmac_psd_enable_stru;

typedef struct {
    frw_timeout_stru psd_timer;
    osal_u32 sampling_count; /* 当前采样次数 */
    osal_u32 max_sampling_count; /* 最大采样次数 */
} hmac_psd_enable_timer;

osal_s32 hmac_config_psd_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_psd_data_report(hmac_vap_stru *hmac_vap, frw_msg *msg);
typedef osal_u8 (*hmac_psd_get_switch_cb)(void);

static osal_u32 hmac_psd_init_weakref(osal_void) __attribute__ ((weakref("hmac_psd_init"), used));
static osal_void hmac_psd_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_psd_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_psd.h */
