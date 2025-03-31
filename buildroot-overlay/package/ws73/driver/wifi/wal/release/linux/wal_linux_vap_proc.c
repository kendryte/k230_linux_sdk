/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal vap info proc api.
 * Create: 2021-07-19
 */

#include "wal_linux_vap_proc.h"

#include "oam_struct.h"
#include "wal_linux_util.h"
#include "wal_linux_ioctl.h"
#include "wlan_msg.h"
#if defined(_PRE_WLAN_FEATURE_SNIFFER)
#include "hmac_sniffer.h"
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
#include "../fs/proc/internal.h"
#endif
#include "plat_firmware.h"
#include "oal_kernel_file.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_VAP_PROC_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) \
    && ((LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 59)))
#ifdef _PRE_WLAN_DFT_STAT
static osal_void wal_sta_info_seq_format_print(struct seq_file *f, osal_u16 idx, hmac_user_stru *hmac_user,
    oam_user_stat_info_stru *oam_user_stat, osal_u8 *addr)
{
    osal_s8 *pac_protocol2string[] = {"11a", "11b", "11g", "11g", "11g",
                                      "11n", "11ac", "11n", "11ac", "11n", "error"};

    seq_printf(f, "%2d: aid: %d\n" "    MAC ADDR: %02X:%02X:%02X:%02X:XX:XX\n" "    status: %d\n"
        "    BW: %d\n" "    NSS: %d\n" "    RSSI: %d\n" "    phy type: %s\n" "    TX rate: %dkbps\n"
        "    RX rate: %dkbps\n" "    user online time: %us\n"
        "    TX packets succ: %u\n" "    TX packets fail: %u\n" "    RX packets succ: %u\n"
        "    RX packets fail: %u\n" "    TX power: %ddBm\n" "    TX bytes: %u\n" "    RX bytes: %u\n"
        "    TX retries: %u\n" "    Tx Throughput: %u\n",
        idx, hmac_user->assoc_id,
        addr[0], addr[1], addr[2], addr[3], /* 地址第0、1、2、3位 */
        hmac_user->user_asoc_state, bw_enum_to_number(hmac_user->avail_bandwidth),
        (hmac_user->avail_num_spatial_stream + 1), /* NSS,加1是为了方便用户查看，软件处理0表示单流 */
        oal_get_real_rssi(hmac_user->rx_rssi), pac_protocol2string[hmac_user->avail_protocol_mode],
        hmac_user->tx_rate, hmac_user->rx_rate, (osal_u32)oal_time_get_runtime(hmac_user->first_add_time,
        (osal_u32)oal_time_get_stamp_ms()) / 1000, /* 1000ms为1s */
        tid_stat_to_user(oam_user_stat->tx_mpdu_succ_num) + tid_stat_to_user(oam_user_stat->tx_mpdu_in_ampdu),
        tid_stat_to_user(oam_user_stat->tx_mpdu_fail_num) + tid_stat_to_user(oam_user_stat->tx_mpdu_fail_in_ampdu),
        oam_user_stat->rx_mpdu_num,   /* RX packets succ */
        0, 20, /* 20db TX power, 暂不使用 后续调用tpc获取tx_power接口 */
        tid_stat_to_user(oam_user_stat->tx_mpdu_bytes) + tid_stat_to_user(oam_user_stat->tx_ampdu_bytes),
        oam_user_stat->rx_mpdu_bytes, oam_user_stat->tx_ppdu_retries, 0);
}
#endif
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
/*****************************************************************************
 函 数 名  : wal_sta_info_seq_start
 功能描述  : proc fops seq_file ops start函数，返回一个指针，作为seq_show的入参
*****************************************************************************/
OAL_STATIC osal_void *wal_sta_info_seq_start(struct seq_file *f, loff_t *pos)
{
    if (0 == *pos) {
        return f->private;
    } else {
        return NULL;
    }
}

