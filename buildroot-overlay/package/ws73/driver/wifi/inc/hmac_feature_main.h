/*
 * Copyright (c) @CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: Header file of hmac&dmac message id defined.
 * Create: 2021-07-14
 */

#ifndef WLAN_FEATURE_H
#define WLAN_FEATURE_H

#include "mac_vap_ext.h"
#include "mac_device_ext.h"
#include "mac_user_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    WLAN_FEATURE_INDEX_FTM,
    WLAN_FEATURE_INDEX_DFS,
    WLAN_FEATURE_INDEX_M2S,
    WLAN_FEATURE_INDEX_BTCOEX,
    WLAN_FEATURE_INDEX_BUTT
} wlan_feature_index_enum;

/*****************************************************************************
 函 数 名  : hmac_device_get_feature_ptr
 功能描述  : 通过特性索引获取hmac_device下的特性指针
*****************************************************************************/
static INLINE__ osal_void *hmac_device_get_feature_ptr(hmac_device_stru *hmac_device,
    wlan_feature_index_enum feature_index)
{
    return (osal_void *)hmac_device->hmac_device_feature_arr[feature_index];
}

/*****************************************************************************
 函 数 名  : hmac_device_feature_registered
 功能描述  : 在hmac_device结构体下注册特性指针
*****************************************************************************/
static INLINE__ osal_u32 hmac_device_feature_registered(hmac_device_stru *hmac_device,
    wlan_feature_index_enum feature_index, osal_void *feature_ptr)
{
    if (hmac_device->hmac_device_feature_arr[feature_index] != OAL_PTR_NULL) {
        return OAL_FAIL;
    }
    hmac_device->hmac_device_feature_arr[feature_index] = feature_ptr;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_device_feature_cancellation
 功能描述  : 在hmac_device结构体下去注册特性指针
*****************************************************************************/
static INLINE__ osal_void hmac_device_feature_unregister(hmac_device_stru *hmac_device,
    wlan_feature_index_enum feature_index)
{
    hmac_device->hmac_device_feature_arr[feature_index] = OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_device_feature_init
 功能描述  : 在hmac_device结构体下特性指针数组内存申请
*****************************************************************************/
static INLINE__ osal_u32 hmac_device_feature_init(hmac_device_stru *hmac_device)
{
    if (hmac_device->hmac_device_feature_arr == OAL_PTR_NULL) {
        hmac_device->hmac_device_feature_arr = (osal_void **) oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT, OAL_TRUE);
        if (hmac_device->hmac_device_feature_arr == OAL_PTR_NULL) {
            return OAL_FAIL;
        }
    }
    (osal_void)memset_s(hmac_device->hmac_device_feature_arr, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT,
        0, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_device_feature_deinit
 功能描述  : 在hmac_device结构体下特性指针数组内存释放
*****************************************************************************/
static INLINE__ osal_u32 hmac_device_feature_deinit(hmac_device_stru *hmac_device)
{
    if (hmac_device->hmac_device_feature_arr != OAL_PTR_NULL) {
        oal_mem_free(hmac_device->hmac_device_feature_arr, OAL_TRUE);
        hmac_device->hmac_device_feature_arr = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_feature_ptr
 功能描述  : 通过特性索引获取hmac_vap下的特性指针
*****************************************************************************/
static INLINE__ osal_void *hmac_vap_get_feature_ptr(const hmac_vap_stru *hmac_vap,
    wlan_feature_index_enum feature_index)
{
    if (hmac_vap->hmac_vap_feature_arr == OSAL_NULL) {
        return OSAL_NULL;
    }
    return (osal_void *)hmac_vap->hmac_vap_feature_arr[feature_index];
}

/*****************************************************************************
 函 数 名  : hmac_vap_feature_registered
 功能描述  : 在hmac_vap结构体下注册特性指针
*****************************************************************************/
static INLINE__ osal_u32 hmac_vap_feature_registered(hmac_vap_stru *hmac_vap,
    wlan_feature_index_enum feature_index, osal_void *feature_ptr)
{
    if (hmac_vap->hmac_vap_feature_arr[feature_index] != OAL_PTR_NULL) {
        return OAL_FAIL;
    }
    hmac_vap->hmac_vap_feature_arr[feature_index] = feature_ptr;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_feature_cancellation
 功能描述  : 在hmac_vap结构体下去注册特性指针
*****************************************************************************/
static INLINE__ osal_void hmac_vap_feature_unregister(hmac_vap_stru *hmac_vap,
    wlan_feature_index_enum feature_index)
{
    hmac_vap->hmac_vap_feature_arr[feature_index] = OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_vap_feature_init
 功能描述  : 在hmac_vap结构体下特性指针数组内存申请
*****************************************************************************/
static INLINE__ osal_u32 hmac_vap_feature_init(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->hmac_vap_feature_arr != OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    hmac_vap->hmac_vap_feature_arr = (osal_void **)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT, OAL_TRUE);
    if (hmac_vap->hmac_vap_feature_arr == OAL_PTR_NULL) {
        return OAL_FAIL;
    }
    (osal_void)memset_s(hmac_vap->hmac_vap_feature_arr, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT,
        0, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_feature_deinit
 功能描述  : 在hmac_vap结构体下特性指针数组内存释放
*****************************************************************************/
static INLINE__ osal_u32 hmac_vap_feature_deinit(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->hmac_vap_feature_arr != OAL_PTR_NULL) {
        oal_mem_free(hmac_vap->hmac_vap_feature_arr, OAL_TRUE);
        hmac_vap->hmac_vap_feature_arr = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_get_feature_ptr
 功能描述  : 通过特性索引获取hmac_user下的特性指针
*****************************************************************************/
static INLINE__ osal_u8 *hmac_user_get_feature_ptr(hmac_user_stru *hmac_user,
    wlan_feature_index_enum feature_index)
{
    return (osal_u8 *)hmac_user->hmac_user_feature_arr[feature_index];
}

/*****************************************************************************
 函 数 名  : hmac_user_feature_registered
 功能描述  : 在hmac_user结构体下注册特性指针
*****************************************************************************/
static INLINE__ osal_u32 hmac_user_feature_registered(hmac_user_stru *hmac_user,
    wlan_feature_index_enum feature_index, osal_void *feature_ptr)
{
    if (hmac_user->hmac_user_feature_arr[feature_index] != OAL_PTR_NULL) {
        return OAL_FAIL;
    }
    hmac_user->hmac_user_feature_arr[feature_index] = feature_ptr;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_feature_cancellation
 功能描述  : 在hmac_user结构体下去注册特性指针
*****************************************************************************/
static INLINE__ osal_void hmac_user_feature_unregister(hmac_user_stru *hmac_user,
    wlan_feature_index_enum feature_index)
{
    hmac_user->hmac_user_feature_arr[feature_index] = OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_user_feature_init
 功能描述  : 在hmac_user结构体下特性指针数组内存申请
*****************************************************************************/
static INLINE__ osal_u32 hmac_user_feature_init(hmac_user_stru *hmac_user)
{
    if (hmac_user->hmac_user_feature_arr == OAL_PTR_NULL) {
        hmac_user->hmac_user_feature_arr = (osal_void **) oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT, OAL_TRUE);
        if (hmac_user->hmac_user_feature_arr == OAL_PTR_NULL) {
            return OAL_FAIL;
        }
    }
    (osal_void)memset_s(hmac_user->hmac_user_feature_arr, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT,
        0, sizeof(osal_void *) * WLAN_FEATURE_INDEX_BUTT);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_feature_deinit
 功能描述  : 在hmac_user结构体下特性指针数组内存释放
*****************************************************************************/
static INLINE__ osal_u32 hmac_user_feature_deinit(hmac_user_stru *hmac_user)
{
    if (hmac_user->hmac_user_feature_arr != OAL_PTR_NULL) {
        oal_mem_free(hmac_user->hmac_user_feature_arr, OAL_TRUE);
        hmac_user->hmac_user_feature_arr = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif
