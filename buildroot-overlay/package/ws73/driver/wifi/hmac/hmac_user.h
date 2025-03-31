/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_user.h
 * 生成日期   : 2012年10月19日
 * 功能描述   : hmac_user.c 的头文件
 */


#ifndef __HMAC_USER_H__
#define __HMAC_USER_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_ext_if.h"
#include "mac_resource_ext.h"
#include "hmac_tid.h"
#include "frw_hmac.h"
#include "msg_user_manage_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_USER_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_ADDBA_EXCHANGE_ATTEMPTS        10  /* 试图建立ADDBA会话的最大允许次数 */

#define hmac_user_is_amsdu_support(_user, _tid)         (((_user)->amsdu_supported) & (0x01 << ((_tid) & 0x07)))

#define hmac_user_set_amsdu_support(_user, _tid)        (((_user)->amsdu_supported) |= (0x01 << ((_tid) & 0x07)))

#define hmac_user_set_amsdu_not_support(_user, _tid) \
    (((_user)->amsdu_supported) &= (osal_u8)(~(0x01 << ((_tid) & 0x07))))

#define hmac_user_stats_pkt_incr(_member, _cnt)          ((_member) += (_cnt))

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

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

/* SA Query 超时定时器 和 间隔定时器 的超时函数入参结构 */
typedef struct {
    hmac_vap_stru     *hmac_vap;                  /* 发送SA Query request的mac vap信息 */
    hmac_user_stru   *hmac_user;                /* 目标user */
} hmac_maxtimeout_timer_stru;
typedef struct {
    hmac_vap_stru       *hmac_vap;                  /* 发送SA Query request的mac vap信息 */
    hmac_user_stru     *hmac_user;                /* 目标user */
    oal_bool_enum_uint8 is_protected;              /* SA Query管理帧加密的使能开关 */
    osal_u16          trans_id;                  /* SA Query request帧的trans id */
} hmac_interval_timer_stru;

/* 存储AP关联请求帧的ie信息，用于上报内核 */
typedef struct {
    osal_u32                      assoc_req_ie_len;
    osal_u8                      *assoc_req_ie_buff;
    osal_u8                       user_mac_addr[WLAN_MAC_ADDR_LEN];
} hmac_asoc_user_req_ie_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_user_ht_support
 功能描述  : 是否为HT用户
 返 回 值  : OAL_TRUE是，OAL_FALSE不是
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_ht_support(const hmac_user_stru *hmac_user)
{
    if (hmac_user->ht_hdl.ht_capable == OAL_TRUE) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_user_xht_support
 功能描述  : 是否支持ht/vht聚合
 输入参数  : hmac_user_stru *hmac_user
 返 回 值  : OAL_STATIC OAL_INLINE oal_bool_enum_uint8
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_user_xht_support(const hmac_user_stru *hmac_user)
{
    if ((hmac_user->cur_protocol_mode >= WLAN_HT_MODE) &&
        (hmac_user->cur_protocol_mode < WLAN_PROTOCOL_BUTT)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_user_set_asoc_req_ie(hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u32 len, osal_u8 reass_flag);
extern osal_u32 hmac_user_free_asoc_req_ie(hmac_user_stru *hmac_user);
extern osal_u32 hmac_user_set_avail_num_space_stream_etc(hmac_user_stru *hmac_user,
    wlan_nss_enum_uint8 vap_nss);
extern osal_u32 hmac_user_del_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32 hmac_user_add_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, osal_u16 *pus_user_index);
extern osal_u32 hmac_user_add_multi_user_etc(hmac_vap_stru *hmac_vap, osal_u16 *pus_user_index);
extern osal_u32 hmac_user_del_multi_user_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_user_add_notify_alg_etc(hmac_vap_stru *hmac_vap, osal_u16 user_idx);
extern osal_u32 hmac_update_user_last_active_time(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
extern osal_void hmac_tid_clear_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_bool lock_free_flag);
extern hmac_user_stru *mac_res_get_hmac_user_etc(osal_u16 idx);
extern hmac_user_stru *mac_vap_get_hmac_user_by_addr_etc(hmac_vap_stru *hmac_vap, const osal_u8 *mac_addr);
extern osal_u32 mac_user_init_etc(hmac_user_stru *hmac_user, osal_u16 user_idx, osal_u8 *mac_addr,
    hmac_vap_stru *hmac_vap);

// from hmac_user_rom.h
/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define HMAC_COMPATIBILITY_PKT_NUM_LIMIT 2000
#define HMAC_INVALID_USER_LUT_INDEX (WLAN_ACTIVE_USER_MAX_NUM) /* 无效的user lut index */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_USER_PS_STATE_ACTIVE = 0,
    HMAC_USER_PS_STATE_DOZE = 1,

    HMAC_USER_PS_STATE_BUTT
} hmac_user_ps_state_enum;
typedef osal_u8 hmac_user_ps_state_enum_uint8;

typedef enum {
    HMAC_USER_STATE_PS,
    HMAC_USER_STATE_ACTIVE,

    HMAC_USER_STATE_BUTT
} hmac_user_state_enum;
typedef osal_u8 hmac_user_state_enum_uint8;

typedef enum {
    PSM_QUEUE_TYPE_NORMAL,
    PSM_QUEUE_TYPE_IMPORTANT,

    PSM_QUEUE_TYPE_BUTT
} psm_queue_type_enum;
typedef osal_u8 psm_queue_type_enum_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u32 uapsd_tx_enqueue_count;       /* hmac_uapsd_tx_enqueue调用次数 */
    osal_u32 uapsd_tx_dequeue_count;       /* 出队帧个数统计 */
    osal_u32 uapsd_tx_enqueue_free_count;  /* 入队过程中MPDU被释放的次数，一次可能释放多个MPDU */
    osal_u32 uapsd_rx_trigger_in_sp;       /* trigger检查发现处于SP中的次数 */
    osal_u32 uapsd_rx_trigger_state_trans; /* trigger检查中发现需要状态转换的次数 */
    osal_u32 uapsd_rx_trigger_dup_seq;     /* trigger帧是重复帧的个数 */
    osal_u32 uapsd_send_qosnull;           /* 队列为空，发送qos null data的个数 */
    osal_u32 uapsd_send_extra_qosnull;     /* 最后一个为管理帧，发送额外qosnull的个数 */
    osal_u32 uapsd_process_queue_error;    /* 队列处理过程中出错的次数 */
    osal_u32 uapsd_flush_queue_error;      /* flush队列处理过程中出错的次数 */
    osal_u32 uapsd_send_qosnull_fail;      /* 发送qosnull失败次数 */
} hmac_usr_uapsd_statis_stru;

