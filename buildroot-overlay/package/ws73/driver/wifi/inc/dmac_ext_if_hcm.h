/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of the DMAC external public interface.
 * Create: 2020-7-9
 */

#ifndef __DMAC_EXT_IF_HCM_H__
#define __DMAC_EXT_IF_HCM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oal_mm.h"
#include "oal_netbuf_ext.h"
#include "frw_ext_if.h"
#include "wlan_types_common.h"
#include "mac_frame.h"
#include "mac_device_ext.h"
#include "mac_user_ext.h"
#include "mac_vap_ext.h"
#include "mac_data.h"
#include "dmac_ext_if_type.h"
#include "dmac_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_INVALID_RX_BA_LUT_INDEX           HAL_MAX_RX_BA_LUT_SIZE

/* 安全加密 :  bss_info 中记录AP 能力标识， WPA or WPA2 */
#define HMAC_WPA_802_11I    BIT0
#define HMAC_RSNA_802_11I   BIT1

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* 抛往dmac侧消息头的长度 */
#define CUSTOM_MSG_DATA_HDR_LEN      (sizeof(custom_cfgid_enum_uint32) + sizeof(osal_u32))
#endif /* _PRE_PLAT_FEATURE_CUSTOMIZE */

/* DMAC CB中用于不同算法对帧进行标记 */
#define HMAC_CB_ALG_TAGS_MUCTRL_MASK        0x1         /* CB中用于多用户流控算法对帧进行标记 */
#define HMAC_CB_ALG_TAGS_TIDSCH_MASK        0x2         /* CB中用于调度算法对队列调度的帧标记 */

#define HMAC_BA_SEQNO_MASK                  0x0FFF      /* max sequece number */
#define HMAC_BA_MAX_SEQNO_BY_TWO            2048
#define HMAC_BA_RX_ALLOW_MIN_SEQNO_BY_TWO   64
#define HMAC_BA_RX_ALLOW_MAX_SEQNO_BY_TWO   4032

#define HMAC_BA_GREATER_THAN_SEQHI          1
#define HMAC_BA_BETWEEN_SEQLO_SEQHI         2
#define HMAC_BA_AMSDU_BACK_SUPPORTED_FLAG   1           /* BA会话对AMSDU的支持标识 */

#define HMAC_BA_DELBA_TIMEOUT               0
#define HMAC_BATX_WIN_STALL_THRESHOLD       6

#define MAC_TX_CTL_SIZE                     OAL_NETBUF_CB_SIZE()

/* DMAC TID中TCP ACK比例 */
#define HMAC_TID_TCK_ACK_PROPORTION_MAX     32
#define HMAC_TID_TCK_ACK_PROPORTION_MIN     1
#define HMAC_TID_TCK_ACK_PROPORTION_THRES   28

#define hmac_ba_seq_add(_seq1, _seq2)       ((osal_u32)((_seq1) + (_seq2)) & HMAC_BA_SEQNO_MASK)
#define hmac_ba_seq_sub(_seq1, _seq2)       ((osal_u32)((_seq1) - (_seq2)) & HMAC_BA_SEQNO_MASK)

#define HMAC_BA_BMP_SIZE                    64

#define HMAC_INVALID_SIGNAL_DELTA      (30)
#define HMAC_RSSI_SIGNAL_MIN           (-103)    /* 信号强度极小值 */
#define HMAC_RSSI_SIGNAL_MAX           (5)       /* 信号强度极大值 */
#define HMAC_INVALID_SIGNAL_INITIAL    (100)     /* 非法初始信号极大值 */

/* 发送BA窗口记录seq number的最大个数，必须是2的整数次幂 */
#define HMAC_TID_MAX_BUFS       128
/* 发送BA窗口记录seq number的bitmap所使用的类型长度 */
#define DMAC_TX_BUF_BITMAP_WORD_SIZE        32
/* 发送BA窗口记录seq number的bit map的长度 */
#define HMAC_TX_BUF_BITMAP_WORDS \
    ((HMAC_TID_MAX_BUFS + DMAC_TX_BUF_BITMAP_WORD_SIZE - 1) / DMAC_TX_BUF_BITMAP_WORD_SIZE)

#define HMAC_TX_MAX_RISF_NUM    6
#define DMAX_TX_QUEUE_SINGLE_DEPTH   2
#define HMAC_TX_QEUEU_MAX_PPDU_NUM   2
#define HMAC_TX_QUEUE_FAIL_CHECK_NUM    1000

