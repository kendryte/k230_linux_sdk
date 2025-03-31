/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: hmac_p2p.c 的头文件.
 */

#ifndef __HMAC_P2P_H__
#define __HMAC_P2P_H__

#include "hmac_fsm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_P2P_H

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_P2P_WILDCARD_SSID      "DIRECT-"
#define HMAC_P2P_WILDCARD_SSID_LEN  7

#define P2P_NOA_DESC_NUM    1


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* p2p 状态码 */
typedef enum {
    P2P_STATUS_DISCOVERY_ON = 0,
    P2P_STATUS_SEARCH_ENABLED,
    P2P_STATUS_IF_ADD,
    P2P_STATUS_IF_DEL,
    P2P_STATUS_IF_DELETING,
    P2P_STATUS_IF_CHANGING,
    P2P_STATUS_IF_CHANGED,
    P2P_STATUS_LISTEN_EXPIRED,
    P2P_STATUS_ACTION_TX_COMPLETED,
    P2P_STATUS_ACTION_TX_NOACK,
    P2P_STATUS_SCANNING,
    P2P_STATUS_GO_NEG_PHASE,
    P2P_STATUS_DISC_IN_PROGRESS
} hmac_cfgp2p_status_enum;
typedef osal_u32 hmac_cfgp2p_status_enum_uint32;

typedef enum {
    P2P_PM_AWAKE_STATE = 0,
    P2P_PM_SLEEP_STATE = 1
} p2p_pm_state_enum;

/*****************************************************************************
  5 内联函数
*****************************************************************************/
static INLINE__ osal_u8 is_p2p_wildcard_ssid(const osal_u8 *ssid, osal_u8 len)
{
    return ((len == HMAC_P2P_WILDCARD_SSID_LEN) &&
        !oal_memcmp(ssid, HMAC_P2P_WILDCARD_SSID, HMAC_P2P_WILDCARD_SSID_LEN));
}

static INLINE__ osal_u8 is_p2p_oppps_enabled(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_ops_param.ops_ctrl != 0);
}

static INLINE__ osal_u8 is_p2p_noa_enabled(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_noa_param.count != 0);
}

static INLINE__ osal_u8 is_p2p_ps_enabled(const hmac_vap_stru *hmac_vap)
{
    return (is_p2p_oppps_enabled(hmac_vap) || is_p2p_noa_enabled(hmac_vap));
}

static INLINE__ osal_u16 get_attr_len(const osal_u8 *buf)
{
    return oal_make_word16(buf[1], buf[2]);
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
wlan_p2p_mode_enum_uint8 hmac_get_p2p_mode_etc(const hmac_vap_stru *hmac_vap);
void hmac_dec_p2p_num_etc(hmac_vap_stru *hmac_vap);
void hmac_inc_p2p_num_etc(hmac_vap_stru *hmac_vap);
osal_s32 hmac_set_p2p_common_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u8 hmac_get_p2p_protocol_info(osal_void);
osal_u8 hmac_get_p2p_common_band_info(osal_void);
osal_u8 hmac_get_p2p_common_enable_info(osal_void);
extern osal_u32 hmac_p2p_send_listen_expired_to_host_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_p2p_send_listen_expired_to_device_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_p2p_check_can_enter_state_etc(hmac_vap_stru *hmac_vap,
    hmac_fsm_input_type_enum_uint8 input_req);
extern osal_void hmac_disable_p2p_pm_etc(hmac_vap_stru *hmac_vap);
osal_void mac_set_p2p_none_noa(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_u8  hmac_p2p_listen_rx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
oal_bool_enum_uint8 hmac_is_p2p_go_neg_req_frame(const osal_u8* data);
oal_bool_enum_uint8 hmac_is_p2p_pd_disc_req_frame(const osal_u8 *data);
oal_bool_enum_uint8 hmac_is_p2p_presence_req_frame(const osal_u8 *data);
osal_u32  hmac_p2p_listen_filter_vap(const hmac_vap_stru *hmac_vap);
osal_u32  hmac_p2p_listen_filter_frame(const hmac_vap_stru *hmac_vap, osal_u8 *frame_body, osal_u16 frame_len);
osal_void hmac_process_p2p_noa(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_u8  *hmac_get_p2p_noa_attr(osal_u8 *frame_body, osal_u16 rx_len, osal_u16 tag_param_offset, osal_u16 *attr_len);
osal_void hmac_p2p_oppps_ctwindow_start_event(hmac_vap_stru *hmac_vap);
osal_u32 hmac_send_p2p_state_to_device(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 pause, wlan_vap_mode_enum_uint8 mode);
oal_bool_enum_uint8 hmac_is_p2p_action_frame(const osal_u8 *data);
osal_s32 hmac_p2p_opps_info_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
oal_bool_enum_uint8 hmac_is_p2p_ie(const osal_u8 *data);
osal_void  mac_set_p2p0_ssid_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u16 frm_type);
osal_void mac_set_p2p_noa(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *e_len);
osal_void mac_set_p2p_status(osal_u8 *buffer, osal_u8 *ie_len, p2p_status_code_t status);
osal_void hmac_p2p_handle_ps(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 pause);
osal_u32 hmac_process_p2p_presence_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_u16 hmac_mgmt_encap_p2p_presence_rsp(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *ra, const osal_u8 *data);
osal_u32 hmac_p2p_event_sync2device(hmac_vap_stru *hmac_vap, osal_u8 type);
extern osal_u8 hmac_get_p2p_status_etc(osal_u32 p2p_status, hmac_cfgp2p_status_enum_uint32 status);
extern osal_void hmac_set_p2p_status_etc(osal_u32 *pul_p2p_status, hmac_cfgp2p_status_enum_uint32 status);
osal_s32 hmac_config_cancel_remain_on_channel(hmac_vap_stru *hmac_vap);
osal_s32 hmac_config_set_p2p_ps_noa(hmac_vap_stru *hmac_vap, mac_cfg_p2p_noa_param_stru *p2p_noa);
osal_s32 hmac_config_set_p2p_ps_ops(hmac_vap_stru *hmac_vap, mac_cfg_p2p_ops_param_stru *p2p_ops);
osal_s32 hmac_config_set_p2p_noa_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_p2p_remain_on_channel_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param);
osal_u32 hmac_p2p_listen_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
osal_s32 hmac_config_remain_on_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_cancel_remain_on_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif  /* _PRE_WLAN_FEATURE_P2P */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_p2p.h */
