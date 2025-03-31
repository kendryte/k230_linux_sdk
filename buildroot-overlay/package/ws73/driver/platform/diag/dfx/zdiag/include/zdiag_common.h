/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag common header file
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_COMMON_H__
#define __ZDIAG_COMMON_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_config.h"

#define MUX_START_FLAG 0xDEADBEEF
#define diag_make_option(ctrl, src) (td_u32)(((td_u32)(src)) << 8 | ((td_u32)(ctrl)))
#define MUX_PKT_VER 1
#define TEMP_HCC_HEAD_SIZE 12
#define DIAG_MSG_CMD_ID 0


typedef enum {
    DIAG_MSG_DIAG_PKT = 1,
    DIAG_MSG_LOG_OUT_PUT = 2,
    DIAG_MSG_TRANSMIT,
} diag_msg_id;

typedef enum {
    ZDIAG_RX_PKT_TYPE_COPY_MUX,
    ZDIAG_RX_PKT_TYPE_ZCOPY_HCC,
    ZDIAG_RX_PKT_TYPE_ZCOPY_CHAR_HCC,
} zdiag_rx_pkt_type;

typedef enum {
    ZDIAG_PKT_TYPE_CMD = 0,
    ZDIAG_PKT_TYPE_IND = 1,
    ZDIAG_PKT_TYPE_MSG = 2,
    ZDIAG_PKT_TYPE_FLAG_HCC = 0x10,
    ZDIAG_PKT_TYPE_FLAG_INNER = 0x20,
} zdiag_pkt_main_type;

typedef enum {
    DIAG_IND_ALWAYS,
} diag_ind_level;

typedef enum {
    MUX_PKT_CMD = 0,
    MUX_PKT_IND = 1,
    MUX_PKT_ACK = 3,
} mux_pkt_type;

typedef enum {
    DIAG_PKT_PROC_USR_ASYNC_CMD_IND,
    DIAG_PKT_PROC_USR_SYNC_CMD_IND,
    DIAG_PKT_PROC_PORT_PKT,
    DIAG_PKT_PPOC_LOCAL_EXEC_Q,
    DIAG_PKT_PPOC_OUT_PUT_Q,
} diag_pkt_proc;

typedef enum {
    DIAG_ADDR_ATTRIBUTE_VALID = 0x1,
    DIAG_ADDR_ATTRIBUTE_HCC = 0x2,
    DIAG_ADDR_ATTRIBUTE_HSO_CONNECT = 0x4,
} diag_addr_attribute;

typedef enum {
    DIAG_PKT_BUF_TYPE_IPC_BUF = 1,
} diag_pkt_buf_type;

typedef struct {
    td_u32 module;   /* source module ID */
    td_u32 dest_mod; /* destination module ID */
    td_u32 no;       /* No. */
    td_u32 id;       /* ID */
    td_u32 time;
    td_s8 data[0]; /* User Data Cache */
} diag_cmd_log_layer_ind_stru;

typedef struct {
    td_u16 sn;    /* cmd sn */
    td_u16 crc16; /* crc16 */

    td_u16 cmd_id;     /* cmd id */
    td_u16 param_size; /* param size */
    td_u8 param[0];    /* data */
} msp_diag_head_req_stru;

typedef struct {
    td_u16 cmd_id;     /* cmd id */
    td_u16 param_size; /* param size */
    td_u8 param[0];    /* data */
} msp_diag_head_ind_stru;

typedef struct {
    td_u16 sn;  /* cmd sn */
    td_u8 ctrl; /* auto ack:0xfe,initiative ack:0 */
    td_u8 pad;
    td_u16 cmd_id;     /* cmd id */
    td_u16 param_size; /* param size */
    td_u8 param[0];    /* data */
} msp_diag_head_cnf_stru;

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
} msp_mux_packet_head_stru;

typedef struct {
    td_u16 mux_size;
    td_u8 need_crc_2_hso : 1;
    td_u8 pkt_type : 7; // zdiag_pkt_main_type
    td_u8 cur_proc;     // diag_pkt_proc
    td_u8 *pkt_buf;
    td_u8 *ipc_msg_info;
} diag_pkt_ctrl;

typedef struct {
    td_u32 need_free_pkt;
    td_u32 pad : 31;
    msp_mux_packet_head_stru *mux_head;
} pkt_router_param;
#endif
