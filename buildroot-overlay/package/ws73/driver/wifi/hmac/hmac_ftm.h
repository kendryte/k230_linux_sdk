/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac ftm head
 * Create: 2020-7-5
 */

#ifndef HMAC_FTM_H
#define HMAC_FTM_H

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"
#include "hmac_feature_main.h"
#include "mac_frame.h"
#include "hal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_FTM
#define MAC_FTM_TIMER_CNT                   4
#define MAX_FTM_RANGE_ENTRY_COUNT           15
#define MAX_FTM_ERROR_ENTRY_COUNT           11
#define MAX_MINIMUN_AP_COUNT                14
#define MAX_REPEATER_NUM                    3         /* 支持的最大定位ap数量 */

/* FTM命令使能标志位 */
#define FTM_INITIATOR                       BIT0
#define FTM_ENABLE                          BIT1
#define FTM_CALI                            BIT2
#define FTM_RESPONDER                       BIT3
#define FTM_SEND_FTM                        BIT4
#define FTM_SEND_IFTMR                      BIT5
#define FTM_SET_CORRECT_TIME                BIT6
#define FTM_SET_M2S                         BIT7
#define FTM_DBG_AUTO_BANDWIDTH              BIT8
#define FTM_RANGE                           BIT9
#define FTM_SEND_RANGE                      BIT10
#define FTM_ADD_TSF                         BIT11

#define PERIOD_OF_FTM_TIMER 12 // 80M:12500  160M:6250    /* 皮秒 当前为fpga版本*/

#define PHY_TX_ADJUTST_VAL 0 /* 发送phy至connector时间 */
#define PHY_RX_ADJUTST_VAL 0 /* 接收connector至phy时间 */

#define T1_FINE_ADJUST_VAL 0 /* 实际测量校准值 */
#define T2_FINE_ADJUST_VAL 0
#define T3_FINE_ADJUST_VAL 0
#define T4_FINE_ADJUST_VAL 0

#define FTM_FRAME_DIALOG_TOKEN_OFFSET 2 /* ftm帧dialog token偏移 */
#define FTM_FRAME_IE_OFFSET 3 /* ftm req帧ie偏移 */
#define FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET 3 /* ftm帧follow up dialog token偏移 */
#define FTM_FRAME_TOD_OFFSET 4 /* ftm帧tod偏移 */
#define FTM_FRAME_TOA_OFFSET 10 /* ftm帧toa偏移 */
#define FTM_FRAME_OPTIONAL_IE_OFFSET 20
#define FTM_FRAME_TSF_SYNC_INFO_OFFSET 3
#define FTM_RANGE_IE_OFFSET 3
#define FTM_REQ_TRIGGER_OFFSET 2 /* ftm req帧Trigger偏移 */

#define FTM_WAIT_TIMEOUT 1000 /* 等待接收FTM_1的时间 */
#define FTM_MAX_SESSION_TIMEOUT 4000 /* 会话有效时间 */

#define FTM_MIN_DETECTABLE_DISTANCE 0 /* FTM最小可检测范围 */
#define FTM_MAX_DETECTABLE_DISTANCE 8192000 /* FTM最大可检测范围 2km 换算单位为1/4096m */

#define FTM_FRAME_TOD_LENGTH 6
#define FTM_FRAME_TOA_LENGTH 6
#define FTM_FRAME_TOD_ERROR_LENGTH 2
#define FTM_FRAME_TOA_ERROR_LENGTH 2
#define FTM_TMIE_MASK 0xFFFFFFFFFFFF /* 48位 */

/* 默认ftm交互的参数 */
/* 一个会话ftm的帧 */
#define FTM_PER_BURST 2
/* FTM帧之间的间隙 */
#define FTM_MIN_DELTA 200 /* 单位100us，即20ms */
/* FTM 一个会话中ftm有效交互时间 */
#define FTM_BURST_DURATION 15 /* No preference */ // 11 /*128ms*/
/* FTM 一个会话时间 */
#define FTM_BURST_PERIOD 4 /* 单位100ms，即400ms */
/* FTM 时钟同步时，t1时间记录点(每个回合的第二帧) */
#define FTM_TIME_SYNC_FTM2 2
/* 最大FTM同时在线的会话 */
#define MAX_FTM_SESSION                     8
/* 默认FTM同时在线的会话 */
#define DEFAULT_FTM_SESSION                 4

