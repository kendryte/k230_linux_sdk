/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: host customize wlan adapt source file
 */

#include <linux/time.h>
#include <linux/etherdevice.h>
#include "soc_customize_wifi_mpxx.h"
#include "soc_ini.h"
#include "oal_plat_type.h"
#include "diag_log_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "oal_main.h"
#include "oal_util.h"
#include "dmac_ext_if_hcm.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "board.h"
#include "hcc_if.h"
#include "hcc_sdio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PRODUCT_ID_HOST)

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_EXT_CUSTOMIZE_WIFI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define DEFAULT_HOST2DEV_SCATT_MAX       64
#define WAL_EXTERMAL_RECORD_FILE_PATH "external_record_file_path"
#define WAL_EXTERMAL_RECORD_FILE_SIZE "external_record_file_size"

osal_void hwifi_custom_adapt_device_ini_perf_param(osal_u8 *data, osal_u32 *data_len);
 
#ifdef _PRE_WLAN_NO_SUPPORT_INI
osal_s32 g_al_host_init_params_etc[WLAN_CFG_INIT_BUTT] = {
    45, // used_mem_for_start,
    25, // used_mem_for_stop,
    8,  // bus_d2h_sched_count,
    12,  // bus_h2d_sched_count,
    // /* 拥塞算法 */
    0, // alg_rx_restore_thres
 
    0x0909, // rf_rx_insertion_loss_2g_b1
    0x0909, // rf_rx_insertion_loss_2g_b2,
    0x0909, // rf_rx_insertion_loss_2g_b3,
 
    /* 5G RF前端 */
    0x1E1E, // rf_rx_insertion_loss_5g_b1
    0x1E1E, // rf_rx_insertion_loss_5g_b2,
    0x1E1E, // rf_rx_insertion_loss_5g_b3,
    0x1E1E, // rf_rx_insertion_loss_5g_b4,
    0x1E1E, // rf_rx_insertion_loss_5g_b5,
    0x1E1E, // rf_rx_insertion_loss_5g_b6,
    0x1E1E, // rf_rx_insertion_loss_5g_b7,
 
    /* 用于定制化计算PWR RF值的偏差 */
    0,          // rf_line_rf_pwr_ref_rssi_db_2g_c0_mult4,
    0,          // rf_line_rf_pwr_ref_rssi_db_2g_c1_mult4,
    0x0000F4F4, // rf_line_rf_pwr_ref_rssi_db_5g_c0_mult4,
    0x0000F4F4, // rf_line_rf_pwr_ref_rssi_db_5g_c1_mult4,
 
    0, // delta_cca_ed_high_20th_2g,
    0, // delta_cca_ed_high_40th_2g,
    0, // delta_cca_ed_high_20th_5g,
    0, // delta_cca_ed_high_40th_5g,
    /* 结温补偿 用于温度保护 */
    5, // junction_temp_diff,
};     /* ini定制化参数数组 */
 
wlan_customize_private_stru g_al_priv_cust_params[WLAN_CFG_PRIV_BUTT] = {
    {0x1FAE, 1}, /* cali_mask */
    {0x14, 1},   /* cali_data_mask */
    {0, 1},      /* cali_auto_cali_mask */
    {1, 1},      /* bus_max_width */
    {0, 0},
    {1, 1}, /* su_bfee */
    {0, 0},
    {0, 0},
    {8, 1}, /* user_num */
    {200, 1},                         /* hmac_m x_ap_num */
    {2, 1},                           /* scan_probe_req_send_times */
    {30, 1},                          /* scan_default_time */
    {2, 1},                           /* scan_default_count */
    {1, 1},                           /* scan_probe_req_all_ie */
    {1, 1},                           /* scan_probe_req_del_wps_para */
    {1, 1},                           /* random_mac_addr_scan */
    {0, 0},
    {12, 1}, /* pcap_file_len_max */
    {1, 1},                 /* ldpc */
    {0, 1},                 /* ba_32bitmap */
    {0, 1},                 /* mtid_aggr_rx */
    {0, 1},                 /* self_healing_enable */
    {5000, 1},              /* self_healing_period */
    {10, 1},                /* self_healing_cnt */
    {50, 1},                /* dbac_sta_gc_ratio */
    {30, 1},                /* dbac_sta_go_ratio */
    {-60, 1},               /* tpc_far_rssi */
    {4, 1},                 /* amsdu_num */
    {1, 1},                 /* amsdu_tx_on */
    {1, 1},                 /* ampdu_amsdu_tx_on */
    {14, 1},                /* ampdu_tx_max_num */
    {32, 1},                /* ampdu_rx_max_num */
    {32, 1},                /* ampdu_tx_baw_size */
    {0, 1},                 /* data_sample */
    {0, 1},                 /* er_su_disable */
    {3, 1},                 /* dcm_constellation_tx */
    {1, 1},                 /* bandwidth_extended_range */
    {1, 1},                 /* apf_enable */
    {15, 1},                /* wow_event */
    {1, 1},                 /* smooth_phase_en */
    {1, 1},                 /* rx_stbc */
    {1, 1},                 /* wow_enable */
    {0, 1},                 /* udp_hcc_fc_drop */
    {-78, 1},               /* roam_trigger_rssi_2g */
    {12, 1},                /* roam_delta_rssi_2g */
    {1, 1},                 /* over_ds_en */
    {0, 1},                 /* protocol_enable */
    {1, 1},                 /* tcp_ack_filter_enable */
    {2, 1},                 /* tcp_ack_max_num_start_process */
    {0, 1},                 /* front_switch */
    {0, 1},                 /* txdata_que_limit */
};                            /* 私有定制化参数数组 */
 
