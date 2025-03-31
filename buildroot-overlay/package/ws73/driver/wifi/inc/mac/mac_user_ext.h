/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac_user_ext.c.
 * Create: 2020-7-8
 */
#ifndef __MAC_USER_EXT_H__
#define __MAC_USER_EXT_H__

/* 文件合一后去掉HMAC、DMAC两个宏 */
#include "mac_device_ext.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "wlan_mib_hcm.h"
#include "mac_frame.h"
#include "mac_device_ext.h"
#include "oal_util.h"
#include "oal_ext_if.h"
#include "hal_ext_if.h"
#include "mac_user_common.h"
#include "wlan_resource_common.h"
#include "msg_rekey_offload_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 发送BA窗口记录seq number的最大个数，必须是2的整数次幂 */
#define HMAC_TID_MAX_BUFS 128
/* 发送BA窗口记录seq number的bitmap所使用的类型长度 */
#define DMAC_TX_BUF_BITMAP_WORD_SIZE 32
/* 发送BA窗口记录seq number的bit map的长度 */
#define HMAC_TX_BUF_BITMAP_WORDS ((HMAC_TID_MAX_BUFS + DMAC_TX_BUF_BITMAP_WORD_SIZE - 1) / DMAC_TX_BUF_BITMAP_WORD_SIZE)

/*****************************************************************************
    宏定义
*****************************************************************************/

/* 与发送描述符和CB字段，实际申请user idx三者同时对应，无效用户id取最大值，为全board最大用户LIMIT */
#define MAC_INVALID_USER_ID WLAN_USER_MAX_USER_LIMIT

#ifdef _PRE_WLAN_FEATURE_WAPI
#define WAPI_KEY_LEN                    16
#define WAPI_PN_LEN                     16
#define HMAC_WAPI_MAX_KEYID             2
#endif

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
#define MAX_JUDGE_CACHE_LENGTH          20  /* 业务识别-用户待识别队列长度 */
#define MAX_CONFIRMED_FLOW_NUM          2   /* 业务识别-用户已识别业务总数 */
#endif
/* 特性宏END，第二阶段需要挪走 */

/*****************************************************************************
    枚举定义
*****************************************************************************/

/* 特性枚举START，第二阶段需要挪走 */
typedef enum {
    WLAN_TX_TCP_DATA = 0,    /* 发送TCP data */
    WLAN_RX_TCP_DATA = 1,    /* 接收TCP data */
    WLAN_TX_UDP_DATA = 2,    /* 发送UDP data */
    WLAN_RX_UDP_DATA = 3,    /* 接收UDP data */

    WLAN_TXRX_DATA_BUTT = 4,
} wlan_txrx_data_type_enum;
typedef osal_u8 wlan_txrx_data_enum_uint8;

typedef enum {
    BTCOEX_BLACKLIST_TPYE_FIX_BASIZE  = 0,
    BTCOEX_BLACKLIST_TPYE_NOT_AGGR    = 1,

    BTCOEX_BLACKLIST_TPYE_BUTT
} btcoex_blacklist_type_enum;
typedef osal_u8 btcoex_blacklist_type_enum_uint8;

/* 特性枚举END，第二阶段需要挪走 */

/* BA会话的状态枚举 */
typedef enum {
    HMAC_BA_INIT        = 0,    /* BA会话未建立 */
    HMAC_BA_INPROGRESS,         /* BA会话建立过程中 */
    HMAC_BA_COMPLETE,           /* BA会话建立完成 */
    HMAC_BA_HALTED,             /* BA会话节能暂停 */
    HMAC_BA_FAILED,             /* BA会话建立失败 */

    HMAC_BA_BUTT
} hmac_ba_conn_status_enum;
typedef osal_u8 hmac_ba_conn_status_enum_uint8;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_WUR_TX
typedef enum {
    HMAC_USER_WUR_MODE_OFF = 0,
    HMAC_USER_WUR_MODE_ON = 1,
    HMAC_USER_WUR_MODE_SUSPEND = 2
} hmac_user_wur_status_enum;

/* 用户WUR能力字段信息 */
typedef struct {
    osal_u8 support_2g : 1, support_5g : 1, rsvd : 6; /* 支持频段 */
    osal_u8 transition_delay; /* 转换时延 */
    osal_u8 vl_wur_frame_support : 1,
            wur_group_ids_support : 2,
            rsvd1 : 1,
            wur_basic_ppdu_hdr_support_20mhz : 1,
            wur_fdma_support : 1,
	        wur_short_wakeup_frame_support : 1,
            rsvd2 : 1; /* 能力信息 */
    osal_u8 rsvd3; /* 保留1字节对齐 */
} mac_user_wur_cap_ie_stru;

typedef struct hmac_user_wur_info {
    osal_u8 multi_time_cnt; /* 组播缓存/BSS参数更新，发送完WUR FL WAKE UP帧后计时 */
    osal_u8 wur_status : 2, /* WUR服务的状态 */
            unicast_buffered : 1, /* 用户标记单播缓存 */
            multi_buffered : 1, /* 组播用户标记组播缓存 */
            wur_ps_enqueue : 1, /* 是否进节能队列 */
            send_multi_wakeup : 1, /* WUR FL WAKE UP帧已发送标记 */
            rsvd : 2;
    osal_u16 wur_id; /* WUR ID单播用户标识 */
} hmac_user_wur_info_stru;
#endif

