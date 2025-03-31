/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: HMAC to ALG的API接口文件。
 * Create: 2020-7-3
 */

#ifndef __HMAC_ALG_IF_STRUCT_DEFINE_H__
#define __HMAC_ALG_IF_STRUCT_DEFINE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "alg_common_rom.h"
#include "msg_alg_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 枚举定义
*****************************************************************************/
/* 算法ID，唯一识别某个算法，ID=2的整数值 */
typedef enum {
    HMAC_ALG_ID_SCHEDULE     = 0x00000001,
    HMAC_ALG_ID_AUTORATE     = 0x00000002,
    HMAC_ALG_ID_SMARTANT     = 0x00000004,
    HMAC_ALG_ID_ACS          = 0x00000008,
    HMAC_ALG_ID_TPC          = 0x00000010,
    HMAC_ALG_ID_TRAFFIC_CTL  = 0x00000020,
    HMAC_ALG_ID_TXBF         = 0x00000040,
    HMAC_ALG_ID_DBAC         = 0x00000080,
    HMAC_ALG_ID_ANTI_INTF    = 0x00000100,
    HMAC_ALG_ID_MWO_DET      = 0x00000200,
    HMAC_ALG_ID_EDCA_OPT     = 0x00000400,
    HMAC_ALG_ID_CCA_OPT      = 0x00000800,
    HMAC_ALG_ID_INTF_DET     = 0x00001000,
    HMAC_ALG_ID_RX_DSCR_CTL  = 0x00002000,
    HMAC_ALG_ID_TEMP_PROTECT = 0x00004000,
    HMAC_ALG_ID_BUTT
} hmac_alg_id_enum;
typedef osal_u32 hmac_alg_id_enum_uint32;

/* 数据结构类型枚举定义 */
typedef enum {
    HMAC_ALG_CHIP_STRU_ID_START,
    HMAC_ALG_CHIP_STRU_ID_BUTT
} hmac_alg_chip_stru_id_enum;
typedef osal_u8 hmac_alg_chip_stru_id_enum_uint8;

typedef enum {
    HMAC_ALG_USER_STRU_ID_SCHEDULE,
    HMAC_ALG_USER_STRU_ID_AUTORATE,
    HMAC_ALG_USER_STRU_ID_SMARTANT,
    HMAC_ALG_USER_STRU_ID_TXBF,
    HMAC_ALG_USER_STRU_ID_ANTI_INTF,
    HMAC_ALG_USER_STRU_ID_MWO_DET,
    HMAC_ALG_USER_STRU_ID_TPC_POW,      // 功率信息结构体
    HMAC_ALG_USER_STRU_ID_TPC_DIS,      // 距离信息结构体
    HMAC_ALG_USER_STRU_ID_TPC_RSSI,     // rssi信息结构体
    HMAC_ALG_USER_STRU_ID_CCA_OPT,
    HMAC_ALG_USER_STRU_ID_INTF_DET,
    HMAC_ALG_USER_STRU_ID_TRAFFIC_CTL,

    HMAC_ALG_USER_STRU_ID_BUTT
} hmac_alg_user_stru_id_enum;
typedef osal_u8 hmac_alg_user_stru_id_enum_uint8;

typedef enum {
    HMAC_ALG_TID_STRU_ID_SCHEDULE,
    HMAC_ALG_TID_STRU_ID_DBAC,

    HMAC_ALG_TID_STRU_ID_BUTT
} hmac_alg_tid_stru_id_enum;
typedef osal_u8 hmac_alg_tid_stru_id_enum_uint8;

typedef enum {
    HMAC_ALG_VAP_UP_START  = 0,
    HMAC_ALG_VAP_UP_DBAC   = HMAC_ALG_VAP_UP_START,
    HMAC_ALG_VAP_UP_TPC,

    HMAC_ALG_VAP_UP_BUTT
} hmac_alg_vap_up_notify_enum;
typedef osal_u8 hmac_alg_vap_up_notify_enum_uint8;

typedef enum {
    HMAC_ALG_VAP_DOWN_START  = 0,
    HMAC_ALG_VAP_DOWN_DBAC   = HMAC_ALG_VAP_DOWN_START,

    HMAC_ALG_VAP_DOWN_BUTT
} hmac_alg_vap_down_notify_enum;
typedef osal_u8 hmac_alg_vap_down_notify_enum_uint8;

/* 数据帧发送流程上获取发送参数的回调函数 */
typedef enum {
    HMAC_ALG_TX_START = 0,
    HMAC_ALG_TX_AUTORATE = HMAC_ALG_TX_START,
    HMAC_ALG_TX_TPC,
    HMAC_ALG_TX_TXBF,
    HMAC_ALG_TX_DBAC,
    HMAC_ALG_TX_EDCA_OPT,
    HMAC_ALG_TX_COMMON,
    HMAC_ALG_TX_CCA_OPT,

    HMAC_ALG_TX_NOTIFY_BUTT
} hmac_alg_tx_notify_enum;
typedef osal_u8 hmac_alg_tx_notify_enum_uint8;

