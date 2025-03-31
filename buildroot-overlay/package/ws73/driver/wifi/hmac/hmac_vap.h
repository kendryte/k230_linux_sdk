/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_vap.h
 * 生成日期   : 2012年10月19日
 * 功能描述   : hmac_vap.c 的头文件
 */


#ifndef __HMAC_VAP_H__
#define __HMAC_VAP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_user.h"
#include "hmac_main.h"
#include "mac_resource_ext.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt_struc.h"
#include "hcc_host_if.h"
#endif
#include "msg_vap_manage_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VAP_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_HS20
#define MAX_QOS_UP_RANGE  8
#define MAX_DSCP_EXCEPT   21  /* maximum of DSCP Exception fields for QoS Map set */
#endif

#define HMAC_DSCP_VALUE_INVALID      0xA5

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
    hmac_vap_stru *hmac_vap;
    mac_vap_state_enum_uint8 old_state;
    mac_vap_state_enum_uint8 new_state;
    osal_u8 resv[2]; /* 保留2个字节对齐 */
} hmac_vap_state_change_notify_stru;

typedef struct {
    struct osal_list_head timeout_head;
} hmac_mgmt_timeout_stru;

typedef struct {
    struct osal_list_head           entry;
    osal_u8                     bssid[WLAN_MAC_ADDR_LEN];
    osal_u8                     reserved[2];
    osal_u8                     pmkid[WLAN_PMKID_LEN];
} hmac_pmksa_cache_stru;

typedef enum _hmac_cac_event_ {
    HMAC_CAC_STARTED = 0,
    HMAC_CAC_FINISHED,
    HMAC_CAC_ABORTED,

    HMAC_CAC_BUTT
} hmac_cac_event;

typedef struct {
    hmac_cac_event                          type;
    osal_u32                              freq;
    wlan_channel_bandwidth_enum_uint8       bw_mode;
    osal_u8                               auc[23];
} hmac_cac_event_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_set_host_rx_ampdu_amsdu(osal_u8 val);
osal_u8 hmac_get_host_rx_ampdu_amsdu(osal_void);
extern osal_s32 hmac_vap_destroy_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_u32 hmac_vap_init_etc(
    hmac_vap_stru              *hmac_vap,
    osal_u8                   chip_id,
    osal_u8                   device_id,
    osal_u8                   vap_id,
    hmac_cfg_add_vap_param_stru *param);

extern osal_u32  hmac_vap_creat_netdev_etc(hmac_vap_stru *hmac_vap, osal_s8 *netdev_name, osal_u8 name_len,
    osal_s8 *mac_addr);


osal_u16 hmac_vap_check_ht_capabilities_ap_etc(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 msg_len,
    hmac_user_stru *hmac_user, osal_bool is_tkip);
extern  osal_u32  hmac_search_ht_cap_ie_ap_etc(
    hmac_vap_stru               *hmac_vap,
    hmac_user_stru              *hmac_user,
    osal_u8                   *payload,
    osal_u16                   offset,
    oal_bool_enum                prev_asoc_ht);
extern oal_bool_enum_uint8 hmac_vap_addba_check(
    hmac_vap_stru      *hmac_vap,
    hmac_user_stru     *hmac_user,
    osal_u8           tidno);

extern osal_void hmac_handle_disconnect_rsp_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_reason_code_enum_uint16  disasoc_reason);
extern osal_u8 *hmac_vap_get_pmksa_etc(hmac_vap_stru *hmac_vap, osal_u8 *bssid);
osal_u32 hmac_tx_get_mac_vap_etc(osal_u8 vap_id, hmac_vap_stru **hmac_vap);
hmac_vap_stru *hmac_get_vap(osal_u32 idx);
osal_void mac_vap_intrrupt_enable(osal_void);
osal_void mac_vap_intrrupt_disable(osal_void);
wlan_protocol_mode_enum_uint8 hmac_get_protocol_mode_by_wlan_protocol(wlan_protocol_enum_uint8 protocol);
osal_bool hmac_vap_need_update_protocol(const hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 new_wlan_protocol);
extern osal_u8 hmac_vap_is_connecting(hmac_vap_stru *mac_vap);

