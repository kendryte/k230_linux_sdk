/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: define file id and module id
 */

#ifndef __SOC_DIAG_CMD_H__
#define __SOC_DIAG_CMD_H__

#include <td_base.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EXT_DIAG_VER_FULL_PRODUCT_NAME_MAX_SIZE 60 /* VER Release Notes */
#define EXT_DIAG_VER_SDK_PRODUCT_NAME_MAX_SIZE 24  /* SDK version number */
#define EXT_DIAG_VER_BOOT_NAME_MAX_SIZE 8
#define EXT_DIAG_VER_FULL_HW_INFO_MAX_SIZE 40      /* Hardware Information */
#define EXT_DIAG_VER_FULL_DEV_VER_NAME_MAX_SIZE 40 /* Device Information */
#define EXT_DIAG_VER_PRODUCT_BUILD_SEP3_MAX_SIZE 1
#define EXT_DIAG_VER_PRODUCT_BUILD_WD_SIZE 1
#define EXT_DIAG_VER_CHIP_NAME_MAX_SIZE 4
#define MAX_NV_READ_LENGTH 1024 /* 1024 kv max len */
#define HNV_ITEM_MAXLEN 256
#define CYCLE_READ_MEM_STOP_REGISTER 0x80004238
#define CYCLE_READ_MEM_COUNT_REGISTER 0x80020790
#define CYCLE_READ_MEM_MAX_CYCLE 100

#define EXT_CMD_VER 0
#define PRODUCT_CFG_C_VER 0
#define PRODUCT_CFG_API_VER 1
#define PRODUCT_CFG_HAPI_VER 1

#define DIAG_READ_MEN_LEN 80

#define IMEI_SIZE 16
/* diag connect req pub_a */
#define DIAG_CONN_PUB_DATA_SIZE 128
#define DIAG_CONN_USR_NAME_SIZE 20

#define EXT_DIAG_CONN_ERR_OK 0              /* Connection verification succeeded */
#define EXT_DIAG_CONN_ERR_SYS_CALL 1        /* Connection verification system error */
#define EXT_DIAG_CONN_ERR_PASS_WORD_WRONG 2 /* Connection verification password error */
#define EXT_DIAG_CONN_ERR_ACCOUNT_LOCK 3    /* Connection verification account lock */
#define EXT_DIAG_CONN_ERR_BUSY 6            /* Connection needs to preempt */

/*
 * Reduce the structure, remotely query the version number command (mini_ver) structure,
 * and the members are extracted by the VER structure.
 */
typedef struct {
    td_char ver[EXT_DIAG_VER_FULL_PRODUCT_NAME_MAX_SIZE];
    td_char sdk_ver[EXT_DIAG_VER_SDK_PRODUCT_NAME_MAX_SIZE];
    td_char dev[EXT_DIAG_VER_FULL_DEV_VER_NAME_MAX_SIZE];
    td_char hw[EXT_DIAG_VER_FULL_HW_INFO_MAX_SIZE];
    td_char boot_ver[EXT_DIAG_VER_BOOT_NAME_MAX_SIZE];
} diag_cmd_soft_new_ver_stru;

typedef struct _diag_cmd_hso_qry_ue_api_ver_ind_s_ {
    td_u16 cmd_ver;
    td_u16 h_api_ver;
    td_u16 api_ver;
    td_u16 ver;
    td_u32 err_code;
    td_u32 pad;
} diag_cmd_hso_qry_ue_api_ver_ind;

typedef struct {
    td_u32 id;
} ext_dbg_stat_q;

typedef struct {
    td_u32 len;
    td_u32 data[DIAG_READ_MEN_LEN];
} read_mem_ind;

typedef struct {
    td_u32 addr;
    td_u32 len;
} read_mem;

typedef struct {
    td_u32 start_addr;
    td_u32 end_addr;
} cycle_read_mem;

