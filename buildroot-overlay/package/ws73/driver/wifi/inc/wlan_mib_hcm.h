/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Header file for hmac_tcp_opt_struc.c.
 * Create: 2022-12-15
 */
#ifndef __WLAN_MIB_HCM_H__
#define __WLAN_MIB_HCM_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "wlan_types_common.h"
#include "wlan_mib_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef osal_u16 wlan_cfgid_enum_uint16;

/* Start of dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 } */
typedef struct {
    osal_u8           dot11_station_id[WLAN_MAC_ADDR_LEN];          /* dot11StationID MacAddress, */
    osal_u8           resv[2];

    osal_u32          dot11_authentication_response_time_out;    /* dot11AuthenticationResponseTimeOut Unsigned32, */
    osal_u32          dot11_beacon_period;                     /* dot11BeaconPeriod Unsigned32, */
    osal_u32          dot11_dtim_period;                       /* dot11DTIMPeriod Unsigned32, */
    osal_u32          dot11_association_response_time_out;       /* dot11AssociationResponseTimeOut Unsigned32, */
    osal_u32    dot11_association_sa_query_maximum_timeout; /* dot11AssociationSAQueryMaximumTimeout Unsigned32 */
    osal_u32          dot11_association_sa_query_retry_timeout; /* dot11AssociationSAQueryRetryTimeout Unsigned32 */

    osal_u8           dot11_desired_ssid[32 + 1];             /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    osal_u16           dot11_max_assoc_user_nums;

    oal_bool_enum_uint8  dot11_tx_aggregate_actived;                  /* VAP协议能力是否支持发送聚合 */
    oal_bool_enum_uint8  dot11_cfg_ampdu_tx_ative;                     /* 配置命令是否允许发送聚合 */
    oal_bool_enum_uint8  dot11_amsdu_plus_ampdu_active;                 /* ADDBA REQ中标志是否支持amsdu */
    oal_bool_enum_uint8  dot11_cfg_amsdu_tx_ative;                     /* 配置命令是否支持amsdu */

    oal_bool_enum_uint8  dot11_amsdu_aggregate_ative;                 /* VAP协议能力是否支持amsdu发送聚合 */
    osal_u8            dot11_rx_ba_session_number;                   /* 该vap下，rx BA会话的数目 */
    osal_u8            dot11_tx_ba_session_number;                   /* 该vap下，tx BA会话的数目 */

    wlan_auth_alg_mode_enum_uint8        dot11_authentication_mode;          /* 认证算法 */
    wlan_addba_mode_enum_uint8           dot11_add_ba_mode;
    wlan_mib_desired_bsstype_enum_uint8  dot11_desired_bss_type;        /* dot11DesiredBSSType INTEGER, */
    wlan_mib_pwr_mgmt_mode_enum_uint8    dot11_power_management_mode;   /* dot11PowerManagementMode INTEGER, */

    osal_u8            dot11_sta_auth_count;                        /* 记录STA发起关联的次数 */
    osal_u8            dot11_sta_assoc_count;
    osal_u8            dot11_support_rate_set_nums;
    osal_u8            dot11_tx_traffic_classify_flag;               /* 业务识别功能开关 */

    /* Byte1. All TruthValue */
    oal_bool_enum_uint8 dot11_multi_domain_capability_activated      : 1; /* dot11MultiDomainCapabilityActivated */
    oal_bool_enum_uint8 dot11_spectrum_management_implemented       : 1; /* dot11SpectrumManagementImplemented */
    oal_bool_enum_uint8 dot11_spectrum_management_required          : 1; /* dot11SpectrumManagementRequired */
    oal_bool_enum_uint8 dot11_qos_option_implemented                : 1; /* dot11QosOptionImplemented */
    /* dot11ImmediateBlockAckOptionImplemented */
    oal_bool_enum_uint8 dot11_immediate_block_ack_option_implemented  : 1;
    oal_bool_enum_uint8 dot11_delayed_block_ack_option_implemented : 1; /* dot11DelayedBlockAckOptionImplemented */
    oal_bool_enum_uint8 dot11_apsd_option_implemented               : 1; /* dot11APSDOptionImplemented */
    oal_bool_enum_uint8 dot11_qbss_load_implemented                 : 1; /* dot11QBSSLoadImplemented */
    /* Byte2. All TruthValue */
    oal_bool_enum_uint8 dot11_radio_measurement_activated           : 1; /* dot11RadioMeasurementActivated */
    oal_bool_enum_uint8 dot11_extended_channel_switch_activated      : 1; /* dot11ExtendedChannelSwitchActivated */
    oal_bool_enum_uint8 dot11_rm_link_measurement_activated          : 1; /* dot11RMLinkMeasurementActivated */
    /* dot11RMBeaconPassiveMeasurementActivated */
    oal_bool_enum_uint8 dot11_rm_beacon_passive_measurement_activated : 1;
    /* dot11RMBeaconActiveMeasurementActivated */
    oal_bool_enum_uint8 dot11_rm_beacon_active_measurement_activated  : 1;
    /* dot11RMBeaconTableMeasurementActivated */
    oal_bool_enum_uint8 dot11_rm_beacon_table_measurement_activated   : 1;
    oal_bool_enum_uint8 dot11_wireless_management_implemented       : 1; /* dot11WirelessManagementImplemented */
    oal_bool_enum_uint8 resv1                                    : 1;
    /* Byte3 */
    oal_bool_enum_uint8 dot112040_switch_prohibited                : 1;
    oal_bool_enum_uint8 dot11_wps_active                           : 1;
    oal_bool_enum_uint8 resv2                                       : 6;
    /* Byte4 */
    osal_u8             dot11_80211i_mode;   /* 指示当前的方式时WPA还是WPA2, bit0 = 1,WPA; bit1 = 1, RSN */

    oal_bool_enum_uint8 dot11_rm_neighbor_report_activated;                          /* TruthValue */
    oal_bool_enum_uint8 dot11_rm_channel_load_measurement_activated;                  /* TruthValue, */

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    oal_bool_enum_uint8 dot11_wur_option_implemented;                               /* TruthValue, */
#endif

    wlan_bw_cap_enum_uint8 dot11_vap_max_band_width;      /* 当前vap可用的最大带宽能力 */
} wlan_mib_dot11stationconfigentry_stru; /* Dot11StationConfigEntry */

