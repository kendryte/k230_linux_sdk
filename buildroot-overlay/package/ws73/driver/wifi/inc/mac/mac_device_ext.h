/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac_device_etc.c.
 * Create: 2020-7-8
 */
#ifndef __MAC_DEVICE_EXT_H__
#define __MAC_DEVICE_EXT_H__

#include "mac_frame.h"

#include "mac_regdomain.h"
#include "wlan_spec.h"
#include "wlan_mib_hcm.h"
#include "wlan_types_common.h"
#include "frw_osal.h"

#include "oam_ext_if.h"

#include "frw_ext_if.h"
#include "hal_common_ops.h"

#include "osal_types.h"
#include "osal_adapt.h"
#include "oal_ext_if.h"
#include "hal_ext_if.h"
#include "wlan_resource_common.h"
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAC_NET_DEVICE_NAME_LENGTH          16
#define MAC_BAND_CAP_NAME_LENGTH            16
#define MAC_DATARATES_PHY_80211G_NUM        12

#define MAC_EXCEPTION_TIME_OUT              10000
#define MULTI_BSSID_BITMAP                  32
#define WLAN_USER_MAX_SUPP_RATES            16

#define mac_device_get_cap_bw(_pst_device)      ((_pst_device)->device_cap.channel_width)
#define mac_device_get_nss_num(_pst_device)     ((_pst_device)->device_cap.nss_num)
#define mac_device_get_cap_ldpc(_pst_device)    ((_pst_device)->device_cap.ldpc_is_supp)
#define mac_device_get_cap_txstbc(_pst_device)  ((_pst_device)->device_cap.tx_stbc_is_supp)
#define mac_device_get_cap_rxstbc(_pst_device)  ((_pst_device)->device_cap.rx_stbc_is_supp)
#define mac_device_get_cap_subfer(_pst_device)  ((_pst_device)->device_cap.su_bfmer_is_supp)
#define mac_device_get_cap_subfee(_pst_device)  ((_pst_device)->device_cap.su_bfmee_is_supp)
#define mac_device_get_cap_mubfer(_pst_device)  ((_pst_device)->device_cap.mu_bfmer_is_supp)
#define mac_device_get_cap_mubfee(_pst_device)  ((_pst_device)->device_cap.mu_bfmee_is_supp)
#define mac_device_get_cap_rx_mtid(_pst_device) ((_pst_device)->device_cap.mtid_aggregation_rx_support)
#define mac_device_get_cap_32bitmap(_pst_device)    ((_pst_device)->device_cap.ba_bitmap_support_32bit)
#ifdef _PRE_WLAN_FEATURE_SMPS
#define mac_device_get_mode_smps(_pst_device)   ((_pst_device)->mac_smps_mode)
#endif

#define MAX_PNO_SSID_COUNT              16
#define MAX_PNO_REPEAT_TIMES            4
#define PNO_SCHED_SCAN_INTERVAL         (60 * 1000)

#define MAC_SCAN_FUNC_MEAS              0x1
#define MAC_SCAN_FUNC_STATS             0x2
#define MAC_SCAN_FUNC_RADAR             0x4
#define MAC_SCAN_FUNC_BSS               0x8
#define MAC_SCAN_FUNC_P2P_LISTEN        0x10
#define MAC_SCAN_FUNC_ALL      (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS | MAC_SCAN_FUNC_RADAR | MAC_SCAN_FUNC_BSS)

#define MAC_SCAN_CHANNEL_INTERVAL_DEFAULT               6
#define MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT           110
#define MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE           2
#define MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE       60

#ifdef _PRE_WLAN_FEATURE_DBAC
#define mac_dbac_enable(_pst_device) (_pst_device->en_dbac_enabled == OAL_TRUE)
#else
#define mac_dbac_enable(_pst_device) (OAL_FALSE)
#endif

#define MAC_FCS_DBAC_IGNORE                 0
#define MAC_FCS_DBAC_NEED_CLOSE             1
#define MAC_FCS_DBAC_NEED_OPEN              2

#define BTCOEX_BSS_NUM_IN_BLACKLIST         16


typedef enum {
    MAC_CH_TYPE_NONE      = 0,
    MAC_CH_TYPE_PRIMARY   = 1,
    MAC_CH_TYPE_SECONDARY = 2,

    MAC_CH_TYPE__BUTT
} mac_ch_type_enum;
typedef osal_u8 mac_ch_type_enum_uint8;

typedef enum {
    MAC_CHAN_NOT_SUPPORT = 0,
    MAC_CHAN_AVAILABLE_ALWAYS,
    MAC_CHAN_AVAILABLE_TO_OPERATE,
    MAC_CHAN_DFS_REQUIRED,
    MAC_CHAN_BLOCK_DUE_TO_RADAR,

    MAC_CHAN_STATUS_BUTT
} mac_chan_status_enum;
typedef osal_u8 mac_chan_status_enum_uint8;

typedef enum {
    MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_SWITCH_GET_TYPE,
    MAC_RESET_STATUS_GET_TYPE,
    MAC_RESET_STATUS_SET_TYPE,
    MAC_RESET_SWITCH_SYS_TYPE = MAC_RESET_SWITCH_SET_TYPE,
    MAC_RESET_STATUS_SYS_TYPE = MAC_RESET_STATUS_SET_TYPE,

    MAC_RESET_SYS_TYPE_BUTT
} mac_reset_sys_type_enum;
typedef osal_u8 mac_reset_sys_type_enum_uint8;

