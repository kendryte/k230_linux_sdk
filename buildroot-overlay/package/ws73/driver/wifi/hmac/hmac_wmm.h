/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_wmm.h
 * 生成日期   : 2022年9月21日
 * 功能描述   : hmac_wmm.c的头文件
 */


#ifndef __HMAC_WMM_H__
#define __HMAC_WMM_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_netbuf_ext.h"
#include "hmac_vap.h"
#include "hmac_user.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WMM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/

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
    osal_u32 ac;
    osal_u32 value;
} hmac_config_edca_param;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_config_wmm_switch(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 wmm);
osal_s32 hmac_config_open_wmm_cb(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_config_set_wmm_register(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 wmm);
osal_void  hmac_config_set_machw_wmm(const hmac_vap_stru *hmac_vap);
osal_void  hmac_config_set_wmm_open_cfg(const mac_wme_param_stru  *wmm);
osal_void  hmac_config_set_wmm_close_cfg(const mac_wme_param_stru *wmm);
osal_void hmac_set_ac_mode(mac_tx_ctl_stru *tx_ctl);
osal_s32 hmac_config_get_wmm_params(hmac_vap_stru *hmac_vap, hmac_config_wmm_para_stru *cfg_stru);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
osal_s32 hmac_config_set_qap_aifsn(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user);
osal_s32 hmac_config_set_qap_cwmax(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user);
osal_s32 hmac_config_set_qap_cwmin(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user);
osal_s32 hmac_config_set_qap_msdu_lifetime(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user);
osal_s32 hmac_config_set_qap_txop_limit(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user);
#endif
osal_u32 hmac_wmm_init(osal_void);
osal_void hmac_wmm_deinit(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_wmm.h */
