/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: oal usb bus ops source file.
 * Author: Huanghe
 * Create: 2021-07-28
 */
#ifdef CONFIG_HCC_SUPPORT_USB
#define USB_CTRL_MSG_RETRY_MAX 3
#include "hcc_usb_host.h"
#include "osal_list.h"

/* TX urb buf fifo list */
__attribute((aligned (4))) urb_queue g_urb_tx_work_queue = {
    .qlen = 0
};   /* using for urb buf fifo */

__attribute((aligned (4))) urb_buf_stru *g_urb_tx_queue_node[URB_TX_MAX_NUM] = {TD_NULL};

td_u8 g_zero_pkt_aggr_send = TD_FALSE;
static td_u32 g_tx_num = 0;
static td_u32 g_usb_min_trans_size = USB_MIN_PACKAGE_LEN;
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
static td_u32 g_usb_bulk_out_timeout_ms = USB_DOWNLOAD_FW_TIMEOUT;
#endif

static td_void usb_claim_host(usb_host_reg_lock *reg_lock);
static td_void usb_release_host(usb_host_reg_lock *reg_lock);

static td_s32 oal_usb_control_msg(hcc_bus *pst_bus, td_u8 request, td_u8 requesttype, td_u16 value,
    td_u16 index, td_void *data, td_u16 size, td_s32 timeout)
{
    struct hcc_usb* usb = (struct hcc_usb*)pst_bus->data;
    struct usb_dev_info* dev;
    td_void *dmadata = TD_NULL;
    td_s32 ret;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe;
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    struct usb_host_endpoint *uhe;
#endif

    dev = usb_get_intfdata(usb->interface_wlan);
    dmadata = osal_kmalloc(size, OSAL_GFP_KERNEL);
    if (dmadata == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "mem alloc fail");
        return EXT_ERR_FAILURE;
    }

    if (memcpy_s(dmadata, size, data, size) != EOK) {
        osal_kfree(dmadata);
        oal_usb_log(BUS_LOG_ERR, "mem cpy fail");
        return EXT_ERR_FAILURE;
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = (requesttype & USB_DIR_IN) ? usb_rcvctrlpipe(dev->udev, 0) : usb_sndctrlpipe(dev->udev, 0);
    ret = usb_control_msg(dev->udev, pipe, request, requesttype,
        value, index, dmadata, size, timeout);
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    uhe = (requesttype & USB_DIR_IN) ? usb_rcvctrlpipe(dev->udev, 0) : usb_sndctrlpipe(dev->udev, 0);
    ret = usb_control_msg(dev->udev, uhe, request, requesttype,
        value, index, dmadata, size, timeout);
#endif
    if (memcpy_s(data, size, dmadata, size) != EOK) {
        osal_kfree(dmadata);
        oal_usb_log(BUS_LOG_ERR, "get control rsp failed");
        return EXT_ERR_FAILURE;
    }
    osal_kfree(dmadata);
    return ret;
}

static td_s32 oal_usb_control(struct hcc_usb *usb, td_u8 request, td_u8 request_dir,
    td_void *data, td_u16 size)
{
    td_s32 ret = EXT_ERR_FAILURE;
    td_u32 retry = 0;
    osal_mutex_lock(&usb->ctrl_ep_lock);

    while ((ret < 0) && (retry++ < USB_CTRL_MSG_RETRY_MAX)) {
        if (usb_get_bus_state() != BUS_USB_WORK) {
            oal_usb_log(BUS_LOG_ERR, "usb state not work, return!\r\n");
            break;
        }
        ret = oal_usb_control_msg(usb->pst_bus, request, USB_TYPE_CLASS | USB_RECIP_INTERFACE | request_dir,
            0, 0, data, size, USB_CTRL_SET_TIMEOUT);
    }
    if (ret < 0) {
        osal_mutex_unlock(&usb->ctrl_ep_lock);
        return EXT_ERR_FAILURE;
    }
    osal_mutex_unlock(&usb->ctrl_ep_lock);
    return EXT_ERR_SUCCESS;
}

static td_void oal_usb_blocking_completion(struct urb *urb)
{
    struct usb_urb_context *ctx = urb->context;

    if (ctx != TD_NULL) {
        ctx->status = urb->status;
        complete(&ctx->done);
    }
}

static td_s32 oal_usb_start_wait_urb(struct urb *urb, td_s32 timeout, td_s32 *actual_length,
    struct usb_urb_context *ctx)
{
    unsigned long expire;
    td_s32 retval;
    td_s32 wait_ret;

    init_completion(&ctx->done);
    urb->context = ctx;
    urb->actual_length = 0;
    retval = usb_submit_urb(urb, GFP_NOIO);
    if (OAL_UNLIKELY(retval)) {
        oal_usb_log(BUS_LOG_ERR, "usb_submit_urb=%d", retval);
        goto out;
    }

    expire = timeout ? msecs_to_jiffies(timeout) : MAX_SCHEDULE_TIMEOUT;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    wait_ret = wait_for_completion_timeout(&ctx->done, expire);
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wait_ret = wait_for_completion_interruptible_timeout(&ctx->done, expire);
#endif
    if (wait_ret == 0) {
        usb_kill_urb(urb);
        retval = (ctx->status == -ENOENT ? -ETIMEDOUT :ctx->status);
    } else {
        retval = ctx->status;
        if (retval == ERROR_USB_SUSPEND_STATUS) {
            usb_kill_urb(urb);
        }
    }

out:
    if (actual_length) {
        *actual_length = urb->actual_length;
    }

    usb_free_urb(urb);
    return retval;
}

static struct urb *oal_usb_msg_get_buf(td_void **dmadata, td_s32 len)
{
    struct urb *urb = TD_NULL;
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (urb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "usb_alloc_urb");
        return TD_NULL;
    }
    *dmadata = osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (*dmadata == TD_NULL) {
        usb_free_urb(urb);
        oal_usb_log(BUS_LOG_ERR, "msg alloc");
        return TD_NULL;
    }
    return urb;
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
static td_void usb_fill_int_urb(struct urb *urb, struct usb_device *dev,
    struct usb_host_endpoint *uhe, void *buf, int length,
    usb_complete_t callback, void *arg, int interval)
{
    urb->dev = dev;
    urb->endpoint = uhe;
    urb->transfer_buffer = buf;
    urb->transfer_buffer_length = length;
    urb->complete = callback;
    urb->context = arg;

    if (dev->speed == USB_SPEED_HIGH || dev->speed >= USB_SPEED_SUPER) {
        /* make sure interval is within allowed range */
        interval = osal_max(interval, 1);
        interval = osal_min(interval, 16); // 1~16 为USB协议规定范围
        urb->interval = 1 << (interval - 1);
    } else {
        urb->interval = interval;
    }

    urb->start_frame = -1;
}
#endif

