/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: oal net interface
 * Create: 2020-05-02
 */

#ifndef __OAL_NET_HCM_H__
#define __OAL_NET_HCM_H__
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/version.h>
#include <net/iw_handler.h>
#include <linux/netdevice.h>
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "oal_mm.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "eth_ip_common_rom.h"
#endif
#include "oal_util.h"
#include "oal_skbuff.h"
#include "oal_netbuf_ext.h"
#include "oal_net_cfg80211.h"
// 在 oal_net.h 头部添加：
#include <linux/netdevice.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* xr util还未融合，OAL_DECLARE_PACKED暂时在此处重复定义 */
#define OAL_DECLARE_PACKED    __attribute__((__packed__))
#define OAL_NETBUF_DEFAULT_DATA_OFFSET 48  /* 5115上实际测得data比head大48，用于netbuf data指针复位 */

#define OAL_ASSOC_REQ_IE_OFFSET        28    /* 上报内核关联请求帧偏移量 */
#define OAL_ASSOC_RSP_IE_OFFSET        30    /* 上报内核关联响应帧偏移量 */
#define OAL_AUTH_IE_OFFSET             30
#define OAL_FT_ACTION_IE_OFFSET        40
#define OAL_ASSOC_RSP_FIXED_OFFSET     6     /* 关联响应帧帧长FIXED PARAMETERS偏移量 */
#define OAL_PMKID_LEN                  16
#define OAL_WPA_KEY_LEN                32
#define OAL_WPA_SEQ_LEN                16
#define OAL_WLAN_SA_QUERY_TR_ID_LEN    2
#define OAL_BITFIELD_LITTLE_ENDIAN      0
#define OAL_BITFIELD_BIG_ENDIAN         1
/*
 * Byte order/swapping support.
 */
#define OAL_LITTLE_ENDIAN    1234
#define OAL_BIG_ENDIAN       4321
#define OAL_BYTE_ORDER OAL_BIG_ENDIAN
#define OAL_BYTE_HALF_WIDTH  4

/* ICMP协议报文 */
#define MAC_ICMP_PROTOCAL 1
#define MAC_ICMP_REQUEST 0x08
#define MAC_ICMP_RESPONSE 0x00

/*****************************************************************************
  2.10 IP宏定义
*****************************************************************************/
#define IPV6_ADDR_MULTICAST    	0x0002U
#define IPV6_ADDR_UNICAST      	0x0001U
#define IPV6_ADDR_SCOPE_TYPE(scope)	((scope) << 16)
#define IPV6_ADDR_SCOPE_NODELOCAL	0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL	0x02
#define IPV6_ADDR_SCOPE_SITELOCAL	0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL	0x08
#define IPV6_ADDR_SCOPE_GLOBAL		0x0e
#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U
#define IPV6_ADDR_RESERVED  0x2000U /* reserved address space */

#define WLAN_DSCP_PRI_SHIFT         2
#define WLAN_IP_PRI_SHIFT               5
#define WLAN_IPV6_PRIORITY_MASK         0x0FF00000
#define WLAN_IPV6_PRIORITY_SHIFT        20

/* ICMP codes for neighbour discovery messages */
#define OAL_NDISC_ROUTER_SOLICITATION       133
#define OAL_NDISC_ROUTER_ADVERTISEMENT      134
#define OAL_NDISC_NEIGHBOUR_SOLICITATION    135
#define OAL_NDISC_NEIGHBOUR_ADVERTISEMENT   136
#define OAL_NDISC_REDIRECT                  137

#define OAL_ND_OPT_TARGET_LL_ADDR           2
#define OAL_ND_OPT_SOURCE_LL_ADDR           1
#define OAL_IPV6_ADDR_ANY                   0x0000U
#define OAL_IPV6_ADDR_MULTICAST             0x0002U
#define OAL_IPV6_MAC_ADDR_LEN               16

/*
 * support for ARP/ND offload. add 2015.6.12
 */
#define OAL_IPV4_ADDR_SIZE      4
#define OAL_IPV6_ADDR_SIZE      16
#define OAL_IPV6_ADDR16_SIZE                  8
#define OAL_IPV6_ADDR32_SIZE                  4
#define OAL_IP_ADDR_MAX_SIZE    OAL_IPV6_ADDR_SIZE

#define OAL_IP4_ADDR    IP4_ADDR

/* ip头到协议类型字段的偏移 */
#define IP_HDR_LEN               20

#define OAL_IPPROTO_UDP             17         /* User Datagram Protocot */
#define OAL_IPPROTO_ICMPV6          58         /* ICMPv6 */

/*****************************************************************************
  2.12 LLC SNAP宏定义
*****************************************************************************/
#define LLC_UI                  0x3
#define SNAP_LLC_FRAME_LEN      8
#define SNAP_LLC_LSAP           0xaa
#define SNAP_RFC1042_ORGCODE_0  0x00
#define SNAP_RFC1042_ORGCODE_1  0x00
#define SNAP_RFC1042_ORGCODE_2  0x00
#define SNAP_BTEP_ORGCODE_0     0x00
#define SNAP_BTEP_ORGCODE_1     0x00
#define SNAP_BTEP_ORGCODE_2     0xf8

/*****************************************************************************
  2.13 ETHER宏定义
*****************************************************************************/
#define IP6_ETHER_ADDR_LEN                16                /* efuse中ipv6 MAC地址大小 */

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define ETHER_ADDR_LEN  6   /* length of an Ethernet address */
#define ETHER_TYPE_LEN  2   /* length of the Ethernet type field */
#define ETHER_CRC_LEN   4   /* length of the Ethernet CRC */
#define ETHER_HDR_LEN                   14
#define ETHER_MAX_LEN                   1518
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define ETH_ALEN    6
#define ETH_HLEN    14
#define ETH_FRAME_LEN 1514
#ifndef ETHER_HDR_LEN
#define ETHER_HDR_LEN     ETH_HLEN
#endif
#ifndef ETHER_MAX_LEN
#define ETHER_MAX_LEN    (ETH_FRAME_LEN + ETHER_CRC_LEN)
#endif
#endif
#define ETHER_MTU        (ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

#define ETH_SENDER_IP_ADDR_LEN       4  /* length of an Ethernet send ip address */
#define ETH_TARGET_IP_ADDR_LEN       4  /* length of an Ethernet target ip address */

/* ether type */
#define ETHER_TYPE_START 0x0600
#define ETHER_TYPE_RARP  0x8035
#define ETHER_TYPE_PAE   0x888e  /* EAPOL PAE/802.1x */
#define ETHER_TYPE_IP    0x0800  /* IP protocol */
#define ETHER_TYPE_AARP  0x80f3  /* Appletalk AARP protocol */
#define ETHER_TYPE_IPX   0x8137  /* IPX over DIX protocol */
#define ETHER_TYPE_ARP   0x0806  /* ARP protocol */
#define ETHER_TYPE_IPV6  0x86dd  /* IPv6 */
#define ETHER_TYPE_VLAN  0x8100  /* VLAN TAG protocol */
#define ETHER_TYPE_TDLS  0x890d  /* TDLS */
#define ETHER_TYPE_WAI   0x88b4  /* WAI/WAPI */
#define ETHER_LLTD_TYPE  0x88D9  /* LLTD */
#define ETHER_ONE_X_TYPE 0x888E  /* 802.1x Authentication */
#define ETHER_TUNNEL_TYPE 0x88bd  /* 自定义tunnel协议 */
#define ETHER_TYPE_PPP_DISC 0x8863      /* PPPoE discovery messages */
#define ETHER_TYPE_PPP_SES  0x8864      /* PPPoE session messages */
#define ETHER_TYPE_6LO      0xa0ed      /* 6lowpan */

#define BROADCAST_MACADDR ((const osal_u8 *)"\xFF\xFF\xFF\xFF\xFF\xFF")  /* 广播MAC地址 */


