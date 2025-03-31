/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: thruput test header file.
 */

#ifndef __HMAC_SAMPLE_DAQ_H__
#define __HMAC_SAMPLE_DAQ_H__

#include "oal_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "mac_vap_ext.h"
#include "diag_sample_data.h"

#define HMAC_SAMPLE_DAQ_CNT        256     /* 数采get 数据 256 次 */
#define HMAC_SAMPLE_DAQ_BUF_LEN    512
#define HMAC_SAMPLE_DAQ_LEN          4
#define HMAC_SAMPLE_DAQ_TX_RX_CNT    9
#define HMAC_SAMPLE_DAQ_APB_CNT      3
#define HMAC_SAMPLE_DAQ_RSSI_CNT     1
#define TX_RX_SAMPLE_ADDR        0xa0
#define APB_SAMPLE_ADDR          0xa1
#define RSSI_SAMPLE_ADDR0        0xe0
#define RSSI_SAMPLE_ADDR1        0xe1
#define RSSI_SAMPLE_ADDR2        0xe2
#define RSSI_SAMPLE_ADDR3        0xe3

/* 维测4 维测8 */
#define HMAC_SAMPLE_DAQ_TEST4_CNT    30  /* 维测4子特性个数 */
#define HMAC_SAMPLE_DAQ_TEST8_CNT    1   /* 维测8子特性个数 */

#define HMAC_SAMPLE_SOURCE_WLAN   0x5    /* 采集源：wlan */
#define HMAC_SAMPLE_SOURCE_SOC    0xB    /* 采集源：soc=>coex */

/* sample data */
#define DIAG_CMD_ID_BSLE_SAMPLE   0x71D4
#define DIAG_CMD_ID_WLAN_SAMPLE   0x71D5
#define DIAG_CMD_ID_SOC_SAMPLE    0x71D6
#define DIAG_CMD_ID_SAMPLE_DATA   0x7194
#define DIAG_CMD_ID_SAMPLE_FINISH 0x7196   /* 数采数据上报结束ID */

typedef enum {
      /* wlan维测1的4个数采数据结构类型 */
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_RX_INFO = 0x570,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_TX_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_APB_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RSSI_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU0_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU1_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU2_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU3_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU0_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU1_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU2_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU3_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_ENCR_RC4_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_DECR_RC4_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_ENCR_AES_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_DECR_AES_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_MULTI_BSS_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_HE_ROM_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TRIGGER_NFRP_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_COEX_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_P2P_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_BCN_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TSF_VAP0_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TSF_VAP1_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_INTR_STATUS_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_SOUNDING_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_CSI_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_MU_EDCA_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_ERR_INTR_PADDING_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_ERR2_INTR_PADDING_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_DBAC_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_FSM_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_SOUDING_CSI_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_TRIG_PADDING_INFO,
    SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_VECTOR_INFO
} hal_soc_msg_id;

/* sample sub_mode结构体 */
typedef struct {
    osal_u8 sub_mode_flag;          /* 子特性标志位 */
    osal_u8 sub_mode_type_cnt;      /* 子特性类型报文个数 */
    hal_soc_msg_id msg_id;    /* 子特性msg id */
} diag_sample_sub_mode_stru;

osal_u32 hmac_sample_wlan_data_enable(diag_wlan_sample_cmd *sample_cmd, osal_u32 len);
osal_u32 hmac_sample_soc_data_enable(diag_wlan_sample_cmd *sample_cmd, osal_u32 len);
#endif

static osal_u32 hmac_sample_daq_init_weakref(osal_void) __attribute__ ((weakref("hmac_sample_daq_init"), used));
static osal_void hmac_sample_daq_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_sample_daq_deinit"), used));
#endif /* end of hmac_sample_daq.h */
