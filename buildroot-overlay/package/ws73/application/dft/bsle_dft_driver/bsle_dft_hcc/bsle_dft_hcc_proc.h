/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BSLE_DFT_HCC_PROC_H
#define BSLE_DFT_HCC_PROC_H

#include "td_base.h"

ext_errno sle_hcc_init(void);
ext_errno ble_hcc_init(void);
void sle_hci_send_frame(uint8_t *data_buf, uint16_t len);
void ble_hci_send_frame(uint8_t *data_buf, uint16_t len);
#endif