#define HMAC_SCAN_MAX_AP_NUM_TO_GNSS            32
#define GNSS_DMAC_SCAN_RESULTS_VALID_MS         5000
#define HMAC_TX_ONE_MPDU_AMSDU_NUM_MAX 4

#define THRUPUT_BYPASS_LEN  1300    /* WIFI侧hw bypass发送长度:仅bypass iperf跑流长包 */

/* 不区分offload架构的CB字段 */
#define mac_get_cb_is_4address(_pst_tx_ctrl)             ((_pst_tx_ctrl)->use_4_addr)
#define mac_get_cb_is_amsdu(_pst_tx_ctrl)                ((_pst_tx_ctrl)->is_amsdu)
#define mac_get_cb_is_large_skb_amsdu(_pst_tx_ctrl)      ((_pst_tx_ctrl)->is_large_skb_amsdu)
#define mac_get_cb_has_ether_head(_pst_tx_ctrl)          ((_pst_tx_ctrl)->ether_head_including)
#define mac_get_cb_is_first_msdu(_pst_tx_ctrl)           ((_pst_tx_ctrl)->is_first_msdu)
#define mac_get_cb_is_need_resp(_pst_tx_ctrl)            ((_pst_tx_ctrl)->need_rsp)
#define mac_get_cb_is_eapol_key_ptk(_pst_tx_ctrl)        ((_pst_tx_ctrl)->is_eapol_key_ptk)
#define mac_get_cb_is_roam_data(_pst_tx_ctrl)            ((_pst_tx_ctrl)->roam_data)
#define mac_get_cb_is_from_ps_queue(_pst_tx_ctrl)        ((_pst_tx_ctrl)->is_get_from_ps_queue)
#define mac_get_cb_is_mcast(_pst_tx_ctrl)                ((_pst_tx_ctrl)->ismcast)
#define mac_get_cb_is_needretry(_pst_tx_ctrl)            ((_pst_tx_ctrl)->is_needretry)
#define mac_get_cb_get_probe_data_type(_pst_tx_ctrl)     ((_pst_tx_ctrl)->probe_data_type)
#define mac_get_cb_alg_tags(_pst_tx_ctrl)                ((_pst_tx_ctrl)->alg_frame_tag)

#define mac_get_cb_mgmt_frame_id(_pst_tx_ctrl)           ((_pst_tx_ctrl)->mgmt_frame_id)
#define mac_get_cb_mpdu_len(_pst_tx_ctrl)                ((_pst_tx_ctrl)->mpdu_payload_len)
#define mac_get_cb_frame_type(_pst_tx_ctrl)              ((_pst_tx_ctrl)->frame_type)
#define mac_get_cb_frame_subtype(_pst_tx_ctrl)           ((_pst_tx_ctrl)->frame_subtype)