/*****************************************************************************
  WLAN MAC宏定义
*****************************************************************************/
/* CCMP加密字节数 */
#define WLAN_CCMP_ENCRYP_LEN 16
/* CCMP256加密字节数 */
#define WLAN_CCMP256_GCMP_ENCRYP_LEN 24

/* Probe Rsp APP IE长度超过该值，发送帧netbuf采用大包 */
#define OAL_MGMT_NETBUF_APP_PROBE_RSP_IE_LEN_LIMIT 450

#define WLAN_DATA_VIP_TID              WLAN_TIDNO_BCAST

/* MAC地址占的字符长度 */
#define WLAN_MAC_ADDR_BYTE_LEN          17

#define OAL_MAX_FT_ALL_LEN             518   /* MD:5 FT:257 RSN:256 */

#define AP_WPS_P2P_IE_CMD_BEACON        0x1
#define AP_WPS_P2P_IE_CMD_PROBE_RSP     0x2
#define AP_WPS_P2P_IE_CMD_ASSOC_RSP     0x4


#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define oal_smp_mb() smp_mb()
#else
#define oal_smp_mb()
#endif
#define OAL_CONTAINER_OF(_member_ptr, _stru_type, _stru_member_name)  \
    container_of(_member_ptr, _stru_type, _stru_member_name)
#define ETHER_IS_MULTICAST(_a)   (*(_a) & 0x01)

/* is address bcast */
#define ETHER_IS_BROADCAST(_a)   \
    ((_a)[0] == 0xff &&          \
     (_a)[1] == 0xff &&          \
     (_a)[2] == 0xff &&          \
     (_a)[3] == 0xff &&          \
     (_a)[4] == 0xff &&          \
     (_a)[5] == 0xff)

/* IPv4永久组地址判断: 224.0.0.0～224.0.0.255为永久组地址 */
#define OAL_IPV4_PERMANET_GROUP_ADDR           0x000000E0
#define OAL_IPV4_IS_PERMANENT_GROUP(_a)       ((((_a) & 0x00FFFFFF) ^ OAL_IPV4_PERMANET_GROUP_ADDR) == 0)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef gfp_t        oal_gfp_enum_uint8;

typedef enum _wlan_net_queue_type_ {
    WLAN_HI_QUEUE = 0,
    WLAN_NORMAL_QUEUE,

    WLAN_TCP_DATA_QUEUE,
    WLAN_TCP_ACK_QUEUE,

    WLAN_UDP_DATA_QUEUE,

    WLAN_NET_QUEUE_BUTT
} wlan_net_queue_type;

#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
static inline osal_u8 wlan_data_queue_to_hcc(osal_u16 qid)
{
    return ((qid == WLAN_UDP_DATA_QUEUE) ? DATA_UDP_DATA_QUEUE :
        (qid == WLAN_TCP_DATA_QUEUE) ? DATA_TCP_DATA_QUEUE :
        (qid == WLAN_TCP_ACK_QUEUE) ? DATA_TCP_ACK_QUEUE : DATA_LO_QUEUE);
}

static inline osal_u16 hcc_data_queue_to_wlan(osal_u8 qid)
{
    return ((qid == DATA_UDP_DATA_QUEUE) ? WLAN_UDP_DATA_QUEUE :
        (qid == DATA_TCP_DATA_QUEUE) ? WLAN_TCP_DATA_QUEUE :
        (qid == DATA_TCP_ACK_QUEUE) ? WLAN_TCP_ACK_QUEUE : WLAN_NORMAL_QUEUE);
}
#endif

typedef enum {
    OAL_IEEE80211_MLME_AUTH     = 0,    /* MLME下发认证帧相关内容 */
    OAL_IEEE80211_MLME_ASSOC    = 1,    /* MLME下发关联帧相关内容 */
    OAL_IEEE80211_MLME_REASSOC  = 2,   /* MLME下发重关联帧相关内容 */
    OAL_IEEE80211_MLME_NUM
}mlme_type_enum;
typedef osal_u8 mlme_type_enum_uint8;

/* 以下不区分操作系统 */
/* 内核下发的扫描类型 */
typedef enum {
    OAL_PASSIVE_SCAN = 0,
    OAL_ACTIVE_SCAN = 1,

    OAL_SCAN_BUTT
} oal_scan_enum;
typedef osal_u8 oal_scan_enum_uint8;

/* 内核下发的扫描频段 */
typedef enum {
    OAL_SCAN_2G_BAND = 1,
    OAL_SCAN_5G_BAND = 2,
    OAL_SCAN_ALL_BAND = 3,

    OAL_SCAN_BAND_BUTT
} oal_scan_band_enum;
typedef osal_u8 oal_scan_band_enum_uint8;

enum APP_IE_BITMAP {
    BEACON_IE       = (1 << 0),
    PROBE_REQ_IE    = (1 << 1),
    PROBE_RSP_IE    = (1 << 2),
    ASSOC_REQ_IE    = (1 << 3),
    ASSOC_RSP_IE    = (1 << 4),
    REASSOC_REQ_IE  = (1 << 5),
    AUTH_REQ_IE     = (1 << 6)
};
typedef osal_u8 app_ie_bitmap;

enum APP_IE_TYPE {
    OAL_APP_BEACON_IE       = 0,
    OAL_APP_PROBE_REQ_IE    = 1,
    OAL_APP_PROBE_RSP_IE    = 2,
    OAL_APP_ASSOC_REQ_IE    = 3,
    OAL_APP_ASSOC_RSP_IE    = 4,
    OAL_APP_FT_IE           = 5,
    OAL_APP_REASSOC_REQ_IE  = 6,
    OAL_APP_EXTEND_IE1      = 7,
    OAL_APP_EXTEND_IE2      = 8,
    OAL_APP_EXTEND_IE3      = 9,
    OAL_APP_EXTEND_IE4      = 10,
    OAL_APP_VENDOR_IE       = 11, /* vendor IE管理帧上报 */
    OAL_APP_IE_NUM
};
typedef osal_u8 app_ie_type_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 不分平台通用结构体 */
typedef struct {
    osal_u8 ssid[OAL_IEEE80211_MAX_SSID_LEN]; /* ssid array */
    osal_u8 ssid_len;                          /* length of the array */
    osal_u8 arry[3];                          /* 3字节存储其他数据 */
} oal_ssids_stru;

typedef union {
    td_u8 byte;
    struct  {
        td_u8 high : 4,
        low : 4;
    } bits;
} bitfield_un;

