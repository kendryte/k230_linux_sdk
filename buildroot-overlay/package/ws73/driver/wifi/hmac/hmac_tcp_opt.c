/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: TCP Processing.
 * Create: 2021-12-15
 */

#ifdef _PRE_WLAN_TCP_OPT

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tcp_opt.h"
#include <linux/jiffies.h>
#include "oam_ext_if.h"
#include "hmac_tcp_opt_struc.h"
#include "hcc_host_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "mac_data.h"
#include "oal_netbuf.h"
#include "oal_types.h"
#include "hmac_rx_data.h"
#include "hmac_feature_dft.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TCP_OPT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 结构体定义
*****************************************************************************/

/*****************************************************************************
  3 宏定义
*****************************************************************************/

/* defined for ut test */
#if defined(WIN32)
osal_u32 jiffies;

oal_bool_enum_uint8 time_before_eq(osal_u32 a, osal_u32 b)
{
    return OAL_TRUE;
}
#endif

/*****************************************************************************
  4 全局变量定义
*****************************************************************************/


/*****************************************************************************
  5 内部静态函数声明
*****************************************************************************/

/*****************************************************************************
  4 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_tcp_opt_ack_count_reset_etc
 功能描述  : 复位TCP ACK的统计值
 输入参数  : struct hcc_handler *hcc
             hcc_chan_type dir
             osal_u16 stream
*****************************************************************************/
void hmac_tcp_opt_ack_count_reset_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir, osal_u16 stream)
{
    if (!hmac_vap) {
        wifi_printf("%s error:hmac_vap is null", __FUNCTION__);
        return;
    };

    hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[stream] = 0;
    hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_duplicate_ack_count[stream] = 0;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_find_oldest_node_etc
 功能描述  : 查找时间最旧的node
 输入参数  : struct hcc_handler *hcc
             hcc_chan_type dir
*****************************************************************************/
struct tcp_list_node *hmac_tcp_opt_find_oldest_node_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node;
    struct tcp_list_node *oldest_node = NULL;
    osal_ulong  oldest_time = jiffies;  /* init current time */
    struct wlan_perform_tcp_list *tmp_list;
    struct osal_list_head        *entry;

    tmp_list = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list;
    osal_list_for_each(entry, &tmp_list->tcp_list) {
        node = osal_list_entry(entry, struct tcp_list_node, list);
        if (time_before_eq(node->last_ts, oldest_time) == OAL_TRUE) {
            oldest_time = node->last_ts;
            oldest_node = node;
            oam_info_log3(0, OAM_SF_ANY, "{dir:%d find last_ts %ld   oldest_time %ld.}", dir,
                          node->last_ts,
                          oldest_time);
        }
    }
    if (oldest_node != NULL) {
        osal_list_del(&oldest_node->list);
        OSAL_INIT_LIST_HEAD(&oldest_node->list);
    } else {
        oam_error_log0(0, OAM_SF_ANY, "can't find oldest node xx");
    }
    return oldest_node;
}
/*****************************************************************************
 函 数 名  : hmac_tcp_opt_get_buf_etc
 功能描述  : get buf for tcp ack
 输入参数  : struct hcc_handler *hcc
             hcc_chan_type dir
*****************************************************************************/
struct tcp_list_node *hmac_tcp_opt_get_buf_etc(hmac_vap_stru    *hmac_vap, hcc_chan_type dir)
{
    struct tcp_list_node *node = NULL;
    osal_u16  tcp_stream_index;
    struct wlan_perform_tcp_list *tmp_ack_list = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list;

    if (tmp_ack_list->free_count == 0) {
        node = hmac_tcp_opt_find_oldest_node_etc(hmac_vap, dir);
        return node;
    }

    for (tcp_stream_index = 0; tcp_stream_index < HMAC_TCP_STREAM; tcp_stream_index++) {
        if (tmp_ack_list->tcp_pool[tcp_stream_index].used == 0) {
            tmp_ack_list->tcp_pool[tcp_stream_index].used = 1;
            tmp_ack_list->free_count--;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            wifi_printf("\r\n====dir:%d get buf %d free:%d====\r\n}",
                dir, tcp_stream_index, tmp_ack_list->free_count);
#endif
            node = &tmp_ack_list->tcp_pool[tcp_stream_index];
            break;
        }
    }
    return node;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_add_node_etc
 功能描述  : add node to ack list
 输入参数  : struct hcc_handler *hcc
             struct wlan_tcp_flow *tcp_info
             hcc_chan_type dir
*****************************************************************************/
osal_u32 hmac_tcp_opt_add_node_etc(hmac_vap_stru *hmac_vap, struct wlan_tcp_flow *tcp_info, hcc_chan_type dir)
{
    struct tcp_list_node *node;

    if ((hmac_vap == NULL) || (tcp_info == NULL)) {
        return OAL_FAIL;
    }

    node = hmac_tcp_opt_get_buf_etc(hmac_vap, dir);
    if (node == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "Invalid NULL node!");
        return OAL_FAIL;
    }