td_s32 oal_usb_interrupt_msg(struct usb_device *usb_dev, struct usb_host_endpoint *ep_in,
    td_void *data, td_s32 len, td_s32 *actual_length, td_s32 timeout, struct usb_urb_context *ctx)
{
    struct urb *urb = TD_NULL;
    td_void *dmadata = TD_NULL;
    td_s32 ret;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe = 0;
    pipe = usb_rcvintpipe(usb_dev, ep_in->desc.bEndpointAddress);
#endif

    if (ep_in == TD_NULL || len <= 0) {
        oal_usb_log(BUS_LOG_ERR, "param err=%d", len);
        return EXT_ERR_FAILURE;
    }
    urb = oal_usb_msg_get_buf(&dmadata, len);
    if (urb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "interrupt msg pre alloc err");
        return EXT_ERR_FAILURE;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    usb_fill_int_urb(urb, usb_dev, pipe, dmadata, len,
        oal_usb_blocking_completion, TD_NULL, ep_in->desc.bInterval);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    usb_fill_int_urb(urb, usb_dev, ep_in, dmadata, len,
        oal_usb_blocking_completion, TD_NULL, ep_in->desc.bInterval);
#endif
    ret = oal_usb_start_wait_urb(urb, timeout, actual_length, ctx);
    if (ret == -EPIPE) {
        td_s32 tmp_ret = usb_clear_halt(usb_dev, pipe);
        oal_usb_log(BUS_LOG_ERR, "usb_clear_halt ret=[%d].\r\n", tmp_ret);
    }

    if (memcpy_s(data, len, dmadata, len) != EOK) {
        osal_kfree(dmadata);
        return EXT_ERR_FAILURE;
    }
    osal_kfree(dmadata);
    return ret;
}

td_s32 oal_usb_bulk_in_msg(struct usb_device *usb_dev, struct usb_host_endpoint *bulk_in_hep,
    td_void *data, td_s32 len, td_s32 *actual_length, td_s32 timeout, struct usb_urb_context *ctx)
{
    struct urb *urb;
    td_void *dmadata = TD_NULL;
    td_s32 ret;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe = 0;
#endif

    if (bulk_in_hep == TD_NULL || len <= 0) {
        oal_usb_log(BUS_LOG_ERR, "param err=%d", len);
        return EXT_ERR_FAILURE;
    }

    urb = oal_usb_msg_get_buf(&dmadata, len);
    if (!urb) {
        oal_usb_log(BUS_LOG_ERR, "bulk msg pre alloc err");
        return EXT_ERR_FAILURE;
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = usb_rcvbulkpipe(usb_dev, bulk_in_hep->desc.bEndpointAddress);
    usb_fill_bulk_urb(urb, usb_dev, pipe, dmadata, len,
        oal_usb_blocking_completion, TD_NULL);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    usb_fill_bulk_urb(urb, usb_dev, bulk_in_hep, dmadata, len,
        oal_usb_blocking_completion, TD_NULL);
#endif

    ret = oal_usb_start_wait_urb(urb, timeout, actual_length, ctx);
    if (ret == -EPIPE) {
        td_s32 tmp_ret = usb_clear_halt(usb_dev, pipe);
        oal_usb_log(BUS_LOG_ERR, "usb_clear_halt ret=[%d].\r\n", tmp_ret);
    }

    if (memcpy_s(data, len, dmadata, len) != EOK) {
        osal_kfree(dmadata);
        return EXT_ERR_FAILURE;
    }
    osal_kfree(dmadata);
    return ret;
}

td_void usb_sched_tx_thread(td_void)
{
    struct hcc_usb *usb_tmp = usb_get_handle();
    osal_atomic_set(&usb_tmp->usb_tx_cond, 1);
    osal_wait_wakeup(&usb_tmp->usb_tx_wq);
}

static td_void oal_usb_tx_urb_bulk_submit(struct urb *urb)
{
    errno_t err_num;
    urb_buf_stru *urb_buf = urb->context;
    err_num = memset_s(urb_buf->buf, USB_TX_MAX_SIZE, 0, HIUSB_PACKAGE_HEARDER_SIZE);
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset buf failed:[%d]", err_num);
    }

    urb_list_add_tail(&g_urb_tx_work_queue, urb_buf);
    if (g_urb_tx_work_queue.qlen > URB_TX_HIGH_FIFO) {
        usb_sched_tx_thread();
    }
}

td_s32 usb_thread_condtion(const td_void *data)
{
    osal_atomic *atomic_in = (osal_atomic *)data;

    td_s32 ret = osal_atomic_read(atomic_in);
    if (OAL_LIKELY(ret == 1)) {
        osal_atomic_set(atomic_in, 0);
    }
    return (td_bool)ret;
}
static td_void oal_usb_free_send_urb(struct urb *urb)
{
    usb_free_urb(urb);
}

static td_s32 oal_usb_tx_urb_submit(struct usb_device *usb_dev, struct usb_host_endpoint *ep_out,
    td_void *data, td_s32 len, urb_buf_stru *urb_buf)
{
    td_s32 ret;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe;
#endif

    if (ep_out == TD_NULL || len < 0) {
        return EXT_ERR_FAILURE;
    }
    if (urb_buf->urb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "usb urb is not malloc");
        return EXT_ERR_FAILURE;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = usb_sndbulkpipe(usb_dev, ep_out->desc.bEndpointAddress);
    usb_fill_bulk_urb(urb_buf->urb, usb_dev, pipe, data, len,
        oal_usb_tx_urb_bulk_submit, TD_NULL);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    usb_fill_bulk_urb(urb_buf->urb, usb_dev, ep_out, data, len,
        oal_usb_tx_urb_bulk_submit, TD_NULL);
#endif

    if (g_zero_pkt_aggr_send == TD_TRUE) {
#ifndef CONFIG_HCC_USB_ZLP_FIXME
        g_zero_pkt_aggr_send = TD_FALSE;
        urb_buf->urb->transfer_flags |= URB_ZERO_PACKET;
#endif
    }

    urb_buf->urb->context = urb_buf;
    ret = usb_submit_urb(urb_buf->urb, GFP_ATOMIC);
    if (OAL_UNLIKELY(ret)) {
        oal_usb_log(BUS_LOG_ERR, "usb_submit_urb ret=%d", ret);
        urb_list_add_tail(&g_urb_tx_work_queue, urb_buf);
        usb_kill_urb(urb_buf->urb);
    }

#ifdef CONFIG_HCC_USB_ZLP_FIXME
    if (g_zero_pkt_aggr_send == TD_TRUE) {
        struct urb *urb_zero = usb_alloc_urb(0, GFP_ATOMIC);
        g_zero_pkt_aggr_send = TD_FALSE;
        if (urb_zero != TD_NULL) {
            urb_zero->transfer_flags |= URB_ZERO_PACKET;
            usb_fill_bulk_urb(urb_zero, usb_dev, pipe, NULL, 0,
                oal_usb_free_send_urb, TD_NULL);
            usb_submit_urb(urb_zero, GFP_ATOMIC);
        }
    }
#endif
    return ret;
}

static td_s32 hcc_usb_xfer_write_desc(struct usb_dev_info *dev, usb_package *package,
    urb_buf_stru *urb_buf)
{
    td_s32 len;
    td_s32 ret = EXT_ERR_SUCCESS;

    package->msg_type = USB_HCC_CONTROL_MSG;
    package->len = 0;
    package->reserve = g_tx_num++;

    len = sizeof(usb_package) + HIUSB_HOST2DEV_SCATT_MAX;

    ret = oal_usb_tx_urb_submit(dev->udev, dev->bulk_out_hep, urb_buf->buf, len, urb_buf);
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "write desc ret=%d", ret);
    }
    return ret;
}

