/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Latency Stat hmac function.
 * Create: 2021-08-20
 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_latency_stat.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#endif
#include "mac_resource_ext.h"
#include "hmac_vap.h"
#include "frw_ext_if.h"
#include "hmac_main.h"
#include "hmac_feature_interface.h"
#include "mac_frame_common_rom.h"
#include "hmac_hook.h"
#include "hmac_ccpriv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_LATENCY_STAT_C

#undef  THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
latency_stat_switch_stru g_hmac_latency_stat_switch = {0};
osal_u32                 g_latency_record_pkt_num_tx = 0;
osal_u16               **g_hmac_latency_tx = OSAL_NULL;
osal_u16               **g_hmac_latency_rx = OSAL_NULL;

#define LATENCY_TRACE_MAX_CNT 255

/*****************************************************************************
 功能描述  : 命令下发设置host侧时延统计开关与统计模式
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_switch_init(const osal_u8 *param)
{
    if (memcpy_s(&g_hmac_latency_stat_switch, sizeof(latency_stat_switch_stru), param,
        sizeof(latency_stat_switch_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_latency_switch_init::memcpy failed.");
    }

    /* 重置统计报文数为0 */
    g_latency_record_pkt_num_tx = 0;
}

/*****************************************************************************
 功能描述  : 判断新下发配置与已有配置是否一致
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_latency_switch_need_init(const osal_u8 *param)
{
    osal_u8 *old_config = OSAL_NULL;

    old_config = (osal_u8 *)(&g_hmac_latency_stat_switch);
    if (osal_memcmp(param, old_config, sizeof(g_hmac_latency_stat_switch) == 0)) {
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : 设置host侧时延统计开关状态
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_set_switch_state(osal_u8 enable)
{
    g_hmac_latency_stat_switch.report_enable = enable;
}

/*****************************************************************************
 功能描述  : 返回host侧时延统计开关状态
*****************************************************************************/
OSAL_STATIC osal_u8 hmac_latency_get_switch_state(osal_void)
{
    return g_hmac_latency_stat_switch.report_enable;
}

/*****************************************************************************
 功能描述  : 返回host侧时延统计方向
*****************************************************************************/
OSAL_STATIC latency_direct_enum_uint8 hmac_latency_get_report_direct(osal_void)
{
    return g_hmac_latency_stat_switch.direct;
}

/*****************************************************************************
 功能描述  : 返回host侧时延统计模式
*****************************************************************************/
OSAL_STATIC latency_direct_enum_uint8 hmac_latency_get_mode(osal_void)
{
    return g_hmac_latency_stat_switch.report_mode;
}

/*****************************************************************************
 功能描述  : 返回host侧时延统计存储结构
*****************************************************************************/
osal_u16** hmac_latency_get_storage_tx(osal_void)
{
    return g_hmac_latency_tx;
}

/*****************************************************************************
 功能描述  : 返回host侧时延统计存储结构
*****************************************************************************/
osal_u16** hmac_latency_get_storage_rx(osal_void)
{
    return g_hmac_latency_rx;
}

