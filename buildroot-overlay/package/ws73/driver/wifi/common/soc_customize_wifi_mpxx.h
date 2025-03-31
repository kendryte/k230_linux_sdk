/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Header file for soc_customize_wifi.c.
 * Create: 2021-12-15
 */

#ifndef __SOC_CUSTOMIZE_WIFI_MPXX_H__
#define __SOC_CUSTOMIZE_WIFI_MPXX_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PRODUCT_ID_HOST)

// #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define NVRAM_PARAMS_ARRAY      "nvram_params"

#define STR_COUNTRY_CODE    ("country_code")
#define NV_WLAN_NUM             193
#define NV_WLAN_VALID_SIZE      12
#define MAC_LEN                 6
#define MACFMT_ALL              "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACFMT                  "%02x:%s:%s:%s:%02x:%02x"
#define CUS_TAG_INI                     0x11
#define CUS_TAG_NV                      0x13
#define CUS_TAG_PRIV_INI                0x14
#define CUS_TAG_RF_FE                   0x16    /* 射频前端定制化项解析 */
#define CUS_TAG_RF_FE_TX_POWER          0x17    /* 射频前端定制化项解析 功率部分 */
#define CALI_TXPWR_PA_DC_REF_MAX        (0x8000)
#define CALI_TXPWR_PA_DC_REF_MIN        (1000)
#define CALI_TXPWR_PA_DC_FRE_MAX        (78)
#define CALI_TXPWR_PA_DC_FRE_MIN        (0)
#define CALI_BT_TXPWR_PA_DC_REF_MAX     (15000)
#define RF_LINE_TXRX_GAIN_DB_2G_MIN     (-100)
#define RF_LINE_TXRX_GAIN_DB_MAX        (40)
#define RF_LINE_TXRX_GAIN_DB_5G_MIN     (-48)
#define PSD_THRESHOLD_MAX               (-10)
#define PSD_THRESHOLD_MIN               (-15)
#define LNA_GAIN_DB_MAX                 (80)
#define LNA_GAIN_DB_MIN                 (-40)

#define MAC_NUM_5G_BAND                 7   /* 5g band数 */
#define MAC_NUM_2G_BAND                 3   /* 2g band数 */
#define MAC_2G_CHANNEL_NUM              13
/* NVRAM中存储的参数值的总个数，包括4个基准功率 */
#define NUM_OF_NV_PARAMS                (NUM_OF_NV_MAX_TXPOWER + NUM_OF_NV_DPD_MAX_TXPOWER + 4)
#define TX_RATIO_MAX                    (2000)                          /* tx占空比的最大有效值 */
#define TX_PWR_COMP_VAL_MAX             (50)                            /* 发射功率补偿值的最大有效值 */
#define MORE_PWR_MAX                    (50)                            /* 根据温度额外补偿的发射功率的最大有效值 */
#define COUNTRY_CODE_LEN                (3)                             /* 国家码位数 */
#define DEFAULT_COUNTRY_CODE "CN"
#define MAX_COUNTRY_COUNT               (300)                           /* 支持定制的国家的最大个数 */
#define DELTA_CCA_ED_HIGH_TH_RANGE      15                              /* δ调整上限，最大向上或向下调整15dB */
#define CUS_NUM_5G_BW                   4   /* 定制化5g带宽数 */
#define CUS_NUM_FCC_2G_PRO              3   /* 定制化2g FCC 11B+OFDM_20M+OFDM_40M */
#define CUS_NUM_5G_20M_SIDE_BAND        6   /* 定制化5g边带数 */
#define CUS_NUM_5G_40M_SIDE_BAND        6
#define CUS_NUM_5G_80M_SIDE_BAND        5
#define CUS_NUM_5G_160M_SIDE_BAND       2
#define CUS_NUM_OF_SAR_PARAMS           8   /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define CUS_NUM_OF_SAR_LVL              3   /* 定制化降SAR档位数 */
#define CUS_MIN_OF_SAR_VAL              (0x28)  /* 定制化降SAR最小值 4dbm */
#define CUS_PARAMS_LEN_MAX              (104)   /* 定制项最大长度 */
#define DY_2G_CALI_PARAMS_NUM           (4)    /* 动态校准参数个数,2.4g */
/* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5*2(high & low)个band */
#define DY_CALI_PARAMS_NUM              (14)
#define DY_CALI_PARAMS_BASE_NUM         (9)    /* 动态校准参数个数,2.4g 4个(ofdm 20/40 11b cw),5g 5(high)个band */
#define DY_CALI_PARAMS_TIMES            (3)    /* 动态校准参数二次项系数个数 */
#define DY_CALI_NUM_5G_BAND             (5)    /* 动态校准5g band1 2&3 4&5 6 7 */
#define DY_CALI_FIT_PRECISION_A1        (6)
#define DY_CALI_FIT_PRECISION_A0        (16)
#define CUS_MAX_BASE_TXPOWER_VAL        (220)  /* 最大基准发送功率的最大有效值 */
#define CUS_MIN_BASE_TXPOWER_VAL        (50)   /* 最小基准发送功率的最大有效值 */
#define CUS_NUM_2G_DELTA_RSSI_NUM       (2)    /* 20M/40M */
#define CUS_NUM_5G_DELTA_RSSI_NUM       (4)    /* 20M/40M/80M/160M */
#define CUS_BASE_PWR_NUM_5G              DY_CALI_NUM_5G_BAND    /* 5g Base power 5个 band1 2&3 4&5 6 7 */
#define CUS_BASE_PWR_NUM_2G             (1)
// 打印自定义功率
#define NV_PRINT_LEN 128
#define NV_PRINT_STEP_1 1
#define NV_PRINT_STEP_3 3
#define NV_PRINT_STEP_4 4
#define NV_PRINT_STEP_5 5
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN1    (6250)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN2    (5362)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN3    (4720)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN4    (4480)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN5    (4470)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN6    (4775)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN7    (5200)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN8    (5450)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN9    (5600)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN10   (6100)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN11   (6170)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN12   (6350)
#define DTS_CALI_PARAMS_ETC_REF_2G_VAL_CHAN13   (6530)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND1    (2500)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND2    (2800)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND3    (3100)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND4    (3600)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND5    (3600)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND6    (3700)
#define DTS_CALI_PARAMS_ETC_REF_5G_VAL_BAND7    (3800)

