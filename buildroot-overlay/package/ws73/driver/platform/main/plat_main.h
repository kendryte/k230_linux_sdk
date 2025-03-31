/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: plat_main header file.
 * Author: Huanghe
 * Create: 2020-09-01
 */

#ifndef __PLAT_MAIN_H__
#define __PLAT_MAIN_H__

#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_MAIN_H

osal_s32 plat_init_etc(void);
void plat_exit_etc(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