    node->wlan_tcp_info.dst_ip   = tcp_info->dst_ip;
    node->wlan_tcp_info.src_ip   = tcp_info->src_ip;
    node->wlan_tcp_info.src_port = tcp_info->src_port;
    node->wlan_tcp_info.dst_port = tcp_info->dst_port;
    node->wlan_tcp_info.protocol = tcp_info->protocol;
    node->last_ts       = jiffies;

    osal_list_add_tail(&node->list, &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list.tcp_list);

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    wifi_printf("\r\n====dir:%d,index = %d,add node succ====\r\n", dir, node->index);
#endif

    return node->index;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_init_filter_tcp_ack_pool_etc
 功能描述  : init tcp ack pool
 输入参数  : struct hcc_handler* hcc
*****************************************************************************/
osal_u32 hmac_tcp_opt_init_filter_tcp_ack_pool_etc(hmac_vap_stru *hmac_vap)
{
    osal_u16     dir_index;
    osal_u16     tcp_index;
    osal_u16     tcp_queue_index;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_init_filter_tcp_ack_poolr fail:hmac_vap is null}");
        return OAL_FAIL;
    }

    /* init downline tcp ack pool */

    /* init tx_worker_state */

    for (dir_index = 0; dir_index < HCC_DIR_COUNT; dir_index++) {
        osal_spin_lock_init(&hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack.hmac_tcp_ack_lock);
        for (tcp_index = 0; tcp_index < HMAC_TCP_STREAM; tcp_index++) {
            oal_netbuf_list_head_init(
                &hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack.hcc_ack_queue[tcp_index]);
            hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack_list.tcp_pool[tcp_index].used = 0;
            hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack_list.tcp_pool[tcp_index].index =
                tcp_index;
            hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack_list.tcp_pool[tcp_index].last_ts = jiffies;
        }
        for (tcp_queue_index = 0; tcp_queue_index < HMAC_TCP_OPT_QUEUE_BUTT; tcp_queue_index++) {
            osal_spin_lock_init(&hmac_vap->hmac_tcp_ack[dir_index].data_queue_lock[tcp_queue_index]);
            oal_netbuf_head_init(&hmac_vap->hmac_tcp_ack[dir_index].data_queue[tcp_queue_index]);
        }
        OSAL_INIT_LIST_HEAD(&(hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack_list.tcp_list));
        hmac_vap->hmac_tcp_ack[dir_index].hmac_tcp_ack_list.free_count = HMAC_TCP_STREAM;
        oam_info_log1(0, OAM_SF_ANY, "{wifi tcp perform dir:%d init done.}", dir_index);
    }
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    hmac_vap->hmac_tcp_ack[HCC_TX].filter[HMAC_TCP_ACK_QUEUE] = hmac_tcp_opt_tx_tcp_ack_filter_etc;
    hmac_vap->hmac_tcp_ack[HCC_RX].filter[HMAC_TCP_ACK_QUEUE] = hmac_tcp_opt_rx_tcp_ack_filter_etc;
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_free_ack_list_etc
 功能描述  : 删除vap时释放ack list
 输入参数  : struct hcc_handler *hcc
*****************************************************************************/
void hmac_tcp_opt_free_ack_list_etc(hmac_vap_stru *hmac_vap, osal_u8 dir, osal_u8 type)
{
#if !defined(WIN32)
    oal_netbuf_head_stru  head_t;
    oal_netbuf_head_stru *head;
    oal_netbuf_stru *pst_netbuf;
    oal_netbuf_head_stru *hcc_ack_queue;
    struct wlan_perform_tcp_list *tmp_list;
    struct osal_list_head        *entry;
    struct osal_list_head        *entry_temp;
    struct tcp_list_node       *node;

    oal_netbuf_head_init(&head_t);
    head = &hmac_vap->hmac_tcp_ack[dir].data_queue[type];
    oal_netbuf_queue_splice_tail_init(head, &head_t);
    while (!!(pst_netbuf = oal_netbuf_delist(&head_t))) {
        oal_netbuf_free(pst_netbuf);
    }

    tmp_list = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list;
    osal_list_for_each_safe(entry, entry_temp, &tmp_list->tcp_list) {
        node = osal_list_entry(entry, struct tcp_list_node, list);

        if (node->used == 0) {
            continue;
        }
        hcc_ack_queue = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[node->index];
        while (!!(pst_netbuf = oal_netbuf_delist(hcc_ack_queue))) {
            oal_netbuf_free(pst_netbuf);
        }
    }
#endif
}
/*****************************************************************************
 函 数 名  : hmac_tcp_opt_deinit_list_etc
 功能描述  : deinit tcp ack list
 输入参数  : struct hcc_handler *hcc
*****************************************************************************/
void hmac_tcp_opt_deinit_list_etc(hmac_vap_stru *hmac_vap)
{
    hmac_tcp_opt_free_ack_list_etc(hmac_vap, HCC_TX, HMAC_TCP_ACK_QUEUE);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    hmac_tcp_opt_free_ack_list_etc(hmac_vap, HCC_RX, HMAC_TCP_ACK_QUEUE);
#endif
}


