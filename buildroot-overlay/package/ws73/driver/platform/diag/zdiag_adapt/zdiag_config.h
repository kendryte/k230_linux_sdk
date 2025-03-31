/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag usr config
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_CONFIG_H__
#define __ZDIAG_CONFIG_H__
#include "td_base.h"
#include "soc_errno.h"

#define DIAG_CONFIG_FILE_INCLUDE

#define CONFIG_DIAG_CMD_TBL_NUM 10 /* Maximum number of non-system command lists */
#define CONFIG_DIAG_IND_TBL_NUM 1  /* Maximum number of non-system command lists */
#define CONFIG_DIAG_RX_BUF_SIZE 512
#define CONFIG_STAT_CMD_LIST_NUM 10 /* Maximum number of statistics list */

typedef enum {
    DIAG_CHANNEL_ID_0,
    DIAG_CHANNEL_ID_1,
    DIAG_CHANNEL_ID_2,
    DIAG_SUPPORT_CHANNEL_CNT,
    DIAG_CHANNEL_ID_INVALID = 0xFF,
} diag_channel_id;

typedef td_u8 zdiag_addr;


#define CONFIG_ZDIAG_CACHE_VAL_DEFAULT 1
#define CONFIG_ZDIAG_CACHE_VAL_IPC 2

#define FEATURE_YES 1
#define FEATURE_NO 0

#if defined(CONFIG_PROT_CORE)
#define CONFIG_ZDIAG_VAL CONFIG_ZDIAG_CACHE_VAL_IPC
#else
#define CONFIG_ZDIAG_VAL CONFIG_ZDIAG_CACHE_VAL_DEFAULT
#endif

#endif