/*****************************************************************************
 功能描述  : 发送事件查询STA的RSSI和rate
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
static inline osal_void wal_sta_query_rssi_and_rate(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head             *entry, *dlist_tmp;
    hmac_user_stru                  *hmac_user = OAL_PTR_NULL;
    osal_s32                        l_ret;
    frw_msg                          cfg_info;
    mac_cfg_query_rssi_stru         rssi_fill_msg = {0};
    mac_cfg_query_rate_stru         rate_fill_msg = {0};
    mac_cfg_query_rate_stru         query_rate_param = {0};

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        if (mac_res_get_hmac_user_etc(hmac_user->assoc_id) == OAL_PTR_NULL) {
            continue;
        }

        if (dlist_tmp == OAL_PTR_NULL) {
            /* 此for循环线程会暂停，期间会有删除用户事件，会出现dlist_tmp为空。为空时直接跳过获取dmac信息 */
            break;
        }

        memset_s(query_rate_param, OAL_SIZEOF(query_rate_param), 0, OAL_SIZEOF(query_rate_param));
        query_rate_param.user_id = hmac_user->assoc_id;

        memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
        cfg_msg_init((osal_u8 *)&query_rate_param, OAL_SIZEOF(query_rate_param),
            (osal_u8 *)&rate_fill_msg, OAL_SIZEOF(rate_fill_msg), &cfg_info);

        l_ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_W2D_C_CFG_QUERY_RATE, &cfg_info, OSAL_TRUE);
        if ((l_ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] query rate ret:%d", hmac_vap->vap_id, l_ret);
        }

        hmac_user->tx_rate       = rate_fill_msg.tx_rate;
        hmac_user->rx_rate       = rate_fill_msg.rx_rate;
    }
}
#endif

/*****************************************************************************
 功能描述  : 打印STA的seq信息
*****************************************************************************/
static inline osal_void wal_sta_info_seq_print(struct seq_file *f, hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry, *dlist_tmp;
    hmac_user_stru *hmac_user;
    osal_u8 *addr;
    osal_u16 idx = 1;
    oam_user_stat_info_stru *oam_user_stat;

    seq_printf(f, "Total user nums: %d\n", hmac_vap->user_nums);
    seq_printf(f, "-- STA info table --\n");

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (OAL_PTR_NULL == hmac_user) {
            continue;
        }

        oam_user_stat = &(OAM_STAT_GET_STAT_ALL()->user_stat_info[hmac_user->assoc_id]);
        addr = hmac_user->user_mac_addr;

        wal_sta_info_seq_format_print(f, idx, hmac_user, oam_user_stat, addr);
        idx++;
        if (OAL_PTR_NULL == dlist_tmp) {
            break;
        }
    }
}

/*****************************************************************************
 函 数 名  : wal_sta_info_seq_show
 功能描述  : proc fops seq_file ops show函数，输出AP下面所有用户的信息
*****************************************************************************/
OAL_STATIC osal_s32 wal_sta_info_seq_show(struct seq_file *f, void *v)
{
    hmac_vap_stru                    *hmac_vap  = (hmac_vap_stru *)v;

    /* step1. 同步要查询的dmac信息 */
#ifdef _PRE_WLAN_DFT_STAT
    wal_sta_query_rssi_and_rate(hmac_vap);

    /* step2. proc文件输出用户信息 */
    wal_sta_info_seq_print(f, hmac_vap);
#endif

    return 0;
}

/*****************************************************************************
 函 数 名  : wal_sta_info_seq_next
 功能描述  : proc fops seq_file ops next函数
*****************************************************************************/
OAL_STATIC osal_void *wal_sta_info_seq_next(struct seq_file *f, void *v, loff_t *pos)
{
    return NULL;
}

/*****************************************************************************
 函 数 名  : wal_sta_info_seq_stop
 功能描述  : proc fops seq_file ops stop函数
*****************************************************************************/
OAL_STATIC osal_void wal_sta_info_seq_stop(struct seq_file *f, void *v)
{
    return;
}

/*****************************************************************************
    hmac_sta_info_seq_ops: 定义seq_file ops
*****************************************************************************/
OAL_STATIC OAL_CONST struct seq_operations wal_sta_info_seq_ops = {
    .start = wal_sta_info_seq_start,
    .next  = wal_sta_info_seq_next,
    .stop  = wal_sta_info_seq_stop,
    .show  = wal_sta_info_seq_show
};

/*****************************************************************************
 函 数 名  : wal_sta_info_seq_open
 功能描述  : sta_info_proc_fops open函数
*****************************************************************************/
OAL_STATIC osal_s32 wal_sta_info_seq_open(struct inode *inode, struct file *filp)
{
    osal_s32 ret;
    struct seq_file *seq_file;
    struct proc_dir_entry  *pde = PDE(inode);

    ret = seq_open(filp, &wal_sta_info_seq_ops);
    if (ret == OAL_SUCC) {
        seq_file = (struct seq_file *)filp->private_data;

        seq_file->private = pde->data;
    }

    return ret;
}