wlan_cust_rf_fe_params g_cust_rf_fe_params = {
    .rf_rx_loss = {
        .rx_insert_loss_2g = {0, 0, 0},
        },
 
    .rf_power_cali = {
        .ref_power_2g = { 180, 180, 180,     /* ref_tx_power_2g_high */
                           90, 90, 90 },     /* ref_tx_power_2g_low */
 
        .power_cure_2g = { { 574, -31, 58,     /* curve_tx_power_2g_high_11b */
                             335, 72, 15,      /* curve_tx_power_2g_high_ofdm20m */
                             426, 6, 55, },       /* curve_tx_power_2g_high_ofdm40m */
                              { 258, 120, -18,    /* curve_tx_power_2g_low_11b */
                                95, 353, -57,     /* curve_tx_power_2g_low_ofdm20m */
                                62, 384, -61 } },     /* curve_tx_power_2g_low_ofdm40m */
 
        .curve_factor_2g = { 26, 12, 0,        /* factor_tx_power_2g_high */
                             25, 13, 0 },        /* factor_tx_power_2g_low */
    },
 
    .rf_rssi = {
        .rssi_comp = {0, 0, 0},     /* rf_rx_rssi_comp_2g */
        .rssi_sel_bits = 0,
    },
    .rf_power = {
        .chip_max_power_2g = { 230 },
        .target_power_2g = { 0x2E, 0x2E, 0x2E, 0x2B, /* target_tx_power_2g_11b */
                             0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x28, 0x26,              /* target_tx_power_2g_11g */
                            /* target_tx_power_2g_20m */
                             0x28, 0x28, 0x28, 0x25, 0x25, 0x25, 0x25, 0x24, 0x21, 0x1E,
                            /* target_tx_power_2g_40m */
                             0x28, 0x28, 0x28, 0x25, 0x25, 0x25, 0x25, 0x24, 0x21, 0x1E, 0x16 },
        .limit_power_2g = { 0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch1 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch2 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch3 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch4 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch5 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch6 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch7 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch8 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch9 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch10 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch11 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch12 */
                            0x3C, 0x3C, 0x3C, 0x3C, /* limit_tx_power_2g_ch13 */
                            0x3C, 0x3C, 0x3C, 0x3C }, /* limit_tx_power_2g_ch14 */
        .sar_power_2g = {0x3C, 0x3C, 0x3C }, /* sar_tx_power_2g */
        .special_cta_coef_switch = { 0 },   /* cta_coef_switch */
    },
    .ini_ret = 0xf /* ini 解析状态 0xf代表均已解析 */
};
osal_char g_ac_country_code_etc[COUNTRY_CODE_LEN] = "CN";
osal_s8 g_external_record_file_path[CUS_PARAMS_LEN_MAX] = "/opt/wifi_external_record.txt";
osal_u32 g_external_record_file_size = 10; /* 对外交互日志文件最大值为10k，可自定义配置 */
static wlan_region_table g_region_table[] = {
    {REGDOMAIN_COMMON, INI_MODU_WIFI, "region_table_default", "CN"}, // 大区0 默认
    {REGDOMAIN_JAPAN, INI_MODU_JP_NVRAM, "region_table_jp", "JP"},
    {REGDOMAIN_FCC, INI_MODU_FCC_NVRAM, "region_table_fcc", "US,CA,KH"},
    {REGDOMAIN_ETSI, INI_MODU_CE_NVRAM, "region_table_ce",
     "RU,AU,MY,ID,TR,PL,FR,PT,IT,DE,ES,AR,ZA,MA,PH,TH,GB,CO,MX,EC,PE,CL,SA,EG,AE"},
};
#else
osal_s32 g_al_host_init_params_etc[WLAN_CFG_INIT_BUTT] = {0};      /* ini定制化参数数组 */
wlan_customize_private_stru g_al_priv_cust_params[WLAN_CFG_PRIV_BUTT] = {{0, 0}};  /* 私有定制化参数数组 */
wlan_cust_rf_fe_params g_cust_rf_fe_params = {0};
osal_char g_ac_country_code_etc[COUNTRY_CODE_LEN] = "00";
/* DFX定制化 */
osal_s8 g_external_record_file_path[CUS_PARAMS_LEN_MAX] = {0};
osal_u32 g_external_record_file_size = 10; /* 对外交互日志文件最大值为10k，可自定义配置 */
OAL_STATIC wlan_cfg_cmd g_wifi_config_priv[] = {
    /* 校准开关 */
    {"cali_mask",                   WLAN_CFG_PRIV_CALI_MASK},
    /* #bit0:开wifi重新校准 bit1:开wifi重新上传 bit2:开机校准 bit3:动态校准调平Debug
       #bit4:不读取NV区域的数据(1:不读取 0：读取) */
    {"cali_data_mask",              WLAN_CFG_PRIV_CALI_DATA_MASK},
    {"cali_auto_cali_mask",         WLAN_CFG_PRIV_CALI_AUTOCALI_MASK},
    {"bw_max_width",                WLAN_CFG_PRIV_BW_MAX_WITH},
    {"su_bfer",                     WLAN_CFG_PRIV_SU_BFER},
    {"su_bfee",                     WLAN_CFG_PRIV_SU_BFEE},
    {"mu_bfer",                     WLAN_CFG_PRIV_MU_BFER},
    {"mu_bfee",                     WLAN_CFG_PRIV_MU_BFEE},
    {"user_num",                    WLAN_CFG_PRIV_USER_NUM},
    {"hmac_max_ap_num",             WLAN_CFG_PRIV_HMAC_MAX_AP_NUM},
    {"scan_probe_req_send_times",   WLAN_CFG_PRIV_SCAN_PROBE_SEND_TIMES},
    {"scan_default_time",           WLAN_CFG_PRIV_SCAN_DEFAULT_TIME},
    {"scan_default_count",          WLAN_CFG_PRIV_SCAN_DEFAULT_COUNT},
    {"scan_probe_req_all_ie",       WLAN_CFG_PRIV_SCAN_PROBE_REQ_ALL_IE},
    {"scan_probe_req_del_wps_para", WLAN_CFG_PRIV_SCAN_PROBE_REQ_DEL_WPS_IE},
    {"random_mac_addr_scan",        WLAN_CFG_PRIV_RANDOM_MAC_ADDR_SCAN},
    {"random_scan_oui",             WLAN_CFG_PRIV_RANDOM_SCAN_OUI},
    {"pcap_file_len_max",           WLAN_CFG_PRIV_PCAP_FILE_LEN_MAX},
    {"ldpc",                        WLAN_CFG_PRIV_LDPC},
    {"ba_32bitmap",                 WLAN_CFG_PRIV_BA_32BIT_BITMAP},
    {"mtid_aggr_rx",                WLAN_CFG_PRIV_MTID_AGGR_RX},
    {"self_healing_enable",         WLAN_CFG_PRIV_SELF_HEALING_ENABLE},
    {"self_healing_period",         WLAN_CFG_PRIV_SELF_HEALING_PERIOD},
    {"self_healing_cnt",            WLAN_CFG_PRIV_SELF_HEALING_CNT},
    {"dbac_sta_gc_ratio",           WLAN_CFG_PRIV_DBAC_STA_GC_SLOT_RATIO},
    {"dbac_sta_go_ratio",           WLAN_CFG_PRIV_DBAC_STA_GO_SLOT_RATIO},
    {"tpc_far_rssi",                WLAN_CFG_PRIV_TPC_FAR_RSSI},
    {"amsdu_num",                   WLAN_CFG_PRIV_AMSDU_NUM},
    {"amsdu_tx_on",                 WLAN_CFG_PRIV_AMSDU_TX_ON},
    {"ampdu_amsdu_tx_on",           WLAN_CFG_PRIV_AMPDU_AMSDU_TX_ON},
    {"ampdu_tx_max_num",            WLAN_CFG_INIT_AMPDU_TX_MAX_NUM},
    {"ampdu_rx_max_num",            WLAN_CFG_INIT_AMPDU_RX_MAX_NUM},
    {"ampdu_tx_baw_size",           WLAN_CFG_INIT_AMPDU_TX_BAW_SIZE},
    {"data_sample",                 WLAN_CFG_PRIV_DATA_SAMPLE},
    {"er_su_disable",               WLAN_CFG_PRIV_ER_SU_DISABLE},
    {"dcm_constellation_tx",        WLAN_CFG_PRIV_DCM_CONSTELLATION_TX},
    {"bandwidth_extended_range",    WLAN_CFG_PRIV_BANDWIDTH_EXTENDED_RANGE},
	{"apf_enable",                  WLAN_CFG_PRIV_APF_ENABLE},
    {"wow_event",                   WLAN_CFG_PRIV_WOW_EVENT},
	{"smooth_phase_en",             WLAN_CFG_PRIV_SMOOTH_PHASE_EN},
    {"rx_stbc",                     WLAN_CFG_PRIV_RX_STBC},
    {"wow_enable",                  WLAN_CFG_PRIV_WOW_ENABLE},
    {"udp_hcc_fc_drop",             WLAN_CFG_PRIV_UDP_HCC_FC_DROP},
    {"roam_trigger_rssi_2g",        WLAN_CFG_PRIV_ROAM_TRIGGER_RSSI_2G},
    {"roam_delta_rssi_2g",          WLAN_CFG_PRIV_ROAM_DELTA_RSSI_2G},
    {"over_ds_en",                  WLAN_CFG_PRIV_OVER_DS_EN},
    {"protocol_enable",             WLAN_CFG_PRIV_PROTOCOL_ENABLE},
    {"tcp_ack_filter_enable",       WLAN_CFG_PRIV_TCP_ACK_FILTER_EN},
    {"tcp_ack_max_num_start_process", WLAN_CFG_PRIV_TCP_ACK_MAX_NUM_START_PROCESS},
    {"front_switch",                WLAN_CFG_PRIV_FRONT_SWITCH},
    {"txdata_que_limit",            WLAN_CFG_PRIV_TXDATA_QUE_LIMIT},
    {OAL_PTR_NULL, 0}
};
 
