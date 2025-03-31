/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: oal usb host adapt source file.
 * Author: Huanghe
 * Create: 2021-07-28
 */

#ifdef CONFIG_HCC_SUPPORT_USB
#include "hcc.h"
#include "hcc_service.h"
#include "hcc_usb_host.h"
#include "plat_pm_wlan.h"

typedef td_s32 (*usb_thread_callback_fuc)(td_void *data);

static struct hcc_usb *g_usb = TD_NULL;
struct hcc_usb *usb_get_handle(td_void)
{
    return g_usb;
}

static struct completion g_usb_disconnect;
static struct completion g_usb_boot_probe;
static struct completion g_usb_wifi_probe;
struct completion g_usb_resume_wlan_complete;
struct completion g_usb_resume_msg_complete;

static const struct usb_device_id g_usb_table[] = {
    { USB_DEVICE(0x12D1, 0x897d) },
    { USB_DEVICE(0xFFFF, 0x3733) },
    { }
};

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
MODULE_DEVICE_TABLE(usb, g_usb_table);
#endif

#define USB_PKT_ALLOC_MAX 4096
#define USB_RX_PKT_MAX 20
static td_u32 g_usb_rx_error_pkts = 0;
/* RX work queue for urb submitting, free queue for urb filling */
static td_bool g_urb_rx_queue_init = TD_FALSE;
static td_bool g_rx_res_should_release = TD_FALSE;
__attribute((aligned (4))) urb_queue g_urb_rx_work_queue;
__attribute((aligned (4))) urb_queue g_urb_rx_free_queue;
__attribute((aligned (4))) urb_buf_stru *g_urb_rx_queue_node[URB_RX_MAX_NUM] = {TD_NULL};

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
td_u8 *g_usb_bulk_out_buffer = TD_NULL;
#endif
#endif

struct usb_urb_context g_wlan_ctx;
struct usb_urb_context g_bt_ctx;
struct usb_urb_context g_interrupt_ctx;

static td_void oal_usb_disconnect(struct usb_interface *interface);
static td_s32 usb_hcc_task_init_func(td_void);

static td_bool g_is_usb_log_enable = TD_TRUE;

td_bool usb_log_enable_get(td_void)
{
    return g_is_usb_log_enable;
}

td_void usb_log_enable_set(td_bool enable)
{
    g_is_usb_log_enable = enable;
}

static td_void usb_set_bus_state(td_u32 state)
{
    td_u32 old_state;
    static const osal_char *state_name[] = {
        "OFF", "INIT", "DISCONNECT", "SUSPEND_DISCONNECT", "BOOT_PROBE", "BOOT",
        "PROBE", "SUSPEND", "RESUME", "WORK"
    };
    if (state > BUS_USB_WORK) {
        oal_usb_log(BUS_LOG_ERR, "unknow usb state[%u]\n", state);
        return;
    }
    old_state = (td_u32)osal_atomic_read(&g_usb->usb_driver_status);
    osal_atomic_set(&g_usb->usb_driver_status, state);
    if (old_state <= BUS_USB_WORK) {
        oal_usb_log(BUS_LOG_INFO, "%s --> %s\n", state_name[old_state], state_name[state]);
    } else {
        oal_usb_log(BUS_LOG_INFO, "unknow state[%u] --> %s\n", old_state, state_name[state]);
    }
}

td_u32 usb_get_bus_state(td_void)
{
    return (td_u32)osal_atomic_read(&g_usb->usb_driver_status);
}

/* 用于关闭USB的发送和接收功能 */
td_void usb_disable_state(hcc_bus *pst_bus, td_u32 mask)
{
    struct hcc_usb *usb;

    if (OAL_WARN_ON(!pst_bus)) {
        oal_usb_log(BUS_LOG_ERR, "pst_bus is null");
        return;
    }

    usb = (struct hcc_usb *)pst_bus->data;
    if (usb->connect != TD_TRUE) {
        return;
    }

    osal_mutex_lock(&usb->state_lock);
    usb->state &= ~mask;
    osal_mutex_unlock(&usb->state_lock);
}

/* 使能USB的发送和接收功能 */
static td_void usb_enable_state(hcc_bus *pst_bus, td_u32 mask)
{
    struct hcc_usb *usb;
    if (OAL_WARN_ON(!pst_bus)) {
        oal_usb_log(BUS_LOG_ERR, "pst_bus is null");
        return;
    }

    usb = (struct hcc_usb *)pst_bus->data;
    if (usb->connect != TD_TRUE) {
        oal_usb_log(BUS_LOG_ERR, "usb is not connect");
        return;
    }

    osal_mutex_lock(&usb->state_lock);
    usb->state |= mask;
    osal_mutex_unlock(&usb->state_lock);
}

static td_void usb_dev_delete(struct kref *kref)
{
    struct usb_dev_info *dev = container_of(kref, struct usb_dev_info, kref);

    if (dev->bulk_in_buffer) {
        osal_kfree(dev->bulk_in_buffer);
    }

    if (dev->bulk_out_buffer) {
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
        dev->bulk_out_buffer = TD_NULL;
#else
        osal_kfree(dev->bulk_out_buffer);
#endif
    }

    if (dev->int_in_buffer) {
        osal_kfree(dev->int_in_buffer);
    }

    osal_mutex_destroy(&dev->io_mutex);
    osal_kfree(dev);
}

static td_s32 usb_reg_lock_init(usb_host_reg_lock *reg_rw_lock)
{
    td_s32 ret;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    memset_s(reg_rw_lock, sizeof(usb_host_reg_lock), 0, sizeof(usb_host_reg_lock));
    ret = osal_spin_lock_init(&reg_rw_lock->lock);
    init_waitqueue_head(&reg_rw_lock->wq);
#else
    ret = osal_mutex_init(&reg_rw_lock->lock);
#endif
    return ret;
}

static td_void usb_reg_lock_deinit(usb_host_reg_lock *reg_rw_lock)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_spin_lock_destroy(&reg_rw_lock->lock);
#else
    osal_mutex_destroy(&reg_rw_lock->lock);
#endif
}

/* ****************************************************************************
功能描述  : 释放hcc_usb申请的内存
**************************************************************************** */
static td_void usb_hcc_res_lock_deinit(struct hcc_usb *usb)
{
    osal_wait_destroy(&usb->usb_tx_wq);
    osal_wait_destroy(&usb->usb_rx_wq);
    osal_mutex_destroy(&usb->ctrl_ep_lock);
    osal_mutex_destroy(&usb->state_lock);
    usb_reg_lock_deinit(&usb->reg_rw_lock);
}

static td_void usb_hcc_res_deinit(td_void)
{
    struct usb_dev_info *dev = TD_NULL;

    if (g_usb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "g_usb is null");
        return;
    }
    usb_hcc_res_lock_deinit(g_usb);
    usb_rx_urb_list_deinit();
    usb_tx_urb_list_deinit();

    if (g_usb->interface_wlan) {
        dev = usb_get_intfdata(g_usb->interface_wlan);
        if (dev) {
            kref_put(&dev->kref, usb_dev_delete);
        }
        g_usb->interface_wlan = TD_NULL;
    }

    if (g_usb->descript_unc.buf) {
        osal_kfree(g_usb->descript_unc.buf);
        g_usb->descript_unc.buf = TD_NULL;
        g_usb->descript_unc.length = 0;
    }
    hcc_list_head_deinit(&g_usb->tx_assemble_info.assembled_head);

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    osal_kfree(g_usb_bulk_out_buffer);
    g_usb_bulk_out_buffer = TD_NULL;
#endif
#endif

    osal_kfree(g_usb);
    g_usb = TD_NULL;
}

static td_s32 usb_wlan_bulk_in(struct hcc_usb *pst_usb, td_u8 *buf, td_u32 len, td_u32 timeout)
{
    td_s32 ret = 0;
    td_s32 returnlen;
    struct usb_dev_info *dev = TD_NULL;

    if (pst_usb == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    dev = usb_get_intfdata(pst_usb->interface_wlan);
    if (dev == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    ret = oal_usb_bulk_in_msg(dev->udev, dev->bulk_in_hep, buf, len, &returnlen, timeout, &g_wlan_ctx);
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "usb_bulk_msg ret=%d", ret);
        return EXT_ERR_FAILURE;
    }
    return returnlen;
}

