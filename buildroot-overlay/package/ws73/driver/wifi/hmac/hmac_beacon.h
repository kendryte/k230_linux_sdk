/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_beacon_rom.c 的头文件
 * Date: 2020-07-07
 */

#ifndef HMAC_BEACON_H
#define HMAC_BEACON_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "mac_resource_ext.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_WMM_QOS_PARAMS_HDR_LEN 8
#define HMAC_MAX_WAIT_BEACON_TIMES      5

#define WLAN_PROTECTION_NON_ERP_AGING_THRESHOLD        (60)   /* ERP 老化计数最大值 */
#define WLAN_PROTECTION_NON_HT_AGING_THRESHOLD         (60)   /* HT 老化计数最大值 */

/* 接收或者发送成功门限增值 */
#define LINKLOSS_THRESHOLD_INCR 1
/* 用于计算linkloss门限的最大beacon周期 */
#define LINKLOSS_THRESHOLD_BEACON_MAX_INTVAL 400

#define LINKLOSS_PROBE_REQ_MIN_CNT      10  /* linkloss发送probe req最小阈值 */
#define LINKLOSS_PORBE_REQ_MAX_CNT      60  /* linkloss发送probe req最大阈值 */
#define LINKLOSS_PROBE_REQ_MIN_MOD      3   /* 每3次tbtt中断发送一次probe req */
#define LINKLOSS_PROBE_REQ_MAX_MOD      10  /* 每10次tbtt中断发送一次probe req */

#define RX_DATA_RATE                            (50000) /* 50M */
#define STA_TBTT_INTERVAL_TH      5 // STA tbtt执行间隔阈值
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* beacon帧需要更新的信息元素偏移量 */
typedef struct {
    osal_u16   cap_offset;
    osal_u16   bss_load_offset;
    osal_u16   wmm_offset;
    osal_u16   ht_cap_offset;
    osal_u16   ht_operation_offset;
    osal_u16   erp_offset;
    osal_u16   pwr_constrain_offset;
    osal_u16   tpc_report_offset;
    osal_u16   tim_offset;
    osal_u8    wmm_update_flag;
    osal_u8    auc_resv[1];
} hmac_beacon_ie_offset_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_void hmac_sta_post_bw_verify_switch(const hmac_vap_stru *hmac_vap,
    hmac_sta_bw_switch_type_enum_enum_uint8 verify_reslt);
osal_u32 hmac_sta_set_bandwith_handler(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth);
osal_u32 hmac_beacon_alloc(hmac_vap_stru *hmac_vap);
osal_u32 hmac_beacon_free(hmac_vap_stru *hmac_vap);
osal_s32 hmac_tbtt_event_handler(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_tbtt_event_sta(osal_void *arg);
osal_void hmac_vap_linkloss_channel_clean(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_u32 hmac_sta_up_rx_beacon_check(hmac_vap_stru   *hmac_vap, oal_netbuf_stru *netbuf, osal_u8 *go_on);
osal_u16  hmac_encap_beacon(hmac_vap_stru *hmac_vap, osal_u8 *beacon_buffer);
osal_void hmac_sta_beacon_offload_test(hmac_vap_stru *hmac_vap, osal_u8 *host_sleep);
osal_u32 hmac_protection_update_mib_ap(hmac_vap_stru *hmac_vap);
osal_u32 hmac_protection_del_user(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_void hmac_protection_stop_timer(hmac_vap_stru  *hmac_vap);
osal_u32  hmac_set_protection_mode(hmac_vap_stru *hmac_vap,   const hmac_user_stru *hmac_user,
    wlan_prot_mode_enum_uint8 prot_mode);
osal_void hmac_ap_up_rx_obss_beacon(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_void hmac_ap_wait_start_rx_obss_beacon(hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf);
osal_void hmac_vap_linkloss_init(hmac_vap_stru *hmac_vap);
osal_void hmac_vap_linkloss_clean(hmac_vap_stru *hmac_vap);
osal_s32 hmac_config_set_linkloss_params(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_irq_tbtt_ap_isr(osal_u8 mac_vap_id);
osal_void hmac_sta_stop_tbtt_timer(hmac_vap_stru *hmac_vap);
osal_void hmac_sta_align_tbtt_timer(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_addition);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_beacon.h */