OAL_STATIC wlan_cfg_cmd g_wifi_config_cmds[] = {
    /* 性能 */
    {"used_mem_for_start",              WLAN_CFG_INIT_USED_MEM_FOR_START},
    {"used_mem_for_stop",               WLAN_CFG_INIT_USED_MEM_FOR_STOP},
    {"bus_d2h_sched_count",             WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT},
    {"bus_h2d_sched_count",             WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT},
    /* 流控算法 */
    {"alg_rx_restore_thres",            WLAN_CFG_INIT_RX_RESTORE_THRES},
    /* 2G RF前端插损 */
    {"rf_rx_insertion_loss_2g_b1",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1},
    {"rf_rx_insertion_loss_2g_b2",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2},
    {"rf_rx_insertion_loss_2g_b3",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3},
    /* 5G RF前端插损 */
    {"rf_rx_insertion_loss_5g_b1",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1},
    {"rf_rx_insertion_loss_5g_b2",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2},
    {"rf_rx_insertion_loss_5g_b3",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3},
    {"rf_rx_insertion_loss_5g_b4",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4},
    {"rf_rx_insertion_loss_5g_b5",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5},
    {"rf_rx_insertion_loss_5g_b6",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6},
    {"rf_rx_insertion_loss_5g_b7",     WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7},
    /* 用于定制化计算PWR RF值的偏差 */
    {"rf_line_rf_pwr_ref_rssi_db_2g_c0_mult4",     WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4},
    {"rf_line_rf_pwr_ref_rssi_db_2g_c1_mult4",     WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4},
    {"rf_line_rf_pwr_ref_rssi_db_5g_c0_mult4",     WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4},
    {"rf_line_rf_pwr_ref_rssi_db_5g_c1_mult4",     WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4},
 
    {"delta_cca_ed_high_20th_2g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G},
    {"delta_cca_ed_high_40th_2g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G},
    {"delta_cca_ed_high_20th_5g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G},
    {"delta_cca_ed_high_40th_5g",       WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G},
    /* 结温温度偏差补偿 用于温度保护 */
    {"junction_temp_diff",              WLAN_CFG_INIT_JUNCTION_TEMP_DIFF},
};
 
OAL_STATIC wlan_cfg_cmd g_rf_fe_param_config_ini[] = {
    {"rf_rx_insert_loss_2g",      WLAN_CFG_INI_RF_FE_RX_INSERT_LOSS_2G}
};
OAL_STATIC wlan_cfg_cmd g_rf_fe_power_config_ini[] = {
    {"rf_chip_max_power_2g",    WLAN_CFG_INI_RF_FE_POWER_CHIP_MAX_2G},
    {"target_tx_power_2g_11b",  WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_11B},
    {"target_tx_power_2g_11g",  WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_11G},
    {"target_tx_power_2g_20m",  WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_20M},
    {"target_tx_power_2g_40m",  WLAN_CFG_INI_RF_FE_POWER_TARGET_2G_40M},
    {"limit_tx_power_2g_ch1",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH1},
    {"limit_tx_power_2g_ch2",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH2},
    {"limit_tx_power_2g_ch3",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH3},
    {"limit_tx_power_2g_ch4",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH4},
    {"limit_tx_power_2g_ch5",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH5},
    {"limit_tx_power_2g_ch6",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH6},
    {"limit_tx_power_2g_ch7",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH7},
    {"limit_tx_power_2g_ch8",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH8},
    {"limit_tx_power_2g_ch9",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH9},
    {"limit_tx_power_2g_ch10",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH10},
    {"limit_tx_power_2g_ch11",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH11},
    {"limit_tx_power_2g_ch12",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH12},
    {"limit_tx_power_2g_ch13",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH13},
    {"limit_tx_power_2g_ch14",  WLAN_CFG_INI_RF_FE_POWER_LIMIT_2G_CH14},
    {"sar_tx_power_2g",         WLAN_CFG_INI_RF_FE_POWER_SAR_2G},
    {"cta_coef_switch",         WLAN_CFG_INI_RF_FE_CTA_COEF_SWITCH}
};
OAL_STATIC wlan_cfg_cmd g_rf_fe_power_cali_config_ini[] = {
    {"ref_tx_power_2g_high",            WLAN_CFG_INI_RF_FE_POWER_REF_2G_HIGH},
    {"ref_tx_power_2g_low",             WLAN_CFG_INI_RF_FE_POWER_REF_2G_LOW},
    {"curve_tx_power_2g_high_11b",      WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_11B},
    {"curve_tx_power_2g_high_ofdm20m",  WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_OFDM20M},
    {"curve_tx_power_2g_high_ofdm40m",  WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_HIGH_OFDM40M},
    {"curve_tx_power_2g_low_11b",       WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_11B},
    {"curve_tx_power_2g_low_ofdm20m",   WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_OFDM20M},
    {"curve_tx_power_2g_low_ofdm40m",   WLAN_CFG_INI_RF_FE_POWER_CURVE_2G_LOW_OFDM40M},
    {"factor_tx_power_2g_high",         WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_HIGH},
    {"factor_tx_power_2g_low",          WLAN_CFG_INI_RF_FE_CURVE_FACTOR_2G_LOW}
};
 
OAL_STATIC wlan_cfg_cmd g_rf_fe_rssi_config_ini[] = {
    {"rf_rx_rssi_comp_2g",            WLAN_CFG_INI_RF_FE_RX_RSSI_COMP_2G},
};
 
static wlan_region_table g_region_table[] = {
    {REGDOMAIN_COMMON, INI_MODU_WIFI, "region_table_default"}, // 大区0 默认
    {REGDOMAIN_JAPAN, INI_MODU_JP_NVRAM, "region_table_jp"},
    {REGDOMAIN_FCC, INI_MODU_FCC_NVRAM, "region_table_fcc"},
    {REGDOMAIN_ETSI, INI_MODU_CE_NVRAM, "region_table_ce"},
};
#endif /* _PRE_WLAN_NO_SUPPORT_INI */

/*
 * 2 Global Variable Definition
 */
wlan_cust_country_code_ingore_flag_stru g_st_cust_country_code_ignore_flag = {0}; /* 定制化国家码配置 */
osal_u16 g_nvram_tag = INI_MODU_WIFI;
osal_u8 g_uc_wlan_open_cnt = 0;
osal_u8 g_protocol_enable = 0; /* 强制配置AP/GO为11AX模式开关 */
osal_u16 g_hmac_max_ap_num = 0;
osal_u32 g_self_healing_enable = 0; /* 异常自愈开关，默认不打开 */
osal_u32 g_self_healing_period = 5000; /* 异常自愈检测周期，默认5000ms */
osal_u32 g_self_healing_cnt = 10; /* 异常自愈上，表示上报100次，上报频率减少10倍 */
osal_u8 g_amsdu_max_num = 4; /* 最大amsdu聚合个数 */
osal_u8 g_amsdu_tx_active = 1; /* amsdu_tx_on开关，默认开启 */
osal_u8 g_ampdu_amsdu_tx_active = 1; /* ampdu_amsdu_tx_on开关，默认开启 */
osal_u8 g_ampdu_tx_max_num = 16; /* TX ampdu最大聚合个数，默认值16 */
osal_u8 g_ampdu_rx_max_num = 32; /* RX ampdu最大聚合个数，默认值32（最大接收32聚合度的报文） */
osal_u8 g_ampdu_tx_baw_size = 32; /* TX聚合窗口大小，默认值32（发送报文聚合度需不超过窗口1/2） */
osal_u8 g_udp_hcc_fc_drop = 0; /* UDP丢包反压HCC_FC_DROP默认关闭，只有在过认证的时候开启 */
osal_bool g_data_sample = 0;
osal_bool g_apf_enable = 1;
osal_bool g_smooth_phase_en = 0;
osal_u32 g_wow_event = 0xf;
osal_u8 g_wow_enable = 1;
osal_u8 g_sniffer_file_len_max = 12; /* sniffer文件大小最大值默认为12M，可根据不同芯片进行配置 */
osal_u32 g_stbc_enable = 1;
/*
 * 定制化结构体
 * default values as follows:
 * ampdu_tx_max_num:            WLAN_AMPDU_TX_MAX_NUM               = 64
 * switch:                      ON                                  = 1
 * scan_orthogonal:             ROAM_SCAN_CHANNEL_ORG_BUTT          = 4
 */
wlan_customize_stru g_wlan_customize_etc = {
    64,  /* addba_buffer_size */
    1,   /* roam switch */
    4,   /* roam scan org */
    -78, /* roam trigger 2G */
    -128, /* roam trigger 5G */
    12,  /* roam delta 2G */
    12,  /* roam delta 5G */
    0,
    0,
    0,
    0,
    0,
    0,
    1,  /* random mac addr scan */
    0,  /* disable_capab_2ght40 */
    0,  /* lte_gpio_check_switch */
    0,  /* ism_priority */
    0,  /* lte_rx */
    77, /* lte_tx */
    76, /* lte_inact */
    0,  /* ism_rx_act */
    0,  /* bant_pri */
    0,  /* bant_status */
    0,  /* want_pri */
    0   /* want_status */
};

/**
 *  regdomain <-> country code map table
 *  max support country num: MAX_COUNTRY_COUNT
 *
 **/
OAL_STATIC countryinfo_stru g_country_info_table[] = {
    /* Note:too few initializers for unsigned char [3] */
    {REGDOMAIN_COMMON, {'0', '0'}}, // WORLD DOMAIN
    {REGDOMAIN_FCC, {'A', 'D'}}, // ANDORRA
    {REGDOMAIN_ETSI, {'A', 'E'}}, // UAE
    {REGDOMAIN_ETSI, {'A', 'L'}}, // ALBANIA
    {REGDOMAIN_ETSI, {'A', 'M'}}, // ARMENIA
    {REGDOMAIN_ETSI, {'A', 'N'}}, // NETHERLANDS ANTILLES
    {REGDOMAIN_FCC, {'A', 'R'}}, // ARGENTINA
    {REGDOMAIN_FCC, {'A', 'S'}}, // AMERICAN SOMOA
    {REGDOMAIN_ETSI, {'A', 'T'}}, // AUSTRIA
    {REGDOMAIN_FCC, {'A', 'U'}}, // AUSTRALIA
    {REGDOMAIN_ETSI, {'A', 'W'}}, // ARUBA
    {REGDOMAIN_ETSI, {'A', 'Z'}}, // AZERBAIJAN
    {REGDOMAIN_ETSI, {'B', 'A'}}, // BOSNIA AND HERZEGOVINA
    {REGDOMAIN_FCC, {'B', 'B'}}, // BARBADOS
    {REGDOMAIN_ETSI, {'B', 'D'}}, // BANGLADESH
    {REGDOMAIN_ETSI, {'B', 'E'}}, // BELGIUM
    {REGDOMAIN_ETSI, {'B', 'G'}}, // BULGARIA
    {REGDOMAIN_ETSI, {'B', 'H'}}, // BAHRAIN
    {REGDOMAIN_ETSI, {'B', 'L'}}, //
    {REGDOMAIN_FCC, {'B', 'M'}}, // BERMUDA
    {REGDOMAIN_ETSI, {'B', 'N'}}, // BRUNEI DARUSSALAM
    {REGDOMAIN_ETSI, {'B', 'O'}}, // BOLIVIA
    {REGDOMAIN_ETSI, {'B', 'R'}}, // BRAZIL
    {REGDOMAIN_FCC, {'B', 'S'}}, // BAHAMAS
    {REGDOMAIN_ETSI, {'B', 'Y'}}, // BELARUS
    {REGDOMAIN_ETSI, {'B', 'Z'}}, // BELIZE
    {REGDOMAIN_FCC, {'C', 'A'}}, // CANADA
    {REGDOMAIN_ETSI, {'C', 'H'}}, // SWITZERLAND
    {REGDOMAIN_ETSI, {'C', 'L'}}, // CHILE
    {REGDOMAIN_COMMON, {'C', 'N'}}, // CHINA
    {REGDOMAIN_FCC, {'C', 'O'}}, // COLOMBIA
    {REGDOMAIN_ETSI, {'C', 'R'}}, // COSTA RICA
    {REGDOMAIN_ETSI, {'C', 'S'}},
    {REGDOMAIN_ETSI, {'C', 'Y'}}, // CYPRUS
    {REGDOMAIN_ETSI, {'C', 'Z'}}, // CZECH REPUBLIC
    {REGDOMAIN_ETSI, {'D', 'E'}}, // GERMANY
    {REGDOMAIN_ETSI, {'D', 'K'}}, // DENMARK
    {REGDOMAIN_FCC, {'D', 'O'}}, // DOMINICAN REPUBLIC
    {REGDOMAIN_ETSI, {'D', 'Z'}}, // ALGERIA
    {REGDOMAIN_ETSI, {'E', 'C'}}, // ECUADOR
    {REGDOMAIN_ETSI, {'E', 'E'}}, // ESTONIA
    {REGDOMAIN_ETSI, {'E', 'G'}}, // EGYPT
    {REGDOMAIN_ETSI, {'E', 'S'}}, // SPAIN
    {REGDOMAIN_ETSI, {'F', 'I'}}, // FINLAND
    {REGDOMAIN_ETSI, {'F', 'R'}}, // FRANCE
    {REGDOMAIN_ETSI, {'G', 'B'}}, // UNITED KINGDOM
    {REGDOMAIN_FCC, {'G', 'D'}},  // GRENADA
    {REGDOMAIN_ETSI, {'G', 'E'}}, // GEORGIA
    {REGDOMAIN_ETSI, {'G', 'F'}}, // FRENCH GUIANA
    {REGDOMAIN_ETSI, {'G', 'L'}}, // GREENLAND
    {REGDOMAIN_ETSI, {'G', 'P'}}, // GUADELOUPE
    {REGDOMAIN_ETSI, {'G', 'R'}}, // GREECE
    {REGDOMAIN_FCC, {'G', 'T'}},  // GUATEMALA
    {REGDOMAIN_FCC, {'G', 'U'}},  // GUAM
    {REGDOMAIN_ETSI, {'H', 'U'}}, // HUNGARY
    {REGDOMAIN_FCC, {'I', 'D'}},  // INDONESIA
    {REGDOMAIN_ETSI, {'I', 'E'}}, // IRELAND
    {REGDOMAIN_ETSI, {'I', 'L'}}, // ISRAEL
    {REGDOMAIN_ETSI, {'I', 'N'}}, // INDIA
    {REGDOMAIN_ETSI, {'I', 'R'}}, // IRAN, ISLAMIC REPUBLIC OF
    {REGDOMAIN_ETSI, {'I', 'S'}}, // ICELNAD
    {REGDOMAIN_ETSI, {'I', 'T'}}, // ITALY
    {REGDOMAIN_FCC, {'J', 'M'}},  // JAMAICA
    {REGDOMAIN_JAPAN, {'J', 'P'}}, // JAPAN
    {REGDOMAIN_ETSI, {'J', 'O'}}, // JORDAN
    {REGDOMAIN_ETSI, {'K', 'E'}}, // KENYA
    {REGDOMAIN_ETSI, {'K', 'H'}}, // CAMBODIA
    {REGDOMAIN_ETSI, {'K', 'P'}}, // KOREA, DEMOCRATIC PEOPLE's REPUBLIC OF
    {REGDOMAIN_ETSI, {'K', 'R'}}, // KOREA, REPUBLIC OF
    {REGDOMAIN_ETSI, {'K', 'W'}}, // KUWAIT
    {REGDOMAIN_ETSI, {'K', 'Z'}}, // KAZAKHSTAN
    {REGDOMAIN_ETSI, {'L', 'B'}}, // LEBANON
    {REGDOMAIN_ETSI, {'L', 'I'}}, // LIECHTENSTEIN
    {REGDOMAIN_ETSI, {'L', 'K'}}, // SRI-LANKA
    {REGDOMAIN_ETSI, {'L', 'T'}}, // LITHUANIA
    {REGDOMAIN_ETSI, {'L', 'U'}}, // LUXEMBOURG
    {REGDOMAIN_ETSI, {'L', 'V'}},  // LATVIA
    {REGDOMAIN_ETSI, {'M', 'A'}}, // MOROCCO
    {REGDOMAIN_ETSI, {'M', 'C'}}, // MONACO
    {REGDOMAIN_ETSI, {'M', 'K'}}, // MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
    {REGDOMAIN_FCC, {'M', 'N'}}, // MONGOLIA
    {REGDOMAIN_FCC, {'M', 'O'}}, // MACAO
    {REGDOMAIN_FCC, {'M', 'P'}}, // NORTHERN MARIANA ISLANDS
    {REGDOMAIN_ETSI, {'M', 'Q'}}, // MARTINIQUE
    {REGDOMAIN_FCC, {'M', 'T'}}, // MALTA
    {REGDOMAIN_ETSI, {'M', 'U'}}, // MAURITIUS
    {REGDOMAIN_ETSI, {'M', 'W'}}, // MALAWI
    {REGDOMAIN_FCC, {'M', 'X'}}, // MEXICO
    {REGDOMAIN_ETSI, {'M', 'Y'}}, // MALAYSIA
    {REGDOMAIN_ETSI, {'N', 'G'}}, // NIGERIA
    {REGDOMAIN_FCC, {'N', 'I'}}, // NICARAGUA
    {REGDOMAIN_ETSI, {'N', 'L'}}, // NETHERLANDS
    {REGDOMAIN_ETSI, {'N', 'O'}}, // NORWAY
    {REGDOMAIN_ETSI, {'N', 'P'}}, // NEPAL
    {REGDOMAIN_FCC, {'N', 'Z'}}, // NEW-ZEALAND
    {REGDOMAIN_FCC, {'O', 'M'}}, // OMAN
    {REGDOMAIN_FCC, {'P', 'A'}}, // PANAMA
    {REGDOMAIN_ETSI, {'P', 'E'}}, // PERU
    {REGDOMAIN_ETSI, {'P', 'F'}}, // FRENCH POLYNESIA
    {REGDOMAIN_ETSI, {'P', 'G'}}, // PAPUA NEW GUINEA
    {REGDOMAIN_FCC, {'P', 'H'}}, // PHILIPPINES
    {REGDOMAIN_ETSI, {'P', 'K'}}, // PAKISTAN
    {REGDOMAIN_ETSI, {'P', 'L'}}, // POLAND
    {REGDOMAIN_FCC, {'P', 'R'}}, // PUERTO RICO
    {REGDOMAIN_FCC, {'P', 'S'}}, // PALESTINIAN TERRITORY, OCCUPIED
    {REGDOMAIN_ETSI, {'P', 'T'}}, // PORTUGAL
    {REGDOMAIN_FCC, {'P', 'Y'}}, // PARAGUAY
    {REGDOMAIN_ETSI, {'Q', 'A'}}, // QATAR
    {REGDOMAIN_ETSI, {'R', 'E'}}, // REUNION
    {REGDOMAIN_ETSI, {'R', 'O'}}, // ROMAINIA
    {REGDOMAIN_ETSI, {'R', 'S'}}, // SERBIA
    {REGDOMAIN_ETSI, {'R', 'U'}}, // RUSSIA
    {REGDOMAIN_FCC, {'R', 'W'}}, // RWANDA
    {REGDOMAIN_ETSI, {'S', 'A'}}, // SAUDI ARABIA
    {REGDOMAIN_ETSI, {'S', 'E'}}, // SWEDEN
    {REGDOMAIN_ETSI, {'S', 'G'}}, // SINGAPORE
    {REGDOMAIN_ETSI, {'S', 'I'}}, // SLOVENNIA
    {REGDOMAIN_ETSI, {'S', 'K'}}, // SLOVAKIA
    {REGDOMAIN_ETSI, {'S', 'V'}}, // EL SALVADOR
    {REGDOMAIN_ETSI, {'S', 'Y'}}, // SYRIAN ARAB REPUBLIC
    {REGDOMAIN_ETSI, {'T', 'H'}}, // THAILAND
    {REGDOMAIN_ETSI, {'T', 'N'}}, // TUNISIA
    {REGDOMAIN_ETSI, {'T', 'R'}}, // TURKEY
    {REGDOMAIN_ETSI, {'T', 'T'}}, // TRINIDAD AND TOBAGO
    {REGDOMAIN_FCC, {'T', 'W'}}, // TAIWAN, PRIVINCE OF CHINA
    {REGDOMAIN_FCC, {'T', 'Z'}}, // TANZANIA, UNITED REPUBLIC OF
    {REGDOMAIN_ETSI, {'U', 'A'}}, // UKRAINE
    {REGDOMAIN_ETSI, {'U', 'G'}}, // UGANDA
    {REGDOMAIN_FCC, {'U', 'S'}}, // USA
    {REGDOMAIN_ETSI, {'U', 'Y'}}, // URUGUAY
    {REGDOMAIN_FCC, {'U', 'Z'}}, // UZBEKISTAN
    {REGDOMAIN_ETSI, {'V', 'E'}}, // VENEZUELA
    {REGDOMAIN_FCC, {'V', 'I'}}, // VIRGIN ISLANDS, US
    {REGDOMAIN_ETSI, {'V', 'N'}}, // VIETNAM
    {REGDOMAIN_ETSI, {'Y', 'E'}}, // YEMEN
    {REGDOMAIN_ETSI, {'Y', 'T'}}, // MAYOTTE
    {REGDOMAIN_ETSI, {'Z', 'A'}}, // SOUTH AFRICA
    {REGDOMAIN_ETSI, {'Z', 'W'}}, // ZIMBABWE

    {REGDOMAIN_COUNT, {'9', '9'}}
};

OAL_STATIC wlan_cust_adapt_priv_params  g_cust_adapt_priv_params_ini[] = {
    {"max_bw", WLAN_CFG_PRIV_BW_MAX_WITH, CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID},
    {"su bfer", WLAN_CFG_PRIV_SU_BFER, CUSTOM_CFGID_PRIV_INI_SU_BFER_ID},
    {"su bfee", WLAN_CFG_PRIV_SU_BFEE, CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID},
    {"mu bfer", WLAN_CFG_PRIV_MU_BFER, CUSTOM_CFGID_PRIV_INI_MU_BFER_ID},
    {"mu bfee", WLAN_CFG_PRIV_MU_BFEE, CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID},
    {"ldpc", WLAN_CFG_PRIV_LDPC, CUSTOM_CFGID_PRIV_INI_LDPC_ID},
    {"front_switch", WLAN_CFG_PRIV_FRONT_SWITCH, CUSTOM_CFGID_PRIV_INI_FRONT_SWITCH_ID},
    {"priv_cali_data_up_down", WLAN_CFG_PRIV_CALI_DATA_MASK, CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID},
    {"auto_cali_mask", WLAN_CFG_PRIV_CALI_AUTOCALI_MASK, CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID}
};

/*****************************************************************************
 功能描述  : 根据国家码找到对应的regdomain
*****************************************************************************/
regdomain_enum hwifi_get_regdomain_from_country_code(const countrycode_t country_code)
{
    regdomain_enum  regdomain = REGDOMAIN_COMMON;
    osal_s32           table_idx = 0;

    while (g_country_info_table[table_idx].regdomain != REGDOMAIN_COUNT) {
        if (memcmp(country_code, g_country_info_table[table_idx].auc_country_code, COUNTRY_CODE_LEN) == 0) {
            regdomain = g_country_info_table[table_idx].regdomain;
            break;
        }
        ++table_idx;
    }

    return regdomain;
}

/*****************************************************************************
 功能描述  : 国家码改变后，对应的regdomain是否有变化
*****************************************************************************/
osal_s32 hwifi_is_regdomain_changed_etc(const countrycode_t country_code_old, const countrycode_t country_code_new)
{
    return hwifi_get_regdomain_from_country_code(country_code_old) !=
        hwifi_get_regdomain_from_country_code(country_code_new);
}

/*****************************************************************************
 功能描述  : 配置定制化参数结束标志
*****************************************************************************/
OAL_STATIC osal_void hwifi_custom_adapt_device_ini_end_param (osal_u8 *data, osal_u32 *data_len)
{
    hmac_to_dmac_cfg_custom_data_stru     syn_msg;

    if (data == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param data::NULL data_len[%d].}", *data_len);
        return;
    }

    syn_msg.syn_id = CUSTOM_CFGID_INI_ENDING_ID;
    syn_msg.len    = 0;

    if (memcpy_s(data, OAL_SIZEOF(syn_msg), &syn_msg, OAL_SIZEOF(syn_msg)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param data::memcpy err.}");
    }

    *data_len += OAL_SIZEOF(syn_msg);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_end_param::da_len[%d].}", *data_len);
}

osal_void hwifi_custom_cali_priv_ini_param(osal_u8 *priv_cfg_value)
{
    /* 开机默认打开校准数据上传下发 */
    g_uc_wlan_open_cnt++;
    *priv_cfg_value = hwifi_get_low_current_boot_mode_cali_data_mask(*priv_cfg_value, g_uc_custom_cali_done_etc);
}

/*****************************************************************************
 功能描述  : 下发私有开机device配置定制化项到device
*****************************************************************************/
OAL_STATIC osal_s32 hwifi_custom_adapt_priv_ini_param(wlan_cfg_priv_id_uint8 cfg_id, osal_u8 *data, osal_u32 *len)
{
    osal_s32 ret, cfg_idx;
    hmac_to_dmac_cfg_custom_data_stru syn_msg;
    osal_s32 priv_val = 0 ;
    osal_u8 priv_cfg_value;

    if (data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_mac_device_priv_ini_param::data is NULL data_len[%d].}", *len);
        return OAL_FAIL;
    }

    ret = hwifi_get_init_priv_value(cfg_id, &priv_val);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    priv_cfg_value = (osal_u8)(osal_u32)priv_val;

    for (cfg_idx = 0; cfg_idx < OAL_ARRAY_SIZE(g_cust_adapt_priv_params_ini); cfg_idx++) {
        if (cfg_id == WLAN_CFG_PRIV_CALI_DATA_MASK) {
            hwifi_custom_cali_priv_ini_param(&priv_cfg_value);
            syn_msg.syn_id = CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID;
            wifi_printf(
                "hwifi_custom_adapt_mac_device_priv_ini_param::g_uc_wlan_open_cnt[%d]priv_cali_data_up_down[0x%x]\r\n",
                g_uc_wlan_open_cnt, priv_cfg_value);
            break;
        }
        if (cfg_id == WLAN_CFG_PRIV_CALI_AUTOCALI_MASK) {
            /* 开机默认不打开开机校准 */
            priv_cfg_value = (g_uc_custom_cali_done_etc == OAL_FALSE) ? OAL_FALSE : priv_cfg_value;
            syn_msg.syn_id = CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID;
            wifi_printf(
                "hwifi_custom_adapt_mac_device_priv_ini_param::g_uc_custom_cali_done_etc[%d]auto_cali_mask[0x%x]\r\n",
                g_uc_custom_cali_done_etc, priv_cfg_value);
            break;
        }
        if (g_cust_adapt_priv_params_ini[cfg_idx].cfg_singal_id == cfg_id) {
            syn_msg.syn_id = g_cust_adapt_priv_params_ini[cfg_idx].syn_singal_id;
            wifi_printf("hwifi_custom_adapt_mac_device_priv_ini_param::%s[%d].\r\n",
                g_cust_adapt_priv_params_ini[cfg_idx].priv_name, priv_cfg_value);
            break;
        }
    }

    syn_msg.len = (osal_u32)OAL_SIZEOF(priv_cfg_value);
    if (memcpy_s(data, CUSTOM_MSG_DATA_HDR_LEN, &syn_msg, CUSTOM_MSG_DATA_HDR_LEN) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_mac_device_priv_ini_param data::memcpy err.}");
    }
    if (memcpy_s(data + CUSTOM_MSG_DATA_HDR_LEN, OAL_SIZEOF(priv_cfg_value),
        &priv_cfg_value, OAL_SIZEOF(priv_cfg_value)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_mac_device_priv_ini_param data::memcpy p2 err.}");
    }

    *len += ((osal_u32)sizeof(priv_cfg_value) + CUSTOM_MSG_DATA_HDR_LEN);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 下发私有开机校准配置定制化项到device
