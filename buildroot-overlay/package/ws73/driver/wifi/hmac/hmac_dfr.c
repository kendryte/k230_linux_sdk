/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: 维测功能相关定义和实现
 * Date: 2021-04-14
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_dfr.h"
#include "oal_net.h"
#include "oam_ext_if.h"
#include "oal_mem_hcm.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "frw_ext_if.h"
#include "common_dft.h"
#include "hmac_config.h"
#include "hmac_tx_mpdu_queue.h"
#ifdef _PRE_WLAN_DFR_STAT
#include "msg_dfr_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_DFR_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_FEATURE_DFR
hmac_ap_dfr_recovery_info g_dfr_ap_info; /* ap 保存的配置信息 */
hmac_dfr_info_stru g_dfr_recovery_info; /* DFR异常复位开关 */

osal_void hmac_dfr_fill_ap_recovery_info(osal_u16 cfg_id, void *data, hmac_vap_stru *hmac_vap)
{
    errno_t ret_err;
    mac_addkey_param_stru *addkey_param = OSAL_NULL;
    mac_key_params_stru *key = OSAL_NULL;
    osal_u8 key_cipher;
    /* 非ap不管 */
    if (is_legacy_ap(hmac_vap) == 0) {
        return;
    }
    if (cfg_id == WLAN_MSG_W2H_CFG_CFG80211_SET_CHANNEL) {
        ret_err = memcpy_s(&g_dfr_ap_info.channel_info, sizeof(mac_cfg_channel_param_stru),
            data, sizeof(mac_cfg_channel_param_stru));
        if (ret_err != EOK) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: memcpy_s failed.}");
        }
    } else if (cfg_id == WLAN_MSG_W2H_CFG_SSID) {
        ret_err = memcpy_s(&g_dfr_ap_info.ssid_info, sizeof(mac_cfg_ssid_param_stru),
            data, sizeof(mac_cfg_ssid_param_stru));
        if (ret_err != EOK) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: memcpy_s failed.}");
        }
    } else if (cfg_id == WLAN_MSG_W2H_CFG_AUTH_MODE) {
        ret_err = memcpy_s(&g_dfr_ap_info.auth_algs, sizeof(wlan_auth_alg_mode_enum_uint8),
            data, sizeof(wlan_auth_alg_mode_enum_uint8));
        if (ret_err != EOK) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: memcpy_s failed.}");
        }
    } else if (cfg_id == WLAN_MSG_W2H_CFG_CFG80211_CONFIG_BEACON) {
        ret_err = memcpy_s(&g_dfr_ap_info.beacon_param, sizeof(mac_beacon_param_stru),
            data, sizeof(mac_beacon_param_stru));
        if (ret_err != EOK) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: memcpy_s failed.}");
        }
    } else if (cfg_id == WLAN_MSG_W2H_CFG_ADD_KEY) {
        addkey_param = (mac_addkey_param_stru *)data;
        key = &(addkey_param->key);
        key_cipher = key->cipher & 0xFF;
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: save key cipher[%d]}", key_cipher);
        /* igtk */
        if (key_cipher >= WLAN_80211_CIPHER_SUITE_BIP && key_cipher <= WLAN_80211_CIPHER_SUITE_BIP_CMAC_256) {
            ret_err = memcpy_s(&g_dfr_ap_info.igtk_key_params, sizeof(mac_addkey_param_stru),
                data, sizeof(mac_addkey_param_stru));
        } else {
            /* gtk */
            ret_err = memcpy_s(&g_dfr_ap_info.payload_params, sizeof(mac_addkey_param_stru),
                data, sizeof(mac_addkey_param_stru));
        }
        if (ret_err != EOK) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_fill_ap_recovery_info:: memcpy_s failed.}");
        }
    }
}

osal_u8 hmac_dfr_get_recovery_flag(osal_void)
{
    return g_dfr_recovery_info.bit_ready_to_recovery_flag;
}
#endif

osal_void hmac_dfr_record_ap_follow_channel(osal_u8 channel, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    g_dfr_ap_info.channel_info.channel = channel;
    g_dfr_ap_info.channel_info.en_bandwidth = bandwidth;
#else
    unref_param(channel);
    unref_param(bandwidth);
#endif
}

osal_u8 hmac_dfr_get_reset_process_flag(osal_void)
{
    osal_u8 dfr_process_flag = OSAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_DFR
    dfr_process_flag = g_dfr_recovery_info.bit_device_reset_process_flag;
#endif
    return dfr_process_flag;
}

#ifdef _PRE_WLAN_DFR_STAT
osal_void hmac_dfr_check_tid_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);
osal_void hmac_dfr_check_tx_dscr_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);
osal_void hmac_dfr_check_rx_dscr_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);
osal_void hmac_dfr_check_mempool_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);
osal_void hmac_dfr_check_frw_event_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);

osal_u32 hmac_get_user_tid_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);
osal_u32 hmac_get_tx_dscr_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);
osal_u32 hmac_get_rx_dscr_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);
osal_u32 hmac_get_frw_event_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);
osal_u32 hmac_get_mempool_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);

