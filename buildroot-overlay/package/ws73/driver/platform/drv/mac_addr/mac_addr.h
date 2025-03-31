/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: wal common msg api.
 * Author: Huanghe
 * Create: 2021-08-16
 */

#ifndef __MAC_ADDR_H__
#define __MAC_ADDR_H__
#include <linux/string.h>

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_COMMON_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IFTYPE_BLE 0xF1
#define IFTYPE_SLE 0xF2
typedef unsigned int(*mac_derivation_ptr)(unsigned char* origin_mac, unsigned char num,
    unsigned char type, unsigned char *output_mac, unsigned char out_put_num);
void set_mac_derivation_ptr(mac_derivation_ptr ptr);
void init_dev_addr(void);
unsigned int get_dev_addr(unsigned char *pc_addr, unsigned char addr_len, unsigned char type);
unsigned int set_dev_addr(const unsigned char *pc_addr, unsigned char mac_len, unsigned char type);
unsigned int plat_parse_mac_addr(unsigned char *cust_param_info, unsigned char *coe_params,
    unsigned char *param_num_val, unsigned char max_idx);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