static td_u32 hcc_usb_xfer_write_data(struct usb_dev_info *dev, hcc_data_queue *head, td_u32 *sum_len,
    urb_buf_stru *urb_buf)
{
    td_s32 ret;
    td_u32 tx_num = 0;
    td_u32 data_offset = sizeof(usb_package);

    hcc_unc_struc *uncbuf = TD_NULL;
    hcc_unc_struc *tmp = TD_NULL;

    osal_list_for_each_entry_safe(uncbuf, tmp, (&head->data_queue), list) {
        td_u32 actual_len = uncbuf->length;
        td_u32 padding_len = osal_max((td_u32)actual_len, g_usb_min_trans_size); // 每包最小字节数

        if (tx_num == 0) { // 第一包数据为usb package + 控制数据/下一包长度描述区
            if (actual_len > HIUSB_HOST2DEV_SCATT_MAX + sizeof(struct hcc_descr_header)) {
                oal_usb_log(BUS_LOG_ERR, "buf desc overflow:[%d]", actual_len);
                return 0;
            }

            ret = memcpy_s(urb_buf->buf + data_offset, USB_TX_MAX_SIZE - data_offset, uncbuf->buf, actual_len);
            if (ret != EOK) {
                oal_usb_log(BUS_LOG_ERR, "buf_head cpy ret=%d", ret);
                return 0;
            }
        } else {
            // 后面包数据填充到第一包之后
            data_offset = *sum_len;

            if (data_offset + padding_len > USB_TX_MAX_SIZE) {
                oal_usb_log(BUS_LOG_ERR, "buf overflow:[%u]-[%d]", data_offset, padding_len);
                return 0;
            }

            hcc_print_hex_dump((td_u8*)uncbuf->buf, uncbuf->length, "host tx data:");
            // 按真实长度拷贝
            ret = memcpy_s(urb_buf->buf + data_offset, USB_TX_MAX_SIZE - data_offset, uncbuf->buf, actual_len);
            if (ret != EOK) {
                oal_usb_log(BUS_LOG_ERR, "buf cpy failed ret=%d", ret);
                return 0;
            }
        }

        *sum_len += padding_len;
        tx_num++;
    }

    return tx_num;
}

static td_s32 hcc_usb_xfer_write_func(struct usb_dev_info *dev, hcc_data_queue *head)
{
    td_u32 tx_num;
    usb_package *package;
    td_s32 ret;
    td_u32 sum_len = 0;
    urb_buf_stru *urb_buf = TD_NULL;
    struct hcc_usb *usb_tmp = usb_get_handle();

    /* free queue pop */
    if (g_urb_tx_work_queue.qlen <= URB_TX_LOW_FIFO) {
        ret = osal_wait_timeout_interruptible(&usb_tmp->usb_tx_wq, usb_thread_condtion,
            (td_void *)(&usb_tmp->usb_tx_cond), USB_URB_MAX_TIMEOUT);
        if (OAL_UNLIKELY(ret == -ERESTARTSYS)) {
            oal_usb_log(BUS_LOG_ERR, "urb work queue capcity low");
            return EXT_ERR_FAILURE;
        }
        if (OAL_UNLIKELY(ret == 0)) {
            oal_usb_log(BUS_LOG_ERR, "time out");
        }
    }

    if ((g_urb_tx_work_queue.qlen == 0) || (usb_tmp->connect != TD_TRUE)) {
        oal_usb_log(BUS_LOG_ERR, "hcc_usb_xfer_write_func qlen:%d, con:%d", g_urb_tx_work_queue.qlen, usb_tmp->connect);
        return EXT_ERR_FAILURE;
    }

    // 选择一个空闲urb
    urb_buf = urb_list_dequeue(&g_urb_tx_work_queue);
    if (urb_buf == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "urb_buf is not init");
        return EXT_ERR_FAILURE;
    }
    tx_num = hcc_usb_xfer_write_data(dev, head, &sum_len, urb_buf);
    if (tx_num == 0) {
        urb_list_add_tail(&g_urb_tx_work_queue, urb_buf);
        return EXT_ERR_FAILURE;
    }

    package = (usb_package*)urb_buf->buf;
    if (tx_num == HIUSB_H2D_CONTROL_DES) {
        return hcc_usb_xfer_write_desc(dev, package, urb_buf);
    }

    if (tx_num == HIUSB_H2D_SINGLE_NETBUF) {
        package->msg_type = USB_SINGLE_MSG;
    } else {
        package->msg_type = USB_ARRG_GET_DATA;
    }
    // usb驱动每次发送的单包/聚合包的数据总长度
    package->len = sum_len;
    package->reserve = g_tx_num++;
    if (osal_is_aligned(sum_len, dev->bulk_out_max_pkt_size)) {
        g_zero_pkt_aggr_send = TD_TRUE;
    }

    return oal_usb_tx_urb_submit(dev->udev, dev->bulk_out_hep, urb_buf->buf, sum_len, urb_buf);
}

static td_s32 hcc_usb_data_send(hcc_bus *pst_bus, hcc_data_queue *data_head)
{
    td_s32 ret;

    struct hcc_usb *usb = TD_NULL;
    struct usb_dev_info* dev = TD_NULL;

    usb = (struct hcc_usb *)pst_bus->data;
    dev = usb_get_intfdata(usb->interface_wlan);
    if (dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "dev is null");
        return EXT_ERR_FAILURE;
    }

    ret = hcc_usb_xfer_write_func(dev, data_head);
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "Failed to write scatt USB, len:[%d]", hcc_list_len(data_head));
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_usb_send_descr_control_data(hcc_bus *pst_bus, hcc_descr_type descr_type,
    td_void *data, td_u32 size, hcc_trans_queue *queue)
{
    td_s32 ret;
    hcc_unc_struc desc_control;
    hcc_data_queue *head_send = &queue->send_head;
    struct hcc_descr_header *dscr_hdr = TD_NULL;
    td_u8 buffer[HIUSB_HOST2DEV_SCATT_MAX + sizeof(struct hcc_descr_header)] = {0};

    desc_control.buf = buffer;
    desc_control.length = HIUSB_HOST2DEV_SCATT_MAX + sizeof(struct hcc_descr_header);

    dscr_hdr = (struct hcc_descr_header *)(desc_control.buf);
    dscr_hdr->descr_type = descr_type;

    if (size != 0) {
        if (OAL_WARN_ON(data == TD_NULL)) {
            return EXT_ERR_FAILURE;
        }

        if (OAL_WARN_ON(sizeof(struct hcc_descr_header) + size > desc_control.length)) {
            oal_usb_log(BUS_LOG_ERR, "invaild request size:%u, max size:%u\r\n",
                (td_u32)(size + sizeof(struct hcc_descr_header)), (td_u32)(desc_control.length));
            return EXT_ERR_FAILURE;
        }

        if (memcpy_s((td_void *)(desc_control.buf + sizeof(struct hcc_descr_header)),
            desc_control.length - sizeof(struct hcc_descr_header), data, size) != EOK) {
            return EXT_ERR_FAILURE;
        }
    }

    hcc_list_add_tail(head_send, &desc_control);
    ret = hcc_usb_data_send(pst_bus, head_send);
    hcc_list_dequeue(head_send);
    return ret;
}

static td_s32 hcc_usb_single_pkg_prepare(hcc_trans_queue *queue, td_u16 *total_send)
{
    /*
     * SDIO / USB不论是否聚合模式下都使用描述符；
     * IPC / PCIE 单包模式下不使用描述符；
     * 单包模式下：先将数据插入发送队列，如果需要描述符最后将描述符插入队列头；
     */
    hcc_unc_struc *unc_buf = hcc_list_dequeue(&queue->queue_info);
    if (unc_buf == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "unc_buf null\r\n");
        return EXT_ERR_FAILURE;
    }

    hcc_list_add_tail(&queue->send_head, unc_buf);
    /* SDIO单包模式下，若携带描述符，描述符中填0，表示当前为单包发送;
     * device根据上一次的描述符判断当前是单包模式或最后一个聚合包;
     * 在writestart中断中device就能够获取到总包长
     */
    *total_send = 1;
    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_usb_assemble_pkg_prepare(usb_host_assem_info* assemble_info,
    hcc_trans_queue *queue, td_u16 *total_send)
{
    hcc_data_queue *send_head = &queue->send_head;
    /*
     * 聚合模式下：将数据插入发送队列，最后将描述符插入队列头; assembled_head存储下一次聚合的数据;
     * 如果 assembled_head 队列为空，则为第一包，第一包只发送一包数据;
     */
    if (hcc_is_list_empty(&assemble_info->assembled_head)) {
        *total_send = 0;
        hcc_printf("first pkg\r\n");
    } else {
        hcc_list_splice_init(&assemble_info->assembled_head, send_head);
        *total_send = assemble_info->assemble_num;
        hcc_printf("assemble pkg\r\n");
    }

    return EXT_ERR_SUCCESS;
}