#define ERROR_THRESHOLD1   20
#define ERROR_THRESHOLD2   100    /* 两个数据算出20%的阈值范围 */
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 异常检测定时器的检测周期，单位毫秒，默认值为5000ms */
static osal_u32 g_period_task_timeout = 5000;
static frw_timeout_stru g_hmac_dfr_timer;
static osal_u32 g_enable_switch = 0;
static hmac_dfr_error_time g_hmac_record = {0};
/* 累计上报次数的统计值 */
static osal_u32 g_report_value = 0;
/* 上报了一定次数以后，将上报周期扩大10倍 */
static osal_u32 g_cnt_period = 10;

/* 缓存每个tid队列的一个周期内的值异常统计值 */
osal_u32 g_pack_in_cnt[MAC_RES_MAX_USER_LIMIT][WLAN_TID_MAX_NUM] = {0};
osal_u32 g_pack_out_cnt[MAC_RES_MAX_USER_LIMIT][WLAN_TID_MAX_NUM] = {0};
osal_u32 g_dispatch_cnt[MAC_RES_MAX_USER_LIMIT][WLAN_TID_MAX_NUM] = {0};

/* 缓存事件队列、内存池、tx和rx描述队列一个周期内的异常统计值 */
hmac_dfr_queue_statistics g_frw_event_statistics[WLAN_FRW_MAX_NUM_CORES][FRW_POST_PRI_NUM] = {0};
hmac_dfr_mempool_statistics g_mempool_statistics[OAL_MEM_POOL_ID_BUTT] = {0};
hmac_dfr_queue_statistics g_tx_dscr_statistics[HAL_TX_QUEUE_NUM] = {0};
osal_u32 g_rx_dscr_statistics[HAL_RX_QUEUE_NUM] = {0};

/* 缓存device侧事件队列、内存池、tx和rx描述队列一个周期内的异常统计值 */
dfr_info_rsp_stru g_dfr_info[OAL_MEM_POOL_ID_BUTT] = {0};

/* 首次统计的标志，首次统计时将各统计值复位 */
static osal_bool g_first_time_statistic = OSAL_TRUE;

/* 存放异常检测标志位和检测函数的数组定义 */
const hmac_dfr_check_module_exception_stru check_module[HMAC_DFR_MAX_NUM] = {
    {1 << HMAC_DFR_TID_REPORT, hmac_dfr_check_tid_exception},
    {1 << HMAC_DFR_TX_REPORT, hmac_dfr_check_tx_dscr_exception},
    {1 << HMAC_DFR_RX_REPORT, hmac_dfr_check_rx_dscr_exception},
    {1 << HMAC_DFR_FRW_REPORT, hmac_dfr_check_frw_event_exception},
    {1 << HMAC_DFR_MEM_POOL_REPORT, hmac_dfr_check_mempool_exception},
};

/* 存放模块信息查询标志位和查询函数的数组定义 */
const hmac_dfr_query_module_statistics_stru query_module[HMAC_DFR_MAX_NUM] = {
    {1 << HMAC_DFR_TID_REPORT, hmac_get_user_tid_info},
    {1 << HMAC_DFR_TX_REPORT, hmac_get_tx_dscr_info},
    {1 << HMAC_DFR_RX_REPORT, hmac_get_rx_dscr_info},
    {1 << HMAC_DFR_FRW_REPORT, hmac_get_frw_event_info},
    {1 << HMAC_DFR_MEM_POOL_REPORT, hmac_get_mempool_info},
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/
void hmac_dfr_report_tid_frw_check(osal_u32 *report_flag, osal_u32 period_cnt)
{
    osal_u32 user_id, tid, core_id, idx;

    /* TID队列的异常上报 */
    for (user_id = 0; user_id < MAC_RES_MAX_USER_LIMIT; user_id++) {
        for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
            if (g_hmac_record.tid_halt_cur[user_id][tid] >= period_cnt) {
                oam_warning_log2(0, OAM_SF_DFR,
                    "{hmac_dfr_report_tid_frw_check: user[%u] tid[%u], HMAC_DFR_TID_QUEUE_HALT_SET}", user_id, tid);
                g_hmac_record.tid_halt_cur[user_id][tid] = 0;
                *report_flag = 1;
            }
            if (g_hmac_record.tid_busy_cur[user_id][tid] >= period_cnt) {
                oam_warning_log2(0, OAM_SF_DFR,
                    "{hmac_dfr_report_tid_frw_check: user[%u] tid[%u], HMAC_DFR_TID_QUEUE_BUSY_SET}", user_id, tid);
                g_hmac_record.tid_busy_cur[user_id][tid] = 0;
                *report_flag = 1;
            }
        }
    }
    /* FRW的异常上报 */
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        for (idx = 0; idx < FRW_POST_PRI_NUM; idx++) {
            if (g_hmac_record.frw_halt_cur[core_id][idx] >= period_cnt) {
                oam_warning_log2(0, OAM_SF_DFR,
                    "{hmac_dfr_report_tid_frw_check: core_id[%u] idx[%u], HMAC_DFR_FRW_QUEUE_HALT}", core_id, idx);
                g_hmac_record.frw_halt_cur[core_id][idx] = 0;
                *report_flag = 1;
            }
            if (g_hmac_record.frw_fail_cur[core_id][idx] >= period_cnt) {
                oam_warning_log2(0, OAM_SF_DFR,
                    "{hmac_dfr_report_tid_frw_check: core_id[%u] idx[%u], HMAC_DFR_FRW_QUEUE_FAIL}", core_id, idx);
                g_hmac_record.frw_fail_cur[core_id][idx] = 0;
                *report_flag = 1;
            }
        }
    }
}