td_s32 usb_transfer_rx_reserved_buff(struct hcc_usb *usb)
{
    td_s32 ret = EXT_ERR_SUCCESS;
    td_u32 rx_nums;
    td_u8 *buf;
    struct usb_dev_info *dev;
    td_u32 extend_len;

    extend_len = usb->aggr_info[WLAN_TYPE].xfer_count;
    dev = usb_get_intfdata(usb->interface_wlan);
    if (0 == extend_len) {
        return EXT_ERR_FAILURE;
    }

    buf = osal_kmalloc(dev->bulk_in_max_pkt_size, OSAL_GFP_KERNEL);
    if (buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    rx_nums = extend_len / dev->bulk_in_max_pkt_size;
    if (extend_len % dev->bulk_in_max_pkt_size) {
        rx_nums += 1;
    }

    while (rx_nums) {
        ret = usb_wlan_bulk_in(usb, buf, dev->bulk_in_max_pkt_size, 0);
        if (ret < 0) {
            break;
        }
        rx_nums--;
    }
    osal_kfree(buf);

    return ret;
}

/* ****************************************************************************
功能描述  : 分配hcc链表,存放收到的device数据
**************************************************************************** */
static td_s32 usb_build_rx_netbuf_list(struct hcc_usb *usb, hcc_data_queue *head)
{
    td_u32 sum_len = 0;
    hcc_unc_struc *netbuf = TD_NULL;
    td_s32 i;
    td_u16 buff_len;

    if (OAL_UNLIKELY(!hcc_is_list_empty(head))) {
        oal_usb_log(BUS_LOG_ERR, "list not empty");
        return EXT_ERR_FAILURE;
    }

    if (usb->pst_bus == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "usb bus null");
        return EXT_ERR_FAILURE;
    }

    for (i = 0; i < HIUSB_DEV2HOST_SCATT_MAX; i++) {
        buff_len = usb->aggr_info[WLAN_TYPE].aggr_len[i];
        if ((buff_len == 0) || (buff_len > USB_PKT_ALLOC_MAX)) {
            break;
        }

        netbuf = hcc_adapt_alloc_unc_buf((hcc_handler *)usb->pst_bus->hcc, buff_len, usb->hcc_queue_id);
        if (netbuf == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "usb alloc unc fail to drop");
            goto failed_netbuf_alloc;
        }

        sum_len += buff_len;
        hcc_list_add_tail(head, netbuf);
    }

    sum_len = sum_len + HIUSB_PACKAGE_HEARDER_SIZE;
    if (OAL_WARN_ON(usb->aggr_info[WLAN_TYPE].xfer_count != sum_len)) {
        oal_usb_log(BUS_LOG_ERR, "received len err:[%u]-[%u]", usb->aggr_info[WLAN_TYPE].xfer_count, sum_len);
        return EXT_ERR_FAILURE;
    }

    if (OAL_UNLIKELY(hcc_is_list_empty(head))) {
        oal_usb_log(BUS_LOG_ERR, "list empty:%d", i);
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
failed_netbuf_alloc:
    hcc_list_free((hcc_handler *)usb->pst_bus->hcc, head);
    usb_transfer_rx_reserved_buff(usb);
    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 解析收到的device数据的usb package结构
**************************************************************************** */
td_s32 usb_read_data_header(struct hcc_usb *usb, td_u8 *buf)
{
    errno_t err_num;
    usb_package *package = (usb_package *)buf;

    if (package->msg_type != USB_ARRG_DEV_HOST_DATA) {
        if (package->msg_type == 0 && package->len == 0) {
            // 空报文直接丢弃, 不累积错误值
            return EXT_ERR_USB_INVALID_DATA_LEN_CODE;
        }
        oal_usb_log(BUS_LOG_INFO, "msg_type[0x%x],len[0x%x],rsv[0x%x]",
            package->msg_type, package->len, package->reserve);
        g_usb_rx_error_pkts++;
        if (g_usb_rx_error_pkts <= USB_RX_PKT_MAX) {
            return EXT_SEC_SUCCESS;
        }
        return EXT_ERR_FAILURE;
    }
    g_usb_rx_error_pkts = 0;

    usb->aggr_info[WLAN_TYPE].xfer_count = package->len;
    usb->hcc_queue_id = package->reserve;
    err_num = memcpy_s(usb->aggr_info[WLAN_TYPE].aggr_len, sizeof(usb->aggr_info[WLAN_TYPE].aggr_len),
        buf + sizeof(usb_package), sizeof(usb->aggr_info[WLAN_TYPE].aggr_len));
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "read aggr info[%d]", err_num);
    }

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 从urb中读取数据到hcc链表
**************************************************************************** */
static td_s32 usb_wlan_xfer_read_func(hcc_data_queue *head, td_u8 *rx_buf)
{
    td_s32 err_num;
    td_s32 offset = 0;
    hcc_unc_struc *uncbuf = TD_NULL;
    hcc_unc_struc *tmp = TD_NULL;

    if (rx_buf == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "param invalid");
        return EXT_ERR_FAILURE;
    }

    rx_buf += HIUSB_PACKAGE_HEARDER_SIZE;
    osal_list_for_each_entry_safe(uncbuf, tmp, (&head->data_queue), list) {
        err_num = memcpy_s(uncbuf->buf, uncbuf->length, rx_buf + offset, uncbuf->length);
        if (err_num != EOK) {
            oal_usb_log(BUS_LOG_ERR, "memcpy err:[%d]", err_num);
            return EXT_ERR_FAILURE;
        }

        offset += uncbuf->length;
    }

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 解析device递交的urb数据
**************************************************************************** */
static td_s32 usb_rx_netbuf(hcc_bus *pst_bus, hcc_data_queue *data_head)
{
    td_s32 ret = EXT_ERR_SUCCESS;
    urb_buf_stru *urb_buf = TD_NULL;
    struct hcc_usb *usb = (struct hcc_usb *)pst_bus->data;

    if (OAL_WARN_ON(!usb)) {
        oal_usb_log(BUS_LOG_ERR, "usb is null");
        return EXT_ERR_FAILURE;
    }
    if (usb_get_bus_state() <= BUS_USB_SUSPEND_DISCONNECT) {
        oal_usb_log(BUS_LOG_ERR, "usb device state[%u]!", usb_get_bus_state());
        return EXT_ERR_FAILURE;
    }

    if (usb->connect != TD_TRUE) {
        oal_usb_log(BUS_LOG_ERR, "usb disconnect!");
        return EXT_ERR_FAILURE;
    }

    if (urb_list_is_empty(&g_urb_rx_work_queue) == TD_TRUE) {
        /* work queue is enexcepted */
        oal_usb_log(BUS_LOG_ERR, "urb work queue len:[%d]", g_urb_rx_work_queue.qlen);
        return EXT_ERR_FAILURE;
    }

    urb_buf = urb_list_dequeue(&g_urb_rx_work_queue);
    ret = usb_read_data_header(usb, urb_buf->buf);
    if (ret != EXT_ERR_SUCCESS) {
        if (ret != EXT_ERR_USB_INVALID_DATA_LEN_CODE) {
            oal_usb_log(BUS_LOG_ERR, "read usb pkt hdr err:[%d]", ret);
        }
        urb_list_add_tail(&g_urb_rx_free_queue, urb_buf);
        return ret;
    }

    ret = usb_build_rx_netbuf_list(usb, data_head);
    if (ret != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "rx netbuf list err:[%d]", ret);
        urb_list_add_tail(&g_urb_rx_free_queue, urb_buf);
        return EXT_ERR_FAILURE;
    }

    ret = usb_wlan_xfer_read_func(data_head, urb_buf->buf);
    if (OAL_UNLIKELY(ret != EXT_ERR_SUCCESS)) {
        oal_usb_log(BUS_LOG_ERR, "read buz data err:[%d]", ret);
        urb_list_add_tail(&g_urb_rx_free_queue, urb_buf);
        return EXT_ERR_FAILURE;
    }

    // 业务数据根据usb pkg头读取,所以只清理头部空间
    ret = memset_s(urb_buf->buf, USB_RX_MAX_SIZE, 0, HIUSB_PACKAGE_HEARDER_SIZE);
    if (ret != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset urb buf err:[%d]", ret);
    }
    urb_list_add_tail(&g_urb_rx_free_queue, urb_buf);
    return EXT_ERR_SUCCESS;
}

td_void usb_reset_status(td_void)
{
    td_u32 usb_state = usb_get_bus_state();
    if (usb_state >= BUS_USB_INIT) {
        reinit_completion(&g_usb_boot_probe);
        reinit_completion(&g_usb_wifi_probe);
    }
}

static td_s32 usb_wlan_interupt_in(struct hcc_usb *pst_usb, td_u8 *buf, td_u32 len, td_u32 timeout)
{
    struct usb_dev_info *dev = TD_NULL;
    td_s32 ret;
    size_t size;
    td_s32 returnlen;
    if (pst_usb == TD_NULL) {
        return 0;
    }
    dev = usb_get_intfdata(pst_usb->interface_wlan);
    if (dev == TD_NULL) {
        return 0;
    }

    size = osal_min((size_t)len, (size_t)INT_TRANS_MAX_PACKAGE_SIZE);
    ret = oal_usb_interrupt_msg(dev->udev, dev->int_in_hep, buf, size, &returnlen, timeout, &g_interrupt_ctx);
    if (ret < 0) {
        g_usb_rx_error_pkts++;
        oal_usb_log(BUS_LOG_ERR, "usb_bulk_msg ret=%d", ret);
        return 0;
    }
    return returnlen;
}