/*****************************************************************************
    gst_sta_info_proc_fops: 定义sta info proc fops
*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations sta_info_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = wal_sta_info_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release
};
#else
OAL_STATIC const struct proc_ops sta_info_proc_fops = {
    .proc_open       = wal_sta_info_seq_open,
    .proc_read       = seq_read,
    .proc_lseek      = seq_lseek,
    .proc_release    = seq_release
};
#endif

OAL_STATIC int wal_read_vap_info_update_rx_length(int len, char *page, const oam_vap_stat_info_stru *oam_vap_stat,
    const hmac_vap_stru *hmac_vap)
{
    int rx_len;
#ifdef _PRE_WLAN_DFT_STAT
    rx_len = snprintf_s(page + len, PAGE_SIZE - len, PAGE_SIZE - len,
        "  RX bytes: %u\n" "  RX packets: %u\n" "  RX packets error: %u\n" "  RX packets discard: %u\n"
        "  RX unicast packets: %u\n" "  RX multicast packets: %u\n" "  RX broadcast packets: %u\n"
        "  RX unhnown protocol packets: %u\n" "  Br_rate_num: %u\n" "  Nbr_rate_num: %u\n"
        "  Max_rate: %u\n" "  Min_rate: %u\n" "  Channel num: %d\n" "  ANI:\n"
        "    hmac_device_distance: %d\n""    cca_intf_state: %d\n" "    co_intf_state: %d\n",
        oam_vap_stat->rx_bytes_to_lan, oam_vap_stat->rx_pkt_to_lan,
        oam_vap_stat->rx_defrag_process_dropped,
        oam_vap_stat->rx_no_buff_dropped + oam_vap_stat->rx_da_check_dropped,
        oam_vap_stat->rx_pkt_to_lan - oam_vap_stat->rx_mcast_cnt, 0,
        oam_vap_stat->rx_mcast_cnt, 0, hmac_vap->curr_sup_rates.br_rate_num, hmac_vap->curr_sup_rates.nbr_rate_num,
        hmac_vap->curr_sup_rates.max_rate, hmac_vap->curr_sup_rates.min_rate,
        hmac_vap->channel.chan_number, hmac_vap->device_distance, hmac_vap->intf_state_cca, hmac_vap->intf_state_co);
#else
    rx_len = snprintf_s(page + len, PAGE_SIZE - len, PAGE_SIZE - len,
        "  RX bytes: %u\n" "  RX packets: %u\n" "  RX packets error: %u\n"
        "  RX packets discard: %u\n" "  RX unicast packets: %u\n" "  RX multicast packets: %u\n"
        "  RX broadcast packets: %u\n" "  RX unhnown protocol packets: %u\n",
        oam_vap_stat->rx_bytes_to_lan, oam_vap_stat->rx_pkt_to_lan, oam_vap_stat->rx_defrag_process_dropped,
        oam_vap_stat->rx_no_buff_dropped + oam_vap_stat->rx_da_check_dropped,
        oam_vap_stat->rx_pkt_to_lan - oam_vap_stat->rx_mcast_cnt, 0, oam_vap_stat->rx_mcast_cnt, 0);
#endif
    return rx_len;
}

OAL_STATIC int wal_read_vap_info_update_tx_length(char *page, const oam_vap_stat_info_stru *oam_vap_stat)
{
    int tx_len;
    tx_len = snprintf_s(page, PAGE_SIZE, PAGE_SIZE, "vap stats:\n" "  TX bytes: %u\n" "  TX packets: %u\n"
        "  TX packets error: %u\n" "  TX packets discard: %u\n" "  TX unicast packets: %u\n"
        "  TX multicast packets: %u\n" "  TX broadcast packets: %u\n", oam_vap_stat->tx_bytes_from_lan,
        oam_vap_stat->tx_pkt_num_from_lan,
        oam_vap_stat->tx_abnormal_msdu_dropped + oam_vap_stat->tx_security_check_faild,
        0,
        oam_vap_stat->tx_pkt_num_from_lan - oam_vap_stat->tx_m2u_mcast_cnt, 0, oam_vap_stat->tx_m2u_mcast_cnt);
    return tx_len;
}

/*****************************************************************************
 函 数 名  : wal_vap_proc_read_vap_info
 功能描述  : vap info proc读函数
*****************************************************************************/
OAL_STATIC int  wal_read_vap_info_proc(char *page, char **start, off_t off,
                                       int count, int *eof, void *data)
{
    int                      len;
    hmac_vap_stru           *hmac_vap = (hmac_vap_stru *)data;
    oam_stat_info_stru      *oam_stat;
    oam_vap_stat_info_stru  *oam_vap_stat;

#ifdef _PRE_WLAN_DFT_STAT
    mac_cfg_query_ani_stru    query_ani_param = {0};
    osal_s32                 ret;
    frw_msg                   cfg_info;
    mac_cfg_query_ani_stru   fill_msg = {0};

    /***********************************************************************/
    /*                 获取dmac vap的ANI信息                               */
    /***********************************************************************/
    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&query_ani_param, OAL_SIZEOF(query_ani_param),
        (osal_u8 *)&fill_msg, OAL_SIZEOF(fill_msg), &cfg_info);

    ret = send_sync_cfg_to_host(hmac_vap->vap_id, WLAN_MSG_W2H_C_CFG_QUERY_ANI, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] wal_read_vap_info_proc: send query rssi cfg event ret:%d",
                         hmac_vap->vap_id, ret);
    }

    hmac_vap->device_distance    = fill_msg.device_distance;
    hmac_vap->intf_state_cca     = fill_msg.intf_state_cca;
    hmac_vap->intf_state_co      = fill_msg.intf_state_co;