void hmac_dfr_report_mem_txrx_check(osal_u32 *report_flag, osal_u32 period_cnt)
{
    osal_u32 pool_id, queue_id;

    /* MEM_POOL的异常上报 */
    for (pool_id = 0; pool_id < osal_array_size(g_hmac_record.mem_no_free_cur); pool_id++) {
        if (g_hmac_record.mem_no_free_cur[pool_id] >= period_cnt) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{hmac_dfr_report_mem_txrx_check::mempool_id: [%u], HMAC_DFR_MEM_POOL_NO_FREE}", pool_id);
            g_hmac_record.mem_no_free_cur[pool_id] = 0;
            *report_flag = 1;
        }
        if (g_hmac_record.mem_fail_cur[pool_id] >= period_cnt) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{hmac_dfr_report_mem_txrx_check::mempool_id: [%u], HMAC_DFR_MEM_POOL_FAIL}", pool_id);
            g_hmac_record.mem_fail_cur[pool_id] = 0;
            *report_flag = 1;
        }
    }
    /* TX的异常上报 */
    for (queue_id = 0; queue_id < osal_array_size(g_hmac_record.tx_halt_cur); queue_id++) {
        if (g_hmac_record.tx_halt_cur[queue_id] >= period_cnt) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{hmac_dfr_report_mem_txrx_check::tx_dsrc id [%u], HMAC_DFR_TX_DSCR_QUEUE_HALT}", queue_id);
            g_hmac_record.tx_halt_cur[queue_id] = 0;
            *report_flag = 1;
        }
    }
    /* RX的异常上报 */
    for (queue_id = 0; queue_id < osal_array_size(g_hmac_record.rx_halt_cur); queue_id++) {
        if (g_hmac_record.rx_halt_cur[queue_id] >= period_cnt) {
            oam_warning_log1(0, OAM_SF_DFR,
                "{hmac_dfr_report_mem_txrx_check::rx_dsrc id [%u], HMAC_DFR_RX_DSCR_QUEUE_HALT}", queue_id);
            g_hmac_record.rx_halt_cur[queue_id] = 0;
            *report_flag = 1;
        }
    }
}

void hmac_dfr_report_event_check(osal_u32 *report_flag, osal_u32 period_cnt)
{
    hmac_dfr_report_tid_frw_check(report_flag, period_cnt);
    hmac_dfr_report_mem_txrx_check(report_flag, period_cnt);
}

osal_void hmac_dfr_check_tid_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event)
{
    hmac_tid_stru *tid_queue = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 user_id, tid;

    /* 判断当前的vap_id下是否有user */
    if (hmac_vap->user_nums == 0) {
        return;
    }

    for (user_id = 0; user_id < MAC_RES_MAX_USER_LIMIT; user_id++) {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
        if (hmac_user == OSAL_NULL) {
            continue;
        }

        for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
            tid_queue = &(hmac_user->tx_tid_queue[tid]);
            if (g_first_time_statistic) {
                /* 将tid队列中的值清空，进入的包的初值为上次队列的长度 */
                tid_queue->in_num = tid_queue->mpdu_num;
                tid_queue->dispatch_num = 0;
                continue;
            }
            /* 非首次检测才更新统计值，上报事件 */
            g_pack_in_cnt[user_id][tid] = tid_queue->in_num;
            g_pack_out_cnt[user_id][tid] = tid_queue->in_num - tid_queue->mpdu_num;
            g_dispatch_cnt[user_id][tid] = tid_queue->dispatch_num;

            /* 1）单位时间内，tid队列非pause 状态下队列非空，不出队 */
            if ((tid_queue->is_paused == 0) && (g_pack_in_cnt[user_id][tid] != 0) &&
                (g_pack_out_cnt[user_id][tid] == 0)) {
                dfr_event->tid_content[user_id][0] |= (1 << tid);
                dfr_event->hmac_user_id_info |= (1 << user_id);
                dfr_event->event_type |= HMAC_DFR_TID_QUEUE_HALT_SET;
                g_hmac_record.tid_halt_cur[user_id][tid]++;
            }
            /* 2）单位时间内，tid 队列非pause状态下，队列非空，发生调度的次数为0 */
            if ((tid_queue->is_paused == 0) && (g_pack_in_cnt[user_id][tid] != 0) &&
                (g_dispatch_cnt[user_id][tid] == 0)) {
                dfr_event->tid_content[user_id][1] |= (1 << tid);
                dfr_event->hmac_user_id_info |= (1 << user_id);
                dfr_event->event_type |= HMAC_DFR_TID_QUEUE_BUSY_SET;
                g_hmac_record.tid_busy_cur[user_id][tid]++;
            }
            tid_queue->in_num = tid_queue->mpdu_num;
            tid_queue->dispatch_num = 0;
        }
    }
}