/*  通过中断端点读取device事件 */
static td_s32 usb_read_device_event(struct hcc_usb *usb)
{
    td_s32 ret;
    td_s32 bit = 0;
    struct usb_dev_notification package = {0};

    ret = usb_wlan_interupt_in(usb, (td_u8*)&package, sizeof(struct usb_dev_notification), 0);
    if ((ret <= 0) && (g_usb_rx_error_pkts > USB_RX_PKT_MAX)) {
        return EXT_ERR_FAILURE;
    }
    oal_usb_log(BUS_LOG_DBG, "notification:0x%x, 0x%x\n", package.notification, package.dev_mem_highpri_pool);
    if (ret != sizeof(struct usb_dev_notification)) {
        oal_usb_log(BUS_LOG_ERR, "error notification:0x%x 0x%x\n", package.notification, ret);
        return EXT_ERR_SUCCESS;
    }
    g_usb_rx_error_pkts = 0;

    usb->dev_mem_highpri_pool = package.dev_mem_highpri_pool;
    /* 执行对应的回调函数 */
    if (test_bit(D2H_MSG_DEVICE_PANIC, (unsigned long*)&package.notification)) {
        oal_usb_log(BUS_LOG_ERR, "usb rx device panic msg");
        usb_disable_state(usb->pst_bus, HCC_BUS_STATE_ALL);
    }

    /* 优先处理Panic消息 */
    if (test_and_clear_bit(D2H_MSG_DEVICE_PANIC, (unsigned long*)&package.notification)) {
        bit = D2H_MSG_DEVICE_PANIC;
        if (hcc_get_exception_status() == TD_FALSE) {
            /* panic恢复流程 */
            hcc_bus_call_rx_message(usb->pst_bus, bit);
        }
    }

    bit = 0;
    for_each_set_bit(bit, (const unsigned long *)&package.notification, D2H_MSG_COUNT) {
        hcc_bus_call_rx_message(usb->pst_bus, bit);
    }
    return EXT_ERR_SUCCESS;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static td_void usb_kthread_set_fifo_priority(osal_task *task, td_u32 priority)
{
    osal_kthread_set_priority(task, priority);
    osal_kthread_set_affinity(task, OSAL_CPU_3);
}
#endif

static td_s32 usb_task_init(struct hcc_usb *usb,
    usb_thread_callback_fuc threadfn,
    const char* namefmt,
    osal_task** rx_handle)
{
    osal_task *handle = osal_kthread_create(threadfn, usb, namefmt, USB_TASK_STACK_SIZE);
    if (handle == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    *rx_handle = handle;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    usb_kthread_set_fifo_priority(handle, HCC_TRANS_THERAD_PRIORITY);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_kthread_set_priority(handle, HCC_TRANS_THERAD_PRIORITY);
#endif

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 分配urb流水资源
**************************************************************************** */
td_s32 usb_rx_urb_list_init(td_void)
{
    td_u32 i;

    if (g_urb_rx_queue_init) {
        return EXT_ERR_FAILURE;
    }

    /* init queue */
    urb_list_head_init(&g_urb_rx_work_queue);
    urb_list_head_init(&g_urb_rx_free_queue);

    for (i = 0; i < URB_RX_MAX_NUM; i++) {
        g_urb_rx_queue_node[i] = osal_kmalloc(sizeof(urb_buf_stru), OSAL_GFP_ATOMIC);
        if (g_urb_rx_queue_node[i] == TD_NULL) {
            return EXT_ERR_MALLOC_FAILURE;
        }

        /* malloc urb buf */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        usb_trx_urb_list_malloc_memory_opt(g_urb_rx_queue_node, i);
#else
        g_urb_rx_queue_node[i]->buf = osal_kmalloc(USB_RX_MAX_SIZE, OSAL_GFP_ATOMIC);
#endif
        if (g_urb_rx_queue_node[i]->buf == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "alloc node buf");
            return EXT_ERR_FAILURE;
        }

        /* malloc urb */
        g_urb_rx_queue_node[i]->urb = usb_alloc_urb(0, GFP_ATOMIC);
        if (g_urb_rx_queue_node[i]->urb == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "alloc node urb");
            return EXT_ERR_FAILURE;
        }
    }
    g_urb_rx_queue_init = TD_TRUE;
    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : urb流水资源回收
**************************************************************************** */
td_void usb_rx_urb_list_deinit(td_void)
{
    td_u32 i;

    if (!g_urb_rx_queue_init) {
        return;
    }

    for (i = 0; i < URB_RX_MAX_NUM; i++) {
        if (g_urb_rx_queue_node[i] != TD_NULL) {
            if (g_urb_rx_queue_node[i]->buf != TD_NULL) {
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                usb_trx_urb_list_free_memory_opt(g_urb_rx_queue_node, i);
#else
                osal_kfree(g_urb_rx_queue_node[i]->buf);
#endif
                g_urb_rx_queue_node[i]->buf = TD_NULL;
            }

            /* release urb resource whether urb is in the usb core or not */
            if (g_urb_rx_queue_node[i]->urb != TD_NULL) {
                usb_kill_urb(g_urb_rx_queue_node[i]->urb);
                usb_free_urb(g_urb_rx_queue_node[i]->urb);
                g_urb_rx_queue_node[i]->urb = TD_NULL;
            }
            osal_kfree(g_urb_rx_queue_node[i]);
            g_urb_rx_queue_node[i] = TD_NULL;
        }
    }
    osal_spin_lock_destroy(&g_urb_rx_work_queue.data_queue_lock);
    osal_spin_lock_destroy(&g_urb_rx_free_queue.data_queue_lock);
    g_urb_rx_queue_init = TD_FALSE;
}

td_void usb_rx_urb_list_reset(td_void)
{
    td_u32 i;

    /* init queue */
    urb_list_head_reset(&g_urb_rx_work_queue);
    urb_list_head_reset(&g_urb_rx_free_queue);

    for (i = 0; i < URB_RX_MAX_NUM; i++) {
        (osal_void)memset_s(g_urb_rx_queue_node[i]->buf, USB_RX_MAX_SIZE, 0, USB_RX_MAX_SIZE);
        urb_list_add_tail(&g_urb_rx_free_queue, g_urb_rx_queue_node[i]);
    }
}

td_void usb_rx_urb_list_clean(td_void)
{
    td_u32 i;

    urb_list_head_reset(&g_urb_rx_work_queue);
    urb_list_head_reset(&g_urb_rx_free_queue);

    for (i = 0; i < URB_RX_MAX_NUM; i++) {
        usb_kill_urb(g_urb_rx_queue_node[i]->urb);
    }
}

/* remove mark, force sched usb urb rx thread */
td_void usb_sched_rx_thread(td_void)
{
    osal_atomic_set(&g_usb->usb_rx_cond, 1);
    osal_wait_wakeup(&g_usb->usb_rx_wq);
}

/* ****************************************************************************
功能描述  : urb提交完成,将urb放入urb work队列.
          用于rx data线程构建hcc队列
**************************************************************************** */
static td_void usb_rx_urb_bulk_submit(struct urb *urb)
{
    urb_buf_stru *urb_buf = urb->context;
    /* add urb_buf to work queue. Urb rx handler thread will handle it */
    if (usb_get_bus_state() != BUS_USB_WORK) {
        oal_usb_log(BUS_LOG_ERR, "state");
        urb_list_add_head(&g_urb_rx_free_queue, urb_buf);
    } else {
        urb_list_add_tail(&g_urb_rx_work_queue, urb_buf);
    }
    usb_sched_rx_thread();
}

/* ****************************************************************************
功能描述  : urb提交
**************************************************************************** */
static td_s32 oal_usb_rx_urb_submit(struct usb_dev_info *dev)
{
    td_s32 ret;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe;
#endif
    urb_buf_stru *urb_buf = TD_NULL;

    if (dev == TD_NULL || dev->bulk_in_hep == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "invalid param");
        return EXT_ERR_FAILURE;
    }

    if (urb_list_is_empty(&g_urb_rx_free_queue) == TD_TRUE) {
        /* No free urb buf in list */
        return 0;
    }

    /* pop urb_buf, filling urb */
    urb_buf = urb_list_dequeue(&g_urb_rx_free_queue);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep);
    usb_fill_bulk_urb(urb_buf->urb, dev->udev, pipe, urb_buf->buf, USB_RX_MAX_SIZE,
        usb_rx_urb_bulk_submit, TD_NULL);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    usb_fill_bulk_urb(urb_buf->urb, dev->udev, dev->bulk_in_hep, urb_buf->buf, USB_RX_MAX_SIZE,
        usb_rx_urb_bulk_submit, TD_NULL);
#endif

    urb_buf->urb->context = urb_buf;
    urb_buf->urb->actual_length = 0;
    /* submit urb and waiting urb rx handler handle it */
    ret = usb_submit_urb(urb_buf->urb, GFP_ATOMIC);
    if (OAL_UNLIKELY(ret)) {
        oal_usb_log(BUS_LOG_ERR, "urb submit err:[%d]", ret);
        urb_list_add_head(&g_urb_rx_free_queue, urb_buf);
        return EXT_ERR_FAILURE;
    }

    return 1;
}