#endif

    oam_stat = OAM_STAT_GET_STAT_ALL();

    oam_vap_stat = &(oam_stat->vap_stat_info[hmac_vap->vap_id]);

    len = wal_read_vap_info_update_tx_length(page, oam_vap_stat)
    if (len >= PAGE_SIZE) {
        oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] wal_read_vap_info_proc:len: %d out of page size: %d",
                         hmac_vap->vap_id, len, PAGE_SIZE);
        return (PAGE_SIZE - 1);
    }
    len += wal_read_vap_info_update_rx_length(len, page, oam_vap_stat, hmac_vap);
    if (len >= PAGE_SIZE) {
        oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] wal_read_vap_info_proc:len: %d out of page size: %d",
                         hmac_vap->vap_id, len, PAGE_SIZE);
        return (PAGE_SIZE - 1);
    }
    return len;
}

/*****************************************************************************
 函 数 名  : wal_vap_proc_read_rf_info
 功能描述  : mib_rf proc读函数
*****************************************************************************/
OAL_STATIC int  wal_read_rf_info_proc(char *page, char **start, off_t off,
                                      int count, int *eof, void *data)
{
    int                  len;
    hmac_vap_stru        *hmac_vap = (hmac_vap_stru *)data;

    len = snprintf_s(page, PAGE_SIZE, PAGE_SIZE, "rf info:\n  channel_num: %d\n",
        hmac_vap->channel.chan_number);
    if (len >= PAGE_SIZE) {
        len = PAGE_SIZE - 1;
    }
    return len;
}

