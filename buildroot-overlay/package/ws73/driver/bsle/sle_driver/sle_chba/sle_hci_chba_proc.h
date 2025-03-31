/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_HCI_CHBA_PROC_H
#define SLE_HCI_CHBA_PROC_H

#include "sle_data_type.h"

#define SLE_MAX_TCID_NUMS 101
#define HCI_DATA_TCID_POS 5
#define HCI_DATA_TCID_LEN 2

typedef struct hci_link_info {
    bool has_tcid;
    sle_link_info_t link_info;
} hci_link_info_t;

enum {
    HCI_DATATYPE_SLE_CMD = 0xA1,
    HCI_DATATYPE_SLE_EVENT = 0xA2,
    HCI_DATATYPE_SLE_ACB = 0xA3,
    HCI_DATATYPE_SLE_ICB = 0xA4,
};

/**
 * @brief 发送HCI数据给CHBA
 *
 * @param data
 * @param data_len
 * @return uint32_t 0 - 需要继续上报给 SLE host   1 - CHBA 链路数据 不用继续上报 SLE host
 */
uint32_t sle_hci_send_to_chba(uint8_t *data, uint16_t data_len);
uint32_t sle_hci_recv_from_chba(uint8_t *data, uint16_t data_len);
void hci_add_logic_link_info(sle_link_info_t link_info);
void hci_kernel_init(void);
void sle_hci_register_send_hcc_interface(int (*send)(uint8_t *, uint16_t));
void sle_hci_register_chba_recv_interface(int (*recv)(uint8_t *, uint16_t));

#endif /* SLE_HCI_CHBA_PROC_H */