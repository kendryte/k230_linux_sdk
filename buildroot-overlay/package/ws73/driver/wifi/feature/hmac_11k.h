/*
 * Copyright (c) CompanyNameMagicTag. 2017-2024. All rights reserved.
 * 文 件 名   : hmac_11k.h
 * 生成日期   : 2017年3月15日
 * 功能描述   : 11K
 */

#ifndef __HMAC_11K_H__
#define __HMAC_11K_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap_ext.h"
#include "hmac_user.h"
#include "hmac_vap.h"
#include "oal_schedule.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11K_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WAL_WIFI_FEATURE_SUPPORT_11K            0
#define MAC_RRM_BCN_REPORTING_DETAIL_LEN        1
#define MAC_RRM_BCN_EID_REPORTING_DATAIL        2
#define MAC_RRM_BCN_REQ_PASSIVE_SCAN_TIME       120
#define MAC_RRM_BCN_RPT_DETAIL_TRU              4
#define MAC_RRM_BCN_RPT_DETAIL_TRU_DFS          13
#define MAC_RRM_VAP_MEAS_STAUTS_TIME            (10 * 1000)
#define MAC_BCN_MEASURE_INTERVAL                100
#define MAC_11K_SUPPORT_AP_CHAN_RPT_NUM 8
#define MAC_MEASUREMENT_RPT_FIX_LEN 5
#define MAC_BEACON_RPT_FIX_LEN 26
#define MAC_MAX_RPT_DETAIL_LEN 224 /* 255 - 26(bcn fix) - 3(Meas rpt fix) - 2(subid 1) */

/* 255 - 26(bcn fix) - 5(Meas rpt fix + hdr) - 2(subid 1) -7(subid 2&164) */
#define MAC_MAX_RPT_MBO_DETAIL_LEN          215

#define MAC_ACTION_RPT_FIX_LEN 3
#define MAC_RADIO_MEAS_ACTION_REQ_FIX_LEN 5
#define MAC_NEIGHBOR_REPORT_ACTION_REQ_FIX_LEN 3
#define MAC_NEIGHBOR_RPT_FIX_LEN 15

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    RM_RADIO_MEASUREMENT_BASIC              = 0,
    RM_RADIO_MEAS_CHANNEL_LOAD              = 3,
    RM_RADIO_MEAS_BCN                       = 5,
    RM_RADIO_MEAS_FRM                       = 6,
    RM_RADIO_MEAS_STA_STATS                 = 7,
    RM_RADIO_MEASUREMENT_LCI                = 8,
    RM_RADIO_MEAS_TSC                       = 9,
    RM_RADIO_MEASUREMENT_LOCATION_CIVIC     = 11,
    RM_RADIO_MEASUREMENT_FTM_RANGE          = 16,
    RM_RADIO_MEAS_PAUSE                     = 255
} mac_radio_meas_type_enum;
typedef osal_u8 mac_radio_meas_type_enum_uint8;

typedef enum {
    PHY_TYPE_FHSS           = 1,
    PHY_TYPE_DSSS           = 2,
    PHY_TYPE_IR_BASEBAND    = 3,
    PHY_TYPE_OFDM           = 4,
    PHY_TYPE_HRDSSS         = 5,
    PHY_TYPE_BUTT
} phy_type_enum;
typedef osal_u8 phy_type_enum_uint8;

typedef enum {
    RM_BCN_REQ_MEAS_MODE_PASSIVE = 0,
    RM_BCN_REQ_MEAS_MODE_ACTIVE  = 1,
    RM_BCN_REQ_MEAS_MODE_TABLE   = 2,
    RM_BCN_REQ_MEAS_MODE_BUTT
} rm_bcn_req_meas_mode_enum;
typedef osal_u8 rm_bcn_req_meas_mode_enum_uint8;

