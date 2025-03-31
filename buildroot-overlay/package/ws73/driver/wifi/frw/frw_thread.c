/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: frw线程处理代码.
 * Author: Huanghe
 * Create: 2021-07-06
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "frw_thread.h"
#include "securec.h"
#include "oal_mem_hcm.h"
#include "oam_ext_if.h"
#include "oam_struct.h"
#include "frw_hmac_adapt.h"
#include "frw_hmac.h"
#include "hmac_ext_if.h"
#include "wlan_msg.h"
#include "hmac_thruput_test.h"
#include "frw_hmac_hcc_adapt.h"
#include "osal_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_THREAD_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

typedef struct {
    frw_msg msg;
    struct osal_list_head list;
    osal_s32 cb_return; // callback return value
    osal_u16 msg_id;
    osal_u8 wait_cond : 1;
    osal_u8 sync : 1;
    osal_u8 wait_cond_thread : 1; /* 只由frw_thread唤醒 */
    osal_u8 rsv : 5;
    osal_u8 vap_id;
    osal_u16 time_out;
    osal_u16 seq;
    osal_atomic wait_fail;
    osal_wait wait_q;
} frw_msg_node;

/* FRW线程ID枚举 */
typedef enum {
    FRW_THREAD_ID_MSG = 0,    /* wifi_frw0 负责其他消息处理 */
    FRW_THREAD_ID_TXDATA = 1, /* wifi_frw1 负责发送方向报文处理以及发送完成中断消息处理 */
    FRW_THREAD_ID_NUM
} frw_thread_id;

/* FRW线程状态 */
typedef enum {
    FRW_THREAD_EXIT = 0, /* 线程退出,必须设置为0 */
    FRW_THREAD_RUN = 1,  /* 线程运行 */
} frw_thread_status;

/* 支持处理的队列类型枚举 */
typedef enum {
    FRW_THREAD_QUE_POST_SYNC_MSG,
    FRW_THREAD_QUE_POST_ASYNC_MSG,
    FRW_THREAD_QUE_SEND_SYNC_MSG,
    FRW_THREAD_QUE_RCV_ASYNC_MSG,
    FRW_THREAD_QUE_NETBUF,
    FRW_THREAD_QUE_NUM
} frw_thread_que_type;

typedef osal_s32 (*frw_que_process)(osal_void *data);

typedef struct {
    struct osal_list_head list;
    osal_u32 cnt;
    osal_u32 seq;
    frw_que_process que_fn;
    osal_spinlock lock;
} frw_thread_que;

typedef struct {
    osal_task *thread;
    osal_wait wq;
    frw_thread_que thread_que[FRW_THREAD_QUE_NUM];
    osal_slong pid;
    /* dfx */
    osal_u8 id;
    osal_u8 is_running;
    osal_u8 status;
} frw_thread;

typedef struct {
    frw_thread frw_threads[FRW_THREAD_ID_NUM];
    osal_slong hcc_pid; /* 保存hcc线程id，用于检查线程自己调用自己的异常场景 */
} frw_thread_ctrl;

OSAL_STATIC const osal_char *g_frw_thread_name[FRW_THREAD_ID_NUM] = {
    [FRW_THREAD_ID_MSG] = "wifi_frw_msg",
    [FRW_THREAD_ID_TXDATA] = "wifi_frw_txdata"
};

OSAL_STATIC osal_u32 g_frw_stack_size[FRW_THREAD_ID_NUM] = {
    [FRW_THREAD_ID_MSG] = FRW_MSG_TASK_STACK_SIZE,
    [FRW_THREAD_ID_TXDATA] = FRW_TX_DATA_TASK_STACK_SIZE
};

OSAL_STATIC osal_u32 g_frw_priority[FRW_THREAD_ID_NUM] = {
    [FRW_THREAD_ID_MSG] = FRW_THREAD_TASK_PRIORITY,
    [FRW_THREAD_ID_TXDATA] = FRW_TXDATA_TASK_PRIORITY
};

OSAL_STATIC frw_thread_ctrl g_frw_thread_ctrl;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OSAL_STATIC osal_u32 g_frw_txdata_que_limit = 2000; /* 2000 txdata队列最大长度限制 */
#else
OSAL_STATIC osal_u32 g_frw_txdata_que_limit = 0;    /* txdata队列最大长度限制，0表示不限制 */
#endif

WIFI_HMAC_TCM_TEXT OSAL_STATIC frw_thread_ctrl *frw_get_thread_ctrl(osal_void)
{
    return &g_frw_thread_ctrl;
}

static inline frw_thread_que *frw_get_thread_que(frw_thread_id id, frw_thread_que_type type)
{
    return &(frw_get_thread_ctrl()->frw_threads[id].thread_que[type]);
}

