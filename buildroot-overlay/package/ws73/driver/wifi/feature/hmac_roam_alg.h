/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_alg.h
 * 生成日期   : 2015年3月18日
 * 功能描述   : hmac_roam_alg.c 的头文件
 */


#ifndef __HMAC_ROAM_ALG_H__
#define __HMAC_ROAM_ALG_H__

#include "hmac_roam_connect.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_ALG_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ROAM_LIST_MAX                     (4)          /* 名单规格 */
#define ROAM_BLACKLIST_NORMAL_AP_TIME_OUT (30000)      /* 黑名单正常超时时间 */
#define ROAM_BLACKLIST_REJECT_AP_TIME_OUT (30000)      /* 黑名单拒绝模式超时时间 */
#define ROAM_BLACKLIST_COUNT_LIMIT        (1)           /* 黑名单超时次数限制 */

#define ROAM_HISTORY_BSS_TIME_OUT         (20000)       /* 历史优选名单超时时间 */
#define ROAM_HISTORY_COUNT_LIMIT          (1)           /* 历史优选超时次数限制 */
#define ROAM_RSSI_LEVEL                   (3)
#define ROAM_CONCURRENT_USER_NUMBER       (10)
#define ROAM_THROUGHPUT_THRESHOLD         (1000)

#define ROAM_RSSI_NE80_DB                 (-80)
#define ROAM_RSSI_NE75_DB                 (-75)
#define ROAM_RSSI_NE70_DB                 (-70)
#define ROAM_RSSI_NE68_DB                 (-68)

#define ROAM_RSSI_DIFF_4_DB               (4)
#define ROAM_RSSI_DIFF_6_DB               (6)
#define ROAM_RSSI_DIFF_8_DB               (8)
#define ROAM_RSSI_DIFF_10_DB              (10)

#define ROAM_RSSI_CMD_TYPE                (-128)
#define ROAM_RSSI_LINKLOSS_TYPE           (-121)
#define ROAM_RSSI_MAX_TYPE                (-126)

#define ROAM_TRIGGER_COUNT_THRESHOLD (5)
#define ROAM_TRIGGER_RSSI_NE80_DB (-80)
#define ROAM_TRIGGER_RSSI_NE75_DB (-75)
#define ROAM_TRIGGER_RSSI_NE70_DB (-70)
#define ROAM_TRIGGER_INTERVAL_10S (10 * 1000)
#define ROAM_TRIGGER_INTERVAL_15S (15 * 1000)
#define ROAM_TRIGGER_INTERVAL_20S (20 * 1000)
#define ROAM_WPA_CONNECT_INTERVAL_TIME (5 * 1000) /* 漫游与正常关联之间的时间间隔，WIFI+ 上层切换频繁 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 漫游黑名单类型 */
typedef enum {
    ROAM_BLACKLIST_TYPE_NORMAL_AP       = 0,
    ROAM_BLACKLIST_TYPE_REJECT_AP       = 1,
    ROAM_BLACKLIST_TYPE_BUTT
} roam_blacklist_type_enum;
typedef osal_u8  roam_blacklist_type_enum_uint8;

/* 漫游触发条件 */
typedef enum {
    ROAM_TRIGGER_DMAC                 = 0,
    ROAM_TRIGGER_APP                  = 1,
    ROAM_TRIGGER_COEX                 = 2,
    ROAM_TRIGGER_11V                  = 3,

    ROAM_TRIGGER_BUTT
} roam_trigger_condition_enum;
typedef osal_u8 roam_trigger_enum_uint8;

/* DMAC触发的漫游场景类型 */
typedef enum {
    ROAM_ENV_SPARSE_AP            = 0,  /* default AP roaming environment, based on [-78, -74] RSSI threshold */
    ROAM_ENV_LINKLOSS             = 1,
    ROAM_ENV_DENSE_AP             = 2,  /* dense AP roaming environment, based on [-72, -68] RSSI threshold */

    ROAM_ENV_BUTT
} roam_scenario_enum;
typedef osal_u8 roam_scenario_enum_uint8;