/*****************************************************************************
 函 数 名  : hmac_tcp_opt_get_flow_index_etc
 功能描述  : get follow index for each tcp ack
 输入参数  : struct hcc_handler     *hcc
             oal_ip_header_stru  *ip_hdr
             oal_tcp_header_stru *tcp_hdr
             hcc_chan_type dir
*****************************************************************************/
osal_u32 hmac_tcp_opt_get_flow_index_etc(hmac_vap_stru       *hmac_vap,
    oal_ip_header_stru  *ip_hdr,
    oal_tcp_header_stru *tcp_hdr,
    hcc_chan_type        dir)
{
    struct wlan_tcp_flow   tcp_flow_info;
    struct tcp_list_node  *node;
    struct osal_list_head        *entry;
    struct wlan_perform_tcp_list *tmp_list;

    tcp_flow_info.src_ip   = ip_hdr->saddr;
    tcp_flow_info.dst_ip   = ip_hdr->daddr;
    tcp_flow_info.src_port = tcp_hdr->sport;
    tcp_flow_info.dst_port = tcp_hdr->dport;
    tcp_flow_info.protocol = ip_hdr->protocol;

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    wifi_printf("\r\n====hmac_tcp_opt_get_flow_index_etc enter====\r\n");
#endif
    /* get the queue index of tcp ack */
    tmp_list = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list;
    osal_list_for_each(entry, &tmp_list->tcp_list) {
        node = (struct tcp_list_node *)entry;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        wifi_printf("\r\n====dir:%d node [index:%d],used = %d====\r\n", dir, node->index, node->used);
#endif
        if ((node->wlan_tcp_info.src_ip == tcp_flow_info.src_ip)
            && (node->wlan_tcp_info.dst_ip == tcp_flow_info.dst_ip)
            && (node->wlan_tcp_info.src_port == tcp_flow_info.src_port)
            && (node->wlan_tcp_info.dst_port == tcp_flow_info.dst_port)
            && (node->wlan_tcp_info.protocol == tcp_flow_info.protocol)
            && (node->used == 1)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            wifi_printf("\r\n====dir:%d find the same tcp flow info [index:%d]====\r\n", dir, node->index);
#endif
            node->last_ts = jiffies; /* renew the last pkt time */
            return node->index;
        }
    }

    /* new follow, save in new node */

    return hmac_tcp_opt_add_node_etc(hmac_vap, &tcp_flow_info, dir);
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_get_tcp_ack_type_etc
 功能描述  : get follow index for each tcp ack
 输入参数  : hmac_vap_stru    *hmac_vap
             oal_ip_header_stru     *ip_hdr
             hcc_chan_type dir
             osal_u16 index
*****************************************************************************/
oal_tcp_ack_type_enum_uint8 hmac_tcp_opt_get_tcp_ack_type_etc(hmac_vap_stru      *hmac_vap,
    oal_ip_header_stru *ip_hdr,
    hcc_chan_type       dir,
    osal_u16          us_index)
{
    oal_tcp_header_stru *tcp_hdr;
    osal_u32 tcp_ack_no;
    osal_u32 *tmp_tcp_ack_no;
    osal_u32 ip_hdr_len;

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    wifi_printf("\r\n====hmac_tcp_opt_get_tcp_ack_type_etc:us_index = %d ====\r\n", us_index);
#endif
    ip_hdr_len = ip_hdr->ihl << 2; /* 2左移位数 */
    tcp_hdr     = (oal_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
    tcp_ack_no = tcp_hdr->acknum;

    /* 检测duplicat ack是否存在，如果存在则累计ack流最大成员数 */
    tmp_tcp_ack_no = hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_tcp_ack_no;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    wifi_printf("\r\n====stream:%d ack no:%u  tcp ack no:%u ====\r\n", us_index, tcp_ack_no, tmp_tcp_ack_no[us_index]);
#endif
    if (tcp_ack_no == tmp_tcp_ack_no[us_index]) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log1(0, OAM_SF_ANY, "{dir:%d:**duplicate ack is coming**.}", dir);
#endif
        hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[us_index].ull_dup_ack_count++;
        return TCP_ACK_DUP_TYPE;
    }

    tmp_tcp_ack_no[us_index] = tcp_hdr->acknum;

    /* 赋值tcp_cb */
    return TCP_ACK_FILTER_TYPE;
}

/*****************************************************************************
 函 数 名  : hmac_judge_rx_netbuf_is_tcp_ack_etc
 功能描述  : 判断接收帧是否是TCP_ACK帧
 输入参数  : mac_llc_snap *snap
*****************************************************************************/
oal_bool_enum_uint8 hmac_judge_rx_netbuf_is_tcp_ack_etc(mac_llc_snap_stru *snap)
{
    oal_ip_header_stru  *ip_hdr;
    oal_bool_enum_uint8 is_tcp_ack = OAL_FALSE;
    oal_tcp_header_stru    *tcp_hdr;
    osal_u32 ip_hdr_len;

    if (snap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_judge_rx_netbuf_is_tcp_ack_etc:  snap is null!}");
        return OAL_FALSE;
    }
    switch (snap->ether_type) {
        /* 屏蔽Info -- Constant expression evaluates to 0 in operation '&' */
        case oal_host2net_short(ETHER_TYPE_IP):

            ip_hdr = (oal_ip_header_stru *)(snap + 1);      /* 偏移一个snap，取ip头 */

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            oam_warning_log1(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:  ip_hdr->protocol = %d**!}",
                             ip_hdr->protocol);
#endif
            if ((ip_hdr->protocol == MAC_TCP_PROTOCAL) && (oal_netbuf_is_tcp_ack_etc(ip_hdr) == OAL_TRUE)) {
                ip_hdr_len = ip_hdr->ihl << 2; /* 2左移位数 */
                tcp_hdr = (oal_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
                /* option3:SYN FIN RST URG有为1的时候不过滤 */
                if ((tcp_hdr->flags) & FILTER_FLAG_MASK) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                    oam_warning_log0(0, OAM_SF_RX,
                        "{hmac_judge_rx_netbuf_is_tcp_ack_etc:  **specific tcp pkt, can't be filter**!}");
#endif
                } else {
                    is_tcp_ack = OAL_TRUE;
                }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                oam_warning_log0(0, OAM_SF_RX, "{oal_judge_rx_netbuf_is_tcp_ack:: tcp ack frame!}");
#endif
            }
            break;
        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            wifi_printf("{oal_judge_rx_netbuf_is_tcp_ack::unkown ether_type[%d]}\r\n", snap->ether_type);
#endif
            break;
    }

    return is_tcp_ack;
}

