/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file for oal_net_pkt_rom
 */
#ifndef OAL_NET_PKT_ROM_H
#define OAL_NET_PKT_ROM_H
#include "osal_types.h"
#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
typedef enum {
    PKT_BUF_SRC_DMAC = 0,
    PKT_BUF_SRC_LWIP,

    PKT_BUF_SRC_BUTT
} oal_pkt_buf_src_type;
#endif

/*****************************************************************************
  ws63共核编译，提取device/frw/romable/oal_mem_rom.h和
  host\inc\oal\oal_netbuf_ext.h中完全相同的枚举结构体。
*****************************************************************************/
/*****************************************************************************
  枚举名  : oal_netbuf_id_enum_uint8
  协议表格:
  枚举说明: 对外提供netbuf内存池ID
*****************************************************************************/
typedef enum {
    OAL_NORMAL_NETBUF = 0,                /* 包括短帧、长帧内存池 */
    OAL_MGMT_NETBUF   = 1,                /* 管理帧内存池 */

    OAL_NETBUF_POOL_ID_BUTT
}oal_netbuf_id_enum;
typedef osal_u8 oal_netbuf_id_enum_uint8;

/*****************************************************************************
  枚举名  : oal_netbuf_priority_enum_uint8
  协议表格:
  枚举说明: netbuf优先级
*****************************************************************************/
typedef enum {
    OAL_NETBUF_PRIORITY_LOW     = 0,  // 最低优先级,不能跨池申请
    OAL_NETBUF_PRIORITY_MID     = 1,  // 中间优先级，可以跨池申请，但不能申请大包的最后N片
    OAL_NETBUF_PRIORITY_HIGH    = 2,  // 最高优先级，可以跨池申请，且可以申请大包的最后N片

    OAL_NETBUF_PRIORITY_BUTT
}oal_netbuf_priority_enum;
typedef osal_u8 oal_netbuf_priority_enum_uint8;

#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
#define ZERO_COPY_PBUF_HDR_LEN      40    // = sizeof(struct pbuf)   36
#define ZERO_COPY_SKB_HDR_LEN       120   // = sizeof(oal_netbuf_stru),  116 rom化前编程可配置的方式
#define LWIP_ZERO_COPY_HDR    (ZERO_COPY_PBUF_HDR_LEN + ZERO_COPY_SKB_HDR_LEN)
#else
#define ZERO_COPY_PBUF_HDR_LEN      0
#define ZERO_COPY_SKB_HDR_LEN       0
#define LWIP_ZERO_COPY_HDR   0
#endif

#define OAL_MEM_INFO_SIZE 4

#define OAL_MACHDR_LEN 80 /* netbuf mac头长度 */
#define MAC_HEAD_OFFSET (OAL_MACHDR_LEN - OAL_MAX_MAC_HDR_LEN)
#define CB_OFFSET (OAL_MACHDR_LEN - OAL_MAX_MAC_HDR_LEN - OAL_MAX_CB_LEN)

#define WLAN_MEM_MAX_SUBPOOL_NUM 8 /* 内存池中出netbuf内存池外最大子内存池个数 */

/*****************************************************************************
  枚举名  : oal_mem_state_enum_uint8
  协议表格:
  枚举说明: 内存块状态
*****************************************************************************/
typedef enum {
    OAL_MEM_STATE_FREE = 0, /* 该内存空闲 */
    OAL_MEM_STATE_ALLOC,    /* 该内存已分配 */
    OAL_MEM_STATE_BUTT
} oal_mem_state_enum;
typedef osal_u8 oal_mem_state_enum_uint8;

/*****************************************************************************
   结构名  : oal_dmac_netbuf_stru
   结构说明: 描述每一个netbuf内存块信息
 *****************************************************************************/
/* netbuf_stru加dmac */
struct oal_dmac_netbuf_stru_tag {
    struct oal_dmac_netbuf_stru_tag  *next;
    oal_mem_state_enum_uint8    mem_state;              /* 内存块状态 */
    osal_u8                       is_high_priority : 1;   /* 是否是被高优先级申请 */
    osal_u8                       subpool_id : 3;         /* 记录所属子池id */
    osal_u8                       resv : 4;
    osal_u16                      index;                  /* 序号 */
#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
    osal_s16                      pkt_buf_offset;
    osal_u8                       pkt_src;
#endif
#ifdef _PRE_MEM_DEBUG
    osal_u32                      return_addr;            /* 调用alloc内存的函数地址 */
#endif
};
typedef struct oal_dmac_netbuf_stru_tag oal_dmac_netbuf_stru;

/* netbuf_stru加dmac */
typedef struct {
    oal_dmac_netbuf_stru *next;
    oal_dmac_netbuf_stru *prev;
    osal_u32 num;
    osal_u8 auc_resv[4]; /* 此处添加4个保留字节是为了让整个结构体与linux下大小一致 */
} oal_dmac_netbuf_head_stru;

oal_dmac_netbuf_stru *oal_mem_netbuf_alloc(osal_u8 netbuf_id, osal_u16 len, oal_netbuf_priority_enum_uint8 flag);
osal_void oal_mem_netbuf_free(oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_dmac_netbuf_end(const oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_dmac_netbuf_payload(const oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_netbuf_hcchdr(const oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_netbuf_mac_header(const oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_dmac_netbuf_cb(const oal_dmac_netbuf_stru *netbuf);
#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
osal_u8 *oal_netbuf_lwip(const oal_dmac_netbuf_stru *netbuf);
osal_u8 *oal_netbuf_skb(const oal_dmac_netbuf_stru *netbuf);
#endif

/* ****************************************************************************
 功能描述  : 获取skb数据头部(偏移mac头后的数据)
**************************************************************************** */
static inline osal_u8 *oal_dmac_netbuf_data(const oal_dmac_netbuf_stru *netbuf)
{
    osal_u8 *payload = oal_dmac_netbuf_payload(netbuf);
    return (payload + OAL_MACHDR_LEN);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_net_pkt_rom.h */

