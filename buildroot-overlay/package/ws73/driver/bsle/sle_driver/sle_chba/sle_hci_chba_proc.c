/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include "sle_hci_chba_proc.h"
#include "oal_schedule.h"

static hci_link_info_t g_hci_link_info[SLE_MAX_TCID_NUMS];
int (*g_hci_send_msg_func)(uint8_t *, uint16_t) = NULL;
int (*g_hci_chba_recv_func)(uint8_t *, uint16_t) = NULL;

void hci_add_logic_link_info(sle_link_info_t link_info)
{
    uint16_t tcid = 0;
    tcid = link_info.tcid;
    g_hci_link_info[tcid].has_tcid = true;
    memcpy_s(&(g_hci_link_info[tcid].link_info), sizeof(sle_link_info_t),
        &link_info, sizeof(sle_link_info_t));
}

uint16_t hci_get_data_tcid(uint8_t *data, uint16_t data_len)
{
    errno_t ret;
    uint16_t tcid = 0;
    ret = memcpy_s(&tcid, HCI_DATA_TCID_LEN, data + HCI_DATA_TCID_POS, HCI_DATA_TCID_LEN);
    if (ret != 0) {
        oal_print_err("hci_get_data_tcid memcpy_s is fail\n");
        return false;
    }
    return tcid;
}

void hci_kernel_init(void)
{
    memset_s(&g_hci_link_info, sizeof(hci_link_info_t) * SLE_MAX_TCID_NUMS,
        0, sizeof(hci_link_info_t) * SLE_MAX_TCID_NUMS);
}

void sle_hci_register_send_hcc_interface(int (*send)(uint8_t *, uint16_t))
{
    g_hci_send_msg_func = send;
}

void sle_hci_register_chba_recv_interface(int (*recv)(uint8_t *, uint16_t))
{
    g_hci_chba_recv_func = recv;
}

uint32_t sle_hci_recv_from_chba(uint8_t *data, uint16_t data_len)
{
    if (g_hci_send_msg_func == NULL) {
        oal_print_err("g_hci_send_msg_func is NULL\n");
        return EXT_ERR_FAILURE;
    }
    g_hci_send_msg_func(data, data_len);
    return EXT_ERR_SUCCESS;
}

uint32_t sle_hci_send_to_chba(uint8_t *data, uint16_t data_len)
{
    if (g_hci_chba_recv_func != NULL) {
        return g_hci_chba_recv_func(data, data_len);
    }
    return 0;
}