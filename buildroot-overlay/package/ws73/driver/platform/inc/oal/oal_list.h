/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal list header
 * Author: Huanghe
 * Create: 2023-01-06
 */

#ifndef __OAL_LIST_H__
#define __OAL_LIST_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_mm.h"
#include "oal_ext_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct tag_oal_dlist_head_stru {
    struct tag_oal_dlist_head_stru *next;
    struct tag_oal_dlist_head_stru *prev;
} oal_dlist_head_stru;


OAL_STATIC OAL_INLINE oal_void  oal_dlist_add(oal_dlist_head_stru *pst_new,
                            oal_dlist_head_stru *prev,
                            oal_dlist_head_stru *next)
{
    next->prev  = pst_new;
    pst_new->next   = next;
    pst_new->prev   = prev;
    prev->next  = pst_new;
}

OAL_STATIC OAL_INLINE oal_void  oal_dlist_delete(oal_dlist_head_stru *prev, oal_dlist_head_stru *next)
{
    next->prev = prev;
    prev->next = next;
}

OAL_STATIC OAL_INLINE oal_void  oal_dlist_add_tail(oal_dlist_head_stru *pst_new, oal_dlist_head_stru *pst_head)
{
    oal_dlist_add(pst_new, pst_head->prev, pst_head);
}

OAL_STATIC OAL_INLINE oal_void  oal_dlist_delete_entry(oal_dlist_head_stru *pst_entry)
{
    if (unlikely((pst_entry->next == OAL_PTR_NULL) || (pst_entry->next == OAL_PTR_NULL))) {
        return;
    }

    oal_dlist_delete(pst_entry->prev, pst_entry->next);
    pst_entry->next = OAL_PTR_NULL;
    pst_entry->prev = OAL_PTR_NULL;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_list.h */
