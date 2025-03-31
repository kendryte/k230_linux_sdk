/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header file for netbuf adapter.
 * Create: 2022-04-06
 */

#ifndef OAL_NETBUFF_EXT_H
#define OAL_NETBUFF_EXT_H

#include "oal_skbuff.h"
#include "td_type.h"
#include "securec.h"
#include "oal_netbuf_common_rom.h"
#include "oal_types.h"

/* oal_netbuf.h */
#define ETH_P_CONTROL                       0x0016      /* Card specific control frames */
#define OAL_NETBUF_SUCC                     0
#define OAL_NETBUF_FAIL                     (-1)


#define OAL_NETBUF_NEXT(_pst_buf)                   ((_pst_buf)->next)
#define OAL_NETBUF_PREV(_pst_buf)                   ((_pst_buf)->prev)
#define OAL_NETBUF_HEAD_NEXT(_pst_buf_head)         ((_pst_buf_head)->next)
#define OAL_NETBUF_HEAD_PREV(_pst_buf_head)         ((_pst_buf_head)->prev)

#define OAL_NETBUF_PROTOCOL(_pst_buf)               ((_pst_buf)->protocol)
#define OAL_NETBUF_DATA(_pst_buf)                   ((_pst_buf)->data)
#define OAL_NETBUF_HEADER(_pst_buf)                 ((_pst_buf)->data)
#define OAL_NETBUF_PAYLOAD(_pst_buf)                 ((_pst_buf)->data)

#define OAL_NETBUF_CB(_pst_buf)                     ((_pst_buf)->cb)
#define OAL_NETBUF_CB_SIZE()                        (sizeof(((oal_netbuf_stru*)0)->cb))
#define OAL_NETBUF_LEN(_pst_buf)                    ((_pst_buf)->len)
#define OAL_NETBUF_PRIORITY(_pst_buf)               ((_pst_buf)->priority)
#define OAL_NETBUF_TAIL                              skb_tail_pointer
#define OAL_NETBUF_QUEUE_TAIL                        skb_queue_tail
#define OAL_NETBUF_QUEUE_HEAD_INIT                   skb_queue_head_init
#define OAL_NETBUF_DEQUEUE                           skb_dequeue

#define WLAN_MEM_NETBUF_ALIGN               4       /* netbuf对齐 */

oal_netbuf_stru* oal_netbuf_alloc(td_u32 size, td_u32 l_reserve, td_u32 align);
td_u32 oal_netbuf_free(oal_netbuf_stru *netbuf);
td_void oal_netbuf_free_any(oal_netbuf_stru *netbuf);

#define OAL_MEM_NETBUF_ALLOC(subpool_id, len, netbuf_priority) \
    oal_netbuf_alloc(len, 0, WLAN_MEM_NETBUF_ALIGN)

typedef struct {
    oal_netbuf_head_stru                  buff_header;   /* netbuf队列头结点 */
    osal_u32                              mpdu_cnt;       /* 队列中报文数量 */
} oal_netbuf_queue_header_stru;

static inline oal_netbuf_stru* oal_netbuf_alloc_ext(oal_netbuf_id_enum_uint8 netbuf_id,
    td_u32 size, oal_netbuf_priority_enum_uint8 pri)
{
    td_u32 extend_len = OAL_HDR_TOTAL_LEN;
    unref_param(netbuf_id);
    unref_param(pri);
    return oal_netbuf_alloc(size + extend_len, OAL_HDR_TOTAL_LEN - OAL_MAX_MAC_HDR_LEN, WLAN_MEM_NETBUF_ALIGN);
}

static inline td_u8* oal_netbuf_payload(const oal_netbuf_stru *netbuf)
{
    return netbuf->data;
}

static inline const td_u8* oal_netbuf_payload_const(const oal_netbuf_stru *netbuf)
{
    return netbuf->data;
}

static inline td_u8* oal_netbuf_cb(const oal_netbuf_stru *netbuf)
{
    return (td_u8 *)netbuf->cb;
}

static inline const td_u8* oal_netbuf_cb_const(const oal_netbuf_stru *netbuf)
{
    return (td_u8 *)netbuf->cb;
}

static inline td_u8* oal_netbuf_data(oal_netbuf_stru *netbuf)
{
    return netbuf->data;
}

static inline td_u8* oal_netbuf_data_offset(oal_netbuf_stru *netbuf, td_u32 offset)
{
    return netbuf->data + offset;
}

static inline td_u8* oal_netbuf_header(const oal_netbuf_stru *netbuf)
{
    return netbuf->data;
}

