/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Source file defined by the corresponding frame structure and operation interface (cannot be called by
 * the HAL module).
 * Create: 2020-7-9
 */

#ifndef __MAC_FRAME_COMMON_ROM_H__
#define __MAC_FRAME_COMMON_ROM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "wlan_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_80211_FRAME_LEN 24      /* 非四地址情况下，MAC帧头的长度 */

/* A-MSDU情况下，submsdu的偏移宏 */
#define MAC_SUBMSDU_HEADER_LEN 14    /* |da = 6|sa = 6|len = 2| submsdu的头的长度 */

/* 信息元素长度定义 */
#define MAC_IE_HDR_LEN 2              /* 信息元素头部 1字节EID + 1字节长度 */
#define MAC_TIME_STAMP_LEN 8
#define MAC_BEACON_INTERVAL_LEN 2
#define MAC_CAP_INFO_LEN 2
#define MAC_MIN_TIM_LEN 4
#define MAC_AC_PARAM_LEN 4

#define MAC_QOS_CTRL_FIELD_OFFSET 24
#define MAC_QOS_CTRL_FIELD_OFFSET_4ADDR 30
#define MAC_80211_QOS_FRAME_LEN 26

#define MAC_SLP_RM_IE      0x20

#define MAC_BEACON_OFFSET   (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN) /* beacon固定长度 */
#define MAC_LISTEN_INTERVAL_MAX_LEN         10  /* 定义关联STA最大LISTEN INTERVAL的值 */

/* TCP协议类型，chartiot tcp连接端口号 */
#define MAC_TCP_PROTOCAL 6
#define MAC_UDP_PROTOCAL 17

/* DHCP message types */
#define MAC_DHCP_DISCOVER 1
#define MAC_DHCP_OFFER 2
#define MAC_DHCP_REQUEST 3
#define MAC_DHCP_ACK 5

/* eapol key 结构宏定义 */
#define WPA_REPLAY_COUNTER_LEN 8
#define WPA_NONCE_LEN 32
#define WPA_KEY_RSC_LEN 8
#define IEEE802_1X_TYPE_EAPOL_KEY 3
#define WPA_KEY_INFO_KEY_TYPE (1U << (3)) /* 1 = Pairwise, 0 = Group key */
#define MAC_MAX_START_SPACING 7
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* Action Frames: Category字段枚举 */
typedef enum {
    MAC_ACTION_CATEGORY_SPECMGMT = 0,
    MAC_ACTION_CATEGORY_QOS = 1,
    MAC_ACTION_CATEGORY_DLS = 2,
    MAC_ACTION_CATEGORY_BA = 3,
    MAC_ACTION_CATEGORY_PUBLIC = 4,
    MAC_ACTION_CATEGORY_RADIO_MEASURMENT = 5,
    MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION = 6,
    MAC_ACTION_CATEGORY_HT = 7,
    MAC_ACTION_CATEGORY_SA_QUERY = 8,
    MAC_ACTION_CATEGORY_PROTECTED_DUAL_OF_ACTION = 9,
    MAC_ACTION_CATEGORY_WNM = 10,
    MAC_ACTION_CATEGORY_MESH = 13,
    MAC_ACTION_CATEGORY_MULTIHOP = 14,
    MAC_ACTION_CATEGORY_VHT = 21,
    MAC_ACTION_CATEGORY_S1G = 22,
    MAC_ACTION_CATEGORY_HE = 30,
    MAC_ACTION_CATEGORY_PROTECTED_HE = 31,
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    MAC_ACTION_CATEGORY_WUR = 32,
#endif
    MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED = 126,
    MAC_ACTION_CATEGORY_VENDOR = 127,
} mac_action_category_enum;
typedef osal_u8 mac_category_enum_uint8;

/* ACTION帧中，各个域的偏移量 */
typedef enum {
    MAC_ACTION_OFFSET_CATEGORY = 0,
    MAC_ACTION_OFFSET_ACTION = 1,
    MAC_ACTION_OFFSET_BODY = 2
} mac_action_offset_enum;
typedef osal_u8 mac_action_offset_enum_uint8;

/* Public Category下的Action枚举值 */
typedef enum {
    MAC_PUB_COEXT_MGMT = 0,            /* 20/40 BSS Coexistence Management */
    MAC_PUB_EX_CH_SWITCH_ANNOUNCE = 4, /* Extended Channel Switch Announcement */
    MAC_PUB_VENDOR_SPECIFIC = 9,
    MAC_PUB_GAS_INIT_RESP = 11,   /* public Action: GAS Initial Response(0x0b) */
    MAC_PUB_GAS_COMBAK_RESP = 13, /* public Action: GAS Comeback Response(0x0d) */
    MAC_PUB_FTM_REQ = 32,
    MAC_PUB_FTM = 33
} mac_public_action_type_enum;
typedef osal_u8 mac_public_action_type_enum_uint8;

/* S1G下的Action值的枚举 */
typedef enum {
    MAC_S1G_ACTION_AID_SWITCH_REQ = 0,
    MAC_S1G_ACTION_AID_SWITCH_RESP = 1,
    MAC_S1G_ACTION_SYNC_CONTROL = 2,
    MAC_S1G_ACTION_STA_INFORMATION_ANNOUNCEMENT = 3,
    MAC_S1G_ACTION_EDCA_PARAMETER_SET = 4,
    MAC_S1G_ACTION_EL_OPERATION = 5,
    MAC_S1G_ACTION_TWT_SETUP = 6,
    MAC_S1G_ACTION_TWT_TEARDOWN = 7,
    MAC_S1G_ACTION_SECTORIZED_GROUP_ID_LIST = 8,
    MAC_S1G_ACTION_SECTOR_ID_FEEDBACK = 9,
    MAC_S1G_ACTION_RESERVE = 10,
    MAC_S1G_ACTION_TWT_INFORMATION = 11,
    MAC_S1G_ACTION_BUTT
} mac_s1g_action_type_enum;
typedef osal_u8 mac_s1g_action_type_enum_uint8;

/* 新增枚举类型, 在更新TWT传事件时，用于区分来源 */
typedef enum {
    MAC_TWT_UPDATE_SOURCE_ASSOCIATION = 0,
    MAC_TWT_UPDATE_SOURCE_SETUP = 1,
    MAC_TWT_UPDATE_SOURCE_TEARDOWN = 2,
    MAC_TWT_UPDATE_SOURCE_INFORMATION = 3,
    MAC_TWT_UPDATE_SOURCE_MAX
} mac_twt_update_source_enum;
typedef osal_u8 mac_twt_update_source_enum_uint8;