/*****************************************************************************
 函 数 名  : hmac_judge_rx_netbuf_classify_etc
 功能描述  : 接收帧进行分类
 输入参数  : oal_netbuf_stru* netbuf
*****************************************************************************/
oal_bool_enum_uint8 hmac_judge_rx_netbuf_classify_etc(oal_netbuf_stru *netbuff)
{
    mac_llc_snap_stru             *snap;

    snap = (mac_llc_snap_stru *)(netbuff);
    if (snap == OAL_PTR_NULL) {
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log1(0, OAM_SF_ANY,
        "**hmac_judge_rx_netbuf_classify_etc, ether_type = %d**", snap->ether_type);
#endif
    return hmac_judge_rx_netbuf_is_tcp_ack_etc(snap);
}


/*****************************************************************************
 函 数 名  : hmac_judge_tx_netbuf_is_tcp_ack_etc
 功能描述  : 判断接收帧是否是TCP_ACK帧
 输入参数  : oal_ether_header_stru *ps_ethmac_hdr
*****************************************************************************/
oal_bool_enum_uint8 hmac_judge_tx_netbuf_is_tcp_ack_etc(oal_ether_header_stru *ps_ethmac_hdr)
{
    oal_ip_header_stru     *pst_ip;
    oal_tcp_header_stru    *tcp_hdr;
    oal_bool_enum_uint8     is_tcp_ack = OAL_FALSE;

    if (ps_ethmac_hdr == OAL_PTR_NULL) {
        return OAL_FALSE;
    }
    switch (ps_ethmac_hdr->ether_type) {
        /* 屏蔽Info -- Constant expression evaluates to 0 in operation '&' */
        case oal_host2net_short(ETHER_TYPE_IP):

            pst_ip = (oal_ip_header_stru *)(ps_ethmac_hdr + 1);      /* 偏移一个snap，取ip头 */

            if ((pst_ip->protocol == MAC_TCP_PROTOCAL) && (oal_netbuf_is_tcp_ack_etc(pst_ip) == OAL_TRUE)) {
                tcp_hdr = (oal_tcp_header_stru *)(pst_ip + 1);
                /* option3:SYN FIN RST URG有为1的时候不过滤 */
                if ((tcp_hdr->flags) & FILTER_FLAG_MASK) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                    oam_warning_log0(0, OAM_SF_ANY, "**specific tcp pkt, can't be filter**");
#endif
                } else {
                    is_tcp_ack = OAL_TRUE;
                }
            }
            break;

        default:
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            wifi_printf("{hmac_judge_tx_netbuf_is_tcp_ack_etc::unkown ether_type[%d]}\r\n", ps_ethmac_hdr->ether_type);
#endif
            break;
    }

    return is_tcp_ack;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_get_tcp_ack
 功能描述  : get tcp ack from the skb data
 输入参数  : struct oal_netbuf_stru *skb
             struct hcc_handler *hcc
             osal_u16 *p_us_index
             osal_u8 dir
******************************************************************************/
oal_tcp_ack_type_enum_uint8 hmac_tcp_opt_rx_get_tcp_ack_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap,
    osal_u16 *p_us_index, osal_u8 dir)
{
    oal_ip_header_stru  *ip_hdr;
    oal_tcp_header_stru *tcp_hdr;
    mac_llc_snap_stru             *snap;
    mac_rx_ctl_stru                   *pst_rx_ctrl;                        /* 指向MPDU控制块信息的指针 */
    osal_u32 ip_hdr_len;

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(skb);
    snap = (mac_llc_snap_stru *)(skb->data + pst_rx_ctrl->mac_header_len);

    if (hmac_judge_rx_netbuf_is_tcp_ack_etc(snap) == OAL_FALSE) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        /* not tcp ack data */
        oam_info_log0(0, OAM_SF_ANY, "{**not tcp packet return direct**}\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    ip_hdr = (oal_ip_header_stru *)(snap + 1);      /* 偏移一个snap，取ip头 */
    ip_hdr_len = ip_hdr->ihl << 2;       /* 左移2bit:值对应4字节 */
    tcp_hdr     = (oal_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);

    /* option4:flow index取不到时不过滤 */
    *p_us_index =
        (osal_u16)hmac_tcp_opt_get_flow_index_etc(hmac_vap, ip_hdr, tcp_hdr, (hcc_chan_type)dir);
    if (*p_us_index == 0xFFFF) {
        return TCP_TYPE_ERROR;
    }

    return hmac_tcp_opt_get_tcp_ack_type_etc(hmac_vap, ip_hdr, (hcc_chan_type)dir, *p_us_index);
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_tx_get_tcp_ack_etc
 功能描述  : get tcp ack from the skb data
 输入参数  : struct oal_netbuf_stru *skb
             struct hcc_handler *hcc
             osal_u16*p_us_index
             osal_u8 dir
*****************************************************************************/
oal_tcp_ack_type_enum_uint8  hmac_tcp_opt_tx_get_tcp_ack_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap,
    osal_u16 *p_us_index, osal_u8 dir)
{
    oal_ip_header_stru  *ip_hdr;
    oal_tcp_header_stru *tcp_hdr;
    oal_ether_header_stru *eth_hdr;
    osal_u8 ip_hdr_len;
    eth_hdr = (oal_ether_header_stru *)oal_netbuf_data(skb);
    if (hmac_judge_tx_netbuf_is_tcp_ack_etc(eth_hdr) == OAL_FALSE) {
        /* not tcp ack data */
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        wifi_printf("\r\n====**not tcp packet return direct**====\r\n");
#endif
        return TCP_TYPE_ERROR;
    }
    ip_hdr = (oal_ip_header_stru *)(eth_hdr + 1);      /* 偏移一个snap，取ip头 */
    ip_hdr_len   = ip_hdr->ihl << 2; /* 左移2位 */
    tcp_hdr     = (oal_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
    /* option4:flow index取不到时不过滤 */
    *p_us_index =
        (osal_u16)hmac_tcp_opt_get_flow_index_etc(hmac_vap, ip_hdr, tcp_hdr, (hcc_chan_type)dir);
    if (*p_us_index == 0xFFFF) {
        return TCP_TYPE_ERROR;
    }

    return hmac_tcp_opt_get_tcp_ack_type_etc(hmac_vap, ip_hdr, (hcc_chan_type)dir, *p_us_index);
}
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
/*****************************************************************************
 函 数 名  : hmac_tcp_opt_tcp_ack_list_filter_etc
 功能描述  : TCP ACK过滤函数
 输入参数  : struct hcc_handler* hcc
             hmac_tcp_opt_queue type
             hcc_chan_type dir
             oal_netbuf_head_stru  *head
*****************************************************************************/
osal_u16 hmac_tcp_opt_tcp_ack_list_filter_etc(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type,
    hcc_chan_type dir, oal_netbuf_head_stru *head)
{
    struct tcp_list_node *node;
    oal_netbuf_stru *skb;
    oal_netbuf_head_stru  head_t;
    struct wlan_perform_tcp_list *tmp_list;
    struct osal_list_head *entry, *entry_temp;

    if (!hmac_vap) {
        wifi_printf("%s error:hmac_vap is null\n", __FUNCTION__);
        return 0;
    };

    if (!oal_netbuf_list_len(head)) {
        return 0;
    }
    oal_netbuf_head_init(&head_t);

    while (!!(skb = oal_netbuf_delist(head))) {
        if (hmac_tcp_opt_tcp_ack_filter_etc(skb, hmac_vap, dir)) {
            oal_netbuf_list_tail(&head_t, skb);
        } else {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
            wifi_printf("\r\n====found tcp ack...====\r\n");
#endif
        }
    }
    oal_netbuf_splice_init(&head_t, head);
    hmac_vap->hmac_tcp_ack[dir].filter_info.ull_ignored_count += oal_netbuf_list_len(head);

    tmp_list = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack_list;
    osal_list_for_each_safe(entry, entry_temp, &tmp_list->tcp_list) {
        oal_netbuf_head_stru *hcc_ack_queue;

        node = osal_list_entry(entry, struct tcp_list_node, list);
        if (node->used == 0) {
            continue;
        }
        hcc_ack_queue = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[node->index];

        hmac_tcp_opt_ack_count_reset_etc(hmac_vap, (hcc_chan_type)dir, (osal_u16)node->index);
        hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[node->index].ull_send_count +=
            oal_netbuf_list_len(hcc_ack_queue);

        oal_netbuf_queue_splice_tail_init(hcc_ack_queue, head);
    }
    return 0;
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_rx_tcp_ack_filter_etc
 功能描述  : 接收TCP ACK过滤函数
 输入参数  : struct hcc_handler* hcc
             hmac_tcp_opt_queue type
             hcc_chan_type dir
             void* data
*****************************************************************************/
osal_u16 hmac_tcp_opt_rx_tcp_ack_filter_etc(void *hmac_vap_tmp, hmac_tcp_opt_queue type, hcc_chan_type dir,
    void *data)
{
    hmac_vap_stru       *hmac_vap = (hmac_vap_stru *)hmac_vap_tmp;
    hmac_device_stru    *hmac_device;
    oal_netbuf_head_stru  *head = (oal_netbuf_head_stru *)data;

    if ((!hmac_vap) || (!data) || (type != HMAC_TCP_ACK_QUEUE) || (dir != HCC_RX)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:param invalid}");
        return OAL_FAIL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY,
                       "vap_id[%d] {hmac_tcp_opt_rx_tcp_ack_filter_etc fail: hmac_device is null}\r\n",
                       hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (!hmac_device->sys_tcp_rx_ack_opt_enable) {
        return 0;
    }
    return hmac_tcp_opt_tcp_ack_list_filter_etc(hmac_vap, type, dir, head);
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_tx_tcp_ack_filter_etc
 功能描述  : 发送TCP ACK过滤函数
 输入参数  : struct hcc_handler* hcc
             hmac_tcp_opt_queue type
             hcc_chan_type dir
             void* data
*****************************************************************************/
osal_u16 hmac_tcp_opt_tx_tcp_ack_filter_etc(void *hmac_vap_tmp, hmac_tcp_opt_queue type, hcc_chan_type dir,
    void *data)
{
    hmac_vap_stru       *hmac_vap = (hmac_vap_stru *)hmac_vap_tmp;
    hmac_device_stru    *hmac_device;
    oal_netbuf_head_stru  *head = (oal_netbuf_head_stru *)data;

    if ((!hmac_vap) || (!data) || (type != HMAC_TCP_ACK_QUEUE) || (dir != HCC_TX)) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_tcp_opt_rx_tcp_ack_filter_etc fail:param invalid}");
        return OAL_FAIL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY,
                       "vap_id[%d] {hmac_tcp_opt_tx_tcp_ack_filter_etc fail: hmac_device is null}\r\n",
                       hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (!hmac_device->sys_tcp_tx_ack_opt_enable) {
        return 0;
    }
    return hmac_tcp_opt_tcp_ack_list_filter_etc(hmac_vap, type, dir, head);
}
#endif

OAL_STATIC osal_void hmac_tcp_opt_ack_filter_dup(hmac_vap_stru *hmac_vap, oal_netbuf_stru *skb, hcc_chan_type dir,
    osal_u16 tcp_stream_index)
{
    oal_netbuf_head_stru *hcc_ack_queue = OAL_PTR_NULL;
    oal_netbuf_stru *ack = NULL;
    osal_u32 ret;

    /* 处理发送dup ack */
    hcc_ack_queue = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[tcp_stream_index];
    /* 将dup ack帧流队列中的帧全部发送 */
    while (!!(ack = oal_netbuf_delist(hcc_ack_queue))) {
        ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, ack);
        /* 调用失败，要释放内核申请的netbuff内存池 */
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oal_netbuf_free(ack);
        } else {
            hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[tcp_stream_index].ull_send_count++;
        }

        hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index]--;
    }
    /* 当前dup ack帧发送 */
    ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, skb);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oal_netbuf_free(skb);
    } else {
        hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[tcp_stream_index].ull_send_count++;
    }
}