typedef struct {
    osal_u16    spectrum_mgmt        : 1,  /* 频谱管理: 0=不支持, 1=支持 */
                qos                  : 1,  /* QOS: 0=非QOS站点, 1=QOS站点 */
                barker_preamble_mode : 1,  /* 供STA保存BSS中站点是否都支持short preamble.0=支持.1=不支持 */
                apsd                 : 1,  /* 自动节能: 0=不支持, 1=支持 */
                                               /* 目前apsd只有写没有读，wifi联盟已自己定义了WMM
                                                  节能IE代替cap apsd功能, 此处预留为后续可能出的兼容性问题提供接口 */
                pmf_active           : 1,  /* 管理帧加密使能开关 */
                erp_use_protect      : 1,  /* 供STA保存AP是否启用了ERP保护 */
                txbf_11n              : 1,
                proxy_arp            : 1,
                histream_cap         : 1,
                cap_1024qam          : 1,  /* Support 1024QAM */
                bss_transition       : 1,  /* Support bss transition */
                mdie                 : 1,  /* mobility domain IE presented, for 11r cap */
                enable_11k           : 1,
                smps_cap             : 1,  /* sta模式下，标识对端是否支持smps */
                dcm_cap                  : 1,
                use_wpa3             : 1;
    osal_u16   resv;
}mac_user_cap_info_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
osal_u32 max_mcs_1ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_2ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_3ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_4ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_5ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_6ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_7ss : 3,  /* 一个空间流的MCS最大支持MAP */
         max_mcs_8ss : 3;  /* 一个空间流的MCS最大支持MAP */
} mac_max_he_mcs_map_stru;
typedef mac_max_he_mcs_map_stru mac_tx_max_he_mcs_map_stru;
typedef mac_max_he_mcs_map_stru mac_rx_max_he_mcs_map_stru;
#endif

/* user结构体，对SA Query流程信息的保存结构 */
typedef struct {
    osal_u32 sa_query_start_time;  /* sa_query 流程开始时间,单位ms */
    osal_u16 sa_query_trans_id;    /* trans id */
    osal_u16 sa_query_count;       /* number of pending SA Query requests, 0 = no SA Query in progress */
    frw_timeout_stru sa_query_interval_timer;  /* SA Query 间隔定时器，相关dot11AssociationSAQueryRetryTimeout */
} mac_sa_query_stru;

#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef struct {
    td_u8 user_addr[WLAN_MAC_ADDR_LEN];      /* 用户mac地址 */
    td_u8 conn_rx_rssi;
    mac_user_asoc_state_enum_uint8 assoc_state;
    td_u8 bcn_prio;
    td_u8 is_mesh_user;
    td_u8 is_initiative_role;
}mac_user_assoc_info_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* user结构中，ts信息的保存结果 */
typedef struct mac_ts {
    osal_u32                           medium_time;
    osal_u8                            tsid;
    osal_u8                            up;
    mac_wmmac_direction_enum_uint8       direction;
    mac_ts_conn_status_enum_uint8        ts_status;
    osal_u8                            ts_dialog_token;
    osal_u8                            vap_id;
    osal_u16                           mac_user_idx;
#if defined(_PRE_PRODUCT_ID_HOST)
    /* 每个ts下都需要建立定时器，此处不放到hmac_user_stru */
    frw_timeout_stru                     addts_timer;
    osal_u8                            second_flag;
    osal_u8                            rsv[3];
#endif
} mac_ts_stru;
#endif

/*
 * 802.1X-port 状态结构体
 * 1X端口状态说明:
 * 1) portvalid && keydone 都是 TRUE: 表示端口处于合法状态
 * 2) portvalid == TRUE && keydone == FALSE:表示端口处于未知状态密钥还未生效
 * 3) portValid == FALSE && keydone== TRUE:表示端口处于非法状态密钥获取失败
 * 4) portValid && keyDone are FALSE: 表示端口处于合法状态密钥还未生效
 */
typedef struct {
    oal_bool_enum_uint8             keydone;                      /* 端口合法性是否允许测试 */
    oal_bool_enum_uint8             portvalid;                    /* 端口合法性标识 */
    osal_u8                         resv0[2];
} mac_8021x_port_status_stru;


/* 供AP查看STA是否被统计到对应项 */
typedef struct {
    osal_u8 no_short_slot_stats_flag     : 1; /* user是否被统计到no short slot num中, 0未被统计, 1已被统计 */
    osal_u8 no_short_preamble_stats_flag : 1; /* user是否被统计到no short preamble num中 */
    osal_u8 no_erp_stats_flag            : 1; /* user是否被统计到no erp num中 */
    osal_u8 no_ht_stats_flag             : 1; /* user是否被统计到no ht num中 */
    osal_u8 no_gf_stats_flag             : 1; /* user是否被统计到no gf num中 */
    osal_u8 only_stats_20m_flag          : 1; /* user是否被统计到no 20M only num中 */
    osal_u8 no_40dsss_stats_flag         : 1; /* user是否被统计到no 40dsss num中 */
    osal_u8 no_lsig_txop_stats_flag      : 1; /* user是否被统计到no lsig txop num中 */
    osal_u8 resv0[3];
} mac_user_stats_flag_stru;