/* Status Codes for Authentication and Association Frames */
typedef enum {
    MAC_SUCCESSFUL_STATUSCODE = 0,
    MAC_UNSPEC_FAIL = 1,
    MAC_UNSUP_CAP = 10,
    MAC_REASOC_NO_ASOC = 11,
    MAC_FAIL_OTHER = 12,
    MAC_UNSUPT_ALG = 13,
    MAC_AUTH_SEQ_FAIL = 14,
    MAC_CHLNG_FAIL = 15,
    MAC_AUTH_TIMEOUT = 16,
    MAC_AP_FULL = 17,
    MAC_UNSUP_RATE = 18,
    MAC_SHORT_PREAMBLE_UNSUP = 19,
    MAC_PBCC_UNSUP = 20,
    MAC_CHANNEL_AGIL_UNSUP = 21,
    MAC_MISMATCH_SPEC_MGMT = 22,
    MAC_MISMATCH_POW_CAP = 23,
    MAC_MISMATCH_SUPP_CHNL = 24,
    MAC_SHORT_SLOT_UNSUP = 25,
    MAC_OFDM_DSSS_UNSUP = 26,
    MAC_MISMATCH_HTCAP = 27,
    MAC_MISMATCH_PCO = 29,
    MAC_REJECT_TEMP = 30,
    MAC_MFP_VIOLATION = 31,
    MAC_UNSPEC_QOS_FAIL = 32,
    MAC_QAP_INSUFF_BANDWIDTH_FAIL = 33,
    MAC_POOR_CHANNEL_FAIL = 34,
    MAC_REMOTE_STA_NOT_QOS = 35,
    MAC_REQ_DECLINED = 37,
    MAC_INVALID_REQ_PARAMS = 38,
    MAC_RETRY_NEW_TSPEC = 39,
    MAC_INVALID_INFO_ELMNT = 40,
    MAC_INVALID_GRP_CIPHER = 41,
    MAC_INVALID_PW_CIPHER = 42,
    MAC_INVALID_AKMP_CIPHER = 43,
    MAC_UNSUP_RSN_INFO_VER = 44,
    MAC_INVALID_RSN_INFO_CAP = 45,
    MAC_CIPHER_REJ = 46,
    MAC_RETRY_TS_LATER = 47,
    MAC_DLS_NOT_SUPP = 48,
    MAC_DST_STA_NOT_IN_QBSS = 49,
    MAC_DST_STA_NOT_QSTA = 50,
    MAC_LARGE_LISTEN_INT = 51,
    MAC_STATUS_INVALID_PMKID = 53,
    MAC_ANTI_CLOGGING = 76,
    MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED = 77,
    MAC_MISMATCH_VHTCAP = 104,

    /* 私有的定义 */
    MAC_JOIN_RSP_TIMEOUT = 5200,
    MAC_AUTH_RSP2_TIMEOUT = 5201,
    MAC_AUTH_RSP4_TIMEOUT = 5202,
    MAC_ASOC_RSP_TIMEOUT = 5203,
#ifdef _PRE_WLAN_FEATURE_CHBA
    /* CHBA: 添加私有code */
    MAC_CHBA_INIT_CODE = 6200,
    MAC_CHBA_INVAILD_CONNECT_CMD = 6201,
    MAC_CHBA_REPEAT_CONNECT_CMD = 6202, /* 重复建链命令，且两条建链命令参数不匹配 */
    MAC_CHBA_CREATE_NEW_USER_FAIL = 6203, /* 创建新用户失败，一般是因为超过芯片规格 */
    MAC_CHBA_UNSUP_ASSOC_CHANNEL, /* 无法在下发的建链信道上完成建链 */
    MAC_CHBA_COEX_FAIL, /* 不满足vap共存条件 */
    MAC_CHBA_UNSUP_PARALLEL_CONNECT, /* 暂不支持并行建链 */
#endif
} mac_status_code_enum;
typedef osal_u16 mac_status_code_enum_uint16;

/* BA会话管理确认策略 */
typedef enum {
    MAC_BA_POLICY_DELAYED = 0,
    MAC_BA_POLICY_IMMEDIATE,

    MAC_BA_POLICY_BUTT
} mac_ba_policy_enum;
typedef osal_u8 mac_ba_policy_enum_uint8;

/* 发起DELBA帧的端点的枚举 */
typedef enum {
    MAC_RECIPIENT_DELBA = 0, /* 数据的接收端 */
    MAC_ORIGINATOR_DELBA,    /* 数据的发起端 */

    MAC_BUTT_DELBA
} mac_delba_initiator_enum;
typedef osal_u8 mac_delba_initiator_enum_uint8;

/* 发起DELBA帧的业务类型的枚举 */
typedef enum {
    MAC_DELBA_TRIGGER_COMM = 0, /* 配置命令触发 */
    MAC_DELBA_TRIGGER_BTCOEX,   /* BT业务触发 */

    MAC_DELBA_TRIGGER_BUTT
} mac_delba_trigger_enum;
typedef osal_u8 mac_delba_trigger_enum_uint8;

/* 802.11n下的私有请求 */
typedef enum {
    MAC_A_MPDU_START = 0,
    MAC_A_MPDU_END = 1,

    MAC_A_MPDU_BUTT
} mac_priv_req_11n_enum;
typedef osal_u8 mac_priv_req_11n_enum_uint8;

/* BlockAck Category下的Action值的枚举 */
typedef enum {
    MAC_BA_ACTION_ADDBA_REQ = 0,
    MAC_BA_ACTION_ADDBA_RSP = 1,
    MAC_BA_ACTION_DELBA = 2,

    MAC_BA_ACTION_BUTT
} mac_ba_action_type_enum;
typedef osal_u8 mac_ba_action_type_enum_uint8;

