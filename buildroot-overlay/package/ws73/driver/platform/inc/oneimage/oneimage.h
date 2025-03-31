/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: platform oneimage file
 * Create: 2020-10-13
 */

#ifndef __ONEIMAGE_H__
#define __ONEIMAGE_H__

#include "platform/platform_linux_osal_oneimage.h"
#if !defined CONFIG_NO_SECUREC
#include "platform/platform_security_oneimage.h"
#endif
#include "platform/platform_linux_oal_oneimage.h"
#include "platform/platform_hcc_oneimage.h"

#define platform_oneimage_rename(NAME)  NAME##_ws73

#endif /* #ifndef __ONEIMAGE_H__ */