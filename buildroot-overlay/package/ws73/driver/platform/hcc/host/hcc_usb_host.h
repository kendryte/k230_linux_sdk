/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: oal usb host adapt header file.
 * Author: Huanghe
 * Create: 2021-07-28
 */

#ifndef _HCC_USB_HOST_H
#define _HCC_USB_HOST_H

#ifdef CONFIG_HCC_SUPPORT_USB
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/wait.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <uapi/linux/sched/types.h>
#endif

#include "td_base.h"
#include "td_type.h"
#include "soc_osal.h"
#include "hcc_bus_types.h"
#include "hcc_bus_usb_comm.h"
#include "hcc.h"
#include "hcc_if.h"
#include "hcc_list.h"
#include "hcc_bus.h"
#include "plat_firmware.h"
#include "securec.h"
#include "oal_hcc_bus_os_adapt.h"

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kref.h>
#include <linux/usb.h>
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include <linux/completion.h>
#include "linux_usb.h"
#include "usb_core.h"
#include "usb_busdma.h"
#include "usb_transfer.h"
#include "usb_device.h"
#include "usbdi.h"
#endif

#define HIUSB_H2D_SCATT_BUFFLEN_ALIGN_BITS 3
#define HIUSB_H2D_SINGLE_NETBUF 2
#define HIUSB_H2D_CONTROL_DES 1
#define HIUSB_H2D_DATA_LEN_ALIGN 8

#define DEVICE_KERNEL_EP_NUM 5
#define DEVICE_BOOT_EP_NUM   2
#define BULK_EP_IN_IND 0
#define BULK_EP_OUT_IND  1
#define INT_EP_IN_IND  2
#define RW_REG_EP_OUT_IND 3
#define RW_REG_EP_IN_IND 4

#define RW_REG_DIR_READ 0
#define RW_REG_DIR_WRITE 1
#define RW_REG_RETRY_TIMES 3
#define RW_REG_TIMEOUT_MS 2000
#define RW_REG_STAGE_SETUP 0x01
#define RW_REG_STAGE_DATA 0x02
#define RW_REG_ADDR_LEN 4
#define RW_REG_ADDR_ALIGN_LEN 4

#define USB_WAIT_TIME 5

#define ERROR_USB_SUSPEND_STATUS (-1)
#define USB_SUS_NORMAL 0x0
#define USB_SUS_SUSPEND 0x1
#define USB_SUS_DISCONNECT 0x2

#define URB_RX_MAX_NUM 3
#define USB_RX_MAX_SIZE (20 * 1024)
#define USB_TX_MAX_SIZE (20 * 1024)
#define USB_URB_MAX_TIMEOUT 10000
#define URB_TX_LOW_FIFO 2
#define URB_TX_HIGH_FIFO 6
#define URB_TX_MAX_NUM 8

#define ALLOW_TO_SLEEP_VALUE 1
#define DISALLOW_TO_SLEEP_VALUE 0
#define INT_TRANS_MAX_PACKAGE_SIZE            8

#define HCC_TRANS_THERAD_PRIORITY 10
#define USB_TASK_STACK_SIZE  8192
#define USB_THREDD_PRI 99
#define USB_THREDD_DATA_PRI 99
#define USB_DOWNLOAD_FW_TIMEOUT 30000

#define USB_ENUM_DEVICE_TIMEOUT 100000
#define USB_RM_DEVICE_TIMEOUT   1000
#define USB_RM_DEVICE_TIMES     100
#define USB_TASK_DEINIT_TIMES   1000

#define H2D_DESC_BUF_SZIE 0x40
#define USB_MIN_PACKAGE_LEN 0x40
#define USB_HOST2DEV_PKTS_MAX_LEN 1680

#define USB_MODULE_NAME "[USB]"

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef struct usb_host_reg_lock_t {
    td_u32 claimed;
    td_u32 claim_cnt;
    osal_spinlock lock;
    wait_queue_head_t wq;
    struct task_struct *claimer;
} usb_host_reg_lock;
#ifdef CONTROLLER_CUSTOMIZATION
#define oal_usb_log(loglevel, fmt, arg...) \
        do { \
            if ((loglevel <= BUS_LOG_INFO) && (usb_log_enable_get() != TD_FALSE)) { \
                printk(KERN_EMERG "%s%s" fmt"[%s:%d]\r\n", USB_MODULE_NAME, g_loglevel[loglevel], ##arg, __FUNCTION__, \
                __LINE__); \
            } \
        } while (0)
