/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag tx proc
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_tx_proc.h"

#include <linux/in.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "td_base.h"
#include "osal_types.h"
#include "oal_schedule.h"
#include "soc_zdiag_channel.h"
#include "zdiag_adapt_hcc_comm.h"
#include "zdiag_common.h" // for struct msp_mux_packet_head_stru
#if defined(CONFIG_DIAG_SUPPORT_SOCKET) && (CONFIG_DIAG_SUPPORT_SOCKET == 1)
#include "zdiag_linux_socket.h"
#endif
#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
#include "zdiag_linux_uart.h"
#endif
#include "diag_adapt_layer.h"
#include "hcc_diag_log.h"
#include "zdiag_adapt_os.h"
#include "zdiag_lib_dbg.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "zdiag_local_log.h"
#endif
#include "diag_cmd_connect.h"

#define ZDIAG_TASK_WAIT_TIMEOUT       30000
#define ZDIAG_TASK_WAIT_EXIT_TIMEOUT  1000
#define ZDIAG_TX_RETRY_CNT            3
#define ZDIAG_TX_RETRY_WAIT_MTIME     50

OSAL_STATIC zdiag_ctx_stru g_zdiag_ctx = { 0 };

zdiag_ctx_stru *zdiag_get_ctx(osal_void)
{
    return (zdiag_ctx_stru *)&g_zdiag_ctx;
}

