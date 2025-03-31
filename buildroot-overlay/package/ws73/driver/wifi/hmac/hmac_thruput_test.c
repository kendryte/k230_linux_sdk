/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: thruput test file.
 */

#include "hmac_thruput_test.h"
#include "oam_ext_if.h"
#include "dmac_ext_if_hcm.h"
#if defined(CONFIG_CACHE_MISS_SUPPORT) || defined(_PRE_WLAN_PEAK_PERFORMANCE_DFX)
#include "arch/regs.h"
#include "los_task_pri.h"
#endif
#ifdef _PRE_WLAN_PEAK_PERFORMANCE_DFX
#include "tcxo.h"
#include "hcc_dfx.h"
#endif
#include "hmac_alg_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_THRUPUT_TEST_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#define THRUPUT_TEST_1S 1000

osal_u8  g_thruput_type[THRUPUT_TYPE_BUTT] = {0};
osal_u32 g_thruput_time;
osal_u32 g_thruput_value;

#if defined(CONFIG_CACHE_MISS_SUPPORT) || defined(_PRE_WLAN_PEAK_PERFORMANCE_DFX)
static WIFI_TCM_TEXT osal_void hmac_thruput_cache_stat_start(osal_void);
static WIFI_TCM_TEXT osal_void hmac_thruput_cache_stat_end(osal_void);
#endif

/*****************************************************************************
 功能描述  : 双天线RSSI处理逻辑
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u8 hmac_is_thruput_test(osal_void)
{
    return (g_thruput_type[THRUPUT_NOACK] != 0) || (g_thruput_type[THRUPUT_DEV_WIFI] != 0) ||
           (g_thruput_type[THRUPUT_HAL_BYPASS] != 0);
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u8 hmac_is_thruput_enable(osal_u8 type)
{
    if (type >= THRUPUT_TYPE_BUTT) {
        return 0;
    }
    return g_thruput_type[type];
}

static WIFI_TCM_TEXT void hmac_thruput_dfx_process(osal_u8 type, osal_u8 value)
{
#if defined(CONFIG_CACHE_MISS_SUPPORT) || defined(_PRE_WLAN_PEAK_PERFORMANCE_DFX)
    if (type == THRUPUT_CACHE_MISS_STATISTIC) {
        if (value == 0) {
            hmac_thruput_cache_stat_start();
            return;
        } else {
            hmac_thruput_cache_stat_end();
            return;
        }
    } else if (type == THRUPUT_TASK_CPU) {
        OsDbgTskInfoGet(OS_ALL_TASK_MASK); /* 打印所有任务的CPU利用率 */
        return;
    }
#endif
#ifdef _PRE_WLAN_PEAK_PERFORMANCE_DFX
    unref_param(value);
    if (type == THRUPUT_HCC_STAT) {
        hcc_bus_dfx_statics_print(HCC_BUS_IPC);
        return;
    }
#else
#if defined(_PRE_WIFI_DEBUG) && defined(_PRE_WLAN_SUPPORT_CCPRIV_CMD)
    if (type == THRUPUT_TEST_QUERY_RATE_STAT) {
        hmac_query_alg_tx_rate_stats(value);
        hmac_query_alg_rx_rate_stats(value);
        return;
    }
#endif
#endif
}

WIFI_TCM_TEXT void hmac_set_thruput_test(osal_u8 type, osal_u8 value)
{
    oam_warning_log2(0, 0, "oal_set_thruput_bypass_enable::type[%d] value[%d].", type, value);
    if (type >= THRUPUT_TYPE_BUTT) {
        return;
    }
    g_thruput_type[type] = value;
    g_thruput_value = 0;
    g_thruput_time = (osal_u32)osal_get_time_stamp_ms();
    hmac_thruput_dfx_process(type, value);
}

