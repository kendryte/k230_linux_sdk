/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Header file for hal_mfg.c
 */

#ifndef HAL_MFG_H
#define HAL_MFG_H
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
#include "hal_ext_if.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define XO_TRIM_FINE ((osal_u32)(1 << 6))
#define XO_TRIM_COARSE ((osal_u32)(1 << 3))
#define XO_TRIM_ENABLE 1
#define XO_TRIM_DISABLE 0
#define EFUSE_NAME_LEN 16
#define CMD_MAX_LEN 256
#define EFUSE_GROUP_MAX 2
#define WLAN_CFG_TEMPER_GEARS_STEP 10
#define EXT_EFUSE_IDX_GROUP 48
#define WLAN_CFG_CON_OFFSET_NUM 2
#define EFUSE_MAX_LEN 24
#define WLAN_CFG_MAC_GROUP_NUM 3
#define SIZE_3_BITS 3

typedef enum {
    EFUSE_OP_READ,
    EFUSE_OP_WRITE,
    EFUSE_OP_REMAIN,
    EFUSE_OP_READ_ALL_MFG_DATA
} efuse_op;

typedef struct {
    osal_char efuse_name[EFUSE_NAME_LEN];
    osal_u32 len;
    osal_u8 val[CMD_MAX_LEN];
    osal_u8 op;
    osal_u8 resv[3];
} efuse_operate_stru;

typedef struct {
    osal_char efuse_name[EFUSE_NAME_LEN];
    osal_u32 size;
    osal_u8 group_id[EFUSE_GROUP_MAX];
    osal_u8 lock_id[EFUSE_GROUP_MAX];
} efuse_id_info_stru;

typedef osal_s32(*read_efuse_parse_func)(efuse_operate_stru *info);

typedef struct {
    osal_char efuse_name[EFUSE_NAME_LEN];
    osal_u32 size;
    read_efuse_parse_func func;
} efuse_id_info_back_stru;

typedef struct {
    osal_u8 rssi_offset_0;
    osal_u8 rssi_offset_1;
    osal_u8 rssi_offset_2;
    osal_u8 resv;
} efuse_rssi_offset_stru;

typedef struct {
    osal_s8 curve_factor_hig;
    osal_s8 curve_factor_low;
    osal_u8 resv[2];
    osal_u8 band1_hign[WLAN_CFG_CON_OFFSET_NUM];
    osal_u8 band1_low[WLAN_CFG_CON_OFFSET_NUM];
    osal_u8 ofdm_20m_hign[WLAN_CFG_CON_OFFSET_NUM];
    osal_u8 ofdm_20m_low[WLAN_CFG_CON_OFFSET_NUM];
    osal_u8 ofdm_40m_hign[WLAN_CFG_CON_OFFSET_NUM];
    osal_u8 ofdm_40m_low[WLAN_CFG_CON_OFFSET_NUM];
} efuse_power_offset_stru;

typedef struct {
    osal_u8 *data;
    osal_u8 len;
} efuse_mfg_data_status;

typedef struct {
    osal_u8 temp;
    osal_u8 resv[3];
    osal_u8 group_left[EXT_EFUSE_IDX_GROUP];
    efuse_power_offset_stru pwr_offset;
    efuse_rssi_offset_stru rssi_offset;
} efuse_mfg_data_stru;

osal_void hal_rf_set_default_cmu_xo_trim(osal_void);
osal_void hal_set_mfg_mode(osal_u8 mode);
osal_void hal_set_xo_trim_coarse(osal_u32 xo_trim_coarse, osal_u32 *coarse_reg);
osal_void hal_set_xo_trim_fine(osal_u32 xo_trim_fine, osal_u32 *fine_reg);
osal_void hal_get_xo_trim(osal_u32 *xo_trim_coarse, osal_u32 *xo_trim_fine);
osal_void hal_get_temp(osal_void);
osal_s32 hal_config_efuse_operate(hmac_vap_stru *hmac_vap, efuse_operate_stru *param);
osal_s32 hal_read_efuse_cmu_xo_trim(osal_u8 *coarse_reg, osal_u8 *fine_reg);
osal_s32 hal_efuse_cmu_xo_trim_write(osal_u8 coarse_reg, osal_u8 fine_reg);
osal_s32 hal_ccpriv_get_valid_mac(osal_void);
osal_s32 hal_efuse_write_mfg_flag(osal_void);
extern osal_u32 uapi_tsensor_read_temperature(osal_s16 *temperature);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif
#endif