typedef struct {
    td_u16 send_uart_fail_cnt;
    td_u16 ack_ind_malloc_fail_cnt;
    td_u16 msg_malloc_fail_cnt;
    td_u16 msg_send_fail_cnt;
    td_u16 msg_overbig_cnt;
    td_u16 ind_send_fail_cnt;
    td_u16 ind_malloc_fail_cnt;
    td_u8 diag_queue_used_cnt;
    td_u8 diag_queue_total_cnt;
    td_u8 dec_fail_cnt;
    td_u8 enc_fail_cnt;
    td_u16 pkt_size_err_cnt;
    td_u32 local_req_cnt;
    td_u32 remote_req_cnt;
    td_u16 req_cache_overflow_cnt;
    td_u8 conn_excep_cnt;
    td_u8 conn_bu_cnt;
    td_u8 chl_busy_cnt;
    td_u8 req_overbig1_cnt;
    td_u16 rx_remote_req_cnt;
    td_u16 rx_remote_req_invalid_cnt;
    td_u8 cmd_list_total_cnt;  /* < the total number of command lists that support registration */
    td_u8 cmd_list_used_cnt;   /* < the number of registered command lists */
    td_u8 stat_list_total_cnt; /* < the total number of statistical object lists that support registration */
    td_u8 stat_list_used_cnt;  /* < the number of registered statistics object lists */
    td_u8 req_overbig2_cnt;
    td_u8 invalid_dec_id;
    td_u8 heart_beat_timeout_cnt;
    td_u8 rx_start_flag_wrong_cnt;
    td_u8 rx_ver_wrong_cnt;
    td_u8 rx_pkt_data_size_wrong_cnt;
    td_u8 rx_crc16_req_wrong_cnt;
    td_u8 rx_crc16_mux_wrong_cnt;
    td_u8 pad[2]; /* pad 2 byte */
} ext_stat_diag_qry;

typedef struct {
    td_u16 cmd_id;
    td_u8 ack_ctrl;
    td_u8 pad;
    td_u16 port_index;
    td_u16 buffer_size;
    td_pvoid buffer;
    td_u32 time_out;
} diag_send_packet_info_stru;


typedef struct {
    td_u32 major_minor_version;
    td_u32 revision_version;
    td_u32 build_version;
} ext_ue_soft_version;
typedef struct {
    td_u16 version_v;
    td_u16 version_r;
    td_u16 version_c;
    td_u16 version_b;
    td_u16 version_spc;
    td_u16 reserved[3]; /* pad 3 item */
} ext_ue_product_ver;

typedef struct {
    td_u32 rc; /* result code */
    td_char imei[IMEI_SIZE];
    ext_ue_soft_version ue_soft_version;
    ext_ue_product_ver build_version;
} diag_cmd_host_connect_cnf_stru;

typedef struct {
    td_u16 err_no;        /* Error code (as above) */
    td_u16 res_try_cnt;   /* Remaining retries (units) */
    td_u16 res_lock_time; /* Remaining lock duration (unit s) */
    td_u16 pad;           /* Reserved */
} diag_cmd_conn_auth_ind_stru;

typedef struct {
    td_u8 pub_a[DIAG_CONN_PUB_DATA_SIZE];
} diag_cmd_conn_req_pub_a_stru;

/* diag connect rsp pub_b */
#define DIAG_CONN_SALT_SIZE 20
typedef struct {
    td_u8 pub_b[DIAG_CONN_PUB_DATA_SIZE];
    td_u8 salt[DIAG_CONN_SALT_SIZE];
    td_u32 salt_len;
    diag_cmd_conn_auth_ind_stru ret_ind; /* In order to be compatible with the old version, put it at the end */
} diag_cmd_conn_rsp_pub_b_stru;

/* diag connect req m */
#define DIAG_CONN_KEY_HASH_SIZE 32
typedef struct {
    td_u8 req_m1[DIAG_CONN_KEY_HASH_SIZE];
} diag_cmd_conn_req_m_stru;

/* diag connect rsp m */
typedef struct {
    td_u32 ret; /* EXT_ERR_SUCCESS;EXT_ERR_FAILURE. */
    td_u8 rsp_m2[DIAG_CONN_KEY_HASH_SIZE];
    diag_cmd_conn_auth_ind_stru ret_ind; /* In order to be compatible with the old version, put it at the end */
} diag_cmd_conn_rsp_m_stru;

