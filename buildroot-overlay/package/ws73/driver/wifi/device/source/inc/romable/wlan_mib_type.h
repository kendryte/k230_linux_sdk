/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file of device and host wlan_mib public interface.
 * Create: 2021-09-16
 */

#ifndef __WLAN_MIB_TYPE_H__
#define __WLAN_MIB_TYPE_H__

#include "osal_types.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_MIB_TOKEN_STRING_MAX_LENGTH    32      /* 支持与网管兼容的token的字符串最长长度，此定义后续可能与版本相关 */
#define WLAN_HT_MCS_BITMASK_LEN             10      /* MCS bitmask长度为77位，加上3个保留位 */

/* Number of Cipher Suites Implemented */
#define WLAN_PAIRWISE_CIPHER_SUITES         2
#define WLAN_AUTHENTICATION_SUITES          2

#define MAC_PAIRWISE_CIPHER_SUITES_NUM      2   /* 表征WPA2有CCMP和TKIP两种加密套件，WPA有WEP和WEP104两种加密套件 */
#define MAC_AUTHENTICATION_SUITE_NUM        2   /* 表示RSN 认证方法数 */

#define WAL_HIPRIV_RSSI_DEFAULT_THRESHOLD   (-95)     /* 默认RSSI门限 -95dbm */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* RowStatus ::= TEXTUAL-CONVENTION                                         */
/* The status column has six  values:`active', `notInService',`notReady',   */
/* `createAndGo', `createAndWait', `destroy'  as described in   rfc2579     */
typedef enum {
    WLAN_MIB_ROW_STATUS_ACTIVE     = 1,
    WLAN_MIB_ROW_STATUS_NOT_INSERVICE,
    WLAN_MIB_ROW_STATUS_NOT_READY,
    WLAN_MIB_ROW_STATUS_CREATE_AND_GO,
    WLAN_MIB_ROW_STATUS_CREATE_AND_WAIT,
    WLAN_MIB_ROW_STATUS_DEATROY,

    WLAN_MIB_ROW_STATUS_BUTT
} wlan_mib_row_status_enum;
typedef osal_u8 wlan_mib_row_status_enum_uint8;

/* dot11PowerManagementMode INTEGER, { active(1), powersave(2) } */
typedef enum {
    WLAN_MIB_PWR_MGMT_MODE_ACTIVE     = 1,
    WLAN_MIB_PWR_MGMT_MODE_PWRSAVE    = 2,

    WLAN_MIB_PWR_MGMT_MODE_BUTT
} wlan_mib_pwr_mgmt_mode_enum;
typedef osal_u8 wlan_mib_pwr_mgmt_mode_enum_uint8;

/* dot11AuthenticationAlgorithm INTEGER, {openSystem(1),sharedKey(2), */
/* fastBSSTransition(3),simultaneousAuthEquals(4) }                  */
typedef enum {
    WLAN_MIB_AUTH_ALG_OPEN_SYS          = 1,
    WLAN_MIB_AUTH_ALG_SHARE_KEY         = 2,
    WLAN_MIB_AUTH_ALG_FAST_BSS_TRANS    = 3,
    WLAN_MIB_AUTH_ALG_SIM_AUTH_EQUAL    = 4,

    WLAN_MIB_AUTH_ALG_BUTT
} wlan_mib_auth_alg_enum;
typedef osal_u8 wlan_mib_auth_alg_enum_uint8;

/* dot11DesiredBSSType INTEGER, {infrastructure(1), independent(2), any(3) } */
typedef enum {
    WLAN_MIB_DESIRED_BSSTYPE_INFRA       = 1,
    WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT = 2,
    WLAN_MIB_DESIRED_BSSTYPE_ANY         = 3,

    WLAN_MIB_DESIRED_BSSTYPE_BUTT
} wlan_mib_desired_bsstype_enum;
typedef osal_u8 wlan_mib_desired_bsstype_enum_uint8;

