/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of hmac public interface.
 * Create: 2020-01-01
 */

#ifndef __HMAC_EXT_IF_H__
#define __HMAC_EXT_IF_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device_ext.h"
#include "mac_vap_ext.h"
#include "mac_user_ext.h"
#include "mac_frame.h"
#include "dmac_ext_if_hcm.h"
#include "frw_hmac.h"
#include "oal_net.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_EXT_IF_H

/*****************************************************************************
    枚举定义
*****************************************************************************/
/* hmac tx返回值类型枚举 */
typedef enum {
    HMAC_TX_PASS = 0,     /* 继续往下 */
    HMAC_TX_BUFF = 1,     /* 已被缓存 */
    HMAC_TX_DONE = 2,     /* 组播转成单播已发送 */

    HMAC_TX_DROP_PROXY_ARP    = 3, /* PROXY ARP检查后丢弃 */
    HMAC_TX_DROP_USER_UNKNOWN,     /* 未知user */
    HMAC_TX_DROP_USER_NULL,        /* user结构体为NULL */
    HMAC_TX_DROP_USER_INACTIVE,    /* 目的user未关联 */
    HMAC_TX_DROP_SECURITY_FILTER,  /* 安全检查过滤掉 */
    HMAC_TX_DROP_BA_SETUP_FAIL,    /* BA会话创建失败 */
    HMAC_TX_DROP_AMSDU_ENCAP_FAIL, /* amsdu封装失败 */
    HMAC_TX_DROP_AMSDU_BUILD_FAIL, /* amsdu组帧失败 */
    HMAC_TX_DROP_MUSER_NULL,       /* 组播user为NULL */
    HMAC_TX_DROP_MTOU_FAIL,        /* 组播转单播失败 */
    HMAC_TX_DROP_80211_ENCAP_FAIL, /* 802.11 head封装失败 */
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    HMAC_TX_DROP_NOSMART,          /* 15 组播报文丢弃(非智能单品) */
    HMAC_TX_DROP_NOADAP,           /* 16 组播报文丢弃(非配网模式) */
#endif
    HMAC_TX_BUTT
} hmac_tx_return_type_enum;
typedef osal_u8 hmac_tx_return_type_enum_uint8;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* HOST CTX事件子类型枚举 */
typedef enum {
    HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA = 0,     /* STA　扫描完成子类型 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA,         /* STA 关联完成子类型 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP,        /* AP 上报新加入BSS的STA情况 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP,      /* AP 上报离开BSS的STA情况 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_ACS_RESPONSE,          /* 上报ACS命令执行结果 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT,               /* 上报接收到的管理帧 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_INIT,
    HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS,

#ifdef _PRE_WLAN_ONLINE_DPD
    HMAC_HOST_CTX_EVENT_SUB_TYPE_DPD,
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    HMAC_HOST_CTX_EVENT_SUB_TYPE_M2S_STATUS,       /* 上报m2s事件 */
#endif
    HMAC_HOST_CTX_EVENT_SUB_TYPE_SDP_RX_DATA,       /* 上报SDP帧数据 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_CH_SWITCH_NOTIFY,
    HMAC_HOST_CTX_EVENT_SUB_TYPE_BUTT
} hmac_host_ctx_event_sub_type_enum;
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
/*****************************************************************************
  枚举名  : hmac_host_ctx_event_sub_type_enum_uint8
  协议表格:
  枚举说明: HOST CTX事件子类型定义
*****************************************************************************/
typedef enum {
    HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA = 0,      /* STA 扫描完成子类型 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_ASOC_COMP_STA,         /* STA 关联完成子类型 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_DISASOC_COMP_STA,      /* STA 上报去关联完成 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP,        /* AP 上报新加入BSS的STA情况 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP,      /* AP 上报离开BSS的STA情况 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_MIC_FAILURE,           /* 上报MIC攻击 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_ACS_RESPONSE,          /* 上报ACS命令执行结果 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_RX_MGMT,               /* 上报接收到的管理帧 */
#ifdef _PRE_WLAN_FEATURE_P2P
    HMAC_HOST_CTX_EVENT_SUB_TYPE_LISTEN_EXPIRED,        /* 上报监听超时 */
#endif
    HMAC_HOST_CTX_EVENT_SUB_TYPE_INIT,
    HMAC_HOST_CTX_EVENT_SUB_TYPE_MGMT_TX_STATUS,
    HMAC_HOST_CTX_EVENT_SUB_TYPE_FT_EVENT_STA,         /* STA 漫游完成子类型 */

#ifdef _PRE_WLAN_ONLINE_DPD
    HMAC_HOST_CTX_EVENT_SUB_TYPE_DPD,
#endif

#ifdef _PRE_WLAN_RF_AUTOCALI
    HMAC_HOST_CTX_EVENT_SUB_TYPE_AUTOCALI_REPORT,
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    HMAC_HOST_CTX_EVENT_SUB_TYPE_CAC_REPORT,       /* 上报CAC事件 */
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    HMAC_HOST_CTX_EVENT_SUB_TYPE_M2S_STATUS,       /* 上报m2s事件 */
#endif
    HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONN_RESULT,

    HMAC_HOST_CTX_EVENT_SUB_TYPE_SDP_RX_DATA,
#ifdef _PRE_WLAN_FEATURE_MESH
    HMAC_HOST_CTX_EVENT_SUB_TYPE_PEER_CLOSE_MESH,   /* 通知WPA与远端设备断开连接 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_TX_DATA_INFO,  /* 通知lwip单播数据帧相关信息 */
    HMAC_HOST_CTX_EVENT_SUB_TYPE_MESH_USER_INFO, /* 通知lwip用户关联状态信息 */
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    HMAC_HOST_CTX_EVENT_SUB_TYPE_P2P_TX_STATUS,
#endif
    HMAC_HOST_CTX_EVENT_SUB_TYPE_CHANNEL_SWITCH,
    HMAC_HOST_CTX_EVENT_SUB_TYPE_AP_USER_INFO,
#ifdef _PRE_WLAN_FEATURE_WPA3
    HMAC_HOST_CTX_EVENT_SUB_TYPE_EXTERNAL_AUTH,
#endif

    HMAC_HOST_CTX_EVENT_SUB_TYPE_BUTT
}hmac_host_ctx_event_sub_type_enum;
#endif

typedef osal_u8 hmac_host_ctx_event_sub_type_enum_uint8;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFR
/* 自愈dfr保存ap信息结构体 */
typedef struct {
    mac_cfg_channel_param_stru channel_info; // 设置信道
    mac_cfg_ssid_param_stru ssid_info; // 设置ssid信息
    wlan_auth_alg_mode_enum_uint8 auth_algs; // 设置auth mode信息
    mac_beacon_param_stru beacon_param; // 设置beacon时间间隔，timeperiod以及安全配置信息等
    // 启动vap成功后，再设置key, open模式不会设置
    // 只恢复组播的，单播的用户接入的时候，再进行恢复(需注意组播密钥更新的时候，dfr触发会导致怎样)
    mac_addkey_param_stru payload_params;
    // pmf 场景, igtk单独保存
    mac_addkey_param_stru igtk_key_params;
    oal_ap_settings_stru ap_settings;
} hmac_ap_dfr_recovery_info;

/* 自愈相关信息结构体 */
typedef struct {
    // p2p cl和dev共用一个业务vap,netdev的个数不会大于最大业务vap个数3
    oal_net_device_stru *dfr_netdev[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    osal_u32 netdev_num;
    osal_u32 bit_device_reset_process_flag : 1; /* device挂死异常复位操作启动 */
    osal_u32 bit_ready_to_recovery_flag : 1;
    osal_u32 dfr_status : 3; /* WiFi DFR状态: wal_dfr_status_enum */
    osal_u32 bit_resv : 27;
    osal_u32 dfr_num;   /* DFR 挂死次数 */
} hmac_dfr_info_stru;
#endif  // _PRE_WLAN_FEATURE_DFR

/* HMAC抛去关联完成事件结构体 */
typedef struct {
    osal_u8              *msg;
} hmac_disasoc_comp_event_stru;

/* 扫描结果 */
typedef struct {
    osal_u8     num_dscr;
    osal_u8     result_code;
    osal_u8     auc_resv[2];
} hmac_scan_rsp_stru;

/* Status code for MLME operation confirm */
typedef enum {
    HMAC_MGMT_SUCCESS             = 0,
    HMAC_MGMT_INVALID             = 1,
    HMAC_MGMT_TIMEOUT             = 2,
    HMAC_MGMT_REFUSED             = 3,
    HMAC_MGMT_TOMANY_REQ          = 4,
    HMAC_MGMT_ALREADY_BSS         = 5
} hmac_mgmt_status_enum;
typedef osal_u8   hmac_mgmt_status_enum_uint8;

/* 关联结果 */
typedef struct {
    hmac_mgmt_status_enum_uint8  result_code;         /* 关联成功,超时等 */
    osal_u8                    auc_resv1[1];
    mac_status_code_enum_uint16  status_code;         /* ieee协议规定的16位状态码 */
    osal_u8                    addr_ap[WLAN_MAC_ADDR_LEN];
    osal_u8                    resv2[2];
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_u16                   freq;
#endif
    osal_u32                   asoc_req_ie_len;
    osal_u32                   asoc_rsp_ie_len;

    osal_u8                    *asoc_req_ie_buff;
    osal_u8                    *asoc_rsp_ie_buff;
} hmac_asoc_rsp_stru;

/* CSA通道切换完成结果 */
typedef struct {
    osal_s32 freq;
    osal_u8 chan_number;
    osal_u8 band;
    osal_u8 bandwidth;
    osal_u8 resv;
} hmac_csa_chan_switch_done_stru;

/* 漫游结果 */
typedef struct {
    osal_u8                    bssid[WLAN_MAC_ADDR_LEN];
    osal_u8                    auc_resv1[2];
    mac_channel_stru           st_channel;
    osal_u32                   asoc_req_ie_len;
    osal_u32                   asoc_rsp_ie_len;
    osal_u8                   *asoc_req_ie_buff;
    osal_u8                   *asoc_rsp_ie_buff;
} hmac_roam_rsp_stru;

/* mic攻击 */
typedef struct {
    osal_u8 user_mac[WLAN_MAC_ADDR_LEN];
    osal_u8 reserve[2];
    oal_nl80211_key_type key_type;
    osal_s32 key_id;
} hmac_mic_event_stru;

/* 上报接收到管理帧事件的数据结构 */
typedef struct {
    osal_u8                  *buf;
    osal_u16                  len;
    osal_u8                   rssi;        /* 已经在驱动加上HMAC_FBT_RSSI_ADJUST_VALUE将负值转成正值 */
    osal_u8                   rsv[1];
    osal_s32                   l_freq;
    osal_s8                    ac_name[OAL_IF_NAME_SIZE];
} hmac_rx_mgmt_event_stru;

/* 上报监听超时数据结构 */
typedef struct {
    oal_ieee80211_channel_stru  st_listen_channel;
    osal_u64                  ull_cookie;
    oal_wireless_dev_stru      *wdev;
} hmac_p2p_listen_expired_stru;

/* 上报接收到管理帧事件的数据结构 */
typedef struct {
    osal_u8                   dev_mode;
    osal_u8                   vap_mode;
    osal_u8                   vap_status;
    osal_u8                   write_read;
    osal_u32                  val;
} hmac_cfg_rx_filter_stru;

#define ipaddr(addr) \
        ((osal_u8*)&(addr))[0], \
        ((osal_u8*)&(addr))[1], \
        ((osal_u8*)&(addr))[2]

#define ipaddr6(addr) \
        ntohs((addr).s6_addr16[0]), \
        ntohs((addr).s6_addr16[1]), \
        (ntohs((addr).s6_addr16[2]) & 0xff00) >> 8

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef struct {
    td_u8 is_assoc;                      /* 标识是关联事件/去关联事件 */
    td_u8 conn_to_mesh;                  /* 标识是否关联到Mesh/普通AP */
    td_u8 rssi;                          /* 关联的节点的扫描RSSI */
    td_u8 auc_mac_addr[WLAN_MAC_ADDR_LEN];
    td_u8 rsv[3];                    /* 3:预留数组大小 */
}hmac_sta_report_assoc_info_stru;
#endif

#ifdef _PRE_WLAN_WIRELESS_EXT
#define SYNC_DATA_MAX 128
typedef enum {
    IWLIST_SET_SCAN_MODE,
    WAIT_SYNC_MODE_BUTT
} wait_sync_mode_enum;

typedef struct {
    osal_u8 status;
    osal_u8 len;
    wait_sync_mode_enum mode_id;
    osal_u8 data[SYNC_DATA_MAX];
} hmac_to_wal_sync_data_stru;

typedef struct {
    osal_u8 essid_enable;
    osal_u8 essid_len;
    osal_u8 essid[WLAN_SSID_MAX_LEN];
} iwlist_scan_essid_enable_stru;
#endif

#define WIFI_SEPCIAL_IPV4_PKT_TAG "wifi:special_ipv4_pkt,"
#define HWMACSTR "%02x:%02x:%02x:%02x:**:**"
#define hwmac2str(a) (a)[0], (a)[1], (a)[2], (a)[3]

#ifndef GFP_ATOMIC
#define GFP_ATOMIC 0
#endif

/*****************************************************************************
    函数声明
*****************************************************************************/
extern osal_s32  hmac_main_init_etc(void);
extern void  hmac_main_exit_etc(void);
extern osal_s32 hmac_tx_wlan_to_wlan_ap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_TCP_OPT
extern osal_u32  hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf);
#endif
extern osal_u32  hmac_tx_lan_to_wlan_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
extern osal_s32 frw_host_post_msg(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg);
extern osal_s32 frw_host_post_data(osal_u16 netbuf_type, osal_u8 vap_id, oal_netbuf_stru *netbuf);
extern osal_void frw_set_data_queue_limit(osal_u32 que_size);
extern osal_s32 hmac_bridge_vap_xmit_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_u16 hmac_free_netbuf_list_etc(oal_netbuf_stru *netbuf);
extern oal_net_device_stru  *hmac_vap_get_net_device_etc(osal_u8 vap_id);
extern osal_s8  *hmac_vap_get_desired_country_etc(osal_u8 vap_id);
#ifdef _PRE_WLAN_FEATURE_11D
extern osal_u32  hmac_vap_get_updata_rd_by_ie_switch_etc(osal_u8 vap_id, oal_bool_enum_uint8 *update_rd_by_ie_sw);
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
extern osal_s32 hmac_config_user_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_vap_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_s32 hmac_config_11i_remove_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef _PRE_WLAN_FEATURE_11D
extern osal_s32 hmac_config_set_rd_by_ie_switch_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

extern osal_s32 hmac_config_add_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_del_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_start_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_down_vap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_11i_get_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_set_regdomain_pwr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_get_regdomain_pwr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_set_random_mac_addr_scan(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_shpreamble_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_shortgi20_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_shortgi40_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_shortgi80_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_txpower_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32 hmac_config_safe_del_vap(hmac_vap_stru *hmac_vap);
extern osal_s32  hmac_config_set_nobeacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_prot_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_dtimperiod_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_random_mac_oui_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32 hmac_config_set_mac_addr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_thruput_test(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_freq_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
extern osal_s32  hmac_cfg80211_start_sched_scan_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_s32  hmac_config_delba_req_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_rx_dhcp_debug_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_service_control_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_amsdu_tx_on_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_bss_type_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_auth_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_addba_req_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_cfg80211_start_scan_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_set_mib_by_bw(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_set_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_set_beacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_ssid_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_11i_add_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_wmmswitch(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_country_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_u32  hmac_config_bg_noise_info(hmac_vap_stru *hmac_vap, osal_u16 *pus_len, osal_u8 *param);

extern osal_u32  hmac_config_set_mode_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
extern osal_u32  hmac_sta_initiate_join_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr);
extern osal_s32  hmac_config_get_txpower_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32  hmac_config_get_shortgi20_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_shortgi40_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_shortgi80_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_shpreamble_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_nobeacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_prot_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_dtimperiod_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32  hmac_config_get_sta_list_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_freq_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_get_channel_list(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_bss_type_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_auth_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_get_ssid_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_u32  hmac_config_set_max_user_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u32 max_user);
extern osal_s32 hmac_config_set_max_user_cb(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_reg_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg_info);
extern void    hmac_mgmt_send_deauth_frame_etc(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code,
    oal_bool_enum_uint8 is_protected);
extern void    hmac_mgmt_send_disassoc_frame_etc(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code,
    oal_bool_enum_uint8 is_protected);
#ifdef _PRE_WLAN_CFGID_DEBUG
extern osal_s32  hmac_config_set_2040_coext_support_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_ampdu_tx_on_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_amsdu_ampdu_switch_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_s32 hmac_config_kick_user_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_connect_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_u32 hmac_check_capability_mac_phy_supplicant_etc(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr);
extern osal_u32 hmac_config_11i_add_wep_entry_etc(hmac_vap_stru *hmac_vap, const osal_u8 *param);

extern osal_s32  hmac_config_get_assoc_req_ie_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32  hmac_config_get_country_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_u32 hmac_config_set_app_ie_to_vap_etc(hmac_vap_stru *hmac_vap, oal_app_ie_stru *app_ie,
    app_ie_type_uint8 type);

extern osal_u32 hmac_config_set_acs_cmd(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);

extern osal_u32  hmac_config_open_wmm(hmac_vap_stru *hmac_vap, osal_u16 len, const osal_u8 *param);

#ifdef _PRE_WLAN_FEATURE_STA_PM
extern osal_u32  hmac_config_set_pm_by_module_etc(hmac_vap_stru *hmac_vap, mac_pm_ctrl_type_enum pm_ctrl_type,
    mac_pm_switch_enum pm_enable);
#endif

extern osal_void hmac_tx_report_eth_frame_etc(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf);

osal_s32 hmac_config_fbt_kick_user(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_FEATURE_ISOLATION
extern osal_u32  hmac_config_show_isolation(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
extern osal_u32  hmac_config_set_isolation_mode(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
extern osal_u32  hmac_config_set_isolation_type(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
extern osal_u32  hmac_config_set_isolation_forword(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
extern osal_u32  hmac_config_set_isolation_clear(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
#endif

extern osal_s32 hmac_config_scan_abort_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_FEATURE_P2P
extern osal_s32  hmac_config_set_p2p_ps_ops_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_p2p_ps_noa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_config_set_p2p_ps_stat(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
osal_s32  hmac_config_set_qos_map(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_s32 hmac_config_set_vendor_ie(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_s32   hmac_config_cfg_vap_h2d_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
osal_s32  hmac_config_wifi_sniffer_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_s32 hmac_config_set_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32  hmac_wpas_mgmt_tx_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_del_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_flush_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_u32  hmac_config_user_rate_info_syn_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32  hmac_config_user_info_syn_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
#ifdef _PRE_WLAN_FEATURE_M2S
extern osal_u32  hmac_config_vap_m2s_info_syn(hmac_vap_stru *hmac_vap);
#endif
extern osal_u32  hmac_config_sta_vap_info_syn_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_init_user_security_port_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32 hmac_config_ch_status_sync(hmac_device_stru *hmac_device);
extern osal_u32 hmac_find_p2p_listen_channel_etc(hmac_vap_stru *hmac_vap, osal_u16 len,
    osal_u8 *param, osal_u8 *p2p_listen_channel);
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
extern  void hmac_print_data_wakeup_en_etc(oal_bool_enum_uint8 en);
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern osal_s32 hmac_cfg80211_dump_survey_etc(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    osal_s32 idx, hmac_survey_info_stru *survey_info);
#endif

osal_s32 hmac_config_vendor_cmd_get_channel_list_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifndef CONFIG_HAS_EARLYSUSPEND
extern void  hmac_do_suspend_action_etc(hmac_device_stru *hmac_device, osal_u8  uc_in_suspend);
#endif
osal_s32 hmac_config_set_scan_params_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_multi_bssid_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_bandwidth(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifndef CONFIG_HAS_EARLYSUSPEND
osal_s32 hmac_config_set_suspend_mode(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_config_set_customize_ie(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_config_set_channel_list(hmac_vap_stru *hmac_vap, mac_cfg_set_scan_param *scan_param);
#ifdef _PRE_WLAN_FEATURE_11AX
osal_s32 hmac_config_set_11ax_softap_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_ext_if.h */