osal_u32 hmac_get_dfr_period_cnt(osal_u32 report_value, osal_u32 max_count)
{
    if (report_value < max_count) {
        return 1;
    }
    return g_cnt_period;
}

osal_u32 hmac_period_check_task_timeout(osal_void *arg)
{
    /* 从上层ccpriv命令传入的使能符号列表 */
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;
    hmac_dfr_event event_values;
    errno_t ret_err;
    osal_u32 module_cnt;
    osal_u32 report_flag = 0;
    osal_u32 period_cnt;
    osal_u32 ret;
    frw_msg msg_info = {0};

    ret_err = memset_s(&event_values, sizeof(event_values), 0, sizeof(event_values));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_period_check_task_timeout:: memset_s failed.}");
        return OAL_FAIL;
    }

    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg_info);

    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DFR_REQ, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d] {frw_send_msg_to_device::fail [%d].}\r\n", hmac_vap->vap_id, ret);
        return ret;
    }

    for (module_cnt = 0; module_cnt < osal_array_size(check_module); module_cnt++) {
        if (check_module[module_cnt].module_set & g_enable_switch) {
            check_module[module_cnt].check_exception_func(hmac_vap, &event_values);
        }
    }
    period_cnt = hmac_get_dfr_period_cnt(g_report_value, 100); /* 上报次数100次 */

    /* 检测是否打印SDT日志，是否上报事件 */
    hmac_dfr_report_event_check(&report_flag, period_cnt);

    if (g_first_time_statistic == OSAL_TRUE) {
        g_first_time_statistic = OSAL_FALSE;
    }
    oam_info_log3(0, OAM_SF_DFR,
        "{hmac_period_check_task_timeout::period_cnt[%u] report_flag[%u] g_first_time_statistic[%u]",
        period_cnt, report_flag, g_first_time_statistic);
    return OAL_SUCC;
}

osal_void hmac_period_task_init(osal_void)
{
    (osal_void)memset_s(&g_pack_in_cnt, sizeof(g_pack_in_cnt), 0, sizeof(g_pack_in_cnt));
    (osal_void)memset_s(&g_pack_out_cnt, sizeof(g_pack_out_cnt), 0, sizeof(g_pack_out_cnt));
    (osal_void)memset_s(&g_dispatch_cnt, sizeof(g_dispatch_cnt), 0, sizeof(g_dispatch_cnt));
    (osal_void)memset_s(&g_frw_event_statistics, sizeof(g_frw_event_statistics), 0, sizeof(g_frw_event_statistics));
    (osal_void)memset_s(&g_mempool_statistics, sizeof(g_mempool_statistics), 0, sizeof(g_mempool_statistics));
    (osal_void)memset_s(&g_tx_dscr_statistics, sizeof(g_tx_dscr_statistics), 0, sizeof(g_tx_dscr_statistics));
    (osal_void)memset_s(&g_rx_dscr_statistics, sizeof(g_rx_dscr_statistics), 0, sizeof(g_rx_dscr_statistics));
    (osal_void)memset_s(&g_hmac_record, sizeof(g_hmac_record), 0, sizeof(g_hmac_record));
    g_report_value = 0;
}

osal_u32 hmac_stop_period_task(osal_void)
{
    if (g_hmac_dfr_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_dfr_timer);
    }

    /* 删除定时器后，static中的值清空，下次打开重新统计 */
    hmac_period_task_init();
    return OAL_SUCC;
}

osal_u32 hmac_start_period_task(const hmac_vap_stru *hmac_vap)
{
    /* 启动周期检测维测定时器 */
    if (g_hmac_dfr_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_dfr_timer);
    }
    frw_create_timer_entry(&g_hmac_dfr_timer, hmac_period_check_task_timeout, g_period_task_timeout,
        (osal_void *)hmac_vap, OAL_FALSE);
    g_first_time_statistic = OSAL_TRUE;
    oam_warning_log1(0, OAM_SF_DFR, "hmac_start_period_task::g_first_time_statistic[%u].", g_first_time_statistic);
    return OAL_SUCC;
}

osal_s32 hmac_config_set_period_task(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    period_check_cfg *period_check_param = (period_check_cfg *)msg->data;
    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_DFR, "{hmac_config_set_period_task::hmac_vap(%p) or param(%p) is NULL.}",
            (uintptr_t)hmac_vap, (uintptr_t)msg->data);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_DFR,
        "hmac_config_set_period_task::g_period_task_timeout(%u) g_enable_switch(%u) g_cnt_period(%u).",
        period_check_param->timeout, period_check_param->switch_enable, period_check_param->cnt_period);
    if (period_check_param->switch_enable >= (1 << HMAC_DFR_MAX_NUM)) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_config_set_period_task::g_enable_switch(%u) out of range.}",
            period_check_param->switch_enable);
        return OAL_FAIL;
    }
    /* 时间周期为1000ms到60000ms */
    if ((period_check_param->timeout > 60000) || (period_check_param->timeout < 1000)) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_config_set_period_task::g_period_task_timeout(%u) out of range.}",
            period_check_param->timeout);
        return OAL_FAIL;
    }
    if (period_check_param->cnt_period < 1) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_config_set_period_task::g_cnt_period(%u) out of range.}",
            period_check_param->cnt_period);
        return OAL_FAIL;
    }

    g_period_task_timeout = period_check_param->timeout;
    g_enable_switch = period_check_param->switch_enable;
    g_cnt_period = period_check_param->cnt_period;
    /* 目前只有5个开关 */
    if ((g_enable_switch & ((1 << HMAC_DFR_MAX_NUM) - 1)) == 0) {
        /* 关闭定时器 */
        oam_warning_log0(0, OAM_SF_DFR, "hmac_config_set_period_task::stop the hmac_config_set_period_task.");
        return hmac_stop_period_task();
    }

    return hmac_start_period_task(hmac_vap);
}