/* dot11RSNAConfigGroupRekeyMethod  INTEGER { disabled(1), timeBased(2),  */
/* packetBased(3), timepacketBased(4)  */
typedef enum {
    WLAN_MIB_RSNACFG_GRPREKEY_DISABLED = 1,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEBASED  = 2,
    WLAN_MIB_RSNACFG_GRPREKEY_PACKETBASED = 3,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEPACKETBASED = 4,

    WLAN_MIB_RSNACFG_GRPREKEY_BUTT
} wlan_mib_rsna_cfg_grp_rekey_enum;
typedef osal_u8 wlan_mib_rsna_cfg_grp_rekey_enum_uint8;

/* dot11RMRqstType INTEGER {channelLoad(3),noiseHistogram(4),beacon(5), */
/* frame(6),staStatistics(7),lci(8),transmitStream(9),pause(255) }      */
typedef enum {
    WLAN_MIB_RMRQST_TYPE_CH_LOAD         = 3,
    WLAN_MIB_RMRQST_TYPE_NOISE_HISTOGRAM = 4,
    WLAN_MIB_RMRQST_TYPE_BEACON          = 5,
    WLAN_MIB_RMRQST_TYPE_FRAME           = 6,
    WLAN_MIB_RMRQST_TYPE_STA_STATISTICS  = 7,
    WLAN_MIB_RMRQST_TYPE_LCI             = 8,
    WLAN_MIB_RMRQST_TYPE_TRANS_STREAM    = 9,
    WLAN_MIB_RMRQST_TYPE_PAUSE           = 255,

    WLAN_MIB_RMRQST_TYPE_BUTT
} wlan_mib_rmrqst_type_enum;
typedef osal_u16 wlan_mib_rmrqst_type_enum_uint16;

/* dot11RMRqstBeaconRqstMode INTEGER{  passive(0), active(1),beaconTable(2) */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_MODE_PASSIVE      = 0,
    WLAN_MIB_RMRQST_BEACONRQST_MODE_ACTIVE       = 1,
    WLAN_MIB_RMRQST_BEACON_MODE_BEACON_TABLE = 2,

    WLAN_MIB_RMRQST_BEACONRQST_MODE_BUTT
} wlan_mib_rmrqst_beaconrqst_mode_enum;
typedef osal_u8 wlan_mib_rmrqst_beaconrqst_mode_enum_uint8;

typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_NOBODY           = 0,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_FIXED_FLDANDELMT = 1,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_ALLBODY          = 2,

    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_BUTT
} wlan_mib_rmrqst_beaconrqst_detail_enum;
typedef osal_u8 wlan_mib_rmrqst_beaconrqst_detail_enum_uint8;

typedef enum {
    WLAN_MIB_RMRQST_FRAMERQST_TYPE_FRAME_COUNTREP      = 1,

    WLAN_MIB_RMRQST_FRAMERQST_TYPE_BUTT
} wlan_mib_rmrqst_famerqst_type_enum;
typedef osal_u8 wlan_mib_rmrqst_famerqst_type_enum_uint8;

/* dot11RMRqstBeaconReportingCondition INTEGER {afterEveryMeasurement(0), */
/* rcpiAboveAbsoluteThreshold(1),rcpiBelowAbsoluteThreshold(2),           */
/* rsniAboveAbsoluteThreshold(3),rsniBelowAbsoluteThreshold(4),           */
/* rcpiAboveOffsetThreshold(5),rcpiBelowOffsetThreshold(6),               */
/* rsniAboveOffsetThreshold(7),rsniBelowOffsetThreshold(8),               */
/* rcpiInBound(9),rsniInBound(10) }                                       */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRPT_CDT_AFTER_EVERY_MEAS            = 0,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_ABS_THRESHOLD    = 1,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_ABS_THRESHOLD    = 2,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_ABS_THRESHOLD    = 3,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_ABS_THRESHOLD    = 4,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_OFFSET_THRESHOLD = 5,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_OFFSET_THRESHOLD = 6,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_OFFSET_THRESHOLD = 7,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_OFFSET_THRESHOLD = 8,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_IN_BOUND               = 9,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_IN_BOUND               = 10,

    WLAN_MIB_RMRQST_BEACONRPT_CDT_BUTT
} wlan_mib_rmrqst_beaconrpt_cdt_enum;
typedef osal_u8 wlan_mib_rmrqst_beaconrpt_cdt_enum_uint8;