/* 漫游主状态机状态 */
typedef enum {
    ROAM_MAIN_STATE_INIT               = 0,
    ROAM_MAIN_STATE_FAIL               = ROAM_MAIN_STATE_INIT,
    ROAM_MAIN_STATE_SCANING            = 1,
    ROAM_MAIN_STATE_CONNECTING         = 2,
    ROAM_MAIN_STATE_UP                 = 3,

    ROAM_MAIN_STATE_BUTT
} roam_main_state_enum;
typedef osal_u8  roam_main_state_enum_uint8;

/* 漫游主状态机事件类型 */
typedef enum {
    ROAM_MAIN_FSM_EVENT_START          = 0,
    ROAM_MAIN_FSM_EVENT_SCAN_RESULT    = 1,
    ROAM_MAIN_FSM_EVENT_START_CONNECT  = 2,
    ROAM_MAIN_FSM_EVENT_CONNECT_FAIL   = 3,
    ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL = 4,
    ROAM_MAIN_FSM_EVENT_CONNECT_SUCC   = 5,
    ROAM_MAIN_FSM_EVENT_TIMEOUT        = 6,
    ROAM_MAIN_FSM_EVENT_TYPE_BUTT
} roam_main_fsm_event_type_enum;

/* 漫游扫描信道正交属性参数,命令行传入 */
typedef enum {
    ROAM_SCAN_CHANNEL_ORG_0       = 0, /* no scan */
    ROAM_SCAN_CHANNEL_ORG_1       = 1, /* scan only one channel */
    ROAM_SCAN_CHANNEL_ORG_3       = 2, /* 2.4G channel 1\6\11 */
    ROAM_SCAN_CHANNEL_ORG_4       = 3, /* 2.4G channel 1\5\7\11 */
    ROAM_SCAN_CHANNEL_ORG_BUTT
} roam_channel_org_enum;
typedef osal_u8  roam_channel_org_enum_uint8;

typedef enum {
    WPAS_CONNECT_STATE_INIT = 0,
    WPAS_CONNECT_STATE_START = 1,
    WPAS_CONNECT_STATE_ASSOCIATED = 2,
    WPAS_CONNECT_STATE_HANDSHAKED = 3,
    WPAS_CONNECT_STATE_IPADDR_OBTAINED = 4,
    WPAS_CONNECT_STATE_IPADDR_REMOVED = 5,
    WPAS_CONNECT_STATE_BUTT
} wpas_connect_state_enum;
typedef osal_u32 wpas_connect_state_enum_uint32;

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
/* 漫游配置结构体 */
typedef struct {
    osal_u8                       scan_band;                /* 扫描频段 */
    roam_channel_org_enum_uint8     scan_orthogonal;          /* 扫描信道正交属性 */
    osal_s8                        c_trigger_rssi_2g;           /* 2G时的触发门限 */
    osal_s8                        c_trigger_rssi_5g;           /* 5G时的触发门限 */
    osal_u8                       delta_rssi_2g;            /* 2G时的增益门限 */
    osal_u8                       delta_rssi_5g;            /* 5G时的增益门限 */
    osal_s8                        c_dense_env_trigger_rssi_2g; /* 密集AP环境中2G时的触发门限 */
    osal_s8                        c_dense_env_trigger_rssi_5g; /* 密集AP环境中5G时的触发门限 */
    oal_bool_enum_uint8             scenario_enable;          /* 是否使能场景识别 */
    osal_s8                        c_candidate_good_rssi;       /* 密集AP场景识别，强信号强度AP的门限 */
    osal_u8                       candidate_good_num;       /* 密集AP场景识别，强信号强度AP的数目 */
    osal_u8                       candidate_weak_num;       /* 密集AP场景识别，弱信号强度AP的数目 */
    osal_u32                      blacklist_expire_sec;     /* not used for now */
    osal_u32                      buffer_max;               /* not used for now */
} hmac_roam_config_stru;

/* 漫游统计结构体 */
typedef struct {
    osal_u32                 trigger_rssi_cnt;             /* rssi触发漫游扫描计数 */
    osal_u32                 trigger_linkloss_cnt;         /* linkloss触发漫游扫描计数 */
    osal_u32                 scan_cnt;                     /* 漫游扫描次数 */
    osal_u32                 scan_result_cnt;              /* 漫游扫描返回次数 */
    osal_u32                 connect_cnt;                  /* 漫游连接计数 */
    osal_u32                 roam_old_cnt;                 /* 漫游失败计数 */
    osal_u32                 roam_new_cnt;                 /* 漫游成功计数 */
    osal_u32                 roam_scan_fail;
    osal_u32                 roam_eap_fail;
} hmac_roam_static_stru;

