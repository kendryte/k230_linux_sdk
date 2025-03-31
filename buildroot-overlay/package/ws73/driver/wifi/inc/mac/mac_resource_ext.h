/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac_regdomain_rom.c.
 * Create: 2020-7-8
 */
#ifndef __MAC_RESOURCE_EXT_H__
#define __MAC_RESOURCE_EXT_H__

/* 文件合一后去掉HMAC、DMAC两个宏 */
#include "oal_ext_if.h"
#include "mac_device_ext.h"
#include "wlan_resource_common.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    宏定义
*****************************************************************************/
/* 最大dev数量 */
#define MAC_RES_MAX_DEV_NUM     WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC

/* board最大关联用户数 = 1个CHIP支持的最大关联用户数 * board上面的CHIP数目 */
#define MAC_RES_MAX_ASOC_USER_NUM    (WLAN_ASSOC_USER_MAX_NUM * WLAN_CHIP_MAX_NUM_PER_BOARD)
/* board最大组播用户数 = 整board支持的业务vap个数(每个业务vap一个组播用户) */
#define MAC_RES_MAX_BCAST_USER_NUM   (WLAN_MULTI_USER_MAX_NUM_LIMIT)
/* board资源最大用户数 = 最大关联用户数 + 组播用户个数 */
#define MAC_RES_MAX_USER_LIMIT       (MAC_RES_MAX_ASOC_USER_NUM + MAC_RES_MAX_BCAST_USER_NUM)

osal_u16  mac_chip_get_max_asoc_user(osal_u8 chip_id);
osal_u16  hmac_board_get_max_user(void);
hmac_device_stru *hmac_res_get_mac_dev_etc(osal_u32 dev_idx);
void* mac_res_get_hmac_vap(osal_u8 idx);
void *_mac_res_alloc_hmac_user(osal_u16 idx);
void _mac_res_free_hmac_user(osal_u16 idx);
void*  _mac_res_get_hmac_user(osal_u16 idx);
osal_u32 mac_get_hal_vap_id(osal_u8 p2p_mode, osal_u8 vap_mode, osal_u8 *hal_vap_id);

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
typedef struct {
    oal_queue_stru      queue;
    osal_ulong            idx[MAC_RES_MAX_DEV_NUM];
    osal_u8           auc_user_cnt[MAC_RES_MAX_DEV_NUM];
    osal_u8           auc_resv[2];
} mac_res_device_stru;

typedef struct {
    void               *past_vap_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    oal_queue_stru          queue;
    osal_ulong                idx[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    osal_u8               auc_user_cnt[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    osal_u8               auc_resv[2];
} mac_res_vap_stru;

typedef struct {
    void               *past_user_info[MAC_RES_MAX_USER_LIMIT];
    oal_queue_stru_16       queue;
    osal_ulong                *pul_idx;
    osal_u8              *user_cnt;
} mac_res_user_stru;

typedef struct {
    mac_res_device_stru dev_res;
    mac_res_vap_stru    vap_res;
    mac_res_user_stru   user_res;
} mac_res_stru;

/* 存储hmac device结构的资源结构体 */
typedef struct {
    hmac_device_stru    hmac_dev_info[MAC_RES_MAX_DEV_NUM];
    oal_queue_stru      queue;
    osal_ulong            idx[MAC_RES_MAX_DEV_NUM];
    osal_u8           auc_user_cnt[MAC_RES_MAX_DEV_NUM];
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
    osal_u8           auc_resv[2];   /* 单芯片下MAC_RES_MAX_DEV_NUM是1，双芯片下MAC_RES_MAX_DEV_NUM是2 */
#else
    osal_u8           auc_resv[3];
#endif
} hmac_res_device_stru;

/* 存储hmac res资源结构体 */
typedef struct {
    hmac_res_device_stru hmac_dev_res;
} hmac_res_stru;

/*****************************************************************************
    全局变量声明
*****************************************************************************/
/*****************************************************************************
    函数声明
*****************************************************************************/
mac_res_stru *mac_get_pst_mac_res(osal_void);
hmac_res_stru *mac_pst_hmac_res_etc(osal_void);
/*****************************************************************************
 功能描述  : 分配HMAC层的VAP资源
*****************************************************************************/
osal_u16 hmac_board_get_max_user(void);

static INLINE__ osal_u32  mac_res_alloc_hmac_vap(osal_u8 *idx)
{
    osal_ulong  idx_temp;
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(idx == OAL_PTR_NULL))
    {
        return OAL_FAIL;
    }
    mac_res = mac_get_pst_mac_res();
    idx_temp = (osal_ulong)oal_queue_dequeue(&(mac_res->vap_res.queue));

    /* 0为无效值 */
    if (idx_temp == 0)
    {
        return OAL_FAIL;
    }

    *idx = (osal_u8)(idx_temp - 1);

    (mac_res->vap_res.auc_user_cnt[idx_temp - 1])++;

    return OAL_SUCC;
}

static INLINE__ osal_u32  mac_res_alloc_hmac_user(osal_u16 *pus_idx)
{
    osal_ulong  idx_temp;
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(pus_idx == OAL_PTR_NULL))
    {
        return OAL_FAIL;
    }
    mac_res = mac_get_pst_mac_res();
    idx_temp = (osal_ulong)oal_queue_dequeue_8user(&(mac_res->user_res.queue));

    /* 0为无效值 */
    if (idx_temp == 0)
    {
        return OAL_FAIL;
    }

    *pus_idx = (osal_u16)(idx_temp - 1);

    (mac_res->user_res.user_cnt[idx_temp - 1])++;

    return OAL_SUCC;
}

/*****************************************************************************
    函数声明
*****************************************************************************/
extern osal_u32  mac_res_alloc_hmac_dev_etc(osal_u8 *dev_idx);
extern osal_u32  mac_res_free_dev_etc(osal_u32 dev_idx);
extern osal_u32  hmac_res_free_mac_user_etc(osal_u16 idx);
extern osal_u32  hmac_res_free_mac_vap_etc(osal_u32 idx);
extern osal_u32  mac_res_init_etc(void);
extern osal_u32  hmac_res_exit(void);

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_res_alloc_mac_dev_etc(osal_u32 dev_idx);
extern osal_u32 hmac_res_free_mac_dev_etc(osal_u32 dev_idx);
extern hmac_device_stru *hmac_res_get_mac_dev_etc(osal_u32 dev_idx);
extern osal_u32 hmac_res_init_etc(osal_void);
extern osal_u32 hmac_res_exit_etc(mac_board_stru *hmac_board);

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif
