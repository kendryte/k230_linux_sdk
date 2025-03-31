/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_main.h
 * 生成日期   : 2015年3月18日
 * 功能描述   : hmac_roam_main.c 的头文件
 */

#ifndef __HMAC_ROAM_MAIN_H__
#define __HMAC_ROAM_MAIN_H__

#include "oam_ext_if.h"
#include "hmac_vap.h"
#include "hmac_roam_alg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_MAIN_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ROAM_SCAN_TIME_MAX        (3 * 1000)       /* 扫描超时时间 单位ms */
#define ROAM_CONNECT_TIME_MAX     (10 * 1000)      /* 关联超时时间 单位ms */
#define ROAM_INVALID_SCAN_MAX     (5)              /* 连续无效扫描门限    */

/* 漫游场景识别，识别出密集AP场景和低密AP场景 */
#define WLAN_FULL_CHANNEL_NUM           (20)       /* 判断全信道扫描的数目门限 */
#define ROAM_ENV_CANDIDATE_GOOD_NUM     (5)        /* 统计漫游环境中强信号强度AP的个数 */
#define ROAM_ENV_CANDIDATE_WEAK_NUM     (10)       /* 统计漫游环境中弱信号强度AP的个数 */
#define ROAM_ENV_RSSI_NE60_DB          (-60)       /* 漫游环境中强信号强度AP RSSI的门限 */
#define ROAM_ENV_DENSE_TO_SPARSE_PERIOD (5)        /* 从高密场景向低密场景切换的扫描周期 */

/* 漫游场景识别，识别出不满足漫游门限、但是有更好信号强度AP的场景 */
#define ROAM_ENV_BETTER_RSSI_PERIOD     (5)        /* 不满足漫游门限，但是有更好信号强度AP的扫描周期 */
#define ROAM_ENV_BETTER_RSSI_DISTANSE   (20)       /* 同一个ESS中最强信号强度AP和当前关联AP的RSSI间隔 */
#define ROAM_RSSI_LINKLOSS_TYPE (-121)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

#define ROAM_BAND_2G_BIT BIT0
#define ROAM_BAND_5G_BIT BIT1

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

osal_void hmac_roam_main_clear(hmac_roam_info_stru *roam_info);
osal_void hmac_roam_main_change_state(hmac_roam_info_stru *roam_info, roam_main_state_enum_uint8 state);
osal_u32 hmac_roam_connect_to_bss(hmac_roam_info_stru *roam_info, osal_void *p_param);
hmac_roam_info_stru *hmac_get_roam_info(osal_u8 vap_id);
osal_void hmac_roam_main_del_timer(hmac_roam_info_stru *roam_info);
osal_u32 hmac_roam_main_fsm_action_etc(hmac_roam_info_stru *roam_info, roam_main_fsm_event_type_enum event,
    osal_void *p_param);
osal_u32 hmac_roam_start_etc(hmac_vap_stru *hmac_vap, roam_channel_org_enum scan_type,
    oal_bool_enum_uint8 cur_bss_ignore, roam_trigger_enum_uint8 roam_trigger);
osal_u32  hmac_roam_pause_user_etc(hmac_vap_stru *hmac_vap);
osal_void hmac_roam_connect_change_state(hmac_roam_info_stru *roam_info,
    roam_connect_state_enum_uint8 state);
osal_u32 hmac_roam_connect_check_state(hmac_roam_info_stru *roam_info,
    mac_vap_state_enum_uint8     vap_state,
    roam_main_state_enum_uint8 main_state,
    roam_connect_state_enum_uint8 connect_state);
osal_void hmac_roam_connect_start_timer(hmac_roam_info_stru *roam_info, osal_u32 timeout);
osal_u32 hmac_roam_connect_fail(hmac_roam_info_stru *roam_info);
osal_u32 hmac_roam_connect_succ(hmac_roam_info_stru *roam_info, osal_void *param);
osal_u32 hmac_roam_connect_fsm_action_etc(hmac_roam_info_stru *roam_info,
    roam_connect_fsm_event_type_enum event, osal_void *p_param);
osal_u32 hmac_roam_send_reassoc_req(hmac_roam_info_stru *roam_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_roam_main.h */