/* Beacon Request */
/* Reporting Detail Values */
typedef enum {
    MAC_BCN_REPORTING_DETAIL_NO_FIXED_FIELD_OR_ELEM         = 0,
    MAC_BCN_REPORTING_DETAIL_FIXED_FILELD_AND_ANY_ELEM      = 1,
    MAC_BCN_REPORTING_DETAIL_FIXED_FIELD_AND_ELEM           = 2,
    MAC_BCN_REPORTING_DETAIL_BUTT
} mac_rrm_bcn_reporting_detail;
typedef osal_u8 mac_rrm_bcn_reporting_detail_uint8;

typedef enum {
    MAC_BCN_RPT_FRAG_NO_LAST_INDICATION = 0,
    MAC_BCN_RPT_FRAG_WITH_LAST_INDICATION = 1,
    MAC_BCN_RPT_NO_FRAG = 2
} mac_bcn_rpt_frag_type_enum;

typedef enum {
    HMAC_RRM_RPT_NOTIFY_HILINK      = 0,
    HMAC_RRM_RPT_NOTIFY_11V         = 1,
    HMAC_RRM_RPT_NOTIFY_CHN_LOAD    = 2,

    HMAC_RRM_RPT_NOTIFY_BUTT
} mac_rrm_rpt_notify_enum;
typedef osal_u8 mac_rrm_rpt_notify_enum_uint8;

typedef enum {
    MAC_RRM_STATE_INIT              = 0,   /* 初始状态 */
    MAC_RRM_STATE_WAIT_RSP          = 1,   /* 等待RSP */
    MAC_RRM_STATE_GET_RSP           = 2,   /* 收到RSP */
    MAC_RRM_STATE_RSP_TIMEOUT       = 3,   /* RSP超时 */
    MAC_RRM_STATE_BUTT
} mac_rrm_state_enum;
typedef osal_u8 mac_rrm_state_enum_uint8;

typedef enum {
    /* Radio Measurement Type */
    MAC_RRM_TYPE_CHANNEL_LOAD       = 0,
    MAC_RRM_TYPE_BCN                = 1,
    /* Neighbor Report Request */
    MAC_RRM_TYPE_NEIGHBOR_RPT       = 10,
    MAC_RRM_MEAS_TYPE_BUTT
} mac_rrm_type_enum;
typedef osal_u8 mac_rrm_type_enum_uint8;

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
#pragma pack(push, 1)
struct oal_rrm_enabled_cap_ie {
    osal_u8 link_cap : 1,          /* bit0: Link Measurement capability enabled */
        neighbor_rpt_cap : 1,      /* bit1: Neighbor Report capability enabled */
        parallel_cap : 1,          /* bit2: Parallel Measurements capability enabled */
        repeat_cap : 1,            /* bit3: Repeated Measurements capability enabled */
        bcn_passive_cap : 1,       /* bit4: Beacon Passive Measurements capability enabled */
        bcn_active_cap : 1,        /* bit5: Beacon Active Measurements capability enabled */
        bcn_table_cap : 1,         /* bit6: Beacon Table Measurements capability enabled */
        bcn_meas_rpt_cond_cap : 1; /* bit7: Beacon Measurement Reporting Conditions capability enabled */
    osal_u8 frame_cap : 1,         /* bit8: Frame Measurement capability enabled */
        chn_load_cap : 1,          /* bit9: Channel Load Measurement capability enabled */
        noise_histogram_cap : 1,   /* bit10: Noise Histogram Measurement capability enabled */
        stat_cap : 1,              /* bit11: Statistics Measurement capability enabled */
        lci_cap : 1,               /* bit12: LCI Measurement capability enabled */
        lci_azimuth_cap : 1,       /* bit13: LCI Azimuth capability enabled */
        tsc_cap : 1,               /* bit14: Transmit Stream/Category Measurement capability enabled */
        triggered_tsc_cap : 1;     /* bit15: Triggered  Transmit Stream/Category Measurement capability enabled */
    osal_u8 ap_chn_rpt_cap : 1,    /* bit16: AP Channel Report capability enabled */
        rm_mib_cap : 1,            /* bit17: RM MIB capability enabled */
        oper_chn_max_meas_duration : 3,     /* bit18-20: Operating Channel Max Measurement Duration */
        non_oper_chn_max_meas_duration : 3; /* bit21-23: Non-operating Channel Max Measurement Durationg */
    osal_u8 meas_pilot_cap : 3,             /* bit24-26: Measurement Pilot capability */
        meas_pilot_trans_info_cap : 1,   /* bit27: Measurement Pilot Transmission Information capability enabled */
        neighbor_rpt_tsf_offset_cap : 1, /* bit28: Neighbor Report TSF Offset capability enabled */
        rcpi_cap : 1,                    /* bit29: RCPI Measurement capability enabled */
        rsni_cap : 1,                    /* bit30: RSNI Measurement capability enabled */
        bss_avg_access_dly : 1;          /* bit31: BSS Average Access Delay capability enabled */
    osal_u8 avail_admission_capacity_cap : 1, /* bit32: BSS Available Admission Capacity capability enabled */
        antenna_cap : 1,                      /* bit33: Antenna capability enabled */
        ftm_range_report_cap : 1,             /* bit34: FTM range report capability enabled */
        bit_rsv : 5;                              /* bit35-39: Reserved */
};
typedef struct oal_rrm_enabled_cap_ie oal_rrm_enabled_cap_ie_stru;
typedef struct oal_rrm_enabled_cap_ie mac_rrm_enabled_cap_ie_stru;