/******************************************************************************/
/* dot11WEPDefaultKeys  TABLE - members of Dot11WEPDefaultKeysEntry           */
/******************************************************************************/
/* Conceptual table for WEP default keys. This table contains the four WEP    */
/* default secret key values corresponding to the four possible KeyID values. */
/* The WEP default secret keys are logically WRITE-ONLY. Attempts to read the */
/* entries in this table return unsuccessful status and values of null or 0.  */
/* The default value of each WEP default key is null.                         */
typedef struct {
    osal_u8  dot11_wep_default_key_value[WLAN_MAX_WEP_STR_SIZE];    /* dot11WEPDefaultKeyValue WEPKeytype  */
    osal_u8  rsv;
} wlan_mib_dot11wepdefaultkeysentry_stru;

/******************************************************************************/
/* dot11RSNAConfig TABLE (RSNA and TSN) - members of dot11RSNAConfigEntry     */
/******************************************************************************/
/* An entry in the dot11RSNAConfigTable                                       */
typedef struct {
    osal_u8             dot11_rsna_config_ptksa_replay_counters; /* Unsigned32, */
    osal_u8             dot11_rsna_config_gtksa_replay_counters; /* Unsigned32, */
    osal_u32            wpa_group_suite;
    osal_u32            rsn_group_suite;
    osal_u32            wpa_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    osal_u32            wpa_akm_suites[WLAN_AUTHENTICATION_SUITES];
    osal_u32            rsn_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    osal_u32            rsn_akm_suites[WLAN_AUTHENTICATION_SUITES];
    osal_u32            rsn_group_mgmt_suite;
} wlan_mib_dot11rsnaconfigentry_stru;

