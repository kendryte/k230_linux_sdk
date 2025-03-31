/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/serial_core.h>
#include <linux/poll.h>
#include <linux/skbuff.h>
#include "bt_module.h"
#include "securec.h"
#include "oal_schedule.h"
#include "oal_types.h"
#include "ble_host.h"
#include "bt_dev.h"

#ifndef BT_DEV_LOG_LEVEL_ERROR
#define BT_DEV_LOG_LEVEL_ERROR 1
#endif

#ifndef BT_DEV_LOG_LEVEL_WARN
#define BT_DEV_LOG_LEVEL_WARN  2
#endif

#ifndef BT_DEV_LOG_LEVEL_INFO
#define BT_DEV_LOG_LEVEL_INFO 3
#endif
#ifndef BT_DEV_LOG_LEVEL_DEBUG
#define BT_DEV_LOG_LEVEL_DEBUG 4
#endif

#ifndef BT_DEV_LOG_LEVEL
#define BT_DEV_LOG_LEVEL BT_DEV_LOG_LEVEL_INFO
#endif

#if BT_DEV_LOG_LEVEL >= BT_DEV_LOG_LEVEL_DEBUG
#define bt_dev_err(fmt, arg...)          printk("[BT_DEV_ERR] "fmt, ##arg)
#define bt_dev_warn(fmt, arg...)         printk("[BT_DEV_WARN] "fmt, ##arg)
#define bt_dev_info(fmt, arg...)         printk("[BT_DEV_INFO] "fmt, ##arg)
#define bt_dev_debug(fmt, arg...)        printk("[BT_DEV_DBG] "fmt, ##arg)
#endif

#if BT_DEV_LOG_LEVEL == BT_DEV_LOG_LEVEL_INFO
#define bt_dev_err(fmt, arg...)          printk("[BT_DEV_ERR] "fmt, ##arg)
#define bt_dev_warn(fmt, arg...)         printk("[BT_DEV_WARN] "fmt, ##arg)
#define bt_dev_info(fmt, arg...)         printk("[BT_DEV_INFO] "fmt, ##arg)
#define bt_dev_debug(fmt, arg...)
#endif

#define BT_TYPE_UNKNOWN 0xFF

#define BLE_DEVICE_OPEN_FINISH_MAX_WAIT_TIME 2500
#define BLE_DEVICE_CLOSE_FINISH_MAX_WAIT_TIME 5000
#define BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME 10

#define BLE_OPEN_BTC_FINISH 1
#define BLE_CLOSE_BTC_FINISH 0

static void ble_open_close_btc_finish_handle(osal_u8 event);

struct bt_dev_stru {
    dev_tx tx;
    struct sk_buff_head rx_queue;
    wait_queue_head_t rx_wait;
    osal_spinlock rx_lock;
};

static struct bt_dev_stru g_bt_dev = {0};

static int rx_push(uint8_t *data, uint16_t len)
{
    struct sk_buff *skb = NULL;
    if (g_bt_dev.rx_queue.qlen >= RX_BT_QUE_MAX_NUM) {
        bt_dev_debug("BT rx queue too large! qlen=%d\n", g_bt_dev.rx_queue.qlen);
        wake_up_interruptible(&g_bt_dev.rx_wait);
        return -EINVAL;
    }

    // 业务侧处理完成释放
    skb = alloc_skb(len, GFP_ATOMIC);
    skb_put(skb, len);
    if (memcpy_s(skb->data, len, data, len) != EOK) {
        kfree_skb(skb);
        return -EINVAL;
    }
    osal_spin_lock(&g_bt_dev.rx_lock);
    skb_queue_tail(&g_bt_dev.rx_queue, skb);
    osal_spin_unlock(&g_bt_dev.rx_lock);
    wake_up_interruptible(&g_bt_dev.rx_wait);
    return EOK;
}

static struct sk_buff *rx_pop(void)
{
    struct sk_buff *curr_skb = NULL;
    osal_spin_lock(&g_bt_dev.rx_lock);
    curr_skb = skb_dequeue(&g_bt_dev.rx_queue);
    osal_spin_unlock(&g_bt_dev.rx_lock);
    return curr_skb;
}

