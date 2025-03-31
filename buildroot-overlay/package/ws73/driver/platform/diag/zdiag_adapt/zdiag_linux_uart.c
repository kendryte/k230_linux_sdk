/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag uart utils for linux kernel
 * This file should be changed only infrequently and with great care.
 */

#include "zdiag_linux_uart.h"
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/tty.h>
#include <linux/version.h>
#include "soc_diag_cmd_id.h"
#include "soc_osal.h"
#include "securec.h"
#include "diag_cmd_connect.h"
#include "zdiag_channel.h"
#include "zdiag_filter.h"
#include "oal_schedule.h"
#include "zdiag_adapt_os.h"
#include "diag_adapt_layer.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_tx_proc.h"
#include "zdiag_lib_dbg.h"

#define ZDIAG_TASK_RX_BUF_SIZE 256

#define ZDIAG_UART_RX_TASK_PRIORITY   10
#define ZDIAG_TASK_WAIT_EXIT_TIMEOUT  (1000)

#define DIAG_UART_DEV_FILE  "/dev/ttyAMA2"
#define DIAG_UART_FILE_MODE 0644
#define DIAG_UART_RX_TASK_MSLEEP_CNT 100

typedef struct {
    td_u8 diag_chan_id;
    td_u8 hso_addr;
} zdiag_uart_kernel_init_param;

zdiag_uart_ctrl g_zdiag_uart_ctrl = {
    .uart_rx_cnt = 0,
    .uart_tx_cnt = 0,
};

zdiag_uart_kernel_init_param g_uart_init_param = {
    .diag_chan_id = DIAG_CHANNEL_ID_1,
    .hso_addr = ACORE_HSOB_ADDR,
};

OSAL_STATIC struct file *g_diag_uart_file = OSAL_NULL;
OSAL_STATIC osal_task *g_diag_uart_rx_task = OSAL_NULL;
OSAL_STATIC osal_bool g_diag_uart_rx_exit_flag = OSAL_FALSE;
OSAL_STATIC oal_completion g_uart_rx_task_done;

zdiag_uart_ctrl *zdiag_uart_get_ctrl(td_void)
{
    return &g_zdiag_uart_ctrl;
}

static struct file *zdiag_uart_file_fetch(td_void)
{
    if (g_diag_uart_file != TD_NULL) {
        return g_diag_uart_file;
    }
    g_diag_uart_file = filp_open(DIAG_UART_DEV_FILE, O_RDWR | O_NOCTTY | O_NONBLOCK, DIAG_UART_FILE_MODE);
    if ((g_diag_uart_file == TD_NULL) || IS_ERR(g_diag_uart_file)) {
        oam_warn("zdiag_uart, fd:%d\r\n", (td_s32)IS_ERR(g_diag_uart_file));
        return TD_NULL;
    }
    return g_diag_uart_file;
}

static td_void zdiag_uart_file_release(td_void)
{
    if (g_diag_uart_file != TD_NULL) {
        filp_close(g_diag_uart_file, TD_NULL);
        g_diag_uart_file = TD_NULL;
    }
}

// ext_errno zdiag_uart_output_buf(td_u8 *hcc_buf, td_u16 len)
// {
//     td_u32 ret;
//     ssize_t real_len;
//     mm_segment_t old_fs;
//     struct file *fd = g_diag_uart_file;
//     zdiag_ctx_stru *ctx = zdiag_get_ctx();

//     if (fd == TD_NULL) {
//         return EXT_ERR_FAILURE;
//     }

// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
//     old_fs = get_fs();
//     set_fs(get_ds());
// #elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     old_fs = get_fs();
//     set_fs(KERNEL_DS);
// #else
//     old_fs = force_uaccess_begin();
// #endif

//     real_len = fd->f_op->write(fd, hcc_buf, len, &fd->f_pos);
//     if (real_len == len) {
//         g_zdiag_uart_ctrl.uart_tx_cnt += real_len;
//         ctx->tx_succ_cnt++;
//         ret = EXT_ERR_SUCCESS;
//     } else {
//         ret = EXT_ERR_BLOCKED;
//     }
// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     set_fs(old_fs);
// #else
//     force_uaccess_end(old_fs);
// #endif