OSAL_STATIC const osal_char *frw_get_thread_name(frw_thread_id id)
{
    if (id >= FRW_THREAD_ID_NUM) {
        return "wifi_frw_unknown";
    }

    return g_frw_thread_name[id];
}

static inline void frw_atomic_set(osal_atomic *atomic, osal_s32 val)
{
    osal_adapt_atomic_set(atomic, val);
}

static inline osal_s32 frw_atomic_read(osal_atomic *atomic)
{
    return osal_adapt_atomic_read(atomic);
}

static inline osal_void frw_wait_wakeup(osal_wait *wait)
{
    osal_adapt_wait_wakeup(wait);
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_void frw_host_free_msg_node(frw_msg_node *node)
{
    if (node->msg.data != OSAL_NULL) {
        osal_kfree(node->msg.data);
        node->msg.data = OSAL_NULL;
        node->msg.data_len = 0;
    } else if (node->msg.rsp != OSAL_NULL) {
        osal_kfree(node->msg.rsp);
        node->msg.rsp = OSAL_NULL;
        node->msg.rsp_buf_len = 0;
    }

    osal_adapt_wait_destroy(&node->wait_q);

#ifndef _PRE_WLAN_FEATURE_WS63
    osal_adapt_atomic_destroy(&node->wait_fail);
#endif

    osal_kfree(node);
}

static WIFI_HMAC_TCM_TEXT frw_msg_node* frw_alloc_node_mem(osal_u16 msg_id, osal_u8 vap_id, osal_bool sync,
    osal_u16 time_out)
{
    frw_msg_node *node = OSAL_NULL;
    node = (frw_msg_node*)osal_kmalloc(sizeof(frw_msg_node), OSAL_GFP_ATOMIC);
    if (node == OSAL_NULL) {
        oam_error_log2(vap_id, OAM_SF_FRW, "frw_host_alloc_msg_node: alloc failed. msg_id %d sync %d", msg_id, sync);
        return node;
    }
    memset_s(node, sizeof(frw_msg_node), 0, sizeof(frw_msg_node));
    OSAL_INIT_LIST_HEAD(&node->list);

    if (osal_wait_init(&node->wait_q) != OSAL_SUCCESS) {
        osal_kfree(node);
        node = OSAL_NULL;
        return node;
    }
    osal_adapt_atomic_init(&node->wait_fail);
    node->wait_cond_thread = OSAL_FALSE;
    node->msg_id = msg_id;
    node->vap_id = vap_id;
    node->sync = sync ? 1 : 0;
    node->time_out = time_out;
    frw_atomic_set(&node->wait_fail, 0);
    return node;
}
#define MAX_FRW_MSG_ALLOC_LEN 4096
WIFI_HMAC_TCM_TEXT OSAL_STATIC frw_msg_node *frw_host_alloc_msg_node(frw_msg *msg, osal_u16 msg_id, osal_u8 vap_id,
    osal_bool sync, osal_u16 time_out)
{
    osal_u8 *data;
    frw_msg_node *node = OSAL_NULL;
    osal_s32 ret;

    if (msg->data_len + msg->rsp_buf_len > MAX_FRW_MSG_ALLOC_LEN) {
        return node;
    }

    node = frw_alloc_node_mem(msg_id, vap_id, sync, time_out);
    if (node == OSAL_NULL) {
        return node;
    }

    if (msg->data_len == 0 && msg->rsp_buf_len == 0) {
        return node;
    }

    data = osal_kmalloc(msg->data_len + msg->rsp_buf_len, OSAL_GFP_ATOMIC);
    if (data == OSAL_NULL) {
        oam_error_log2(vap_id, OAM_SF_FRW, "frw_host_alloc_msg_node: alloc failed. msg_id %d sync %d", msg_id, sync);
        osal_adapt_wait_destroy(&node->wait_q);
        osal_adapt_atomic_destroy(&node->wait_fail);
        osal_kfree(node);
        node = OSAL_NULL;
        return node;
    }

    if (msg->data_len != 0 && msg->data != OSAL_NULL) {
        ret = memcpy_s(data, msg->data_len, msg->data, msg->data_len);
        if (ret != EOK) {
            frw_host_free_msg_node(node);
            return OSAL_NULL;
        }
        node->msg.data = data;
        node->msg.data_len = msg->data_len;
    }

    if (msg->rsp_buf_len != 0) {
        node->msg.rsp = data + msg->data_len;
        node->msg.rsp_buf_len = msg->rsp_buf_len;
        memset_s(node->msg.rsp, node->msg.rsp_buf_len, 0, node->msg.rsp_buf_len);
    }
    return node;
}

static inline osal_void frw_msg_cnt_dec(frw_thread_que *msg_que)
{
    if (msg_que->cnt > 0) {
        msg_que->cnt--;
    }
}

static inline osal_void frw_msg_cnt_inc(frw_thread_que *msg_que)
{
    msg_que->cnt++;
}

static inline osal_void frw_msg_queue_lock(frw_thread_que *msg_que)
{
    if (msg_que != OSAL_NULL) {
        osal_spin_lock_bh(&msg_que->lock);
    }
}

static inline osal_void frw_msg_queue_unlock(frw_thread_que *msg_que)
{
    if (msg_que != OSAL_NULL) {
        osal_spin_unlock_bh(&msg_que->lock);
    }
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_void frw_node_enqueue(frw_msg_node *node, frw_thread_que *msg_que, osal_bool sync)
{
    osal_list_add_tail(&node->list, &msg_que->list);
    frw_msg_cnt_inc(msg_que);
    if (sync) {
        node->seq = (osal_u16)msg_que->seq;
        msg_que->seq++;
    }
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_void frw_node_dequeue(frw_msg_node *node, frw_thread_que *msg_que)
{
    osal_list_del(&node->list);
    frw_msg_cnt_dec(msg_que);
}

/* 根据dmac上报的msg_id和msg_seq在device_sync_msg_q队列中查找对应的frw_msg_node */
OSAL_STATIC frw_msg_node *frw_find_msg_node(osal_u16 msg_id, osal_u32 msg_seq, const frw_thread_que *msg_que)
{
    frw_msg_node *node = OSAL_NULL;
    frw_msg_node *tmp = OSAL_NULL;

    osal_list_for_each_entry_safe(node, tmp, &msg_que->list, list) {
        if ((node->msg_id == msg_id) && (node->seq == msg_seq)) {
            return node;
        }
    }
    return OSAL_NULL;
}

OSAL_STATIC osal_void frw_msg_copy_rsp(frw_msg *msg, frw_msg_node *node)
{
    osal_u16 size = node->msg.rsp_len;
    if (size > msg->rsp_buf_len) {
        size = msg->rsp_buf_len; // 这种情况实际上不应该出现,说明回调函数没有正确处理
        oam_error_log3(0, OAM_SF_FRW, "msg[%d],frw_msg_copy_rsp:node->msg.rsp_len[%d] > msg->rsp_buf_len[%d] error.",
            node->msg_id, node->msg.rsp_len, msg->rsp_buf_len);
    }

    if (msg->rsp_buf_len != 0 && msg->rsp != OSAL_NULL && node->msg.rsp_buf_len != 0 &&
        node->msg.rsp != OSAL_NULL && size != 0) {
        if (memcpy_s(msg->rsp, msg->rsp_buf_len, node->msg.rsp, size) != EOK) {
            oam_error_log3(0, OAM_SF_FRW, "msg[%d],frw_msg_copy_rsp:memcpy_s error rsp_buf_len[%d], size[%d].",
                node->msg_id, msg->rsp_buf_len, size);
        }
        msg->rsp_len = size;
    }
    return;
}

/* 线程唤醒判断 */
OSAL_STATIC osal_s32 frw_thread_wait_cond(const void *param)
{
    return (((frw_msg_node *)param)->wait_cond == OSAL_TRUE);
}

osal_s32 frw_msg_check_frw_deadlock(osal_s32 msg_id)
{
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    frw_thread *th = &tc->frw_threads[FRW_THREAD_ID_MSG];

    if (th->pid == osal_adapt_get_current_tid()) {
        wifi_printf("\n U want crash the frw, u r a terrorist! msg[%d] frw_pid[%d]\n", msg_id, (osal_u32)th->pid);
        return OAL_ERR_CODE_THREAD_DEADLOCK;
    }
    return OAL_SUCC;
}

osal_s32 frw_msg_check_hcc_deadlock(osal_s32 msg_id)
{
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    if (tc->hcc_pid == osal_adapt_get_current_tid()) {
        wifi_printf("\n U want crash the frw, u r a terrorist! msg[%d] hcc_pid[%d]\n", msg_id, (osal_u32)tc->hcc_pid);
        return OAL_ERR_CODE_THREAD_DEADLOCK;
    }
#else
    unref_param(msg_id);
#endif

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 frw_host_post_sync_msg(frw_thread *th, frw_msg_node *node, frw_msg *msg,
    frw_thread_que_type type)
{
    osal_s32 ret = OAL_ERR_CODE_INVALID_CONFIG;
    osal_u16 msg_id = node->msg_id;
    osal_u8 vap_id = node->vap_id;
    frw_thread_que *que = &th->thread_que[type];

    frw_msg_queue_lock(que);
    if (th->status == FRW_THREAD_EXIT) {
        frw_msg_queue_unlock(que);
        return OAL_ERR_CODE_PTR_NULL;
    }
    frw_node_enqueue(node, que, OSAL_TRUE);
    frw_msg_queue_unlock(que);

    if (type == FRW_THREAD_QUE_POST_SYNC_MSG) {
        frw_wait_wakeup(&th->wq);
        return OAL_SUCC;
    } else if (type == FRW_THREAD_QUE_SEND_SYNC_MSG) {
        ret = frw_send_msg_adapt(msg_id, msg, vap_id, node->seq, OSAL_TRUE);
        if (ret == OAL_SUCC) {
            return ret;
        } else {
            oam_warning_log2(vap_id, OAM_SF_FRW, "frw_host_post_sync: send failed, ret 0x%x msg_id %d", ret, msg_id);
        }
    }

    frw_msg_queue_lock(que);
    frw_node_dequeue(node, que);
    frw_msg_queue_unlock(que);
    return ret;
}

OSAL_STATIC osal_void frw_free_fail_sync_node(frw_thread *th, frw_thread_que *que, osal_u16 msg_id, osal_u32 msg_seq)
{
    osal_bool is_empty;
    frw_msg_node *node = OSAL_NULL;

    frw_msg_queue_lock(que);
    node = frw_find_msg_node(msg_id, msg_seq, que);
    if (node != OSAL_NULL) {
        frw_node_dequeue(node, que);
    }
    is_empty = osal_list_empty(&(que->list));
    frw_msg_queue_unlock(que);
    if (node != OSAL_NULL) {
        frw_host_free_msg_node(node);
    }
    /* 线程已退出，同步消息队列为空，释放队列锁 */
    if (th->status == FRW_THREAD_EXIT && is_empty == OSAL_TRUE) {
        osal_spin_lock_destroy(&(que->lock));
    }
}

/* 根据线程id和队列类型将消息入队 */
OSAL_STATIC osal_s32 frw_host_post_sync(frw_thread_id id, frw_thread_que_type type,
    osal_u16 msg_id, osal_u8 vap_id, frw_msg *msg, osal_u16 time_out)
{
    osal_s32 ret;
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    frw_thread *th = &tc->frw_threads[id];
    frw_thread_que *que = &th->thread_que[type];
    frw_msg_node *node = OSAL_NULL;
    osal_u32 msg_seq;

    node = frw_host_alloc_msg_node(msg, msg_id, vap_id, OSAL_TRUE, time_out);
    if (node == OSAL_NULL) {
        oam_error_log1(vap_id, OAM_SF_FRW, "node alloc failed. msg_id %d", msg_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    ret = frw_host_post_sync_msg(th, node, msg, type);
    if (ret != OAL_SUCC) {
        frw_host_free_msg_node(node);
        return ret;
    }

    msg_seq = node->seq;
    /* 同步消息，则需要将调用者线程挂起，等到frw_thread执行回调函数完成，返回消息之后，再唤醒调用者线程 */
    ret = osal_wait_timeout_interruptible(&node->wait_q, frw_thread_wait_cond, node, time_out);
    if (ret == 0) {
        /* 超时，且条件为false */
        frw_atomic_set(&node->wait_fail, 1);
        oam_warning_log2(vap_id, OAM_SF_FRW, "frw_host_post_sync: thread id %d TIMEOUT! msg_id %d", id, msg_id);
        wifi_printf("frw_host_post_sync: thread id %d TIMEOUT! msg_id %d seq %d\r\n", id, msg_id, msg_seq);
        frw_free_fail_sync_node(th, que, msg_id, msg_seq);
        return OAL_ERR_CODE_CONFIG_TIMEOUT;
    } else if (ret >= 1) {
        /* cb_ret == 1: 超时，且条件为true; cb_ret > 1, 提前执行完，返回 */
        frw_msg_copy_rsp(msg, node);
        ret = node->cb_return;
        frw_host_free_msg_node(node);
        return ret;
    } else {
        /* 被信号打断       if (cb_ret == -ERESTARTSYS) */
        frw_atomic_set(&node->wait_fail, 1);
        oam_warning_log2(vap_id, OAM_SF_FRW, "frw_host_post_sync: thread id %d INTERRUPTTED! msg_id %d", id, msg_id);
        frw_free_fail_sync_node(th, que, msg_id, msg_seq);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }
}

/* 根据线程id和队列类型将消息入队 */
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_s32 frw_host_post_async(frw_thread_id id, frw_thread_que_type type,
    osal_u16 msg_id, osal_u8 vap_id, frw_msg *msg)
{
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    frw_thread *th = &tc->frw_threads[id];
    frw_thread_que *que = &th->thread_que[type];
    frw_msg_node *node = OSAL_NULL;

    node = frw_host_alloc_msg_node(msg, msg_id, vap_id, OSAL_FALSE, 0);
    if (node == OSAL_NULL) {
        oam_error_log1(vap_id, OAM_SF_FRW, "node alloc failed. msg_id %d", msg_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    frw_msg_queue_lock(que);
    if (th->status == FRW_THREAD_EXIT) {
        frw_msg_queue_unlock(que);
        frw_host_free_msg_node(node);
        return OAL_ERR_CODE_PTR_NULL;
    }
    frw_node_enqueue(node, que, OSAL_FALSE);
    frw_msg_queue_unlock(que);
    frw_wait_wakeup(&th->wq);

    return OAL_SUCC;
}

/* host侧发送同步消息给host侧 */
osal_s32 frw_host_post_msg_sync(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg)
{
    if (frw_msg_check_frw_deadlock(msg_id) != OAL_SUCC) {
        return OAL_ERR_CODE_THREAD_DEADLOCK;
    }

    return frw_host_post_sync(FRW_THREAD_ID_MSG, FRW_THREAD_QUE_POST_SYNC_MSG, msg_id, vap_id, msg, time_out);
}

/* host侧发送同步消息给device侧 */
osal_s32 frw_send_cfg_to_device_sync(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg)
{
    return frw_host_post_sync(FRW_THREAD_ID_MSG, FRW_THREAD_QUE_SEND_SYNC_MSG, msg_id, vap_id, msg, time_out);
}

/* host侧发送异步消息给host侧 */
osal_s32 frw_host_post_msg_async(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg)
{
    unref_param(pri);
    return frw_host_post_async(FRW_THREAD_ID_MSG, FRW_THREAD_QUE_POST_ASYNC_MSG, msg_id, vap_id, msg);
}

/* host侧发送异步消息给host侧 */
WIFI_HMAC_TCM_TEXT osal_s32 frw_host_post_msg(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id,
    frw_msg *msg)
{
    unref_param(pri);
    return frw_host_post_async(FRW_THREAD_ID_TXDATA, FRW_THREAD_QUE_POST_ASYNC_MSG, msg_id, vap_id, msg);
}

osal_void frw_set_data_queue_limit(osal_u32 que_size)
{
    g_frw_txdata_que_limit = que_size;
}

/* host侧异步发送报文给host侧 */
osal_s32 frw_host_post_data(osal_u16 netbuf_type, osal_u8 vap_id, oal_netbuf_stru *netbuf)
{
    frw_msg msg = {0};
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    frw_thread *th = &tc->frw_threads[FRW_THREAD_ID_TXDATA];

    /* 保存报文内存二级指针 */
    msg.data = (osal_u8 *)&netbuf;
    msg.data_len = (osal_u16)sizeof(oal_netbuf_stru **);

    if (netbuf_type == FRW_NETBUF_W2H_DATA_FRAME) {
        frw_thread_que *que = &th->thread_que[FRW_THREAD_QUE_NETBUF];

        if ((g_frw_txdata_que_limit > 0) && (que->cnt > g_frw_txdata_que_limit)) {
            oal_netbuf_free(netbuf);
            OAM_STAT_VAP_INCR(vap_id, tx_abnormal_msdu_dropped, 1);
            return OAL_SUCC;
        }
    }

    if (frw_host_post_async(FRW_THREAD_ID_TXDATA, FRW_THREAD_QUE_NETBUF, netbuf_type, vap_id, &msg) != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

/* 从device侧接收到的异步消息 */
static osal_s32 frw_host_rcv_msg_async(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg)
{
    unref_param(pri);

    return frw_host_post_async(FRW_THREAD_ID_MSG, FRW_THREAD_QUE_RCV_ASYNC_MSG, msg_id, vap_id, msg);
}

/* 注册给hcc的钩子函数，负责处理所有从hcc接收到的消息 */
static osal_s32 frw_host_rcv_msg_callback(osal_u16 msg_id, osal_u8 vap_id, osal_u32 seq, osal_s32 dev_cb_ret,
    frw_msg *msg)
{
    frw_thread_ctrl *tc = frw_get_thread_ctrl();
    frw_thread_que *que = frw_get_thread_que(FRW_THREAD_ID_MSG, FRW_THREAD_QUE_SEND_SYNC_MSG);
    frw_msg_node *node = OSAL_NULL;
    osal_u32 len;

    unref_param(tc);
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
    if (tc->hcc_pid == 0) {
        tc->hcc_pid = osal_adapt_get_current_tid();
    }
#endif

    /* FRW_THREAD_ID_MSG线程处理接收方向消息 */
    frw_msg_queue_lock(que);
    if (g_frw_thread_ctrl.frw_threads[FRW_THREAD_ID_MSG].status == FRW_THREAD_EXIT) {
        frw_msg_queue_unlock(que);
        return OAL_ERR_CODE_PTR_NULL;
    }
    node = frw_find_msg_node(msg_id, seq, que);
    if (node == OSAL_NULL) {
        frw_msg_queue_unlock(que);
        /* 同步消息已被超时释放 */
        if (msg->sync != 0) {
            return OAL_SUCC;
        }
        /* 发送完成中断由线程FRW_THREAD_ID_TXDATA处理 */
        if (msg_id == WLAN_MSG_D2H_C_CFG_TX_COMP_SCHEDULE) {
            return frw_host_post_async(FRW_THREAD_ID_TXDATA, FRW_THREAD_QUE_RCV_ASYNC_MSG, msg_id, vap_id, msg);
        } else {
            return frw_host_rcv_msg_async(msg_id, 0, vap_id, msg);
        }
    }
    frw_node_dequeue(node, que);
    frw_msg_queue_unlock(que);

    /* hmac侧下发同步配置消息，收到了device侧返回的response，需要唤醒hmac侧调用者线程 */
    len = msg->rsp_len;
    node->cb_return = dev_cb_ret;
    if ((node->msg.rsp_buf_len != 0) && (node->msg.rsp != OSAL_NULL)) {
        if (len > node->msg.rsp_buf_len) {
            len = node->msg.rsp_buf_len;
        }
        if (memcpy_s(node->msg.rsp, node->msg.rsp_buf_len, msg->rsp, len) != EOK) {
            node->cb_return = OAL_FAIL;
        }
        node->msg.rsp_len = (osal_u16)len;
    }
    if (frw_atomic_read(&node->wait_fail) != 0) {
        frw_host_free_msg_node(node);
        return OAL_SUCC;
    }

    /* 唤醒被跨hcc同步消息阻塞的frw线程 */
    node->wait_cond = OSAL_TRUE;
    frw_wait_wakeup(&node->wait_q);
    return OAL_SUCC;
}

static WIFI_HMAC_TCM_TEXT frw_msg_node *frw_node_fetch(osal_void *data)
{
    frw_thread_que *que = (frw_thread_que *)data;
    struct osal_list_head *que_list = &que->list;
    frw_msg_node *node = OSAL_NULL;

    frw_msg_queue_lock(que);
    if (osal_list_empty(que_list) != 0) {
        oam_warning_log1(0, OAM_SF_FRW, "msg_que empty, cnt %d", que->cnt);
        frw_msg_queue_unlock(que);
        return OSAL_NULL;
    }

    /* 执行完就将节点从链表摘除 */
    node = osal_list_first_entry(que_list, frw_msg_node, list);
    frw_node_dequeue(node, que);
    frw_msg_queue_unlock(que);
    return node;
}

static osal_s32 frw_msg_que_handle(osal_void *data)
{
    osal_u32 runtime_before;
    osal_u32 runtime_after;
    frw_msg_node *node;

    node = frw_node_fetch(data);
    if (node == OSAL_NULL) {
        return OAL_FAIL;
    }
    runtime_before = (osal_u32)oal_time_get_stamp_ms();
    node->cb_return = frw_msg_exec_callback(node->msg_id, node->vap_id, &node->msg);
    runtime_after = osal_get_runtime(runtime_before, (osal_u32)oal_time_get_stamp_ms());
    if (runtime_after > (TIME_OUT_MS >> 1)) {
        oam_warning_log2(0, OAM_SF_FRW, "{frw_msg_exec_callback:: execute msg_id[%d] too long time[%d]}", node->msg_id,
            runtime_after);
    }

    /* 异步消息，调用完直接返回，调用者不关心回调函数的实际返回值, 因此需要释放内存 */
    if (node->sync == 0 || frw_atomic_read(&node->wait_fail) != 0) {
        frw_host_free_msg_node(node);
        return OAL_SUCC;
    }

    /* 同步消息，调用者需要关心返回值，因此需要唤醒被挂起的调用者线程，并且，node由调用者被唤醒后释放 */
    node->wait_cond = OSAL_TRUE;
    frw_wait_wakeup(&node->wait_q);
    return OAL_SUCC;
}

static WIFI_HMAC_TCM_TEXT osal_s32 frw_netbuf_que_handle(osal_void *data)
{
    frw_thread_que *que = (frw_thread_que *)data;
    frw_msg_node *node;

    node = frw_node_fetch(data);
    if (node == OSAL_NULL) {
        return OAL_FAIL;
    }

    while (node != NULL) {
        node->cb_return = frw_netbuf_exec_callback(node->msg_id, node->vap_id, &node->msg);
        frw_host_free_msg_node(node);
        node = OSAL_NULL;
        if (que->cnt > 0) {
            node = frw_node_fetch(data);
        }
    }

    return OAL_SUCC;
}

OSAL_STATIC const frw_que_process g_frw_que_fn[FRW_THREAD_QUE_NUM] = {
    (frw_que_process)frw_msg_que_handle,   /* FRW_THREAD_QUE_POST_SYNC_MSG */
    (frw_que_process)frw_msg_que_handle,   /* FRW_THREAD_QUE_POST_ASYNC_MSG */
    OSAL_NULL,                             /* FRW_THREAD_QUE_SEND_SYNC_MSG */
    (frw_que_process)frw_msg_que_handle,   /* FRW_THREAD_QUE_RCV_ASYNC_MSG */
    (frw_que_process)frw_netbuf_que_handle /* FRW_THREAD_QUE_NETBUF */
};

OSAL_STATIC osal_void frw_clear_netbuf(osal_u32 queue_id, const frw_msg_node *cur_node)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    if (queue_id == FRW_THREAD_QUE_NETBUF || cur_node->msg_id == WLAN_MSG_H2H_CRX_RX ||
        cur_node->msg_id == WLAN_MSG_H2H_RX) {
        netbuf = *(oal_netbuf_stru **)(cur_node->msg.data);
        if (netbuf != OSAL_NULL) {
            oal_netbuf_free(netbuf);
        }
    }
}

OSAL_STATIC osal_s32 frw_queue_clear(frw_thread *th)
{
    struct osal_list_head *que_list;
    frw_msg_node *cur_node;
    frw_thread_que *cur_que;
    osal_u32 j;

    for (j = 0; j < sizeof(th->thread_que) / sizeof(th->thread_que[0]); j++) {
        cur_que = &(th->thread_que[j]);
        que_list = &(cur_que->list);

        if (j == FRW_THREAD_QUE_SEND_SYNC_MSG || j == FRW_THREAD_QUE_POST_SYNC_MSG) {
            osal_bool is_empty;
            /* 同步消息队列为空直接释放锁 */
            frw_msg_queue_lock(cur_que);
            is_empty = osal_list_empty(que_list);
            frw_msg_queue_unlock(cur_que);
            if (is_empty == OSAL_TRUE) {
                osal_spin_lock_destroy(&(cur_que->lock));
            }
            continue;
        }
        frw_msg_queue_lock(cur_que);
        while (osal_list_empty(que_list) != OSAL_TRUE) {
            cur_node = osal_list_first_entry(que_list, frw_msg_node, list);
            frw_clear_netbuf(j, cur_node);
            frw_node_dequeue(cur_node, cur_que);
            frw_host_free_msg_node(cur_node);
        }
        frw_msg_queue_unlock(cur_que);
        osal_spin_lock_destroy(&(cur_que->lock));
    }
    return OAL_SUCC;
}

static inline osal_bool frw_thread_should_wake(const frw_thread *th)
{
    return (th->thread_que[FRW_THREAD_QUE_POST_SYNC_MSG].cnt > 0 ||
            th->thread_que[FRW_THREAD_QUE_POST_ASYNC_MSG].cnt > 0 ||
            th->thread_que[FRW_THREAD_QUE_RCV_ASYNC_MSG].cnt > 0 ||
            th->thread_que[FRW_THREAD_QUE_NETBUF].cnt > 0);
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_s32 frw_thread_should_stop(const void *param)
{
    return (osal_adapt_kthread_should_stop() != 0 || frw_thread_should_wake((frw_thread *)param));
}

WIFI_HMAC_TCM_TEXT osal_bool frw_check_data_thread_exit(osal_void)
{
    return (g_frw_thread_ctrl.frw_threads[FRW_THREAD_ID_TXDATA].status == FRW_THREAD_EXIT);
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_s32 frw_task_process(osal_void *data)
{
    frw_thread *th = (frw_thread *)data;
    osal_s32 ret;
    osal_u32 n;

    th->pid = osal_adapt_get_current_tid();
    osal_adapt_kthread_set_priority(th->thread, g_frw_priority[th->id]);
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_kthread_set_affinity(th->thread, OSAL_CPU_3);
#endif
    /* 进入线程循环主逻辑 */
    while (1) {
        th->is_running = OSAL_FALSE;
        ret = osal_wait_interruptible(&th->wq, frw_thread_should_stop, th);
        if (osal_adapt_kthread_should_stop() != 0) {
            break;
        }

        if (ret == (-ERESTARTSYS)) {
            oam_warning_log1(0, OAM_SF_FRW, "frw_msg_thread: thread %d INTERRUPTTED!", osal_get_current_tid());
            break;
        }
        th->is_running = OSAL_TRUE;
        for (n = 0; n < sizeof(th->thread_que) / sizeof(th->thread_que[0]); n++) {
            frw_thread_que *que = &(th->thread_que[n]);
            if (que && que->cnt > 0 && que->que_fn) {
                (osal_void)que->que_fn(que);
            }
        }
    }
    frw_queue_clear(th);
    osal_adapt_wait_destroy(&th->wq);
    /* 清理事件表 */
    frw_msg_host_exit();
    return 0;
}

#define FRW_DUMP_MSG_CNT_MAX 4
osal_void frw_thread_dump(osal_void)
{
    osal_u32 n;
    osal_u32 i;
    frw_thread_ctrl *tc = frw_get_thread_ctrl();

    for (n = 0; n < sizeof(tc->frw_threads) / sizeof(tc->frw_threads[0]); n++) {
        frw_thread *th = &(tc->frw_threads[n]);
        wifi_printf("{frw_thread_dump:: thread_name: %s, is_running: %d}\r\n", frw_get_thread_name(n), th->is_running);
        for (i = 0; i < sizeof(th->thread_que) / sizeof(th->thread_que[0]); i++) {
            if (th->thread_que[i].cnt == 0) {
                continue;
            }
            wifi_printf("{frw_thread_dump:: queue_id:%d, msg_cnt:%d}\r\n", i, th->thread_que[i].cnt);
        }
    }
}

OSAL_STATIC osal_task *frw_thread_create(osal_kthread_handler handler, osal_u32 id, void *data)
{
    osal_task *thread = OSAL_NULL;

    if (id >= FRW_THREAD_ID_NUM) {
        return NULL;
    }
    thread = osal_adapt_kthread_create(handler, data, frw_get_thread_name(id), g_frw_stack_size[id]);
    if (thread == NULL) {
        wifi_printf("osal_adapt_kthread_create task frw_thread failed\n");
        return NULL;
    }

    osal_adapt_kthread_set_priority(thread, g_frw_priority[id]);
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_kthread_set_affinity(thread, OSAL_CPU_3);
#endif
    return thread;
}

osal_s32 frw_thread_init(osal_void)
{
    osal_u32 n;
    osal_u32 i;
    frw_thread_ctrl *tc = frw_get_thread_ctrl();

    for (n = 0; n < sizeof(tc->frw_threads) / sizeof(tc->frw_threads[0]); n++) {
        frw_thread *th = &(tc->frw_threads[n]);
        /* 注册FRW队列处理回调函数 */
        for (i = 0; i < sizeof(th->thread_que) / sizeof(th->thread_que[0]); i++) {
            OSAL_INIT_LIST_HEAD(&(th->thread_que[i].list));
            th->thread_que[i].que_fn = g_frw_que_fn[i];
            osal_spin_lock_init(&(th->thread_que[i].lock));
        }
        osal_wait_init(&th->wq);
        th->status = FRW_THREAD_RUN;
        th->thread = frw_thread_create(frw_task_process, n, (void *)th);
        th->id = n;
        if (th->thread == OSAL_NULL) {
            return OAL_FAIL;
        }
    }

    frw_host_register_rcv_hook((frw_host_rcv_callback)frw_host_rcv_msg_callback);

    return OAL_SUCC;
}

osal_s32 frw_thread_exit(osal_bool thread_stop)
{
    osal_u32 i;
    frw_thread_ctrl *tc = frw_get_thread_ctrl();

    frw_host_register_rcv_hook(OSAL_NULL);

    for (i = 0; i < sizeof(tc->frw_threads) / sizeof(tc->frw_threads[0]); i++) {
        frw_thread *th = &(tc->frw_threads[i]);
        th->status = FRW_THREAD_EXIT; /* 禁止新消息入队 */
        if (thread_stop) {
            osal_adapt_kthread_destroy(th->thread, thread_stop);
        }
    }
    return OAL_SUCC;
}

osal_u32 frw_pm_queue_empty_check(osal_void)
{
    osal_u32 n;
    frw_thread_ctrl *tc = frw_get_thread_ctrl();

    for (n = 0; n < sizeof(tc->frw_threads) / sizeof(tc->frw_threads[0]); n++) {
        frw_thread *th = &(tc->frw_threads[n]);
        if (th->thread_que[FRW_THREAD_QUE_SEND_SYNC_MSG].cnt > 0 ||
            th->thread_que[FRW_THREAD_QUE_RCV_ASYNC_MSG].cnt > 0 ||
            th->thread_que[FRW_THREAD_QUE_NETBUF].cnt > 0) {
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
