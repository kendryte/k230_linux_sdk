/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac侧alg相关配置下发文件
 * Create: 2023-02-25
 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_alg_config.h"
#include "alg_main.h"
#include "hmac_alg_notify.h"
#include "hmac_al_tx_rx.h"
#ifdef _PRE_WLAN_FEATURE_RTS
#include "alg_rts.h"
#endif
#include "msg_alg_rom.h"
#include "hmac_ccpriv.h"
#include "wal_ccpriv_common.h"
#include "alg_autorate.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ALG_CONFIG_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  1. 宏定义
*****************************************************************************/
/* 根据alg_autorate_tx_rate_rank_stru定义的以下偏移量 */
#define ALG_CFG_AUTORATE_MCS_SHIFT          4
#define ALG_CFG_AUTORATE_PROTOCOL_BIT       0x7
#define ALG_CFG_AUTORATE_PROTOCOL_SHIFT     0
#define ALG_CFG_AUTORATE_FREQ_BW_BIT        0x38
#define ALG_CFG_AUTORATE_FREQ_BW_SHIFT      3
#define ALG_CFG_AUTORATE_PREAMBLE_BIT       0x40
#define ALG_CFG_AUTORATE_PREAMBLE_SHIFT     6
#define ALG_CFG_AUTORATE_FEC_CODING_BIT     0x80
#define ALG_CFG_AUTORATE_FEC_CODING_SHIFT   7
#define ALG_CFG_AUTORATE_GI_BIT             0x300
#define ALG_CFG_AUTORATE_GI_SHIFT           8
#define ALG_CFG_AUTORATE_HE_LTF_BIT         0xC00
#define ALG_CFG_AUTORATE_HE_LTF_SHIFT       10
#define ALG_CFG_AUTORATE_TX_CNT_BIT         0x7000
#define ALG_CFG_AUTORATE_TX_CNT_SHIFT       12
#define ALG_CFG_AUTORATE_HE_DCM_BIT         0x8000
#define ALG_CFG_AUTORATE_HE_DCM_SHIFT       15
#define ALG_CFG_AUTORATE_NSS_MCS_BIT        0x3F0000
#define ALG_CFG_AUTORATE_NSS_MCS_SHIFT      16
#define ALG_CFG_AUTORATE_RANK_BIT           0xC00000
#define ALG_CFG_AUTORATE_RANK_SHIFT         22
#define ALG_CFG_AUTORATE_FRAME_TYPE_BIT     0x3000000
#define ALG_CFG_AUTORATE_FRAME_TYPE_SHIFT   24
#define ALG_RTS_CFG_RATE_SHIFT              3

/*****************************************************************************
  2. 枚举定义
*****************************************************************************/
typedef enum {
    ALG_FRAME_TYPE_UCAST_DATA = 0,
    ALG_FRAME_TYPE_BCAST_DATA = 1,
    ALG_FRAME_TYPE_UCAST_MGMT = 2,
    ALG_FRAME_TYPE_BCAST_MGMT = 3,
} alg_ar_frame_type;

/*****************************************************************************
  3. 结构体定义
*****************************************************************************/
/* 算法参数配置结构体 */
typedef struct {
    osal_char                      *pc_name;        /* 配置命令字符串 */
    mac_alg_cfg_enum_uint16         alg_cfg;     /* 配置命令对应的枚举值 */
    osal_u8                         auc_resv[2];    /* 字节对齐 */
} wal_ioctl_alg_cfg_stru;

/* alg_cfg 字符串-数值 转换表 */
typedef struct {
    osal_char                      *pc_name;        /* 配置命令字符串 */
    osal_u32                        value;        /* 配置命令字符串对应的值 */
} alg_cfg_rts_param_tbl_stru;
typedef struct {
    osal_u32                        rank;
    osal_u32                        protocol;
    osal_u32                        bw;
    osal_u32                        preamble;
    osal_u32                        fec_coding;
    osal_u32                        gi;
    osal_u32                        he_ltf;
    osal_u32                        tx_cnt;
    osal_u32                        he_dcm;
    osal_u32                        nss;
    osal_u32                        nss_rate;
    osal_u32                        frame_type;
} alg_cfg_autorate_param_stru;

/* alg_cfg 字符串-数值 转换表 */
typedef struct {
    osal_char                      *pc_name;        /* 配置命令字符串 */
    osal_u32                        value;        /* 配置命令字符串对应的值 */
} alg_cfg_autorate_param_tbl_stru;

typedef struct {
    wlan_phy_protocol_enum_uint8   protocol_type;
    uint8_t                        table_size;
    uint8_t                        resv[2];         /* 空余填充2字节 */
    const void                     *table_addr;
} alg_cfg_autorate_table_info_stru;

typedef struct {
    osal_char                      *pc_name;        /* 配置命令字符串 */
    wlan_phy_protocol_enum_uint8    protocol;
    hal_channel_assemble_enum_uint8 bw;
    osal_u8 rev[2];
} alg_cfg_autorate_protocol_bw_param_tbl_stru;

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
/* 算法参数配置结构体 */
typedef struct {
    osal_char                         *name;        /* 配置命令字符串 */
    mac_alg_intrf_mode_cfg_enum_uint8  intrf_mode_cfg;     /* 配置命令对应的枚举值 */
    osal_u8                            auc_resv[3];    /* 字节对齐 */
} wal_ioctl_alg_intrf_mode_stru;
#endif

OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_alg_ar_cfg_11b_rate_tbl[] = {
    {"1M",       WLAN_PHY_RATE_1M},
    {"2M",       WLAN_PHY_RATE_2M},
    {"5.5M",     WLAN_PHY_RATE_5HALF_M},
    {"11M",      WLAN_PHY_RATE_11M},
};

OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_alg_ar_cfg_11g_rate_tbl[] = {
    {"6M",       WLAN_PHY_RATE_6M},
    {"9M",       WLAN_PHY_RATE_9M},
    {"12M",      WLAN_PHY_RATE_12M},
    {"18M",      WLAN_PHY_RATE_18M},
    {"24M",      WLAN_PHY_RATE_24M},
    {"36M",      WLAN_PHY_RATE_36M},
    {"48M",      WLAN_PHY_RATE_48M},
    {"54M",      WLAN_PHY_RATE_54M},
};

OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_alg_ar_cfg_11n_rate_tbl[] = {
    {"mcs0",       WLAN_HT_MCS0},
    {"mcs1",       WLAN_HT_MCS1},
    {"mcs2",       WLAN_HT_MCS2},
    {"mcs3",       WLAN_HT_MCS3},
    {"mcs4",       WLAN_HT_MCS4},
    {"mcs5",       WLAN_HT_MCS5},
    {"mcs6",       WLAN_HT_MCS6},
    {"mcs7",       WLAN_HT_MCS7},
};

#ifdef _PRE_WLAN_SUPPORT_VHT
OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_alg_ar_cfg_11ac_rate_tbl[] = {
    {"mcs0",       WLAN_VHT_MCS0},
    {"mcs1",       WLAN_VHT_MCS1},
    {"mcs2",       WLAN_VHT_MCS2},
    {"mcs3",       WLAN_VHT_MCS3},
    {"mcs4",       WLAN_VHT_MCS4},
    {"mcs5",       WLAN_VHT_MCS5},
    {"mcs6",       WLAN_VHT_MCS6},
    {"mcs7",       WLAN_VHT_MCS7},
    {"mcs8",       WLAN_VHT_MCS8},
};
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_alg_ar_cfg_11ax_rate_tbl[] = {
    {"mcs0",       WLAN_HE_MCS0},
    {"mcs1",       WLAN_HE_MCS1},
    {"mcs2",       WLAN_HE_MCS2},
    {"mcs3",       WLAN_HE_MCS3},
    {"mcs4",       WLAN_HE_MCS4},
    {"mcs5",       WLAN_HE_MCS5},
    {"mcs6",       WLAN_HE_MCS6},
    {"mcs7",       WLAN_HE_MCS7},
    {"mcs8",       WLAN_HE_MCS8},
    {"mcs9",       WLAN_HE_MCS9},
};
#endif

OAL_STATIC OAL_CONST alg_cfg_autorate_table_info_stru g_alg_ar_rate_index_table_info[] = {
    {WLAN_11B_PHY_PROTOCOL_MODE, osal_array_size(g_alg_ar_cfg_11b_rate_tbl), {0, 0}, g_alg_ar_cfg_11b_rate_tbl},
    {WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE, osal_array_size(g_alg_ar_cfg_11g_rate_tbl), {0, 0}, g_alg_ar_cfg_11g_rate_tbl},
    {WLAN_HT_PHY_PROTOCOL_MODE, osal_array_size(g_alg_ar_cfg_11n_rate_tbl), {0, 0}, g_alg_ar_cfg_11n_rate_tbl},
#ifdef _PRE_WLAN_SUPPORT_VHT
    {WLAN_VHT_PHY_PROTOCOL_MODE, osal_array_size(g_alg_ar_cfg_11ac_rate_tbl), {0, 0}, g_alg_ar_cfg_11ac_rate_tbl},
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    {WLAN_HE_SU_FORMAT, osal_array_size(g_alg_ar_cfg_11ax_rate_tbl), {0, 0}, g_alg_ar_cfg_11ax_rate_tbl},
    {WLAN_HE_EXT_SU_FORMAT, osal_array_size(g_alg_ar_cfg_11ax_rate_tbl), {0, 0}, g_alg_ar_cfg_11ax_rate_tbl}
#endif
};

OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_ast_alg_ar_cfg_11bg_gi_tbl[] = {
    {"long",       0},
};

OAL_STATIC OAL_CONST alg_cfg_autorate_param_tbl_stru g_ast_alg_ar_cfg_11n_gi_tbl[] = {
    {"long",       0},
    {"short",      1},
};

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
OAL_STATIC OAL_CONST wal_ioctl_alg_intrf_mode_stru g_alg_intrf_mode_cfg_map_etc[] = {
    {"11b_switch",              MAC_ALG_INTRF_MODE_11B_SWITCH, {0, 0, 0}},
    {"cca_switch",              MAC_ALG_INTRF_MODE_CCA_SWITCH, {0, 0, 0}},
    {"edca_switch",             MAC_ALG_INTRF_MODE_EDCA_SWITCH, {0, 0, 0}},
    {"11n_switch",              MAC_ALG_INTRF_MODE_11N_SWITCH, {0, 0, 0}},
    {"descend_11b_switch",      MAC_ALG_INTRF_MODE_NO_11B_SWITCH, {0, 0, 0}},
    {"long_range_intrf_switch", MAC_ALG_INTRF_MODE_LONG_RANGE_INTRF_SWITCH, {0, 0, 0}},

    {OSAL_NULL}
};
#endif

/* 发布debug标志按照产品设置 */
STATIC osal_u8 g_alg_debug_flag = ALG_DEBUG_FLAG;