static inline osal_u8 a2x(const osal_char c)
{
    if (c >= '0' && c <= '9') {
        return (osal_u8)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (osal_u8)0xa + (osal_u8)(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return (osal_u8)0xa + (osal_u8)(c - 'A');
    }
    return 0;
}

    /* ipv6 组播mac地址 */
#define ETHER_IS_IPV6_MULTICAST(_a)  (((_a)[0]) == 0x33 && ((_a)[1]) == 0x33)
    /* IPv6未指定地址: ::/128 ,该地址仅用于接口还没有被分配IPv6地址时与其它节点
       通讯作为源地址,例如在重复地址检测DAD中会出现. */
#define OAL_IPV6_IS_UNSPECIFIED_ADDR(_a)   \
    ((_a)[0] == 0x00 && (_a)[1]  == 0x00 && (_a)[2]  == 0x00 &&          \
    (_a)[3]  == 0x00 && (_a)[4]  == 0x00 && (_a)[5]  == 0x00 &&          \
    (_a)[6]  == 0x00 && (_a)[7]  == 0x00 && (_a)[8]  == 0x00 &&          \
    (_a)[9]  == 0x00 && (_a)[10] == 0x00 && (_a)[11] == 0x00 &&          \
    (_a)[12] == 0x00 && (_a)[13] == 0x00 && (_a)[14] == 0x00 &&          \
    (_a)[15] == 0x00)

    /* IPv6链路本地地址: 最高10位值为1111111010, 例如:FE80:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX  */
#define OAL_IPV6_IS_LINK_LOCAL_ADDR(_a)       (((_a)[0] == 0xFE) && ((_a)[1] >> 6 == 2))

    /* IGMP record type */
#define MAC_IGMP_QUERY_TYPE       0x11
#define MAC_IGMPV1_REPORT_TYPE    0x12
#define MAC_IGMPV2_REPORT_TYPE    0x16
#define MAC_IGMPV2_LEAVE_TYPE     0x17
#define MAC_IGMPV3_REPORT_TYPE    0x22

    /* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE        1
#define IGMPV3_MODE_IS_EXCLUDE        2
#define IGMPV3_CHANGE_TO_INCLUDE      3
#define IGMPV3_CHANGE_TO_EXCLUDE      4
#define IGMPV3_ALLOW_NEW_SOURCES      5
#define IGMPV3_BLOCK_OLD_SOURCES      6

    /* IGMP record type */
#define MLD_QUERY_TYPE            130
#define MLDV1_REPORT_TYPE      131
#define MLDV1_DONE_TYPE         132
#define MLDV2_REPORT_TYPE        143

    /* MLD V2 group record types [grec_type] */
#define MLDV2_MODE_IS_INCLUDE        1
#define MLDV2_MODE_IS_EXCLUDE        2
#define MLDV2_CHANGE_TO_INCLUDE      3
#define MLDV2_CHANGE_TO_EXCLUDE      4
#define MLDV2_ALLOW_NEW_SOURCES      5
#define MLDV2_BLOCK_OLD_SOURCES      6

#define SERVERNAME_LEN          64
#define BOOTFILE_LEN            128

    /* DHCP message type */
#define DHCP_DISCOVER           1
#define DHCP_OFFER              2
#define DHCP_REQUEST            3
#define DHCP_ACK                5
#define DHCP_NAK                6

#define DHO_PAD                 0
#define DHO_IPADDRESS           50
#define DHO_MESSAGETYPE         53
#define DHO_SERVERID            54
#define DHO_END                 255

#define DNS_MAX_DOMAIN_LEN  (100)

#define DHCP_SERVER_PORT    (67)
#define DHCP_CLIENT_PORT    (68)
#define DNS_SERVER_PORT     (53)

#define OAL_NETDEVICE_OPS(_pst_dev)                         ((_pst_dev)->netdev_ops)
#define OAL_NETDEVICE_MAC_ADDR(_pst_dev)                    ((_pst_dev)->dev_addr)
#define OAL_NETDEVICE_IFALIAS(_pst_dev)                     ((_pst_dev)->ifalias)
#define OAL_NETDEVICE_WDEV(_pst_dev)                        ((_pst_dev)->ieee80211_ptr)
#define OAL_NETDEVICE_HEADROOM(_pst_dev)                    ((_pst_dev)->needed_headroom)
#define OAL_NETDEVICE_TAILROOM(_pst_dev)                    ((_pst_dev)->needed_tailroom)
#define OAL_NETDEVICE_FLAGS(_pst_dev)                       ((_pst_dev)->flags)
#define OAL_NETDEVICE_WATCHDOG_TIMEO(_pst_dev)              ((_pst_dev)->watchdog_timeo)
#define OAL_WIRELESS_DEV_WIPHY(_pst_wireless_dev)           ((_pst_wireless_dev)->wiphy)
#define oal_netdevice_master(_pst_dev)                      ((_pst_dev)->master)

#define OAL_IFF_RUNNING         0x40
#define OAL_IFF_UP              0x01

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
#define oal_is_broadcast_ether_addr(a) (((a)[0] & (a)[1] & (a)[2] & (a)[3] & (a)[4] & (a)[5]) == 0xff)
/* is address mcast */
typedef struct {
    osal_u16 ar_hrd; /* format of hardware address */
    osal_u16 ar_pro; /* format of protocol address */

    osal_u8 ar_hln; /* length of hardware address */
    osal_u8 ar_pln; /* length of protocol address */
    osal_u16 ar_op; /* ARP opcode (command) */

    osal_u8 ar_sha[ETHER_ADDR_LEN];         /* sender hardware address */
    osal_u8 ar_sip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */
    osal_u8 ar_tha[ETHER_ADDR_LEN];         /* target hardware address */
    osal_u8 ar_tip[ETH_TARGET_IP_ADDR_LEN]; /* target IP address */
} oal_eth_arphdr_stru;

/* WIN32和linux共用结构体  */
typedef struct {
    osal_u8 type;
    osal_u8 len;
    osal_u8 addr[6]; /* hardware address6字节 */
} oal_eth_icmp6_lladdr_stru;

typedef struct {
    osal_u8 op;         /* packet opcode type */
    osal_u8 htype;      /* hardware addr type */
    osal_u8 hlen;       /* hardware addr length */
    osal_u8 hops;       /* gateway hops */
    osal_u32 xid;       /* transaction ID */
    osal_u16 secs;      /* seconds since boot began */
    osal_u16 flags;     /* flags */
    osal_u32 ciaddr;    /* client IP address */
    osal_u32 yiaddr;    /* 'your' IP address */
    osal_u32 siaddr;    /* server IP address */
    osal_u32 giaddr;    /* gateway IP address */
    osal_u8 chaddr[16]; /* client hardware address16字节 */
    osal_u8 sname[64];  /* server host name字符串长度64 */
    osal_u8 file[128];  /* boot file name字符串长度128 */
    osal_u8 options[4]; /* variable-length options field 4字节 */
} oal_dhcp_packet_stru;

struct oal_ip_header {
#if (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)            /* LITTLE_ENDIAN */
    osal_u8      ihl : 4,
                 version_ihl : 4;
#else
    osal_u8      version_ihl : 4,
                 ihl : 4;
#endif
    osal_u8    tos;
    osal_u16   tot_len;
    osal_u16   id;
    osal_u16   frag_off;
    osal_u8    ttl;
    osal_u8    protocol;
    osal_u16   check;
    osal_u32   saddr;
    osal_u32   daddr;
} OAL_DECLARE_PACKED;
typedef struct oal_ip_header oal_ip_header_stru;

typedef struct {
    osal_u16  sport;
    osal_u16  dport;
    osal_u32  seqnum;
    osal_u32  acknum;
    osal_u8   offset;
    osal_u8   flags;
    osal_u16  window;
    osal_u16  check;
    osal_u16  urgent;
}oal_tcp_header_stru;

typedef struct {
    osal_u16 source;
    osal_u16 dest;
    osal_u16 len;
    osal_u16 check;
}oal_udp_header_stru;

struct oal_ether_header {
    osal_u8    ether_dhost[ETHER_ADDR_LEN];
    osal_u8    ether_shost[ETHER_ADDR_LEN];
    osal_u16   ether_type;
} OAL_DECLARE_PACKED;
typedef struct oal_ether_header oal_ether_header_stru;
#endif

/*
wal 层到hmac侧传递使用该数据, WLAN_WPS_IE_MAX_SIZE扩容到608字节，超过事件队列大小，
wal到hmac ie data采用指针传递
*/
struct oal_w2h_app_ie {
    osal_u32              ie_len;
    app_ie_type_uint8    app_ie_type;
    osal_u8               rsv[3];
    osal_u8              *data_ie;
} OAL_DECLARE_PACKED;
typedef struct oal_w2h_app_ie oal_w2h_app_ie_stru;

struct oal_mlme_ie {
    mlme_type_enum_uint8    mlme_type;                    /* MLME类型 */
    osal_u8                  seq;                          /* 认证帧序列号 */
    osal_u16                 reason;                       /* 原因码 */
    osal_u8                  macaddr[6];
    osal_u16                 optie_len;
    osal_u8                  optie[OAL_MAX_FT_ALL_LEN];
};
typedef struct oal_mlme_ie oal_mlme_ie_stru;

/*
 *    Header in on cable format
 */
struct mac_igmp_header {
    osal_u8  type;
    osal_u8  code;        /* For newer IGMP */
    osal_u16 csum;
    osal_u32 group;
} OAL_DECLARE_PACKED;
typedef struct mac_igmp_header mac_igmp_header_stru;

/*  Group record format
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Record Type  |  Aux Data Len |     Number of Sources (N)     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Multicast Address                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Source Address [1]                      |
      +-                                                             -+
      |                       Source Address [2]                      |
      +-                                                             -+
      .                               .                               .
      .                               .                               .
      .                               .                               .
      +-                                                             -+
      |                       Source Address [N]                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                         Auxiliary Data                        .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_grec {
    osal_u8     grec_type;
    osal_u8     grec_auxwords;
    osal_u16    grec_nsrcs;
    osal_u32    grec_group_ip;
} OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_grec mac_igmp_v3_grec_stru;

/* IGMPv3 report format
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Type = 0x22  |    Reserved   |           Checksum            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved            |  Number of Group Records (M)  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [1]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [2]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                               .                               |
      .                               .                               .
      |                               .                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                                                               .
      .                        Group Record [M]                       .
      .                                                               .
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
struct mac_igmp_v3_report {
    osal_u8     type;
    osal_u8     resv1;
    osal_u16    csum;
    osal_u16    resv2;
    osal_u16    ngrec;
} OAL_DECLARE_PACKED;
typedef struct mac_igmp_v3_report mac_igmp_v3_report_stru;


struct mac_mld_v1_head {
    osal_u8     type;
    osal_u8     code;
    osal_u16    check_sum;
    osal_u16    max_response_delay;
    osal_u16    reserved;
    osal_u8     group_ip[OAL_IPV6_ADDR_SIZE];
} OAL_DECLARE_PACKED;
typedef struct mac_mld_v1_head mac_mld_v1_head_stru;

struct mac_mld_v2_report {
    osal_u8     type;
    osal_u8     code;
    osal_u16    check_sum;
    osal_u16    reserved;
    osal_u16    group_address_num;
} OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_report mac_mld_v2_report_stru;

struct mac_mld_v2_group_record {
    osal_u8     grec_type;
    osal_u8     grec_auxwords;                                              // 辅助数据长度
    osal_u16    grec_srcaddr_num;                                           //  原地址个数
    osal_u8    group_ip[OAL_IPV6_ADDR_SIZE];                             // 组播组地址
} OAL_DECLARE_PACKED;
typedef struct mac_mld_v2_group_record mac_mld_v2_group_record_stru;

struct mac_vlan_tag {
    osal_u16    tpid;              // tag   ID
    osal_u16    user_pri : 3,     // 帧的优先级
                  cfi : 1,
                  vlan_id : 12;     // 可配置的VLAN ID
} OAL_DECLARE_PACKED;
typedef struct mac_vlan_tag mac_vlan_tag_stru;

typedef struct {
    osal_u16          id;    /* transaction id */
    osal_u16          flags; /* message future */
    osal_u16          qdcount;   /* question record count */
    osal_u16          ancount;   /* answer record count */
    osal_u16          nscount;   /* authority record count */
    osal_u16          arcount;   /* additional record count */
}oal_dns_hdr_stru;

