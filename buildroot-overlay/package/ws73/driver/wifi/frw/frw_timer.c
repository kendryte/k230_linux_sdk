/*
 * Copyright (c) @CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Timer process implementation of the FRW module.
 * Author: Huanghe
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "frw_timer.h"
#include "frw_hmac.h"
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_TIMER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define OAM_MODULE_ID_HMAC 2

/*****************************************************************************
    全局变量定义
*****************************************************************************/
struct osal_list_head g_timer_list_head[WLAN_FRW_MAX_NUM_CORES];
osal_spinlock g_timer_list_lock[WLAN_FRW_MAX_NUM_CORES];
osal_timer g_os_timer[WLAN_FRW_MAX_NUM_CORES];
osal_u32 g_next_start_time_stamp[WLAN_FRW_MAX_NUM_CORES] = {0}; /* 记录下一次软中断定时器启动时间 */
osal_u32 g_os_timer_tick_cnt[WLAN_FRW_MAX_NUM_CORES] = {0};     /* 记录硬timer到期次数，统计timer消息频率 */
osal_u32 g_timer_stoped = OSAL_FALSE;

/*****************************************************************************
    函数实现
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_s32 frw_time_after(osal_u32 a, osal_u32 b)
{
    /* return true if the time a is after time b,in case of overflow and wrap around to zero */
    return ((osal_s32)((b) - (a)) < 0);
}

OAL_STATIC OAL_INLINE osal_bool frw_timer_memory_valid(const frw_timeout_stru *timeout)
{
    /* check whether the timeout entry has already been released or memset. return 1 if memory is valid */
    return (timeout->func_p == FRW_TIMER_ENTRY_MAGIC_NUM && timeout->entry.prev != OSAL_NULL &&
        timeout->entry.next != OSAL_NULL);
}

OAL_STATIC OAL_INLINE osal_void frw_timer_entry_check(frw_timeout_stru *timeout)
{
    if (!frw_timer_memory_valid(timeout)) {
        oam_error_log1(0, OAM_SF_FRW, "{frw_timer_entry_check:: timer 0x%x released or memset}", (uintptr_t)timeout);
        frw_debug("{frw_timer_entry_check:: timer 0x%x released or memset}\r\n", (osal_u32)(uintptr_t)timeout);
    }
}

/*****************************************************************************
 功能描述 : 初始化定时器
*****************************************************************************/
osal_void frw_timer_init(osal_u32 delay, oal_timer_func func, osal_ulong arg)
{
    osal_u32 core_id;
    osal_s32 ret;
    unref_param(arg);
    frw_debug("frw_timer_init enter\r\n");
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        OSAL_INIT_LIST_HEAD(&g_timer_list_head[core_id]);
        osal_spin_lock_init(&g_timer_list_lock[core_id]);

        ret = osal_adapt_timer_init(&g_os_timer[core_id], func, 0, delay);
        if (ret != OSAL_SUCCESS) {
            oam_error_log2(0, OAM_SF_FRW, "{frw_timer_init:: core[%u], osal_timer_init fail 0x%x! }", core_id, ret);
            (osal_void)osal_adapt_timer_destroy(&g_os_timer[core_id]);
            g_os_timer[core_id].timer = OSAL_NULL;
        }
        g_next_start_time_stamp[core_id] = 0;
    }
    frw_msg_hook_register(WLAN_MSG_H2H_FRW_TIMEOUT, (frw_msg_callback)frw_timer_timeout_proc);
    g_timer_stoped = OSAL_FALSE;
}

/*****************************************************************************
 功能描述 : 定时器退出函数
*****************************************************************************/
osal_void frw_timer_exit(osal_void)
{
    osal_u32 core_id;

    g_timer_stoped = OSAL_TRUE;
    frw_debug("frw_timer_exit enter\r\n");
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        if (osal_list_empty(&g_timer_list_head[core_id]) == OSAL_FALSE) {
            frw_debug("there are timers not destory, core[%u]!!\r\n", core_id);
            frw_timer_dump(core_id);
        }
        OSAL_INIT_LIST_HEAD(&g_timer_list_head[core_id]);
        (osal_void)osal_adapt_timer_destroy(&g_os_timer[core_id]);
        g_os_timer[core_id].timer = OSAL_NULL;
        g_next_start_time_stamp[core_id] = 0;
        osal_spin_lock_destroy(&g_timer_list_lock[core_id]);
    }
}

