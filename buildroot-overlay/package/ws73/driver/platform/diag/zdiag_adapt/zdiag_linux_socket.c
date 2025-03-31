/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag linux socket
 * This file should be changed only infrequently and with great care.
 */
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "td_base.h"
#include "osal_types.h"
#include "oal_schedule.h"
#include "soc_osal.h"
#include "soc_zdiag_channel.h"
#include "zdiag_adapt_hcc_comm.h"
#include "zdiag_common.h" // for struct msp_mux_packet_head_stru
#include "diag_adapt_layer.h"
#include "hcc_diag_log.h"
#include "zdiag_adapt_os.h"
#include "zdiag_tx_proc.h"
#include "zdiag_lib_dbg.h"
#include "zdiag_linux_socket.h"

#define ZDIAG_SOCK_HSO_ADDR 101
#define ZDIAG_SOCK_HSO_PORT 30000
#define ZDIAG_TASK_RX_BUF_SIZE 256
#define ZDIAG_TASK_EXIT_WAIT_TIMEOUT 2000 /* unit: ms */
#define KERN_SOCK_LISTEN_WAIT_TIME 200 /* unit: ms */
#define KERN_SOCK_LISTEN_BACK_LOG 10
#define KERN_SOCK_ACCEPT_FLAGS 0 /* O_NONBLOCK is set by setsockopt */
#define KERN_SOCK_RECVMSG_BYTENUM_PER_INPUT 100
#define ZDIAG_SOCKET_WAIT_MS 1000

typedef struct {
    td_u16 port;
    struct socket *sock;
} tcp_listen_socket;

typedef struct {
    struct socket *sock;
    td_s32 conn_res;
} tcp_transmit_socket;

typedef struct {
    zdiag_socket_kernel_init_param param;
    tcp_transmit_socket trans_sock;
    osal_semaphore tx_sem;
    struct completion task_exit_done;
} zdiag_socket_ctrl;

zdiag_socket_ctrl g_zdiag_socket_ctrl;

OSAL_STATIC osal_task *g_diag_sock_rx_task = OSAL_NULL; // similar with 86's diag_tcp_task_id;
OSAL_STATIC osal_bool g_diag_task_break_flag = OSAL_FALSE;

OSAL_STATIC zdiag_socket_kernel_init_param g_diag_sock_param = {
    .diag_chan_id = 0,
    .hso_addr = ZDIAG_SOCK_HSO_ADDR,
    .port_num = ZDIAG_SOCK_HSO_PORT,
};

td_s32 osal_sock_setsockopt(struct socket *sock, int level, int optname, td_char *opt, td_u32 optlen)
{
    int ret;
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    ret = kernel_setsockopt(sock, level, optname, (char *)opt, optlen);
#else
    sockptr_t optval = KERNEL_SOCKPTR(opt);
    ret = sock_setsockopt(sock, level, optname, optval, optlen);
#endif
    if (ret) {
        oam_error("kernel setsockopt=%d\n", ret);
        return ret;
    }
    return EXT_ERR_SUCCESS;
}

/* Description: diag socket trans_conn release handler
 *  Note: Releasing listen_sock is handled after zdiag sock accept done, in zdiag_sock_rx_task
 */
td_void zdiag_sock_conn_release(td_void)
{
    if (g_zdiag_socket_ctrl.trans_sock.sock != TD_NULL) {
        osal_sem_down(&g_zdiag_socket_ctrl.tx_sem);
        sock_release(g_zdiag_socket_ctrl.trans_sock.sock);
        g_zdiag_socket_ctrl.trans_sock.sock = TD_NULL;
        osal_sem_up(&g_zdiag_socket_ctrl.tx_sem);
        diag_hso_conn_state_set(DIAG_HSO_CONN_NONE);
    }
    oal_print_dbg("zdiag_socket: release sock.\n");
}

/* Description: diag socket listen_fd release handler
 *  Note: Releasing listen_sock is handled after zdiag sock accept done, in zdiag_sock_rx_task
 */
