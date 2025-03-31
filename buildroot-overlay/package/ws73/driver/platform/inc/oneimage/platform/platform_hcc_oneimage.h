/*
 * Copyright (c) CompanyNameMagicTag. 2020-2023. All rights reserved.
 * Description: platform hcc oneimage file
 * Author: CompanyName
 * Create: 2023-06-27
 */

#ifndef __PLATFORM_HCC_ONEIMAGE_H__
#define __PLATFORM_HCC_ONEIMAGE_H__

#define hcc_get_state                 platform_oneimage_rename(hcc_get_state)
#define hcc_init                      platform_oneimage_rename(hcc_init)
#define hcc_deinit                    platform_oneimage_rename(hcc_deinit)

#endif /* #ifndef __PLATFORM_HCC_ONEIMAGE_H__ */