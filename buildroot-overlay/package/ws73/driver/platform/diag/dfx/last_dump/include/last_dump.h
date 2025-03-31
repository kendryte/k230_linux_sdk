/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: last dump
 * This file should be changed only infrequently and with great care.
 */

#ifndef __LAST_DUMP_H__
#define __LAST_DUMP_H__

#include "td_type.h"

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void dfx_last_dump(char *name, uintptr_t addr, uint32_t size);
#endif

#endif // !__LAST_DUMP_H__