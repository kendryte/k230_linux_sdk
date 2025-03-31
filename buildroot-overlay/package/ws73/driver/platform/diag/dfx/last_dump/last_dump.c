/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: last dump
 * This file should be changed only infrequently and with great care.
 */

#include "last_dump.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "dfx_adapt_layer.h"
#include "zdiag_adapt_layer.h"
#include "last_dump_st.h"
#include "securec.h"

#define DIAG_CMD_ID_LAST_DUMP        0x71A4
#define DIAG_CMD_ID_LAST_DUMP_FINISH 0x71A5

#define LAST_DUMP_PER_SIZE (4 * 1024)

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void dfx_last_dump(char *name, uintptr_t addr, uint32_t size)
{
    last_dump_data_ind_t ind = { 0 };
    last_dump_data_ind_finish_t ind_f = { 0 };
    uint32_t offset = 0;
    uint16_t size_len;

    if (strncpy_s(ind.name, LAST_DUMP_NAME_LEN, name, strlen(name)) != EOK) {
        zdiag_printf("dfx_last_dump(%s) error \r\n", name);
        return;
    }
    if (strncpy_s(ind_f.name, LAST_DUMP_NAME_LEN, name, strlen(name)) != EOK) {
        zdiag_printf("dfx_last_dump(%s) ind_f error \r\n", name);
        return;
    }

    size_len = size;
    ind.total_size = (uint32_t)(sizeof(last_dump_data_ind_t) + size_len);
    while (size_len) {
        uint16_t per_size = uapi_min(LAST_DUMP_PER_SIZE, size_len);
        zdiag_printf(KERN_INFO "test per_size: %d\n", per_size);
        ind.offset = offset;
        ind.size = per_size;
        zdiag_report_packet_with_buf(DIAG_CMD_ID_LAST_DUMP, (td_u8 *)&ind, sizeof(last_dump_data_ind_t), (td_u8 *)addr,
            per_size);
        addr += per_size;
        size_len -= per_size;
        offset += per_size;
    }

    ind_f.total_size = (uint32_t)(sizeof(last_dump_data_ind_finish_t));
    zdiag_printf(KERN_INFO "ind_f.total_size: %d\n", ind_f.total_size);
    zdiag_report_packet_with_buf(DIAG_CMD_ID_LAST_DUMP_FINISH, (td_u8 *)&ind_f, sizeof(last_dump_data_ind_finish_t),
        NULL, 0);
}
#endif
