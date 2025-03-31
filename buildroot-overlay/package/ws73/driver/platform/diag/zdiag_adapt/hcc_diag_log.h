/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag hcc log
 * This file should be changed only infrequently and with great care.
 */
#ifndef __HCC_DIAG_LOG_H__
#define __HCC_DIAG_LOG_H__

#include "oam_ext_if.h"

typedef struct {
    td_u32 start_flag; /* start flag fixed 0xDEADBEAF */
    td_u32 au_id;      /* MSP AUID. */
    td_u8 type;        /* ind:0,ack:2,system ack:3 */
    td_u8 ver;         /* diag cmd version,0xff:old version,0xfe:crc16version,0xfd:nb version */
    td_u16 cmd_id;     /* mark unencrypted cmd id only 0x5000,0x5002,0x5003 */

    td_u16 crc16; /* crc16 */

    td_u8 dst;
    td_u8 src;

    td_u8 ctrl;
    td_u8 pad[1];
    td_u16 packet_data_size; /* puc_packet_data size, not include sizeof(MSP_MUX_PACKET_HEAD_STRU). */

    td_u8 puc_packet_data[0]; /* data:Encrypted means encrypted data, otherwise plain text */
} msp_mux_packet_head_stru_device;

td_s32  hcc_diag_log_init(td_void);
td_void hcc_diag_log_exit(td_void);

#endif /* end of __HCC_DIAG_LOG_H__ */
