/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include "sle_tm_chba_proc.h"
#include "oal_schedule.h"

int (*g_tm_chba_recv_func)(uint8_t *, uint16_t) = NULL;

void sle_tm_get_link_info(uint8_t *data, uint16_t data_len)
{
    sle_link_info_t link_info = {0};
    hci_add_logic_link_info(link_info);
}

void sle_tm_data_pre_proc(uint8_t *data, uint16_t data_len)
{
    if (*data == TM_KERNEL_TYPE_SEND_LINK_INFO) {
        sle_tm_get_link_info(data, data_len);
    }
}

void sle_tm_register_chba_recv_interface(int (*recv)(uint8_t *, uint16_t))
{
    g_tm_chba_recv_func = recv;
}

uint32_t sle_tm_send_to_chba(uint8_t *data, uint16_t data_len)
{
    oal_print_err("send sle tm msg to CHBA, len is %d\n", data_len);
    sle_tm_data_pre_proc(data, data_len);

    if (g_tm_chba_recv_func != NULL) {
        g_tm_chba_recv_func(data, data_len);
    }
    return EXT_ERR_SUCCESS;
}