/* ****************************************************************************
功能描述  : 检查是否有ready的urb并构建hcc队列
**************************************************************************** */
static td_s32 usb_read_rx_urb_data(struct hcc_usb *usb, hcc_data_queue *rx_assemble_head)
{
    td_s32 ret = EXT_ERR_SUCCESS;

    if (urb_list_is_empty(&g_urb_rx_work_queue) == TD_FALSE) {
        /* 读取数据,构建hcc数据队列 */
        ret = usb_rx_netbuf(usb->pst_bus, rx_assemble_head);
        if (ret == EXT_ERR_SUCCESS) {
            hcc_bus_rx_handler(usb->pst_bus, usb->hcc_queue_id, rx_assemble_head);
        } else if (ret == EXT_ERR_FAILURE) {
            if (osal_atomic_read(&g_usb->usb_suspend) != USB_SUS_SUSPEND) {
                oal_usb_log(BUS_LOG_ERR, "rx_urb_data[%d]", ret);
                return EXT_ERR_FAILURE;
            }
        }
    }

    /* work queue empty, block the thread. Make sure cpu is relax */
    if ((urb_list_is_empty(&g_urb_rx_work_queue) == TD_TRUE) &&
            (urb_list_is_empty(&g_urb_rx_free_queue) == TD_TRUE)) {
        /* Make sure blocking is before the interrupt, otherwise will cause dead lock */
        ret = osal_wait_interruptible(&g_usb->usb_rx_wq, usb_thread_condtion, (td_void *)(&g_usb->usb_rx_cond));
        if (OAL_UNLIKELY(ret == -ERESTARTSYS)) {
            oal_usb_log(BUS_LOG_ERR, "task was interrupted by a signal");
        }
    }

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : host接受bulk数据线程
**************************************************************************** */
static td_s32 usb_transfer_rx_data_handle(struct hcc_usb *usb, struct usb_dev_info *dev)
{
    td_s32 ret = EXT_ERR_SUCCESS;
    hcc_data_queue rx_assemble_head;

    usb->is_wlan_rx_thread_running = TSK_USB_RUNING;
    if (hcc_list_head_init(&rx_assemble_head)) {
        oal_usb_log(BUS_LOG_ERR, "init rx assemble info failed");
        return EXT_ERR_FAILURE;
    }
    oal_usb_log(BUS_LOG_INFO, "start");
    usb_kthread_set_fifo_priority(g_usb->rx_data_handle, USB_THREDD_DATA_PRI);
    while (1) {
        if (OAL_UNLIKELY(osal_kthread_should_stop() != 0) || (usb->is_wlan_rx_thread_running <= TSK_USB_SHOULD_STOP)) {
            oal_usb_log(BUS_LOG_ERR, "rx data thread[%d] leave", usb->is_wlan_rx_thread_running);
            break;
        }

        if ((osal_atomic_read(&g_usb->usb_suspend) == USB_SUS_SUSPEND) &&
                !wait_for_completion_timeout(&g_usb_resume_wlan_complete, MAX_SCHEDULE_TIMEOUT)) {
            oal_usb_log(BUS_LOG_ERR, "wait usb resume timeout");
            break;
        }

        /* work queue is not empty, sched rx handle to receive urb data */
        ret = usb_read_rx_urb_data(usb, &rx_assemble_head);
        if (ret < 0) {
            break;
        }

        ret = oal_usb_rx_urb_submit(dev);
        if ((ret < 0) && (osal_atomic_read(&g_usb->usb_suspend) != USB_SUS_SUSPEND)) {
            oal_usb_log(BUS_LOG_ERR, "urb submit err:[%d]", ret);
            break;
        }
    }

    hcc_list_free((hcc_handler *)usb->pst_bus->hcc, &rx_assemble_head);
    hcc_list_head_deinit(&rx_assemble_head);
    oal_usb_log(BUS_LOG_INFO, "usb_transfer_rx_data_handle exit!\n");
    /* 如果数据异常尝试恢复流程 */
    hcc_exception_reset_work();
    return ret;
}

static td_s32 usb_xfer_wifi_rx_data_thread(td_void *data)
{
    struct hcc_usb *usb = (struct hcc_usb *)data;
    td_s32 ret = EXT_ERR_SUCCESS;
    struct usb_dev_info *dev = TD_NULL;

    if (usb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "invalid param");
        return EXT_ERR_FAILURE;
    }

    if (usb->interface_wlan == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    dev = usb_get_intfdata(usb->interface_wlan);
    if (dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "dev is null");
        return EXT_ERR_FAILURE;
    }

    usb_rx_urb_list_reset();
    osal_atomic_set(&g_usb->usb_rx_cond, 1);
    ret = usb_transfer_rx_data_handle(usb, dev);
    if (g_rx_res_should_release) {
        g_rx_res_should_release = TD_FALSE;
        usb_rx_urb_list_deinit();
    } else {
        usb_rx_urb_list_clean();
    }

    // 完成USB资源释放后再置线程退出标志
    usb->is_wlan_rx_thread_running = TSK_USB_EXIT;
    return ret;
}