static inline const td_u8* oal_netbuf_header_const(const oal_netbuf_stru *netbuf)
{
    return netbuf->data;
}

/*****************************************************************************
功能描述  : 获取将skb的数据长度
*****************************************************************************/
static inline td_u32  oal_netbuf_get_len(oal_netbuf_stru *netbuf)
{
    return netbuf->len;
}

static inline td_void oal_netbuf_copy_queue_mapping(oal_netbuf_stru *netbuf_to,
    const oal_netbuf_stru *netbuf_from)
{
    skb_copy_queue_mapping(netbuf_to, netbuf_from);
}


/*****************************************************************************
功能描述  : 在缓冲区尾部增加数据
*****************************************************************************/
static inline td_u8* oal_netbuf_put(oal_netbuf_stru *netbuf, td_u32 len)
{
    return skb_put(netbuf, len);
}

/*****************************************************************************
功能描述  : 在缓冲区开头增加数据
*****************************************************************************/
static inline td_u8*  oal_netbuf_push(oal_netbuf_stru *netbuf, td_u32 len)
{
    return skb_push(netbuf, len);
}

static inline td_u8* oal_netbuf_pull(oal_netbuf_stru *netbuf, td_u32 len)
{
    if (len > netbuf->len) {
        return TD_NULL;
    }
    return skb_pull(netbuf, len);
}

/*****************************************************************************
 功能描述  : 链接skb list前一个成员--DMAC
*****************************************************************************/
static inline td_void oal_set_netbuf_prev(oal_netbuf_stru *netbuf, oal_netbuf_stru *prev)
{
    netbuf->prev = prev;
}

/*****************************************************************************
 功能描述  : 链接skb list下一个成员--DMAC
*****************************************************************************/
static inline td_void oal_set_netbuf_next(oal_netbuf_stru *netbuf,  oal_netbuf_stru *next)
{
    if (netbuf == TD_NULL) {
        return;
    } else {
        netbuf->next = next;
    }
}

/*****************************************************************************
 功能描述  : 取skb list下一个成员--DMAC
*****************************************************************************/
static inline oal_netbuf_stru* oal_get_netbuf_next(oal_netbuf_stru *netbuf)
{
    return netbuf->next;
}

/*****************************************************************************
 功能描述  : 判断skb list是否为空--DMAC
*****************************************************************************/
static inline td_s32  oal_netbuf_list_empty(const oal_netbuf_head_stru* list_head)
{
    return skb_queue_empty(list_head);
}

/*****************************************************************************
 功能描述  : 将报文结构体的data指针和tail指针同时下移
*****************************************************************************/
static inline td_void  oal_netbuf_reserve(oal_netbuf_stru *netbuf, td_u32 l_len)
{
    skb_reserve(netbuf, l_len);
}

/*****************************************************************************
 功能描述  : 判断一个skb是否为克隆的，是则copy一份新的skb，否则直接返回传入的skb
*****************************************************************************/
static inline oal_netbuf_stru* oal_netbuf_unshare(oal_netbuf_stru *netbuf, td_u32 pri)
{
    return skb_unshare(netbuf, pri);
}

/*****************************************************************************
 功能描述  : 获取头部空间大小
*****************************************************************************/
static inline td_u32  oal_netbuf_headroom(const oal_netbuf_stru *netbuf)
{
    return (td_u32)skb_headroom(netbuf);
}

/*****************************************************************************
 功能描述  : 获取尾部空间大小
*****************************************************************************/
static inline td_u32  oal_netbuf_tailroom(const oal_netbuf_stru *netbuf)
{
    return (td_u32)skb_tailroom(netbuf);
}

/*****************************************************************************
 功能描述  : 将skb加入skb链表中的尾部--DMAC
*****************************************************************************/
static inline td_void  oal_netbuf_add_to_list_tail(oal_netbuf_stru *netbuf, oal_netbuf_head_stru* head)
{
    skb_queue_tail(head, netbuf);
}

/*****************************************************************************
 功能描述  : skb链表出队--DMAC
*****************************************************************************/
static inline oal_netbuf_stru* oal_netbuf_delist(oal_netbuf_head_stru *list_head)
{
    return skb_dequeue(list_head);
}

/*****************************************************************************
 功能描述  : skb链表取出其中skb--DMAC
*****************************************************************************/
static inline void oal_netbuf_unlink(oal_netbuf_stru *netbuf, oal_netbuf_head_stru *list_head)
{
    skb_unlink(netbuf, list_head);
}

/*****************************************************************************
 功能描述  : 初始化skb队列头--DMAC
*****************************************************************************/
static inline td_void  oal_netbuf_list_head_init(oal_netbuf_head_stru* list_head)
{
    skb_queue_head_init(list_head);
}

