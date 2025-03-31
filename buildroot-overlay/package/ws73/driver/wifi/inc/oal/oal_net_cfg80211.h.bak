/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: oal net interface
 * Create: 2020-05-02
 */

#ifndef __OAL_NET_CFG80211_HCM_H__
#define __OAL_NET_CFG80211_HCM_H__

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <net/iw_handler.h>
#include <net/rtnetlink.h>
#include <net/netlink.h>
#include <net/sch_generic.h>
#include <net/ip6_checksum.h>
#include <net/ipv6.h>
#include <net/arp.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#include <net/sock.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#include <net/cfg80211.h>
#else
#include <net/wireless.h>
#endif
#include <../net/wireless/nl80211.h>

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <linux/socket.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/rtnetlink.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_arp.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/icmpv6.h>
#include <linux/ipv6.h>
#include <linux/inet.h>
#include <linux/nl80211.h>
#include <linux/ieee80211.h>
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "lwip/netif.h"
#include "oal_skbuff.h"
#endif

#include "wlan_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef osal_u8 app_ie_type_uint8;
#define OAL_MAC_ADDR_LEN   6
#define OAL_IF_NAME_SIZE   16
#define OAL_IEEE80211_MAX_SSID_LEN          32      /* 最大SSID长度 */
#define OAL_NL80211_MAX_NR_CIPHER_SUITES    5
#define BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY 123
#define MAX_STA_NUM_OF_ONE_GROUP    8      /* 一个组播组最大支持32个用户 */
#define MAX_NUM_OF_GROUP            8      /* 最大支持256个组播组 */

typedef enum {
    OAL_NETDEV_TX_OK     = 0x00,
    OAL_NETDEV_TX_BUSY   = 0x10,
    OAL_NETDEV_TX_LOCKED = 0x20,
} oal_net_dev_tx_enum;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define IEEE80211_HT_MCS_MASK_LEN       10
#define OAL_NL80211_MAX_NR_AKM_SUITES   2
#define OAL_WLAN_SA_QUERY_TR_ID_LEN     2
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define OAL_MAX_SCAN_CHANNELS           16
#else
#define OAL_MAX_SCAN_CHANNELS           14
#endif
#define WLAN_SA_QUERY_TR_ID_LEN         2
#define NL80211_RRF_NO_OFDM         (1<<0)
#define NL80211_RRF_DFS             (1<<4)
#define NL80211_RRF_NO_OUTDOOR      (1<<3)

typedef td_u8 oal_nl80211_channel_type_uint8;
typedef td_u8 nl80211_iftype_uint8;
typedef td_u8 cfg80211_signal_type_uint8;
typedef td_u8 oal_ieee80211_band_enum_uint8;
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum nl80211_band {
    NL80211_BAND_2GHZ,
    NL80211_BAND_5GHZ,
};

enum ieee80211_band {
    IEEE80211_BAND_2GHZ = NL80211_BAND_2GHZ,
    IEEE80211_BAND_5GHZ = NL80211_BAND_5GHZ,

    /* keep last */
    IEEE80211_NUM_BANDS
};

#define OAL_IEEE80211_BAND_2GHZ    IEEE80211_BAND_2GHZ
#define OAL_IEEE80211_BAND_5GHZ    IEEE80211_BAND_5GHZ
#define OAL_IEEE80211_NUM_BANDS    IEEE80211_NUM_BANDS

enum nl80211_channel_type {
    NL80211_CHAN_NO_HT,
    NL80211_CHAN_HT20,
    NL80211_CHAN_HT40MINUS,
    NL80211_CHAN_HT40PLUS
};
typedef enum nl80211_channel_type oal_nl80211_channel_type;

enum nl80211_key_type {
    NL80211_KEYTYPE_GROUP,
    NL80211_KEYTYPE_PAIRWISE,
    NL80211_KEYTYPE_PEERKEY,
    NUM_NL80211_KEYTYPES
};
typedef enum nl80211_key_type oal_nl80211_key_type;

enum nl80211_iftype {
    NL80211_IFTYPE_UNSPECIFIED,
    NL80211_IFTYPE_ADHOC,
    NL80211_IFTYPE_STATION,
    NL80211_IFTYPE_AP,
    NL80211_IFTYPE_AP_VLAN,
    NL80211_IFTYPE_WDS,
    NL80211_IFTYPE_MONITOR,
    NL80211_IFTYPE_MESH_POINT,
    NL80211_IFTYPE_P2P_CLIENT,
    NL80211_IFTYPE_P2P_GO,
    NL80211_IFTYPE_P2P_DEVICE,
    /* keep last */
    NUM_NL80211_IFTYPES,
    NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};

enum cfg80211_signal_type {
    CFG80211_SIGNAL_TYPE_NONE,
    CFG80211_SIGNAL_TYPE_MBM,
    CFG80211_SIGNAL_TYPE_UNSPEC,
};

enum station_info_flags {
    STATION_INFO_INACTIVE_TIME      = 1 << 0,
    STATION_INFO_RX_BYTES           = 1 << 1,
    STATION_INFO_TX_BYTES           = 1 << 2,
    STATION_INFO_LLID               = 1 << 3,
    STATION_INFO_PLID               = 1 << 4,
    STATION_INFO_PLINK_STATE        = 1 << 5,
    STATION_INFO_SIGNAL             = 1 << 6,
    STATION_INFO_TX_BITRATE         = 1 << 7,
    STATION_INFO_RX_PACKETS         = 1 << 8,
    STATION_INFO_TX_PACKETS         = 1 << 9,
    STATION_INFO_TX_RETRIES         = 1 << 10,
    STATION_INFO_TX_FAILED          = 1 << 11,
    STATION_INFO_RX_DROP_MISC       = 1 << 12,
    STATION_INFO_SIGNAL_AVG         = 1 << 13,
    STATION_INFO_RX_BITRATE         = 1 << 14,
    STATION_INFO_BSS_PARAM          = 1 << 15,
    STATION_INFO_CONNECTED_TIME     = 1 << 16,
    STATION_INFO_ASSOC_REQ_IES      = 1 << 17,
    STATION_INFO_STA_FLAGS          = 1 << 18,
    STATION_INFO_BEACON_LOSS_COUNT  = 1 << 19,
    STATION_INFO_T_OFFSET           = 1 << 20,
    STATION_INFO_LOCAL_PM           = 1 << 21,
    STATION_INFO_PEER_PM            = 1 << 22,
    STATION_INFO_NONPEER_PM         = 1 << 23,
    STATION_INFO_RX_BYTES64         = 1 << 24,
    STATION_INFO_TX_BYTES64         = 1 << 25,
};

/* 原hmac和dmac结构体SAE/EAP成员定义是反的。dmac未使用以hmac为准 */
typedef enum nl80211_auth_type {
    NL80211_AUTHTYPE_OPEN_SYSTEM,
    NL80211_AUTHTYPE_SHARED_KEY,
    NL80211_AUTHTYPE_FT,
    NL80211_AUTHTYPE_SAE,
    NL80211_AUTHTYPE_NETWORK_EAP,
    /* keep last */
    NL80211_AUTHTYPE_AUTOMATIC
} oal_nl80211_auth_type_enum;
typedef td_u8 oal_nl80211_auth_type_enum_uint8;

