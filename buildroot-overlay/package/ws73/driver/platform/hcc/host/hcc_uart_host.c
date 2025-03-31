/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: UART Function Implementation
 * Author: @CompanyNameTag
 */

#ifdef CONFIG_HCC_SUPPORT_UART
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/serial_core.h>

#include "hcc.h"
#include "hcc_if.h"
#include "hcc_list.h"
#include "hcc_bus.h"
#include "hcc_bus_types.h"
#include "hcc_adapt.h"
#include "hcc_comm.h"
#include "hcc_adapt_uart.h"
#include "hcc_dfx.h"
#include "hcc_test.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
#include "hcc_uart_user_ctrl.h"
#endif
#include "hcc_uart_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PACKET_START_SIGNAL 0x7e
#define PACKET_END_SIGNAL   0xe7

#define HCC_UART_DMA_DATA_ALIGN   1
#define UART_MSG_LEN        6
#define UART_DATA_MAX_LEN   2176
#define RX_PACKET_ERR      (-1)
#define BYTE_BIT_LEN        8

#define N_HW_BFG           (NR_LDISCS - 1)

#define UART_RETRY_TOTAL_CNT    5
#define UART_RETRY_CNT          500
#define UART_BAUD_RATE_CONFIG UART_BAUD_RATE_1M

#define FLOW_CTRL_ENABLE  1
#define FLOW_CTRL_DISABLE 0
#define PUBLIC_BUF_MAX (8 * 1024)
#define TTY_READ_TIMOUT_DEFAULT_MS  1000
#define TTY_WRITE_TIMOUT_DEFAULT_MS 1000

#define DELAY_500MS 500

typedef struct uart_packet_end_t {
    td_u8 packet_end;
} uart_packet_end;

typedef enum {
    HCC_UART_TYPE_DATA,
    HCC_UART_TYPE_MSG,
    HCC_UART_TYPE_FRAG,
    HCC_UART_PKT_TYPE_MAX,
} hcc_uart_packet_type;

static td_u32 g_tty_read_timeout_ms = TTY_READ_TIMOUT_DEFAULT_MS;

struct platform_device *g_uart_device = TD_NULL;
struct tty_struct *g_tty_device = TD_NULL;
hcc_bus *g_uart_bus = TD_NULL;
td_s32 (*hcc_tty_recv_func)(td_void *, const td_u8 *, td_s32);
typedef struct {
    td_u64 recv_total_len;
    td_u32 head_err_cnt;
    td_u32 tail_err_cnt;
    td_u32 type_err_cnt;
    td_u32 queid_err_cnt;
    td_u32 msg_err_cnt;
    td_u32 length_err_cnt;
} hcc_uart_recv_stat;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
enum TTY_COMPLETE_TYPE_ENUM {
    TTY_LDISC_UNINSTALL    = 0,   /* 通知octty关闭uart */
    TTY_LDISC_INSTALL      = 1,   /* 通知octty打开uart */
    TTY_LDISC_RECONFIG     = 2,   /* 通知octty修改uart的配置参数，波特率和流控 */
    TTY_LDISC_TYPE_BUTT,
};
#endif
hcc_uart_recv_stat g_uart_recv_stat;

hcc_uart_priv_data *hcc_get_uart_priv_data(td_void)
{
    if (g_uart_device == NULL) {
        return NULL;
    }
    return (hcc_uart_priv_data *)dev_get_drvdata(&g_uart_device->dev);
}

td_u32 tty_read_timeout_get(td_void)
{
    return g_tty_read_timeout_ms;
}

td_void tty_read_timeout_set(td_u32 timeout_ms)
{
    g_tty_read_timeout_ms = timeout_ms;
}

static td_void hcc_dfx_uart_recv_head_err_inc(td_void)
{
    g_uart_recv_stat.head_err_cnt++;
}

static td_void hcc_dfx_uart_recv_type_err_inc(td_void)
{
    g_uart_recv_stat.type_err_cnt++;
}

static td_void hcc_dfx_uart_recv_queid_err_inc(td_void)
{
    g_uart_recv_stat.queid_err_cnt++;
}

static td_void hcc_dfx_uart_recv_length_err_inc(td_void)
{
    g_uart_recv_stat.length_err_cnt++;
}

static td_void hcc_dfx_uart_recv_tail_err_inc(td_void)
{
    g_uart_recv_stat.tail_err_cnt++;
}

static td_void hcc_dfx_uart_recv_msg_err_inc(td_void)
{
    g_uart_recv_stat.msg_err_cnt++;
}

static td_void hcc_dfx_uart_recv_total_len_stat(td_u32 len)
{
    g_uart_recv_stat.recv_total_len += len;
}

static td_void hcc_uart_dfx_recv_stat_print(td_void)
{
    hcc_dfx_print("uart recv_total_len %llu\r\n", g_uart_recv_stat.recv_total_len);
    hcc_dfx_print("uart recv head_err_cnt %u\r\n", g_uart_recv_stat.head_err_cnt);
    hcc_dfx_print("uart recv tail_err_cnt %u\r\n", g_uart_recv_stat.tail_err_cnt);
    hcc_dfx_print("uart recv type_err_cnt %u\r\n", g_uart_recv_stat.type_err_cnt);
    hcc_dfx_print("uart recv queid_err_cnt %u\r\n", g_uart_recv_stat.queid_err_cnt);
    hcc_dfx_print("uart recv msg_err_cnt %u\r\n", g_uart_recv_stat.msg_err_cnt);
    hcc_dfx_print("uart recv msg_err_cnt %u\r\n", g_uart_recv_stat.length_err_cnt);
}