typedef enum {
    OAL_NS_Q_REQUEST = 0, /* request */
    OAL_NS_Q_RESPONSE = 1, /* response */
} oal_ns_qrcode;

typedef enum {
    OAL_NS_O_QUERY = 0,     /* Standard query. */
    OAL_NS_O_IQUERY = 1,    /* Inverse query (deprecated/unsupported). */
}oal_ns_opcode;

/*
 * Currently defined response codes.
 */
typedef enum {
    OAL_NS_R_NOERROR = 0,   /* No error occurred. */
}oal_ns_rcode;

typedef enum {
    OAL_NS_T_INVALID = 0,   /* Cookie. */
    OAL_NS_T_A = 1,         /* Host address. */
}oal_ns_type;

/* 邻居AP列表的BSS描述信息结构体 */
struct oal_bssid_infomation {
    osal_u8       ap_reachability : 2,                            /* AP的可到达性 */
                    security : 1,                                 /* 该AP的加密规则与当前连接是否一致 */
                    key_scope : 1,                                /* 该AP的认证信息是否与当前上报一直 */
                    spectrum_mgmt : 1,                            /* 能力位: 支持频谱管理 */    /* 能力位字段与beacon定义一致 */
                    qos : 1,                                      /* 能力位: 支持QOS */
                    apsd : 1,                                     /* 能力位: 支持APSD */
                    radio_meas : 1;                               /* 能力位: 波长测量 */
    osal_u8       delay_block_ack : 1,                            /* 能力位: 阻塞延迟应答 */
                    immediate_block_ack : 1,                      /* 能力位: 阻塞立即应答 */
                    mobility_domain : 1,                          /* 该AP的beacon帧中是否含有MDE，且与此次上报一致 */
                    high_throughput : 1,                          /* 该AP的beacon帧中是否含有高吞吐量元素，且与此次上报一致 */
                    resv1 : 4;                                    /* 预留 */
    osal_u8       resv2;
    osal_u8       resv3;
} OAL_DECLARE_PACKED;
typedef struct oal_bssid_infomation  oal_bssid_infomation_stru;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS != _PRE_OS_VERSION)
/*****************************************************************************
 函 数 名  : ether_is_multicast
 功能描述  : 判断一个地址是否为组播地址
*****************************************************************************/
static INLINE__ osal_u8 ether_is_multicast(const osal_u8 *addr)
{
    return ((*addr) & 0x01);
}

/*****************************************************************************
 函 数 名  : ether_is_broadcast
 功能描述  : 判断一个地址是否为广播地址
*****************************************************************************/
static INLINE__ osal_u8 ether_is_broadcast(const osal_u8 *addr)
{
    return ((addr[0] == 0xff) && (addr[1] == 0xff) && (addr[2] == 0xff) &&
        (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff));
}

/*****************************************************************************
 函 数 名  : ether_is_ipv4_multicast
 功能描述  : 判断一个地址是否为IPv4组播地址
*****************************************************************************/
static INLINE__ osal_u8 ether_is_ipv4_multicast(const osal_u8 *addr)
{
    return ((addr[0] == 0x01) && (addr[1] == 0x00) && (addr[2] == 0x5e));
}