typedef enum {
    MAC_REPORT_INFO_FLAGS_HARDWARE_INFO =           BIT0,
    MAC_REPORT_INFO_FLAGS_QUEUE_INFO =              BIT1,
    MAC_REPORT_INFO_FLAGS_MEMORY_INFO =             BIT2,
    MAC_REPORT_INFO_FLAGS_EVENT_INFO =              BIT3,
    MAC_REPORT_INFO_FLAGS_VAP_INFO =                BIT4,
    MAC_REPORT_INFO_FLAGS_USER_INFO =               BIT5,
    MAC_REPORT_INFO_FLAGS_TXRX_PACKET_STATISTICS =  BIT6,
    MAC_REPORT_INFO_FLAGS_BUTT,
} mac_report_info_flags;
typedef osal_u8 mac_report_info_flags_enum_uint8;

typedef enum {
    MAC_DEVICE_DISABLE = 0,
    MAC_DEVICE_2G,
    MAC_DEVICE_5G,
    MAC_DEVICE_2G_5G,

    MAC_DEVICE_BUTT,
} mac_device_radio_cap_enum;
typedef osal_u8 mac_device_radio_cap_enum_uint8;

typedef enum {
    MAC_DFR_TIMER_STEP_1 = 0,
    MAC_DFR_TIMER_STEP_2 = 1,
} mac_dfr_timer_step_enum;
typedef osal_u8 mac_dfr_timer_step_enum_uint8;

#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef enum {
    MAC_SET_DSCR_TYPE_UCAST_DATA  = 0,
    MAC_SET_DSCR_TYPE_MCAST_DATA,
    MAC_SET_DSCR_TYPE_BCAST_DATA,
    MAC_SET_DSCR_TYPE_UCAST_MGMT_2G,
    MAC_SET_DSCR_TYPE_MBCAST_MGMT_2G,

    MAC_SET_DSCR_TYPE_BUTT,
} mac_set_dscr_frame_type_enum;
typedef td_u8 mac_set_dscr_frame_type_enum_uint8;
#endif

typedef enum {
    MAC_SDT_MODE_WRITE = 0,
    MAC_SDT_MODE_READ,
    MAC_SDT_MODE_WRITE16,
    MAC_SDT_MODE_READ16,

    MAC_SDT_MODE_BUTT
} mac_sdt_rw_mode_enum;
typedef osal_u8 mac_sdt_rw_mode_enum_uint8;

typedef enum {
    MAC_CSA_FLAG_NORMAL = 0,
    MAC_CSA_FLAG_START_DEBUG,
    MAC_CSA_FLAG_CANCLE_DEBUG,

    MAC_CSA_FLAG_BUTT
} mac_csa_flag_enum;
typedef osal_u8 mac_csa_flag_enum_uint8;

typedef enum {
    MAC_M2S_MODE_QUERY        = 0,
    MAC_M2S_MODE_MSS          = 1,
    MAC_M2S_MODE_DELAY_SWITCH = 2,
    MAC_M2S_MODE_SW_TEST      = 3,
    MAC_M2S_MODE_HW_TEST      = 4,
    MAC_M2S_MODE_RSSI         = 5,

    MAC_M2S_MODE_BUTT,
} mac_m2s_mode_enum;
typedef osal_u8 mac_m2s_mode_enum_uint8;

typedef enum {
    MAC_M2S_COMMAND_STATE_SISO_C0   = 0,
    MAC_M2S_COMMAND_STATE_SISO_C1   = 1,
    MAC_M2S_COMMAND_STATE_MIMO      = 2,
    MAC_M2S_COMMAND_STATE_MISO_C0   = 3,
    MAC_M2S_COMMAND_STATE_MISO_C1   = 4,

    MAC_M2S_COMMAND_STATE_BUTT,
} mac_m2s_command_state_enum;
typedef osal_u8 mac_m2s_command_state_enum_uint8;

typedef enum {
    MAC_M2S_COMMAND_MODE_SET_AUTO = 0,
    MAC_M2S_COMMAND_MODE_SET_SISO_C0 = 1,
    MAC_M2S_COMMAND_MODE_SET_SISO_C1 = 2,
    MAC_M2S_COMMAND_MODE_SET_MIMO = 3,
    MAC_M2S_COMMAND_MODE_GET_STATE = 4,

    MAC_M2S_COMMAND_MODE_BUTT,
} mac_m2s_command_mode_enum;
typedef osal_u8 mac_m2s_command_mode_enum_uint8;

typedef enum {
    HAL_M2S_RSSI_SHOW_TH,
    HAL_M2S_RSSI_SHOW_MGMT,
    HAL_M2S_RSSI_SHOW_DATA,
    HAL_M2S_RSSI_SET_MIN_TH,
    HAL_M2S_RSSI_SET_DIFF_TH,
} hal_dev_rssi_enum;
typedef osal_u8 hal_dev_rssi_enum_uint8;