/*****************************************************************************
 函 数 名  : wal_add_vap_proc_file
 功能描述  : 创建proc文件，用于输出vap的维测信息
*****************************************************************************/
osal_void wal_add_vap_proc_file(hmac_vap_stru *hmac_vap, const osal_s8 *pc_name)
{
    oal_proc_dir_entry_stru             *proc_dir;
    oal_proc_dir_entry_stru             *proc_vapinfo;
    oal_proc_dir_entry_stru             *proc_stainfo;
    oal_proc_dir_entry_stru             *proc_mibrf;

    proc_dir = proc_mkdir(pc_name, NULL);
    if (OAL_PTR_NULL == proc_dir) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] wal_add_vap_proc_file: proc_mkdir return null",
                         hmac_vap->vap_id);
        return;
    }

    proc_vapinfo = oal_create_proc_entry("ap_info", 420, proc_dir); /* mode420 */
    if (OAL_PTR_NULL == proc_vapinfo) {
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    proc_stainfo = oal_create_proc_entry("sta_info", 420, proc_dir); /* mode420 */
    if (OAL_PTR_NULL == proc_stainfo) {
        oal_remove_proc_entry("ap_info", proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    proc_mibrf = oal_create_proc_entry("mib_rf", 420, proc_dir); /* mode420 */
    if (OAL_PTR_NULL == proc_mibrf) {
        oal_remove_proc_entry("ap_info", proc_dir);
        oal_remove_proc_entry("sta_info", proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    /* vap info */
    proc_vapinfo->read_proc  = wal_read_vap_info_proc;
    proc_vapinfo->data       = hmac_vap;

    /* sta info，对于文件比较大的proc file，通过proc_fops的方式输出 */
    proc_stainfo->data       = hmac_vap;
    proc_stainfo->proc_fops  = &sta_info_proc_fops;

    /* rf info */
    proc_mibrf->read_proc  = wal_read_rf_info_proc;
    proc_mibrf->data       = hmac_vap;

    hmac_vap->proc_dir = proc_dir;
}

/*****************************************************************************
 函 数 名  : wal_del_vap_proc_file
 功能描述  : 删除vap对应的proc文件
*****************************************************************************/
osal_void wal_del_vap_proc_file(oal_net_device_stru *net_dev)
{
    hmac_vap_stru  *hmac_vap;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == hmac_vap)) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_del_vap_proc_file: hmac_vap is null ptr! net_dev:%x",
                         (osal_u32)net_dev);
        return;
    }

    if (hmac_vap->proc_dir) {
        oal_remove_proc_entry("mib_rf", hmac_vap->proc_dir);
        oal_remove_proc_entry("sta_info", hmac_vap->proc_dir);
        oal_remove_proc_entry("ap_info", hmac_vap->proc_dir);
        oal_remove_proc_entry(hmac_vap->name, NULL);
        hmac_vap->proc_dir = OAL_PTR_NULL;
    }
}
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
/*****************************************************************************
 函 数 名  : wal_sta_info_seq_show_sec
 功能描述  : proc fops seq_file ops show函数，输出AP下面所有用户的信息
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
OAL_STATIC osal_s32 wal_sta_info_seq_show_sec(struct seq_file *f, void *v)
{
    hmac_vap_stru *hmac_vap  = (hmac_vap_stru *)(f->private);
    struct osal_list_head *entry;
    struct osal_list_head *dlist_tmp;
    hmac_user_stru *hmac_user;
    osal_u8 *addr;
    osal_u16 idx = 1;
    oam_stat_info_stru *oam_stat;
    oam_user_stat_info_stru *oam_user_stat;
    osal_u32 curr_time;
    osal_s32 ret;
    frw_msg cfg_info;
    mac_cfg_query_rate_stru rate_fill_msg = {0};
    mac_cfg_query_rate_stru query_rate_param = {0};

    /* step1. 同步要查询的dmac信息 */
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (OAL_PTR_NULL == hmac_user) {
            continue;
        }

        if (OAL_PTR_NULL == dlist_tmp) {
            /* 此for循环线程会暂停，期间会有删除用户事件，会出现dlist_tmp为空。为空时直接跳过获取dmac信息 */
            break;
        }

        memset_s(&query_rate_param, OAL_SIZEOF(query_rate_param), 0, OAL_SIZEOF(query_rate_param));
        query_rate_param.user_id = hmac_user->assoc_id;

        memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
        cfg_msg_init((osal_u8 *)&query_rate_param, OAL_SIZEOF(query_rate_param),
            (osal_u8 *)&rate_fill_msg, OAL_SIZEOF(rate_fill_msg), &cfg_info);

        ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_W2D_C_CFG_QUERY_RATE, &cfg_info, OSAL_TRUE);
        if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] wal_sta_info_seq_show: send query rate cfg event ret:%d",
                             hmac_vap->vap_id, ret);
        }

        hmac_user->tx_rate       = rate_fill_msg.tx_rate;
        hmac_user->rx_rate       = rate_fill_msg.rx_rate;
    }

    /* step2. proc文件输出用户信息 */
    seq_printf(f, "Total user nums: %d\n", hmac_vap->user_nums);
    seq_printf(f, "-- STA info table --\n");

    oam_stat = OAM_STAT_GET_STAT_ALL();
    curr_time = (osal_u32)oal_time_get_stamp_ms();

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        oam_user_stat = &(oam_stat->user_stat_info[hmac_user->assoc_id]);
        addr = hmac_user->user_mac_addr;

        wal_sta_info_seq_format_print(f, idx, hmac_user, oam_user_stat, addr);
        idx++;
        if (dlist_tmp == OAL_PTR_NULL) {
            break;
        }
    }

    return 0;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
OAL_STATIC osal_void wal_vap_info_seq_format(struct seq_file *f, hmac_vap_stru *hmac_vap,
    oam_vap_stat_info_stru *oam_vap_stat)
{
    seq_printf(f, "  RX bytes: %u\n" "  RX packets: %u\n" "  RX packets error: %u\n"
        "  RX packets discard: %u\n" "  RX unicast packets: %u\n" "  RX multicast packets: %u\n"
        "  RX broadcast packets: %u\n" "  RX unhnown protocol packets: %u\n" "  Br_rate_num: %u\n"
        "  Nbr_rate_num: %u\n" "  Max_rate: %u\n" "  Min_rate: %u\n" "  Channel num: %d\n"
        "  ANI:\n" "    hmac_device_distance: %d\n" "    cca_intf_state: %d\n" "    co_intf_state: %d\n",
        oam_vap_stat->rx_bytes_to_lan, oam_vap_stat->rx_pkt_to_lan, oam_vap_stat->rx_defrag_process_dropped,
        oam_vap_stat->rx_no_buff_dropped + oam_vap_stat->rx_da_check_dropped,
        oam_vap_stat->rx_pkt_to_lan - oam_vap_stat->rx_mcast_cnt,
        0, oam_vap_stat->rx_mcast_cnt, 0, hmac_vap->curr_sup_rates.br_rate_num,
        hmac_vap->curr_sup_rates.nbr_rate_num, hmac_vap->curr_sup_rates.max_rate, hmac_vap->curr_sup_rates.min_rate,
        hmac_vap->channel.chan_number, hmac_vap->device_distance, hmac_vap->intf_state_cca, hmac_vap->intf_state_co);
}