/*****************************************************************************
 功能描述  : 返回链表中指定节点的下一个节点

*****************************************************************************/
static inline oal_netbuf_stru* oal_netbuf_list_next(const oal_netbuf_stru *netbuf)
{
    return netbuf->next;
}
/*****************************************************************************
 功能描述  : add a netbuf to skb list

*****************************************************************************/
static inline td_void oal_netbuf_list_tail(oal_netbuf_head_stru* list, oal_netbuf_stru *netbuf)
{
    skb_queue_tail(list, netbuf);
}

static inline td_void oal_netbuf_list_tail_nolock(oal_netbuf_head_stru *list, oal_netbuf_stru *newsk)
{
    __skb_queue_tail(list, newsk);
}

/*****************************************************************************
 功能描述  : join two skb lists and reinitialise the emptied list
*****************************************************************************/
static inline td_void oal_netbuf_splice_init(oal_netbuf_head_stru* list, oal_netbuf_head_stru* head)
{
    skb_queue_splice_init(list, head);
}

/*****************************************************************************
 功能描述  :  join two skb lists and reinitialise the emptied list
*****************************************************************************/
static inline td_void oal_netbuf_queue_splice_tail_init(oal_netbuf_head_stru* list, oal_netbuf_head_stru* head)
{
    skb_queue_splice_tail_init(list, head);
}

/*****************************************************************************
功能描述  : init netbuf list
*****************************************************************************/
static inline td_void oal_netbuf_head_init(oal_netbuf_head_stru *list)
{
    skb_queue_head_init(list);
}

/*****************************************************************************
功能描述  : 返回skb链表中的第一个元素--DMAC
*****************************************************************************/
static inline oal_netbuf_stru* oal_netbuf_peek(const oal_netbuf_head_stru *head)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    return skb_peek(head);
#else
    oal_netbuf_stru *skb = head->next;
    if (skb == (oal_netbuf_stru *)head)
        skb = NULL;
    return skb;
#endif
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return skb_peek(head);
#endif
}

/* ****************************************************************************
功能描述  : 返回skb链表中的第一个元素的下一个元素--HMAC
**************************************************************************** */
static inline oal_netbuf_stru* oal_netbuf_peek_next(oal_netbuf_stru* netbuf, oal_netbuf_head_stru *head)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
    return skb_peek_next(netbuf, head);
#else
    oal_netbuf_stru *next = netbuf->next;
    if (next == (oal_netbuf_stru *)head)
        next = NULL;
    return next;
#endif
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return skb_peek_next(netbuf, head);
#endif
}

/*****************************************************************************
功能描述  : 返回skb链表中的最后一个元素
*****************************************************************************/
static inline oal_netbuf_stru* oal_netbuf_tail(const oal_netbuf_head_stru *head)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    return skb_peek_tail(head);
#else
    oal_netbuf_stru *skb = head->prev;
    if (skb == (oal_netbuf_stru *)head)
        skb = NULL;
    return skb;
#endif
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return skb_peek_tail(head);
#endif
}

/*****************************************************************************
功能描述  : 获取netbuf双向链表中buf的个数
*****************************************************************************/
static inline td_u32 oal_netbuf_get_buf_num(const oal_netbuf_head_stru *netbuf_head)
{
    return netbuf_head->qlen;
}

/*****************************************************************************
功能描述  : 将skb中的内容先偏移ul_offset后 按指定长度拷贝到指定内存中
*****************************************************************************/
static inline td_u32 oal_netbuf_copydata(const oal_netbuf_stru *netbuf, td_u32 offset, td_void *dst,
    td_u32 dst_len, td_u32 len)
{
    const td_void *scr = oal_netbuf_payload_const(netbuf) + offset;

    if (memcpy_s(dst, dst_len, scr, len) != EOK) {
        return OAL_NETBUF_FAIL;
    }

    return OAL_NETBUF_SUCC;
}

/*****************************************************************************
功能描述  : 剥去skb中尾部的信息
*****************************************************************************/
static inline td_void oal_netbuf_trim(oal_netbuf_stru *netbuf, td_u32 len)
{
    skb_trim(netbuf, netbuf->len - len);
}

/*****************************************************************************
功能描述  : 将skb的数据内容长度设置为指定长度
*****************************************************************************/
static inline td_void oal_netbuf_set_len(oal_netbuf_stru *netbuf, td_u32 len)
{
    if (netbuf->len > len) {
        skb_trim(netbuf, len);
    } else {
        skb_put(netbuf, (len - netbuf->len));
    }
}

