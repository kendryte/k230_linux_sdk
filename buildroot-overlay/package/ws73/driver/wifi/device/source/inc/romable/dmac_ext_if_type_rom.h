/*
 * Copyright (c) CompanyNameMagicTag. 2021-2024. All rights reserved.
 * Header file of device and host hmac_ext_if public interface.
 * Create: 2021-09-16
 */

#ifndef __DMAC_EXT_IF_TYPE_ROM_H__
#define __DMAC_EXT_IF_TYPE_ROM_H__

#include "osal_types.h"

#define DMAC_TX_QUEUE_AGGR_DEPTH     2

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* hmac to dmac定制化配置同步消息结构 */
typedef enum {
    CUSTOM_CFGID_NV_ID = 0,
    CUSTOM_CFGID_INI_ID,
    CUSTOM_CFGID_DTS_ID,
    CUSTOM_CFGID_PRIV_INI_ID,
    CUSTOM_CFGID_MSG,
    CUSTOM_CFGID_WOW,

    CUSTOM_CFGID_BUTT,
} custom_cfgid_enum;
typedef osal_u32 custom_cfgid_enum_uint32;
typedef enum {
    CUSTOM_CFGID_INI_ENDING_ID = 0,
    CUSTOM_CFGID_INI_PERF_ID = 1,
    /* 私有定制 */
    CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID = 2,
    CUSTOM_CFGID_PRIV_INI_SU_BFER_ID = 3,
    CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID = 4,
    CUSTOM_CFGID_PRIV_INI_MU_BFER_ID = 5,
    CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID = 6,
    CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID = 7,
    CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID = 8,
    CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID = 9,
    CUSTOM_CFGID_PRIV_INI_LDPC_ID = 10,
    CUSTOM_CFGID_PRIV_INI_BA_32BITMAP_ID = 11,
    CUSTOM_CFGID_PRIV_INI_MTID_AGGR_RX_ID = 12,
    CUSTOM_CFGID_PRIV_INI_CUSTOM_DATA_END = 13,
    CUSTOM_CFGID_PRIV_INI_EXT_COEX_EN_ID = 14,
    CUSTOM_CFGID_PRIV_INI_FRONT_SWITCH_ID = 15,
    CUSTOM_CFGID_INI_BUTT,
} custom_cfgid_h2d_ini_enum;

typedef struct {
    custom_cfgid_enum_uint32 syn_id; /* 同步配置ID */
    osal_u32 len;                      /* DATA payload长度 */
    osal_u8 msg_body[4];              /* DATA payload4字节 */
} hmac_to_dmac_cfg_custom_data_stru;

typedef struct {
    osal_u8 bus_h2d_sched_count;
    osal_u8 bus_d2h_sched_count;
} bus_cus_config_stru;
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

typedef enum {
    LINKLOSS_CNT_UNDER_8 = 0,                       /* linkloss cnt 0~7 */
    LINKLOSS_RATE_BETWEEN_8_AND_ONE_THIRD = 1,      /* linkloss cnt大于8但是小于门限的1/3(门限最低为25) */
    LINKLOSS_RATE_BETWEEN_ONE_THIRD_AND_A_HALF = 2, /* linkloss cnt位于门限的1/3到1/2 */
    LINKLOSS_RATE_MORE_THAN_A_HALF = 3,             /* linkloss cnt位于门限的1/2到1 */
    LINKLOSS_RATE_BUTT
} host_to_device_linkloss_rate_enum;
typedef osal_u8 dmac_linkloss_rate_uint8;

/* TXRX函数回调出参枚举 */
typedef enum {
    DMAC_TXRX_PASS = 0,     /* 继续往下 */
    DMAC_TXRX_DROP = 1,     /* 需要丢包 */
    DMAC_TXRX_SENT = 2,     /* 已被发送 */
    DMAC_TXRX_BUFF = 3,     /* 已被缓存 */

    DMAC_TXRX_BUTT
} dmac_txrx_output_type_enum;
typedef osal_u8 dmac_txrx_output_type_enum_uint8;

/* 算法的报文探测标志 (注:该枚举只允许使用3bit空间, 因此有效枚举值最大为7) */
typedef enum {
    DMAC_USER_ALG_NON_PROBE                     = 0,    /* 非探测报文 */
    DMAC_USER_ALG_TXBF_SOUNDING                 = 1,    /* TxBf sounding报文 */
    DMAC_USER_ALG_AUOTRATE_PROBE                = 2,    /* Autorate探测报文 */
    DMAC_USER_ALG_AGGR_PROBE                    = 3,    /* 聚合探测报文 */
    DMAC_USER_ALG_TPC_PROBE                     = 4,    /* TPC探测报文 */
    DMAC_USER_ALG_TX_MODE_PROBE                 = 5,    /* 发送模式探测报文(TxBf, STBC, Chain) */
    DMAC_USER_ALG_SMARTANT_NULLDATA_PROBE       = 6,    /* 智能天线NullData训练报文 */
    DMAC_USER_ALG_SMARTANT_DATA_PROBE           = 7,    /* 智能天线Data训练报文 */

    DMAC_USER_ALG_PROBE_BUTT
} dmac_user_alg_probe_enum;
typedef osal_u8 dmac_user_alg_probe_enum_uint8;

typedef enum {
    DMAC_TX_MODE_NORMAL  = 0,
    DMAC_TX_MODE_RIFS    = 1,
    DMAC_TX_MODE_AGGR    = 2,
    DMAC_TX_MODE_BUTT
} dmac_tx_mode_enum;
typedef osal_u8 dmac_tx_mode_enum_uint8;

