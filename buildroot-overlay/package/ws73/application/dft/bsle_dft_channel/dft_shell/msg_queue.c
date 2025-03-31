/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
#include "securec.h"
#include "msg_queue.h"

static int test_suite_creat_msg_queue(int def)
{
    key_t queue_id = ftok(PATH, PROJ_ID);  /* queue_id标识唯一的消息队列 */
    if (queue_id < 0) {
        perror("ftok");
        return -EINVAL;
    }
    int msgqueue_id = msgget(queue_id, def);
    if (msgqueue_id < 0) {
        perror("msgget");  /* 创建消息队列失败 */
        return -EINVAL;
    }
    return msgqueue_id;
}

int test_suite_creat_queue(void)
{
    /* IPC_CREAT和IPC_EXCL一起使用保证创建一个全新的消息队列，IPC_EXCL单独使用没有用，必须和IPC_CREAT配合着使用，
       若消息队列存在，则错误返回；0666是使创建的消息队列的权限是666 */
    int def = IPC_CREAT | IPC_EXCL | 0666;
    return test_suite_creat_msg_queue(def);
}

int test_suite_get_queue(void)
{
    int def = IPC_CREAT;  /* 如果该队列不存在，则创建它，否则打开它 */
    return test_suite_creat_msg_queue(def);
}

int test_suite_new_queue(void)
{
    int msgqueue_id = test_suite_creat_queue();
    if (msgqueue_id < 0) { /* 如果该队列存在,删除重建 */
        msgqueue_id = test_suite_get_queue();
        test_suite_delete_queue(msgqueue_id);
        msgqueue_id = test_suite_creat_queue();
    }
    return msgqueue_id;
}

/* 生产消息 */
int test_suite_send_msg(int msgqueue_id, int msg_src, const char *msg)
{
    struct msgbuf_sle _buf;  /* _buf存放要发送的消息 */
    if (memset_s(&_buf, sizeof(_buf), '\0', sizeof(_buf)) != EOK) {
        perror("memset_s error");
    }
    _buf.mtype = msg_src;  /* 指定是谁发送的消息，也可以指定是谁接受该条消息 */
    if (strncpy_s(_buf.mtext, SIZE, msg, strlen(msg) + 1) != EOK) {
        perror("strncpy_s error");
    }
    return msgsnd(msgqueue_id, &_buf, sizeof(_buf.mtext), 0);
}

int test_suite_recv_msg(int msgqueue_id, int msg_dst, char out[], int out_len)
{  // 消费消息
    struct msgbuf_sle _buf;
    if (memset_s(&_buf, sizeof(_buf), '\0', sizeof(_buf)) != EOK) {
        perror("memset_s error");
    }
    /* 想要把want的_buf里面的消息删除，成功则返回消息的大小 */
    int ret = msgrcv(msgqueue_id, &_buf, sizeof(_buf.mtext), msg_dst, 0);
    if (ret <= -1) {
        perror("msgrcv");
        return -1;
    }
    if (memset_s(out, out_len, '\0', out_len) != EOK) {
        perror("memset_s error");
    }
    if (strcpy_s(out, out_len, _buf.mtext) != EOK) {
        perror("strcpy_s error");
    }
    return 0;
}

/* 删除消息队列 */
int test_suite_delete_queue(int msgqueue_id)
{
    int ret = msgctl(msgqueue_id, IPC_RMID, NULL);
    if (ret < 0) {
        perror("msgctl");
        return ret;
    }
    return 0;
}