/* Reason Codes for Deauthentication and Disassociation Frames */
typedef enum {
    MAC_UNSPEC_REASON = 1,
    MAC_AUTH_NOT_VALID = 2,
    MAC_DEAUTH_LV_SS = 3,
    MAC_INACTIVITY = 4,
    MAC_AP_OVERLOAD = 5,
    MAC_NOT_AUTHED = 6,
    MAC_NOT_ASSOCED = 7,
    MAC_DISAS_LV_SS = 8,
    MAC_ASOC_NOT_AUTH = 9,
    MAC_INVLD_ELEMENT = 13,
    MAC_MIC_FAIL = 14,
    MAC_4WAY_HANDSHAKE_TIMEOUT = 15,
    MAC_IEEE_802_1X_AUTH_FAIL = 23,
    MAC_UNSPEC_QOS_REASON = 32,
    MAC_QAP_INSUFF_BANDWIDTH = 33,
    MAC_POOR_CHANNEL = 34,
    MAC_STA_TX_AFTER_TXOP = 35,
    MAC_QSTA_LEAVING_NETWORK = 36,
    MAC_QSTA_INVALID_MECHANISM = 37,
    MAC_QSTA_SETUP_NOT_DONE = 38,
    MAC_QSTA_TIMEOUT = 39,
    MAC_QSTA_CIPHER_NOT_SUPP = 45
} mac_reason_code_enum;
typedef osal_u16 mac_reason_code_enum_uint16;

/*****************************************************************************
  信息元素(Infomation Element)的Element ID
  协议521页，Table 8-54—Element IDs
*****************************************************************************/
typedef enum {
    MAC_EID_SSID = 0,
    MAC_EID_RATES = 1,
    MAC_EID_FHPARMS = 2,
    MAC_EID_DSPARMS = 3,
    MAC_EID_CFPARMS = 4,
    MAC_EID_TIM = 5,
    MAC_EID_IBSSPARMS = 6,
    MAC_EID_COUNTRY = 7,
    MAC_EID_REQINFO = 10,
    MAC_EID_QBSS_LOAD = 11,
    MAC_EID_TSPEC = 13,
    MAC_EID_TCLAS = 14,
    MAC_EID_CHALLENGE = 16,
    /* 17-31 reserved */
    MAC_EID_PWRCNSTR = 32,
    MAC_EID_PWRCAP = 33,
    MAC_EID_TPCREQ = 34,
    MAC_EID_TPCREP = 35,
    MAC_EID_SUPPCHAN = 36,
    MAC_EID_CHANSWITCHANN = 37, /* Channel Switch Announcement IE */
    MAC_EID_MEASREQ = 38,
    MAC_EID_MEASREP = 39,
    MAC_EID_QUIET = 40,
    MAC_EID_IBSSDFS = 41,
    MAC_EID_ERP = 42,
    MAC_EID_TCLAS_PROCESS = 44,
    MAC_EID_HT_CAP = 45,
    MAC_EID_QOS_CAP = 46,
    MAC_EID_RESERVED_47 = 47,
    MAC_EID_RSN = 48,
    MAC_EID_RESERVED_49 = 49,
    MAC_EID_XRATES = 50,
    MAC_EID_AP_CHAN_REPORT = 51,
    MAC_EID_NEIGHBOR_REPORT = 52,
    MAC_EID_MOBILITY_DOMAIN = 54,
    MAC_EID_FT = 55,
    MAC_EID_TIMEOUT_INTERVAL = 56,
    MAC_EID_RDE = 57,
    MAC_EID_OPERATING_CLASS = 59,  /* Supported Operating Classes */
    MAC_EID_EXTCHANSWITCHANN = 60, /* Extended Channel Switch Announcement IE */
    MAC_EID_HT_OPERATION = 61,
    MAC_EID_SEC_CH_OFFSET = 62, /* Secondary Channel Offset IE */
    MAC_EID_WAPI = 68,          /* IE for WAPI */
    MAC_EID_TIME_ADVERTISEMENT = 69,
    MAC_EID_RRM = 70,                /* Radio resource measurement */
    MAC_EID_MULTIPLE_BSSID = 71,     /* Multiple BSSID */
    MAC_EID_2040_COEXT = 72,         /* 20/40 BSS Coexistence IE */
    MAC_EID_2040_INTOLCHREPORT = 73, /* 20/40 BSS Intolerant Channel Report IE */
    MAC_EID_OBSS_SCAN = 74,          /* Overlapping BSS Scan Parameters IE */
    MAC_EID_MMIE = 76,               /* 802.11w Management MIC IE */
    MAC_EID_EVENT_REPORT = 79,
    MAC_EID_NONTRANS_BSSID_CAPA = 83, /* Nontransmitted BSSID Capability element */
    MAC_EID_MULTI_BSSID_IDX = 85,    /* Multiple BSSID-Index element */
    MAC_EID_FMS_DESCRIPTOR = 86,     /* 802.11v FMS descriptor IE */
    MAC_EID_FMS_REQUEST = 87,        /* 802.11v FMS request IE */
    MAC_EID_FMS_RESPONSE = 88,       /* 802.11v FMS response IE */
    MAC_EID_BSSMAX_IDLE_PERIOD = 90, /* BSS MAX IDLE PERIOD */
    MAC_EID_TFS_REQUEST = 91,
    MAC_EID_TFS_RESPONSE = 92,
    MAC_EID_TIM_BCAST_REQUEST = 94,
    MAC_EID_TIM_BCAST_RESPONSE = 95,
    MAC_EID_INTERWORKING = 107,
    MAC_EID_EXT_CAPS = 127, /* Extended Capabilities IE */
    MAC_EID_VHT_TYPE = 129,
    MAC_EID_11NTXBF = 130, /* 802.11n txbf */
    MAC_EID_RESERVED_133 = 133,
    MAC_EID_TPC = 150,
    MAC_EID_CCKM = 156,
    MAC_EID_VHT_CAP = 191,
    MAC_EID_VHT_OPERN = 192,         /* VHT Operation IE */
    MAC_EID_WIDE_BW_CH_SWITCH = 194, /* Wide Bandwidth Channel Switch IE */
    MAC_EID_OPMODE_NOTIFY = 199,     /* Operating Mode Notification IE */
    MAC_EID_FTMP = 206,
    MAC_EID_TWT = 216,
    MAC_EID_FTMSI = 255,
    MAC_EID_VENDOR = 221, /* vendor private */
    MAC_EID_WMM = 221,
    MAC_EID_WPA = 221,
    MAC_EID_WPS = 221,
    MAC_EID_P2P = 221,
    MAC_EID_WFA_TPC_RPT = 221,
    MAC_EID_RSNX = 244,
    MAC_EID_HE = 255,
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    MAC_EID_WUR = 255,
#endif
} mac_eid_enum;
typedef osal_u8 mac_eid_enum_uint8;