/******************************************************************************/
/* dot11WEPKeyMappings  TABLE - members of Dot11WEPKeyMappingsEntry           */
/******************************************************************************/
/* Conceptual table for WEP Key Mappings. The MIB supports the ability to     */
/* share a separate WEP key for each RA/TA pair. The Key Mappings Table con-  */
/* tains zero or one entry for each MAC address and contains two fields for   */
/* each entry: WEPOn and the corresponding WEP key. The WEP key mappings are  */
/* logically WRITE-ONLY. Attempts to read the entries in this table return    */
/* unsuccessful status and values of null or 0. The default value for all     */
/* WEPOn fields is false                                                      */
/******************************************************************************/
/* dot11Privacy TABLE - members of Dot11PrivacyEntry                          */
/******************************************************************************/
/* Group containing attributes concerned with IEEE 802.11 Privacy. Created    */
/* as a table to allow multiple instantiations on an agent.                   */
typedef struct {
    /* dot11PrivacyInvoked TruthValue */
    oal_bool_enum_uint8 dot11_privacy_invoked                 : 1;
    /* dot11RSNAActivated TruthValue */
    oal_bool_enum_uint8 dot11_rsna_activated                  : 1;
    /* dot11RSNAPreauthenticationActivated TruthValue */
    oal_bool_enum_uint8 dot11_rsna_preauthentication_activated : 1;
    /* dot11RSNAManagementFramerProtectionCapbility TruthValue  */
    oal_bool_enum_uint8 dot11_rsnamfpc                       : 1;
    /* dot11RSNAManagementFramerProtectionRequired TruthValue  */
    oal_bool_enum_uint8 dot11_rsnamfpr                       : 1;
    oal_bool_enum_uint8 en_resv                                : 3;
    /* dot11WEPDefaultKeyID Unsigned8 */
    osal_u8             dot11_wep_default_key_id;
    osal_u8             reserve[2];
    wlan_mib_dot11wepdefaultkeysentry_stru wlan_mib_wep_dflt_key[WLAN_NUM_DOT11WEPDEFAULTKEYVALUE];
    wlan_mib_dot11rsnaconfigentry_stru     wlan_mib_rsna_cfg;
} wlan_mib_dot11privacyentry_stru;

/**************************************************************************************/
/* dot11FastBSSTransitionConfig TABLE  - members of Dot11FastBSSTransitionConfigEntry */
/**************************************************************************************/
/* The table containing fast BSS transition configuration objects                     */
typedef struct {
    oal_bool_enum_uint8 dot11_fast_bss_transition_activated  : 1; /* dot11FastBSSTransitionActivated TruthValue, */
    oal_bool_enum_uint8 dot11_ft_over_ds_activated           : 1; /* dot11FTOverDSActivated TruthValue,          */
    oal_bool_enum_uint8 dot11_ft_resource_request_supported  : 1; /* dot11FTResourceRequestSupported TruthValue, */
    oal_bool_enum_uint8 en_resv                             : 5;

    osal_u8           resv;
    osal_u16          dot11_ft_mobility_domain_id;      /* dot11FTMobilityDomainID OCTET STRING,SIZE(2) */
} wlan_mib_dot11fastbsstransitionconfigentry_stru;

/**************************************************************************************/
/* dot11HTStationConfig TABLE  - members of Dot11HTStationConfigEntry                 */
/**************************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple            */
/* instances on an agent.                                                             */
typedef struct {
    oal_bool_enum_uint8 dot11_ht_control_field_supported             : 1; /* TruthValue, */
    oal_bool_enum_uint8 dot11_rd_responder_option_implemented        : 1; /* TruthValue, */
    oal_bool_enum_uint8 dot11_pco_option_implemented                : 1; /* TruthValue, */
    oal_bool_enum_uint8 dot11_lsig_txop_protection_option_implemented : 1; /* TruthValue, */
    oal_bool_enum_uint8 dot11_high_throughput_option_implemented     : 1; /* TruthValue, */
    osal_u8             dot11_minimum_mpdu_start_spacing             : 3; /* Unsigned32, */
    osal_u8             resv[3];

    wlan_mib_max_amsdu_lenth_enum_uint16 dot11_max_amsdu_length;    /* INTEGER, */
    wlan_mib_mimo_power_save_enum_uint8 dot11_mimo_power_save;      /* INTEGER, */
    wlan_mib_mcs_feedback_opt_implt_enum_uint8 dot11_mcs_feedback_option_implemented; /* INTEGER, */

    osal_u32          dot11_max_rx_ampdu_factor;  /* Unsigned32, */
    osal_u32          dot11_transition_time;    /* Unsigned32, */
} wlan_mib_dot11htstationconfigentry_stru;