STATIC OAL_CONST wal_ioctl_alg_cfg_stru g_ast_alg_cfg_debug_map_etc[] = {
    {"sch_cycle_ms",            MAC_ALG_CFG_SCHEDULE_SCH_CYCLE_MS, {0, 0}},
    {"sch_method",              MAC_ALG_CFG_SCHEDULE_SCH_METHOD, {0, 0}},
    {"sch_fix_mode",            MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE, {0, 0}},
    {"sch_txtime_method",       MAC_ALG_CFG_SCHEDULE_TX_TIME_METHOD, {0, 0}},
    {"sch_txtime_debug",        MAC_ALG_CFG_SCHEDULE_TX_TIME_DEBUG, {0, 0}},
    {"sch_stat_log",            MAC_ALG_CFG_SCHEDULE_STAT_LOG, {0, 0}},
    {"sch_stat_dmac_log",       MAC_ALG_CFG_SCHEDULE_DMAC_STAT_LOG, {0, 0}},

    {"rate_mode",               MAC_ALG_CFG_AUTORATE_RATE_MODE, {0, 0}},
    {"fix_rate",                MAC_ALG_CFG_AUTORATE_FIX_RATE, {0, 0}},
    {"autorate_debug",          MAC_ALG_CFG_AUTORATE_DEBUG_LOG, {0, 0}},
    {"autorate_auto_bw",        MAC_ALG_CFG_AUTORATE_AUTO_BW, {0, 0}},
    {"autorate_event_trig_descend_bw", MAC_ALG_CFG_AUTORATE_EVENT_TRIG_DESCEND_BW, {0, 0}},
    {"autorate_auto_protocol",  MAC_ALG_CFG_AUTORATE_AUTO_PROTOCOL, {0, 0}},
    {"autorate_switch",         MAC_ALG_CFG_AUTORATE_IS_ENABLE, {0, 0}},

    {"temp_pro_start",          MAC_ALG_CFG_TEMP_PROTECT_START, {0, 0}},
    {"temp_pro_get",            MAC_ALG_CFG_TEMP_PROTECT_GET, {0, 0}},
    {"temp_pro_safe_th",        MAC_ALG_CFG_TEMP_PROTECT_SAFE_TH_SET, {0, 0}},
    {"temp_pro_over_th",        MAC_ALG_CFG_TEMP_PROTECT_OVER_TH_SET, {0, 0}},
    {"temp_pro_paoff_th",       MAC_ALG_CFG_TEMP_PROTECT_PAOFF_TH_SET, {0, 0}},
    {"temp_pro_times",          MAC_ALG_CFG_TEMP_PROTECT_TIMES, {0, 0}},
    {"temp_pro_debug",          MAC_ALG_CFG_TEMP_PROTECT_DEBUG, {0, 0}},
    {"temp_pro_temp_set",       MAC_ALG_CFG_TEMP_PROTECT_TEMP_SET, {0, 0}},
    /* 开启或关闭聚合自适应算法: "vap0 alg_cfg aggr_enable [1|0]" */
    {"aggr_enable",             MAC_ALG_CFG_AGGR_ENABLE, {0, 0}},
    /* 设置聚合自适应算法模式: "vap0 alg_cfg aggr_mode [1：固定|0：动态]" */
    {"aggr_mode",               MAC_ALG_CFG_AGGR_MODE, {0, 0}},
    /* 设置聚合自适应算法固定档位: "vap0 alg_cfg aggr_mode [0-5]" */
    {"aggr_fixed_time_idx",     MAC_ALG_CFG_AGGR_FIXED_TIME_IDX, {0, 0}},
    /* 设置聚合自适应算法最大聚合个数: "vap0 alg_cfg aggr_max_aggr_num [1-16]" */
    {"aggr_max_aggr_num",       MAC_ALG_CFG_AGGR_MAX_AGGR_NUM, {0, 0}},
    /* 设置聚合自适应算法最小聚合档位: "vap0 alg_cfg aggr_min_time_idx [0-5]" */
    {"aggr_min_time_idx",       MAC_ALG_CFG_AGGR_MIN_TIME_IDX, {0, 0}},
    /* 开启或关闭聚合自适应算法维测日志: "vap0 alg_cfg aggr_dbg_log [1|0]" */
    {"aggr_dbg_log",            MAC_ALG_CFG_AGGR_DBG_LOG, {0, 0}},
    /* 开启或关闭聚合自适应算法维测日志: "vap0 alg_cfg aggr_serial_log [1|0]" */
    {"aggr_serial_log",         MAC_ALG_CFG_AGGR_SERIAL_LOG, {0, 0}},
    /* 使能或关闭VO聚合功能: "vap0 alg_cfg aggr_vo_en [1|0]" */
    {"aggr_vo_en",              MAC_ALG_CFG_AGGR_VO_EN, {0, 0}},
    /* 设置聚合自适应算法探测间隔报文个数: "vap0 alg_cfg aggr_probe_intvl_num [NUM]" */
    {"aggr_probe_intvl_num",    MAC_ALG_CFG_AGGR_PROBE_INTVL_NUM, {0, 0}},
    /* 设置聚合自适应在最优速率改变时不进行探测的个数: "vap0 alg_cfg aggr_non_probe_pck_num [NUM]" */
    {"aggr_non_probe_pck_num",  MAC_ALG_CFG_AGGR_NON_PROBE_PCK_NUM, {0, 0}},
    /* 设置聚合自适应的统计移位值: "vap0 alg_cfg aggr_stat_shift [NUM]" */
    {"aggr_stat_shift",         MAC_ALG_CFG_AGGR_STAT_SHIFT, {0, 0}},

    /* 弱干扰免疫中non-direct使能: "vap0 alg_cfg anti_inf_imm_en 0|1" */
    {"anti_inf_imm_en",         MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE, {0, 0}},
    /* 弱干扰免疫中dynamic unlock使能: "vap0 alg_cfg anti_inf_unlock_en 0|1" */
    {"anti_inf_unlock_en",      MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE, {0, 0}},
    /* 弱干扰免疫中rssi统计周期: "vap0 anti_inf_stat_time [time]" */
    {"anti_inf_stat_time",      MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE, {0, 0}},
    /* 弱干扰免疫中unlock关闭周期: "vap0 anti_inf_off_time [time]" */
    {"anti_inf_off_time",       MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE, {0, 0}},
    /* 弱干扰免疫中unlock关闭持续时间: "vap0 anti_inf_off_dur [time]" */
    {"anti_inf_off_dur",        MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME, {0, 0}},
    /* 抗干扰nav免疫使能: "vap0 alg_cfg anti_inf_nav_en 0|1" */
    {"anti_inf_nav_en",         MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE, {0, 0}},
    /* 弱干扰免疫goodput下降门限: "vap0 alg_cfg anti_inf_gd_th [num]" */
    {"anti_inf_gd_th",          MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH, {0, 0}},
    /* 弱干扰免疫探测保持最大周期数: "vap0 alg_cfg anti_inf_keep_max [num]" */
    {"anti_inf_keep_max",       MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM, {0, 0}},
    /* 弱干扰免疫探测保持最大周期数: "vap0 alg_cfg anti_inf_keep_min [num]" */
    {"anti_inf_keep_min",       MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM, {0, 0}},
    /* 弱干扰免疫是否使能tx time探测: "vap0 anti_inf_tx_pro_en 0|1" */
    {"anti_inf_per_pro_en",     MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN, {0, 0}},
    /* tx time下降门限: "vap0 alg_cfg anti_inf_txtime_th [val]" */
    {"anti_inf_txtime_th",      MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH, {0, 0}},
    /* per下降门限: "vap0 alg_cfg anti_inf_per_th [val]" */
    {"anti_inf_per_th",         MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH, {0, 0}},
    /* goodput抖动门限: "vap0 alg_cfg anti_inf_gd_jitter_th [val]" */
    {"anti_inf_gd_jitter_th",   MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH, {0, 0}},
    /* 弱干扰免疫debug的打印信息: "vap0 alg_cfg anti_inf_debug_mode 0|1|2" */
    {"anti_inf_debug_mode",     MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE, {0, 0}},
    /* 抗干扰nav免疫保护次数: "vap0 alg_cfg anti_inf_nav_enquiry 0|1" */
    {"anti_inf_nav_enquiry",         MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENQUIRY, {0, 0}},
    /* 设置干扰检测周期(ms):"vap0 alg_cfg intf_det_cycle [val]" */
    {"intf_det_cycle",          MAC_ALG_CFG_INTF_DET_CYCLE, {0, 0}},
    /* 设置干扰检测模式(同频开/邻频叠频开/都开):"vap0 alg_cfg intf_det_mode 0|1" */
    {"intf_det_mode",           MAC_ALG_CFG_INTF_DET_MODE, {0, 0}},
    /* 设置干扰检测debug模式(关/开HSO日志/开串口日志):"vap0 alg_cfg intf_det_debug 0|1|2" */
    {"intf_det_debug",          MAC_ALG_CFG_INTF_DET_DEBUG, {0, 0}},
    /* 设置干扰检测sta阈值(千分之x):"vap0 alg_cfg intf_det_cothr_sta [val]" */
    {"intf_det_cothr_sta",      MAC_ALG_CFG_INTF_DET_COCH_THR_STA, {0, 0}},
    /* 设置干扰检测sta无干扰阈值(千分之x):"vap0 alg_cfg intf_det_nointf_thr_sta [val]" */
    {"intf_det_nointf_thr_sta", MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA, {0, 0}},
    /* 设置干扰检测邻频干扰扫描周期:"vap0 alg_cfg intf_det_adjscan_cyc [val]" */
    {"intf_det_adjscan_cyc",    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC, {0, 0}},
    /* 设置干扰检测邻频叠频干扰繁忙度阈值(千分之x):"vap0 alg_cfg intf_det_adjratio_thr [val]" */
    {"intf_det_adjratio_thr",   MAC_ALG_CFG_INTF_DET_ADJRATIO_THR, {0, 0}},
    /* 设置干扰检测邻频叠频干扰sync error阈值(千分之x):"vap0 alg_cfg intf_det_sync_th [val]" */
    {"intf_det_sync_th",        MAC_ALG_CFG_INTF_DET_SYNC_THR, {0, 0}},
    /* 设置干扰检测零频叠频干扰平均rssi阈值(千分之x):"vap0 alg_cfg intf_det_ave_rssi [val]" */
    {"intf_det_ave_rssi",       MAC_ALG_CFG_INTF_DET_AVE_RSSI, {0, 0}},
    /* 设置干扰检测非邻/叠频干扰繁忙度阈值(千分之x):"vap0 alg_cfg intf_det_no_adjratio_th [val]" */
    {"intf_det_no_adjratio_th", MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH, {0, 0}},
    /* 设置干扰检测非邻/叠频干扰计数阈值:"vap0 alg_cfg intf_det_no_adjcyc_th [val]" */
    {"intf_det_no_adjcyc_th",   MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH, {0, 0}},
    /* 获取干扰类型:"vap0 alg_cfg intf_type_enquiry" */
    {"intf_type_enquiry",   MAC_ALG_CFG_INTF_DET_GET_INTF_TYPE, {0, 0}},
    /* 设置pk模式:"vap0 alg_cfg pk_mode_enable 0|1" */
    {"pk_mode_enable",   MAC_ALG_CFG_INTF_DET_SET_PK_MODE, {0, 0}},
    /* sta模式下edca优化使能模式: "vap0 alg_cfg edca_opt_en_sta 0|1" */
    {"edca_opt_en_sta",         MAC_ALG_CFG_EDCA_OPT_STA_EN, {0, 0}},
    /* sta模式下edca txop limit优化使能模式: "vap0 alg_cfg txop_limit_en_sta 0|1" */
    {"txop_limit_en_sta",       MAC_ALG_CFG_TXOP_LIMIT_STA_EN, {0, 0}},
    {"edca_opt_det_collosion_th",   MAC_ALG_CFG_EDCA_DET_COLLISION_TH, {0, 0}},
    {"edca_opt_neg_det_nonprobe_th",   MAC_ALG_CFG_EDCA_NEG_DET_NONPROBE_TH, {0, 0}},
    /* CCA优化功能使能: "vap0 alg_cfg cca_opt_alg_en_mode 0|1" */
    {"cca_opt_alg_en_mode",         MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE, {0, 0}},
    /* CCA优化DEBUG模式启动: "vap0 alg_cfg cca_opt_debug_mode 0|1" */
    {"cca_opt_debug_mode",          MAC_ALG_CFG_CCA_OPT_DEBUG_MODE, {0, 0}},
    /* CCA优化信道扫描的时间(ms):"vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    {"cca_opt_set_cca_th_debug",    MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG, {0, 0}},
    /* CCA设置能量和协议门限: "vap0 alg_cfg cca_opt_set_cca_th [20ed]" */
    {"cca_opt_set_cca_ed20",    MAC_ALG_CFG_CCA_OPT_SET_CCA_ED20, {0, 0}},
    /* CCA设置能量和协议门限: "vap0 alg_cfg cca_opt_set_cca_th [40ed]" */
    {"cca_opt_set_cca_ed40",    MAC_ALG_CFG_CCA_OPT_SET_CCA_ED40, {0, 0}},
    /* CCA设置能量和协议门限: "vap0 alg_cfg cca_opt_set_cca_th [dsss]" */
    {"cca_opt_set_cca_dsss",    MAC_ALG_CFG_CCA_OPT_SET_CCA_DSSS, {0, 0}},
    /* CCA设置能量和协议门限: "vap0 alg_cfg cca_opt_set_cca_th [odfm]" */
    {"cca_opt_set_cca_ofdm",    MAC_ALG_CFG_CCA_OPT_SET_CCA_OFDM, {0, 0}},

    {"tpc_mode",                MAC_ALG_CFG_TPC_MODE, {0, 0}},   /* 设置TPC工作模式 */
    {"tpc_dbg",                 MAC_ALG_CFG_TPC_DEBUG, {0, 0}},  /* 设置TPC的debug开关 */
    /* 查询tpc相关信息 */
    {"tpc_query",               MAC_ALG_CFG_TPC_QUERY, {0, 0}},
    {"tpc_fix_tx_pwr",          MAC_ALG_CFG_TPC_FIX_PWR_CODE, {0, 0}},             /* 2g发送功率增强值设置 */

    {"traffic_enable",             MAC_ALG_CFG_TRAFFIC_CTL_ENABLE, {0, 0}},
    {"traffic_timeout",            MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT, {0, 0}},
    {"traffic_log_debug",          MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG, {0, 0}},
    {"traffic_rx_enable",          MAC_ALG_CFG_TRAFFIC_CTL_RX_ENABLE, {0, 0}},
    {"traffic_tx_enable",          MAC_ALG_CFG_TRAFFIC_CTL_TX_ENABLE, {0, 0}},

    {"traffic_rx_restore_thres",   MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_THRESHOLD, {0, 0}},
    {"traffic_pkt_low_thres",      MAC_ALG_CFG_TRAFFIC_CTL_PKT_LOW_THRESHOLD, {0, 0}},
    {"traffic_pkt_high_thres",     MAC_ALG_CFG_TRAFFIC_CTL_PKT_HIGH_THRESHOLD, {0, 0}},
    {"traffic_pkt_busy_thres",     MAC_ALG_CFG_TRAFFIC_CTL_PKT_BUSY_THRESHOLD, {0, 0}},
    {"traffic_dscr_min_thres",     MAC_ALG_CFG_TRAFFIC_CTL_DSCR_MIN_THRESHOLD, {0, 0}},
    {"traffic_dscr_low_thres",     MAC_ALG_CFG_TRAFFIC_CTL_DSCR_LOW_THRESHOLD, {0, 0}},
    {"traffic_dscr_high_thres",    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_HIGH_THRESHOLD, {0, 0}},
    {"traffic_dscr_busy_thres",    MAC_ALG_CFG_TRAFFIC_CTL_DSCR_BUSY_THRESHOLD, {0, 0}},

    {"traffic_rx_small_restore_thres",   MAC_ALG_CFG_TRAFFIC_CTL_SMALL_RX_RESTORE_THRESHOLD, {0, 0}},
    {"traffic_small_pkt_low_thres",      MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_LOW_THRESHOLD, {0, 0}},
    {"traffic_small_pkt_high_thres",     MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_HIGH_THRESHOLD, {0, 0}},
    {"traffic_small_pkt_busy_thres",     MAC_ALG_CFG_TRAFFIC_CTL_SMALL_PKT_BUSY_THRESHOLD, {0, 0}},
    {"traffic_small_dscr_min_thres",     MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_MIN_THRESHOLD, {0, 0}},
    {"traffic_small_dscr_low_thres",     MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_LOW_THRESHOLD, {0, 0}},
    {"traffic_small_dscr_high_thres",    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_HIGH_THRESHOLD, {0, 0}},
    {"traffic_small_dscr_busy_thres",    MAC_ALG_CFG_TRAFFIC_CTL_SMALL_DSCR_BUSY_THRESHOLD, {0, 0}},
    {"traffic_tx_window_size",           MAC_ALG_CFG_TRAFFIC_CTL_TX_WINDOW_SIZE, {0, 0}},
    {"traffic_tx_netbuf_resv_size",      MAC_ALG_CFG_TRAFFIC_CTL_TX_NETBUF_RESV_SIZE, {0, 0}},
    {"traffic_tx_max_flowctl_cycle",     MAC_ALG_CFG_TRAFFIC_CTL_TX_MAX_FLOWCTL_CNT, {0, 0}},
    {"traffic_debug_dump",               MAC_ALG_CFG_TRAFFIC_CTL_INFO_DUMP, {0, 0}},
    {"traffic_debug_clear",              MAC_ALG_CFG_TRAFFIC_CTL_INFO_CLEAR, {0, 0}},

#ifdef _PRE_WLAN_FEATURE_MWO_DET
    {"mwo_det_enable",          MAC_ALG_CFG_MWO_DET_ENABLE, {0, 0}},                    /* 微波炉检测使能信号 */
    /* 停止微波炉信号发送时间计时的天线口功率门限（单位dBm） */
    {"mwo_det_end_rssi_th",     MAC_ALG_CFG_MWO_DET_END_RSSI_TH, {0, 0}},
    /* 启动微波炉信号发送时间计时的天线口功率门限（单位dBm） */
    {"mwo_det_start_rssi_th",   MAC_ALG_CFG_MWO_DET_START_RSSI_TH, {0, 0}},
    /* 发送描述符中anti_intf_1thr,c2 大于本门限选择无干扰速率，c2小于此门限选择有干扰速率 */
    {"mwo_det_debug",           MAC_ALG_CFG_MWO_DET_DEBUG},
#endif

    {"rts_status",              MAC_ALG_CFG_RTS_ENABLE, {0, 0}},   /* RTS算法使能配置命令 */
    {"rts_log",                 MAC_ALG_CFG_RTS_DEBUG, {0, 0}},    /* RTS算法日志开启配置命令 */
    {"rts_mode",                MAC_ALG_CFG_RTS_MODE, {0, 0}},     /* RTS模式配置命令 */
    {"rts_rate",                MAC_ALG_CFG_RTS_RATE, {0, 0}},     /* RTS速率配置命令 */
    {"rts_get_mode",            MAC_ALG_CFG_GET_RTS_MODE, {0, 0}}, /* 获取rts模式配置命令 */

    {"dbac_log_off",            MAC_ALG_CFG_DBAC_LOG_OFF, {0, 0}},
    {"dbac_log_on",             MAC_ALG_CFG_DBAC_LOG_ON, {0, 0}},
    {"dbac_fix_ratio",          MAC_ALG_CFG_DBAC_FIX_RATIO, {0, 0}},
    {"dbac_stats_info",         MAC_ALG_CFG_DBAC_STATS_INFO, {0, 0}},
    {"dbac_run_info",           MAC_ALG_CFG_DBAC_RUN_INFO, {0, 0}},
    {"dbac_stats_clear",        MAC_ALG_CFG_DBAC_STATS_CLEAR, {0, 0}},

    {"gla_usual_switch",        MAC_ALG_CFG_GLA_USUAL_SWITCH, {0, 0}},
    {"get_gla_usual_switch",    MAC_ALG_CFG_GET_GLA_USUAL_SWITCH, {0, 0}},
    {"gla_optional_switch",     MAC_ALG_CFG_GLA_OPTIONAL_SWITCH, {0, 0}},
    {"get_gla_optional_switch", MAC_ALG_CFG_GET_GLA_OPTIONAL_SWITCH, {0, 0}},

    {"ar_parameter_debug",         MAC_ALG_CFG_AR_PARAMETER_DEBUG, {0, 0}},

    {OAL_PTR_NULL}
};

