/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BSLE_DFT_CHAN_KERNEL_H
#define BSLE_DFT_CHAN_KERNEL_H

#include <net/netlink.h>
#include "customize_bsle_ext.h"
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc_adapt.h"

extern osal_s32 pm_sle_open(void);
extern osal_s32 pm_sle_close(void);
extern osal_s32 pm_ble_open(void);
extern osal_s32 pm_ble_close(void);

typedef void (*sle_send_frame_cb_t)(uint8_t *data, uint16_t data_len);

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
    sle_send_frame_cb_t send;
    struct netlink_kernel_cfg *netlink_cfg;
    struct sock *nlsk_sock;
} sle_chan_cfg_t;

int sle_hci_chan_recv_frame(uint8_t *data, uint16_t data_len);
int ble_hci_chan_recv_frame(uint8_t *data, uint16_t data_len);
void sle_hci_register_send_frame_cb(sle_send_frame_cb_t cb);
void ble_hci_register_send_frame_cb(sle_send_frame_cb_t cb);
int sle_chan_init(void);
void __exit sle_chan_exit(void);

#endif