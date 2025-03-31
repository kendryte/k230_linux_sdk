/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * Description: wal_config.c 的头文件
 * Create: 2012年11月6日
 */

#ifndef __WAL_CONFIG_H__
#define __WAL_CONFIG_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "mac_vap_ext.h"
#include "frw_ext_if.h"

#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CONFIG_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WAL_MSG_WRITE_MSG_HDR_LENGTH   (OAL_SIZEOF(wal_msg_hdr_stru))

#define WAL_MSG_WRITE_MAX_LEN \
(WLAN_HMEM_EVENT_SIZE2 - WAL_MSG_WRITE_MSG_HDR_LENGTH)

#define WAL_BCAST_MAC_ADDR       255
#define WAL_MAX_RATE_NUM        16


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 配置消息类型 */
typedef enum {
    WAL_MSG_TYPE_QUERY,     /* 查询 */
    WAL_MSG_TYPE_WRITE,     /* 设置 */
    WAL_MSG_TYPE_RESPONSE,  /* 返回 */

    WAL_MSG_TYPE_BUTT
} wal_msg_type_enum;
typedef osal_u8 wal_msg_type_enum_uint8;


/* 速率集种类，常发使用 */
typedef enum {
    WAL_RF_TEST_11B_LEGACY_RATES,
    WAL_RF_TEST_20M_NORMAL_RATES,
    WAL_RF_TEST_20M_SHORT_GI_RATES,
    WAL_RF_TEST_40M_NORMAL_RATES,
    WAL_RF_TEST_40M_SHORT_GI_RATES,

    WAL_RF_TEST_RATES_BUTT
} wal_rf_test_enum;
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
/* 配置消息头 */
typedef struct {
    wal_msg_type_enum_uint8     msg_type;       /* msg type:W or Q */
    osal_u8                   msg_sn;         /* msg 序列号 */
    osal_u16                  msg_len;        /* msg 长度 */
} wal_msg_hdr_stru;

/* write消息格式 */
typedef struct {
    wlan_cfgid_enum_uint16  wid;
    osal_u16              len;
    osal_u8               auc_value[WAL_MSG_WRITE_MAX_LEN];
} wal_msg_write_stru;

/* response消息格式，与Write消息格式相同 */
typedef wal_msg_write_stru wal_msg_rsp_stru;

/* WMM SET消息格式 */
typedef struct {
    wlan_cfgid_enum_uint16      cfg_id;
    osal_u8                   resv[2];
    osal_u32                  ac;
    osal_u32                  value;
} wal_msg_wmm_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 宏定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void wal_cfg_init(osal_void);
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_void wal_cfg_debug_init(osal_void);
#endif
extern osal_u32  wal_config_get_wmm_params_etc(oal_net_device_stru *net_dev, osal_u8 *param);
extern osal_s32 wal_config_get_assoc_req_ie_etc(hmac_vap_stru *hmac_vap, osal_u16 *pus_len, osal_u8 *param);
extern osal_u32 wal_send_cali_data_etc(oal_net_device_stru *net_dev);

#ifdef _PRE_WLAN_FEATURE_DFS
extern osal_u32  wal_config_get_dfs_chn_status(oal_net_device_stru *net_dev, osal_u8 *param);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_config.h */