typedef enum {
    MAC_PM_DEBUG_SISO_RECV_BCN = 0,
    MAC_PM_DEBUG_NO_PS_FRM_INT = 1,
    MAC_PM_DEBUG_APF = 2,
    MAC_PM_DEBUG_AO = 3,

    MAC_PM_DEBUG_CFG_BUTT
} mac_pm_debug_cfg_enum;
typedef osal_u8 mac_pm_debug_cfg_enum_uint8;

typedef enum {
    MAC_DBDC_CHANGE_HAL_DEV = 0,
    MAC_DBDC_SWITCH         = 1,
    MAC_FAST_SCAN_SWITCH    = 2,
    MAC_DBDC_STATUS         = 3,

    MAC_DBDC_CMD_BUTT
} mac_dbdc_cmd_enum;
typedef osal_u8 mac_dbdc_cmd_enum_uint8;

typedef enum {
    MAC_ACS_RSN_INIT,
    MAC_ACS_RSN_LONG_TX_BUF,
    MAC_ACS_RSN_LARGE_PER,
    MAC_ACS_RSN_MWO_DECT,
    MAC_ACS_RSN_RADAR_DECT,

    MAC_ACS_RSN_BUTT
} mac_acs_rsn_enum;
typedef osal_u8 mac_acs_rsn_enum_uint8;

typedef enum {
    MAC_ACS_SW_NONE = 0x0,
    MAC_ACS_SW_INIT = 0x1,
    MAC_ACS_SW_DYNA = 0x2,
    MAC_ACS_SW_BOTH = 0x3,

    MAC_ACS_SW_BUTT
} mac_acs_sw_enum;
typedef osal_u8 mac_acs_sw_enum_uint8;

typedef enum {
    MAC_ACS_SET_CH_DNYA = 0x0,
    MAC_ACS_SET_CH_INIT = 0x1,

    MAC_ACS_SET_CH_BUTT
} mac_acs_set_ch_enum;
typedef osal_u8 mac_acs_set_ch_enum_uint8;

typedef enum {
    MAC_RSSI_LIMIT_SHOW_INFO,
    MAC_RSSI_LIMIT_ENABLE,
    MAC_RSSI_LIMIT_DELTA,
    MAC_RSSI_LIMIT_THRESHOLD,

    MAC_RSSI_LIMIT_TYPE_BUTT
} mac_rssi_limit_type_enum;
typedef osal_u8 mac_rssi_limit_type_enum_uint8;

typedef struct {
    oal_bool_enum_uint8     en_11k;
    oal_bool_enum_uint8     en_11v;
    oal_bool_enum_uint8     en_11r;
    oal_bool_enum_uint8     en_11r_ds;
} mac_device_voe_custom_stru;

typedef struct {
    osal_u8                 rx_ba_lut_idx_table[MAC_RX_BA_LUT_BMAP_LEN];
} mac_lut_table_stru;

typedef struct {
    osal_u8                 device_id[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP];
    osal_u8                 device_nums;
    osal_u8                 chip_id;
    oal_bool_enum_uint8     chip_state;
    osal_u32                chip_ver;
    hal_to_dmac_chip_stru   *hal_chip;
    mac_lut_table_stru      lut_table;
    void                    *p_alg_priv;

    frw_timeout_stru        active_user_timer;
} mac_chip_stru;

typedef struct {
    mac_chip_stru           chip[WLAN_CHIP_MAX_NUM_PER_BOARD];
    osal_u8                 chip_id_bitmap;
    osal_u8                 auc_resv[3];
} mac_board_stru;

typedef struct {
    osal_u16 num_networks;
    osal_u8 resv[2];
    osal_u8 bssid_array[WLAN_MAX_SCAN_BSS_PER_CH][WLAN_MAC_ADDR_LEN];
} mac_bss_id_list_stru;

typedef struct {
    mac_reset_sys_type_enum_uint8 reset_sys_type;
    osal_u8 value;
    osal_u8 resv[2];
} mac_reset_sys_stru;

typedef struct {
    osal_u16 num_networks;
    mac_ch_type_enum_uint8 ch_type;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_chan_status_enum_uint8 ch_status;
#else
    osal_u8 auc_resv[1];
#endif
} mac_ap_ch_info_stru;

typedef struct {
    wlan_nss_enum_uint8 nss_num : 4;
    wlan_bw_cap_enum_uint8 channel_width : 4;

    oal_bool_enum_uint8 nb_is_supp : 1;
    oal_bool_enum_uint8 is_supp_1024qam : 1;
    oal_bool_enum_uint8 mc_is_supp_80211 : 1;
    oal_bool_enum_uint8 ldpc_is_supp : 1;
    oal_bool_enum_uint8 tx_stbc_is_supp : 1;
    oal_bool_enum_uint8 rx_stbc_is_supp : 1;
    oal_bool_enum_uint8 su_bfmer_is_supp : 1;
    oal_bool_enum_uint8 su_bfmee_is_supp : 1;

    oal_bool_enum_uint8 mu_bfmer_is_supp : 1;
    oal_bool_enum_uint8 mu_bfmee_is_supp : 1;
    oal_bool_enum_uint8 switch_11ax : 1;
    oal_bool_enum_uint8 ba_bitmap_support_32bit : 1;
    osal_u8 mtid_aggregation_rx_support         : 3;
    oal_bool_enum_uint8 resv                    : 1;

    oal_bool_enum_uint8 resv1;
} mac_device_capability_stru;