/*****************************************************************************
 函 数 名  : wal_vap_info_seq_show_sec
 功能描述  : proc fops seq_file ops show函数，输出AP的信息
*****************************************************************************/
OAL_STATIC osal_s32 wal_vap_info_seq_show_sec(struct seq_file *f, void *v)
{
    hmac_vap_stru *hmac_vap  = (hmac_vap_stru *)(f->private);
    oam_stat_info_stru *oam_stat;
    oam_vap_stat_info_stru *oam_vap_stat;
    mac_cfg_query_ani_stru query_ani_param;
    osal_s32 ret;
    frw_msg cfg_info;
    mac_cfg_query_ani_stru fill_msg = {0};

    /***********************************************************************/
    /*                  获取dmac vap的ANI信息                              */
    /***********************************************************************/
    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&query_ani_param, OAL_SIZEOF(query_ani_param),
        (osal_u8 *)&fill_msg, OAL_SIZEOF(fill_msg), &cfg_info);

    ret = send_sync_cfg_to_host(hmac_vap->vap_id, WLAN_MSG_W2H_C_CFG_QUERY_ANI, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] wal_vap_info_seq_show_sec: send query rssi cfg event ret:%d",
                         hmac_vap->vap_id, ret);
    }

    hmac_vap->device_distance    = fill_msg.device_distance;
    hmac_vap->intf_state_cca     = fill_msg.intf_state_cca;
    hmac_vap->intf_state_co      = fill_msg.intf_state_co;

    oam_stat = OAM_STAT_GET_STAT_ALL();
    oam_vap_stat = &(oam_stat->vap_stat_info[hmac_vap->vap_id]);
    seq_printf(f, "vap stats:\n"
        "  TX bytes: %u\n"
        "  TX packets: %u\n"
        "  TX packets error: %u\n"
        "  TX packets discard: %u\n"
        "  TX unicast packets: %u\n"
        "  TX multicast packets: %u\n"
        "  TX broadcast packets: %u\n",
        oam_vap_stat->tx_bytes_from_lan,
        oam_vap_stat->tx_pkt_num_from_lan,
        oam_vap_stat->tx_abnormal_msdu_dropped + oam_vap_stat->tx_security_check_faild,
        0,
        oam_vap_stat->tx_pkt_num_from_lan - oam_vap_stat->tx_m2u_mcast_cnt,
        0,
        oam_vap_stat->tx_m2u_mcast_cnt);
    wal_vap_info_seq_format(f, hmac_vap, oam_vap_stat);
    return 0;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
OAL_STATIC osal_s32 wal_sta_info_proc_open(struct inode *inode, struct file *filp)
{
    osal_s32               ret;
    struct proc_dir_entry  *pde = PDE(inode);
    ret = single_open(filp, wal_sta_info_seq_show_sec, pde->data);

    return ret;
}