enum nl80211_sae_pwe_mechanism {
    NL80211_SAE_PWE_UNSPECIFIED,
    NL80211_SAE_PWE_HUNT_AND_PECK,
    NL80211_SAE_PWE_HASH_TO_ELEMENT,
    NL80211_SAE_PWE_BOTH,
};

enum nl80211_hidden_ssid {
    NL80211_HIDDEN_SSID_NOT_IN_USE,
    NL80211_HIDDEN_SSID_ZERO_LEN,
    NL80211_HIDDEN_SSID_ZERO_CONTENTS
};

/*  enum nl80211_mfp - Management frame protection state
 * @NL80211_MFP_NO: Management frame protection not used
 * @NL80211_MFP_REQUIRED: Management frame protection required */
typedef enum nl80211_mfp {
    NL80211_MFP_NO,
    NL80211_MFP_REQUIRED,
} oal_nl80211_mfp_enum;
typedef td_u8 oal_nl80211_mfp_enum_uint8;

enum nl80211_acl_policy {
    NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED,
    NL80211_ACL_POLICY_DENY_UNLESS_LISTED,
};

enum wiphy_flags {
    WIPHY_FLAG_CUSTOM_REGULATORY        = bit(0),
    WIPHY_FLAG_STRICT_REGULATORY        = bit(1),
    WIPHY_FLAG_DISABLE_BEACON_HINTS     = bit(2),
    WIPHY_FLAG_NETNS_OK                 = bit(3),
    WIPHY_FLAG_PS_ON_BY_DEFAULT         = bit(4),
    WIPHY_FLAG_4ADDR_AP                 = bit(5),
    WIPHY_FLAG_4ADDR_STATION            = bit(6),
    WIPHY_FLAG_CONTROL_PORT_PROTOCOL    = bit(7),
    WIPHY_FLAG_IBSS_RSN                 = bit(8),
    WIPHY_FLAG_MESH_AUTH                = bit(10),
    WIPHY_FLAG_SUPPORTS_SCHED_SCAN      = bit(11),
    /* use hole at 12 */
    WIPHY_FLAG_SUPPORTS_FW_ROAM         = bit(13),
    WIPHY_FLAG_AP_UAPSD                 = bit(14),
    WIPHY_FLAG_SUPPORTS_TDLS            = bit(15),
    WIPHY_FLAG_TDLS_EXTERNAL_SETUP      = bit(16),
    WIPHY_FLAG_HAVE_AP_SME              = bit(17),
    WIPHY_FLAG_REPORTS_OBSS             = bit(18),
    WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD    = bit(19),
    WIPHY_FLAG_OFFCHAN_TX               = bit(20),
    WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL    = bit(21),
};

enum nl80211_mesh_power_mode {
    NL80211_MESH_POWER_UNKNOWN,
    NL80211_MESH_POWER_ACTIVE,
    NL80211_MESH_POWER_LIGHT_SLEEP,
    NL80211_MESH_POWER_DEEP_SLEEP,

    __NL80211_MESH_POWER_AFTER_LAST,
    NL80211_MESH_POWER_MAX              = __NL80211_MESH_POWER_AFTER_LAST - 1
};

enum rate_info_flags {
    RATE_INFO_FLAGS_MCS              = bit(0),
    RATE_INFO_FLAGS_VHT_MCS          = bit(1),
    RATE_INFO_FLAGS_40_MHZ_WIDTH     = bit(2),
    RATE_INFO_FLAGS_80_MHZ_WIDTH     = bit(3),
    RATE_INFO_FLAGS_80P80_MHZ_WIDTH  = bit(4),
    RATE_INFO_FLAGS_160_MHZ_WIDTH    = bit(5),
    RATE_INFO_FLAGS_SHORT_GI         = bit(6),
    RATE_INFO_FLAGS_60G              = bit(7),
};

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
    td_u32  handle;
} oal_qdisc_stru;

/* iw_handler_def结构体封装 */
typedef struct {
    td_u16       cmd;        /* Wireless Extension command */
    td_u16       flags;      /* More to come ;-) */
} oal_iw_request_info_stru;

typedef struct {
    td_void*      pointer;       /* Pointer to the data  (in user space) */
    td_u16    length;         /* number of fields or size in bytes */
    td_u16    flags;          /* Optional params */
} oal_iw_point_stru;

typedef struct {
    td_s32     value;      /* The value of the parameter itself */
    td_u8      fixed;      /* Hardware should not use auto select */
    td_u8      disabled;   /* Disable the feature */
    td_u16     flags;      /* Various specifc flags (if any) */
} oal_iw_param_stru;

typedef struct {
    td_s32       m;       /* Mantissa */
    td_s16       e;       /* Exponent */
    td_u8        i;       /* List index (when in range struct) */
    td_u8        flags;   /* Flags (fixed/auto) */
} oal_iw_freq_stru;

typedef struct {
    td_u8        qual;       /* link quality (%retries, SNR, %missed beacons or better...) */
    td_u8        level;      /* signal level (dBm) */
    td_u8        noise;      /* noise level (dBm) */
    td_u8        updated;    /* Flags to know if updated */
} oal_iw_quality_stru;

typedef struct {
    td_u16       sa_family;      /* address family, AF_xxx   */
    td_u8        sa_data[14];    /* 14 bytes of protocol address */
} oal_sockaddr_stru;

typedef struct netif                        oal_lwip_netif;
struct ieee80211_mcs_info {
    td_u8   rx_mask[IEEE80211_HT_MCS_MASK_LEN];
    td_u16  rx_highest;
    td_u8   tx_params;
    td_u8   reserved[3];  /* 保留3字节对齐 */
};

struct ieee80211_freq_range {
    td_u32 start_freq_khz;
    td_u32 end_freq_khz;
    td_u32 max_bandwidth_khz;
};

struct ieee80211_power_rule {
    td_u32 max_antenna_gain;
    td_u32 max_eirp;
};

struct ieee80211_reg_rule {
    struct ieee80211_freq_range freq_range;
    struct ieee80211_power_rule power_rule;
    td_u32 flags;
};

struct ieee80211_msrment_ie {
    td_u8 token;
    td_u8 mode;
    td_u8 type;
    td_u8 request[0];
};

struct ieee80211_ext_chansw_ie {
    td_u8 mode;
    td_u8 new_operating_class;
    td_u8 new_ch_num;
    td_u8 count;
};

struct callback_head {
    struct callback_head *next;
    td_void (*func)(struct callback_head *head);
};

#define RCU_HEAD callback_head

struct ieee80211_regdomain {
    struct RCU_HEAD RCU_HEAD;
    td_u32 n_reg_rules;
    td_char alpha2[2]; /* 国家码2字符 */
    td_u8 dfs_region;
    struct ieee80211_reg_rule reg_rules[];
};

typedef struct ieee80211_regdomain          oal_ieee80211_regdomain_stru;

typedef struct oal_cfg80211_ssid_tag {
    td_u8   ssid[OAL_IEEE80211_MAX_SSID_LEN];
    td_u8   ssid_len;
    td_u8   arry[3];  /* 3: SIZE(0..3) */
} oal_cfg80211_ssid_stru;

/* 原结构体hmac和dmac不一致 以hmac为准 */
typedef struct ieee80211_channel {
    oal_ieee80211_band_enum_uint8 band;
    td_u16          center_freq;
    td_u16          hw_value;
    td_u32          flags;
    td_s32          max_antenna_gain;
    td_s32          max_power;
    td_bool         beacon_found;
    td_u8           resv[3];  /* 3 字节对齐 */
    td_u32          orig_flags;
    td_s32          orig_mag;
    td_s32          orig_mpwr;
} oal_ieee80211_channel_stru;
typedef struct ieee80211_channel            oal_ieee80211_channel;

