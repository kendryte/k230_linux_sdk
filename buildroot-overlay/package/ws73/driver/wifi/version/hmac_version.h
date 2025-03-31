/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: define hmac version
 * Create: 2021-9-28
 */

#ifndef __HMAC_VERSION_H__
#define __HMAC_VERSION_H__

#include "osal_types.h"

/*
 * WSxx [  a  ].[  c  ].T[  c  ].[  d  ]
 * WSxx [1-255].[1-255].T[1-255].[1-255]
 */
typedef union {
    osal_u32 ver;
    struct {
        osal_u32 d : 8; /* [ 7,  0] */
        osal_u32 c : 8; /* [15,  8] */
        osal_u32 b : 8; /* [23, 16] */
        osal_u32 a : 8; /* [31, 24] */
    } bits;
} version_u;

typedef union {
    osal_u32 ver;
    struct {
        osal_u32 c : 8; /* [7..0] */
        osal_u32 r : 8; /* [15..8] */
        osal_u32 v : 16; /* [31..16] */
    } bits;
} rom_version_u;

typedef struct {
    version_u host;
    version_u device;
    rom_version_u device_rom;
} version_t;

osal_u32 hmac_get_version(osal_void);
osal_u32 hmac_get_device_version(osal_void);
osal_u32 hmac_get_device_rom_version(osal_void);

#endif