typedef struct {
    osal_u8           action_code;
    osal_u8           dialog_token;
    osal_u8           rsv[1];
} mac_rrm_req_info_stru;

/* Radio Measurement Request */
typedef struct mac_action_rm_req {
    osal_u8   category;
    osal_u8   action_code;
    osal_u8   dialog_token;
    osal_u16  num_rpt;
    osal_u8   req_ies[1];
} mac_action_rm_req_stru;

/* Neighbor Report Request */
typedef struct mac_action_neighbor_req {
    osal_u8   category;
    osal_u8   action_code;
    osal_u8   dialog_token;
    osal_u8   subelm[1];
}mac_action_neighbor_req_stru;

/* Neighbor Report Elements */
typedef struct mac_bssid_info {
    osal_u32                  ap_reachability : 2,
                              security : 1,
                              key_scope : 1,
                              spec_management : 1,
                              qos : 1,
                              apsd : 1,
                              radio_meas : 1,
                              delayed_ba : 1,
                              immediate_ba : 1,
                              bit_rsv : 22;
} mac_bssid_info_stru;

typedef struct mac_neighbor_rpt_ie {
    osal_u8                   eid;         /* IEEE80211_ELEMID_MEASRPT */
    osal_u8                   len;
    osal_u8                   bssid[WLAN_MAC_ADDR_LEN];
    mac_bssid_info_stru         bssid_info;
    osal_u8                   optclass;
    osal_u8                   channum;
    osal_u8                   phy_type;
    osal_u8                   subelm[1];      /* varialbe len sub element fileds */
} mac_neighbor_rpt_ie_stru;

typedef struct mac_meas_sub_ie {
    osal_u8           sub_eid;         /* IEEE80211_ELEMID_MEASREQ */
    osal_u8           len;
    osal_u8           meas_sub_data[1];     /* varialbe len measurement requet */
} mac_meas_sub_ie_stru;

/* Beacon report request */
typedef struct mac_bcn_req {
    osal_u8                           optclass;
    osal_u8                           channum;         /*  请求测量的信道号 */
    osal_u16                          random_ivl;
    osal_u16                          duration;
    rm_bcn_req_meas_mode_enum_uint8     mode;
    osal_u8                           bssid[WLAN_MAC_ADDR_LEN];
    osal_u8                           subelm[];     /* varialbe len sub element fileds */
} mac_bcn_req_stru;

/* AP Channel Report */
typedef struct mac_ap_chn_rpt {
    osal_u8   eid;
    osal_u8   length;
    osal_u8   oper_class;
    osal_u8   chan[1];
} mac_ap_chn_rpt_stru;

