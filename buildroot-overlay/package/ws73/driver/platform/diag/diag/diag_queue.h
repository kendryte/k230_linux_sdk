/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: diag queue file
 */
#ifndef __DIAG_QUEUE_H__
#define __DIAG_QUEUE_H__

#include "osal_types.h"

#define diag_print(fmt, ...)

typedef struct {
    osal_u32 msg_id;
    osal_u8 *buf;
    osal_u32 len;
} diag_msg_que;

typedef struct list_node {
    diag_msg_que msg;
    struct list_node *next;
} list_node;

typedef struct diag_queue {
    list_node *front;
    list_node *rear;
    osal_u32 icnt;
    osal_u32 ocnt;
    osal_u32 len;
} diag_queue;

diag_queue *init_diag_queue(osal_void);
osal_s32 deinit_diag_queue(diag_queue *que);
osal_s32 insert_diag_queue(diag_queue *queue, diag_msg_que *msg);
osal_s32 output_diag_queue(diag_queue *queue, diag_msg_que *msg);
osal_bool is_diag_queue_empty(diag_queue *queue);

#endif
