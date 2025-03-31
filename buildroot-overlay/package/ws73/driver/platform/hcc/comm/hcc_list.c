/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc list
 * Author: CompanyName
 * Create: 2021-09-11
 */

#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_adapt.h"
#include "soc_osal.h"
#include "osal_list.h"
#include "hcc_list.h"

td_bool hcc_is_list_empty(hcc_data_queue *hcc_queue)
{
    return (osal_list_empty(&(hcc_queue->data_queue)) != 0);
}

td_void hcc_list_head_deinit(hcc_data_queue *hcc_queue)
{
    osal_spin_lock_destroy(&hcc_queue->data_queue_lock);
}

td_u32 hcc_list_head_init(hcc_data_queue *hcc_queue)
{
    td_u32 ret = (td_u32)osal_spin_lock_init(&hcc_queue->data_queue_lock);
    hcc_queue->data_queue.next = hcc_queue->data_queue.prev = &hcc_queue->data_queue;
    hcc_queue->qlen = 0;
    return ret;
}

td_u32 hcc_list_len(TD_CONST hcc_data_queue *hcc_queue)
{
    return hcc_queue->qlen;
}

td_void hcc_list_add_tail(hcc_data_queue *hcc_queue, hcc_unc_struc *unc_buf)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&hcc_queue->data_queue_lock, &flags);
    osal_list_add_tail(&unc_buf->list, &hcc_queue->data_queue);
    hcc_queue->qlen++;
    osal_spin_unlock_irqrestore(&hcc_queue->data_queue_lock, &flags);
}

td_void hcc_list_add_head(hcc_data_queue* hcc_queue, hcc_unc_struc *unc_buf)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&hcc_queue->data_queue_lock, &flags);
    osal_list_add(&unc_buf->list, &hcc_queue->data_queue);
    hcc_queue->qlen++;
    osal_spin_unlock_irqrestore(&hcc_queue->data_queue_lock, &flags);
}

struct osal_list_head *hcc_list_peek(TD_CONST struct osal_list_head *head)
{
    struct osal_list_head *list = head->next;
    if (list == head) {
        list = TD_NULL;
    }
    return list;
}

td_void _hcc_list_unlink(struct osal_list_head *node)
{
    struct osal_list_head *next = node->next;
    struct osal_list_head *prev = node->prev;

    node->next  = node->prev = TD_NULL;
    next->prev = prev;
    prev->next = next;
}

struct osal_list_head *hcc_list_peek_tail(TD_CONST struct osal_list_head *head)
{
    struct osal_list_head *list = head->prev;
    if (list == head) {
        list = TD_NULL;
    }
    return list;
}

struct osal_list_head *hcc_list_delete_tail(hcc_data_queue *TD_CONST hcc_queue)
{
    unsigned long flags;
    struct osal_list_head *node = TD_NULL;
    osal_spin_lock_irqsave(&hcc_queue->data_queue_lock, &flags);
    node = hcc_list_peek_tail(&hcc_queue->data_queue);
    if (node) {
        _hcc_list_unlink(node);
        hcc_queue->qlen--;
    }
    osal_spin_unlock_irqrestore(&hcc_queue->data_queue_lock, &flags);
    return node;
}


struct osal_list_head *hcc_list_delete_head(hcc_data_queue *TD_CONST hcc_queue)
{
    unsigned long flags;
    struct osal_list_head *node = TD_NULL;
    osal_spin_lock_irqsave(&hcc_queue->data_queue_lock, &flags);
    node = hcc_list_peek(&hcc_queue->data_queue);
    if (node) {
        _hcc_list_unlink(node);
        hcc_queue->qlen--;
    }
    osal_spin_unlock_irqrestore(&hcc_queue->data_queue_lock, &flags);
    return node;
}

hcc_unc_struc *hcc_list_dequeue(hcc_data_queue *TD_CONST hcc_queue)
{
    struct osal_list_head *node = hcc_list_delete_head(hcc_queue);
    return (node == TD_NULL) ? TD_NULL : osal_list_entry(node, hcc_unc_struc, list);
}

hcc_unc_struc *hcc_list_dequeue_tail(hcc_data_queue *TD_CONST hcc_queue)
{
    struct osal_list_head *node = hcc_list_delete_tail(hcc_queue);
    return (node == TD_NULL) ? TD_NULL : osal_list_entry(node, hcc_unc_struc, list);
}

td_void hcc_list_free(hcc_handler *hcc, hcc_data_queue *hcc_queue)
{
    hcc_unc_struc *unc_buf = TD_NULL;
    for (;;) {
        unc_buf = hcc_list_dequeue(hcc_queue);
        if (unc_buf == TD_NULL) {
            break;
        }
        hcc_adapt_mem_free(hcc, unc_buf);
    }
}

td_void hcc_list_restore(hcc_data_queue *dst_queue, hcc_data_queue *src_queue)
{
    hcc_unc_struc *unc_buf = TD_NULL;
    for (;;) {
        unc_buf = hcc_list_dequeue_tail(src_queue);
        if (unc_buf == TD_NULL) {
            break;
        }
        hcc_list_add_head(dst_queue, unc_buf);
    }
}

td_void _hcc_list_splice(TD_CONST struct osal_list_head *list, struct osal_list_head *prev,
    struct osal_list_head *next)
{
    struct osal_list_head *first = list->next;
    struct osal_list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

td_void hcc_list_splice_init(hcc_data_queue *hcc_queue, hcc_data_queue *head)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&hcc_queue->data_queue_lock, &flags);
    if (!hcc_is_list_empty(hcc_queue)) {
        _hcc_list_splice(&hcc_queue->data_queue, &head->data_queue, head->data_queue.next);
        head->qlen += hcc_queue->qlen;
        hcc_queue->qlen = 0;
        hcc_queue->data_queue.next = hcc_queue->data_queue.prev = &hcc_queue->data_queue;
    }
    osal_spin_unlock_irqrestore(&hcc_queue->data_queue_lock, &flags);
}
