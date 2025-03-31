
/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: plat misc driver header file.
 * Author: CompanyName
 * Create: 2022-11-29
 */

#ifndef __PLAT_MISC__H__
#define __PLAT_MISC__H__

#include "td_base.h"
#include "plat_hcc_msg_type.h"

#define PLAT_TRNG_RANDOM_BYTES_MAX 88

// WS73封装类型
typedef enum {
    CHIP_TYPE_U = 0,
    CHIP_TYPE_S,
    CHIP_TYPE_E,
    CHIP_TYPE_MAX
} device_chip_type;

// WS73总线类型
typedef enum {
    BUS_TYPE_USB = 0,
    BUS_TYPE_SDIO,
    BUS_TYPE_UART,
    BUS_TYPE_MAX
} device_bus_type;

typedef struct {
    td_u32 data_size;
    td_u8 data[PLAT_TRNG_RANDOM_BYTES_MAX];
    osal_bool wait_cond;
    osal_wait wait_r; /* Wait Condition: Random Data Ready */
} plat_msg_ctl_struct;

typedef struct {
    osal_u8 *data;
    osal_u32 data_len;
} plat_data_len_struct;

plat_msg_ctl_struct *plat_get_temp_ctl_ptr(osal_void);

td_u32 plat_trng_get_random_bytes(td_u8 *randnum, td_u32 size, td_u32 *get_size);
td_u32 uapi_drv_cipher_trng_get_random_bytes(td_u8 *randnum, td_u32 size);
osal_u32 send_message_to_device(plat_data_len_struct *input_data, plat_cfg_h2d_msg_type msg_type,
    plat_msg_ctl_struct *ctl_struct, plat_data_len_struct *output_data);
td_u32 uapi_get_unique_code(td_u8 *unique_code);

td_void plat_misc_init(td_void);
td_void plat_misc_exit(td_void);

td_u32 plat_d2h_msg_callback(plat_msg_ctl_struct *msg_ctl, td_u8 *data, td_u16 len);
#endif