STATIC OAL_CONST wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map_etc[] = {
    /* 设置聚合自适应算法最大聚合个数: "vap0 alg_cfg aggr_max_aggr_num [1-16]" */
    {"aggr_max_aggr_num",       MAC_ALG_CFG_AGGR_MAX_AGGR_NUM, {0, 0}},
        /* 获取干扰类型:"vap0 alg_cfg intf_type_enquiry" */
    {"intf_type_enquiry",   MAC_ALG_CFG_INTF_DET_GET_INTF_TYPE, {0, 0}},
    {"tpc_fix_tx_pwr",          MAC_ALG_CFG_TPC_FIX_PWR_CODE, {0, 0}},             /* 2g发送功率增强值设置 */
    {"rts_mode",                MAC_ALG_CFG_RTS_MODE, {0, 0}},     /* RTS模式配置命令 */
    {OAL_PTR_NULL}
};
/*****************************************************************************
  3. 函数定义
*****************************************************************************/
/*****************************************************************************
 功能描述  : 同步是否加载debug.ko的标志位
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_alg_debug_flag(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 flag = *(osal_u8 *)msg->data;
    unref_param(hmac_vap);
    g_alg_debug_flag = flag;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_alg_param_etc
 功能描述  : hmac, 算法配置命令示例
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_alg_param_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 通知各个算法填充配置参数 */
    hmac_alg_para_cfg_notify(hmac_vap, msg);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/*****************************************************************************
 功能描述  : 同步host pk mode的相关信息
*****************************************************************************/
osal_u32 hmac_config_pk_mode(hmac_vap_stru *hmac_vap, osal_u8 pk_mode)
{
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    hmac_alg_cfg_intf_det_pk_mode_notify(hmac_vap, pk_mode);
#endif
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 获取命令参数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_alg_get_alg_cfg_type(const osal_s8 *name, mac_alg_cfg_enum_uint16 *value)
{
    OAL_CONST wal_ioctl_alg_cfg_stru *alg_cfg_map = OSAL_NULL;
    osal_u32 map_size;
    osal_u32 map_index = 0;
    if (g_alg_debug_flag == OAL_TRUE) {
        alg_cfg_map = g_ast_alg_cfg_debug_map_etc;
        map_size = OAL_ARRAY_SIZE(g_ast_alg_cfg_debug_map_etc);
    } else {
        alg_cfg_map = g_ast_alg_cfg_map_etc;
        map_size = OAL_ARRAY_SIZE(g_ast_alg_cfg_map_etc);
    }

    for (map_index = 0; map_index < map_size; map_index++) {
        /* 增加判空，防止strcmp空指针访问 */
        if (alg_cfg_map[map_index].pc_name == OAL_PTR_NULL) {
            continue;
        }
        if (osal_strcmp(alg_cfg_map[map_index].pc_name, (const osal_char *)name) == 0) {
            break;
        }
    }

    /* 没有找到对应的命令，则报错 */
    if (map_index >= map_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_alg_get_alg_cfg_type::invalid alg_cfg command!}");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    *value = alg_cfg_map[map_index].alg_cfg;
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 获取dbac参数配置
*****************************************************************************/
OSAL_STATIC osal_u8* hmac_alg_get_dbac_cfg_params(osal_u16 cfg_type, const osal_s8 *pc_param, osal_u8 *len)
{
    osal_u32 ret;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_ioctl_alg_param_stru), OAL_TRUE);
    if (alg_param == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{alloc fail len [%u]!}", sizeof(mac_ioctl_alg_param_stru));
        return NULL;
    }
    (void)memset_s(alg_param, sizeof(mac_ioctl_alg_param_stru), 0, sizeof(mac_ioctl_alg_param_stru));
    alg_param->alg_cfg = cfg_type;
    if (cfg_type == MAC_ALG_CFG_DBAC_FIX_RATIO) {
        /* 获取参数配置值 */
        ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_name, OAL_SIZEOF(ac_name));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_dbac_cfg_params get arg return err_code [%d]!}", ret);
            oal_mem_free(alg_param, OAL_TRUE);
            return NULL;
        }
        /* 记录参数配置值 */
        alg_param->value = (osal_u32)oal_atoi((const osal_s8 *)ac_name);
    }
    *len = (osal_u8)sizeof(mac_ioctl_alg_param_stru);
    return (osal_u8*)alg_param;
}