osal_u32 hmac_get_user_tid_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info)
{
    osal_u16 user_id = index_info->hmac_dfr_check_info.tid_content.tid_user_id;
    osal_u16 tid_id = index_info->hmac_dfr_check_info.tid_content.tid_index;
    hmac_tid_stru *tid_queue = OSAL_NULL;
    hmac_user_stru *hmac_user = mac_res_get_hmac_user_etc(user_id);

    if (hmac_user == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_user_tid_info::hmac_user is NULL.}");
        return OAL_FAIL;
    }

    if (tid_id >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_get_user_tid_info::tid_id(%u) is out of range.}", tid_id);
        return OAL_FAIL;
    }
    tid_queue = &(hmac_user->tx_tid_queue[tid_id]);

    // 获取当前tid队列下的需要上报到HMAC的信息放在一个结构体中
    check_info->hmac_dfr_check_report_value.tid_content.is_paused = tid_queue->is_paused;
    check_info->hmac_dfr_check_report_value.tid_content.in_num = g_pack_in_cnt[user_id][tid_id];
    check_info->hmac_dfr_check_report_value.tid_content.out_num = g_pack_out_cnt[user_id][tid_id];
    check_info->hmac_dfr_check_report_value.tid_content.dispatch_num = g_dispatch_cnt[user_id][tid_id];

    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_user_tid_info::check_info is_paused(%u).", tid_queue->is_paused);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_user_tid_info::check_info in_num(%u).", g_pack_in_cnt[user_id][tid_id]);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_user_tid_info::check_info out_num(%u).",
        g_pack_out_cnt[user_id][tid_id]);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_user_tid_info::check_info dispatch_num(%u).",
        g_dispatch_cnt[user_id][tid_id]);

    return OAL_SUCC;
}

osal_u32 hmac_get_tx_dscr_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info)
{
    errno_t ret_err;
    osal_u16 index = index_info->hmac_dfr_check_info.tx_dscr_index;
    if (index >= osal_array_size(g_tx_dscr_statistics)) {
        oam_warning_log1(0, OAM_SF_DFR, "{tx_dscr_index::[%u] out of range.}.", index);
        return OAL_FAIL;
    }

    ret_err = memcpy_s(&check_info->hmac_dfr_check_report_value.tx_dscr_statistics_value,
        sizeof(check_info->hmac_dfr_check_report_value.tx_dscr_statistics_value),
        &g_tx_dscr_statistics[index], sizeof(check_info->hmac_dfr_check_report_value.tx_dscr_statistics_value));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_tx_dscr_info::hmac_get_tx_dscr_info:: memcpy_s failed.}");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_tx_dscr_info::tx_dscr_indexr[%u].", index);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_tx_dscr_info::g_tx_dscr_statistics: enqueue[%u].",
        g_tx_dscr_statistics[index].enqueue_total_cnt);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_tx_dscr_info::g_tx_dscr_statistics::dequeue[%u].",
        g_tx_dscr_statistics[index].dequeue_total_cnt);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_tx_dscr_info::g_tx_dscr_statistics: fail[%u].",
        g_tx_dscr_statistics[index].queue_fail_cnt);
    return OAL_SUCC;
}

osal_u32 hmac_get_rx_dscr_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info)
{
    osal_u16 index = index_info->hmac_dfr_check_info.rx_dscr_index;

    if (index >= osal_array_size(g_rx_dscr_statistics)) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_get_rx_dscr_info::[%u] out of range.}.", index);
        return OAL_FAIL;
    }

    check_info->hmac_dfr_check_report_value.rx_dscr_statistics_value = g_rx_dscr_statistics[index];

    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_rx_dscr_info::rx_dscr_indexr[%u].", index);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_rx_dscr_info::g_tx_dscr_statistics: available_cnt[%u].",
        g_rx_dscr_statistics[index]);
    return OAL_SUCC;
}

osal_u32 hmac_get_frw_event_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info)
{
    errno_t ret_err;
    osal_u16 core_id = index_info->hmac_dfr_check_info.frw_index.core_id;
    osal_u16 index = index_info->hmac_dfr_check_info.frw_index.event_index;

    if ((core_id >= WLAN_FRW_MAX_NUM_CORES) || (index >= FRW_POST_PRI_NUM)) {
        oam_warning_log2(0, OAM_SF_DFR, "{frw_event_info core_id::[%u] index[%u] out of range.}", core_id, index);
        return OAL_FAIL;
    }

    ret_err = memcpy_s(&check_info->hmac_dfr_check_report_value.frw_statistics_value,
        sizeof(check_info->hmac_dfr_check_report_value.frw_statistics_value),
        &g_frw_event_statistics[core_id][index], sizeof(check_info->hmac_dfr_check_report_value.frw_statistics_value));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_frw_event_info::memcpy_s failed.}");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_frw_event_info::frw_index.event_index[%u].", index);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_frw_event_info: g_frw_event_statistics::fail[%u].",
        g_frw_event_statistics[core_id][index].queue_fail_cnt);
    return OAL_SUCC;
}

