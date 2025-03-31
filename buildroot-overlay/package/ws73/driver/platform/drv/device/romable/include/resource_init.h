/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: RESOURCE INIT HEADER.
 */

#ifndef __RESOURCE_INIT_H__
#define __RESOURCE_INIT_H__
#include <td_base.h>

typedef struct {
    td_u32 reg_base_addr;
    td_u32 irq_number; /* done and err are the same irq num */
    td_u32 channel_max_num;
    td_u32 peripheral_max_num;
    td_u32 lowest_priority;
} ext_dma_device;

td_void dma_resource_init(td_void);
td_void uapi_malloc_resource_init(td_void);
td_void hrtimer_resource_init(td_void);
td_void uapi_tsensor_resource_init(td_void);
td_void uapi_io_resource_init(td_void);
td_void io_pulldown_cfg(td_void);
td_void pmu_cmu_vset_cfg(td_void);
td_void init_uart_pinmux_io(td_void);

#endif