/* AP侧keepalive活动的控制结构体 */
typedef struct {
    osal_u8  keepalive_count_ap;        /* AP定时保活检测操作计数器 */
    osal_u8  timer_to_keepalive_count;  /* 复用aging timer/STA省电模式定时清醒策略，定时发送keepalive帧的计数器 */
    osal_u8  delay_flag;                /* 标志用户进入睡眠状态或者其他不能马上收帧反馈的状态 */
    osal_u8  auc_resv[1];
} mac_user_keepalive;

/* 空间流信息结构体 */
typedef struct {
    osal_u16            user_idx;
    wlan_nss_enum_uint8   avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8   user_num_spatial_stream;             /* 用户支持的空间流个数 */
} mac_user_nss_stru;

/* m2s user信息结构体 */
typedef struct {
    osal_u16              user_idx;
    wlan_nss_enum_uint8     user_num_spatial_stream;             /* 用户自身空间流能力 */
    wlan_nss_enum_uint8     avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */

    wlan_nss_enum_uint8     avail_bf_num_spatial_stream;         /* 用户支持的Beamforming空间流个数 */
    wlan_bw_cap_enum_uint8  avail_bandwidth;                     /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8  cur_bandwidth;                       /* 默认值与avail_bandwidth相同,供算法调用修改 */
    wlan_mib_mimo_power_save_enum_uint8 cur_smps_mode;
} mac_user_m2s_stru;

/* ------- hmac_user_rom.h START ---------- */
typedef struct {
    osal_spinlock lock_ps;                   /* 对队列和ul_mpdu_num操作时需要加锁 */
    oal_netbuf_head_stru ps_queue_head;           /* 用户节能缓存队列头 */
    osal_atomic mpdu_num;                          /* 用户节能缓存队列中已存在的mpdu个数 */
    osal_u8 ps_time_count;
    osal_u8 auc_resv[3];                            /* 保留2个字节用作对齐 */
} hmac_user_ps_stru;

/* ------- hmac_user_rom.h END ---------- */

/* ------- hmac_ext_if_hcm.h START ---------- */
typedef struct {
    osal_u32 cur_rate_goodput_kbps;
    osal_u32 rate_kbps;       /* 速率大小(单位:kbps) */
    osal_u8 aggr_subfrm_size; /* 聚合子帧数门限值 */
    osal_u8 per;              /* 该速率的PER(单位:%) */
#ifdef _PRE_WLAN_DFT_STAT
    osal_u8 best_rate_per; /* 最优速率的PER(单位:%) */
    osal_u8 resv[1];
#else
    osal_u8 resv[2];       /* 预留2字节对齐 */
#endif
} hmac_tx_normal_rate_stats_stru;

typedef struct {
    osal_u16 baw_start;                              /* 第一个未确认的MPDU的序列号 */
    osal_u16 last_seq_num;                           /* 最后一个发送的MPDU的序列号 */
    osal_u16 baw_size;                               /* Block_Ack会话的buffer size大小 */
    osal_u16 baw_head;                               /* bitmap中记录的第一个未确认的包的位置 */
    osal_u16 baw_tail;                               /* bitmap中下一个未使用的位置 */
    oal_bool_enum_uint8 is_ba;                     /* Session Valid Flag */
    hmac_ba_conn_status_enum_uint8 ba_conn_status; /* BA会话的状态 */
    mac_back_variant_enum_uint8 back_var;          /* BA会话的变体 */
    osal_u8 dialog_token;                            /* ADDBA交互帧的dialog token */
    osal_u8 ba_policy;                               /* Immediate=1 Delayed=0 */
    oal_bool_enum_uint8 amsdu_supp;                /* BLOCK ACK支持AMSDU的标识 */
    osal_u8 *dst_addr;                              /* BA会话接收端地址 */
    osal_u16 ba_timeout;                             /* BA会话交互超时时间 */
    osal_u8 ampdu_max_num;                           /* BA会话下，能够聚合的最大的mpdu的个数 */
    osal_u8 tx_ba_lut;                               /* BA会话LUT session index */
    osal_u16 mac_user_idx;
    osal_u16 pre_baw_start;    /* 记录前一次判断ba窗是否卡死时的ssn */
    osal_u16 pre_last_seq_num; /* 记录前一次判断ba窗是否卡死时的lsn */
    osal_u16 ba_jamed_cnt;     /* BA窗卡死统计次数 */
    osal_u32 tx_buf_bitmap[HMAC_TX_BUF_BITMAP_WORDS];
} hmac_ba_tx_hdl_stru;

