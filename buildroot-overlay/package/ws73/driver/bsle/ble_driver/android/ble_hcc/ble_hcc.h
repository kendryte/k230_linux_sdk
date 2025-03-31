/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BT_HCC_H
#define BT_HCC_H

#include "customize_bsle_ext.h"
#include "hcc_if.h"
#include "hcc_adapt.h"
#include "oal_schedule.h"
#include "securec.h"

typedef int (*ble_hcc_rx)(uint8_t *buf, uint32_t len);

ext_errno ble_hcc_init(void);

ext_errno ble_hcc_deinit(void);

int ble_hci_send_frame(uint8_t *buff, uint32_t len, void *user_param);

void ble_hcc_rx_register(ble_hcc_rx hcc_rx);

#endif