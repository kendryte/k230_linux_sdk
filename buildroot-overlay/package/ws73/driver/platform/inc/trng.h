/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: trng interface
 * Author: Huanghe
 * Create: 2023-03-31
 */

#ifndef TRNG_H
#define TRNG_H

#include "td_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

td_u32 uapi_drv_cipher_trng_get_random_bytes(td_u8 *randnum, td_u32 size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
