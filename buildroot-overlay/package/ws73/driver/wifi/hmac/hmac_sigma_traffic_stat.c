/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * 文 件 名 : hmac_sigma_traffic_stat.c
 * 生成日期 : 2023年1月5日
 * 功能描述 : linux ip分片重组性能差，不满足认证要求，改为在驱动层面计算收包数
 */

#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_sigma_traffic_stat.h"
#include "oal_net.h"
#include "mac_frame.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59)
#include "../fs/proc/internal.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SIGMA_TRAFFIC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_udp_stat_stru g_udp_rx_stat;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述 : 非分片的UDP报文rx流量统计
*****************************************************************************/
OAL_STATIC osal_void hmac_rx_sigma_udp_nofrag_stat(const oal_ip_header_stru *ip_hdr)
{
    osal_u32 idx;
    osal_u32 ip_hdr_len = ip_hdr->ihl << 2;       /* 左移2bit:值对应4字节 */
    mac_udp_header_stru *udp_hdr = (mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
    osal_u16 ip_total_len = oal_host2net_short(ip_hdr->tot_len);

    for (idx = 0; idx < g_udp_rx_stat.stream_cnt; idx++) {
        if (oal_net2host_short(udp_hdr->des_port) == g_udp_rx_stat.port[idx]) {
            g_udp_rx_stat.rx_bytes[idx] += (ip_total_len - sizeof(oal_ip_header_stru) - sizeof(mac_udp_header_stru));
            g_udp_rx_stat.rx_frames[idx]++;
        }
    }
}

/*****************************************************************************
 功能描述 : 分片的UDP报文rx流量统计
*****************************************************************************/
OAL_STATIC osal_void hmac_rx_sigma_udp_frag_stat(const oal_ip_header_stru *ip_hdr, osal_u16 frag, osal_u16 offset)
{
    osal_u32 idx;
    osal_u16 ip_len = oal_host2net_short(ip_hdr->tot_len);

    /* 找到UDP首片报文 */
    if ((frag != 0) && (offset == 0)) {
        osal_u32 ip_hdr_len = ip_hdr->ihl << 2;       /* 左移2bit:值对应4字节 */
        mac_udp_header_stru *udp_hdr = (mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
        for (idx = 0; idx < g_udp_rx_stat.stream_cnt; idx++) {
            if (oal_net2host_short(udp_hdr->des_port) == g_udp_rx_stat.port[idx]) {
                g_udp_rx_stat.udp_tot_len[idx] = oal_host2net_short(udp_hdr->udp_len);
                g_udp_rx_stat.ip_id[idx] = ip_hdr->id;
                g_udp_rx_stat.frag_offset[idx] = ip_len - sizeof(oal_ip_header_stru);
                break;
            }
        }
        return;
    }

    /* 非首片报文通过id查找后续分片报文 */
    for (idx = 0; idx < g_udp_rx_stat.stream_cnt; idx++) {
        if (ip_hdr->id == g_udp_rx_stat.ip_id[idx]) {
            break;
        }
    }

    if (idx >= g_udp_rx_stat.stream_cnt) {
        return;
    }

    /* id一致且为分片报文且分片偏移符合预期(后续还有分片,更新偏移值) */
    if ((frag != 0) && (offset == g_udp_rx_stat.frag_offset[idx])) {
        g_udp_rx_stat.frag_offset[idx] += (ip_len - sizeof(oal_ip_header_stru));
        return;
    }

    /* id一致且不为分片报文且分片偏移符合预期为最后一片报文统计接收bytes */
    if ((frag == 0) && (offset == g_udp_rx_stat.frag_offset[idx])) {
        /* 长度符合表示所有分片报文均上报协议栈,统计接收报文长度 */
        if (g_udp_rx_stat.udp_tot_len[idx] == (g_udp_rx_stat.frag_offset[idx] + ip_len - sizeof(oal_ip_header_stru))) {
            g_udp_rx_stat.rx_bytes[idx] += (g_udp_rx_stat.udp_tot_len[idx] - sizeof(mac_udp_header_stru));
            g_udp_rx_stat.rx_frames[idx]++;
        }
    }
}

/*****************************************************************************
 功能描述 : Sigma跑流wifi流量统计,暂仅支持UDP报文统计
*****************************************************************************/
osal_void hmac_rx_sigma_traffic_stat(const mac_ether_header_stru *ether_hdr)
{
    oal_ip_header_stru *ip_hdr = OSAL_NULL;
    osal_u16 ip_frag;
    osal_u16 frag_offset;
    osal_u16 is_frag;

    if ((g_udp_rx_stat.flag == 0) || (ether_hdr->ether_type != oal_host2net_short(ETHER_TYPE_IP))) {
        return;
    }

    ip_hdr = (oal_ip_header_stru *)(ether_hdr + 1);
    if (ip_hdr->protocol != OAL_IPPROTO_UDP) {
        return;
    }

    /* 进行UDP数据校验 */
    ip_frag = (osal_u16)oal_host2net_short(ip_hdr->frag_off);
    frag_offset = (ip_frag & 0x1FFF) * 8; /* 每个数值表示8bytes */
    is_frag = ip_frag & BIT13;
    if ((is_frag == 0) && (frag_offset == 0)) { /* 未分片报文统计 */
        hmac_rx_sigma_udp_nofrag_stat(ip_hdr);
        return;
    }

    hmac_rx_sigma_udp_frag_stat(ip_hdr, is_frag, frag_offset);
}

/*****************************************************************************
 功能描述 : 获取配置的流数
*****************************************************************************/
OAL_STATIC osal_u8 hmac_sigma_traffic_get_cnt(osal_void)
{
    return g_udp_rx_stat.stream_cnt;
}

/*****************************************************************************
 功能描述 : 获取对应流的接收统计值
*****************************************************************************/
OAL_STATIC osal_void hmac_sigma_traffic_get_rx_stat(osal_u8 stream_id, osal_u32 *rx_frames, osal_u64 *rx_bytes)
{
    if (stream_id >= g_udp_rx_stat.stream_cnt) {
        *rx_frames = 0;
        *rx_bytes = 0;
        return;
    }

    *rx_frames = g_udp_rx_stat.rx_frames[stream_id];
    *rx_bytes = g_udp_rx_stat.rx_bytes[stream_id];
}

/* 仅支持高版本linux内核 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
/*****************************************************************************
 功能描述 : sigma rx流量统计文件节点读函数
*****************************************************************************/
OAL_STATIC ssize_t hmac_sigma_rx_traffic_read(struct file *filp, osal_char __user *buffer,
    size_t count, loff_t *f_pos)
{
    osal_char str[MAX_SIGMA_RX_VALUE * MAX_SIGMA_STREAM_CNT];
    osal_u32 idx, ret;
    osal_u32 len = 0;
    osal_u32 rx_frames = 0;
    osal_u64 rx_bytes = 0;

    for (idx = 0; idx < hmac_sigma_traffic_get_cnt(); idx++) {
        hmac_sigma_traffic_get_rx_stat(idx, &rx_frames, &rx_bytes);
        ret = sprintf_s(str + len, MAX_SIGMA_RX_VALUE, "%d %lld\n", rx_frames, rx_bytes);
        if (ret <= 0) {
            return -OAL_EINVAL;
        }
        len += ret;
    }

    if (*f_pos >= len) {
        return 0;
    }

    len -= *f_pos;
    if (len > count) {
        len = count;
    }

    /* 将内核态数据拷贝到用户态 */
    if (osal_copy_to_user(buffer, str + *f_pos, len)) {
        return -OAL_EINVAL;
    }

    *f_pos += len;
    return len;
}

OAL_STATIC osal_s32 hmac_sigma_traffic_seq_show(struct seq_file *f, void *v)
{
    return 0;
}

OAL_STATIC osal_s32 hmac_sigma_traffic_proc_open(struct inode *inode, struct file *filp)
{
    osal_s32               ret;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
    struct proc_dir_entry  *pde = PDE(inode);

    ret = single_open(filp, hmac_sigma_traffic_seq_show, pde->data);
#else
    ret = single_open(filp, hmac_sigma_traffic_seq_show, PDE_DATA(inode));
#endif
    return ret;
}

OAL_STATIC oal_proc_dir_entry_stru *g_sigma_proc_dir = OSAL_NULL;
OAL_STATIC const struct file_operations g_sigma_rx_stat = {
    .owner      = THIS_MODULE,
    .open       = hmac_sigma_traffic_proc_open,
    .read       = hmac_sigma_rx_traffic_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#endif

/*****************************************************************************
 功能描述 : 配置事件下发开启或者关闭wifi流量统计
*****************************************************************************/
osal_s32 hmac_config_set_traffic_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59)) /* 仅支持高版本linux内核 */
    oal_proc_dir_entry_stru *proc_info = OSAL_NULL;
    char traffic_dir[] = "wifi_traffic_stat";
    char rx_stat[] = "rx_stat";
    osal_u8 flag;

    flag = *((osal_u8 *)msg->data);
    if (flag == g_udp_rx_stat.flag) {
        return OAL_SUCC;
    }

    oam_info_log1(0, 0, "hmac_config_set_traffic_stat::flag[%d]", flag);
    if (flag == 1) { /* sigma跑流开始使能数据统计 */
        if (g_sigma_proc_dir != OSAL_NULL) {
            oal_remove_proc_entry(traffic_dir, NULL);
        }

        g_sigma_proc_dir = proc_mkdir(traffic_dir, NULL);
        if (g_sigma_proc_dir == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_config_set_traffic_stat: proc_mkdir return null");
            return OAL_FAIL;
        }

        proc_info = proc_create(rx_stat, 420, g_sigma_proc_dir, &g_sigma_rx_stat); /* 权限:420 */
        if (proc_info == OSAL_NULL) {
            oal_remove_proc_entry(traffic_dir, NULL);
            oam_warning_log0(0, OAM_SF_CFG, "hmac_config_set_traffic_stat: proc_create return null");
            return OAL_FAIL;
        }

        memset_s(&g_udp_rx_stat, sizeof(hmac_udp_stat_stru), 0, sizeof(hmac_udp_stat_stru));
    }

    g_udp_rx_stat.flag = flag;
#else
    unref_param(hmac_vap);
    unref_param(msg);
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 配置事件下发配置接收的UDP端口号
*****************************************************************************/
osal_s32 hmac_config_add_traffic_port(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u16 port = *((osal_u16 *)msg->data);
    osal_u8 idx = g_udp_rx_stat.stream_cnt;

    if (idx >= MAX_SIGMA_STREAM_CNT) {
        oam_warning_log1(0, 0, "hmac_config_add_traffic_port::exceed max stream, port[%d]", port);
        return OAL_FAIL;
    }

    g_udp_rx_stat.port[idx] = port;
    g_udp_rx_stat.stream_cnt++;
    oam_info_log2(0, 0, "hmac_config_add_traffic_port::id[%d] port[%d]", idx, g_udp_rx_stat.port[idx]);
    return OAL_SUCC;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  /* end of _PRE_WLAN_FEATURE_WFA_SUPPORT */