/* VIP数据帧 */
#define mac_get_cb_is_vipframe(_pst_tx_ctrl) \
    ((WLAN_CB_FRAME_TYPE_DATA == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
    (MAC_DATA_VIP_FRAME >= mac_get_cb_frame_subtype(_pst_tx_ctrl)))

#define mac_get_cb_is_smps_frame(_pst_tx_ctrl) \
    ((WLAN_CB_FRAME_TYPE_ACTION == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
    (WLAN_ACTION_SMPS_FRAME_SUBTYPE == mac_get_cb_frame_subtype(_pst_tx_ctrl)))
#define mac_get_cb_is_opmode_frame(_pst_tx_ctrl) \
    ((WLAN_CB_FRAME_TYPE_ACTION == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
    (WLAN_ACTION_OPMODE_FRAME_SUBTYPE == mac_get_cb_frame_subtype(_pst_tx_ctrl)))

/* 模块发送流程控制信息结构体的信息元素获取 */
#define mac_get_cb_mpdu_num(_pst_tx_ctrl)                ((_pst_tx_ctrl)->mpdu_num)
#define mac_get_cb_netbuf_num(_pst_tx_ctrl)              ((_pst_tx_ctrl)->netbuf_num)
#define mac_get_cb_frame_header_length(_pst_tx_ctrl)     ((_pst_tx_ctrl)->frame_header_length)
#define mac_get_cb_ack_polacy(_pst_tx_ctrl)              ((_pst_tx_ctrl)->ack_policy)
#define mac_get_cb_tx_vap_index(_pst_tx_ctrl)            ((_pst_tx_ctrl)->tx_vap_index)
#define mac_get_cb_tx_user_idx(_pst_tx_ctrl)             ((_pst_tx_ctrl)->tx_user_idx)
#define mac_get_cb_wme_ac_type(_pst_tx_ctrl)             ((_pst_tx_ctrl)->ac)
#define mac_get_cb_wme_tid_type(_pst_tx_ctrl)            ((_pst_tx_ctrl)->tid)
#define mac_get_cb_retried_num(_pst_tx_ctrl)             ((_pst_tx_ctrl)->retried_num)
#define mac_get_cb_alg_pktno(_pst_tx_ctrl)               ((_pst_tx_ctrl)->alg_pktno)
#define mac_get_cb_tcp_ack(_pst_tx_ctrl)                 ((_pst_tx_ctrl)->is_tcp_ack)
#define mac_get_cb_msdu_num(_pst_tx_ctrl)                ((_pst_tx_ctrl)->msdu_num)
#define mac_get_cb_pk_mode(_pst_tx_ctrl)                 ((_pst_tx_ctrl)->resrv_6)

#define mac_get_cb_is_data_frame(_pst_tx_ctrl) \
    ((WLAN_DATA_BASICTYPE == mac_get_cb_wlan_frame_type(_pst_tx_ctrl)) && \
    ((WLAN_DATA == mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl)) || \
    (WLAN_QOS_DATA == mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl))))

#define mac_get_cb_wlan_frame_type(_pst_tx_ctrl)                ((_pst_tx_ctrl)->expand_cb.frame_type)
#define mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl) \
    (((_pst_tx_ctrl)->frame_header)->frame_control.sub_type)
#define mac_get_cb_seq_num(_pst_tx_ctrl)                        ((_pst_tx_ctrl)->expand_cb.seqnum)
#define mac_get_cb_frame_header_addr(_pst_tx_ctrl)              ((_pst_tx_ctrl)->expand_cb.frame_header)
#define mac_set_cb_is_qos_data(_pst_tx_ctrl, _flag)
#define mac_get_cb_is_qos_data(_pst_tx_ctrl)             OAL_FALSE

#define mac_get_rx_cb_frame_len(_pst_rx_ctl)                    ((_pst_rx_ctl)->frame_len)
#define mac_get_rx_cb_mac_header_len(_pst_rx_ctl)               ((_pst_rx_ctl)->mac_header_len)
#define mac_get_rx_cb_mac_vap_id(_pst_rx_ctl)                   ((_pst_rx_ctl)->mac_vap_id)
#define mac_get_rx_cb_hal_vap_idx(_pst_rx_ctl)                  ((_pst_rx_ctl)->vap_id)

/* DMAC模块接收流程控制信息结构体的信息元素获取 */
#define mac_get_rx_cb_ta_user_idx(_pst_rx_ctl)                  ((_pst_rx_ctl)->ta_user_idx)
#define mac_get_rx_cb_payload_len(_pst_rx_ctl) \
    ((_pst_rx_ctl)->frame_len - (_pst_rx_ctl)->mac_header_len)
#define mac_get_rx_cb_mac_header_addr(_pst_rx_ctl)      ((osal_u8 *)(mac_get_rx_cb_mac_hdr(_pst_rx_ctl)))
#define mac_get_rx_payload_addr(_pst_rx_ctl, _pst_buf) \
            ((osal_u8 *)(mac_get_rx_cb_mac_hdr(_pst_rx_ctl)) + mac_get_rx_cb_mac_header_len(_pst_rx_ctl))
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  枚举名  : hmac_wlan_ctx_event_sub_type_enum_uint8
  协议表格:
  枚举说明: WLAN CTX事件子类型定义
*****************************************************************************/
typedef enum {
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT = 0, /* 管理帧处理 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER,

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC,  /* 收到wlan的Delba和addba rsp用于到dmac的同步 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_PRIV_REQ, /* 11N自定义的请求的事件类型 */

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,       /* 扫描请求 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ, /* PNO调度扫描请求 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM,      /* 收到认证请求 关联请求，复位用户的节能状态 */

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_CONN_RESULT, /* 关联结果 */

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_ASOC_WRITE_REG, /* AP侧处理关联时，修改SEQNUM_DUPDET_CTRL寄存器 */

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG,       /* STA收到beacon和assoc rsp时，更新EDCA寄存器 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_DEFAULT_VO_REG, /* 如果AP不是WMM的，则STA会去使能EDCA寄存器，并设置VO寄存器 */

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN, /* 切换至新信道事件 */
    HMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN,        /* 设置信道事件 */
    HMAC_WALN_CTX_EVENT_SUB_TYPR_DISABLE_TX,         /* 禁止硬件发送 */
    HMAC_WALN_CTX_EVENT_SUB_TYPR_ENABLE_TX,          /* 恢复硬件发送 */
#ifdef _PRE_WLAN_FEATURE_DFS
    HMAC_WLAN_CTX_EVENT_SUB_TYPR_DFS_TEST,
    HMAC_WALN_CTX_EVENT_SUB_TYPR_DFS_CAC_CTRL_TX, /* DFS 1min CAC把vap状态位置为pause或者up,同时禁止或者开启硬件发送 */
#endif

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_MATRIX_HMAC2DMAC,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_MU_EDCA_REG,      /* STA收到beacon和assoc rsp时，更新MU EDCA寄存器 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_AFFECTED_ACS_REG, /* STA收到MU EDCA Control Frame时，更新相关寄存器 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_FEEDBACK_STATUS,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_SPATIAL_REUSE_REG,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_CLOSE_11AX_RELATED_PARA,

#ifdef _PRE_WLAN_FEATURE_TWT
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT, /* STA收到twt 时，更新寄存器 */
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_WUR, /* STA收到WUR相关帧时，更新相关参数 */
#endif

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SEND_PROBE_REQUEST_FRAME,

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_MULTI_BSSID_SET,

    HMAC_WLAN_CTX_EVENT_SUB_TYPE_APF_CMD,
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_NEW_FRW_MGMT, /* 新frw机制的管理帧发送处理消息，
                                                 此 ID为保证新老机制共存而增加,待调好之后删除 */
    HMAC_WLAN_CTX_EVENT_SUB_TYPE_BUTT
} hmac_wlan_ctx_event_sub_type_enum;
typedef osal_u8 hmac_wlan_ctx_event_sub_type_enum_uint8;

/* MISC杂散事件 */
typedef enum {
    HMAC_MISC_SUB_TYPE_RADAR_DETECT,
    HMAC_MISC_SUB_TYPE_DISASOC,
    HMAC_MISC_SUB_TYPE_CALI_TO_HMAC,
    HMAC_MISC_SUB_TYPE_HMAC_TO_CALI,
// #ifdef _PRE_SUPPORT_ACS
    HMAC_MISC_SUB_TYPE_RESCAN,
// #endif

#ifdef _PRE_WLAN_ONLINE_DPD
    HMAC_TO_HMAC_DPD,
#endif

    HMAC_MISC_SUB_TYPE_BUTT
} hmac_misc_sub_type_enum;

typedef enum {
    THRUPUT_NOACK,               /* 使用最大速率NOACK测试峰值性能 */
    THRUPUT_DEV_WIFI,            /* DEV wifi线程峰值性能测试 frw死循环noack发包 */
    THRUPUT_HAL_BYPASS,          /* 逻辑bypass峰值性能测试,不写入FIFO队列软件产生发送完成中断 */
    THRUPUT_HMAC_PK,             /* HMAC pk版本，bypass不相关流程 */
    THRUPUT_FIX_AGGR,            /* 配置强制聚合数 */
    THRUPUT_COMP_SCHE_BYPASS,    /* 发送完成上报HMAC消息bypass使能 */
    THRUPUT_CACHE_MISS_STATISTIC, /* CACHE MISS DFX */
    THRUPUT_DELAY_START_NUM,     /* 延时统计开始序号 */
    THRUPUT_DELAY_END_NUM,       /* 延时统计结束序号 */
    THRUPUT_TX_BEFORE_HCC,       /* TX HCC之前丢包 */
    THRUPUT_RX_AFTER_HCC,        /* RX HCC之后丢包 */
    THRUPUT_RX_REPORT,           /* RX 流量上报 */
    THRUPUT_TASK_CPU,            /* 查看任务CPU使用率 */
    THRUPUT_TX_PK,               /* TX PK流程 */
    THRUPUT_RX_PK,               /* RX PK流程 */
    THRUPUT_HCC_STAT,            /* HCC队列查询 */
    THRUPUT_RESUME_FRW_TX_DATA,  /* 数据通路TX恢复走wifi_frw_txdata任务 */
    THRUPUT_ALG_BYPASS,          /* bypass host侧tx rx逐包通知 */
    THRUPUT_RESUME_FRW_RX_DATA,  /* 数据通路RX恢复走wifi_frw_txdata任务 */
    THRUPUT_TEST_SCHEDULE,       /* 开关打开后，下行数据报文入tid队列 */
    THRUPUT_TEST_NO_SCHEDULE,    /* 开关打开后，下行数据报文都不入tid队列 */
    THRUPUT_TX_ENQUE_TID,        /* 开关打开后，TX方向性能模式关闭 */
    THRUPUT_TEST_QUERY_RATE_STAT, /* 查询一段时间收发包速率统计 */
    THRUPUT_TYPE_BUTT
} hmac_thruput_enum;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/* DMAC与HMAC模块共用的WLAN DRX事件结构体 */
typedef struct {
    oal_netbuf_stru *netbuf; /* netbuf链表一个元素 */
    osal_u16 netbuf_num;        /* netbuf链表的个数 */
    osal_u8 resv[2];           /* 2字节对齐 */
} hmac_wlan_drx_event_stru;

/* DMAC与HMAC模块共用的WLAN CRX事件结构体 */
typedef struct {
    oal_netbuf_stru *netbuf; /* 指向管理帧对应的netbuf */
    //    osal_u8              *chtxt;          /* Shared Key认证用的challenge text */
} hmac_wlan_crx_event_stru;

/* DMAC与HMAC模块共用的DTX事件结构体 */
typedef struct {
    oal_netbuf_stru *netbuf; /* netbuf链表一个元素 */
    osal_u16 frame_len;
    osal_u16 remain;
    osal_u8 data[0];
} dmac_tx_event_stru;

typedef struct {
    mac_channel_stru channel;
    mac_ch_switch_info_stru ch_switch_info;

    oal_bool_enum_uint8 switch_immediately; /* 1 - 马上切换  0 - 暂不切换, 推迟到tbtt中切换 */
    oal_bool_enum_uint8 check_cac;
    oal_bool_enum_uint8 dot11_40mhz_intolerant;
    osal_u8 resv[1];
} hmac_set_chan_stru;

/* 扫描请求事件payload结构体 */
typedef struct {
    mac_scan_req_stru *scan_params; /* 将扫描参数传下去 */
} hmac_ctx_scan_req_stru;

/* 处理MPDU的MSDU的处理状态的结构体的定义 */
typedef struct {
    oal_netbuf_stru *curr_netbuf; /* 当前处理的netbuf指针 */
    osal_u8 *curr_netbuf_data;      /* 当前处理的netbuf的data指针 */
    osal_u16 submsdu_offset;         /* 当前处理的submsdu的偏移量,   */
    osal_u8 msdu_nums_in_netbuf;     /* 当前netbuf包含的总的msdu数目 */
    osal_u8 procd_msdu_in_netbuf;    /* 当前netbuf中已处理的msdu数目 */
    osal_u8 netbuf_nums_in_mpdu;     /* 当前MPDU的中的总的netbuf的数目 */
    osal_u8 procd_netbuf_nums;       /* 当前MPDU中已处理的netbuf的数目 */
    osal_u8 procd_msdu_nums_in_mpdu; /* 当前MPDU中已处理的MSDU数目 */

    osal_u8 flag;
} hmac_msdu_proc_state_stru;

/* 每一个MSDU包含的内容的结构体的定义 */
typedef struct {
    osal_u8 sa[WLAN_MAC_ADDR_LEN]; /* MSDU发送的源地址 */
    osal_u8 da[WLAN_MAC_ADDR_LEN]; /* MSDU接收的目的地址 */
    osal_u8 ta[WLAN_MAC_ADDR_LEN]; /* MSDU接收的发送地址 */
    osal_u8 priority;
    osal_u8 resv[1];

    oal_netbuf_stru *netbuf; /* MSDU对应的netbuf指针(可以使clone的netbuf) */
} hmac_msdu_stru;

typedef struct {
    osal_char signal;          /* 信号强度 */
    osal_u16 asoc_id;         /* Association ID of the STA */
    osal_u32 rx_packets;      /* total packets received */
    osal_u32 tx_packets;      /* total packets transmitted */
    osal_u32 rx_bytes;        /* total bytes received     */
    osal_u32 tx_bytes;        /* total bytes transmitted  */
    osal_u32 tx_retries;      /* 发送重传次数 */
    osal_u32 rx_dropped_misc; /* 接收失败次数 */
    osal_u32 tx_failed;       /* 发送失败次数 */
    osal_s16 free_power;      /* 底噪 */
    osal_u16 chload;          /* 信道繁忙程度 */
    station_info_extend_stru station_info_extend;
    mac_rate_info_stru txrate; /* vap当前速率 */
} hmac_stat_info_rsp_stru;

#ifdef _PRE_WLAN_FEATURE_M2S
typedef struct {
    wlan_m2s_mgr_vap_stru m2s_comp_vap[WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE];
    oal_bool_enum_uint8 m2s_result;
    osal_u8 m2s_mode;  /* 当前切换业务 */
    osal_u8 m2s_state; /* 当前m2s状态 */
    osal_u8 vap_num;
} hmac_m2s_complete_syn_stru;
#endif

/* MU EDCA Control Field */
typedef struct {
    /* 协议中规定的B0~B3分别对应BK、BE、VI、VO，而硬件刚好相反，对应VO、VI、BE、BK */
    osal_u8 affected_acs_b0 : 1;
    osal_u8 affected_acs_b1 : 1;
    osal_u8 affected_acs_b2 : 1;
    osal_u8 affected_acs_b3 : 1;
    osal_u8 reserved : 4;
} mac_mu_edca_control;

typedef struct {
    osal_u8                      scan_idx;
    osal_u8                      resv[3];
    wlan_scan_chan_stats_stru    chan_result;
} hmac_crx_chan_result_stru;

typedef struct {
    hal_reset_hw_type_enum_uint8 reset_type; /* 命令类型0|1|2|3(all|phy|mac|debug) */
    hal_reset_mac_submod_enum_uint8 reset_mac_mod;
    osal_u8 reset_phy_reg         : 1;
    osal_u8 reset_mac_reg         : 1;
    osal_u8 is_rx_dscrlist_reset  : 1;
    osal_u8 resv                  : 5;
    osal_u8 resv1[1];
} hmac_reset_para_stru;

typedef struct {
    dmac_disasoc_misc_reason_enum_uint16   disasoc_reason;
    osal_u16                               user_idx;
} hmac_disasoc_misc_stru;

typedef struct {
    osal_u16 user_id;
    osal_u8  ba_size;
    oal_bool_enum_uint8 need_delba;
} hmac_to_hmac_btcoex_rx_delba_trigger_event_stru;

typedef struct {
    osal_u8                      device_id;
    wlan_nss_enum_uint8          m2s_nss;
    wlan_m2s_type_enum_uint8     m2s_type;   /* 0:软切换 1:硬切换 */
    osal_u8                      reserve[1];
} hmac_to_hmac_m2s_event_stru;

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* 挂载在hmac_ctx_action_event_stru.resv[2]传递给dmac */
typedef struct {
    osal_u8   ac;
    osal_u8   psb       : 1;
    osal_u8   direction : 7;
} hmac_addts_info_stru;
#endif

/* DMAC与HMAC模块共用的CTX子类型ACTION对应的事件的结构体,当DMAC自身产生DELBA帧时,使用该结构体向HMAC模块抛事件 */
typedef struct {
    mac_category_enum_uint8   action_category;     /* ACTION帧的类型 */
    osal_u8                   action;              /* 不同ACTION类下的子帧类型 */
    osal_u16                  user_idx;
    osal_u16                  frame_len;           /* 帧长度 */
    osal_u8                   hdr_len;             /* 帧头长度 */
    osal_u8                   tidno;               /* tidno，部分action帧使用 */
    osal_u8                   initiator;           /* 触发端方向 */

    /* 以下为接收到req帧，发送rsp帧后，需要同步到dmac的内容 */
    osal_u8                       status;              /* rsp帧中的状态 */
    osal_u16                      baw_start;           /* 窗口开始序列号 */
    osal_u16                      baw_size;            /* 窗口大小 */
    osal_u8                       ampdu_max_num;       /* BA会话下的最多聚合的AMPDU的个数 */
    oal_bool_enum_uint8           amsdu_supp;          /* 是否支持AMSDU */
    osal_u16                      ba_timeout;          /* BA会话交互超时时间 */
    mac_back_variant_enum_uint8   back_var;            /* BA会话的变体 */
    osal_u8                       dialog_token;        /* ADDBA交互帧的dialog token */
    osal_u8                       ba_policy;           /* Immediate=1 Delayed=0 */
    osal_u8                       lut_index;           /* LUT索引 */
    osal_u8                       mac_addr[WLAN_MAC_ADDR_LEN];    /* 用于DELBA/DELTS查找HMAC用户 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
    osal_u8                       tsid;                /* TS相关Action帧中的tsid值 */
    osal_u8                       ts_dialog_token;     /* ADDTS交互帧的dialog token */
    hmac_addts_info_stru          addts_info;
#endif
} hmac_ctx_action_event_stru;

/* Update join req 参数写寄存器的结构体定义 */
typedef struct {
    osal_u8               bssid[WLAN_MAC_ADDR_LEN];     /* 加入的AP的BSSID */
    osal_u16              beacon_period;
    mac_channel_stru      current_channel;               /* 要切换的信道信息 */
    osal_u32              beacon_filter;                 /* 过滤beacon帧的滤波器开启标识位 */
    osal_u32              non_frame_filter;              /* 过滤no_frame帧的滤波器开启标识位 */
    osal_u8               ssid[WLAN_SSID_MAX_LEN];      /* 加入的AP的SSID  */
    osal_u8               dtim_period;                   /* dtim period      */
    oal_bool_enum_uint8   dot11_40mhz_operation_implemented; /* dot11FortyMHzOperationImplemented */
    osal_u8               resv;
    hmac_set_rate_stru    min_rate;      /* Update join req 需要配置的速率集参数 */
} hmac_ctx_join_req_set_reg_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    mac_frame_he_spatial_reuse_parameter_set_ie_stru sr_ie;
} hmac_ctx_sta_asoc_set_sr_reg_stru;
#endif