/* 一个站点下的每一个TID下的聚合接收的状态信息 */
typedef struct {
    hmac_ba_conn_status_enum_uint8 ba_conn_status; /* BA会话的状态 */
    osal_u8 lut_index;                               /* 接收端Session H/w LUT Index */
    mac_ba_policy_enum_uint8 ba_policy;            /* Immediate=1 Delayed=0 */
    osal_u8 resv;
} hmac_ba_rx_hdl_stru;
/* ------- hmac_ext_if_hcm.h END ---------- */

/* ================= FEATURE Start 特性相关 =========================== */

/* ================= FEATURE End   特性相关 =========================== */

/*****************************************************************************
    结构体定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
/* 业务识别-五元组结构体: 用于唯一地标识业务流 */
typedef struct {
    osal_u32                          sip;                         /* ip */
    osal_u32                          dip;

    osal_u16                          sport;                       /* 端口 */
    osal_u16                          dport;

    osal_u32                          proto;                       /* 协议 */
} hmac_tx_flow_info_stru;

/* 业务识别-待识别队列结构体: */
typedef struct {
    hmac_tx_flow_info_stru            flow_info;

    osal_u32                          len;                        /* 来包长度 */
    osal_u8                           flag;                       /* 有效位，用于计数 */

    osal_u8                           udp_flag;                   /* udp flag为1即为UDP帧 */
    osal_u8                           tcp_flag;                   /* tcp flag为1即为TCP帧 */

    osal_u8                           rtpver;                     /* RTP version */
    osal_u32                          rtpssrc;                    /* RTP SSRC */
    osal_u32                          payload_type;               /* RTP:标记1bit、有效载荷类型(PT)7bit */
} hmac_tx_judge_info_stru;

/* 业务识别-待识别队列主要业务结构体: */
typedef struct {
    hmac_tx_flow_info_stru            flow_info;

    osal_u32                          average_len;                /* 业务来包平均长度 */
    osal_u8                           flag;                       /* 有效位 */

    osal_u8                           udp_flag;                   /* udp flag为1即为UDP帧 */
    osal_u8                           tcp_flag;                   /* tcp flag为1即为TCP帧 */

    osal_u8                           rtpver;                     /* RTP version */
    osal_u32                          rtpssrc;                    /* RTP SSRC */
    osal_u32                          payload_type;               /* 标记1bit、有效载荷类型(PT)7bit */

    osal_u32                          wait_check_num;             /* 待检测列表中此业务包个数 */
} hmac_tx_major_flow_stru;

/* 业务识别-用户已识别结构体: */
typedef struct {
    hmac_tx_flow_info_stru            cfm_flow_info;               /* 已识别业务的五元组信息 */

    osal_ulong                         last_jiffies;                /* 记录已识别业务的最新来包时间 */
    osal_u16                          cfm_tid;                     /* 已识别业务tid */

    osal_u16                          cfm_flag;                    /* 有效位 */
} hmac_tx_cfm_flow_stru;

/* 业务识别-用户待识别业务队列: */
typedef struct {
    osal_ulong                         jiffies_st;      /* 记录待识别业务队列的起始时间与最新来包时间 */
    osal_ulong                         jiffies_end;
    osal_u32                          to_judge_num;    /* 用户待识别业务队列长度 */

    hmac_tx_judge_info_stru           judge_cache[MAX_JUDGE_CACHE_LENGTH];    /* 待识别流队列 */
} hmac_tx_judge_list_stru;
#endif

typedef struct {
    oal_bool_enum_uint8             in_use;             /* 缓存BUF是否被使用 */
    osal_u8                         num_buf;         /* 占用的netbuf个数与head中的qlen作用一致 */
    osal_u16                        seq_num;         /* MPDU对应的序列号 */
    oal_netbuf_head_stru            netbuf_head;     /* MPDU对应的描述符首地址 */
    osal_u32                        rx_time;         /* 报文被缓存的时间戳 */
} hmac_rx_buf_stru;

typedef struct {
    osal_void                       *ba;
    osal_u8                         tid;
    mac_delba_initiator_enum_uint8  direction;
    osal_u8                         resv[1];
    osal_u8                         vap_id;
    osal_u16                        mac_user_idx;
    osal_u16                        timeout_times;
} hmac_ba_alarm_stru;

/* TID对应的发送BA会话的状态 */
typedef struct {
    frw_timeout_stru                addba_timer;
    hmac_ba_alarm_stru              alarm_data;

    hmac_ba_conn_status_enum_uint8  ba_status;       /* 该TID对应的BA会话的状态 */
    osal_u8                       addba_attemps;   /* 启动建立BA会话的次数 */
    osal_u8                       dialog_token;    /* 随机标记数 */
    oal_bool_enum_uint8           ba_policy;       /* Immediate=1 Delayed=0 */
    osal_spinlock                 ba_status_lock;  /* 该TID对应的BA会话的状态锁 */
} hmac_ba_tx_stru;

typedef struct {
    osal_u32    short_preamble      : 1,        /* 是否支持802.11b短前导码 0=不支持， 1=支持 */
                erp                 : 1,        /* AP保存STA能力使用,指示user是否有ERP能力， 0=不支持，1=支持 */
                short_slot_time     : 1,        /* 短时隙: 0=不支持, 1=支持 */
                support_11ac2g              : 1,
                owe : 1,
                bit_resv                : 27;
} hmac_user_cap_info_stru;

