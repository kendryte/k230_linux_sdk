/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform dyn mem header file
 * Author: Huanghe
 * Create: 2021-06-08
 */

#ifndef __PLAT_CALI_H__
#define __PLAT_CALI_H__
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define OAL_2G_CHANNEL_NUM         (3)
#define OAL_5G_20M_CHANNEL_NUM     (7)
#define OAL_5G_80M_CHANNEL_NUM     (7)
#define OAL_5G_160M_CHANNEL_NUM    (2)
#define OAL_5G_CHANNEL_NUM         (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM + OAL_5G_160M_CHANNEL_NUM)
#define OAL_5G_DEVICE_CHANNEL_NUM  (7)
#define OAL_CALI_HCC_BUF_NUM       (3)
#define OAL_CALI_HCC_BUF_SIZE      (1500)
#define OAL_CALI_IQ_TONE_NUM       (16)
#define OAL_CALI_TXDC_GAIN_LVL_NUM (16)           /* tx dc补偿值档位数目 */
#define OAL_BT_RF_FEQ_NUM          (79)           /* total Rf frequency number */
#define OAL_BT_CHANNEL_NUM         (8)            /* total Rf frequency number */
#define OAL_BT_POWER_CALI_CHANNEL_NUM         (3)
#define OAL_BT_NVRAM_DATA_LENGTH   (104)
#define OAL_BT_NVRAM_NAME           "BTCALNV"
#define OAL_BT_NVRAM_NUMBER        (352)


#define WIFI_2_4G_ONLY              (0x2424)
#define SYS_EXCEP_REBOOT            (0xC7C7)
#define OAL_CALI_PARAM_ADDITION_LEN (8)
#define OAL_5G_IQ_CALI_TONE_NUM         (8)

#define CHECK_5G_ENABLE             "radio_cap_0"

/* wifi校准buf长度 */
#define RF_CALI_DATA_BUF_LEN              (0x6cd8)
#define RF_SINGLE_CHAN_CALI_DATA_BUF_LEN  (RF_CALI_DATA_BUF_LEN >> 1)
/* 校准结构体大小 */
#define OAL_SINGLE_CALI_DATA_STRU_LEN     (RF_CALI_DATA_BUF_LEN + 4)
#define OAL_DOUBLE_CALI_DATA_STRU_LEN     (OAL_SINGLE_CALI_DATA_STRU_LEN << 1)
/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量定义
*****************************************************************************/
extern  osal_u32 g_ul_cali_update_channel_info;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct {
    osal_u16 us_analog_rxdc_cmp;
    osal_u16 us_digital_rxdc_cmp_i;
    osal_u16 us_digital_rxdc_cmp_q;
    osal_u8 auc_reserve[2];
} oal_rx_dc_comp_val_stru;

typedef struct {
    osal_u8 upc_ppa_cmp;
    osal_s8 ac_atx_pwr_cmp;
    osal_u8 dtx_pwr_cmp;
    osal_u8 auc_reserve[1];
} oal_2G_tx_power_comp_val_stru;

typedef struct {
    osal_u8 upc_ppa_cmp;
    osal_u8 upc_mx_cmp;
    osal_s8 ac_atx_pwr_cmp;
    osal_u8 dtx_pwr_cmp;
} oal_5G_tx_power_comp_val_stru;

typedef struct {
    osal_u16 us_txdc_cmp_i;
    osal_u16 us_txdc_cmp_q;
} oal_txdc_comp_val_stru;

typedef struct {
    osal_u8 uc_ppf_val;
    osal_u8 auc_reserve[3];
} oal_ppf_comp_val_stru;

typedef struct {
    osal_u16 us_txiq_cmp_p;
    osal_u16 us_txiq_cmp_e;
} oal_txiq_comp_val_stru;

typedef struct {
    osal_u16  ul_cali_time;
    osal_u16 bit_temperature    : 3,
             uc_5g_chan_idx1    : 5,
             uc_5g_chan_idx2    : 5,
             en_update_bt       : 3;
} oal_update_cali_channel_stru;

typedef struct {
    osal_u32 ul_wifi_2_4g_only;
    osal_u32 ul_excep_reboot;
    osal_u32 ul_reserve[OAL_CALI_PARAM_ADDITION_LEN];
} oal_cali_param_addition_stru;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
typedef osal_u32 (*custom_cali_func)(osal_void);
void set_custom_cali_func(custom_cali_func func);
custom_cali_func get_custom_cali_func(void);
void set_custom_exit_later_func(custom_cali_func func);
custom_cali_func get_custom_exit_later_func(void);
#endif

/* 以下5个宏定义，如果要修改长度，需要同步修改device的宏定义 */
#define BFGX_BT_CALI_DATA_SIZE               (492)
#define WIFI_CALI_DATA_FOR_FM_RC_CODE_SIZE   (20)
#define BFGX_NV_DATA_SIZE                    (128)
#define BFGX_BT_CUST_INI_SIZE                (512)
#define WIFI_CALI_DATA_FOR_BT                (896)

extern osal_void cali_dyn_mem_cfg(osal_void);
extern osal_void bsle_cali_dyn_mem_cfg(osal_void);
#ifdef CONFIG_NO_SUPPORT_INI
td_void hwifi_config_cmu_xo_trim_temp_comp(td_void);
#endif
td_void hwifi_update_cmu_xo_trim_temp_comp_timer(td_void);
td_void hwifi_config_get_cmu_xo_trim_temp_comp(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of plat_cali.h */