/* ****************************************************************************
  函数功能: 更新流量值并每隔1s打印
**************************************************************************** */
WIFI_TCM_TEXT void hmac_update_thruput(osal_u32 thruput)
{
    osal_u32 cur_time = (osal_u32)osal_get_time_stamp_ms();

    g_thruput_value += thruput;
    /* 1000ms 每隔1s打印,调试用忽略翻转 */
    if (cur_time < g_thruput_time) {
        g_thruput_time = cur_time;
        return;
    }

    if ((cur_time - g_thruput_time) >= THRUPUT_TEST_1S) {
#ifdef _PRE_WIFI_PRINTK
        osal_u32 bits_value = g_thruput_value;
        osal_u32 kbits = (osal_u32)((bits_value >> 7) % 1000); /* 7: 右移10位转换为KB,再左移3位转换为Kb. 1000:取余得到KB */
        osal_u32 mbits = (osal_u32)(bits_value >> 17);         /* 17: 右移20位转换为MB,再左移3位转换为Mb */
        wifi_printf("\t%d.%d Mbits/sec \r\n", mbits, kbits);
#endif
        g_thruput_time = cur_time;
        g_thruput_value = 0;
    }
}

#if defined(CONFIG_CACHE_MISS_SUPPORT) || defined(_PRE_WLAN_PEAK_PERFORMANCE_DFX)
osal_u64 g_icache_miss_cnt = 0;
osal_u64 g_icache_access = 0;
osal_u64 g_dcache_miss_cnt = 0;
osal_u64 g_dcache_access = 0;
#define HMAC_CACHE_MISS_STAT_TIME 5000

/* ****************************************************************************
  函数功能: 192->ic_acc 193->ic_miss 194->dc_acc 195->dc_miss
**************************************************************************** */
static WIFI_TCM_TEXT uint32_t hmac_thruput_set_event(osal_void)
{
    WRITE_CUSTOM_CSR_VAL(0x323, 193); /* 0x323为counter3, 193对应ic_miss */
    WRITE_CUSTOM_CSR_VAL(0x325, 192); /* 0x325为counter5, 192对应ic_acc */

    WRITE_CUSTOM_CSR_VAL(0x324, 195); /* 0x324为counter4, 195对应dc_miss */
    WRITE_CUSTOM_CSR_VAL(0x326, 194); /* 0x326为counter6, 194对应dc_acc */
    return 0;
}

/* ****************************************************************************
  函数功能: 0xc83 0xc03  performance-monitoring counter 3
**************************************************************************** */
static WIFI_TCM_TEXT uint64_t hmac_thruput_get_ic_miss(osal_void)
{
    uint64_t count = 0;
    count = READ_CUSTOM_CSR(0xc83); /* 0xc83对应counter3高位 */
    count <<= 32; /* 左移32个bit */
    count |= READ_CUSTOM_CSR(0xc03); /* 0xc03对应counter3低位 */
    return count;
}

/* ****************************************************************************
  函数功能: 0xc85 0xc05  performance-monitoring counter 5
**************************************************************************** */
static WIFI_TCM_TEXT uint64_t hmac_thruput_get_ic_acc(osal_void)
{
    uint64_t count = 0;
    count = READ_CUSTOM_CSR(0xc85); /* 0xc85对应counter5高位 */
    count <<= 32; /* 左移32个bit */
    count |= READ_CUSTOM_CSR(0xc05); /* 0xc05对应counter5低位 */
    return count;
}

/* ****************************************************************************
  函数功能: 0xc84 0xc04  performance-monitoring counter 4
**************************************************************************** */
static WIFI_TCM_TEXT uint64_t hmac_thruput_get_dc_miss(osal_void)
{
    uint64_t count = 0;
    count = READ_CUSTOM_CSR(0xc84); /* 0xc84对应counter4高位 */
    count <<= 32; /* 左移32个bit */
    count |= READ_CUSTOM_CSR(0xc04); /* 0xc04对应counter4低位 */
    return count;
}

/* ****************************************************************************
  函数功能: 0xc86 0xc06  performance-monitoring counter 6
**************************************************************************** */
static WIFI_TCM_TEXT uint64_t hmac_thruput_get_dc_acc(osal_void)
{
    uint64_t count = 0;
    count = READ_CUSTOM_CSR(0xc86); /* 0xc86对应counter6高位 */
    count <<= 32; /* 左移32个bit */
    count |= READ_CUSTOM_CSR(0xc06); /* 0xc06对应counter6低位 */
    return count;
}

