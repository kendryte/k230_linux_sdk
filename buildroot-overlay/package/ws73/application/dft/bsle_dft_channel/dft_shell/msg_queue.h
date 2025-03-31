/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define PATH "."  /* 当前路径下 */
#define PROJ_ID 0x6654
#define SIZE 3000

#define SERVER_TYPE 2
#define CLIENT_TYPE 1

#define START_MSG "start\n"

struct msgbuf_sle {
    long mtype;          /* 标识是谁发的消息，也可以标识是给谁发的消息，此处举例是前者 */
    char mtext[SIZE];   /* 存放发的消息 */
};

int test_suite_send_msg(int msgqueue_id, int msg_src, const char *msg);           /* 生产消息 */
int test_suite_recv_msg(int msgqueue_id, int msg_dst, char out[], int out_len);   /* 消费消息 */

int test_suite_delete_queue(int msgqueue_id);  /* 删除消息队列 */

#endif /* MSG_QUEUE_H */
