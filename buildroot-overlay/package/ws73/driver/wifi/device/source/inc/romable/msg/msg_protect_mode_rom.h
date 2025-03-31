/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_PROTECT_MODE_ROM_H
#define MSG_PROTECT_MODE_ROM_H

#include "osal_types.h"

typedef struct {
    osal_u8 erp_mode;
    osal_u8 resv[3]; /* 保留3字节对齐 */
} mac_protection_sync_stru;

#endif