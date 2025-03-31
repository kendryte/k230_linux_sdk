/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Downlink data service identification.
 * Create: 2021-12-15
 */

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN

/*****************************************************************************
  1头文件包含
*****************************************************************************/

#include "hmac_traffic_classify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TRAFFIC_CLASSIFY_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2宏定义
*****************************************************************************/
#define RTP_VERSION                 2           /* RTP协议版本号，占2位，当前协议版本号为2 */
#define RTP_VER_SHIFT               6           /* RTP协议版本号位移量 */
#define RTP_CSRC_MASK               0x0f        /* CSRC计数器，占4位，指示CSRC标识符的个数 */
#define RTP_CSRC_LEN_BYTE           4           /* 每个CSRC标识符占32位，共4字节 */
#define RTP_HDR_LEN_BYTE            12          /* RTP帧头固定字节数(不包含CSRC字段) */
#define TCP_HTTP_VI_LEN_THR         1000        /* HTTP视频流报文长度阈值 */
#define JUDGE_CACHE_LIFETIME        1           /* 待识别队列失效时间: 1s */
#define IP_FRAGMENT_MASK            0x1FFF      /* IP分片Fragment Offset字段 */
/* RTP Payload_Type 编号:RFC3551 */
#define RTP_PT_VO_G729              18          /* RTP载荷类型:18-Audio-G729 */
#define RTP_PT_VI_CELB              25          /* RTP载荷类型:25-Video-CelB */
#define RTP_PT_VI_JPEG              26          /* RTP载荷类型:26-Video-JPEG */
#define RTP_PT_VI_NV                28          /* RTP载荷类型:28-Video-nv */
#define RTP_PT_VI_H261              31          /* RTP载荷类型:31-Video-H261 */
#define RTP_PT_VI_MPV               32          /* RTP载荷类型:32-Video-MPV */
#define RTP_PT_VI_MP2T              33          /* RTP载荷类型:33-Video-MP2T */
#define RTP_PT_VI_H263              34          /* RTP载荷类型:34-Video-H263 */
/* HTTP流媒体端口 */
#define HTTP_PORT_80                80          /* HTTP协议默认端口号80 */
#define HTTP_PORT_8080              8080        /* HTTP协议默认端口号8080 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_void hmac_tx_traffic_classify_etc_handle(hmac_user_stru *hmac_user, osal_u8 *puc_tid,
    hmac_tx_flow_info_stru *flow_info, const mac_ip_header_stru *pst_ip, mac_udp_header_stru *udp_hdr);

/*****************************************************************************
 函 数 名  : hmac_tx_add_cfm_traffic
 功能描述  : 将用户已识别业务加入已识别记录表
 输入参数  : hmac用户结构体指针，TID指针，hmac_tx_major_flow_stru结构体指针
 返 回 值  : 成功返回OAL_SUCC,失败返回OAL_FAIL
*****************************************************************************/