typedef struct {
    osal_u32                 time_stamp;           /* 记录黑名单时间戳 */
    osal_u32                 timeout;              /* 记录黑名单超时时限 */
    osal_u16                 count_limit;          /* 记录黑名单超时前的加入次数限制 */
    osal_u16                 count;                /* 黑名单超时过程中，添加该Bssid的次数 */
    osal_u8                  bssid[WLAN_MAC_ADDR_LEN];
} hmac_roam_bss_info_stru;

/* 漫游blacklist结构体 */
typedef struct {
    hmac_roam_bss_info_stru     bss[ROAM_LIST_MAX];
} hmac_roam_bss_list_stru;

/* 漫游算法结构体 */
typedef struct {
    hmac_roam_bss_list_stru      blacklist;          /* 漫游黑名单AP历史记录 */
    hmac_roam_bss_list_stru      history;            /* 漫游优选AP历史记录 */
    osal_u32                   max_capacity;       /* 记录 scan 结果的最大 capacity */
    mac_bss_dscr_stru           *max_capacity_bss;  /* 记录 scan 结果的最大 capacity 的 bss */
    osal_s8                     c_current_rssi;        /* 当前 dmac 触发 rssi */
    osal_s8                     c_max_rssi;            /* 记录 scan 结果的最大 rssi */
    osal_u8                    another_bss_scaned; /* 是否扫描到了非当前关联的 bss */
    osal_u8                    invalid_scan_cnt;   /* 连续扫描到当前关联且首次关联的 bss 的次数 */
    osal_u8                    candidate_bss_num;        /* 相同SSID的BSS Num总数目，包括good, weak, and roaming */
    osal_u8                    candidate_good_rssi_num;  /* 相同SSID强信号强度的BSS Num */
    osal_u8                    candidate_weak_rssi_num;  /* 相同SSID弱信号强度的BSS Num */
    osal_u8                    scan_period;              /* 从高密场景向低密场景切换的扫描周期 */
    osal_u8                    better_rssi_scan_period;  /* 有更好信号强度AP的连续扫描次数 */
    osal_u8                    rsv[3];
    mac_bss_dscr_stru           *max_rssi_bss;      /* 记录 scan 结果的最大 rssi 的 bss */
} hmac_roam_alg_stru;

/* 漫游connect结构体 */
typedef struct {
    roam_connect_state_enum_uint8   state;
    mac_status_code_enum_uint16   status_code;
    osal_u8                       auth_num;
    osal_u8                       assoc_num;
    osal_u8                       ft_num;
    osal_u8                       ft_force_air;    /* 0:可以使用ds，1:ds失败，禁止使用ds */
    osal_u8                       ft_failed;       /* 0:ds漫游未失败，1:ds漫游失败 */
    frw_timeout_stru                timer;           /* 漫游connect使用的定时器 */
    mac_bss_dscr_stru              *bss_dscr;
} hmac_roam_connect_stru;
typedef struct {
    osal_s8                       c_rssi;
    osal_u8                       resv;
    osal_u16                      capacity_kbps;
} hmac_roam_rssi_capacity_stru;

/* 旧bss数据结构体 */
typedef struct {
    osal_u8                       bssid[WLAN_MAC_ADDR_LEN];
    osal_u16                      sta_aid;
    mac_cap_flag_stru               cap_flag;
    mac_channel_stru                st_channel;
    wlan_mib_ieee802dot11_stru      mib_info;
    mac_user_cap_info_stru          cap_info;
    mac_key_mgmt_stru               key_info;
    mac_user_tx_param_stru          user_tx_info;    /* TX相关参数 */
    mac_rate_stru                   op_rates;
    mac_user_ht_hdl_stru            ht_hdl;
    mac_vht_hdl_stru                vht_hdl;
    wlan_bw_cap_enum_uint8          bandwidth_cap;
    wlan_bw_cap_enum_uint8          avail_bandwidth;
    wlan_bw_cap_enum_uint8          cur_bandwidth;
    wlan_protocol_enum_uint8        protocol_mode;
    wlan_protocol_enum_uint8        avail_protocol_mode;
    wlan_protocol_enum_uint8        cur_protocol_mode;
    wlan_nss_enum_uint8             user_num_spatial_stream;
    wlan_nss_enum_uint8             avail_num_spatial_stream;
    wlan_nss_enum_uint8             cur_num_spatial_stream;
    wlan_nss_enum_uint8             avail_bf_num_spatial_stream;
    osal_u16                      us_cap_info;        /* 旧的bss的能力位信息 */
} hmac_roam_old_bss_stru;

