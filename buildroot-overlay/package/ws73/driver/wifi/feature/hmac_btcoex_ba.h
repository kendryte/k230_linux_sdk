/*
 * Copyright: Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_btcoex_ba.c的头文件
 * Date: 2023-02-09 09:51
 */
#ifndef __HMAC_BTCOEX_BA_H__
#define __HMAC_BTCOEX_BA_H__

#include "hmac_btcoex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  8 ROM预留回调函数类型定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_BTCOEX
osal_void hmac_btcoex_update_ba_size(const hmac_vap_stru *hmac_vap,
    hmac_user_btcoex_delba_stru *btcoex_delba, const hal_btcoex_btble_status_stru *btble_status);
osal_u32 hmac_btcoex_delba_event_process(const hal_btcoex_btble_status_stru *btble_status,
    hmac_vap_stru *hmac_vap);
osal_void hmac_btcoex_delba_trigger(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_delba,
    hmac_user_btcoex_delba_stru *btcoex_delba);
osal_s32 hmac_btcoex_action_dela_ba_handler(hmac_vap_stru *hmac_vap);
osal_void hmac_btcoex_tx_addba_rsp_check(const oal_netbuf_stru *netbuf, hmac_user_stru *hmac_user);
#endif

typedef osal_u32 (*hmac_btcoex_check_by_ba_size_etc_cb)(hmac_user_stru *hmac_user);
typedef osal_void (*hmac_btcoex_adjust_addba_rsp_basize_cb)(hmac_vap_stru *hmac_vap,
    hmac_ba_rx_stru *addba_rsp);

osal_u32 hmac_btcoex_ba_init(osal_void);
osal_void hmac_btcoex_ba_deinit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_BA_H__ */