static td_void hcc_usb_build_next_assemble_descr(hcc_bus *bus, hcc_trans_queue *queue, td_u16 remain_pkt_nums)
{
    td_s32 i;
    errno_t err_num;
    hcc_unc_struc *unc_buf = TD_NULL;
    struct hcc_usb *usb = (struct hcc_usb *)bus->data;

    td_s32 len = usb->descript_unc.length;
    td_u8 *buf = usb->descript_unc.buf;

    usb_host_assem_info *assem_info = &usb->tx_assemble_info;
    td_u32 *tx_descr_num = &assem_info->assemble_num;
    td_u16 assemble_max_count = osal_min((td_u32)(bus->tx_sched_count), (td_u32)(bus->max_assemble_cnt));
    td_u16 current_trans_len = osal_min(assemble_max_count, remain_pkt_nums);
    td_u32 min_pkt_len = 0;

    err_num = memset_s(buf, len, 0, len);
    if (err_num != EOK) {
        oal_usb_log(BUS_LOG_ERR, "memset buf failed:[%d]", err_num);
    }

    *tx_descr_num = 0;
    if (current_trans_len <= 1) {
        return;
    }

    for (i = 0; i < len && current_trans_len > 0; i++) {
        /* move the buf from head queue to prepare-send queue, head->tail */
        unc_buf = hcc_list_dequeue(&queue->queue_info);
        if (unc_buf == TD_NULL) {
            break;
        }

        current_trans_len--;
        hcc_list_add_tail(&assem_info->assembled_head, unc_buf);
        min_pkt_len = osal_max(unc_buf->length, g_usb_min_trans_size);
        buf[i] = (td_u8)(min_pkt_len >> bus->descr_align_bit);
        (*tx_descr_num)++;
    }
    hcc_printf("assemb count %u", *tx_descr_num);
}

static td_u32 hcc_usb_tx_control_data(hcc_bus *pst_bus, hcc_trans_queue *queue)
{
    static hcc_queue_type last_queue_id = HCC_QUEUE_COUNT;
    td_s32 ret = EXT_ERR_SUCCESS;
        // 如果发送队列发生切换
    if (last_queue_id != queue->queue_id || pst_bus->force_update_queue_id != 0) {
        /*
         * 通知device切换队列并重置聚合信息
         * assemble列表在切换之前已经在hcc层还原
         */
        ret = hcc_usb_send_descr_control_data(pst_bus, HCC_NETBUF_QUEUE_SWITCH,
            &queue->queue_id, sizeof(queue->queue_id), queue);
        if (ret != EXT_ERR_SUCCESS) {
            return ret;
        }
        last_queue_id = queue->queue_id;
        pst_bus->force_update_queue_id = 0;
        ret = hcc_usb_send_descr_control_data(pst_bus, HCC_DESCR_ASSEM_RESET, TD_NULL, 0, queue);
    }
    return ret;
}

/* ****************************************************************************
功能描述  : host侧hcc发送接口(host2device)
**************************************************************************** */
static td_u32 hcc_usb_tx_proc(hcc_bus *pst_bus, hcc_trans_queue *queue, td_u16 *remain_pkt_nums)
{
    td_s32 ret;
    td_u16 remain_pkts = 0;
    td_u16 total_send = 0;
    int err = 0;
    struct hcc_usb *usb = TD_NULL;
    hcc_data_queue *head_send = TD_NULL;

    if ((pst_bus == TD_NULL) || (queue == TD_NULL) || (remain_pkt_nums == TD_NULL)) {
        return EXT_ERR_FAILURE;
    }

    usb = (struct hcc_usb *)pst_bus->data;
    if (OAL_WARN_ON(!usb)) {
        return EXT_ERR_FAILURE;
    }

    if ((usb_get_bus_state() != BUS_USB_WORK) || (usb->connect != TD_TRUE)) {
        oal_usb_log(BUS_LOG_ERR, "usb state[%u]!", usb_get_bus_state());
        return EXT_ERR_FAILURE;
    }
    if (hcc_usb_tx_control_data(pst_bus, queue) != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "hcc_usb_tx_control_data");
        return EXT_ERR_FAILURE;
    }

    if (queue->queue_ctrl->transfer_mode == HCC_SINGLE_MODE || *remain_pkt_nums == 1) {
        ret = hcc_usb_single_pkg_prepare(queue, &total_send);
        if (usb->descript_unc.buf) {
            err = memset_s(usb->descript_unc.buf, usb->descript_unc.length, 0, usb->descript_unc.length);
            if (err != EOK) {
                oal_usb_log(BUS_LOG_ERR, "memset buf failed:[%d]", err);
            }
        }
    } else {
        ret = hcc_usb_assemble_pkg_prepare(&usb->tx_assemble_info, queue, &total_send);
        remain_pkts = (*remain_pkt_nums > total_send) ? (*remain_pkt_nums - total_send) : 0;
        hcc_usb_build_next_assemble_descr(pst_bus, queue, remain_pkts);
        if (total_send == 0) {
            return hcc_usb_send_descr_control_data(pst_bus, HCC_NETBUF_PKT_DESCR,
                usb->descript_unc.buf, usb->descript_unc.length, queue);
        }
    }

    if (ret != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "build usb data failed:[%d]", ret);
        return EXT_ERR_FAILURE;
    }

    head_send = &queue->send_head;
    hcc_list_add_head(head_send, &usb->descript_unc);
    ret = hcc_usb_data_send(pst_bus, head_send);
    hcc_list_dequeue(head_send);

    if (ret == EXT_ERR_SUCCESS) {
        *remain_pkt_nums = (*remain_pkt_nums > total_send) ? (*remain_pkt_nums - total_send) : 0;
        /* 发送成功全部释放 */
        hcc_list_free((hcc_handler *)pst_bus->hcc, head_send);
    } else {
        /* 发送失败全部插回原队列 */
        oal_usb_log(BUS_LOG_ERR, "send usb data failed:[%d]", ret);
        hcc_list_restore(&queue->queue_info, head_send);
    }

    /* 主动收包防止速度较快时device侧积攒buffer过多, 比如UDP轰包场景 */
    if ((total_send == queue->queue_ctrl->low_waterline) || (total_send >= pst_bus->tx_sched_count - 1)) {
        usb_sched_rx_thread();
    }
    return EXT_ERR_SUCCESS;
}

static ext_errno hcc_usb_send_msg(hcc_bus *pst_bus, td_u32 val)
{
    td_s32 ret;
    struct hcc_usb *usb_tmp = usb_get_handle();
    struct hcc_usb *usb = (struct hcc_usb *)pst_bus->data;

    if (OAL_UNLIKELY(!usb)) {
        oal_usb_log(BUS_LOG_ERR, "usb is null");
        return EXT_ERR_FAILURE;
    }

    if (usb->connect != TD_TRUE || osal_atomic_read(&usb_tmp->usb_suspend)) {
        oal_usb_log(BUS_LOG_ERR, "usb disconnect!");
        return EXT_ERR_FAILURE;
    }

    if (OAL_WARN_ON(!usb->interface_wlan)) {
        oal_usb_log(BUS_LOG_ERR, "usb interface is null");
        return EXT_ERR_FAILURE;
    }

    if (val >= H2D_MSG_COUNT) {
        oal_usb_log(BUS_LOG_ERR, "[Error]invaild param[%u]!", val);
        return EXT_ERR_FAILURE;
    }
    ret = oal_usb_control(usb, HOST_TO_DEV_SEND_MSG, USB_DIR_OUT, &val, sizeof(val));
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "Failed to send msg[%d] ret(%d)", val, ret);
        return EXT_ERR_FAILURE;
    }

    return EXT_ERR_SUCCESS;
}