/*****************************************************************************
 功能描述 : 将所有定时器dump出来
*****************************************************************************/
osal_void frw_timer_dump(osal_u32 core_id)
{
    struct osal_list_head *timeout_entry;
    frw_timeout_stru *timeout_element;

    timeout_entry = g_timer_list_head[core_id].next;
    while (timeout_entry != &g_timer_list_head[core_id]) {
        if (timeout_entry == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_FRW, "{frw_timer_dump:: core[%u], time broken break}", core_id);
            break;
        }
        timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
        frw_timer_entry_check(timeout_element);
        timeout_entry = timeout_entry->next;
        oam_warning_log4(0, OAM_SF_FRW, "{frw_timer_dump:: node_fp[0x%x] timeout[%d] enabled[%d] func[0x%x]}",
            (uintptr_t)timeout_element, timeout_element->timeout,
            timeout_element->is_enabled, (uintptr_t)timeout_element->func);
    }
    frw_debug("{frw_timer_dump:: timer tick count[%d]\r\n", g_os_timer_tick_cnt[core_id]);
}

OAL_STATIC osal_void frw_timer_timeout_proc_restart_timer(osal_u32 *present_time, osal_u32 core_id)
{
    struct osal_list_head *timeout_entry = OSAL_NULL;
    frw_timeout_stru *timeout_element = OSAL_NULL;
    osal_u32 frw_timer_start;
    osal_s32 ret;

    if (osal_list_empty(&g_timer_list_head[core_id]) == OAL_FALSE) {
        timeout_entry = g_timer_list_head[core_id].next;
        timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
        frw_timer_entry_check(timeout_element);
        *present_time = (osal_u32)oal_time_get_stamp_ms();
        if (frw_time_after(timeout_element->time_stamp, *present_time) != 0) {
            frw_timer_start = (osal_u32)osal_get_runtime(*present_time, timeout_element->time_stamp);
            g_next_start_time_stamp[core_id] = timeout_element->time_stamp;
        } else {
            frw_timer_start = FRW_TIMER_DEFAULT_TIME;
            g_next_start_time_stamp[core_id] = (*present_time + FRW_TIMER_DEFAULT_TIME);
        }
        ret = osal_adapt_timer_mod(&g_os_timer[core_id], frw_timer_start);
        if (ret != OSAL_SUCCESS) {
            oam_error_log1(0, OAM_SF_FRW, "{frw_timer_timeout_proc_restart_timer:: osal_timer_start fail 0x%x!}", ret);
            (osal_void)osal_adapt_timer_destroy(&g_os_timer[core_id]);
            g_os_timer[core_id].timer = OSAL_NULL;
        }
    } else {
        g_next_start_time_stamp[core_id] = 0;
    }
}

