/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include "securec.h"
#include "bsle_dft_chan_user.h"

static struct sockaddr_nl g_dst_addr = {
    .nl_family = AF_NETLINK,
    .nl_pad = 0,
    .nl_pid = 0,
    .nl_groups = 0,
};

static struct sockaddr_nl g_src_addr_hci = {
    .nl_family = AF_NETLINK,
    .nl_pad = 0,
    .nl_pid = SLE_CHAN_USER_PORT_HCI,
    .nl_groups = 0,
};

static struct sockaddr_nl g_src_addr_tm = {
    .nl_family = AF_NETLINK,
    .nl_pad = 0,
    .nl_pid = BLE_CHAN_USER_PORT_HCI,
    .nl_groups = 0,
};

sle_chan_cfg_t g_sle_chan_cfg[SLE_CHAN_MAX] = {
    {
        .chan_id = SLE_CHAN_HCI,
        .user_port = SLE_CHAN_USER_PORT_HCI,
        .netlink_protocol = SLE_CHAN_NETLINK_HCI,
        .src_addr = &g_src_addr_hci,
        .dst_addr = &g_dst_addr,
        .nl_fd = -1,
    },
    {
        .chan_id = BLE_CHAN_HCI,
        .user_port = BLE_CHAN_USER_PORT_HCI,
        .netlink_protocol = BLE_CHAN_NETLINK_HCI,
        .src_addr = &g_src_addr_tm,
        .dst_addr = &g_dst_addr,
        .nl_fd = -1,
    }
};

static sle_chan_cfg_t *sle_chan_find_cfg(sle_chan_id_t chan_id)
{
    for (int i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].chan_id == chan_id) {
            return &g_sle_chan_cfg[i];
        }
    }
    return NULL;
}

static void sle_register_recv_data_cb(sle_chan_id_t chan_id, sle_recv_data_cb_t cb)
{
    sle_chan_cfg_t *chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        printf("ERROR: [sle_register_recv_data_cb] not find chan, chan_id: %d\n", chan_id);
        return;
    }
    chan_cfg->recv = cb;
}
void sle_hci_register_recv_data_cb(sle_recv_data_cb_t cb)
{
    sle_register_recv_data_cb(SLE_CHAN_HCI, cb);
}

void ble_hci_register_recv_data_cb(sle_recv_data_cb_t cb)
{
    sle_register_recv_data_cb(BLE_CHAN_HCI, cb);
}

static int sle_chan_recv_data(sle_chan_id_t chan_id, uint8_t **data_buf, uint16_t *buf_size)
{
    sle_nl_msg_t sle_msg;
    socklen_t addr_len;
    size_t data_len;
    sle_chan_cfg_t *chan_cfg = NULL;

    memset_s(&sle_msg, sizeof(sle_nl_msg_t), 0, sizeof(sle_nl_msg_t));
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (data_buf == NULL || buf_size == NULL || chan_cfg == NULL) {
        printf("ERROR: [sle_chan_recv_data] not find chan, chan_id: %d\n", chan_id);
        return SLE_CHAN_PARAM_INVALID;
    }
    if (chan_cfg->nl_fd == -1) {
        printf("ERROR: [sle_chan_recv_data] chan not init: %d\n", chan_id);
        return SLE_CHAN_FAILED;
    }
    int ret = recvfrom(chan_cfg->nl_fd,
        &sle_msg,
        sizeof(sle_nl_msg_t),
        0,
        (struct sockaddr *)(uintptr_t)chan_cfg->dst_addr,
        &addr_len);
    if (!ret) {
        printf("ERROR: recvfrom error: %d\n", ret);
        return SLE_CHAN_FAILED;
    }
    data_len = sle_msg.hdr.nlmsg_len - sizeof(struct nlmsghdr);

    if (data_len > sle_msg.hdr.nlmsg_len) {
        return SLE_CHAN_FAILED;
    }
    *data_buf = (uint8_t *)malloc(data_len);
    if (*data_buf == NULL) {
        printf("ERROR: [sle_chan_recv_data] memory malloc failed!\n");
        return SLE_CHAN_MEMORY_ERR;
    }
    if (memcpy_s(*data_buf, data_len, sle_msg.data, data_len) != EOK) {
        printf("ERROR: [sle_chan_recv_data] memcpy_s failed!\n");
    }
    *buf_size = data_len;
    if (chan_cfg->recv) {
        chan_cfg->recv(sle_msg.data, data_len);
    }
    return SLE_CHAN_SUCCESS;
}

