/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: OAL kernel file source file.
 * Author: Huanghe
 * Create: 2020-10-13
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_kernel_file.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID  OAM_FILE_ID_OAL_KERNEL_FILE_C

#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
OAL_STATIC oal_kobject* g_conn_syfs_root_object = NULL;
oal_kobject* oal_get_sysfs_root_object_etc(oal_void)
{
    if (g_conn_syfs_root_object != NULL) {
        return g_conn_syfs_root_object;
    }

    g_conn_syfs_root_object = kobject_create_and_add("ccsys", OAL_PTR_NULL);
    return g_conn_syfs_root_object;
}

oal_void oal_put_sysfs_root_object_etc(oal_void)
{
    if (g_conn_syfs_root_object == NULL) {
        return;
    }
    kobject_put(g_conn_syfs_root_object);
    g_conn_syfs_root_object = NULL;
}

EXPORT_SYMBOL(oal_get_sysfs_root_object_etc);
EXPORT_SYMBOL(oal_put_sysfs_root_object_etc);
#else
oal_kobject* oal_get_sysfs_root_object_etc(oal_void)
{
    return NULL;
}

oal_void oal_put_sysfs_root_object_etc(oal_void)
{
    return;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