osal_u32 hmac_get_mempool_info(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info)
{
    errno_t ret_err;
    osal_u32 index = index_info->hmac_dfr_check_info.mem_pool_index;

    if (index >= OAL_MEM_POOL_ID_BUTT) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_get_mempool_info::[%u] out of range.}", index);
        return OAL_FAIL;
    }

    ret_err = memcpy_s(&check_info->hmac_dfr_check_report_value.mem_pool_statistics_value,
        sizeof(check_info->hmac_dfr_check_report_value.mem_pool_statistics_value),
        &g_mempool_statistics[index], sizeof(check_info->hmac_dfr_check_report_value.mem_pool_statistics_value));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_mempool_info::memcpy_s failed.}");
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_mempool_info::hmac_dfr_info->mem_pool_index[%u].", index);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_mempool_info::g_mempool_statistics: alloc[%u].",
        g_mempool_statistics[index].alloc_total_cnt);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_mempool_info::g_mempool_statistics: free[%u].",
        g_mempool_statistics[index].free_total_cnt);
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_mempool_info::g_mempool_statistics: fail[%u].",
        g_mempool_statistics[index].fail_cnt);
    return OAL_SUCC;
}

osal_s32 hmac_get_dfr_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 queue_id;
    dfr_info_rsp_stru *rsp = (dfr_info_rsp_stru *)(msg->data);

    /* 缓存从device侧获取到的内存使用情况 */
    g_dfr_info[OAL_MEM_POOL_ID_NETBUF].mem_alloc_cnt = rsp->mem_alloc_cnt;
    g_dfr_info[OAL_MEM_POOL_ID_NETBUF].mem_total_cnt = rsp->mem_total_cnt;
    g_dfr_info[OAL_MEM_POOL_ID_NETBUF].mem_used_cnt = rsp->mem_used_cnt;
    g_dfr_info[OAL_MEM_POOL_ID_NETBUF].mem_fail_cnt = rsp->mem_fail_cnt;

    /* 缓存frw队列未被调用的数量 */
    g_dfr_info[FRW_POST_PRI_NUM].high_cnt = rsp->high_cnt;
    g_dfr_info[FRW_POST_PRI_NUM].low_cnt = rsp->low_cnt;

    /* 缓存从device侧获取到的tx、rx描述符队列情况 */
    for (queue_id = 0; queue_id < osal_array_size(rsp->tx_dscr_queue); queue_id++) {
        g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue[queue_id] = rsp->tx_dscr_queue[queue_id];
    }
    for (queue_id = 0; queue_id < osal_array_size(rsp->rx_dscr_queue); queue_id++) {
        g_dfr_info[HAL_RX_QUEUE_NUM].rx_dscr_queue[queue_id] = rsp->rx_dscr_queue[queue_id];
    }

    return OAL_SUCC;
}

osal_s32 hmac_get_check_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    errno_t ret_err;
    osal_u32 module_cnt;
    hmac_dfr_check_report check_info;
    hmac_dfr_info *hmac_get_dfr_info = (hmac_dfr_info *)msg->data;

    unref_param(hmac_vap);

    ret_err = memset_s(&check_info, sizeof(hmac_dfr_check_report), 0, sizeof(hmac_dfr_check_report));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_check_info::memset_s failed.}");
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_DFR, "hmac_get_check_info::hmac_dfr_info->event_type[%u].",
        hmac_get_dfr_info->event_type);

    for (module_cnt = 0; module_cnt < osal_array_size(query_module); module_cnt++) {
        if ((query_module[module_cnt].module_set & hmac_get_dfr_info->event_type) &&
            query_module[module_cnt].query_module_func != OSAL_NULL) {
            ret = query_module[module_cnt].query_module_func(hmac_get_dfr_info, &check_info);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_DFR, "{hmac_get_check_info::module_cnt[%u] failed.}", module_cnt);
            }
        }
    }
    check_info.event_type = hmac_get_dfr_info->event_type;

    ret_err = memcpy_s(msg->rsp, sizeof(check_info), (osal_u8 *)&check_info, sizeof(check_info));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_get_check_info::memset_s failed.}");
        return OAL_FAIL;
    }

    msg->rsp_len = (osal_u16)sizeof(check_info);
    return OAL_SUCC;
}

