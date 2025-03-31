/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag shell proc
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_shell_proc.h"
#include <net/sock.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/in.h>
#include <linux/un.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include "securec.h"
#include "osal_types.h"
#include "soc_osal.h"
#include "soc_zdiag.h"
#include "plat_debug.h"
#include "diag_cmd_shell.h"
#include "diag_adapt_layer.h"
#include "zdiag_linux_socket.h"
#include "zdiag_adapt_layer.h"

#if defined(CONFIG_DFX_SUPPORT_SHELL_PROC)
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

#define ZDIAG_SH_CMD_MAX_STRLEN             256
#define ZDIAG_TASK_WAIT_TIMEOUT             30000
#define ZDIAG_SH_TASK_STACK_SIZE            0x1000
#define ZDIAG_SH_TASK_PRIORITY              10
#define ZDIAG_SH_TASK_EXIT_WAIT_TIMEOUT_MS  3000
#define ZDIAG_SH_PROC_WAIT_TIMEOUT_MS       16000

#define ZDIAG_SOCK_MODE_NONBLOCK   1 // flag for non-block and timeout

#define ZDIAG_SH_REPORT_SIZE_UNIT  4096

//
// Zdiag Shell Process Structure
//
typedef struct {
    osal_bool sh_task_exit_flag; /* flag indicates if shell task exits */
    osal_wait sh_task_exit_done; /* wait for shell task exit */

    osal_semaphore  sh_proc_sem; /* used to trig a cmd proc */
    osal_bool       sh_proc_wait_cond;
    osal_wait       sh_proc_wait; /* wait for shell cmd proc done */
    osal_task      *sh_proc_task;
    struct socket  *sh_sock;
    osal_char       sh_cmd_str[ZDIAG_SH_CMD_MAX_STRLEN];
    osal_char       sh_cmd_slen;
} zdiag_sh_proc_s;

static td_char *g_shell_proxy_pathstr = "/tmp/zdiag_shell_sock_pipe";

static zdiag_sh_proc_s g_zdiag_sh_proc = { 0 };

static zdiag_sh_proc_s *zdiag_shell_get_proc(osal_void)
{
    return (zdiag_sh_proc_s *)&g_zdiag_sh_proc;
}

/* Wait Condition: Random Data Ready */
static td_s32 zdiag_shell_run_cmd_wait_cond(const td_void *param)
{
    const zdiag_sh_proc_s* sh_proc = (const zdiag_sh_proc_s *)param;
    if (!sh_proc) {
        return OSAL_TRUE;
    }
    return (sh_proc->sh_proc_wait_cond == OSAL_TRUE);
}

static ext_errno zdiag_shell_proc_socket_init(td_void);