static td_s32 hcc_usb_sleep_request(hcc_bus *pst_bus)
{
    td_s32 ret;
    td_s32 val = ALLOW_TO_SLEEP_VALUE;
    struct hcc_usb *usb = (struct hcc_usb *)pst_bus->data;
    if (OAL_WARN_ON(TD_NULL == usb)) {
        oal_usb_log(BUS_LOG_ERR, "hcc_usb is null");
        return EXT_ERR_FAILURE;
    }

    if (usb->connect != TD_TRUE) {
        oal_usb_log(BUS_LOG_ERR, "usb is disconnect!");
        return EXT_ERR_FAILURE;
    }

    usb_disable_state(pst_bus, HCC_BUS_STATE_ALL);
    // 发送自定义的消息
    ret = oal_usb_control(usb, HOST_TO_DEV_SEND_PM_MSG, USB_DIR_OUT, &val, sizeof(val));
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "Failed to sleep dev (%d)", ret);
    }
    return ret;
}

static td_s32 hcc_usb_wakeup_request(hcc_bus *pst_bus)
{
    td_s32 ret;
    td_s32 val = DISALLOW_TO_SLEEP_VALUE;
    struct hcc_usb *usb = (struct hcc_usb *)pst_bus->data;
    if (OAL_WARN_ON(usb == TD_NULL)) {
        oal_usb_log(BUS_LOG_ERR, "hcc_usb is null");
        return EXT_ERR_FAILURE;
    }

    if (usb->connect != TD_TRUE) {
        oal_usb_log(BUS_LOG_ERR, "usb is disconnect!");
        return EXT_ERR_FAILURE;
    }
    ret = oal_usb_control(usb, HOST_TO_DEV_SEND_PM_MSG, USB_DIR_OUT, &val, sizeof(val));
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "Failed to wakeup dev (%d)", ret);
    }
    return ret;
}

static td_s32 hcc_usb_power_action(hcc_bus *pst_bus, hcc_bus_power_action_type action)
{
    if (action == HCC_BUS_POWER_DOWN) {
        usb_log_enable_set(TD_FALSE);
        usb_task_exit();
        usb_log_enable_set(TD_TRUE);
    }

    return EXT_ERR_SUCCESS;
}

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
td_u32 hcc_usb_bulk_out_timeout_get(td_void)
{
    return g_usb_bulk_out_timeout_ms;
}

td_void hcc_usb_bulk_out_timeout_set(td_u32 timeout_ms)
{
    g_usb_bulk_out_timeout_ms = timeout_ms;
}

static struct usb_dev_info *usb_patch_get_usb_info(hcc_bus *usb_bus)
{
    struct hcc_usb *usb = TD_NULL;

    if (OAL_WARN_ON(usb_bus == TD_NULL)) {
        oal_usb_log(BUS_LOG_ERR, "usb_bus is null");
        return TD_NULL;
    }
    usb = (struct hcc_usb *)usb_bus->data;
    if (OAL_WARN_ON(usb == TD_NULL)) {
        oal_usb_log(BUS_LOG_ERR, "hcc_usb is null");
        return TD_NULL;
    }

    if (usb->connect != TD_TRUE) {
        oal_usb_log(BUS_LOG_ERR, "usb disconnect!");
        return TD_NULL;
    }
    if (usb_get_bus_state() != BUS_USB_BOOT) {
        oal_usb_log(BUS_LOG_ERR, "usb state is not BOOT, state[%u]!", usb_get_bus_state());
        return TD_NULL;
    }
    return usb_get_intfdata(usb->interface_wlan);
}

static td_s32 oal_usb_patch_bulk_in_msg(struct usb_dev_info *dev, td_u8 *buf, td_u32 len, td_u32 timeout)
{
    td_s32 ret;
    td_s32 size;
    td_s32 returnlen;
    td_void *dmadata = TD_NULL;
    struct hcc_usb *usb_tmp = usb_get_handle();
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe = 0;
#endif

    size = osal_min((size_t)len, (size_t)MAX_FIRMWARE_FILE_RX_BUF_LEN);
    dmadata = osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (dmadata == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "alloc fail");
        return EXT_ERR_FAILURE;
    }
    usb_claim_host(&usb_tmp->reg_rw_lock);
    if (memcpy_s(dmadata, len, buf, len) != EOK) {
        osal_kfree(dmadata);
        return EXT_ERR_FAILURE;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep);
    ret = usb_bulk_msg(dev->udev, pipe, dmadata, size, &returnlen, timeout);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    ret = usb_bulk_msg(dev->udev, dev->bulk_in_hep, dmadata, size, &returnlen, timeout);
#endif
    usb_release_host(&usb_tmp->reg_rw_lock);
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "usb_bulk_msg ret=%d", ret);
        osal_kfree(dmadata);
        return EXT_ERR_FAILURE;
    }
    if (memcpy_s(buf, len, dmadata, len) != EOK) {
        osal_kfree(dmadata);
        return EXT_ERR_FAILURE;
    }
    osal_kfree(dmadata);
    return returnlen;
}

static td_s32 hcc_usb_patch_read(hcc_bus *usb_bus, td_u8 *buf, td_u32 len, td_u32 timeout)
{
    struct usb_dev_info *dev = TD_NULL;

    dev = usb_patch_get_usb_info(usb_bus);
    if (dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "get dev info fail");
        return EXT_ERR_FAILURE;
    }

    return oal_usb_patch_bulk_in_msg(dev, buf, len, timeout);
}

static td_s32 oal_usb_bulk_out_msg(struct usb_dev_info *dev, td_u8 *buffer, td_u32 len, td_u32 timeout)
{
    td_s32 ret = 0;
    td_s32 size;
    td_s32 returnlen;
    struct hcc_usb *usb_tmp = usb_get_handle();
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_u32 pipe = 0;
#endif

    if (dev->bulk_out_buffer == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "bulk out NULL");
        return EXT_ERR_FAILURE;
    }

    usb_claim_host(&usb_tmp->reg_rw_lock);
    size = osal_min((size_t)len, (size_t)dev->buik_out_buffer_size);
    ret = memcpy_s(dev->bulk_out_buffer, dev->buik_out_buffer_size, buffer, size);
    if (ret != EOK) {
        usb_release_host(&usb_tmp->reg_rw_lock);
        oal_usb_log(BUS_LOG_ERR, "memcpy failed:[%d]", ret);
        return EXT_ERR_FAILURE;
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pipe = usb_sndbulkpipe(dev->udev, dev->bulk_out_ep);
    ret = usb_bulk_msg(dev->udev, pipe, dev->bulk_out_buffer, size, &returnlen, timeout);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    ret = usb_bulk_msg(dev->udev, dev->bulk_out_hep, dev->bulk_out_buffer, size, &returnlen, timeout);
#endif
    usb_release_host(&usb_tmp->reg_rw_lock);
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, " usb_bulk_msg ret=%d", ret);
        return  ret;
    }
    return returnlen;
}