static td_s32 usb_xfer_rx_msg_thread(td_void *data)
{
    td_s32 ret;
    struct hcc_usb *usb = (struct hcc_usb *)data;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    usb_kthread_set_fifo_priority(g_usb->rx_msg_handle, USB_THREDD_PRI);
#endif
    if ((usb == TD_NULL) || (usb->interface_wlan == TD_NULL)) {
        oal_usb_log(BUS_LOG_ERR, "usb is null");
        return EXT_ERR_FAILURE;
    }
    usb->is_rx_msg_thread_running = TSK_USB_RUNING;

    while (1) {
        if ((OAL_UNLIKELY(osal_kthread_should_stop() != 0)) ||
            (usb->is_rx_msg_thread_running <= TSK_USB_SHOULD_STOP)) {
            oal_usb_log(BUS_LOG_INFO, "usb_xfer_rx_msg_thread[%d] leave", usb->is_rx_msg_thread_running);
            break;
        }

        if (osal_atomic_read(&g_usb->usb_suspend) &&
            !wait_for_completion_timeout(&g_usb_resume_msg_complete, MAX_SCHEDULE_TIMEOUT)) {
            oal_usb_log(BUS_LOG_INFO, "usb_xfer_rx_msg_thread timeout");
            break;
        }

        ret = usb_read_device_event(usb);
        if (ret < 0) {
            oal_usb_log(BUS_LOG_INFO, "usb_read_device_event");
            if (osal_atomic_read(&g_usb->usb_suspend)) {
                continue;
            }
            break;
        }
    }
    usb->is_rx_msg_thread_running = TSK_USB_EXIT;
    oal_usb_log(BUS_LOG_INFO, "usb_xfer_rx_msg_thread exit!");
    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 保存寄存器读写断点信息
**************************************************************************** */
static td_s32 oal_usb_init_rw_reg_ep(struct usb_dev_info *dev, struct usb_host_interface *iface_desc)
{
    dev->reg_out_hep = &iface_desc->endpoint[RW_REG_EP_OUT_IND];
    dev->reg_in_hep = &iface_desc->endpoint[RW_REG_EP_IN_IND];
    dev->rw_reg_bulk_out_ep = iface_desc->endpoint[RW_REG_EP_OUT_IND].desc.bEndpointAddress;
    dev->rw_reg_bulk_in_ep = iface_desc->endpoint[RW_REG_EP_IN_IND].desc.bEndpointAddress;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (usb_setup_endpoint(dev->udev, dev->reg_out_hep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup reg_out");
        return EXT_ERR_FAILURE;
    }
    if (usb_setup_endpoint(dev->udev, dev->reg_in_hep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup reg_in");
        return EXT_ERR_FAILURE;
    }
#endif
    oal_usb_log(BUS_LOG_INFO, "rw reg ep_in:[%u], ep_out:[%u]", dev->rw_reg_bulk_in_ep, dev->rw_reg_bulk_out_ep);
    return EXT_ERR_SUCCESS;
}

static td_s32 oal_usb_system_init_bulk_ep(struct usb_dev_info *dev, struct usb_host_interface *iface_desc,
    hcc_usb_device_speed *usb_mode)
{
    struct usb_host_endpoint *bulk_out_ep;
    struct usb_host_endpoint *bulk_in_ep;

    bulk_in_ep = &iface_desc->endpoint[BULK_EP_IN_IND];
    bulk_out_ep = &iface_desc->endpoint[BULK_EP_OUT_IND];
    if (bulk_out_ep == TD_NULL || bulk_in_ep == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "Bulk ep not found");
        return EXT_ERR_FAILURE;
    }

    oal_usb_log(BUS_LOG_INFO, "%x %x %x %x %x %x", bulk_in_ep->desc.bLength,
        bulk_in_ep->desc.bDescriptorType,
        bulk_in_ep->desc.bEndpointAddress,
        bulk_in_ep->desc.bmAttributes,
        bulk_in_ep->desc.wMaxPacketSize,
        bulk_in_ep->desc.bInterval);

    if (bulk_in_ep->desc.wMaxPacketSize == 0x400) {
        *usb_mode = HCC_USB_SPEED_SUPER;
    } else {
        *usb_mode = HCC_USB_SPEED_HIGH;
    }

    dev->bulk_in_hep = bulk_in_ep;
    dev->bulk_out_hep = bulk_out_ep;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (usb_setup_endpoint(dev->udev, bulk_in_ep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup bulk in");
        return EXT_ERR_FAILURE;
    }
    if (usb_setup_endpoint(dev->udev, bulk_out_ep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup bulk out");
        return EXT_ERR_FAILURE;
    }
#endif

    dev->bulk_in_ep = bulk_in_ep->desc.bEndpointAddress;
    dev->bulk_out_ep = bulk_out_ep->desc.bEndpointAddress;
    dev->bulk_in_buffer = TD_NULL;
    dev->buik_in_buffer_size = 0;
    dev->bulk_in_max_pkt_size = bulk_in_ep->desc.wMaxPacketSize;

    dev->bulk_out_buffer = TD_NULL;
    dev->buik_out_buffer_size= 0;
    dev->bulk_out_max_pkt_size = bulk_out_ep->desc.wMaxPacketSize;
    return EXT_ERR_SUCCESS;
}

static struct usb_dev_info *usb_priv_dev_create(struct usb_interface *interface, struct usb_device *udev)
{
    errno_t err_num;
    struct usb_dev_info *dev;
    dev = osal_kmalloc(sizeof(*dev), OSAL_GFP_KERNEL);
    if (!dev) {
        oal_usb_log(BUS_LOG_ERR, "Out of memory");
        return TD_NULL;
    }
    err_num = memset_s(dev, sizeof(*dev), 0, sizeof(*dev));
    if (err_num != EOK) {
        osal_kfree(dev);
        oal_usb_log(BUS_LOG_ERR, "memset dev failed:[%d]", err_num);
        return TD_NULL;
    }
    if (osal_mutex_init(&dev->io_mutex) != OSAL_SUCCESS) {
        osal_kfree(dev);
        oal_usb_log(BUS_LOG_ERR, "memset dev failed:[%d]", err_num);
        return TD_NULL;
    }
    kref_init(&dev->kref);
    dev->udev = udev;
    dev->interface = interface;
    return dev;
}

static td_s32 oal_usb_system_ep_init(struct usb_interface *interface, struct usb_device *udev,
    hcc_usb_device_speed *usb_mode)
{
    struct usb_dev_info *dev;
    struct usb_host_endpoint *int_in_ep;
    struct usb_host_interface *iface_desc;
    iface_desc = interface->cur_altsetting;

    dev = usb_priv_dev_create(interface, udev);
    if (dev == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    if (oal_usb_system_init_bulk_ep(dev, iface_desc, usb_mode) != EXT_ERR_SUCCESS) {
        goto error;
    }
    if (iface_desc->desc.bInterfaceNumber == 0) {
        int_in_ep = &iface_desc->endpoint[INT_EP_IN_IND];
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
        if (usb_setup_endpoint(dev->udev, int_in_ep, EP_SETUP_BUF_LEN)) {
            oal_usb_log(BUS_LOG_ERR, "usb_setup int in");
            goto error;
        }
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        if (udev->speed == USB_SPEED_HIGH || udev->speed == USB_SPEED_SUPER) {
            int_in_ep->desc.bInterval = 4; // 中断传输轮询周期改为 2^(4-1) * 125us = 1000us
        }
#endif
        dev->int_in_hep = int_in_ep;
        dev->int_in_ep = int_in_ep->desc.bEndpointAddress;
        dev->int_in_buffer = osal_kmalloc(INT_TRANS_MAX_PACKAGE_SIZE, OSAL_GFP_KERNEL);
        if (!dev->int_in_buffer) {
            oal_usb_log(BUS_LOG_ERR, "Out of memory");
            goto error;
        }
        dev->int_in_buffer_size = INT_TRANS_MAX_PACKAGE_SIZE;
    }
    oal_usb_log(BUS_LOG_INFO, "EP bulk_in:%d, bulk_out:%d, int_in:%d",
        dev->bulk_in_ep, dev->bulk_out_ep, dev->int_in_ep);
    usb_set_intfdata(interface, dev);

    if (oal_usb_init_rw_reg_ep(dev, iface_desc) != EXT_ERR_SUCCESS) {
        goto error;
    }
    return EXT_ERR_SUCCESS;
error:
    if (dev) {
        kref_put(&dev->kref, usb_dev_delete);
    }
    return EXT_ERR_FAILURE;
}

static td_s32 oal_usb_system_init_config(struct usb_interface *interface, struct usb_device *udev)
{
    td_s32 ret = EXT_ERR_FAILURE;
    struct usb_host_interface *iface_desc;
    hcc_usb_device_speed usb_mode = HCC_USB_SPEED_HIGH;
    td_u32 old_state = usb_get_bus_state();
    struct usb_dev_info *dev = TD_NULL;

    iface_desc = interface->cur_altsetting;
    hcc_set_exception_status(TD_FALSE);

    ret = oal_usb_system_ep_init(interface, udev, &usb_mode);
    if (ret != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "oal_usb_system_ep_init");
        return ret;
    }

    // 将接口存放在这个里面，然后通过接口，获取dev
    if (iface_desc->desc.bInterfaceNumber == 0) {
        g_usb->interface_wlan= interface;
    }

    if (g_usb->interface_wlan) {
        g_usb->connect = TD_TRUE;
        g_usb->usb_mode = usb_mode;

        usb_tx_urb_list_reset();
        if (usb_hcc_task_init_func() != EXT_ERR_SUCCESS) {
            goto error;
        }

        usb_set_bus_state(BUS_USB_WORK);
        if (old_state == BUS_USB_SUSPEND_DISCONNECT) {
            /* notify exception msg restart */
        } else {
            complete(&g_usb_wifi_probe);
        }
        g_usb->pst_bus->state = 1;
        oal_usb_log(BUS_LOG_INFO, "oal_usb_probe success!");
    }
    return EXT_ERR_SUCCESS;
error:
    dev = usb_get_intfdata(interface);
    if (dev) {
        kref_put(&dev->kref, usb_dev_delete);
    }
    return ret;
}

static td_s32 usb_hcc_res_lock_init(struct hcc_usb *usb)
{
    td_s32 ret;
    ret = osal_wait_init(&usb->usb_tx_wq);
    if (ret != OSAL_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    ret = osal_wait_init(&usb->usb_rx_wq);
    if (ret != OSAL_SUCCESS) {
        goto faile_usb_rx_wq;
    }
    // 状态锁
    ret = osal_mutex_init(&usb->state_lock);
    if (ret != OSAL_SUCCESS) {
        goto faile_usb_stae_lock;
    }
    ret = osal_mutex_init(&usb->ctrl_ep_lock);
    if (ret != OSAL_SUCCESS) {
        goto faile_ctrl_ep_lock;
    }
    ret = usb_reg_lock_init(&usb->reg_rw_lock);
    if (ret != OSAL_SUCCESS) {
        goto faile_reg_lock;
    }
    return EXT_ERR_SUCCESS;

faile_reg_lock:
    osal_mutex_destroy(&usb->ctrl_ep_lock);
faile_ctrl_ep_lock:
    osal_mutex_destroy(&usb->state_lock);
faile_usb_stae_lock:
    osal_wait_destroy(&usb->usb_rx_wq);
faile_usb_rx_wq:
    osal_wait_destroy(&usb->usb_tx_wq);
    return EXT_ERR_FAILURE;
}

static struct hcc_usb *usb_hcc_res_init(td_void)
{
    errno_t err_num;
    struct hcc_usb *usb = TD_NULL;

    usb = (struct hcc_usb*)osal_kmalloc(sizeof(struct hcc_usb), OSAL_GFP_KERNEL);
    if (usb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "alloc");
        return TD_NULL;
    }
    err_num = memset_s((td_void*)usb, sizeof(struct hcc_usb), 0, sizeof(struct hcc_usb));
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset[%d]", err_num);
    }

    if (usb_hcc_res_lock_init(usb) != EXT_ERR_SUCCESS) {
        osal_kfree(usb);
        return TD_NULL;
    }

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    g_usb_bulk_out_buffer = osal_kmalloc(MAX_FIRMWARE_FILE_TX_BUF_LEN, OSAL_GFP_KERNEL);
    if (g_usb_bulk_out_buffer == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "alloc bulk_out_buffer fail");
        usb_hcc_res_lock_deinit(usb);
        osal_kfree(usb);
        return TD_NULL;
    }
    (td_void)memset_s(g_usb_bulk_out_buffer, MAX_FIRMWARE_FILE_TX_BUF_LEN, 0, MAX_FIRMWARE_FILE_TX_BUF_LEN);
#endif
#endif

    if (hcc_list_head_init(&usb->tx_assemble_info.assembled_head)) {
        oal_usb_log(BUS_LOG_ERR, "init tx assemble");
        goto error2;
    }

    usb->descript_unc.buf = (td_u8 *)osal_kmalloc(H2D_DESC_BUF_SZIE, OSAL_GFP_KERNEL);
    if (usb->descript_unc.buf == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "alloc desc");
        goto error1;
    }

    err_num = memset_s(usb->descript_unc.buf, H2D_DESC_BUF_SZIE, 0, H2D_DESC_BUF_SZIE);
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset desc[%d]", err_num);
    }
    usb->descript_unc.length = HIUSB_HOST2DEV_SCATT_MAX;
    usb->descript_unc.service_type = HCC_SERVICE_TYPE_MAX;
    usb->descript_unc.user_param = TD_NULL;
    OSAL_INIT_LIST_HEAD(&usb->descript_unc.list);

    osal_atomic_init(&usb->usb_tx_cond);
    osal_atomic_init(&usb->usb_rx_cond);
    osal_atomic_init(&usb->usb_suspend);
    osal_atomic_init(&usb->usb_driver_status);
    if (usb_tx_urb_list_init() != EXT_ERR_SUCCESS) {
        usb_tx_urb_list_deinit();
        osal_kfree(usb->descript_unc.buf);
        goto error1;
    }

    if (usb_rx_urb_list_init() != EXT_ERR_SUCCESS) {
        usb_rx_urb_list_deinit();
        usb_tx_urb_list_deinit();
        osal_kfree(usb->descript_unc.buf);
        goto error1;
    }

    return usb;
error1:
    hcc_list_head_deinit(&usb->tx_assemble_info.assembled_head);
error2:
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    osal_kfree(g_usb_bulk_out_buffer);
    g_usb_bulk_out_buffer = TD_NULL;
#endif
#endif
    usb_hcc_res_lock_deinit(usb);
    osal_kfree(usb);
    return TD_NULL;
}