typedef struct {
    osal_u32            cmd_bit_map;
    osal_u8             srb_switch;
    osal_u8             dto_switch;
    osal_u8             nfi_switch;
    osal_u8             apf_switch;
    osal_u8             ao_switch;
} mac_pm_debug_cfg_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    osal_u8                   ext_eid;
    osal_u8                   element_body_len;
    osal_u8                   *element_body;
} mac_11ax_element_stru;
#endif

typedef struct {
    osal_u32                         tx_seqnum;
    osal_u16                         seqnum_used_times;
    osal_u16                         incr_constant;
} mac_tx_seqnum_struc;

typedef struct {
    osal_u8 exceed_count; // 当前超过阈值的次数
    osal_u8 exceed_limit; // 超过阈值的限制次数 当exceed_count >= exceed_limit时, 信道随机化
    osal_bool randomize;  // 信道是否需要随机化
} hmac_bss_list_exceed_info_stru;

typedef struct {
    osal_spinlock lock;
    struct osal_list_head bss_list_head;
    osal_u32          bss_num;
    hmac_bss_list_exceed_info_stru bss_list_exceed_info;
} hmac_bss_mgmt_stru;

typedef struct {
    wlan_m2s_mgr_vap_stru m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM];
    osal_u8 blacklist_cnt;
} mac_m2s_ie_stru;

typedef struct {
    mac_rssi_limit_type_enum_uint8 rssi_limit_type;
    oal_bool_enum_uint8 rssi_limit_enable_flag;
    osal_s8 rssi;
    osal_s8 rssi_delta;
} mac_cfg_rssi_limit_stru;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef struct {
    oal_bool_enum_uint8 is_pk_mode;
    wlan_bw_cap_enum_uint8 curr_bw_cap;
    wlan_protocol_cap_enum_uint8 curr_protocol_cap;
    wlan_nss_enum_uint8 curr_num_spatial_stream;

    osal_u32 tx_bytes;
    osal_u32 rx_bytes;
    osal_u32 dur_time;
} mac_cfg_pk_mode_stru;
#endif

typedef struct {
    osal_u8                         auc_mac_addr[OAL_MAC_ADDR_LEN];
    osal_u8                         reserved[2];
    osal_u32                        cfg_time;
    osal_u32                        aging_time;
    osal_u32                        drop_counter;
} mac_blacklist_stru;

typedef struct {
    osal_u8                         auc_mac_addr[OAL_MAC_ADDR_LEN];
    osal_u8                         reserved[2];
    osal_u32                        cfg_time;
    osal_u32                        asso_counter;
} mac_autoblacklist_stru;

typedef struct {
    osal_u8                         enabled;
    osal_u8                         list_num;
    osal_u8                         reserved[2];
    osal_u32                        ul_threshold;
    osal_u32                        reset_time;
    osal_u32                        aging_time;
    mac_autoblacklist_stru          autoblack_list[WLAN_BLACKLIST_MAX];
} mac_autoblacklist_info_stru;

typedef struct {
    osal_u8                         mode;
    osal_u8                         list_num;
    osal_u8                         blacklist_vap_index;
    osal_u8                         blacklist_device_index;
    mac_autoblacklist_info_stru     autoblacklist_info;
    mac_blacklist_stru              black_list[WLAN_BLACKLIST_MAX];
} mac_blacklist_info_stru;

typedef struct {
    osal_u8                    p2p_mode;
    mac_vap_state_enum_uint8   last_vap_state;
    osal_u8                    roc_need_switch;
    osal_u8                    p2p_ps_pause;
    oal_net_device_stru       *p2p_net_device;
    osal_u64                   ull_send_action_id;
    osal_u64                   ull_last_roc_id;
    oal_ieee80211_channel_stru st_listen_channel;
    oal_nl80211_channel_type   listen_channel_type;
    oal_net_device_stru       *primary_net_device;
    oal_net_device_stru       *second_net_device;
} mac_p2p_info_stru;

typedef enum {
    MAC_SCAN_SUCCESS = 0,
    MAC_SCAN_PNO     = 1,
    MAC_SCAN_TIMEOUT = 2,
    MAC_SCAN_REFUSED = 3,
    MAC_SCAN_ABORT   = 4,
    MAC_SCAN_ABORT_SYNC = 5,
    MAC_SCAN_STATUS_BUTT,
} mac_scan_status_enum;
typedef osal_u8 mac_scan_status_enum_uint8;

typedef struct {
    mac_scan_status_enum_uint8  scan_rsp_status;
    osal_u8                     resv[3];
    osal_u64                    cookie;
} mac_scan_rsp_stru;

typedef osal_void (*mac_scan_cb_fn)(osal_void *p_scan_record);

#pragma pack(push, 1)
typedef struct {
    osal_s32                            rssi;
    osal_s8                             ant0_rssi;
    osal_s8                             ant1_rssi;
    wlan_mib_desired_bsstype_enum_uint8 bss_type;

    osal_u8                             channel;
    osal_s8                             snr_ant0;
    osal_s8                             snr_ant1;

    osal_u8                             resv[2];

    osal_u32                            parent_tsf;

    wlan_nss_enum_uint8                 support_max_nss;
    oal_bool_enum_uint8                 support_opmode;
    osal_u8                             num_sounding_dim;
    osal_u8                             resv1;
} mac_scanned_result_extend_info_stru;
#pragma pack(pop)

