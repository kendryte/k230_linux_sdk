/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Header file of the DMAC external public interface.
 * Create: 2022-2-19
 */

#ifndef __DMAC_EXT_IF_DEVICE_ROM_H__
#define __DMAC_EXT_IF_DEVICE_ROM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "osal_types.h"
#include "oal_types_device_rom.h"
#include "dmac_ext_if_type_rom.h"
#include "wlan_types.h"
#include "hal_common_ops_device_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_BA_SEQNO_MASK 0x0FFF /* max sequece number */
#define DMAC_BA_MAX_SEQNO_BY_TWO 2048
#define DMAC_BA_BMP_SIZE 64

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* Type of Tx Descriptor status */
typedef enum {
    DMAC_TX_INVALID = 0,     /* 无效 */
    DMAC_TX_SUCC,            /* 成功 */
    DMAC_TX_FAIL,            /* 发送失败（超过重传限制：接收响应帧超时） */
    DMAC_TX_TIMEOUT,         /* lifetime超时（没法送出去） */
    DMAC_TX_RTS_FAIL,        /* RTS 发送失败（超出重传限制：接收cts超时） */
    DMAC_TX_NOT_COMPRASS_BA, /* 收到的BA是非压缩块确认 */
    DMAC_TX_TID_MISMATCH,    /* 收到的BA中TID与发送时填写在描述符中的TID不一致 */
    DMAC_TX_KEY_SEARCH_FAIL, /* Key search failed */
    DMAC_TX_AMPDU_MISMATCH,  /* 描述符异常 */
    DMAC_TX_PENDING,         /* 02:没有中断均为pending;03:发送过程中为pending */
    DMAC_TX_FAIL_RESV,       /* resv */
    DMAC_TX_FAIL_BW_TOO_BIG, /* 带宽超过PHY的最大工作带宽或流数超过最大天线数， 软件回收该帧 */
    DMAC_TX_FAIL_ABORT,                  /* 发送失败（因为abort） */
    DMAC_TX_FAIL_STATEMACHINE_PHY_ERROR, /* MAC发送该帧异常结束（状态机超时、phy提前结束等原因） */
    DMAC_TX_SOFT_PSM_BACK,               /* 软件节能回退 */
    DMAC_TX_AMPDU_BITMAP_MISMATCH       /* 硬件解析bitmap，当前mpdu未被确认 */
} dmac_tx_dscr_status_enum;
typedef osal_u8 dmac_tx_dscr_status_enum_uint8;

/* cb字段action帧子类型枚举定义 */
typedef enum {
    WLAN_ACTION_BA_ADDBA_REQ = 0, /* 聚合action */
    WLAN_ACTION_BA_ADDBA_RSP,
    WLAN_ACTION_BA_DELBA,
    WLAN_ACTION_SMPS_FRAME_SUBTYPE,   /* SMPS节能action */
    WLAN_ACTION_OPMODE_FRAME_SUBTYPE, /* 工作模式通知action */
    WLAN_ACTION_P2PGO_FRAME_SUBTYPE,  /* host发送的P2P go帧，主要是discoverability request */
    WLAN_ACTION_TWT_SETUP_REQ,
    WLAN_ACTION_TWT_TEARDOWN_REQ,
    WLAN_ACTION_TWT_INFORMATION_REQ,

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    WLAN_ACTION_WUR_MODE_SETUP, /* WUR Mode Setup帧 */
    WLAN_ACTION_WUR_MODE_TEARDOWN, /* WUR Mode Teardown帧 */
#endif

    WLAN_FRAME_TYPE_ACTION_BUTT
} wlan_cb_action_subtype_enum;
typedef osal_u8 wlan_cb_frame_subtype_enum_uint8;

typedef hal_rx_ctl_stru mac_rx_ctl_stru;

/* DMAC模块接收流程控制信息结构，存放于对应的netbuf的CB字段中，最大值为48字节,
   如果修改，一定要通知sdt同步修改，否则解析会有错误!!!!!!!!!!!!!!!!!!!!!!!!! */
typedef struct {
    hal_rx_ctl_stru rx_info;            /* dmac需要传给hmac的数据信息 20 bytes */
    hal_rx_status_stru rx_status;       /* 保存加密类型及帧长信息 4 bytes */
    hal_rx_statistic_stru rx_statistic; /* 保存接收描述符的统计信息     8 bytes */
} dmac_rx_ctl_stru;

/* 1字节对齐 */
#pragma pack(push, 1)
typedef struct {
    mac_ieee80211_frame_stru *frame_header;     /* 802.11帧头 */
    osal_u16 seqnum;                           /* SN */
    wlan_frame_type_enum_uint8 frame_type;   /* 管理帧/控制帧/数据帧  */
    osal_u8 mac_head_type_80211 : 1; /*  */
    osal_u8 res : 7;
} mac_tx_expand_cb_stru;