*****************************************************************************/
OAL_STATIC osal_s32 hwifi_custom_adapt_device_priv_ini_cali_mask_param(osal_u8 *data, osal_u32 *data_len)
{
    osal_s32                              ret;
    hmac_to_dmac_cfg_custom_data_stru      syn_msg;
    osal_s32                              priv_val   = 0 ;
    osal_u16                             cali_mask;

    if (data == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hwifi_custom_adapt_device_priv_ini_cali_mask_param::data is NULL data_len[%d].}", *data_len);
        return OAL_FAIL;
    }

    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_MASK, &priv_val);
    if (ret == OAL_SUCC) {
        cali_mask = (osal_u16)(osal_u32)priv_val;
        wifi_printf("hwifi_custom_adapt_device_priv_ini_cali_mask_param::read cali_mask[%d]ret[%d]\r\n",
            cali_mask, ret);
    } else {
        return OAL_FAIL;
    }

    syn_msg.syn_id = CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID;
    syn_msg.len = OAL_SIZEOF(cali_mask) ;
    if (memcpy_s(data, CUSTOM_MSG_DATA_HDR_LEN, &syn_msg, CUSTOM_MSG_DATA_HDR_LEN) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_cali_mask_param data::memcpy err.}");
    }
    if (memcpy_s(data + CUSTOM_MSG_DATA_HDR_LEN, OAL_SIZEOF(cali_mask), &cali_mask, OAL_SIZEOF(cali_mask)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_cali_mask_param data p2::memcpy err.}");
    }

    *data_len += (OAL_SIZEOF(cali_mask) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log2(0, OAM_SF_CFG,
        "{hwifi_custom_adapt_device_priv_ini_cali_mask_param::da_len[%d] cali_mask[0x%x].}", *data_len, cali_mask);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : ini device侧上电前定制化参数适配
*****************************************************************************/
osal_u32 hwifi_custom_adapt_device_ini_param(osal_u8 *data)
{
    osal_u32                     data_len = 0;

    if (data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_param::data is NULL.}");
        return INI_FAILED;
    }

    /* 发送消息的格式如下:                                                   */
    /* +-------------------------------------------------------------------+ */
    /* | CFGID0    |DATA0 Length| DATA0 Value | ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* | 4 Bytes   |4 Byte      | DATA  Length| ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* 性能 */
    hwifi_custom_adapt_device_ini_perf_param(data + data_len, &data_len);

    /* 结束 */
    hwifi_custom_adapt_device_ini_end_param(data + data_len, &data_len);

    return data_len;
}

