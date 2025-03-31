/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: wal_linux_ioctl.h file.
 */

#ifndef __WAL_LINUX_IOCTL_H__
#define __WAL_LINUX_IOCTL_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IOCTL_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 用户pwr ref reg的定制化保护阈值 */
#define WAL_HIPRIV_PWR_REF_DELTA_HI         40
#define WAL_HIPRIV_PWR_REF_DELTA_LO         (-40)

#define WAL_IOCTL_ROM_INFO_MAX_LENGTH          2
#define WAL_IOCTL_ROM_INFO_BW_MAX              3
#define WAL_IOCTL_ROM_INFO_NSS_MAX             1

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

typedef enum {
    WAL_DSCR_PARAM_TXRTS_ANTENNA        = 0,
    WAL_DSCR_PARAM_RXCTRL_ANTENNA       = 1,
    WAL_DSCR_PARAM_DATA_RATE0           = 2,
    WAL_DSCR_PARAM_DATA_RATE1           = 3,
    WAL_DSCR_PARAM_DATA_RATE2           = 4,
    WAL_DSCR_PARAM_DATA_RATE3           = 5,
    WAL_DSCR_PARAM_POWER                = 6,
    WAL_DSCR_PARAM_PREAMBLE_MODE        = 7,
    WAL_DSCR_PARAM_RTSCTS               = 8,
    WAL_DSCR_PARAM_LSIGTXOP             = 9,
    WAL_DSCR_PARAM_SMOOTH               = 10,
    WAL_DSCR_PARAM_SOUNDING             = 11,
    WAL_DSCR_PARAM_TXBF                 = 12,
    WAL_DSCR_PARAM_STBC                 = 13,
    WAL_DSCR_PARAM_GET_ESS              = 14,
    WAL_DSCR_PARAM_DYN_BW               = 15,
    WAL_DSCR_PARAM_DYN_BW_EXIST         = 16,
    WAL_DSCR_PARAM_CH_BW_EXIST          = 17,
    WAL_DSCR_PARAM_MCS                  = 18,
    WAL_DSCR_PARAM_NSS                  = 19,
    WAL_DSCR_PARAM_BW                   = 20,
    WAL_DSCR_PARAM_LTF                  = 21,
    WLA_DSCR_PARAM_GI                   = 22,
    WLA_DSCR_PARAM_TXCHAIN              = 23,
    WLA_DSCR_PARAM_FEC                  = 24,
    WLA_DSCR_PARAM_DCM                  = 25,
    WLA_DSCR_PARAM_PE                   = 26,
    WAL_DSCR_PARAM_PROTOCOL             = 27,
    WAL_DSCR_PARAM_UPLINK_FLAG          = 28,
    WAL_DSCR_PARAM_BUTT
} wal_dscr_param_enum;

typedef osal_u8 wal_dscr_param_enum_uint8;

typedef enum {
    WAL_TX_POW_PARAM_SET_RF_REG_CTL = 0,
    WAL_TX_POW_PARAM_SET_FIX_LEVEL,
    WAL_TX_POW_PARAM_SET_MAG_LEVEL,
    WAL_TX_POW_PARAM_SET_CTL_LEVEL,
    WAL_TX_POW_PARAM_SET_AMEND,
    WAL_TX_POW_PARAM_SET_NO_MARGIN,
    WAL_TX_POW_PARAM_SET_SHOW_LOG,
    WAL_TX_POW_PARAM_SET_SAR_LEVEL,

    WAL_TX_POW_PARAM_BUTT
} wal_tx_pow_param_enum;
typedef osal_u8 wal_tx_pow_param_enum_uint8;

typedef enum {
    WAL_AMPDU_DISABLE,
    WAL_AMPDU_ENABLE,

    WAL_AMPDU_CFG_BUTT
} wal_ampdu_idx_enum;

typedef osal_u8 wal_ampdu_idx_enum_uint8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u8 wal_cfg80211_convert_value_to_vht_width(osal_s32 l_channel_value);
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
extern int fastip_attach_wifi(struct net_device *dev);
extern void fastip_detach_wifi(unsigned int idx);
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern osal_u32 hwifi_config_init_dts_main_etc(oal_net_device_stru *cfg_net_dev);
extern osal_s32 wal_set_custom_process_func_etc(osal_void);
osal_void hwifi_config_init_ini_main(oal_net_device_stru *cfg_net_dev);
extern osal_u32 wal_custom_cali_etc(osal_void);
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

osal_void hwifi_config_init_ini_country(oal_net_device_stru *cfg_net_dev);
osal_u32 hwifi_force_refresh_rf_params(oal_net_device_stru *net_dev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_linux_ioctl.h */