typedef struct {
    osal_s8 trigger_2g;     /* 2G漫游触发门限   */
    osal_s8 trigger_5g;     /* 5G漫游触发门限   */
    osal_u8 ip_addr_obtained; /* IP地址是否已获取 */
    osal_u8 recv;             /* 保留2个字节用于对齐 */
    osal_u32 cnt;             /* 漫游触发器计数       */
    osal_u32 time_stamp;      /* 漫游触发器时间戳     */
    osal_u32 ip_obtain_stamp; /* 上层获取IP地址时间戳 */
} hmac_vap_roam_trigger_stru;

/* 漫游主结构体 */
typedef struct {
    osal_u8                         enable;          /* 漫游使能开关 */
    roam_trigger_enum_uint8         roam_trigger;    /* 漫游触发源 */
    roam_main_state_enum_uint8      main_state;      /* 漫游主状态 */
    roam_scenario_enum_uint8        rssi_type;       /* 何种rssi类型触发漫游 */
    osal_u8                         invalid_scan_cnt;    /* 无效扫描统计 */
    oal_bool_enum_uint8             current_bss_ignore;  /* 是否支持漫游回自己(支持驱动发起重关联) */
    osal_u8                         ip_addr_obtained; /* IP地址是否获取 */
    osal_u8                         linkloss_roam_done; /* 记录是否发生过漫游扫描 */
    hmac_vap_stru                  *hmac_vap;       /* 漫游对应的vap */
    hmac_user_stru                 *hmac_user;      /* 漫游对应的BSS user */
    hmac_roam_old_bss_stru          old_bss;         /* 漫游之前保存旧bss相关信息 */
    mac_scan_req_stru               scan_params;     /* 漫游扫描参数 */
    hmac_roam_config_stru           config;          /* 漫游相关配置信息 */
    hmac_roam_connect_stru          connect;         /* 漫游connect信息 */
    hmac_roam_alg_stru              alg;             /* 漫游算法信息 */
    hmac_roam_static_stru           st_static;          /* 漫游统计信息 */
    frw_timeout_stru                timer;           /* 漫游使用的定时器 */
    wpas_connect_state_enum_uint32  connected_state; /* 外部关联的状态进度 */
    hmac_vap_roam_trigger_stru      trigger;         /* 实际采用的漫游触发条件 */
} hmac_roam_info_stru;

/* roam hmac 同步 dmac数据结构体 */
typedef struct {
    osal_u16 sta_aid;
    osal_u16 pad;
    mac_channel_stru channel;
    mac_user_cap_info_stru cap_info;
    mac_key_mgmt_stru key_info;
    mac_user_tx_param_stru user_tx_info;
    osal_u32 back_to_old;
} mac_h2d_roam_sync_stru;

typedef osal_u32(*hmac_roam_fsm_func)(hmac_roam_info_stru *roam_info, osal_void *p_param);

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_roam_alg_add_blacklist_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid,
    roam_blacklist_type_enum_uint8 list_type);
osal_u32 hmac_roam_alg_add_history_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid);
osal_u32 hmac_roam_alg_bss_check_etc(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr);
osal_u32 hmac_roam_alg_scan_channel_init_etc(hmac_roam_info_stru *roam_info, mac_scan_req_stru *scan_params);
osal_void hmac_roam_alg_init_etc(hmac_roam_info_stru *roam_info, osal_s8 c_current_rssi);
mac_bss_dscr_stru *hmac_roam_alg_select_bss_etc(hmac_roam_info_stru *roam_info);
oal_bool_enum_uint8 hmac_roam_alg_find_in_blacklist_etc(hmac_roam_info_stru *roam_info, osal_u8 *bssid);
oal_bool_enum_uint8 hmac_roam_alg_need_to_stop_roam_trigger_etc(hmac_roam_info_stru *roam_info);
osal_u32 hmac_roam_alg_bss_in_ess_etc(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_roam_alg.h */