#else
#define oal_usb_log(loglevel, fmt, arg...) \
        do { \
            if ((loglevel <= BUS_LOG_INFO) && (usb_log_enable_get() != TD_FALSE)) { \
                printk("%s%s" fmt"[%s:%d]\r\n", USB_MODULE_NAME, g_loglevel[loglevel], ##arg, __FUNCTION__, __LINE__); \
            } \
        } while (0)
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef struct _usb_host_reg_lock_t {
    osal_mutex lock;
} usb_host_reg_lock;

#define oal_usb_log(loglevel, fmt, arg...) \
        do { \
            if ((loglevel <= BUS_LOG_INFO) && (usb_log_enable_get() != TD_FALSE)) { \
                printf("%s%s" fmt"[%s:%d]\r\n", USB_MODULE_NAME, g_loglevel[loglevel], ##arg, __FUNCTION__, __LINE__); \
            } \
        } while (0)

#define MAX_SCHEDULE_TIMEOUT    LONG_MAX
#define USB_CTRL_SET_TIMEOUT    5000
#define GFP_NOIO                0
#define EP_SETUP_BUF_LEN        512

static inline void reinit_completion(struct completion *x)
{
    x->uwComCount = 0;
}
#endif

typedef enum {
    HCC_USB_SPEED_HIGH,
    HCC_USB_SPEED_FULL,
    HCC_USB_SPEED_LOW,
    HCC_USB_SPEED_FULL_48M,
    HCC_USB_SPEED_SUPER = 4,
} hcc_usb_device_speed;

typedef enum {
    USB_ISOC_RX_RUNNING,
    USB_ISOC_TX_RUNNING,
    USB_ISOC_RX_BUSY,
    USB_ISOC_TX_BUSY,
} usb_isoc_status;

typedef enum _hcc_descr_type {
    HCC_DESCR_ASSEM_RESET = 0,
    HCC_NETBUF_QUEUE_SWITCH = 1,
    HCC_NETBUF_PKT_DESCR = 4,
    HCC_DESCR_TYPE_BUTT
} hcc_descr_type;

/* This descr buff is reversed in device,
  the callback function can't cost much time,
  just for transfer sdio buff message */
struct hcc_descr_header {
    td_u32 descr_type;
};

typedef struct {
    td_u32 reserved1;
    td_u32 reserved2;

    td_u32 reg_off : 16;
    td_u32 reserved3 : 16;

    td_u32 dir : 1;
    td_u32 reg_len : 11;
    td_u32 reserved4 : 18;
    td_u32 cur_stage : 2;
} rw_reg_setup_bytes;

typedef struct {
    td_u32 data;
} rw_reg_data_bytes;

struct usb_dev_info {
    struct usb_device *udev;         /* the usb device for this device */
    struct usb_interface *interface; /* the interface for this device */
    td_u8 bulk_in_ep;
    td_u8 bulk_out_ep;
    td_u8 int_in_ep;
    td_u8 rw_reg_bulk_in_ep; /* bulk in ep for read/write reg */
    td_u8 rw_reg_bulk_out_ep; /* bulk out ep for read/write reg */
    td_u8 *bulk_in_buffer;
    td_u32 buik_in_buffer_size;
    td_u32 bulk_in_max_pkt_size;
    td_u8 *bulk_out_buffer;
    td_u32 buik_out_buffer_size;
    td_u32 bulk_out_max_pkt_size;
    td_u8 *int_in_buffer;
    td_u32 int_in_buffer_size;
    struct usb_host_endpoint *bulk_in_hep;
    struct usb_host_endpoint *bulk_out_hep;
    struct usb_host_endpoint *reg_in_hep;
    struct usb_host_endpoint *reg_out_hep;
    struct usb_host_endpoint *int_in_hep;
    struct kref kref;
    osal_mutex io_mutex; /* synchronize I/O with disconnect */
};

typedef enum {
    HOST_TO_DEV_SEND_MSG,
    HOST_TO_DEV_RESPONES_MSG,
    HOST_TO_DEV_SEND_PM_MSG,
    HOST_TO_DEV_RESPONES_PM_MSG,
    HOST_TO_DEV_AUDIO_MSG_TYPE,
    HOST_TO_DEV_GET_CREDIT_TYPE,
    HOST_TO_DEV_GET_FLOW_CTR_TYPE,
} usb_send_msg_type;

typedef enum {
    USB_SINGLE_MSG,
    USB_ARRG_GET_DATA,
    USB_ARRG_DEV_HOST_DATA,
    USB_HCC_CONTROL_MSG,
    USB_MSG_INFO_TYPE,
    USB_BT_DATA_START_TX_TYPE,
    USB_BT_DATA_TRANSFER_TX_TYPE,
    USB_BT_DATA_RX_TYPE
} usb_msg_type;

typedef enum {
    USB_ISOC_RX_START,
    USB_ISOC_RX_STOP,
    USB_ISOC_TX_START,
    USB_ISOC_TX_STOP,
    USB_UNKNOWN_MSG,
} usb_isoc_ctr_msg_type;

typedef enum {
    BT_TYPE,
    WLAN_TYPE,
    EP_BULT_TYPE
} ep_type;

enum {
    USB_READ = 0,
    USB_WRITE,
    USB_OPT_BUTT
};

typedef struct {
    td_u32 msg_type;
    td_u32 len;
    td_u32 reserve;
} usb_package;

struct usb_dev_notification {
    td_u32 notification;         /* Notification */
    td_u32 dev_mem_highpri_pool; /* Reserved */
};

typedef struct {
    td_u32 xfer_count;
    td_u32 credit_info;
    osal_u16 aggr_len[HIUSB_DEV2HOST_SCATT_MAX];
} hiusb_aggr_info;

struct usb_urb_context {
    struct completion done; // LINUX
    td_u32 status;
};

typedef struct _usb_host_assem_info_ {
    td_u32 assemble_num;
    /* next assem pkts list */
    hcc_data_queue assembled_head;
} usb_host_assem_info;

struct hcc_usb {
    struct usb_interface *interface_wlan;
    hcc_bus *pst_bus;
    td_u32 state;
    osal_mutex ctrl_ep_lock;
    osal_mutex state_lock;
    usb_host_reg_lock reg_rw_lock;
    osal_wait usb_rx_wq;
    osal_wait usb_tx_wq;

    osal_atomic usb_rx_cond;
    osal_atomic usb_tx_cond;
    osal_atomic usb_suspend;
    osal_atomic usb_driver_status;

    osal_task *rx_data_handle;
    osal_task *rx_msg_handle;
    hiusb_aggr_info aggr_info[EP_BULT_TYPE];
    td_u32 dev_mem_highpri_pool;
    td_u8 connect;
    td_u8 is_wlan_rx_thread_running;
    td_u8 is_rx_msg_thread_running;
    td_u16 hcc_queue_id;
    hcc_usb_device_speed usb_mode;
    hcc_unc_struc descript_unc;
    usb_host_assem_info tx_assemble_info;
    bus_pm_notify suspend_notify_func;
    bus_pm_notify resume_notify_func;
};

typedef struct {
    td_u8 *buf; /* data buf */
    struct urb *urb;
    struct osal_list_head list;
} urb_buf_stru;

typedef struct _urb_queue_ {
    struct osal_list_head data_queue;
    td_u32 qlen;
    osal_spinlock data_queue_lock;
} urb_queue;

typedef enum {
    BUS_USB_OFF = 0,
    BUS_USB_INIT = 1,
    BUS_USB_DISCONNECT = 2,
    BUS_USB_SUSPEND_DISCONNECT = 3,
    BUS_USB_BOOT_PROBE = 4,
    BUS_USB_BOOT = 5,
    BUS_USB_PROBE = 6,
    BUS_USB_SUSPEND = 7,
    BUS_USB_RESUME = 8,
    BUS_USB_WORK = 9,
} bus_usb_state;

typedef enum {
    TSK_USB_EXIT = 0,
    TSK_USB_SHOULD_STOP = 1,
    TSK_USB_RUNING = 2,
} usb_tsk_status;

static inline td_bool urb_list_is_empty(urb_queue *urb_queue)
{
    unsigned long flags;
    td_bool ret;
    osal_spin_lock_irqsave(&urb_queue->data_queue_lock, &flags);
    ret = osal_list_empty(&urb_queue->data_queue);
    osal_spin_unlock_irqrestore(&urb_queue->data_queue_lock, &flags);
    return ret;
}

static inline td_u32 urb_list_len(const urb_queue *urb_queue)
{
    return urb_queue->qlen;
}

static inline td_void urb_list_add_tail(urb_queue *urb_queue, urb_buf_stru *urb_buf)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&urb_queue->data_queue_lock, &flags);
    osal_list_add_tail(&urb_buf->list, &urb_queue->data_queue);
    urb_queue->qlen++;
    osal_spin_unlock_irqrestore(&urb_queue->data_queue_lock, &flags);
}