/*****************************************************************************
 功能描述  : 获取gla参数配置
*****************************************************************************/
OSAL_STATIC osal_u8* hmac_alg_get_gla_cfg_params(osal_u16 cfg_type, const osal_s8 *pc_param, osal_u8 *len)
{
    osal_u32 ret;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN];
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_ioctl_alg_param_stru), OAL_TRUE);
    if (alg_param == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_gla_cfg_params:alloc fail len [%u]!}",
            sizeof(mac_ioctl_alg_param_stru));
        return NULL;
    }
    (void)memset_s(alg_param, sizeof(mac_ioctl_alg_param_stru), 0, sizeof(mac_ioctl_alg_param_stru));
    alg_param->alg_cfg = cfg_type;
    switch (cfg_type) {
        case MAC_ALG_CFG_GLA_USUAL_SWITCH :
        case MAC_ALG_CFG_GLA_OPTIONAL_SWITCH :
            /* 获取参数配置值 */
            ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_name, OAL_SIZEOF(ac_name));
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_gla_cfg_params:get_cmd_one_arg return err[%d]!}", ret);
                oal_mem_free(alg_param, OAL_TRUE);
                return NULL;
            }
            /* 记录参数配置值 */
            alg_param->value = (osal_u32)oal_atoi((const osal_s8 *)ac_name);
            break;
        case MAC_ALG_CFG_GET_GLA_USUAL_SWITCH :
        case MAC_ALG_CFG_GET_GLA_OPTIONAL_SWITCH :
            alg_param->value = 0;
            break;
        default:
            oal_mem_free(alg_param, OAL_TRUE);
            return NULL;
    }
    *len = (osal_u8)sizeof(mac_ioctl_alg_param_stru);
    return (osal_u8*)alg_param;
}

OAL_STATIC osal_u32 hmac_ccpriv_set_rts_status(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    alg_cfg_rts_param_tbl_stru rts_cfg_status_tbl[] = {
        {"enable",      0},
        {"disable",     1},
    };
    osal_u32 ul_ret, cfg_rts_table_size, status, idx;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    alg_cfg_rts_param_tbl_stru *cfg_rts_table = NULL;

    ul_ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_rts_status::hmac_ccpriv_get_one_arg err_code [%d]!}", ul_ret);
        return ul_ret;
    }
    ac_arg[sizeof(ac_arg) - 1] = '\0';    /* 确保以null结尾 */
    cfg_rts_table = &rts_cfg_status_tbl[0];
    cfg_rts_table_size = (osal_u32)(sizeof(rts_cfg_status_tbl) / sizeof(rts_cfg_status_tbl[0]));
    for (idx = 0; idx < cfg_rts_table_size; idx++) {
        if (osal_strcmp(cfg_rts_table[idx].pc_name, (const osal_char *)ac_arg) == 0) {
            break;
        }
    }
    if (idx >= cfg_rts_table_size) {
        return OAL_FAIL;
    }
    status = cfg_rts_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_status::status[%d]}\r\n", status);
    alg_param->value = status;

    return ul_ret;
}

OAL_STATIC osal_u32 hmac_ccpriv_set_rts_mode(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    alg_cfg_rts_param_tbl_stru rts_cfg_mode_tbl[] = {
        {"enable",      0},
        {"disable",     1},
        {"auto",        2},
        {"rank0off",    3},
        {"threshold",   4},
    };
    osal_u32 ul_ret, cfg_rts_table_size, mode, idx;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    alg_cfg_rts_param_tbl_stru *cfg_rts_table = NULL;

    ul_ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_rts_mode::hmac_ccpriv_get_one_arg err [%d]!}", ul_ret);
        return ul_ret;
    }
    ac_arg[sizeof(ac_arg) - 1] = '\0';    /* 确保以null结尾 */
    cfg_rts_table = &rts_cfg_mode_tbl[0];
    cfg_rts_table_size = (osal_u32)(sizeof(rts_cfg_mode_tbl) / sizeof(rts_cfg_mode_tbl[0]));
    for (idx = 0; idx < cfg_rts_table_size; idx++) {
        if (osal_strcmp(cfg_rts_table[idx].pc_name, (const osal_char *)ac_arg) == 0) {
            break;
        }
    }
    if (idx >= cfg_rts_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_mode::get mode fail!}\r\n");
        return OAL_FAIL;
    }
    mode = cfg_rts_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_mode::mode[%d]}\r\n", mode);
    alg_param->value = mode;

    return ul_ret;
}

OAL_STATIC osal_u32 hmac_ccpriv_set_rts_rate_part2(const osal_s8 **pc_param, osal_u32 *value)
{
    osal_u32 ul_ret, rate, idx, cfg_rts_table_size;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    alg_cfg_rts_param_tbl_stru *cfg_rts_table = NULL;

    alg_cfg_rts_param_tbl_stru ast_alg_rts_cfg_rate_tbl[] = {
        {"1Mlong",        0},
        {"5.5M",          1},
        {"11M",           2},
        {"6M",            3},
        {"12M",           4},
        {"24M",           5},
        {"auto",          6},
    };

    /* 3 獲取rate */
    ul_ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_rts_rate_part2::hmac_ccpriv_get_one_arg err [%d]!}", ul_ret);
        return ul_ret;
    }
    cfg_rts_table = &ast_alg_rts_cfg_rate_tbl[0];
    cfg_rts_table_size = (osal_u32)(sizeof(ast_alg_rts_cfg_rate_tbl) / sizeof(ast_alg_rts_cfg_rate_tbl[0]));
    for (idx = 0; idx < cfg_rts_table_size; idx++) {
        if (osal_strcmp(cfg_rts_table[idx].pc_name, (const osal_char *)ac_arg) == 0) {
            break;
        }
    }
    if (idx >= cfg_rts_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_rate_part2::get rate fail!}\r\n");
        return OAL_FAIL;
    }
    rate = cfg_rts_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_rate_part2::rate[%d]}\r\n", rate);
    *value |= (rate << ALG_RTS_CFG_RATE_SHIFT);

    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_rts_rate_part2:: rts rate command config failed!err[%d]}", ul_ret);
        return OAL_FAIL;
    }

    return ul_ret;
}

OAL_STATIC osal_u32 hmac_ccpriv_set_rts_rate(mac_ioctl_alg_param_stru *alg_param, const osal_s8 **pc_param)
{
    alg_cfg_rts_param_tbl_stru alg_rts_cfg_rank_tbl[] = {
        {"rank0",       0},
        {"rank1",       1},
        {"rank2",       2},
        {"rank3",       3},
    };
    osal_u32 ul_ret, idx, rank, cfg_rts_table_size;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 value = 0;
    alg_cfg_rts_param_tbl_stru *cfg_rts_table = NULL;
    /* 2. 獲取rank */
    ul_ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_rts_rate::hmac_ccpriv_get_one_arg err [%d]!}", ul_ret);
        return ul_ret;
    }
    cfg_rts_table = &alg_rts_cfg_rank_tbl[0];
    cfg_rts_table_size = (osal_u32)(sizeof(alg_rts_cfg_rank_tbl) / sizeof(alg_rts_cfg_rank_tbl[0]));
    for (idx = 0; idx < cfg_rts_table_size; idx++) {
        if (osal_strcmp(cfg_rts_table[idx].pc_name, (const osal_char *)ac_arg) == 0) {
            break;
        }
    }
    if (idx >= cfg_rts_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_rate::get rank fail!}\r\n");
        return OAL_FAIL;
    }
    rank = cfg_rts_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_rate::rank[%d]}\r\n", rank);
    value |= rank;

    ul_ret = hmac_ccpriv_set_rts_rate_part2(pc_param, &value);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts_rate_par3::cmd part2 fail!}\r\n");
        return OAL_FAIL;
    }
    alg_param->value = value;

    return ul_ret;
}

/*****************************************************************************
 功能描述  : 设置rts配置信息
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ccpriv_set_rts(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    osal_u32 ul_ret = OAL_SUCC;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_RTS_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_RTS_END)) {
        return OAL_SUCC;
    }

    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_RTS_ENABLE:
        case MAC_ALG_CFG_RTS_DEBUG:
            ul_ret = hmac_ccpriv_set_rts_status(alg_param, pc_param);
            break;

        case MAC_ALG_CFG_RTS_MODE:
            /* mode : 1 -- ALLENBALE; 0 -- ALLCLOSED; 2 -- auto; 3 -- rank0off; 4 -- mib */
            ul_ret = hmac_ccpriv_set_rts_mode(alg_param, pc_param);
            break;

        case MAC_ALG_CFG_RTS_RATE:
            ul_ret = hmac_ccpriv_set_rts_rate(alg_param, &pc_param);
            break;

        case MAC_ALG_CFG_GET_RTS_MODE:
            alg_param->value = 1;
            break;

        default:
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rts:param error[%d]!}", alg_param->alg_cfg);
            ul_ret = OAL_FAIL;
            break;
    }
    return ul_ret;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_frame_type(const osal_s8 **pc_param, osal_u32 *frame_type)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_frame_type_tbl[] = {
        {"ucast_data",       0},
        {"bcast_data",       1},
        {"ucast_mgmt",       2},
        {"bcast_mgmt",       3},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_frame_type::get_cmd_arg err[%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    cfg_ar_table = &alg_ar_cfg_frame_type_tbl[0];
    cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_frame_type_tbl) / sizeof(alg_ar_cfg_frame_type_tbl[0]));
    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_frame_type::get rank fail!}\r\n");
        return OAL_FAIL;
    }
    *frame_type = cfg_ar_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_frame_type::frame_type[%d]}\r\n", *frame_type);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_frame_type::frame_type[%d]}\r\n", *frame_type);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_rank(const osal_s8 **pc_param, osal_u32 *rank)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_rank_tbl[] = {
        {"rank0",       0},
        {"rank1",       1},
        {"rank2",       2},
        {"rank3",       3},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_rank::get_cmd_one_arg err[%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    cfg_ar_table = &alg_ar_cfg_rank_tbl[0];
    cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_rank_tbl) / sizeof(alg_ar_cfg_rank_tbl[0]));
    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_rank::get rank fail!}\r\n");
        return OAL_FAIL;
    }
    *rank = cfg_ar_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_rank::rank[%d]}\r\n", *rank);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_rank::rank[%d]}\r\n", *rank);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw(const osal_s8 **pc_param, osal_u32 *protocol,
    osal_u32 *bw)
{
    alg_cfg_autorate_protocol_bw_param_tbl_stru alg_ar_cfg_protocol_bw_tbl[] = {
        {"11g",             WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,       WLAN_BAND_ASSEMBLE_20M, {0, 0}},
        {"11b",             WLAN_11B_PHY_PROTOCOL_MODE,               WLAN_BAND_ASSEMBLE_20M, {0, 0}},
        {"11n20M",          WLAN_HT_PHY_PROTOCOL_MODE,                WLAN_BAND_ASSEMBLE_20M, {0, 0}},
        {"11n40M",          WLAN_HT_PHY_PROTOCOL_MODE,                WLAN_BAND_ASSEMBLE_40M, {0, 0}},
#ifdef _PRE_WLAN_SUPPORT_VHT
        {"11ac20M",         WLAN_VHT_PHY_PROTOCOL_MODE,               WLAN_BAND_ASSEMBLE_20M, {0, 0}},
#endif
        {"11ax20M",         WLAN_HE_SU_FORMAT,                        WLAN_BAND_ASSEMBLE_20M, {0, 0}},
        {"11axer20M",       WLAN_HE_EXT_SU_FORMAT,                    WLAN_BAND_ASSEMBLE_20M, {0, 0}},
        {"11axer106tone",   WLAN_HE_EXT_SU_FORMAT,                    WLAN_BAND_ASSEMBLE_SU_ER_106_TONE, {0, 0}},
    };
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw::get_cmd_one_arg er[%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_protocol_bw_tbl) / sizeof(alg_ar_cfg_protocol_bw_tbl[0]));
    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(alg_ar_cfg_protocol_bw_tbl[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw::get rank fail!}\r\n");
        return OAL_FAIL;
    }
    *protocol = alg_ar_cfg_protocol_bw_tbl[idx].protocol;
    *bw = alg_ar_cfg_protocol_bw_tbl[idx].bw;
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    hmac_set_al_tx_protocol((osal_u8)*protocol);
#endif
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_ccpriv_autorate_fix_rate_prtocol_bw::protocol[%d] bw[%d]}", *protocol, *bw);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw::protocol[%d] bw[%d]}\r\n", *protocol, *bw);
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_nss(const osal_s8 **pc_param, osal_u32 *nss)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_nss_tbl[] = {
        {"1ss",       0},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_nss::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    cfg_ar_table = &alg_ar_cfg_nss_tbl[0];
    cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_nss_tbl) / sizeof(alg_ar_cfg_nss_tbl[0]));
    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        return OAL_FAIL;
    }
    *nss = cfg_ar_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_nss::nss[%d]}\r\n", *nss);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_nss::nss[%d]}\r\n", *nss);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_idx(const osal_s8 **pc_param, osal_u32 *nss_rate,
    osal_u32 nss, osal_u32 protocol)
{
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size, rate_idx;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_idx::get_cmd_one_arg err[%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    for (idx = 0; idx < osal_array_size(g_alg_ar_rate_index_table_info); idx++) {
        if (g_alg_ar_rate_index_table_info[idx].protocol_type == protocol) {
            break;
        }
    }
    if (idx >= osal_array_size(g_alg_ar_rate_index_table_info)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_idx::err protocol[%d]!}", protocol);
        return OAL_FAIL;
    }

    cfg_ar_table = (alg_cfg_autorate_param_tbl_stru *)g_alg_ar_rate_index_table_info[idx].table_addr;
    cfg_ar_table_size = (osal_u32)g_alg_ar_rate_index_table_info[idx].table_size;

    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_idx::get RATE fail!}\r\n");
        return OAL_FAIL;
    }
    rate_idx = cfg_ar_table[idx].value;
    *nss_rate = (protocol == WLAN_HT_PHY_PROTOCOL_MODE) ? (rate_idx + WLAN_HT_MAX_BITMAP_LEN * nss) :
        ((nss << ALG_CFG_AUTORATE_MCS_SHIFT) | rate_idx);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    hmac_set_al_tx_mcs(rate_idx);