typedef  hal_rx_ctl_stru mac_rx_ctl_stru;

/* hmac to dmac配置同步消息结构体 */
typedef struct {
    wlan_cfgid_enum_uint16            syn_id;        /* 同步事件ID */
    osal_u16                          len;           /* 事件payload长度 */
    osal_u8                           msg_body[0];   /* 事件payload */
} hmac_to_dmac_cfg_msg_stru;

/* HMAC到DMAC配置同步操作结构体 */
typedef struct {
    wlan_cfgid_enum_uint16    cfgid;
    osal_u8                   resv[2];
    osal_u32(*set_func)(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
} hmac_config_syn_stru;

/* 1字节对齐 */
#pragma pack(push, 1)
#pragma pack(pop)

/*****************************************************************************
    函数定义
*****************************************************************************/
/*****************************************************************************
 功能描述  : 从当前的netbuf指针开始，释放后续n个netbuf元素
*****************************************************************************/
static INLINE__ osal_u32  hmac_rx_free_netbuf_list(
    oal_netbuf_head_stru       *netbuf_hdr,
    oal_netbuf_stru           **pst_netbuf,
    osal_u16                    nums)
{
    oal_netbuf_stru    *netbuf_temp;
    osal_u16          us_netbuf_num;

    if (OAL_UNLIKELY((netbuf_hdr == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (us_netbuf_num = 0; us_netbuf_num < nums; us_netbuf_num++) {
        netbuf_temp = oal_get_netbuf_next(*pst_netbuf);

        oal_netbuf_delete(*pst_netbuf, netbuf_hdr);
        if (oal_netbuf_free(*pst_netbuf) == OAL_ERR_CODE_OAL_MEM_ALREADY_FREE) {
        }
        *pst_netbuf = netbuf_temp;
        if (netbuf_hdr == (oal_netbuf_head_stru *)(*pst_netbuf)) {
            break;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据netbuf获取payload地址
*****************************************************************************/
static INLINE__ osal_u8* mac_netbuf_get_payload(const oal_netbuf_stru *pst_netbuf)
{
#if defined(_PRE_PRODUCT_ID_HIMPXX_DEV)
    return OAL_NETBUF_PAYLOAD(pst_netbuf);
#else
    /* 注意!对51而言，此处只偏移了24字节，所以如果mac header长度不是24字节的不要使用该函数 */
    return OAL_NETBUF_PAYLOAD(pst_netbuf) + MAC_80211_FRAME_LEN;
#endif
}

#define mac_get_netbuf_by_cb(ptr) \
    ((oal_netbuf_stru *)((osal_s8 *)(ptr) - (osal_ulong)(&((oal_netbuf_stru *)0)->cb)))

/*****************************************************************************
 函 数 名  : mac_get_wlan_frame_type
 功能描述  : 根据tx control block获取帧类型
*****************************************************************************/
static INLINE__ osal_u16 mac_get_wlan_frame_type(const mac_tx_ctl_stru *tx_ctl)
{
    oal_netbuf_stru *netbuf = mac_get_netbuf_by_cb(tx_ctl);
    return ((mac_ieee80211_frame_stru *)(oal_netbuf_header(netbuf)))->frame_control.type;
}

/*****************************************************************************
 函 数 名  : mac_get_wlan_frame_subtype
 功能描述  : 根据tx control block获取帧子类型
*****************************************************************************/
static INLINE__ osal_u16 mac_get_wlan_frame_subtype(const mac_tx_ctl_stru *tx_ctl)
{
    oal_netbuf_stru *netbuf = mac_get_netbuf_by_cb(tx_ctl);
    return ((mac_ieee80211_frame_stru *)(oal_netbuf_header(netbuf)))->frame_control.sub_type;
}

/*****************************************************************************
 函 数 名  : mac_get_is_data_frame
 功能描述  : 判断一个帧是否为数据帧中的data帧或qos data帧
*****************************************************************************/
static INLINE__ osal_u8 mac_get_is_data_frame(const mac_tx_ctl_stru *tx_ctrl)
{
    if ((mac_get_wlan_frame_type(tx_ctrl) == WLAN_DATA_BASICTYPE) &&
        (mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_DATA ||
        mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_QOS_DATA)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : mac_get_is_qos_data
 功能描述  : 判断一个帧是否为数据帧中的qos data帧或qos null data帧
*****************************************************************************/
static INLINE__ osal_u8 mac_get_is_qos_data(const mac_tx_ctl_stru *tx_ctrl)
{
    if ((mac_get_wlan_frame_type(tx_ctrl) == WLAN_DATA_BASICTYPE) &&
        (mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_QOS_DATA ||
        mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_QOS_NULL_FRAME)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : mac_get_is_bar
 功能描述  : 判断一个帧是否为控制帧中的block ack req帧
*****************************************************************************/
static INLINE__ osal_u8 mac_get_is_bar(const mac_tx_ctl_stru *tx_ctrl)
{
    if (mac_get_wlan_frame_type(tx_ctrl) == WLAN_CONTROL &&
        mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_BLOCKACK_REQ) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}
/*****************************************************************************
 函 数 名  : mac_set_rx_cb_mac_hdr
 功能描述  : 设置mac头的值

*****************************************************************************/
static INLINE__ osal_void mac_set_rx_cb_mac_hdr(mac_rx_ctl_stru *cb_ctrl, osal_u32 *mac_hdr_start_addr)
{
    unref_param(cb_ctrl);
    unref_param(mac_hdr_start_addr);
    return;
}

/* to do */
/*****************************************************************************
 函 数 名  : mac_get_frame_header_addr
 功能描述  : 根据tx control block获取对应的帧头地址
*****************************************************************************/
static INLINE__ mac_ieee80211_frame_stru *mac_get_frame_header_addr(const mac_tx_ctl_stru *tx_ctl)
{
    oal_netbuf_stru *netbuf = mac_get_netbuf_by_cb(tx_ctl);
    return (mac_ieee80211_frame_stru *)(oal_netbuf_header(netbuf));
}

/*****************************************************************************
 函 数 名  : mac_get_rx_cb_mac_hdr
 功能描述  : 获取mac头的值

*****************************************************************************/
static INLINE__ osal_u32 *mac_get_rx_cb_mac_hdr(mac_rx_ctl_stru *cb_ctrl)
{
    oal_netbuf_stru *netbuf = mac_get_netbuf_by_cb(cb_ctrl);
    return (osal_u32 *)(oal_netbuf_header(netbuf));
}

static INLINE__ osal_bool mac_pk_mode(const oal_netbuf_stru *netbuf)
{
    return (mac_get_cb_pk_mode((mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf)) == OSAL_TRUE);
}

osal_void mac_set_rx_cb_mac_hdr(mac_rx_ctl_stru *cb_ctrl, osal_u32 *mac_hdr_start_addr);
osal_u32 hmac_vap_set_cb_tx_user_idx(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl, const osal_u8 *data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of file */