/* 计算绝对值 */
#define cus_abs(val)                    ((val) > 0 ? (val) : -(val))

/* 判断CCA能量门限调整值是否超出范围, 最大调整幅度:DELTA_CCA_ED_HIGH_TH_RANGE */
#define cus_delta_cca_ed_high_th_out_of_range(val)  (cus_abs(val) > DELTA_CCA_ED_HIGH_TH_RANGE ? 1 : 0)

/* 取16位数 */
#define cus_get_low_16bit(val)    ((osal_u16)(((osal_u32)(val)) & 0x0000FFFF))
#define cus_get_high_16bit(val)   ((osal_u16)((((osal_u32)(val)) & 0xFFFF0000) >> 16))

/* The max scatter buffers when host to device */
#define HISDIO_HOST2DEV_SCATT_MAX       40

/* The max scatter buffers when device to host */
#define HISDIO_DEV2HOST_SCATT_MAX       (63)
/* 前端定制化参数宏 */
#define WLAN_RF_FE_CALI_DATA_MASK_UPLOAD 0x1
#define WLAN_RF_FE_CALI_DATA_MASK_DOWNLOAD 0x2
#define SOC_CUSTOM_RF_FE_RX_LOSS_NUM_2G 3
#define SOC_CUSTOM_RF_FE_RX_LOSS_NUM SOC_CUSTOM_RF_FE_RX_LOSS_NUM_2G
#define SOC_CUSTOM_RF_FE_RX_LOSS_NUM_RSV 1 // 插损保留位数
#define SOC_CUSTOM_RF_FE_MAX_POWER_NUM_2G 1
#define SOC_CUSTOM_RF_FE_TARGET_POWER_NUM_2G 33
#define SOC_CUSTOM_RF_FE_LIMIT_POWER_NUM_2G 56
#define SOC_CUSTOM_RF_FE_SAR_POWER_NUM_2G 3
#define SOC_CUSTOM_RF_FE_CTA_COEF_SWITCH_2G 1
#define SOC_CUSTOM_RF_FE_POWER_NUM_RSV 2 // 功率保留位数
#define SOC_CUSTOM_RF_FE_REF_POWER_NUM_2G 6
#define SOC_CUSTOM_RF_FE_CURVE_NUM_LVL 2
#define SOC_CUSTOM_RF_FE_CURVE_NUM_2G 9
#define SOC_CUSTOM_RF_FE_FACTOR_NUM_2G 6
#define SOC_CUSTOM_RF_FE_RSSI_COMP_NUM_2G 3
/*****************************************************************************
  3 枚举定义
*****************************************************************************/

