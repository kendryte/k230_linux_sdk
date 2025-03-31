/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: STA侧管理帧处理头文件.
 */

#ifndef __HMAC_11I_H__
#define __HMAC_11I_H__
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "hmac_user.h"
#include "soc_diag_wdk.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11I_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_UPDATE_GTK_TIMEOUT 2000

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u8 vap_id;
    osal_u8 key_idx;
    wlan_ciper_protocol_type_enum_uint8 cipher_type;
    osal_u8 resv;
} hmac_add_gtk_notify_stru;

typedef hmac_add_gtk_notify_stru hmac_del_gtk_notify_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_check_igtk_exist(osal_u8 igtk_index);
osal_u32 hmac_11i_add_key_from_user(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

osal_void hmac_11i_tkip_mic_failure_handler(const hmac_vap_stru *hmac_vap, const osal_u8 *user_mac,
    oal_nl80211_key_type key_type);
osal_u32 hmac_reset_gtk_token(const hmac_vap_stru *hmac_vap);

osal_u32 hmac_init_security_etc(hmac_vap_stru *hmac_vap, const osal_u8 *addr);
osal_void hmac_sta_protocol_down_by_chipher(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr);
osal_u32 hmac_en_mic_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *pst_netbuf,
    osal_u8 *iv_len);
osal_u32 hmac_de_mic_etc(hmac_user_stru *hmac_user, oal_netbuf_stru *pst_netbuf);
osal_u32 hmac_11i_ether_type_filter_etc(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u16 ether_type);
osal_u32 hmac_config_11i_add_key_set_reg(hmac_vap_stru *hmac_vap, osal_u8 pairwise,
    osal_u8 key_index, hmac_user_stru *hmac_user);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
