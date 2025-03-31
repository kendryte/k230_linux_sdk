 /*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: OAM struct define
 */

#ifndef __OAM_STRUCT_H__
#define __OAM_STRUCT_H__

#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "common_dft.h"

/* 统计相关宏 */
#define OAM_MAC_ERROR_TYPE_CNT 25
#define OAM_RX_DSCR_QUEUE_CNT 2
#define OAM_IRQ_FREQ_STAT_MEMORY 50

#define OAM_TID_TRACK_NUM 4

/* 每一个事件队列中事件个数最大值 */
#define OAM_MAX_EVENT_NUM_IN_EVENT_QUEUE 8

/* 日志结构体相关宏，用于oam_log_info_stru */
#define OAM_FUNC_NAME_MAX_LEN 48         /* 函数名的最大长度 */
#define OAM_LOG_INFO_MAX_LEN 100         /* 日志信息最大长度 */
#define OAM_LOG_INFO_IN_MEM_MAX_NUM 5000 /* oam模块最多保存5000条log信息，超过以后从头自动覆盖 */

/* 消息上报给SDT封装的结构体 */
/* EVENT消息体最大长度 */
#define OAM_EVENT_INFO_MAX_LEN 48

/* VAP级别统计信息结构 */
typedef struct {
    /* **************************************************************************
                                接收包统计
    ************************************************************************** */

    /* 发往lan的数据包统计 */
    osal_u32 rx_pkt_to_lan;   /* 接收流程发往以太网的数据包数目，MSDU */
    osal_u32 rx_bytes_to_lan; /* 接收流程发往以太网的字节数 */

    /* MPDU级别进行处理时发生错误释放MPDU个数统计 */
    osal_u32 rx_da_check_dropped;        /* 检查目的端地址异常，释放一个MPDU */
    osal_u32 rx_no_buff_dropped;              /* 组播帧或者wlan_to_wlan流程申请buff失败 */
    osal_u32 rx_defrag_process_dropped;       /* 去分片处理失败 */

    /* 接收到组播帧个数 */
    osal_u32 rx_mcast_cnt;

    /* **************************************************************************
                                发送包统计
    ************************************************************************** */

    /* 从lan接收到的数据包统计 */
    osal_u32 tx_pkt_num_from_lan; /* 从lan过来的包数目,MSDU */
    osal_u32 tx_bytes_from_lan;   /* 从lan过来的字节数 */

    /* 发送流程发生异常导致释放的数据包统计，MSDU级别 */
    osal_u32 tx_abnormal_msdu_dropped; /* 异常情况释放MSDU个数，指vap或者user为空等异常 */
    osal_u32 tx_security_check_faild;  /* 安全检查失败释放MSDU */

    /* 组播转单播发送流程统计 */
    osal_u32 tx_m2u_mcast_cnt;    /* 组播转单播 组播发送成功个数 */
} oam_vap_stat_info_stru;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* USER级别统计信息结构 */
typedef struct {
    /* 接收包统计，从某个用户处接收到的包统计(TO DS) */
    osal_u32 rx_mpdu_num;   /* 从某个用户接收到的MPDU数目 */
    osal_u32 rx_mpdu_bytes; /* 某个用户接收到的MPDU字节数 */

    /* 发送包统计，发送给某个用户的包统计(FROM DS)，粒度是TID(具体到从VAP的某个TID发出) */
    osal_u32 tx_mpdu_succ_num[WLAN_TIDNO_BUTT];      /* 发送MPDU总个数 */
    osal_u32 tx_mpdu_bytes[WLAN_TIDNO_BUTT];         /* 发送MPDU总字节数 */
    osal_u32 tx_mpdu_fail_num[WLAN_TIDNO_BUTT];      /* 发送MPDU失败个数 */
    osal_u32 tx_ampdu_bytes[WLAN_TIDNO_BUTT];        /* 发送AMPDU总字节数 */
    osal_u32 tx_mpdu_in_ampdu[WLAN_TIDNO_BUTT];      /* 属于AMPDU的MPDU发送总个数 */
    osal_u32 tx_mpdu_fail_in_ampdu[WLAN_TIDNO_BUTT]; /* 属于AMPDU的MPDU发送失败个数 */
    osal_u32 tx_ppdu_retries;                         /* 空口重传的PPDU计数 */
} oam_user_stat_info_stru;

/* 统计结构信息，包括设备级别，vap级别，user级别的各种统计信息 */
typedef struct {
    /* VAP级别的统计信息 */
    oam_vap_stat_info_stru vap_stat_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

    /* USER级别的统计信息 */
    oam_user_stat_info_stru user_stat_info[WLAN_USER_MAX_USER_LIMIT];
} oam_stat_info_stru;
#else
/* 统计结构信息，包括vap级别的统计信息 */
typedef struct {
    /* VAP级别的统计信息 */
    oam_vap_stat_info_stru vap_stat_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
} oam_stat_info_stru;
#endif

#define OAM_STAT_VAP_INCR(_uc_vap_id, _member, _num) do { \
            if (_uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)          \
            {                                                   \
                g_st_stat_info_etc.vap_stat_info[_uc_vap_id]._member += (_num);   \
            }                                                                               \
        } while (0)

#define OAM_STAT_GET_STAT_ALL() (&g_st_stat_info_etc)

extern oam_stat_info_stru g_st_stat_info_etc;
#define OAM_IO_PRINTK(fmt, ...)

/* 字符串以0结尾上报，实际字符串内容最大长度 */
#define WLAN_SDT_NETBUF_MAX_PAYLOAD         1600
#define OAM_REPORT_MAX_STRING_LEN (WLAN_SDT_NETBUF_MAX_PAYLOAD - 1) /* 以\0结束 */

#endif