/*****************************************************************************
 功能描述  : Trace统计模式存储空间初始化
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_latency_trace_init_proc(latency_direct_enum_uint8 direct, osal_u16 trace_cnt,
    osal_u8 num_parts)
{
    osal_u16 **hmac_latency_trace = OSAL_NULL;
    osal_u16   i;

    hmac_latency_trace = (osal_u16 **)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, trace_cnt * sizeof(osal_u16 *), OAL_TRUE);
    if (hmac_latency_trace == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_latency_trace_init_proc::oal_mem_alloc failed.");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (i = 0; i < trace_cnt; i++) {
        hmac_latency_trace[i] = OSAL_NULL;
    }

    if (direct == LATENCY_RPT_DIR_TX) {
        g_hmac_latency_tx = hmac_latency_trace;
    } else {
        g_hmac_latency_rx = hmac_latency_trace;
    }

    for (i = 0; i < trace_cnt; i++) {
        hmac_latency_trace[i] = (osal_u16 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            num_parts * sizeof(osal_u16), OAL_TRUE);
        if (hmac_latency_trace[i] == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_latency_trace_init_proc::oal_mem_alloc failed.");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        (osal_void)memset_s(hmac_latency_trace[i], num_parts * sizeof(osal_u16), 0, num_parts * sizeof(osal_u16));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Trace统计模式存储空间初始化
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_latency_trace_init(osal_void)
{
    osal_u16                  trace_cnt;
    osal_u32                  ret;
    latency_direct_enum_uint8 direct;

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        ret = hmac_latency_trace_init_proc(LATENCY_RPT_DIR_TX, trace_cnt, TX_HMAC_NUM_PARTS);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_latency_trace_init::trace init tx fail[%d]}", ret);
            return ret;
        }
    }

    if (latency_report_has_rx(direct)) {
        ret = hmac_latency_trace_init_proc(LATENCY_RPT_DIR_RX, trace_cnt, RX_HMAC_NUM_PARTS);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_latency_trace_init::trace init rx fail[%d]}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Trace统计模式存储空间释放
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_trace_deinit_proc(latency_direct_enum_uint8 direct, osal_u16 trace_cnt)
{
    osal_u16   i;

    if (direct == LATENCY_RPT_DIR_TX) {
        for (i = 0; i < trace_cnt; i++) {
            if (g_hmac_latency_tx[i] != OSAL_NULL) {
                oal_mem_free(g_hmac_latency_tx[i], OAL_TRUE);
                g_hmac_latency_tx[i] = OSAL_NULL;
            }
        }
        if (g_hmac_latency_tx != OSAL_NULL) {
            oal_mem_free(g_hmac_latency_tx, OAL_TRUE);
            g_hmac_latency_tx = OSAL_NULL;
        }
    } else {
        for (i = 0; i < trace_cnt; i++) {
            if (g_hmac_latency_rx[i] != OSAL_NULL) {
                oal_mem_free(g_hmac_latency_rx[i], OAL_TRUE);
                g_hmac_latency_rx[i] = OSAL_NULL;
            }
        }
        if (g_hmac_latency_rx != OSAL_NULL) {
            oal_mem_free(g_hmac_latency_rx, OAL_TRUE);
            g_hmac_latency_rx = OSAL_NULL;
        }
    }
}

/*****************************************************************************
 功能描述  : Trace统计模式存储空间释放
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_trace_deinit(osal_void)
{
    osal_u16 trace_cnt;
    latency_direct_enum_uint8 direct;

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        hmac_latency_trace_deinit_proc(LATENCY_RPT_DIR_TX, trace_cnt);
    }

    if (latency_report_has_rx(direct)) {
        hmac_latency_trace_deinit_proc(LATENCY_RPT_DIR_RX, trace_cnt);
    }
}

/*****************************************************************************
 功能描述  : Trace模式时延结果上报
*****************************************************************************/
OSAL_STATIC osal_void hmac_report_latency_trace_proc(latency_direct_enum_uint8 direct, osal_u16 trace_cnt,
    osal_u8 num_parts)
{
    osal_u16   i, j;
    osal_u16 **hmac_latency_trace = OSAL_NULL;
    osal_u32  *total_delay_time = OSAL_NULL;
    char      *dir = OSAL_NULL;
    char      *comment = OSAL_NULL;
    char      *header = OSAL_NULL;

    if (direct == LATENCY_RPT_DIR_TX) {
        hmac_latency_trace = g_hmac_latency_tx;
        dir = "TX";
        comment = "HMAC_STAGE_1: Process pkt (amsdu/broadcast); HMAC_STAGE_2: Schedule and Tid process";
        header = "pktid,  stage_1,  stage_2  |  total  |";
    } else {
        hmac_latency_trace = g_hmac_latency_rx;
        dir = "RX";
        comment = "HMAC_STAGE_1: In hmac - Send to LAN; HMAC_STAGE_2: In ko - from Entry to hmac";
        header = "pktid,  stage_1,  stage_2  |  total  |";
    }

    if (hmac_latency_trace == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_report_latency_trace_proc::Nothing to report!}");
        return;
    }

    total_delay_time = (osal_u32 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, trace_cnt * sizeof(osal_u32), OAL_TRUE);
    if (total_delay_time == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_report_latency_trace_proc::not enough memory for report!}");
        return;
    }
    (osal_void)memset_s(total_delay_time, trace_cnt * sizeof(osal_u32), 0, trace_cnt * sizeof(osal_u32));

    /* 计算每个报文所有阶段整体时间 */
    for (i = 0; i < trace_cnt; i++) {
        for (j = 0; j < num_parts; j++) {
            total_delay_time[i] += hmac_latency_trace[i][j];
        }
    }

    wifi_printf("Latency Data Report - TRACE[%s][HMAC] - start\r\n", dir);
    wifi_printf("%s\r\n", comment);
    wifi_printf("%s\r\n", header);
    for (i = 0; i < trace_cnt; i++) {
        if (direct == LATENCY_RPT_DIR_TX) {
            wifi_printf("%5u %7u %7u     | %7u |\r\n", i + 1, hmac_latency_trace[i][0], hmac_latency_trace[i][1],
                total_delay_time[i]); /* 2:索引号 */
        } else {
            wifi_printf("%5u  %7u  %7u    | %7u |\r\n", i + 1, hmac_latency_trace[i][0], hmac_latency_trace[i][1],
                total_delay_time[i]);
        }
    }

    oal_mem_free(total_delay_time, OAL_TRUE);
}

/*****************************************************************************
 功能描述  : Trace模式时延结果上报
*****************************************************************************/
OSAL_STATIC osal_void hmac_report_latency_trace(osal_void)
{
    osal_u16 trace_cnt;
    latency_direct_enum_uint8 direct;

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        hmac_report_latency_trace_proc(LATENCY_RPT_DIR_TX, trace_cnt, TX_HMAC_NUM_PARTS);
    }
    if (latency_report_has_rx(direct)) {
        hmac_report_latency_trace_proc(LATENCY_RPT_DIR_RX, trace_cnt, RX_HMAC_NUM_PARTS);
    }
}