typedef struct oal_cfg80211_crypto_settings_tag {
    td_u32              wpa_versions;
    td_u32              cipher_group;
    td_s32              n_ciphers_pairwise;
    td_u32              ciphers_pairwise[OAL_NL80211_MAX_NR_CIPHER_SUITES];
    td_s32              n_akm_suites;
    td_u32              akm_suites[OAL_NL80211_MAX_NR_AKM_SUITES];
    enum nl80211_sae_pwe_mechanism sae_pwe;

    td_u8               control_port;
    td_u8               arry[3];  /* 3: SIZE(0..3) */
} oal_cfg80211_crypto_settings_stru;

typedef struct cfg80211_add_key_info_stru {
    td_u8    key_index;
    td_bool  pairwise;
}cfg80211_add_key_info_stru;

typedef struct cfg80211_get_key_info_stru {
    td_u8 key_index;
    td_u8 arsvd[2];  /* 2: 保留字段 */
    bool pairwise;
    const td_u8 *mac_addr;
}cfg80211_get_key_info_stru;

/* net_device ioctl结构体定义 */
/* hostapd/wpa_supplicant 下发的信息元素结构 */
/* 该结构为事件内存池大小，保存从hostapd/wpa_supplicant下发的ie 信息 */
/* 注意: 整个结构体长度为事件内存池大小，如果事件内存池有修改，则需要同步修改app 数据结构 */
struct oal_app_ie {
    osal_u32              ie_len;
    app_ie_type_uint8    app_ie_type;
    osal_u8               ie_type_bitmap;
    osal_u8               rsv[2];     /* 保留2字节对齐 */
    /* ie 中保存信息元素，长度 = (事件内存池大小 - 保留长度) */
    osal_u8               ie[WLAN_WPS_IE_MAX_SIZE];
};
typedef struct oal_app_ie oal_app_ie_stru;

typedef struct {
    td_u32  fake;
} oal_iw_statistics_stru;

typedef union {
    /* Config - generic */
    td_char             name[OAL_IF_NAME_SIZE];
    oal_iw_point_stru   essid;      /* Extended network name */
    oal_iw_param_stru   nwid;       /* network id (or domain - the cell) */
    oal_iw_freq_stru    freq;       /* frequency or channel : * 0-1000 = channel * > 1000 = frequency in Hz */
    oal_iw_param_stru   sens;       /* signal level threshold */
    oal_iw_param_stru   bitrate;    /* default bit rate */
    oal_iw_param_stru   txpower;    /* default transmit power */
    oal_iw_param_stru   rts;        /* RTS threshold threshold */
    oal_iw_param_stru   frag;       /* Fragmentation threshold */
    td_u32              mode;       /* Operation mode */
    oal_iw_param_stru   retry;      /* Retry limits & lifetime */
    oal_iw_point_stru   encoding;   /* Encoding stuff : tokens */
    oal_iw_param_stru   power;      /* PM duration/timeout */
    oal_iw_quality_stru qual;       /* Quality part of statistics */
    oal_sockaddr_stru   ap_addr;    /* Access point address */
    oal_sockaddr_stru   addr;       /* Destination address (hw/mac) */
    oal_iw_param_stru   param;      /* Other small parameters */
    oal_iw_point_stru   data;       /* Other large parameters */
} oal_iwreq_data_union;

struct oal_net_device;
typedef td_u32 (*oal_iw_handler)(struct oal_net_device* dev, oal_iw_request_info_stru* info,
    oal_iwreq_data_union* wrqu, td_s8* extra);

/* 私有IOCTL接口信息 */
typedef struct {
    td_u32       cmd;                       /* ioctl命令号 */
    td_u16       set_args;                  /* 类型和参数字符个数 */
    td_u16       get_args;                  /* 类型和参数字符个数 */
    td_char      name[OAL_IF_NAME_SIZE];    /* 私有命令名 */
} oal_iw_priv_args_stru;

typedef struct {
    const oal_iw_handler*    standard;
    td_u16                   num_standard;
    td_u16                   num_private;

    /* FangBaoshun For wrl31 Compile */
    const oal_iw_handler*    private;
    /* FangBaoshun For wrl31 Compile */
    td_u8                       auc_resv[2]; /* 2: bytes保留字段 */
    td_u16                      num_private_args;

    const oal_iw_handler*        private_win32;

    const oal_iw_priv_args_stru* private_args;
    oal_iw_statistics_stru*     (*get_wireless_stats)(struct oal_net_device* dev);
} oal_iw_handler_def_stru;

typedef struct _oal_ext_eapol_stru {
    td_bool                 register_code;
    td_u8                   eapol_cnt;    /* 当前链表中eapol数量 */
    td_u16                  enqueue_time; /* 记录的eapol帧最早入队时间 单位s 用于超时老化 */
    td_void*                context;
    td_void                 (*notify_callback)(td_void* name, td_void* context);
    oal_netbuf_head_stru    eapol_skb_head;
} oal_ext_eapol_stru;

typedef struct {
    td_u32   rx_packets;     /* total packets received   */
    td_u32   tx_packets;     /* total packets transmitted    */
    td_u32   rx_bytes;       /* total bytes received     */
    td_u32   tx_bytes;       /* total bytes transmitted  */
    td_u32   rx_errors;      /* bad packets received     */
    td_u32   tx_errors;      /* packet transmit problems */
    td_u32   rx_dropped;     /* no space in linux buffers    */
    td_u32   tx_dropped;     /* no space available in linux  */
    td_u32   multicast;      /* multicast packets received   */
    td_u32   collisions;

    /* detailed rx_errors: */
    td_u32   rx_length_errors;
    td_u32   rx_over_errors;     /* receiver ring buff overflow  */
    td_u32   rx_crc_errors;      /* recved pkt with crc error    */
    td_u32   rx_frame_errors;    /* recv'd frame alignment error */
    td_u32   rx_fifo_errors;     /* recv'r fifo overrun      */
    td_u32   rx_missed_errors;   /* receiver missed packet   */

    /* detailed tx_errors */
    td_u32   tx_aborted_errors;
    td_u32   tx_carrier_errors;
    td_u32   tx_fifo_errors;
    td_u32   tx_heartbeat_errors;
    td_u32   tx_window_errors;

    /* for cslip etc */
    td_u32   rx_compressed;
    td_u32   tx_compressed;
} oal_net_device_stats_stru;

/* 原hmac和dmac结构体不一致 以HMAC为准 */
typedef struct oal_net_device {
    td_char                      name[OAL_IF_NAME_SIZE];
    td_void*                     ml_priv;
    struct oal_net_device_ops*   netdev_ops;
    td_u32                      last_rx;
    td_u32                      flags;
    oal_iw_handler_def_stru*     wireless_handlers;
    td_u8                       dev_addr[ETHER_ADDR_LEN];
    td_u8                       addr_idx;
    td_u8                       resv;
    td_s32                      tx_queue_len;
    td_u16                      hard_header_len;
    td_u16                      type;
    td_u16                      needed_headroom;
    td_u16                      needed_tailroom;
    struct oal_net_device*       master;
    struct wireless_dev*         ieee80211_ptr;
    oal_qdisc_stru*              qdisc;
    td_u8*                       ifalias;
    td_u8                       addr_len;
    td_u8                       resv2[3];  /* 3: bytes保留字段 */
    td_s32                      watchdog_timeo;
    oal_net_device_stats_stru   stats;
    td_u32                      mtu;
    td_void                     (*destructor)(struct oal_net_device*);
    td_void*                     priv;
    td_u32                      num_tx_queues;
    oal_ext_eapol_stru          ext_eapol;   /* EAPOL报文收发数据结构 */
    oal_lwip_netif*              lwip_netif; /* LWIP协议栈数据结构 */
#ifdef _PRE_WLAN_FEATURE_VLWIP
    oal_lwip_netif*              vlwip_netif;
#endif
} oal_net_device_stru;