typedef struct {
    hmac_bss_mgmt_stru          bss_mgmt;
    wlan_scan_chan_stats_stru   chan_results[WLAN_MAX_CHANNEL_NUM];
    osal_u8                     chan_numbers;
    osal_u8                     device_id : 4;
    osal_u8                     chip_id   : 4;
    osal_u8                     vap_id;
    mac_scan_status_enum_uint8  scan_rsp_status;

    oal_time_us_stru            scan_start_timestamp;
    mac_scan_cb_fn              p_fn_cb;

#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_u8                     is_any_scan;
    osal_u8                     n_is_mesh_sta_scan;
    osal_u8                     resv[2];
#endif

    osal_u64                    ull_cookie;
    mac_vap_state_enum_uint8    vap_last_state;
    osal_u64                    scan_start_time;
    osal_u8 is_chan_meas_scan;
    osal_u8 rsv[3];
} hmac_scan_record_stru;

typedef struct {
    oal_bool_enum_uint8                     is_scanning;
    osal_u8 is_random_mac_addr_scan;
    oal_bool_enum_uint8                     complete;
    oal_bool_enum_uint8                     sched_scan_complete;

    oal_cfg80211_scan_request_stru          *request;
    oal_cfg80211_sched_scan_request_stru    *sched_scan_req;
    osal_wait wait_queue;
    osal_mutex                              scan_request_mutexlock;

    frw_timeout_stru                        scan_timeout;
    osal_u8                                 random_mac[WLAN_MAC_ADDR_LEN];
    osal_u8                                 auc_resv[2];

    hmac_scan_record_stru                   scan_record_mgmt;

    osal_u32                                scan_2g_ch_list_map;
} hmac_scan_stru;

typedef struct {
    wlan_mib_desired_bsstype_enum_uint8 bss_type;
    osal_u8                             dtim_period;
    osal_u8                             dtim_cnt;
    oal_bool_enum_uint8                 en_11ntxbf;

    oal_bool_enum_uint8                 new_scan_bss;
    osal_u8                             multi_bssid_capa;
    osal_s8                             c_rssi;
    osal_char                           ac_ssid[WLAN_SSID_MAX_LEN];
    osal_u16                            beacon_period;
    osal_u16                            us_cap_info;
    osal_u8                             auc_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8                             bssid[WLAN_MAC_ADDR_LEN];
    mac_channel_stru                    st_channel;
    osal_u8                             wmm_cap;
    osal_u8                             uapsd_cap;
    oal_bool_enum_uint8                 desired;
    osal_u8                             num_supp_rates;
    osal_u8                             supp_rates[WLAN_USER_MAX_SUPP_RATES];

    osal_u8                             need_drop;
    osal_u8                             bss_assoc_disallowed;
    osal_u8                             resv3[2];

#ifdef _PRE_WLAN_FEATURE_11D
    osal_s8                             ac_country[WLAN_COUNTRY_STR_LEN];
    osal_u8                             resv2[1];
    osal_u8                             *country_ie;
#endif

    osal_u8                             *rsn_ie;
    osal_u8                             *wpa_ie;

    oal_bool_enum_uint8                 ht_capable;
    oal_bool_enum_uint8                 vht_capable;
    oal_bool_enum_uint8                 vendor_vht_capable;
    wlan_bw_cap_enum_uint8              bw_cap;
    wlan_channel_bandwidth_enum_uint8   channel_bandwidth;
    osal_u8                             coex_mgmt_supp;
    oal_bool_enum_uint8                 ht_ldpc;
    oal_bool_enum_uint8                 ht_stbc;
    osal_u8                             wapi;
    osal_u8                             vendor_novht_capable;
#ifdef _PRE_WLAN_MP13_DDC_BUGFIX
    oal_bool_enum_uint8                 ddc_whitelist_chip_oui;
#else
    osal_u8                             resv0;
#endif
    oal_bool_enum_uint8                 btcoex_ps_blacklist_chip_oui;
    osal_u32                            timestamp;
    osal_u8                             phy_type;
    osal_s8                             ac_rsni[2];
    osal_u8                             auc_resv3[1];
    osal_u32                            parent_tsf;
#ifdef _PRE_WLAN_WIRELESS_EXT
    osal_u16                          ht_cap_info;
    wlan_bw_cap_enum_uint8            real_bw;
    osal_u8                          *wps_ie;
    osal_u8                          *wapi_ie;
    osal_u8                           resv4[3];
#endif

    wlan_nss_enum_uint8                 support_max_nss;
    oal_bool_enum_uint8                 support_opmode;
    osal_u8                             num_sounding_dim;
    osal_u8                             resv1;

    oal_bool_enum_uint8                 he_capable;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_11ax_element_stru               multiple_bssid_configuration;
    mac_11ax_element_stru               he_capabilities;
    mac_11ax_element_stru               he_operation;
    mac_11ax_element_stru               uora_parameter_set;
    mac_11ax_element_stru               bss_color_change_announcement;
    mac_11ax_element_stru               spatial_reuse;
    mac_11ax_element_stru               mu_edca_parameter_set;
    mac_11ax_element_stru               ess_report;
    mac_11ax_element_stru               ndp_feedback_report_parameter_set;
    mac_11ax_element_stru               he_bss_load;
    mac_11ax_element_stru               he_6ghz_band_capabilities;
#endif

    oal_bool_enum_uint8                 support_rrm;
#ifdef _PRE_WLAN_FEATURE_1024QAM
    oal_bool_enum_uint8                 support_1024qam;
#endif
    osal_s8                             c_ant0_rssi;
    osal_s8                             c_ant1_rssi;

    mac_scanned_all_bss_info            all_bss_info;

    osal_u32                            mgmt_len;
    osal_u8                             mgmt_buff[MAC_80211_QOS_HTC_4ADDR_FRAME_LEN];
} mac_bss_dscr_stru;