td_s32 zdiag_shell_sock_send(td_u8 *buf, td_u32 len, td_u32 *real_send)
{
    td_s32 ret;
    struct kvec vec;
    struct msghdr msg;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    *real_send = 0;
    vec.iov_base = buf;
    vec.iov_len = len;
    memset_s(&msg, sizeof(msg), 0, sizeof(msg));

    if (sh_proc->sh_sock == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    ret = kernel_sendmsg(sh_proc->sh_sock, &msg, &vec, 1, vec.iov_len);
    oal_print_dbg("[SH_PROC] sock send len: %zu, ret %d\n", vec.iov_len, ret);
    if (ret < 0) {
        return ret;
    }
    *real_send = ret;

    return EXT_ERR_SUCCESS;
}

td_s32 zdiag_shell_sock_recv(td_u8 *buf, td_u16 len, td_u32 *real_recv)
{
    struct kvec vec;
    struct msghdr msg;
    td_s32 recved_len;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    memset_s(&vec, sizeof(vec), 0, sizeof(vec));
    memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    vec.iov_base = buf;
    vec.iov_len = len;

    recved_len = kernel_recvmsg(sh_proc->sh_sock, &msg, &vec, 1, len, 0);
    if (recved_len < 0) {
        return recved_len;
    }
    buf[recved_len] = '\0';
    zdiag_printf("[SH_PROC] rcv_len=%d\r\n rcv_buf:\n%s\n\n", recved_len, buf);
    *real_recv = recved_len;
    return EXT_ERR_SUCCESS;
}

// read ret and result txt
static ext_errno zdiag_shell_report_packet(td_u16 cmd_id, diag_option *option)
{
    td_s32 ret;
    td_s32 recved_len;
    td_u8 *rsp_buf;

    // because [-Werror=frame-larger-than=], use heap instead of stack
    rsp_buf = (td_u8 *)osal_kmalloc(ZDIAG_SH_REPORT_SIZE_UNIT, OSAL_GFP_KERNEL);
    if (rsp_buf == TD_NULL) {
        oal_print_err("[SH_PROC] malloc res_buf[%d] failed!\n", ZDIAG_SH_REPORT_SIZE_UNIT);
        return EXT_ERR_FAILURE;
    }
    do {
        memset_s(rsp_buf, ZDIAG_SH_REPORT_SIZE_UNIT, 0, ZDIAG_SH_REPORT_SIZE_UNIT);
        ret = zdiag_shell_sock_recv(rsp_buf, ZDIAG_SH_REPORT_SIZE_UNIT, &recved_len);
        if (ret != EXT_ERR_SUCCESS) {
            oal_print_err("[SH_PROC] sock recv %d!\n", ret);
            osal_kfree(rsp_buf);
            return ret;
        }
        oal_print_dbg("[SH_PROC] recved_len: %d, rsp_buf:\n%s\n", recved_len, rsp_buf);
        uapi_zdiag_report_packet(cmd_id, option, rsp_buf, recved_len, TD_TRUE);
    } while (recved_len > 0);

    osal_kfree(rsp_buf);
    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_shell_run_handler(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    td_s32 ret;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    (void)memset_s(sh_proc->sh_cmd_str, ZDIAG_SH_CMD_MAX_STRLEN, 0, ZDIAG_SH_CMD_MAX_STRLEN);
    ret = memcpy_s(sh_proc->sh_cmd_str, ZDIAG_SH_CMD_MAX_STRLEN, cmd_param, cmd_param_size);
    sh_proc->sh_cmd_slen = strlen(sh_proc->sh_cmd_str);
    oal_print_dbg("[SH_PROC] sh_proc->sh_cmd_str: %s, cmd_param_size %d, sh_proc->sh_cmd_slen: %d\n",
        sh_proc->sh_cmd_str, cmd_param_size, sh_proc->sh_cmd_slen);

    if (sh_proc->sh_sock == OSAL_NULL) {
        ret = zdiag_shell_proc_socket_init();
        if (ret != EXT_ERR_SUCCESS) {
            return ret;
        }
    }

    sh_proc->sh_proc_wait_cond = OSAL_FALSE;
    osal_sem_up(&sh_proc->sh_proc_sem);
    ret = osal_wait_timeout_interruptible(&sh_proc->sh_proc_wait,
        zdiag_shell_run_cmd_wait_cond, sh_proc, ZDIAG_SH_PROC_WAIT_TIMEOUT_MS);
    if (ret == -1) {
        return EXT_ERR_FAILURE;
    }
    oal_print_dbg("[SH_PROC] Command [%s] exec ok.\n", sh_proc->sh_cmd_str);

    zdiag_shell_report_packet(cmd_id, option);
    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_shell_proc_body(td_void)
{
    const osal_char *cmd_suffix = ";";
    td_s32 ret = 0;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();
    td_u32 real_send;

    if (memcpy_s(sh_proc->sh_cmd_str + sh_proc->sh_cmd_slen, ZDIAG_SH_CMD_MAX_STRLEN - sh_proc->sh_cmd_slen,
        cmd_suffix, strlen(cmd_suffix) + 1) != EOK) {
        oal_print_err("[SH_PROC] zdiag_shell_proc_body memcpy_s error\n");
        return EXT_ERR_MEMCPYS_FAIL;
    }
    sh_proc->sh_cmd_slen = strlen(sh_proc->sh_cmd_str) + 1;
    oal_print_dbg("[SH_PROC] sh_cmd_slen: %d, sh_cmd_str: %s\n", sh_proc->sh_cmd_slen, sh_proc->sh_cmd_str);

    ret = zdiag_shell_sock_send(sh_proc->sh_cmd_str, strlen(sh_proc->sh_cmd_str), &real_send);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("[SH_PROC] sock send error%d!\n", ret);
        sock_release(sh_proc->sh_sock);
        sh_proc->sh_sock = OSAL_NULL;
        return ret;
    } else {
        oal_print_dbg("[SH_PROC] sock send succ, real_send[%d].\n", real_send);
    }

    sh_proc->sh_proc_wait_cond = OSAL_TRUE;
    osal_wait_wakeup(&sh_proc->sh_proc_wait);

    return EXT_ERR_SUCCESS;
}

static td_s32 zdiag_shell_proc_task(void *data)
{
    td_s32 ret = EXT_ERR_SUCCESS;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();
    td_u32 jiffies = osal_msecs_to_jiffies(ZDIAG_TASK_WAIT_TIMEOUT);
    sh_proc->sh_task_exit_flag = OSAL_FALSE;
    for (;;) {
        ret = osal_sem_down_timeout(&sh_proc->sh_proc_sem, jiffies); // sem_down similar to sem_lock
        if (ret != EXT_ERR_SUCCESS) {
            continue;
        }
        if ((sh_proc->sh_task_exit_flag == OSAL_TRUE) || unlikely(osal_kthread_should_stop())) {
            oal_print_err("[SH_PROC] task exit.\n");
            break;
        }
        zdiag_shell_proc_body();
    }
    osal_wait_wakeup(&sh_proc->sh_task_exit_done);

    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_shell_proc_register_api(td_void)
{
    diag_shell_api api = {0};

    api.run_sh = zdiag_shell_run_handler;
    diag_shell_api_register(&api);
    oal_print_info("[SH_PROC] register api, run_handler().\n");
    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_shell_sock_set_rcv_timeout(struct socket *sun_sock)
{
#ifdef ZDIAG_SOCK_MODE_NONBLOCK
    int retval;
    osal_timeval timeout;

    timeout.tv_sec = 1; // 1s
    timeout.tv_usec = 1; // 1us

     /* osal_sock_setsockopt */
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
    retval = kernel_setsockopt(sun_sock, SOL_SOCKET, SO_RCVTIMEO, (td_char *)&timeout, sizeof(timeout));
#else
    {
        sockptr_t optval = KERNEL_SOCKPTR((td_char *)&timeout);
        retval = sock_setsockopt(sun_sock, SOL_SOCKET, SO_RCVTIMEO_OLD, optval, sizeof(timeout));
    }
#endif

    if (retval != 0) {
        oal_print_err("[SH_PROC] un socket: set SO_RCVTIMEO fail"); /* SO_RCVTIMEO */
        sock_release(sun_sock);
        return EXT_ERR_FAILURE;
    }
#endif
    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_shell_proc_socket_init(td_void)
{
    errno_t ret;
    struct socket *sun_sock = NULL;
    int retval;
    int flags = 0; /* O_NONBLOCK */
    struct sockaddr_un sun_server;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    sh_proc->sh_sock = OSAL_NULL;
    (void)memset_s(&sun_server, sizeof(sun_server), 0, sizeof(sun_server));
    sun_server.sun_family = PF_UNIX;
    ret = strcpy_s(sun_server.sun_path, UNIX_PATH_MAX, g_shell_proxy_pathstr);
    if (ret != EOK) {
        oal_print_err("[SH_PROC] sun_path strcpy_s error %d\n", ret);
        return EXT_ERR_FAILURE;
    }
    retval = sock_create(AF_UNIX, SOCK_STREAM, 0, &sun_sock); /* 0 or IPPROTO_TCP */
    if (retval < 0) {
        oal_print_err("[SH_PROC] creating su socket error, %d\n", retval);
        return retval;
    }

    // kernel_bind, sock->ops->bind
    retval = sun_sock->ops->connect(sun_sock, (struct sockaddr *)&sun_server, sizeof(sun_server) - 1, flags);
    if (retval < 0) {
        oal_print_err("[SH_PROC] un socket connect server error\n");
        sock_release(sun_sock);
        return retval;
    }
    oal_print_info("[SH_PROC] un socket: connect ok!\n");

    retval = zdiag_shell_sock_set_rcv_timeout(sun_sock);
    if (retval < 0) {
        oal_print_err("[SH_PROC] un socket set rcv timeout error\n");
        return retval;
    }

    sh_proc->sh_sock = sun_sock;

    oal_print_info("[SH_PROC] Command [%s] init sock ok.\n", sh_proc->sh_cmd_str);
    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_shell_proc_init(td_void)
{
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    if (osal_sem_init(&sh_proc->sh_proc_sem, 0) != OSAL_SUCCESS) {
        oal_print_err("[SH_PROC] sem init error\n");
        return EXT_ERR_FAILURE;
    }

    if (osal_wait_init(&sh_proc->sh_proc_wait) != OSAL_SUCCESS) {
        osal_sem_destroy(&sh_proc->sh_proc_sem);
        oal_print_err("[SH_PROC] done_wait init failed\n");
        return EXT_ERR_FAILURE;
    }

    if (osal_wait_init(&sh_proc->sh_task_exit_done) != OSAL_SUCCESS) {
        osal_sem_destroy(&sh_proc->sh_proc_sem);
        osal_wait_destroy(&sh_proc->sh_proc_wait);
        oal_print_err("[SH_PROC] task exit done failed\n");
        return EXT_ERR_FAILURE;
    }

    sh_proc->sh_proc_task = osal_kthread_create(zdiag_shell_proc_task, OSAL_NULL, "diag_sh_proc",
        ZDIAG_SH_TASK_STACK_SIZE);
    if (sh_proc->sh_proc_task == OSAL_NULL) {
        osal_sem_destroy(&sh_proc->sh_proc_sem);
        osal_wait_destroy(&sh_proc->sh_proc_wait);
        osal_wait_destroy(&sh_proc->sh_task_exit_done);
        oal_print_err("[SH_PROC] thread create failed!");
        return EXT_ERR_FAILURE;
    }
    osal_kthread_set_priority(sh_proc->sh_proc_task, ZDIAG_SH_TASK_PRIORITY);

    zdiag_shell_proc_register_api();
    oal_print_info("[SH_PROC] zdiag_shell_proc_init! done!\n");

    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_shell_proc_exit(td_void)
{
    osal_s32 ret = EXT_ERR_FAILURE;
    zdiag_sh_proc_s *sh_proc = zdiag_shell_get_proc();

    if (sh_proc->sh_proc_task) {
        sh_proc->sh_task_exit_flag = OSAL_TRUE;
        osal_sem_up(&sh_proc->sh_proc_sem);
        osal_kthread_destroy(sh_proc->sh_proc_task, OSAL_TRUE);
        ret = osal_wait_timeout_interruptible(&sh_proc->sh_task_exit_done,
            OSAL_NULL, OSAL_NULL, ZDIAG_SH_TASK_EXIT_WAIT_TIMEOUT_MS);
        if (ret == 0) {
            oal_print_err("[SH_PROC] wait event timeout!");
            ret = EXT_ERR_TIMEOUT;
        } else if (ret < 0) {
            oal_print_err("[SH_PROC] wait error: %d!", ret);
            ret = EXT_ERR_INVALID_STATE;
        } else {
            sh_proc->sh_proc_task = OSAL_NULL;
            ret = EXT_ERR_SUCCESS;
        }
    }

    osal_wait_destroy(&sh_proc->sh_task_exit_done);
    osal_wait_destroy(&sh_proc->sh_proc_wait);
    osal_sem_destroy(&sh_proc->sh_proc_sem);

    return ret;
}
#endif
#endif