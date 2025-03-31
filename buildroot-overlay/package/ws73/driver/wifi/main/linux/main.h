/*
 * Copyright (c) CompanyNameMagicTag. 2020-2024. All rights reserved.
 * Description: Header file of main.c.
 * Create: 2020-01-01
 */

#ifndef __MAIN_H__
#define __MAIN_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_H

/*****************************************************************************
    函数声明
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HIMPXX_DEV))
osal_s32 mpxx_device_main_init(void);
void device_main_init(void);
osal_u8 device_psm_main_function(void);
#elif (defined(_PRE_PRODUCT_ID_HOST))
osal_s32 mpxx_host_main_init(void);
void mpxx_host_main_exit(void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of main.h */