static td_s32 hcc_usb_patch_write(hcc_bus *usb_bus, td_u8 *buf, td_u32 len)
{
    struct usb_dev_info *dev = TD_NULL;

    dev = usb_patch_get_usb_info(usb_bus);
    if (dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "get dev info fail");
        return EXT_ERR_FAILURE;
    }
    return oal_usb_bulk_out_msg(dev, buf, len, g_usb_bulk_out_timeout_ms);
}
#endif

#ifdef CONFIG_HCC_SUPPORT_REG_OPT
static td_s32 usb_reg_opt_check_param(td_u32 addr)
{
    hcc_bus *pst_bus;
    struct hcc_usb *usb_tmp = usb_get_handle();

    if (osal_is_aligned(addr, RW_REG_ADDR_ALIGN_LEN) == TD_FALSE) {
        oal_usb_log(BUS_LOG_ERR, "addr:[0x%x] is not aligned", addr);
        return EXT_ERR_FAILURE;
    }
    if (usb_tmp == TD_NULL || usb_tmp->connect != TD_TRUE) {
        return EXT_ERR_FAILURE;
    }
    pst_bus = usb_tmp->pst_bus;
    if (pst_bus == TD_NULL || pst_bus->state == 0) {
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : usb寄存器读写完成回调
**************************************************************************** */
static td_s32 oal_usb_dev_urb_init(struct urb **send_urb, struct usb_dev_info **dev)
{
    *dev = usb_get_intfdata(usb_get_handle()->interface_wlan);
    if (*dev == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "dev is null");
        return EXT_ERR_FAILURE;
    }

    *send_urb = usb_alloc_urb(0, GFP_NOIO);
    if (*send_urb == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "alloc urb failed");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
/* ****************************************************************************
功能描述  : usb寄存器bulk读写
**************************************************************************** */
td_s32 oal_usb_rw_reg_send(td_void *data, td_s32 len, td_bool b_bulk_out, td_u32 time_ms)
{
    td_s32 result;
    struct usb_host_endpoint *ep = TD_NULL;
    struct usb_urb_context urb_ctx;
    struct urb *send_urb = TD_NULL;
    struct usb_dev_info *dev = TD_NULL;

    if (oal_usb_dev_urb_init(&send_urb, &dev) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }

    if (b_bulk_out) {
        ep = dev->reg_out_hep;
    } else {
        ep = dev->reg_in_hep;
    }

    if (time_ms) {
        usb_fill_bulk_urb(send_urb, dev->udev, ep, data, len,
            oal_usb_blocking_completion, TD_NULL);
        result = oal_usb_start_wait_urb(send_urb, time_ms, &len, &urb_ctx);
    } else {
        usb_fill_bulk_urb(send_urb, dev->udev, ep, data, len,
            oal_usb_free_send_urb, TD_NULL);
        result = usb_submit_urb(send_urb, GFP_ATOMIC);
    }

    if (OAL_UNLIKELY(result)) {
        oal_usb_log(BUS_LOG_ERR, "urb submit failed:[%d]", result);
        return EXT_ERR_FAILURE;
    }

    return EXT_ERR_SUCCESS;
}
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* ****************************************************************************
功能描述  : usb寄存器bulk读写
**************************************************************************** */
td_s32 oal_usb_rw_reg_send(td_void *data, td_s32 len, td_bool b_bulk_out, td_u32 time_ms)
{
    td_s32 ret;
    td_u32 pipe = 0;
    struct urb *send_urb = TD_NULL;
    struct usb_urb_context urb_ctx;
    struct usb_dev_info *dev = TD_NULL;

    if (oal_usb_dev_urb_init(&send_urb, &dev) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }

    if (b_bulk_out) {
        pipe = usb_sndbulkpipe(dev->udev, dev->rw_reg_bulk_out_ep);
    } else {
        pipe = usb_rcvbulkpipe(dev->udev, dev->rw_reg_bulk_in_ep);
    }

    if (time_ms) {
        usb_fill_bulk_urb(send_urb, dev->udev, pipe, data, len,
            oal_usb_blocking_completion, TD_NULL);
        ret = oal_usb_start_wait_urb(send_urb, time_ms, &len, &urb_ctx);
    } else {
        usb_fill_bulk_urb(send_urb, dev->udev, pipe, data, len,
            oal_usb_free_send_urb, TD_NULL);
        ret = usb_submit_urb(send_urb, GFP_ATOMIC);
    }

    if (OAL_UNLIKELY(ret)) {
        oal_usb_log(BUS_LOG_ERR, "urb submit failed:[%d]", ret);
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}
#endif

static td_u32 *g_reg_data = TD_NULL;
static rw_reg_setup_bytes *g_setup_info = TD_NULL;

td_u32 usb_reg_info_init(td_void)
{
    g_setup_info = osal_kmalloc(sizeof(rw_reg_setup_bytes), OSAL_GFP_KERNEL);
    if (g_setup_info == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    g_reg_data = osal_kmalloc(sizeof(td_u32), OSAL_GFP_KERNEL);
    if (g_reg_data == TD_NULL) {
        osal_kfree(g_setup_info);
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

td_void usb_free_reg_info(td_void)
{
    if (g_setup_info != TD_NULL) {
        osal_kfree(g_setup_info);
    }
    if (g_reg_data != TD_NULL) {
        osal_kfree(g_reg_data);
    }
}

/* ****************************************************************************
功能描述  : 寄存器读写, 发送的setup消息
**************************************************************************** */
static td_s32 usb_rw_reg_send_setup(td_u32 addr, td_bool access_dir)
{
    td_s32 result;

    g_setup_info->reg_off = addr;
    g_setup_info->dir = access_dir;
    g_setup_info->reg_len = RW_REG_ADDR_LEN;
    g_setup_info->cur_stage = RW_REG_STAGE_SETUP;

    result = oal_usb_rw_reg_send(g_setup_info, sizeof(*g_setup_info), TD_TRUE, 0);
    if (result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "send setup failed:[%d]", result);
    }

    return result;
}

/* ****************************************************************************
功能描述  : 发送bulk out, 设置device寄存器的值
          此函数会负责最后的完成等待
**************************************************************************** */
static td_s32 usb_rw_reg_write_data(td_u32 data)
{
    td_s32 result;

    *g_reg_data = data;
    result = oal_usb_rw_reg_send(g_reg_data, sizeof(*g_reg_data), TD_TRUE, RW_REG_TIMEOUT_MS);
    if (result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "send data failed:[%d]", result);
        return EXT_ERR_FAILURE;
    }

    return EXT_ERR_SUCCESS;
}

/* ****************************************************************************
功能描述  : 发送bulk in请求,读取device寄存器的值
          此函数会负责最后的完成等待
**************************************************************************** */
static td_s32 usb_rw_reg_get_data_req(td_u32 *value)
{
    td_s32 result;

    *g_reg_data = 0;
    result = oal_usb_rw_reg_send(g_reg_data, sizeof(*g_reg_data), TD_FALSE, RW_REG_TIMEOUT_MS);
    if (result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "send data failed:[%d]", result);
        return result;
    }
    *value = *g_reg_data;
    return EXT_ERR_SUCCESS;
}

static td_s32 usb_write_reg_for_fw(td_u32 addr, td_u32 value)
{
    td_s32 rw_result;
    rw_result = usb_rw_reg_send_setup(addr, RW_REG_DIR_WRITE);
    if (rw_result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "write setup addr:[0x%x] failed:[%d]", addr, rw_result);
        return rw_result;
    }

    rw_result = usb_rw_reg_write_data(value);
    if (rw_result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "write addr:[0x%x] failed:[%d]", addr, rw_result);
        return rw_result;
    }
    return EXT_ERR_SUCCESS;
}

static td_s32 usb_read_reg_for_fw(td_u32 addr, td_u32 *value)
{
    td_s32 rw_result;
    rw_result = usb_rw_reg_send_setup(addr, RW_REG_DIR_READ);
    if (rw_result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "read setup addr:[0x%x] failed:[%d]", addr, rw_result);
        return rw_result;
    }

    rw_result = usb_rw_reg_get_data_req(value);
    if (rw_result != EXT_ERR_SUCCESS) {
        oal_usb_log(BUS_LOG_ERR, "read addr:[0x%x] failed:[%d]", addr, rw_result);
        return rw_result;
    }

    return EXT_ERR_SUCCESS;
}