OAL_STATIC osal_void frw_timer_timeout_proc_run(osal_u32 *present_time, osal_u32 core_id)
{
    struct osal_list_head *timeout_entry = OSAL_NULL;
    frw_timeout_stru *timeout_element = OSAL_NULL;
    osal_u32 runtime_func_start;
    osal_u32 runtime_func_end;
    osal_u32 endtime_func;

    osal_spin_lock_bh(&g_timer_list_lock[core_id]);
    timeout_entry = g_timer_list_head[core_id].next;
    while (timeout_entry != &g_timer_list_head[core_id]) {
        if (timeout_entry == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_FRW, "{frw_timer_timeout_proc_run:: the timer list is broken!}");
            break;
        }
        timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
        frw_timer_entry_check(timeout_element);
        /* 一个定时器超时处理函数中创建新的定时器，如果定时器超时，则将相应的定时器进行删除，取消en_is_deleting标记 */
        if (frw_time_after(*present_time, timeout_element->time_stamp) != 0) {
            /* 删除超时定时器，如果是周期定时器，则将其再添加进去:delete first,then add periodic_timer */
            timeout_element->is_registerd = OAL_FALSE;
            osal_list_del(&timeout_element->entry);

            if ((timeout_element->is_periodic == OAL_TRUE) || (timeout_element->is_enabled == OAL_FALSE)) {
                timeout_element->time_stamp = *present_time + timeout_element->timeout;
                timeout_element->is_registerd = OAL_TRUE;
                frw_timer_add_timer(timeout_element);
            }

            runtime_func_start = (osal_u32)oal_time_get_stamp_ms();
            if (timeout_element->is_enabled) {
                osal_spin_unlock_bh(&g_timer_list_lock[core_id]);
                timeout_element->func(timeout_element->timeout_arg);
                osal_spin_lock_bh(&g_timer_list_lock[core_id]);
            }

            endtime_func = (osal_u32)oal_time_get_stamp_ms();
            runtime_func_end = (osal_u32)osal_get_runtime(runtime_func_start, endtime_func);
            if ((osal_u32)oal_jiffies_to_msecs(2) <= runtime_func_end) {  /* 2转换时间 */
                oam_warning_log2(0, OAM_SF_FRW, "{frw_timer_timeout_proc_run::timer 0x%x, runtime=%u}",
                    (uintptr_t)timeout_element->func, runtime_func_end);
            }
        } else {
            break;
        }
        timeout_entry = g_timer_list_head[core_id].next;
    }
    /* 获得链表的最小超时时间，重启定时器 */
    frw_timer_timeout_proc_restart_timer(present_time, core_id);
    osal_spin_unlock_bh(&g_timer_list_lock[core_id]);
}

/*****************************************************************************
 功能描述 : 遍历timer链表执行到期超时函数
*****************************************************************************/
osal_s32 frw_timer_timeout_proc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 present_time;
    osal_u32 end_time;
    osal_u32 runtime;
    osal_u32 core_id;

    unref_param(hmac_vap);
    unref_param(msg);

    if (g_timer_stoped == OSAL_TRUE) {
        return OAL_SUCC;
    }

    present_time = (osal_u32)oal_time_get_stamp_ms();
    core_id = OAL_GET_CORE_ID();

    /* 执行超时定时器 */
    frw_timer_timeout_proc_run(&present_time, core_id);

    end_time = (osal_u32)oal_time_get_stamp_ms();
    runtime = (osal_u32)osal_get_runtime(present_time, end_time);
    /* 同device侧检测日志时限一致 */
    if (runtime > (osal_u32)oal_jiffies_to_msecs(2)) {    /* 2转换时间 */
        oam_warning_log1(0, OAM_SF_FRW, "{frw_timer_timeout_proc_etc:: timeout process exucte time too long time[%d]}",
            runtime);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 向链表中按从小到大的顺序插入节点
*****************************************************************************/
OAL_STATIC osal_void frw_timer_add_in_order(struct osal_list_head *new_node, struct osal_list_head *head_node)
{
    struct osal_list_head *timeout_entry;
    frw_timeout_stru *timeout_element;
    frw_timeout_stru *timeout_element_new = osal_list_entry(new_node, frw_timeout_stru, entry);

    /* 搜索链表，查找第一个比pst_timeout_element_new->time_stamp大的位置 */
    if (head_node != OSAL_NULL) {
        timeout_entry = head_node->next;
        while (timeout_entry != head_node) {
            if (timeout_entry == OSAL_NULL) {
                oam_error_log0(0, OAM_SF_FRW, "{frw_timer_add_in_order:: the timer list is broken!}");
                oam_error_log4(0, OAM_SF_FRW,
                    "{frw_timer_add_in_order:: new time_stamp[0x%x] timeout[%d] enabled[%d] func[0x%x]}",
                    timeout_element_new->time_stamp, timeout_element_new->timeout,
                    timeout_element_new->is_enabled, (uintptr_t)timeout_element_new->func);
                break;
            }
            timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
            frw_timer_entry_check(timeout_element);
            if (frw_time_after(timeout_element->time_stamp, timeout_element_new->time_stamp) != 0) {
                break;
            }
            timeout_entry = timeout_entry->next;
        }
        if ((timeout_entry != OSAL_NULL) && (timeout_entry->prev != OSAL_NULL)) {
            osal_list_add_tail(new_node, timeout_entry);
        } else {
            oam_error_log0(0, OAM_SF_FRW, "{frw_timer_add_in_order::timer list is broken!}");
        }
    }
}

/*****************************************************************************
 功能描述 : 删除定时器
*****************************************************************************/
WIFI_TCM_TEXT osal_void frw_timer_add_timer(frw_timeout_stru *timeout)
{
    osal_s32 val;
    osal_s32 ret;
    if (timeout == NULL) {
        return;
    }

    if (osal_list_empty(&g_timer_list_head[timeout->core_id]) == OAL_TRUE) {
        g_next_start_time_stamp[timeout->core_id] = 0;
    }

    /* 将Frw的无序链表改为有序 */
    frw_timer_add_in_order(&timeout->entry, &g_timer_list_head[timeout->core_id]);
    val = frw_time_after(g_next_start_time_stamp[timeout->core_id], timeout->time_stamp);
    if ((g_next_start_time_stamp[timeout->core_id] == 0) || (val > 0)) {
        ret = osal_adapt_timer_mod(&g_os_timer[timeout->core_id], timeout->timeout);
        if (ret != OSAL_SUCCESS) {
            oam_error_log1(0, OAM_SF_FRW, "{frw_timer_add_timer:: osal_timer_start fail 0x%x!}", ret);
            (osal_void)osal_adapt_timer_destroy(&g_os_timer[timeout->core_id]);
            g_os_timer[timeout->core_id].timer = OSAL_NULL;
        }
        g_next_start_time_stamp[timeout->core_id] = timeout->time_stamp;
    }
}

WIFI_TCM_TEXT osal_void frw_timer_create_timer_inner(osal_u32 file_id, osal_u32 line_num,
    frw_timeout_stru *timeout, osal_u32 time_val)
{
    unref_param(file_id);
    unref_param(line_num);

    timeout->timeout = time_val;
    timeout->time_stamp = (osal_u32)oal_time_get_stamp_ms() + time_val;
    timeout->is_enabled = OAL_TRUE; /* 默认使能 */

    if (timeout->is_registerd != OAL_TRUE) {
        timeout->is_registerd = OAL_TRUE; /* 默认注册 */
        frw_timer_add_timer(timeout);
    } else {
        osal_list_del(&timeout->entry);
        frw_timer_add_timer(timeout);
    }
}

/*****************************************************************************
 功能描述 : 立即删除定时器，无锁
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void frw_timer_destroy_timer_inner(osal_u32 file_id, osal_u32 line_num,
    frw_timeout_stru *timeout)
{
    unref_param(file_id);
    unref_param(line_num);

    if (timeout->is_registerd == OAL_FALSE) {
        oam_warning_log3(0, OAM_SF_FRW, "{frw_timer_destroy_timer::timer 0x%x is not registered! file:%d, line:%d}",
            (uintptr_t)timeout->func, file_id, line_num);
        return;
    }

    /* 只对已注册的timer, 做内存合法性检查 */
    if (!frw_timer_memory_valid(timeout)) {
        oam_error_log2(0, OAM_SF_FRW,
            "{frw_timer_destroy_timer::This timer has been released! file:%d, line:%d}", file_id, line_num);
        return;
    }

    timeout->is_enabled = OAL_FALSE;
    timeout->is_registerd = OAL_FALSE;
    timeout->func_p = 0;

    osal_list_del(&timeout->entry);

    if (osal_list_empty(&g_timer_list_head[timeout->core_id]) == OAL_TRUE) {
        g_next_start_time_stamp[timeout->core_id] = 0;
    }
}

OAL_STATIC OAL_INLINE osal_void frw_timer_restart_inner(frw_timeout_stru *timeout, osal_u32 timeout_val,
    oal_bool_enum_uint8 is_periodic, osal_u32 file_id, osal_u32 line)
{
    /* avoid periodic timer being temporarily unregistered in frw_timer_timeout_proc_run */
    if (timeout->is_registerd == OAL_FALSE) {
        oam_warning_log3(0, OAM_SF_FRW, "{frw_timer_restart_timer::timer 0x%x is not registered! file:%d, line:%d}",
            (uintptr_t)timeout->func, file_id, line);
        return;
    }

    if (!frw_timer_memory_valid(timeout)) {
        oam_error_log2(0, OAM_SF_FRW,
            "{frw_timer_restart_timer::This timer has been released! file_id:%d, line:%d}", file_id, line);
        return;
    }

    osal_list_del(&timeout->entry);

    timeout->time_stamp = (osal_u32)oal_time_get_stamp_ms() + timeout_val;
    timeout->timeout = timeout_val;
    timeout->is_periodic = is_periodic;
    timeout->is_enabled = OAL_TRUE;

    frw_timer_add_timer(timeout);
}