typedef enum {
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_11B      = 0,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM20,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM40,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW,
    HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_BUTT,
}hwifi_dyn_2g_pwr_sngl_mode_enum;


/* NV map idx */
typedef enum {
    HWIFI_CFG_NV_WINVRAM_NUMBER           = 340,
    HWIFI_CFG_NV_WITXNVCCK_NUMBER         = 367,
    HWIFI_CFG_NV_WITXNVC1_NUMBER          = 368,
    HWIFI_CFG_NV_WITXNVBWC0_NUMBER        = 369,
    HWIFI_CFG_NV_WITXNVBWC1_NUMBER        = 370,
    HWIFI_CFG_NV_WITXL2G5G0_NUMBER        = 384,
    HWIFI_CFG_NV_WITXL2G5G1_NUMBER        = 385,
    HWIFI_CFG_NV_MUFREQ_5G160_C0_NUMBER,
    HWIFI_CFG_NV_MUFREQ_5G160_C1_NUMBER,
    HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER    = 396,
    HWIFI_CFG_NV_MUFREQ_2G20_C1_NUMBER,
    HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,
    HWIFI_CFG_NV_MUFREQ_2G40_C1_NUMBER,
    HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,
    HWIFI_CFG_NV_MUFREQ_CCK_C1_NUMBER,
}wlan_nvram_idx;

typedef enum {
    REGDOMAIN_FCC        = 0,
    REGDOMAIN_ETSI       = 1,
    REGDOMAIN_JAPAN      = 2,
    REGDOMAIN_COMMON     = 3,

    REGDOMAIN_COUNT
} regdomain_enum;

/* 私有定制化 PRIV CONFIG ID */
typedef enum {
    /* 校准开关 */
    WLAN_CFG_PRIV_CALI_MASK,
    WLAN_CFG_PRIV_CALI_DATA_MASK,
    WLAN_CFG_PRIV_CALI_AUTOCALI_MASK,
    WLAN_CFG_PRIV_BW_MAX_WITH,
    WLAN_CFG_PRIV_SU_BFER,
    WLAN_CFG_PRIV_SU_BFEE,
    WLAN_CFG_PRIV_MU_BFER,
    WLAN_CFG_PRIV_MU_BFEE,
    WLAN_CFG_PRIV_USER_NUM,
    WLAN_CFG_PRIV_HMAC_MAX_AP_NUM,
    WLAN_CFG_PRIV_SCAN_PROBE_SEND_TIMES,
    WLAN_CFG_PRIV_SCAN_DEFAULT_TIME,
    WLAN_CFG_PRIV_SCAN_DEFAULT_COUNT,
    WLAN_CFG_PRIV_SCAN_PROBE_REQ_ALL_IE,
    WLAN_CFG_PRIV_SCAN_PROBE_REQ_DEL_WPS_IE,
    WLAN_CFG_PRIV_RANDOM_MAC_ADDR_SCAN,
    WLAN_CFG_PRIV_RANDOM_SCAN_OUI,
    WLAN_CFG_PRIV_PCAP_FILE_LEN_MAX,
    WLAN_CFG_PRIV_LDPC,
    WLAN_CFG_PRIV_BA_32BIT_BITMAP,
    WLAN_CFG_PRIV_MTID_AGGR_RX,
    WLAN_CFG_PRIV_SELF_HEALING_ENABLE,
    WLAN_CFG_PRIV_SELF_HEALING_PERIOD,
    WLAN_CFG_PRIV_SELF_HEALING_CNT,
    WLAN_CFG_PRIV_DBAC_STA_GC_SLOT_RATIO,
    WLAN_CFG_PRIV_DBAC_STA_GO_SLOT_RATIO,
    WLAN_CFG_PRIV_TPC_FAR_RSSI,
    WLAN_CFG_PRIV_AMSDU_NUM,
    WLAN_CFG_PRIV_AMSDU_TX_ON,
    WLAN_CFG_PRIV_AMPDU_AMSDU_TX_ON,
    WLAN_CFG_INIT_AMPDU_TX_MAX_NUM,
    WLAN_CFG_INIT_AMPDU_RX_MAX_NUM,
    WLAN_CFG_INIT_AMPDU_TX_BAW_SIZE,
    WLAN_CFG_PRIV_DATA_SAMPLE,
    WLAN_CFG_PRIV_ER_SU_DISABLE,
    WLAN_CFG_PRIV_DCM_CONSTELLATION_TX,
    WLAN_CFG_PRIV_BANDWIDTH_EXTENDED_RANGE,
    WLAN_CFG_PRIV_APF_ENABLE,
    WLAN_CFG_PRIV_WOW_EVENT,
    WLAN_CFG_PRIV_SMOOTH_PHASE_EN,
    WLAN_CFG_PRIV_RX_STBC,
    WLAN_CFG_PRIV_WOW_ENABLE,
    WLAN_CFG_PRIV_UDP_HCC_FC_DROP,
    WLAN_CFG_PRIV_ROAM_TRIGGER_RSSI_2G,
    WLAN_CFG_PRIV_ROAM_DELTA_RSSI_2G,
    WLAN_CFG_PRIV_OVER_DS_EN,
    WLAN_CFG_PRIV_PROTOCOL_ENABLE,
    WLAN_CFG_PRIV_TCP_ACK_FILTER_EN,
    WLAN_CFG_PRIV_TCP_ACK_MAX_NUM_START_PROCESS,
    WLAN_CFG_PRIV_FRONT_SWITCH,
    WLAN_CFG_PRIV_TXDATA_QUE_LIMIT,
    WLAN_CFG_PRIV_BUTT,
} wlan_cfg_priv;
typedef osal_u8 wlan_cfg_priv_id_uint8;

