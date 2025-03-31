/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: head file of trng driver
 * Author: CompanyName
 * Create: 2020-02-24
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#include "td_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */
#define TIMER_ENABLE_FREE_MODE 0x80
#define CTL_OFFSET_ADDR  0x8
#define VALUE_OFFSET_ADDR  0x4
#define MAX_DELAY_TICK 0xffffffff
#define MHZ_TO_HZ   1000000
#define MS_TO_US    1000

td_void uapi_delay_init(td_u32 hrtimer_addr, td_u32 freq);
td_u32 uapi_udelay(td_u32 delayus);
td_u32 uapi_mdelay(td_u32 delayms);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */

#endif /* CIPHER_TRNG_H */
