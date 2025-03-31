/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: print base define header file.
 * Author: Huanghe
 * Create: 2023-01-01
 */

#ifndef __OAL_DEBUG_BASE_H__
#define __OAL_DEBUG_BASE_H__

#include "td_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

enum PLAT_LOGLEVLE {
    PLAT_LOG_EMERG = 0,
    PLAT_LOG_ALERT = 1,
    PLAT_LOG_CRIT = 2,
    PLAT_LOG_ERR = 3,
    PLAT_LOG_WARNING = 4,
    PLAT_LOG_NOTICE = 5,
    PLAT_LOG_INFO = 6,
    PLAT_LOG_DEBUG = 7,
};

enum BUG_ON_CTRL {
    BUG_ON_DISABLE = 0,
    BUG_ON_ENABLE  = 1,
};

extern volatile td_s32 g_plat_loglevel_etc;
extern td_s32 g_bug_on_enable_etc;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_debug_base.h */