typedef struct {
    osal_u8     rs_nrates;                                 /* 个数 */
    osal_u8     rs_rates[WLAN_USER_MAX_SUPP_RATES];       /* 速率 */
} hmac_rate_stru;

/* Hmac侧接收侧BA会话句柄 */
typedef struct {
    osal_u16                      baw_start;               /* 第一个未收到的MPDU的序列号 */
    osal_u16                      baw_end;                 /* 最后一个可以接收的MPDU的序列号 */
    osal_u16                      baw_tail;                /* 目前Re-Order队列中，最大的序列号 */
    osal_u16                      baw_size : 10;           /* Block_Ack会话的buffer size大小 */
    osal_u16                      timer_triggered : 1;     /* 上一次上报是否为定时器上报 */
    osal_u16                      is_ba : 1;               /* Session Valid Flag */
    osal_u16                      ba_status : 4;           /* 该TID对应的BA会话的状态 */

    /* 以下action帧相关 */
    osal_u8                       dialog_token;     /* ADDBA交互帧的dialog token */
    osal_u8                       back_var : 6;     /* BA会话的变体 */
    osal_u8                       ba_policy : 1;    /* Immediate=1 Delayed=0 */
    osal_u8                       amsdu_supp : 1;   /* BLOCK ACK支持AMSDU的标识 */
    osal_u16                      status_code;      /* 返回状态码 */
    osal_u8                      *transmit_addr;    /* BA会话发送端地址 */
    osal_u32                      rx_buf_bitmap[2];
    osal_u16                      ba_timeout;       /* BA会话交互超时时间 */
    osal_u16                      baw_head;         /* bitmap的起始序列号 */
    osal_u8                       ba_resp_buf_size; /* ADDBA Response帧回复的buffer size大小 */
    osal_u8                       lut_index;        /* 接收端Session H/w LUT Index */

    osal_u8                       resv;
    osal_u8                       mpdu_cnt;         /* 当前Re-Order队列中，MPDU的数目 */
    hmac_rx_buf_stru              re_order_list[WLAN_AMPDU_RX_BUFFER_SIZE];  /* Re-Order队列 */
    hmac_ba_alarm_stru            alarm_data;
} hmac_ba_rx_stru;

/* user结构中，TID对应的BA信息的保存结构 */
typedef struct {
    osal_u8 tid          : 4; /* 通信标识符 */
    osal_u8 is_paused    : 2; /* TID被暂停调度 */
    osal_u8 tx_mode      : 2; /* 发送模式: rifs,aggr,normal发送 */

    osal_u8 is_delba_ing : 1; /* 该tid是否正在发delba */
    osal_u8 vap_id       : 7;

    osal_u8 rx_wrong_ampdu_num; /* 该tid未建立BA会话时收到的聚合子帧数(一般是DELBA失败) */
    oal_bool_enum_uint8 ba_handle_tx_enable : 1;     /* 该tid下发送方向是否允许建BA，配置命令需求 */
    oal_bool_enum_uint8 ba_handle_rx_enable : 1;     /* 该tid下接送方向是否允许建BA，配置命令需求 */
    osal_u8 resv : 6;

    osal_u16 mpdu_num; /* MPDU个数 */
    osal_u16 user_idx; /* 无效值为MAC_RES_MAC_USER_NUM */
    osal_u32 in_num;   /* 进入队列的包的个数 */
    osal_u32 out_num;  /* 出队列的包的个数 */
    osal_u32 dispatch_num;  /* 发生tid队列调度的次数 */
    osal_u32 tid_buffer_frame_len; /* 各tid缓存帧长 */
    oal_netbuf_head_stru buff_head; /* 发送缓存队列链表 */
    osal_void *alg_priv;                             /* TID级别算法私有结构体 */
    hmac_ba_rx_hdl_stru ba_rx_hdl; /* dmac rx ba info */
    hmac_ba_tx_stru ba_tx_info;
    osal_spinlock ba_rx_lock;                /* 用于ba_rx hcc线程和事件线程并发 */
    hmac_ba_rx_stru *ba_rx_info;             /* 由于部分处理上移，这部分内存到LocalMem中申请 */
    frw_timeout_stru ba_timer;               /* 接收重排序缓冲超时 */
} hmac_tid_stru;

typedef struct {
    oal_netbuf_head_stru        msdu_head;         /* msdu链表头 */
    frw_timeout_stru            amsdu_timer;
    osal_spinlock               amsdu_lock;        /* amsdu task lock */

    osal_u16                    amsdu_size;        /* Present size of the AMSDU */

    osal_u8                     amsdu_maxnum : 4;
    osal_u8                     msdu_num : 4;          /* Number of sub-MSDUs accumulated */
    osal_u8                     last_pad_len;      // 51合入后可删除/* 最后一个msdu的pad长度 */
} hmac_amsdu_stru;