typedef struct {
    wlan_mib_desired_bsstype_enum_uint8 bss_type;
    wlan_scan_type_enum_uint8 scan_type;
    osal_u8 bssid_num;
    osal_u8 ssid_num;

    osal_u8 sour_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8 p2p0_listen_channel;
    osal_u8 max_scan_count_per_channel;

    osal_u8 bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN];
    mac_ssid_stru mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];

    osal_u8 max_send_probe_req_count_per_channel;
    osal_u8 is_p2p0_scan : 1;
    osal_u8 is_radom_mac_saved : 1;
    osal_u8 radom_mac_saved_to_dev : 2;
    osal_u8 desire_fast_scan : 1;
    osal_u8 single_probe_times : 3;

    oal_bool_enum_uint8 abort_scan_flag;
    oal_bool_enum_uint8 is_random_mac_addr_scan;

    oal_bool_enum_uint8 need_switch_back_home_channel;
    osal_u8 scan_channel_interval;
    osal_u16 work_time_on_home_channel;

    mac_channel_stru channel_list[WLAN_MAX_CHANNEL_NUM];

    osal_u8 channel_nums;
    osal_u8 probe_delay;
    osal_u16 scan_time;

    wlan_scan_mode_enum_uint8 scan_mode;
    osal_u8 scan_flag;
    osal_u8 scan_func;
    osal_u8 vap_id;
    osal_u64 cookie;

    mac_scan_cb_fn fn_cb;
} mac_scan_req_stru;

typedef struct {
    frw_timeout_stru    rx_dscr_opt_timer;
    osal_u32            rx_pkt_num;
    osal_u32            rx_pkt_opt_limit;
    osal_u32            rx_pkt_reset_limit;
    oal_bool_enum_uint8 dscr_opt_state;
    oal_bool_enum_uint8 dscr_opt_enable;
} hmac_rx_dscr_opt_stru;

typedef struct {
    wlan_nss_enum_uint8                 nss_num;
    wlan_mib_mimo_power_save_enum_uint8 smps_mode;
    osal_u8                             reserved[2];
} mac_device_m2s_stru;

typedef struct {
    mac_m2s_mode_enum_uint8 cfg_m2s_mode;
    union {
        struct {
            oal_bool_enum_uint8 m2s_type;
            osal_u8 master_id;
            mac_m2s_command_state_enum_uint8 m2s_state;
            wlan_m2s_trigger_mode_enum_uint8 trigger_mode;
        } test_mode;

        struct {
            oal_bool_enum_uint8 mss_on;
        } mss_mode;

        struct {
            osal_u8 opt;
            osal_char value;
        } rssi_mode;
    } pri_data;
} mac_m2s_mgr_stru;

typedef struct {
    osal_u8 user_idx;
    osal_u8 enable;
    osal_u8 cfg_match_ta_ra_sel;
    osal_u8 resv;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8 frame_type_filter_bitmap;
    osal_u8 sub_type_filter_enable;
    osal_u8 sub_type_filter;
    osal_u8 ppdu_filter_bitmap;
    osal_u16 period;
} mac_csi_usr_config_stru;

typedef struct {
    osal_u8 csi_data_blk_num;
    osal_u16 csi_data_max_len;
    osal_u8 resv[1];
} mac_csi_buffer_stru;

typedef struct {
    osal_u8     category;
    osal_u8     action_code;
    osal_u8     oui[3];
    osal_u8     eid;
    osal_u8     lenth;
    osal_u8     location_type;
    osal_u8     mac_server[WLAN_MAC_ADDR_LEN];
    osal_u8     mac_client[WLAN_MAC_ADDR_LEN];
    osal_u8     payload[4];
} mac_location_event_stru;

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN

typedef struct {
    osal_u8 ssid[WLAN_SSID_MAX_LEN];
    oal_bool_enum_uint8 scan_ssid;
    osal_u8 auc_resv[2];
} pno_match_ssid_stru;

typedef struct {
    pno_match_ssid_stru match_ssid_set[MAX_PNO_SSID_COUNT];
    osal_s32 ssid_count;
    osal_s32 rssi_thold;
    osal_u32 pno_scan_interval;
    osal_u8 sour_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8 pno_scan_repeat;
    oal_bool_enum_uint8 is_random_mac_addr_scan;

    mac_scan_cb_fn fn_cb;
} mac_pno_scan_stru;