/*****************************************************************************
 功能描述 : 重启定时器
*****************************************************************************/
osal_void frw_timer_restart_timer_inner(frw_timeout_stru *timeout, osal_u32 timeout_val,
    oal_bool_enum_uint8 is_periodic, osal_u32 file_id, osal_u32 line)
{
    if (timeout == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_FRW,
            "{frw_timer_restart_timer:: timeout is NULL! file_id:%d, line:%d}", file_id, line);
        return;
    }

    osal_spin_lock_bh(&g_timer_list_lock[timeout->core_id]);
    frw_timer_restart_inner(timeout, timeout_val, is_periodic, file_id, line);
    osal_spin_unlock_bh(&g_timer_list_lock[timeout->core_id]);
}

/*****************************************************************************
 功能描述 : 停止定时器, 可被重启
*****************************************************************************/
osal_void frw_timer_stop_timer(frw_timeout_stru *timeout)
{
    if (timeout == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_stop_timer:: timeout is NULL}");
        return;
    }

    osal_spin_lock_bh(&g_timer_list_lock[timeout->core_id]);
    timeout->is_enabled = OAL_FALSE;
    osal_spin_unlock_bh(&g_timer_list_lock[timeout->core_id]);
}

/*****************************************************************************
 功能描述 : 15毫秒定时中断处理
*****************************************************************************/
#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
osal_u32 g_frw_timer_cpu_count[CONFIG_NR_CPUS] = {0};
#endif

#ifdef _PRE_WLAN_FEATURE_WS73
osal_void frw_timer_timeout_proc_event_etc(uintptr_t arg)
#else
osal_void frw_timer_timeout_proc_event_etc(unsigned long arg)
#endif
{
    frw_msg msg = {0};
    osal_u32 core_id;
    unref_param(arg);

    if (g_timer_stoped == OSAL_TRUE) {
        return;
    }

#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
    do {
        osal_u32 cpu_id = smp_processor_id();
        if (cpu_id < CONFIG_NR_CPUS) {
            g_frw_timer_cpu_count[cpu_id]++;
        }
    } while (0);
#endif
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        if (frw_task_get_state_etc(core_id)) {
#else
            core_id = 0;
#endif
            if (frw_asyn_host_post_msg(WLAN_MSG_H2H_FRW_TIMEOUT, FRW_POST_PRI_HIGH, 0, &msg) != OAL_SUCC) {
                osal_u32 present_time = (osal_u32)oal_time_get_stamp_ms();
                osal_spin_lock_bh(&g_timer_list_lock[core_id]);
                frw_timer_timeout_proc_restart_timer(&present_time, core_id);
                osal_spin_unlock_bh(&g_timer_list_lock[core_id]);
            } else {
                g_os_timer_tick_cnt[core_id]++;
            }
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
        }
    }
#endif
    unref_param(core_id);
}

/*****************************************************************************
 功能描述 : 将定时器链表中的所有定时器删除，用于异常时外部模块清理定时器
            不能在定时器回调函数中调用
*****************************************************************************/
osal_void frw_timer_delete_all_timer(osal_void)
{
    struct osal_list_head *timeout_entry;
    frw_timeout_stru *timeout_element;
    osal_u32 core_id;

    frw_debug("frw_timer_delete_all_timer enter\r\n");
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        osal_spin_lock_bh(&g_timer_list_lock[core_id]);
        timeout_entry = g_timer_list_head[core_id].next;
        while (timeout_entry != &g_timer_list_head[core_id]) {
            if (timeout_entry == OSAL_NULL) {
                oam_error_log0(0, OAM_SF_FRW, "{frw_timer_delete_all_timer:: the timer list is broken!}");
                break;
            }
            timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
            frw_timer_entry_check(timeout_element);
            timeout_entry = timeout_entry->next;
            timeout_element->is_registerd = OAL_FALSE;
            timeout_element->func_p = 0;
            osal_list_del(&timeout_element->entry);
        }
        g_next_start_time_stamp[core_id] = 0;
        osal_spin_unlock_bh(&g_timer_list_lock[core_id]);
    }
}