/*
 * Prototype    : ps_write_tty
 * Description  : can be called when write data to uart
 *                This is the internal write function - a wrapper
 *                to tty->ops->write
 * input        : data -> the ptr of send data buf
 *                count -> the size of send data buf
 *                hcc_flag -> if the caller is hcc thread, the value is true
 */
td_s32 ps_write_tty(const td_u8 *data, td_u32 count, td_bool hcc_flag)
{
    td_s32 ret;
    td_s32 tty_write_cnt;
    td_s32 total_write_cnt = (td_s32)count;
    td_s32 write_fail_cnt = 0;
    struct tty_struct *tty = g_tty_device;
    const td_u8 *write_pos = data;
    hcc_uart_priv_data *uart_priv = TD_NULL;
    struct platform_device *dev = g_uart_device;
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);

    if (g_tty_device == TD_NULL || data == TD_NULL || dev == TD_NULL) {
        hcc_printf_err_log("ps_write_tty param err\r\n");
        return EXT_ERR_FAILURE;
    }
    uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("uart_priv is null\r\n");
        return EXT_ERR_FAILURE;
    }

    osal_mutex_lock(&uart_priv->tx_lock);
    while (total_write_cnt > 0) {
        tty_write_cnt = tty->ops->write(tty, write_pos, total_write_cnt);
        if (hcc_flag && osal_atomic_read(&hcc->hcc_state) != HCC_ON) {  // dfr场景
            hcc_printf_err_log("dfr happen, left len %d\r\n", total_write_cnt);
            break;
        }
        if (tty_write_cnt <= 0) {
            ret = osal_wait_timeout_interruptible(&uart_priv->tx_wait, NULL, NULL, TTY_WRITE_TIMOUT_DEFAULT_MS);
            if (ret <= 0 && write_fail_cnt++ > UART_RETRY_TOTAL_CNT) {
                osal_mutex_unlock(&uart_priv->tx_lock);
                hcc_printf_err_log("ps_write_tty ret=%d cnt=%d written=%u total=%u\r\n",
                    tty_write_cnt, write_fail_cnt, write_pos - data, count);
                return EXT_ERR_FAILURE;
            }
            continue;
        } else {
            write_fail_cnt = 0;
            write_pos = write_pos + tty_write_cnt;
        }
        total_write_cnt = total_write_cnt > tty_write_cnt ? total_write_cnt - tty_write_cnt : 0;
    }
    osal_mutex_unlock(&uart_priv->tx_lock);
    return EXT_ERR_SUCCESS;
}

static td_s32 rx_wait_cond_func(const void *data)
{
    struct platform_device *dev = g_uart_device;
    hcc_uart_priv_data *uart_priv = dev_get_drvdata(&dev->dev);
    return uart_priv->rx_cur_len >= (td_s32)(uintptr_t)data;
}
td_s32 ps_read_tty(td_u8 *data, td_s32 len)
{
    hcc_uart_priv_data *uart_priv = TD_NULL;
    struct platform_device *dev = g_uart_device;

    if (dev == TD_NULL || data == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("uart_priv is null\r\n");
        return -EFAULT;
    }

    if (osal_wait_timeout_interruptible(&uart_priv->rx_wait, rx_wait_cond_func,
        (void *)(uintptr_t)len, g_tty_read_timeout_ms) == 0) {
        hcc_printf_err_log("ps_read_tty timeout, len %u, cur_len %u\r\n", len, uart_priv->rx_cur_len);
        return -EFAULT;
    }

    osal_spin_lock(&uart_priv->rx_lock);
    if (memcpy_s(data, len, uart_priv->rx_raw_buf, len) != EOK) {
        osal_spin_unlock(&uart_priv->rx_lock);
        return EXT_ERR_FAILURE;
    }

    if (memmove_s(uart_priv->rx_raw_buf, uart_priv->rx_max_len,
                  &(uart_priv->rx_raw_buf[len]), (uart_priv->rx_cur_len - len)) != EOK) {
        hcc_printf_err_log("ps_read_tty: memmove_s fail\r\n");
    }
    uart_priv->rx_cur_len -= (td_u32)len;
    osal_spin_unlock(&uart_priv->rx_lock);
    return len;
}

