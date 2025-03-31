/*
 * Copyright (c) CompanyNameMagicTag. 2021-2021. All rights reserved.
 * Description: wifi dmac os adapt layer
 * Create: 2021-08-05
 */

#ifndef __FRW_LIST_ROM_H__
#define __FRW_LIST_ROM_H__

#include "osal_types.h"
#include "osal_list.h"

/* 双向链表定义 */
typedef struct osal_list_head osal_dlist_head_stru;

/* 遍列双向链表，并可安全删除某个节点 */
#define osal_dlist_search_for_each_safe osal_list_for_each_safe

/* 遍历双向链表使用时请在其后加大括号 */
#define osal_dlist_search_for_each osal_list_for_each

/* 获得双向链表中指定的节点 第一个参数为模板链表结构体指针； 第二个参数为链表中数据结构体类型名；
 * 第三个参数为数据结构体中模板链表结构体的名字 */
#define osal_dlist_get_entry osal_list_entry

/* 链表初始化函数 */
#define osal_dlist_init_head OSAL_INIT_LIST_HEAD

/* 从链表的指定位置删除一个节点 */
#define osal_dlist_delete osal___list_del

/* 判断一个链表是否为空 */
#define osal_dlist_is_empty osal_list_empty

/* 往链表头部插入节点 */
#define osal_dlist_add_head osal_list_add

/* 向链表尾部插入节点 */
#define osal_dlist_add_tail osal_list_add_tail


/* 删除链表中的指定节点,不负责释放，不判断链表是否为空，请注意 */
static inline void osal_dlist_delete_entry(struct osal_list_head * const entry)
{
    if (osal_unlikely((entry->next == NULL) || (entry->prev == NULL))) {
        return;
    }

    osal___list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

/* 将链表2 加入链表1的尾部 */
static inline void osal_dlist_join_tail(struct osal_list_head * const head1, const struct osal_list_head *head2)
{
    struct osal_list_head *dlist1_tail;
    struct osal_list_head *dlist2_tail;
    struct osal_list_head *dlist2_first;

    dlist1_tail = head1->prev;
    dlist2_tail = head2->prev;
    dlist2_first = head2->next;
    dlist1_tail->next = dlist2_first;
    dlist2_first->prev = dlist1_tail;
    head1->prev = dlist2_tail;
    dlist2_tail->next = head1;
}

/* 将链表2 加入链表1的头部 也可用于将新链表 加入链表的指定节点后 */
static inline void osal_dlist_join_head(struct osal_list_head * const head1, const struct osal_list_head *head2)
{
    struct osal_list_head *head2_first = NULL;
    struct osal_list_head *head2_tail = NULL;
    struct osal_list_head *head1_first = NULL;

    if (osal_list_empty(head2) == OSAL_TRUE) {
        return;
    }

    head2_first = head2->next;
    head2_tail = head2->prev;
    head1_first = head1->next;

    head1->next = head2_first;
    head2_first->prev = head1;
    head2_tail->next = head1_first;
    head1_first->prev = head2_tail;
}

#endif // endif __FRW_LIST_ROM_H__