static td_void usb_claim_host(usb_host_reg_lock *reg_lock)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    DECLARE_WAITQUEUE(wait, current);
    unsigned long flags;

    add_wait_queue(&reg_lock->wq, &wait);
    osal_spin_lock_irqsave(&reg_lock->lock, &flags);
    while (1) {
        set_current_state(TASK_UNINTERRUPTIBLE);
        if (!reg_lock->claimed || reg_lock->claimer == current)
            break;
        osal_spin_unlock_irqrestore(&reg_lock->lock, &flags);
        schedule();
        osal_spin_lock_irqsave(&reg_lock->lock, &flags);
    }
    set_current_state(TASK_RUNNING);
    reg_lock->claimed = 1;
    reg_lock->claimer = current;
    reg_lock->claim_cnt += 1;
    osal_spin_unlock_irqrestore(&reg_lock->lock, &flags);
    remove_wait_queue(&reg_lock->wq, &wait);
#else
    osal_mutex_lock(&reg_lock->lock);
#endif
}

static td_void usb_release_host(usb_host_reg_lock *reg_lock)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    unsigned long flags;
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 1))
    struct wait_queue_entry *next;
#else
    wait_queue_t *next;
#endif
    struct list_head *next_task_list;

    osal_spin_lock_irqsave(&reg_lock->lock, &flags);
    if (--reg_lock->claim_cnt) {
        osal_spin_unlock_irqrestore(&reg_lock->lock, &flags);
    } else {
        reg_lock->claimed = 0;
        reg_lock->claimer = TD_NULL;
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 1))
        if (!list_empty(&reg_lock->wq.head)) {
            next_task_list = reg_lock->wq.head.prev;
            next = osal_container_of(next_task_list, struct wait_queue_entry, entry);
#else
        if (!list_empty(&reg_lock->wq.task_list)) {
            next_task_list = reg_lock->wq.task_list.prev;
            next = osal_container_of(next_task_list, wait_queue_t, task_list);
#endif
            reg_lock->claimed = 1;
            reg_lock->claimer = next->private;
        }
        osal_spin_unlock_irqrestore(&reg_lock->lock, &flags);
        wake_up(&reg_lock->wq);
    }
#else
    osal_mutex_unlock(&reg_lock->lock);
#endif
}
/* ****************************************************************************
功能描述  : 写device 寄存器的值
**************************************************************************** */
static td_s32 hcc_usb_write_reg(hcc_bus *pst_bus, td_u32 addr, td_u32 value)
{
    td_s32 result = EXT_ERR_FAILURE;
    td_u32 index;
    struct hcc_usb *usb_tmp = usb_get_handle();

    if (pst_bus == TD_NULL || usb_tmp == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    if (usb_reg_opt_check_param(addr) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }

    usb_claim_host(&usb_tmp->reg_rw_lock);
    for (index = 0; index < RW_REG_RETRY_TIMES; ++index) {
        if (usb_get_bus_state() == BUS_USB_BOOT) {
            // boot writem
            result = write_device_reg(addr, value);
        } else if (usb_get_bus_state() == BUS_USB_WORK) {
            // fw鐩村啓
            result = usb_write_reg_for_fw(addr, value);
        }
        if (result == EXT_ERR_SUCCESS) {
            break;
        }
    }
    usb_release_host(&usb_tmp->reg_rw_lock);
    return result;
}

/* ****************************************************************************
功能描述  : 读device 寄存器的值
**************************************************************************** */
static td_s32 hcc_usb_read_reg(hcc_bus *pst_bus, td_u32 addr, td_u32 *value)
{
    td_u32 index;
    td_s32 result = EXT_ERR_FAILURE;
    struct hcc_usb *usb_tmp = usb_get_handle();

    if (pst_bus == TD_NULL || value == TD_NULL || usb_tmp == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "invalid param");
        return EXT_ERR_FAILURE;
    }
    if (usb_reg_opt_check_param(addr) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }

    usb_claim_host(&usb_tmp->reg_rw_lock);
    for (index = 0; index < RW_REG_RETRY_TIMES; ++index) {
        if (usb_get_bus_state() == BUS_USB_BOOT) {
            // boot readm
            result = read_device_reg(addr, value);
        } else if (usb_get_bus_state() == BUS_USB_WORK) {
            // fw直写
            result = usb_read_reg_for_fw(addr, value);
        }
        if (result == EXT_ERR_SUCCESS) {
            break;
        }
    }
    usb_release_host(&usb_tmp->reg_rw_lock);
    return result;
}
#endif

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
static td_s32 hcc_usb_get_credit(hcc_bus *pst_bus, td_u32 *free_cnt)
{
    td_s32 ret;
    struct hcc_usb *usb = TD_NULL;

    if ((pst_bus == TD_NULL) || (free_cnt == TD_NULL)) {
        oal_usb_log(BUS_LOG_ERR, "invalid param");
        return EXT_ERR_FAILURE;
    }

    usb = (struct hcc_usb *)pst_bus->data;
    if (OAL_WARN_ON(!usb)) {
        oal_usb_log(BUS_LOG_ERR, "usb is null");
        return EXT_ERR_FAILURE;
    }

    // 发送控制传输获取高优先级内存值
    ret = oal_usb_control(usb, HOST_TO_DEV_GET_CREDIT_TYPE, USB_DIR_IN, free_cnt, sizeof(td_u32));
    if (ret < 0) {
        oal_usb_log(BUS_LOG_ERR, "Failed to get credit (%d)", ret);
    }

    return ret;
}
#endif

td_s32 hcc_usb_pm_notify_register(hcc_bus *pst_bus, bus_pm_notify suspend_func, bus_pm_notify resume_func)
{
    struct hcc_usb *usb_priv = (struct hcc_usb *)pst_bus->data;
    if (usb_priv == TD_NULL) {
        oal_usb_log(BUS_LOG_ERR, "usb_priv null noty\n");
        return EXT_ERR_FAILURE;
    }
    usb_priv->suspend_notify_func = suspend_func;
    usb_priv->resume_notify_func = resume_func;
    return EXT_ERR_SUCCESS;
}

static td_bool hcc_usb_is_busy(hcc_queue_dir dir)
{
    unsigned long flags;
    td_bool is_busy = TD_FALSE;

    if (dir == HCC_DIR_TX) {
        osal_spin_lock_irqsave(&g_urb_tx_work_queue.data_queue_lock, &flags);
        is_busy = (g_urb_tx_work_queue.qlen < URB_TX_MAX_NUM) ? TD_TRUE : TD_FALSE;
        osal_spin_unlock_irqrestore(&g_urb_tx_work_queue.data_queue_lock, &flags);
    }

    return is_busy;
}