typedef struct oal_net_notify {
    td_u32 ip_addr;
    td_u32 notify_type;
} oal_net_notify_stru;

// 适配HW_LITEOS_OPEN_VERSION_NUM >= kernel_version(1,4,2))
typedef struct ifreq oal_ifreq_stru;

typedef struct oal_net_device_ops {
    td_s32                      (*ndo_init)(oal_net_device_stru*);
    td_s32                      (*ndo_open)(struct oal_net_device*);
    td_s32                      (*ndo_stop)(struct oal_net_device*);
    oal_net_dev_tx_enum         (*ndo_start_xmit) (oal_netbuf_stru*, struct oal_net_device*);
    td_void                     (*ndo_set_multicast_list)(struct oal_net_device*);
    oal_net_device_stats_stru*  (*ndo_get_stats)(oal_net_device_stru*);
    td_s32                      (*ndo_do_ioctl)(struct oal_net_device*, oal_ifreq_stru*, td_s32);
    td_s32                      (*ndo_change_mtu)(struct oal_net_device*, td_s32);
    td_s32                      (*ndo_set_mac_address)(struct oal_net_device*, oal_sockaddr_stru*);
    td_s16                      (*ndo_select_queue)(oal_net_device_stru* dev, oal_netbuf_stru*);
    td_s32                      (*ndo_netif_notify)(oal_net_device_stru*, oal_net_notify_stru*);
} oal_net_device_ops_stru;

/* 此结构体成员命名是为了保持跟linux一致 */
typedef struct ieee80211_rate {
    td_u32 flags;
    td_u16 bitrate;
    td_u16 hw_value;
    td_u16 hw_value_short;
    td_u8  rsv[2];   /* 2: bytes保留字段 */
} oal_ieee80211_rate;

typedef struct ieee80211_sta_ht_cap {
    td_u16          cap;                    /* use IEEE80211_HT_CAP_ */
    td_u8           ht_supported;
    td_u8           ampdu_factor;
    td_u8           ampdu_density;
    td_u8           rsv[3];  /* 3: bytes保留字段 */
    struct          ieee80211_mcs_info mcs;
} oal_ieee80211_sta_ht_cap;

typedef struct ieee80211_supported_band {
    oal_ieee80211_channel*       channels;
    oal_ieee80211_rate*          bitrates;
    oal_ieee80211_band_enum_uint8         band;
    td_s32                      n_channels;
    td_s32                      n_bitrates;
    oal_ieee80211_sta_ht_cap    ht_cap;
} oal_ieee80211_supported_band;

typedef struct oal_wiphy_tag {
    oal_ieee80211_supported_band*               bands[IEEE80211_NUM_BANDS];
    td_u8                                       priv[4];       /* 4: SIZE(0..4) */
} oal_wiphy_stru;

typedef struct cfg80211_chan_def {
    oal_ieee80211_channel*   chan;
    oal_nl80211_channel_type width;
    td_s32                center_freq1;
    td_s32                center_freq2;
} oal_cfg80211_chan_def;

typedef struct cfg80211_pmksa               oal_cfg80211_pmksa_stru;

typedef struct oal_key_params_tag {
    td_u8*   key;
    td_u8*   seq;
    td_s32  key_len;
    td_s32  seq_len;
    td_u32  cipher;
} oal_key_params_stru;

typedef struct oal_cfg80211_scan_request_tag {
    oal_cfg80211_ssid_stru*        ssids;
    td_u32                         n_ssids;
    td_u32                         n_channels;
    td_u32                         ie_len;

    /* internal */
    oal_net_device_stru*           dev;
    struct wireless_dev*           wdev;

    td_u8                          aborted;
    td_u8                          prefix_ssid_scan_flag;
#if defined(_PRE_WLAN_FEATURE_WS92_MERGE) && defined(_PRE_WLAN_FEATURE_ACS)
    td_u8                          acs_scan_flag;
    td_u8                          resv;
#else
    td_u8                          resv[2];  /* 2: 锟斤拷锟斤拷锟街讹拷 */
#endif

    td_u8*                         ie;
    /* keep last */
    oal_ieee80211_channel_stru*    channels[OAL_MAX_SCAN_CHANNELS];
} oal_cfg80211_scan_request_stru;

typedef struct cfg80211_sched_scan_request {
    struct cfg80211_ssid*        ssids;
    td_s32                       n_ssids;
    td_u32                       n_channels;
    td_u32                       interval;
    const td_u8*                 ie;
    size_t                       ie_len;
    td_u32                       flags;
    struct cfg80211_match_set*   match_sets;
    td_s32                       n_match_sets;
    td_s32                       min_rssi_thold;
    td_s32                       rssi_thold; /* just for backward compatible */

    /* internal */
    struct wiphy*                wiphy;
    struct oal_net_device_stru*  dev;
    td_u32                       scan_start;
    /* keep last */
    struct ieee80211_channel*    channels[0];
} oal_cfg80211_sched_scan_request_stru;

typedef struct oal_cfg80211_connect_params_tag {
    oal_ieee80211_channel_stru*         channel;

    td_u8*                              bssid;
    td_u8*                              ssid;
    td_u8*                              ie;

    td_u32                              ssid_len;
    td_u32                              ie_len;
    oal_cfg80211_crypto_settings_stru   crypto;
    const td_u8*                        key;

    oal_nl80211_auth_type_enum_uint8    auth_type;
    td_u8                               privacy;
    td_u8                               key_len;
    td_u8                               key_idx;
    oal_nl80211_mfp_enum_uint8          mfp;
    td_u8                               auc_resv[3];  /* 3: 閿熸枻鎷烽敓鏂ゆ嫹閿熻鏂ゆ嫹 */
} oal_cfg80211_connect_params_stru;

