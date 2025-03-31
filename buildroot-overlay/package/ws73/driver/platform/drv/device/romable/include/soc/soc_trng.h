/*
 * Copyright (c) CompanyNameMagicTag. 2020-2020. All rights reserved.
 * Description: trng header file.
 * Author: Huanghe
 * Create: 2020-01-01
 */

#ifndef __SOC_TRNG_H__
#define __SOC_TRNG_H__

#include <td_type.h>

td_u32 uapi_drv_cipher_trng_get_random(td_u32 *val);
td_u32 uapi_drv_cipher_trng_get_random_bytes(td_u8 *val, td_u32 size);
#ifdef HH503_WIFI
td_void trng_init(td_void);
#endif

#endif