// similar with 86's (z)diag_send_dms_fram
ext_errno zdiag_send_oam_log(td_u8 *buf, td_u16 buf_len, td_u32 msg_id)
{
    osal_s32 ret = OAL_SUCC;
    diag_msg_que msg = { 0 };
    zdiag_ctx_stru *ctx = zdiag_get_ctx();
    unsigned long irq_save = 0;

    if (buf == TD_NULL) {
        zdiag_print("zdiag_send_oam_log %d buf null\n", __LINE__);
        return EXT_ERR_FAILURE;
    }

    msg.msg_id = msg_id;
    msg.buf = buf;
    msg.len = buf_len; /* buf_len, the valid data len can be sent. */
    ctx->tx_all_cnt++;

    osal_spin_lock_irqsave(&ctx->tx_q_spinlock, &irq_save);
    ret = insert_diag_queue(ctx->tx_queue, &msg); /* not check queue, just check send result */
    osal_spin_unlock_irqrestore(&ctx->tx_q_spinlock, &irq_save);
    if (ret != OAL_SUCC) {
        ctx->tx_fail_cnt++;
        osal_kfree(msg.buf);
        zdiag_print("zdiag_send_oam_log %d insert_diag_queue \n", __LINE__);
        return EXT_ERR_FAILURE;
    }

    zdiag_print("insert queue a msg\n");
    osal_wait_wakeup(&ctx->diag_tx_wait);
    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_tx_send_host_log2diag(td_u8 *hcc_buf, td_u16 len)
{
    msp_mux_packet_head_stru_device *mem;
    osal_u32 length;
    osal_u8 *buf = OSAL_NULL;

    if (hcc_buf == NULL) {
        return (ext_errno)OAL_FAIL;
    }

    mem = (msp_mux_packet_head_stru_device *)hcc_buf;
    length = mem->packet_data_size + (td_u16)sizeof(msp_mux_packet_head_stru_device);
    zdiag_print("zdiag_tx_send_host_log2diag length %d\n", length);
    zdiag_print("zdiag_tx_send_host_log2diag len %d\n", len);

    buf = (osal_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (buf == OSAL_NULL) {
        return (ext_errno)OAL_FAIL;
    }
    if (memcpy_s(buf, len, hcc_buf, len) != EOK) {
        osal_kfree(buf);
        return (ext_errno)OAL_FAIL;
    }

    zdiag_send_oam_log((td_u8 *)buf, len, ID_MSG_ZDIAG_HOST_LOG_TX);
    return EXT_ERR_SUCCESS;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
static td_u32 zdiag_local_file_output(td_u8 *buf, td_u16 buf_len)
{
    td_u32 ret;
    td_u32 tmp;
    td_u16 all_heads_len;
    msp_mux_packet_head_stru *mem = (msp_mux_packet_head_stru *)buf;

    // get local log stat
    if (zdiag_log_mode_get() != DIAG_LOG_TO_FILE) {
        return 0;
    }

    all_heads_len = (td_u16)sizeof(msp_mux_packet_head_stru) + (td_u16)sizeof(msp_diag_head_ind_stru) +
        (td_u16)sizeof(diag_cmd_log_layer_ind_stru);
    if (buf_len < all_heads_len) {
        oal_print_info("buf_len: %d, all_heads_len: %d \n", buf_len, all_heads_len);
        return 0;
    }
    tmp = *(td_u32 *)(mem + 1); // save msp_diag_head_ind_stru(struct)[cmd_id(td_u16)+param_size(td_u16)]
    *(td_u32 *)(mem + 1) = buf_len - all_heads_len;
    ret = zdiag_offline_log_output((td_u8 *)(mem + 1), buf_len - sizeof(msp_mux_packet_head_stru));
    if (ret == EXT_ERR_FS_IO_ERROR) {
        oam_error("diag offline write failed! \n");
    } else if (ret == EXT_ERR_PTR_NULL) {
        oam_error("diag offline null ptr! \n");
    } else if (ret != EXT_SUCCESS) {
        oam_error("diag offline log ret:0x%x \n", ret);
    }
    *(td_u32 *)(mem + 1) = tmp; // restore msp_diag_head_ind_stru(struct)[cmd_id(td_u16)+param_size(td_u16)]
    return ret;
}
#endif
#endif

td_u32 zdiag_tx_output_buf_adapt(td_u8 *buf, td_u16 buf_len)
{
    diag_hso_conn_stat state;
    td_u32 ret = EXT_ERR_SUCCESS;

    if (zdiag_log_mode_get() == DIAG_LOG_TO_FILE) {
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
        ret = zdiag_local_file_output(buf, buf_len);
#endif
#endif
        return ret;
    }

    state = diag_hso_conn_state_get();
    zdiag_print("zdiag tx output buf, hso_conn_state %d\n", state);
    if (state == DIAG_HSO_CONN_SOCK) {
#if defined(CONFIG_DIAG_SUPPORT_SOCKET) && (CONFIG_DIAG_SUPPORT_SOCKET == 1)
        ret = zdiag_sock_output_buf(buf, buf_len);
#endif
    } else if (state == DIAG_HSO_CONN_UART) {
#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
        ret = zdiag_uart_output_buf(buf, buf_len);
#endif
    }
    return ret;
}

static td_u32 zdiag_send_data_to_pc(const diag_msg_que *msg)
{
    if ((msg->buf == 0) || (msg->len == 0)) {
        return EXT_ERR_DIAG_INVALID_PARAMETER;
    }

    return zdiag_tx_output_buf_adapt(msg->buf, msg->len);
}

static td_u32 zdiag_tx_msg_free(diag_msg_que *msg)
{
    td_u32 ret = EXT_ERR_SUCCESS;

    /* free msg buffer */
    zdiag_print("zdiag msg free: msg->msg_id %d\n", msg->msg_id);
    osal_kfree(msg->buf);
    msg->buf = OSAL_NULL;

    return ret;
}

/* Description : diag tx process func called by task. */
static td_u32 zdiag_tx_msg_proc(diag_msg_que *msg)
{
    td_u32 ret = EXT_ERR_SUCCESS;

    zdiag_print("zdiag tx msg proc: msg->msg_id %d\n", msg->msg_id);
    if ((msg->msg_id == ID_MSG_ZDIAG_CHIP_LOG_TX) ||
        (msg->msg_id == ID_MSG_ZDIAG_HOST_LOG_TX)) {
        /* use diag buffer */
        ret = zdiag_send_data_to_pc(msg);
    } else {
        ret = EXT_ERR_DIAG_NOT_FOUND;
    }
    return ret;
}

static td_s32 zdiag_tx_thread_should_stop(const void *param)
{
    zdiag_ctx_stru *ctx = (zdiag_ctx_stru *)param;
    return (osal_kthread_should_stop() != 0 || ctx->tx_queue->len > 0);
}

static td_s32 zdiag_tx_proc_task(osal_pvoid unused)
{
    diag_msg_que msg;
    td_s32 ret = EXT_ERR_SUCCESS;
    zdiag_ctx_stru *ctx = zdiag_get_ctx();
    unsigned long irq_save = 0;
    td_u32 send_retry = 0;

    for (;;) {
        osal_wait_timeout_interruptible(&ctx->diag_tx_wait, zdiag_tx_thread_should_stop,
            ctx, ZDIAG_TASK_WAIT_TIMEOUT);
        if (osal_kthread_should_stop() != 0) {
            oal_print_alert("zdiag tx proc break. \n");
            break;
        }

        (osal_void)memset_s(&msg, sizeof(diag_msg_que), 0, sizeof(diag_msg_que));
        osal_spin_lock_irqsave(&ctx->tx_q_spinlock, &irq_save);
        ret = output_diag_queue(ctx->tx_queue, &msg);
        osal_spin_unlock_irqrestore(&ctx->tx_q_spinlock, &irq_save);
        if (ret != OAL_SUCC) {
            continue;
        }
        zdiag_print("output queue a msg, %d, len:%d\n", msg.msg_id, msg.len);

        /* 发送阻塞，等待重试 */
        while (zdiag_tx_msg_proc(&msg) == EXT_ERR_BLOCKED && send_retry < ZDIAG_TX_RETRY_CNT) {
            osal_msleep(ZDIAG_TX_RETRY_WAIT_MTIME);
            send_retry++;
        }
        zdiag_tx_msg_free(&msg);
        if (send_retry >= ZDIAG_TX_RETRY_CNT) {
            ctx->tx_fail_cnt++;
        }
        send_retry = 0;
    }

    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_tx_proc_init(td_void)
{
    td_s32 ret;
    zdiag_ctx_stru *ctx = zdiag_get_ctx();

    ret = osal_wait_init(&ctx->diag_tx_wait); // encap sema_init
    if (ret != OSAL_SUCCESS) {
        oam_error("diag proc tx_queue wait init failed, ret:%d", ret);
        return EXT_ERR_FAILURE;
    }

    ret = osal_spin_lock_init(&ctx->tx_q_spinlock);
    if (ret != OSAL_SUCCESS) {
        osal_wait_destroy(&ctx->diag_tx_wait);
        oam_error("diag proc tx_queue spinlock init failed, ret:%d", ret);
        return EXT_ERR_FAILURE;
    }

    ctx->tx_queue = init_diag_queue();
    if (ctx->tx_queue == TD_NULL) {
        osal_spin_lock_destroy(&ctx->tx_q_spinlock);
        osal_wait_destroy(&ctx->diag_tx_wait);
        oam_error("diag proc tx_queue fail!\r\n");
        return EXT_ERR_FAILURE;
    }
    ctx->tx_fail_cnt = 0;
    ctx->tx_succ_cnt = 0;
    ctx->tx_all_cnt = 0;

    /* create thread for diag to get data from tx queue */
    ctx->diag_tx_proc_task = osal_kthread_create(zdiag_tx_proc_task, OSAL_NULL,
        "diag_tx_proc", ZDIAG_TX_PROC_TASK_PRIORITY); /* 优先级10 */
    if (ctx->diag_tx_proc_task == OSAL_NULL) {
        osal_spin_lock_destroy(&ctx->tx_q_spinlock);
        osal_wait_destroy(&ctx->diag_tx_wait);
        deinit_diag_queue(ctx->tx_queue);
        ctx->tx_queue = TD_NULL;
        oam_error("diag create diag_tx_proc_task fail!\r\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_tx_proc_exit(td_void)
{
    zdiag_ctx_stru *ctx = zdiag_get_ctx();

    if (ctx->diag_tx_proc_task != TD_NULL) {
        osal_kthread_destroy(ctx->diag_tx_proc_task, OSAL_TRUE);
        ctx->diag_tx_proc_task = TD_NULL;
    }

    if (ctx->tx_queue != TD_NULL) {
        unsigned long irq_save = 0;
        osal_spin_lock_irqsave(&ctx->tx_q_spinlock, &irq_save);
        deinit_diag_queue(ctx->tx_queue);
        ctx->tx_queue = TD_NULL;
        osal_spin_unlock_irqrestore(&ctx->tx_q_spinlock, &irq_save);
    }
    ctx->tx_fail_cnt = 0;
    ctx->tx_succ_cnt = 0;
    ctx->tx_all_cnt = 0;

    osal_wait_destroy(&ctx->diag_tx_wait);
    osal_spin_lock_destroy(&ctx->tx_q_spinlock);

    return EXT_ERR_SUCCESS;
}
