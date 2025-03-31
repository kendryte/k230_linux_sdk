/*
 * Copyright (c) CompanyNameMagicTag 2017-2023. All rights reserved.
 * 文 件 名   : hmac_car.h
 * 生成日期   : 2017年03月24日
 * 功能描述   : 限速CAR(Committed Access Rate)功能
 */


#ifndef __HMAC_CAR_H__
#define __HMAC_CAR_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CAR_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_CAR_CYCLE_MS           100     // CAR默认定时器超时时间ms
#define HMAC_CAR_WIFI_ETH           20      // 上行需要减20个字节；wifi 34(26+8), eth 14(6+6+2); wifi-eth;用eth统计
#define HMAC_CAR_MULTICAST_PPS_NUM  0       // CAR组播默认pps的数目:0表示不使用pps限速，>0表示用pps限速的数值


/* 遍历VAP/USER操作封装 */
#define hmac_device_foreach_vap(_hmac_vap, _pst_device, _uc_vap_index)   \
    for ((_uc_vap_index) = 0,   \
        (_hmac_vap) = ((_pst_device)->vap_num > 0)? \
        ((hmac_vap_stru *)mac_res_get_hmac_vap((_pst_device)->vap_id[0])) : OAL_PTR_NULL;   \
        (_uc_vap_index) < (_pst_device)->vap_num;   \
        (_uc_vap_index) ++,                             \
        (_hmac_vap) = ((_uc_vap_index) < (_pst_device)->vap_num)? \
        ((hmac_vap_stru *)mac_res_get_hmac_vap((_pst_device)->vap_id[_uc_vap_index])) : OAL_PTR_NULL) \
        if (OAL_PTR_NULL != (_hmac_vap))

#define hmac_vap_foreach_user(_pst_user, _hmac_vap, _pst_list_pos)       \
    for ((_pst_list_pos) = (_hmac_vap)->mac_user_list_head.next,  \
        (_pst_user) = osal_list_entry((_pst_list_pos), hmac_user_stru, user_dlist);      \
        (_pst_list_pos) != &((_hmac_vap)->mac_user_list_head);                               \
        (_pst_list_pos) = (_pst_list_pos)->next,                                           \
        (_pst_user) = osal_list_entry((_pst_list_pos), hmac_user_stru, user_dlist))     \
        if (OAL_PTR_NULL != (_pst_user))

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

extern osal_u32  hmac_car_device_bw_limit(hmac_device_stru *hmac_device, mac_cfg_car_stru *car_cfg_param);

extern osal_u32  hmac_car_vap_bw_limit(hmac_vap_stru *hmac_vap, mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_user_bw_limit(hmac_vap_stru *hmac_vap, mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_device_multicast(hmac_device_stru *hmac_device, mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_device_timer_cycle_limit(hmac_device_stru *hmac_device,
    mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_process(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *buf,
    hmac_car_up_down_type_enum_uint8 car_type);
extern osal_u32  hmac_car_process_uplink(osal_u16 ta_user_idx, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *buf, hmac_car_up_down_type_enum_uint8 car_type);
extern osal_u32  hmac_car_enable_switch(hmac_device_stru *hmac_device, mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_show_info(hmac_device_stru *hmac_device);
extern osal_u32  hmac_car_multicast_process(hmac_device_stru *hmac_device, oal_netbuf_stru *buf);
extern osal_u32  hmac_car_device_multicast_pps_num(hmac_device_stru *hmac_device,
    mac_cfg_car_stru *car_cfg_param);
extern osal_u32  hmac_car_init(hmac_device_stru *hmac_device);
extern osal_u32  hmac_car_exit(hmac_device_stru *hmac_device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