typedef struct {
    osal_u8               ssid[WLAN_SSID_MAX_LEN];
    osal_u8               action_dialog_token;
    osal_u8               meas_token;
    osal_u8               oper_class;
    osal_u16              ssid_len;
    osal_u8               bssid[WLAN_MAC_ADDR_LEN];
    osal_u16              duration;
    osal_u8               auc_resv[2];
} mac_vap_rrm_trans_req_info_stru;

/* Beacon report */
typedef struct mac_bcn_rpt {
    osal_u8    optclass;
    osal_u8    channum;
    osal_u32   act_meas_start_time[2];
    osal_u16   duration;
    osal_u8    condensed_phy_type : 7,
               rpt_frm_type       : 1;
    osal_u8    rcpi;
    osal_u8    rsni;
    osal_u8    bssid[WLAN_MAC_ADDR_LEN];
    osal_u8    antenna_id;
    osal_u32   parent_tsf;
    osal_u8    subelm[];     /* varialbe len sub element fileds */
} mac_bcn_rpt_stru;

/* Channel Load request */
typedef struct mac_chn_load_req {
    osal_u8                           optclass;
    osal_u8                           channum;         /*  请求测量的信道号 */
    osal_u16                          random_ivl;
    osal_u16                          duration;
    osal_u8                           chn_load;
    osal_u8                           subelm[1];     /* varialbe len sub element fileds */
} mac_chn_load_req_stru;

/* Channel Load Report */
typedef struct mac_chn_load_rpt {
    osal_u8    optclass;
    osal_u8    channum;
    osal_u32   act_meas_start_time[2];
    osal_u16   duration;
    osal_u8    channel_load;
} mac_chn_load_rpt_stru;

/* Measurement Report Mode */
typedef struct mac_meas_rpt_mode {
    osal_u8   late        : 1,
              incapable   : 1,
              refused     : 1,
              rsvd        : 5;
} mac_meas_rpt_mode_stru;

/* Radio Measurement Report */
typedef struct mac_action_rm_rpt {
    osal_u8   category;
    osal_u8   action_code;
    osal_u8   dialog_token;
    osal_u8   rpt_ies[1];
} mac_action_rm_rpt_stru;

/* Measurement Report Elements */
typedef struct mac_meas_rpt_ie {
    osal_u8                   eid;         /* IEEE80211_ELEMID_MEASRPT */
    osal_u8                   len;
    osal_u8                   token;
    mac_meas_rpt_mode_stru      rptmode;
    osal_u8                   rpttype;
    osal_u8                   meas_rpt[1];     /* varialbe len measurement report */
} mac_meas_rpt_ie_stru;

/* Measurement Report Mode */
typedef struct mac_mr_mode {
    osal_u8   parallel    : 1,
              enable      : 1,
              request     : 1,
              rpt         : 1,
              duration_mandatory  : 1,
              rsvd        : 3;
} mac_mr_mode_stru;

/* Measurement Request Elements */
typedef struct mac_meas_req_ie {
    osal_u8           eid;         /* IEEE80211_ELEMID_MEASREQ */
    osal_u8           len;
    osal_u8           token;
    mac_mr_mode_stru    reqmode;
    osal_u8           reqtype;
    osal_u8           meas_req[1];     /* varialbe len measurement requet */
} mac_meas_req_ie_stru;
#pragma pack(pop)

typedef struct {
    osal_u8           dialog_token;
    osal_u8           meas_token;
    osal_u8           meas_type;
    osal_u8           opt_class;

    osal_u8           rpt_detail : 4;
    osal_u8           rpt_frag_type : 2;
    osal_u8           resv : 2;
    osal_u8           req_ie_num;
    mac_mr_mode_stru  mr_mode;
    osal_u8           ssid_len;

    osal_u16          meas_duration;
    osal_u16          repetition;

    osal_u8           *reqinfo_ieid;
    osal_u8           *ssid;

    osal_u8           bssid[WLAN_MAC_ADDR_LEN];
    osal_u8           rpt_condition;
    osal_u8           rpt_ref_val;
} mac_bcn_req_info_stru;