static inline td_void urb_list_head_reset(urb_queue *urb_queue)
{
    OSAL_INIT_LIST_HEAD(&urb_queue->data_queue);
    urb_queue->qlen = 0;
}

static inline td_void urb_list_head_init(urb_queue *urb_queue)
{
    osal_spin_lock_init(&urb_queue->data_queue_lock);
    urb_list_head_reset(urb_queue);
}

static inline td_void _urb_list_unlink(struct osal_list_head *node, urb_queue *urb_queue)
{
    urb_queue->qlen--;
    osal_list_del(node);
}

static inline struct osal_list_head *_urb_list_peek(urb_queue *urb_queue)
{
    struct osal_list_head *list = urb_queue->data_queue.next;
    if (list == &urb_queue->data_queue) {
        list = TD_NULL;
    }
    return list;
}

static inline urb_buf_stru *urb_list_peek(urb_queue *urb_queue)
{
    urb_buf_stru *urb_buf = TD_NULL;
    struct osal_list_head *node = TD_NULL;
    node = urb_queue->data_queue.next;
    if (node == &urb_queue->data_queue) {
        return TD_NULL;
    }
    urb_buf = osal_list_entry(node, urb_buf_stru, list);
    return urb_buf;
}

static inline struct osal_list_head *_urb_list_dequeue(urb_queue *urb_queue)
{
    struct osal_list_head *node = _urb_list_peek(urb_queue);
    if (node != TD_NULL) {
        _urb_list_unlink(node, urb_queue);
    }
    return node;
}