/*****************************************************************************
 函 数 名  : hmac_tcp_opt_tcp_ack_filter_etc
 功能描述  : save it to tcp ack queue
 输入参数  : struct oal_netbuf_stru *skb
             struct hcc_handler *hcc
             hcc_chan_type dir
*****************************************************************************/
osal_u32 hmac_tcp_opt_tcp_ack_filter_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap, hcc_chan_type dir)
{
    oal_tcp_ack_type_enum_uint8 ret;
    osal_u16 tcp_stream_index;
    osal_u32 ack_limit;
    oal_netbuf_head_stru *hcc_ack_queue;
    oal_netbuf_stru *ack = NULL;

    if (dir == HCC_TX) {
        ret = hmac_tcp_opt_tx_get_tcp_ack_etc(skb, hmac_vap, &tcp_stream_index, dir);
    } else {
        ret = hmac_tcp_opt_rx_get_tcp_ack_etc(skb, hmac_vap, &tcp_stream_index, dir);
    }

    if (ret == TCP_ACK_DUP_TYPE) {
        hmac_tcp_opt_ack_filter_dup(hmac_vap, skb, dir, tcp_stream_index);
        return OAL_SUCC;
    } else if (ret == TCP_TYPE_ERROR) {
        /* 异常逻辑，将帧放入vap queue发送 */
        return OAL_FAIL;
    }

    /* 正常ack帧处理逻辑 */
    ack_limit = hmac_vap->hmac_tcp_ack[dir].filter_info.ack_limit;
    hcc_ack_queue = &hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_queue[tcp_stream_index];

    /* if normal ack received, del until ack_limit ack left */
    while (hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index] >= ack_limit) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log4(0, OAM_SF_ANY, "dir:%d:stream:%d : ack count:%u, qlen:%u", dir, tcp_stream_index,
                         hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index],
                         oal_netbuf_list_len(hcc_ack_queue));
