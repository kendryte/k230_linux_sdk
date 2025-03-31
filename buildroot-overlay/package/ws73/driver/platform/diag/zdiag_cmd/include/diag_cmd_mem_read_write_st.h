/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: diag cmd mem read write st.
 * This file should be changed only infrequently and with great care.
 */
#ifndef DIAG_CMD_MEM_READ_WRITE_ST_H
#define DIAG_CMD_MEM_READ_WRITE_ST_H
#include "td_type.h"

typedef struct {
    uintptr_t start_addr;
    td_u32 cnt;
} mem_read_cmd_t;

typedef struct {
    uintptr_t start_addr;
    td_u32 size;
} mem_read_ind_head_t;

typedef struct {
    mem_read_ind_head_t head;
    td_u32 data[0];
} mem_read32_ind_t;

typedef struct {
    mem_read_ind_head_t head;
    td_u16 data[0];
} mem_read16_ind_t;

typedef struct {
    mem_read_ind_head_t head;
    td_u8 data[0];
} mem_read8_ind_t;

typedef struct {
    uintptr_t start_addr;
    td_u32 val;
} mem_write_cmd_t;

typedef struct {
    td_u32 ret;
} mem_write_ind_t;
#endif