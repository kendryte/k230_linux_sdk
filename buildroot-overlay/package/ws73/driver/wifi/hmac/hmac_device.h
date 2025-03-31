/*
 * Copyright (c) CompanyNameMagicTag 2015-2023. All rights reserved.
 * 文 件 名   : hmac_device.h
 * 生成日期   : 2015年1月31日
 * 功能描述   : hmac_device.c 的头文件，包括hmac device结构的定义
 */

#ifndef __HMAC_DEVICE_H__
#define __HMAC_DEVICE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wlan_spec.h"
#include "dmac_ext_if_hcm.h"
#include "mac_vap_ext.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#include "hcc_host_if.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEVICE_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAX_MAC_ERR_IN_TBTT 5 /* 每一个tbtt间隙允许出现的最大错误数 */

// 新用户关联rssi门限为a，已关联用户被剔除rssi门限为b，a = b+HMAC_RSSI_LIMIT_DELTA; 配置命令传进来的为b, delta默认为5，可修改
#define HMAC_RSSI_LIMIT_DELTA 5

#ifdef _PRE_WLAN_TCP_OPT
#define HCC_TRANS_THREAD_POLICY    SCHED_FIFO
#define HCC_TRANS_THERAD_PRIORITY       (10)
#define HCC_TRANS_THERAD_NICE           (-10)
#endif

#define is_equal_rates(rate1, rate2)    (((rate1) & 0x7f) == ((rate2) & 0x7f))

typedef enum {
    HMAC_ACS_TYPE_INIT  = 0,         /* 初始自动信道选择 */
    HMAC_ACS_TYPE_CMD   = 1,         /* 命令触发的自动信道选择 */
    HMAC_ACS_TYPE_FREE  = 2,         /* 空闲的自动信道选择 */
} hmac_acs_type_enum;
typedef osal_u8 hmac_acs_type_enum_uint8;
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

/* 存储每个扫描到的bss信息 */
typedef struct {
    struct osal_list_head    dlist_head;       /* 链表指针 */
    mac_bss_dscr_stru      bss_dscr_info;    /* bss描述信息，包括上报的管理帧 */
} hmac_scanned_bss_info;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
static INLINE__ mac_fcs_mgr_stru *hmac_fcs_get_mgr_stru(hmac_device_stru *hmac_device)
{
    return &hmac_device->fcs_mgr;
}

/*****************************************************************************
  10.1 公共结构体初始化、删除
*****************************************************************************/
osal_void hmac_board_exit(osal_void);
osal_u32  hmac_board_init(osal_void);
/*****************************************************************************
  10.2 公共成员访问部分
*****************************************************************************/
hal_to_dmac_device_stru *hmac_device_get_another_h2d_dev(const hmac_device_stru *hmac_device,
    const hal_to_dmac_device_stru *ori_hal_dev);
osal_u8 hmac_device_check_fake_queues_empty(osal_void);
osal_s32 hmac_config_reset_mac_phy(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32  hmac_device_init_etc(osal_void);
osal_u32 hmac_device_exit_etc(hmac_device_stru *hmac_device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_device.h */