typedef struct {
    osal_u32 drv_rx_pkts;     /* 接收数据(硬件上报，包含rx描述符不成功的帧)数目，仅仅统计数据帧 */
    osal_u32 drv_rx_bytes;    /* 驱动接收字节数，不包括80211头尾 */
    osal_u32 rx_dropped_misc; /* 接收失败(决定丢弃的帧)次数 */
} hmac_user_query_stats_stru;
/* 特性结构体END，第二阶段需要挪走 */

/* 判断重传auth帧 */
#define HMAC_RETRY_AUTH_PROTECT_DURATION 100                       /* auth重复帧保护时间100ms */
typedef struct {
    oal_bool_enum_uint8           is_rx_auth;                    /* 是否收到过auth帧 */
    osal_u8                       resv;                          /* 增加1字节rsv字段，保持4字节对齐 */
    osal_u16                      rx_auth_seqnum;                /* 上一次收到auth帧的sequence num */
    osal_u32                      rx_auth_timestamp;             /* 上一次接收auth帧的时间 */
} hmac_auth_re_rx_protect_stru;
/* 声明该结构体变量后请一定要调用 memset_s 0 */
typedef struct {
    /* -->> Public Start */
    osal_u8 device_id; /* 设备ID */
    osal_u8 chip_id; /* 芯片ID */
    osal_u8 resv0[2];
    osal_atomic use_cnt; /* 用于user引用计数使用，该值在自增自减时需要使用原子操作 */

    /* 当前VAP工作在AP或STA模式，以下字段为user是STA或AP时公共字段，新添加字段请注意!!! */
    struct osal_list_head user_dlist; /* 用于用户遍历 */

    osal_u8 user_mac_addr[WLAN_MAC_ADDR_LEN]; /* user对应的MAC地址 */

    /* user对应资源池索引值; user为STA时，表示填在管理帧中的AID，值为用户的资源池索引值1~32(协议规定范围为1~2007) */
    osal_u16 assoc_id;
    osal_u8 vap_id; /* vap ID */

    osal_u8 protocol_mode       : 4; /* 用户工作协议 */
    osal_u8 avail_protocol_mode : 4; /* 用户和VAP协议模式交集, 供算法调用 */

    osal_u8 cur_protocol_mode        : 4; /* 默认值与en_avail_protocol_mode值相同, 供算法调用修改 */
    osal_u8 is_multi_user            : 1;
    osal_u8 avail_num_spatial_stream : 3; /* Tx和Rx支持Nss的交集,供算法调用 */

    osal_u8 user_num_spatial_stream     : 3; /* 用户支持的空间流个数 */
    osal_u8 avail_bf_num_spatial_stream : 3; /* 用户支持的Beamforming空间流个数 */
    osal_u8 port_valid                  : 1; /* 802.1X端口合法性标识 */
    osal_u8 is_user_alloced             : 1; /* 标志此user资源是否已经被申请 */

    osal_u8 resv1[2];

    mac_rate_stru avail_op_rates; /* 用户和VAP可用的11a/b/g速率交集，供算法调用 */
    mac_user_tx_param_stru user_tx_info; /* TX相关参数 */

    osal_u8 bandwidth_cap   : 4; /* 用户带宽能力信息 */
    osal_u8 avail_bandwidth : 4; /* 用户和VAP带宽能力交集,供算法调用 */

    osal_u8 cur_bandwidth         : 4; /* 默认值与avail_bandwidth相同,供算法调用修改 */
    osal_u8 qos_enhance_sta_state : 4; /* 判断是否是远端设备 */

    mac_user_asoc_state_enum_uint8 user_asoc_state; /* 用户关联状态 */
    oal_bool_enum_uint8 flag_change_user_state_to_auth_complete : 1; /* 已关联状态收到auth帧,将标志置为1 */
    osal_u8 is_wds : 1; /* 是否是wds用户 */
    osal_u8 bit_resv : 6;
    osal_u8 pub_resv0;

    mac_user_cap_info_stru cap_info; /* user基本能力信息位 */
    mac_user_ht_hdl_stru ht_hdl; /* HT capability IE和 operation IE的解析信息 */
    mac_vht_hdl_stru vht_hdl; /* VHT capability IE和 operation IE的解析信息 */

    mac_key_mgmt_stru key_info;

    /* --------- Public STA成员 Start ----------------- */
    /* --------- Public STA成员 End ------------------- */

    /* --------- Public AP成员 Start ----------------- */
    /* --------- Public AP成员 End ------------------- */

    /* -->> Public End */

    /* -->> Private Start */
    /* 当前VAP工作在AP或STA模式，以下字段为user是STA或AP时公共字段，新添加字段请注意!!! */
    osal_u8 amsdu_supported; /* 每个位代表某个TID是否支持AMSDU */
    osal_u8 resv2;
    osal_u16 amsdu_maxsize; /* amsdu最大长度 */

    hmac_amsdu_stru hmac_amsdu[WLAN_TID_MAX_NUM]; /* asmdu数组 */

    osal_u32 last_timestamp[WLAN_TID_MAX_NUM]; /* 时间戳用于实现5个连续报文建立BA */
    osal_u8 *ch_text; /* WEP用的挑战明文 */

    frw_timeout_stru mgmt_timer; /* 认证关联用定时器 */
    /* 该标志表示该TID是否可以建立BA会话，大于等于5时可以建立BA会话。该标志在用户初始化、删除BA会话时清零 */
    osal_u8 ba_flag[WLAN_TID_MAX_NUM];
    /* user可选速率,记录对端assoc req或者assoc rsp帧中supported rates和assoc rsp宣称的速率 */
    hmac_rate_stru op_rates;
    hmac_user_cap_info_stru hmac_cap_info; /* hmac侧用户能力标志位 */
    osal_u32 rssi_last_timestamp; /* 获取user rssi所用时间戳, 1s最多更新一次rssi */

    osal_u32 rx_pkt_drop; /* 接收数据包host侧被drop的计数 */

    osal_u32 first_add_time; /* 用户创建时的时间，用于统计用户在线时长 */

    /* dmac配置同步信息 */
    mac_user_type_enum_uint8 user_type;
    osal_u32 tx_rate;
    osal_u32 rx_rate;

    /* --------- Private STA成员 Start ----------------- */
    /* 当前VAP工作在STA模式，以下字段为user是AP时独有字段，新添加字段请注意!!! */
    mac_user_stats_flag_stru user_stats_flag; /* 当user是sta时候，指示user是否被统计到对应项 */
    /* --------- Private STA成员 End ------------------- */

    /* --------- Private AP成员 Start ----------------- */
    /* 当前VAP工作在AP模式，以下字段为user是STA时独有字段，新添加字段请注意!!! */
    osal_u32 assoc_req_ie_len;
    osal_u8 *assoc_req_ie_buff;
    oal_bool_enum_uint8 user_vendor_vht_capable;
    oal_bool_enum_uint8 user_vendor_novht_capable;
    osal_u8 resv3;
    /* --------- Private AP成员 End ------------------- */

    /* -->> Private End */

    /* -->> Features Start */
#ifdef _PRE_WLAN_FEATURE_PMF
    mac_sa_query_stru sa_query_info; /* sa query流程的控制信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    osal_u8 cfm_num; /* 用户已被识别业务个数 */
    osal_u8 resv5[3];
    hmac_tx_cfm_flow_stru cfm_flow_list[MAX_CONFIRMED_FLOW_NUM]; /* 已识别业务 */
    hmac_tx_judge_list_stru judge_list; /* 待识别流队列 */
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    hmac_user_wur_info_stru wur_info; /* wur用户的信息 */
    mac_user_wur_cap_ie_stru wur_cap_ie; /* 用户WUR的能力字段信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    mac_ts_stru ts_info[WLAN_WME_AC_BUTT]; /* 保存ts相关信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru he_hdl; /* HE Capability IE */
#endif
    /* -->> Features End */

    /* -->> Private Start */
    /* 添加信息上报字段 */
    hmac_user_query_stats_stru query_stats;

    osal_u8 groupid;
    osal_u8 lut_index; /* user对应的硬件索引，活跃用户id */
    osal_u8 resv7[2]; /* 对齐预留2字节 */

    osal_u64 last_active_timestamp; /* user最后活跃时刻时间戳，user老化和活跃用户替换使用(使用OSAL提供的时间戳函数赋值) */

    /* 当前VAP工作在AP或STA模式，以下字段为user是STA或AP时公共字段，新添加字段请注意!!! */
    /* 发送tid缓存队列 */
    hmac_tid_stru tx_tid_queue[WLAN_TID_MAX_NUM];
    osal_void *alg_priv; /* 用户级别算法私有结构体 */
    osal_u16 partial_aid;
    osal_s16 rx_rssi; /* 用户接收RSSI统计量 */

    /* --------- Private STA成员 Start ------------------ */

    /* --------- Private STA成员 End ------------------ */

    /* --------- Private AP成员 Start ------------------ */
    /* 当前VAP工作在AP模式，以下字段为user是STA时独有字段，新添加字段请注意!!! */
    osal_u8 ps_mode      : 1; /* ap保留的用户节能模式，PSM用 */
    osal_u8 active_user  : 1; /* 是否活跃用户，用户管理用 */
    osal_u8 new_add_user : 1; /* 1: 是否是新创建用户(不执行通知算法)，0为其他状态需要执行算法通知，例如替换状态 */
    osal_u8 resv8        : 4;

    osal_u8 vip_flag              : 1; /* 只算法调度用，通过配置命令配置，TRUE: VIP用户, FALSE: 非VIP用户 */
    osal_u8 delete_ba_flag        : 1; /* 删除ba标志，只算法通过接口修改，autorate降协议模式时置为true */
    osal_u8 ptk_need_install      : 1; /* 需要更新单播秘钥 */
    osal_u8 is_rx_eapol_key_open  : 1; /* 记录接收的eapol-key 是否为加密 */
    osal_u8 eapol_key_4_4_tx_succ : 1; /* eapol-key 4/4 发送成功 */
    osal_u8 ptk_key_idx           : 3; /* 保存单播秘钥key_idx */

    osal_u8 last_frame_pn;

    /* 用户的节能结构,单播用户与组播用户都会用 */
    hmac_user_ps_stru ps_structure;
    /* --------- Private AP成员 End ------------------ */

    /* -->> Private End */

    /* -->> HAL Start */
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    hal_dyn_cali_usr_record_stru dyn_cali_pow[WLAN_RF_CHANNEL_NUMS];
#endif
    /* 用户级别功率信息结构体 */
    /* -->> HAL End */

    /* -->> Features Start */
    osal_u8 resv10[3];
    osal_u8 mgmt_frm_type;

    /* -->> Features End */

    /* -->> Features Array Start */
    osal_void **hmac_user_feature_arr;
    /* -->> Features Array Start */
    hmac_auth_re_rx_protect_stru    auth_re_rx_protect;       /* 用于重传帧过滤 */
#ifdef _PRE_WLAN_FEATURE_WPA3
    osal_u32 owe_ie_len;
    osal_u8 *owe_ie;
    osal_u16 auth_alg;
    osal_u8 resv11[2];
#endif
    osal_u8 _rom[4];
} hmac_user_stru;

