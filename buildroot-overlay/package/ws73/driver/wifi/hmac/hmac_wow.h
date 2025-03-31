/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: WOW hmac function.
 * Create: 2021-04-15
 */

#ifndef __HMAC_WOW_H__
#define __HMAC_WOW_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "wlan_types_common.h"
#include "mac_vap_ext.h"
#include "frw_hmac.h"
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
#include "hmac_arp_offload.h"
#include "hmac_dhcp_offload.h"
#include "hmac_rekey_offload.h"
#endif
#include "msg_wow_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WOW_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WOW_MAX_SIZE (32 * 1024)
#define WOW_MAX_BLOCK_NUM 32
#define WOW_BLOCK_SIZE 1024
#define WOW_SEND_RETRY 3

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_HOST_STATE_INVALID,
    HMAC_HOST_STATE_WAKEUP,
    HMAC_HOST_STATE_SLEPT,
    HMAC_HOST_STATE_BUTT
} hmac_host_sleep_state_enum;
typedef osal_u8 hmac_host_sleep_state_enum_uint8;

typedef enum {
    MAC_WOW_PATTERN_PARAM_OPTION_ADD,
    MAC_WOW_PATTERN_PARAM_OPTION_DEL,
    MAC_WOW_PATTERN_PARAM_OPTION_CLR,
    MAC_WOW_PATTERN_PARAM_OPTION_BUTT
} mac_wow_pattern_param_option_enum;

typedef enum {
    MAC_WOW_NOTIFY_MEM_RSV,
    MAC_WOW_NOTIFY_MEM_RESTORE,
    MAC_WOW_NOTIFY_LOAD,
    MAC_WOW_NOTIFY_BUTT
} mac_wow_autoload_msg_enum;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u16   pattern_option;
    osal_u16   pattern_index;
    osal_u32   pattern_len;
    osal_u8    pattern_value[WOW_NETPATTERN_MAX_LEN];
} hmac_cfg_wow_pattern_param_stru;

typedef struct {
    hmac_host_sleep_state_enum_uint8  host_sleep_state;
    osal_u8                           wakeup_reason;
    osal_u8                           resv[2];
    mac_wow_cfg_stu                   wow_cfg;
} hmac_wow_info_stru;

typedef void (*pm_wow_wkup_cb)(void);

/*****************************************************************************
  10 函数声明
*****************************************************************************/

static osal_u32 hmac_wow_init_weakref(osal_void) __attribute__ ((weakref("hmac_wow_init"), used));
static osal_void hmac_wow_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_wow_deinit"), used));
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
extern osal_void pm_wifi_wkup_cb_host_register(pm_wow_wkup_cb cb);
#endif
osal_s32 hmac_config_wow_activate_switch(hmac_vap_stru *hmac_vap, osal_u32 wow_en);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __HMAC_WOW_H__ */