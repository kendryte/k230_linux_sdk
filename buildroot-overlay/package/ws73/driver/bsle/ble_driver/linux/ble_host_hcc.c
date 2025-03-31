/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 */
#include "ble_host_hcc.h"

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <asm/unaligned.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>

#define BT_TX_MAX_FRAME             1500      /* tx bt data max lenth */
#define SKB_FRAME_TYPE_LEN          1         /* skb frame type lenth */

#define BT_MAC_ADDR_LEN 6 // bt addr len 6
#define CMD_WRITE_BD_ADDRESS    0xfc32
#define ADV_INTERVAL_MIN 0x20 // adv interval min 20

struct bt_hci_cmd_hdr {
    uint16_t opcode;
    uint8_t plen;
} __attribute__((packed));

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
#define hci_skb_pkt_type(skb) bt_cb((skb))->pkt_type
#define hci_skb_expect(skb) bt_cb((skb))->expect
#define HCI_PRIMARY HCI_BREDR
#endif
#endif

#define BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME 2000
#define BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME 10

#define BLE_OPEN_BTC_FINISH 1
#define BLE_CLOSE_BTC_FINISH 0

static bool g_ble_state = BLE_OFF;
static void ble_open_close_btc_finish_handle(osal_u8 event);

MODULE_LICENSE("GPL");
static int ext_recv(struct hci_dev *hdev, const void *data, int count);

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0))
// 该函数直接从linux函数移植过来，低版本内核中无此函数的实现
static inline void *skb_put_data(struct sk_buff *skb, const void *data, unsigned int len)
{
    int32_t ret;
    void *tmp = skb_put(skb, len);
    ret = memcpy_s(tmp, len, data, len);
    if (ret != EOK) {
        return NULL;
    }
    return tmp;
}
#endif
#endif

struct ext_bt_dev {
    struct hci_dev *hdev;
    osal_atomic hdev_state;
};

struct ext_bt_dev st_bt_dev;

typedef enum {
    HDEV_STATE_DOWN = 0,
    HDEV_STATE_UP,
    HDEV_STATE_BUTT
} hdev_state;

struct hbt_recv_pkt {
    uint8_t type;    /* Packet type */
    uint8_t hlen;    /* Header length */
    uint8_t loff;    /* Data length offset in header */
    uint8_t lsize;   /* Data length field size */
    uint16_t maxlen; /* Max overall packet length */
    int (*recv)(struct hci_dev *hdev, struct sk_buff *skb);
};

/* bt host向device发送的消息,hcc需要额外分配sdio传输相应的32字节对齐的内存空间。在hcc发送消息成功后,会进入此回调函数,
用于释放bluez协议栈HCI组装的skb数据报文和hcc申请的内存空间 */
static td_void hcc_adapt_bt_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    struct sk_buff *skb;
    skb = (struct sk_buff *)user_param;
    if (skb != TD_NULL) {
        kfree_skb(skb);
    }
    if (buf != TD_NULL) {
        osal_kfree(buf);
    }
}