static bus_dev_ops g_usb_opt_ops = {
    .tx_proc            = hcc_usb_tx_proc,
    .is_busy            = hcc_usb_is_busy,
    .send_and_clear_msg = hcc_usb_send_msg,
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    .update_credit      = TD_NULL,
    .get_credit         = hcc_usb_get_credit,      /* 获取device内存反压值 */
#endif
#ifdef CONFIG_HCC_SUPPORT_REG_OPT
    .read_reg = hcc_usb_read_reg,
    .write_reg = hcc_usb_write_reg,
#endif
    .reinit             = hcc_usb_reinit,
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
    .patch_read         = hcc_usb_patch_read,
    .patch_write        = hcc_usb_patch_write,
#endif
    .power_action       = hcc_usb_power_action,
    .sleep_request      = hcc_usb_sleep_request,
    .wakeup_request     = hcc_usb_wakeup_request,
    .pm_notify_register = hcc_usb_pm_notify_register,
};

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*  linux 伙伴算法是依靠2的N次幂分配内存，为节约内存，采用以下分配方式
    TX: 申请3次，   1次64KB，  需要保持8个20
	64  =  20 +  20 +  20  + 4(多余)
	64  =  20 +  20 +  20  + 4(多余)
	64  =  20 +  20 +  20(多余)  + 4(多余)

    RX 申请1次 64KB    RX保持 3个20
	64  =  20 +  20 +  20  + 4(多余)
    */
#define USB_TRX_MAX_MALLOC_SIZE  (64 * 1024)
#define USB_TRX_MALLOC_GROUP      3
osal_void usb_trx_urb_list_malloc_memory_opt(urb_buf_stru *urb_trx_queue_node[], osal_u32 i)
{
    td_u32 start_index = 0;
    if (i % USB_TRX_MALLOC_GROUP == 0) {
        urb_trx_queue_node[i]->buf = osal_kmalloc(USB_TRX_MAX_MALLOC_SIZE, OSAL_GFP_KERNEL);
        if (urb_trx_queue_node[i]->buf == TD_NULL) {
            return;
        }
    } else {
        // i 为 0 1 2， start_index: 0，  i为 3 4 5， start_index: 3,  i为 6 7，  start_index: 6
        start_index = (i / USB_TRX_MALLOC_GROUP) * USB_TRX_MALLOC_GROUP;
        if (urb_trx_queue_node[start_index]->buf == TD_NULL) {
            urb_trx_queue_node[i]->buf = TD_NULL;
            return;
        }
        urb_trx_queue_node[i]->buf = urb_trx_queue_node[start_index]->buf +
            (i % USB_TRX_MALLOC_GROUP) * USB_TX_MAX_SIZE;
    }
}

osal_void usb_trx_urb_list_free_memory_opt(urb_buf_stru *urb_trx_queue_node[], osal_u32 i)
{
    if (i % USB_TRX_MALLOC_GROUP == 0) {
        osal_kfree(urb_trx_queue_node[i]->buf);
        return;
    }
}
#endif

td_void usb_tx_urb_list_deinit(td_void)
{
    td_u32 i;
    for (i = 0; i < URB_TX_MAX_NUM; i++) {
        /* release urb resource whether urb is in the usb core or not */
        if (g_urb_tx_queue_node[i] != TD_NULL) {
            if (g_urb_tx_queue_node[i]->urb != TD_NULL) {
                usb_kill_urb(g_urb_tx_queue_node[i]->urb);
                usb_free_urb(g_urb_tx_queue_node[i]->urb);
                g_urb_tx_queue_node[i]->urb = TD_NULL;
            }

            if (g_urb_tx_queue_node[i]->buf != TD_NULL) {
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                usb_trx_urb_list_free_memory_opt(g_urb_tx_queue_node, i);
#else
                osal_kfree(g_urb_tx_queue_node[i]->buf);
#endif
                g_urb_tx_queue_node[i]->buf = TD_NULL;
            }
            osal_kfree(g_urb_tx_queue_node[i]);
            g_urb_tx_queue_node[i] = TD_NULL;
        }
    }
    osal_spin_lock_destroy(&g_urb_tx_work_queue.data_queue_lock);
}

td_s32 usb_tx_urb_list_init(td_void)
{
    td_u32 i;

    if (urb_list_len(&g_urb_tx_work_queue) > 0) {
        return EXT_ERR_SUCCESS;
    }

    /* init queue */
    urb_list_head_init(&g_urb_tx_work_queue);
    for (i = 0; i < URB_TX_MAX_NUM; i++) {
        g_urb_tx_queue_node[i] = osal_kmalloc(sizeof(urb_buf_stru), OSAL_GFP_KERNEL);
        if (g_urb_tx_queue_node[i] == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "node[%d] malloc failed", i);
            return EXT_ERR_FAILURE;
        }

        /* malloc urb buf */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        usb_trx_urb_list_malloc_memory_opt(g_urb_tx_queue_node, i);
#else
        g_urb_tx_queue_node[i]->buf = osal_kmalloc(USB_TX_MAX_SIZE, OSAL_GFP_KERNEL);
#endif
        if (g_urb_tx_queue_node[i]->buf == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "buf[%d] malloc failed", i);
            return EXT_ERR_FAILURE;
        }

        /* malloc urb */
        g_urb_tx_queue_node[i]->urb = usb_alloc_urb(0, GFP_ATOMIC);
        if (g_urb_tx_queue_node[i]->urb == TD_NULL) {
            oal_usb_log(BUS_LOG_ERR, "urb[%d] malloc failed", i);
            return EXT_ERR_FAILURE;
        }
        urb_list_add_tail(&g_urb_tx_work_queue, g_urb_tx_queue_node[i]);
    }

    return EXT_ERR_SUCCESS;
}

td_void usb_tx_urb_list_reset(td_void)
{
    td_u32 i;
    td_u32 mask = 0;
    urb_buf_stru *tmp;

    while (g_urb_tx_work_queue.qlen > 0) {
        tmp = urb_list_dequeue(&g_urb_tx_work_queue);
        for (i = 0; i < URB_TX_MAX_NUM; i++) {
            if (tmp == g_urb_tx_queue_node[i]) {
                mask |= (1 << i);
            }
        }
    }

    for (i = 0; i < URB_TX_MAX_NUM; i++) {
        if ((mask & (1 << i)) == 0) {
            usb_kill_urb(g_urb_tx_queue_node[i]->urb);
        }
        urb_list_add_tail(&g_urb_tx_work_queue, g_urb_tx_queue_node[i]);
    }
    usb_sched_tx_thread();
}

#define USB_HOST2DEV_SCATT_MAX 12
hcc_bus *usb_bus_init(struct hcc_usb *usb)
{
    hcc_bus *pst_bus = TD_NULL;

    pst_bus = hcc_alloc_bus();
    if (TD_NULL == pst_bus) {
        oal_usb_log(BUS_LOG_ERR, "alloc usb_bus:%u", (td_u32)sizeof(hcc_bus));
        return TD_NULL;
    }

    pst_bus->bus_type = HCC_BUS_USB;
    pst_bus->bus_ops  = &g_usb_opt_ops;
    pst_bus->cap_max_trans_size = 0x7fffffff;
    pst_bus->data = (td_void*)usb;
    pst_bus->state = 0;

    pst_bus->max_trans_size = USB_HOST2DEV_PKTS_MAX_LEN;
    pst_bus->addr_align = HCC_ADAPT_BUS_ADDR_ALIGN_MIN;
    pst_bus->len_align = HIUSB_H2D_DATA_LEN_ALIGN;
    pst_bus->max_assemble_cnt = USB_HOST2DEV_SCATT_MAX;
    pst_bus->descr_align_bit = HIUSB_H2D_SCATT_BUFFLEN_ALIGN_BITS;
    pst_bus->tx_sched_count = HIUSB_HOST2DEV_SCATT_MAX;

    usb->pst_bus = pst_bus;
    return pst_bus;
}
#endif
