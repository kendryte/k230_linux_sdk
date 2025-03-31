/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc adapt SDIO completion.
 * Author: CompanyName
 * Create: 2021-07-26
 */

#ifndef HCC_ADAPT_SDIO_HEADER
#define HCC_ADAPT_SDIO_HEADER

#ifdef CONFIG_HCC_SUPPORT_SDIO

#include "td_base.h"
#include "hcc_bus_types.h"

hcc_bus *hcc_adapt_sdio_load(td_void);
td_void hcc_adapt_sdio_unload(td_void);
#endif /* CONFIG_HCC_SUPPORT_SDIO */
#endif /* HCC_ADAPT_SDIO_HEADER */
