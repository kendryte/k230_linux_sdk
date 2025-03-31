/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: platform oneimage file
 * Create: 2020-10-13
 */

#ifndef __PLATFORM_SECURITY_ONEIMAGE_H__
#define __PLATFORM_SECURITY_ONEIMAGE_H__

#define memcpy_s                    platform_oneimage_rename(memcpy_s)
#define memset_s                    platform_oneimage_rename(memset_s)
#define snprintf_s                  platform_oneimage_rename(snprintf_s)
#define vsnprintf_s                 platform_oneimage_rename(vsnprintf_s)
#define vsnprintf_truncated_s       platform_oneimage_rename(vsnprintf_truncated_s)
#define GetHwSecureCVersion         platform_oneimage_rename(GetHwSecureCVersion)
#define snprintf_truncated_s        platform_oneimage_rename(snprintf_truncated_s)
#define sprintf_s                   platform_oneimage_rename(sprintf_s)
#define sscanf_s                    platform_oneimage_rename(sscanf_s)
#define strcat_s                    platform_oneimage_rename(strcat_s)
#define strcpy_s                    platform_oneimage_rename(strcpy_s)
#define strncat_s                   platform_oneimage_rename(strncat_s)
#define strncpy_s                   platform_oneimage_rename(strncpy_s)
#define strtok_s                    platform_oneimage_rename(strtok_s)
#define memmove_s                   platform_oneimage_rename(memmove_s)
#define vsprintf_s                  platform_oneimage_rename(vsprintf_s)
#define vsscanf_s                   platform_oneimage_rename(vsscanf_s)

#endif /* #ifndef __PLATFORM_SECURITY_ONEIMAGE_H__ */