// from dmac_vap_rom.h
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_MAX_SW_RETRIES 10                      /* 数据报文重传次数 */
#define HMAC_MGMT_MAX_SW_RETRIES 3                  /* 管理报文重传次数 */
#define HMAC_MAX_AMPDU_LENGTH_PERFOMANCE_COUNT 32   /* 进行计数的最大的聚合长度 */

#define HMAC_BW_VERIFY_MAX_THRESHOLD        60

#define HMAC_DEF_BCN_TIMEOUT                30      /* STA接收bcn超时时间单位ms */
#define HMAC_DEF_BCN_TIMEOUT_MIN            10      /* STA接收bcn超时最小时间单位ms */
#define HMAC_DEF_BCN_TIMEOUT_MAX            100     /* STA接收bcn超时最大时间单位ms */
#define HMAC_DEF_AP_TBTT_OFFSET             15000   /* AP的tbtt提前量单位us */
#define HMAC_DEF_BCN_TOUT_MAX_CNT           5       /* STA接收bcn超时时间最大次数,超过次数后暂停休眠 */

#define HMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD_BTCOEX 40 /* 连续发送失败的打印RF寄存器门限 */
#define HMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD 20        /* 连续发送失败的打印RF寄存器门限 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC = 0,
    HMAC_STA_BW_SWITCH_EVENT_BEACON_20M,
    HMAC_STA_BW_SWITCH_EVENT_BEACON_40M,
    HMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE,
    HMAC_STA_BW_SWITCH_EVENT_USER_DEL,
    HMAC_STA_BW_SWITCH_EVENT_RSV,
    HMAC_STA_BW_SWITCH_EVENT_BUTT
} wlan_sta_bw_switch_event_enum;
typedef osal_u8 wlan_sta_bw_switch_event_enum_uint8;

/* 统计的AMPDU的门限值枚举值 */
typedef enum {
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_1 = 1,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_14 = 14,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_15 = 15,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_17 = 17,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_18 = 18,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_30 = 30,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_31 = 31,
    HMAC_AMPDU_LENGTH_COUNT_LEVEL_32 = 32
} hmac_count_by_ampdu_length_level_enum;

typedef enum {
    HMAC_DBDC_START,
    HMAC_DBDC_STOP,
    HMAC_DBDC_STATE_BUTT
} hmac_dbdc_state_enum;
typedef osal_u8 hmac_dbdc_state_enum_uint8;

typedef enum {
    HMAC_STA_BW_SWITCH_FSM_INIT = 0,
    HMAC_STA_BW_SWITCH_FSM_NORMAL,
    HMAC_STA_BW_SWITCH_FSM_VERIFY20M, // 20m校验
    HMAC_STA_BW_SWITCH_FSM_VERIFY40M, // 40m校验
    HMAC_STA_BW_SWITCH_FSM_INVALID,
    HMAC_STA_BW_SWITCH_FSM_BUTT
} hmac_sta_bw_switch_fsm_enum;

typedef enum {
    HMAC_STA_BW_VERIFY_20M_TO_40M = 0,
    HMAC_STA_BW_VERIFY_40M_TO_20M,
    HMAC_STA_BW_VERIFY_SWITCH_BUTT
} hmac_sta_bw_switch_type_enum;
typedef osal_u8 hmac_sta_bw_switch_type_enum_enum_uint8;

/* device多vap状态切换 */
typedef enum {
    HMAC_VAP_MVAP_STATE_SINGLE = 0, /* device单vap工作 */
    HMAC_VAP_MVAP_STATE_DBAC,       /* dbac状态 */
    HMAC_VAP_MVAP_STATE_DBDC,       /* dbdc状态 */
    HMAC_VAP_MVAP_STATE_DCHN,       /* 同信道 */

    HMAC_VAP_MVAP_STATE_BUTT
} hmac_vap_mvap_state_enum;

typedef enum {
    HMAC_VAP_MVAP_STATE_TRANS_NONE = 0, /* 单vap切信道 */

    HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DBDC, /* dbdc状态到dbac状态切换 */
    HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DCHN, /* 切换信道后退出dbac状态到同信道状态 */
    HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DBAC, /* 切换信道后仍然停留在dbac状态 */

    HMAC_VAP_MVAP_STATE_TRANS_DBDC_TO_DBDC, /* dbdc状态不变 */
    HMAC_VAP_MVAP_STATE_TRANS_DBDC_TO_DBAC, /* dbdc到dbac状态切换 */

    HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DCHN, /* 同频同信道状态不变 */
    HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DBAC, /* 同频同信道到dbac */

    HMAC_VAP_MVAP_STATE_TRANS_BUTT
} hmac_vap_mvap_state_trans_enum;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* vap tx参数，从架构分析文档获得成员 */
typedef struct {
    osal_u16 rts_threshold; /* rts阈值 */
    osal_u8 mcast_rate;     /* 广播速率 */
    osal_u8 auc_resv[1];    /* 字节对齐 */
} hmac_vap_tx_param_stru;