#if defined(CONFIG_USB_DESR_DEBUG) || defined(_PRE_PLAT_SMOKE_TEST)
static td_void usb_debug_print_dev_info(struct usb_device_descriptor *dev)
{
    hcc_debug("usb dev descriptor:\r\n");

    hcc_debug("dev->bLength:0x%x\r\n", dev->bLength);
    hcc_debug("dev->bDescriptorType:0x%x\r\n", dev->bDescriptorType);
    hcc_debug("dev->bcdUSB:0x%x\r\n", dev->bcdUSB);
    hcc_debug("dev->bDeviceClass:0x%x\r\n", dev->bDeviceClass);
    hcc_debug("dev->bDeviceSubClass:0x%x\r\n", dev->bDeviceSubClass);
    hcc_debug("dev->bDeviceProtocol:0x%x\r\n", dev->bDeviceProtocol);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hcc_debug("dev->bMaxPacketSize0:0x%x\r\n", dev->bMaxPacketSize0);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    hcc_debug("dev->bMaxPacketSize:0x%x\r\n", dev->bMaxPacketSize);
#endif

    hcc_debug("dev->idVendor:0x%x\r\n", dev->idVendor);
    hcc_debug("dev->idProduct:0x%x\r\n", dev->idProduct);
    hcc_debug("dev->bcdDevice:0x%x\r\n", dev->bcdDevice);
    hcc_debug("dev->iManufacturer:0x%x\r\n", dev->iManufacturer);
    hcc_debug("dev->iProduct:0x%x\r\n", dev->iProduct);
    hcc_debug("dev->iSerialNumber:0x%x\r\n", dev->iSerialNumber);
    hcc_debug("dev->bNumConfigurations:0x%x\r\n", dev->bNumConfigurations);
}

static td_void usb_debug_print_intf_info(struct usb_interface_descriptor *intf)
{
    hcc_debug("usb iface descriptor:\r\n");

    hcc_debug("intf->bLength:0x%x\r\n", intf->bLength);
    hcc_debug("intf->bDescriptorType:0x%x\r\n", intf->bDescriptorType);
    hcc_debug("intf->bInterfaceNumber:0x%x\r\n", intf->bInterfaceNumber);
    hcc_debug("intf->bAlternateSetting:0x%x\r\n", intf->bAlternateSetting);
    hcc_debug("intf->bNumEndpoints:0x%x\r\n", intf->bNumEndpoints);
    hcc_debug("intf->bInterfaceClass:0x%x\r\n", intf->bInterfaceClass);
    hcc_debug("intf->bInterfaceSubClass:0x%x\r\n", intf->bInterfaceSubClass);
    hcc_debug("intf->bInterfaceProtocol:0x%x\r\n", intf->bInterfaceProtocol);
    hcc_debug("intf->iInterface:0x%x\r\n", intf->iInterface);
}

static td_void usb_debug_print_ep_info(struct usb_host_endpoint *ep_info, td_u32 ep_num)
{
    td_u32 i;

    hcc_debug("usb ep descriptor:\r\n");

    for (i = 0; i < ep_num; i++) {
        struct usb_endpoint_descriptor *ep = &(ep_info[i].desc);
        hcc_debug("ep[%d].bLength:0x%x\r\n", i, ep->bLength);
        hcc_debug("ep[%d].bDescriptorType:0x%x\r\n", i, ep->bDescriptorType);
        hcc_debug("ep[%d].bEndpointAddress:0x%x\r\n", i, ep->bEndpointAddress);
        hcc_debug("ep[%d].bmAttributes:0x%x\r\n", i, ep->bmAttributes);
        hcc_debug("ep[%d].wMaxPacketSize:0x%x\r\n", i, ep->wMaxPacketSize);
        hcc_debug("ep[%d].bInterval:0x%x\r\n", i, ep->bInterval);
    }
}

static td_void usb_debug_desr_info(struct usb_device *udev, struct usb_host_interface *iface_desc)
{
    usb_debug_print_dev_info(&(udev->descriptor));
    usb_debug_print_intf_info(&(iface_desc->desc));
    usb_debug_print_ep_info(iface_desc->endpoint, iface_desc->desc.bNumEndpoints);

    hcc_debug("usb string descriptor:\r\n");
    hcc_debug("manufacturer:%s\r\nproduct:%s\r\nserial:%s\r\n", udev->manufacturer, udev->product, udev->serial);
}
#endif

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
td_s32 oal_usb_boot_bulk_in_probe(struct usb_dev_info *dev, struct usb_host_interface *iface_desc)
{
    struct usb_host_endpoint *bulk_in_ep;

    bulk_in_ep = &iface_desc->endpoint[BULK_EP_IN_IND];
    if (bulk_in_ep == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "Bulk in not found");
        return EXT_ERR_FAILURE;
    }

    dev->bulk_in_hep = bulk_in_ep;
    dev->bulk_in_ep = bulk_in_ep->desc.bEndpointAddress;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (usb_setup_endpoint(dev->udev, bulk_in_ep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup bulk in");
        return EXT_ERR_FAILURE;
    }
#endif
    dev->bulk_in_buffer = TD_NULL;
    dev->buik_in_buffer_size = 0;
    dev->bulk_in_max_pkt_size = bulk_in_ep->desc.wMaxPacketSize;
    oal_usb_log(BUS_LOG_INFO, "bulk_in_ep:0x%x", dev->bulk_in_ep);
    return EXT_ERR_SUCCESS;
}