typedef struct {
    osal_u32 psm_enqueue_succ_cnt;      /* psm队列成功入队帧个数 */
    osal_u32 psm_enqueue_fail_cnt;      /* psm队列入队失败被释放的帧个数 */
    osal_u32 psm_dequeue_fail_cnt;      /* psm队列出队失败的帧个数 */
    osal_u32 psm_dequeue_succ_cnt;      /* psm队列出队成功的帧个数 */
    osal_u32 psm_send_data_fail_cnt;    /* psm队列出队的数据帧发送失败个数 */
    osal_u32 psm_send_mgmt_fail_cnt;    /* psm队列出队的管理帧发送失败个数 */
    osal_u32 psm_send_null_fail_cnt;    /* AP发送null data失败的次数 */
    osal_u32 psm_resv_pspoll_send_null; /* AP收到用户的pspoll，但是队列中没有缓存帧的次数 */
    osal_u32 psm_rsp_pspoll_succ_cnt;   /* AP收到用户的pspoll，发送缓存帧成功的次数 */
    osal_u32 psm_rsp_pspoll_fail_cnt;   /* AP收到用户的pspoll，发送缓存帧失败的次数 */
} hmac_user_psm_stats_stru;


/* 软件平均rssi统计信息结构体 */
typedef struct {
    osal_s32 tx_rssi;            /* 记录ACK RSSI的累计值 */
    osal_s32 rx_rssi;            /* 记录接收RSSI的累计值 */
    osal_u16 tx_rssi_stat_count; /* 发送平均rssi统计的发包数目 */
    osal_u16 rx_rssi_stat_count; /* 接收平均rssi统计的发包数目 */
} hmac_rssi_stat_info_stru;

/* 软件平均速率统计信息结构体 */
typedef struct {
    osal_u32 tx_rate;            /* 记录发送速率的累计值 */
    osal_u32 rx_rate;            /* 记录接收速率的累计值 */
    osal_u16 tx_rate_stat_count; /* 发送平均rate统计的发包数目 */
    osal_u16 rx_rate_stat_count; /* 接收平均rate统计的发包数目 */
} hmac_rate_stat_info_stru;