typedef osal_u8 hmac_linkloss_status_enum_uint8;


typedef struct {
    oal_fsm_stru oal_fsm;                /* 状态机 */
    oal_bool_enum_uint8 is_fsm_attached; /* 状态机是否已经注册 */
    osal_u8 verify_fail_cnt_20m;
    osal_u8 verify_fail_cnt_40m;
    osal_u8 rsv[1];
} hmac_sta_bw_switch_fsm_info_stru;


typedef enum {
    HMAC_BEACON_TX_POLICY_SINGLE = 0,
    HMAC_BEACON_TX_POLICY_SWITCH = 1,
    HMAC_BEACON_TX_POLICY_DOUBLE = 2,
} hmac_beacon_tx_policy_enum_uint8;

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_PMF
static INLINE__ osal_u8 is_open_pmf_reg(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->user_pmf_status != 0);
}
#endif

static INLINE__ osal_u8 get_current_linkloss_threshold(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->linkloss_info.linkloss_info[hmac_vap->linkloss_info.linkloss_mode].linkloss_threshold;
}

static INLINE__ osal_u16 get_current_linkloss_cnt(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->linkloss_info.linkloss_info[hmac_vap->linkloss_info.linkloss_mode].link_loss;
}

static INLINE__ osal_u8 get_current_linkloss_int_threshold(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->linkloss_info.linkloss_info[hmac_vap->linkloss_info.linkloss_mode].int_linkloss_threshold;
}

static INLINE__ osal_u8 hmac_is_linkloss(const hmac_vap_stru *hmac_vap)
{
    return (get_current_linkloss_cnt(hmac_vap) > get_current_linkloss_threshold(hmac_vap));
}

static INLINE__ osal_bool hmac_is_hml(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->hml == 1) ? OSAL_TRUE : OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_vap_get_bss_type
 功能描述  : 获取VAP类型，指针是否为空，调用者判断
*****************************************************************************/
static INLINE__ wlan_vap_mode_enum_uint8 hmac_vap_get_bss_type(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->vap_mode;
}

/*****************************************************************************
 函 数 名  : hmac_user_get_hal_device
 功能描述  : 根据mac_user获取对应的hal_device指针
*****************************************************************************/
static INLINE__ hal_to_dmac_device_stru *hmac_user_get_hal_device(const hmac_user_stru *hmac_user)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_NULL;
    }

    return hmac_vap->hal_device;
}

/*****************************************************************************
 函 数 名  : hmac_user_get_hal_chip
 功能描述  : 根据mac_user获取对应的hal_chip指针
*****************************************************************************/
static INLINE__ hal_to_dmac_chip_stru *hmac_user_get_hal_chip(const hmac_user_stru *hmac_user)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_NULL;
    }

    return hmac_vap->hal_chip;
}

osal_u8 hmac_sta_bw_fsm_get_current_state(const hmac_vap_stru *hmac_vap);

static INLINE__ osal_u8 mac_vap_get_curren_bw_state(const hmac_vap_stru *hmac_vap)
{
    return hmac_sta_bw_fsm_get_current_state(hmac_vap);
}

static INLINE__ osal_u8 mac_vap_bw_fsm_beacon_avail(const hmac_vap_stru *hmac_vap)
{
    return ((!is_legacy_sta(hmac_vap)) ||
        (is_legacy_sta(hmac_vap) && (mac_vap_get_curren_bw_state(hmac_vap) > HMAC_STA_BW_SWITCH_FSM_INIT) &&
        (mac_vap_get_curren_bw_state(hmac_vap) < HMAC_STA_BW_SWITCH_FSM_INVALID)));
}

