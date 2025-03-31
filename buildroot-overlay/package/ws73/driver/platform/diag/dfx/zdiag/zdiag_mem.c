/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag mem
 * This file should be changed only infrequently and with great care.
 */

#include "zdiag_mem.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_dfx.h"

typedef struct {
    td_u16 type;
    td_u16 size;
} zdiag_mem_head;

td_pvoid diag_malloc(diag_mem_type type, td_u32 size)
{
    zdiag_mem_head *head = TD_NULL;
    td_u8 *ptr = diag_adapt_malloc(size + sizeof(zdiag_mem_head));
    if (ptr == TD_NULL) {
        return TD_NULL;
    }

    head = (zdiag_mem_head *)ptr;
    head->size = (td_u16)size;
    head->type = type;

    return ptr + sizeof(zdiag_mem_head);
}

td_void diag_free(td_pvoid p)
{
    td_u8 *ptr = (td_u8 *)p - sizeof(zdiag_mem_head);
    diag_adapt_free(ptr);
    return;
}

td_void diag_pkt_malloc(diag_pkt_malloc_param *param, diag_pkt_malloc_result *result)
{
    if (param->basic.size == 0) {
        result->basic.buf = TD_NULL;
        result->basic.size = 0;
        return;
    }

    diag_adapt_pkt_malloc(param, result);
    if (result->basic.buf) {
        diag_dfx_alloc_pkt(result->basic.type, result->basic.size);
    }
}

td_void diag_pkt_free(diag_pkt_malloc_result *result)
{
    if (result->basic.buf == TD_NULL || result->basic.size == 0) {
        return;
    }
    
    diag_dfx_free_pkt(result->basic.type, result->basic.size);
    diag_adapt_pkt_free(result);
    result->basic.buf = TD_NULL;
    result->basic.size = 0;
}
