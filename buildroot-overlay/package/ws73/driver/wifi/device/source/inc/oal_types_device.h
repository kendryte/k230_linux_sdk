/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: oal type head file.
 * Create: 2022-2-21
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __OAL_TYPES_DEVICE_H__
#define __OAL_TYPES_DEVICE_H__
#include "oal_types_device_rom.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_EIO 5     /* I/O error */
#define OAL_ENOMEM 12 /* Out of memory */
#define OAL_EFAUL 14  /* Bad address */
#define OAL_EBUSY 16  /* Device or resource busy */
#define OAL_ENODEV 19 /* No such device */

/* 三种业务类型 */
typedef enum {
    OAL_TRAFFIC_NORMAL = 0,              /* 一般业务 */
    OAL_TRAFFIC_MULTI_USER_MULTI_AC = 1, /* 多用户多优先级业务 */
    OAL_TRAFFIC_MULTI_USER_ONLY_BE = 2,  /* 32用户单优先级业务 */
    OAL_TRAFFIC_BUTT
} oal_traffic_type_enum;
typedef osal_u8 oal_traffic_type_enum_uint8;

typedef osal_u32 oal_err_code_enum_uint32;
typedef enum {
    OAL_CB_CONTINUE = 0,
    OAL_CB_RETURN = 1,
    OAL_ROM_CB_RSLT_BUTT
} oal_rom_cb_result_enum;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_types_device.h */