/**************************************************************************************/
/* dot11WirelessMgmtOptions TABLE  - members of Dot11WirelessMgmtOptionsEntry         */
/**************************************************************************************/
/* Wireless Management attributes. In tabular form to allow for multiple      */
/* instances on an agent. This table only applies to the interface if         */
/* dot11WirelessManagementImplemented is set to true in the                   */
/* dot11StationConfigTable. Otherwise this table should be ignored.           */
/* For all Wireless Management features, an Activated MIB variable is used    */
/* to activate/enable or deactivate/disable the corresponding feature.        */
/* An Implemented MIB variable is used for an optional feature to indicate    */
/* whether the feature is implemented. A mandatory feature does not have a    */
/* corresponding Implemented MIB variable. It is possible for there to be     */
/* multiple IEEE 802.11 interfaces on one agent, each with its unique MAC     */
/* address. The relationship between an IEEE 802.11 interface and an          */
/* interface in the context of the Internet standard MIB is one-to-one.       */
/* As such, the value of an ifIndex object instance can be directly used      */
/* to identify corresponding instances of the objects defined herein.         */
/* ifIndex - Each IEEE 802.11 interface is represented by an ifEntry.         */
/* Interface tables in this MIB module are indexed by ifIndex.                */
typedef struct {
    oal_bool_enum_uint8 dot11_mgmt_option_multi_bssid_implemented;           /* TruthValue, */
    oal_bool_enum_uint8 dot11_mgmt_option_bss_transition_implemented;        /* TruthValue, */
    oal_bool_enum_uint8 dot11_mgmt_option_bss_transition_activated;          /* TruthValue, */
    oal_bool_enum_uint8 dot11_fine_timing_msmt_init_activated;               /* TruthValue, */
    oal_bool_enum_uint8 dot11_fine_timing_msmt_resp_activated;               /* TruthValue, */
    oal_bool_enum_uint8 dot11_rm_fine_timing_msmt_range_rep_activated;         /* TruthValue, */
    osal_u8 resv[2];
} wlan_mib_dot11wirelessmgmtoptionsentry_stru;

/****************************************************************************/
/* dot11VHTStationConfig TABLE  - members of Dot11VHTStationConfigEntry */
/****************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple   */
/* instances on an agent. */
typedef struct {
    osal_u32            dot11_max_mpdu_length;                         /* INTEGER, */
    osal_u32            dot11_vht_max_rx_ampdu_factor;                   /* Unsigned32, */
    osal_u32            dot11_vht_rx_highest_data_rate_supported;         /* Unsigned32, */
    osal_u32            dot11_vht_tx_highest_data_rate_supported;         /* Unsigned32, */
    osal_u16            dot11_vht_tx_mcs_map;                           /* OCTET STRING, */
    osal_u16            dot11_vht_rx_mcs_map;                           /* OCTET STRING, */
    oal_bool_enum_uint8 dot11_vht_control_field_supported;              /* TruthValue, */
    oal_bool_enum_uint8 dot11_vhttxop_power_save_option_implemented;     /* TruthValue, */
    oal_bool_enum_uint8 dot11_vht_option_implemented;                  /* TruthValue */
    oal_bool_enum_uint8 dot11_operating_mode_notification_implemented;  /* TruthValue */
} wlan_mib_dot11vhtstationconfigentry_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    osal_u32 dot11_txop_duration_rts_threshold; /* dot11TXOPDurationRTSThreshold Unsigned32 */
    osal_u32 dot11_he_tx_mcs_map; /* dot11HETxMCSMap OCTET STRING, */
    osal_u32 dot11_he_rx_mcs_map; /* dot11HERxMCSMap OCTET STRING, */

    osal_u8  dot11_trigger_mac_padding_duration;
    osal_u8  dot11_max_ampdu_length;

    osal_u8  dot11_he_option_implemented         : 1; /* dot11HEOptionImplemented TruthValue */
    osal_u8  dot11_heht_control_field_supported   : 1; /* dot11_heht_control_field_supported TruthValue */
    osal_u8  dot11_twt_option_activated          : 1; /* dot11TWTOptionActivated TruthValue */
    osal_u8  dot11_trs_option_implemented        : 1; /* dot11TRSOptionImplemented TruthValue */
    osal_u8  dot11_he_psr_option_implemented     : 1; /* dot11HEPSROptionImplemented TruthValue */
    osal_u8  dot11_he_bqr_control_implemented    : 1; /* dot11HEBQRControlImplemented TruthValue */
    osal_u8  dot11_support_response_nfrp_trigger : 1;
    osal_u8  dot11_he_ba_bitmap_support_32bit    : 1;

    osal_u8  dot11_he_mtid_aggregation_rx_support : 3;
    osal_u8  dot11_he_bsr_control_implemented     : 1; /* dot11HEBSRControlImplemented TruthValue */
    osal_u8  dot11_ht_vht_trigger_option_implemented : 1; /* dot11HTVHTTriggerOptionImplemented TruthValue */
    osal_u8  dot11_he_oper_om_implemented         : 1; /* dot11HEOperationModeControlImplemented TruthValue */
    osal_u8  resv                                 : 2; /* resv 2 bits */
} wlan_mib_dot11hestationconfigentry_stru;

typedef struct {
    oal_bool_enum_uint8   dot11_he_dual_band_support;
    oal_bool_enum_uint8   dot11_heldpc_coding_in_payload;
    oal_bool_enum_uint8   dot11_su_beamformer;
    oal_bool_enum_uint8   dot11_su_beamformee;
    oal_bool_enum_uint8   dot11_mu_beamformer;
    oal_bool_enum_uint8   dot11_hestbc_rx_below_80mhz;
    oal_bool_enum_uint8   dot11_ppe_thresholds_required;
    osal_u32 dot11_he_num_sounding_dims_below_80mhz; /* dot11HENumberSoundingDimensionsLessThanOrEqualTo80 Unsigned32 */
    osal_u8 dot11_he_ng16_mufeedback_support : 1; /* dot11HENG16MUFeedbackSupport TruthValue */
    osal_u8 dot11_he_codebooksize_mufeedback_support : 1; /* dot11HECodebookSizePhi7Psi5MUFeedbackSupport TruthValue */
    osal_u8 dot11_he_partial_bw_dlmumimo_implemented : 1; /* dot11HEPartialBWDLMUMIMOImplemented TruthValue */
    osal_u8 resv : 5; /* 1个字节还剩余 5 比特 */
    osal_u8 resv1[3]; /* 填充3字节用于对齐 */
} wlan_mid_dot11hephycapability_stru;

typedef struct {
    oal_bool_enum_uint8   dot11_ppe_thresholds_required;
    osal_u8               rsv[3];
} wlan_mid_dot11hephycapability_rom_stru;

typedef struct {
    osal_u32   dot11_he_bss_color         : 6;
    osal_u32   dot11_he_partial_bss_color : 1;
    osal_u32   dot11_he_bss_color_disable : 1;
    osal_u32   rsv                        : 24;
} wlan_mid_dot11heoper_stru;
#endif

/****************************************************************************************/
/*          Start of dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }                  */
/*              --  MAC GROUPS                                                          */
/*              --  dot11OperationTable ::= { dot11mac 1 }                              */
/*              --  dot11CountersTable ::= { dot11mac 2 }                               */
/*              --  dot11GroupAddressesTable ::= { dot11mac 3 }                         */
/*              --  dot11EDCATable ::= { dot11mac 4 }                                   */
/*              --  dot11QAPEDCATable ::= { dot11mac 5 }                                */
/*              --  dot11QosCountersTable ::= { dot11mac 6 }                            */
/*              --  dot11ResourceInfoTable    ::= { dot11mac 7 }                        */
/****************************************************************************************/
/****************************************************************************************/
/* dot11OperationTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11OperationEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group contains MAC attributes pertaining to the operation of the MAC.          */
/*      This has been implemented as a table in order to allow for multiple             */
/*      instantiations on an agent."                                                    */
/* ::= { dot11mac 1 }                                                                   */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_rifs_mode                            : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_dual_cts_protection                   : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_lsigtxop_full_protection_activated     : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_non_gf_entities_present                : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_pco_activated                        : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_40mhz_intolerant                  : 1; /* TruthValue */
    oal_bool_enum_uint8 dot112040_bss_coexistence_management_support : 1; /* TruthValue */
    oal_bool_enum_uint8 en_resv                                     : 1;

    wlan_mib_ht_protection_enum_uint8   dot11_ht_protection;            /* INTEGER */
    osal_u8   resv[2];
    osal_u32  dot11_rts_threshold;                                      /* Unsigned32 */
    osal_u32  dot11_fragmentation_threshold;                            /* Unsigned32 */
    osal_u32  dot11_bss_width_trigger_scan_interval;                       /* Unsigned32 */
    osal_u32  dot11_bss_width_channel_transition_delay_factor;              /* Unsigned32 */
    osal_u32  dot11_obss_scan_passive_dwell;                              /* Unsigned32 */
    osal_u32  dot11_obss_scan_active_dwell;                               /* Unsigned32 */
    osal_u32  dot11_obss_scan_passive_total_per_channel;                    /* Unsigned32 */
    osal_u32  dot11_obss_scan_active_total_per_channel;                     /* Unsigned32 */
    osal_u32  dot11_obss_scan_activity_threshold;                         /* Unsigned32 */
} wlan_mib_dot11operationentry_stru;

/****************************************************************************************/
/* dot11EDCATable OBJECT-TYPE                                                           */
/* SYNTAX SEQUENCE OF Dot11EDCAEntry                                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at a non-AP STA. This       */
/*      table contains the four entries of the EDCA parameters corresponding to         */
/*      four possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3      */
/*      to AC_VI, and index 4 to AC_VO."                                                */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.2.4.2"                                                   */
/* ::= { dot11mac 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    osal_u8   dot11_edca_table_index;                 /* dot11EDCATableIndex     Unsigned32 */
    osal_u8   dot11_edca_table_c_wmin;                 /* dot11EDCATableCWmin Unsigned32 */
    osal_u8   dot11_edca_table_c_wmax;                 /* dot11EDCATableCWmax Unsigned32 */
    osal_u8   dot11_edca_table_aifsn;                 /* dot11EDCATableAIFSN Unsigned32 */
    osal_u16  dot11_edca_table_txop_limit;             /* dot11EDCATableTXOPLimit Unsigned32 */
    oal_bool_enum_uint8 dot11_edca_table_mandatory;     /* dot11EDCATableMandatory TruthValue */
    osal_u8   resv;
} wlan_mib_dot11cfgedcaentry_stru;
/****************************************************************************************/
/* dot11QAPEDCATable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11QAPEDCAEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at the AP. This table       */
/*      contains the four entries of the EDCA parameters corresponding to four          */
/*      possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3 to        */
/*      AC_VI, and index 4 to AC_VO."                                                   */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.19.2"                                                    */
/* ::= { dot11mac 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    osal_u8 dot11_qapedca_table_c_wmin;               /* dot11QAPEDCATableCWmin Unsigned32 */
    osal_u8 dot11_qapedca_table_c_wmax;               /* dot11QAPEDCATableCWmax Unsigned32 */
    osal_u8 dot11_qapedca_table_aifsn;               /* dot11QAPEDCATableAIFSN Unsigned32 */
    oal_bool_enum_uint8 dot11_qapedca_table_mandatory;  /* dot11QAPEDCATableMandatory TruthValue */
    osal_u16 dot11_qapedca_table_txop_limit;           /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    osal_u16 dot11_qapedca_table_msdu_lifetime;        /* dot11_qapedca_table_msdu_lifetime Unsigned32 */
} wlan_mib_dot11qapedcaentry_stru;
typedef struct {
    wlan_mib_dot11cfgedcaentry_stru               wlan_mib_edca[WLAN_WME_AC_BUTT];
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    wlan_mib_dot11qapedcaentry_stru            wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
} wlan_mib_dot11edcaentry_stru;

/****************************************************************************************/
/* dot11SupportedMCSTxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSTxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "he Transmit MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 16 }                                                                  */
/****************************************************************************************/
typedef struct {
    osal_u8   dot11_supported_mcs_tx_value[WLAN_HT_MCS_BITMASK_LEN];  /* dot11SupportedMCSTxValue Unsigned32 */
} wlan_mib_dot11supportedmcstxentry_stru;

/****************************************************************************************/
/* dot11SupportedMCSRxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSRxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The receive MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 17 }                                                                  */
/****************************************************************************************/
typedef struct {
    osal_u8   dot11_supported_mcs_rx_value[WLAN_HT_MCS_BITMASK_LEN];   /* dot11SupportedMCSRxValue Unsigned32 */
} wlan_mib_dot11supportedmcsrxentry_stru;

typedef struct {
    wlan_mib_dot11supportedmcstxentry_stru     supported_mcstx;

    wlan_mib_dot11supportedmcsrxentry_stru     supported_mcsrx;
} wlan_mib_dot11supportedmcsentry_stru;

/****************************************************************************************/
/* dot11PhyAntennaTable OBJECT-TYPE                                                     */
/* SYNTAX SEQUENCE OF Dot11PhyAntennaEntry                                              */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for PhyAntenna. Implemented as a table indexed on          */
/*      ifIndex to allow for multiple instances on an agent."                           */
/* ::= { dot11phy 2}                                                                    */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_antenna_selection_option_implemented              : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_transmit_explicit_csi_feedback_as_option_implemented : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_transmit_indices_feedback_as_option_implemented     : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_explicit_csi_feedback_as_option_implemented         : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_receive_antenna_selection_option_implemented       : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_transmit_sounding_ppdu_option_implemented          : 1; /* TruthValue */
    oal_bool_enum_uint8 en_resv                                                : 2;

    oal_bool_enum_uint8 resv[3];
} wlan_mib_dot11phyantennaentry_stru;

/****************************************************************************************/
/* dot11PhyHRDSSSTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11PhyHRDSSSEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHRDSSSEntry. Implemented as a table            */
/*      indexed on ifIndex to allow for multiple instances on an Agent."                */
/* ::= { dot11phy 12 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_short_preamble_option_implemented; /* dot11ShortPreambleOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_pbcc_option_implemented;          /* dot11PBCCOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_channel_agility_present;          /* dot11ChannelAgilityPresent TruthValue */
} wlan_mib_dot11phyhrdsssentry_stru;