static int rx_restore(struct sk_buff *skb)
{
    osal_spin_lock(&g_bt_dev.rx_lock);
    skb_queue_head(&g_bt_dev.rx_queue, skb);
    osal_spin_unlock(&g_bt_dev.rx_lock);
    return EOK;
}

int bt_dev_rx(uint8_t *data, uint16_t len)
{
    bt_dev_debug("%s:%d len:%d\n", __FUNCTION__, __LINE__, len);
    return rx_push(data, len);
}

void bt_dev_tx_register(dev_tx tx)
{
    g_bt_dev.tx = tx;
}

static int bt_misc_dev_open(struct inode *inode, struct file *filp)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    int ret;

    bt_dev_info("%s:%d\n", __FUNCTION__, __LINE__);
    if (get_ble_state() == BLE_ON) {
        bt_dev_info("ble already open \r\n");
        return EOK;
    }
    pm_ble_enable_reply_cb_host_register(ble_open_close_btc_finish_handle);
    ret = pm_ble_enable();
    if (ret != OAL_SUCC) {
        bt_dev_info("ble send open btc msg error ret %d\r\n", ret);
        return -EINVAL;
    }
    while (get_ble_state() == BLE_OFF) {
        osal_msleep(BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + BLE_DEVICE_OPEN_FINISH_MAX_WAIT_TIME) {
            bt_dev_info("ble open device finish fail ,time out \n");
            return -EINVAL;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    bt_dev_info(" ble open time:%lld \n", end - start);
    return EOK;
}

static osal_u8 type = BT_TYPE_UNKNOWN;

static ssize_t bt_misc_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    if (get_ble_state() == BLE_OFF) {
        bt_dev_debug("bt misc dev write ble closed \n");
        return -EINVAL;
    }
    if (buf == NULL) {
        bt_dev_debug("buf is NULL\n");
        return -EINVAL;
    }
    if (count > BT_TX_MAX_FRAME) {
        bt_dev_debug("buf len is too large!\n");
        return -EINVAL;
    }
    if (count == 1) { // android 9版本会传输type字段
        osal_u8 __user *puser = (osal_u8 __user *)buf;
        get_user(type, puser);
        bt_dev_debug("type:%d\r\n", type);
        return count;
    }
    if (g_bt_dev.tx) {
        td_u8 *tx_buf;
        if (type != BT_TYPE_UNKNOWN) {
            tx_buf = (td_u8 *)osal_kmalloc(count + 1, OSAL_GFP_KERNEL);
            if (tx_buf == NULL) {
                return -ENOMEM;
            }
            tx_buf[0] = type;
            if (copy_from_user(tx_buf + 1, buf, count)) {
                bt_dev_info("copy_from_user from bt is err\n");
                osal_kfree(tx_buf);
                return -EFAULT;
            }
            g_bt_dev.tx(tx_buf, count + 1);
        } else {
            tx_buf = (td_u8 *)osal_kmalloc(count, OSAL_GFP_KERNEL);
            if (tx_buf == NULL) {
                return -ENOMEM;
            }
            if (copy_from_user(tx_buf, buf, count)) {
                bt_dev_info("copy_from_user from bt is err\n");
                osal_kfree(tx_buf);
                return -EFAULT;
            }
            g_bt_dev.tx(tx_buf, count);
        }
        osal_kfree(tx_buf);
    }
    return count;
}

static ssize_t bt_misc_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    size_t len;
    struct sk_buff *skb = NULL;
    bt_dev_debug("%s:%d count:%d\n", __FUNCTION__, __LINE__, count);
    if (get_ble_state() == BLE_OFF) {
        bt_dev_debug("bt misc dev read ble closed \n");
        return -EINVAL;
    }
    skb = rx_pop();
    if (skb == NULL) {
        bt_dev_debug("rx_queue is null\r\n");
        return EOK;
    }
    len = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, len)) {
        bt_dev_debug("copy_to_user err\r\n");
    }
    skb_pull(skb, len);
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        rx_restore(skb);
    }
    bt_dev_debug("%s:%d len:%d\n", __FUNCTION__, __LINE__, len);
    return len;
}