/*****************************************************************************
 函 数 名  : oal_ipv4_is_permanent_group
 功能描述  : 判断一个IPv4地址是否为永久组地址:224.0.0.0～224.0.0.255
*****************************************************************************/
static INLINE__ osal_u8 oal_ipv4_is_permanent_group(osal_u32 addr)
{
    return (((addr & 0x00FFFFFF) ^ OAL_IPV4_PERMANET_GROUP_ADDR) == 0);
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_multicast
 功能描述  : 判断一个地址是否为IPv6组播地址:FFXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX
*****************************************************************************/
static INLINE__ osal_u8 oal_ipv6_is_multicast(const osal_u8 *addr)
{
    return ((osal_u8)(addr[0]) == 0xff);
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_unspecified_addr
 功能描述  : 判断一个地址是否为IPv6未指定地址
           IPv6未指定地址: ::/128 ,该地址仅用于接口还没有被分配IPv6地址时
           与其它节点通讯作为源地址,例如在重复地址检测DAD中会出现.
*****************************************************************************/
static INLINE__ osal_u8 oal_ipv6_is_unspecified_addr(const osal_u8 *addr)
{
    return ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00) &&
        (addr[4] == 0x00) && (addr[5] == 0x00) && (addr[6] == 0x00) && (addr[7] == 0x00) &&
        (addr[8] == 0x00) && (addr[9] == 0x00) && (addr[10] == 0x00) && (addr[11] == 0x00) &&
        (addr[12] == 0x00) && (addr[13] == 0x00) && (addr[14] == 0x00) && (addr[15] == 0x00));
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_link_local_addr
 功能描述  : 判断一个地址是否为IPv6链路本地地址.最高10位值为1111111010.
           例如:FE80:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX
*****************************************************************************/
static INLINE__ osal_u8 oal_ipv6_is_link_local_addr(const osal_u8 *addr)
{
    return ((addr[0] == 0xFE) && ((addr[1] >> 6) == 2));
}

static INLINE__ osal_u8 *oal_netbuf_put_rsv(const oal_netbuf_stru *netbuf, osal_u32 len)
{
    /* device侧注空啥都不做 */
    unref_param(netbuf);
    unref_param(len);
    return 0;
}
#endif

/***************************************xr 分界线***************************************************/
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef iw_handler                                  oal_iw_handler;
#define oal_netif_running(_pst_net_dev)             netif_running(_pst_net_dev)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#define OAL_NETDEVICE_DESTRUCTOR(_pst_dev)                  ((_pst_dev)->priv_destructor)
#else
#define OAL_NETDEVICE_LAST_RX(_pst_dev)                     ((_pst_dev)->last_rx)
#define OAL_NETDEVICE_DESTRUCTOR(_pst_dev)                  ((_pst_dev)->destructor)
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
#define oal_netdevice_qdisc(_pst_dev, val)                  ((_pst_dev)->qdisc = val)
#else
#define oal_netdevice_qdisc(_pst_dev, val)
#endif

#define OAL_SIOCIWFIRSTPRIV     SIOCIWFIRSTPRIV

/* iw_priv参数类型OAL封装 */
#define OAL_IW_PRIV_TYPE_BYTE   IW_PRIV_TYPE_BYTE       /* Char as number */
#define OAL_IW_PRIV_TYPE_CHAR   IW_PRIV_TYPE_CHAR       /* Char as character */
#define OAL_IW_PRIV_TYPE_INT    IW_PRIV_TYPE_INT        /* 32 bits int */
#define OAL_IW_PRIV_TYPE_ADDR   IW_PRIV_TYPE_ADDR       /* struct sockaddr */
#define OAL_IW_PRIV_SIZE_FIXED  IW_PRIV_SIZE_FIXED      /* Variable or fixed number of args */
#define OAL_IW_PRIV_SIZE_MASK   IW_PRIV_SIZE_MASK       /* Max number of those args */

/* iwconfig mode oal封装 */
#define OAL_IW_MODE_AUTO        IW_MODE_AUTO            /* Let the driver decides */
#define OAL_IW_MODE_INFRA       IW_MODE_INFRA           /* Multi cell network, roaming, ... */

/* Transmit Power flags available */
#define OAL_IW_TXPOW_DBM        IW_TXPOW_DBM            /* Value is in dBm */

/* 主机与网络字节序转换 */
#define oal_host2net_short(_x)  htons(_x)
#define oal_net2host_short(_x)  ntohs(_x)
#define oal_host2net_long(_x)   htonl(_x)
#define oal_net2host_long(_x)   ntohl(_x)

#if  LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
#define OAL_VLAN_PRIO_SHIFT     VLAN_PRIO_SHIFT
#else
#define OAL_VLAN_PRIO_SHIFT     13
#endif

/* ARP protocol opcodes. */
#define OAL_ARPOP_REQUEST      ARPOP_REQUEST         /* ARP request          */
#define OAL_ARPOP_REPLY        ARPOP_REPLY           /* ARP reply            */

#define OAL_IPPROTO_TCP     IPPROTO_TCP         /* Transmission Control Protocol */

#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
#define oal_wdev_match(_netif, _req)   ((_netif)->ieee80211_ptr == (_req)->wdev)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#define oal_wdev_match(_netif, _req)   ((_netif) == (_req)->dev)
#else
#define oal_wdev_match(_netif, _req)   ((_netif)->ifindex == (_req)->ifidx)
#endif

typedef struct sock                 oal_sock_stru;
typedef struct net                  oal_net_stru;
typedef struct module               oal_module_stru;
typedef struct nlmsghdr             oal_nlmsghdr_stru;

typedef struct ethhdr               oal_ethhdr;
typedef struct nf_hook_ops          oal_nf_hook_ops_stru;
typedef struct net_bridge_port      oal_net_bridge_port;

typedef struct ipv6hdr              oal_ipv6hdr_stru;
typedef struct icmp6hdr             oal_icmp6hdr_stru;
typedef struct in6_addr             oal_in6_addr;
typedef struct nd_msg               oal_nd_msg_stru;
typedef struct nd_opt_hdr           oal_nd_opt_hdr;
typedef struct netlink_skb_parms    oal_netlink_skb_parms;
#ifdef _PRE_WLAN_FEATURE_WPA3
typedef struct cfg80211_external_auth_params oal_external_auth_stru;
#endif

/* netlink���� */
#define OAL_NLMSG_HDRLEN                 NLMSG_HDRLEN

#define OAL_NLA_FOR_EACH_ATTR(pos, head, len, rem)            nla_for_each_attr(pos, head, len, rem)

typedef struct ieee80211_iface_limit        oal_ieee80211_iface_limit;
typedef struct ieee80211_iface_combination  oal_ieee80211_iface_combination;

#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define OAL_NETDEVICE_DESTRUCTOR(_pst_dev)                  ((_pst_dev)->destructor)
#define oal_netdevice_qdisc(_pst_dev, val)                  ((_pst_dev)->qdisc = val)

#define IEEE80211_MAX_SSID_LEN              32
#define OAL_VLAN_PRIO_SHIFT     13
#define OAL_SIOCIWFIRSTPRIV     0x8BE0

/* iw_priv参数类型OAL封装 */
#define OAL_IW_PRIV_TYPE_BYTE   0x1000       /* Char as number */
#define OAL_IW_PRIV_TYPE_CHAR   0x2000       /* Char as character */
#define OAL_IW_PRIV_TYPE_INT    0x4000       /* 32 bits int */
#define OAL_IW_PRIV_TYPE_ADDR   0x6000       /* struct sockaddr */
#define OAL_IW_PRIV_SIZE_FIXED  0x0800       /* Variable or fixed number of args */
#define OAL_IW_PRIV_SIZE_MASK   0x07FF       /* Max number of those args */

/* iwconfig mode oal封装 */
#define OAL_IW_MODE_AUTO    0    /* Let the driver decides */
#define OAL_IW_MODE_ADHOC   1    /* Single cell network */
#define OAL_IW_MODE_INFRA   2    /* Multi cell network, roaming, ... */
#define OAL_IW_MODE_MASTER  3    /* Synchronisation master or Access Point */
#define OAL_IW_MODE_REPEAT  4    /* Wireless Repeater (forwarder) */
#define OAL_IW_MODE_SECOND  5    /* Secondary master/repeater (backup) */
#define OAL_IW_MODE_MONITOR 6    /* Passive monitor (listen only) */
#define OAL_IW_MODE_MESH    7    /* Mesh (IEEE 802.11s) network */

 /* Transmit Power flags available */
#define OAL_IW_TXPOW_TYPE       0x00FF            /* Type of value */
#define OAL_IW_TXPOW_DBM        0x0000            /* Value is in dBm */
#define OAL_IW_TXPOW_MWATT      0x0001            /* Value is in mW */
#define OAL_IW_TXPOW_RELATIVE   0x0002            /* Value is in arbitrary units */
#define OAL_IW_TXPOW_RANGE      0x1000            /* Range of value between min/max */

/* 主机与网络字节序转换 */

#define oal_host2net_short(_x)  oal_swap_byteorder_16(_x)
#define oal_net2host_short(_x)  oal_swap_byteorder_16(_x)
#define oal_host2net_long(_x)   oal_swap_byteorder_32(_x)
#define oal_net2host_long(_x)   oal_swap_byteorder_32(_x)

 /* ARP protocol opcodes. */
#define OAL_ARPOP_REQUEST      1           /* ARP request */
#define OAL_ARPOP_REPLY        2           /* ARP reply   */


#define  OAL_IPPROTO_TCP     6             /* Transmission Control Protocol */

#define OAL_INIT_NET            init_net
#define OAL_THIS_MODULE         THIS_MODULE
#define OAL_MSG_DONTWAIT        MSG_DONTWAIT

/* 管制域相关结构体定义 */
#define mhz_to_khz(freq) ((freq) * 1000)
#define dbi_to_mbi(gain) ((gain) * 100)
#define dbm_to_mbm(gain) ((gain) * 100)

#define reg_rule(start, end, bw, gain, eirp, reg_flags) \
        { \
            .freq_range.start_freq_khz = mhz_to_khz(start), \
                                         .freq_range.end_freq_khz = mhz_to_khz(end), \
                                         .freq_range.max_bandwidth_khz = mhz_to_khz(bw), \
                                         .power_rule.max_antenna_gain = dbi_to_mbi(gain), \
                                         .power_rule.max_eirp = dbm_to_mbm(eirp), \
                                         .flags = (reg_flags), \
        }

/* netlink相关 */
#define OAL_NLMSG_ALIGNTO               4
#define oal_nlmsg_align(_len)           (((_len) + OAL_NLMSG_ALIGNTO - 1) & ~(OAL_NLMSG_ALIGNTO - 1))

/* VLAN以太网头 win32封装 */
typedef struct {
    td_u8 h_dest[6];       /* 6字节目的地址 */
    td_u8 h_source[6];     /* 6字节源地址 */
    td_u16 h_vlan_proto;
    td_u16 h_vlan_TCI;
    td_u16 h_vlan_encapsulated_proto;
} oal_vlan_ethhdr_stru;
#endif

static inline td_s32 oal_ieee80211_channel_to_frequency(td_s32 l_channel,
    oal_ieee80211_band_enum_uint8 band)
{
    if (l_channel <= 0) {
        return 0; /* not supported */
    }

    if (band == OAL_IEEE80211_BAND_2GHZ) {
        /* 14:信道  2484/2407:频率 5:两个信道中心频率相差5MHZ */
        return ((l_channel == 14) ? 2484 : (2407 + l_channel * 5));
    } else if (band == OAL_IEEE80211_BAND_5GHZ) {
        /* 182/196:信道  4000/5000:频率 5:两个信道中心频率相差5MHZ */
        return ((l_channel >= 182 && l_channel <= 196) ? (4000 + l_channel * 5) : (5000 + l_channel * 5));
    }

    /* not supported */
    return 0;
}

static inline td_s32  oal_ieee80211_frequency_to_channel(td_s32 l_center_freq)
{
    td_s32 l_channel;

    /* see 802.11 17.3.8.3.2 and Annex J */
    if (l_center_freq == 0) {
        l_channel = 0;
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    } else if (l_center_freq == 2512) { // 频率为2512
        l_channel = 15; // 频率为2512对应信道号为15
    } else if (l_center_freq == 2384) { // 频率为2384
        l_channel = 16; // 频率为2384对应信道号为16
#endif
    } else if (l_center_freq == 2484) { /* 频率为2484 */
        l_channel = 14; /* 14信道对应2484 */
    } else if (l_center_freq < 2484) { /* 频率小于2484 */
        l_channel = (l_center_freq - 2407) / 5; /* 频率小于2484对应的信道号 = （频率 - 2407）/ 5 */
    } else if (l_center_freq >= 4910 && l_center_freq <= 4980) { /* 频率在[4910, 4980] */
        l_channel = (l_center_freq - 4000) / 5;  /* 对应的信道号 =(频率 - 4000) / 5 */
    } else if (l_center_freq <= 45000) { /* DMG band lower limit 45000 */
        l_channel = (l_center_freq - 5000) / 5; /* 对应的信道号 =(频率 - 5000) / 5 */
    } else if (l_center_freq >= 58320 && l_center_freq <= 64800) { /* 频率在[58320, 64800] */
        l_channel = (l_center_freq - 56160) / 2160; /* 对应的信道号 =(频率 - 56160)/2160 */
    } else {
        l_channel = 0;
    }
    return l_channel;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static inline oal_ieee80211_channel_stru* oal_ieee80211_get_channel(oal_wiphy_stru *wiphy,
    td_s32 freq)
{
    return ieee80211_get_channel(wiphy, freq);
}

static inline oal_wiphy_stru* oal_wiphy_new(oal_cfg80211_ops_stru *ops, td_s32 sizeof_priv)
{
    return wiphy_new(ops, sizeof_priv);
}

static inline td_s32 oal_wiphy_register(oal_wiphy_stru *wiphy)
{
    return wiphy_register(wiphy);
}


static inline td_void  oal_wiphy_unregister(oal_wiphy_stru *wiphy)
{
    return wiphy_unregister(wiphy);
}


static inline void oal_wiphy_free(oal_wiphy_stru *wiphy)
{
    wiphy_free(wiphy);
}

static inline void *oal_wiphy_priv(oal_wiphy_stru *wiphy)
{
    return wiphy_priv(wiphy);
}


static inline void oal_wiphy_apply_custom_regulatory(oal_wiphy_stru *wiphy,
    const oal_ieee80211_regdomain_stru *regd)
{
    wiphy_apply_custom_regulatory(wiphy, regd);
}

static inline td_u16 oal_eth_type_trans(oal_netbuf_stru *pst_netbuf, oal_net_device_stru *device)
{
    return eth_type_trans(pst_netbuf, device);
}

static inline td_void oal_ether_setup(oal_net_device_stru *p_net_device)
{
    if (TD_NULL == p_net_device) {
        return;
    }

    ether_setup(p_net_device);

    return;
}

static inline oal_net_device_stru* oal_dev_get_by_name(const td_s8 *pc_name)
{
    return dev_get_by_name(&init_net, pc_name);
}


#define oal_dev_put(_pst_dev) dev_put(_pst_dev)

static inline td_void  oal_net_close_dev(oal_net_device_stru *netdev)
{
    rtnl_lock();
    dev_close(netdev);
    rtnl_unlock();
}

static inline oal_net_device_stru* oal_net_alloc_netdev(td_u32 sizeof_priv, td_s8 *name,
    td_void *p_set_up)
{
    if ((TD_NULL == name) || (TD_NULL == p_set_up)) {
        return TD_NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    return alloc_netdev(sizeof_priv, name, NET_NAME_UNKNOWN, p_set_up);
#else
    return alloc_netdev(sizeof_priv, name, p_set_up);
#endif
}

static inline oal_net_device_stru* oal_net_alloc_netdev_mqs(td_u32 sizeof_priv, td_s8 *name,
    td_void *p_set_up, td_u32 txqs, td_u32 rxqs)
{
    if ((TD_NULL == name) || (TD_NULL == p_set_up)) {
        return TD_NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    return alloc_netdev_mq(sizeof_priv, name, NET_NAME_UNKNOWN, p_set_up, txqs);
#else
    return alloc_netdev_mq(sizeof_priv, name, p_set_up, txqs);
#endif
}

static inline td_void oal_net_tx_wake_all_queues(oal_net_device_stru *dev)
{
    if (TD_NULL == dev) {
        return;
    }

    return netif_tx_wake_all_queues(dev);
}


static inline td_void oal_net_tx_stop_all_queues(oal_net_device_stru *dev)
{
    if (TD_NULL == dev) {
        return;
    }

    return netif_tx_stop_all_queues(dev);
}

static inline td_void oal_net_wake_subqueue(oal_net_device_stru *dev, td_u16 queue_idx)
{
    if (TD_NULL == dev) {
        return;
    }

    return netif_wake_subqueue(dev, queue_idx);
}


static inline td_void oal_net_stop_subqueue(oal_net_device_stru *dev, td_u16 queue_idx)
{
    if (TD_NULL == dev) {
        return;
    }

    return netif_stop_subqueue(dev, queue_idx);
}

static inline td_void oal_net_free_netdev(oal_net_device_stru *netdev)
{
    if (TD_NULL == netdev) {
        return ;
    }

    free_netdev(netdev);
}

static inline td_s32 oal_net_register_netdev(oal_net_device_stru *p_net_device)
{
    if (TD_NULL == p_net_device) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Just For HCC */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* ETHER HEAD 4bytes */
    OAL_NETDEVICE_HEADROOM(p_net_device) = 74; /* netdevice headroom指定大小74 */
#else
    OAL_NETDEVICE_HEADROOM(p_net_device) = 64; /* netdevice headroom指定大小64 */
#endif
    OAL_NETDEVICE_TAILROOM(p_net_device) = 32; /* netdevice tailroom指定大小32 */

    return register_netdev(p_net_device);
}

static inline td_void oal_net_unregister_netdev(oal_net_device_stru *p_net_device)
{
    if (TD_NULL == p_net_device) {
        return ;
    }

    if (p_net_device->reg_state != NETREG_REGISTERED) {
        wifi_printf("net device not registed regstate:%d!!\n", p_net_device->reg_state);
        return;
    }
    unregister_netdev(p_net_device);
}

static inline td_s32  oal_net_device_open(oal_net_device_stru *dev)
{
    dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

static inline td_s32 oal_net_device_close(oal_net_device_stru *dev)
{
    dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

static inline td_s32 oal_net_device_set_macaddr(oal_net_device_stru *dev, td_void *addr)
{
    oal_sockaddr_stru *mac;

    mac = (oal_sockaddr_stru *)addr;

    (td_void)memcpy_s(dev->dev_addr, ETHER_ADDR_LEN, mac->sa_data, ETHER_ADDR_LEN);
    return OAL_SUCC;
}

static inline td_s32 oal_net_device_init(oal_net_device_stru *dev)
{
    return OAL_SUCC;
}

static inline oal_net_device_stats_stru *oal_net_device_get_stats(oal_net_device_stru *dev)
{
    oal_net_device_stats_stru *stats;

    stats = &dev->stats;

    stats->tx_errors     = 0;
    stats->tx_dropped    = 0;
    stats->tx_packets    = 0;
    stats->rx_packets    = 0;
    stats->rx_errors     = 0;
    stats->rx_dropped    = 0;
    stats->rx_crc_errors = 0;

    return stats;
}

static inline td_s32 oal_net_device_ioctl(oal_net_device_stru *dev, oal_ifreq_stru *ifr, td_s32 ul_cmd)
{
    return -OAL_EINVAL;
}

static inline td_s32 oal_net_device_change_mtu(oal_net_device_stru *dev, td_s32 mtu)
{
    dev->mtu = mtu;
    return OAL_SUCC;
}

#ifdef _PRE_SKB_TRACE
#define oal_netif_rx(pst_netbuf)                 \
({                                                  \
    mem_trace_delete_node((osal_ulong)(pst_netbuf)); \
    netif_rx(pst_netbuf);                        \
})
#else
static inline td_s32  oal_netif_rx(oal_netbuf_stru *pst_netbuf)
{
    return netif_rx(pst_netbuf);
}
#endif

static inline td_s32 oal_nla_put_u32(oal_netbuf_stru *skb, td_s32 l_attrtype, td_u32 value)
{
    return nla_put_u32(skb, l_attrtype, value);
}

static inline td_s32 oal_nla_put_u16(oal_netbuf_stru *skb, td_s32 l_attrtype, td_u32 value)
{
    return nla_put_u16(skb, l_attrtype, value);
}

static inline td_s32 oal_nla_put_flag(oal_netbuf_stru *skb, td_s32 l_attrtype)
{
    return nla_put_flag(skb, l_attrtype);
}

static inline td_s32  oal_nla_put(oal_netbuf_stru *skb, td_s32 l_attrtype, td_s32 l_attrlen, const td_void *p_data)
{
    return nla_put(skb, l_attrtype, l_attrlen, p_data);
}

/*
 * nla_put_nohdr - Add a netlink attribute without header
 * @skb: socket buffer to add attribute to
 * @attrlen: length of attribute payload
 * @data: head of attribute payload
 *
 * Returns -EMSGSIZE if the tailroom of the skb is insufficient to store
 * the attribute payload.
 */
static inline td_s32  oal_nla_put_nohdr(oal_netbuf_stru *skb, td_s32 l_attrlen, const td_void *p_data)
{
    return nla_put_nohdr(skb, l_attrlen, p_data);
}

static inline  oal_netbuf_stru *oal_nlmsg_new(td_s32 payload, oal_gfp_enum_uint8 flags)
{
    return nlmsg_new(payload, flags);
}

static inline td_void oal_nlmsg_free(oal_netbuf_stru *skb)
{
    return nlmsg_free(skb);
}

static inline td_s32  oal_genlmsg_multicast(oal_genl_family_stru *family, oal_netbuf_stru *skb, td_u32 pid,
    td_u32 group, oal_gfp_enum_uint8 flags)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
    return genlmsg_multicast(skb, pid, group, flags);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
    return genlmsg_multicast(family, skb, pid, group, flags);
#else
    /* Linux genlmsg_multicast */
    return OAL_SUCC;
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)) */
}

static inline td_void *oal_genlmsg_put(oal_netbuf_stru *skb, td_u32 pid, td_u32 seq,
    oal_genl_family_stru *family, td_s32 flags, td_u8 cmd)
{
    return genlmsg_put(skb, pid, seq, family, flags, cmd);
}

static inline oal_nlattr_stru *oal_nla_nest_start(oal_netbuf_stru *skb, td_s32 l_attrtype)
{
    return nla_nest_start(skb, l_attrtype);
}

static inline td_void  oal_genlmsg_cancel(oal_netbuf_stru *skb, td_void *hdr)
{
    return genlmsg_cancel(skb, hdr);
}

static inline td_s32  oal_nla_nest_end(oal_netbuf_stru *skb, oal_nlattr_stru *start)
{
    return nla_nest_end(skb, start);
}

static inline td_s32  oal_genlmsg_end(oal_netbuf_stru *skb, td_void *hdr)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
    return genlmsg_end(skb, hdr);
#else
    genlmsg_end(skb, hdr);
    return OAL_SUCC;
#endif
}

static inline td_void *oal_nla_data(const oal_nlattr_stru *nla)
{
    return nla_data(nla);
}

static inline td_u32 oal_nla_get_u8(const oal_nlattr_stru *nla)
{
    return nla_get_u8(nla);
}

static inline td_u32 oal_nla_get_u16(const oal_nlattr_stru *nla)
{
    return nla_get_u16(nla);
}

static inline td_u32 oal_nla_get_u32(const oal_nlattr_stru *nla)
{
    return nla_get_u32(nla);
}


static inline td_u32 oal_nla_total_size(const oal_nlattr_stru *nla)
{
    td_s32 payload = nla_len(nla);
    return nla_total_size(payload);
}

static inline td_s32 oal_nla_len(const oal_nlattr_stru *nla)
{
    return nla_len(nla);
}


static inline td_s32 oal_nla_type(const oal_nlattr_stru *nla)
{
    return nla_type(nla);
}

static inline oal_cfg80211_registered_device_stru *oal_wiphy_to_dev(oal_wiphy_stru *wiphy)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
    return wiphy_to_dev(wiphy);
#else
    return wiphy_to_rdev(wiphy);
#endif
}

static inline td_u16  oal_csum_ipv6_magic(oal_in6_addr *ipv6_s,       oal_in6_addr *ipv6_d,
    td_u32 len, td_u16 proto, td_u32 sum)
{
    return csum_ipv6_magic(ipv6_s, ipv6_d, len, proto, sum);
}

static inline td_u32  oal_csum_partial(const td_void *p_buff,      td_s32 l_len, td_u32 sum)
{
    return csum_partial(p_buff, l_len, sum);
}

static inline td_s32  oal_ipv6_addr_type(oal_in6_addr *ipv6)
{
#ifdef _PRE_WLAN_FEATURE_SUPPORT_IPV6
    return ipv6_addr_type(ipv6);
#else
    return (td_s32)IPV6_ADDR_RESERVED;
#endif
}

static inline td_s32  oal_pskb_may_pull(oal_netbuf_stru *nb, td_u32 len)
{
    return pskb_may_pull(nb, len);
}

static inline oal_netbuf_stru  *oal_arp_create(td_s32 l_type, td_s32 l_ptype, td_u32 dest_ip,
    oal_net_device_stru *dev, td_u32 src_ip, td_u8 *dest_hw, td_u8 *src_hw, td_u8 *target_hw)
{
    return arp_create(l_type, l_ptype, dest_ip, dev, src_ip, dest_hw, src_hw, target_hw);
}
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp_port_etc(const oal_udp_header_stru *udp_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_dhcp6_etc(oal_ipv6hdr_stru *ipv6hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6_etc(oal_ipv6hdr_stru  *ipv6hdr);
extern osal_u16 oal_netbuf_select_queue_etc(oal_netbuf_stru *buf);
extern oal_bool_enum_uint8 oal_netbuf_is_tcp_ack_etc(oal_ip_header_stru  *ip_hdr);
extern oal_bool_enum_uint8 oal_netbuf_is_icmp_etc(oal_ip_header_stru  *ip_hdr);

#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)

typedef struct ieee80211_channel_sw_ie {
    osal_u8 mode;
    osal_u8 new_ch_num;
    osal_u8 count;
    osal_u8 rsv[1];
} oal_ieee80211_channel_sw_ie;

td_u32 hwal_netif_rx(oal_net_device_stru* netdev, oal_netbuf_stru* netbuf);

#define chan2g(_channel, _freq, _flags)  \
{                       \
    .band                   = OAL_IEEE80211_BAND_2GHZ,          \
    .center_freq            = (_freq),                      \
    .hw_value               = (_channel),                   \
    .flags                  = (_flags),                     \
    .max_antenna_gain       = 0,                            \
    .max_power              = 30,                           \
}

#define ratetab_ent(_rate, _rateid, _flags)     \
{                                                               \
    .bitrate        = (_rate),                                  \
    .hw_value       = (_rateid),                                \
    .flags          = (_flags),                                 \
}

static inline osal_void oal_net_tx_stop_all_queues(const oal_net_device_stru *dev)
{
    if (dev == OAL_PTR_NULL) {
        return;
    }

    return;
}

static inline osal_void oal_net_tx_wake_all_queues(const oal_net_device_stru *dev)
{
    if (dev == OAL_PTR_NULL) {
        return;
    }

    return ;
}

static inline osal_void oal_net_stop_subqueue(const oal_net_device_stru *dev, osal_u16 queue_idx)
{
    unref_param(queue_idx);
    if (dev == OAL_PTR_NULL) {
        return;
    }

    return ;
}


static inline osal_s32 oal_netif_rx(oal_netbuf_stru *pst_netbuf)
{
    return (osal_s32)hwal_netif_rx(pst_netbuf->dev, pst_netbuf);
}


static inline osal_u8 oal_netbuf_get_bitfield(osal_void)
{
    bitfield_un un_bitfield;

    un_bitfield.byte = 0x12;
    if (un_bitfield.bits.low == 0x2) {
        return OAL_BITFIELD_LITTLE_ENDIAN;
    } else {
        return OAL_BITFIELD_BIG_ENDIAN;
    }
}


#ifdef _PRE_SKB_TRACE
#define oal_netbuf_copy(pst_netbuf, priority)        \
({ \
    oal_netbuf_stru *__no_pst_copy_netbuf; \
    __no_pst_copy_netbuf = skb_copy(pst_netbuf, priority); \
    if (__no_pst_copy_netbuf) \
    { \
        mem_trace_add_node((osal_ulong)__no_pst_copy_netbuf); \
    } \
    __no_pst_copy_netbuf; \
})
#else
#define oal_netbuf_copy(pst_netbuf, priority) _oal_netbuf_copy(pst_netbuf, priority)
#endif

static inline osal_void oal_net_wake_subqueue(const oal_net_device_stru *dev, osal_u16 queue_idx)
{
    unref_param(queue_idx);
    if (dev == OAL_PTR_NULL) {
        return;
    }

    return ;
}

static inline osal_u16  oal_eth_type_trans(oal_netbuf_stru *pst_netbuf, oal_net_device_stru *device)
{
    unref_param(device);
    oal_netbuf_pull(pst_netbuf, sizeof(oal_ether_header_stru));
    return 0;
}

static inline osal_void  oal_net_close_dev(oal_net_device_stru *netdev)
{
    OAL_NETDEVICE_FLAGS(netdev) &= ~OAL_IFF_RUNNING;
}

static inline osal_s32  oal_net_device_open(oal_net_device_stru *dev)
{
    dev->flags |= OAL_IFF_RUNNING;

    return OAL_SUCC;
}

static inline osal_s32 oal_net_device_close(oal_net_device_stru *dev)
{
    dev->flags &= ~OAL_IFF_RUNNING;

    return OAL_SUCC;
}

static inline osal_s32 oal_net_device_set_macaddr(oal_net_device_stru *dev, osal_void *addr)
{
    oal_sockaddr_stru *mac;

    mac = (oal_sockaddr_stru *)addr;

    (osal_void)memcpy_s(dev->dev_addr, 6, mac->sa_data, 6); /* 6:mac长度 */
    return OAL_SUCC;
}

static inline osal_s32 oal_net_device_init(oal_net_device_stru *dev)
{
    unref_param(dev);
    return OAL_SUCC;
}

static inline oal_net_device_stats_stru *oal_net_device_get_stats(oal_net_device_stru *dev)
{
    oal_net_device_stats_stru *stats;

    stats = &dev->stats;

    stats->tx_errors     = 0;
    stats->tx_dropped    = 0;
    stats->tx_packets    = 0;
    stats->rx_packets    = 0;
    stats->rx_errors     = 0;
    stats->rx_dropped    = 0;
    stats->rx_crc_errors = 0;

    return stats;
}

static inline osal_s32 oal_net_device_ioctl(oal_net_device_stru *dev, oal_ifreq_stru *ifr, osal_s32 ul_cmd)
{
    unref_param(dev);
    unref_param(ifr);
    unref_param(ul_cmd);
    return -OAL_EINVAL;
}

static inline osal_s32 oal_net_device_change_mtu(oal_net_device_stru *dev, osal_s32 mtu)
{
    dev->mtu = (osal_u32)mtu;
    return OAL_SUCC;
}

#ifdef _PRE_SKB_TRACE
#define oal_netif_rx(pst_netbuf)                 \
({                                                  \
    mem_trace_delete_node((osal_ulong)(pst_netbuf)); \
    netif_rx(pst_netbuf);                        \
})
#else
static inline osal_s32 oal_netif_rx(oal_netbuf_stru *pst_netbuf)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return (osal_s32)hwal_netif_rx(pst_netbuf->dev, pst_netbuf);
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return netif_rx(pst_netbuf);
#endif
}
#endif

oal_bool_enum_uint8 oal_netbuf_is_dhcp_port_etc(const oal_udp_header_stru *udp_hdr);
oal_bool_enum_uint8 oal_netbuf_is_dhcp6_etc(oal_ipv6hdr_stru *ipv6hdr);
oal_net_device_stru* oal_net_alloc_netdev(const td_char *name, td_u8 max_name_len);
td_void oal_net_free_netdev(oal_net_device_stru *netdev);
td_u32 oal_net_register_netdev(oal_net_device_stru* netdev);
td_void oal_net_unregister_netdev(oal_net_device_stru* netdev);

oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6_etc(oal_ipv6hdr_stru  *ipv6hdr);
osal_u16 oal_netbuf_select_queue_etc(oal_netbuf_stru *buf);
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack_etc(oal_ip_header_stru  *ip_hdr);
oal_bool_enum_uint8 oal_netbuf_is_icmp_etc(oal_ip_header_stru  *ip_hdr);

/* ws61 */
oal_net_device_stru* oal_get_past_net_device_by_index(td_u32 netdev_index);
td_void oal_set_past_net_device_by_index(td_u32 netdev_index, oal_net_device_stru *netdev);
oal_net_device_stru* oal_get_netdev_by_name(const td_char* pc_name);
oal_net_device_stru* oal_get_netdev_by_type(nl80211_iftype_uint8 type);
td_u32 oal_net_check_and_get_devname(nl80211_iftype_uint8 type, char* dev_name, td_u32* len);
td_void oal_net_tx_wake_all_queues(const oal_net_device_stru *dev);


#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of file */