typedef struct ieee80211_mgmt {
    td_u16 frame_control;
    td_u16 duration;
    td_u8 da[6];    /* 6: SIZE(0..6) */
    td_u8 sa[6];    /* 6: SIZE(0..6) */
    td_u8 bssid[6]; /* 6: SIZE(0..6) */
    td_u16 seq_ctrl;
    union {
        struct {
            td_u16 auth_alg;
            td_u16 auth_transaction;
            td_u16 status_code;
            /* possibly followed by Challenge text */
            td_u8 variable[0];
        }  auth;
        struct {
            td_u16 reason_code;
        }  deauth;
        struct {
            td_u16 capab_info;
            td_u16 listen_interval;
            /* followed by SSID and Supported rates */
            td_u8 variable[0];
        }  assoc_req;
        struct {
            td_u16 capab_info;
            td_u16 status_code;
            td_u16 aid;
            /* followed by Supported rates */
            td_u8 variable[0];
        }  assoc_resp, reassoc_resp;
        struct {
            td_u16 capab_info;
            td_u16 listen_interval;
            td_u8 current_ap[6];  /* 6: SIZE(0..6) */
            /* followed by SSID and Supported rates */
            td_u8 variable[0];
        }  reassoc_req;
        struct {
            td_u16 reason_code;
        }  disassoc;
        struct {
            td_u64 timestamp;
            td_u16 beacon_int;
            td_u16 capab_info;
            /* followed by some of SSID, Supported rates,
             * FH Params, DS Params, CF Params, IBSS Params, TIM */
            td_u8 variable[4];  /* 4: FH Params, DS Params, CF Params, IBSS Params */
        }  beacon;
        struct {
            td_u64 timestamp;
            td_u16 beacon_int;
            td_u16 capab_info;
            /* followed by some of SSID, Supported rates,
             * FH Params, DS Params, CF Params, IBSS Params */
            td_u8 variable[4];  /* 4: FH Params, DS Params, CF Params, IBSS Params */
        }  probe_resp;
        struct {
            td_u8 category;
            union {
                struct {
                    td_u8 action_code;
                    td_u8 dialog_token;
                    td_u8 status_code;
                    td_u8 variable[0];
                }  wme_action;
                struct {
                    td_u8 action_code;
                    td_u8 variable[0];
                }  chan_switch;
                struct {
                    td_u8 action_code;
                    struct ieee80211_ext_chansw_ie data;
                    td_u8 variable[0];
                }  ext_chan_switch;
                struct {
                    td_u8 action_code;
                    td_u8 dialog_token;
                    td_u8 element_id;
                    td_u8 length;
                    struct ieee80211_msrment_ie msr_elem;
                }  measurement;
                struct {
                    td_u8 action_code;
                    td_u8 dialog_token;
                    td_u16 capab;
                    td_u16 timeout;
                    td_u16 start_seq_num;
                }  addba_req;
                struct {
                    td_u8 action_code;
                    td_u8 dialog_token;
                    td_u16 status;
                    td_u16 capab;
                    td_u16 timeout;
                }  addba_resp;
                struct {
                    td_u8 action_code;
                    td_u8 resv;
                    td_u16 params;
                    td_u16 reason_code;
                }  delba;
                struct {
                    td_u8 action_code;
                    td_u8 variable[0];
                }  self_prot;
                struct {
                    td_u8 action_code;
                    td_u8 variable[0];
                }  mesh_action;
                struct {
                    td_u8 action;
                    td_u8 trans_id[WLAN_SA_QUERY_TR_ID_LEN];
                }  sa_query;
                struct {
                    td_u8 action;
                    td_u8 smps_control;
                }  ht_smps;
                struct {
                    td_u8 action_code;
                    td_u8 chanwidth;
                }  ht_notify_cw;
                struct {
                    td_u8 action_code;
                    td_u8 dialog_token;
                    td_u16 capability;
                    td_u8 variable[0];
                }  tdls_discover_resp;
                struct {
                    td_u8 action_code;
                    td_u8 operating_mode;
                }  vht_opmode_notif;
            } u;
        }  action;
    } u;
} oal_ieee80211_mgmt_stru;

struct cfg80211_crypto_settings {
    td_u32 wpa_versions;
    td_u32 cipher_group;
    td_s32 n_ciphers_pairwise;
    td_u32 ciphers_pairwise[OAL_NL80211_MAX_NR_CIPHER_SUITES];
    td_s32 n_akm_suites;
    td_u32 akm_suites[OAL_NL80211_MAX_NR_AKM_SUITES];
    td_u16 control_port_ethertype;
    td_bool control_port;
    td_bool control_port_no_encrypt;
};

struct ieee80211_vht_mcs_info {
    td_u16 rx_mcs_map;
    td_u16 rx_highest;
    td_u16 tx_mcs_map;
    td_u16 tx_highest;
};

typedef struct cfg80211_bss                 oal_cfg80211_bss_stru;

typedef struct rate_info {
    td_u8    flags;
    td_u8    mcs;
    td_u16   legacy;
    td_u8    nss;
    td_u8    resv;
} oal_rate_info_stru;

typedef struct vif_params {
    td_s32        use_4addr;
    td_u8*        macaddr;
} oal_vif_params_stru;

typedef struct cfg80211_update_ft_ies_params {
    td_u16 md;
    const td_u8 *ie;
    size_t ie_len;
} oal_cfg80211_update_ft_ies_stru;

typedef struct cfg80211_ft_event_params {
    td_u8 *ies;
    size_t ies_len;
    td_u8 target_ap[OAL_MAC_ADDR_LEN];
    td_u8 resv[2];
    const td_u8 *ric_ies;
    size_t ric_ies_len;
} oal_cfg80211_ft_event_stru;

/* To be implement! */
typedef struct cfg80211_beacon_data {
    const td_u8 *head;
    const td_u8 *tail;
    const td_u8 *beacon_ies;
    const td_u8 *proberesp_ies;
    const td_u8 *assocresp_ies;
    const td_u8 *probe_resp;

    size_t head_len, tail_len;
    size_t beacon_ies_len;
    size_t proberesp_ies_len;
    size_t assocresp_ies_len;
    size_t probe_resp_len;
} oal_beacon_data_stru;

struct oal_mac_address {
    td_u8 addr[OAL_MAC_ADDR_LEN];
};

struct cfg80211_acl_data {
    enum nl80211_acl_policy acl_policy;
    td_s32 n_acl_entries;

    /* Keep it last */
    struct oal_mac_address mac_addrs[];
};

typedef struct cfg80211_ap_settings {
    struct cfg80211_chan_def        chandef;
    struct cfg80211_beacon_data     beacon;

    td_s32                          beacon_interval;
    td_s32                          dtim_period;
    const td_u8*                    ssid;
    size_t                          ssid_len;

    enum nl80211_hidden_ssid        hidden_ssid;
    enum nl80211_auth_type          auth_type;
    enum nl80211_sae_pwe_mechanism  sae_pwe;
    struct cfg80211_crypto_settings crypto;

    td_s32                          inactivity_timeout;
    td_bool                         privacy;
    td_bool                         p2p_opp_ps;
    td_u8                           p2p_ctwindow;
    td_bool                         radar_required;

    const struct cfg80211_acl_data* acl;
} oal_ap_settings_stru;

typedef struct bss_parameters {
    td_s32 use_cts_prot;
    td_s32 use_short_preamble;
    td_s32 use_short_slot_time;
    const td_u8 *basic_rates;
    td_u8 basic_rates_len;
    td_s32 ap_isolate;
    td_s32 ht_opmode;
    td_s8 p2p_ctwindow;
    td_s8 p2p_opp_ps;
} oal_bss_parameters;

struct beacon_parameters {
    td_u8 *head, *tail;
    td_s32 interval, dtim_period;
    td_s32 head_len, tail_len;
};
typedef struct beacon_parameters            oal_beacon_parameters;
typedef struct ieee80211_mgmt               oal_ieee80211_mgmt;