OAL_STATIC osal_u32 hmac_tx_add_cfm_traffic(hmac_user_stru *hmac_user, osal_u8 tid,
    hmac_tx_major_flow_stru *max)
{
    osal_ulong  time_stamp;
    osal_u8  mark         = 0;
    osal_u8  traffic_idx  = 0;

    hmac_tx_cfm_flow_stru *cfm_info;

    if (hmac_user->cfm_num == MAX_CONFIRMED_FLOW_NUM) {
        /* 已识别列表已满，将列表中最长时间没有来包的业务进行替换 */
        time_stamp = hmac_user->cfm_flow_list[traffic_idx].last_jiffies;

        for (traffic_idx = 1; traffic_idx < MAX_CONFIRMED_FLOW_NUM; traffic_idx++) {
            cfm_info = (hmac_user->cfm_flow_list + traffic_idx);
            if (oal_time_after(time_stamp, cfm_info->last_jiffies)) {
                time_stamp = cfm_info->last_jiffies;
                mark = traffic_idx;
            }
        }
    } else {
        /* 已识别列表不满，找到可记录的index */
        for (traffic_idx = 0; traffic_idx < MAX_CONFIRMED_FLOW_NUM; traffic_idx++) {
            cfm_info = (hmac_user->cfm_flow_list + traffic_idx);
            if (cfm_info->cfm_flag == OAL_FALSE) {
                mark = traffic_idx;
                hmac_user->cfm_num++;
                cfm_info->cfm_flag = OAL_TRUE;
                break;
            }
        }
    }

    /* 更新列表 */
    cfm_info = (hmac_user->cfm_flow_list + mark);

    (osal_void)memcpy_s(&cfm_info->cfm_flow_info, OAL_SIZEOF(hmac_tx_flow_info_stru), &max->flow_info,
        OAL_SIZEOF(hmac_tx_flow_info_stru));

    cfm_info->cfm_tid      = tid;
    cfm_info->last_jiffies = (osal_ulong)osal_get_time_stamp_ms();

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_traffic_judge
 功能描述  : 对主要业务进行业务识别处理
 输入参数  : hmac用户结构体指针，主要业务结构体指针，TID指针
 输出参数  : TID
*****************************************************************************/
OAL_STATIC osal_u32 hmac_tx_traffic_judge(hmac_user_stru          *hmac_user,
    hmac_tx_major_flow_stru *major_flow,
    osal_u8               *puc_tid)
{
    osal_u32                  ul_ret = OAL_FAIL;
    osal_u8                   cache_idx;
    osal_u32                  pt;

    hmac_tx_judge_list_stru     *judge_list = &(hmac_user->judge_list);
    hmac_tx_judge_info_stru     *judge_info;

    /* 主要业务帧为UDP帧，进行RTP帧检测 */
    for (cache_idx = 0; cache_idx < MAX_JUDGE_CACHE_LENGTH; cache_idx++) {
        judge_info = (hmac_tx_judge_info_stru *)(judge_list->judge_cache + cache_idx);

        if (!osal_memcmp(&judge_info->flow_info, &major_flow->flow_info,
            OAL_SIZEOF(hmac_tx_flow_info_stru))) {
            /* RTP帧判断标准:version位保持为2，SSRC、PT保持不变，且帧长度大于RTP包头长度 */
            if (((judge_info->rtpver >> RTP_VER_SHIFT) != RTP_VERSION) ||
                (major_flow->rtpssrc      != judge_info->rtpssrc) ||
                (major_flow->payload_type != judge_info->payload_type) ||
                (major_flow->average_len  < (osal_u32)(judge_info->rtpver & RTP_CSRC_MASK) *
                RTP_CSRC_LEN_BYTE + RTP_HDR_LEN_BYTE)) {
                hmac_user->judge_list.to_judge_num = 0;   /* 识别失败，清空队列 */
                return OAL_FAIL;
            }
        }
    }

    pt = (major_flow->payload_type & (~BIT7));
    if (pt <= RTP_PT_VO_G729) {  /* 依据PayloadType判断RTP载荷类型 */
        *puc_tid = WLAN_TIDNO_VOICE;
    } else if ((pt == RTP_PT_VI_CELB) ||
               (pt == RTP_PT_VI_JPEG) ||
               (pt == RTP_PT_VI_NV) ||
               ((pt >= RTP_PT_VI_H261) && (pt <= RTP_PT_VI_H263))) {
        *puc_tid = WLAN_TIDNO_VIDEO;
    }

    /* 识别成功，更新用户已识别流列表 */
    if ((*puc_tid == WLAN_TIDNO_VOICE) || (*puc_tid == WLAN_TIDNO_VIDEO)) {
        ul_ret = hmac_tx_add_cfm_traffic(hmac_user, *puc_tid, major_flow);
    }
    hmac_user->judge_list.to_judge_num = 0;   /* 识别完成，清空队列 */

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_tx_find_major_traffic
 功能描述  : 找到待识别队列中主要业务
 输入参数  : hmac用户结构体指针，TID指针
 返 回 值  : 成功返回OAL_SUCC，失败返回OAL_FAIL
*****************************************************************************/
OAL_STATIC osal_u32 hmac_tx_find_major_traffic(hmac_user_stru *hmac_user, osal_u8 *puc_tid)
{
    osal_u8                       cache_idx_i;
    osal_u8                       cache_idx_j;

    hmac_tx_major_flow_stru         mark;
    hmac_tx_major_flow_stru         max;

    hmac_tx_judge_list_stru         *judge_list = &(hmac_user->judge_list);
    hmac_tx_judge_info_stru         *judge_info;

    /* 初始化 */
    max.wait_check_num  = 0;

    /* 队列超时，清空队列记录 */
    if (osal_get_runtime(judge_list->jiffies_st, judge_list->jiffies_end) > (JUDGE_CACHE_LIFETIME * USEC_TO_MSEC)) {
        hmac_user->judge_list.to_judge_num = 0;    /* 清空队列 */
        return OAL_FAIL;
    }

    /* 队列已满且未超时 */
    for (cache_idx_i = 0; cache_idx_i < (MAX_JUDGE_CACHE_LENGTH >> 1); cache_idx_i++) {
        judge_info = (hmac_tx_judge_info_stru *)(judge_list->judge_cache + cache_idx_i);

        if (judge_info->flag == OAL_FALSE) {
            continue;
        }

        judge_info->flag = OAL_FALSE;
        (osal_void)memcpy_s(&mark, OAL_SIZEOF(hmac_tx_judge_info_stru), judge_info,
            OAL_SIZEOF(hmac_tx_judge_info_stru));
        mark.wait_check_num = 1;

        for (cache_idx_j = 0; cache_idx_j < MAX_JUDGE_CACHE_LENGTH; cache_idx_j++) {
            judge_info = (hmac_tx_judge_info_stru *)(judge_list->judge_cache + cache_idx_j);

            if ((judge_info->flag == OAL_TRUE) &&
                !osal_memcmp(&judge_info->flow_info,
                &mark.flow_info, OAL_SIZEOF(hmac_tx_flow_info_stru))) {
                judge_info->flag     = OAL_FALSE;
                mark.average_len      += judge_info->len;
                mark.wait_check_num   += 1;
            }

            if (mark.wait_check_num <= max.wait_check_num) {
                continue;
            }

            (osal_void)memcpy_s(&max, OAL_SIZEOF(hmac_tx_major_flow_stru), &mark, OAL_SIZEOF(hmac_tx_major_flow_stru));
            if (max.wait_check_num >= (MAX_JUDGE_CACHE_LENGTH >> 1)) {
                /* 已找到主要业务流，不必继续搜索 */
                max.average_len = max.average_len / max.wait_check_num;
                return hmac_tx_traffic_judge(hmac_user, &max, puc_tid);
            }
        }
    }

    if (max.wait_check_num < (MAX_JUDGE_CACHE_LENGTH >> 2)) { /* 2偏移值 */
        /* 认为没有主要业务流 */
        hmac_user->judge_list.to_judge_num = 0;    /* 清空队列 */
        return OAL_FAIL;
    }

    max.average_len = max.average_len / max.wait_check_num;
    return hmac_tx_traffic_judge(hmac_user, &max, puc_tid);
}

/*****************************************************************************
 函 数 名  : hmac_tx_traffic_classify_etc
 功能描述  : 对下行数据包进行处理:
                若业务已被识别，直接返回TID, 若否，提取包头信息并进入待检测队列
 输入参数  : netbuff CB字段指针，ip头指针，TID指针
*****************************************************************************/
osal_void hmac_tx_traffic_classify_etc(mac_tx_ctl_stru     *tx_ctl,
    mac_ip_header_stru  *pst_ip,
    osal_u8           *puc_tid)
{
    mac_udp_header_stru         *udp_hdr = OAL_PTR_NULL;
    hmac_tx_flow_info_stru       flow_info;
    hmac_tx_cfm_flow_stru       *cfm_info = OAL_PTR_NULL;
    osal_u32                     ip_hdr_len = mac_ip_hdr_len(pst_ip);
    osal_u8                      loop;
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_cb_tx_user_idx(tx_ctl));

    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_tx_traffic_classify_etc::cannot find hmac_user[%d] may has freed!!",
                         mac_get_cb_tx_user_idx(tx_ctl));
        return;
    }

    /* 功能裁剪，只处理UDP报文，以及识别WifiDisplay RTSP业务为VI */
    if (pst_ip->protocol != MAC_UDP_PROTOCAL) {
        if (pst_ip->protocol == MAC_TCP_PROTOCAL) {
            mac_tcp_header_stru *tcp_hdr = (mac_tcp_header_stru *)((osal_u8 *)pst_ip + ip_hdr_len);

            /* 识别WifiDisplay RTSP业务为VI */
            if (tcp_hdr->sport == OAL_NTOH_16(MAC_WFD_RTSP_PORT)) {
                *puc_tid = WLAN_TIDNO_VIDEO;
                return;
            }
        }
        return;
    }

    /* 若为IP分片帧，没有端口号，直接返回 */
    if ((OAL_NTOH_16(pst_ip->frag_off) & IP_FRAGMENT_MASK) != 0) {
        return;
    }

    udp_hdr = (mac_udp_header_stru *)((osal_u8 *)pst_ip + ip_hdr_len);  /* 偏移一个IP头，取UDP头 */

    /* 提取五元组 */
    flow_info.dport = udp_hdr->des_port;
    flow_info.sport = udp_hdr->src_port;
    flow_info.dip   = pst_ip->daddr;
    flow_info.sip   = pst_ip->saddr;
    flow_info.proto = (osal_u32)(pst_ip->protocol);

    /* 若来包属于用户已识别业务，直接返回TID */
    for (loop = 0; loop < hmac_user->cfm_num; loop++) {
        cfm_info = (hmac_tx_cfm_flow_stru *)(hmac_user->cfm_flow_list + loop);

        if (!osal_memcmp(&cfm_info->cfm_flow_info, &flow_info, OAL_SIZEOF(hmac_tx_flow_info_stru))) {
            *puc_tid = (osal_u8)(cfm_info->cfm_tid);
            cfm_info->last_jiffies = (osal_ulong)osal_get_time_stamp_ms();   /* 更新业务最新来包时间 */
            return;
        }
    }

    hmac_tx_traffic_classify_etc_handle(hmac_user, puc_tid, &flow_info, pst_ip, udp_hdr);
}

