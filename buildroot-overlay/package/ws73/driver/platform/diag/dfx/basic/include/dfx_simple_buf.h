/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx simplebuf
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_SIMPLE_BUF_H__
#define __DFX_SIMPLE_BUF_H__
#include "td_type.h"
#include "td_base.h"

typedef struct {
    td_u8 *data;
    td_u32 total_size;
    td_u32 w_pos;
    td_u32 r_pos;
} dfx_simple_buf_handle;

td_void dfx_simple_buf_init(dfx_simple_buf_handle *handle, td_u8 *data, td_u32 size);
td_u32 dfx_simple_buf_get_free_size(dfx_simple_buf_handle *handle);
td_s32 dfx_simple_buf_write(dfx_simple_buf_handle *handle, td_u8 *data, td_u32 len);
td_s32 dfx_simple_buf_read(dfx_simple_buf_handle *handle, td_u8 *data, td_u32 len);

#endif