typedef struct {
    /* 用户基本信息 */
    wlan_protocol_enum_uint8 protocol;            /* 协议模式 */
    hal_channel_assemble_enum_uint8 en_bandwidth; /* 工作带宽 */

    /* 算法用到的各个标志位 */
    oal_bool_enum_uint8 hmac_rssi_stat_flag; /* 是否进行平均rssi统计 */
    oal_bool_enum_uint8 hmac_rate_stat_flag; /* 是否进行平均速率统计 */

    hmac_rssi_stat_info_stru dmac_rssi_stat_info; /* 软件平均rssi统计信息结构体指针 */
    hmac_rate_stat_info_stru dmac_rate_stat_info; /* 软件平均速率统计信息结构体指针 */
} hmac_user_rate_info_stru;

typedef osal_u32 (*hmac_user_keepalive_timer_proc_cb)(osal_void *arg);
/*****************************************************************************
  8 内联函数定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_get_user_support_vht
 功能描述  : 获取vht_capable字段的值
*****************************************************************************/
static INLINE__ osal_u8 hmac_get_user_support_vht(const hmac_user_stru *user_info)
{
    return ((user_info->vht_hdl.vht_capable == 1) ? OSAL_TRUE : OSAL_FALSE);
}

/*****************************************************************************
 函 数 名  : hmac_get_user_support_ht
 功能描述  : 获取ht_capable字段的值
*****************************************************************************/
static INLINE__ osal_u8 hmac_get_user_support_ht(const hmac_user_stru *user_info)
{
    return ((user_info->ht_hdl.ht_capable == 1) ? OSAL_TRUE : OSAL_FALSE);
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_user_set_lut_info(hal_to_dmac_device_stru *hal_device, osal_u8 lut_index, osal_u8 *mac_addr);
osal_void hmac_user_del_lut_info(osal_u8 lut_index);
osal_u32 hmac_user_set_bandwith_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    wlan_bw_cap_enum_uint8 bw_cap);
osal_u32 hmac_user_add_multi_user(hmac_vap_stru *hmac_vap, osal_u16 multi_user_idx);
osal_s32 hmac_user_add_notify_alg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, frw_msg *msg);

osal_void hmac_user_del(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_ap_type_enum_uint8 ap_type);
osal_void hmac_user_key_search_fail_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ieee80211_frame_stru *frame_hdr);
osal_void hmac_user_pause(hmac_user_stru *hmac_user);
osal_u32 hmac_user_resume(hmac_user_stru *hmac_user);
osal_void hmac_user_init_slottime(const hmac_user_stru *hmac_user);
osal_u32 hmac_user_get_tid_by_num(hmac_user_stru *hmac_user, osal_u8 tid_num, hmac_tid_stru **tid_queue);

osal_void hmac_ap_pause_all_user(const hmac_vap_stru *hmac_vap);
osal_void hmac_rx_compatibility_identify(hmac_user_stru *hmac_user, osal_u32 rate_kbps);
osal_void hmac_rx_compatibility_show_stat(hmac_user_stru *hmac_user);
oal_bool_enum_uint8 hmac_psm_is_psm_empty(hmac_user_stru *hmac_user);
oal_bool_enum_uint8 hmac_psm_is_tid_empty(const hmac_user_stru *hmac_user);
osal_u32 hmac_psm_tid_mpdu_num(const hmac_user_stru *hmac_user);
osal_void hmac_ap_resume_all_user(hmac_vap_stru *hmac_vap);
osal_void hmac_user_ps_queue_overrun_notify(const hmac_vap_stru *hmac_vap);

osal_s32 hmac_user_sync(hmac_user_stru *hmac_user);
osal_void hmac_user_use_cnt_dec(hmac_user_stru *hmac_user);
osal_void hmac_user_use_cnt_inc(osal_u16 user_idx, hmac_user_stru **user);
osal_u32 hmac_user_keepalive_timer(osal_void *arg);
osal_s32 hmac_config_set_user_app_ie_cb(hmac_vap_stru *hmac_vap, frw_msg *msg);
oal_bool_enum_uint8  hmac_user_get_ps_mode(const hmac_user_stru *hmac_user);
osal_u32 hmac_alg_distance_notify_hook(hal_to_dmac_device_stru *hal_device, osal_u8 distance);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_user.h */