td_void zdiag_sock_listen_release(tcp_listen_socket *listen_sock)
{
    if (listen_sock->sock) {
        sock_release(listen_sock->sock);
        listen_sock->sock = TD_NULL;
    }
}

/* Description: diag socket set option handler
 *  based on osal_sock_setsockopt
 */
ext_errno zdiag_sock_setsockopt(struct socket *cur_sock)
{
    td_s32 ret;
    td_s32 one = 1;
    osal_timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    ret = osal_sock_setsockopt(cur_sock, IPPROTO_TCP, TCP_NODELAY, (td_char *)&one, sizeof(one));
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sockopt TCP_NODELAY fail\r\n");
        return EXT_ERR_FAILURE;
    }
    ret = osal_sock_setsockopt(cur_sock, SOL_SOCKET, SO_REUSEADDR, (td_char *)&one, sizeof(one));
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sockopt SO_REUSEADDR fail\r\n");
        return EXT_ERR_FAILURE;
    }
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    ret = osal_sock_setsockopt(cur_sock, SOL_SOCKET, SO_REUSEPORT, (td_char *)&one, sizeof(one));
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sockopt SO_REUSEPORT fail\r\n");
        return EXT_ERR_FAILURE;
    }
#endif
    ret = osal_sock_setsockopt(cur_sock, SOL_SOCKET, OSAL_SO_RCVTIMEO, (td_char *)&timeout, sizeof(timeout));
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sockopt OSAL_SO_RCVTIMEO fail\r\n");
        return EXT_ERR_FAILURE;
    }
    ret = osal_sock_setsockopt(cur_sock, SOL_SOCKET, OSAL_SO_SNDTIMEO, (td_char *)&timeout, sizeof(timeout));
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sockopt OSAL_SO_SNDTIMEO fail\r\n");
        return EXT_ERR_FAILURE;
    }

    return EXT_ERR_SUCCESS;
}

/* Description: diag socket used to listen
 *  refactored based on socket_listen
 */