/*****************************************************************************
 功能描述  : Trace模式时延结果清零
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_clear_latency_trace(osal_void)
{
    osal_u16 trace_cnt;
    osal_u16 i;
    latency_direct_enum_uint8 direct;

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct) && g_hmac_latency_tx == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_trace::clear latency results failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (latency_report_has_rx(direct) && g_hmac_latency_rx == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_trace::clear latency results failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (i = 0; i < trace_cnt; i++) {
        if (latency_report_has_tx(direct)) {
            if (memset_s(g_hmac_latency_tx[i], TX_HMAC_NUM_PARTS * sizeof(osal_u16), 0,
                         TX_HMAC_NUM_PARTS * sizeof(osal_u16)) != EOK) {
                oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_trace::clear latency results failed.");
                return OAL_FAIL;
            };
        }
        if (latency_report_has_rx(direct)) {
            if (memset_s(g_hmac_latency_rx[i], RX_HMAC_NUM_PARTS * sizeof(osal_u16), 0,
                         RX_HMAC_NUM_PARTS * sizeof(osal_u16)) != EOK) {
                oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_trace::clear latency results failed.");
                return OAL_FAIL;
            };
        }
    }
    g_latency_record_pkt_num_tx = 0;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Stats统计模式存储空间初始化
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_latency_stats_init_proc(latency_direct_enum_uint8 direct, osal_u8 num_parts)
{
    osal_u16 **hmac_latency_stats = OSAL_NULL;
    osal_u16   i;

    hmac_latency_stats = (osal_u16 **)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, num_parts * sizeof(osal_u16 *), OAL_TRUE);
    if (hmac_latency_stats == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_latency_stats_init_proc::oal_mem_alloc failed.");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (i = 0; i < num_parts; i++) {
        hmac_latency_stats[i] = OSAL_NULL;
    }

    if (direct == LATENCY_RPT_DIR_TX) {
        g_hmac_latency_tx = hmac_latency_stats;
    } else {
        g_hmac_latency_rx = hmac_latency_stats;
    }

    for (i = 0; i < num_parts; i++) {
        hmac_latency_stats[i] = (osal_u16 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, LATENCY_STATS_NUM_TIME_SLOTS *
            sizeof(osal_u16), OAL_TRUE);
        if (hmac_latency_stats[i] == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_latency_stats_init_proc::oal_mem_alloc failed.");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        (osal_void)memset_s(hmac_latency_stats[i], LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16), 0,
            LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Stats统计模式存储空间初始化
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_latency_stats_init(osal_void)
{
    osal_u32                  ret;
    latency_direct_enum_uint8 direct;

    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        ret = hmac_latency_stats_init_proc(LATENCY_RPT_DIR_TX, TX_HMAC_NUM_PARTS);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_latency_stats_init::trace init tx fail[%d]}", ret);
            return ret;
        }
    }

    if (latency_report_has_rx(direct)) {
        ret = hmac_latency_stats_init_proc(LATENCY_RPT_DIR_RX, RX_HMAC_NUM_PARTS);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_latency_stats_init::trace init rx fail[%d]}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Stats统计模式存储空间释放
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_stats_deinit_proc(latency_direct_enum_uint8 direct)
{
    osal_u16   idx;

    if (direct == LATENCY_RPT_DIR_TX) {
        for (idx = 0; idx < TX_HMAC_NUM_PARTS; idx++) {
            if (g_hmac_latency_tx[idx] != OSAL_NULL) {
                oal_mem_free(g_hmac_latency_tx[idx], OAL_TRUE);
                g_hmac_latency_tx[idx] = OSAL_NULL;
            }
        }
        if (g_hmac_latency_tx != OSAL_NULL) {
            oal_mem_free(g_hmac_latency_tx, OAL_TRUE);
            g_hmac_latency_tx = OSAL_NULL;
        }
    } else {
        for (idx = 0; idx < RX_HMAC_NUM_PARTS; idx++) {
            if (g_hmac_latency_rx[idx] != OSAL_NULL) {
                oal_mem_free(g_hmac_latency_rx[idx], OAL_TRUE);
                g_hmac_latency_rx[idx] = OSAL_NULL;
            }
        }
        if (g_hmac_latency_rx != OSAL_NULL) {
            oal_mem_free(g_hmac_latency_rx, OAL_TRUE);
            g_hmac_latency_rx = OSAL_NULL;
        }
    }
}

/*****************************************************************************
 功能描述  : Stats统计模式存储空间释放
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_stats_deinit(osal_void)
{
    latency_direct_enum_uint8 direct;

    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        hmac_latency_stats_deinit_proc(LATENCY_RPT_DIR_TX);
    }

    if (latency_report_has_rx(direct)) {
        hmac_latency_stats_deinit_proc(LATENCY_RPT_DIR_RX);
    }
}

/*****************************************************************************
 功能描述  : Stats模式时延结果上报
*****************************************************************************/
OSAL_STATIC osal_void hmac_report_latency_stats_proc(latency_direct_enum_uint8 direct, osal_u8 num_parts)
{
    osal_u16   i;
    osal_u16 **hmac_latency_stats = OSAL_NULL;
    char      *dir = OSAL_NULL;

    if (direct == LATENCY_RPT_DIR_TX) {
        hmac_latency_stats = g_hmac_latency_tx;
        dir = "TX";
    } else {
        hmac_latency_stats = g_hmac_latency_rx;
        dir = "RX";
    }

    if (hmac_latency_stats == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_report_latency_trace_proc::Nothing to report!}");
        return;
    }

    /* report stats data */
    wifi_printf("Latency Data Report - STATS[%s][HMAC] - start\r\n", dir);
    wifi_printf("Time Slots      | 0-50us |, |50-200us|, |200us-1ms|, |1ms-10ms|, |10ms-40ms|, | >40ms |\r\n");
    for (i = 0; i < num_parts; i++) {
        wifi_printf("%s_HMAC_STAGE_%d [%6u, %6u, %6u, %6u, %6u, %6u]\r\n", dir, (i + 1),
            /* 间隔定义 0:[0-50us], 1:[50-200us], 2:[200us-1ms], 3:[1ms-10ms], 4:[10ms-40ms], 5:[>40ms] */
            hmac_latency_stats[i][0], hmac_latency_stats[i][1], hmac_latency_stats[i][2], /* 2引索号 */
            hmac_latency_stats[i][3], hmac_latency_stats[i][4], hmac_latency_stats[i][5]); /* 3,4,5引索号 */
    }
}

