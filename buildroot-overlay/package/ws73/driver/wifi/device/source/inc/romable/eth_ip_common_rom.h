/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: oam log printing interface
 */

#ifndef __ETH_IP_COMMON_ROM_H__
#define __ETH_IP_COMMON_ROM_H__

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_IPV4_PERMANET_GROUP_ADDR 0x000000E0

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN          6 /* length of an Ethernet address */
#endif
#define ETH_SENDER_IP_ADDR_LEN  4 /* length of an Ethernet send ip address */
#define ETH_TARGET_IP_ADDR_LEN  4 /* length of an Ethernet target ip address */

struct oal_ether_header {
    osal_u8 ether_dhost[ETHER_ADDR_LEN];
    osal_u8 ether_shost[ETHER_ADDR_LEN];
    osal_u16 ether_type;
};
typedef struct oal_ether_header oal_ether_header_stru;

/*
 * Structure of the IP frame
 */
struct oal_ip_header {
/* LITTLE_ENDIAN */
#if defined(_PRE_LITTLE_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) && (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)
    osal_u8      ihl : 4,
                 version_ihl : 4;
#else
    osal_u8      version_ihl : 4,
                 ihl : 4;
#endif
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
typedef struct oal_ip_header oal_ip_header_stru;

/* WIN32和linux共用结构体  */
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

typedef struct {
    osal_u16 sport;
    osal_u16 dport;
    osal_u32 seqnum;
    osal_u32 acknum;
    osal_u8 offset;
    osal_u8 flags;
    osal_u16 window;
    osal_u16 check;
    osal_u16 urgent;
} oal_tcp_header_stru;

typedef struct {
    osal_u16 source;
    osal_u16 dest;
    osal_u16 len;
    osal_u16 check;
} oal_udp_header_stru;

typedef struct {
    union {
        osal_u8 u6_addr8[16];       /* 数组大小为16 */
        osal_u16 u6_addr16[8];      /* 数组大小为8 */
        osal_u32 u6_addr32[4];      /* 数组大小为4 */
    } in6_u;
#define S6_ADDR in6_u.u6_addr8
#define S6_ADDR16 in6_u.u6_addr16
#define S6_ADDR32 in6_u.u6_addr32
} oal_in6_addr;

typedef struct {
    osal_u8 version : 4, priority : 4;
    osal_u8 flow_lbl[3];              /* 数组大小为3 */
    osal_u16 payload_len;

    osal_u8 nexthdr;
    osal_u8 hop_limit;

    oal_in6_addr saddr;
    oal_in6_addr daddr;
} oal_ipv6hdr_stru;

/* IPv6伪首部，用于校验和计算（伪首部中下一首部字段的值必须为58） */
typedef struct {
    oal_in6_addr saddr;
    oal_in6_addr daddr;

    osal_u32 payload_len;
    osal_u32 nexthdr;
} oal_ipv6_pseudo_hdr_stru;

typedef struct {
    osal_u8 nd_opt_type;
    osal_u8 nd_opt_len;
} oal_nd_opt_hdr;

typedef struct {
    osal_u32 reserved : 5, override : 1, solicited : 1, router : 1, reserved2 : 24;
} icmpv6_nd_advt;

typedef struct {
    osal_u8 icmp6_type;
    osal_u8 icmp6_code;
    osal_u16 icmp6_cksum;
    union {
        osal_u32 un_data32[1];           /* 1个uint32位 */
        osal_u16 un_data16[2];          /* 2个short32位 */
        osal_u8 un_data8[4];            /* 4字节32位 */
        icmpv6_nd_advt u_nd_advt;
    } icmp6_dataun;

#define icmp6_solicited icmp6_dataun.u_nd_advt.solicited
#define icmp6_override icmp6_dataun.u_nd_advt.override
} oal_icmp6hdr_stru;

/* 多了4字节，记得减去4 */
typedef struct {
    oal_icmp6hdr_stru icmph;
    oal_in6_addr target;
    osal_u8 opt[1];
    osal_u8 rsv[7];           /* 预留字节 7 bytes en_is_dad 为false的时候需要填写mac地址 */
} oal_nd_msg_stru;

/* 表示wiphy 结构中对应的参数是否被修改 */
enum wiphy_params_flags {
    WIPHY_PARAM_RETRY_SHORT = 1 << 0,
    WIPHY_PARAM_RETRY_LONG = 1 << 1,
    WIPHY_PARAM_FRAG_THRESHOLD = 1 << 2,
    WIPHY_PARAM_RTS_THRESHOLD = 1 << 3,
    WIPHY_PARAM_COVERAGE_CLASS = 1 << 4,
};
typedef enum wiphy_params_flags oal_wiphy_params_flags;
/*****************************************************************************
 函 数 名  : ether_is_multicast
 功能描述  : 判断一个地址是否为组播地址
*****************************************************************************/
static inline td_u8 ether_is_multicast(const osal_u8 *addr)
{
    return ((*addr) & 0x01);
}

/*****************************************************************************
 函 数 名  : ether_is_broadcast
 功能描述  : 判断一个地址是否为广播地址
*****************************************************************************/
static inline td_u8 ether_is_broadcast(const osal_u8 *addr)
{
    return ((addr[0] == 0xff) && (addr[1] == 0xff) && (addr[2] == 0xff) && /* 地址第0:1:2位 */
        (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff)); /* 地址第3:4:5位 */
}

/*****************************************************************************
 函 数 名  : ether_is_ipv4_multicast
 功能描述  : 判断一个地址是否为IPv4组播地址
*****************************************************************************/
static inline td_u8 ether_is_ipv4_multicast(const osal_u8 *addr)
{
    return ((addr[0] == 0x01) && (addr[1] == 0x00) && (addr[2] == 0x5e)); /* 地址第0:1:2位 */
}

/*****************************************************************************
 函 数 名  : oal_ipv4_is_permanent_group
 功能描述  : 判断一个IPv4地址是否为永久组地址:224.0.0.0～224.0.0.255
*****************************************************************************/
static inline td_u8 oal_ipv4_is_permanent_group(osal_u32 addr)
{
    return (((addr & 0x00FFFFFF) ^ OAL_IPV4_PERMANET_GROUP_ADDR) == 0);
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_multicast
 功能描述  : 判断一个地址是否为IPv6组播地址:FFXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX
*****************************************************************************/
static inline td_u8 oal_ipv6_is_multicast(const osal_u8 *addr)
{
    return ((osal_u8)(addr[0]) == 0xff);
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_unspecified_addr
 功能描述  : 判断一个地址是否为IPv6未指定地址
           IPv6未指定地址: ::/128 ,该地址仅用于接口还没有被分配IPv6地址时
           与其它节点通讯作为源地址,例如在重复地址检测DAD中会出现.
*****************************************************************************/
static inline td_u8 oal_ipv6_is_unspecified_addr(const osal_u8 *addr)
{
    return ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00) && /* 0:1:2:3 地址索引值 */
        (addr[4] == 0x00) && (addr[5] == 0x00) && (addr[6] == 0x00) && (addr[7] == 0x00) && /* 4,5,6,7 地址索引值 */
        (addr[8] == 0x00) && (addr[9] == 0x00) && (addr[10] == 0x00) && (addr[11] == 0x00) && /* 8,9,10,11 地址索引值 */
        (addr[12] == 0x00) && (addr[13] == 0x00) && (addr[14] == 0x00) && (addr[15] == 0x00)); /* 12,13,14,15 地址索引 */
}

/*****************************************************************************
 函 数 名  : oal_ipv6_is_link_local_addr
 功能描述  : 判断一个地址是否为IPv6链路本地地址.最高10位值为1111111010.
           例如:FE80:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX
*****************************************************************************/
static inline td_u8 oal_ipv6_is_link_local_addr(const osal_u8 *addr)
{
    return ((addr[0] == 0xFE) && ((addr[1] >> 6) == 2)); /* 6:地址右偏移6位 2:表示数值2 */
}

/*****************************************************************************
 函 数 名  : oal_csum_ipv6_magic
 功能描述  : 计算ICMPv6校验和
 输入参数  : osal_u32 len
             osal_u8 *buffer
 输出参数  : 无
 返 回 值  : static inline osal_u16
*****************************************************************************/
static inline osal_u16 oal_csum_ipv6_magic(osal_u32 len, osal_u8 *buffer)
{
    osal_u32 cksum = 0;
    osal_u16 *p = (osal_u16 *)buffer;
    osal_u32 size = (len >> 1) + (len & 0x1);

    while (size > 0) {
        cksum += *p;
        p++;
        size--;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff); /* 16:右偏移16位 */
    cksum = (cksum >> 16) + (cksum & 0xffff); /* 16:右偏移16位 */

    return (osal_u16)(~cksum);
}

static inline osal_u8 oal_is_broadcast_ether_addr(const osal_u8 *addr)
{
    return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff; /* 0,1,2,3,4,5 地址索引值 */
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of eth_ip_common.h */
