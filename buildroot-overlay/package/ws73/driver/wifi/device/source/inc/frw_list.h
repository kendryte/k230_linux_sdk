/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: wifi dmac os adapt layer
 * Author: AuthorNameMagicTag
 * Create: 2021-08-05
 */

#ifndef __FRW_LIST_H__
#define __FRW_LIST_H__

#include "frw_list_rom.h"

/* 从双向链表头部删除一个节点,不判断链表是否为空，不负责释放内存 请注意 */
static inline struct osal_list_head *osal_dlist_delete_head(const struct osal_list_head *head)
{
    struct osal_list_head *node;

    node = head->next;
    if (node == NULL || node == head) {
        return NULL;
    }

    osal_dlist_delete_entry(node);

    return node;
}

/* 从双向链表头部删除一个节点,不判断链表是否为空，不负责释放内存 请注意 */
static inline struct osal_list_head *osal_dlist_delete_tail(const struct osal_list_head *head)
{
    struct osal_list_head *node;

    node = head->prev;
    if (node == NULL || node == head) {
        return NULL;
    }

    osal_dlist_delete_entry(node);

    return node;
}

#endif // endif __FRW_LIST_ROM_H__

