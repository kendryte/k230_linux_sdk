/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal util api.
 * Author: Huanghe
 * Create: 2021-07-16
 */

#ifndef __WAL_LINUX_UTIL_H__
#define __WAL_LINUX_UTIL_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_common.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_UTIL_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WAL_IOCTL_PRIV_SUBCMD_MAX_LEN 20

#define WAL_NETDEV_USER_MAX_NUM             (WLAN_ACTIVE_USER_MAX_NUM + 4)
#define WAL_NETDEV_SUBQUEUE_PER_USE         4
#define WAL_NETDEV_SUBQUEUE_MAX_NUM         ((WAL_NETDEV_USER_MAX_NUM) * (WAL_NETDEV_SUBQUEUE_PER_USE))

typedef enum {
    WLAN_WIFI_RECORD_ATCMD,      /* uapi_atcmdsrv_wifi_priv_cmd_etc */
    WLAN_WIFI_RECORD_IWINT,      /* uapi_iwpriv_get_param */
    WLAN_WIFI_RECORD_IWCHAR,     /* uapi_iwpriv_get_param_char */
    WLAN_WIFI_RECORD_WITPCMD,    /* uapi_witp_wifi_priv_cmd */

    WLAN_WIFI_HIPRIV_CMD,        /* g_ast_ccpriv_cmd */
    WLAN_WIFI_HIPRIV_DEBUG_CMD,  /* g_ast_ccpriv_cmd_debug_etc */
    WLAN_WIFI_ANDROID_CMD,       /* g_android_cmd */
    WLAN_WIFI_ONE_INT_ARGS,      /* g_one_int_args */
    WLAN_WIFI_DEBUG_CMD,         /* g_cmd_debug */

    WLAN_WIFI_CFG80211_OPS,      /* g_wal_cfg80211_ops */
    WLAN_WIFI_VENDER_CMDS,       /* wal_vendor_cmds */
    WLAN_WIFI_NETDEV_OPS,        /* g_st_wal_net_dev_ops_etc */
    WLAN_WIFI_IW_HANDLERS,       /* g_ast_iw_handlers */
    WLAN_WIFI_IW_PRIV_HANDLERS,  /* g_ast_iw_priv_handlers */

    WLAN_WIFI_RECORD_BUTT
} wal_wifi_record_log;

#ifdef _PRE_WLAN_DFT_STAT
osal_void wal_set_external_record_enable(osal_u8 enable_value);
#endif
#ifdef _PRE_WLAN_DFT_STAT
osal_void wal_record_external_log_by_id(osal_u8 type, osal_u32 id);
osal_void wal_record_external_log_by_name(osal_u8 type, osal_u8 *name, osal_u32 ret);
osal_void wal_record_wifi_external_log(osal_u8 type, const osal_s8 *name);
#else
#define wal_record_external_log_by_id(type, id)
#define wal_record_external_log_by_name(type, name, ret)
#define wal_record_wifi_external_log(type, name)
#endif
extern oal_iw_handler_def_stru g_st_iw_handler_def_etc;

typedef struct {
    osal_u32 ap_max_user;                      /* ap最大用户数 */
    osal_s8   ac_ap_mac_filter_mode[257];          /* AP mac地址过滤命令参数,最长256 */
    osal_s32  l_ap_power_flag;                     /* AP上电标志 */
} wal_ap_config_stru;

osal_u32 wal_get_parameter_from_cmd(osal_s8 *pc_cmd, osal_s8 *pc_arg, OAL_CONST osal_s8 *token,
    osal_u32 *pul_cmd_offset, osal_u32 param_max_len);
osal_u32  wal_post2hmac_uint32_data(oal_net_device_stru *net_dev, osal_s8 *param, osal_u16 msg_id);
osal_s32 wal_set_random_mac_to_mib_etc(oal_net_device_stru *net_dev);
osal_s32 wal_util_set_country_code(oal_net_device_stru *net_dev, osal_s8 *country_code);
osal_s32 wal_get_init_state_vap(oal_net_device_stru *net_dev, hmac_vap_stru **hmac_vap);

osal_s32 wal_cfg80211_start_connect_etc(oal_net_device_stru *net_dev,
    mac_conn_param_stru *connect_param);
osal_s32 wal_cfg80211_start_disconnect_etc(oal_net_device_stru *net_dev,
    mac_cfg_kick_user_param_stru *disconnect_param);
osal_void wal_connect_set_auth_type(osal_u8 orgin_type, osal_u8 *new_type);
extern osal_u32 get_dev_addr(osal_u8 *pc_addr, osal_u8 addr_len, osal_u8 type);
extern osal_u32 set_dev_addr(osal_u8 *pc_addr, osal_u8 mac_len, osal_u8 type);
osal_u8 bw_enum_to_number(osal_u8 bw);
osal_u32 tid_stat_to_user(osal_u32 *stat);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