/* 接收流程上的回调函数 */
typedef enum {
    HMAC_ALG_RX_START = 0,
    HMAC_ALG_RX_AUTORATE = HMAC_ALG_RX_START,
    HMAC_ALG_RX_CCA_OPT,
    HMAC_ALG_RX_DBAC,
    HMAC_ALG_RX_ACS,
    HMAC_ALG_RX_ANTI_INTF,
    HMAC_ALG_RX_TPC,
    HMAC_ALG_RX_TXBF,
    HMAC_ALG_RX_EDCA_OPT,

    HMAC_ALG_RX_NOTIFY_BUTT
} hmac_alg_rx_notify_enum;
typedef osal_u8 hmac_alg_rx_notify_enum_uint8;

/* 添加关联用户回调函数枚举定义 */
typedef enum {
    HMAC_ALG_ADD_USER_NOTIFY_SCHEDULE,
    HMAC_ALG_ADD_USER_NOTIFY_AUTROATE,
    HMAC_ALG_ADD_USER_NOTIFY_SMARTANT,
    HMAC_ALG_ADD_USER_NOTIFY_DBAC,
    HMAC_ALG_ADD_USER_NOTIFY_ACS,
    HMAC_ALG_ADD_USER_NOTIFY_ANTI_INTF,
    HMAC_ALG_ADD_USER_NOTIFY_CCA,
    HMAC_ALG_ADD_USER_NOTIFY_TXBF,
    HMAC_ALG_ADD_USER_NOTIFY_TPC,
    HMAC_ALG_ADD_USER_NOTIFY_MWO_DET,

    HMAC_ALG_ADD_USER_NOTIFY_INTF_DET,
    HMAC_ALG_ADD_USER_NOTIFY_TRAFFIC_CTL,
    HMAC_ALG_ADD_USER_NOTIFY_EDCA,

    HMAC_ALG_ADD_USER_NOTIFY_BUTT,
} hmac_alg_add_user_notify_enum;
typedef osal_u8 hmac_alg_add_user_notify_enum_uint8;

/* 删除关联用户回调函数枚举定义 */
typedef enum {
    HMAC_ALG_DEL_USER_NOTIFY_SCHEDULE,
    HMAC_ALG_DEL_USER_NOTIFY_AUTORATE,
    HMAC_ALG_DEL_USER_NOTIFY_SMARTANT,
    HMAC_ALG_DEL_USER_NOTIFY_DBAC,
    HMAC_ALG_DEL_USER_NOTIFY_ACS,
    HMAC_ALG_DEL_USER_NOTIFY_ANTI_INTF,
    HMAC_ALG_DEL_USER_NOTIFY_CCA,
    HMAC_ALG_DEL_USER_NOTIFY_TXBF,
    HMAC_ALG_DEL_USER_NOTIFY_TPC,
    HMAC_ALG_DEL_USER_NOTIFY_INTF_DET,
    HMAC_ALG_DEL_USER_NOTIFY_CCA_OPT,
    HMAC_ALG_DEL_USER_NOTIFY_TRAFFIC_CTL,

    HMAC_ALG_DEL_USER_NOTIFY_BUTT,
} hmac_alg_del_user_notify_enum;
typedef osal_u8 hmac_alg_del_user_notify_enum_uint8;

/* 设置信道回调函数枚举定义 */
typedef enum {
    HMAC_ALG_CFG_CHANNEL_NOTIFY_ACS,
    HMAC_ALG_CFG_CHANNEL_NOTIFY_TXBF,
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    HMAC_ALG_CFG_CHANNEL_NOTIFY_CCA_OPT,
#endif
    HMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT,
} hmac_alg_cfg_channel_notify_enum;
typedef osal_u8 hmac_alg_cfg_channel_notify_enum_uint8;

/* 参数配置钩子枚举定义 */
typedef enum {
    ALG_PARAM_CFG_START = 0,
    ALG_PARAM_CFG_SCHEDULE = ALG_PARAM_CFG_START,
    ALG_PARAM_CFG_ANTI_INTF,
    ALG_PARAM_CFG_INTF_DET,
    ALG_PARAM_CFG_EDCA,
    ALG_PARAM_CFG_CCA_OPT,
    ALG_PARAM_CFG_TEMP_PROTECT,
    ALG_PARAM_CFG_TPC,
    ALG_PARAM_CFG_AUTORATE,
    ALG_PARAM_CFG_AGGR,
    ALG_PARAM_CFG_RTS,
    ALG_PARAM_CFG_TRAFFIC_CTL,
    ALG_PARAM_CFG_DBAC,
    ALG_PARAM_CFG_GLA,
    ALG_PARAM_CFG_NOTIFY_BUTT
} alg_param_cfg_notify_enum;
typedef osal_u8 alg_param_cfg_notify_enum_uint8;

