/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx fault event
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_FAULT_EVENT_H__
#define __DFX_FAULT_EVENT_H__
#include "td_type.h"
#include "td_base.h"

/* naming rule:FAULT + sub system name + module name + other name */
typedef enum {
    FAULT_DFX_MONITOR_REGISTER_FAIL,
    FAULT_DFX_MONITOR_INIT_FAIL,
    FAULT_DFX_DIAG_REGISTER_CMD_FAIL,
} dfx_fault_event_id;

td_void dfx_fault_event_data(td_u32 event_id, td_u8 *data, td_u16 len);
#endif