/*****************************************************************************
 功能描述  : ini device侧上电前定制化参数适配
*****************************************************************************/
osal_u32 hwifi_custom_adapt_device_priv_ini_param(osal_u8 *data)
{
    osal_u32                     data_len = 0;

    if (data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_priv_ini_param::data is NULL.}");
        return INI_FAILED;
    }

    /* 发送消息的格式如下:                                                   */
    /* +-------------------------------------------------------------------+ */
    /* | CFGID0    |DATA0 Length| DATA0 Value | ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* | 4 Bytes   |4 Byte      | DATA  Length| ......................... | */
    /* +-------------------------------------------------------------------+ */

    /* 私有定制化 */
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_LDPC, data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_FRONT_SWITCH, data + data_len, &data_len);
    hwifi_custom_adapt_device_priv_ini_cali_mask_param(data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CALI_DATA_MASK, data + data_len, &data_len);
    hwifi_custom_adapt_priv_ini_param(WLAN_CFG_PRIV_CALI_AUTOCALI_MASK, data + data_len, &data_len);
    wifi_printf("hwifi_custom_adapt_device_priv_ini_param::data_len[%d]\r\n", data_len);

    return data_len;
}

/*****************************************************************************
 功能描述  : 下发定制化配置命令
*****************************************************************************/
osal_s32 hwifi_hcc_custom_ini_data_buf(osal_u16 syn_id)
{
    osal_u32 data_len = 0;
    osal_s32 ret;
    osal_u8 *tx_buf;
    osal_u8 *data_buf;
    hcc_transfer_param hcc_transfer_param = {0};

    tx_buf = kzalloc(WLAN_LARGE_NETBUF_SIZE, GFP_KERNEL);
    if (tx_buf == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::alloc tx_buf fail.");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    data_buf = tx_buf + hcc_get_head_len();

    if (syn_id == CUSTOM_CFGID_INI_ID) {
        /* INI hmac to dmac 配置项 */
        data_len = hwifi_custom_adapt_device_ini_param(data_buf);
    } else if (syn_id == CUSTOM_CFGID_PRIV_INI_ID) {
        /* 私有定制化配置项 */
        data_len = hwifi_custom_adapt_device_priv_ini_param(data_buf);
    } else {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::unknown syn_id[%d]", syn_id);
    }

    if ((data_len + hcc_get_head_len() > WLAN_LARGE_NETBUF_SIZE) || (data_len == 0)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::got wrong data_len[%d]", data_len);
        kfree(tx_buf);
        return OAL_FAIL;
    }

    wifi_printf("***hwifi_hcc_custom_ini_data_buf:%d ***********\r\n", data_len);

    hcc_transfer_param.service_type = HCC_ACTION_TYPE_CUSTOMIZE;
    hcc_transfer_param.sub_type = syn_id;
    hcc_transfer_param.queue_id = CTRL_QUEUE;
    hcc_transfer_param.fc_flag = 0;
    hcc_transfer_param.user_param = NULL;

    ret = (osal_u32)hcc_tx_data(HCC_CHANNEL_AP, tx_buf, WLAN_LARGE_NETBUF_SIZE, &hcc_transfer_param);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf fail ret[%d] buf[%p]", ret, (uintptr_t)data_buf);
        kfree(tx_buf);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 协议栈初始化前定制化配置入口
*****************************************************************************/
osal_s32 hwifi_hcc_customize_h2d_data_cfg(void)
{
    osal_s32                              ret;

#ifndef _PRE_WLAN_NO_SUPPORT_INI
    /* wifi上电时重读定制化配置 */
    ret = hwifi_custom_host_read_cfg_init();
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg data ret[%d]", ret);
    }
#endif

    ret = hwifi_hcc_custom_ini_data_buf(CUSTOM_CFGID_PRIV_INI_ID);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg priv ini data fail, ret[%d]", ret);
        return OAL_FAIL;
    }
    ret = hwifi_hcc_custom_ini_data_buf(CUSTOM_CFGID_INI_ID);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg ini data fail, ret[%d]", ret);
        return OAL_FAIL;
    }

    return INI_SUCC;
}

#ifdef _PRE_WLAN_NO_SUPPORT_INI
osal_char *hwifi_get_country_code_etc(void)
{
    return g_ac_country_code_etc;
}

/* 根据大区index设置当前大区功率tag */
osal_void hwifi_set_nvram_tag_by_region_index(osal_s8 region_idx)
{
    osal_u8 index;
    // 先设置为默认, 找到则刷新
    g_nvram_tag = INI_MODU_WIFI;
    for (index = 0; index < sizeof(g_region_table) / sizeof(wlan_region_table); index++) {
        if (g_region_table[index].region_idx == region_idx) {
            g_nvram_tag = g_region_table[index].region_tag;
            break;
        }
    }
}

/*
    读取国家码所在大区index, 查询失败, 返回-1
*/
#define REGION_BUF_LEN 256
osal_s8 hwifi_get_region(const osal_s8 *country_code)
{
    osal_u8 index;
    osal_s8 region_buf[REGION_BUF_LEN];

    (osal_void)memset_s(region_buf, REGION_BUF_LEN, 0, REGION_BUF_LEN);
    /* 在配置文件中依次查询四个大区的内容 */
    for (index = 0; index < sizeof(g_region_table) / sizeof(wlan_region_table); index++) {
        if (strstr(g_region_table[index].contry_code, country_code) != OSAL_NULL) {
            wifi_printf("hwifi_get_region find %s in %s\r\n", country_code, g_region_table[index].region_str);
            return g_region_table[index].region_idx;
        }
    }
    wifi_printf("hwifi_get_region %s: region not found\r\n", country_code);
    return INI_FAILED;
}
#else
/*****************************************************************************
 功能描述  : 初始化私有定制全局变量数组
*****************************************************************************/
OAL_STATIC osal_s32 hwifi_config_init_private_custom(void)
{
    osal_s32               cfg_id;
    osal_s32               ret = INI_FAILED;

    for (cfg_id = 0; cfg_id < WLAN_CFG_PRIV_BUTT; cfg_id++) {
        /* 获取 private 的配置值 */
        ret = get_cust_conf_int32_etc(INI_MODU_WIFI,
            g_wifi_config_priv[cfg_id].name, &(g_al_priv_cust_params[cfg_id].val));
        if (ret == INI_FAILED) {
            g_al_priv_cust_params[cfg_id].value_state = OAL_FALSE;
            continue;
        }

        g_al_priv_cust_params[cfg_id].value_state = OAL_TRUE;
    }

    oam_warning_log0(0, OAM_SF_CFG, "hwifi_config_init_private_custom read from ini success!");

    return INI_SUCC;
}