static WIFI_TCM_TEXT osal_u32 hmac_thruput_cache_stat_end_time(osal_void *param)
{
    unref_param(param);
    hmac_thruput_cache_stat_end();
    return OSAL_SUCCESS;
}


/* ****************************************************************************
  函数功能: 启动cache miss统计
**************************************************************************** */
WIFI_TCM_TEXT osal_void hmac_thruput_cache_stat_start(osal_void)
{
    hmac_thruput_set_event();
    g_icache_miss_cnt = hmac_thruput_get_ic_miss();
    g_icache_access = hmac_thruput_get_ic_acc();
    g_dcache_miss_cnt = hmac_thruput_get_dc_miss();
    g_dcache_access = hmac_thruput_get_dc_acc();
    static frw_timeout_stru cache_miss_timer = {0};

    /* 5s 后自动打印cache miss数据 */
    frw_create_timer_entry(&cache_miss_timer,
        hmac_thruput_cache_stat_end_time,
        HMAC_CACHE_MISS_STAT_TIME,
        OSAL_NULL,
        OAL_FALSE);
    return;
}

/* ****************************************************************************
  函数功能: 结束cache miss统计并打印结果
**************************************************************************** */
WIFI_TCM_TEXT osal_void hmac_thruput_cache_stat_end(osal_void)
{
    uint64_t imiss, iacc, dmiss, dacc;
    uint64_t imiss_value, iacc_value, dmiss_value, dacc_value;
    uint32_t imiss_value_short, iacc_value_short, dmiss_value_short, dacc_value_short;
    imiss = hmac_thruput_get_ic_miss();
    iacc = hmac_thruput_get_ic_acc();
    dmiss = hmac_thruput_get_dc_miss();
    dacc = hmac_thruput_get_dc_acc();
    imiss_value = imiss - g_icache_miss_cnt;
    iacc_value = iacc - g_icache_access;
    dmiss_value = dmiss - g_dcache_miss_cnt;
    dacc_value = dacc - g_dcache_access;
    imiss_value_short = (uint32_t)(imiss_value >> 10); /* 右移10位 */
    iacc_value_short = (uint32_t)(iacc_value >> 10); /* 右移10位 */
    dmiss_value_short = (uint32_t)(dmiss_value >> 10); /* 右移10位 */
    dacc_value_short = (uint32_t)(dacc_value >> 10); /* 右移10位 */
    wifi_printf("---------------\r\n");
    wifi_printf("Icache miss count:(%lld)\r\n", imiss_value);
    wifi_printf("Icache acc count:(%lld)\r\n", iacc_value);
    wifi_printf("Dcache miss count:(%lld)\r\n", dmiss_value);
    wifi_printf("Dcache acc count:(%lld)\r\n", dacc_value);
    wifi_printf("Icache miss:%u\r\n", imiss_value_short * 1000 / iacc_value_short); /* 放大1000倍 */
    wifi_printf("Dcache miss:%u\r\n", dmiss_value_short * 1000 / dacc_value_short); /* 放大1000倍 */
    wifi_printf("---------------\r\n");
    return;
}
#endif

/* ****************************************************************************
  函数功能: hmac流程时延统计函数
**************************************************************************** */
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_delay(osal_u32 pos)
{
#ifdef _PRE_WLAN_PEAK_PERFORMANCE_DFX
    static osal_u64 ts_start;
    osal_u64 ts_end = 0;
    osal_u32 delay_start = hmac_is_thruput_enable(THRUPUT_DELAY_START_NUM);
    osal_u32 delay_end = hmac_is_thruput_enable(THRUPUT_DELAY_END_NUM);
    /* delay = delay[pos+1] - delay[pos] */
    if (pos != delay_start && pos != delay_end) {
        return;
    }

    if (pos == delay_start) {
        ts_start = uapi_tcxo_get_us();
    }

    if (pos == delay_end) {
        ts_end = uapi_tcxo_get_us();
        wifi_printf("ts[%u:%u]:%llu\r\n", delay_start, delay_end, ts_end - ts_start);
    }
    return;
#else
    unref_param(pos);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