//     return ret;
// }
ext_errno zdiag_uart_output_buf(td_u8 *hcc_buf, td_u16 len)
{
    ssize_t real_len;
    struct file *fd = g_diag_uart_file;
    zdiag_ctx_stru *ctx = zdiag_get_ctx();

    if (fd == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    /* 直接使用 kernel_write，无需权限切换 */
    real_len = kernel_write(fd, hcc_buf, len, &fd->f_pos);
    if (real_len == len) {
        g_zdiag_uart_ctrl.uart_tx_cnt += real_len;
        ctx->tx_succ_cnt++;
        return EXT_ERR_SUCCESS;
    } else {
        return EXT_ERR_BLOCKED;
    }
}

ext_errno zdiag_uart_output_pkt(diag_pkt_malloc_result *mem)
{
    return zdiag_uart_output_buf(mem->basic.buf, mem->basic.size);
}

// static td_s32 zdiag_uart_recv_data(struct file* file, td_u8 *buf, td_u16 len)
// {
//     mm_segment_t old_fs;
//     td_s32 result;

// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
//     old_fs = get_fs();
//     set_fs(get_ds());
// #elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     old_fs = get_fs();
//     set_fs(KERNEL_DS);
// #else
//     old_fs = force_uaccess_begin();
// #endif

//     result = file->f_op->read(file, buf, len, &file->f_pos);
//     if (result > 0) {
//         file->f_pos += result;
//     }

// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     set_fs(old_fs);
// #else
//     force_uaccess_end(old_fs);
// #endif

//     return result;
// }
static td_s32 zdiag_uart_recv_data(struct file* file, td_u8 *buf, td_u16 len)
{
    td_s32 result;

    /* 直接使用 kernel_read，无需权限切换 */
    result = kernel_read(file, buf, len, &file->f_pos);
    if (result > 0) {
        file->f_pos += result;
    }

    return result;
}

int zdiag_uart_rx_task(td_void * unused)
{
    ext_errno ret = EXT_ERR_SUCCESS;
    td_u8 buf[ZDIAG_TASK_RX_BUF_SIZE];
    td_s32 rcv_len;
    struct file *fd = TD_NULL;

    oal_print_err("zdiag uart rx task enter\r\n");

    for (;;) {
        if ((g_diag_uart_rx_exit_flag == TD_TRUE) || osal_kthread_should_stop()) {
            diag_hso_conn_state_set(DIAG_HSO_CONN_NONE);
            break;
        }
        zdiag_print("zdiag_uart, msleep:%d\r\n", DIAG_UART_RX_TASK_MSLEEP_CNT);
        msleep(DIAG_UART_RX_TASK_MSLEEP_CNT);

        fd = zdiag_uart_file_fetch();
        if (fd == TD_NULL) {
            continue;
        }
        zdiag_print("zdiag_uart, recv buf=%u\r\n", (td_u32)sizeof(buf));

        rcv_len = zdiag_uart_recv_data(fd, buf, sizeof(buf));
        zdiag_print("zdiag_uart=%d, rcv_len=%d\r\n", g_uart_init_param.diag_chan_id, rcv_len);
        if (rcv_len <= 0) {
            oam_warn("zdiag_uart, recv len=%d\r\n", rcv_len);
            continue;
        }
        g_zdiag_uart_ctrl.uart_rx_cnt += rcv_len;
        diag_hso_conn_state_set(DIAG_HSO_CONN_UART);
        rcv_len = soc_diag_channel_rx_mux_char_data(g_uart_init_param.diag_chan_id, buf, rcv_len);
    }

    if (fd != TD_NULL) {
        zdiag_uart_file_release();
    }

    complete(&g_uart_rx_task_done);

    return ret;
}

// static td_void zdiag_uart_set_tty_termios(struct file *fd)
// {
//     struct termios newtio;
//     mm_segment_t old_fs;
//     int ret;

//     newtio.c_iflag = 0;
//     newtio.c_oflag = 0;
//     newtio.c_cflag = 0;
//     newtio.c_lflag = 0;
//     newtio.c_cflag |= (B921600 | CS8 | CLOCAL | CREAD);
//     newtio.c_cflag &= ~CSTOPB;
//     newtio.c_cflag &= ~PARENB; // 无奇偶校验
//     newtio.c_cflag  |= IXON | IXOFF | IXANY; // 使用软件流控

//     newtio.c_iflag |= IGNPAR; // 忽略帧错误或奇偶校验错
//     newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//     newtio.c_oflag &= ~OPOST; // 原始数据（RAW）输出

// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
//     old_fs = get_fs();
//     set_fs(get_ds());
// #elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     old_fs = get_fs();
//     set_fs(KERNEL_DS);
// #else
//     old_fs = force_uaccess_begin();
// #endif

//     if (fd->f_op->unlocked_ioctl) {
//         ret = fd->f_op->unlocked_ioctl(fd, TCFLSH, 0);
//         ret = fd->f_op->unlocked_ioctl(fd, TCSETS, (unsigned long int)(uintptr_t)&newtio);
//     } else {
//         ret = -ENOTTY;
//     }

// #if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
//     set_fs(old_fs);
// #else
//     force_uaccess_end(old_fs);
// #endif

//     oal_print_err("tty unlocked_ioctl() ret: %d\n", ret);
// }
static td_void zdiag_uart_set_tty_termios(struct file *fd)
{
    struct termios newtio;
    int ret;

    newtio.c_iflag = 0;
    newtio.c_oflag = 0;
    newtio.c_cflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cflag |= (B921600 | CS8 | CLOCAL | CREAD);
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag  |= IXON | IXOFF | IXANY;

    newtio.c_iflag |= IGNPAR;
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag &= ~OPOST;

    /* 直接调用 unlocked_ioctl，无需权限切换 */
    if (fd->f_op->unlocked_ioctl) {
        ret = fd->f_op->unlocked_ioctl(fd, TCFLSH, 0);
        ret = fd->f_op->unlocked_ioctl(fd, TCSETS, (unsigned long)&newtio);
    } else {
        ret = -ENOTTY;
    }

    oal_print_err("tty unlocked_ioctl() ret: %d\n", ret);
}

// zdiag uart initial for adapting linux kernel
static ext_errno zdiag_uart_kernel_init(td_void)
{
    struct file *fd;

    fd = zdiag_uart_file_fetch();
    if (fd == TD_NULL) {
        goto fail0;
    }

    zdiag_uart_set_tty_termios(fd);

    return EXT_ERR_SUCCESS;

fail0:
    g_diag_uart_file = TD_NULL;
    oal_print_err("diag uart file open failed\n");
    return EXT_ERR_FAILURE;
}

ext_errno zdiag_uart_channel_init(td_void)
{
    ext_errno ret;
    zdiag_uart_kernel_init_param *param = &g_uart_init_param;

    ret = zdiag_uart_kernel_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }

    soc_diag_channel_init(param->diag_chan_id, SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF);
    soc_diag_channel_set_connect_hso_addr(param->diag_chan_id, param->hso_addr);
    soc_diag_channel_set_tx_hook(param->diag_chan_id, zdiag_uart_output_pkt);

    init_completion(&g_uart_rx_task_done);
    g_diag_uart_rx_exit_flag = OSAL_FALSE;
    g_diag_uart_rx_task = osal_kthread_create(zdiag_uart_rx_task, OSAL_NULL,
        "diag_uart_rx", ZDIAG_UART_RX_TASK_PRIORITY);

    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_uart_channel_exit(td_void)
{
    oal_print_err("zdiag_uart: channel exit enter.\n");
    if (g_diag_uart_rx_task != OSAL_NULL) {
        g_diag_uart_rx_exit_flag = OSAL_TRUE;
        oal_wait_for_completion_timeout(&g_uart_rx_task_done,
            (oal_uint32)msecs_to_jiffies(ZDIAG_TASK_WAIT_EXIT_TIMEOUT));
        osal_kthread_destroy(g_diag_uart_rx_task, 0);
        g_diag_uart_rx_task = OSAL_NULL;
    }

    if (soc_diag_channel_exit(g_uart_init_param.diag_chan_id, SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF) != EXT_ERR_SUCCESS) {
        oal_print_err("zdiag_uart: channel exit error.\n");
    }

    zdiag_uart_file_release();

    return EXT_ERR_SUCCESS;
}