/* bt device向host发送的消息,hcc接收到原始消息需要申请32字节对齐的内存空间,*buf即是内存申请指向的空间 */
static td_u32 hcc_adapt_bt_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_ATOMIC);
#else
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
#endif
    if (*buf == TD_NULL) {
        hcc_debug("failed\r\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

/* bt host的rx处理,user_param指向的是hcc原始分配的32字节对齐的起始内存空间地址,buf是device发送的实际消息的
内存起始地址,len的大小代表了device发送消息的实际长度,user_param和buf共享同一片内存空间 */
static td_u32 hcc_bt_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    struct hci_dev *hdev = NULL;
    if (buf == NULL || user_param == NULL) {
        return EXT_ERR_FAILURE;
    }
    if (g_ble_state == BLE_OFF) {
        hcc_debug("hcc bt rx proc ble closed\n");
        osal_kfree(user_param);
        return EXT_ERR_FAILURE;
    }
    hdev = st_bt_dev.hdev;
    if (hdev == NULL) {
        hcc_debug("hdev is null, drop recv pkt, buf is%p, %s\n", buf, __func__);
        osal_kfree(user_param);
        return EXT_ERR_FAILURE;
    }

    ext_recv(hdev, buf, len);
    osal_kfree(user_param);
    return EXT_ERR_SUCCESS;
}

hcc_adapt_ops g_hcc_ble_adapt = {
    .free = hcc_adapt_bt_free,
    .alloc = hcc_adapt_bt_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
    .rx_proc = hcc_bt_rx_proc,
};

static const struct hbt_recv_pkt g_ext_recv_pkts[] = {
    {.type = HCI_ACLDATA_PKT,
     .hlen = HCI_ACL_HDR_SIZE,
     .loff = 2,
     .lsize = 2,
     .maxlen = HCI_MAX_FRAME_SIZE,
     .recv = hci_recv_frame},
    {.type = HCI_SCODATA_PKT,
     .hlen = HCI_SCO_HDR_SIZE,
     .loff = 2,
     .lsize = 1,
     .maxlen = HCI_MAX_SCO_SIZE,
     .recv = hci_recv_frame},
    {.type = HCI_EVENT_PKT,
     .hlen = HCI_EVENT_HDR_SIZE,
     .loff = 1,
     .lsize = 1,
     .maxlen = HCI_MAX_EVENT_SIZE,
     .recv = hci_recv_frame},
};

static int ext_recv_buf_update_len(struct hci_dev *hdev, struct sk_buff **skb, const struct hbt_recv_pkt *pkts,
    unsigned int pkts_index)
{
    u16 dlen;
    if ((*skb)->len == (&pkts[pkts_index])->hlen) {
        switch ((&pkts[pkts_index])->lsize) {
            case 0: // size 0
                /* No variable data length */
                dlen = 0;
                break;
            case 1: // size 1
                /* Single octet variable length */
                dlen = (*skb)->data[(&pkts[pkts_index])->loff];
                hci_skb_expect(*skb) += dlen;

                if (skb_tailroom(*skb) < dlen) {
                    hcc_debug("skb pointer is %p, %s\n", *skb, __func__);
                    kfree_skb(*skb);
                    return -1; /* ERR_PTR(-EMSGSIZE) */
                }
                break;
            case 2: // size 2
                /* Double octet variable length */
                dlen = get_unaligned_le16((*skb)->data + (&pkts[pkts_index])->loff);
                hci_skb_expect(*skb) += dlen;

                if (skb_tailroom(*skb) < dlen) {
                    hcc_debug("skb pointer is %p, %s\n", *skb, __func__);
                    kfree_skb(*skb);
                    return -1; /* ERR_PTR(-EMSGSIZE) */
                }
                break;
            default:
                /* Unsupported variable length */
                hcc_debug("skb pointer is %p, %s\n", *skb, __func__);
                kfree_skb(*skb);
                return -1; /* ERR_PTR(-EILSEQ) */
        }

        if (!dlen) {
            /* No more data, complete frame */
            (&pkts[pkts_index])->recv(hdev, (*skb));
            (*skb) = NULL;
        }
    } else {
        /* Complete frame */
        (&pkts[pkts_index])->recv(hdev, (*skb));
        (*skb) = NULL;
    }

    return 0;
}

static struct sk_buff *ext_get_bt_head(const unsigned char *buffer, const struct hbt_recv_pkt *pkts, int pkts_count)
{
    struct sk_buff *skb = NULL;
    int i;

    for (i = 0; i < pkts_count; i++) {
        if (buffer[0] != (&pkts[i])->type) {
            continue;
        }

        skb = bt_skb_alloc((&pkts[i])->maxlen, GFP_ATOMIC);
        if (!skb) {
            return NULL;
        }

        hci_skb_pkt_type(skb) = (&pkts[i])->type;
        hci_skb_expect(skb) = (&pkts[i])->hlen;
        break;
    }

    return skb;
}

static struct sk_buff *ext_recv_buf(struct hci_dev *hdev,
    struct sk_buff *skb_buff,
    const unsigned char *buff,
    int buf_count)
{
    int err;
    struct sk_buff *skb = skb_buff;
    const unsigned char *buffer = buff;
    int count = buf_count;
    int pkts_count = osal_array_size(g_ext_recv_pkts);
    int i, len;

    if (g_ble_state == BLE_OFF) {
        hcc_debug("ext recv buf ble close \r\n");
        return NULL;
    }

    while (count) {
        if (!count) {
            break;
        }

        if (skb == NULL) {
            skb = ext_get_bt_head(buffer, g_ext_recv_pkts, pkts_count);
            /* Check for invalid packet type */
            if (skb == NULL) {
                hcc_debug("skb pointer is %p, %s\n", skb, __func__);
                return ERR_PTR(-EILSEQ);
            }

            count -= 1;
            buffer += 1;
        }

        len = min_t(uint, hci_skb_expect(skb) - skb->len, count);
        skb_put_data(skb, buffer, len);

        count -= len;
        buffer += len;

        /* Check for partial packet */
        if (skb->len < hci_skb_expect(skb)) {
            continue;
        }

        for (i = 0; i < pkts_count; i++) {
            if (hci_skb_pkt_type(skb) == (&g_ext_recv_pkts[i])->type) {
                break;
            }
        }

        if (i >= pkts_count) {
            hcc_debug("skb pointer is %p, %s\n", skb, __func__);
            kfree_skb(skb);
            return ERR_PTR(-EILSEQ);
        }

        err = ext_recv_buf_update_len(hdev, &skb, g_ext_recv_pkts, i);
        if (err != 0) {
            return ERR_PTR(-EMSGSIZE);
        }
    }

    return skb;
}

static int ext_recv(struct hci_dev *hdev, const void *data, int count)
{
    struct sk_buff *rx_skb;
    rx_skb = ext_recv_buf(hdev, NULL, (const unsigned char *)data, count);
    if (IS_ERR(rx_skb)) {
        int err = PTR_ERR(rx_skb);
        hcc_debug("ERROR rx_skb=%d\n", err);
        return err;
    }

    hdev->stat.byte_rx += count;
    return count;
}

/* Initialize device */
static int hci_bt_open(struct hci_dev *hdev)
{
    int ret = EXT_ERR_SUCCESS;
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    hcc_debug("enter %s\n", __func__);
    if (g_ble_state == BLE_ON) {
        hcc_debug("ble btc already opened \r\n");
        return EXT_ERR_FAILURE;
    }
    pm_ble_enable_reply_cb_host_register(ble_open_close_btc_finish_handle);
    ret = pm_ble_enable();
    if (ret != OAL_SUCC) {
        hcc_debug("ble send open btc msg error ret %d\r\n", ret);
        return EXT_ERR_FAILURE;
    }
    while (g_ble_state == BLE_OFF) {
        osal_msleep(BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME) {
            hcc_debug("ble open device finish fail ,time out \n");
            return -EINVAL;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    hcc_debug(" ble open time:%lld \n", end - start);
    // 进行状态置位
    osal_atomic_set(&(st_bt_dev.hdev_state), HDEV_STATE_UP);
    return ret;
}

/* Close device */
static int hci_bt_close(struct hci_dev *hdev)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end = 0;
    int ret;
    hcc_debug("hci_bt_close\n");
    if (g_ble_state == BLE_OFF) {
        hcc_debug("ble btc already closed \r\n");
        return EXT_ERR_FAILURE;
    }
    ret = pm_ble_disable();
    if (ret != OAL_SUCC) {
        hcc_debug("ble send close btc msg error ret %d\r\n", ret);
        return -EINVAL;
    }
    while (g_ble_state == BLE_ON) {
        osal_msleep(BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + BLE_DEVICE_OPEN_CLOSE_MSG_FINISH_MAX_WAIT_TIME) {
            hcc_debug("ble close device finish fail ,time out \n");
            g_ble_state = BLE_OFF;
            pm_ble_enable_reply_cb_host_register(NULL);
            return EXT_ERR_SUCCESS;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    hcc_debug(" ble close time:%lld \n", end - start);
    pm_ble_enable_reply_cb_host_register(NULL);
    hdev->flush = NULL;
    osal_atomic_set(&(st_bt_dev.hdev_state), HDEV_STATE_DOWN);
    return EXT_ERR_SUCCESS;
}

static void ble_open_close_btc_finish_handle(osal_u8 event)
{
    switch (event) {
        case BLE_OPEN_BTC_FINISH: {
            hcc_debug(" ble btc open finish \n");
            g_ble_state = BLE_ON;
            break;
        }
        case BLE_CLOSE_BTC_FINISH: {
            hcc_debug(" ble btc close finish \n");
            g_ble_state = BLE_OFF;
            break;
        }
        default:
            break;
    }
    return;
}

static void hci_bt_tx_complete(struct hci_dev *hdev, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
    int pkt_type = hci_skb_pkt_type(skb);
#else
    int pkt_type = bt_cb(skb)->pkt_type;
#endif

    /* Update HCI stat counters */
    switch (pkt_type) {
        case HCI_COMMAND_PKT:
            hdev->stat.cmd_tx++;
            break;

        case HCI_ACLDATA_PKT:
            hdev->stat.acl_tx++;
            break;

        case HCI_SCODATA_PKT:
            hdev->stat.sco_tx++;
            break;

        default:
            /* debug */
            hcc_debug("hci default type=%d\n", pkt_type);
            break;
    }
}

/* Send frames from HCI layer */
int ble_hci_send_frame(uint8_t *buff, uint32_t len, void *user_param)
{
    int ret;
    hcc_transfer_param bt_transfer_param = { 0 };
    if (len > BT_TX_MAX_FRAME) {
        hcc_debug("len is too large, len=%u\n", len);
        return -EINVAL;
    }
    bt_transfer_param.service_type = HCC_ACTION_TYPE_BT;
    bt_transfer_param.sub_type = 0;
    bt_transfer_param.queue_id = BT_DATA_QUEUE;
    bt_transfer_param.fc_flag = 0;
    bt_transfer_param.user_param = (td_u8 *)user_param;
    // 调用完tx_data后，若hcc返回成功，则进入到hcc_adapt_bt_free释放buf,否则只释放hci组装的skb消息
    ret = hcc_bt_tx_data(HCC_CHANNEL_AP, buff, len, &bt_transfer_param);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_debug("send tx data failed\n");
        return ret;
    }
    return ret;
}

static int hci_bt_send_frame(struct hci_dev *hdev, struct sk_buff *skb)
{
    int ret;
    if (skb->len > BT_TX_MAX_FRAME) {
        hcc_debug("bt skb len is too large, len=%u\n", skb->len);
        return -EINVAL;
    }

    if (g_ble_state == BLE_OFF) {
        hcc_debug("hci bt send frame ble not open \r\n");
        return -EINVAL;
    }

    /* Prepend skb with frame type */
    // 此处处理未用安全函数，直接使用linux内核中的处理语句
    ret = memcpy_s(skb_push(skb, SKB_FRAME_TYPE_LEN), SKB_FRAME_TYPE_LEN, &hci_skb_pkt_type(skb), SKB_FRAME_TYPE_LEN);
    if (ret != EOK) {
        hcc_debug("bt skb memcpy error\n");
        return -EINVAL;
    }

    hdev->stat.byte_tx += skb->len;
    hci_bt_tx_complete(hdev, skb);
    ret = ble_hci_send_frame(skb->data, skb->len, skb);
    return ret;
}

/* Reset device */
static int hci_bt_flush(struct hci_dev *hdev)
{
    hcc_debug("%s", __func__);
    if (g_ble_state == BLE_OFF) {
        hcc_debug("hci bt flush ble not open \r\n");
    }
    return EXT_ERR_SUCCESS;
}

static int hci_bt_setup(struct hci_dev *hdev)
{
    hcc_debug("hci_bt_setup\n");
    return EXT_ERR_SUCCESS;
}

static int adv_interval = 0x0080;
static int supv_timeout = 0x008a;
static int scan_interval = 0x0060;
static int scan_window = 0x0030;
static int conn_interval = 0x0038;

module_param(adv_interval, int, S_IRUSR);
module_param(supv_timeout, int, S_IRUSR);
module_param(scan_interval, int, S_IRUSR);
module_param(scan_window, int, S_IRUSR);
module_param(conn_interval, int, S_IRUSR);

static int bt_set_hci_dev_param(struct hci_dev *hdev)
{
    hdev->le_adv_min_interval = adv_interval;
    hdev->le_adv_max_interval = adv_interval;
    hdev->le_supv_timeout = supv_timeout;
    hdev->le_scan_interval = scan_interval;
    hdev->le_scan_window = scan_window;
    hdev->le_conn_min_interval = conn_interval;
    hdev->le_conn_max_interval = conn_interval;
    return EXT_ERR_SUCCESS;
}

// static int bt_register_hci_dev(void)
static int bt_register_hci_dev(void)
{
    int err;
    struct hci_dev *hdev;
    hcc_debug("%s enter\n", __func__);
    /* Initialize and register HCI device */
    hdev = hci_alloc_dev();
    if (hdev == NULL) {
        hcc_debug("can't allocate HCI device\n");
        return -ENOMEM;
    }

    bt_set_hci_dev_param(hdev);
    hdev->dev_type = HCI_PRIMARY;
    #ifdef WSCFG_BUS_USB
    hdev->bus = HCI_USB;
    #else
    hdev->bus = HCI_SDIO;
    #endif
    hdev->manufacturer = 16; /* 16 is hci manufacture */

    hdev->open = hci_bt_open;
    hdev->close = hci_bt_close;
    hdev->flush = hci_bt_flush;
    hdev->send = hci_bt_send_frame;
    hdev->setup = hci_bt_setup;

    err = hci_register_dev(hdev);
    if (err < 0) {
        hcc_debug("can't register HCI device, err=%d\n", err);
        hci_free_dev(hdev);
        return -ENODEV;
    }

    st_bt_dev.hdev = hdev;
    hcc_debug("%s exit\n", __func__);
    return EXT_ERR_SUCCESS;
}

// static void bt_unregister_hci_dev(void)
static void bt_unregister_hci_dev(void)
{
    struct hci_dev *hdev = NULL;
    hcc_debug("enter bt_unregister_hci_dev\n");
    if (st_bt_dev.hdev == NULL) {
        hcc_debug("hdev is null\n");
        return;
    }

    hdev = st_bt_dev.hdev;
    st_bt_dev.hdev = NULL;
    hci_unregister_dev(hdev);
    hci_free_dev(hdev);

    hcc_debug("exit bt_unregister_hci_dev\n");
}

static oal_int32 ext_bluetooth_dfr_recovery(void);

static oal_int32 set_bt_addr(void)
{
    oal_int32 ret;
    struct bt_hci_cmd_hdr cmd_hdr;
    osal_u8 bt_addr[BT_MAC_ADDR_LEN];
    osal_u8 tx_buf[1 + sizeof(struct bt_hci_cmd_hdr) + BT_MAC_ADDR_LEN];
    int mac_swap_index = 0;
    // 获取bt地址
#if defined(CONFIG_BLE_MAC_FORK) && CONFIG_BLE_MAC_FORK
    ret = get_dev_addr(bt_addr, BT_MAC_ADDR_LEN, IFTYPE_BLE);
#else
    ret = get_dev_addr(bt_addr, BT_MAC_ADDR_LEN, IFTYPE_ORG);
#endif
    if (ret != OAL_SUCC) {
        hcc_debug("get bt mac fail\r\n");
        return OAL_FAIL;
    }
    hcc_debug("BLE Mac Addr: %02x:%02x:%02x:%02x:**:**\n", bt_addr[0], bt_addr[0x1], bt_addr[0x2], bt_addr[0x3]);
    while (mac_swap_index < BT_MAC_ADDR_LEN / 2) { // divide 2
        osal_u8 tmp = bt_addr[mac_swap_index];
        bt_addr[mac_swap_index] = bt_addr[BT_MAC_ADDR_LEN - mac_swap_index - 1];
        bt_addr[BT_MAC_ADDR_LEN - mac_swap_index - 1] = tmp;
        mac_swap_index++;
    }
    cmd_hdr.opcode = CMD_WRITE_BD_ADDRESS;
    cmd_hdr.plen = BT_MAC_ADDR_LEN;
    tx_buf[0] = 0x1;
    (void)memcpy_s(
        tx_buf + 1, sizeof(struct bt_hci_cmd_hdr) + BT_MAC_ADDR_LEN, &cmd_hdr, sizeof(struct bt_hci_cmd_hdr));
    (void)memcpy_s(tx_buf + 1 + sizeof(struct bt_hci_cmd_hdr), BT_MAC_ADDR_LEN, bt_addr, BT_MAC_ADDR_LEN);
    return ble_hci_send_frame(tx_buf, 1 + sizeof(struct bt_hci_cmd_hdr) + BT_MAC_ADDR_LEN, NULL);
}

static oal_int32 ext_bluetooth_init(void)
{
    oal_int32 ret;
    bool boot_finish;

    hcc_debug("set default params\n");
    hcc_debug("adv_interval=0x%04x\n", adv_interval);
    hcc_debug("scan_interval=0x%04x\n", scan_interval);
    hcc_debug("scan_window=0x%04x\n", scan_window);
    hcc_debug("conn_interval=0x%04x\n", conn_interval);
    hcc_debug("supv_timeout=0x%04x\n", supv_timeout);
    if (adv_interval < ADV_INTERVAL_MIN || scan_interval < 0x4 || scan_window < 0x4 || conn_interval < 0x6 ||
        supv_timeout < 0xA) {
        hcc_debug("param invalid\n");
        return -EINVAL;
    }

    ret = pm_ble_open();
    if (ret != OAL_SUCC) {
        hcc_debug("pm_ble_open failed\n");
    } else {
        hcc_debug("pm_ble_open success\n");
    }

    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }

    ret = hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT, &g_hcc_ble_adapt);
    if (ret != OAL_SUCC) {
        hcc_debug("initial hcc bt service failed\n");
        return OAL_FAIL;
    }
    set_bt_addr();
    hcc_debug("begin to register hci device\n");

    ret = bt_register_hci_dev();
    if (ret != EXT_ERR_SUCCESS) {
        hcc_debug("bt register hci dev failed\n");
        return OAL_FAIL;
    }

#if defined(CONFIG_PLAT_SUPPORT_DFR)
    // 注册dfr回调
    plat_bt_exception_rst_register_etc(ext_bluetooth_dfr_recovery);
#endif
    return OAL_SUCC;
}

static osal_void ext_bluetooth_deinit(void)
{
    oal_int32 ret;
    hcc_debug("enter:%s\n", __func__);
    bt_unregister_hci_dev();
    ret = pm_ble_close();
    if (ret != OAL_SUCC) {
        hcc_debug("pm_ble_close failed\n");
    } else {
        hcc_debug("finish: pm_ble_close\n");
    }

    hbsle_hcc_customize_reset_device_status();
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT);
    hcc_debug("finish: H2D_MSG_BT_CLOSE\n");
}