osal_void hmac_dfr_check_every_frw_queue(hmac_dfr_event *dfr_event, osal_u32 core_id, osal_u32 queue_id)
{
    if (g_first_time_statistic) {
        return;
    }
    /* 备份统计值 */
    if (queue_id == FRW_POST_PRI_HIGH) {
        g_frw_event_statistics[core_id][queue_id].queue_fail_cnt = g_dfr_info[FRW_POST_PRI_NUM].high_cnt;
    } else {
        g_frw_event_statistics[core_id][queue_id].queue_fail_cnt = g_dfr_info[FRW_POST_PRI_NUM].low_cnt;
    }

     /* 由统计值判断队列异常，并记录异常类型和队列编号 */
    if (g_frw_event_statistics[core_id][queue_id].queue_fail_cnt != 0) {
        dfr_event->event_type |= HMAC_DFR_FRW_QUEUE_FAIL_SET;
        g_hmac_record.frw_fail_cur[core_id][queue_id]++;
        dfr_event->frw_content[core_id] |= (1 << queue_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfr_check_frw_event_exception
 功能描述  : 检测frw事件队列异常信息
*****************************************************************************/
osal_void hmac_dfr_check_frw_event_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event)
{
    osal_u32 pri;
    osal_u32 core_id = 0;

    unref_param(hmac_vap);
    if (dfr_event == NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_check_frw_event_exception::dfr_event == NULL}");
        return;
    }

    /* 遍历每个frw事件队列 */
    for (pri = FRW_POST_PRI_HIGH; pri < FRW_POST_PRI_NUM; pri++) {
        /* 根据核找到相应的核上的事件管理结构体 */
        hmac_dfr_check_every_frw_queue(dfr_event, core_id, pri);
    }
}

/*****************************************************************************
 函 数 名: hmac_dfr_check_mempool_exception
 功能描述  : 检测内存池异常信息
*****************************************************************************/
osal_void hmac_dfr_check_mempool_exception(hmac_vap_stru *mac_vap, hmac_dfr_event *dfr_event)
{
    osal_u32 pool_id = OAL_MEM_POOL_ID_NETBUF;

    unref_param(mac_vap);
    if (dfr_event == NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_dfr_check_mempool_exception::dfr_event == NULL!}");
        return;
    }

    /* 备份统计值并检测异常 */
    if (!g_first_time_statistic) {
        g_mempool_statistics[pool_id].alloc_total_cnt = g_dfr_info[pool_id].mem_alloc_cnt;
        g_mempool_statistics[pool_id].free_total_cnt =
            g_dfr_info[pool_id].mem_alloc_cnt - g_dfr_info[pool_id].mem_used_cnt;
        g_mempool_statistics[pool_id].fail_cnt = g_dfr_info[pool_id].mem_fail_cnt;
        if (g_mempool_statistics[pool_id].free_total_cnt == 0 &&
            (g_dfr_info[pool_id].mem_used_cnt == g_dfr_info[pool_id].mem_total_cnt)) {
            dfr_event->mempool_content |= HMAC_DFR_MEM_POOL_NO_FREE_SET;
            g_hmac_record.mem_no_free_cur[pool_id]++;
            dfr_event->mempool_content |= (1 << pool_id);
        }
    }
    g_dfr_info[pool_id].mem_alloc_cnt = (osal_u32)(g_dfr_info[pool_id].mem_used_cnt);
    g_dfr_info[pool_id].mem_fail_cnt = 0;
    /* 由统计值判断内存池异常，并记录异常类型和内存池编号 */
    if (g_mempool_statistics[pool_id].fail_cnt != 0) {
        dfr_event->mempool_content |= HMAC_DFR_MEM_POOL_FAIL_SET;
        g_hmac_record.mem_fail_cur[pool_id]++;
        dfr_event->mempool_content |= (1 << pool_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfr_check_tx_dscr_exception
 功能描述  : 检测tx 硬件描述符队列异常信息
*****************************************************************************/
osal_void hmac_dfr_check_tx_dscr_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event)
{
    osal_u32 queue_id;
    osal_u32 queue_min_size;

    queue_min_size = (osal_array_size(g_tx_dscr_statistics) <=
        osal_array_size(g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue)) ?
        osal_array_size(g_tx_dscr_statistics) : osal_array_size(g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue);

    for (queue_id = 0; queue_id < queue_min_size; queue_id++) {
        /* 非首次检测时，统计队列是否卡住的错误 */
        if (!g_first_time_statistic) {
            g_tx_dscr_statistics[queue_id].enqueue_total_cnt =
                g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue[queue_id].enqueue_total_cnt;
            g_tx_dscr_statistics[queue_id].dequeue_total_cnt =
                g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue[queue_id].enqueue_total_cnt -
                g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue[queue_id].ppdu_cnt;

            if (g_tx_dscr_statistics[queue_id].dequeue_total_cnt == 0 &&
                g_tx_dscr_statistics[queue_id].enqueue_total_cnt != 0 &&
                g_dfr_info[HAL_TX_QUEUE_NUM].tx_dscr_queue[queue_id].queue_status != HAL_DSCR_QUEUE_BUSY) {
                dfr_event->event_type |= HMAC_DFR_TX_DSCR_QUEUE_HALT_SET;
                dfr_event->tx_dscr_content |= (1 << queue_id);
                g_hmac_record.tx_halt_cur[queue_id]++;
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfr_check_rx_dscr_exception
 功能描述  : 检测rx 硬件描述符队列异常信息
*****************************************************************************/
osal_void hmac_dfr_check_rx_dscr_exception(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event)
{
    osal_u32 queue_id;
    osal_u32 queue_min_size;
    osal_u16 threshold_list[] = {
        WLAN_MEM_NORMAL_RX_MAX_CNT * ERROR_THRESHOLD1 / ERROR_THRESHOLD2,
        WLAN_MEM_HIGH_RX_MAX_CNT * ERROR_THRESHOLD1 / ERROR_THRESHOLD2 + 1,
        WLAN_MEM_SMALL_RX_MAX_CNT * ERROR_THRESHOLD1 / ERROR_THRESHOLD2
    };

    queue_min_size = (osal_array_size(g_rx_dscr_statistics) <=
        osal_array_size(g_dfr_info[HAL_RX_QUEUE_NUM].rx_dscr_queue)) ?
        osal_array_size(g_rx_dscr_statistics) : osal_array_size(g_dfr_info[HAL_RX_QUEUE_NUM].rx_dscr_queue);

    for (queue_id = 0; queue_id < queue_min_size; queue_id++) {
        /* 非首次检测时，统计队列是否卡住的错误 */
        if (!g_first_time_statistic) {
            g_rx_dscr_statistics[queue_id] = g_dfr_info[HAL_RX_QUEUE_NUM].rx_dscr_queue[queue_id].element_cnt;
            /* 此处做防呆处理，判断pending_pkt中有数据，防止STA未连接时，产生误报 */
            if (g_rx_dscr_statistics[queue_id] < threshold_list[queue_id]) {
                dfr_event->event_type |= HMAC_DFR_RX_DSCR_QUEUE_HALT_SET;
                dfr_event->rx_dscr_content |= (1 << queue_id);
                g_hmac_record.rx_halt_cur[queue_id]++;
            }
        }
    }
}

osal_u32 hmac_reset_hw(hal_to_dmac_device_stru *hal_device, const hmac_reset_para_stru *reset_para)
{
    hal_error_state_stru error_state;
    osal_u32 int_save;
    error_state.error1_val.u32 = 0xffffffff;
    error_state.error2_val.u32 = 0xffffffff;

    /* 关闭中断 */
    int_save = frw_osal_irq_lock();
    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend();
    /* 停止mac phy接收 */
    hal_save_machw_phy_pa_status(hal_device);
    hal_disable_machw_phy_and_pa();
    /* 首先清空接收事件队列 */
    frw_event_flush_event_queue(WLAN_MSG_D2D_CRX_RX);
    frw_event_flush_event_queue(WLAN_MSG_D2D_RX);
    frw_event_flush_event_queue(WLAN_MSG_D2D_TX);

    /* 清除硬件发送缓冲区 */
    hal_clear_hw_fifo(hal_device);
    /* 清所有的中断 */
    hal_clear_mac_error_int_status(&error_state);
    hal_clear_mac_int_status(0xffffffff);
    /* 复位macphy */
    hal_reset_phy_machw(reset_para->reset_type, reset_para->reset_mac_mod,
        reset_para->reset_phy_reg, reset_para->reset_mac_reg);

    /* 使能中断 */
    frw_osal_irq_restore(int_save);
    /* 恢复 mac phy接收 */
    hal_recover_machw_phy_and_pa(hal_device);
    /* 使能硬件发送 */
    hal_set_machw_tx_resume();

    /* 重新触发调度 */
    hmac_tx_schedule(hal_device, WLAN_WME_AC_BE);
    return OAL_SUCC;
}

osal_s32 hmac_config_reset_hw(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_reset_para_stru reset_para;
    hal_reset_hw_type_enum_uint8 reset_type = *(msg->data);
    hal_to_dmac_device_stru *hal_device_base = hmac_vap->hal_device;
    if (hal_device_base == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_reset_hw::hal_device_base null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(&reset_para, sizeof(hmac_reset_para_stru), 0, sizeof(hmac_reset_para_stru));
    reset_para.reset_type = reset_type;
    reset_para.reset_mac_mod = HAL_RESET_MAC_LOGIC;
    reset_para.is_rx_dscrlist_reset = OSAL_TRUE;
    if (reset_para.reset_type == HAL_RESET_HW_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_reset_hw::reset type is BUTT}");
        return OAL_SUCC;
    }
    (osal_void)hmac_reset_hw(hal_device_base, &reset_para);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置队列复位
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_queue_reset(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_queue_reset_stru *event_queue = (mac_cfg_queue_reset_stru *)msg->data;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;
    unref_param(hmac_vap);

    oam_info_log2(0, OAM_SF_CFG, "{hmac_config_set_queue_reset::index(%d) mode(%d).}",
        event_queue->index, event_queue->mode);
    switch (event_queue->index) {
        case 0: /* 清除FRW队列 */
            if (event_queue->mode >= FRW_POST_PRI_NUM) {
                return OAL_ERR_CODE_BUTT;
            }
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_queue_reset::frw_event_flush_event_queue type(%d).}",
                event_queue->mode);
            break;
        case 1: /* 清除TID队列 */
            /* 删除用户tid队列中的所有信息 */
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_queue_reset::hmac_tid_clear.user_id(%d)}",
                event_queue->mode);
            hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(event_queue->mode);
            if (hmac_user == OSAL_NULL) {
                oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_queue_reset:user null}");
                return OAL_ERR_CODE_PTR_NULL;
            }
            hmac_tid_clear(hmac_user, hmac_device, OSAL_FALSE);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