typedef enum {
    MAC_EID_EXT_OWE_DH_PARAM = 32,
    MAC_EID_EXT_HE_CAP = 35,
    MAC_EID_EXT_HE_OPERATION = 36,
    MAC_EID_EXT_UORA_PARAMETER_SET = 37,
    MAC_EID_EXT_HE_EDCA = 38,
    MAC_EID_EXT_HE_SRP = 39,
    MAC_EID_EXT_NFR_PARAM_SET = 41,
    MAC_EID_EXT_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT = 42,
    MAC_EID_EXT_QUIET_TIME_PERIOD_SETUP = 43,
    MAC_EID_EXT_ESS_REPORT = 45,
    MAC_EID_EXT_OPS = 46,
    MAC_EID_EXT_HE_BSS_LOAD = 47,
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    MAC_EID_EXT_WUR_CAP = 48,
    MAC_EID_EXT_WUR_OPERATION = 49,
    MAC_EID_EXT_WUR_MODE = 50,
#endif
    MAC_EID_EXT_MULTIPLE_BSSID_CONFIGURATION = 55,
    MAC_EID_EXT_NON_INHERITANCE = 56,
    MAC_EID_EXT_KNOWN_BSSID = 57,
    MAC_EID_EXT_SHORT_SSID_LIST = 58,
    MAC_EID_EXT_HE_6GHZ_BAND_CAPABILITIES = 59,
    MAC_EID_EXT_UL_MU_POWER_CAPABILITIES = 60,

    MAC_EID_EXT_BUT
} mac_eid_extension_num;
typedef osal_u8 mac_eid_extension_num_uint8;

typedef enum {
    MAC_SUB_EID_NONTRANS_BSSID_PROFILE = 0,
    MAC_SUB_EID_VENDOR_SPECIFIC = 221,

    MAC_SUB_EID_BUTT
}mac_sub_eid_enum;

#ifdef _PRE_WLAN_FEATURE_TWT
/* TWT命令类型 */
typedef enum {
    MAC_TWT_COMMAND_REQUEST = 0,
    MAC_TWT_COMMAND_SUGGEST = 1,
    MAC_TWT_COMMAND_DEMAND = 2,
    MAC_TWT_COMMAND_GROUPING = 3,
    MAC_TWT_COMMAND_ACCEPT = 4,
    MAC_TWT_COMMAND_ALTERNATE = 5,
    MAC_TWT_COMMAND_DICTATE = 6,
    MAC_TWT_COMMAND_REJECT = 7,
} mac_twt_command_enum;
typedef osal_u8 mac_twt_command_enum_uint8;
#endif

/* VHT Category�µ�Actionֵ��ö�� */
typedef enum {
    MAC_VHT_ACTION_COMPRESSED_BEAMFORMING = 0,
    MAC_VHT_ACTION_GROUPID_MANAGEMENT = 1,
    MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION = 2,

    MAC_VHT_ACTION_BUTT
} mac_vht_action_type_enum;
typedef osal_u8 mac_vht_action_type_enum_uint8;

/* sizes for DHCP options */
#ifndef DHCP_CHADDR_LEN
#define DHCP_CHADDR_LEN         16
#endif
#define SERVERNAME_LEN          64
#define BOOTFILE_LEN            128
#define DHCP_OPTION_LEN         72

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 02 dev侧用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack(1)
/* 此文件中结构体与协议一致，要求1字节对齐，统一加__OAL_DECLARE_PACKED */
struct mac_ether_header {
    osal_u8 ether_dhost[ETHER_ADDR_LEN];
    osal_u8 ether_shost[ETHER_ADDR_LEN];
    osal_u16 ether_type;
};
typedef struct mac_ether_header mac_ether_header_stru;

struct mac_llc_snap {
    osal_u8 llc_dsap;
    osal_u8 llc_ssap;
    osal_u8 control;
    osal_u8 org_code[3];  /* 3字节存储OUI组织代码? */
    osal_u16 ether_type;
};
typedef struct mac_llc_snap mac_llc_snap_stru;

/* eapol帧头 */
struct mac_eapol_header {
    osal_u8 version;
    osal_u8 type;
    osal_u16 length;
};
typedef struct mac_eapol_header mac_eapol_header_stru;

/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
/* EAPOL KEY 结构定义 */
struct mac_eapol_key {
    osal_u8 type;
    /* Note: key_info, key_length, and key_data_length are unaligned */
    osal_u8 key_info[2];   /* big endian,2字节用来储存EAPOL KEY信息 */
    osal_u8 key_length[2]; /* big endian,2字节用来储存EAPOL KEY长度 */
    osal_u8 replay_counter[WPA_REPLAY_COUNTER_LEN];
    osal_u8 key_nonce[WPA_NONCE_LEN];
    osal_u8 key_iv[16];      /* 16字节储存密钥信息 */
    osal_u8 key_rsc[WPA_KEY_RSC_LEN];
    osal_u8 key_id[8]; /* Reserved in IEEE 802.11i/RSN,8字节 */
    osal_u8 key_mic[16]; /* 16字节储存EAPOL KEY的完整性信息？ */
    osal_u8 key_data_length[2]; /* big endian,2字节储存EAPOL KEY数据段长度 */
    /* followed by key_data_length bytes of key_data */
};
typedef struct mac_eapol_key mac_eapol_key_stru;

/*
 * Structure of the IP frame
 */
struct mac_ip_header {
    osal_u8 version_ihl;
    osal_u8 tos;
    osal_u16 tot_len;
    osal_u16 id;
    osal_u16 frag_off;
    osal_u8 ttl;
    osal_u8 protocol;
    osal_u16 check;
    osal_u32 saddr;
    osal_u32 daddr;
    /* The options start here. */
};
typedef struct mac_ip_header mac_ip_header_stru;

struct mac_tcp_header {
    osal_u16 sport;
    osal_u16 dport;
    osal_u32 seqnum;
    osal_u32 acknum;
    osal_u8 offset;
    osal_u8 flags;
    osal_u16 window;
    osal_u16 check;
    osal_u16 urgent;
};
typedef struct mac_tcp_header mac_tcp_header_stru;

/* UDP头部结构 */
struct mac_udp_header {
    osal_u16   src_port;
    osal_u16   des_port;
    osal_u16   udp_len;
    osal_u16   check_sum;
};
typedef struct mac_udp_header mac_udp_header_stru;