/*****************************************************************************
    枚举定义
*****************************************************************************/
typedef enum {
    MAC_FTM_DISABLE_MODE = 0,
    MAC_FTM_RESPONDER_MODE = 1,
    MAC_FTM_INITIATOR_MODE = 2,
    MAC_FTM_MIX_MODE = 3,

    MAC_FTM_MODE_BUTT,
} mac_ftm_mode_enum;
typedef osal_u8 mac_ftm_mode_enum_uint8;

typedef enum {
    FTM_BW_20M = 0,
    FTM_BW_40M = 1,
    FTM_BW_80M = 2,
    FTM_BW_160M = 3,

    FTM_BW_BUTT,
} ftm_time_enum;

typedef enum {
    FTM_FORMAT_NO_PREFERENCE = 0,
    FTM_FORMAT_NO_HT_5M = 4,
    FTM_FORMAT_NO_HT_10M = 6,
    FTM_FORMAT_NO_HT_20M = 8,
    FTM_FORMAT_HT_MIXED_20M,
    FTM_FORMAT_VHT_20M,
    FTM_FORMAT_HT_MIXED_40M,
    FTM_FORMAT_VHT_40M,
    FTM_FORMAT_VHT_80M,
    FTM_FORMAT_VHT_80PLUS80M,
    FTM_FORMAT_VHT_160M_TWO_RF,
    FTM_FORMAT_VHT_160M_SINGLE_RF,
    FTM_FORMAT_DMG_2160 = 31
} ftm_format_and_bandwidth_enum;

typedef enum {
    NO_LOCATION    = 0,
    ROOT_AP        = 1,
    REPEATER       = 2,
    STA            = 3,
    LOCATION_TYPE_BUTT
} oal_location_type_enum;
typedef osal_u8 oal_location_type_enum_uint8;

typedef struct {
    osal_u64 t1;
    osal_u64 t2;
    osal_u64 t3;
    osal_u64 t4;

    osal_u8 dialog_token;
    osal_char rssi_dbm;
    osal_u8 rsv[2]; /* 保留2个字节用于对齐 */

    union {
        osal_u32 reg_value;
        struct {
            osal_u32 rpt_ftm_phase_incr : 13;
            osal_u32 reserved0 : 3;
            osal_u32 rpt_ftm_snr : 13;
            osal_u32 reserved1 : 3;
        } reg0;
    } ftm_rpt_reg0;

    union {
        osal_u32 reg_value;
        struct {
            osal_u32 rpt_ftm_max_power : 13;
            osal_u32 reserved0 : 3;
            osal_u32 rpt_ftm_max_pre_power : 13;
            osal_u32 reserved1 : 3;
        } reg1;
    } ftm_rpt_reg1;

    union {
        osal_u32 reg_value;
        struct {
            osal_u32 rpt_ftm_max_next_power : 13;
            osal_u32 reserved0 : 3;
            osal_u32 rpt_ftm_max_pre1_power : 13;
            osal_u32 reserved1 : 3;
        } reg2;
    } ftm_rpt_reg2;

    union {
        osal_u32 reg_value;
        struct {
            osal_u32 rpt_ftm_max_pre2_power : 13;
            osal_u32 reserved0 : 3;
            osal_u32 rpt_ftm_max_pre3_power : 13;
            osal_u32 reserved1 : 3;
        } reg3;
    } ftm_rpt_reg3;

    union {
        osal_u32 reg_value;
        struct {
            osal_u32 xwGROtRIOIz8OGwmKOGvqmw_ : 13;
            osal_u32 reserved0 : 3;
            osal_u32 x_uLwNLSwSGPwu_WxwuiTW__ : 13;
            osal_u32 reserved1 : 3;
        } reg4;
    } ftm_rpt_reg4;
} ftm_timer_stru;

typedef struct {
    osal_void *hmac_vap;
    osal_u8 session_id;
    osal_u8 auc_resv[3]; /* 保留3个字节用于对齐 */
} ftm_timeout_arg_stru;