/* 定制化 INI CONFIG ID */
typedef enum {
    /* 性能 */
    WLAN_CFG_INIT_USED_MEM_FOR_START,
    WLAN_CFG_INIT_USED_MEM_FOR_STOP,
    WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT,
    WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT,
    /* 拥塞算法 */
    WLAN_CFG_INIT_RX_RESTORE_THRES,
    /* 2G RF前端 */
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START,    // 33
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1 = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_END = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3,
    /* 5G RF前端 */
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START,     // 37
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1 = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_END = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7,

    /* 用于定制化计算PWR RF值的偏差 */
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4,

    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G,
    /* 结温补偿 用于温度保护 */
    WLAN_CFG_INIT_JUNCTION_TEMP_DIFF,
    WLAN_CFG_INIT_BUTT,
} wlan_cfg_init;

/* 射频前端定制化解析类型枚举 */
enum {
    WLAN_CFG_INI_RF_FE_TYPE_PARAMS,
    WLAN_CFG_INI_RF_FE_TYPE_POWER,
    WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI,
    WLAN_CFG_INI_RF_FE_TYPE_RSSI,
    WLAN_CFG_INI_RF_FE_TYPE_BUTT
};
/* 射频前端定制化参数枚举 */
enum {
    WLAN_CFG_INI_RF_FE_RX_INSERT_LOSS_2G,
    WLAN_CFG_INI_RF_FE_POWER_CUSTOMIZE_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_CHIP_MAX_2G = WLAN_CFG_INI_RF_FE_POWER_CUSTOMIZE_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_CUSTOMIZE_END,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_BEGIN = WLAN_CFG_INI_RF_FE_POWER_CUSTOMIZE_END,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_11B = WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_11G,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_20M,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_40M,
    WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_BEGIN = WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH1 = WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH2,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH3,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH4,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH5,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH6,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH7,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH8,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH9,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH10,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH11,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH12,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH13,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH14,
    WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_SAR_2G_BEGIN = WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_SAR_2G = WLAN_CFG_INI_RF_FE_POWER_SAR_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_SAR_2G_END,
    WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH_BEGIN = WLAN_CFG_INI_RF_FE_POWER_SAR_2G_END,
    WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH = WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH_BEGIN,
    WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH_END,
    WLAN_CFG_INI_RF_FE_POWER_REF_2G_BEGIN = WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH_END,
    WLAN_CFG_INI_RF_FE_POWER_REF_2G_HIGH = WLAN_CFG_INI_RF_FE_POWER_REF_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_REF_2G_LOW,
    WLAN_CFG_INI_RF_FE_POWER_REF_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_BEGIN = WLAN_CFG_INI_RF_FE_POWER_REF_2G_END,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_11B = WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_OFDM20M,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_OFDM40M,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_11B,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_OFDM20M,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_OFDM40M,
    WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_END,
    WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_BEGIN = WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_END,
    WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_HIGH = WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_LOW,
    WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_END,
    WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G_BEGIN = WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_END,
    WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G = WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G_BEGIN,
    WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G_END,
    WLAN_CFG_INI_RF_FE_BUTT = WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G_END
};
typedef enum {
    LOW_CURRENT_BOOT_MODE_DEFAULT = 0x0,   /* 默认启动模式 */
    LOW_CURRENT_BOOT_MODE_WITHOUT_PWR_CALI = 0x1,  /* 关闭功率校准的低电流启动模式 */
    LOW_CURRENT_BOOT_MODE_WITH_ONCE_PWR_CALI = 0x2,    /* 关闭非首次功率校准的低电流启动模式 */
    LOW_CURRENT_BOOT_MODE_WITHOUT_CALI = 0xFF, /* 关闭全部校准的低电流启动模式 */
} low_current_boot_mode;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef osal_u8 countrycode_t[COUNTRY_CODE_LEN];
typedef struct {
    regdomain_enum       regdomain;
    countrycode_t        auc_country_code;
} countryinfo_stru;