struct dhcp_message {
    osal_u8 op;                           /* message type */
    osal_u8 hwtype;                       /* hardware address type */
    osal_u8 hwlen;                        /* hardware address length */
    osal_u8 hwopcount;                    /* should be zero in client message */
    osal_u32 xid;                         /* transaction id */
    osal_u16 secs;                        /* elapsed time in sec. from boot */
    osal_u16 flags;
    osal_u32 ciaddr;                      /* (previously allocated) client IP */
    osal_u32 yiaddr;                      /* 'your' client IP address */
    osal_u32 siaddr;                      /* should be zero in client's messages */
    osal_u32 giaddr;                      /* should be zero in client's messages */
    osal_u8 chaddr[DHCP_CHADDR_LEN];      /* client's hardware address */
    osal_u8 servername[SERVERNAME_LEN];   /* server host name */
    osal_u8 bootfile[BOOTFILE_LEN];       /* boot file name */
    osal_u32 cookie;
    osal_u8 options[DHCP_OPTION_LEN];     /* message options, cookie */
};
typedef struct dhcp_message dhcp_message_stru;

/* frame control字段结构体 */
struct mac_header_frame_control {
    osal_u16 protocol_version : 2, /* 协议版本 */
           type : 2,                /* 帧类型 */
           sub_type : 4,            /* 子类型 */
           to_ds : 1,               /* 发送DS */
           from_ds : 1,             /* 来自DS */
           more_frag : 1,           /* 分段标识 */
           retry : 1,               /* 重传帧 */
           power_mgmt : 1,          /* 节能管理 */
           more_data : 1,           /* 更多数据标识 */
           protected_frame : 1,     /* 加密标识 */
           order : 1;               /* 次序位 */
};
typedef struct mac_header_frame_control mac_header_frame_control_stru;

/* 基础802.11帧结构 */
struct mac_ieee80211_frame {
    mac_header_frame_control_stru   frame_control;
    osal_u16                        duration_value  : 15,
                                    duration_flag   : 1;
    osal_u8                         address1[WLAN_MAC_ADDR_LEN];
    osal_u8                         address2[WLAN_MAC_ADDR_LEN];
    osal_u8                         address3[WLAN_MAC_ADDR_LEN];
    osal_u16                        frag_num        : 4,
                                    seq_num         : 12;
};
typedef struct mac_ieee80211_frame mac_ieee80211_frame_stru;

/* ps poll帧结构 */
struct mac_ieee80211_pspoll_frame {
    mac_header_frame_control_stru   frame_control;
    osal_u16                        aid_value   : 14,                   /* ps poll 下的AID字段 */
                                    aid_flag1   : 1,
                                    aid_flag2   : 1;
    osal_u8                         bssid[WLAN_MAC_ADDR_LEN];
    osal_u8                         trans_addr[WLAN_MAC_ADDR_LEN];
};
typedef struct mac_ieee80211_pspoll_frame mac_ieee80211_pspoll_frame_stru;

/* qos帧结构 */
struct mac_ieee80211_qos_frame {
    mac_header_frame_control_stru   frame_control;
    osal_u16                      duration_value      : 15,           /* duration/id */
                                  duration_flag       : 1;
    osal_u8                       address1[WLAN_MAC_ADDR_LEN];
    osal_u8                       address2[WLAN_MAC_ADDR_LEN];
    osal_u8                       address3[WLAN_MAC_ADDR_LEN];
    osal_u16                      frag_num        : 4,                /* sequence control */
                                  seq_num         : 12;
    osal_u8                       qc_tid          : 4,
                                  qc_eosp         : 1,
                                  qc_ack_polocy   : 2,
                                  qc_amsdu        : 1;
    union {
        osal_u8                   qc_txop_limit;
        osal_u8                   qc_ps_buf_state_resv        : 1,
                                  qc_ps_buf_state_inducated   : 1,
                                  qc_hi_priority_buf_ac       : 2,
                                  qc_qosap_buf_load           : 4;
    } qos_control;
};
typedef struct mac_ieee80211_qos_frame mac_ieee80211_qos_frame_stru;

/* qos+HTC 帧结构 */
struct mac_ieee80211_qos_htc_frame {
    mac_header_frame_control_stru frame_control;
    osal_u16                      duration_value      : 15,           /* duration/id */
                                  duration_flag       : 1;
    osal_u8                       address1[WLAN_MAC_ADDR_LEN];
    osal_u8                       address2[WLAN_MAC_ADDR_LEN];
    osal_u8                       address3[WLAN_MAC_ADDR_LEN];
    osal_u16                      frag_num     : 4,                /* sequence control */
                                  seq_num      : 12;
    osal_u8                       qc_tid          : 4,
                                  qc_eosp         : 1,
                                  qc_ack_polocy   : 2,
                                  qc_amsdu        : 1;
    union {
        osal_u8                   qc_txop_limit;
        osal_u8                   qc_ps_buf_state_resv        : 1,
                                  qc_ps_buf_state_inducated   : 1,
                                  qc_hi_priority_buf_ac       : 2,
                                  qc_qosap_buf_load           : 4;
    } qos_control;

    osal_u32                      htc;
};
typedef struct mac_ieee80211_qos_htc_frame mac_ieee80211_qos_htc_frame_stru;


/* TIM信息元素结构体 */
struct mac_tim_ie {
    osal_u8 tim_ie; /* MAC_EID_TIM */
    osal_u8 tim_len;
    osal_u8 dtim_count;     /* DTIM count */
    osal_u8 dtim_period;    /* DTIM period */
    osal_u8 tim_bitctl;     /* bitmap control */
    osal_u8 tim_bitmap[1]; /* variable-length bitmap */
};
typedef struct mac_tim_ie mac_tim_ie_stru;

/* 扩展能力信息元素结构体定义 */
struct mac_ext_cap_ie {
    osal_u8       coexistence_mgmt_2040 : 1,
                resv1             : 1,
                ext_chan_switch       : 1,
                resv2                 : 1,
                psmp                  : 1,
                resv3                 : 1,
                s_psmp                : 1,
                event                 : 1;
    osal_u8   resv4                : 4,
            proxyarp             : 1,
            resv13               : 3;   /* bit13~bit15 */
    osal_u8   resv5                : 3,
            bss_transition       : 1,   /* bit19 */
            resv14               : 2,   /* bit20~bit23 */
            multiple_bssid       : 1,
            resv15               : 1;
    osal_u8   resv6                : 7,
            interworking         : 1;
    osal_u8   resv7                         : 5,
            tdls_prhibited                : 1,
            tdls_channel_switch_prhibited : 1,
            resv8                         : 1;

