/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * 文 件 名   : hmac_roam_if.h
 * 生成日期   : 2023年2月22日
 * 功能描述   : 漫游特性对外接口声明头文件
 */

#ifndef __HMAC_ROAM_IF_H__
#define __HMAC_ROAM_IF_H__

#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "hmac_roam_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef osal_u32 (*hmac_roam_start_etc_cb)(hmac_vap_stru *hmac_vap, roam_channel_org_enum scan_type,
    oal_bool_enum_uint8 cur_bss_ignore, roam_trigger_enum_uint8 roam_trigger);
typedef hmac_roam_info_stru *(*hmac_get_roam_info_cb)(osal_u8 vap_id);
typedef osal_u32 (*hmac_roam_proc_connect_cmd_cb)(hmac_vap_stru *hmac_vap, mac_conn_param_stru *connect_param);
typedef osal_u32 (*hmac_roam_check_bkscan_result_etc_cb)(hmac_vap_stru *hmac_vap,
    hmac_scan_record_stru *scan_record);
typedef osal_void (*hmac_roam_add_key_done_etc_cb)(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 pairwise);
typedef osal_void (*hmac_sta_roam_update_pmf_etc_cb)(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *mac_bss_dscr);
typedef osal_void (*hmac_sta_roam_trigger_event_send_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_roam_change_app_ie_cb)(hmac_vap_stru *hmac_vap,
    app_ie_type_uint8 *app_ie_type, const osal_u8 *curr_bssid);
typedef osal_void (*hmac_roam_update_user_mac_cb)(hmac_vap_stru *hmac_vap,
    hmac_ctx_join_req_set_reg_stru *reg_params);
typedef osal_void (*hmac_roam_set_linkloss_done_cb)(osal_u8 vap_id, osal_u8 val);

static osal_u32 hmac_roam_sta_init_weakref(osal_void) __attribute__ ((weakref("hmac_roam_sta_init"), used));
static osal_void hmac_roam_sta_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_roam_sta_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
