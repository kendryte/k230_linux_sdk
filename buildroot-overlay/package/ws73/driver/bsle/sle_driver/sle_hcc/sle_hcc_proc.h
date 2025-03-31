/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_HCC_PROC_H
#define SLE_HCC_PROC_H

#include "td_base.h"

#ifndef SLE_ON
#define SLE_ON 1
#endif
#ifndef SLE_OFF
#define SLE_OFF 0
#endif

typedef int (*sle_hcc_rx)(uint8_t *data, uint16_t len);

ext_errno sle_hcc_init(void);
ext_errno sle_hcc_deinit(void);
int sle_hci_send_frame(uint8_t *data_buf, uint16_t len);
void sle_dev_rx_register(sle_hcc_rx hcc_rx);
#endif