    osal_u8   resv9                : 8;
    osal_u8   resv10               : 8;

    osal_u8   resv11                        : 6,
            operating_mode_notification   : 1, /* 11ac Operating Mode Notification特性标志 */
            resv12                        : 1;

    osal_u8 resv16 : 6,
          ftm_resp : 1, /* bit70 Fine Timing Measurement Responder */
          ftm_int : 1; /* bit71 Fine Timing Measurement Initiator */

    osal_u8 resv17 : 5,
          twt_requester_support : 1, /* 11ax TWT requester 特性标志 */
          resv18 : 2;
};
typedef struct mac_ext_cap_ie mac_ext_cap_ie_stru;

struct mac_twt_request_type {
    osal_u16 request : 1;
    osal_u16 setup_command : 3;
    osal_u16 trigger : 1;
    osal_u16 implicit : 1;
    osal_u16 flow_type : 1;
    osal_u16 flow_id : 3;
    osal_u16 intrval_exponent : 5;
    osal_u16 protection : 1;
};
typedef struct mac_twt_request_type mac_twt_request_type_stru;

struct mac_twt_control_field {
    osal_u8 ndp_paging_indicator : 1;
    osal_u8 responder_pm_mode : 1;
    osal_u8 negotiation : 2;
    osal_u8 twt_info_frame_disable : 1;
    osal_u8 wake_duration_unit : 1;
    osal_u8 resv : 2;
};
typedef struct mac_twt_control_field mac_twt_control_field_stru;

/* 新增结构体，用以表示TWT Element */
struct mac_twt_ie_individual {
    osal_u8 element_id;
    osal_u8 len;
    mac_twt_control_field_stru control;
    mac_twt_request_type_stru request_type;
    osal_u64 twt;
    osal_u8 min_duration;
    osal_u16 intrval_mantissa;
    osal_u8 channel;
};
typedef struct mac_twt_ie_individual mac_twt_ie_individual_stru;

/* 封装了twt信息元素，修改了结构体名称 */
struct mac_individual_twt_setup_frame {
    osal_u8 category;
    osal_u8 action;
    osal_u8 dialog_token;
    mac_twt_ie_individual_stru twt_element;
};
typedef struct mac_individual_twt_setup_frame mac_individual_twt_setup_frame_stru;

struct mac_twt_teardown {
    osal_u8 category;
    osal_u8 action;
    osal_u8 flow_id          : 3,
            resv1            : 2,
            nego_type        : 2,
            teardown_all_twt : 1;
};
typedef struct mac_twt_teardown mac_twt_teardown_stru;

struct mac_twt_information_field {
    osal_u8 flow_id : 3;
    osal_u8 response_requested : 1;
    osal_u8 next_twt_request : 1;
    osal_u8 next_twt_subfield_size : 2;
    osal_u8 all_twt : 1;
};
typedef struct mac_twt_information_field mac_twt_information_field_stru;

struct mac_twt_information_frame {
    osal_u8 category;
    osal_u8 action;
    mac_twt_information_field_stru twt_information_filed;
    osal_u64 next_twt;
};
typedef struct mac_twt_information_frame mac_twt_information_frame_stru;

typedef struct {
    mac_llc_snap_stru snap;
    mac_ip_header_stru ip_hdr;
    mac_udp_header_stru udp_hdr;
    dhcp_message_stru dhcp;
} dhcp_record_frame;

/* HE_Cap:he mac cap 字段，Len=6  */
struct mac_frame_he_mac_cap {
    osal_u8 htc_he_support : 1, /* B0-指示站点是否支持接收Qos Data、Qos Null、管理帧携带 HE 变体的HT Control field */
          twt_requester_support : 1, /* B1-是否支持TWT Requester */
          twt_responder_support : 1, /* B2-是否支持TWT Responder */
          fragmentation_support : 2, /* B3-动态分片能力 */
          msdu_max_fragment_num : 3; /* B5-msdu最大分片数 */

    osal_u16 min_fragment_size : 2,            /* B8-最小分片的长度 */
           trigger_mac_padding_duration : 2,    /* B10-trigger mac padding 时长 */
           mtid_aggregation_rx_support : 3,     /* B12-接收多tid聚合支持 */
           he_link_adaptation : 2,              /* B15-使用HE 变体的HT Control field 调整 */
           all_ack_support : 1,                 /* B17-支持接收M-BA */
           trs_support : 1,                         /* B18-A-Control 支持 TSR */
           bsr_support : 1,                     /* B19-A-Control 支持BSR */
           broadcast_twt_support : 1,           /* B20-支持广播twt */
           ba_bitmap_support_32bit : 1,         /* B21-支持32位bitmap ba */
           mu_cascading_support : 1,            /* B22-mu csacade */
           ack_enabled_aggregation_support : 1; /* B23-A-MPDU ack使能 */

    osal_u8 reserved2 : 1,                      /* B24 */
          om_control_support : 1,           /* B25-A-Control 支持OMI  */
          ofdma_ra_support : 1,             /* B26-OFDMA 随机接入 */
          max_ampdu_length_exponent : 2,    /* B27-A-MPDU最大长度 */
          amsdu_fragment_support : 1,       /* B29- */
          flex_twt_schedule_support : 1,    /* B30- */
          rx_control_frame_to_multibss : 1; /* B31- */

    osal_u16          bsrp_bqrp_ampdu_addregation                        : 1, /* B32- */
                    qtp_support                                        : 1,
                    bqr_support                                        : 1,
                    sr_responder                                       : 1,
                    ndp_feedback_report_support                        : 1,
                    ops_support                                        : 1,
                    amsdu_ampdu_support                                : 1,
                    mtid_aggregation_tx_support                        : 3,
                    he_subchannel_sel_trans_support                        : 1, /* B42 */
                    ru_support_2x996_tone                               : 1, /* B43 */
                    om_control_ul_mu_data_disable_rx_support               : 1, /* B44 */
                    dynamic_sm_ps                                          : 1, /* B45 */
                    punctured_sounding_support                             : 1, /* B46 */
                    ht_vht_tf_rx_support                                   : 1; /* B47 */
};
typedef struct mac_frame_he_mac_cap mac_frame_he_mac_cap_stru;

/* HE_CAP: PHY Cap 字段 Len=9   */
struct mac_frame_he_phy_cap {
    osal_u8 reserved : 1,
          channel_width_set : 7; /* B1-7: */