OAL_STATIC wlan_cfg_cmd *hwifi_get_rf_fe_custom_common_type_config_ini(osal_u8 rf_custom_type, osal_u16 *cmd_len)
{
    wlan_cfg_cmd *config_ini = OAL_PTR_NULL;

    switch (rf_custom_type) {
        case WLAN_CFG_INI_RF_FE_TYPE_PARAMS:
            config_ini = g_rf_fe_param_config_ini;
            *cmd_len = sizeof(g_rf_fe_param_config_ini) / sizeof(wlan_cfg_cmd);
            break;
        case WLAN_CFG_INI_RF_FE_TYPE_POWER:
            config_ini = g_rf_fe_power_config_ini;
            *cmd_len = sizeof(g_rf_fe_power_config_ini) / sizeof(wlan_cfg_cmd);
            break;
        case WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI:
            config_ini = g_rf_fe_power_cali_config_ini;
            *cmd_len = sizeof(g_rf_fe_power_cali_config_ini) / sizeof(wlan_cfg_cmd);
            break;
        case WLAN_CFG_INI_RF_FE_TYPE_RSSI:
            config_ini = g_rf_fe_rssi_config_ini;
            *cmd_len = sizeof(g_rf_fe_rssi_config_ini) / sizeof(wlan_cfg_cmd);
            break;
        default:
            break;
    }
    return config_ini;
}
// tag_index:INI_MODU_WIFI rf_custom_type:WLAN_CFG_INI_RF_FE_TYPE_PARAMS param_byte:INI_PARAM_BYTE_ONE
OAL_STATIC osal_s32 hwifi_config_rf_fe_custom_common(osal_s32 tag_index, osal_u8 rf_custom_type, osal_u8 param_byte,
    osal_u8 *output, osal_u16 output_len)
{
    char ini_str[INI_READ_VALUE_LEN] = { 0 };
    wlan_cfg_cmd *config_ini = OAL_PTR_NULL;
    osal_u16 cfg_id = 0;
    osal_s32 ret = INI_SUCC;
    osal_u8 param_list[INI_READ_VALUE_COUNT] = {0};
    osal_u8 param_len = 0;
    osal_u8 cur_mem_len = 0;
    osal_u16 cfg_cmd_count = 0;

    if (output == OAL_PTR_NULL || output_len == 0) {
        oam_error_log1(0, OAM_SF_CUSTOM, "rf_fe_custom_common output param wrong, len:%d\n", output_len);
        return INI_FAILED;
    }
    config_ini = hwifi_get_rf_fe_custom_common_type_config_ini(rf_custom_type, &cfg_cmd_count);
    if (config_ini == OAL_PTR_NULL || cfg_cmd_count == 0) {
        oam_error_log1(0, OAM_SF_CUSTOM, "rf_fe_custom_common config ini wrong, len:%d\n", cfg_cmd_count);
        return INI_FAILED;
    }

    // 如果定制化中没有对应的参数定义 则直接跳过
    for (; cfg_id < cfg_cmd_count; cfg_id++) {
        ret = get_cust_conf_string_etc(tag_index, config_ini[cfg_id].name, ini_str, INI_READ_VALUE_LEN);
        // 没有该定制项的参数就跳过
        if (ret != INI_SUCC) {
            oam_warning_log2(0, OAM_SF_CUSTOM, "rf_fe_custom_common get_cust_conf_string_etc:%d tag:%d!!\r\n", ret,
                tag_index);
            continue;
        }
        ret = ini_get_cust_item_list(ini_str, param_list, param_byte, &param_len, INI_READ_VALUE_COUNT);
        // 定制化项解析参数异常
        if (ret != INI_SUCC) {
            oam_error_log2(0, OAM_SF_CUSTOM, "rf_fe_custom_common tag:%d ini_get_cust_item_list:%d!!\r\n", tag_index,
                ret);
            break;
        }
        ret = memcpy_s(output + cur_mem_len, output_len - cur_mem_len, param_list, param_len * param_byte);
        // 定制化项参数拷贝异常 内存不足等情况
        if (ret != INI_SUCC) {
            oam_error_log2(0, OAM_SF_CUSTOM, "rf_fe_custom_common tag:%d memcpy_s ret:%d!!\r\n", tag_index, ret);
            break;
        }
        // 继续下一个参数字符串解析 相关参数恢复默认值 并判断内存是否正常
        cur_mem_len += param_len * param_byte;
        param_len = 0;
        (void)memset_s(param_list, INI_READ_VALUE_COUNT, 0, INI_READ_VALUE_COUNT);
        if (output_len <= cur_mem_len) {
            break;
        }
    }
    oam_info_log4(0, OAM_SF_CUSTOM, "rf_fe_custom_common cfg:%d cur_mem_len:%d tag:%d type:%d\n", cfg_id, cur_mem_len,
        tag_index, rf_custom_type);
    return ret;
}

OAL_STATIC osal_u32 hwifi_config_rf_fe_rx_insert_loss_custom(osal_void)
{
    osal_u8 *rf_rx_loss_addr = (osal_u8 *)&g_cust_rf_fe_params.rf_rx_loss;
    osal_u16 max_mem_len = sizeof(g_cust_rf_fe_params.rf_rx_loss);
    osal_s32 ret;

    ret = hwifi_config_rf_fe_custom_common(INI_MODU_WIFI, WLAN_CFG_INI_RF_FE_TYPE_PARAMS,
        INI_PARAM_BYTE_ONE, rf_rx_loss_addr, max_mem_len);
    if (ret == INI_SUCC) {
        g_cust_rf_fe_params.ini_ret |= (1 << WLAN_CFG_INI_RF_FE_TYPE_PARAMS);
    }

    return INI_SUCC;
}

OAL_STATIC osal_u32 hwifi_config_rf_fe_power_custom(osal_void)
{
    osal_u8 *rf_power = (osal_u8 *)&g_cust_rf_fe_params.rf_power;
    osal_u8 max_mem_len = sizeof(g_cust_rf_fe_params.rf_power);
    osal_s32 tag_index = g_nvram_tag; // 按照国家码选择对应的tag index 如INI_MODU_FCC_NVRAM
    osal_s32 ret;

    ret = hwifi_config_rf_fe_custom_common(tag_index, WLAN_CFG_INI_RF_FE_TYPE_POWER,
        INI_PARAM_BYTE_ONE, rf_power, max_mem_len);
    if (ret == INI_SUCC) {
        g_cust_rf_fe_params.ini_ret |= (1 << WLAN_CFG_INI_RF_FE_TYPE_POWER);
    }
    return INI_SUCC;
}

OAL_STATIC osal_u32 hwifi_config_rf_fe_power_cali_custom(osal_void)
{
    osal_u8 *rf_power_cali = (osal_u8 *)&g_cust_rf_fe_params.rf_power_cali;
    osal_u8 max_mem_len = sizeof(g_cust_rf_fe_params.rf_power_cali);
    osal_s32 ret;

    ret = hwifi_config_rf_fe_custom_common(INI_MODU_WIFI, WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI,
        INI_PARAM_BYTE_TWO, rf_power_cali, max_mem_len);
    if (ret == INI_SUCC) {
        g_cust_rf_fe_params.ini_ret |= (1 << WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI);
    }
    return INI_SUCC;
}

OAL_STATIC osal_u32 hwifi_config_rf_fe_rssi_custom(osal_void)
{
    osal_u8 *rssi = (osal_u8 *)&g_cust_rf_fe_params.rf_rssi;
    osal_u8 max_mem_len = sizeof(g_cust_rf_fe_params.rf_rssi);
    osal_s32 ret;

    ret = hwifi_config_rf_fe_custom_common(INI_MODU_WIFI, WLAN_CFG_INI_RF_FE_TYPE_RSSI,
        INI_PARAM_BYTE_ONE, rssi, max_mem_len);
    if (ret == INI_SUCC) {
        g_cust_rf_fe_params.ini_ret |= (1 << WLAN_CFG_INI_RF_FE_TYPE_RSSI);
    }
    return INI_SUCC;
}

OAL_STATIC osal_s32 hwifi_config_rf_fe_custom(osal_void)
{
    // 状态更新 除功率外都置0 解析后成功的刷1
    g_cust_rf_fe_params.ini_ret &= (1 << WLAN_CFG_INI_RF_FE_TYPE_POWER);
    hwifi_config_rf_fe_rx_insert_loss_custom();
    hwifi_config_rf_fe_power_cali_custom();
    hwifi_config_rf_fe_rssi_custom();
    return INI_SUCC;
}
// 单独处理功率部分 国家码变更时刷新 其他ini配置项不需要刷新
OAL_STATIC osal_s32 hwifi_config_rf_fe_custom_tx_power(osal_void)
{
    // 状态更新 将功率置0 解析后成功刷1
    g_cust_rf_fe_params.ini_ret &= (~(1 << WLAN_CFG_INI_RF_FE_TYPE_POWER));
    hwifi_config_rf_fe_power_custom();
    return INI_SUCC;
}

/*****************************************************************************
 功能描述  : 给定制化参数全局数组 g_al_host_init_params_etc 附初值
             ini文件读取失败时用初值
*****************************************************************************/
OAL_STATIC osal_void host_params_init_first(osal_void)
{
    /* 性能 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_USED_MEM_FOR_START]                = 45; /* used mem for start set to 45 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_USED_MEM_FOR_STOP]                 = 25; /* used mem for stop set to 25 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT]               = 8; /* used bus_d2h_sched_count to 8 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT]              = 12; /* used bus_h2d_sched_count to 12 */

    g_al_host_init_params_etc[WLAN_CFG_INIT_RX_RESTORE_THRES] = 0;

    /* 2G RF前端 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1]    = 0xF4F4;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2]    = 0xF4F4;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3]    = 0xF4F4;
    /* 5G RF前端 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6]    = 0xF8F8;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7]    = 0xF8F8;

    /* 用于定制化计算PWR RF值的偏差 */
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4]    = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4]    = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4]    = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4]    = 0;

    g_al_host_init_params_etc[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G]         = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G]         = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G]         = 0;
    g_al_host_init_params_etc[WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G]         = 0;

    g_al_host_init_params_etc[WLAN_CFG_INIT_JUNCTION_TEMP_DIFF] = 0;
}

