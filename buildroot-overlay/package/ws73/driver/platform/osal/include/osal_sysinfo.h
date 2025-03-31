/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 */

#ifndef _OSAL_SYSINFO_H
#define _OSAL_SYSINFO_H

#include "osal_types.h"
#include "osal_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/***************************************cpup*******************************************/
typedef osal_void (*osal_cpup_period_user_hook)(osal_void);

typedef struct {
    osal_u16 id;
    osal_u16 status;
    osal_u32 usage;
} osal_cpup_item_usage_info;

typedef enum {
    OSAL_CPUP_LAST_USER_TIME = 0,
    OSAL_CPUP_ALL_TIME = 0xffff
} osal_cpup_static_mode;

osal_void osal_cpup_register_user_hook(osal_cpup_period_user_hook hook);
osal_u32 osal_cpup_get_all_usage(osal_u16 max_num, osal_cpup_item_usage_info *cpup_info, osal_u32 mode, osal_u16 flag);
osal_void osal_cpup_reset(osal_void);
osal_u32 osal_cpup_get_history_cpu_usage(osal_u32 mode);
/***************************************os_stat*******************************************/
/**
 * @ingroup os_stat
 * System resource usage statistic.
 */
typedef struct {
    osal_u8 timer_usage;  /* Number of used system timers. */
    osal_u8 task_usage;   /* Number of used tasks. */
    osal_u8 sem_usage;    /* Number of used semaphores. */
    osal_u8 queue_usage;  /* Number of used message queues. */
    osal_u8 mux_usage;    /* Number of used mutexes. */
} osal_os_resource_use_stat;

osal_errno osal_os_get_resource_status(osal_os_resource_use_stat *os_resource_stat);
/***************************************task*******************************************/
#define OSAL_TASK_NAME_LEN     32
#define MILLISECOND_PER_TICK   10

typedef struct {
    osal_char name[OSAL_TASK_NAME_LEN]; /* Task entrance function. */
    osal_u32 id;                        /* Task ID. */
    osal_u16 status;                    /* Task status. Status detail see los_task_pri.h. */
    osal_u16 priority;                  /* Task priority. */
    osal_pvoid task_sem;                /* Semaphore pointer. */
    osal_pvoid task_mutex;              /* Mutex pointer. */
    osal_u32 event_mask;                /* Event mask. */
    osal_u32 stack_size;                /* Task stack size. */
    osal_u32 top_of_stack;              /* Task stack top. */
    osal_u32 bottom_of_stack;           /* Task stack bottom. */
    osal_u32 sp;                        /* Task SP pointer. */
    osal_u32 curr_used;                 /* Current task stack usage. */
    osal_u32 peak_used;                 /* Task stack usage peak. */
    osal_u32 overflow_flag;             /* Flag that indicates whether a task stack overflow occurs. */
} osal_task_info;

osal_u32 osal_task_usage(osal_u32 *maxtasknum);
osal_errno osal_os_get_task_info(osal_u32 taskid, osal_task_info *info);
/***************************************mem_info*******************************************/
/**
 * @ingroup iot_mem
 * Overall memory information.CNcomment:整体内存信息。CNend
 */
typedef struct {
    osal_u32 total;                /* Total space of the memory pool (unit: byte). */
    osal_u32 used;                 /* Used space of the memory pool (unit: byte). */
    osal_u32 free;                 /* Free space of the memory pool (unit: byte). */
    osal_u32 free_node_num;        /* Number of free nodes in the memory pool. */
    osal_u32 used_node_num;        /* Number of used nodes in the memory pool. */
    osal_u32 max_free_node_size;   /* Maximum size of the node in the free space of the memory pool (unit: byte). */
    osal_u32 malloc_fail_count;    /* Number of memory application failures. */
    osal_u32 peek_size;            /* Peak memory usage of the memory pool. */
} osal_mdm_mem_info;

osal_errno osal_os_get_mem_sys_info(osal_mdm_mem_info* mem_inf);

#ifdef __cplusplus
#if __cplusplus
}
#endif // __cplusplus end
#endif // __cplusplus end

#endif // _OSAL_LIST_H end

