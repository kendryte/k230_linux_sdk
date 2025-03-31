/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/serial_core.h>
#include <linux/poll.h>
#include <linux/skbuff.h>
#include "sle_module.h"
#include "securec.h"
#include "oal_schedule.h"
#include "oal_types.h"
#include "sle_host_register.h"
#include "sle_dev.h"

#define SLE_TYPE_UNKNOWN 0xFF

#define SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME 2000
#define SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME 10

#define SLE_OPEN_BTC_FINISH 1
#define SLE_CLOSE_BTC_FINISH 0

static osal_u8 type = SLE_TYPE_UNKNOWN;

static void sle_open_close_btc_finish_handle(osal_u8 event);

struct sle_dev_stru {
    sle_dev_tx tx;
    struct sk_buff_head rx_queue;
    wait_queue_head_t rx_wait;
    osal_spinlock rx_lock;
};

static struct sle_dev_stru g_sle_dev = {0};

static int rx_push(uint8_t *data, uint16_t len)
{
    struct sk_buff *skb = NULL;
    if (g_sle_dev.rx_queue.qlen >= RX_BT_QUE_MAX_NUM) {
        printk(KERN_ERR"rx_push sle rx queue too large! qlen=%u\n", g_sle_dev.rx_queue.qlen);
        wake_up_interruptible(&g_sle_dev.rx_wait);
        return -EINVAL;
    }

    // 业务侧处理完成释放
    skb = alloc_skb(len, GFP_ATOMIC);
    skb_put(skb, len);
    if (memcpy_s(skb->data, len, data, len) != EOK) {
        printk(KERN_ERR"rx_push memcpy fail! data len=%u\n", len);
        kfree_skb(skb);
        return -EINVAL;
    }
    osal_spin_lock(&g_sle_dev.rx_lock);
    skb_queue_tail(&g_sle_dev.rx_queue, skb);
    osal_spin_unlock(&g_sle_dev.rx_lock);
    wake_up_interruptible(&g_sle_dev.rx_wait);
    return EOK;
}

static struct sk_buff *rx_pop(void)
{
    struct sk_buff *curr_skb = NULL;
    osal_spin_lock(&g_sle_dev.rx_lock);
    curr_skb = skb_dequeue(&g_sle_dev.rx_queue);
    osal_spin_unlock(&g_sle_dev.rx_lock);
    return curr_skb;
}

static int rx_restore(struct sk_buff *skb)
{
    osal_spin_lock(&g_sle_dev.rx_lock);
    skb_queue_head(&g_sle_dev.rx_queue, skb);
    osal_spin_unlock(&g_sle_dev.rx_lock);
    return EOK;
}

int sle_dev_rx(uint8_t *data, uint16_t len)
{
    return rx_push(data, len);
}

void sle_dev_tx_register(sle_dev_tx tx)
{
    g_sle_dev.tx = tx;
}

static int sle_misc_dev_open(struct inode *inode, struct file *filp)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    int ret;

    printk(KERN_INFO"%s:%d\n", __FUNCTION__, __LINE__);
    if (get_sle_state() == SLE_ON) {
        printk(KERN_ERR"sle device already open \n");
        return EOK;
    }
    pm_sle_enable_reply_cb_host_register(sle_open_close_btc_finish_handle);
    ret = pm_sle_enable();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle send open btc msg fail ret 0x%x\n", ret);
        return -EINVAL;
    }

    while (get_sle_state() == SLE_OFF) {
        osal_msleep(SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME) {
            printk(KERN_ERR"sle open device finish fail ,time out \n");
            return -EINVAL;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    printk(KERN_INFO" sle open time:%lld \n", end - start);
    return EOK;
}

static ssize_t sle_misc_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    if (buf == NULL) {
        printk(KERN_ERR"sle_misc_dev_write buf is NULL\n");
        return -EINVAL;
    }
    if (count > SLE_TX_MAX_FRAME) {
        printk(KERN_ERR"sle_misc_dev_write buf len is too large!\n");
        return -EINVAL;
    }

    if (g_sle_dev.tx) {
        td_u8 *tx_buf;
        if (type != SLE_TYPE_UNKNOWN) {
            tx_buf = (td_u8 *)osal_kmalloc(count + 1, OSAL_GFP_KERNEL);
            if (tx_buf == NULL) {
                printk(KERN_ERR"sle_misc_dev_write kmalloc fail, data type:0x%x\n", type);
                return -ENOMEM;
            }
            tx_buf[0] = type;
            if (copy_from_user(tx_buf + 1, buf, count)) {
                printk(KERN_ERR"sle_misc_dev_write copy_from_user from sle is err, data type:0x%x\n", type);
                osal_kfree(tx_buf);
                return -EFAULT;
            }
            g_sle_dev.tx(tx_buf, count + 1);
        } else {
            tx_buf = (td_u8 *)osal_kmalloc(count, OSAL_GFP_KERNEL);
            if (tx_buf == NULL) {
                printk(KERN_ERR"sle_misc_dev_write kmalloc fail, data type:0x%x\n", type);
                return -ENOMEM;
            }
            if (copy_from_user(tx_buf, buf, count)) {
                printk(KERN_ERR"sle_misc_dev_write copy_from_user from bt is err, data type:0x%x\n", type);
                osal_kfree(tx_buf);
                return -EFAULT;
            }
            g_sle_dev.tx(tx_buf, count);
        }
        osal_kfree(tx_buf);
    }
    return count;
}