/****************************************************************************************/
/* dot11PhyERPTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyERPEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyERPEntry. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 14 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_dsssofdm_option_activated;        /* dot11DSSSOFDMOptionActivated TruthValue */
    /* dot11ShortSlotTimeOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_short_slot_time_option_implemented;
    oal_bool_enum_uint8 dot11_short_slot_time_option_activated;   /* dot11ShortSlotTimeOptionActivated TruthValue */
} wlan_mib_dot11phyerpentry_stru;

/****************************************************************************************/
/* dot11PhyDSSSTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyDSSSEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyDSSSEntry. Implemented as a table indexed      */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    wlan_mib_dot11phyhrdsssentry_stru          phy_hrdsss;
    wlan_mib_dot11phyerpentry_stru             phy_erp;
} wlan_mib_dot11phydsssentry_stru;

/****************************************************************************************/
/* dot11TransmitBeamformingConfigTable OBJECT-TYPE                                      */
/* SYNTAX SEQUENCE OF Dot11TransmitBeamformingConfigEntry                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11TransmitBeamformingConfigTable. Implemented       */
/*      as a table indexed on ifIndex to allow for multiple instances on an             */
/*      Agent."                                                                         */
/* ::= { dot11phy 18 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_receive_stagger_sounding_option_implemented                 : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_transmit_stagger_sounding_option_implemented                : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_receive_ndp_option_implemented                             : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_transmit_ndp_option_implemented                            : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_implicit_transmit_beamforming_option_implemented            : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_explicit_csi_transmit_beamforming_option_implemented         : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_explicit_non_compressed_beamforming_matrix_option_implemented : 1; /* TruthValue */
    oal_bool_enum_uint8 en_resv                                                         : 1;

    oal_bool_enum_uint8 dot11_calibration_option_implemented                            : 2;   /* INTEGER */
    oal_bool_enum_uint8 dot11_explicit_transmit_beamforming_csi_feedback_option_implemented : 2;   /* INTEGER */
    oal_bool_enum_uint8 dot11_explicit_non_compressed_beamforming_feedback_option_implemented : 2; /* INTEGER */
    oal_bool_enum_uint8 dot11_explicit_compressed_beamforming_feedback_option_implemented   : 2;  /* INTEGER */

    osal_u8          dot11_number_beam_forming_csi_support_antenna;                         /* Unsigned32 */
    osal_u8          dot11_number_non_compressed_beamforming_matrix_support_antenna;         /* Unsigned32 */
    osal_u8          dot11_number_compressed_beamforming_matrix_support_antenna;            /* Unsigned32 */
} wlan_mib_dot11transmitbeamformingconfigentry_stru;
/****************************************************************************************/
/* dot11PhyHTTable OBJECT-TYPE                                                          */
/* SYNTAX SEQUENCE OF Dot11PhyHTEntry                                                   */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHTTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 15 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 dot11_2g_40mhz_operation_implemented      : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_5g_40mhz_operation_implemented      : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_ht_greenfield_option_implemented       : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_short_gi_option_in_twenty_implemented    : 1; /* TruthValue */
    oal_bool_enum_uint8 dot112_g_short_gi_option_in_forty_implemented   : 1; /* TruthValue */
    oal_bool_enum_uint8 dot115_g_short_gi_option_in_forty_implemented   : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_ldpc_coding_option_implemented         : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_ldpc_coding_option_activated           : 1; /* TruthValue */

    oal_bool_enum_uint8 dot11_tx_stbc_option_implemented             : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_tx_stbc_option_activated               : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_rx_stbc_option_implemented             : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_tx_mcs_set_defined                     : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_tx_rx_mcs_set_not_equal                  : 1; /* TruthValue */
    oal_bool_enum_uint8 dot11_tx_unequal_modulation_supported        : 1; /* TruthValue */
    oal_bool_enum_uint8 en_resv                                     : 2;

    osal_u32            dot11_highest_supported_data_rate;                /* Unsigned32 */
    osal_u32            dot11_tx_maximum_number_spatial_streams_supported;  /* Unsigned32 */

    wlan_mib_dot11transmitbeamformingconfigentry_stru  wlan_mib_txbf_config;
} wlan_mib_dot11phyhtentry_stru;

