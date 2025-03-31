/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BSLE_DFT_CHAN_USER_H
#define BSLE_DFT_CHAN_USER_H
#include <linux/netlink.h>

/* 创建nl通道和接收内核数据在一个进程中实现(进程ID相同) */
#define MSG_LEN 256       // 需要修改成sle业务里面数据量的宏定义
typedef struct sle_nl_msg {
    struct nlmsghdr hdr;
    uint8_t data[MSG_LEN];
} sle_nl_msg_t;


typedef void (*sle_recv_data_cb_t)(uint8_t *data, uint16_t datalen);

typedef enum {
    SLE_CHAN_SUCCESS = 0,
    SLE_CHAN_FAILED = -1,
    SLE_CHAN_MEMORY_ERR = -2,
    SLE_CHAN_PARAM_INVALID = -3,
} sle_chan_error_code_t;

typedef enum {
    SLE_CHAN_USER_PORT_HCI = 100,
    BLE_CHAN_USER_PORT_HCI,
} sle_chan_user_port_t;

typedef enum {
    SLE_CHAN_NETLINK_HCI = 22,
    BLE_CHAN_NETLINK_HCI,
} sle_chan_netlink_protocol_t;

typedef enum {
    SLE_CHAN_HCI,
    BLE_CHAN_HCI,
    SLE_CHAN_MAX,
} sle_chan_id_t;

typedef struct sle_chan_cfg {
    sle_chan_id_t chan_id;
    sle_chan_user_port_t user_port;
    sle_chan_netlink_protocol_t netlink_protocol;
    struct sockaddr_nl *src_addr;
    struct sockaddr_nl *dst_addr;
    sle_recv_data_cb_t recv;
    int nl_fd;
} sle_chan_cfg_t;

void sle_hci_register_recv_data_cb(sle_recv_data_cb_t cb);
void ble_hci_register_recv_data_cb(sle_recv_data_cb_t cb);

int ble_hci_chan_recv_data(uint8_t **data_buf, uint16_t *buf_size);
int sle_hci_chan_recv_data(uint8_t **data_buf, uint16_t *buf_size);

int sle_hci_chan_send_data(uint8_t *data, uint16_t data_len);
int ble_hci_chan_send_data(uint8_t *data, uint16_t data_len);

void sle_chan_init(void);
void sle_chan_deinit(void);

#endif /* __SLE_CHAN_USER_H */