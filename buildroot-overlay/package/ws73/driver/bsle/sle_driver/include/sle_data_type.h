/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_DATA_TYPE_H
#define SLE_DATA_TYPE_H
#include "td_base.h"

#define BD_ADDR_LEN 6 /*!< MAC地址长度 */

typedef struct {
    uint8_t type; /*!< 地址类型，参见addr_type定义 */
    uint8_t addr[BD_ADDR_LEN];
} sle_addr_t;

typedef struct sle_link_info {
    uint16_t lcid;
    uint16_t tcid;
    sle_addr_t addr;
} sle_link_info_t;

#endif /* SLE_DATA_TYPE_H */