/*****************************************************************************
 功能描述  : Stats模式时延结果上报
*****************************************************************************/
OSAL_STATIC osal_void hmac_report_latency_stats(osal_void)
{
    latency_direct_enum_uint8 direct;

    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct)) {
        hmac_report_latency_stats_proc(LATENCY_RPT_DIR_TX, TX_HMAC_NUM_PARTS);
    }
    if (latency_report_has_rx(direct)) {
        hmac_report_latency_stats_proc(LATENCY_RPT_DIR_RX, RX_HMAC_NUM_PARTS);
    }
}

/*****************************************************************************
 功能描述  : Stats模式时延结果清零
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_clear_latency_stats(osal_void)
{
    osal_u16                  i;
    latency_direct_enum_uint8 direct;

    direct = g_hmac_latency_stat_switch.direct;

    if (latency_report_has_tx(direct) && g_hmac_latency_tx == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_stats::clear latency results failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (latency_report_has_rx(direct) && g_hmac_latency_rx == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_stats::clear latency results failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (latency_report_has_tx(direct)) {
        for (i = 0; i < TX_HMAC_NUM_PARTS; i++) {
            if (memset_s(g_hmac_latency_tx[i], LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16), 0,
                         LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16)) != EOK) {
                oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_stats::clear latency results failed.");
                return OAL_FAIL;
            };
        }
    }
    if (latency_report_has_rx(direct)) {
        for (i = 0; i < RX_HMAC_NUM_PARTS; i++) {
            if (memset_s(g_hmac_latency_rx[i], LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16), 0,
                         LATENCY_STATS_NUM_TIME_SLOTS * sizeof(osal_u16)) != EOK) {
                oam_warning_log0(0, OAM_SF_CFG, "hmac_clear_latency_stats::clear latency results failed.");
                return OAL_FAIL;
            };
        }
    }

    /* 重置统计报文个数，打开统计开关 */
    g_latency_record_pkt_num_tx = 0;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 判断时延使能开关是否打开，是否是TX方向统计
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_latency_is_enbale_tx(osal_void)
{
    if ((hmac_latency_get_switch_state() == OAL_TRUE) &&
        latency_report_has_tx(hmac_latency_get_report_direct())) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断时延使能开关是否打开，是否是RX方向统计
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_latency_is_enable_rx(osal_void)
{
    if (((hmac_latency_get_switch_state()) == OAL_TRUE) &&
        latency_report_has_rx(hmac_latency_get_report_direct())) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : TX Entry Point记录时间戳
*****************************************************************************/
OSAL_STATIC osal_void hmac_latency_stat_tx_entry(oal_netbuf_stru *buf)
{
    osal_u8                 index;
    osal_u8                 mode;
    osal_u16                trace_cnt;
    osal_u32                curr_timestamp;
    mac_tx_ctl_stru        *cb = OSAL_NULL;

    if (hmac_latency_is_enbale_tx() != OAL_TRUE) {
        return;
    }

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    mode = g_hmac_latency_stat_switch.report_mode;

    /* 报文未标记状态时index值为0，为区分，标记时index从1开始 */
    g_latency_record_pkt_num_tx++;
    index = (osal_u8)g_latency_record_pkt_num_tx;

    /* TRACE模式下仅统计trace_cnt个报文 */
    if (mode == LATENCY_REPORT_MODE_TRACE && g_latency_record_pkt_num_tx > trace_cnt) {
        return;
    }

    cb = (mac_tx_ctl_stru *)oal_netbuf_cb(buf);
    cb->latency_index = index;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    curr_timestamp = osal_get_time_stamp_ts() & LATENCY_TIMESTAMP_MASK;
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    curr_timestamp = ((osal_u32)ktime_to_us(ktime_get())) & LATENCY_TIMESTAMP_MASK;
#endif
    cb->latency_timestamp_us = curr_timestamp;
}

/*****************************************************************************
 功能描述  : 存储阶段耗时 - Stats模式
*****************************************************************************/
OSAL_STATIC osal_void hmac_record_latency_stats(latency_direct_enum_uint8 direct, osal_u32 diff_timestamp,
    latency_tx_parts_enum_uint8 part)
{
    osal_u16 **hmac_latency_trace = OSAL_NULL;

    if (direct == LATENCY_RPT_DIR_TX) {
        hmac_latency_trace = g_hmac_latency_tx;
    } else {
        hmac_latency_trace = g_hmac_latency_rx;
    }

    /* 间隔定义 0:[0-50us], 1:[50-200us], 2:[200us-1ms], 3:[1ms-10ms], 4:[10ms-40ms], 5:[>40ms] */
    if (diff_timestamp <= LATENCY_STATS_TIME_SLOT_1) {
        hmac_latency_trace[part][0]++;
    } else if (diff_timestamp <= LATENCY_STATS_TIME_SLOT_2) {
        hmac_latency_trace[part][1]++;
    } else if (diff_timestamp <= LATENCY_STATS_TIME_SLOT_3) {
        hmac_latency_trace[part][2]++;                 /* 2:[200us-1ms] */
    } else if (diff_timestamp <= LATENCY_STATS_TIME_SLOT_4) {
        hmac_latency_trace[part][3]++;                 /* 3:[1ms-10ms] */
    } else if (diff_timestamp <= LATENCY_STATS_TIME_SLOT_5) {
        hmac_latency_trace[part][4]++;                 /* 4:[10ms-40ms] */
    } else {
        hmac_latency_trace[part][5]++;                 /* 5:[>40ms] */
    }
}

/*****************************************************************************
 功能描述  : TX 转发节点记录阶段耗时
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_record_latency_tx(oal_netbuf_stru *buf, latency_tx_parts_enum_uint8 part)
{
    osal_u8                 mode;
    osal_u8                 index;
    osal_u16                trace_cnt;
    osal_u32                curr_timestamp;
    osal_u32                last_timestamp;
    osal_u16                diff_timestamp;
    mac_tx_ctl_stru        *cb = OSAL_NULL;

    if (hmac_latency_is_enbale_tx() != OAL_TRUE) {
        return OAL_FALSE;
    }

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    mode = g_hmac_latency_stat_switch.report_mode;

    if (buf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_record_latency_tx::buf is null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取index信息 */
    cb = (mac_tx_ctl_stru *)oal_netbuf_cb(buf);
    index = cb->latency_index;

    /* TRACE模式下，没有标记的报文不做统计 */
    if (mode == LATENCY_REPORT_MODE_TRACE && index == 0) {
        return OAL_SUCC;
    }

    /* 获取时间信息 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    curr_timestamp = osal_get_time_stamp_ts() & LATENCY_TIMESTAMP_MASK;
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    curr_timestamp = ((osal_u32)ktime_to_us(ktime_get())) & LATENCY_TIMESTAMP_MASK;
#endif
    last_timestamp = cb->latency_timestamp_us;

    /* 计算阶段耗时，考虑翻转情况，并存入当前时间戳 */
    diff_timestamp = (osal_u16)get_runtime(last_timestamp, curr_timestamp);
    cb->latency_timestamp_us = curr_timestamp;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    /* osal_get_time_stamp_ts精度为31.25微秒，所以乘125除4 */
    diff_timestamp = diff_timestamp * 125 / 4;
#endif

    /* 根据用户输入模式存入耗时时间差，由于index标记从1开始，存储时需-1 */
    if ((mode == LATENCY_REPORT_MODE_TRACE) && (index <= trace_cnt) && (index > 0)) {
        g_hmac_latency_tx[index - 1][part] = diff_timestamp;
    } else if (mode == LATENCY_REPORT_MODE_STATS) {
        hmac_record_latency_stats(LATENCY_RPT_DIR_TX, diff_timestamp, part);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : RX Entry Point记录时间戳
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_latency_stat_rx_entry(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    osal_u8                   mode;
    osal_u8                   index;
    osal_u32                  curr_timestamp;
    dmac_rx_ctl_stru         *cb = OSAL_NULL;
    hal_rx_ctl_stru          *cb_latency = OSAL_NULL;
    osal_u8 sub_type;
    unref_param(hmac_vap);

    if (hmac_latency_is_enable_rx() != OAL_TRUE || netbuf == OSAL_NULL || *netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    mode = g_hmac_latency_stat_switch.report_mode;
    cb = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf);
    cb_latency = (hal_rx_ctl_stru *)&(cb->rx_info);
    index = cb_latency->latency_index;

    /* TRACE模式下，没有标记的报文不做统计 */
    if (mode == LATENCY_REPORT_MODE_TRACE && index == 0) {
        return OAL_CONTINUE;
    }

    sub_type = mac_get_frame_type_and_subtype((osal_u8 *)oal_netbuf_header(*netbuf));
    if (sub_type == (WLAN_FC0_SUBTYPE_NODATA | WLAN_FC0_TYPE_DATA)) {
        return OAL_CONTINUE;
    }

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    curr_timestamp = osal_get_time_stamp_ts() & LATENCY_TIMESTAMP_MASK;
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    curr_timestamp = ((osal_u32)ktime_to_us(ktime_get())) & LATENCY_TIMESTAMP_MASK;
#endif
    cb_latency->latency_timestamp_us = curr_timestamp;
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : RX 转发节点记录阶段耗时
*****************************************************************************/
OSAL_STATIC osal_void hmac_record_latency_rx(oal_netbuf_stru *buf, latency_rx_parts_enum_uint8 part)
{
    osal_u8                 mode;
    osal_u8                 index;
    osal_u16                trace_cnt;
    osal_u16                delay_time;
    osal_u32                curr_timestamp;
    osal_u32                last_timestamp;
    dmac_rx_ctl_stru       *cb = OSAL_NULL;
    hal_rx_ctl_stru        *cb_latency = OSAL_NULL;
    osal_u8 sub_type;

    if (hmac_latency_is_enable_rx() != OAL_TRUE) {
        return;
    }

    trace_cnt = g_hmac_latency_stat_switch.trace_cnt;
    mode = g_hmac_latency_stat_switch.report_mode;

    cb = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);
    cb_latency = (hal_rx_ctl_stru *)&(cb->rx_info);
    index = cb_latency->latency_index;

    /* TRACE模式下，没有标记的报文不做统计 */
    if ((mode == LATENCY_REPORT_MODE_TRACE) && (index == 0)) {
        return;
    }

    sub_type = mac_get_frame_type_and_subtype((osal_u8 *)oal_netbuf_header(buf));
    if (sub_type == (WLAN_FC0_SUBTYPE_NODATA | WLAN_FC0_TYPE_DATA)) {
        return;
    }

    /* 获取时间信息 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    curr_timestamp = osal_get_time_stamp_ts() & LATENCY_TIMESTAMP_MASK;
#elif defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    curr_timestamp = ((osal_u32)ktime_to_us(ktime_get())) & LATENCY_TIMESTAMP_MASK;
#endif
    last_timestamp = cb_latency->latency_timestamp_us;

    /* 计算阶段耗时，考虑翻转情况，并存入当前时间戳 */
    delay_time = (osal_u16)get_runtime(last_timestamp, curr_timestamp);
    cb_latency->latency_timestamp_us = curr_timestamp;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    /* osal_get_time_stamp_ts精度为31.25微秒，所以乘125除4 */
    delay_time = delay_time * 125 / 4;
#endif

    /* 根据用户输入模式存入耗时时间差，由于index标记从1开始，存储时需-1 */
    if ((mode == LATENCY_REPORT_MODE_TRACE) && (index <= trace_cnt) && (index > 0)) {
        g_hmac_latency_rx[index - 1][part] = delay_time;
    } else if (mode == LATENCY_REPORT_MODE_STATS) {
        hmac_record_latency_stats(LATENCY_RPT_DIR_RX, delay_time, part);
    }
}

OSAL_STATIC osal_u32 hmac_record_latency_rx_one(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
    if (netbuf == OSAL_NULL || *netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }
    hmac_record_latency_rx(*netbuf, RX_HMAC_KO_PROC);
    return OAL_CONTINUE;
}

OSAL_STATIC osal_u32 hmac_record_latency_rx_two(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
    if (netbuf == OSAL_NULL || *netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }
    hmac_record_latency_rx(*netbuf, RX_TRANS_TO_LAN);
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 时延统计开启初始化内存
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_latency_stat_on(const latency_stat_switch_stru *param)
{
    osal_s32 ret;
    latency_direct_enum_uint8 direct;

    direct = hmac_latency_get_report_direct();
    /* 避免重复申请内存 */
    if (latency_report_has_tx(direct) && LATENCY_TX_STORAGE_EXIST) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_latency_stat_on::tx storage already exists}");
        return OAL_FAIL;
    }

    if (latency_report_has_rx(direct) && LATENCY_RX_STORAGE_EXIST) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_latency_stat_on::rx storage already exists}");
        return OAL_FAIL;
    }

    /* 根据用户设定的统计模式初始化对应的储存结构 */
    if (param->report_mode == LATENCY_REPORT_MODE_TRACE) {
        ret = (osal_s32)hmac_latency_trace_init();
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_latency_stat_on::trace memory alloc fail[%d].}", ret);
            return ret;
        }
    } else {
        ret = (osal_s32)hmac_latency_stats_init();
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_latency_stat_on::trace memory alloc fail[%d].}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 时延统计关闭释放内存
*****************************************************************************/
OSAL_STATIC osal_void hmac_config_latency_stat_off(osal_void)
{
    latency_direct_enum_uint8 direct;

    direct = hmac_latency_get_report_direct();

    hmac_latency_set_switch_state(OSAL_FALSE);

    /* 避免重复释放内存 */
    if (latency_report_has_tx(direct) && !LATENCY_TX_STORAGE_EXIST) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_latency_stat_off::tx storage already freed}");
        return;
    }

    if (latency_report_has_rx(direct) && !LATENCY_RX_STORAGE_EXIST) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_latency_stat_off::rx storage already freed}");
        return;
    }

    if (hmac_latency_get_mode() == LATENCY_REPORT_MODE_TRACE) {
        hmac_latency_trace_deinit();
    } else {
        hmac_latency_stats_deinit();
    }
}