typedef struct {
    wlan_mib_desired_bsstype_enum_uint8 bss_type;
    wlan_scan_type_enum_uint8 scan_type;
    wlan_scan_mode_enum_uint8 scan_mode;
    osal_u8 probe_delay;
    osal_u8 scan_func;
    osal_u8 max_scan_count_per_channel;
    osal_u8 max_send_probe_req_count_per_channel;
    osal_u8 scan_flag;
    osal_u16 scan_time;
    osal_u16 work_time_on_home_channel;
} mac_pno_scan_ctrl;

typedef struct {
    mac_pno_scan_stru sched;
    mac_pno_scan_ctrl ctrl;
} mac_pno_scan_param;


typedef struct {
    mac_pno_scan_param pno_sched_scan_params;
    osal_void *hmac_vap;
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_timer pno_sched_scan_timer;
    osal_u32 pno_sched_scan_timer_id;
#endif
    osal_u8 curr_pno_sched_scan_times;
    oal_bool_enum_uint8 is_found_match_ssid;
    osal_u8 auc_resv[2];
} mac_pno_sched_scan_mgmt_stru;
#endif

typedef struct {
    osal_u8 vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    osal_u8 resv;

    osal_u8 cfg_vap_id   : 4;
    osal_u8 vap_num      : 4;
    osal_u8 up_vap_num   : 4;
    osal_u8 mac_vap_id   : 4;
    osal_u8 sta_num;

    oal_bool_enum_uint8 device_state            : 1;
    oal_bool_enum_uint8 is_random_mac_addr_scan : 1;
    osal_u8 resv0                               : 6;

    wlan_protocol_cap_enum_uint8 protocol_cap        : 4;
    wlan_band_cap_enum_uint8 band_cap                : 4;
    wlan_channel_band_enum_uint8 max_band            : 4;
    wlan_channel_bandwidth_enum_uint8 max_bandwidth  : 4;

    osal_u8 max_channel;

    osal_u8 hw_addr[WLAN_MAC_ADDR_LEN];
    osal_u8 mac_oui[WLAN_RANDOM_MAC_OUI_LEN];

    mac_data_rate_stru mac_rates_11g[MAC_DATARATES_PHY_80211G_NUM];

    mac_scan_req_stru scan_params;
    frw_timeout_stru obss_scan_timer;

    oal_wiphy_stru *wiphy;

    mac_bss_id_list_stru bss_id_list;

    mac_device_capability_stru device_cap;

    mac_ap_ch_info_stru ap_channel_list[MAC_MAX_SUPP_CHANNEL];

    osal_u8 device_id;
    osal_u8 chip_id;
    osal_u8 uc_in_suspend;
    osal_u8 wapi;
    hmac_scan_stru scan_mgmt;
    hmac_rx_dscr_opt_stru rx_dscr_opt;

    mac_p2p_info_stru st_p2p_info;
#ifdef _PRE_WLAN_FEATURE_SMPS
    wlan_mib_mimo_power_save_enum_uint8 mac_smps_mode;
#endif

#ifdef _PRE_WIFI_DMT
    osal_u8 desired_bss_num;
    osal_u8 desired_bss_idx[WLAN_MAX_SCAN_BSS_NUM];
#endif
    osal_wait netif_change_event;

#ifdef _PRE_WLAN_TCP_OPT
    oal_bool_enum_uint8 sys_tcp_rx_ack_opt_enable;
    oal_bool_enum_uint8 sys_tcp_tx_ack_opt_enable;
#endif
    oal_bool_enum_uint8 start_via_priv : 1;
    oal_bool_enum_uint8 en_dbac_enabled : 1;
    oal_bool_enum_uint8 en_dbac_running : 1;
    osal_u8 resv3                      : 5;

#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u8 set_mode_iftype;
    osal_u8 p2p_listen_channel;
    osal_u16 beacon_interval;
    osal_u32 p2p_intf_status;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
#endif

    osal_void **hmac_device_feature_arr;

    oal_bool_enum_uint8 in_suspend            : 1;
    oal_bool_enum_uint8 txop_enable           : 1;
    mac_scan_state_enum_uint8 curr_scan_state : 4;
    osal_u8 resv4                             : 2;

    osal_u16 aus_ac_mpdu_num[WLAN_WME_AC_BUTT];
    osal_u16 total_mpdu_num;
    osal_u16 psm_mpdu_num;

    osal_u32 scan_timestamp;

    mac_fcs_cfg_stru fcs_cfg;
    mac_fcs_mgr_stru fcs_mgr;

    osal_u8 ra_lut_index_table[WLAN_ACTIVE_USER_IDX_BMAP_LEN];

    frw_timeout_stru keepalive_timer;
    mac_cfg_rssi_limit_stru rssi_limit;

    hal_to_dmac_device_stru *hal_device[WLAN_DEVICE_MAX_NUM_PER_CHIP];

    oal_bool_enum_uint8 wmm                  : 1;
    oal_bool_enum_uint8 en_2040bss_switch    : 1;
    oal_bool_enum_uint8 arpoffload_switch    : 1;
    oal_bool_enum_uint8 dbac_enabled         : 1;
    oal_bool_enum_uint8 dbac_running         : 1;
    oal_bool_enum_uint8 intol_bit_recd_40mhz : 1;
    oal_bool_enum_uint8 sniffer_switch       : 1;
    oal_bool_enum_uint8 en_wmm               : 1;

    oal_bool_enum_uint8 arp_offload_switch;
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    mac_pno_sched_scan_mgmt_stru *pno_sched_scan_mgmt;
#endif
    mac_channel_stru p2p_vap_channel;

    osal_u32 vap_pause_bitmap;
    osal_u8 rx_ba_lut_idx_table[MAC_RX_BA_LUT_BMAP_LEN];

#if (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
    osal_spinlock lock;
#endif
} hmac_device_stru;