OAL_STATIC osal_void hmac_tx_traffic_classify_etc_handle(hmac_user_stru *hmac_user, osal_u8 *puc_tid,
    hmac_tx_flow_info_stru *flow_info, const mac_ip_header_stru *pst_ip, mac_udp_header_stru *udp_hdr)
{
    hmac_tx_rtp_hdr             *rtp_hdr = OAL_PTR_NULL;
    hmac_tx_judge_list_stru     *judge_list;
    hmac_tx_judge_info_stru     *judge_info;

    /* 来包尚未识别，存入用户待识别队列 */
    judge_list = &(hmac_user->judge_list);
    judge_info = (hmac_tx_judge_info_stru *)(judge_list->judge_cache + judge_list->to_judge_num);

    if (judge_list->to_judge_num >= MAX_JUDGE_CACHE_LENGTH) { /* 队列已满，识别过程中的来包不作记录 */
        return;
    }

    judge_list->jiffies_end  = (osal_ulong)osal_get_time_stamp_ms();       /* 更新最新来包时间 */
    if (judge_list->to_judge_num == 0) {             /* 若队列为空 */
        judge_list->jiffies_st  = (osal_ulong)osal_get_time_stamp_ms();    /* 更新队列产生时间 */
    }
    judge_list->to_judge_num += 1;                   /* 更新队列长度 */

    memset_s(judge_info, OAL_SIZEOF(hmac_tx_judge_info_stru), 0, OAL_SIZEOF(hmac_tx_judge_info_stru));
    (osal_void)memcpy_s(&(judge_info->flow_info), OAL_SIZEOF(hmac_tx_flow_info_stru),
        flow_info, OAL_SIZEOF(hmac_tx_flow_info_stru));

    rtp_hdr = (hmac_tx_rtp_hdr *)(udp_hdr + 1);                 /* 偏移一个UDP头，取RTP头 */

    judge_info->flag         = OAL_TRUE;
    judge_info->len          = oal_net2host_short(pst_ip->tot_len) - (osal_u32)OAL_SIZEOF(mac_ip_header_stru) -
        (osal_u32)OAL_SIZEOF(mac_udp_header_stru);
    judge_info->rtpver       = rtp_hdr->version_and_csrc;
    judge_info->payload_type = (osal_u32)(rtp_hdr->payload_type);
    judge_info->rtpssrc      = rtp_hdr->ssrc;

    /* 若待识别队列已满，尝试提取队列主要业务并进行业务识别 */
    /* "<=":防止进程并发使此值大于待识别队列长度而踩内存 */
    if (judge_list->to_judge_num >= MAX_JUDGE_CACHE_LENGTH) {
        hmac_tx_find_major_traffic(hmac_user, puc_tid);
    }

    return;
}

oal_module_symbol(hmac_tx_traffic_classify_etc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