hmac_netbuf_hook_stru g_rx_latency_stat_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_HIGH,
    .hook_func = hmac_latency_stat_rx_entry,
};

hmac_netbuf_hook_stru g_rx_latency_record_one_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_record_latency_rx_one,
};

hmac_netbuf_hook_stru g_rx_latency_record_two_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_H2O,
    .priority = HMAC_HOOK_PRI_LOW,
    .hook_func = hmac_record_latency_rx_two,
};

OSAL_STATIC osal_void hmac_latency_feature_hook_register(osal_void)
{
    /* 注册对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_LATENCY_STAT_TX_ENTRY, hmac_latency_stat_tx_entry);
    hmac_feature_hook_register(HMAC_FHOOK_RECORD_LATENCY_TX, hmac_record_latency_tx);
    (osal_void)hmac_register_netbuf_hook(&g_rx_latency_stat_hook);
    (osal_void)hmac_register_netbuf_hook(&g_rx_latency_record_one_hook);
    (osal_void)hmac_register_netbuf_hook(&g_rx_latency_record_two_hook);
    return;
}

OSAL_STATIC osal_void hmac_latency_feature_hook_unregister(osal_void)
{
    /* 去注册对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_LATENCY_STAT_TX_ENTRY);
    hmac_feature_hook_unregister(HMAC_FHOOK_RECORD_LATENCY_TX);
    (osal_void)hmac_unregister_netbuf_hook(&g_rx_latency_stat_hook);
    (osal_void)hmac_unregister_netbuf_hook(&g_rx_latency_record_one_hook);
    (osal_void)hmac_unregister_netbuf_hook(&g_rx_latency_record_two_hook);
    return;
}

/*****************************************************************************
 功能描述  : 设置时延统计开关与统计模式
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_latency_stat_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    latency_stat_switch_stru *switch_param = (latency_stat_switch_stru *)(msg->data);

    /* 根据开关状态初始化或释放内存 */
    if (switch_param->report_enable == OAL_TRUE) {
        /* 重复enable且配置一致时无需后续初始化操作 */
        if (hmac_latency_switch_need_init(msg->data) == OSAL_FALSE) {
            return OAL_SUCC;
        }

        /* 重复enable且新旧配置不一致时需先释放原有存储结构，再重新初始化新内存 */
        if (hmac_latency_get_switch_state() == OSAL_TRUE) {
            hmac_config_latency_stat_off();
        }

        /* 设置统计参数 */
        hmac_latency_switch_init(msg->data);

        /* 初始化内存 */
        ret = hmac_config_latency_stat_on(switch_param);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
                "{hmac_config_latency_stat_switch::switch on init failed[%d].}", ret);
            /* 初始化异常时关闭开关， 释放内存 */
            hmac_config_latency_stat_off();
            return ret;
        }
        /* 对外接口注册 */
        hmac_latency_feature_hook_register();
    } else {
        /* 对外接口去注册 */
        hmac_latency_feature_hook_unregister();
        /* 关闭开关，释放内存 */
        hmac_config_latency_stat_off();
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_LATENCY_STAT_SWITCH, msg, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
            "{hmac_config_latency_stat_switch::hmac_config_send_event_etc failed[%d].}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 时延统计结果输出
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_report_latency_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    if (hmac_latency_get_switch_state() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_config_report_latency_stat::switch is off, nothing to report.");
        return OSAL_FALSE;
    }

    if (hmac_latency_get_mode() == LATENCY_REPORT_MODE_TRACE) {
        hmac_report_latency_trace();
    } else {
        hmac_report_latency_stats();
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_REPORT_LATENCY_STAT, msg, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
            "{hmac_config_report_latency_stat::send msg failed[%d].}", ret);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 时延统计清空
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_clear_latency_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    osal_u32 ret_latency;

    oam_warning_log0(0, OAM_SF_CFG, "clear stats in hmac!");

    if (hmac_latency_get_mode() == LATENCY_REPORT_MODE_TRACE) {
        ret_latency = hmac_clear_latency_trace();
        if (ret_latency != OAL_SUCC) {
            oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
                "{hmac_config_clear_latency_stat::clear latency stat failed[%d].}", ret_latency);
        }
    } else {
        ret_latency = hmac_clear_latency_stats();
        if (osal_unlikely(ret_latency != OAL_SUCC)) {
            oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
                "{hmac_config_clear_latency_stat::clear latency stat failed[%d].}", ret_latency);
        }
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_CLEAR_LATENCY_STAT, msg, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
            "{hmac_config_clear_latency_stat::hmac_config_send_event_etc failed[%d].}", ret);
    }

    return ret;
}

