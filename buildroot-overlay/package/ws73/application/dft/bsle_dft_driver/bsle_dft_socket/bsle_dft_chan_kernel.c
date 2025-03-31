/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */
#include <net/net_namespace.h>
#include <crypto/internal/skcipher.h>
#include <crypto/internal/rng.h>
#include <crypto/akcipher.h>
#include <crypto/kpp.h>
#include <linux/sched.h>
#include <linux/security.h>

#include "oal_schedule.h"
#include "securec.h"
#include "bsle_dft_hcc_proc.h"
#include "bsle_dft_chan_kernel.h"

static void sle_hci_chan_send_frame(struct sk_buff *skb);
static void ble_hci_chan_send_frame(struct sk_buff *skb);
#define BT_POWER_FLAG 4

struct netlink_kernel_cfg cfg_hci = {
    .groups = 0,
    .flags = 0,
    .input = sle_hci_chan_send_frame,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL
};
struct netlink_kernel_cfg cfg_tm = {
    .groups = 0,
    .flags = 0,
    .input = ble_hci_chan_send_frame,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL
};
sle_chan_cfg_t g_sle_chan_cfg[SLE_CHAN_MAX] = {
    {
        .chan_id = SLE_CHAN_HCI,
        .user_port = SLE_CHAN_USER_PORT_HCI,
        .netlink_protocol = SLE_CHAN_NETLINK_HCI,
        .netlink_cfg = &cfg_hci,
    },
    {
        .chan_id = BLE_CHAN_HCI,
        .user_port = BLE_CHAN_USER_PORT_HCI,
        .netlink_protocol = BLE_CHAN_NETLINK_HCI,
        .netlink_cfg = &cfg_tm,
    }
};

static sle_chan_cfg_t *sle_chan_find_cfg(sle_chan_id_t chan_id)
{
    int i;
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].chan_id == chan_id) {
            return &g_sle_chan_cfg[i];
        }
    }
    return NULL;
}
static void sle_register_send_frame_cb(sle_chan_id_t chan_id, sle_send_frame_cb_t cb)
{
    sle_chan_cfg_t *chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_register_send_frame_cb] not find chan, chan_id: %d\n", chan_id);
        return;
    }
    chan_cfg->send = cb;
}

void sle_hci_register_send_frame_cb(sle_send_frame_cb_t cb)
{
    sle_register_send_frame_cb(SLE_CHAN_HCI, cb);
}
void ble_hci_register_send_frame_cb(sle_send_frame_cb_t cb)
{
    sle_register_send_frame_cb(BLE_CHAN_HCI, cb);
}

static int sle_chan_recv_frame(sle_chan_id_t chan_id, uint8_t *data, uint16_t data_len)
{
    struct nlmsghdr *nlh;
    int ret;
    struct sk_buff *nl_skb;
    sle_chan_cfg_t *chan_cfg;
    if (data == NULL || data_len == 0) {
        return SLE_CHAN_PARAM_INVALID;
    }
    nl_skb = nlmsg_new(data_len, GFP_ATOMIC);
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_chan_recv_frame] not find chan, chan_id: %d\n", chan_id);
        return SLE_CHAN_PARAM_INVALID;
    }
    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, chan_cfg->netlink_protocol, data_len, 0);
    if (nlh == NULL) {
        oal_print_err("nlmsg_put failaure\n");
        nlmsg_free(nl_skb);
        return SLE_CHAN_MEMORY_ERR;
    }
    ret = memcpy_s(nlmsg_data(nlh), data_len, data, data_len);
    if (ret != EOK) {
        return SLE_CHAN_MEMORY_ERR;
    }
    ret = netlink_unicast(chan_cfg->nlsk_sock, nl_skb, chan_cfg->user_port, MSG_DONTWAIT);
    return ret;
}
int sle_hci_chan_recv_frame(uint8_t *data, uint16_t data_len)
{
    return sle_chan_recv_frame(SLE_CHAN_HCI, data, data_len);
}
int ble_hci_chan_recv_frame(uint8_t *data, uint16_t data_len)
{
    return sle_chan_recv_frame(BLE_CHAN_HCI, data, data_len);
}

static void sle_chan_send_frame_pm_open(void)
{
    bool boot_finish;
    int32_t ret = pm_sle_open();
    if (ret != OAL_SUCC) {
        printk("pm_sle_open failed\n");
    }
    ret = pm_ble_open();
    if (ret != OAL_SUCC) {
        printk("pm_ble_open failed\n");
    }

    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        printk("device boot not finish \n");
    }

    ret = sle_hcc_init();
    if (ret != OAL_SUCC) {
        printk("initial hcc sle service failed\n");
        return;
    }

    ret = ble_hcc_init();
    if (ret != OAL_SUCC) {
        printk("initial hcc ble service failed\n");
        return;
    }
    return;
}

static void sle_chan_send_frame_pm_close(void)
{
    int32_t ret;
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_SLE);
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT);
    hbsle_hcc_customize_reset_device_status();
    ret = pm_sle_close();
    if (ret != OAL_SUCC) {
        printk("pm_sle_close failed\n");
    }
    ret = pm_ble_close();
    if (ret != OAL_SUCC) {
        printk("pm_sle_close failed\n");
    }
    oal_print_err("enter:%s\n", __func__);
    return;
}
static void sle_chan_send_frame(sle_chan_id_t chan_id, struct sk_buff *skb)
{
    uint8_t pm_open[BT_POWER_FLAG] = {00, 00, 00, 00};
    uint8_t pm_close[BT_POWER_FLAG] = {01, 01, 01, 01};
    struct nlmsghdr *nlh = NULL;
    uint8_t *umsg = NULL;
    sle_chan_cfg_t *chan_cfg;
    uint16_t udata_len = 0;
    if (skb == NULL || skb->len < nlmsg_total_size(0)) {
        oal_print_err("netlink channel skb err! %d\n", chan_id);
        return;
    }
    nlh = nlmsg_hdr(skb);
    umsg = NLMSG_DATA(nlh);
    udata_len = (uint16_t)nlmsg_len(nlh);
    if (memcmp(umsg, pm_open, udata_len) == 0) {
        printk("CMD Send OK\n");
        sle_chan_send_frame_pm_open();
        return;
    }
    if (memcmp(umsg, pm_close, udata_len) == 0) {
        printk("CMD Send OK\n");
        sle_chan_send_frame_pm_close();
        return;
    }
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_chan_send_frame] not find chan, chan_id: %d\n", chan_id);
        return;
    }
    if (chan_cfg->send) {
        chan_cfg->send(umsg, udata_len);
    } else {
        oal_print_err("[sle_chan_send_frame] chan send fun null, chan_id: %d\n", chan_id);
    }
}

static void sle_hci_chan_send_frame(struct sk_buff *skb)
{
    sle_chan_send_frame(SLE_CHAN_HCI, skb);
}

static void ble_hci_chan_send_frame(struct sk_buff *skb)
{
    sle_chan_send_frame(BLE_CHAN_HCI, skb);
}

int sle_chan_init(void)
{
    uint32_t i;
    printk("[sle_chan_init] init sle chan\n");
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        struct sock *sle_nlsk
            = netlink_kernel_create(&init_net, g_sle_chan_cfg[i].netlink_protocol, g_sle_chan_cfg[i].netlink_cfg);
        if (sle_nlsk == NULL) {
            return SLE_CHAN_FAILED;
        }
        g_sle_chan_cfg[i].nlsk_sock = sle_nlsk;
    }
    return SLE_CHAN_SUCCESS;
}

void __exit sle_chan_exit(void)
{
    int i;
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].nlsk_sock) {
            netlink_kernel_release(g_sle_chan_cfg[i].nlsk_sock);
        }
    }
}
