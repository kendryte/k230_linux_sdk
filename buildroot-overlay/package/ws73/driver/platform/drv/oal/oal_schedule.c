/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: oal schedule source file.
 * Author: Huanghe
 * Create: 2020-10-13
 */

#include "oal_schedule.h"
#include "osal_list.h"
#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OSAL_LIST_HEAD(g_wakelock_head);

struct osal_list_head *oal_get_wakelock_head(void)
{
    return &g_wakelock_head;
}
EXPORT_SYMBOL(oal_get_wakelock_head);

#endif
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
