/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: efuse header file.
 */

#ifndef __FE_HAL_EFUSE_IF_H__
#define __FE_HAL_EFUSE_IF_H__

#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define FE_EFUSE_DATA_MAX_LEN 4
#define FE_EFUSE_NAME_LEN 16
#define FE_EFUSE_GROUP_MAX 2

typedef struct {
    osal_char efuse_name[FE_EFUSE_NAME_LEN];  /* efuse功能名 */
    osal_u32 size;                            /* efuse_name对应大小，单位bit */
    osal_u8 group_id[FE_EFUSE_GROUP_MAX];     /* efuse_name对应的PG位 */
    osal_u8 lock_id[FE_EFUSE_GROUP_MAX];      /* efuse_name对应的LOCk位 */
} fe_efuse_id_info_stru;

typedef struct {
    osal_char efuse_name[FE_EFUSE_NAME_LEN]; /* efuse功能名 */
    osal_u32 len;                            /* 数据长度 */
    osal_u8 val[FE_EFUSE_DATA_MAX_LEN];      /* 操作数据 */
} fe_efuse_operate_stru;

osal_s32 fe_hal_efuse_read(fe_efuse_operate_stru *info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  /* __FE_HAL_EFUSE_IF_H__ */