#endif
        ack = oal_netbuf_delist(hcc_ack_queue);
        if (ack == OAL_PTR_NULL) {
            break;
        }
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log1(0, OAM_SF_ANY, "{dir:%d ------drop packet------.}", dir);
#endif
        hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[tcp_stream_index].ull_drop_count++;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log4(0, OAM_SF_ANY, "{dir:%d: ack count:%d , dupcount:%d ull_drop_count:%d.}", dir,
            hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index],
            hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_duplicate_ack_count[tcp_stream_index],
            hmac_vap->hmac_tcp_ack[dir].filter_info.tcp_info[tcp_stream_index].ull_drop_count);
#endif
        oal_netbuf_free_any(ack);
        hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index]--;
    }

    oal_netbuf_add_to_list_tail(skb, hcc_ack_queue);
    hmac_vap->hmac_tcp_ack[dir].hmac_tcp_ack.hcc_ack_count[tcp_stream_index]++;

    return OAL_SUCC;
}

void hmac_trans_queue_filter_etc(hmac_vap_stru    *hmac_vap, oal_netbuf_head_stru  *head_t,
    hmac_tcp_opt_queue type, hcc_chan_type dir)
{
    osal_u32 old_len, new_len;

    if (hmac_vap->hmac_tcp_ack[dir].filter[type]) {
        old_len = oal_netbuf_list_len(head_t);
        hmac_vap->hmac_tcp_ack[dir].filter[type](hmac_vap, type, dir, head_t);
        new_len = oal_netbuf_list_len(head_t);
        hmac_vap->hmac_tcp_ack[dir].all_ack_count[type] += old_len;
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        wifi_printf("\r\n====hmac_trans_queue_filter_etc[Queue:%d]Before filter len:%u,After filter len:%u====\r\n",
            type, old_len, new_len);
#endif
        if (OAL_UNLIKELY(new_len > old_len)) {
            oam_warning_log2(0, OAM_SF_TX, "The filter len:%u is more than before filter:%u",
                             new_len, old_len);
        } else {
            hmac_vap->hmac_tcp_ack[dir].drop_count[type] += (old_len - new_len);
        }
    }
}
void hmac_tcp_ack_process_hcc_queue_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir, hmac_tcp_opt_queue type)
{
    oal_netbuf_head_stru  head_t;
    oal_netbuf_head_stru *head;
    oal_netbuf_stru *pst_netbuf;
    osal_u32 ul_ret;

    if (hmac_vap->hmac_tcp_ack[dir].filter[type] != OAL_PTR_NULL) {
        oal_netbuf_head_init(&head_t);
        head = &hmac_vap->hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &head_t);
        hmac_trans_queue_filter_etc(hmac_vap, &head_t, type, dir);
        oal_netbuf_splice_init(&head_t, head);
    }
    if (dir == HCC_RX) {
        oal_netbuf_head_init(&head_t);
        head = &hmac_vap->hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &head_t);
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            hmac_rx_process_data_ap_tcp_ack_opt_etc(&head_t);
        } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_rx_process_data_sta_tcp_ack_opt_etc(hmac_vap, &head_t);
        }
    } else {
        oal_netbuf_head_init(&head_t);
        head = &hmac_vap->hmac_tcp_ack[dir].data_queue[type];
        oal_netbuf_queue_splice_tail_init(head, &head_t);
        while (!!(pst_netbuf = oal_netbuf_delist(&head_t))) {
            ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, pst_netbuf);
            /* 调用失败，要释放内核申请的netbuff内存池 */
            if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
                wifi_printf("\r\n====hmac_tcp_ack_process_hcc_queue_etc send fail:vap_id = %d====\r\n",
                    hmac_vap->vap_id);
#endif
                oal_netbuf_free(pst_netbuf);
            }
        }
    }
}
// 此函数最好带入参
osal_s32 hmac_tcp_ack_process_etc(void)
{
    osal_u8            vap_idx;
    hmac_vap_stru       *hmac_vap;
    hmac_device_stru    *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tcp_ack_process_etc::hmac_device[0] null.}");
        return 0;
    }
    if ((hmac_device->sys_tcp_rx_ack_opt_enable != OAL_TRUE) &&
        (hmac_device->sys_tcp_tx_ack_opt_enable != OAL_TRUE) &&
        (hmac_tcp_ack_need_schedule_etc() != OAL_TRUE)) {
        return 0;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(
            hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_add_vap_etc::hmac_vap null.}", vap_idx);
            continue;
        }
        if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) &&
            (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
        hmac_tcp_ack_process_hcc_queue_etc(hmac_vap, HCC_TX, HMAC_TCP_ACK_QUEUE);
#endif
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        hmac_tcp_ack_process_hcc_queue_etc(hmac_vap, HCC_RX, HMAC_TCP_ACK_QUEUE);
#endif
    }

    return 0;
}
oal_bool_enum_uint8 hmac_tcp_ack_need_schedule_etc(void)
{
    osal_u8             vap_idx;
    hmac_vap_stru        *hmac_vap;
    oal_netbuf_head_stru *head;
    hmac_device_stru     *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    /* 如果队列中有帧，则可以调度 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_tcp_ack_need_schedule_etc::hmac_vap null.}", vap_idx);
            continue;
        }

        if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) &&
            (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }

        head = &hmac_vap->hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE];
        /* 队列中有ack帧，则需要调度线程 */
        if (oal_netbuf_list_len(head) > 0) {
            return OAL_TRUE;
        }
        /* 接收方向未使能，无需判断 */
    }

    return OAL_FALSE;
}