static inline urb_buf_stru *urb_list_dequeue(urb_queue *urb_queue)
{
    unsigned long flags;
    urb_buf_stru *urb_buf = TD_NULL;
    struct osal_list_head *node;
    osal_spin_lock_irqsave(&urb_queue->data_queue_lock, &flags);
    node = _urb_list_dequeue(urb_queue);
    if (node != TD_NULL) {
        urb_buf = osal_list_entry(node, urb_buf_stru, list);
    }
    osal_spin_unlock_irqrestore(&urb_queue->data_queue_lock, &flags);
    return urb_buf;
}

static inline td_void urb_list_add_head(urb_queue *urb_queue, urb_buf_stru *urb_buf)
{
    unsigned long flags;
    osal_spin_lock_irqsave(&urb_queue->data_queue_lock, &flags);
    osal_list_add(&urb_buf->list, &urb_queue->data_queue);
    urb_queue->qlen++;
    osal_spin_unlock_irqrestore(&urb_queue->data_queue_lock, &flags);
}

td_bool usb_log_enable_get(td_void);
td_void usb_log_enable_set(td_bool enable);

td_s32 hcc_usb_wait_disconnect(void);
td_s32 hcc_usb_reinit(hcc_bus *pst_bus);
td_s32 hcc_usb_reload(hcc_bus *pst_bus);
struct hcc_usb *usb_get_handle(td_void);
td_u32 usb_get_bus_state(td_void);
td_void usb_disable_state(hcc_bus *pst_bus, td_u32 mask);
td_void usb_sched_rx_thread(td_void);
td_s32 oal_usb_interrupt_msg(struct usb_device *usb_dev, struct usb_host_endpoint *ep_in,
    td_void *data, td_s32 len, td_s32 *actual_length, td_s32 timeout, struct usb_urb_context *ctx);
td_s32 oal_usb_bulk_in_msg(struct usb_device *usb_dev, struct usb_host_endpoint *bulk_in_hep,
    td_void *data, td_s32 len, td_s32 *actual_length, td_s32 timeout, struct usb_urb_context *ctx);
td_void usb_sched_tx_thread(td_void);
td_s32 usb_thread_condtion(const td_void *data);
td_u32 usb_reg_info_init(td_void);
td_void usb_free_reg_info(td_void);
td_s32 usb_tx_urb_list_init(td_void);
td_void usb_tx_urb_list_deinit(td_void);
hcc_bus *usb_bus_init(struct hcc_usb *usb);
td_void usb_tx_urb_list_reset(td_void);
td_void usb_reset_status(td_void);
td_s32 usb_rx_urb_list_init(td_void);
td_void usb_rx_urb_list_deinit(td_void);
td_void usb_rx_urb_list_reset(td_void);
td_void usb_rx_urb_list_clean(td_void);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_void usb_trx_urb_list_malloc_memory_opt(urb_buf_stru *urb_trx_queue_node[], osal_u32 i);
osal_void usb_trx_urb_list_free_memory_opt(urb_buf_stru *urb_trx_queue_node[], osal_u32 i);
#endif

td_void usb_task_exit(td_void);

td_u32 hcc_usb_bulk_out_timeout_get(td_void);
td_void hcc_usb_bulk_out_timeout_set(td_u32 timeout_ms);

#endif // CONFIG_HCC_SUPPORT_USB
#endif
