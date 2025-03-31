/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: plat debug header file
 * Author: Huanghe
 * Create: 2020-10-13
 */

#ifndef __PLAT_DEBUG_H__
#define __PLAT_DEBUG_H__

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "linux/oal_debug.h"
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "liteos/oal_debug.h"
#endif

#endif /* __PLAT_DEBUG_H__ */