typedef struct {
    osal_u8           ssid_len;
    osal_u8           *ssid;
} mac_neighbor_req_info_stru;

typedef struct mac_user_rrm_info_tag {
    struct osal_list_head                  meas_rpt_list;          /* 接收Report链表 */
    frw_timeout_stru                     timer;                  /* 发送请求定时器 */

    osal_u8                            dialog_token;           /* 保存Request 的Token,与report对应 */
    osal_u8                            meas_token;             /* 保存Request 的Token,与report对应 */
    mac_rrm_state_enum                   meas_status;            /* 测量状态 */
    mac_rrm_rpt_notify_enum              rpt_notify_id;          /* 调用模块 */
    mac_rrm_type_enum                    reqtype;                /* 测量类型 */
    mac_meas_rpt_mode_stru               rptmode;                /* RPT模式  */
    osal_u8                            auc_resv1[2];
} mac_user_rrm_info_stru;

typedef struct mac_rrm_req_tag {
    mac_rrm_rpt_notify_enum              rpt_notify_id;          /* 发起模块类型 */
    mac_rrm_type_enum                    reqtype;                /* 测量类型 */
    osal_u16                           rpt_num;                /* 重复测量次数 */
    osal_u8                            req_mode;
    void                                 *p_arg;
    osal_u8                            auc_resv1[1];
} mac_rrm_req_cfg_stru;

typedef struct {
    /* Measurement Report field */
    osal_u8                   eid;         /* IEEE80211_ELEMID_MEASRPT */
    osal_u8                   len;
    osal_u8                   token;
    osal_u8                   late        : 1,
                              incapable   : 1,
                              refused     : 1,
                              rsvd        : 5;

    osal_u8                   rpttype;
    /* beacon report field */
    osal_u8                   optclass;
    osal_u8                   channum;
    osal_u8                   condensed_phy_type : 7,
                              rpt_frm_type       : 1;

    osal_u8                   rcpi;
    osal_u8                   rsni;
    osal_u8                   antenna_id;
    osal_u8                   rsv;

    osal_u8                   bssid[6];
    osal_u32                  act_meas_start_time[2];
    osal_u16                  duration;

    osal_u32                  parent_tsf;
} mac_meas_rpt_bcn_item_stru;

typedef struct {
    struct osal_list_head         dlist_head;
    mac_meas_rpt_bcn_item_stru *meas_rpt_bcn_item;      /* 包含bcn rpt的meas rpt */
    osal_u8                  *rpt_detail_data;
    osal_u32                  rpt_detail_act_len;      /* rpt detail data实际长度 */
    osal_u8                   bit_beacon_report_id; /* beacon report编号 */
    osal_u8                   bit_fragment_id_number : 7; /* beacon report分段编号 */
    osal_u8                   bit_more_frame_body_fragments : 1; /* 当前beacon report后续是否还有更多分段，[1:有|0:没有] */
    osal_u8                   resv[2];
} mac_meas_rpt_bcn_stru;

typedef struct mac_vap_rrm_info_tag {
    mac_action_rm_rpt_stru *rm_rpt_action; /* Report Frame Addr */
    mac_meas_rpt_ie_stru *meas_rpt_ie;     /* Measurement Report IE Addr */
    oal_netbuf_stru *rm_rpt_mgmt_buf;      /* Report Frame Addr for Transfer */

    osal_u8 action_code;
    osal_u8 dialog_token;
    oal_bool_enum_uint8 is_measuring;
    osal_u8 rsv1[1];

    osal_u16 req_user_id;
    osal_u16 rm_rpt_action_len; /* Report Frame Length for Transfer */
    struct osal_list_head        meas_rpt_list;     /* Report链表 */
    mac_meas_req_ie_stru *meas_req_ie;
    mac_bcn_req_info_stru bcn_req_info;           /* 保存发送的bcn请求 */
    mac_neighbor_req_info_stru neighbor_req_info; /* 保存发送的neighbor请求 */
    osal_u32 act_meas_start_time[2];

    frw_timeout_stru meas_status_timer;
} mac_vap_rrm_info_stru;