static td_s32 patch_tty_receive(td_void *disc_data, const td_u8 *data, td_s32 len)
{
    td_u32 cur_len;
    td_u32 max_len;
    hcc_uart_priv_data *uart_priv = (hcc_uart_priv_data *)disc_data;
    if (disc_data == TD_NULL || data == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    osal_spin_lock(&uart_priv->rx_lock);
    cur_len = uart_priv->rx_cur_len;
    max_len = uart_priv->rx_max_len;
    if (cur_len + (td_u32)len <= max_len) {
        if (memcpy_s(&(uart_priv->rx_raw_buf[cur_len]), max_len - cur_len, data, len) != EOK) {
            osal_spin_unlock(&uart_priv->rx_lock);
            return EXT_ERR_FAILURE;
        }
        uart_priv->rx_cur_len = cur_len + (td_u32)len;
        osal_wait_wakeup(&uart_priv->rx_wait);
    } else {
        osal_spin_unlock(&uart_priv->rx_lock);
        hcc_printf_err_log("uart rx_raw_buf is full\r\n");
        return EXT_ERR_FAILURE;
    }
    osal_spin_unlock(&uart_priv->rx_lock);
    return EXT_ERR_SUCCESS;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static td_s32 ps_uart_pm_resume(struct tty_struct *tty)
{
    td_s32 try_cnt = 0;
    struct uart_state *state = TD_NULL;

    state = (struct uart_state *)(tty->driver_data);

    while (state->uart_port->suspended != 0) {
        osal_msleep(10); // 单次等待10ms
        try_cnt++;
        if (try_cnt > 300) { // 最多等待300次(3s)
            hcc_printf_err_log("wait port resume timeout\r\n");
            return -ETIME;
        }
    }
    hcc_debug("tty wait pm resume cnt %d\r\n", try_cnt);
    return EXT_ERR_SUCCESS;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static struct tty_struct *ps_tty_kopen(const char* dev_name)
{
    struct tty_struct *tty = TD_NULL;
    int ret;
    dev_t dev_no;

    ret = tty_dev_name_to_number(dev_name, &dev_no);
    if (ret != 0) {
        hcc_printf_err_log("can't found tty:%s ret=%d\r\n", dev_name, ret);
        return TD_NULL;
    }

    /* open tty */
    tty = tty_kopen(dev_no);
    if (IS_ERR_OR_NULL(tty)) {
        hcc_printf_err_log("open tty %s failed ret=%d\r\n", dev_name, PTR_ERR_OR_ZERO(tty));
        return TD_NULL;
    }

    if (ps_uart_pm_resume(tty) < 0) {
        tty_unlock(tty);
        hcc_printf_err_log("tty open wait exit suspend fail\r\n");
        return TD_NULL;
    }

    if (tty->ops->open) {
        ret = tty->ops->open(tty, TD_NULL);
    } else {
        hcc_printf_err_log("tty->ops->open is NULL\r\n");
        ret = -ENODEV;
    }

    if (ret != 0) {
        tty_unlock(tty);
        return TD_NULL;
    } else {
        g_tty_device = tty;
        return tty;
    }
}
#endif

static void ps_close_tty_drv(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    /* 内核关闭tty */
    struct tty_struct *tty = g_tty_device;

    /* close tty */
    if (tty == TD_NULL) {
        hcc_printf_err_log("tty is null, ignore\r\n");
        return;
    }

    tty_lock(tty);
    if (tty->ops->close) {
        tty->ops->close(tty, TD_NULL);
    } else {
        hcc_printf_err_log("tty->ops->close is null\r\n");
    }
    tty_unlock(tty);
    tty_kclose(tty);
#else
    struct platform_device *dev = g_uart_device;
    hcc_uart_priv_data *uart_priv = dev_get_drvdata(&dev->dev);
    /* 通知octty关闭tty */
    uart_priv->ldisc_install = TTY_LDISC_UNINSTALL;
    sysfs_notify(hcc_get_sys_object(), NULL, "install");
#endif
}

static void ps_ktty_set_termios(struct tty_struct *tty, long baud_rate, td_u8 enable_flowctl)
{
    struct ktermios ktermios;

    ktermios = tty->termios;
    /* close soft flowctrl */
    ktermios.c_iflag &= ~IXON;
    /* set uart cts/rts flowctrl */
    ktermios.c_cflag &= ~CRTSCTS;
    if (enable_flowctl == FLOW_CTRL_ENABLE) {
        ktermios.c_cflag |= CRTSCTS;
    }

    /* set csize */
    ktermios.c_cflag &= ~(CSIZE);
    ktermios.c_cflag |= CS8;
    /* set uart baudrate */
    ktermios.c_cflag &= ~CBAUD;
    ktermios.c_cflag |= BOTHER;
    tty_termios_encode_baud_rate(&ktermios, baud_rate, baud_rate);
    tty_set_termios(tty, &ktermios);
    hcc_debug("set baud_rate=%d, except=%d\r\n", (int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

#define PLAT_WAIT_TTY_OPEN_TIMEOUT_MS 1000000
static td_s32 ps_open_tty_drv(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    /* 内核打开tty */
    td_s32 ret;
    struct tty_struct *tty = TD_NULL;
    const char *tty_name = CONFIG_TTY_DEV_NAME;

    /* open tty */
    tty = ps_tty_kopen(tty_name);
    if (tty == TD_NULL) {
        hcc_printf_err_log("failed to open ktty\r\n");
        return -ENODEV;
    }

    ps_ktty_set_termios(tty, UART_BAUD_RATE_1M, FLOW_CTRL_DISABLE);
    // tty_lock在tty_kopen时执行，成功时不释放锁，只有失败的时候才会释放
    tty_unlock(tty);

    /* set line ldisc */
    ret = tty_set_ldisc(tty, N_HW_BFG); /* export after 4.13 */
    if (ret != 0) {
        hcc_printf_err_log("failed to set ldisc on tty, ret=%d\r\n", ret);
        return ret;
    }
    hcc_debug("hcc open tty success\r\n");
#else
    const char *tty_name = CONFIG_TTY_DEV_NAME;
    hcc_uart_priv_data *uart_priv = dev_get_drvdata(&g_uart_device->dev);
    /* 通知octty打开tty */
    init_completion(&uart_priv->ldisc_installed);
    uart_priv->ldisc_install = TTY_LDISC_INSTALL;
    /* 设置uart的参数，octty会来读取 */
    (td_void)memset_s(uart_priv->dev_name, UART_DEV_NAME_LEN, 0, UART_DEV_NAME_LEN);
    (td_void)memcpy_s((char *)uart_priv->dev_name, UART_DEV_NAME_LEN, tty_name, strlen(tty_name));
    uart_priv->flow_cntrl = FLOW_CTRL_ENABLE;
    uart_priv->baud_rate = UART_BAUD_RATE_1M;
    while (!is_occtty_ready()) {
        sysfs_notify(hcc_get_sys_object(), NULL, "install");
        osal_msleep(DELAY_500MS);
    }
    if (wait_for_completion_timeout(&uart_priv->ldisc_installed,
        (td_u32)msecs_to_jiffies(PLAT_WAIT_TTY_OPEN_TIMEOUT_MS)) == 0) {
        hcc_printf_err_log("wait tty open timeout %d ms ", PLAT_WAIT_TTY_OPEN_TIMEOUT_MS);
        return EXT_ERR_FAILURE;
    }
    /* 等待20ms,等octty波特率切换完成 */
    osal_msleep(20);
#endif
    hcc_printf_err_log("tty has open\n");
    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_uart_check_rx_format(hcc_handler *hcc, td_u8 *buf, td_u32 len, hcc_uart_priv_data *uart_priv)
{
    uart_packet_head *uart_head = (uart_packet_head *)buf;
    td_u16 data_len = uart_head->len_l | (uart_head->len_h << BYTE_BIT_LEN);

    if (uart_head->packet_start != PACKET_START_SIGNAL) {
        hcc_printf_err_log("rx format start err:0x%x, len-%u, total-%u\r\n",
            uart_head->packet_start, len, uart_priv->rx_decode_total);
        hcc_dfx_uart_recv_head_err_inc();
        return RX_PACKET_ERR;
    }
    /* if count less then sys packet lenth and continue */
    if (len < UART_MSG_LEN) {
        uart_priv->rx_decode_len = len;
        return HCC_UART_TYPE_FRAG;
    }
    if (uart_head->packet_info.type >= HCC_UART_TYPE_FRAG) {
        hcc_printf_err_log("rx format type err:%d-%d\r\n", uart_head->packet_info.type, len);
        hcc_dfx_uart_recv_type_err_inc();
        return RX_PACKET_ERR;
    }

    if (uart_head->packet_info.type == HCC_UART_TYPE_DATA) {
        if (uart_head->packet_info.queue_id >= hcc->que_max_cnt) {
            hcc_printf_err_log("rx format queue_id err:%d-%d\r\n", uart_head->packet_info.queue_id, len);
            hcc_dfx_uart_recv_queid_err_inc();
            return RX_PACKET_ERR;
        }
        if ((data_len > UART_DATA_MAX_LEN) || (data_len < UART_MSG_LEN)) {
            hcc_printf_err_log("rx format len err:%d-%d\r\n", data_len, len);
            hcc_dfx_uart_recv_length_err_inc();
            return RX_PACKET_ERR;
        }
    } else if (uart_head->packet_info.type == HCC_UART_TYPE_MSG) {
        if (data_len != UART_MSG_LEN) {
            hcc_printf_err_log("rx format msglen err:%d-%d\r\n", data_len, len);
            hcc_dfx_uart_recv_msg_err_inc();
            return RX_PACKET_ERR;
        }
    }

    if (len < data_len) {
        uart_priv->rx_decode_len = len;
        return HCC_UART_TYPE_FRAG;
    } else if (buf[data_len - 1] != PACKET_END_SIGNAL) {
        hcc_printf_err_log("rx format end err:0x%x, len:%d-%d\r\n", buf[data_len - 1], data_len, len);
        hcc_dfx_uart_recv_tail_err_inc();
        return RX_PACKET_ERR;
    }
    uart_priv->rx_decode_len = data_len;
    return uart_head->packet_info.type;
}


static td_u32 hcc_uart_rx_proc(hcc_handler *hcc, td_u8 *buf)
{
    hcc_unc_struc *unc_buf;
    uart_packet_head *uart_head = (uart_packet_head *)buf;
    td_u16 data_len = uart_head->len_l | (uart_head->len_h << BYTE_BIT_LEN);

    if ((data_len <= sizeof(uart_packet_head) + 1) || (data_len > UART_DATA_MAX_LEN)) {
        hcc_printf_err_log("hcc_uart_rx_proc data_len fail:%d\r\n", data_len);
        return EXT_ERR_FAILURE;
    }
    data_len -= sizeof(uart_packet_head) + 1;

    unc_buf = hcc_adapt_alloc_unc_buf(hcc, data_len, uart_head->packet_info.queue_id);
    if (unc_buf != TD_NULL) {
        if (memcpy_s(unc_buf->buf, data_len, buf + sizeof(uart_packet_head), data_len) != EOK) {
            hcc_adapt_mem_free(hcc, unc_buf);
            return EXT_ERR_FAILURE;
        }
        hcc_unc_buf_enqueue(hcc, uart_head->packet_info.queue_id, unc_buf);
        return EXT_ERR_SUCCESS;
    } else {
        hcc_printf_err_log("hcc_uart_rx_proc malloc fail:%d\r\n", uart_head->packet_info.queue_id);
        return EXT_ERR_FAILURE;
    }
}

static td_void hcc_uart_msg_proc(td_u8 *buf)
{
    td_u8 msg_id = *(buf + sizeof(uart_packet_head));
    hcc_bus_call_rx_message(g_uart_bus, msg_id);
}

static td_void hcc_tty_rx_buf_set(hcc_uart_priv_data *uart_priv, td_u32 data_len)
{
    uart_priv->rx_decode_total = 0;
    uart_priv->rx_cur_len = data_len;
    uart_priv->rx_decode_buf = uart_priv->rx_raw_buf;
}

static td_bool hcc_rx_buf_update(hcc_uart_priv_data *uart_priv, const td_u8 *data, td_u32 count)
{
    td_u8 *ptr = TD_NULL;
    if (count <= 0) {
        return TD_FALSE;
    }

    if (uart_priv->rx_cur_len > 0) {
        /* if curr packet is breaked package, first copy all uart buf data to public buf */
        ptr = uart_priv->rx_raw_buf + uart_priv->rx_cur_len;
        if (unlikely(memcpy_s(ptr, uart_priv->rx_max_len - uart_priv->rx_cur_len,
            data, count) != EOK)) {
            hcc_tty_rx_buf_set(uart_priv, 0);
            return TD_FALSE;
        }
    } else {
        /* if not breaked package, direct decode in uart buf */
        uart_priv->rx_decode_buf = (td_u8 *)data;
    }

    uart_priv->rx_cur_len += count;
    return TD_TRUE;
}

static td_void hcc_uart_recv_report_data(hcc_handler *hcc, td_u8 *buf, td_s32 rx_format)
{
    if (rx_format == HCC_UART_TYPE_DATA) {
        hcc_uart_rx_proc(hcc, buf);
    } else if (rx_format == HCC_UART_TYPE_MSG) {
        hcc_uart_msg_proc(buf);
    }
}

static td_s32 hcc_uart_recv_decode_data(hcc_uart_priv_data *uart_priv, td_s32 *out_len)
{
    td_s32 rx_format;
    td_s32 data_len = *out_len;
    hcc_handler *hcc = (hcc_handler *)(g_uart_bus->hcc);

    rx_format = hcc_uart_check_rx_format(hcc, uart_priv->rx_decode_buf, data_len, uart_priv);
    if (rx_format == HCC_UART_TYPE_DATA || rx_format == HCC_UART_TYPE_MSG) {
        hcc_uart_recv_report_data(hcc, uart_priv->rx_decode_buf, rx_format);
        uart_priv->rx_decode_total += uart_priv->rx_decode_len;
        uart_priv->rx_decode_buf += uart_priv->rx_decode_len;
        data_len = data_len - (td_s32)uart_priv->rx_decode_len;
    } else if (rx_format == HCC_UART_TYPE_FRAG) {
        /* copy breaked pkt to buf head */
        if (uart_priv->rx_raw_buf != uart_priv->rx_decode_buf) {
            if (unlikely(memmove_s(uart_priv->rx_raw_buf, uart_priv->rx_max_len,
                uart_priv->rx_decode_buf, data_len) != EOK)) {
                hcc_printf_err_log("hcc_rx memcpy err:%d\r\n", data_len);
                hcc_tty_rx_buf_set(uart_priv, 0);
                return HCC_UART_PKT_TYPE_MAX;
            }
        }
        hcc_tty_rx_buf_set(uart_priv, data_len);
        data_len = 0;
    } else {
        uart_priv->rx_decode_total++;
        uart_priv->rx_decode_buf++;
        data_len--;
    }
    *out_len = data_len;
    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_uart_recv_data(td_void *disc_data, const td_u8 *data, td_s32 len)
{
    td_s32 data_len;
    hcc_uart_priv_data *uart_priv = (hcc_uart_priv_data *)disc_data;
    if (g_uart_bus == TD_NULL || disc_data == TD_NULL || data == TD_NULL) {
        hcc_printf_err_log("rx data param fail\r\n");
        return EXT_ERR_FAILURE;
    }

    if (hcc_rx_buf_update(uart_priv, data, len) != TD_TRUE) {
        hcc_printf_err_log("hcc_rx_buf_update fail\r\n");
        return EXT_ERR_FAILURE;
    }

    data_len = (td_s32)uart_priv->rx_cur_len;
    hcc_dfx_uart_recv_total_len_stat(len);
    while (data_len > 0) {
        if (hcc_uart_recv_decode_data(uart_priv, &data_len) == HCC_UART_PKT_TYPE_MAX) {
            return EXT_ERR_FAILURE;
        }
    }

    /* have decode all public buf data, reset ptr and lenth */
    if (uart_priv->rx_decode_total == uart_priv->rx_cur_len) {
        hcc_tty_rx_buf_set(uart_priv, 0);
    }
    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_uart_packet_data(td_u8 *buf, td_u16 len, td_u8 type, td_u8 queue_id)
{
    uart_packet_head *uart_head = (uart_packet_head *)buf;
    if (len <= sizeof(uart_packet_head) + 1) {
        return EXT_ERR_FAILURE;
    }

    uart_head->packet_start = PACKET_START_SIGNAL;
    uart_head->len_h = (len >> BYTE_BIT_LEN) & 0xFF;
    uart_head->len_l = len & 0xFF;
    uart_head->packet_info.queue_id = queue_id;
    uart_head->packet_info.type = type;
    buf[len - 1] = PACKET_END_SIGNAL;
    return EXT_ERR_SUCCESS;
}

static td_u32 hcc_uart_send_data(hcc_handler *hcc, hcc_trans_queue *queue, td_u16 send_nums)
{
    td_u32 i;
    hcc_unc_struc *unc_buf;

    for (i = 0; i < send_nums; i++) {
        unc_buf = hcc_list_dequeue(&queue->queue_info);
        if (osal_atomic_read(&hcc->hcc_state) != HCC_ON) { // dfr场景,获取的unc可能为空
            hcc_adapt_mem_free(hcc, unc_buf);
            return i;
        }
        if (hcc_uart_packet_data(unc_buf->buf, unc_buf->length, HCC_UART_TYPE_DATA, queue->queue_id) !=
            EXT_ERR_SUCCESS) {
            hcc_adapt_mem_free(hcc, unc_buf);
            hcc_printf_err_log("hcc_uart_send_data len err[%u]\n", unc_buf->length);
            break;
        }
        if (ps_write_tty(unc_buf->buf, unc_buf->length, TD_TRUE) !=  EXT_ERR_SUCCESS) {
            hcc_printf_err_log("hcc_uart_send_data Q[%d] fail\r\n", queue->queue_id);
            hcc_list_add_head(&queue->queue_info, unc_buf);
            break;
        }
        hcc_adapt_mem_free(hcc, unc_buf);
    }
    return i;
}

static td_u32 hcc_uart_tx_proc(hcc_bus *bus, hcc_trans_queue *queue, td_u16 *remain_pkt_nums)
{
    td_u32 send_succ;

    hcc_handler *hcc = (hcc_handler *)(bus->hcc);
    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    if (osal_atomic_read(&hcc->hcc_state) != HCC_ON) {
        return EXT_ERR_FAILURE;
    }

    send_succ = hcc_uart_send_data(hcc, queue, *remain_pkt_nums);
    if (send_succ > 0) {
        *remain_pkt_nums -= send_succ;
        return EXT_ERR_SUCCESS;
    } else {
        return EXT_ERR_FAILURE;
    }
}

td_u8 g_uart_msg[UART_MSG_LEN] = {0};
static ext_errno hcc_uart_send_msg(hcc_bus *bus, td_u32 msg_id)
{
    g_uart_msg[sizeof(uart_packet_head)] = (td_u8)msg_id;
    hcc_uart_packet_data(g_uart_msg, UART_MSG_LEN, HCC_UART_TYPE_MSG, 0);
    return (ext_errno)ps_write_tty(g_uart_msg, UART_MSG_LEN, TD_FALSE);
}

static td_s32 hcc_uart_patch_read(hcc_bus *bus, td_u8 *buf, td_u32 len, td_u32 timeout)
{
    return ps_read_tty(buf, len);
}

static td_s32 hcc_uart_patch_write(hcc_bus *bus, td_u8 *buf, td_u32 len)
{
    return ps_write_tty(buf, len, TD_FALSE);
}

td_u32 g_uart_baud_rate = UART_BAUD_RATE_CONFIG;
td_u8 g_transfer_mode = 0;
void hcc_uart_set_transfer_mode(td_u8 mode)
{
    g_transfer_mode = mode;
    if (mode == 0) { // 0代表boot 1代表firmware
        g_uart_baud_rate = UART_BAUD_RATE_1M;
    } else {
        g_uart_baud_rate = HCC_UART_FW_BAUDRATE;
    }
}

static td_s32 hcc_uart_reinit(hcc_bus *bus)
{
    struct tty_struct *tty = g_tty_device;
    struct platform_device *dev = g_uart_device;
    hcc_uart_priv_data *uart_priv = TD_NULL;

    if (tty == TD_NULL || dev == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc_debug("hcc_uart_reinit\r\n");
    uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("uart_priv is null\r\n");
        return EXT_ERR_FAILURE;
    }
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
    hcc_tty_rx_buf_set(uart_priv, 0);

    osal_mdelay(2); // Wait 2 ms for QUIT process.

    tty_lock(tty);
    ps_ktty_set_termios(tty, g_uart_baud_rate, FLOW_CTRL_ENABLE); /* 对于老版本reinit时也在内核态切波特率 */
    tty_unlock(tty);
    if (g_transfer_mode == 0) {
        hcc_tty_recv_func = patch_tty_receive;
    } else {
        hcc_tty_recv_func = hcc_uart_recv_data;
    }
    return EXT_ERR_SUCCESS;
}

static bus_dev_ops g_uart_opt_ops = {
    .tx_proc = hcc_uart_tx_proc,
    .send_and_clear_msg = hcc_uart_send_msg,
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    .update_credit              = TD_NULL,
    .get_credit                 = TD_NULL,
#endif
#ifdef CONFIG_HCC_SUPPORT_REG_OPT
    .read_reg = TD_NULL,
    .write_reg = TD_NULL,
#endif
    .reinit = hcc_uart_reinit,
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
    .patch_read = hcc_uart_patch_read,
    .patch_write = hcc_uart_patch_write,
#endif
    .power_action = TD_NULL,
    .sleep_request = TD_NULL,
    .wakeup_request = TD_NULL,
    .pm_notify_register = TD_NULL,
};

static hcc_bus *hcc_uart_bus_init(td_void)
{
    hcc_bus *bus = osal_kmalloc(sizeof(hcc_bus), 0);
    if (bus != TD_NULL) {
        memset_s(bus, sizeof(hcc_bus), 0, sizeof(hcc_bus));
        bus->bus_type = HCC_BUS_UART;
        bus->max_trans_size = 0xFFF; // no limit
        bus->addr_align = HCC_UART_DMA_DATA_ALIGN;
        bus->len_align = HCC_UART_DMA_DATA_ALIGN;
        bus->max_assemble_cnt = 0;
        bus->descr_align_bit = HCC_ADAPT_BUS_DESCR_ALIGN_BIT_MIN;
        bus->rsv_buf = TD_NULL;

        bus->bus_ops = &g_uart_opt_ops;
        bus->tx_sched_count = 0;
        bus->state = 1;
        bus->hcc_bus_dfx = hcc_uart_dfx_recv_stat_print;
        g_uart_bus = bus;
        hcc_tty_recv_func = patch_tty_receive;
    }
    return bus;
}

static td_void hcc_uart_bus_deinit(td_void)
{
    if (g_uart_bus != TD_NULL) {
        osal_kfree(g_uart_bus);
        g_uart_bus = TD_NULL;
    }
}

/*
 * Prototype    : hcc_tty_open
 * Description  : called by tty uart itself when open tty uart from octty
 * input        : tty -> have opened tty
 */
static td_s32 hcc_tty_open(struct tty_struct *tty)
{
    hcc_uart_priv_data *uart_priv = TD_NULL;
    struct platform_device *dev = g_uart_device;

    hcc_debug("hcc_tty_open\r\n");
    uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("uart_priv is null\r\n");
        return -EFAULT;
    }
    tty->disc_data = uart_priv;

    /* don't do an wakeup for now */
    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
    /* set mem already allocated */
    tty->receive_room = PUBLIC_BUF_MAX; // 8192: buffer size
    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    /* 通知加载任务tty已经打开 */
    g_tty_device = tty;
    complete(&uart_priv->ldisc_installed);
#endif
    return EXT_ERR_SUCCESS;
}

/*
 * Prototype    : hcc_tty_close
 * Description  : called by tty uart when close tty uart from octty
 * input        : tty -> have opened tty
 */
static td_void hcc_tty_close(struct tty_struct *tty)
{
    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
    tty->disc_data = TD_NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    /* 通知加载任务tty已经打开 */
    g_tty_device = NULL;
#endif
    hcc_debug("hcc_tty_close: tty %s close done!\r\n", tty_name(tty));
}

/*
 * Prototype    : hcc_tty_receive
 * Description  : called by tty uart when recive data from tty uart
 * input        : tty   -> have opened tty
 *                data -> recive data ptr
 *                count-> recive data count
 */
static td_void hcc_tty_receive(struct tty_struct *tty, const td_u8 *data, char *tty_flags, td_s32 count)
{
    hcc_uart_priv_data *uart_priv = tty->disc_data;
    hcc_tty_recv_func(uart_priv, data, count);
}

/*
 * Prototype    : hcc_tty_wakeup
 * Description  : called by tty uart when wakeup from suspend
 * input        : tty   -> have opened tty
 */
static td_void hcc_tty_wakeup(struct tty_struct *tty)
{
    struct platform_device *dev = g_uart_device;
    hcc_uart_priv_data *uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv != NULL) {
        osal_wait_wakeup(&uart_priv->tx_wait);
    }
    return;
}

td_void hcc_uart_tx_thread_wakeup(td_void)
{
    hcc_tty_wakeup(NULL);
}
/*
 * Prototype    : hcc_tty_flush_buffer
 * Description  : called by tty uart when flush buffer
 * input        : tty   -> have opened tty
 */
static td_void hcc_tty_flush_buffer(struct tty_struct *tty)
{
    return;
}

static struct tty_ldisc_ops g_hcc_ldisc_ops = {
    .magic = TTY_LDISC_MAGIC,
    .name = "n_ps",
    .open = hcc_tty_open,
    .close = hcc_tty_close,
    .receive_buf = hcc_tty_receive,
    .write_wakeup = hcc_tty_wakeup,
    .flush_buffer = hcc_tty_flush_buffer,
    .owner = THIS_MODULE
};

static td_s32 hcc_uart_init(td_s32 ldisc_num)
{
#ifdef N_HW_BFG
    if (ldisc_num < 0 || ldisc_num > N_HW_BFG) {
        hcc_printf_err_log("ldisc num %d invalid\n", ldisc_num);
        return EXT_ERR_FAILURE;
    }

    return tty_register_ldisc(ldisc_num, &g_hcc_ldisc_ops);
#else
    return EXT_ERR_SUCCESS;
#endif
}

static td_void hcc_uart_exit(td_s32 ldisc_num)
{
#ifdef N_HW_BFG
    td_s32  ret;

    if (ldisc_num < 0 || ldisc_num > N_HW_BFG) {
        hcc_printf_err_log("ldisc num %d invalid\r\n", ldisc_num);
        return;
    }

    ret = tty_unregister_ldisc(ldisc_num);
    if (ret != 0) {
        hcc_printf_err_log("unregister ldisc ret %d\r\n", ret);
    }
#endif
}

static td_s32 hcc_uart_probe(struct platform_device *dev)
{
    hcc_uart_priv_data *uart_priv = TD_NULL;
    td_u8 *rx_buf = TD_NULL;

    uart_priv = (hcc_uart_priv_data *)osal_kmalloc(sizeof(hcc_uart_priv_data), OSAL_GFP_KERNEL);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("alloc uart_priv[%d] failed.\r\n", (td_s32)sizeof(hcc_uart_priv_data));
        return EXT_ERR_FAILURE;
    }
    memset_s((void *)uart_priv, sizeof(hcc_uart_priv_data), 0, sizeof(hcc_uart_priv_data));

    // set private data
    dev->dev.platform_data = uart_priv;
    dev_set_drvdata(&dev->dev, uart_priv);
    g_uart_device = dev;
    // RX init
    if (osal_spin_lock_init(&uart_priv->rx_lock) != EXT_ERR_SUCCESS) {
        goto SPINLOCK_FAIL;
    }
    if (osal_mutex_init(&uart_priv->tx_lock) != EXT_ERR_SUCCESS) {
        goto MUTEX_FAIL;
    }
    if (osal_wait_init(&uart_priv->rx_wait) != EXT_ERR_SUCCESS) {
        goto RX_WAIT_FAIL;
    }
    if (osal_wait_init(&uart_priv->tx_wait) != EXT_ERR_SUCCESS) {
        goto TX_WAIT_FAIL;
    }
    rx_buf = (td_u8 *)osal_kmalloc(PUBLIC_BUF_MAX, OSAL_GFP_KERNEL);
    if (rx_buf == TD_NULL) {
        hcc_printf_err_log("alloc rx_buf[%d] failed.\r\n", PUBLIC_BUF_MAX);
        goto RX_BUFFER_ALLOC_FAIL;
    }
    memset_s((void *)rx_buf, PUBLIC_BUF_MAX, 0, PUBLIC_BUF_MAX);

    uart_priv->rx_raw_buf = rx_buf;
    uart_priv->rx_decode_buf = rx_buf;
    uart_priv->rx_max_len = PUBLIC_BUF_MAX;
    uart_priv->rx_cur_len = 0;
    hcc_uart_init(N_HW_BFG);
    return EXT_ERR_SUCCESS;
RX_BUFFER_ALLOC_FAIL:
    osal_wait_destroy(&uart_priv->tx_wait);
TX_WAIT_FAIL:
    osal_wait_destroy(&uart_priv->rx_wait);
RX_WAIT_FAIL:
    osal_mutex_destroy(&uart_priv->tx_lock);
MUTEX_FAIL:
    osal_spin_lock_destroy(&uart_priv->rx_lock);
SPINLOCK_FAIL:
    osal_kfree(uart_priv);
    hcc_printf_err_log("hcc_uart_probe fail\n");
    return EXT_ERR_FAILURE;
}

static td_s32 hcc_uart_remove(struct platform_device *dev)
{
    hcc_uart_priv_data *uart_priv = dev_get_drvdata(&dev->dev);
    if (uart_priv == TD_NULL) {
        hcc_printf_err_log("uart_priv is null\r\n");
        return -EFAULT;
    }

    hcc_uart_exit(N_HW_BFG);
    osal_spin_lock_destroy(&uart_priv->rx_lock);
    osal_mutex_destroy(&uart_priv->tx_lock);
    osal_wait_destroy(&uart_priv->rx_wait);
    osal_wait_destroy(&uart_priv->tx_wait);
    osal_kfree(uart_priv->rx_raw_buf);
    osal_kfree(uart_priv);
    return 0;
}

static td_s32 hcc_uart_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static td_s32 hcc_uart_resume(struct platform_device *pdev)
{
    return 0;
}

static void hcc_uart_device_release(struct device *dev)
{
    return;
}

static struct platform_driver g_hcc_uart_driver = {
    .probe      = hcc_uart_probe,
    .remove     = hcc_uart_remove,
    .suspend    = hcc_uart_suspend,
    .resume     = hcc_uart_resume,
    .driver     = {
        .name   = "hcc_uart",
        .owner  = THIS_MODULE,
    },
};

static struct platform_device g_hcc_uart_device  = {
    .name       = "hcc_uart",
    .id         = 0,
    .dev = {
        .release = hcc_uart_device_release,
    },
};

hcc_bus *hcc_adapt_uart_load(td_void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    hcc_printf_err_log("the ldisc %u\n", N_HW_BFG);
    bsle_user_ctrl_init();
#endif
    if (platform_device_register(&g_hcc_uart_device)) {
        hcc_printf_err_log("Unable to register platform uart device.\r\n");
        return TD_NULL;
    }

    if (platform_driver_register(&g_hcc_uart_driver)) {
        hcc_printf_err_log("Unable to register platform uart driver.\r\n");
        return TD_NULL;
    }
    if (ps_open_tty_drv() != EXT_ERR_SUCCESS) {
        return TD_NULL;
    }
    return hcc_uart_bus_init();
}

td_void hcc_adapt_uart_unload(td_void)
{
    hcc_uart_bus_deinit();
    ps_close_tty_drv();
    platform_driver_unregister(&g_hcc_uart_driver);
    platform_device_unregister(&g_hcc_uart_device);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // CONFIG_HCC_SUPPORT_UART