/*****************************************************************************
功能描述  : 初始化netbuf
*****************************************************************************/
static inline td_void oal_netbuf_init(oal_netbuf_stru *netbuf, td_u32 len)
{
    oal_netbuf_set_len(netbuf, len);
    netbuf->protocol = ETH_P_CONTROL;
}

/*****************************************************************************
功能描述: 获取netbuf 链表长度--DMAC
*****************************************************************************/
static inline td_u32 oal_netbuf_list_len(const oal_netbuf_head_stru *head)
{
    return skb_queue_len(head);
}

/*****************************************************************************
 功能描述  : 删除链表中的skb.(只有链表头有next 和 pre，所以只能依次从头删)
*****************************************************************************/
static inline td_void oal_netbuf_delete(oal_netbuf_stru *buf, oal_netbuf_head_stru *list_head)
{
    skb_unlink(buf, list_head);
}

/* oal_netbuf.c */
/*****************************************************************************
 功能描述  : 获取当前netbuf元素后的第n个元素
 输入参数  : (1)起始查找节点
             (2)向后查找的个数
 输出参数  : 指向期望的netbuf的指针
 返 回 值  : 期望的betbuf元素的指针或空指针
*****************************************************************************/
static inline td_u32 oal_netbuf_get_appointed_netbuf(oal_netbuf_stru *netbuf, td_u8 num,
    oal_netbuf_stru** expect_netbuf)
{
    td_u8 buf_num;

    if ((netbuf == TD_NULL) || (expect_netbuf == TD_NULL)) {
        return OAL_NETBUF_FAIL;
    }

    *expect_netbuf = TD_NULL;
    for (buf_num = 0; buf_num < num; buf_num++) {
        *expect_netbuf = oal_get_netbuf_next(netbuf);

        if (*expect_netbuf == TD_NULL) {
            break;
        }

        netbuf = *expect_netbuf;
    }

    return OAL_NETBUF_SUCC;
}

/*****************************************************************************
 功能描述  : 向netbu_head的尾部串接netbuf
*****************************************************************************/
static inline td_u32 oal_netbuf_concat(oal_netbuf_stru* netbuf_head, oal_netbuf_stru* netbuf)
{
    if (skb_is_nonlinear(netbuf_head)) {
        oal_netbuf_free(netbuf);
        return OAL_NETBUF_FAIL;
    }

    if (skb_tailroom(netbuf_head) < netbuf->len) {
        oal_netbuf_free(netbuf);
        return OAL_NETBUF_FAIL;
    }

    if (memcpy_s(skb_tail_pointer(netbuf_head), netbuf->len, netbuf->data, netbuf->len)  != EOK) {
        oal_netbuf_free(netbuf);
        return OAL_NETBUF_FAIL;
    }

    skb_put(netbuf_head, netbuf->len);
    oal_netbuf_free(netbuf);
    return OAL_NETBUF_SUCC;
}

/* oal_net_rom.h */
/*****************************************************************************
 功能描述  : 将skb加入skb链表(device单向链表)中的头部
*****************************************************************************/
static inline td_void oal_netbuf_addlist(oal_netbuf_head_stru *head, oal_netbuf_stru *netbuf)
{
    skb_queue_head(head, netbuf);
}

/* oal_net.h */
static inline td_s32 oal_netbuf_expand_head(oal_netbuf_stru *netbuf, td_s32 nhead, td_s32 ntail, td_s32 gfp_mask)
{
    return pskb_expand_head(netbuf, (td_u32)nhead, (td_u32)ntail, gfp_mask);
}

/* get the queue index of the input skbuff */
static inline td_u16 oal_skb_get_queue_mapping(oal_netbuf_stru *netbuf)
{
    return skb_get_queue_mapping(netbuf);
}

static inline td_void oal_skb_set_queue_mapping(oal_netbuf_stru *netbuf, td_u16 queue_mapping)
{
    skb_set_queue_mapping(netbuf, queue_mapping);
}

static inline td_u32 oal_notice_sta_join_result(td_u8 chip_id, td_u8 succ)
{
    unref_param(chip_id);
    unref_param(succ);
    return OAL_NETBUF_SUCC;
}

static inline td_u32 oal_netbuf_reserve_header(oal_netbuf_stru *netbuf)
{
    td_u32 header_len = oal_netbuf_headroom(netbuf);
    if (header_len < (td_u32)OAL_HDR_TOTAL_LEN) {
        return (td_u32)oal_netbuf_expand_head(netbuf, (osal_s32)(OAL_HDR_TOTAL_LEN - header_len), 0, 0);
    }
    return OAL_NETBUF_SUCC;
}
#endif
