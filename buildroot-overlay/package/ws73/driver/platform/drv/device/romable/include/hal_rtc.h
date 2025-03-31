/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: HAL RTC header file
 * Author: CompanyName
 * Create: 2020-08-11
 */
#ifndef __HAL_RTC_H__
#define __HAL_RTC_H__

#include "td_base.h"

td_u64 hal_rtc_get_timestamp_us(td_void);

td_u64 hal_rtc_get_timestamp_cycle(td_void);

#endif
