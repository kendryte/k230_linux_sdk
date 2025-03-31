/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: last dump
 * This file should be changed only infrequently and with great care.
 */

#ifndef __LAST_DUMP_ST_H__
#define __LAST_DUMP_ST_H__

#include "td_type.h"

#define LAST_DUMP_NAME_LEN 64

typedef struct {
    char name[LAST_DUMP_NAME_LEN];
    uint32_t total_size;
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
    uint8_t data[0];
} last_dump_data_ind_t;

typedef struct {
    char name[LAST_DUMP_NAME_LEN];
    uint32_t total_size;
} last_dump_data_ind_finish_t;

#endif