typedef struct {
    /* 获取帧体内容 */
    osal_u16 burst_cnt;     /* 回合个数 */
    osal_u8 min_delta_ftm;  /* FTM帧之间的间隙 */
    osal_u8 burst_duration; /* FTM 一个会话中ftm有效交互时间 */

    osal_u64 time_syn_delta_t1;
    osal_u8 dialog_token;
    osal_u8 follow_up_dialog_token;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN]; /* FTM交互AP的BSSID */

    osal_u32 tsf_sync_info;

    osal_u16 partial_tsf_timer;
    osal_u16 burst_period; /* FTM 一个会话时间 */

    osal_u8 prot_format : 4; /* 指示 协议类型 */
    osal_u8 band_cap : 4;    /* 指示 带宽 */

    osal_u8 ftms_per_burst : 5; /* 每个回合FTM帧的个数 */
    osal_u8 asap : 1;           /* ?? as soon as posible */
    osal_u8 en_asap : 1;        /* 指示 as soon as posible 能力 */

    /* 命令控制 */
    osal_u8 ftm_initiator : 1;      /* STA that supports fine timing measurement as an initiator */
    osal_u8 lci_ie : 1;             /* 指示 携带LCI Measurement request/response elements */
    osal_u8 location_civic_ie : 1;  /* 指示 携带Location Civic Measurement request/response elements */
    osal_u8 ftms_per_burst_cmd : 5; /* 命令设置每个回合FTM帧的个数 */

    /* 其他 */
    osal_u8 iftmr : 1;            /* 将要发送的是第一帧ftm req */
    osal_u8 ftm_trigger : 1;      /* 将要发送的是后面ftm req */
    osal_u8 timer_syn : 1;        /* 是否启用同步功能 0:measure disstance 1: time syn */
    osal_u8 clock_calibrated : 1; /* 标记是否做过时钟校准 */
    osal_u8 time_syn_count : 5;   /* 时钟同步时，接收FTM帧个数 */
    osal_u32 range;               /* 记录本次测量的距离和误差 */

    frw_timeout_stru ftm_tsf_timer; /* ftm回合开始定时器 */
    frw_timeout_stru ftm_all_burst; /* ftm整个回合时器 */
    mac_channel_stru channel_ftm;   /* FTM交互所在信道 */

    ftm_timer_stru ftm_timer[MAC_FTM_TIMER_CNT]; /* 记录时间 */
    ftm_timeout_arg_stru arg;                    /* 定时器超时函数的入参 */
} hmac_ftm_initiator_stru;

typedef struct {
    /* 获取帧体内容 */
    osal_u16 burst_cnt;     /* 回合个数 */
    osal_u8 min_delta_ftm;  /* FTM帧之间的间隙 */
    osal_u8 burst_duration; /* FTM 一个会话中ftm有效交互时间 */

    osal_u8 mac_ra[WLAN_MAC_ADDR_LEN];
    osal_u8 dialog_token;
    osal_u8 follow_up_dialog_token;

    osal_u16 burst_period;    /* FTM 一个会话时间 */
    osal_u8 dialog_token_ack; /* 指示 发送完成中断对应的ftm帧 */
    osal_u8 prot_format : 4;  /* 指示 协议类型 */
    osal_u8 band_cap : 4;     /* 指示 带宽 */

    osal_u8 ftms_per_burst : 5; /* 每个回合FTM帧的个数 */
    osal_u8 asap : 1;           /* 指示 as soon as posible */
    osal_u8 received_iftmr : 1; /* 指示 是否接受过第一帧ftm req */

    /* 命令控制 */
    osal_u8 lci_ie : 1;

    osal_u8 location_civic_ie : 1;
    osal_u8 ftm_parameters : 1;
    osal_u8 ftm_synchronization_information : 1;
    osal_u8 ftm_responder : 1; /* STA that supports fine timing measurement as an responder */
    osal_u8 resv0 : 4;

    /* 其他 */
    osal_u8 ftms_per_burst_varied : 5; /* 每个回合FTM帧的个数，用于发送 */
    osal_u8 resv1 : 3;
    osal_u8 resv2;

    mac_channel_stru channel_ftm;   /* FTM交互所在信道 */
    frw_timeout_stru ftm_tsf_timer; /* ftm回合开始定时器 */
    frw_timeout_stru ftm_all_burst; /* ftm整个回合时器 */
    osal_u64 tod;                   /* 填写tod */
    osal_u64 toa;                   /* 填写toa */

    osal_u16 tod_error; /* 填写tod_error */
    osal_u16 toa_error; /* 填写toa_error */

    osal_u32 tsf;                                /* 填写tsf */
    ftm_timer_stru ftm_timer[MAC_FTM_TIMER_CNT]; /* 记录时间 */
    ftm_timeout_arg_stru arg;                    /* 定时器超时函数的入参 */
} hmac_ftm_responder_stru;

