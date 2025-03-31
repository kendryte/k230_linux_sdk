/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: hmac_blacklist.c 的头文件
 */

#ifndef HMAC_BLAKLIST_H
#define HMAC_BLAKLIST_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define CS_INVALID_AGING_TIME         0
#define CS_DEFAULT_AGING_TIME         3600
#define CS_DEFAULT_RESET_TIME         3600
#define CS_DEFAULT_THRESHOLD          100


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/* 黑名单类型 */
typedef enum {
    CS_BLACKLIST_TYPE_ADD,           /* 增加       */
    CS_BLACKLIST_TYPE_DEL,           /* 删除       */

    CS_BLACKLIST_TYPE_BUTT
} cs_blacklist_type_enum;
typedef osal_u8 cs_blacklist_type_enum_uint8;

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
/* 自动黑名单配置参数 */
typedef struct {
    osal_u8                       enabled;               /* 使能标志 0:未使能  1:使能 */
    osal_u8                       reserved[3];          /* 字节对齐                  */
    osal_u32                      ul_threshold;             /* 门限                      */
    osal_u32                      reset_time;            /* 重置时间                  */
    osal_u32                      aging_time;            /* 老化时间                  */
} hmac_autoblacklist_cfg_stru;

/* 黑白名单配置 */
typedef struct {
    osal_u8                       type;                  /* 配置类型    */
    osal_u8                       mode;                  /* 配置模式    */
    osal_u8                       sa[6];                /* mac地址     */
} hmac_blacklist_cfg_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
typedef osal_u32 (*hmac_ap_up_rx_mgmt_blacklist_filter_cb)(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr);
typedef oal_bool_enum_uint8 (*hmac_blacklist_filter_etc_cb)(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr);
typedef osal_void (*hmac_blacklist_get_pointer_cb)(wlan_vap_mode_enum vap_mod, hmac_vap_stru *hmac_vap,
    osal_u8 chip_id, osal_u8 device_id, osal_u8 vap_id);

static osal_u32 hmac_blacklist_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_blacklist_register_init"), used));
static osal_void hmac_blacklist_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_blacklist_register_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