typedef struct {
    hmac_device_stru *hmac_device;
} mac_wiphy_priv_stru;

osal_u32 mac_device_init_etc(hmac_device_stru *hmac_device, osal_u8 device_id);

osal_u32 mac_device_exit_etc(hmac_device_stru *hmac_device);
osal_void hmac_chip_inc_active_user(hmac_device_stru *hmac_device);
osal_void hmac_chip_dec_active_user(hmac_device_stru *hmac_device);

wlan_mib_vht_supp_width_enum_uint8 hmac_device_trans_bandwith_to_vht_capinfo(wlan_bw_cap_enum_uint8 max_op_bd);

void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 vap_mode, osal_u8 dev_id, osal_u8 chip_id,
    osal_u8 vap_id, mac_blacklist_info_stru **blacklist_info);

osal_void *hmac_device_get_all_rates_etc(const hmac_device_stru *hmac_device);

osal_u32 hmac_device_check_5g_enable(osal_u8 device_id);

mac_device_capability_stru *mac_device_get_capability(osal_void);
mac_device_voe_custom_stru *mac_get_pst_mac_voe_custom_param(osal_void);
osal_void mac_device_set_random_scan_oui(osal_u32 scan_oui);
osal_void mac_set_pst_mac_voe_custom_over_ds_param(osal_u8 over_ds_en);
osal_u8 mac_get_user_res_num(osal_void);
osal_u8 *mac_get_user_res_num_addr(osal_void);
wlan_bw_cap_enum_uint8 hmac_get_device_bw_mode(const hal_to_dmac_device_stru *hal_device);


static INLINE__ oal_bool_enum_uint8 mac_is_dbac_enabled(const hmac_device_stru *hmac_device)
{
    return hmac_device->en_dbac_enabled;
}

static INLINE__  oal_bool_enum_uint8 mac_is_dbac_running(const hmac_device_stru *hmac_device)
{
    if (hmac_device->en_dbac_enabled == OAL_FALSE) {
        return OAL_FALSE;
    }

    return hmac_device->en_dbac_running;
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
static INLINE__  oal_bool_enum_uint8 mac_get_2040bss_switch(hmac_device_stru *hmac_device)
{
    return hmac_device->en_2040bss_switch;
}
static INLINE__  void mac_set_2040bss_switch(hmac_device_stru *hmac_device, oal_bool_enum_uint8 en_switch)
{
    hmac_device->en_2040bss_switch = en_switch;
}
#endif


static INLINE__ oal_bool_enum_uint8 mac_device_check_5g_enable_per_chip(void)
{
    osal_u8            dev_idx    = 0;

    while (dev_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) {
        if (hmac_device_check_5g_enable(dev_idx) != 0) {
            return OAL_TRUE;
        }
        dev_idx++;
    }

    return OAL_FALSE;
}

static INLINE__ oal_bool_enum_uint8  mac_is_hide_ssid(const osal_u8 *ssid_ie, osal_u8 ssid_len)
{
    return (oal_bool_enum_uint8)((ssid_ie == OAL_PTR_NULL) || (ssid_len == 0) || (ssid_ie[0] == '\0'));
}

static INLINE__ oal_bool_enum_uint8  mac_device_is_auto_chan_sel_enabled(const hmac_device_stru *hmac_device)
{
    if (hmac_device->max_channel == 0) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

static INLINE__ oal_bool_enum_uint8 mac_device_is_scaning(const hmac_device_stru *hmac_device)
{
    return (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) ? OSAL_TRUE : OSAL_FALSE;
}

static INLINE__ oal_bool_enum_uint8 mac_device_is_listening(const hmac_device_stru *hmac_device)
{
    return ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
    ((hmac_device->scan_params.scan_func & MAC_SCAN_FUNC_P2P_LISTEN) != 0)) ? OSAL_TRUE : OSAL_FALSE;
}

static INLINE__ wlan_mib_mimo_power_save_enum_uint8 mac_m2s_cali_smps_mode(wlan_nss_enum_uint8 nss)
{
    return (nss == WLAN_SINGLE_NSS) ? WLAN_MIB_MIMO_POWER_SAVE_STATIC : WLAN_MIB_MIMO_POWER_SAVE_MIMO;
}

#ifdef _PRE_WLAN_FEATURE_M2S
static INLINE__ wlan_nss_enum_uint8 mac_m2s_cali_nss_from_smps_mode(wlan_mib_mimo_power_save_enum_uint8 smps_mode)
{
    return (smps_mode == WLAN_MIB_MIMO_POWER_SAVE_STATIC) ? WLAN_SINGLE_NSS : WLAN_DOUBLE_NSS;
}
#endif

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif
