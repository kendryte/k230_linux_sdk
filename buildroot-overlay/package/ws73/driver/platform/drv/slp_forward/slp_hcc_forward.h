/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: host slp hcc head file
 * Create: 2022-12-27
 */
#ifndef __SLP_HCC_FORWARD_H__
#define __SLP_HCC_FORWARD_H__
#include "soc_osal.h"
#include "osal_list.h"
#include "osal_def.h"
#include "securec.h"

#include "hcc_if.h"
#include "hcc_comm.h"
#include "hcc_service.h"

typedef struct {
    osal_u32 len;
} slp_hcc_adapt;

osal_u32 slp_hcc_init(osal_void);
osal_void slp_hcc_exit(osal_void);

#endif