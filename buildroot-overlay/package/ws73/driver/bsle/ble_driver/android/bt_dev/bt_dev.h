/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BT_DEV_H_
#define BT_DEV_H_
#include "soc_osal.h"

typedef int (*dev_tx)(uint8_t *data, uint16_t len);
typedef void (*pm_bsle_enable_reply_cb)(osal_u8);
extern osal_void pm_ble_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb);
int bt_dev_init(void);
void bt_dev_exit(void);
int bt_dev_rx(uint8_t *data, uint16_t len);
void bt_dev_tx_register(dev_tx tx);
bool ble_get_btc_state(void);
extern osal_s32 pm_ble_enable(void);
extern osal_s32 pm_ble_disable(void);
#endif