/* ****************************************************************************
  9 OTHERS定义
**************************************************************************** */
/*****************************************************************************
    函数声明
*****************************************************************************/
osal_u32 hmac_user_add_wep_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key);
osal_u32 hmac_user_add_rsn_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key);
osal_u32 hmac_user_add_bip_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key);

wlan_priv_key_param_stru *hmac_user_get_key_etc(hmac_user_stru *hmac_user, osal_u8 key_id);

osal_void hmac_user_set_port_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 port_valid);

void hmac_user_avail_bf_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value);
void hmac_user_set_avail_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value);
void hmac_user_set_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value);
void hmac_user_set_bandwidth_cap_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 bandwidth_value);
void hmac_user_set_bandwidth_info_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 avail_bandwidth,
    wlan_bw_cap_enum_uint8 cur_bandwidth);
void  hmac_user_get_sta_cap_bandwidth_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 *bandwidth_cap);
osal_u32  mac_user_update_bandwidth(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 bwcap);
osal_u32  hmac_user_update_ap_bandwidth_cap(hmac_user_stru *hmac_user);
void hmac_user_set_avail_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 avail_protocol_mode);
void hmac_user_set_cur_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 cur_protocol_mode);
void hmac_user_set_cur_bandwidth_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8  cur_bandwidth);
void hmac_user_set_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 protocol_mode);
void hmac_user_set_asoc_state(hmac_user_stru *hmac_user, mac_user_asoc_state_enum_uint8 value);
void hmac_user_set_avail_op_rates_etc(hmac_user_stru *hmac_user, osal_u8 rs_nrates, osal_u8 *rs_rates);
void hmac_user_set_vht_hdl_etc(hmac_user_stru *hmac_user, mac_vht_hdl_stru *vht_hdl);
void hmac_user_get_vht_hdl_etc(hmac_user_stru *hmac_user, mac_vht_hdl_stru *vht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
void mac_user_set_he_hdl(hmac_user_stru *hmac_user, mac_he_hdl_stru *he_hdl);
void mac_user_get_he_hdl(hmac_user_stru *hmac_user, mac_he_hdl_stru *he_hdl);
#endif

void hmac_user_set_ht_hdl_etc(hmac_user_stru *hmac_user, mac_user_ht_hdl_stru *ht_hdl);
void hmac_user_get_ht_hdl_etc(hmac_user_stru *hmac_user, mac_user_ht_hdl_stru *ht_hdl);
void hmac_user_set_ht_capable_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 ht_capable);
#ifdef _PRE_WLAN_FEATURE_SMPS
void hmac_user_set_sm_power_save(hmac_user_stru *hmac_user, osal_u8 sm_power_save);
#endif
void hmac_user_set_pmf_active_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 pmf_active);
void hmac_user_set_qos_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 qos_mode);
void hmac_user_set_spectrum_mgmt_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 spectrum_mgmt);
void hmac_user_set_apsd_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 apsd);

osal_void hmac_user_init_key_etc(hmac_user_stru *hmac_user);

osal_void hmac_user_set_key_etc(hmac_user_stru *multiuser, wlan_cipher_key_type_enum_uint8 keytype,
    wlan_ciper_protocol_type_enum_uint8 ciphertype, osal_u8 keyid);

osal_u32 hmac_user_update_wep_key_etc(hmac_user_stru *hmac_user, osal_u16 multi_user_idx);
oal_bool_enum_uint8 hmac_addr_is_zero_etc(osal_u8 *mac);
oal_bool_enum_uint8 hmac_ip_is_zero_etc(osal_u8 *ip_addr);
osal_s32 hmac_user_sync(hmac_user_stru *hmac_user);

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 mac_user_update_bandwidth(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 bwcap);
osal_void hmac_user_set_assoc_id_etc(hmac_user_stru *hmac_user, osal_u16 assoc_id);

osal_void hmac_user_set_barker_preamble_mode_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 barker_preamble_mode);

osal_void *mac_res_get_mac_user_etc(osal_u16 idx);

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif
