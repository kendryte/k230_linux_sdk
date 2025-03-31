/*
 * Copyright (c) CompanyNameMagicTag 2015-2023. All rights reserved.
 * 文 件 名   : hmac_feature_dft.h
 * 生成日期   : 2015年5月20日
 * 功能描述   : hmac_arp_offload.c 的头文件
 */

#ifndef __HMAC_FEATURE_DFT_H__
#define __HMAC_FEATURE_DFT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "wal_ccpriv.h"
#else
#include "wal_linux_ccpriv.h"
#endif

#include "hmac_vap.h"
#ifdef _PRE_WLAN_DFT_STAT
#include "frw_hmac.h"
#endif
#include "common_dft.h"
#include "msg_dft_rom.h"
#include "hmac_uapsd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FEATURE_DFT_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#define HMAC_DFT_PHY_STAT_NODE_0_BASE 0           /* phy统计节点0的基准bit，寄存器PHY_STA_01_EN的bit0 */
#define HMAC_DFT_PHY_STAT_NODE_1_BASE 16          /* phy统计节点1的基准bit，寄存器PHY_STA_01_EN的bit16 */
#define HMAC_DFT_PHY_STAT_NODE_2_BASE 0           /* phy统计节点2的基准bit，寄存器PHY_STA_23_EN的bit0 */
#define HMAC_DFT_PHY_STAT_NODE_3_BASE 16          /* phy统计节点3的基准bit，寄存器PHY_STA_23_EN的bit16 */
#define HMAC_DFT_REPORT_VAP_STAT_TIMEOUT 2000     /* 上报vap吞吐相关统计信息周期 */
#define HMAC_DFT_PHY_INTLAN_GAIN_NUM 8            /* 内置LNA增益数目 */
#define HMAC_DFT_PHY_VGA_GAIN_NUM 32              /* VGA增益数目 */
#define HMAC_DFT_VAP_STAT_RATE_TO_KBPS 250        /* 速率单位是kbps，应该是字节数除以周期(2s)再除以1000在乘以8(B/b) */
#define HMAC_DFT_REG_OTA_LEN 1024 /* 寄存器OTA上报的最大限制 */
#define HMAC_DFT_VAP_AGGRE_REPORT_NUM 20          /* 聚合上报的数目 */

#define DEVICE_VAP_STAT_NUM (HMAC_PKTS_CNT_TYPE_BUTT - HOST_PKTS_CNT_TYPE_END)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* OTA上报的信息结构体定义(包括内存池，vap，user，队列等等)，将结构体统一定义在此处的目的是便于维护 */
/*****************************************************************************
  结构名  : mem_subpool_dft_stru
  结构说明: ota上报的子内存池结构体
*****************************************************************************/
typedef struct {
    osal_u8 sub_pool_id; /* 子内存池的id */
    osal_u8 auc_resv[3]; /* 保留3个字节用作对齐 */
    osal_u16 free_cnt;  /* 本子内存池可用内存块数 */
    osal_u16 total_cnt; /* 本子内存池内存块总数 */
} mem_subpool_dft_stru;

/* OTA上报的内存池使用情况的结构体，此结构体的修改需要通知SDT相关人员进行同步修改，否则会解析错误 */
typedef struct {
    osal_u8 pool_id;        /* 本内存池的pool id */
    osal_u8 subpool_cnt;    /* 本内存池一共有多少子内存池 */
    osal_u16 max_byte_len;  /* 本内存池可分配内存块最大长度 */
    osal_u16 mem_used_cnt;  /* 本内存池已用内存块 */
    osal_u16 mem_total_cnt; /* 本内存池一共有多少内存块 */

    /* 子内存池的信息 */
    mem_subpool_dft_stru subpool_table[WLAN_MEM_MAX_SUBPOOL_NUM];
} mem_pool_dft_stru;

/* OTA上报的硬件信息结构体，此结构体的修改需要通知SDT相关人员进行同步修改，否则会解析错误 */
typedef struct {
    hal_rx_dscr_queue_header_stru rx_dscr_queue[HAL_RX_QUEUE_NUM];
    hal_tx_dscr_queue_header_stru tx_dscr_queue[HAL_TX_QUEUE_NUM];
} hal_device_dft_stru;

/* OTA上报的用户队列的结构体，此结构体的修改需要通知SDT相关人员进行同步修改，否则会解析错误 */
typedef struct {
    hmac_tid_stru tx_tid_queue[WLAN_TID_MAX_NUM]; /* 发送tid缓存队列 */
    hmac_user_uapsd_stru uapsd_stru; /* 用户的U-APSD节能结构 */
} hmac_user_queue_info_dft_stru;

/* OTA上报的dmac vap结构体，此结构体的修改需要通知SDT相关人员进行同步修改，否则会解析错误 */
typedef struct {
    /* 以下元素对应于dmac_vap_stru结构中元素 */
    osal_u8 vap_id; /* vap ID   */       /* 即资源池索引值 */
    osal_u8 resv0[2]; /* 2 BYTE保留字段 */
    wlan_vap_mode_enum_uint8 vap_mode; /* vap模式  */
    osal_u32 core_id;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];     /* BSSID，非MAC地址，MAC地址是mib中的auc_dot11StationID  */
    mac_vap_state_enum_uint8 vap_state; /* VAP状态 */
    wlan_protocol_enum_uint8 protocol;  /* 工作的协议模式 */
    mac_channel_stru channel;           /* vap所在的信道 */
    osal_u8 has_user_bw_limit : 1;        /* 该vap是否存在user限速 */
    osal_u8 vap_bw_limit : 1;             /* 该vap是否已限速 */
    osal_u8 bit_resv : 6;
    osal_u8 tx_power;                    /* 传输功率, 单位dBm */
    wlan_p2p_mode_enum_uint8 p2p_mode; /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */
    osal_u8 resv1;         /* p2p go / cl的hal vap id */
    osal_u16 user_nums;                  /* VAP下已挂接的用户个数 */
    osal_u16 multi_user_idx;             /* 组播用户ID */
    mac_cap_flag_stru cap_flag;
    mac_protection_stru protection; /* 与保护相关变量 */

    /* 以下元素对应于dmac_vap_stru结构中元素 */
    hmac_vap_linkloss_stru linkloss_info;                /* linkloss机制相关信息 */
    hal_tx_txop_alg_stru tx_alg;                         /* 单播数据帧发送参数 */
    hal_tx_txop_alg_stru tx_data_mcast;                  /* 组播数据帧参数 */
    hal_tx_txop_alg_stru tx_data_bcast;                  /* 广播数据帧参数 */
    hal_tx_txop_alg_stru tx_mgmt_ucast[WLAN_BAND_BUTT];  /* 单播管理帧参数 */
    hal_tx_txop_alg_stru tx_mgmt_bmcast[WLAN_BAND_BUTT]; /* 组播、广播管理帧参数 */

    osal_u8 ps_user_num; /* 处于节能模式的用户的数目，AP模式有效 */
    osal_u8 dtim_count;
    osal_u8 uapsd_max_depth; /* U-APSD节能队列的最大深度 */
    osal_u8 resv;
} hmac_vap_dft_stru;

/* OTA上报的dmac user结构体，此结构体的修改需要通知SDT相关人员进行同步修改，否则会解析错误 */
typedef struct {
    /* 以下元素对应于dmac_user_stru结构中元素 */
    osal_u8 user_mac_addr[WLAN_MAC_ADDR_LEN]; /* user对应的MAC地址 */
    osal_u16 assoc_id;                        /* user对应资源池索引值;
                                               user为STA时，表示填在管理帧中的AID，值为用户的资源池索引值1~32(协议规定范围为1~2007) */
    oal_bool_enum_uint8 is_multi_user;
    osal_u8 vap_id;                                 /* vap ID */
    osal_u8 resv0[2]; /* 2 BYTE保留字段 */
    wlan_protocol_enum_uint8 protocol_mode;       /* 用户工作协议 */
    wlan_protocol_enum_uint8 avail_protocol_mode; /* 用户和VAP协议模式交集, 供算法调用 */
    wlan_protocol_enum_uint8 cur_protocol_mode;   /* 默认值与en_avail_protocol_mode值相同, 供算法调用修改 */

    wlan_nss_enum_uint8 avail_num_spatial_stream;    /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8 user_num_spatial_stream;     /* 用户支持的空间流个数 */
    wlan_nss_enum_uint8 avail_bf_num_spatial_stream; /* 用户支持的Beamforming空间流个数 */
    oal_bool_enum_uint8 port_valid;                  /* 802.1X端口合法性标识 */

    mac_rate_stru avail_op_rates;        /* 用户和VAP可用的11a/b/g速率交集，供算法调用 */
    mac_user_tx_param_stru user_tx_info; /* TX相关参数 */

    wlan_bw_cap_enum_uint8 bandwidth_cap;           /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 avail_bandwidth;         /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 cur_bandwidth;           /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    mac_user_asoc_state_enum_uint8 user_asoc_state; /* 用户关联状态 */

    mac_user_cap_info_stru cap_info; /* user基本能力信息位 */
    mac_user_ht_hdl_stru ht_hdl;     /* HT capability IE和 operation IE的解析信息 */
    mac_vht_hdl_stru vht_hdl;        /* VHT capability IE和 operation IE的解析信息 */

    mac_key_mgmt_stru key_info;

    mac_user_uapsd_status_stru uapsd_status; /* uapsd状态 */
    osal_u8 lut_index;                         /* user对应的硬件索引，活跃用户id */
    osal_u8 uapsd_flag;                        /* STA的U-APSD当前的处理状态 */
    osal_u8 max_key_index;                     /* 最大ptk index */
    osal_u8 resv;
} hmac_user_dft_stru;
#endif

typedef enum {
    DFT_STREAM_ID_ZERO = 0, /* 通用报文流id(内部产生报文、管理帧报文) */
    DFT_STREAM_ID_ONE,
    DFT_STREAM_ID_TWO,
    DFT_STREAM_ID_THREE,
    DFT_STREAM_TYPE_BUTT   /* 目前cb域中占用3个bit，最大可支持7条流,目前支持4条流，后续可配置 */
} dft_stream_id_enum;
typedef osal_u8 dft_stream_id_enum_u8;

typedef enum {
    HMAC_RX_PKTS_CNT_START = 0,
    /* 管理帧统计 */
    RX_M_SEND_WAL_SUCC_PKTS = HMAC_RX_PKTS_CNT_START,               /* HMAC发送管理帧给WAL成功 */
    RX_M_SEND_WAL_FAIL_PKTS,               /* HMAC发送管理帧给WAL失败 */

    RX_M_SEND_LAN_SUCC_PKTS,               /* WAL发送管理帧给内核成功 */
    RX_M_SEND_LAN_FAIL_PKTS,               /* WAL发送管理帧给内核失败 */

    RX_M_FROM_DEVICE_PKTS,             /* 接收到的管理帧和控制帧数目 */

    RX_C_FROM_DEVICE_PKTS,              /* 控制报文数目 */

    /* 数据帧统计 */
    RX_D_FROM_DEVICE_PKTS,          /* 驱动接收数据包数目 */

    RX_D_SEND_LAN_SUCC,                    /* HMAC发送数据帧给LAN数目 */
    RX_D_SEND_LAN_FAIL,                    /* HMAC发送数据帧给LAN数目 */

    /***************************************************************************
                             发送包统计
    ***************************************************************************/
    /* 管理帧统计 */
    TX_M_FROM_KERNEL_PKTS,                 /* WAL接收协议栈发来的管理帧数目 */

    TX_M_SEND_HMAC_SUCC_PKTS,              /* WAL管理帧和控制帧发送给HMAC成功数目 */
    TX_M_SEND_HMAC_FAIL_PKTS,              /* WAL管理帧和控制帧发送给HMAC失败数目 */

    TX_M_FROM_WAL_PKTS,                    /* HMAC接收WAL发送过来的管理帧和控制帧数目 */

    TX_M_SEND_DEVICE_SUCC,                 /* 管理帧发送给device成功 */
    TX_M_SEND_DEVICE_FAIL,                 /* 管理帧发送给device失败 */

    /* 数据帧统计 */
    TX_D_FROM_LAN_PKTS,                    /* 收到LAN数据帧数目 */

    TX_D_SEND_DEVICE_SUCC,                 /* 数据帧发送给device成功 */
    TX_D_SEND_DEVICE_FAIL,                 /* 数据帧发送给device失败 */

    HOST_PKTS_CNT_TYPE_END,
    /***************************************************************************
                                接收包统计
    ***************************************************************************/
    /* 数据帧统计 */
    RX_DEVICE_D_FROM_HW_MPDU = HOST_PKTS_CNT_TYPE_END,              /* DMAC从芯片接收到的MPDU数目 */
    RX_DEVICE_D_SEND_HMAC_SUCC,            /* DMAC发送给HMAC的成功NETBUF数目 */

    /* 管理帧统计 */
    RX_DEVICE_M_FROM_HW_MPDU,              /* 接收到的管理帧和控制帧数目 */
    RX_DEVICE_M_SEND_HMAC_SUCC,            /* 接收到的管理帧和控制帧发送给HMAC时成功数目 */

    /***************************************************************************
                                发送包统计
    ***************************************************************************/
    /* 数据帧统计 */
    TX_DEVICE_D_FROM_HMAC_MPDU,                   /* DMAC接收到HMAC的数据帧数目 4 */
    TX_DEVICE_D_FROM_INNER,                       /* 内部产生数据报文 */
    TX_DEVICE_D_COMPLETE_SUCC,                    /* 数据帧发送成功 */

    /* 管理帧统计 */
    TX_DEVICE_M_FROM_HMAC_MPDU,                    /* DMAC收到HMAC侧发送的管理帧数目 11 */
    TX_DEVICE_M_FROM_INNER,                        /* 内部产生管理报文 */
    TX_DEVICE_M_COMPLETE_SUCC,                     /* 管理帧发送成功 */

    HMAC_PKTS_CNT_TYPE_BUTT
} dft_hmac_vap_stat_enum;

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
typedef struct {
    /* 硬件RX 接收的报文统计 */
    hal_mac_rx_mpdu_statis_info_stru mac_rx_statis;
    osal_u32 rx_from_hw_mpdu_cnt;            /* DMAC收到hw发送的mpdu报文个数 */
    /* 硬件TX 接收的报文统计 */
    hal_mac_tx_mpdu_statis_info_stru mac_tx_statis;
    osal_u32 tx_from_dmac_mpdu_cnt;          /* DMAC挂硬件队列发送mpdu报文个数 */
    osal_u32 tx_complete_mpdu_cnt;           /* 硬件上报发送完成中断mpdu数量 */
} dft_hw_statis_cnt_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_hw_stat_display(osal_void);
osal_void hmac_vap_stat_rx_display(osal_u8 vap_id);
osal_void hmac_vap_stat_tx_display(osal_u8 vap_id);
osal_u32 hmac_dft_vap_stat_deinit(osal_u8 vap_id);
osal_u32 hmac_dft_vap_stat_init(osal_u8 vap_id);
osal_s32 hmac_config_vap_stat_enable(dft_vap_statis_command_stru *vap_statis_command);
osal_void hmac_pkts_tx_stat(osal_u8 vap_id, osal_u32 msdu_num, dft_hmac_vap_stat_enum stat_type);
osal_void hmac_pkts_rx_stat(osal_u8 vap_id, osal_u32 msdu_num, dft_hmac_vap_stat_enum stat_type);
osal_void hmac_pkts_rx_stat_by_netbuflist(osal_u8 vap_id, oal_netbuf_stru *netbuf, osal_u16 netbuf_nums,
    dft_hmac_vap_stat_enum stat_type);
