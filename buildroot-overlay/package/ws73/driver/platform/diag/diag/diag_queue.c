/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: diag queue file
 */
#include "diag_queue.h"
#include <oal_schedule.h>
#include "soc_osal.h" // for osal_module_license

#define ZDIAG_TX_QUEUE_MAX_LEN  512 /* 队列缓存log个数限制 */

diag_queue *init_diag_queue(osal_void)
{
    diag_queue *que = (diag_queue *)kmalloc(sizeof(diag_queue), GFP_KERNEL);
    if (que == NULL) {
        return NULL;
    }
    que->front = que->rear = NULL;
    que->ocnt = 0;
    que->icnt = 0;
    que->len = 0;
    return que;
}

osal_s32 deinit_diag_queue(diag_queue *que)
{
    diag_msg_que msg;

    while (!is_diag_queue_empty(que)) {
        if (output_diag_queue(que, &msg) != OAL_SUCC) {
            return -OAL_FAIL;
        }
        osal_kfree(msg.buf);
    }
    osal_kfree(que);
    return OAL_SUCC;
}

osal_bool is_diag_queue_empty(diag_queue *queue)
{
    return (queue->front == NULL);
}

osal_bool is_diag_queue_full(diag_queue *queue)
{
    return (queue->len >= ZDIAG_TX_QUEUE_MAX_LEN);
}

osal_s32 insert_diag_queue(diag_queue *queue, diag_msg_que *msg)
{
    list_node *node = NULL;

    if (is_diag_queue_full(queue)) {
        return -OAL_FAIL;
    }

    node = (list_node *)osal_kmalloc(sizeof(list_node), OSAL_GFP_ATOMIC);
    if (node == NULL) {
        return -OAL_FAIL;
    }

    node->msg.msg_id = msg->msg_id;
    node->msg.buf = msg->buf;
    node->msg.len = msg->len;
    node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = node;
        queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->icnt++;
    queue->len++;
    return OAL_SUCC;
}

osal_s32 output_diag_queue(diag_queue *queue, diag_msg_que *msg)
{
    list_node *node = queue->front;
    if (is_diag_queue_empty(queue)) {
        return -OAL_FAIL;
    }

    msg->msg_id = node->msg.msg_id;
    msg->buf = node->msg.buf;
    msg->len = node->msg.len;

    if (queue->front == queue->rear) {
        queue->front = NULL;
        queue->rear = NULL;
    } else {
        queue->front = queue->front->next;
    }
    osal_kfree(node);
    queue->ocnt++;
    queue->len--;
    return OAL_SUCC;
}