/**
 *  regdomain <-> plat_tag map structure
 *
 **/
typedef struct regdomain_plat_tag_map {
    regdomain_enum  regdomain;
    int             plat_tag;
} regdomain_plat_tag_map_stru;

typedef struct {
    char*   name;
    int     case_entry;
} wlan_cfg_cmd;

typedef struct {
    osal_s32                val;
    oal_bool_enum_uint8     value_state;
} wlan_customize_private_stru;

typedef struct wlan_cus_pwr_fit_para_stru {
    osal_s16 s_pow_par2;   /* 二次项系数 */
    osal_s16 s_pow_par1;   /* 一次 */
    osal_s16 s_pow_par0;   /* 常数项 */
}wlan_customize_pwr_fit_para_stru;

typedef struct {
    osal_u8*   nv_name;
    osal_u8*   param_name;
    osal_u32   nv_map_idx;
    osal_u8    param_idx;
    osal_u8    rsv[3];
} wlan_cfg_nv_map_handler;

/* 定制化HOST全局变量结构体 */
typedef struct {
    /* ba tx 聚合数 */
    unsigned int    ampdu_tx_max_num;
    /* 漫游 */
    unsigned char   roam_switch;
    unsigned char   roam_scan_orthogonal;
    signed char     c_roam_trigger_b;
    signed char     c_roam_trigger_a;
    signed char     c_roam_delta_b;
    signed char     c_roam_delta_a;
    /* 漫游场景识别 */
    signed char     c_dense_env_roam_trigger_b;
    signed char     c_dense_env_roam_trigger_a;
    oal_bool_enum_uint8    scenario_enable;
    signed char     c_candidate_good_rssi;
    unsigned char   candidate_good_num;
    unsigned char   candidate_weak_num;
    /* scan */
    unsigned char   random_mac_addr_scan;
    /* capab */
    unsigned char   disable_capab_2ght40;
    unsigned int   lte_gpio_check_switch;
    unsigned int   ism_priority;
    unsigned int   lte_rx;
    unsigned int   lte_tx;
    unsigned int   lte_inact;
    unsigned int   ism_rx_act;
    unsigned int   bant_pri;
    unsigned int   bant_status;
    unsigned int   want_pri;
    unsigned int   want_status;
} wlan_customize_stru;

/* 不接受cust下发的切换国家码的命令
 * 1、ini文件中国家码被配置成99
 * 2、维测需要:使用ccpriv命令修改过国家码
*/
typedef struct {
    oal_bool_enum_uint8     country_code_ingore_ini_flag;
    oal_bool_enum_uint8     country_code_ingore_ccpriv_flag;
} wlan_cust_country_code_ingore_flag_stru;