static ssize_t sle_misc_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    size_t len;
    struct sk_buff *skb = NULL;
    skb = rx_pop();
    if (skb == NULL) {
        printk(KERN_ERR"sle_misc_dev_read rx_queue is null\r\n");
        return EOK;
    }
    len = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, len)) {
        printk(KERN_ERR"sle_misc_dev_read copy_to_user err\r\n");
    }
    skb_pull(skb, len);
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        rx_restore(skb);
    }
    return len;
}

static unsigned int sle_misc_dev_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = POLLOUT | POLLWRNORM;
    poll_wait(filp, &g_sle_dev.rx_wait, wait);
    if (g_sle_dev.rx_queue.qlen != 0) {
        mask |= POLLIN | POLLRDNORM;
    }
    return mask;
}

static long sle_misc_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return EOK;
}

static int sle_misc_dev_release(struct inode *inode, struct file *filp)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    int ret;

    printk(KERN_INFO"%s:%d\n", __FUNCTION__, __LINE__);
    if (get_sle_state() == SLE_OFF) {
        printk(KERN_ERR"sle device already closed \n");
        return -EINVAL;
    }

    ret = pm_sle_disable();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle send close btc msg fail ret 0x%x\n", ret);
        return -EINVAL;
    }

    while (get_sle_state() == SLE_ON) {
        osal_msleep(SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + SLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME) {
            printk(KERN_ERR"sle_close device finish fail ,time out \n");
            pm_sle_enable_reply_cb_host_register(NULL);
            set_sle_state(SLE_OFF);
            return -EINVAL;
        }
    }

    end = oal_get_time_stamp_from_timeval();
    pm_sle_enable_reply_cb_host_register(NULL);
    printk(KERN_INFO" sle_close time:%lld \n", end - start);
    return EOK;
}

static void sle_open_close_btc_finish_handle(osal_u8 event)
{
    switch (event) {
        case SLE_OPEN_BTC_FINISH: {
            printk(KERN_INFO" sle btc open finish \n");
            set_sle_state(SLE_ON);
            break;
        }
        case SLE_CLOSE_BTC_FINISH: {
            printk(KERN_INFO" sle btc close finish \n");
            set_sle_state(SLE_OFF);
            break;
        }
        default:
            break;
    }
    return;
}

static const struct file_operations sle_fops = {
    .owner = THIS_MODULE,
    .open = sle_misc_dev_open,
    .write = sle_misc_dev_write,
    .read = sle_misc_dev_read,
    .poll = sle_misc_dev_poll,
    .unlocked_ioctl = sle_misc_dev_ioctl,
    .release = sle_misc_dev_release,
};

static struct miscdevice sle_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwsle",
    .fops = &sle_fops,
    .mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
};

static void sle_release(struct device *dev)
{
    printk(KERN_INFO"%s:%d in\n", __FUNCTION__, __LINE__);
}

static struct platform_device sle_platform_device = {
    .name = "sle_soc",
    .dev = {
        .release = sle_release,
    },
};

static int sle_misc_device_register(void)
{
    int ret;
    ret = misc_register(&sle_device);
    return ret;
}

static int sle_probe(struct platform_device *platform_dev)
{
    int ret;
    printk(KERN_INFO"%s:%d in\n", __FUNCTION__, __LINE__);
    ret = sle_misc_device_register();
    osal_spin_lock_init(&g_sle_dev.rx_lock);
    skb_queue_head_init(&g_sle_dev.rx_queue);
    init_waitqueue_head(&g_sle_dev.rx_wait);
    printk(KERN_INFO"sle_probe sle_misc_device_register ret:%d\n", ret);
    return ret;
}

static void sle_misc_device_deregister(void)
{
    misc_deregister(&sle_device);
    printk(KERN_INFO"sle_misc_device_deregister sle device have removed\n");
}

static int sle_remove(struct platform_device *dev)
{
    printk(KERN_INFO"%s:%d in\n", __FUNCTION__, __LINE__);
    sle_misc_device_deregister();
    osal_spin_lock_destroy(&g_sle_dev.rx_lock);
    wake_up_interruptible(&g_sle_dev.rx_wait);
    return EOK;
}

static struct platform_driver sle_platform_driver = {
    .probe      = sle_probe,
    .remove     = sle_remove,
    .driver     = {
        .name   = "sle_soc",
        .owner  = THIS_MODULE,
    },
};

int sle_dev_init(void)
{
    int ret;
    printk(KERN_INFO"sle_dev_init enter\n");
    ret = platform_device_register(&sle_platform_device);
    printk(KERN_INFO"sle_dev_init register sle soc device, ret:%d\n", ret);
    ret = platform_driver_register(&sle_platform_driver);
    printk(KERN_INFO"sle_dev_init register platform sle driver, ret:%d\n", ret);
    return ret;
}

void sle_dev_exit(void)
{
    printk(KERN_INFO"sle_dev_exit enter\n");
    platform_device_unregister(&sle_platform_device);
    printk(KERN_INFO"sle_dev_exit unregister sle soc device\n");
    platform_driver_unregister(&sle_platform_driver);
    printk(KERN_INFO"sle_dev_exit unregister platform sle driverce\n");
}