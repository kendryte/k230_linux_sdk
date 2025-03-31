/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Description: header file.
 * Author: CompanyName
 * Create: 2023-01-01
 */

#ifndef __CUSTOMIZE_BSLE_EXT_H__
#define __CUSTOMIZE_BSLE_EXT_H__

#include "hcc_cfg_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define BSLE_LARGE_NETBUF_SIZE 800
#define CUST_POWER_CALI_NUM 8
#define BD_ADDR_LEN 6

typedef struct {
    osal_char *name;
    int32_t case_entry;
} bsle_cfg_cmd;

typedef struct {
    int32_t val;
    uint8_t value_state;
} bsle_customize_stru;

typedef struct {
    // 6-byte array address value
    uint8_t addr[BD_ADDR_LEN];
} bd_addr_t;

/* enum定义不能超过BSLE_BT_CUST_INI_SIZE/4 (128) */
typedef enum {
    BSLE_CFG_INI_BT_COEX_MODE = 0,
    BSLE_CFG_INI_BLE_DISABLE_LL_PRIVACY,
    BSLE_CFG_INI_BSLE_SUSPEND_MODE,
    BSLE_CFG_INI_BSLE_SUSPEND_SCAN_INTERVAL,
    BSLE_CFG_INI_BSLE_SUSPEND_SCAN_WINDOW,
    BSLE_CFG_INI_BSLE_FRONT_SWITCH,
    BSLE_CFG_INI_BT_MAXPOWER,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_NUM,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND1,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND2,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND3,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND4,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND5,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND6,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND7,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND8,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE1,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE2,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE3,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE4,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE5,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE6,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE7,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE8,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK1,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK2,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK3,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK4,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK5,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK6,
    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK7,
    BSLE_CFG_INI_BT_SRRC_SWITCH,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL1,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL2,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL3,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL4,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL5,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL6,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL7,
    BSLE_CFG_INI_BT_SRRC_PA_REF_VAL8,
    BSLE_CFG_INI_BT_SRRC_PA_FRE1,
    BSLE_CFG_INI_BT_SRRC_PA_FRE2,
    BSLE_CFG_INI_BT_SRRC_PA_FRE3,
    BSLE_CFG_INI_BT_SRRC_PA_FRE4,
    BSLE_CFG_INI_BT_SRRC_PA_FRE5,
    BSLE_CFG_INI_BT_SRRC_PA_FRE6,
    BSLE_CFG_INI_BT_SRRC_PA_FRE7,
    BSLE_CFG_INI_BT_SRRC_PA_FRE8,
    BSLE_CFG_INI_BSLE_USE_FLASH,
    BSLE_CFG_INI_BUTT
} bsle_cfg_ini;

typedef enum {
    BSLE_MSG_HCC_TYPE_CUSTOMIZE_DATA,
    BSLE_MSG_HCC_TYPE_FSM_DATA,
    BSLE_MSG_HCC_TYPE_DEVICE_STATUS,
    BSLE_MSG_HCC_TYPE_DATA_COLLECTION,
    BSLE_MSG_HCC_TYPE_DEVICE_ACTION_STATUS,
    BSLE_MSG_HCC_TYPE_DATA_BUTT
} bsle_msg_hcc_type;

typedef enum {
    BSLE_STATUS_MSG_BLE_OPEN,
    BSLE_STATUS_MSG_BLE_CLOSE,
    BSLE_STATUS_MSG_SLE_OPEN,
    BSLE_STATUS_MSG_SLE_CLOSE,
    BSLE_STATUS_MSG_BUTT
} bsle_hcc_msg_status;

typedef struct {
    uint16_t type;
    uint16_t len;
} bsle_msg_tag;

typedef enum {
    BSLE_STATUS_BOOT_FINISH = 1,
    BSLE_STATUS_CUSTOMIZE_RECEIVED,
    BSLE_DEVICE_MSG_BUTT
} bsle_hcc_msg_c2h;

// 最大长度见平台定义 BFGX_BT_CUST_INI_SIZE / 4
typedef struct {
    uint32_t bt_coex_mode;
    uint32_t ble_disable_ll_privacy;
    uint32_t bsle_suspend_mode;
    uint32_t bsle_suspend_scan_interval;
    uint32_t bsle_suspend_scan_window;
    uint32_t bsle_front_switch;
    // BT power
    int32_t bt_maxpower;
    // 20dbm tx pwr cali
    uint32_t bt_20dbm_txpwr_cali_num;
    // high 16bits = freq ,low 16 bits = ref
    uint32_t bt_20dbm_txpwr_cali_freq_ref1;
    uint32_t bt_20dbm_txpwr_cali_freq_ref2;
    uint32_t bt_20dbm_txpwr_cali_freq_ref3;
    uint32_t bt_20dbm_txpwr_cali_freq_ref4;
    uint32_t bt_20dbm_txpwr_cali_freq_ref5;
    uint32_t bt_20dbm_txpwr_cali_freq_ref6;
    uint32_t bt_20dbm_txpwr_cali_freq_ref7;
    uint32_t bt_20dbm_txpwr_cali_freq_ref8;
    // 校准应用信道分块，如[0,ch1],[ch1,ch2]
    uint32_t bt_20dbm_txpwr_cali_chl1_chl2;
    uint32_t bt_20dbm_txpwr_cali_chl3_chl4;
    uint32_t bt_20dbm_txpwr_cali_chl5_chl6;
    uint32_t bt_20dbm_txpwr_cali_chl7;
    // SRRC switch val = 1 ON,val = 0 OFF
    uint32_t bt_srrc_switch;
    // high 16bits = freq ,low 16 bits = ref
    uint32_t bt_srrc_freq_ref1;
    uint32_t bt_srrc_freq_ref2;
    uint32_t bt_srrc_freq_ref3;
    uint32_t bt_srrc_freq_ref4;
    uint32_t bt_srrc_freq_ref5;
    uint32_t bt_srrc_freq_ref6;
    uint32_t bt_srrc_freq_ref7;
    uint32_t bt_srrc_freq_ref8;
    uint32_t bsle_use_flash;
    // 以下参数只在flash中涉及，ini文件中没有
    int16_t bsle_flash_temperature;
    uint8_t bsle_flash_xotrim_coarse;
    uint8_t bsle_flash_xotrim_fine;
    int16_t bsle_flash_pwr_cali;
    bd_addr_t ble_mac_bdaddr;
    bd_addr_t sle_mac_bdaddr;
} bfgn_bt_customization_stru;

bool hbsle_hcc_customize_get_device_status(bsle_hcc_msg_c2h msg_to_host);

td_void hbsle_hcc_customize_reset_device_status(void);

td_s32 hbsle_hcc_custom_ini_data_buf(void);

int hbsle_hcc_customize_h2d_data_cfg(void);

td_u32 hcc_adapt_bsle_msg_data_col_cmd_tx_proc(td_void *cmd, td_u32 len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