#ifdef _PRE_WLAN_NO_SUPPORT_INI
/* 大区结构体 */
typedef struct {
    osal_u8 region_idx; // 大区号
    osal_u16 region_tag; // 大区功率在配置文件中对应的tag
    osal_s8 region_str[28]; // 大区名 28字节对齐
    osal_s8 *contry_code;
} wlan_region_table;
#else
/* 大区结构体 */
typedef struct {
    osal_u8 region_idx; // 大区号
    osal_u16 region_tag; // 大区功率在配置文件中对应的tag
    osal_s8 region_str[28]; // 大区名 28字节对齐
} wlan_region_table;
#endif

// 插损定制化项 支持5G的芯片 用宏区分
typedef struct {
    osal_u8 rx_insert_loss_2g[SOC_CUSTOM_RF_FE_RX_LOSS_NUM_2G];
    osal_u8 rsv[SOC_CUSTOM_RF_FE_RX_LOSS_NUM_RSV];
} wlan_cust_rf_fe_rx_insert_loss_params;
// 功率定制化项
typedef struct {
    osal_u8 chip_max_power_2g[SOC_CUSTOM_RF_FE_MAX_POWER_NUM_2G];
    osal_u8 target_power_2g[SOC_CUSTOM_RF_FE_TARGET_POWER_NUM_2G];
    osal_u8 limit_power_2g[SOC_CUSTOM_RF_FE_LIMIT_POWER_NUM_2G];
    osal_u8 sar_power_2g[SOC_CUSTOM_RF_FE_SAR_POWER_NUM_2G];
    osal_u8 special_cta_coef_switch[SOC_CUSTOM_RF_FE_CTA_COEF_SWITCH_2G]; /* 特殊滤波系数标志位 */
    osal_u8 rsv[SOC_CUSTOM_RF_FE_POWER_NUM_RSV];
} wlan_cust_rf_fe_power_params;
typedef struct {
    osal_s16 ref_power_2g[SOC_CUSTOM_RF_FE_REF_POWER_NUM_2G];
    osal_s16 power_cure_2g[SOC_CUSTOM_RF_FE_CURVE_NUM_LVL][SOC_CUSTOM_RF_FE_CURVE_NUM_2G];
    osal_s16 curve_factor_2g[SOC_CUSTOM_RF_FE_FACTOR_NUM_2G];
} wlan_cust_rf_fe_power_cali_params;
typedef struct {
    osal_s8 rssi_comp[SOC_CUSTOM_RF_FE_RSSI_COMP_NUM_2G];
    osal_u8 rssi_sel_bits;
} wlan_cust_rf_fe_rssi_params;
typedef struct {
    wlan_cust_rf_fe_rx_insert_loss_params rf_rx_loss;
    wlan_cust_rf_fe_power_params rf_power;
    wlan_cust_rf_fe_power_cali_params rf_power_cali;
    wlan_cust_rf_fe_rssi_params rf_rssi;
    osal_u32 ini_ret;   // ini 解析状态 1:ok 0:fail 用 WLAN_CFG_INI_RF_FE_TYPE_BUTT 偏移位
} wlan_cust_rf_fe_params;

typedef struct {
    osal_s8 *priv_name;
    wlan_cfg_priv_id_uint8 cfg_singal_id;
    custom_cfgid_h2d_ini_enum syn_singal_id;
} wlan_cust_adapt_priv_params;
extern wlan_cust_country_code_ingore_flag_stru g_st_cust_country_code_ignore_flag;
extern osal_s8 g_external_record_file_path[CUS_PARAMS_LEN_MAX];
extern osal_u32 g_external_record_file_size;