td_s32 oal_usb_boot_bulk_out_probe(struct usb_dev_info *dev, struct usb_host_interface *iface_desc)
{
    struct usb_host_endpoint *bulk_out_ep;

    bulk_out_ep = &iface_desc->endpoint[BULK_EP_OUT_IND];
    dev->bulk_out_hep = bulk_out_ep;
    dev->bulk_out_ep = bulk_out_ep->desc.bEndpointAddress;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    if (usb_setup_endpoint(dev->udev, bulk_out_ep, EP_SETUP_BUF_LEN)) {
        oal_usb_log(BUS_LOG_ERR, "usb_setup bulk out");
        return EXT_ERR_FAILURE;
    }
#endif

#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    (td_void)memset_s(g_usb_bulk_out_buffer, MAX_FIRMWARE_FILE_TX_BUF_LEN, 0, MAX_FIRMWARE_FILE_TX_BUF_LEN);
    dev->bulk_out_buffer = g_usb_bulk_out_buffer;
#else
    dev->bulk_out_buffer = osal_kmalloc(MAX_FIRMWARE_FILE_TX_BUF_LEN, OSAL_GFP_KERNEL);
    if (dev->bulk_out_buffer == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "Out of memory");
        return EXT_ERR_FAILURE;
    }
#endif

    dev->buik_out_buffer_size = MAX_FIRMWARE_FILE_TX_BUF_LEN;
    dev->bulk_out_max_pkt_size = bulk_out_ep->desc.wMaxPacketSize;
    oal_usb_log(BUS_LOG_INFO, "dev->bulk_out_ep:0x%x", dev->bulk_out_ep);
    return EXT_ERR_SUCCESS;
}

static td_s32 oal_usb_boot_init_config(struct usb_interface *interface, struct usb_device *udev,
    struct usb_host_interface *iface_desc)
{
    errno_t err_num;
    struct usb_dev_info *dev;

    hcc_switch_status(HCC_BUS_FORBID);
    dev = osal_kmalloc(sizeof(*dev), OSAL_GFP_KERNEL);
    if (dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "Out of memory");
        return EXT_ERR_FAILURE;
    }

    err_num = memset_s(dev, sizeof(*dev), 0, sizeof(*dev));
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset dev failed:[%d]", err_num);
    }

    kref_init(&dev->kref);
    dev->udev = udev;
    dev->interface = interface;
    if (osal_mutex_init(&dev->io_mutex) != OSAL_SUCCESS) {
        goto error;
    }

    if (oal_usb_boot_bulk_in_probe(dev, iface_desc) != EXT_ERR_SUCCESS ||
        oal_usb_boot_bulk_out_probe(dev, iface_desc) != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "set bulk in/out error");
        goto error;
    }

    usb_set_intfdata(interface, dev);
    g_usb->interface_wlan = interface;
    g_usb->connect = TD_TRUE;

    usb_set_bus_state(BUS_USB_BOOT);
    g_usb->pst_bus->state = TD_TRUE;
    /*  disconnect --> boot, need notify exception msg */
    complete(&g_usb_boot_probe);
    oal_usb_log(BUS_LOG_INFO, "usb boot probe success");
    return EXT_ERR_SUCCESS;

error:
    kref_put(&dev->kref, usb_dev_delete);
    return EXT_ERR_FAILURE;
}
#endif

static td_s32 oal_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_host_interface *iface_desc;
    struct usb_device *udev = interface_to_usbdev(interface);

    reinit_completion(&g_usb_disconnect);

    iface_desc = interface->cur_altsetting;
#if defined(CONFIG_USB_DESR_DEBUG) || defined(_PRE_PLAT_SMOKE_TEST)
    usb_debug_desr_info(udev, iface_desc);
#endif
    if (iface_desc->desc.bNumEndpoints == DEVICE_KERNEL_EP_NUM) { // kernel
        oal_usb_log(BUS_LOG_INFO, "kernel usb");
        return oal_usb_system_init_config(interface, udev);
    }
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
    if (iface_desc->desc.bNumEndpoints == DEVICE_BOOT_EP_NUM) { // boot
        oal_usb_log(BUS_LOG_INFO, "boot usb");
        return oal_usb_boot_init_config(interface, udev, iface_desc);
    }
#endif
    oal_usb_log(BUS_LOG_ERR, "usb not support:%d", iface_desc->desc.bNumEndpoints);
    return EXT_ERR_FAILURE;
}

static td_s32 oal_usb_suspend_resume(struct usb_dev_info* dev, td_s32 action)
{
    if (g_usb->connect) {
        switch (action) {
            case USB_SUS_SUSPEND:
                usb_set_bus_state(BUS_USB_SUSPEND);
                osal_atomic_set(&g_usb->usb_suspend, action);
                complete(&g_wlan_ctx.done);
                g_wlan_ctx.status = ERROR_USB_SUSPEND_STATUS;
                complete(&g_interrupt_ctx.done);
                g_interrupt_ctx.status = ERROR_USB_SUSPEND_STATUS;
                break;
            case USB_SUS_NORMAL:
                usb_set_bus_state(BUS_USB_WORK);
                osal_atomic_set(&g_usb->usb_suspend, action);
                complete(&g_usb_resume_wlan_complete);
                complete(&g_usb_resume_msg_complete);
                break;
            default:
                break;
        }
    }
    return 0;
}

static td_s32 oal_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
    struct usb_dev_info *dev = usb_get_intfdata(intf);
    struct hcc_usb *usb_priv = usb_get_handle();
    oal_usb_log(BUS_LOG_INFO, "oal_usb_suspend");

    if (dev == TD_NULL || usb_priv == TD_NULL) {
        return 0;
    }

    /* Wait until no command is running */
    osal_mutex_lock(&dev->io_mutex);
    oal_usb_suspend_resume(dev, USB_SUS_SUSPEND);
    osal_mutex_unlock(&dev->io_mutex);
    if (usb_priv != TD_NULL && usb_priv->suspend_notify_func != TD_NULL) {
        usb_priv->suspend_notify_func();
    }
    return 0;
}

static td_s32 oal_usb_resume(struct usb_interface *intf)
{
    struct usb_dev_info* dev = usb_get_intfdata(intf);
    struct hcc_usb *usb_priv = usb_get_handle();
    oal_usb_log(BUS_LOG_INFO, "oal_usb_resume");
    if (dev == TD_NULL || usb_priv == TD_NULL) {
        return 0;
    }
    osal_mutex_lock(&dev->io_mutex);
    oal_usb_suspend_resume(dev, USB_SUS_NORMAL);
    osal_mutex_unlock(&dev->io_mutex);
    if (usb_priv != TD_NULL && usb_priv->resume_notify_func != TD_NULL) {
        usb_priv->resume_notify_func();
    }
    return 0;
}

static struct usb_driver oal_usb_driver = {
    .name       =   "wireless_usb",
    .probe      =   oal_usb_probe,
    .disconnect =   oal_usb_disconnect,
    .suspend    =   oal_usb_suspend,
    .resume     =   oal_usb_resume,
    .id_table   =   g_usb_table,
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    .disable_hub_initiated_lpm = 1,
#endif
};

static td_void usb_task_rx_data_deinit(td_void)
{
    td_u32 timeout = USB_TASK_DEINIT_TIMES; /* 1000ms */
    if (g_usb->is_wlan_rx_thread_running == TSK_USB_RUNING) {
        g_usb->is_wlan_rx_thread_running = TSK_USB_SHOULD_STOP;
    }
    g_wlan_ctx.status = ERROR_USB_SUSPEND_STATUS;
    if (usb_get_bus_state() == BUS_USB_SUSPEND_DISCONNECT) {
        complete(&g_usb_resume_wlan_complete);
    }
    complete(&g_wlan_ctx.done);
    usb_sched_rx_thread();
    while (g_usb->is_wlan_rx_thread_running != TSK_USB_EXIT) {
        if (g_usb->rx_data_handle == TD_NULL) {
            break;
        }
        if (timeout == 0) {
            oal_usb_log(BUS_LOG_INFO, "wait thread exit timeout:%d\r\n", g_usb->is_wlan_rx_thread_running);
            break;
        }
        timeout--;
        osal_msleep(1);
    }

    osal_kthread_destroy(g_usb->rx_data_handle, 0);

    g_usb->is_wlan_rx_thread_running = TSK_USB_EXIT;
    g_usb->rx_data_handle = TD_NULL;
}

static td_void usb_task_rx_msg_deinit(td_void)
{
    td_u32 timeout = 1000; /* 1000ms */
    if (g_usb->is_rx_msg_thread_running == TSK_USB_RUNING) {
        g_usb->is_rx_msg_thread_running = TSK_USB_SHOULD_STOP;
    }
    g_interrupt_ctx.status = ERROR_USB_SUSPEND_STATUS;
    if (usb_get_bus_state() == BUS_USB_SUSPEND_DISCONNECT) {
        complete(&g_usb_resume_msg_complete);
    }
    complete(&g_interrupt_ctx.done);
    while (g_usb->is_rx_msg_thread_running != TSK_USB_EXIT) {
        if (g_usb->rx_msg_handle == TD_NULL) {
            break;
        }
        if (timeout == 0) {
            oal_usb_log(BUS_LOG_INFO, "wait thread exit timeout:%d\r\n", g_usb->is_rx_msg_thread_running);
            break;
        }
        timeout--;
        osal_msleep(1);
    }

    osal_kthread_destroy(g_usb->rx_msg_handle, 0);

    g_usb->is_rx_msg_thread_running = TSK_USB_EXIT;
    g_usb->rx_msg_handle = TD_NULL;
}