    osal_u16 punctured_preamble_rx : 4,     /* B8-11: */
           device_class : 1,                 /* B12- */
           ldpc_coding_in_paylod : 1,        /* B13- */
           he_su_ppdu_1xltf_08us_gi : 1,     /* B14-HE SU PPDU 1x HE-LTF + 0.8usGI  */
           midamble_rx_max_nsts : 2,         /* B15 */
           ndp_4xltf_32us : 1,               /* B17-ndp 4x HE-LTF + 3.2usGI */
           stbc_tx_below_80mhz : 1,          /* B18 */
           stbc_rx_below_80mhz : 1,          /* B19 */
           dopper_tx : 1,                    /* B20 */
           droper_rx : 1,                    /* B21 */
           full_bandwidth_ul_mu_mimo : 1,    /* B22 */
           partial_bandwidth_ul_mu_mimo : 1; /* B23 */

    osal_u8 dcm_max_constellation_tx : 2,       /* B24- */
          dcm_max_nss_tx : 1,                   /* B26 */
          dcm_max_constellation_rx : 2,         /* B27- */
          dcm_max_nss_rx : 1,                   /* B29- */
          rx_partial_bw_su_in_20mhz_he_mu_ppdu : 1, /* B30 */
          su_beamformer : 1;                    /* B31- */

    osal_u8 su_beamformee : 1,            /* B32- */
          mu_beamformer : 1,              /* B33- */
          beamformee_sts_below_80mhz : 3, /* B34- */
          beamformee_sts_over_80mhz : 3;  /* B37- */

    osal_u8 below_80mhz_sounding_dimensions_num : 3, /* B40- */
          over_80mhz_sounding_dimensions_num : 3,    /* B43- */
          ng16_su_feedback : 1,                      /* B46- */
          ng16_mu_feedback : 1;                      /* B47- */

    osal_u8 codebook_42_su_feedback : 1,              /* B48- */
          codebook_75_mu_feedback : 1,                /* B49- */
          trigger_su_beamforming_feedback : 1,        /* B50- */
          trigger_mu_beamforming_partial_bw_feedback : 1, /* B51 */
          trigger_cqi_feedback : 1,                   /* B52- */
          partial_bandwidth_extended_range : 1,       /* B53- */
          partial_bandwidth_dl_mu_mimo : 1,           /* B54- */
          ppe_threshold_present : 1;                  /* B55- */

    osal_u8 srp_based_sr_support : 1,      /* B56- */
          power_boost_factor_support : 1,  /* B57- */
          he_su_mu_ppdu_4xltf_08us_gi : 1, /* B58-he su ppdu and he mu ppdu 4x HE-LTF + 0.8usGI */
          max_nc : 3,                      /* B59- */
          stbc_tx_over_80mhz : 1,          /* B62- */
          stbc_rx_over_80mhz : 1;          /* B63- */

    osal_u8 he_er_su_ppdu_4xltf_08us_gi : 1, /* B64- */
          he_ppdu_2g_20mhz_in_40mhz : 1,     /* B65- */
          he_ppdu_20mhz_in_160mhz : 1,       /* B66- */
          he_ppdu_80mhz_in_160mhz : 1,       /* B67- */
          he_er_su_ppdu_1xltf_08us_gi : 1,   /* B68- */
          midamble_rx_2x_1x_he_ltf : 1,      /* B69- */
          dcm_max_ru : 2;                        /* B70- */

    osal_u8 longer_16_he_sig_ofdm_symbols_support : 1,              /* B72 */
          non_trigger_cqi_feedback : 1,                             /* B73 */
          tx_1024_qam_below_242_tone_ru_support : 1,                /* B74 */
          rx_1024_qam_below_242_tone_ru_support : 1,                /* B75 */
          rx_full_bw_su_using_he_mu_ppdu_compress_he_sig_b : 1,     /* B76 */
          rx_full_bw_su_using_he_mu_ppdu_non_compress_he_sig_b : 1, /* B77 */
          nominal_packet_padding : 2;                               /* B78 */

    osal_u8 reserved2; /* B80 */
};
typedef struct mac_frame_he_phy_cap mac_frame_he_phy_cap_stru;

/* HE CAP:Tx Rx MCS NSS Support */
struct mac_frame_he_mcs_nss_bit_map {
    osal_u16 max_he_mcs_for_1ss : 2,
           max_he_mcs_for_2ss : 2,
           max_he_mcs_for_3ss : 2,
           max_he_mcs_for_4ss : 2,
           max_he_mcs_for_5ss : 2,
           max_he_mcs_for_6ss : 2,
           max_he_mcs_for_7ss : 2,
           max_he_mcs_for_8ss : 2;
};
typedef struct mac_frame_he_mcs_nss_bit_map mac_frame_he_mcs_nss_bit_map_stru;

struct mac_fram_he_mac_nsss_set {
    mac_frame_he_mcs_nss_bit_map_stru rx_he_mcs_below_80mhz;
    mac_frame_he_mcs_nss_bit_map_stru tx_he_mcs_below_80mhz;
};
typedef struct mac_fram_he_mac_nsss_set mac_fram_he_mac_nsss_set_stru;

struct mac_frame_he_operation_param {
    osal_u16 default_pe_duration : 3,             /* B0_2: */
           twt_required : 1,                    /* B3: */
           txop_duration_rts_threshold : 10,    /* B4_13: */
           vht_operation_info_present : 1,      /* B14: */
           co_located_bss : 1;                  /* B15 */

    osal_u8 er_su_disable : 1,            /* B16； */
          er_6g_oper_info_present : 1,  /* B17: */
          reserved : 6;                 /* B18_23: */
};
typedef struct mac_frame_he_operation_param mac_frame_he_operation_param_stru;

struct mac_frame_he_bss_color_info {
    osal_u8           bss_color                                   : 6,
                      partial_bss_color                           : 1,
                      bss_color_disable                           : 1;
};
typedef struct mac_frame_he_bss_color_info mac_frame_he_bss_color_info_stru;

struct mac_frame_vht_operation_info {
    osal_u8 channel_width;
    osal_u8 center_freq_seg0;
    osal_u8 center_freq_seg1;
};
typedef struct mac_frame_vht_operation_info mac_frame_vht_operation_info_stru;