osal_u32 hmac_dft_rx_get_dscr_msdu(oal_netbuf_stru *netbuf, osal_u32 netbuf_num);

osal_void hmac_dft_report_linkloss_info(const hmac_vap_stru *hmac_sta, const hmac_device_stru *hmac_device);
osal_u32 hmac_dft_start_report_vap_stat(hmac_vap_stru *hmac_vap);
osal_u32 hmac_dft_stop_report_vap_stat(hmac_vap_stru *hmac_vap);
osal_u32 hmac_dft_clear_usr_queue_stat(hmac_user_stru *hmac_user);
osal_u32 hmac_dft_report_usr_queue_stat(hmac_user_stru *hmac_user);
osal_s32 hmac_dft_get_vap_stat(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_dft_tx_get_dscr_msdu(oal_netbuf_stru *netbuf, osal_u32 net_buff_num);
osal_s32 hmac_config_aggre_info_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_get_vap_stat(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_get_sniffer_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_start_sniffer_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_hw_stat_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_get_hw_stat(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_get_vap_stat_new(dft_vap_statis_command_stru *vap_statis_command);

osal_u32 hmac_dft_report_all_ampdu_stat(hmac_user_stru *hmac_user, osal_u8 param);
osal_u32 hmac_dft_report_ampdu_stat(hmac_tid_stru *tid, osal_u8 macaddr[], osal_u8 param);
osal_s32 hmac_query_tx_vap_aggre_info(osal_u8 vap_id);

#ifdef _PRE_WLAN_DFT_STAT
osal_void hmac_dft_print_drop_frame_info(osal_u32 file_id, osal_u16 line_num,  osal_u32 pkt_num,
    oal_netbuf_stru *netbuf);
#else
static inline osal_void hmac_dft_print_drop_frame_info(osal_u32 file_id, osal_u16 line_num, osal_u32 pkt_num,
    oal_netbuf_stru *netbuf)
{
    unref_param(file_id);
    unref_param(line_num);
    unref_param(pkt_num);
    unref_param(netbuf);
    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_arp_offload.h */