oal_int32 reset_hci_dev(void)
{
    oal_int32 ret;
    if (st_bt_dev.hdev == NULL) {
        hcc_debug("hdev is null\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hcc_debug("start reset %s\n", st_bt_dev.hdev->name);
    ret = hci_reset_dev(st_bt_dev.hdev);
    if (ret != OAL_SUCC) {
        hcc_debug("reset %s error:%d \n", st_bt_dev.hdev->name, ret);
    }
    return ret;
}
#if defined(CONFIG_PLAT_SUPPORT_DFR)
static oal_int32 ext_bluetooth_dfr_recovery(void)
{
    oal_int32 ret;
    bool customize_received;
    bool boot_finish;
    int hdev_state;
    hcc_debug("start bluetooth dfr recovery \n");
    /* 恢复HCC状态 */
    hcc_switch_status(HCC_ON);
    hbsle_hcc_customize_reset_device_status();
    /* 重新下发定制化配置 */
    ret = hbsle_hcc_custom_ini_data_buf();
    if (ret != OAL_SUCC) {
        printk("bluetooth_dfr_recovery::hcc ini data,fail ret=%d. \n", ret);
        return ret;
    }
    customize_received = hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED);
    if (customize_received == false) {
        printk("customize data not received \n");
    }
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_BT_OPEN, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        printk("bluetooth_dfr_recovery::sle open,fail, ret=%d. \n", ret);
        return ret;
    }
    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }
    hdev_state = osal_atomic_read(&(st_bt_dev.hdev_state));
    if (hdev_state == HDEV_STATE_UP) {
        /* 重置hci状态 */
        ret = reset_hci_dev();
        if (ret != OAL_SUCC) {
            printk("bluetooth_dfr_recovery::reset_hci_dev,fail, ret=%d. \n", ret);
        }
    }
    return ret;
}
#endif

module_init(ext_bluetooth_init);
module_exit(ext_bluetooth_deinit);