/*****************************************************************************************
 功能描述  : 时延统计功能命令解析
 被调函数  : hmac_ccpriv_latency_stat_parse_args
******************************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_latency_stat_parse_args(latency_stat_switch_stru *latency_stat_switch,
    const osal_s8 *pc_param)
{
    osal_s32 ret;
    osal_u32 trace_cnt;
    osal_u8 enable, mode, direct;
    osal_char arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    // 获取第一个参数enable:[0|1]
    ret = hmac_ccpriv_get_u8_with_check_max(&pc_param, 1, &enable);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_latency_stat_parse_args::get enable arg err_code [%d] !}", ret);
        return ret;
    }

    // 时延统计功能关闭时后续参数无需解析
    if (enable == OSAL_FALSE) {
        return OAL_SUCC;
    }
    latency_stat_switch->report_enable = enable;

    // 获取第二个参数direct，统计方向(0: tx, 1: rx, 2: tx and rx)
    ret = hmac_ccpriv_get_u8_with_check_max(&pc_param, 2, &direct);
    if (ret != OAL_SUCC) {
        // 后续参数皆缺省时，设置默认模式：direct = 2-txrx双向统计, mode = 1-Stats模式
        latency_stat_switch->direct = 2;
        latency_stat_switch->report_mode = 1;
        return OAL_SUCC;
    }
    latency_stat_switch->direct = direct;

    // 获取第三个参数mode，统计模式(0: trace, 1: stats)
    ret = hmac_ccpriv_get_u8_with_check_max(&pc_param, 1, &mode);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_latency_stat_parse_args::get mode arg err_code [%d]!}", ret);
        return ret;
    }
    latency_stat_switch->report_mode = mode;

    // 时延统计模式为stats时后续参数无需解析
    if (mode == LATENCY_REPORT_MODE_STATS) {
        return OAL_SUCC;
    }
    // 获取第四个参数trace_cnt，trace模式下统计的报文个数
    ret = hmac_ccpriv_get_one_arg(&pc_param, (osal_s8 *)arg, OAL_SIZEOF(arg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_latency_stat_parse_args::get trace_cnt arg err_code [%d]!}", ret);
        return ret;
    }

    trace_cnt = (osal_u32)oal_atoi((const osal_s8 *)arg);
    if (trace_cnt <= 0 || trace_cnt > LATENCY_TRACE_MAX_CNT) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_latency_stat_parse_args::trace_cnt arg value not supported!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    latency_stat_switch->trace_cnt = (osal_u16)trace_cnt;

    return OAL_SUCC;
}

/***************************************************************************************************
 功能描述  : 时延统计功能开关与统计模式配置
 使用方法  : echo "wlan0 latency_stat <enable> <direct> <mode> <trace_cnt>" > /sys/ccsys/ccpriv
****************************************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_latency_stat_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32                     ret;
    latency_stat_switch_stru     latency_stat_switch_param;
    frw_msg msg;

    ret = hmac_ccpriv_latency_stat_parse_args(&latency_stat_switch_param, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_latency_stat_switch::get args err_code [%d]!}", ret);
        return ret;
    }

    frw_msg_init((osal_u8 *)&latency_stat_switch_param, sizeof(latency_stat_switch_stru), OSAL_NULL, 0, &msg);
    hmac_config_latency_stat_switch(hmac_vap, &msg);
    return OAL_SUCC;
}

/*****************************************************************************************
 功能描述  : 时延统计结果输出
 使用方法  : echo "wlan0 report_latency_stat" > /sys/ccsys/ccpriv
******************************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_report_latency_stat(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    frw_msg msg;

    unref_param(param);
    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg);
    hmac_config_report_latency_stat(hmac_vap, &msg);
    return OAL_SUCC;
}

/*****************************************************************************************
 功能描述  : 时延统计清空
 使用方法  : echo "wlan0 clear_latency_stat" > /sys/ccsys/ccpriv
******************************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_clear_latency_stat(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    frw_msg msg;

    unref_param(param);
    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg);
    hmac_config_clear_latency_stat(hmac_vap, &msg);
    return OAL_SUCC;
}

osal_u32 hmac_latency_stat_init(osal_void)
{
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"latency_stat", hmac_ccpriv_latency_stat_switch);
    hmac_ccpriv_register((const osal_s8 *)"report_latency_stat", hmac_ccpriv_report_latency_stat);
    hmac_ccpriv_register((const osal_s8 *)"clear_latency_stat", hmac_ccpriv_clear_latency_stat);
    return OAL_SUCC;
}

osal_void hmac_latency_stat_deinit(osal_void)
{
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"latency_stat");
    hmac_ccpriv_unregister((const osal_s8 *)"report_latency_stat");
    hmac_ccpriv_unregister((const osal_s8 *)"clear_latency_stat");
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif