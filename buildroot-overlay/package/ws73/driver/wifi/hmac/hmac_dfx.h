/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: dfx feature head file.
 */

#ifndef __HMAC_DFX_H__
#define __HMAC_DFX_H__

#include "oal_types.h"

#ifdef _PRE_WLAN_DFT_STAT
#include "osal_adapt.h"
#include "osal_list.h"
#include "wlan_types_common.h"
#include "oal_schedule.h"
#include "oal_skbuff.h"
#include "common_dft.h"
#include "hmac_device.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFX_H
/*****************************************************************************
  函数声明
*****************************************************************************/
#define MAX_CONN_INFO_RD_NUM            20
#define MAX_DFX_USER_NUM                16
#define MAX_DFX_USER_RD_LEN             512

#define DFX_SUCCESS         0
#define DFX_FAIL            1

#define DFX_OFFLINE_PERIOD   50
#define DFX_CONN_FAIL_PERIOD 10
#define DFX_CONN_FAIL_CNT    3

typedef enum {
    USER_CONN_FAIL = 0,
    USER_CONN_OFFLINE,
    USER_CONN_STA_STAT,
    USER_CONN_AP_STAT,

    USER_CONN_INFO_BUTT
} user_conn_info_enum;
typedef osal_u8 user_conn_info_enum_uint8;

typedef enum {
    STA_CONN_CANNT_FIND_AP = 1,
    STA_CONN_CHECK_BSS_DSCR_ERR,
    STA_CONN_SET_PARAM_ERR,
    STA_CONN_SYNC_CFG_ERR,
    STA_CONN_AUTH_FSM_ERR,
    STA_CONN_AUTH_RSP_ERR,
    STA_CONN_AUTH_TIMEOUT_ERR,
    STA_CONN_ASOC_FSM_ERR,
    STA_CONN_ASOC_RSP_ERR,
    STA_CONN_ASOC_TIMEOUT_ERR,

    AP_AUTH_RSP_ENCAP_ERR,
    AP_AUTH_FIND_USER_ERR,
    AP_AUTH_RSP_SEND_ERR,
    AP_ASOC_WAIT_TIMEOUT_ERR,           /* AP发送auth rsp后等待asoc req超时 */
    AP_ASOC_NEGO_CAPABILITY_ERR,
    AP_ASOC_RSP_ENCAP_ERR,
    AP_ASOC_RSP_SEND_ERR,

    USER_CONN_FAIL_BUTT
} user_conn_fail_reason_enum;
typedef osal_u16 user_conn_fail_reason_enum_uint16;

typedef enum {
    STA_RCV_AP_DEAUTH_OR_DISASOC = 1,
    STA_UP_OTHER_VAP_CLEAN_BSS,

    AP_RCV_STA_DEAUTH,
    AP_RCV_STA_DISASOC,
    AP_BLACKLIST_DEL_STA,
    AP_DOWN_DEL_STA,
    AP_AUTH_RSP_DEL_OTHER_STA,
    AP_CLEAN_BSS,                       /* P2P上线后清空user */
    AP_STA_NOUP_RCV_DATA,               /* sta未UP时收到数据帧，通知STA下线 */

    CFG_KICK_USER,

    PMF_SA_QUERY_TIMEOUT_DEL_USER,      /* 管理帧加密发送SA QUERY报文超时删除用户 */
    HMAC_DISASOC_NOTICE,                /* dmac去关联时通知hmac，包含dmac_disasoc_misc_reason_enum枚举定义的原因 */
    ROAM_CONNECTING_FAIL,               /* 漫游尝试连接失败后通知下线处理 */
    ROAM_HANDSHAKE_FAIL,                /* 漫游HANDSHAKE失败后通知下线处理 */

    USER_CONN_OFFLINE_BUTT
} user_conn_offline_reason_enum;
typedef osal_u16 user_conn_offline_reason_enum_uint16;

typedef enum {
    DFX_USER_CONN_INFO_RECORD_CMD_ENABLE,
    DFX_USER_CONN_INFO_RECORD_CMD_DISABLE,
    DFX_USER_CONN_INFO_RECORD_CMD_FAIL,
    DFX_USER_CONN_INFO_RECORD_CMD_OFFLINE,
    DFX_USER_CONN_INFO_RECORD_CMD_STATISTIC,
    DFX_USER_CONN_INFO_RECORD_CMD_STATCLR,
    DFX_USER_CONN_INFO_RECORD_CMD_BUTT
} dfx_user_conn_info_record_cmd_enum;
typedef osal_u8 dfx_user_conn_info_record_cmd_enum_u8;

typedef struct {
    osal_timeval    fail_time;
    osal_u16        fail_reason;
    osal_u16        reserved;
} unconn_reason_item;

typedef struct {
    osal_u8             user_rd_ind;
    osal_u8             user_rd_cnt;
    osal_u8             rev[2];
    unconn_reason_item  unconn_rd[MAX_CONN_INFO_RD_NUM];
} user_unconn_reason_rd;

typedef struct {
    osal_u32        stat[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT][WLAN_MGMT_SUBTYPE_BUTT];
} user_statistic_rd;

typedef struct {
    struct osal_list_head user_list;
    osal_u8         user_mac[WLAN_MAC_ADDR_LEN];
    osal_u8         vap_id;
    osal_u8         rev;
    osal_u8         user_rd[0];
} user_conn_info_rd;

typedef struct {
    struct osal_list_head   user_head;
    osal_void               *user_cache;
    osal_u32                user_num;
    osal_u32                type;
    osal_mutex              lock;
} user_conn_info_head;

typedef struct {
    osal_u8 beacon_switch;
    osal_u8 vip_frame_switch; /* 关键帧打印开关 */
    osal_u8 scan_frame_switch;
    osal_u8 reserved;
} special_frame_switch_stru;

/*****************************************************************************
  函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
osal_void dfx_set_service_control(osal_u32 value);
osal_u32 dfx_get_service_control(osal_void);
#endif

/*****************************************************************************
  对外函数声明
*****************************************************************************/
typedef osal_void (*dfx_user_conn_abnormal_record_cb)(osal_u8 vap_id, osal_u8 *mac, osal_u16 reason, osal_u16 type);
typedef osal_void (*dfx_user_conn_statistic_record_cb)(osal_u8 vap_id, osal_u8 *mac, osal_u8 sub_type,
    osal_u16 mode, osal_u8 direction);
typedef osal_void (*dfx_frame_report_hso_cb)(oal_netbuf_stru *netbuf, osal_u8 direction);
typedef osal_void (*dfx_user_rate_rx_info_record_cb)(osal_u8 id, dmac_rx_ctl_stru *cb_ctrl);
typedef osal_void (*dfx_scan_frame_report_hso_cb)(mac_bss_dscr_stru *bss_dscr);

static osal_u32 hmac_dfx_init_weakref(osal_void) __attribute__ ((weakref("hmac_dfx_init"), used));
static osal_void hmac_dfx_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_dfx_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_dfx.h */
