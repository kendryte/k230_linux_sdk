/*
 * Copyright (c) CompanyNameMagicTag 2001-2023. All rights reserved.
 * Description: 与内核接口相关的scan功能函数, wal_linux_scan.c 的头文件
 */

#ifndef __WAL_LINUX_SCAN_H__
#define __WAL_LINUX_SCAN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_device.h"
#include "wal_linux_rx_rsp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_SCAN_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* 扫描上报"最近时间" 范围内的扫描结果 */
#define WAL_SCAN_REPORT_LIMIT         5000       /* 5000 milliseconds */
#define WAL_SCAN_RSSI_MAX             0
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

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_void  wal_inform_all_bss_etc(oal_wiphy_stru  *wiphy, hmac_bss_mgmt_stru  *bss_mgmt,
    osal_u8   vap_id);
osal_u32 wal_scan_work_func_etc(hmac_scan_stru *scan_mgmt, oal_net_device_stru *netdev,
    oal_cfg80211_scan_request_stru *request, hmac_vap_stru *hmac_vap);
extern osal_s32 wal_force_scan_complete_etc(oal_net_device_stru *net_dev, oal_bool_enum is_aborted);
extern osal_s32 wal_force_scan_complete_for_disconnect_scene(oal_net_device_stru   *net_dev);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
extern osal_s32 wal_stop_sched_scan_etc(oal_net_device_stru *netdev);
#endif

osal_void wal_update_bss_etc(oal_wiphy_stru *wiphy, hmac_bss_mgmt_stru *bss_mgmt, osal_u8 *bssid);
osal_u32 is_p2p_scan_req(oal_cfg80211_scan_request_stru *request);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_linux_scan.h */