/* ****************************************************************************
 功能描述  : netdev open 调用的定制化总入口
             读取ini文件，更新 g_al_host_init_params_etc 全局数组
**************************************************************************** */
osal_s32 hwifi_config_init_etc(osal_s32 cus_tag)
{
    osal_s32               cfg_id;
    osal_s32               ret = INI_FAILED;
    osal_s32               ori_val;
    wlan_cfg_cmd       *pgast_wifi_config;
    osal_s32              *pgal_params;
    osal_s32               cfg_value = 0;
    osal_s32               wlan_cfg_butt;

    switch (cus_tag) {
        case CUS_TAG_INI:
            host_params_init_first();
            pgast_wifi_config = g_wifi_config_cmds;
            pgal_params = g_al_host_init_params_etc;
            wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
            break;
        case CUS_TAG_PRIV_INI:
            return hwifi_config_init_private_custom();
        case CUS_TAG_RF_FE:
            return hwifi_config_rf_fe_custom();
        case CUS_TAG_RF_FE_TX_POWER:
            return hwifi_config_rf_fe_custom_tx_power();
        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for (cfg_id = 0; cfg_id < wlan_cfg_butt; cfg_id++) {
        /* 获取ini的配置值 */
        ret = get_cust_conf_int32_etc(INI_MODU_WIFI, pgast_wifi_config[cfg_id].name, &cfg_value);
        if (ret == INI_FAILED) {
            oam_warning_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_init_etc read ini file cfg_id[%d]tag[%d] not exist!", cfg_id, cus_tag);
            continue;
        }
        ori_val = pgal_params[pgast_wifi_config[cfg_id].case_entry];
        pgal_params[pgast_wifi_config[cfg_id].case_entry] = cfg_value;
    }

    return INI_SUCC;
}

/*****************************************************************************
 功能描述  : 包括读取nvram中的dpint和校准系数值
*****************************************************************************/
osal_s32 hwifi_custom_host_read_dyn_cali_nvram(osal_void)
{
    return INI_FILE_TIMESPEC_UNRECONFIG;
}

/*****************************************************************************
 功能描述  : 首次读取定制化配置文件总入口
*****************************************************************************/
osal_s32 hwifi_custom_host_read_cfg_init(void)
{
    osal_s32      nv_read_ret;
    osal_s32      ini_read_ret;
    static osal_s32 ini_read_check = 0;
    /* 先获取私有定制化项 */
    hwifi_config_init_etc(CUS_TAG_PRIV_INI);
    /* 读取nvram参数是否修改 */
    nv_read_ret  = hwifi_custom_host_read_dyn_cali_nvram();
    /* 检查定制化文件中的产线配置是否修改 */
    ini_read_ret =  ini_file_check_conf_update();
    if (ini_read_ret == INI_FILE_TIMESPEC_UNRECONFIG && ini_read_check == 1) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init file is not updated");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ini_read_check = 1;
    hwifi_config_init_etc(CUS_TAG_INI);
    /* 射频前端定制化项解析 */
    hwifi_config_init_etc(CUS_TAG_RF_FE);
    /* 射频前端定制化项解析 功率部分 */
    hwifi_config_init_etc(CUS_TAG_RF_FE_TX_POWER);
    /* 启动完成后，输出打印 */
    wifi_printf("hwifi_custom_host_read_cfg_init finish!\r\n");

    return OAL_SUCC;
}

osal_char *hwifi_get_country_code_etc(void)
{
    osal_s32 ret;
    if (g_ac_country_code_etc[0] != '0' && g_ac_country_code_etc[1] != '0') {
        wifi_printf("hwifi_get_country_code_etc already set country:%s\r\n", g_ac_country_code_etc);
        return g_ac_country_code_etc;
    }
    // INI_MODU_WIFI -> INI_MODU_FCC_NVRAM
    ret = get_cust_conf_string_etc(INI_MODU_WIFI, STR_COUNTRY_CODE, g_ac_country_code_etc, COUNTRY_CODE_LEN);
    if (ret == INI_FAILED) {
        wifi_printf("hwifi_get_country_code_etc failed: code:%s\r\n", g_ac_country_code_etc);
        return OSAL_NULL;
    }
    g_ac_country_code_etc[COUNTRY_CODE_LEN - 1] = '\0'; /* 最后1个赋结束符 */
    wifi_printf("hwifi_get_country_code_etc suc: code:%s\r\n", g_ac_country_code_etc);
    return g_ac_country_code_etc;
}

/* 根据大区index设置当前大区功率tag */
osal_void hwifi_set_nvram_tag_by_region_index(osal_s8 region_idx)
{
    osal_u8 index;
    osal_s32 value;
    // 先设置为默认, 找到则刷新
    g_nvram_tag = INI_MODU_WIFI;
    for (index = 0; index < sizeof(g_region_table) / sizeof(wlan_region_table); index++) {
        if (g_region_table[index].region_idx == region_idx) {
            g_nvram_tag = g_region_table[index].region_tag;
            break;
        }
    }
    // 设置后尝试读取一项配置, 如果读取失败, 则认为此TAG无效, 重置为默认tag
    if (get_cust_conf_int32_etc(g_nvram_tag, g_rf_fe_power_config_ini[0].name, &value) == INI_FAILED) {
        g_nvram_tag = INI_MODU_WIFI;
        wifi_printf("hwifi_set_nvram_tag_by_region_index tag not exist:%x, set default\r\n", g_nvram_tag);
    }
}

/*****************************************************************************
 功能描述      : 从ini文件读取对外交互操作日志目录
****************************************************************************/
osal_void hwifi_get_external_record_file_name(osal_void)
{
    osal_s32 ret;
    osal_s32 size;

    ret = get_cust_conf_string_etc(INI_MODU_WIFI, WAL_EXTERMAL_RECORD_FILE_PATH, g_external_record_file_path,
        CUS_PARAMS_LEN_MAX - 1);
    if (ret != INI_SUCC) {
        oam_warning_log0(0, OAM_SF_DFT, "{hwifi_get_external_record_file_name:find file_path fail.}");
        return;
    }
    ret = get_cust_conf_int32_etc(INI_MODU_WIFI, WAL_EXTERMAL_RECORD_FILE_SIZE, &size);
    if (ret == INI_FAILED || size < 0) {
        oam_warning_log0(0, OAM_SF_DFT, "{hwifi_get_external_record_file_name:find file_size fail or invalid.}");
        return;
    }

    g_external_record_file_size = (osal_u32)size;
}

/*
    读取国家码所在大区index, 查询失败, 返回-1
*/
#define REGION_BUF_LEN 256
osal_s8 hwifi_get_region(const osal_s8 *country_code)
{
    osal_u8 index;
    osal_u32 ret;
    osal_s8 region_buf[REGION_BUF_LEN];
    (osal_void)memset_s(region_buf, REGION_BUF_LEN, 0, REGION_BUF_LEN);
    /* 在配置文件中依次查询四个大区的内容 */
    for (index = 0; index < sizeof(g_region_table) / sizeof(wlan_region_table); index++) {
        ret = get_cust_conf_string_etc(INI_MODU_WIFI, g_region_table[index].region_str, region_buf, REGION_BUF_LEN);
        if (ret == INI_FAILED) {
            wifi_printf("hwifi_get_region load region failed: region:%s\r\n", g_region_table[index].region_str);
            continue;
        }
        if (strstr(region_buf, country_code) != OSAL_NULL) {
            wifi_printf("hwifi_get_region find %s in %s\r\n", country_code, g_region_table[index].region_str);
            return g_region_table[index].region_idx;
        }
    }
    wifi_printf("hwifi_get_region %s: region not found\r\n", country_code);
    return INI_FAILED;
}

#endif

wlan_cust_rf_fe_params *hwifi_get_rf_fe_custom_ini(osal_void)
{
    return &g_cust_rf_fe_params;
}

/*****************************************************************************
 功能描述  : 从nvram中获取mac地址
             如果获取失败，则随机一个mac地址
*****************************************************************************/
osal_s32 hwifi_get_mac_addr_etc(osal_u8 *buf)
{
    // byshichongfu set random mac addr
    eth_random_addr(buf);
    buf[1] = 0x11; /* mac_addr[1]固定为0x11 */
    buf[2] = 0x03; /* mac_addr[2]固定为0x03 */

    return INI_SUCC;
}

osal_s32 hwifi_get_init_value_etc(osal_s32 cus_tag, osal_s32 cfg_id)
{
    osal_s32              *pgal_params = OAL_PTR_NULL;
    osal_s32               wlan_cfg_butt;

    if (cus_tag == CUS_TAG_INI) {
        pgal_params = &g_al_host_init_params_etc[0];
        wlan_cfg_butt = WLAN_CFG_INIT_BUTT;
    } else {
        oam_error_log1(0, OAM_SF_ANY, "hwifi_get_init_value_etc tag number[0x%2x] not correct!", cus_tag);
        return INI_FAILED;
    }

    if (cfg_id < 0 || cfg_id >= wlan_cfg_butt) {
        oam_error_log2(0, OAM_SF_ANY,
            "hwifi_get_init_value_etc cfg id[%d] out of range, max cfg id is:%d", cfg_id, (wlan_cfg_butt - 1));
        return INI_FAILED;
    }

    return pgal_params[cfg_id];
}

osal_s32 hwifi_get_init_priv_value(osal_s32 cfg_id, osal_s32 *priv_value)
{
    if (cfg_id < 0 || cfg_id >= WLAN_CFG_PRIV_BUTT) {
        oam_error_log2(0, OAM_SF_ANY,
            "hwifi_get_init_priv_value cfg id[%d] out of range, max[%d]", cfg_id, WLAN_CFG_PRIV_BUTT - 1);
        return OAL_FAIL;
    }

    if (g_al_priv_cust_params[cfg_id].value_state == OAL_FALSE) {
        return OAL_FAIL;
    }

    *priv_value = g_al_priv_cust_params[cfg_id].val;

    return OAL_SUCC;
}

void hwifi_set_country_code_etc(osal_char *country_code, const osal_u32 len)
{
    if (country_code == OAL_PTR_NULL || len != COUNTRY_CODE_LEN) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_country_code_etc ptr null or illegal len!");
        return;
    }

    (osal_void)memcpy_s(g_ac_country_code_etc, COUNTRY_CODE_LEN, country_code, COUNTRY_CODE_LEN);
    g_ac_country_code_etc[COUNTRY_CODE_LEN - 1] = '\0'; /* 最后1个赋结束符 */
}