typedef struct {
    osal_u32 measurement_start_time;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];
    osal_u8 resv[2]; /* 保留2个字节用于对齐 */
    osal_u32 range : 24;
    osal_u32 max_range_error_exponent : 8;
} ftm_range_entry_stru;

typedef struct {
    osal_u32 measurement_start_time;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];
    osal_u8 error_code;
    osal_u8 resv[1]; /* 保留1个字节用于对齐 */
} ftm_error_entry_stru;

typedef struct {
    osal_u8 range_entry_count;
    osal_u8 error_entry_count;
    osal_u8 resv[2]; /* 保留2个字节用于对齐 */

    ftm_range_entry_stru ftm_range_entry[MAX_FTM_RANGE_ENTRY_COUNT];
    ftm_error_entry_stru ftm_error_entry[MAX_FTM_ERROR_ENTRY_COUNT];
} ftm_range_rpt_stru;

typedef struct {
    osal_u8 mac[WLAN_MAC_ADDR_LEN];
    osal_u8 dialog_token;
    osal_u8 meas_token;

    osal_u16 num_rpt;
    osal_u16 start_delay;

    osal_u8 bssid[MAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    osal_u8 channel[MAX_MINIMUN_AP_COUNT];
    osal_u8 minimum_ap_count;
    osal_u8 resv[1]; /* 保留1个字节用于对齐 */
} ftm_range_req_stru;

typedef struct {
    hmac_ftm_initiator_stru *ftm_init;
    hmac_ftm_responder_stru *ftm_rspder;
    oal_bool_enum_uint8 cali;
    osal_u8 cali_phy_freq;              /* 记录射频校准时，phy时钟频率 */
    osal_u8 cali_adc_freq;              /* 记录射频校准时，adc采样频率 */
    oal_bool_enum_uint8 auto_bandwidth; /* 使用协商模式和速率发帧，还是OFMD格式和6M速率发帧 */
    ftm_range_req_stru send_ftm_range;  /* 设置ftm range命令 */
    ftm_range_rpt_stru ftm_range_rpt;
    osal_s16 add_tsf; /* 认证预留 tsf时间 */
    osal_u8 initor_session_id;
    osal_u8 tx_chain_selection;
    osal_u8 session_num; /* 记录session申请的个数 */
    osal_u8 auc_resv[3]; /* 保留3个字节用于对齐 */

    osal_u32 ftm_cali_time;
} hmac_ftm_stru;

typedef struct {
    /* 测距 */
    osal_u64 ftm_rtt;  /* FTM帧传输时间 */
    osal_u64 distance; /* FTM传输距离 */
    /* 同步 */
    osal_u64 t1_delta;    /* 两个burst之间的时间 */
    osal_u64 time_offset; /* 两个设备的时钟偏差 */
    osal_s64 clock_time;  /* 两个设备的晶振偏差 */
    osal_u8 time_sync;    /* 时间同步标志位 */
    osal_u8 auc_resv[3];  /* 保留3个字节用于对齐 */
} hmac_ftm_data_report;
/* FTM调试开关相关的结构体 */
typedef struct {
    osal_u8               channel_num;
    osal_u8               burst_num;
    oal_bool_enum_uint8 measure_req;
    osal_u8               ftms_per_burst;

    oal_bool_enum_uint8 asap;
    osal_u8               resv[1];
    osal_u8               bssid[WLAN_MAC_ADDR_LEN];
} mac_send_iftmr_stru;

typedef struct {
    osal_u8             resv[2];
    osal_u8             mac[WLAN_MAC_ADDR_LEN];
} mac_send_ftm_stru;

typedef struct {
    oal_bool_enum_uint8 enable;
    osal_u8 session_num;
    osal_u8 reserve[2];  /* 预留2字节对齐 */
} mac_ftm_enable_stru;

typedef struct {
    osal_u8 tx_chain_selection;
    oal_bool_enum_uint8 is_mimo;
    osal_u8 reserve[2];  /* 预留2字节对齐 */
} ftm_m2s_stru;

typedef struct {
    osal_u8                           mac[WLAN_MAC_ADDR_LEN];
    osal_u8                           dialog_token;
    osal_u8                           meas_token;
    osal_u16                          num_rpt;
    osal_u16                          start_delay;
    osal_u8                           bssid[MAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    osal_u8                           channel[MAX_MINIMUN_AP_COUNT];
    osal_u8                           minimum_ap_count;
    osal_u8                           resv[1];
} mac_send_ftm_range_req_stru;

typedef struct {
    osal_u32 ftm_correct_time1;
    osal_u32 ftm_correct_time2;
    osal_u32 ftm_correct_time3;
    osal_u32 ftm_correct_time4;
} mac_set_ftm_time_stru;

typedef struct {
    osal_u32                        cmd_bit_map;
    oal_bool_enum_uint8             ftm_initiator;        /* ftm_initiator命令 */
    oal_bool_enum_uint8             cali;                 /* FTM 环回 */
    oal_bool_enum_uint8             auto_bandwidth;       /* FTM使能协商带宽发帧 */
    oal_bool_enum_uint8             ftm_resp;             /* ftm_resp命令 */
    oal_bool_enum_uint8             ftm_range;            /* ftm_range命令 */
    osal_u8                         resv[1];              /* 预留1字节对齐 */
    osal_s16                        add_tsf;              /* 认证预留 增加的tsf时间 */

    mac_ftm_enable_stru             ftm_enable;           /* 使能FTM */
    mac_send_ftm_stru               send_ftm;             /* 发送ftm命令 */
    mac_send_iftmr_stru             send_iftmr;           /* 发送iftmr命令 */
    mac_set_ftm_time_stru           ftm_time;             /* 设置ftm校准时间 */
    mac_send_ftm_range_req_stru     send_ftm_range;       /* 设置ftm range命令 */
    ftm_m2s_stru                    m2s;
} mac_ftm_debug_switch_stru;

/*****************************************************************************
    函数声明
*****************************************************************************/
osal_s32 hmac_config_ftm_dbg(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_config_d2h_ftm_info(const hmac_vap_stru *hmac_vap, const hmac_ftm_data_report *d2h_data);
osal_s32 hmac_process_ftm_ack_complete(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_process_send_ftm_complete(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_save_tx_ftm_complete_time(hmac_vap_stru *hmac_vap, osal_u8 session_id, osal_u64 ftm_t1,
    osal_u64 ftm_t4);
osal_u32 hmac_process_ftm_ack_complete_process(hmac_ftm_initiator_stru *ftm_init,
    hmac_vap_stru *hmac_vap, const hal_wlan_ftm_t2t3_rx_event_stru *ftm_t2t3, osal_u8 index);
osal_char hmac_ftm_interpolate(osal_u16 right, osal_u16 max, osal_u16 left);
osal_u32 hmac_ftm_get_distance(hmac_vap_stru *hmac_vap, osal_u64 *distance, osal_u8 session_id);
osal_u32 hmac_ftm_get_delta(hmac_vap_stru *hmac_vap,  osal_u8 session_id);
osal_u32 hmac_ftm_initor_rx_ftm(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf);
osal_void hmac_ftm_handle_freq(hmac_vap_stru *hmac_vap,
    osal_u32 *time_sync_enable, osal_u32 *phy_freq, osal_u32 *intp_freq);
osal_u32 hmac_check_tx_ftm_complete(hmac_vap_stru *hmac_vap, dmac_tx_ftm_frame_stru *ftm_info);
osal_u32 hmac_ftm_initor_send_ftm_req(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_u32 hmac_ftm_rspder_send_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_u32 hmac_ftm_initor_send_trigger(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_u32 hmac_ftm_initor_wait_start_burst_timeout(osal_void *arg);
osal_u16 hmac_encap_ftm_mgmt(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *buffer, osal_u8 session_id);
osal_void hmac_ftm_rspder_rx_ftm_req(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf);
osal_void hmac_tx_set_ftm_ctrl_dscr(hmac_vap_stru *hmac_vap, hal_tx_dscr_stru *tx_dscr, const oal_netbuf_stru *netbuf);
osal_void hmac_ftm_start_cali(hmac_vap_stru *hmac_vap, wlan_phy_protocol_enum prot_format,
    wlan_bw_cap_enum band_cap);
osal_u32 hmac_set_ftm_correct_time(mac_set_ftm_time_stru ftm_time);
osal_u32 hmac_ftm_rspder_set_parameter(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_void hmac_ftm_initor_start_bust(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_u32 hmac_vap_ftm_init(hmac_vap_stru *hmac_vap, osal_u8 session_num);
osal_void hmac_vap_ftm_deinit(hmac_vap_stru *hmac_vap);
osal_s8 hmac_ftm_find_session_index(hmac_vap_stru *hmac_vap, mac_ftm_mode_enum_uint8 ftm_mode,
    const osal_u8 peer_mac[WLAN_MAC_ADDR_LEN]);
osal_void hmac_ftm_enable_session_index(hmac_vap_stru *hmac_vap, mac_ftm_mode_enum_uint8 ftm_mode,
    const osal_u8 peer_mac[WLAN_MAC_ADDR_LEN], osal_u8 session_id);
oal_bool_enum_uint8 hmac_check_ftm_switch_channel(hmac_vap_stru *hmac_vap, osal_u8 chan_number);
osal_u32 hmac_ftm_rspder_send(hmac_vap_stru *hmac_vap, const osal_u8 mac[WLAN_MAC_ADDR_LEN],
    wlan_phy_protocol_enum prot_format, wlan_bw_cap_enum band_cap);
wlan_phy_protocol_enum_uint8 hmac_ftm_get_phy_mode(wlan_protocol_enum_uint8 protocol,
    wlan_channel_band_enum_uint8 band);
wlan_bw_cap_enum_uint8 hmac_ftm_get_band_cap(wlan_channel_bandwidth_enum_uint8 en_bandwidth);
osal_u32 hmac_ftm_initor_send(hmac_vap_stru *hmac_vap, mac_send_iftmr_stru *send_iftmr);
osal_void hmac_ftm_initor_wait_end_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_void hmac_ftm_initor_end_session(hmac_ftm_initiator_stru *ftm_init);
osal_void hmac_ftm_rspder_end_session(hmac_ftm_responder_stru *ftm_rspder);
osal_void hmac_ftm_get_cali(hmac_vap_stru *hmac_vap, osal_u32 phy_freq, osal_u32 intp_freq);
osal_void hmac_set_ftm_m2s(hmac_vap_stru *hmac_vap, ftm_m2s_stru m2s);

osal_u32 hmac_save_ftm_range(hmac_vap_stru *hmac_vap, osal_u8 session_id);
osal_u32 hmac_ftm_rrm_proc_rm_request(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_u32 hmac_save_error_ftm_session(hmac_vap_stru *hmac_vap, mac_ftm_parameters_ie_stru *mac_ftmp,
    osal_u8 session_id, osal_u8 *data);
osal_u32 hmac_send_ftm_range_req(hmac_vap_stru *hmac_vap, mac_send_ftm_range_req_stru *ftm_range);
osal_u32 hmac_send_ftm_req_adjust_param(hmac_vap_stru *hmac_vap, hmac_ftm_initiator_stru *ftm_init,
    osal_u8 session_id);

static INLINE__ mac_ftm_mode_enum_uint8 mac_check_ftm_enable(const hmac_vap_stru *hmac_vap)
{
    if ((mac_mib_get_fine_timing_msmt_init_activated(hmac_vap) == OSAL_FALSE) &&
        (mac_mib_get_fine_timing_msmt_resp_activated(hmac_vap) == OSAL_FALSE)) {
        return MAC_FTM_DISABLE_MODE;
    } else if ((mac_mib_get_fine_timing_msmt_init_activated(hmac_vap) == OSAL_FALSE) &&
        (mac_mib_get_fine_timing_msmt_resp_activated(hmac_vap) == OSAL_TRUE)) {
        return MAC_FTM_RESPONDER_MODE;
    } else if ((mac_mib_get_fine_timing_msmt_init_activated(hmac_vap) == OSAL_TRUE) &&
        (mac_mib_get_fine_timing_msmt_resp_activated(hmac_vap) == OSAL_FALSE)) {
        return MAC_FTM_INITIATOR_MODE;
    } else {
        return MAC_FTM_MIX_MODE;
    }
}
#endif /* _PRE_WLAN_FEATURE_FTM */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_ftm.h */