OAL_STATIC osal_s32 wal_vap_info_proc_open(struct inode *inode, struct file *filp)
{
    osal_s32               ret;
    struct proc_dir_entry  *pde = PDE(inode);
    ret = single_open(filp, wal_vap_info_seq_show_sec, pde->data);

    return ret;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
static ssize_t wal_vap_info_proc_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
    char mode;
    if (len < 1) {
        return -EINVAL;
    }

    if (copy_from_user(&mode, buffer, sizeof(mode))) {
        return -EFAULT;
    }

    return len;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*****************************************************************************
 函 数 名  : wal_wifi_sniffer_seq_show_sec
 功能描述  : proc fops seq_file ops show函数，输出AP下面所有用户的信息
*****************************************************************************/
OAL_STATIC osal_s32 wal_wifi_sniffer_seq_show_sec(struct seq_file *f, void *v)
{
    return 0;
}

OAL_STATIC osal_s32 wal_wifi_sniffer_proc_open(struct inode *inode, struct file *filp)
{
    osal_s32               l_ret;
    struct proc_dir_entry  *pde = PDE(inode);
    l_ret = single_open(filp, wal_wifi_sniffer_seq_show_sec, pde->data);

    return l_ret;
}

OAL_STATIC ssize_t wal_wifi_sniffer_proc_read(int file_index, osal_s8 __user *buffer, size_t count, loff_t *f_pos)
{
    size_t len;
    char *file_data = sniffer_file_buffer(file_index);
    int file_pos = sniffer_file_pos(file_index);

    oam_warning_log4(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: [cur max num len]=[%d %d %d %d]",
        g_sniffer_st.cur_file, g_sniffer_st.data_packet_max_len, g_sniffer_st.max_file_num, g_sniffer_st.max_file_len);;

    /* file_index = 0,1,2,3,4 */
    if (file_index >= g_sniffer_st.max_file_num) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: invalid file_index");
        return 0;
    }

    if (file_data == NULL || count == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: file_data=null, or count=%d", count);
        return 0;
    }

    /* can not read data behind the current file_content_pos */
    if (*f_pos >= file_pos) {
        oam_warning_log2(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: *f_pos[%d] >= file_pos[%d]", *f_pos, file_pos);
        return 0;
    }

    len = file_pos - *f_pos;
    if (len > count) {
        len = count;
    }

    if (copy_to_user(buffer, file_data + *f_pos, len)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: copy_to_user fail");
        return -EFAULT;
    }

    *f_pos += len;

    oam_warning_log1(0, OAM_SF_ANY, "wal_wifi_sniffer_proc_read:: return OK, len=%d", len);
    return len;
}

ssize_t wal_wifi_sniffer_proc_read_1(struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
    return wal_wifi_sniffer_proc_read(0, buffer, count, f_pos);
}

ssize_t wal_wifi_sniffer_proc_read_2(struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
    return wal_wifi_sniffer_proc_read(1, buffer, count, f_pos);
}

ssize_t wal_wifi_sniffer_proc_read_3(struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
    return wal_wifi_sniffer_proc_read(2, buffer, count, f_pos);
}

ssize_t wal_wifi_sniffer_proc_read_4(struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
    return wal_wifi_sniffer_proc_read(3, buffer, count, f_pos);
}

ssize_t wal_wifi_sniffer_proc_read_5(struct file *filp, char __user *buffer, size_t count, loff_t *f_pos)
{
    return wal_wifi_sniffer_proc_read(4, buffer, count, f_pos);
}
#endif

/*****************************************************************************
    gst_sta_info_proc_fops_sec: 定义sta info proc fops
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations sta_info_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_sta_info_proc_open,
    .read       = seq_read,
    .write      = wal_vap_info_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops sta_info_proc_sec = {
    .proc_open       = wal_sta_info_proc_open,
    .proc_read       = seq_read,
    .proc_write      = wal_vap_info_proc_write,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif


/*****************************************************************************
    gst_vap_info_proc_fops_sec: 定义vap info proc fops
*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations vap_info_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_vap_info_proc_open,
    .read       = seq_read,
    .write      = wal_vap_info_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops vap_info_proc_sec = {
    .proc_open       = wal_vap_info_proc_open,
    .proc_read       = seq_read,
    .proc_write      = wal_vap_info_proc_write,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif
#endif


#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*****************************************************************************
 功能描述: 定义wifi sniffer proc fops
*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations wifi_sniffer_1_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_wifi_sniffer_proc_open,
    .read       = wal_wifi_sniffer_proc_read_1,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops wifi_sniffer_1_proc_sec = {
    .proc_open       = wal_wifi_sniffer_proc_open,
    .proc_read       = wal_wifi_sniffer_proc_read_1,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations wifi_sniffer_2_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_wifi_sniffer_proc_open,
    .read       = wal_wifi_sniffer_proc_read_2,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops wifi_sniffer_2_proc_sec = {
    .proc_open       = wal_wifi_sniffer_proc_open,
    .proc_read       = wal_wifi_sniffer_proc_read_2,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations wifi_sniffer_3_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_wifi_sniffer_proc_open,
    .read       = wal_wifi_sniffer_proc_read_3,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops wifi_sniffer_3_proc_sec = {
    .proc_open       = wal_wifi_sniffer_proc_open,
    .proc_read       = wal_wifi_sniffer_proc_read_3,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations wifi_sniffer_4_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_wifi_sniffer_proc_open,
    .read       = wal_wifi_sniffer_proc_read_4,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops wifi_sniffer_4_proc_sec = {
    .proc_open       = wal_wifi_sniffer_proc_open,
    .proc_read       = wal_wifi_sniffer_proc_read_4,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
OAL_STATIC OAL_CONST struct file_operations wifi_sniffer_5_proc_sec = {
    .owner      = THIS_MODULE,
    .open       = wal_wifi_sniffer_proc_open,
    .read       = wal_wifi_sniffer_proc_read_5,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#else
OAL_STATIC const struct proc_ops wifi_sniffer_5_proc_sec = {
    .proc_open       = wal_wifi_sniffer_proc_open,
    .proc_read       = wal_wifi_sniffer_proc_read_5,
    .proc_lseek      = seq_lseek,
    .proc_release    = single_release,
};
#endif

#endif

/*****************************************************************************
 函 数 名  : wal_add_vap_proc_file_sec
 功能描述  : 创建proc文件，用于输出vap的维测信息
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
osal_void wal_add_vap_proc_file_sec(hmac_vap_stru *hmac_vap, osal_s8 *pc_name)
{
    oal_proc_dir_entry_stru             *proc_dir;
    oal_proc_dir_entry_stru             *proc_vapinfo;
    oal_proc_dir_entry_stru             *proc_stainfo;
    os_kernel_file_stru *fp = OSAL_NULL;
    osal_u8 *file_name = "/proc/wlan1";

    fp = filp_open(file_name, O_RDONLY, 0);
    if (!IS_ERR(fp)) {
        return;
    }
    proc_dir = proc_mkdir(pc_name, NULL);
    if (OAL_PTR_NULL == proc_dir) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] wal_add_vap_proc_file: proc_mkdir return null",
                         hmac_vap->vap_id);
        return;
    }

    proc_vapinfo = proc_create("ap_info", 420, proc_dir, &vap_info_proc_sec); /* mode 420 */
    if (OAL_PTR_NULL == proc_vapinfo) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] wal_add_vap_proc_file: proc_create return null",
                         hmac_vap->vap_id);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    proc_stainfo = proc_create("sta_info", 420, proc_dir, &sta_info_proc_sec); /* mode 420 */
    if (OAL_PTR_NULL == proc_stainfo) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] wal_add_vap_proc_file: proc_create return null",
                         hmac_vap->vap_id);
        oal_remove_proc_entry("ap_info", proc_dir);
        oal_remove_proc_entry(pc_name, NULL);
        return;
    }

    proc_vapinfo->data       = hmac_vap;
    proc_stainfo->data       = hmac_vap;

    hmac_vap->proc_dir = proc_dir;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*****************************************************************************
 函 数 名  : wal_add_vap_proc_file_sec
 功能描述  : 创建proc文件，用于输出vap的维测信息
