/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: mem.c
 */

#include "wlan_spec.h"
#include "oal_mem_hcm.h"
#include "oam_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_OAL_MEM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_void *oal_mem_alloc_etc(osal_u32 file_id, osal_u32 line_num,
    oal_mem_pool_id_enum_uint8 pool_id, osal_u16 len, osal_u8 lock)
{
    osal_void *mem_addr;
    unref_param(pool_id);
    unref_param(lock);
    if (OAL_UNLIKELY(len == 0)) {
        return OAL_PTR_NULL;
    }
    mem_addr = osal_kmalloc(len, 0);
    if (OAL_UNLIKELY(mem_addr == OAL_PTR_NULL)) {
        wifi_printf("oal_mem_alloc_etc FAIL F=%d, L=%d, id=%d, len=%d.\r\n", file_id, line_num, pool_id, len);
    }
    return mem_addr;
}

osal_u32 oal_mem_free_etc(osal_u32 file_id, osal_u32 line_num, osal_void *data, osal_u8 lock)
{
    unref_param(file_id);
    unref_param(line_num);
    unref_param(lock);

    osal_kfree(data);
    return OAL_SUCC;
}

oal_netbuf_stru *oal_mem_multi_netbuf_alloc(osal_u16 len)
{
    return oal_netbuf_alloc_ext(0, len, 0);
}