int sle_hci_chan_recv_data(uint8_t **data_buf, uint16_t *buf_size)
{
    return sle_chan_recv_data(SLE_CHAN_HCI, data_buf, buf_size);
}
int ble_hci_chan_recv_data(uint8_t **data_buf, uint16_t *buf_size)
{
    return sle_chan_recv_data(BLE_CHAN_HCI, data_buf, buf_size);
}
static int sle_chan_send_data(sle_chan_id_t chan_id, uint8_t *data, uint16_t data_len)
{
    sle_chan_cfg_t *chan_cfg;
    struct nlmsghdr *nlh;
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        printf("ERROR: [sle_chan_send] not find chan, chan_id: %d\n", chan_id);
        return SLE_CHAN_PARAM_INVALID;
    }
    if (chan_cfg->nl_fd == -1) {
        printf("ERROR: [sle_chan_send] chan not init: %d\n", chan_id);
        return SLE_CHAN_FAILED;
    }
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(data_len));          // nlh数据发出之后再内核态释放
    if (nlh == NULL) {
        printf("ERROR: [sle_chan_send] nlh malloc fail\n");
        return SLE_CHAN_FAILED;
    }
    if (memset_s(nlh, NLMSG_SPACE(data_len), 0, sizeof(struct nlmsghdr)) != EOK) {
        printf("ERROR: [sle_chan_recv_data] memset_s failed!\n");
    }
    nlh->nlmsg_len = (uint32_t)NLMSG_LENGTH(data_len);
    /* 修改成进程id， getpid接口 */
    nlh->nlmsg_pid = chan_cfg->user_port;
    if (memcpy_s(NLMSG_DATA(nlh), data_len, data, data_len) != EOK) {
        printf("ERROR: [sle_chan_recv_data] memcpy_s failed!\n");
    }
    int ret = sendto(chan_cfg->nl_fd,
        nlh,
        nlh->nlmsg_len,
        0,
        (struct sockaddr *)(uintptr_t)chan_cfg->dst_addr,
        sizeof(struct sockaddr_nl));
    free(nlh);
    if (!ret) {
        printf("ERROR: sendto error: %d\n", ret);
        return SLE_CHAN_FAILED;
    }
    return SLE_CHAN_SUCCESS;
}

int sle_hci_chan_send_data(uint8_t *data, uint16_t data_len)
{
    return sle_chan_send_data(SLE_CHAN_HCI, data, data_len);
}

int ble_hci_chan_send_data(uint8_t *data, uint16_t data_len)
{
    return sle_chan_send_data(BLE_CHAN_HCI, data, data_len);
}
void sle_chan_init(void)
{
    for (int i = 0; i < SLE_CHAN_MAX; i++) {
        int nl_fd = socket(AF_NETLINK, SOCK_RAW, g_sle_chan_cfg[i].netlink_protocol);
        if (nl_fd == -1) {
            printf("ERROR: create socket error\n");
            return;
        }
        if (bind(nl_fd, (struct sockaddr *)(uintptr_t)g_sle_chan_cfg[i].src_addr, sizeof(struct sockaddr_nl)) != 0) {
            printf("ERROR: bind error\n");
            close(nl_fd);
            return;
        }
        g_sle_chan_cfg[i].nl_fd = nl_fd;
    }
}

void sle_chan_deinit(void)
{
    for (int i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].nl_fd != -1) {
            close(g_sle_chan_cfg[i].nl_fd);
            g_sle_chan_cfg[i].nl_fd = -1;
        }
    }
}