static INLINE__ osal_u8 mac_vap_bw_fsm_verify(const hmac_vap_stru *hmac_vap)
{
    return ((mac_vap_get_curren_bw_state(hmac_vap) == HMAC_STA_BW_SWITCH_FSM_VERIFY20M) ||
        (mac_vap_get_curren_bw_state(hmac_vap) == HMAC_STA_BW_SWITCH_FSM_VERIFY40M));
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_s8 *hmac_config_index2string_etc(osal_u32 index, osal_s8 *string[], osal_u32 max_str_nums);
osal_void hmac_sta_bw_switch_fsm_attach(hmac_vap_stru *hmac_vap);
osal_void hmac_sta_bw_switch_fsm_detach(hmac_vap_stru *hmac_vap);
osal_void hmac_sta_bw_switch_fsm_init(const hmac_vap_stru * const hmac_vap);
osal_u8 hmac_sta_bw_switch_need_new_verify(const hmac_vap_stru *hmac_vap, wlan_bw_cap_enum_uint8 bw_becaon_new);
osal_u32 hmac_sta_bw_switch_fsm_post_event(const hmac_vap_stru *hmac_vap, osal_u16 type, osal_u16 datalen,
    osal_u8 *data);
osal_u32 hmac_vap_init(hmac_vap_stru *hmac_vap, osal_u8 vap_id, const mac_cfg_add_vap_param_stru *param);
osal_u32 hmac_vap_init_tx_frame_params(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag);
osal_void hmac_vap_init_tx_mgmt_rate(const hmac_vap_stru *hmac_vap, hal_tx_txop_alg_stru *tx_mgmt_cast);
osal_u32 hmac_vap_sta_reset(hmac_vap_stru *hmac_vap);
osal_void mac_vap_pause_tx(hmac_vap_stru *hmac_vap);
osal_void hmac_vap_pause_tx(hmac_vap_stru *hmac_vap);

osal_void hmac_vap_resume_tx_by_chl(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device,
    mac_channel_stru *dst_channel);
osal_u32 hmac_vap_is_in_p2p_listen(osal_void);
#ifdef _PRE_WLAN_FEATURE_DBAC
osal_void hmac_vap_restart_dbac(hmac_vap_stru *hmac_vap);
#endif
osal_void hmac_vap_down_notify(hmac_vap_stru *down_vap);
osal_u8   hmac_vap_fake_queue_empty_assert(hmac_vap_stru *hmac_vap, osal_u32 file_id);
osal_void hmac_vap_clear_fake_queue(hmac_vap_stru *hmac_vap);
osal_u32 hmac_vap_get_fake_queue_pkt_nums(osal_void);
osal_void hmac_vap_send_fake_queue(hmac_vap_stru *hmac_vap);
osal_void hmac_vap_send_all_fake_queue(osal_void);
osal_void hmac_vap_update_snr_info(hmac_vap_stru *hmac_vap, const dmac_rx_ctl_stru *const rx_ctrl);
osal_void hmac_vap_work_set_channel(hmac_vap_stru *hmac_vap);

osal_void hmac_vap_flush_rate(hmac_vap_stru *hmac_vap, const hmac_set_rate_stru *const rate);
osal_u32 hmac_vap_init_rate_info(hmac_set_rate_stru *surr_rate);
osal_u32 hmac_config_start_vap_event_etc(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag);
osal_u32 hmac_get_device_vap_num(osal_void);
osal_u32 hmac_config_add_vap_feature(hmac_vap_stru *hmac_vap);
osal_void hmac_config_del_vap_feature(hmac_vap_stru *hmac_vap);
osal_s32 hmac_config_del_vap_success(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_start_vap(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 mgmt_rate_init_flag);
osal_s32 hmac_config_add_vap(hmac_vap_stru *vap_new, frw_msg *msg);
osal_s32 hmac_config_del_vap(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_down_vap(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_vap_sync(hmac_vap_stru *hmac_vap);
osal_void hmac_vap_refresh_tx_bitmap(osal_void);
oal_bool_enum_uint8 hmac_vap_is_pause_tx_bitmap(hmac_vap_stru *hmac_vap);
hmac_vap_stru *hmac_get_valid_vap(osal_u32 idx);
osal_void hmac_vap_net_start_subqueue_etc(hcc_queue_type queue_idx);
osal_void hmac_vap_net_stop_subqueue_etc(hcc_queue_type queue_idx);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_vap.h */