td_void usb_task_exit(td_void)
{
    if (g_usb->rx_data_handle != TD_NULL) {
        usb_task_rx_data_deinit();
    }
    if (g_usb->rx_msg_handle != TD_NULL) {
        usb_task_rx_msg_deinit();
    }
}

static td_void usb_task_deinit(const struct usb_interface *interface)
{
    if (g_usb->interface_wlan == interface) {
        usb_task_exit();
    }
    if ((g_usb->rx_data_handle == TD_NULL) && (g_usb->rx_msg_handle == TD_NULL)) {
        g_usb->interface_wlan = TD_NULL;
    }
}

static td_void oal_usb_disconnect(struct usb_interface *interface)
{
    struct usb_dev_info *dev;
    td_u32 old_state = usb_get_bus_state();

    oal_usb_log(BUS_LOG_INFO, "usb disconnect\r\n");

    if (g_usb != TD_NULL) {
        g_usb->connect = TD_FALSE;
        g_usb->pst_bus->state = 0;
    } else {
        oal_usb_log(BUS_LOG_ERR, "usb disconnect err, usb is unload\r\n");
        return;
    }
    if (old_state == BUS_USB_SUSPEND) {
        usb_set_bus_state(BUS_USB_SUSPEND_DISCONNECT);
    }
    usb_task_deinit(interface);

    dev = usb_get_intfdata(interface);
    if (dev != TD_NULL) {
        dev->interface = TD_NULL;
        kref_put(&dev->kref, usb_dev_delete);
    }
    usb_set_intfdata(interface, TD_NULL);

    if (old_state == BUS_USB_WORK) {
        usb_set_bus_state(BUS_USB_DISCONNECT);
    }
    if (old_state == BUS_USB_BOOT) {
        g_usb->interface_wlan = TD_NULL;
        usb_set_bus_state(BUS_USB_DISCONNECT);
    } else {
        usb_sched_tx_thread();
    }
    complete(&g_usb_disconnect);

    reinit_completion(&g_usb_boot_probe);
    reinit_completion(&g_usb_wifi_probe);
}

static td_s32 usb_hcc_task_init_func(td_void)
{
    td_s32 ret;
    /* 启动USB RX线程，读取device侧WIFI数据,然后回调hcc_bus_rx_handler函数 */
    ret = usb_task_init(g_usb, usb_xfer_wifi_rx_data_thread, "usb_wifi_rx_data_thread", &g_usb->rx_data_handle);
    if (ret != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "usb rx data thread failed");
        return EXT_ERR_FAILURE;
    }

    /* 启动一个线程，通过中断节点读取device MSG消息，通过中断传输 */
    ret = usb_task_init(g_usb, usb_xfer_rx_msg_thread, "usb_rx_msg_thread", &g_usb->rx_msg_handle);
    if (ret != EXT_ERR_SUCCESS) {
        osal_kthread_destroy(g_usb->rx_data_handle, 1);
        g_usb->rx_data_handle = TD_NULL;
        oal_usb_log(BUS_LOG_ERR, "usb rx msg thread failed");
        return EXT_ERR_FAILURE;
    }

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : kernel下载完成之后,重新加载usb驱动
            此阶段，有一个usb接口和三个EP
**************************************************************************** */
td_s32 hcc_usb_reload(hcc_bus *pst_bus)
{
    if (pst_bus == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "invalid param");
        return EXT_ERR_FAILURE;
    }

    if (pst_bus->bus_type != HCC_BUS_USB) {
        oal_usb_log(BUS_LOG_INFO, "bus type:[%u]", pst_bus->bus_type);
        return EXT_ERR_SUCCESS;
    }

    usleep_range(USB_ENUM_DEVICE_TIMEOUT, USB_ENUM_DEVICE_TIMEOUT);
    if (!wait_for_completion_timeout(&g_usb_wifi_probe, USB_WAIT_TIME * HZ)) {
        oal_usb_log(BUS_LOG_ERR, "wait_for_completion_timeout");
        return EXT_ERR_FAILURE;
    }

    osal_atomic_set(&g_usb->usb_tx_cond, 1);
    usb_enable_state(g_usb->pst_bus, HCC_BUS_STATE_ALL);
    oal_usb_log(BUS_LOG_INFO, "usb_reload succ");

    return EXT_ERR_SUCCESS;
}

td_s32 hcc_usb_reinit(hcc_bus *pst_bus)
{
    td_u32 usb_state;
    if (!wait_for_completion_timeout(&g_usb_boot_probe, USB_WAIT_TIME * HZ)) {
        usb_state = usb_get_bus_state();
        oal_usb_log(BUS_LOG_ERR, "Waiting for boot timeout, usb state is [%u]", usb_state);
        if (usb_state != BUS_USB_BOOT) {
            return EXT_ERR_FAILURE;
        }
    }
    oal_usb_log(BUS_LOG_DBG, "success");
    return EXT_ERR_SUCCESS;
}

td_s32 hcc_usb_wait_disconnect(void)
{
    if (g_usb == TD_NULL) {
        return EXT_ERR_SUCCESS;
    }
    if (g_usb->connect == TD_FALSE) {
        return EXT_ERR_SUCCESS;
    }
    if (!wait_for_completion_timeout(&g_usb_disconnect, USB_WAIT_TIME * HZ)) {
        oal_usb_log(BUS_LOG_INFO, "Waiting for disconnect timeout, usb state is [%u]", usb_get_bus_state());
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : ko加载的时候,usb分配资源和驱动注册，
            此阶段不一定探卡成功
**************************************************************************** */
hcc_bus *hcc_adapt_usb_load(td_void)
{
    td_s32 ret;

#ifdef CONFIG_HCC_SUPPORT_REG_OPT
    if (usb_reg_info_init() != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "usb_reg_info_init");
        return TD_NULL;
    }
#endif

    if (g_usb == TD_NULL) {
        g_usb = usb_hcc_res_init();
    }
    if (g_usb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "hcc res init");
        return TD_NULL;
    }

    if (usb_bus_init(g_usb) == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "hcc bus int fail.");
        usb_hcc_res_deinit();
        return TD_NULL;
    }

    init_completion(&g_usb_disconnect);
    init_completion(&g_usb_boot_probe);
    init_completion(&g_usb_wifi_probe);
    init_completion(&g_usb_resume_wlan_complete);
    init_completion(&g_usb_resume_msg_complete);

    init_completion(&g_wlan_ctx.done);
    init_completion(&g_interrupt_ctx.done);

    osal_atomic_set(&g_usb->usb_tx_cond, 1);
    g_rx_res_should_release = TD_FALSE;
    usb_set_bus_state(BUS_USB_INIT);
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    oal_usb_log(BUS_LOG_DBG, "usb driver register");
    usb_linux_register(&oal_usb_driver);
    usb_wait_device_ready();
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = usb_register(&oal_usb_driver);
    if (ret) {
        oal_usb_log(BUS_LOG_ERR, "usb_register[%d]", ret);
        hcc_free_bus(g_usb->pst_bus);
        usb_hcc_res_deinit();
        return TD_NULL;
    }
#endif
    oal_usb_log(BUS_LOG_DBG, "oal_usb_bus_alloc sucuess");
    return g_usb->pst_bus;
}

td_void hcc_adapt_usb_unload(td_void)
{
    td_s32 times = USB_RM_DEVICE_TIMES;

    /* 去初始化流程不要做恢复 */
    hcc_set_exception_status(TD_TRUE);
    usb_set_bus_state(BUS_USB_OFF);
    oal_usb_log(BUS_LOG_INFO, "usb unload, status:[%u]", usb_get_bus_state());
    g_rx_res_should_release = TD_TRUE;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    usb_linux_deregister(&oal_usb_driver);
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    usb_deregister(&oal_usb_driver);
#endif
    while (g_usb != TD_NULL &&
        g_usb->connect &&
        times > 0) {
        usleep_range(USB_RM_DEVICE_TIMEOUT, USB_RM_DEVICE_TIMEOUT);
        times--;
    }

    if (g_usb != TD_NULL && g_usb->pst_bus != TD_NULL) {
        g_usb->pst_bus->data = TD_NULL;
        hcc_free_bus(g_usb->pst_bus);
    }
    usb_hcc_res_deinit();
#ifdef CONFIG_HCC_SUPPORT_REG_OPT
    usb_free_reg_info();
#endif
}
#endif // CONFIG_HCC_SUPPORT_USB