/* 裸系统cb字段 只有20字节可用, 当前使用19字节; HCC[8]+PAD[1]+CB[19]+MAC HEAD[36] */
struct mac_tx_ctl {
    /* byte1-2 */
    wlan_cb_frame_type_enum_uint8 frame_type;       /* 自定义帧类型 */
    wlan_cb_frame_subtype_enum_uint8 frame_subtype; /* 自定义帧子类型 */
    /* byte3 */
    osal_u8 mpdu_num : 7;                           /* ampdu中包含的MPDU个数,实际描述符填写的值为此值-1 */
    osal_u8 netbuf_num : 1;                         /* 每个MPDU占用的netbuff数目,在每个MPDU的第一个NETBUF中有效 */
    /* byte4-5 */
    osal_u16 mpdu_payload_len;                      /* 每个MPDU的长度不包括mac header length */
    /* byte6 */
    osal_u8 frame_header_length : 6;                /* 51四地址32,该MPDU的802.11头长度 */
    osal_u8 is_amsdu : 1;                           /* 是否AMSDU: FALSE不是，TRUE是 */
    osal_u8 is_first_msdu : 1;                      /* 是否是第一个子帧，FALSE不是 TRUE是 */
    /* byte7 */
    osal_u8 tid : 4;                                /* dmac tx 到 tx complete 传递的user结构体，目标用户地址 */
    wlan_wme_ac_type_enum_uint8 ac : 3;             /* ac */
    osal_u8 ismcast : 1;                            /* 该MPDU是单播还是多播:FALSE为单播，TRUE为多播 */
    /* byte8 */
    osal_u8 retried_num : 4;                        /* 重传次数 */
    osal_u8 mgmt_frame_id : 4;                      /* wpas 发送管理帧的frame id */
    /* byte9 */
    osal_u8 tx_user_idx : 6;                       /* 比描述符中userindex多一个bit用于标识无效index */
    osal_u8 roam_data : 1;                         /* 漫游期间帧发送标记 */
    osal_u8 is_get_from_ps_queue : 1;              /* 节能特性用，标识一个MPDU是否从节能队列中取出来的 */
    /* byte10 */
    osal_u8 tx_vap_index : 3;                      /* dmac_vap id */
    wlan_tx_ack_policy_enum_uint8 ack_policy : 3;
    osal_u8 is_needretry : 1;
    osal_u8 need_rsp : 1;                          /* WPAS send mgmt,need dmac response tx status */
    /* byte11 */
    osal_u8 is_eapol_key_ptk : 1;                       /* 4 次握手过程中设置单播密钥EAPOL KEY 帧标识 */
    osal_u8 is_m2u_data : 1;                            /* 是否是组播转单播的数据 */
    osal_u8 is_large_skb_amsdu : 1;                     /* 是否是多子帧大包聚合 */
    osal_u8 ether_head_including : 1;                   /* offload下netbuf头部LLC之前是否有etherhead */
    osal_u8 use_4_addr : 1;                             /* 是否使用4地址，由WDS特性决定 */
    osal_u8 resrv_6 : 3;
    /* byte12-13 */
    osal_u8 alg_pktno;                             /* 算法用到的字段，唯一标示该报文 */
    osal_u8 alg_frame_tag;                         /* 用于算法对帧进行标记 */
    /* byte14 */
    osal_u8 align_padding_offset : 2;
    osal_u8 is_tcp_ack : 1;                        /* 用于标记tcp ack */
    osal_u8 is_bcast : 1;
    osal_u8 is_wur : 3;                            /* 标记是否是WUR帧和具体类型 */
    osal_u8 is_fake_buffer : 1;                    /* 标记报文是否从fake队列中来 */
    /* byte15 */
    osal_u8 msdu_num;                              /* lowdelay 当前layer的总包数 */
    /* byte16 ~ byte19 */
    osal_u32 latency_timestamp_us : 24;            /* 时延耗时统计时间戳，单位1us精度 */
    osal_u32 latency_index : 8;                    /* 时延耗时统计报文标识 */
    /* byte20 ~ byte23 */
    osal_u16 partial_aid : 9,
             ax_bss_color : 6,
             txop_ps_not_allowed : 1;
    osal_u8  tx_hal_vap_index : 2,
             ra_lut_index : 3,
             lsig_txop : 1,
             slp_frame_id : 2;
    osal_u8 retry_flag_hw_bypass : 1,
            duration_hw_bypass : 1,
            timestamp_hw_bypass : 1,
            tx_pn_hw_bypass : 1,
            is_custom_rts : 1,      /* 是否为自定义RTS报文 */
            is_custom : 1,          /* 是否为自定义任意报文 */
            resrv_2 : 2;
    /* byte24 ~ byte27 */
    osal_u8 cipher_key_type;      /* 密钥ID/类型 */
    osal_u8 cipher_protocol_type;
    osal_u8 cipher_key_id;
    osal_u8 resrv_3;
    /* byte28 ~ byte31 */
    osal_u8 ftm_protocol_mode : 4,     /* ftm */
            ftm_bandwidth : 4;
    osal_u8 ftm_cali_en : 1,
            is_ftm : 2,
            ftm_chain : 2,
            resrv_4 : 3;
    osal_u8 rssi_level : 2,    /* tpc */
            tpc_code_is_valid : 1,    /* 判断tpc_code是否有效 */
            probe_data_type : 4,    /* 是否探测帧 */
            tx_tuple_opt_meet : 1;
    osal_u8 tpc_code;    /* 目前用于设置wifi aware action帧功率 */

    /* byte32 */
    osal_u8 al_rx_tid_type;  /* 常收模式tid类型 */

    /* byte33 ~ byte40, expand_cb只在HMAC侧用，不传输到DMAC侧 */
    mac_tx_expand_cb_stru             expand_cb;
};
typedef struct mac_tx_ctl mac_tx_ctl_stru;
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of dmac_ext_if_device.h */
