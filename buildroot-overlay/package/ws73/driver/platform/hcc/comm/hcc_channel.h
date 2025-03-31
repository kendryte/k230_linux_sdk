/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc channel.
 * Author: CompanyName
 * Create: 2021-09-23
 */

#ifndef HCC_CHANNEL_HEADER
#define HCC_CHANNEL_HEADER

#include "td_base.h"
#include "osal_list.h"
#include "hcc_comm.h"

td_s32 hcc_add_handler(hcc_channel_name channel_name, hcc_handler *hcc);
td_void hcc_delete_handler(hcc_channel_name channel_name);
#endif /* HCC_CHANNEL_HEADER */