/*****************************************************************************
 功能描述 : 删除指定模块残留的所有定时器。本函数不能解决残留定时器的所有问题，
            一旦发现有残留，需要进行处理。
*****************************************************************************/
osal_void frw_timer_clean_timer(oam_module_id_enum_uint16 module_id)
{
    struct osal_list_head *timeout_entry;
    frw_timeout_stru *timeout_element;
    osal_u32 core_id;

    frw_debug("frw_timer_clean_timer enter\r\n");
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        osal_spin_lock_bh(&g_timer_list_lock[core_id]);
        timeout_entry = g_timer_list_head[core_id].next;
        while (timeout_entry != &g_timer_list_head[core_id]) {
            if (timeout_entry == OSAL_NULL) {
                oam_error_log0(0, OAM_SF_FRW, "{frw_timer_clean_timer:: the timer list is broken!}");
                frw_debug("{frw_timer_clean_timer:: the timer list is broken!}\r\n");
                break;
            }
            timeout_element = osal_list_entry(timeout_entry, frw_timeout_stru, entry);
            frw_timer_entry_check(timeout_element);
            timeout_entry = timeout_entry->next;
            if (module_id == timeout_element->module_id) {
                timeout_element->is_registerd = OAL_FALSE;
                timeout_element->func_p = 0;
                osal_list_del(&timeout_element->entry);
            }
        }
        if (osal_list_empty(&g_timer_list_head[core_id]) > 0) {
            g_next_start_time_stamp[core_id] = 0;
        }
        osal_spin_unlock_bh(&g_timer_list_lock[core_id]);
    }
}
/*****************************************************************************
 功能描述  : 创建定时器
*****************************************************************************/
WIFI_TCM_TEXT osal_void frw_timer_create_timer(osal_u32 file_id, osal_u32 line_num,
    frw_timeout_stru *timeout, osal_u32 time_val)
{
    if (timeout == OSAL_NULL || g_timer_stoped == OSAL_TRUE) {
        oam_error_log2(0, OAM_SF_FRW,
            "{frw_timer_create_timer:: timeout is NULL! file_id:%d, line:%d}", file_id, line_num);
        return;
    }

    if ((time_val == 0) && (timeout->is_periodic == OSAL_TRUE)) {
        oam_error_log3(0, OAM_SF_FRW, "{frw_timer_create_timer::timer timeout is 0! func:0x%x, file_id:%d, line:%d}",
            (uintptr_t)timeout->func, file_id, line_num);
        return;
    }

    timeout->module_id = OAM_MODULE_ID_HMAC;
    timeout->core_id = OAL_GET_CORE_ID();
    timeout->func_p = FRW_TIMER_ENTRY_MAGIC_NUM; /* use magic num to test if timeout memory being released */

    osal_spin_lock_bh(&g_timer_list_lock[timeout->core_id]);
    frw_timer_create_timer_inner(file_id, line_num, timeout, time_val);
    osal_spin_unlock_bh(&g_timer_list_lock[timeout->core_id]);
}
/*****************************************************************************
 功能描述  : 立即删除定时器
*****************************************************************************/
WIFI_TCM_TEXT osal_void frw_timer_destroy_timer(osal_u32 file_id, osal_u32 line_num, frw_timeout_stru *timeout)
{
    if (timeout == OSAL_NULL || g_timer_stoped == OSAL_TRUE) {
        oam_error_log2(0, OAM_SF_FRW,
            "{frw_timer_destroy_timer:: timeout is NULL! file_id:%d, line:%d}", file_id, line_num);
        return;
    }

    osal_spin_lock_bh(&g_timer_list_lock[timeout->core_id]);
    frw_timer_destroy_timer_inner(file_id, line_num, timeout);
    osal_spin_unlock_bh(&g_timer_list_lock[timeout->core_id]);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
