/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: osal adapt string file
 */
#ifdef _OSAL_LITEOS_SDK_
#include <linux/string.h>
#endif
#include "osal_adapt.h"

int osal_adapt_strncmp(const char *str1, const char *str2, unsigned long size)
{
#ifdef _OSAL_LITEOS_SDK_
    return strncmp(str1, str2, size);
#else /* osal_strncmp_tmp_new 为 临时适配接口 CFBB 已删除 对应接口为 osal_strncmp */
    return osal_strncmp_tmp_new(str1, str2, size);
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(osal_adapt_strncmp);
#endif