typedef enum {
    DMAC_TID_PAUSE_RESUME_TYPE_BA   = 0,
    DMAC_TID_PAUSE_RESUME_TYPE_PS   = 1,
    DMAC_TID_PAUSE_RESUME_TYPE_BUTT
} dmac_tid_pause_type_enum;
typedef osal_u8 dmac_tid_pause_type_enum_uint8;

/* 专用于CB字段自定义帧类型、帧子类型枚举 */
typedef enum {
    WLAN_CB_FRAME_TYPE_START   = 0,    /* cb默认初始化为0 */
    WLAN_CB_FRAME_TYPE_ACTION  = 1,    /* action帧 */
    WLAN_CB_FRAME_TYPE_DATA    = 2,    /* 数据帧 */
    WLAN_CB_FRAME_TYPE_MGMT    = 3,    /* 管理帧，用于p2p等需要上报host */

    WLAN_CB_FRAME_TYPE_BUTT
} wlan_cb_frame_type_enum;
typedef osal_u8 wlan_cb_frame_type_enum_uint8;

/* 上报发送完成状态数据帧类型 */
typedef enum {
    WLAN_TX_EAPOL_KEY_PTK_4_4 = 1,
    WLAN_TX_AL_TX_PKT = 2
} wlan_tx_frame_id;

typedef struct {
    osal_u8 distance;
    osal_u8 resv[3];
} dmac_to_hmac_syn_distance_stru;

typedef struct {
    osal_u16      user_index;
    osal_u8       cur_bandwidth;
    osal_u8       cur_protocol;
} dmac_to_hmac_syn_info_event_stru;

typedef struct {
    osal_u8       voice_aggr;                /* 是否支持Voice聚合 */
    osal_u8       resv[3]; /* 保留3字节对齐 */
} dmac_to_hmac_voice_aggr_event_stru;

typedef enum {
    OAL_QUERY_STATION_INFO_EVENT      = 1,
    OAL_ATCMDSRV_DBB_NUM_INFO_EVENT   = 2,
    OAL_ATCMDSRV_FEM_PA_INFO_EVENT    = 3,
    OAL_ATCMDSRV_GET_RX_PKCG          = 4,
    OAL_ATCMDSRV_LTE_GPIO_CHECK       = 5,
    OAL_ATCMDSRV_GET_ANT              = 6,
    OAL_QUERY_EVNET_BUTT
} oal_query_event_id_enum;
typedef struct {
    osal_u32   event_para;       /* 消息传输的数据 */
    osal_u32   fail_num;
    osal_s16   always_rx_rssi;
    osal_u8    event_id;         /* 消息号 */
    osal_u8    reserved;
    osal_u32   data[11];
} dmac_atcmdsrv_atcmd_response_event;

typedef struct {
    osal_u32                  dscr_status;
    osal_u8                   frame_type;
    osal_u8                   mgmt_frame_id;
    osal_u16                  user_idx;
} dmac_tx_status_stru;

typedef struct {
    osal_u16 user_idx;
    osal_u8  resv[3]; /* 保留3字节对齐 */
} dmac_crx_ps_tx_status_stru;

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#define OAL_IP_ADDR_MAX_SIZE    OAL_IPV6_ADDR_SIZE
#endif

/* 函数执行控制枚举 */
typedef enum {
    DMAC_RX_FRAME_CTRL_GOON,        /* 本数据帧需要继续处理 */
    DMAC_RX_FRAME_CTRL_DROP,        /* 本数据帧需要丢弃 */
    DMAC_RX_FRAME_CTRL_BA_BUF,      /* 本数据帧被BA会话缓存 */

    DMAC_RX_FRAME_CTRL_BUTT
} dmac_rx_frame_ctrl_enum;
typedef osal_u8 dmac_rx_frame_ctrl_enum_uint8;

#ifdef _PRE_WLAN_RF_AUTOCALI
typedef enum {
    AUTOCALI_DATA,            /* 校准数据上报消息 */
    AUTOCALI_ACK,             /* 校准自动化同步消息 */
    AUTOCALI_SWITCH           /* 校准自动化开关消息 */
} dmac_sdt_autocali_msg_type;
#endif

typedef struct {
    osal_u8 tid_num;                            /* 需要发送的tid队列号 */
    dmac_tx_mode_enum_uint8 tx_mode;          /* 调度tid的发送模式 */
    osal_u8 mpdu_num[DMAC_TX_QUEUE_AGGR_DEPTH]; /* 调度得到的需要发送的mpdu个数 */
    osal_u16 user_idx;                          /* 要发送的tid队列隶属的user */
    oal_bool_enum_uint8 ba_is_jamed;            /* 当前BA窗是否卡死的标志位 */
    oal_bool_enum_uint8 is_jamed;               /* 本次调度BA窗是否卡死的标志位 */
} mac_tid_schedule_output_stru;

#ifdef _PRE_WLAN_FEATURE_CSI
typedef enum {
    CSI_MSG_H2D_CMD_INIT  = 1,  /* H2D消息命令 csi init */
    CSI_MSG_H2D_CMD_EXIT  = 2,  /* H2D消息命令 csi exit  */
    CSI_MSG_H2D_BUTT
} hal_csi_msg_h2d_cmd;

typedef struct {
    osal_u8 cmd_id;                         /* command id 1初始化 2退出 */
    osal_u16 csi_data_max_len;              /* 内存块最大长度，默认762，支持配置 */
    osal_u8 csi_data_blk_num;               /* 内存块数量，默认2，支持配置 */
}hal_csi_cfg_info;
#endif

#endif // __DMAC_EXT_IF_TYPE_ROM_H__