/* dot11RMRqstSTAStatRqstGroupID OBJECT-TYPE                             */
/* SYNTAX INTEGER {                                                      */
/* dot11CountersTable(0),dot11MacStatistics(1),                          */
/* dot11QosCountersTableforUP0(2),dot11QosCountersTableforUP1(3),        */
/* dot11QosCountersTableforUP2(4),dot11QosCountersTableforUP3(5),        */
/* dot11QosCountersTableforUP4(6),dot11QosCountersTableforUP5(7),        */
/* dot11QosCountersTableforUP6(8),dot11QosCountersTableforUP7(9),        */
/* bSSAverageAccessDelays(10),dot11CountersGroup3Tablefor31(11),         */
/* dot11CountersGroup3Tablefor32(12),dot11CountersGroup3Tablefor33(13),  */
/* dot11CountersGroup3Tablefor34(14),dot11CountersGroup3Tablefor35(15),  */
typedef enum {
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_TABLE          = 0,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_MAC_STATS              = 1,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP0  = 2,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP1  = 3,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP2  = 4,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP3  = 5,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP4  = 6,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP5  = 7,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP6  = 8,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP7  = 9,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BSS_AVERG_ACCESS_DELAY = 10,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR31     = 11,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR32     = 12,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR33     = 13,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR34     = 14,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR35     = 15,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_RSNA_STATS_TABLE       = 16,

    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BUTT
} wlan_mib_rmrqst_stastatrqst_grpid_enum;
typedef osal_u8 wlan_mib_rmrqst_stastatrqst_grpid_enum_uint8;

/* dot11RMRqstLCIRqstSubject OBJECT-TYPE  */
typedef enum {
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_LOCAL  = 0,
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_REMOTE = 1,

    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_BUTT
} wlan_mib_rmrqst_lcirpt_subject_enum;
typedef osal_u8 wlan_mib_rmrqst_lcirpt_subject_enum_uint8;

/* dot11RMRqstLCIAzimuthType OBJECT-TYPE                  */
typedef enum {
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_FRONT_SURFACE_STA  = 0,
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_RADIO_BEAM         = 1,

    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_BUTT
} wlan_mib_rmrqst_lciazimuth_type_enum;
typedef osal_u8 wlan_mib_rmrqst_lciazimuth_type_enum_uint8;

/* dot11RMRqstChannelLoadReportingCondition OBJECT-TYPE   */
/* SYNTAX INTEGER {                                       */
/* afterEveryMeasurement(0),                             */
/* chanLoadAboveReference(1),                            */
/* chanLoadBelowReference(2) }                           */
typedef enum {
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_AFTER_EVERY_MEAS  = 0,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_BELOW_REF = 2,

    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_BUTT
} wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum;
typedef osal_u8 wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum_uint8;

/* dot11RMRqstNoiseHistogramReportingCondition OBJECT-TYPE  */
/* SYNTAX INTEGER {                                         */
/* afterEveryMeasurement(0),                                */
/* aNPIAboveReference(1),                                   */
/* aNPIBelowReference(2) }                                  */
typedef enum {
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_ABOVE_REF   = 1,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_BELOW_REF   = 2,

    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_BUTT
} wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum;
typedef osal_u8 wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum_uint8;

/* dot11LCIDSEAltitudeType OBJECT-TYPE              */
typedef enum {
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_METERS = 1,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_FLOORS = 2,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_HAGM   = 3,

    WLAN_MIB_LCI_DSEALTITUDE_TYPE_BUTT
} wlan_mib_lci_dsealtitude_type_enum;
typedef osal_u8 wlan_mib_lci_dsealtitude_type_enum_uint8;

/* dot11MIMOPowerSave OBJECT-TYPE                    */
typedef enum {
    WLAN_MIB_MIMO_POWER_SAVE_STATIC  = 1,
    WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC = 2,
    WLAN_MIB_MIMO_POWER_SAVE_MIMO    = 3,

    WLAN_MIB_MIMO_POWER_SAVE_BUTT
} wlan_mib_mimo_power_save_enum;
typedef osal_u8 wlan_mib_mimo_power_save_enum_uint8;

/* dot11MaxAMSDULength OBJECT-TYPE            */
typedef enum {
    WLAN_MIB_MAX_AMSDU_LENGTH_SHORT = 3839,
    WLAN_MIB_MAX_AMSDU_LENGTH_LONG  = 7935,

    WLAN_MIB_MAX_AMSDU_LENGTH_BUTT
} wlan_mib_max_amsdu_lenth_enum;
typedef osal_u16 wlan_mib_max_amsdu_lenth_enum_uint16;

/* dot11MCSFeedbackOptionImplemented OBJECT-TYPE         */
/* 对应枚举含义SYNTAX INTEGER { none(0), unsolicited (2), both (3) } */
typedef enum {
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_NONE = 0,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_UNSOLICITED = 2,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BOTH        = 3,

    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BUTT
} wlan_mib_mcs_feedback_opt_implt_enum;
typedef osal_u8 wlan_mib_mcs_feedback_opt_implt_enum_uint8;

/* dot11LocationServicesLIPReportIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                                        */
/* hours(0),                                               */
/* minutes(1),                                             */
/* seconds(2),                                             */
/* milliseconds(3)                                         */
typedef enum {
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_HOURS      = 0,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MINUTES    = 1,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_SECONDS    = 2,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MILLISECDS = 3,

    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_BUTT
} wlan_mib_lct_servs_liprpt_interval_unit_enum;
typedef osal_u8 wlan_mib_lct_servs_liprpt_interval_unit_enum_uint8;

/* dot11WirelessMGTEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_TRANSITION      = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_RSNA            = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_PEERTOPEER      = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_WNMLOG          = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_BUTT
} wlan_mib_wireless_mgt_event_type_enum;
typedef osal_u8 wlan_mib_wireless_mgt_event_type_enum_uint8;

/* dot11WirelessMGTEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* successful(0),                          */
/* requestFailed(1),                       */
/* requestRefused(2),                      */
/* requestIncapable(3),                    */
/* detectedFrequentTransition(4)           */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_SUCC                = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_FAIL           = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_REFUSE         = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_INCAP          = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_BUTT
} wlan_mib_wireless_mgt_event_status_enum;
typedef osal_u8 wlan_mib_wireless_mgt_event_status_enum_uint8;

/* dot11WirelessMGTEventTransitionReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* certificateToken(8),                                */
/* apFailedIeee8021XEapAuthentication(9),              */
/* apFailed4wayHandshake(10),                          */
/* excessiveDataMICFailures(11),                       */
/* exceededFrameTransmissionRetryLimit(12),            */
/* ecessiveBroadcastDisassociations(13),               */
/* excessiveBroadcastDeauthentications(14),            */
/* previousTransitionFailed(15)                        */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_UNSPEC                           = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT     = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS   = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_FIRST_ASSO_ESS                   = 4,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_LOAD_BALANCE                     = 5,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BETTER_AP_FOUND                  = 6,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP       = 7,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_CERTIF_TOKEN                     = 8,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH       = 9,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE           = 10,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DATA_MIC_FAIL              = 11,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCEED_FRAME_TRANS_RETRY_LIMIT   = 12,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISASSO              = 13,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISAUTH              = 14,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_PRE_TRANSIT_FAIL                 = 15,

    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BUTT
} wlan_mib_wireless_mgt_event_transit_reason_enum;
typedef osal_u8 wlan_mib_wireless_mgt_event_transit_reason_enum_uint8;

/* dot11WNMRqstType OBJECT-TYPE */
/* SYNTAX INTEGER {             */
/* mcastDiagnostics(0),         */
/* locationCivic(1),            */
/* locationIdentifier(2),       */
/* event(3),                    */
/* dignostic(4),                */
/* locationConfiguration(5),    */
/* bssTransitionQuery(6),       */
/* bssTransitionRqst(7),        */
/* fms(8),                      */
/* colocInterference(9)         */
typedef enum {
    WLAN_MIB_WNM_RQST_TYPE_MCAST_DIAG        = 0,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CIVIC    = 1,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_IDTIF    = 2,
    WLAN_MIB_WNM_RQST_TYPE_EVENT             = 3,
    WLAN_MIB_WNM_RQST_TYPE_DIAGOSTIC         = 4,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CFG      = 5,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_QUERY = 6,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_RQST  = 7,
    WLAN_MIB_WNM_RQST_TYPE_FMS               = 8,
    WLAN_MIB_WNM_RQST_TYPE_COLOC_INTERF      = 9,

    WLAN_MIB_WNM_RQST_TYPE_BUTT
} wlan_mib_wnm_rqst_type_enum;
typedef osal_u8 wlan_mib_wnm_rqst_type_enum_uint8;

/* dot11WNMRqstLCRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_LOCAL  = 0,
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lcrrqst_subj_enum;
typedef osal_u8 wlan_mib_wnm_rqst_lcrrqst_subj_enum_uint8;

/* dot11WNMRqstLCRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_SECOND  = 0,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_MINUTE  = 1,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_HOUR    = 2,

    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lcr_interval_unit_enum;
typedef osal_u8 wlan_mib_wnm_rqst_lcr_interval_unit_enum_uint8;

/* dot11WNMRqstLIRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_LOCAL   = 0,
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_REMOTE  = 1,

    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lirrqst_subj_enum;
typedef osal_u8 wlan_mib_wnm_rqst_lirrqst_subj_enum_uint8;

/* dot11WNMRqstLIRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_SECOND  = 0,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_MINUTE  = 1,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_HOUR    = 2,

    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lir_interval_unit_enum;
typedef osal_u8 wlan_mib_wnm_rqst_lir_interval_unit_enum_uint8;

/* dot11WNMRqstEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WNM_RQST_EVENT_TYPE_TRANSITION      = 0,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_RSNA            = 1,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_PEERTOPEER      = 2,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_WNMLOG          = 3,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_EVENT_TYPE_BUTT
} wlan_mib_wnm_rqst_event_tpye_enum;
typedef osal_u8 wlan_mib_wnm_rqst_event_type_enum_uint8;

/* dot11WNMRqstDiagType OBJECT-TYPE */
/* SYNTAX INTEGER {                 */
/* cancelRequest(0),                */
/* manufacturerInfoStaRep(1),       */
/* configurationProfile(2),         */
/* associationDiag(3),              */
/* ieee8021xAuthDiag(4),            */
/* vendorSpecific(221)              */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CANCEL_RQST          = 0,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_MANUFCT_INFO_STA_REP = 1,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CFG_PROFILE          = 2,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_ASSO_DIAG            = 3,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_IEEE8021X_AUTH_DIAG  = 4,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_VENDOR_SPECIFIC      = 221,

    WLAN_MIB_WNM_RQST_DIAG_TYPE_BUTT
} wlan_mib_wnm_rqst_diag_type_enum;
typedef osal_u8 wlan_mib_wnm_rqst_diag_type_enum_uint8;

/* dot11WNMRqstDiagCredentials OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* none(0),                                */
/* preSharedKey(1),                        */
/* usernamePassword(2),                    */
/* x509Certificate(3),                     */
/* otherCertificate(4),                    */
/* oneTimePassword(5),                     */
/* token(6)                                */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_NONT               = 0,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_PRE_SHAREKEY       = 1,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_USERNAME_PASSWORD  = 2,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_X509_CTERTIFICATE  = 3,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_OTHER_CTERTIFICATE = 4,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_ONE_TIME_PASSWORD  = 5,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_TOKEN              = 6,

    WLAN_MIB_WNM_RQST_DIAG_CREDENT_BUTT
} wlan_mib_wnm_rqst_diag_credent_enum;
typedef osal_u8 wlan_mib_wnm_rqst_diag_credent_enum_uint8;

/* dot11WNMRqstBssTransitQueryReason OBJECT-TYPE       */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_UNSPEC                            = 0,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_FRAME_LOSSRATE_POORCDT      = 1,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS    = 2,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS  = 3,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_FIRST_ASSO_ESS                    = 4,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOAD_BALANCE                      = 5,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BETTER_AP_FOUND                   = 6,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_DEAUTH_DISASSO_FROM_PRE_AP        = 7,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_IEEE8021X_EAP_AUTH        = 8,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_4WAY_HANDSHAKE            = 9,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL     = 11,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCEED_MAXNUM_RETANS              = 12,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO   = 13,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH    = 14,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_PRE_TRANSIT_FAIL                  = 15,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOW_RSSI                          = 16,

    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BUTT
} wlan_mib_wnm_rqst_bss_transit_query_reason_enum;
typedef osal_u8 wlan_mib_wnm_rqst_bss_transit_query_reason_enum_uint8;

/* dot11WNMEventTransitRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                                */
/* successful(0),                                  */
/* requestFailed(1),                               */
/* requestRefused(2),                              */
/* requestIncapable(3),                            */
/* detectedFrequentTransition(4)                   */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_SUCC                = 0,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_FAIL           = 1,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_REFUSE         = 2,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_INCAP          = 3,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_transit_rprt_event_status_enum;
typedef osal_u8 wlan_mib_wnm_event_transit_rprt_event_status_enum_uint8;

/* dot11WNMEventTransitRprtTransitReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_UNSPEC                            = 0,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT      = 1,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS    = 2,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS  = 3,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_FIRST_ASSO_ESS                    = 4,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOAD_BALANCE                      = 5,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BETTER_AP_FOUND                   = 6,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP        = 7,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH        = 8,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE            = 9,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL     = 11,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCEED_MAXNUM_RETANS              = 12,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO   = 13,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH    = 14,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_PRE_TRANSIT_FAIL                  = 15,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOW_RSSI                          = 16,

    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BUTT
} wlan_mib_wnm_event_transitrprt_transit_reason_enum;
typedef osal_u8 wlan_mib_wnm_event_transitrprt_transit_reason_enum_uint8;

/* dot11WNMEventRsnaRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                             */
/* successful(0),                               */
/* requestFailed(1),                            */
/* requestRefused(2),                           */
/* requestIncapable(3),                         */
/* detectedFrequentTransition(4)                */
typedef enum {
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_SUCC                = 0,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_FAIL           = 1,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_REFUSE         = 2,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_INCAP          = 3,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_rsnarprt_event_status_enum;
typedef osal_u8 wlan_mib_wnm_event_rsnarprt_event_status_enum_uint8;

/* dot11APLCIDatum OBJECT-TYPE */
/* SYNTAX INTEGER {            */
/* wgs84 (1),                  */
/* nad83navd88 (2),            */
/* nad93mllwvd (3)             */
typedef enum {
    WLAN_MIB_AP_LCI_DATUM_WGS84        = 1,
    WLAN_MIB_AP_LCI_DATUM_NAD83_NAVD88 = 2,
    WLAN_MIB_AP_LCI_DATUM_NAD93_MLLWVD = 3,

    WLAN_MIB_AP_LCI_DATUM_BUTT
} wlan_mib_ap_lci_datum_enum;
typedef osal_u8 wlan_mib_ap_lci_datum_enum_uint8;


/* dot11APLCIAzimuthType OBJECT-TYPE */
/* SYNTAX INTEGER {                  */
/* frontSurfaceOfSTA(0),             */
/* radioBeam(1) }                    */
typedef enum {
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_RADIO_BEAM        = 1,

    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_BUTT
} wlan_mib_ap_lci_azimuth_type_enum;
typedef osal_u8 wlan_mib_ap_lci_azimuth_type_enum_uint8;

/* dot11HTProtection 枚举定义 */
typedef enum {
    WLAN_MIB_HT_NO_PROTECTION           = 0,
    WLAN_MIB_HT_NONMEMBER_PROTECTION    = 1,
    WLAN_MIB_HT_20MHZ_PROTECTION        = 2,
    WLAN_MIB_HT_NON_HT_MIXED            = 3,

    WLAN_MIB_HT_PROTECTION_BUTT
} wlan_mib_ht_protection_enum;
typedef osal_u8 wlan_mib_ht_protection_enum_uint8;

/* dot11ExplicitCompressedBeamformingFeedbackOptionImplemented OBJECT-TYPE */
/* SYNTAX INTEGER {                                                        */
/* inCapable (0),                                                          */
/* delayed (1),                                                            */
/* immediate (2),                                                          */
/* unsolicitedImmediate (3),                                               */
/* aggregated (4),                                                         */
/* delayedAggregated (5),                                                  */
/* immediateAggregated (6),                                                */
/* unsolicitedImmediateAggregated (7)}                                     */
typedef enum {
    WLAN_MIB_HT_ECBF_INCAPABLE    = 0,
    WLAN_MIB_HT_ECBF_DELAYED      = 1,
    WLAN_MIB_HT_ECBF_IMMEDIATE    = 2,

    /* Tip:if you select it(3) and use for HT cap, it stands for both 1 and 2. page656 */
    WLAN_MIB_HT_ECBF_UNSOLIMMEDI  = 3,
    WLAN_MIB_HT_ECBF_AGGREGATE    = 4,
    WLAN_MIB_HT_ECBF_DELAYAGGR    = 5,
    WLAN_MIB_HT_ECBF_IMMEDIAGGR   = 6,
    WLAN_MIB_HT_ECBF_UNSOLIMMAGGR = 7,

    WLAN_MIB_HT_ECBF_TYPE_BUTT
} wlan_mib_ht_ecbf_enum;
typedef osal_u8   wlan_mib_ht_ecbf_enum_uint8;


/* VHT Capabilities Info field 最大MPDU长度枚举 */
typedef enum {
    WLAN_MIB_VHT_MPDU_3895  = 0,
    WLAN_MIB_VHT_MPDU_7991  = 1,
    WLAN_MIB_VHT_MPDU_11454 = 2,

    WLAN_MIB_VHT_MPDU_LEN_BUTT
} wlan_mib_vht_mpdu_len_enum;
typedef osal_u8   wlan_mib_vht_mpdu_len_enum_uint8;

/* VHT Capabilites Info field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_SUPP_WIDTH_80          = 0,    /* 不支持160或者80+80 */
    WLAN_MIB_VHT_SUPP_WIDTH_160         = 1,    /* 支持160 */
    WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80    = 2,    /* 支持160和80+80 */

    WLAN_MIB_VHT_SUPP_WIDTH_BUTT
} wlan_mib_vht_supp_width_enum;
typedef osal_u8   wlan_mib_vht_supp_width_enum_uint8;

