/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 * Description: header file
 * Create: 2012年11月8日
 */

#ifndef __HMAC_TWT_H__
#define __HMAC_TWT_H__

#include "dmac_ext_if_hcm.h"
#include "hmac_fsm.h"
#include "msg_twt_rom.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_H

/* 1 其他头文件包含 */
/* 2 宏定义 */
#define HMAC_TWT_P2P_RUNNING_VAP_NUM    3
#define HMAC_TWT_CATEGORY 0
#define HMAC_TWT_ACTION 1
#define HMAC_TWT_FLOW_ID 2
#define HMAC_TWT_BITS_OFFSET 32
#define TWT_RX_PARA_NUM 11
#define WAKE_DURATION_UNIT0 256
#define WAKE_DURATION_UNIT1 1024
#define MAC_XCAPS_EX_TWT_LEN 10

/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    osal_u8 twt_responder_support : 1,
            twt_requester_support : 1,
            resv : 6;
} mac_device_twt_custom_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    mac_twt_information_field_stru twt_information_filed;
    osal_u8 resv[1]; /* 1 byte保留字段 */
} mac_cfg_twt_information_req_param_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 resv[2];                     /* 2 byte保留字段 */
    mac_cfg_twt_basic_param_stru twt_basic_param;
} mac_cfg_twt_setup_req_param_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 twt_flow_id;                 /* flow ID */
    osal_u8 resv;
} mac_cfg_twt_teardown_req_param_stru;

typedef struct {
    mac_cfg_twt_stru sta_cfg_twt_para;
    mac_device_twt_custom_stru sta_twt_custom_para;
    osal_u8 resv[3];
} sta_twt_para_stru;

/* 8 UNION定义 */

/* 3 枚举定义 */
typedef enum {
    TWT_SESSION_OFF = 0,
    TWT_SESSION_ON = 1,
    TWT_SESSION_SUSPEND = 2,
} hmac_twt_status_enum;

typedef enum {
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_0 = 0,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_32 = 1,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_48 = 2,
    HMAC_NEXT_TWT_SUBFIELD_SIZE_BITS_64 = 3,

    HMAC_NEXT_TWT_SUBFIELD_SIZE_MAX
} hmac_next_twt_subfield_size_enum;

typedef enum {
    HMAC_TWT_START_EVENT = 0,
    HMAC_TWT_END_EVENT = 1,

    HMAC_TWT_EVENT_TYPE_MAX
} hmac_twt_event_type_enum;

/* need same as device define */
typedef enum {
    TWT_UPDATE_SESSION_CFG = 0,
    TWT_UPDATE_PS_PAUSE_CFG = 1,
    TWT_PS_HANDLE_CFG = 2,
} hmac_d2hd_twt_sync_type_enum;
typedef osal_u32 hmac_d2h_twt_sync_type_enum_u32;

/* 10 函数声明 */

typedef osal_u8 (*hmac_twt_is_session_enable_cb)(const hmac_vap_stru *hmac_vap);
typedef osal_bool (*hmac_is_twt_processed_not_need_buff_cb)(const hmac_vap_stru *hmac_vap);
typedef osal_bool (*hmac_is_twt_need_buff_cb)(hmac_vap_stru *hmac_vap);
typedef osal_u32 (*hmac_twt_tx_action_cb)(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf, const mac_tx_ctl_stru *tx_ctl);
typedef osal_u32 (*hmac_p2p_teardown_twt_session_cb)(hmac_device_stru *hmac_device);
typedef osal_void (*hmac_process_update_twt_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u16 msg_len);
typedef osal_u8 (*hmac_twt_get_req_bit_cb)(osal_u8 vap_id);
typedef osal_void (*hmac_set_ext_cap_twt_req_cb)(hmac_vap_stru *hmac_vap,
    mac_ext_cap_ie_stru *ext_cap_ie, osal_u8 *ie_len);

static osal_u32 hmac_twt_sta_init_weakref(osal_void) __attribute__ ((weakref("hmac_twt_sta_init"), used));
static osal_void hmac_twt_sta_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_twt_sta_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of hmac_twt.h */

