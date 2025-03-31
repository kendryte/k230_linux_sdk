/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: dfx monitor
 * This file should be changed only infrequently and with great care.
 */

#ifndef __DFX_MONITOR_H__
#define __DFX_MONITOR_H__
#include "soc_monitor.h"
#include "dfx_feature_config.h"
td_void dfx_monitor_body(td_void);
ext_errno dfx_monitor_init(td_u8 cnt);
#endif