*****************************************************************************/
osal_u32 wal_wifi_sniffer_proc_file_sec(void)
{
    oal_proc_dir_entry_stru *proc_stainfo;

    proc_stainfo = proc_create("wifisniffer_01", 0600, NULL, &wifi_sniffer_1_proc_sec); /* mode 0600 */
    if (proc_stainfo == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "vap_id[%d] wal_wifi_sniffer_proc_file_sec:proc_create wifi_sniffer_1 return null");
        return OAL_FAIL;
    }

    proc_stainfo = proc_create("wifisniffer_02", 0600, NULL, &wifi_sniffer_2_proc_sec); /* mode 0600 */
    if (proc_stainfo == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "vap_id[%d] wal_wifi_sniffer_proc_file_sec:proc_create wifi_sniffer_2 return null");
        return OAL_FAIL;
    }

    proc_stainfo = proc_create("wifisniffer_03", 0600, NULL, &wifi_sniffer_3_proc_sec); /* mode 0600 */
    if (proc_stainfo == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "vap_id[%d] wal_wifi_sniffer_proc_file_sec:proc_create wifi_sniffer_3 return null");
        return OAL_FAIL;
    }

    proc_stainfo = proc_create("wifisniffer_04", 0600, NULL, &wifi_sniffer_4_proc_sec); /* mode 0600 */
    if (proc_stainfo == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "vap_id[%d] wal_wifi_sniffer_proc_file_sec:proc_create wifi_sniffer_4 return null");
        return OAL_FAIL;
    }

    proc_stainfo = proc_create("wifisniffer_05", 0600, NULL, &wifi_sniffer_5_proc_sec); /* mode 0600 */
    if (proc_stainfo == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,
            "vap_id[%d] wal_wifi_sniffer_proc_file_sec:proc_create wifi_sniffer_5 return null");
        return OAL_FAIL;
    }

    oam_warning_log0(0, OAM_SF_CFG, "sniifer file create succ, dir[/proc]");
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : wal_del_vap_proc_file_sec
 功能描述  : 删除vap对应的proc文件
*****************************************************************************/
osal_void wal_del_vap_proc_file_sec(oal_net_device_stru *net_dev)
{
    hmac_vap_stru  *hmac_vap;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == hmac_vap)) {
        return;
    }

    if (hmac_vap->proc_dir) {
        oal_remove_proc_entry("sta_info", hmac_vap->proc_dir);
        oal_remove_proc_entry("ap_info", hmac_vap->proc_dir);
        oal_remove_proc_entry(hmac_vap->name, NULL);
        hmac_vap->proc_dir = OAL_PTR_NULL;
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
