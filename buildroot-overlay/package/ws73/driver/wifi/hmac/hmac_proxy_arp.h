/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_proxy_arp.h
 * 生成日期   : 2014年7月29日
 * 功能描述   : hmac_proxy_arp.c的头文件
 */

#ifndef __HMAC_PROXY_ARP_H__
#define __HMAC_PROXY_ARP_H__

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

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
#define HMAC_PROXY_IPV4_HASHSIZE        MAC_VAP_USER_HASH_MAX_VALUE
#define HMAC_PROXY_IPV6_HASHSIZE        MAC_VAP_USER_HASH_MAX_VALUE

#define hmac_proxy_ipv4_hash(n) \
        (((const osal_u8 *)(&n))[3] % HMAC_PROXY_IPV4_HASHSIZE)

#define hmac_proxy_ipv6_hash(n) \
        (((const osal_u8 *)(n))[15] % HMAC_PROXY_IPV6_HASHSIZE)

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
    struct osal_list_head     entry;
    osal_u32              ipv4;                        /* 记录对应的ipv4地址 */
    osal_u8               mac[WLAN_MAC_ADDR_LEN];     /* 记录对应的mac地址 */
    osal_u8               rsv[2];
} hmac_proxy_ipv4_hash_stru;

typedef struct {
    struct osal_list_head     entry;
    oal_in6_addr            ipv6;                        /* 记录对应的ipv4地址 */
    osal_u8               mac[WLAN_MAC_ADDR_LEN];     /* 记录对应的mac地址 */
    osal_u8               rsv[2];
} hmac_proxy_ipv6_hash_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_proxy_arp_init(hmac_vap_stru *hmac_vap);
extern osal_void hmac_proxy_exit(hmac_vap_stru *hmac_vap);
extern oal_bool_enum_uint8 hmac_proxy_arp_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf);
extern oal_err_code_enum_uint32 hmac_proxy_remove_mac(hmac_vap_stru *hmac_vap, osal_u8 *mac);
extern osal_s32 hmac_proxyarp_on(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_u32 hmac_proxy_display_info(hmac_vap_stru *hmac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */
#endif

