/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */


#include <asm/cacheflush.h>
#include <asm/shmparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/dma-direction.h>
#include "soc_osal.h"

#ifdef OSAL_API_SUPPORT_DCACHE

#ifdef __aarch64__
static void osal_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long size)
{
    /* flush dcache byaddr */
    __flush_dcache_area(kvirt, (size_t)size);
}
#else
static void osal_cpuc_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long size)
{
    /* flush dcache byaddr */
    __cpuc_flush_dcache_area(kvirt, (size_t)size);
}

static void osal_outer_flush_range(void *kvirt, unsigned long phys_addr, unsigned long size)
{
    /* flush dcache by phys_addr */
    if (phys_addr) {
        outer_flush_range(phys_addr, phys_addr + size);
    }
}
#endif

void osal_dcache_region_wb(void *kvirt, unsigned long phys_addr, unsigned long size)
{
#ifdef __aarch64__
    osal_flush_dcache_area(kvirt, phys_addr, size);
#else
    osal_cpuc_flush_dcache_area(kvirt, phys_addr, size);
    osal_outer_flush_range(kvirt, phys_addr, phys_addr + size);
#endif
    mb();
}
EXPORT_SYMBOL(osal_dcache_region_wb);

void osal_dcache_region_inv(void *addr, unsigned long size)
{
    return;
}
EXPORT_SYMBOL(osal_dcache_region_inv);
#endif