typedef struct {
    mac_vap_rrm_info_stru *rrm_info;
    osal_u8 enable_11k;
    osal_u8 bcn_table_switch;
    osal_u8 resv[2];
} hmac_11k_vap_info_stru;

typedef struct {
    mac_user_rrm_info_stru *user_rrm_info;
    mac_rrm_enabled_cap_ie_stru rrm_enabled_cap;
} hmac_11k_user_info_stru;

typedef struct {
    osal_u8 beacon_report_id;
    osal_u8 fragment_id_num;       /* 分片号 */
    osal_u16 frame_len_remaining;  /* 剩余帧体长度 */
    mac_meas_rpt_bcn_stru *meas_rpt_bcn; /* bcn report指针 */
} hmac_rrm_rpt_frag_stru;

typedef struct {
    /* Measurement Report field */
    osal_u8                   eid;         /* IEEE80211_ELEMID_MEASRPT */
    osal_u8                   len;
    osal_u8                   token;
    mac_meas_rpt_mode_stru      rptmode;
    osal_u8                   rpttype;
    /* chn_load report field */
    osal_u8                   optclass;
    osal_u8                   channum;
    osal_u32                  act_meas_start_time[2];
    osal_u16                  duration;
    osal_u8                   chn_load;
} mac_meas_rpt_chn_load_item_stru;

typedef struct {
    struct osal_list_head             dlist_head_chn_load;
    mac_meas_rpt_chn_load_item_stru *meas_rpt_chn_load_item;     /* 包含chn load rpt的meas rpt */
} mac_meas_rpt_chn_load_stru;

typedef struct {
    struct osal_list_head         dlist_head_neighbor;
    mac_neighbor_rpt_ie_stru    *meas_rpt_neighbor_item;    /* 包含neighbor rpt的meas rpt */
} mac_meas_rpt_neighbor_stru;

typedef struct {
    osal_u8 action_type;
    osal_u8 auc_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u16 num_rpt;
    osal_u16 random_ivl;
    osal_u16 duration;
    osal_u8 optclass;
    osal_u8 req_mode;
    osal_u8 means_type;
    osal_u8 channum;
    osal_u8 bcn_mode;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];
} mac_cfg_radio_meas_info_stru;

/* 通知链钩子指针定义 */
typedef osal_u32 (*p_rrm_rpt_notify_func)(hmac_user_stru *hmac_user, mac_rrm_state_enum rpt_state);
/* 钩子函数指针数组结构体 */
typedef struct {
    p_rrm_rpt_notify_func          pa_rrm_rpt_notify_func[HMAC_RRM_RPT_NOTIFY_BUTT][MAC_RRM_MEAS_TYPE_BUTT];
} hmac_rrm_rpt_hook_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
static osal_u32 hmac_11k_init_weakref(osal_void) __attribute__ ((weakref("hmac_11k_init"), used));
static osal_void hmac_11k_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_11k_deinit"), used));

typedef osal_u32 (*hmac_sta_up_update_rrm_capability_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u32 rx_len);
typedef osal_u32  (*hmac_ap_up_update_rrm_capability_cb)(hmac_user_stru *hmac_user, osal_u16 us_cap_info,
    osal_u8 *payload, osal_u32 msg_len);
typedef osal_void (*hmac_set_rrm_enabled_cap_field_etc_cb)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*hmac_11k_get_tsf_cb)(hmac_vap_stru *hmac_vap,
    mac_scanned_result_extend_info_stru *scan_result_ext_info, dmac_rx_ctl_stru *rx_ctrl);
typedef osal_void (*hmac_scan_update_bss_list_rrm_cb)(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len);
typedef osal_void (*hmac_11k_set_tsf_cb)(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    mac_scanned_result_extend_info_stru *scan_result_extend);
typedef osal_void (*hmac_get_11k_cap_cb)(hmac_vap_stru *hmac_vap, osal_s32 *pl_value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