/* HE_CAP:固定长度部分 */
struct mac_frame_he_cap_ie {
    mac_frame_he_mac_cap_stru he_mac_cap;
    mac_frame_he_phy_cap_stru he_phy_cap;
    mac_fram_he_mac_nsss_set_stru he_mcs_nss;
    osal_u8 rsv[7]; /* 7 BYTE保留字段 */
};
typedef struct mac_frame_he_cap_ie mac_frame_he_cap_ie_stru;

struct mac_frame_he_oper_ie {
    mac_frame_he_operation_param_stru  he_oper_param;
    mac_frame_he_bss_color_info_stru   bss_color;
    mac_frame_he_mcs_nss_bit_map_stru  he_basic_mcs_nss;
    osal_u8                            rsv[1];
    mac_frame_vht_operation_info_stru  vht_operation_info;
};
typedef struct mac_frame_he_oper_ie mac_frame_he_oper_ie_stru;
/* 02 dev侧用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

osal_u8 *mac_find_ie_etc(osal_u8 eid, osal_u8 *ies, osal_s32 len);

/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*****************************************************************************
 函 数 名  : mac_hdr_set_frame_control
 功能描述  : This function sets the 'frame control' bits in the MAC header of the
             input frame to the given 16-bit value.

*****************************************************************************/
static inline osal_void mac_hdr_set_frame_control(osal_u8 *header, osal_u16 fc)
{
    *(osal_u16 *)header = fc;
}

/*****************************************************************************
 函 数 名  : mac_hdr_get_from_ds
 功能描述  : This function extracts the 'from ds' bit from the MAC header of the input frame.
             Returns the value in the LSB of the returned value.

*****************************************************************************/
static inline osal_u8 mac_hdr_get_from_ds(osal_u8 *header)
{
    return (osal_u8)((mac_header_frame_control_stru *)(header))->from_ds;
}

/*****************************************************************************
 函 数 名  : mac_hdr_get_to_ds
 功能描述  : This function extracts the 'to ds' bit from the MAC header of the input frame.
             Returns the value in the LSB of the returned value.

*****************************************************************************/
static inline osal_u8 mac_hdr_get_to_ds(osal_u8 *header)
{
    return (osal_u8)((mac_header_frame_control_stru *)(header))->to_ds;
}

/*****************************************************************************
 函 数 名  : mac_get_tid_value
 功能描述  : 四地址获取帧头中的tid

*****************************************************************************/
static inline osal_u8 mac_get_tid_value(osal_u8 *header, oal_bool_enum_uint8 is_4addr)
{
    if (is_4addr) {
        return (header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
    } else {
        return (header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x07); /* B0 - B2 */
    }
}

/*****************************************************************************
 函 数 名  : mac_null_data_encap
 功能描述  : 封装空桢
 输入参数  : header－80211头部指针
             fc frame control类型
             da: 目的mac地址
             sa: 源mac地址

*****************************************************************************/
static inline osal_void mac_null_data_encap(osal_u8 *header, osal_u16 fc, const osal_u8 *da, const osal_u8 *sa)
{
    mac_hdr_set_frame_control(header, fc);

    if ((fc & WLAN_FRAME_FROM_AP) && !(fc & WLAN_FRAME_TO_AP)) {
        /* 设置ADDR1为DA 偏移4字节 */
        oal_set_mac_addr((header + 4), da);

        /* 设置ADDR2为BSSID 偏移10字节 */
        oal_set_mac_addr((header + 10), sa);

        /* 设置ADDR3为SA 偏移16字节 */
        oal_set_mac_addr((header + 16), sa);
    }
    if (!(fc & WLAN_FRAME_FROM_AP) && (fc & WLAN_FRAME_TO_AP)) {
        /* 设置ADDR1为BSSID 偏移4字节 */
        oal_set_mac_addr((header + 4), da);
        /* 设置ADDR2为SA 偏移10字节 */
        oal_set_mac_addr((header + 10), sa);
        /* 设置ADDR3为DA 偏移16字节 */
        oal_set_mac_addr((header + 16), da);
    }
}

/*****************************************************************************
 函 数 名  : mac_get_frame_sub_type
 功能描述  : 获取报文的类型和子类型

*****************************************************************************/
static inline osal_u8 mac_get_frame_type_and_subtype(const osal_u8 *mac_header)
{
    return (mac_header[0] & 0xFC);
}

/*****************************************************************************
 函 数 名  : mac_get_frame_type
 功能描述  : 获取报文类型

*****************************************************************************/
static inline osal_u8 mac_get_frame_type(osal_u8 *mac_header)
{
    return (mac_header[0] & 0x0C);
}

/*****************************************************************************
 函 数 名  : mac_get_frame_sub_type
 功能描述  : 获取报文的子类型

*****************************************************************************/
static inline osal_u8 mac_get_frame_sub_type(const osal_u8 *mac_header)
{
    return (mac_header[0] & 0xF0);
}

/*****************************************************************************
 函 数 名  : mac_frame_get_type_value
 功能描述  : 获取80211帧帧类型，取值0~2

*****************************************************************************/
static inline osal_u8 mac_frame_get_type_value(const osal_u8 *mac_header)
{
    return (mac_header[0] & 0x0C) >> 2; /* 右移2 bit */
}

/*****************************************************************************
 函 数 名  : mac_frame_get_subtype_value
 功能描述  : 获取802.11帧子类型的值(0~15)
             帧第一个字节的高四位

*****************************************************************************/
static inline osal_u8 mac_frame_get_subtype_value(const osal_u8 *mac_header)
{
    return ((mac_header[0] & 0xF0) >> 4); /* 右移4 bit */
}

/*****************************************************************************
 函 数 名  : mac_get_address1
 功能描述  : 拷贝地址1
*****************************************************************************/
static inline osal_void mac_get_address1(osal_u8 *mac_hdr, osal_u8 *addr)
{
    /* 4:addr1相对802.11帧头的偏移量 */
    (osal_void)memcpy_s(addr, WLAN_MAC_ADDR_LEN, mac_hdr + 4, WLAN_MAC_ADDR_LEN);
}

/*****************************************************************************
 函 数 名  : mac_hdr_set_duration
 功能描述  : 设置MAC头duration字段

*****************************************************************************/
static inline osal_void mac_hdr_set_duration(osal_u8 *header, osal_u16 duration)
{
    osal_u16 *dur = (osal_u16 *)(header + WLAN_HDR_DUR_OFFSET);

    *dur = duration;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_frame_common_rom.h */