/* dmac未使用 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#ifndef _PRE_WLAN_FEATURE_MFG_ONLY
typedef ip4_addr_t                          oal_ip_addr_t;
#if LWIP_API_MESH
typedef linklayer_event_ap_conn_t           oal_event_ap_conn_stru;
#endif
#endif
#endif


typedef struct {
    td_s32         sk_wmem_queued;
} oal_sock_stru;

struct sta_bss_parameters {
    td_u8  flags;
    td_u8  dtim_period;
    td_u16 beacon_interval;
};

struct nl80211_sta_flag_update {
    td_u32 mask;
    td_u32 set;
};

typedef struct oal_station_info_tag {
    td_u32                          filled;
    const td_u8*                    assoc_req_ies;
    td_u32                          assoc_req_ies_len;
} oal_station_info_stru;

typedef struct module            oal_module_stru;

typedef struct ieee80211_iface_limit {
    td_u16 max;
    td_u16 types;
} oal_ieee80211_iface_limit;

typedef struct ieee80211_iface_combination {
    const struct ieee80211_iface_limit*  limits;
    td_u32                               num_different_channels;
    td_u16                               max_interfaces;
    td_u8                                n_limits;
    td_u8                                radar_detect_widths;
    td_bool                              beacon_int_infra_match;
    td_u8                                resv[3]; /* 3: 娣囨繄鏆€鐎涙顔? */
} oal_ieee80211_iface_combination;

typedef struct wireless_dev {
    struct oal_net_device*       netdev;
    oal_wiphy_stru*              wiphy;
    nl80211_iftype_uint8         iftype;
    td_u8                        resv[3]; /* reserve 3byte */
    /* 新内核新增字段 */
    oal_cfg80211_chan_def        preset_chandef;
} oal_wireless_dev;

typedef oal_wireless_dev oal_wireless_dev_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
typedef struct oal_cfg80211_ops_tag {
    td_s32 (*add_key)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev,
                      cfg80211_add_key_info_stru *p_cfg80211_add_key_info,
                      const td_u8* mac_addr, oal_key_params_stru* params);
    td_s32 (*get_key)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev,
                      cfg80211_get_key_info_stru *p_cfg80211_get_key_info, td_void* cookie,
                      td_void (*callback)(td_void* cookie, oal_key_params_stru* key));
    td_s32 (*del_key)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8 key_index, td_bool pairwise,
                      const td_u8* mac_addr);
    td_s32 (*set_default_key)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8 key_index, td_bool unicast,
                              td_bool multicast);
    td_s32 (*set_default_mgmt_key)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8 key_index);
    td_s32 (*scan)(oal_wiphy_stru* wiphy,  oal_cfg80211_scan_request_stru* request);
    td_s32 (*connect)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev,
                      oal_cfg80211_connect_params_stru* sme);
    td_s32 (*disconnect)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u16 reason_code);
    td_s32 (*set_channel)(oal_wiphy_stru* wiphy, oal_ieee80211_channel* chan,
                          oal_nl80211_channel_type channel_type);
    td_s32 (*set_wiphy_params)(oal_wiphy_stru* wiphy, td_u32 changed);
    td_s32 (*add_beacon)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, oal_beacon_parameters* info);
    td_s32 (*change_virtual_intf)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, nl80211_iftype_uint8 type,
                                  td_u32* pul_flags, oal_vif_params_stru* params);
    td_s32 (*add_station)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8* mac_addr, td_void* sta_parms);
    td_s32 (*del_station)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, const td_u8* mac_addr);
    td_s32 (*change_station)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8* mac_addr,
                             td_void* sta_parms);
    td_s32 (*get_station)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev, td_u8* mac_addr,
                          oal_station_info_stru* sta_info);
    td_s32 (*dump_station)(oal_wiphy_stru* wiphy, oal_net_device_stru* dev, td_s32 idx, td_u8* mac,
                           oal_station_info_stru* sta_info);
    td_s32 (*change_beacon)(oal_wiphy_stru*  wiphy, oal_net_device_stru* netdev,
                            oal_beacon_data_stru* beacon_info);
    td_s32 (*start_ap)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev,
                       oal_ap_settings_stru* ap_settings);
    td_s32 (*stop_ap)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev);
    td_s32 (*change_bss)(oal_wiphy_stru* wiphy, oal_net_device_stru* netdev,
                         td_void* bss_params);
    td_s32 (*set_power_mgmt)(oal_wiphy_stru*  wiphy, oal_net_device_stru* ndev, td_bool  enabled,
                             td_s32 timeout);
    td_s32 (*sched_scan_start)(oal_wiphy_stru* wiphy, oal_net_device_stru* dev,
                               struct cfg80211_sched_scan_request* request);
    td_s32 (*sched_scan_stop)(oal_wiphy_stru* wiphy, oal_net_device_stru* dev);
    td_s32 (*remain_on_channel)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev, struct ieee80211_channel* chan,
                                td_u32 duration, td_u64* cookie);
    td_s32 (*cancel_remain_on_channel)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev, td_u64 cookie);
    td_s32 (*mgmt_tx)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev, struct ieee80211_channel* chan,
                      td_bool offchan, td_u32 wait, const td_u8* buf, size_t len, td_bool no_cck,
                      td_bool dont_wait_for_ack, td_u64* cookie);
    td_void (*mgmt_frame_register)(struct wiphy* wiphy, struct wireless_dev* wdev, td_u16 frame_type, td_bool reg);
    struct wireless_dev* (*add_virtual_intf)(oal_wiphy_stru* wiphy, const td_char* name, nl80211_iftype_uint8 type,
            td_u32* flags, struct vif_params* params);
    td_s32 (*del_virtual_intf)(oal_wiphy_stru* wiphy, struct  wireless_dev* wdev);
    td_s32 (*mgmt_tx_cancel_wait)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev, td_u64 cookie);
    td_s32 (*start_p2p_device)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev);
    td_void (*stop_p2p_device)(oal_wiphy_stru* wiphy, struct wireless_dev* wdev);
} oal_cfg80211_ops;

#endif

#define MAX_BLACKLIST_NUM          128

typedef struct {
    osal_u8       blkwhtlst_cnt;                                       /* 黑白名单个数 */
    osal_u8       mode;                                                /* 黑白名单模式 */
    osal_u8       auc_resv[2];
    osal_u8       blkwhtlst_mac[MAX_BLACKLIST_NUM][OAL_MAC_ADDR_LEN];
} oal_blkwhtlst_stru;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0))
#define OAL_IEEE80211_BAND_2GHZ    NL80211_BAND_2GHZ
#define OAL_IEEE80211_BAND_5GHZ    NL80211_BAND_5GHZ
#define OAL_IEEE80211_NUM_BANDS    NUM_NL80211_BANDS
#else
#define OAL_IEEE80211_BAND_2GHZ    IEEE80211_BAND_2GHZ
#define OAL_IEEE80211_BAND_5GHZ    IEEE80211_BAND_5GHZ
#define OAL_IEEE80211_NUM_BANDS    IEEE80211_NUM_BANDS
#endif  /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)) */

typedef td_u8 oal_ieee80211_band_enum_uint8;
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* ================================================================ */
/* 以下结构体copy from hmac oal_net.h 适配dmac编译
 * 原dmac头文件linux也使用自定义的，未引用linux定义。
 */

