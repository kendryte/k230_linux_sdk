/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * 文 件 名   : hmac_wds.c
 * 生成日期   : 2016年12月13日
 * 功能描述   : hmac_wds.c的头文件
 */


#ifndef __HMAC_WDS_H__
#define __HMAC_WDS_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WDS_H
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
/*****************************************************************************/
/*****************************************************************************
  2 宏定义
*****************************************************************************/
static INLINE__ osal_u32 mac_calculate_hash_value(const osal_u8 *sta_mac_addr)
{
    return (sta_mac_addr[0] + sta_mac_addr[1] + sta_mac_addr[2] + sta_mac_addr[3] + sta_mac_addr[4] + sta_mac_addr[5]) &
        (osal_u32)(MAC_VAP_USER_HASH_MAX_VALUE - 1);
}

#define wds_calc_mac_hash_val(mac_addr) (mac_calculate_hash_value(mac_addr) & (WDS_HASH_NUM - 1))
#define WDS_TABLE_ADD_ENTRY             0
#define WDS_TABLE_DEL_ENTRY             1
#define WDS_MAX_STAS_NUM                256
#define WDS_MAX_NODE_NUM                4
#define WDS_MAX_NEIGH_NUM               512

#define WDS_MIN_AGE_NUM                 5
#define WDS_MAX_AGE_NUM                 10000

#define WDS_TABLE_DEF_TIMER             15000     /* timer interval as 15 secs */

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#define VMSTA_4ADDR_SUPPORT             0
#define VMSTA_4ADDR_UNSUPPORT           -1
#endif
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
    struct osal_list_head                 entry;
    osal_u8                           mac[WLAN_MAC_ADDR_LEN];
    osal_u8                           stas_num;
    osal_u8                           auc_resv[1];
} hmac_wds_node_stru;

typedef struct {
    struct osal_list_head                 entry;
    osal_u32                          last_pkt_age;
    hmac_wds_node_stru                  *related_node;
    osal_u8                           mac[WLAN_MAC_ADDR_LEN];
    osal_u8                           auc_resv[2];
} hmac_wds_stas_stru;

typedef struct {
    struct osal_list_head                 entry;
    osal_u32                          last_pkt_age;
    osal_u8                           mac[WLAN_MAC_ADDR_LEN];
    osal_u8                           auc_resv[2];
} hmac_wds_neigh_stru;

typedef struct {
    osal_u8                           sta_mac[WLAN_MAC_ADDR_LEN];
    osal_u8                           node_mac[WLAN_MAC_ADDR_LEN];
} mac_cfg_wds_sta_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
typedef osal_u32(*p_hmac_wds_node_func)(hmac_vap_stru *hmac_vap, osal_u8 *addr, osal_void *arg);

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_wds_update_table(hmac_vap_stru *hmac_vap, osal_u8 *node_mac, osal_u8 *sta_mac,
    osal_u8 update_mode);
extern osal_u32 hmac_wds_add_node(hmac_vap_stru *hmac_vap, osal_u8 *node_mac);
extern osal_u32 hmac_wds_del_node(hmac_vap_stru *hmac_vap, osal_u8 *node_mac);
extern osal_u32 hmac_wds_reset_sta_mapping_table(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_wds_add_sta(hmac_vap_stru *hmac_vap, osal_u8 *node_mac, osal_u8 *sta_mac);
extern osal_u32 hmac_wds_del_sta(hmac_vap_stru *hmac_vap, osal_u8 *addr);
extern osal_u32 hmac_find_valid_user_by_wds_sta(hmac_vap_stru *hmac_vap, osal_u8 *sta_mac_addr,
    osal_u16 *pus_user_idx);
extern osal_u32 hmac_wds_node_ergodic(hmac_vap_stru *hmac_vap, osal_u8 *src_addr,
    p_hmac_wds_node_func hmac_wds_node, osal_void *arg);

extern osal_u32 hmac_wds_update_neigh(hmac_vap_stru *hmac_vap, osal_u8 *addr);
extern osal_u32 hmac_wds_neigh_not_expired(hmac_vap_stru *hmac_vap, osal_u8 *addr);
extern osal_u32 hmac_wds_reset_neigh_table(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_wds_table_create_timer(hmac_vap_stru *hmac_vap);
extern oal_bool_enum_uint8 hmac_vmsta_get_user_a4_support(hmac_vap_stru *hmac_vap, osal_u8 *addr);
extern oal_bool_enum_uint8 hmac_vmsta_check_vap_a4_support(osal_u8 *ie, osal_u32 ie_len);
extern oal_bool_enum_uint8 hmac_vmsta_check_user_a4_support(osal_u8 *frame, osal_u32 frame_len);
extern osal_u32 hmac_vmsta_set_vap_a4_enable(hmac_vap_stru *hmac_vap);
extern osal_void hmac_wds_init_table(hmac_vap_stru *hmac_vap);
extern osal_u32  hmac_wds_find_sta(hmac_vap_stru *hmac_vap, osal_u8 *addr,
    hmac_wds_stas_stru **ppst_wds_node);
extern osal_u32 hmac_wds_tx_broadcast_pkt(hmac_vap_stru *hmac_vap, osal_u8 *addr, osal_void *arg);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_wds.h */