static unsigned int bt_misc_dev_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    bt_dev_debug("%s:%d\n", __FUNCTION__, __LINE__);
    if (get_ble_state() == BLE_OFF) {
        bt_dev_debug("bt misc dev poll ble closed \n");
        return -EINVAL;
    }
    poll_wait(filp, &g_bt_dev.rx_wait, wait);
    if (g_bt_dev.rx_queue.qlen != 0) {
        mask |= POLLIN | POLLRDNORM;
    }
    bt_dev_debug("%s:%d ret:%d\n", __FUNCTION__, __LINE__, mask);
    return mask;
}

static long bt_misc_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    bt_dev_debug("%s:%d\n", __FUNCTION__, __LINE__);
    if (get_ble_state() == BLE_OFF) {
        bt_dev_debug("bt misc dev ioctl ble closed \n");
        return -EINVAL;
    }
    return EOK;
}

static int bt_misc_dev_release(struct inode *inode, struct file *filp)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    int ret;
    bt_dev_info("%s:%d\n", __FUNCTION__, __LINE__);
    if (get_ble_state() == BLE_OFF) {
        bt_dev_info("bt misc dev release ble already closed \r\n");
        return -EINVAL;
    }

    ret = pm_ble_disable();
    if (ret != OAL_SUCC) {
        bt_dev_info("ble send close btc msg error ret %d\r\n", ret);
        return -EINVAL;
    }

    while (get_ble_state() == BLE_ON) {
        osal_msleep(BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + BLE_DEVICE_CLOSE_FINISH_MAX_WAIT_TIME) {
            bt_dev_info("ble close device finish fail ,time out \n");
            pm_ble_enable_reply_cb_host_register(NULL);
            set_ble_state(BLE_OFF);
            return EOK;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    bt_dev_info(" ble close ,time:%lld \n", end - start);
    pm_ble_enable_reply_cb_host_register(NULL);
    return EOK;
}

static void ble_open_close_btc_finish_handle(osal_u8 event)
{
    switch (event) {
        case BLE_OPEN_BTC_FINISH: {
            bt_dev_info(" ble btc open finish \n");
            set_ble_state(BLE_ON);
            break;
        }
        case BLE_CLOSE_BTC_FINISH: {
            bt_dev_info(" ble btc close finish \n");
            set_ble_state(BLE_OFF);
            break;
        }
        default:
            break;
    }
    return;
}

static const struct file_operations bt_fops = {
    .owner = THIS_MODULE,
    .open = bt_misc_dev_open,
    .write = bt_misc_dev_write,
    .read = bt_misc_dev_read,
    .poll = bt_misc_dev_poll,
    .unlocked_ioctl = bt_misc_dev_ioctl,
    .release = bt_misc_dev_release,
};

static struct miscdevice bt_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbt",
    .fops = &bt_fops,
    .mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
};

static void bt_release(struct device *dev)
{
}

static struct platform_device bt_platform_device = {
    .name = "bt_soc",
    .dev = {
        .release = bt_release,
    },
};

static int bt_misc_device_register(void)
{
    int ret;
    ret = misc_register(&bt_device);
    return ret;
}

static int bt_probe(struct platform_device *platform_dev)
{
    int ret;
    ret = bt_misc_device_register();
    osal_spin_lock_init(&g_bt_dev.rx_lock);
    skb_queue_head_init(&g_bt_dev.rx_queue);
    init_waitqueue_head(&g_bt_dev.rx_wait);

    bt_dev_info("%s:%d ret:%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

static void bt_misc_device_deregister(void)
{
    misc_deregister(&bt_device);
    bt_dev_info("misc bt device have removed\n");
}

static int bt_remove(struct platform_device *dev)
{
    bt_misc_device_deregister();
    osal_spin_lock_destroy(&g_bt_dev.rx_lock);
    wake_up_interruptible(&g_bt_dev.rx_wait);
    return EOK;
}

static struct platform_driver bt_platform_driver = {
    .probe      = bt_probe,
    .remove     = bt_remove,
    .driver     = {
        .name   = "bt_soc",
        .owner  = THIS_MODULE,
    },
};

int bt_dev_init(void)
{
    int ret;
    ret = platform_device_register(&bt_platform_device);
    bt_dev_info("register platform bt device ret:%d\n", ret);
    ret = platform_driver_register(&bt_platform_driver);
    bt_dev_info("register platform bt driver ret:%d\n", ret);
    return ret;
}

void bt_dev_exit(void)
{
    platform_device_unregister(&bt_platform_device);
    platform_driver_unregister(&bt_platform_driver);
}