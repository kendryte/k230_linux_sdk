/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag lib dbg
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_LIB_DBG_H__
#define __ZDIAG_LIB_DBG_H__

#include "td_base.h"
#include "osal_types.h"

/* refactored based on macros OAM_xxx(DEBUG\INFO\WARNING\ERROR) */
#if defined(CONFIG_DFX_SUPPORT_DEBUG) && (CONFIG_DFX_SUPPORT_DEBUG != 0)

extern td_u8 g_zdiag_dfx_dbg_lvl;

#define oam_debug(fmt, arg...)                                               \
    do {                                                                     \
        if (g_zdiag_dfx_dbg_lvl >= PLAT_LOG_DEBUG) {                         \
            printk_ratelimited("[OAM]D]%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg);      \
        }                                                                    \
    } while (0)

#define oam_info(fmt, arg...)                                                \
    do {                                                                     \
        if (g_zdiag_dfx_dbg_lvl >= PLAT_LOG_INFO) {                          \
            printk_ratelimited("[OAM]I]%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg);      \
        }                                                                    \
    } while (0)

#define oam_warn(fmt, arg...)                                                \
    do {                                                                     \
        if (g_zdiag_dfx_dbg_lvl >= PLAT_LOG_WARNING) {                       \
            printk_ratelimited("[OAM]W]%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg);      \
        }                                                                    \
    } while (0)

#define oam_error(fmt, arg...)                                               \
    do {                                                                     \
        if (g_zdiag_dfx_dbg_lvl >= PLAT_LOG_ERR) {                           \
            printk_ratelimited("[OAM]E]%s:%d]" fmt, __FUNCTION__, __LINE__, ##arg);      \
        }                                                                    \
    } while (0)
#else
#define oam_debug(fmt, arg...)
#define oam_info(fmt, arg...)
#define oam_warn(fmt, arg...)
#define oam_error(fmt, arg...)
#endif

osal_s32 zdiag_dbg_sysfs_init(osal_void);
osal_s32 zdiag_dbg_sysfs_exit(osal_void);

#endif