/* 设置BT状态通知枚举定义 */
typedef enum {
    HMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_AUTORATE,
    HMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_BUTT,
} hmac_alg_cfg_btcoex_state_notify_enum;
typedef osal_u8 hmac_alg_cfg_btcoex_state_notify_enum_uint8;

typedef enum {
    HMAC_ALG_BT_AGGR_TIME_OFF = 0,
    HMAC_ALG_BT_AGGR_TIME_4MS = 1,
    HMAC_ALG_BT_AGGR_TIME_2MS = 2,
    HMAC_ALG_BT_AGGR_TIME_1MS = 3,

    HMAC_ALG_BT_AGGR_TIME_BUTT
} hmac_alg_bt_aggr_time;
typedef osal_u8 hmac_alg_bt_aggr_time_uint8;

/* 接收管理帧的回调函数 */
typedef enum {
    HMAC_ALG_RX_MGMT_CCA_OPT,
    HMAC_ALG_RX_MGMT_TXBF,
    HMAC_ALG_RX_MGMT_ANTI_INTF,
    HMAC_ALG_RX_MGMT_EDCA_OPT,
    HMAC_ALG_RX_MGMT_TPC,

    HMAC_ALG_RX_MGMT_NOTIFY_BUTT
} hmac_alg_rx_mgmt_notify_enum;
typedef osal_u8 hmac_alg_rx_mgmt_notify_enum_uint8;
/* 接收控制帧的回调函数 */
typedef enum {
    HMAC_ALG_RX_CNTL_AUTORATE,

    HMAC_ALG_RX_CNTL_NOTIFY_BUTT
} hmac_alg_rx_cntl_notify_enum;
typedef osal_u8 hmac_alg_rx_cntl_notify_enum_uint8;

/*****************************************************************************
  3 宏定义
*****************************************************************************/
/* 控制面回调函数类型定义 */
typedef osal_u32 (*p_alg_cfg_channel_notify_func)(hmac_vap_stru *hmac_vap, mac_alg_channel_bw_chg_type_uint8 type);

typedef void (*p_alg_cfg_btcoex_state_notify_func)(hal_to_dmac_device_stru *hal_device,
    hmac_alg_bt_aggr_time_uint8 type);

typedef osal_u32 (*p_alg_add_assoc_user_notify_func)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_u32 (*p_alg_delete_assoc_user_notify_func)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

/* 收发流程相关回调 */
typedef osal_u32 (*p_alg_rx_notify_func)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf,
    hal_rx_statistic_stru *rx_stats);
typedef osal_u32 (*p_alg_vap_up_notify_func)(hmac_vap_stru *hmac_vap);
typedef osal_u32 (*p_alg_vap_down_notify_func)(hmac_vap_stru *hmac_vap);
typedef osal_u32 (*p_alg_tx_notify_func)(hmac_user_stru *hmac_user, mac_tx_ctl_stru *cb,
    hal_tx_txop_alg_stru *txop_param);

/* 调度相关回调 */
typedef osal_u32 (*p_alg_tx_schedule_func)(const hal_to_dmac_device_stru *device, osal_u8 ac_num,
    mac_tid_schedule_output_stru *sch_output);
typedef osal_u32 (*p_alg_tx_schedule_timer_func)(const hal_to_dmac_device_stru *device);
typedef osal_u32 (*p_alg_update_tid_notify_func)(hmac_tid_stru *pst_tid, osal_u8 in_mpdu_num);
typedef osal_u32 (*p_alg_update_user_info_notify_func)(const hal_to_dmac_device_stru *hal_device,
    hmac_user_stru *hmac_user, mac_tid_schedule_output_stru schedule_ouput, osal_u8 device_mpdu_full);
/* 事件回调 */
/* 接收管理帧回调 */
typedef osal_u32  (*p_alg_rx_mgmt_notify_func)(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf);
typedef osal_u32  (*p_alg_anti_intf_tbtt_notify_func)(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device);

/* 处理接收控制帧回调 */
typedef osal_u32  (*p_alg_rx_cntl_notify_func)(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const oal_netbuf_stru *buf);

/* 功率表刷新通知TPC算法回调 */
typedef osal_u32 (*p_alg_pow_table_refresh_notify_func)(hmac_vap_stru *hmac_vap);

/* cb更新通知TPC算法回调 */
typedef osal_void (*p_alg_update_cb_rssi_level_notify_func)(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *cb,
    hmac_user_stru *hmac_user);

