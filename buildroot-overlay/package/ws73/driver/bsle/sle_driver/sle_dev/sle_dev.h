/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_DEV_H_
#define SLE_DEV_H_
#include "soc_osal.h"

typedef int (*sle_dev_tx)(uint8_t *data, uint16_t len);
typedef void (*pm_bsle_enable_reply_cb)(osal_u8);
extern osal_void pm_sle_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb);
int sle_dev_init(void);
void sle_dev_exit(void);
int sle_dev_rx(uint8_t *data, uint16_t len);
void sle_dev_tx_register(sle_dev_tx tx);
extern osal_s32 pm_sle_enable(void);
extern osal_s32 pm_sle_disable(void);
#endif
