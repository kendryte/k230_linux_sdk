/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc service.
 * Author: CompanyName
 * Create: 2021-09-11
 */

#ifndef HCC_SERVICE_HEADER
#define HCC_SERVICE_HEADER

#include "td_base.h"
#include "hcc_cfg_comm.h"
#include "hcc_comm.h"

hcc_serv_info *hcc_add_service_list(hcc_handler *hcc, hcc_service_type serv_type);
td_void hcc_del_service_list(hcc_handler *hcc, hcc_service_type serv_type);
hcc_serv_info *hcc_get_serv_info(hcc_handler *hcc, hcc_service_type serv_type);
#endif /* HCC_SERVICE_HEADER */