extern int hwifi_config_init_etc(int);
extern int hwifi_get_init_value_etc(int, int);
extern osal_char *hwifi_get_country_code_etc(void);
extern void hwifi_set_country_code_etc(char*, const unsigned int);
extern int hwifi_get_mac_addr_etc(unsigned char *);
extern osal_void hwifi_scan_set_random_mac_addr_scan(osal_u8 random_scan);
extern void *hwifi_wlan_customize_etc(void);
extern int hwifi_is_regdomain_changed_etc(const countrycode_t, const countrycode_t);
extern int hwifi_hcc_customize_h2d_data_cfg(void);
extern int hwifi_hcc_h2d_priv_customize(void);
extern int hwifi_custom_host_read_cfg_init(void);
extern osal_s32 hwifi_get_init_priv_value(osal_s32 l_cfg_id, osal_s32 *pl_priv_value);
extern regdomain_enum hwifi_get_regdomain_from_country_code(const countrycode_t country_code);
extern osal_u16 hwifi_get_hmac_max_ap_num_etc(void);
extern osal_void hwifi_set_hmac_max_ap_num_etc(osal_u16 max_ap_num);
extern osal_u32 hwifi_get_self_healing_enable_etc(osal_void);
extern osal_void hwifi_set_self_healing_enable_etc(osal_u32 self_healing_enable);
extern osal_u32 hwifi_get_self_healing_period_etc(osal_void);
extern osal_void hwifi_set_self_healing_period_etc(osal_u32 self_healing_period);
extern osal_u32 hwifi_get_self_healing_cnt_etc(osal_void);
osal_void hwifi_set_udp_hcc_fc_drop(osal_u8 active);
osal_u8 hwifi_get_udp_hcc_fc_drop(osal_void);
extern osal_void hwifi_set_self_healing_cnt_etc(osal_u32 self_healing_cnt);
osal_s8 hwifi_get_region(const osal_s8 *country_code);
osal_void hwifi_set_nvram_tag_by_region_index(osal_s8 region_idx);
osal_void hwifi_print_nvram_list(osal_void);
extern osal_void hwifi_set_amsdu_num(osal_u8 amsdu_num);
extern osal_u8 hwifi_get_amsdu_num(osal_void);
extern osal_void hwifi_set_amsdu_tx_active(osal_u8 active);
extern osal_u8 hwifi_get_amsdu_tx_active(osal_void);
extern osal_void hwifi_set_ampdu_amsdu_tx_active(osal_u8 active);
extern osal_u8 hwifi_get_ampdu_amsdu_tx_active(osal_void);
extern osal_void hwifi_set_ampdu_tx_max_num(osal_u8 num);
extern osal_u8 hwifi_get_ampdu_tx_max_num(osal_void);
extern osal_void hwifi_set_ampdu_rx_max_num(osal_u8 num);
extern osal_u8 hwifi_get_ampdu_rx_max_num(osal_void);
extern osal_void hwifi_set_ampdu_tx_baw_size(osal_u8 num);
extern osal_u8 hwifi_get_ampdu_tx_baw_size(osal_void);
extern osal_bool hwifi_get_data_sample(osal_void);
extern osal_void hwifi_set_data_sample(osal_bool data_sample);
extern osal_bool hwifi_get_apf_enable(osal_void);
extern osal_void hwifi_set_apf_enable(osal_bool apf_enable);
extern osal_u8 hwifi_get_protocol_enable(void);
extern osal_void hwifi_set_protocol_enable(osal_u8 protocol_enable);
extern osal_u32 hwifi_get_wow_event(osal_void);
extern osal_void hwifi_set_wow_event(osal_u32 wow_event);
extern osal_u32 hwifi_get_smooth_phase_en(osal_void);
extern osal_void hwifi_set_smooth_phase_en(osal_bool smooth_phase_en);
extern osal_u32 hwifi_get_rx_stbc(osal_void);
extern osal_void hwifi_set_rx_stbc(osal_u32 stbc_enable);
extern osal_void hwifi_set_wow_enable(osal_u8 wow_enable);
extern osal_u8 hwifi_get_wow_enable(osal_void);
extern osal_void hwifi_set_sniffer_file_len_max(osal_u8 sniffer_file_len_max);
extern osal_u8 hwifi_get_sniffer_file_len_max(osal_void);
extern wlan_cust_rf_fe_params *hwifi_get_rf_fe_custom_ini(osal_void);
extern osal_void hwifi_get_external_record_file_name(osal_void);
osal_void hwifi_get_configed_ifname(osal_void);
extern osal_u16 hwifi_get_low_current_boot_mode_cali_data_mask(osal_u16 cali_data_mask, osal_u8 first_cali_done_flag);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
extern osal_u32 hwifi_get_ext_coex_en(osal_void);
#endif
// #endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#endif // #if defined(_PRE_PRODUCT_ID_HOST)


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif // soc_customize_wifi.h