ext_errno zdiag_sock_listen(tcp_listen_socket *listen_sock)
{
    td_s32 ret;
    struct sockaddr_in server_addr;

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    ret = sock_create_kern(AF_INET, SOCK_STREAM, 0, &(listen_sock->sock));
#else
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, 0, &(listen_sock->sock));
#endif
    if (ret < 0) {
        oam_error("sock create fail");
        return EXT_ERR_FAILURE;
    }

    ret = zdiag_sock_setsockopt(listen_sock->sock);
    if (ret != EXT_ERR_SUCCESS) {
        oam_error("sock setsockopt fail\n");
        zdiag_sock_listen_release(listen_sock);
        return ret;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port  = (unsigned short)htons(listen_sock->port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (kernel_bind(listen_sock->sock, (struct sockaddr *)(void*)(&server_addr), sizeof(struct sockaddr_in)) < 0) {
        oam_error("sock bind fail\n");
        zdiag_sock_listen_release(listen_sock);
        return EXT_ERR_FAILURE;
    }

    ret = kernel_listen(listen_sock->sock, KERN_SOCK_LISTEN_BACK_LOG);
    if (ret < 0) {
        oam_debug("tcp listen ret=%d\r\n", ret);
        zdiag_sock_listen_release(listen_sock);
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

/* Description: diag socket access handler
 *  refactored based on socket_access
 */
ext_errno zdiag_sock_access(tcp_listen_socket *listen_sock, tcp_transmit_socket *trans_sock)
{
    int ret;

    ret = kernel_accept(listen_sock->sock, &trans_sock->sock, KERN_SOCK_ACCEPT_FLAGS);
    if (ret < 0) {
        oam_warn("kernel_accept fail=%d\r\n", ret);
        trans_sock->conn_res = -1;
        return ret;
    }

    trans_sock->conn_res = 1;
    oam_debug("tcp accept ret=%d\r\n", ret);
    return ret;
}

/* Description: diag socket recv handler
 *  refactored based on socket_recv_data
 */
td_s32 zdiag_sock_recv(tcp_transmit_socket *trans_sock, td_u8 *buf, td_u16 len)
{
    struct kvec vec;
    struct msghdr msg;
    td_s32 recved_len;

    memset_s(&vec, sizeof(vec), 0, sizeof(vec));
    memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    vec.iov_base = buf;
    vec.iov_len = len;

    recved_len = kernel_recvmsg(trans_sock->sock, &msg, &vec, 1, len, 0);
    oam_debug("rcv_len=%d\r\n", recved_len);
    return recved_len;
}

/* Description: diag socket send handler
 *  refactored based on socket_send_data
 *  ret >= 0: ret real_len
 *  ret < 0: ret failure
 */
td_u32 zdiag_sock_send(tcp_transmit_socket *trans_sock, td_u8 *buf, td_u16 len)
{
    td_s32 ret = 0;
    td_u32 send_len;
    struct kvec vec;
    struct msghdr msg;
    zdiag_ctx_stru *ctx = zdiag_get_ctx();
    vec.iov_base = buf;
    vec.iov_len = len;
    memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    zdiag_print("sock send:%d\n", len);

    osal_sem_down(&g_zdiag_socket_ctrl.tx_sem);
    if (trans_sock->conn_res <= 0) {
        ret = EXT_ERR_FAILURE;
        goto DIAG_SOCK_SEND_EXIT;
    }

    if (trans_sock->sock == TD_NULL) {
        ret = EXT_ERR_FAILURE;
        goto DIAG_SOCK_SEND_EXIT;
    }
    send_len = kernel_sendmsg(trans_sock->sock, &msg, &vec, 1, vec.iov_len);
    zdiag_print("sock send len: %u\n", (td_u32)vec.iov_len);
    if (send_len != len) {
        ret = EXT_ERR_BLOCKED;
    }
    ctx->tx_succ_cnt++;
DIAG_SOCK_SEND_EXIT:
    osal_sem_up(&g_zdiag_socket_ctrl.tx_sem);
    return ret;
}

// similar with zdiag_socket_task_body
int zdiag_sock_rx_task(td_void * unused)
{
    tcp_listen_socket listen_sock;
    ext_errno ret;
    td_u8 buf[ZDIAG_TASK_RX_BUF_SIZE];
    td_s32 rcv_len;

    // 延时1s起socket
    osal_msleep(ZDIAG_SOCKET_WAIT_MS);
    oal_print_alert("tcp_rx_task\r\n");

    for (;;) {
        if ((g_diag_task_break_flag == OSAL_TRUE) || osal_kthread_should_stop()) {
            oam_debug("task loop1 exit flag=%d\r\n", g_diag_task_break_flag);
            break;
        }
        zdiag_sock_conn_release();

        memset_s(&listen_sock, sizeof(tcp_listen_socket), 0, sizeof(tcp_listen_socket));
        memset_s(&g_zdiag_socket_ctrl.trans_sock, sizeof(tcp_transmit_socket), 0, sizeof(tcp_transmit_socket));

        listen_sock.port = g_zdiag_socket_ctrl.param.port_num;
        ret = zdiag_sock_listen(&listen_sock);
        if ((ret != EXT_ERR_SUCCESS) || (listen_sock.sock == TD_NULL)) {
            msleep(KERN_SOCK_LISTEN_WAIT_TIME);
            continue;
        }

        ret = zdiag_sock_access(&listen_sock, &g_zdiag_socket_ctrl.trans_sock);
        zdiag_sock_listen_release(&listen_sock);
        if ((ret != EXT_ERR_SUCCESS) || (g_zdiag_socket_ctrl.trans_sock.sock == TD_NULL)) {
            continue;
        }

        for (;;) {
            if ((g_diag_task_break_flag == OSAL_TRUE) || osal_kthread_should_stop()) {
                oam_debug("task loop2 exit flag=%d\r\n", g_diag_task_break_flag);
                break;
            }
            rcv_len = zdiag_sock_recv(&g_zdiag_socket_ctrl.trans_sock, buf, KERN_SOCK_RECVMSG_BYTENUM_PER_INPUT);
            if (rcv_len == -EAGAIN) {  /* 重试 */
                continue;
            } else if (rcv_len <= 0) {
                break;
            }
            diag_hso_conn_state_set(DIAG_HSO_CONN_SOCK);
            soc_diag_channel_rx_mux_char_data(g_zdiag_socket_ctrl.param.diag_chan_id, buf, rcv_len);
        }
    }

    complete(&g_zdiag_socket_ctrl.task_exit_done);

    return 0;
}

/* Description: diag socket tx hook
 *  refactored based on zdiag_socket_out_put
 */
ext_errno zdiag_sock_tx_hook(diag_pkt_malloc_result * mem)
{
    return zdiag_tx_send_host_log2diag(mem->basic.buf, mem->basic.size);
}

/* Description: diag socket tx hook
 *  buf: buffer to be output on tx socket
 */
td_u32 zdiag_sock_output_buf(td_u8 *buf, td_u16 len)
{
    return zdiag_sock_send(&g_zdiag_socket_ctrl.trans_sock, buf, len);
}

ext_errno zdiag_sock_kernel_init(zdiag_socket_kernel_init_param *param)
{
    soc_diag_channel_init(param->diag_chan_id, SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF);
    soc_diag_channel_set_connect_hso_addr(param->diag_chan_id, param->hso_addr);
    soc_diag_channel_set_tx_hook(param->diag_chan_id, zdiag_sock_tx_hook);

    diag_hso_conn_state_set(DIAG_HSO_CONN_NONE);

    g_diag_task_break_flag = OSAL_FALSE;
    g_diag_sock_rx_task = osal_kthread_create(zdiag_sock_rx_task, OSAL_NULL,
        "diag_sock_rx", ZDIAG_SOCKET_RX_TASK_PRIORITY);
    if (g_diag_sock_rx_task == OSAL_NULL) {
        oal_print_err("zdiag_sock_kernel_init:osal_kthread_create err, sock_rx_task is null.\r\n");
        soc_diag_channel_exit(param->diag_chan_id, SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF);
        return EXT_ERR_FAILURE;
    }

    g_zdiag_socket_ctrl.param = *param;
    osal_sem_init(&g_zdiag_socket_ctrl.tx_sem, 1);
    init_completion(&g_zdiag_socket_ctrl.task_exit_done);
    g_zdiag_socket_ctrl.trans_sock.sock = TD_NULL;

    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_sock_kernel_exit(zdiag_socket_kernel_init_param *param)
{
    oal_print_dbg("zdiag_socket: exit enter.\n");
    if (g_diag_sock_rx_task != OSAL_NULL) {
        g_diag_task_break_flag = OSAL_TRUE;
        oal_print_dbg("zdiag_socket: wait for task exit.\n");
        if (oal_wait_for_completion_interruptible_timeout(&g_zdiag_socket_ctrl.task_exit_done,
            (td_u32)msecs_to_jiffies(ZDIAG_TASK_EXIT_WAIT_TIMEOUT)) == 0) {
            oal_print_err("try 20 second hold, timeout");
        }

        // trans sock release
        zdiag_sock_conn_release();

        oal_print_dbg("zdiag_socket: osal_kthread_destroy.\n");
        osal_kthread_destroy(g_diag_sock_rx_task, 0);
        osal_sem_destroy(&g_zdiag_socket_ctrl.tx_sem);
        g_diag_sock_rx_task = OSAL_NULL;
    }

    if (soc_diag_channel_exit(param->diag_chan_id, SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF) != EXT_ERR_SUCCESS) {
        oal_print_dbg("zdiag_socket: soc_diag_channel_exit error.\n");
    }
    oal_print_dbg("zdiag_socket: exit end.\n");

    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_socket_channel_init(td_void)
{
    return zdiag_sock_kernel_init(&g_diag_sock_param);
}

ext_errno zdiag_socket_channel_exit(td_void)
{
    return zdiag_sock_kernel_exit(&g_diag_sock_param);
}
