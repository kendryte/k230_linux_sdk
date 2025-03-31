/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_TM_CHBA_PROC_H
#define SLE_TM_CHBA_PROC_H

#include "sle_hci_chba_proc.h"

typedef enum tm_kernel_type {
    TM_KERNEL_TYPE_SEND_LINK_INFO = 0xB1,
    TM_KERNEL_TYPE_CREATE_CONN_PARA = 0xB2,
    TM_KERNEL_TYPE_UPDATE_CONN_PARA = 0xB3,
    TM_KERNEL_TYPE_CHAB_UPDATE_CONN_PARA = 0xB4,
    TM_KERNEL_TYPE_CHAB_CONN_QUEST = 0xB5,
    TM_KERNEL_TYPE_SEND_PHY_PARA = 0xB6,
    TM_KERNEL_TYPE_CHBA_UPDATE_PHY_PARA = 0xB7,
} tm_kernel_type_t;

uint32_t sle_tm_send_to_chba(uint8_t *data, uint16_t data_len);
void sle_tm_register_chba_recv_interface(int (*recv)(uint8_t *, uint16_t));

#endif /* SLE_TM_CHBA_PROC_H */