#define DIAG_PWD_MAX_LEN 32
#define DIAG_CONN_VERTIFY_SIZE 128
typedef struct {
    td_u8 old_pwd[DIAG_PWD_MAX_LEN];
    td_u8 new_pwd[DIAG_PWD_MAX_LEN];
    td_u8 new_salt[DIAG_CONN_SALT_SIZE];
    td_u8 old_pwd_len;
    td_u8 new_pwd_len;
    td_u8 new_salt_len;
    td_u8 pad;
} diag_cmd_change_pwd_req_stru;

#define EXT_CHANGE_PWD_ERR_OK 0                  /* password has been updated */
#define EXT_CHANGE_PWD_ERR_SYS_CALL 1            /* System error */
#define EXT_CHANGE_PWD_ERR_PASSWORD_WRONG 2      /* The original password is incorrect */
#define EXT_CHANGE_PWD_ERR_ACCOUNT_LOCK 3        /* Account Lock */
#define EXT_CHANGE_PWD_ERR_PASSWORD_FORMAT_ERR 4 /* The password format is incorrect */

typedef diag_cmd_conn_auth_ind_stru diag_cmd_change_pwd_ind_stru;

#define DIAG_HEART_BEAT_DIR_UP 0x1
#define DIAG_HEART_BEAT_DIR_DOWN 0x2
#define DIAG_HEART_BEAT_DEFAULT_RANDOM_DATA 0x5C5C
typedef struct {
    td_u32 dir;         /* direction bit */
    td_u32 random_data; /* random data, requiring the same uplink and downlink */
} diag_cmd_heart_beat_struct;


/* MSG Configuration Confirmation (Single Board -> HSO) Structure */
typedef struct {
    td_u16 rc;        /* result */
    td_u16 module_id; /* module ID */
} diag_cmd_msg_cfg_cnf_stru;

/* MSG Configuration Request (HSO->Single Board) Structure */
typedef struct {
    td_u16 module_id;     /* Module ID, when the MSG is SYS or USR, marks the subcategories under the two categories */
    td_u8 is_dest_module; /* Is it the target module */
    td_u8 switch_code;    /* Turns the configuration on or off */
} diag_cmd_msg_cfg_req_stru;

/* MSG reset confirmation (single board -> HSO) structure */
typedef struct {
    td_u32 rc;
} diag_cmd_msg_rst_cnf_stru;

/* MSG Reset Request (HSO->Single Board) Structure */
typedef struct {
    td_u32 msg_type; /* MSG type, 0: all; 1: SYS; 2: DEV; 3: USR */
} diag_cmd_msg_cfg_rst_req_stru;

/* *
 * CMD ID : DIAG_CMD_NV_QRY  Search a NV item
 * REQ    : DIAG_CMD_NV_QRY_REQ_STRU
 * CNF    : DIAG_CMD_NV_QRY_CNF_STRU
 * IND    : NONE
 */
typedef struct {
    td_u32 nv_id; /* NV item ID to be acquired */
} diag_cmd_nv_qry_req_stru;

typedef struct {
    td_u32 rc;
    /* indicates whether the execution result is successful, 0 indicates success, and the others are error codes. */
    td_u32 nv_id; /* Get the NV item Id */
    td_u16 crc;   /* crc16 */
    td_u16 pad;
    td_u32 data_size; /* Size of the NV item data obtained */
    td_u8 data[0];    /* Get NV item data */
} diag_cmd_nv_qry_cnf_stru;

/* *
 * CMD ID : DIAG_CMD_NV_WR
 * REQ    : DIAG_CMD_NV_WR_REQ_STRU
 * CNF    : DIAG_CMD_NV_WR_CNF_STRU
 * IND    : NONE
 */
typedef struct {
    td_u32 nv_id; /* NV ID to be written */
    td_u16 crc;   /* crc16 */
    td_u16 pad;
    td_u32 data_size; /* Size of NV item data to be written */
    td_u8 data[0];    /* data buffer */
} diag_cmd_nv_wr_req_stru;

typedef struct {
    td_u32 rc;
    /* indicates whether the execution result is successful, 0 indicates success, and the others are error codes. */
    td_u32 nv_id;
} diag_cmd_nv_wr_cnf_stru;

typedef struct {
    td_u32 id;
    td_u32 data[3];
} test_diag_cmd_req_a;
typedef struct {
    td_u32 id;
    td_u32 data[3];
} test_diag_cmd_ind_a;

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of soc_diag_cmd.h */