typedef osal_u32 (*p_alg_para_sync_notify_func)(alg_param_sync_stru *para);
typedef osal_u32 (*p_alg_para_cfg_notify_func)(hmac_vap_stru *hmac_vap, frw_msg *msg);

typedef osal_u32 (*p_alg_distance_notify_func)(hal_to_dmac_device_stru *hal_device, osal_u8 distance);
typedef void (*p_alg_intf_det_pk_mode_notify_func)(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 is_pk_mode);
typedef osal_u32 (*p_alg_intf_det_edca_notify_func)(osal_void);
typedef osal_u32 (*p_alg_intf_det_cca_notify_func)(hal_to_dmac_device_stru *hal_device, osal_u8 old_intf_mode,
    osal_u8 cur_intf_mode);
typedef osal_void (*p_alg_intf_det_cca_chk_noise_notify_func)(osal_s8 avg_rssi_20,
    oal_bool_enum_uint8 coch_intf_state_sta, osal_u8 cur_intf);
typedef osal_u32 (*p_alg_scan_param_func)(const mac_scan_req_stru *scan_req_params);
typedef osal_void (*p_alg_intf_det_scan_chn_cb_func)(const osal_void *param);
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 挂在chip上的数据结构 */
typedef struct {
    void       *alg_info[HMAC_ALG_CHIP_STRU_ID_BUTT];
} hmac_alg_dev_stru;

/* 挂在各个user上的数据结构 */
typedef struct {
    void        *alg_info[HMAC_ALG_USER_STRU_ID_BUTT];
} hmac_alg_user_stru;

/* 挂在各个tid上的数据结构 */
typedef struct {
    void        *alg_info[HMAC_ALG_TID_STRU_ID_BUTT];
} hmac_alg_tid_stru;

typedef struct {
    osal_u16 head_mpdu_num;
    osal_u16 min_mpdu_len;
    osal_u16 max_mpdu_len;
    osal_u16 resv;    // 预留
} hmac_alg_mpdu_len_stru;

/* 算法主结构体,挂在DEVICE上 */
typedef struct {
    osal_u32 alg_bitmap; /* 算法位图，每位表示一个算法是否注册 */
    p_alg_tx_notify_func pa_tx_notify_func[HMAC_ALG_TX_NOTIFY_BUTT];
    p_alg_rx_notify_func pa_rx_notify_func[HMAC_ALG_RX_NOTIFY_BUTT];
    p_alg_update_tid_notify_func tid_update_func;
    p_alg_update_user_info_notify_func user_info_update_func;
    p_alg_tx_schedule_func tx_schedule_func;
    p_alg_tx_schedule_timer_func tx_schedule_timer_func;
    p_alg_add_assoc_user_notify_func pa_add_assoc_user_notify_func[HMAC_ALG_ADD_USER_NOTIFY_BUTT];
    p_alg_delete_assoc_user_notify_func pa_delete_assoc_user_notify_func[HMAC_ALG_DEL_USER_NOTIFY_BUTT];
    p_alg_cfg_channel_notify_func pa_cfg_channel_notify_func[HMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT];
    p_alg_cfg_btcoex_state_notify_func pa_cfg_btcoex_state_notify_func[HMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_BUTT];

    p_alg_rx_mgmt_notify_func rx_mgmt_func[HMAC_ALG_RX_MGMT_NOTIFY_BUTT];
    p_alg_rx_cntl_notify_func rx_cntl_func[HMAC_ALG_RX_CNTL_NOTIFY_BUTT];
    p_alg_vap_up_notify_func pa_alg_vap_up_notify_func[HMAC_ALG_VAP_UP_BUTT];
    p_alg_vap_down_notify_func pa_alg_vap_down_notify_func[HMAC_ALG_VAP_DOWN_BUTT];
    p_alg_anti_intf_tbtt_notify_func anti_intf_tbtt_func;

    p_alg_scan_param_func scan_param_func;
    p_alg_intf_det_scan_chn_cb_func scan_comp_cb_func;
    p_alg_bfee_report_pow_adjust_notify bfee_report_pow_adjust_notify;
    p_alg_pow_table_refresh_notify_func alg_pow_table_refresh_notify_func;
    p_alg_update_cb_rssi_level_notify_func alg_update_cb_rssi_level_notify_func;

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    p_alg_anti_intf_switch_func anti_intf_switch_func;
    p_alg_anti_intf_tx_time_notify_func anti_intf_tx_time_notify_func;
#endif
    p_alg_para_sync_notify_func alg_para_sync_notify_func[ALG_PARAM_SYNC_NOTIFY_BUTT];
    p_alg_para_cfg_notify_func alg_para_cfg_notify_func[ALG_PARAM_CFG_NOTIFY_BUTT];
} hmac_alg_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_alg_if_strutc_define.h */