/* linux 结构体 */
typedef struct iw_statistics                oal_iw_statistics_stru;
typedef struct net_device                   oal_net_device_stru;
typedef struct net_device_ops               oal_net_device_ops_stru;
typedef struct ethtool_ops                  oal_ethtool_ops_stru;
typedef struct iw_priv_args                 oal_iw_priv_args_stru;
typedef struct iw_handler_def               oal_iw_handler_def_stru;
typedef struct iw_point                     oal_iw_point_stru;
typedef struct iw_param                     oal_iw_param_stru;
typedef struct iw_freq                      oal_iw_freq_stru;
typedef struct iw_request_info              oal_iw_request_info_stru;
typedef struct rtnl_link_ops                oal_rtnl_link_ops_stru;
typedef struct sockaddr                     oal_sockaddr_stru;
typedef struct net_device_stats             oal_net_device_stats_stru;
typedef struct ifreq                        oal_ifreq_stru;
typedef struct Qdisc                        oal_qdisc_stru;
typedef struct vlan_ethhdr                  oal_vlan_ethhdr_stru;
typedef struct wiphy                        oal_wiphy_stru;
typedef struct wireless_dev                 oal_wireless_dev_stru;
typedef struct cfg80211_ops                 oal_cfg80211_ops_stru;
typedef struct wiphy_vendor_command         oal_wiphy_vendor_command_stru;
typedef struct nl80211_vendor_cmd_info      oal_nl80211_vendor_cmd_info_stru;

typedef struct kobj_uevent_env              oal_kobj_uevent_env_stru;
typedef struct iw_quality                   oal_iw_quality_stru;
typedef struct cfg80211_gtk_rekey_data      oal_cfg80211_gtk_rekey_data;
typedef union  iwreq_data                   oal_iwreq_data_union;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
typedef struct cfg80211_pmksa               oal_cfg80211_pmksa_stru;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
typedef struct survey_info                  oal_survey_info_stru;
#endif

typedef struct key_params                   oal_key_params_stru;

typedef struct cfg80211_scan_request        oal_cfg80211_scan_request_stru;
typedef struct cfg80211_ssid                oal_cfg80211_ssid_stru;
typedef struct cfg80211_sched_scan_request  oal_cfg80211_sched_scan_request_stru;
/* linux-2.6.34内核才有以下两个结构体，加密相关 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
typedef enum nl80211_mfp oal_nl80211_mfp_enum_uint8;
typedef struct cfg80211_connect_params      oal_cfg80211_conn_stru;
typedef struct cfg80211_crypto_settings     oal_cfg80211_crypto_settings_stru;
#else
/* linux-2.6.30内核没有这两个加密相关的结构体，命令下发上报也是内核透传，这里打桩 */
typedef struct  cfg80211_crypto_settings {
    td_u32              wpa_versions;
    td_u32              cipher_group;
    td_s32               n_ciphers_pairwise;
    td_u32              ciphers_pairwise[OAL_NL80211_MAX_NR_CIPHER_SUITES];
    td_s32               n_akm_suites;
    td_u32              akm_suites[OAL_NL80211_MAX_NR_AKM_SUITES];

    oal_bool_enum_uint8     control_port;
    td_u8               arry[3];
}oal_cfg80211_crypto_settings_stru;

#endif

typedef struct ieee80211_mgmt               oal_ieee80211_mgmt_stru;
typedef struct ieee80211_channel            oal_ieee80211_channel_stru;
typedef struct cfg80211_bss                 oal_cfg80211_bss_stru;
typedef struct rate_info                    oal_rate_info_stru;
typedef struct station_info                 oal_station_info_stru;
typedef struct station_parameters           oal_station_parameters_stru;
typedef enum station_info_flags             oal_station_info_flags;

typedef struct nlattr                       oal_nlattr_stru;
typedef struct genl_family                  oal_genl_family_stru;
typedef struct genl_multicast_group         oal_genl_multicast_group_stru;
typedef struct cfg80211_registered_device   oal_cfg80211_registered_device_stru;

typedef struct ieee80211_rate               oal_ieee80211_rate;
typedef struct ieee80211_channel            oal_ieee80211_channel;
typedef struct ieee80211_supported_band     oal_ieee80211_supported_band;
typedef enum cfg80211_signal_type           oal_cfg80211_signal_type;
typedef enum nl80211_channel_type           oal_nl80211_channel_type;
typedef struct cfg80211_update_owe_info     oal_cfg80211_update_owe_info;

#ifdef CONTROLLER_CUSTOMIZATION
typedef struct cfg80211_update_p2p_ie       oal_cfg80211_update_p2p_ie;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
typedef enum wiphy_params_flags             oal_wiphy_params_flags;
typedef enum wiphy_flags                    oal_wiphy_flags;
#else
/* linux-2.6.30内核打桩一份wiphy_flags枚举 */
typedef enum wiphy_flags {
    WIPHY_FLAG_CUSTOM_REGULATORY    = BIT(0),
    WIPHY_FLAG_STRICT_REGULATORY    = BIT(1),
    WIPHY_FLAG_DISABLE_BEACON_HINTS = BIT(2),
    WIPHY_FLAG_NETNS_OK     = BIT(3),
    WIPHY_FLAG_PS_ON_BY_DEFAULT = BIT(4),
    WIPHY_FLAG_4ADDR_AP     = BIT(5),
    WIPHY_FLAG_4ADDR_STATION    = BIT(6),
    WIPHY_FLAG_HAVE_AP_SME      = BIT(17),
    WIPHY_FLAG_OFFCHAN_TX       = BIT(20),
    WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL	= BIT(21),
}oal_wiphy_flags;
#endif

typedef struct vif_params                   oal_vif_params_stru;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
typedef enum nl80211_key_type               oal_nl80211_key_type;
#else
enum nl80211_key_type {
    NL80211_KEYTYPE_GROUP,
    NL80211_KEYTYPE_PAIRWISE,
    NL80211_KEYTYPE_PEERKEY,
    NUM_NL80211_KEYTYPES
};
typedef enum nl80211_key_type oal_nl80211_key_type;
#endif

typedef struct ieee80211_sta_ht_cap         oal_ieee80211_sta_ht_cap;
typedef struct ieee80211_regdomain          oal_ieee80211_regdomain_stru;
typedef struct cfg80211_update_ft_ies_params    oal_cfg80211_update_ft_ies_stru;
typedef struct cfg80211_ft_event_params         oal_cfg80211_ft_event_stru;
typedef struct cfg80211_chan_def oal_cfg80211_chan_def;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
/* To be implement! */
typedef struct cfg80211_beacon_data         oal_beacon_data_stru;
typedef struct cfg80211_ap_settings         oal_ap_settings_stru;
typedef struct bss_parameters               oal_bss_parameters;
struct beacon_parameters {
    td_u8 *head, *tail;
    td_s32 interval, dtim_period;
    td_s32 head_len, tail_len;
};
typedef struct beacon_parameters            oal_beacon_parameters;
#else
typedef struct beacon_parameters            oal_beacon_parameters;
#endif
typedef struct ieee80211_channel_sw_ie      oal_ieee80211_channel_sw_ie;
typedef struct ieee80211_msrment_ie         oal_ieee80211_msrment_ie;
typedef struct ieee80211_mgmt               oal_ieee80211_mgmt;

/* ================================================================ */
/* 以下结构体copy from liteos 适配dmac编译 */

/* net_device ioctl结构体定义 */
/* hostapd/wpa_supplicant 下发的信息元素结构 */
/* 该结构为事件内存池大小，保存从hostapd/wpa_supplicant下发的ie 信息 */
/* 注意: 整个结构体长度为事件内存池大小，如果事件内存池有修改，则需要同步修改app 数据结构 */
struct oal_app_ie {
    osal_u32              ie_len;
    app_ie_type_uint8    app_ie_type;
    osal_u8               ie_type_bitmap;
    osal_u8               rsv[2];     /* 保留2字节对齐 */
    /* ie 中保存信息元素，长度 = (事件内存池大小 - 保留长度) */
    osal_u8               ie[WLAN_WPS_IE_MAX_SIZE];
};
typedef struct oal_app_ie oal_app_ie_stru;