/* VHT Operation Info  field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_OP_WIDTH_20_40         = 0,    /* 工作在20/40 */
    WLAN_MIB_VHT_OP_WIDTH_80            = 1,    /* 工作在80 */
    WLAN_MIB_VHT_OP_WIDTH_160           = 2,    /* 工作在160 */
    WLAN_MIB_VHT_OP_WIDTH_80PLUS80      = 3,    /* 工作在80+80 */
    WLAN_MIB_VHT_OP_WIDTH_BUTT
} wlan_mib_vht_op_width_enum;
typedef osal_u8   wlan_mib_vht_op_width_enum_uint8;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
/* 当前MIB表项还未稳定，暂不需要4字节对齐，待后续MIB项稳定后再来调整这些结构体 */
/* 是否需要该结构体表示变长的字符串，待确定 */
typedef struct {
    osal_u16 octet_nums;    /* 字节长度，此值为0时表示没有字符串信息 */
    osal_u8 *octec_string; /* 字符串起始地址 */
} wlan_mib_octet_string_stru;

typedef struct {
    osal_u8 mcs_nums;
    osal_u8 *mcs_set;
} wlan_mib_ht_op_mcs_set_stru; /* dot11HTOperationalMCSSet */

/* dot11LocationServicesLocationIndicationIndicationParameters OBJECT-TYPE */
/* SYNTAX OCTET STRING (SIZE (1..255))                                     */
/* MAX-ACCESS read-create                                                  */
/* STATUS current                                                          */
/* DESCRIPTION                                                             */
/* "This attribute indicates the location Indication Parameters used for   */
/* transmitting Location Track Notification Frames."                       */
/* ::= { dot11LocationServicesEntry 15}                                    */
typedef struct {
    osal_u8  para_nums;
    osal_u8 *para;
} wlan_mib_local_serv_location_ind_ind_para_stru;

#endif // __WLAN_MIB_TYPE_H__

