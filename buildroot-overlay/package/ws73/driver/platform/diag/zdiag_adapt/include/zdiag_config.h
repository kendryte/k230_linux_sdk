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
#define CONFIG_DIAG_IND_TBL_NUM 3  /* Maximum number of non-system ind lists */
#define CONFIG_DIAG_RX_BUF_SIZE 512
#define CONFIG_STAT_CMD_LIST_NUM 10 /* Maximum number of statistics list */

typedef enum {
    DIAG_CHANNEL_ID_0,
    DIAG_CHANNEL_ID_1,
    DIAG_CHANNEL_ID_2,
    DIAG_SUPPORT_CHANNEL_CNT,
    DIAG_CHANNEL_ID_INVALID = 0xFF,
} diag_channel_id;

typedef enum {
    SOC_SAVE_FILE_PC,
    SOC_SAVE_FILE_EMMC,
    SOC_SAVE_FILE_DEFAULT = SOC_SAVE_FILE_EMMC,
} soc_save_file_type;

typedef td_u8 zdiag_addr;


#define FEATURE_YES 1
#define FEATURE_NO 0

#if defined(CONFIG_APPS_CORE)
#define CONFIG_FEATURE_SUPPORT_LOCAL_EMMC
#else
#define CONFIG_FEATURE_SUPPORT_REMOTE_EMMC
#endif

#if defined(CONFIG_APPS_CORE)
#define CONFIG_FEATURE_SUPPORT_UP_MACHINE
#else
#endif

#define CONFIG_ZDIAG_CACHE_VAL_DEFAULT 1
#define CONFIG_ZDIAG_CACHE_VAL_IPC 2
#define CONFIG_ZDIAG_VAL CONFIG_ZDIAG_CACHE_VAL_DEFAULT

#endif