/*****************************************************************************
 功能描述      : 是否开启随机mac扫描
*****************************************************************************/
osal_void hwifi_scan_set_random_mac_addr_scan(osal_u8 random_scan)
{
    g_wlan_customize_etc.random_mac_addr_scan = random_scan;
}

void *hwifi_wlan_customize_etc(void)
{
    return &g_wlan_customize_etc;
}

osal_void hwifi_set_udp_hcc_fc_drop(osal_u8 active)
{
    g_udp_hcc_fc_drop = active;
}

osal_u8 hwifi_get_udp_hcc_fc_drop(osal_void)
{
    return g_udp_hcc_fc_drop;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存ap数量的阈值
*****************************************************************************/
osal_u16 hwifi_get_hmac_max_ap_num_etc(void)
{
    return g_hmac_max_ap_num;
}

/*****************************************************************************
 功能描述      : 初始化g_hmac_ap_list_info
*****************************************************************************/
osal_void hwifi_set_hmac_max_ap_num_etc(osal_u16 max_ap_num)
{
    g_hmac_max_ap_num = max_ap_num;
}
/*****************************************************************************
 功能描述      : 获取hmac侧保存的自愈开关
****************************************************************************/
osal_u32 hwifi_get_self_healing_enable_etc(osal_void)
{
    return g_self_healing_enable;
}

/*****************************************************************************
 功能描述      : 从ini文件读取自愈开关，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_self_healing_enable_etc(osal_u32 self_healing_enable)
{
    g_self_healing_enable = self_healing_enable;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存的自愈检测周期
****************************************************************************/
osal_u32 hwifi_get_self_healing_period_etc(osal_void)
{
    return g_self_healing_period;
}

/*****************************************************************************
 功能描述      : 从ini文件读取自愈检测周期，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_self_healing_period_etc(osal_u32 self_healing_period)
{
    g_self_healing_period = self_healing_period;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存的自愈上报衰减频率
****************************************************************************/
osal_u32 hwifi_get_self_healing_cnt_etc(osal_void)
{
    return g_self_healing_cnt;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存的自愈上报衰减周期
****************************************************************************/
osal_void hwifi_set_self_healing_cnt_etc(osal_u32 self_healing_cnt)
{
    g_self_healing_cnt = self_healing_cnt;
}

osal_void hwifi_set_amsdu_num(osal_u8 amsdu_num)
{
    g_amsdu_max_num = amsdu_num;
}

osal_u8 hwifi_get_amsdu_num(osal_void)
{
    return g_amsdu_max_num;
}

osal_void hwifi_set_amsdu_tx_active(osal_u8 active)
{
    g_amsdu_tx_active = active;
}

osal_u8 hwifi_get_amsdu_tx_active(osal_void)
{
    return g_amsdu_tx_active;
}

osal_void hwifi_set_ampdu_amsdu_tx_active(osal_u8 active)
{
    g_ampdu_amsdu_tx_active = active;
}

osal_u8 hwifi_get_ampdu_amsdu_tx_active(osal_void)
{
    return g_ampdu_amsdu_tx_active;
}

osal_void hwifi_set_ampdu_tx_max_num(osal_u8 num)
{
    g_ampdu_tx_max_num = num;
}

osal_u8 hwifi_get_ampdu_tx_max_num(osal_void)
{
    return g_ampdu_tx_max_num;
}

osal_void hwifi_set_ampdu_rx_max_num(osal_u8 num)
{
    g_ampdu_rx_max_num = num;
}

osal_u8 hwifi_get_ampdu_rx_max_num(osal_void)
{
    return g_ampdu_rx_max_num;
}

osal_void hwifi_set_ampdu_tx_baw_size(osal_u8 num)
{
    g_ampdu_tx_baw_size = num;
}

osal_u8 hwifi_get_ampdu_tx_baw_size(osal_void)
{
    return g_ampdu_tx_baw_size;
}

/*****************************************************************************
 功能描述  : 获取数采开关
****************************************************************************/
osal_bool hwifi_get_data_sample(osal_void)
{
    return g_data_sample;
}

/*****************************************************************************
 功能描述  : 从ini读取数采配置后,设置数采开关
****************************************************************************/
osal_void hwifi_set_data_sample(osal_bool data_sample)
{
    g_data_sample = data_sample;
}

/*****************************************************************************
 功能描述      : 获取hmac侧保存的apf使能开关
****************************************************************************/
osal_bool hwifi_get_apf_enable(osal_void)
{
    return g_apf_enable;
}

/*****************************************************************************
 功能描述      : 从ini文件读取APF使能开关，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_apf_enable(osal_bool apf_enable)
{
    g_apf_enable = apf_enable;
}

/*****************************************************************************
 功能描述      : 获取强制配置为11AX模式的开关
*****************************************************************************/
osal_u8 hwifi_get_protocol_enable(void)
{
    return g_protocol_enable;
}

/*****************************************************************************
 功能描述      : 从ini文件读取配置11AX模式的开关，只在hmac初始化时调用
*****************************************************************************/
osal_void hwifi_set_protocol_enable(osal_u8 protocol_enable)
{
    g_protocol_enable = protocol_enable;
}

/*****************************************************************************
 功能描述  : 获取ini中设置的wow 唤醒源
****************************************************************************/
osal_u32 hwifi_get_wow_event(osal_void)
{
    return g_wow_event;
}

/*****************************************************************************
 功能描述      : 从ini文件读取wow 唤醒源，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_wow_event(osal_u32 wow_event)
{
    g_wow_event = wow_event;
}

/*****************************************************************************
 功能描述      : 从ini文件读取wow开关，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_wow_enable(osal_u8 wow_enable)
{
    g_wow_enable = wow_enable;
}

/*****************************************************************************
 功能描述  : 获取ini中设置的wow开关
****************************************************************************/
osal_u8 hwifi_get_wow_enable(osal_void)
{
    return g_wow_enable;
}

/*****************************************************************************
 功能描述      : 从ini文件读取sniffer文件大小最大值，并设置，只在wal初始化时调用
****************************************************************************/
osal_void hwifi_set_sniffer_file_len_max(osal_u8 sniffer_file_len_max)
{
    g_sniffer_file_len_max = sniffer_file_len_max;
}

/*****************************************************************************
 功能描述  : 获取ini中设置的sniffer文件大小最大值
****************************************************************************/
osal_u8 hwifi_get_sniffer_file_len_max(osal_void)
{
    return g_sniffer_file_len_max;
}

/*****************************************************************************
 功能描述  : 获取ini中设置的smooth_phase_en
****************************************************************************/
osal_u32 hwifi_get_smooth_phase_en(osal_void)
{
    return g_smooth_phase_en;
}

/*****************************************************************************
 功能描述      : 从ini文件读取smooth_phase_en，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_smooth_phase_en(osal_bool smooth_phase_en)
{
    g_smooth_phase_en = smooth_phase_en;
}

/*****************************************************************************
 功能描述  : 获取ini中设置的stbc 唤醒源
****************************************************************************/
osal_u32 hwifi_get_rx_stbc(osal_void)
{
    return g_stbc_enable;
}

/*****************************************************************************
 功能描述      : 从ini文件读取stbc 唤醒源，并设置，只在hmac初始化时调用
****************************************************************************/
osal_void hwifi_set_rx_stbc(osal_u32 stbc_enable)
{
    g_stbc_enable = stbc_enable;
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/*****************************************************************************
 功能描述  : 获取ini中设置的外部共存使能
****************************************************************************/
osal_u32 hwifi_get_ext_coex_en(osal_void)
{
    return 0;
}
#endif

osal_u16 hwifi_get_low_current_boot_mode_cali_data_mask(osal_u16 cali_data_mask, osal_u8 first_cali_done_flag)
{
    osal_u16 mask_bit2 = 0x4;  // cali data mask:cali

    if (first_cali_done_flag) {
        return (cali_data_mask & (~mask_bit2));
    } else {
        return cali_data_mask;
    }
}

osal_void hwifi_custom_adapt_device_ini_perf_param(osal_u8 *data, osal_u32 *data_len)
{
    osal_u32 ret;
    hmac_to_dmac_cfg_custom_data_stru sync_msg = {0};
    bus_cus_config_stru cus_config = {0};

    if (data == OSAL_NULL) {
        return;
    }

    sync_msg.syn_id = CUSTOM_CFGID_INI_PERF_ID;

    cus_config.bus_d2h_sched_count = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT);
    cus_config.bus_h2d_sched_count = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT);
 
    if (cus_config.bus_h2d_sched_count < 1 || cus_config.bus_h2d_sched_count > HCC_SDIO_HOST2DEV_SCATT_MAX) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_custom_adapt_device_ini_perf_param::bus_h2d_sched_count[%d] out of range(0,%d]}",
            cus_config.bus_h2d_sched_count, HCC_SDIO_HOST2DEV_SCATT_MAX);
    } else {
        hcc_set_tx_sched_count(HCC_CHANNEL_AP, cus_config.bus_h2d_sched_count);
    }

    if (cus_config.bus_d2h_sched_count < 1 || cus_config.bus_d2h_sched_count > HCC_SDIO_DEV2HOST_SCATT_MAX) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_custom_adapt_device_ini_perf_param::sdio_assem_d2h[%d] out of range(0,%d]}",
            cus_config.bus_d2h_sched_count, HCC_SDIO_DEV2HOST_SCATT_MAX);
    }

    sync_msg.len = (osal_u32)sizeof(cus_config);

    ret = memcpy_s(data, CUSTOM_MSG_DATA_HDR_LEN, &sync_msg, CUSTOM_MSG_DATA_HDR_LEN);
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        return;
    }

    ret = memcpy_s(data + CUSTOM_MSG_DATA_HDR_LEN, sizeof(cus_config), &cus_config, sizeof(cus_config));
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        return;
    }

    *data_len += ((osal_u32)sizeof(cus_config) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param::da_len[%d].}", *data_len);
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#endif // #if defined(_PRE_PRODUCT_ID_HOST)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