#define TIME_ONE_SEC_IN_MS  1000
WIFI_TCM_TEXT osal_u32 hmac_rx_data_rate_status(osal_u32 thruput, osal_u32 *mbps)
{
    osal_u32 cur_time = (osal_u32)osal_get_time_stamp_ms();
    osal_u32 diff_time = 0;
    static osal_u32 g_tcp_opt_last_rx_time = 0;
    static osal_u32 g_thruput_value = 0;

    g_thruput_value += thruput;

    diff_time = (cur_time > g_tcp_opt_last_rx_time)?(cur_time - g_tcp_opt_last_rx_time):(
        0xFFFFFFFF - g_tcp_opt_last_rx_time + cur_time + 1);
    if (diff_time >= TIME_ONE_SEC_IN_MS) {
        *mbps = (osal_u32)(g_thruput_value >> 17);         /* 17: 右移20位转换为MB,再左移3位转换为Mb */
        g_tcp_opt_last_rx_time = cur_time;
        g_thruput_value = 0;
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_tcp_ack_filter_switch
 功能描述  : 下行性能大于50M时才开启tcp ack过滤功能
*****************************************************************************/
osal_void hmac_tcp_ack_filter_switch(osal_u32 rx_throput_mbps)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru         *hmac_vap;
    osal_u32            limit_throput_high;
    osal_u32            limit_throput_low;
    oal_bool_enum_uint8   tcp_ack_filter;

    /* 每秒吞吐量门限 */
    if ((g_st_tcp_ack_filter.rx_filter_throughput_high != 0) &&
        (g_st_tcp_ack_filter.rx_filter_throughput_low != 0)) {
        limit_throput_high = g_st_tcp_ack_filter.rx_filter_throughput_high;
        limit_throput_low  = g_st_tcp_ack_filter.rx_filter_throughput_low;
    } else {
        limit_throput_high = 15;    /* 大于15Mbps 开始过滤ack */
        limit_throput_low  = 10;    /* 小于10Mbps 停止过滤ack */
    }

    if (rx_throput_mbps > limit_throput_high) {
        tcp_ack_filter = OAL_TRUE;
    } else if (rx_throput_mbps < limit_throput_low) {
        tcp_ack_filter = OAL_FALSE;
    } else {
        return;
    }

    if (g_st_tcp_ack_filter.cur_filter_status == tcp_ack_filter) {
        return;
    }

    if (hmac_device_calc_up_vap_num_etc(hmac_device) != 1) {
        return;
    }

    if ((mac_device_find_up_vap_etc(hmac_device, &hmac_vap) != OAL_SUCC) || (hmac_vap == OAL_PTR_NULL)) {
        return;
    }

    hmac_device->sys_tcp_tx_ack_opt_enable = tcp_ack_filter;
    g_st_tcp_ack_filter.cur_filter_status = tcp_ack_filter;

    oam_warning_log3(0, OAM_SF_ANY,
        "{hmac_tcp_ack_filter_switch: limit_high = [%d],limit_low = [%d],rx_throught= [%d]!}",
        limit_throput_high, limit_throput_low, rx_throput_mbps);
}

OAL_STATIC osal_u32 hmac_tcp_opt_switch_ack_filter(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    osal_u32 mbps = 0;
    dmac_rx_ctl_stru *cb = NULL;

    if (netbuf == NULL) {
        return OAL_CONTINUE;
    }

    if (g_st_tcp_ack_filter.tcp_ack_filter_en != OSAL_TRUE) {
        return OAL_CONTINUE;
    }

    cb = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf);
    if (hmac_rx_data_rate_status(cb->rx_info.frame_len,  &mbps) == OAL_TRUE) {
        hmac_tcp_ack_filter_switch(mbps);
    }

    return OAL_CONTINUE;
}

hmac_netbuf_hook_stru g_tcp_opt_rx_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_HIGHEST,
    .hook_func = hmac_tcp_opt_switch_ack_filter,
};

osal_u32 hmac_tcp_opt_init(osal_void)
{
    (osal_void)hmac_register_netbuf_hook(&g_tcp_opt_rx_hook);
    return OAL_SUCC;
}

osal_void hmac_tcp_opt_deinit(osal_void)
{
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* end of _PRE_WLAN_TCP_OPT */