typedef td_u8 oal_nl80211_auth_type_enum_uint8;

/* ================================================================ */
/* 以下dmac原始定义和linux冲突 待特性确认 */
#endif // (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

typedef struct {
    osal_u8           sta_mac[WLAN_MAC_ADDR_LEN];            /* 要获取的sta的mac */
    oal_bool_enum_uint8 support_11h;                           /* 保存sta是否支持11h，为出参 */
    osal_u8           reserve;
} oal_hilink_get_sta_11h_ability;

typedef struct {
    osal_u8           sta_mac[OAL_MAC_ADDR_LEN];            /* 要获取的sta的mac */
    oal_bool_enum_uint8 support_11r;                           /* 保存sta是否支持11r，为出参 */
    osal_u8           reserve;
} oal_hilink_get_sta_11r_ability;


struct hostap_sta_link_info {
    osal_u8  addr[OAL_MAC_ADDR_LEN];
    osal_u8  rx_rssi;     /* 0 ~ 100 ,0xff为无效值 */
    osal_u8  tx_pwr;      /* 0 ~ 100 */
    osal_u32 rx_rate;     /* avr nego rate in kpbs */
    osal_u32 tx_rate;     /* avr nego rate in kpbs */
    osal_u32 rx_minrate;  /* min nego rx rate in last duration, clean to 0 when app read over */
    osal_u32 rx_maxrate;  /* max nego rx rate in last duration, clean to 0 when app read over */
    osal_u32 tx_minrate;  /* min nego tx rate in last duration, clean to 0 when app read over */
    osal_u32 tx_maxrate;  /* max nego tx rate in last duration, clean to 0 when app read over */
    osal_u64 rx_bytes;
    osal_u64 tx_bytes;
    osal_u32 tx_frames_rty;  /* tx frame retry cnt */
    osal_u32 tx_frames_all;  /* tx total frame cnt */
    osal_u32 tx_frames_fail; /* tx fail */
    osal_u32 SNR;            /* snr */
};

struct hostap_all_sta_link_info {
    unsigned long   buf_cnt;        /* input: sta_info count provided  */
    unsigned long   sta_cnt;        /* outpu: how many sta really */
    unsigned int    cur_channel;
    struct hostap_sta_link_info  *sta_info; /* output */
};
typedef struct hostap_sta_link_info oal_net_sta_link_info_stru;
typedef struct hostap_all_sta_link_info oal_net_all_sta_link_info_stru;

/* sta包含增量信息结构体 */
struct hostap_sta_link_info_ext {
    osal_u8                       auth_st;             /* 认证状态 */
    oal_bool_enum_uint8             band;                /* 工作频段 */
    oal_bool_enum_uint8             wmm_switch;          /* wmm是否使能 */
    osal_u8                       ps_st;               /* 节能状态 */
    osal_u8                       sta_num;             /* 空间流数 */
    osal_u8                       work_mode;           /* 协议模式 */
    osal_s8                        c_noise;                /* 节点的噪声值 */
    osal_u8                       auc_resv[1];
    osal_u32                      associated_time;     /* 用户已连接的时长 */
};

struct hostap_all_sta_link_info_ext {
    unsigned long   buf_cnt;        /* input: sta_info count provided  */
    unsigned long   sta_cnt;        /* outpu: how many sta really */
    unsigned int    cur_channel;
    struct hostap_sta_link_info     *sta_info;
    struct hostap_sta_link_info_ext *sta_info_ext;
};
typedef struct hostap_sta_link_info_ext oal_net_sta_link_info_ext_stru;
typedef struct hostap_all_sta_link_info_ext oal_net_all_sta_link_info_ext_stru;


typedef struct {
    osal_u8 group_mac[WLAN_MAC_ADDR_LEN];
    osal_u8 sta_num;
    osal_u8 reserve;
    osal_u8 sta_mac[MAX_STA_NUM_OF_ONE_GROUP][WLAN_MAC_ADDR_LEN];
} oal_snoop_group_stru;

typedef struct {
    osal_u16              group_cnt;
    osal_u8               auc_resv[3];
    oal_snoop_group_stru    *buf;
} oal_snoop_all_group_stru;

/* net_device ioctl结构体定义 */
typedef struct oal_net_dev_ioctl_data_tag {
    osal_s32 l_cmd;                                  /* 命令号 */
    union {
        struct {
            osal_u8    mac[OAL_MAC_ADDR_LEN];
            osal_u8    rsv[2];
            osal_u32   buf_size;            /* 用户空间ie 缓冲大小 */
            osal_u8   *buf;                /* 用户空间ie 缓冲地址 */
        } assoc_req_ie;                           /* AP 模式，用于获取STA 关联请求ie 信息 */

        struct {
            osal_u32    auth_alg;
        } auth_params;

        struct {
            osal_u8    auc_country_code[4];
        } country_code;

        osal_u8     ssid[OAL_IEEE80211_MAX_SSID_LEN + 4];
        osal_u32    vap_max_user;

        struct {
            osal_s32    l_freq;
            osal_s32    l_channel;
            osal_s32    l_ht_enabled;
            osal_s32    l_sec_channel_offset;
            osal_s32    l_vht_enabled;
            osal_s32    l_center_freq1;
            osal_s32    l_center_freq2;
            osal_s32    l_bandwidth;
        } freq;

        oal_app_ie_stru  app_ie;          /* beacon ie,index 0; proberesp ie index 1; assocresp ie index 2 */

        struct {
            osal_s32                           l_freq;              /* ap所在频段，与linux-2.6.34内核中定义不同 */
            osal_u32                          ssid_len;            /* SSID 长度 */
            osal_u32                          ie_len;

            osal_u8                          *ie;
            OAL_CONST osal_u8                *ssid;               /* 期望关联的AP SSID  */
            OAL_CONST osal_u8                *bssid;              /* 期望关联的AP BSSID  */

            osal_u8                           privacy;             /* 是否加密标志 */
            oal_nl80211_auth_type_enum_uint8    auth_type;           /* 认证类型，OPEN or SHARE-KEY */

            osal_u8                           wep_key_len;         /* WEP KEY长度 */
            osal_u8                           wep_key_index;       /* WEP KEY索引 */
            OAL_CONST osal_u8                *wep_key;            /* WEP KEY密钥 */

            oal_cfg80211_crypto_settings_stru   crypto;              /* 密钥套件信息 */
        } cfg80211_connect_params;
        struct {
            osal_u8            mac[OAL_MAC_ADDR_LEN];
            osal_u16           reason_code;                        /* 去关联 reason code */
        } kick_user_params;

        oal_net_all_sta_link_info_stru all_sta_link_info;
        oal_hilink_get_sta_11h_ability      fbt_get_sta_11h_ability;
        osal_s32                l_frag;                                /* 分片门限值 */
        osal_s32                l_rts;                                 /* RTS 门限值 */

        oal_snoop_all_group_stru        all_snoop_group;
        oal_net_all_sta_link_info_ext_stru  all_sta_link_info_ext;
        oal_blkwhtlst_stru              blkwhtlst;
    } pri_data;
} oal_net_dev_ioctl_data_stru;

#endif /* end of file */