#endif
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_ccpriv_autorate_fix_rate_idx:rate[%d]nss_rate[%d]}", rate_idx, *nss_rate);
    wifi_printf("{hmac_ccpriv_autorate_fix_rate_idx::rate[%d]nss_rate[%d]}", rate_idx, *nss_rate);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_gi(const osal_s8 **pc_param, osal_u32 *gi, osal_u32 protocol)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_11ax_gi_tbl[] = {
        {"long", WLAN_PHY_HE_GI_3DOT2US}, {"short", WLAN_PHY_HE_GI_DOT8US}, {"mid", WLAN_PHY_HE_GI_1DOT6US},
    };
    const alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_gi::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    switch (protocol) {
        case WLAN_11B_PHY_PROTOCOL_MODE:
        case WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE:
            cfg_ar_table = &g_ast_alg_ar_cfg_11bg_gi_tbl[0];
            size = (osal_u32)(sizeof(g_ast_alg_ar_cfg_11bg_gi_tbl) / sizeof(g_ast_alg_ar_cfg_11bg_gi_tbl[0]));
            break;
        case WLAN_HT_PHY_PROTOCOL_MODE:
#ifdef _PRE_WLAN_SUPPORT_VHT
        case WLAN_VHT_PHY_PROTOCOL_MODE:
#endif
            cfg_ar_table = &g_ast_alg_ar_cfg_11n_gi_tbl[0];
            size = (osal_u32)(sizeof(g_ast_alg_ar_cfg_11n_gi_tbl) / sizeof(g_ast_alg_ar_cfg_11n_gi_tbl[0]));
            break;
        case WLAN_HE_SU_FORMAT:
        case WLAN_HE_EXT_SU_FORMAT:
            cfg_ar_table = &alg_ar_cfg_11ax_gi_tbl[0];
            size = (osal_u32)(sizeof(alg_ar_cfg_11ax_gi_tbl) / sizeof(alg_ar_cfg_11ax_gi_tbl[0]));
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_gi::rate, err protocol[%d]!}", protocol);
            return OAL_FAIL;
    }
    for (idx = 0; idx < size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }
    if (idx >= size) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_gi::get RATE fail!}\r\n");
        return OAL_FAIL;
    }
    *gi = cfg_ar_table[idx].value;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_gi::gi[%d]}\r\n", *gi);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_gi::gi[%d]}\r\n", *gi);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_he_ltf(const osal_s8 **pc_param, osal_u32 *he_ltf,
    osal_u32 protocol)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_he_ltf_tbl[] = {
        {"1x",       WLAN_PHY_HE_LTF_1X},
        {"2x",       WLAN_PHY_HE_LTF_2X},
        {"4x",       WLAN_PHY_HE_LTF_4X},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_he_ltf::get_cmd_one_arg err [%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    if (protocol == WLAN_HE_SU_FORMAT || protocol == WLAN_HE_EXT_SU_FORMAT) {
        cfg_ar_table = &alg_ar_cfg_he_ltf_tbl[0];
        cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_he_ltf_tbl) / sizeof(alg_ar_cfg_he_ltf_tbl[0]));
        for (idx = 0; idx < cfg_ar_table_size; idx++) {
            if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
                break;
            }
        }
        if (idx >= cfg_ar_table_size) {
            return OAL_FAIL;
        }
        *he_ltf = cfg_ar_table[idx].value;
    } else {
        *he_ltf = 0;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate_he_ltf::he_ltf[%d]}\r\n", *he_ltf);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate_he_ltf::he_ltf[%d]}\r\n", *he_ltf);
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_he_dcm(const osal_s8 **pc_param, osal_u32 *he_dcm,
    osal_u32 protocol)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_he_dcm_tbl[] = {
        {"dcm_off",      WLAN_PHY_DCM_OFF},
        {"dcm_on",       WLAN_PHY_DCM_ON},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */
    if (protocol == WLAN_HE_SU_FORMAT || protocol == WLAN_HE_EXT_SU_FORMAT) {
        cfg_ar_table = &alg_ar_cfg_he_dcm_tbl[0];
        cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_he_dcm_tbl) / sizeof(alg_ar_cfg_he_dcm_tbl[0]));
        for (idx = 0; idx < cfg_ar_table_size; idx++) {
            if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)ac_name) == 0) {
                break;
            }
        }
        if (idx >= cfg_ar_table_size) {
            return OAL_FAIL;
        }
        *he_dcm = cfg_ar_table[idx].value;
    } else {
        *he_dcm = 0;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate::he_dcm[%d]}\r\n", *he_dcm);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate::he_dcm[%d]}\r\n", *he_dcm);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_preamble_fec_coding(const osal_s8 **param, osal_u32 *data,
    osal_u32 flag)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_fec_coding_tbl[] = {
        {"bcc",   WLAN_BCC_CODE},
        {"ldpc",  WLAN_LDPC_CODE},
    };
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_preamble_tbl[] = {
        {"short_preamble", WLAN_PHY_PREAMBLE_SHORT_HTMF},
        {"long_preamble",  WLAN_PHY_PREAMBLE_LONG_HTGF},
    };
    alg_cfg_autorate_param_tbl_stru *cfg_ar_table = NULL;
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret, idx, cfg_ar_table_size;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    name[sizeof(name) - 1] = '\0';    /* 确保以null结尾 */
    if (flag == 1) {
        cfg_ar_table = &alg_ar_cfg_preamble_tbl[0];
        cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_preamble_tbl) / sizeof(alg_ar_cfg_preamble_tbl[0]));
    } else {
        cfg_ar_table = &alg_ar_cfg_fec_coding_tbl[0];
        cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_fec_coding_tbl) / sizeof(alg_ar_cfg_fec_coding_tbl[0]));
    }
    for (idx = 0; idx < cfg_ar_table_size; idx++) {
        if (osal_strcmp(cfg_ar_table[idx].pc_name, (const osal_char *)name) == 0) {
            break;
        }
    }
    if (idx >= cfg_ar_table_size) {
        return OAL_FAIL;
    }
    *data = cfg_ar_table[idx].value;
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate: preamble or fec_coding: [%d] data[%d] }\r\n",
        flag, *data);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate: preamble or fec_coding: [%d] data[%d]}\r\n", flag, *data);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_preamble(const osal_s8 **param, osal_u32 *preamble)
{
    return hmac_ccpriv_alg_autorate_fix_rate_preamble_fec_coding(param, preamble, 1); /* 1: preamble */
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_fec_coding(const osal_s8 **param, osal_u32 *fec_coding)
{
    return hmac_ccpriv_alg_autorate_fix_rate_preamble_fec_coding(param, fec_coding, 2); /* 2: fec_coding */
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_tx_cnt(const osal_s8 **pc_param, osal_u32 *tx_cnt)
{
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    *tx_cnt = OAL_MIN((osal_u32)oal_atoi((const osal_s8 *)ac_name), 7); // 7: tx cnt 描述符占3bit,最大可配7
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_fix_rate::tx_cnt[%d]}\r\n", *tx_cnt);
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate::tx_cnt[%d]}\r\n", *tx_cnt);
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate_part1(const osal_s8 **pc_param,
    alg_cfg_autorate_param_stru *param)
{
    /* 0. get frame type */
    if (hmac_ccpriv_alg_autorate_fix_rate_frame_type(pc_param, &(param->frame_type)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 1. get rank */
    if (hmac_ccpriv_alg_autorate_fix_rate_rank(pc_param, &(param->rank)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 2. get prtocol_bw */
    if (hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw(pc_param, &(param->protocol),
        &(param->bw)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 3. get nss */
    if (hmac_ccpriv_alg_autorate_fix_rate_nss(pc_param, &(param->nss)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 4. get rate indx */
    if (hmac_ccpriv_alg_autorate_fix_rate_idx(pc_param, &(param->nss_rate), param->nss,
        param->protocol) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_fix_rate(mac_ioctl_alg_param_stru *alg_param, const osal_s8 **pc_param)
{
    alg_cfg_autorate_param_stru param = {0};
    osal_u32 value = 0;

    if (hmac_ccpriv_alg_autorate_fix_rate_part1(pc_param, &param) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 5. get gi */
    if (hmac_ccpriv_alg_autorate_fix_rate_gi(pc_param, &(param.gi), param.protocol) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 6. get he_ltf */
    if (hmac_ccpriv_alg_autorate_fix_rate_he_ltf(pc_param, &(param.he_ltf), param.protocol) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 7. get he_dcm */
    if (hmac_ccpriv_alg_autorate_fix_rate_he_dcm(pc_param, &(param.he_dcm), param.protocol) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 8. get preamble */
    if (hmac_ccpriv_alg_autorate_fix_rate_preamble(pc_param, &(param.preamble)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 9. get fec_coding */
    if (hmac_ccpriv_alg_autorate_fix_rate_fec_coding(pc_param, &(param.fec_coding)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 10. get tx_count */
    if (hmac_ccpriv_alg_autorate_fix_rate_tx_cnt(pc_param, &(param.tx_cnt)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 11. build value */
    value = (value | ((param.frame_type << ALG_CFG_AUTORATE_FRAME_TYPE_SHIFT) & ALG_CFG_AUTORATE_FRAME_TYPE_BIT));
    value = (value | ((param.rank << ALG_CFG_AUTORATE_RANK_SHIFT) & ALG_CFG_AUTORATE_RANK_BIT));
    value = (value | ((param.nss_rate << ALG_CFG_AUTORATE_NSS_MCS_SHIFT) & ALG_CFG_AUTORATE_NSS_MCS_BIT));
    value = (value | ((param.he_dcm << ALG_CFG_AUTORATE_HE_DCM_SHIFT) & ALG_CFG_AUTORATE_HE_DCM_BIT));
    value = (value | ((param.tx_cnt << ALG_CFG_AUTORATE_TX_CNT_SHIFT) & ALG_CFG_AUTORATE_TX_CNT_BIT));
    value = (value | ((param.he_ltf << ALG_CFG_AUTORATE_HE_LTF_SHIFT) & ALG_CFG_AUTORATE_HE_LTF_BIT));
    value = (value | ((param.gi << ALG_CFG_AUTORATE_GI_SHIFT) & ALG_CFG_AUTORATE_GI_BIT));
    value = (value | ((param.fec_coding << ALG_CFG_AUTORATE_FEC_CODING_SHIFT) & ALG_CFG_AUTORATE_FEC_CODING_BIT));
    value = (value | ((param.preamble << ALG_CFG_AUTORATE_PREAMBLE_SHIFT) & ALG_CFG_AUTORATE_PREAMBLE_BIT));
    value = (value | ((param.bw << ALG_CFG_AUTORATE_FREQ_BW_SHIFT) & ALG_CFG_AUTORATE_FREQ_BW_BIT));
    value = (value | ((param.protocol << ALG_CFG_AUTORATE_PROTOCOL_SHIFT) & ALG_CFG_AUTORATE_PROTOCOL_BIT));
    alg_param->value = value;
    wifi_printf("{hmac_ccpriv_alg_autorate_fix_rate::value[%d]}\r\n", value);
    wifi_printf("OK\r\n");
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_rate_mode(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_rate_mode_tbl[] = {
        {"auto",       0},
        {"fixed",      1},
    };
    osal_u32                      ret;
    osal_u32                      prot_idx;
    osal_s8                       ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32  cfg_ar_table_size = (osal_u32)(sizeof(alg_ar_cfg_rate_mode_tbl) / sizeof(alg_ar_cfg_rate_mode_tbl[0]));

    /* 获取参数配置值 */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_rate_mode::get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */

    for (prot_idx = 0; prot_idx < cfg_ar_table_size; prot_idx++) {
        if (osal_strcmp(alg_ar_cfg_rate_mode_tbl[prot_idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }

    if (prot_idx >= cfg_ar_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_rate_mode::unrecognized protocol string!}\r\n");
        return OAL_FAIL;
    }
    alg_param->value = alg_ar_cfg_rate_mode_tbl[prot_idx].value;

    wifi_printf("OK\r\n");
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_sub_feature_switch(mac_ioctl_alg_param_stru *alg_param,
    const osal_s8 *pc_param)
{
    alg_cfg_autorate_param_tbl_stru alg_ar_cfg_sub_feature_swtich_tbl[] = {
        {"disable",      0},
        {"enable",       1},
    };
    osal_u32                      ret;
    osal_u32                      prot_idx;
    osal_s8                       ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32                       table_size;
    table_size = (osal_u32)(sizeof(alg_ar_cfg_sub_feature_swtich_tbl) / sizeof(alg_ar_cfg_sub_feature_swtich_tbl[0]));

    /* 获取参数配置值 */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_sub_feature_switch::get_cmd_one_arg err[%d]!}", ret);
        return ret;
    }

    ac_name[sizeof(ac_name) - 1] = '\0';    /* 确保以null结尾 */

    for (prot_idx = 0; prot_idx < table_size; prot_idx++) {
        if (osal_strcmp(alg_ar_cfg_sub_feature_swtich_tbl[prot_idx].pc_name, (const osal_char *)ac_name) == 0) {
            break;
        }
    }

    if (prot_idx >= table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_sub_feature_switch::unrecognized protocol string!}");
        return OAL_FAIL;
    }
    alg_param->value = alg_ar_cfg_sub_feature_swtich_tbl[prot_idx].value;

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ccpriv_alg_autorate_cfg(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    osal_u32                      ret;
    if (!(alg_param->alg_cfg > MAC_ALG_CFG_AUTORATE_DMAC_START && alg_param->alg_cfg < MAC_ALG_CFG_AUTORATE_END)) {
        return OAL_SUCC;
    }
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_AUTORATE_RATE_MODE:
            ret = hmac_ccpriv_alg_autorate_rate_mode(alg_param, pc_param);
            break;

        case MAC_ALG_CFG_AUTORATE_FIX_RATE:
            ret = hmac_ccpriv_alg_autorate_fix_rate(alg_param, &pc_param);
            break;

        case MAC_ALG_CFG_AUTORATE_DEBUG_LOG:
        case MAC_ALG_CFG_AUTORATE_AUTO_BW:
        case MAC_ALG_CFG_AUTORATE_AUTO_PROTOCOL:
        case MAC_ALG_CFG_AUTORATE_EVENT_TRIG_DESCEND_BW:
        case MAC_ALG_CFG_AUTORATE_IS_ENABLE:
            ret = hmac_ccpriv_alg_autorate_sub_feature_switch(alg_param, pc_param);
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_autorate_cfg:param error[%d]!}", alg_param->alg_cfg);
            return OAL_FAIL;
    }
    return ret;
}

/*****************************************************************************
 功能描述  : 获取算法配置取值
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_alg_get_default_cfg_val(mac_ioctl_alg_param_stru *alg_param, const osal_s8 *pc_param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ret;

    /* 获取参数配置值 */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(&pc_param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_default_cfg_val::hmac_ccpriv_get_one_arg err [%d]!}", ret);
        return ret;
    }
    /* 记录参数配置值 */
    alg_param->value = (osal_u32)oal_atoi((const osal_s8 *)name);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取算法配置信息
*****************************************************************************/
OSAL_STATIC osal_u8* hmac_alg_get_default_cfg_params(osal_u16 cfg_type, const osal_s8 *pc_param, osal_u8 *len)
{
    osal_u32 ret;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_ioctl_alg_param_stru), OAL_TRUE);
    if (alg_param == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{alloc fail len [%u]!}", sizeof(mac_ioctl_alg_param_stru));
        return NULL;
    }
    (void)memset_s(alg_param, sizeof(mac_ioctl_alg_param_stru), 0, sizeof(mac_ioctl_alg_param_stru));
    alg_param->alg_cfg = cfg_type;

    /* default cfg proc */
    ret = hmac_alg_get_default_cfg_val(alg_param, pc_param);
    if (ret != OAL_SUCC) {
        oal_mem_free(alg_param, OAL_TRUE);
        return NULL;
    }

    /* rts cfg proc */
    ret = hmac_ccpriv_set_rts(alg_param, pc_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_cfg_etc::alg_autorate_cfg return err_code [%d]!}", ret);
        oal_mem_free(alg_param, OAL_TRUE);
        return NULL;
    }
    /* autorate cfg proc */
    ret = hmac_ccpriv_alg_autorate_cfg(alg_param, pc_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_cfg_etc::alg_autorate_cfg return err_code [%d]!}", ret);
        oal_mem_free(alg_param, OAL_TRUE);
        return NULL;
    }
    *len = (osal_u8)sizeof(mac_ioctl_alg_param_stru);
    return (osal_u8*)alg_param;
}

/*****************************************************************************
 功能描述  : 获取算法配置信息
*****************************************************************************/
OSAL_STATIC osal_u8* hmac_alg_get_cfg_params(const osal_s8 *param, osal_u8 *len)
{
    osal_u32 ret;
    osal_s8 ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u16 cfg_type = 0;

    /* 获取配置参数名称 */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_alg_get_cfg_params::hmac_ccpriv_get_one_arg return err_code [%d]!}", ret);
        return NULL;
    }
    ret = hmac_alg_get_alg_cfg_type((const osal_s8 *)ac_name, &cfg_type);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_cfg_params::hmac_alg_get_alg_cfg_type err_code [%d]!}", ret);
        return NULL;
    }
    if (cfg_type > MAC_ALG_CFG_DBAC_START && cfg_type < MAC_ALG_CFG_DBAC_END) {
        return hmac_alg_get_dbac_cfg_params(cfg_type, param, len);
    }
    if (cfg_type > MAC_ALG_CFG_GLA_START && cfg_type < MAC_ALG_CFG_GLA_END) {
        return hmac_alg_get_gla_cfg_params(cfg_type, param, len);
    }
    /* 如需自定义配置结构体，可新增获取参数接口 */
    /* default */
    return hmac_alg_get_default_cfg_params(cfg_type, param, len);
}

/*****************************************************************************
 功能描述  : 算法模块参数配置
*****************************************************************************/
osal_s32 hmac_ccpriv_alg_cfg_etc(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8* cfg_params;
    osal_u8  len = 0;
    frw_msg  alg_msg = {0};

    cfg_params = hmac_alg_get_cfg_params(param, &len);
    if (cfg_params == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_alg_get_cfg_params fail!}");
        return OAL_FAIL;
    }
    cfg_msg_init(cfg_params, len, OSAL_NULL, 0, &alg_msg);

    /* 通知各个算法填充配置参数 */
    hmac_alg_para_cfg_notify(hmac_vap, &alg_msg);

    oal_mem_free(cfg_params, OAL_TRUE);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
OSAL_STATIC osal_s32 hmac_alg_get_intrf_mode_cfg(const osal_s8 *name, mac_alg_intrf_mode_cfg_enum_uint8 *value)
{
    wal_ioctl_alg_intrf_mode_stru alg_intrf_mode_cfg_info;
    osal_u8 map_index = 0;

    alg_intrf_mode_cfg_info = g_alg_intrf_mode_cfg_map_etc[0];
    while (alg_intrf_mode_cfg_info.name != OAL_PTR_NULL) {
        if (osal_strcmp(alg_intrf_mode_cfg_info.name, (const osal_char *)name) == 0) {
            break;
        }
        alg_intrf_mode_cfg_info = g_alg_intrf_mode_cfg_map_etc[++map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (alg_intrf_mode_cfg_info.name == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_alg_get_intrf_mode_cfg::invalid alg_intrf_mode_cfg!}");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    *value = g_alg_intrf_mode_cfg_map_etc[map_index].intrf_mode_cfg;
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 干扰场景优化配置
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_alg_intrf_mode(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8                              name[CCPRIV_CMD_NAME_MAX_LEN];
    mac_ioctl_alg_intrf_mode_param_stru  intrf_mode_info = {0};
    mac_ioctl_alg_intrf_mode_param_stru *intrf_cfg_param = &intrf_mode_info;
    osal_s32                             ret;
    frw_msg                              msg_info = {0};

    /* 获取配置参数名称 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ccpriv_alg_intrf_mode::hmac_ccpriv_get_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    ret = hmac_alg_get_intrf_mode_cfg((const osal_s8 *)name, &intrf_cfg_param->alg_intrf_mode_cfg);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ccpriv_alg_intrf_mode::hmac_alg_get_intrf_mode_cfg err [%d]!}", ret);
        return ret;
    }

    /* 获取参数配置值 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ccpriv_alg_intrf_mode::hmac_ccpriv_get_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    /* 记录参数配置值 */
    intrf_cfg_param->value = (osal_u16)oal_atoi((const osal_s8 *)name);

    cfg_msg_init((osal_u8 *)&intrf_mode_info, OAL_SIZEOF(intrf_mode_info), OSAL_NULL, 0, &msg_info);
    ret = alg_intrf_mode_config_param(hmac_vap, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_intrf_mode::alg_intrf_mode_config_param err [%d]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OSAL_STATIC osal_s32 hmac_ccpriv_alg_intrf_mode_mixed(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 mode_value = 0;
    mac_ioctl_alg_intrf_mode_param_stru intrf_mode_info = {0};
    osal_u32 i;
    osal_s32 ret;
    struct {
        osal_u32 bit_pos;
        osal_u32 alg_intrf_mode_cfg;
    } bit_type[] = {
        {BIT0, MAC_ALG_INTRF_MODE_CCA_SWITCH},      // cca_switch映射到bit0
        {BIT1, MAC_ALG_INTRF_MODE_NO_11B_SWITCH},   // descend 11b 映射到bit1
    };
    frw_msg msg_info = {0};

    /* 获取参数配置值 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ccpriv_alg_intrf_mode_mixed::hmac_ccpriv_get_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    mode_value = (osal_u32)oal_atoi((const osal_s8 *)name);

    for (i = 0; i < osal_array_size(bit_type); i++) {
        intrf_mode_info.alg_intrf_mode_cfg = bit_type[i].alg_intrf_mode_cfg;
        intrf_mode_info.value = ((mode_value & bit_type[i].bit_pos) != 0) ? 1 : 0 ;

        (osal_void)memset_s(&msg_info, sizeof(frw_msg), 0, sizeof(frw_msg));
        cfg_msg_init((osal_u8 *)&intrf_mode_info, OAL_SIZEOF(intrf_mode_info), OSAL_NULL, 0, &msg_info);
        ret = alg_intrf_mode_config_param(hmac_vap, &msg_info);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_intrf_mode_mixed::send msg err [%d]!}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 设置11b发送次数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_alg_set_11b_tx_count(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8  name[CCPRIV_CMD_NAME_MAX_LEN];
    osal_s8  tx_count[HAL_TX_RATE_MAX_NUM] = {0};
    osal_s32 ret;
    osal_s32 idx;
    frw_msg  msg_info = {0};
    /* 没加载debug.ko，则命令不生效 */
    if (g_alg_debug_flag == OAL_FALSE) {
        return OAL_FAIL;
    }

    for (idx = 0; idx < HAL_TX_RATE_MAX_NUM; idx++) { /* 4级速率 */
        /* 获取发送次数 */
        ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{hmac_ccpriv_alg_set_11b_tx_count::hmac_ccpriv_get_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        /* 记录参数配置值 */
        tx_count[idx] = (osal_s8)oal_atoi((const osal_s8 *)name);
        if ((tx_count[idx] > HAL_MAX_TX_COUNT) || (tx_count[idx] < 0)) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{hmac_ccpriv_alg_set_11b_tx_count::tx_count=[%u]!}", tx_count[idx]);
            return OAL_FAIL;
        }
    }
    if (tx_count[0] == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_ccpriv_alg_set_11b_tx_count param error! SET FAIL!!!");
        return OAL_FAIL;
    }
    for (idx = 1; idx < HAL_TX_RATE_MAX_NUM; idx++) {
        if ((tx_count[idx - 1] == 0) && (tx_count[idx] != 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "hmac_ccpriv_alg_set_11b_tx_count param error! SET FAIL!!!");
            return OAL_FAIL;
        }
    }

    wifi_printf("hmac_ccpriv_alg_set_11b_tx_count, tx_count=[%u].\r\n", tx_count[0]);

    frw_msg_init((osal_u8 *)tx_count, (sizeof(osal_s8) * HAL_TX_RATE_MAX_NUM), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SET_11B_TX_COUNT, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_set_11b_tx_count:: send msg to device fail=%d!", ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 设置rts门限
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_rts_threshold(hmac_vap_stru *hmac_vap_temp, frw_msg *msg)
{
    mac_cfg_rts_threshold_stru *rts_threshold = OSAL_NULL;
    osal_u32 vap_idx;
    hmac_device_stru *hmac_device  = hmac_res_get_mac_dev_etc(0);
    unref_param(hmac_vap_temp);

    if (msg->data == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_rts_threshold:: param is null ptr %d!}",
            msg->data == OSAL_NULL);
        return OAL_ERR_CODE_PTR_NULL;
    }

    rts_threshold = (mac_cfg_rts_threshold_stru *)msg->data;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_TPC, "vap_id[%d] {hmac_vap null.}", hmac_device->vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        if (hmac_vap->mib_info == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_rts_threshold: hmac_vap->mib_info is null ptr!}",
                hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        wifi_printf("service api :rts thres set ,Value: %d\r\n", rts_threshold->rts_threshold);
        mac_mib_set_rts_threshold(hmac_vap, rts_threshold->rts_threshold);
#ifdef _PRE_WLAN_FEATURE_RTS
        alg_rts_config_threshold(hmac_vap, rts_threshold->rts_threshold);
#endif
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_ccpriv_alg_set_rts_threshold(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32 ret;
    osal_s8 ac_arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_rts_threshold_stru threshold;
    frw_msg msg_info = {0};
    osal_s32 value;

    /* 获取dot11RTSThreshold */
    ret = hmac_ccpriv_get_one_arg(&pc_param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_alg_set_rts_threshold::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    ac_arg[sizeof(ac_arg) - 1] = '\0';    /* 确保以null结尾 */
    value = oal_atoi((const osal_s8 *)ac_arg); // 增加入参判断，防止输入负数
    if (value < 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_alg_set_rts_threshold::out of range!}");
        return OAL_FAIL;
    }
    threshold.rts_threshold = (osal_u32)value;

    cfg_msg_init((osal_u8 *)&threshold, OAL_SIZEOF(mac_cfg_rts_threshold_stru), OSAL_NULL, 0, &msg_info);
    ret = hmac_config_rts_threshold(hmac_vap, &msg_info);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_alg_set_rts_threshold::return err code [%d]!}", ret);
        return ret;
    }

    return ret;
}


/*****************************************************************************
 功能描述  : 固定速率参数解析
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ccpriv_alg_ar_set_tx_rate_parse_param(alg_cfg_tx_rate_param_stru *tx_rate_param,
    osal_u32 *tx_rate_value, osal_u32 frame)
{
    osal_u32 protocol = 0;
    osal_u32 bw = WLAN_BAND_ASSEMBLE_20M; /* 11n支持20M和40M带宽，其他只支持20M */
    osal_u32 preamble = WLAN_PHY_PREAMBLE_SHORT_HTMF; /* 11b时有效，默认配置为0 */
    osal_u32 fec_coding = WLAN_BCC_CODE; /* 默认BCC编码 */
    osal_u32 gi = WLAN_GI_LONG; /* 默认long gi */
    osal_u32 he_ltf = 0; /* he时有效，默认配置为0 */
    osal_u32 tx_cnt = 3; /* 发送次数3 */
    osal_u32 he_dcm = WLAN_PHY_DCM_OFF; /* he时有效，默认关dcm */
    osal_u32 frame_type = frame;
    osal_u32 nss_rate = tx_rate_param->rate_index;
    osal_u32 value = 0;

    /* 如果不是配置固定速率参数，返回 */
    if (tx_rate_param->alg_cfg != MAC_ALG_CFG_AUTORATE_FIX_RATE) {
        return OAL_SUCC;
    }

    switch (tx_rate_param->protocol) {
        case WLAN_11B_PHY_PROTOCOL_MODE:
            preamble = (tx_rate_param->rate_index == WLAN_PHY_RATE_1M) ?
                WLAN_PHY_PREAMBLE_LONG_HTGF : WLAN_PHY_PREAMBLE_SHORT_HTMF;
            break;
        case WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE:
            break;
        case WLAN_HT_PHY_PROTOCOL_MODE:
            bw = tx_rate_param->bw;
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_SU_FORMAT:
            fec_coding = WLAN_LDPC_CODE;
            gi = WLAN_PHY_HE_GI_3DOT2US;
            he_ltf = WLAN_HE_LTF_4X;
            break;
#endif
        default:
            wifi_printf("{hmac_ccpriv_alg_ar_set_tx_rate_parse_param::err protocol[%u]!}", protocol);
            return OAL_FAIL;
    }
    protocol = tx_rate_param->protocol;
    value = (value | ((frame_type << ALG_CFG_AUTORATE_FRAME_TYPE_SHIFT) & ALG_CFG_AUTORATE_FRAME_TYPE_BIT));
    value = (value | ((nss_rate << ALG_CFG_AUTORATE_NSS_MCS_SHIFT) & ALG_CFG_AUTORATE_NSS_MCS_BIT));
    value = (value | ((he_dcm << ALG_CFG_AUTORATE_HE_DCM_SHIFT) & ALG_CFG_AUTORATE_HE_DCM_BIT));
    value = (value | ((tx_cnt << ALG_CFG_AUTORATE_TX_CNT_SHIFT) & ALG_CFG_AUTORATE_TX_CNT_BIT));
    value = (value | ((he_ltf << ALG_CFG_AUTORATE_HE_LTF_SHIFT) & ALG_CFG_AUTORATE_HE_LTF_BIT));
    value = (value | ((gi << ALG_CFG_AUTORATE_GI_SHIFT) & ALG_CFG_AUTORATE_GI_BIT));
    value = (value | ((fec_coding << ALG_CFG_AUTORATE_FEC_CODING_SHIFT) & ALG_CFG_AUTORATE_FEC_CODING_BIT));
    value = (value | ((preamble << ALG_CFG_AUTORATE_PREAMBLE_SHIFT) & ALG_CFG_AUTORATE_PREAMBLE_BIT));
    value = (value | ((bw << ALG_CFG_AUTORATE_FREQ_BW_SHIFT) & ALG_CFG_AUTORATE_FREQ_BW_BIT));
    value = (value | ((protocol << ALG_CFG_AUTORATE_PROTOCOL_SHIFT) & ALG_CFG_AUTORATE_PROTOCOL_BIT));

    *tx_rate_value = value;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 固定速率入参检查
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ccpriv_alg_ar_set_tx_rate_check(const alg_cfg_tx_rate_param_stru *tx_rate_param)
{
    /* 如果不是配置固定速率参数，返回 */
    if (tx_rate_param->alg_cfg != MAC_ALG_CFG_AUTORATE_FIX_RATE) {
        return OAL_SUCC;
    }

    switch (tx_rate_param->protocol) {
        case WLAN_11B_PHY_PROTOCOL_MODE:
            if ((tx_rate_param->rate_index > WLAN_PHY_RATE_11M) ||
                (tx_rate_param->bw == WLAN_BAND_ASSEMBLE_40M)) {
                return OAL_FAIL;
            }
            break;
        case WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE:
            if ((tx_rate_param->rate_index >= WLAN_PHY_RATE_BUTT) ||
                (tx_rate_param->rate_index < WLAN_PHY_RATE_48M) ||
                (tx_rate_param->bw == WLAN_BAND_ASSEMBLE_40M)) {
                return OAL_FAIL;
            }
            break;
        case WLAN_HT_PHY_PROTOCOL_MODE:
            if (tx_rate_param->rate_index >= WLAN_HT_MCS_BUTT) {
                return OAL_FAIL;
            }
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_SU_FORMAT:
            if ((tx_rate_param->rate_index >= WLAN_HE_MCS_BUTT) ||
                (tx_rate_param->bw == WLAN_BAND_ASSEMBLE_40M)) {
                return OAL_FAIL;
            }
            break;
#endif
        default:
            return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 功能描述  : TRC与SETRATE命令参数设置
*****************************************************************************/
OSAL_STATIC osal_void hmac_ccpriv_alg_ar_set_tx_rate_param(alg_cfg_tx_rate_param_stru *tx_rate_param,
    hmac_vap_stru *hmac_vap, osal_u32 value)
{
    frw_msg msg_info = {0};
    mac_ioctl_alg_param_stru alg_info = {0};

    alg_info.alg_cfg = tx_rate_param->alg_cfg;
    alg_info.value = value;
    (osal_void)memset_s(&msg_info, sizeof(frw_msg), 0, sizeof(frw_msg));
    cfg_msg_init((osal_u8 *)&alg_info, sizeof(mac_ioctl_alg_param_stru), OSAL_NULL, 0, &msg_info);
    hmac_config_alg_param_etc(hmac_vap, &msg_info);
}

/*****************************************************************************
 功能描述  : 同步参数设置固定速率
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_ar_set_tx_rate(hmac_vap_stru *hmac_vap_tmp, frw_msg *msg)
{
    osal_u32 rank, vap_idx;
    osal_u32 value = 0;
    osal_u32 value_tmp = 0;
    hmac_device_stru *hmac_device  = hmac_res_get_mac_dev_etc(0);
    alg_cfg_tx_rate_param_stru *tx_rate_param = (alg_cfg_tx_rate_param_stru *)msg->data;
    /* 3:bcast_mgmt 0:ucast_data */
    osal_u32 frame = ALG_FRAME_TYPE_UCAST_DATA;
    unref_param(hmac_vap_tmp);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if (hmac_get_al_tx_frame_type_switch() == 1) {
        frame = ALG_FRAME_TYPE_BCAST_MGMT;
    }
#endif
    if (hmac_ccpriv_alg_ar_set_tx_rate_check(tx_rate_param) != OAL_SUCC) {
        wifi_printf("{hmac_ccpriv_alg_ar_set_txrate_sync::protocol[%u], bw[%u], rate_index[%u]!}",
            tx_rate_param->protocol, tx_rate_param->bw, tx_rate_param->rate_index);
        return OAL_FAIL;
    }
    if (hmac_ccpriv_alg_ar_set_tx_rate_parse_param(tx_rate_param, &value, frame) != OAL_SUCC) {
        return OAL_FAIL;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_TPC, "vap_id[%d] {hmac_vap null.}", hmac_device->vap_id[vap_idx]);
            return OAL_FAIL;
        }
        /* 如果是固定速率参数配置，配置四档速率 */
        for (rank = 0; rank < 4; rank++) { /* 4级速率 */
            value_tmp = (tx_rate_param->alg_cfg == MAC_ALG_CFG_AUTORATE_FIX_RATE) ? (value |
                ((rank << ALG_CFG_AUTORATE_RANK_SHIFT) & ALG_CFG_AUTORATE_RANK_BIT)) : tx_rate_param->fix_rate_mode;
            hmac_ccpriv_alg_ar_set_tx_rate_param(tx_rate_param, hmac_vap, value_tmp);
            /* 如果是速率模式配置，下发一次命令即可 */
            if (tx_rate_param->alg_cfg == MAC_ALG_CFG_AUTORATE_RATE_MODE) {
                break;
            }
        }

        if (frame == ALG_FRAME_TYPE_UCAST_DATA) {
            break;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : debug命令配置固定速率
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_alg_get_udata_fix_rate_cfg(alg_cfg_tx_rate_param_stru *tx_rate_param,
    const osal_s8 **param)
{
    osal_u32 protocol = 0;
    osal_u32 nss_rate;
    osal_u32 bw;

    /* 1. get prtocol_bw */
    if (hmac_ccpriv_alg_autorate_fix_rate_prtocol_bw(param, &protocol, &bw) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 2. get rate indx */
    if (hmac_ccpriv_alg_autorate_fix_rate_idx(param, &nss_rate, 0, protocol) != OAL_SUCC) {
        return OAL_FAIL;
    }

    tx_rate_param->bw = bw;
    tx_rate_param->protocol = (wlan_phy_protocol_enum_uint8)protocol;
    tx_rate_param->rate_index = nss_rate;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : debug命令配置固定速率参数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_alg_set_udata_fix_rate(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    alg_cfg_tx_rate_param_stru tx_rate_param = {0};
    frw_msg msg_info = {0};

    tx_rate_param.alg_cfg = MAC_ALG_CFG_AUTORATE_FIX_RATE;
    if (hmac_ccpriv_alg_get_udata_fix_rate_cfg(&tx_rate_param, &param) != OAL_SUCC) {
        return OAL_FAIL;
    }

    msg_info.data = (osal_u8 *)&tx_rate_param;
    msg_info.data_len = (osal_u16)sizeof(alg_cfg_tx_rate_param_stru);
    if (hmac_config_ar_set_tx_rate(hmac_vap, &msg_info) != OAL_SUCC) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : debug命令配置固定速率模式
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_alg_set_udata_rate_mode(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    alg_cfg_tx_rate_param_stru tx_rate_param = {0};
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN];
    frw_msg msg_info = {0};
    osal_s32 ret;

    tx_rate_param.alg_cfg = MAC_ALG_CFG_AUTORATE_RATE_MODE;

    /* 获取参数配置值 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_alg_set_udata_rate_mode::hmac_ccpriv_get_one_arg err[%d]!}", ret);
        return ret;
    }
    /* 记录参数配置值,判断是否为0或1 */
    tx_rate_param.fix_rate_mode = (osal_u8)oal_atoi((const osal_s8 *)name);
    if ((tx_rate_param.fix_rate_mode != OSAL_FALSE) && (tx_rate_param.fix_rate_mode != OSAL_TRUE)) {
        return OAL_FAIL;
    }
    msg_info.data = (osal_u8 *)&tx_rate_param;
    msg_info.data_len = (osal_u16)sizeof(alg_cfg_tx_rate_param_stru);
    if (hmac_config_ar_set_tx_rate(hmac_vap, &msg_info) != OAL_SUCC) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WIFI_PRINTK
#define ALG_RATE_HE_PRINT_FORMAT "%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u"
#define alg_rate_he_print_data(a, b) a[0].b, a[1].b, a[2].b, a[3].b, a[4].b, a[5].b, a[6].b, a[7].b, a[8].b, a[9].b
#define ALG_RATE_HT_PRINT_FORMAT "%-8u%-8u%-8u%-8u%-8u%-8u%-8u%-8u"
#define alg_rate_ht_print_data(a, b) a[0].b, a[1].b, a[2].b, a[3].b, a[4].b, a[5].b, a[6].b, a[7].b
/* 入参有效性由调用者保证 */
osal_void hmac_query_alg_tx_rate_stats_detail(frw_msg *msg, osal_s32 ret, osal_u8 value)
{
    alg_rate_all_stats_stru *stats = (alg_rate_all_stats_stru *)msg->rsp;
    if (ret != OAL_SUCC || msg->rsp_len < sizeof(alg_rate_all_stats_stru)) {
        wifi_printf("hmac_query_alg_tx_rate_stats_detail ret[%u] msg->rsp_len[%u]\r\n", ret, msg->rsp_len);
        return;
    }
    if (value == ALG_REJISTER_AND_CLEAR_RATE_STATS) {
        wifi_printf("start alg rate stats!\r\n");
        return;
    } else if (value == ALG_UNREJISTER_RATE_STATS) {
        wifi_printf("end alg rate stats!\r\n");
        return;
    }

    wifi_printf("HE TX RATE:\r\n");
    wifi_printf("     MCS0    MCS1    MCS2    MCS3    MCS4    MCS5    MCS6    MCS7    MCS8    MCS9    \r\n");
    wifi_printf("succ:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rate_stats, succ));
    wifi_printf("fail:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rate_stats, fail));
    wifi_printf("rts_:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rate_stats, rts_fail));
    wifi_printf("send:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rate_stats, send_cnt));
    wifi_printf("rtry:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rate_stats, retry));
    wifi_printf("HT TX RATE:\r\n");
    wifi_printf("     MCS0    MCS1    MCS2    MCS3    MCS4    MCS5    MCS6    MCS7    \r\n");
    wifi_printf("succ:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rate_stats, succ));
    wifi_printf("fail:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rate_stats, fail));
    wifi_printf("rts_:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rate_stats, rts_fail));
    wifi_printf("send:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rate_stats, send_cnt));
    wifi_printf("rtry:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rate_stats, retry));
    wifi_printf("LEGACY TX RATE:\r\n");
    wifi_printf("succ:%-8u\r\n", stats->legacy_rate_stats[0].succ);
    wifi_printf("fail:%-8u\r\n", stats->legacy_rate_stats[0].fail);
    wifi_printf("rts_:%-8u\r\n", stats->legacy_rate_stats[0].rts_fail);
    wifi_printf("send:%-8u\r\n", stats->legacy_rate_stats[0].send_cnt);
    wifi_printf("rtry:%-8u\r\n", stats->legacy_rate_stats[0].retry);
}

/* 查询一段时间发包速率统计信息 */
osal_void hmac_query_alg_tx_rate_stats(osal_u8 value)
{
    osal_s32 ret;
    osal_u16 len;
    frw_msg msg = {0};

    msg.data = &value;
    msg.data_len = sizeof(osal_u8);
    len = (osal_u16)sizeof(alg_rate_all_stats_stru);
    msg.rsp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len, OAL_TRUE);
    if (msg.rsp == NULL) {
        return;
    }
    (osal_void)memset_s(msg.rsp, len, 0, len);
    msg.rsp_buf_len = (osal_u16)len;

    /* 抛事件至Device侧alg */
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_ALG_MULTI_PARAM, &msg, OSAL_TRUE);
    hmac_query_alg_tx_rate_stats_detail(&msg, ret, value);
    oal_mem_free(msg.rsp, OAL_TRUE);
}

osal_void hmac_query_alg_rx_rate_stats_detail(alg_rx_rate_all_stats_stru *stats)
{
    wifi_printf("\r\n");
    wifi_printf("HE RX RATE:\r\n");
    wifi_printf("     MCS0    MCS1    MCS2    MCS3    MCS4    MCS5    MCS6    MCS7    MCS8    MCS9    \r\n");
    wifi_printf("cnt_:"ALG_RATE_HE_PRINT_FORMAT"\r\n", alg_rate_he_print_data(stats->he_rx_rate_stats, cnt));
    wifi_printf("HT RX RATE:\r\n");
    wifi_printf("     MCS0    MCS1    MCS2    MCS3    MCS4    MCS5    MCS6    MCS7    \r\n");
    wifi_printf("cnt_:"ALG_RATE_HT_PRINT_FORMAT"\r\n", alg_rate_ht_print_data(stats->ht_rx_rate_stats, cnt));
    wifi_printf("LEGACY RX RATE:\r\n");
    wifi_printf("cnt_:%-8u\r\n", stats->legacy_rx_rate_stats[0].cnt);
}

/* 查询一段时间收包速率统计信息 */
osal_void hmac_query_alg_rx_rate_stats(osal_u8 value)
{
    alg_rx_rate_all_stats_stru stats = {0};
    if (value == ALG_REJISTER_AND_CLEAR_RATE_STATS) {
        alg_autorate_get_rx_rate_stats_clear();
        hmac_alg_register_rx_notify_func(HMAC_ALG_RX_AUTORATE, alg_autorate_rx_data_process);
    } else if (value == ALG_GET_RATE_STATS) {
        alg_autorate_get_rx_rate_stats(&stats);
        hmac_query_alg_rx_rate_stats_detail(&stats);
    } else if (value == ALG_UNREJISTER_RATE_STATS) {
        hmac_alg_unregister_rx_notify_func(HMAC_ALG_RX_AUTORATE);
    }
}
#endif

osal_u32 hmac_alg_config_init(osal_void)
{
    static const ccpriv_cmd_func_stru cmd_func_array[] = {
        {(const osal_s8 *)"alg_cfg", hmac_ccpriv_alg_cfg_etc},
        {(const osal_s8 *)"rts_threshold", hmac_ccpriv_alg_set_rts_threshold},
        {(const osal_s8 *)"set_udata_fix_rate", hmac_ccpriv_alg_set_udata_fix_rate},
        {(const osal_s8 *)"set_udata_rate_mode", hmac_ccpriv_alg_set_udata_rate_mode},
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
        {(const osal_s8 *)"alg_intrf_mode", hmac_ccpriv_alg_intrf_mode},
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        {(const osal_s8 *)"alg_intrf_mode_mixed", hmac_ccpriv_alg_intrf_mode_mixed},
#endif
        {(const osal_s8 *)"set_11b_tx_count", hmac_ccpriv_alg_set_11b_tx_count},
#endif
    };

    static const msg_hook_array_stru msg_hook_array[] = {
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
        {WLAN_MSG_W2H_CFG_ALG_INTRF_MODE_CFG, alg_intrf_mode_config_param},
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
        {WLAN_MSG_W2H_CFG_ALG_INTRF_MODE_DMAC_CFG, alg_intrf_mode_todmac_config_param},
#endif
#endif
        {WLAN_MSG_W2H_CFG_ALG_CFG, hmac_config_set_alg_debug_flag},
        {WLAN_MSG_W2H_CFG_ALG_PARAM, hmac_config_alg_param_etc},
        {WLAN_MSG_W2H_CFG_SET_RTS_THRESHOLD, hmac_config_rts_threshold},
        {WLAN_MSG_W2H_CFG_SET_TX_RATE, hmac_config_ar_set_tx_rate},
    };

    hmac_ccpriv_register_array(cmd_func_array, osal_array_size(cmd_func_array));
    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));

    return OAL_SUCC;
}

osal_void hmac_alg_config_deinit(osal_void)
{
    hmac_ccpriv_unregister((const osal_s8 *)"alg_cfg");
    hmac_ccpriv_unregister((const osal_s8 *)"rts_threshold");
    hmac_ccpriv_unregister((const osal_s8 *)"set_udata_fix_rate");
    hmac_ccpriv_unregister((const osal_s8 *)"set_udata_rate_mode");
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
    hmac_ccpriv_unregister((const osal_s8 *)"alg_intrf_mode");
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_ccpriv_unregister((const osal_s8 *)"alg_intrf_mode_mixed");
#endif
    hmac_ccpriv_unregister((const osal_s8 *)"set_11b_tx_count");

    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ALG_INTRF_MODE_CFG);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ALG_INTRF_MODE_DMAC_CFG);
#endif
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ALG_CFG);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ALG_PARAM);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_RTS_THRESHOLD);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_TX_RATE);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif