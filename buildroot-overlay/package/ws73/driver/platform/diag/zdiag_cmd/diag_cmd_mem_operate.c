/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag mem cmd.
 * This file should be changed only infrequently and with great care.
 */

#include "diag_cmd_mem_operate.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "soc_diag_cmd.h"
#include "td_base.h"
#include "zdiag_adapt_layer.h"

ext_errno diag_cmd_read_mem(td_u16 cmd_id, td_pvoid cmd_param, td_u16 param_size, diag_option *option)
{
    td_u32 ret = 0;
    read_mem *param = (read_mem *)cmd_param;
    read_mem_ind *ack = (read_mem_ind *)diag_adapt_malloc(sizeof(read_mem_ind));
    td_u32 left = param->len;
    td_u32 saved_num = 0;
    td_u32 i;

    uapi_unused(param_size);
    if (ack == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    while (left > 0) {
        if (left < DIAG_READ_MEN_LEN) {
            for (i = 0; i < left; i++) {
                ack->data[i] = *((td_u32 *)(uintptr_t)param->addr + saved_num + i);
            }
            ack->len = left;
            ret |= (td_u32)uapi_zdiag_report_packet(cmd_id, option, (td_pbyte)ack,
                                                    (td_u16)sizeof(read_mem_ind), TD_TRUE);
            break;
        }
        for (i = 0; i < DIAG_READ_MEN_LEN; i++) {
            ack->data[i] = *((td_u32 *)(uintptr_t)param->addr + saved_num + i);
        }
        ack->len = DIAG_READ_MEN_LEN;
        ret |= uapi_zdiag_report_packet(cmd_id, option, (td_pbyte)ack, (td_u16)sizeof(read_mem_ind), TD_TRUE);
        left -= DIAG_READ_MEN_LEN;
        saved_num += DIAG_READ_MEN_LEN;
    }
    return ret;
}

td_u32 diag_read_mem_range(td_u32 start_addr, td_u32 end_addr, td_u16 cmd_id, diag_option *option)
{
    td_u32 ret = EXT_ERR_SUCCESS;
    read_mem_ind *ack = (read_mem_ind *)diag_adapt_malloc(sizeof(read_mem_ind));
    td_u32 current_addr = start_addr;

    if (start_addr > end_addr) {
        return EXT_ERR_INVALID_PARAMETER;
    }
    while (current_addr <= end_addr) {
        td_u32 i;
        for (i = 0; i < DIAG_READ_MEN_LEN; i++) {
            if (current_addr > end_addr) {
                break;
            }
            ack->data[i] = *((td_u32 *)(uintptr_t)current_addr);
            current_addr = current_addr + (td_u16)sizeof(td_u32);
        }

        if (i == DIAG_READ_MEN_LEN) {
            ack->len = DIAG_READ_MEN_LEN;
        } else {
            ack->len = i;
        }
        ret |= (td_u32)uapi_zdiag_report_packet(cmd_id, option, (td_pbyte)ack, (td_u16)sizeof(read_mem_ind), TD_TRUE);
    }
    return ret;
}

td_u32 diag_cmd_cycle_read_mem(td_u16 cmd_id, td_pvoid cmd_param, td_u16 param_size, diag_option *option)
{
    td_u32 ret;
    cycle_read_mem *param = (cycle_read_mem *)cmd_param;
    td_u32 stop_addr = *((td_u32 *)CYCLE_READ_MEM_STOP_REGISTER);
    /* address space = end address - start address + 1 */
    td_u32 addr_range = param->end_addr - param->start_addr + 1;
    /* in cycle read memory, it is a unit of 4 bytes. */
    td_u16 read_count = uapi_lou16(*((td_u32 *)CYCLE_READ_MEM_COUNT_REGISTER)) * 4;
    td_u32 next_start_addr = stop_addr;
    td_u32 count = 0;

    uapi_unused(param_size);
    if (read_count <= addr_range) {
        /* address range: [start, stop) */
        ret = diag_read_mem_range(param->start_addr, stop_addr - 1, cmd_id, option);
        return ret;
    } else {
        /* 31 bits as 1 represents the beginning of a cycle. */
        while (((*((td_u32 *)(uintptr_t)next_start_addr) >> 31) & 0x1) != 1) {
            next_start_addr =
                (next_start_addr < param->end_addr) ? next_start_addr + sizeof(td_u32) : param->start_addr;
            if (count > CYCLE_READ_MEM_MAX_CYCLE) {
                return EXT_ERR_FAILURE;
            }
            count++;
        }
    }
    if (next_start_addr >= stop_addr) {
        ret = diag_read_mem_range(next_start_addr, param->end_addr, cmd_id, option);
        ret = diag_read_mem_range(param->start_addr, stop_addr - 1, cmd_id, option);
    } else {
        ret = diag_read_mem_range(next_start_addr, stop_addr - 1, cmd_id, option);
    }
    return ret;
}

td_u32 diag_cmd_mem_operate(td_u16 cmd_id, td_pvoid cmd_param, td_u16 param_size, diag_option *option)
{
    switch (cmd_id) {
        case ID_DIAG_CMD_READ_MEM:
            return diag_cmd_read_mem(cmd_id, cmd_param, param_size, option);
        case ID_DIAG_CMD_CYCLE_READ_MEM:
            return diag_cmd_cycle_read_mem(cmd_id, cmd_param, param_size, option);
        default:
            return EXT_ERR_NOT_SUPPORT;
    }
}