/****************************************************************************************/
/* dot11VHTTransmitBeamformingConfigTable OBJECT-TYPE                                   */
/* SYNTAX SEQUENCE OF Dot11VHTTransmitBeamformingConfigEntry                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11VHTTransmitBeamformingConfigTable. Imple-         */
/*      mented as a table indexed on ifIndex to allow for multiple instances on an      */
/*      Agent."                                                                         */
/*  ::= { dot11phy 24 }                                                                 */
/****************************************************************************************/
typedef struct {
    /* dot11VHTSUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_vhtsu_beamformee_option_implemented;
    /* dot11VHTSUBeamformerOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_vhtsu_beamformer_option_implemented;
    /* dot11VHTMUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_vhtmu_beamformee_option_implemented;
    /* dot11VHTMUBeamformerOptionImplemented TruthValue */
    oal_bool_enum_uint8 dot11_vhtmu_beamformer_option_implemented;
    osal_u8           dot11_vht_number_sounding_dimensions;        /* dot11VHTNumberSoundingDimensions  Unsigned32 */
} wlan_mib_dot11vhttransmitbeamformingconfigentry_stru;

/****************************************************************************************/
/* dot11PhyVHTTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyVHTEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/*      DESCRIPTION                                                                     */
/*      "Entry of attributes for dot11PhyVHTTable. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 23 }                                                                  */
/****************************************************************************************/
typedef struct {
    wlan_mib_vht_supp_width_enum_uint8  dot11_vht_channel_width_option_implemented         : 2; /* INTEGER */
    oal_bool_enum_uint8                 dot11_vht_short_gi_option_in80_implemented          : 1; /* TruthValue */
    oal_bool_enum_uint8                 dot11_vht_short_gi_option_in_160_and_80p80_implemented : 1; /* TruthValue */
    oal_bool_enum_uint8                 dot11_vhtldpc_coding_option_implemented           : 1; /* TruthValue */
    oal_bool_enum_uint8                 dot11_vht_tx_stbc_option_implemented               : 1; /* TruthValue */
    oal_bool_enum_uint8                 dot11_vht_rx_stbc_option_implemented               : 1; /* TruthValue */
    osal_u8                           rev                                           : 1;

    osal_u8           resv[3];
    wlan_mib_dot11vhttransmitbeamformingconfigentry_stru wlan_mib_vht_txbf_config;
} wlan_mib_dot11phyvhtentry_stru;

typedef struct {
    /***************************************************************************
        dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }
    ****************************************************************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    wlan_mib_dot11stationconfigentry_stru wlan_mib_sta_config;

    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    wlan_mib_dot11privacyentry_stru wlan_mib_privacy;

    /* --  dot11FastBSSTransitionConfigTable ::= { dot11smt 15 } */
    wlan_mib_dot11fastbsstransitionconfigentry_stru wlan_mib_fast_bss_trans_cfg;

    /* --  dot11HTStationConfigTable  ::= { dot11smt 17 } */
    wlan_mib_dot11htstationconfigentry_stru wlan_mib_ht_sta_cfg;

    /* --  dot11WirelessMgmtOptionsTable ::= { dot11smt 18} */
    wlan_mib_dot11wirelessmgmtoptionsentry_stru wlan_mib_wireless_mgmt_op;

    /* --  dot11VHTStationConfig ::= { dot11smt 31 } */
    wlan_mib_dot11vhtstationconfigentry_stru    wlan_mib_vht_sta_config;

#ifdef _PRE_WLAN_FEATURE_11AX
    wlan_mib_dot11hestationconfigentry_stru     wlan_mib_he_sta_config;
    wlan_mid_dot11hephycapability_stru          wlan_mib_he_phy_config;
    wlan_mid_dot11heoper_stru                   wlan_mib_he_oper_config;
#endif
    /***************************************************************************
        dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }
    ****************************************************************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    wlan_mib_dot11operationentry_stru          wlan_mib_operation;
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    wlan_mib_dot11edcaentry_stru               wlan_mib_edca;
    wlan_mib_dot11supportedmcsentry_stru       supported_mcs;

    /* --  dot11PhyAntennaTable ::= { dot11phy 2 } */
    wlan_mib_dot11phyantennaentry_stru     wlan_mib_phy_antenna;

    /* --  dot11PhyDSSSTable ::= { dot11phy 5 } */
    wlan_mib_dot11phydsssentry_stru        wlan_mib_phy_dsss;

    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    wlan_mib_dot11phyhtentry_stru   wlan_mib_phy_ht;

    /* -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac) */
    wlan_mib_dot11phyvhtentry_stru  wlan_mib_phy_vht;
} wlan_mib_ieee802dot11_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
