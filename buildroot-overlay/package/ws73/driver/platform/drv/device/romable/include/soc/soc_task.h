/*
 * Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.
 * Description: Task APIs.
 * Author: CompanyName
 * Create: 2019-10-25
 */

/**
* @file soc_task.h
*
* Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.  \n
* Description: Task APIs.   \n
* Author: CompanyName   \n
* Create: 2019-12-18
*/

/**
 * @defgroup iot_task Task
 * @ingroup osa
 */
#ifndef __SOC_TASK_H__
#define __SOC_TASK_H__

#include <td_base.h>

#define EXT_INVALID_TASK_ID   0xFFFFFFFF
#define EXT_TASK_NAME_LEN     32
#define EXT_DEFAULT_TSKNAME   "default"  /* ext_task_attr default value. CNcomment:ext_task_attr的默认值CNend */
#define EXT_DEFAULT_TSKPRIO   20         /* ext_task_attr default value. CNcomment:ext_task_attr的默认值CNend */
#define EXT_DEFAULT_STACKSIZE (4 * 1024) /* ext_task_attr default value. CNcomment:ext_task_attr的默认值CNend */
#define NOT_BIND_CPU         (-1)

typedef struct {
    td_char name[EXT_TASK_NAME_LEN]; /* Task entrance function.CNcomment:入口函数CNend */
    td_u32 id;                      /* Task ID.CNcomment:任务ID CNend */
    td_u16 status;                  /* Task status. Status detail see los_task_pri.h.CNcomment:任务状态。
                                         详细状态码请参考los_task_pri.h CNend */
    td_u16 priority;                /* Task priority.CNcomment:任务优先级 CNend */
    td_pvoid task_sem;              /* Semaphore pointer.CNcomment:信号量指针CNend */
    td_pvoid task_mutex;            /* Mutex pointer.CNcomment:互斥锁指针CNend */
    td_u32 event_stru[3];           /* Event: 3 nums.CNcomment:3个事件CNend */
    td_u32 event_mask;              /* Event mask.CNcomment:事件掩码CNend */
    td_u32 stack_size;              /* Task stack size.CNcomment:栈大小CNend */
    td_u32 top_of_stack;            /* Task stack top.CNcomment:栈顶CNend */
    td_u32 bottom_of_stack;         /* Task stack bottom.CNcomment:栈底CNend */
    td_u32 sp;                      /* Task SP pointer.CNcomment:当前SP.CNend */
    td_u32 curr_used;               /* Current task stack usage.CNcomment:当前任务栈使用率CNend */
    td_u32 peak_used;               /* Task stack usage peak.CNcomment:栈使用峰值CNend */
    td_u32 overflow_flag;           /* Flag that indicates whether a task stack overflow occurs.
                                       CNcomment:栈溢出标记位CNend */
} ext_task_info;

typedef struct {
    td_u16 task_prio;  /* Task priority.CNcomment:任务优先级 CNend */
    td_u32 stack_size; /* Task stack size.CNcomment:栈大小 CNend */
    td_u32 task_policy; /* Task schedule policy. Not used by default.CNcomment:任务调度策略。默认没有使用。CNend */
    td_u32 task_nice;   /* Task nice value. Not used by default.CNcomment:任务优先权。默认没有使用。CNend */
    td_u32 task_cpuid;  /* CPUID to which the task belongs. Not used by default.
                             CNcomment:任务所属的CPUID。默认没有使用。CNend */
    td_char *task_name; /* Task name.CNcomment:任务名 CNend */
    td_void *resved; /* Reserved. Not used by default.CNcomment:预留字段。默认没有使用。CNend */
} ext_task_attr;

/**
* @ingroup  iot_task
* @brief  Creates a task.CNcomment:创建任务。CNend
*
* @par 描述:
*           Creates a task.CNcomment:创建任务。CNend
*
* @attention
*           @li The space occupied by a task name string must be applied for by the caller and saved statically.
*               The task name is not stored internally in the API.CNcomment:任务名字符串占用空间需要调用者
                申请并静态保存，接口内部不对任务名进行存储。CNend
*           @li If the size of the specified task stack is 0, use the default size specified by
*               #OS_TSK_DEFAULT_STACK_SIZE. CNcomment:若指定的任务栈大小为0，则使用配置项
                EXT_DEFAULT_STACKSIZE指定默认的任务栈大小。CNend
*           @li The size of the task stack should be 16-byte aligned. The principle for determining the task stack
*               size is as follows: Do not use a too large or too small task stack size (to avoid waste or
*               overflow).CNcomment:任务栈的大小按16byte大小对齐。确定任务栈大小的原则：够用即可（多则浪费，
                少则任务栈溢出）。CNend
*           @li The recommended user priority should be within the range of [10, 30]. Do not use the priorities of
*               [0, 5] and [31].CNcomment:用户优先级配置建议使用[10,30]，切记不可使用[0,5]和[31]号的优先级。CNend
*
* @param  taskid         [OUT] type #td_u32*，task ID.CNcomment:任务ID号。CNend
* @param  attr           [IN]  type #const task_attr_t*，task attributes,when NULL was set here,the properties
                               are configured as follows: task_name:"default" task_prio:20  stack_size:(4*1024).
                               The stack size must be at least 2 KB.
                               CNcomment:任务属性,当该值为空时配置如下:任务名:"default"
                               任务优先级:20 任务栈大小:(4*1024)。栈大小最小为2KB。CNend
* @param  task_route     [IN]  type #task_route task entry function.CNcomment:任务入口函数。CNend
* @param  arg            [IN]  type #td_void*，parameter that needs to be input to the task entry when a task is
*                              created. If this parameter does not need to be input, set this parameter to 0.
                               CNcomment:创建任务时需要传给任务入口的参数。如果不需要传递，参数直接填0。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  uapi_task_delete。
*/
ext_errno uapi_task_create(td_u32 *taskid, const ext_task_attr *attr,
    td_void *(*task_route)(td_void *), td_void *arg);

/**
* @ingroup  iot_task
* @brief  Deletes a task.CNcomment:删除任务。CNend
*
* @par 描述:
*          Deletes a task.CNcomment:删除任务。CNend
*
* @attention
*           @li Use this API with caution. A task can be deleted only after the confirmation of the user. The idle task
*           and Swt_Task cannot be deleted.idle.CNcomment:任务及Swt_Task任务不能被删除。CNend
*           @li When deleting a task, ensure that the resources (such as mutex and semaphore) applied by the task have
*           been released.在删除任务时要保证任务申请的资源（如互斥锁、信号量等）已被释放。CNend
*
* @param  taskid      [IN] type #td_u32，task ID. CNcomment:任务ID号。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  uapi_task_create。
*/
ext_errno uapi_task_delete(td_u32 taskid);

/**
* @ingroup  iot_task
* @brief  Suspends a task.CNcomment:挂起任务。CNend
*
* @par 描述:
*           Suspends a task.CNcomment:挂起指定任务。CNend
*
* @attention
*          @li A task cannot be suspended if it is the current task and is locked.
CNcomment:挂起任务的时候若为当前任务且已锁任务，则不能被挂起。CNend
*          @li The idle task and Swt_Task cannot be suspended.
CNcomment:idle任务及Swt_Task任务不能被挂起。CNend
*          @li The task cannot be blocked or suspended in the lock task status.
CNcomment:在锁任务调度状态下，禁止任务阻塞。CNend
*
* @param  taskid      [IN] type #td_u32，task ID. CNcomment:任务ID号。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  uapi_task_resume。
*/
ext_errno uapi_task_suspend(td_u32 taskid);

/**
* @ingroup  iot_task
* @brief  Resumes a task.CNcomment:恢复挂起任务。CNend
*
* @par 描述:
*           Resumes a task.CNcomment:恢复挂起指定任务。CNend
*
* @attention None
* @param  taskid      [IN] type #td_u32，task ID. CNcomment:任务ID号。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  ext_task_suspend。
*/
ext_errno uapi_task_resume(td_u32 taskid);

/**
* @ingroup  iot_task
* @brief  Obtains the task priority.CNcomment:获取任务优先级。CNend
*
* @par 描述:
*           Obtains the task priority.CNcomment:获取任务优先级。CNend
*
* @attention None
*
* @param  taskid      [IN] type #td_u32，task ID. CNcomment:任务ID号。CNend
* @param  priority   [OUT] type #td_u32*，task priority.CNcomment:任务优先级。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  uapi_task_set_priority。
*/
ext_errno uapi_task_get_priority(td_u32 taskid, td_u32 *priority);

/**
* @ingroup  iot_task
* @brief  Sets the task priority.CNcomment:设置任务优先级。CNend
*
* @par 描述:
            Sets the task priority.CNcomment:设置任务优先级。CNend
*
* @attention
*           @li Only the ID of the task created by the user can be configured.
CNcomment:仅可配置用户自己创建的任务ID。CNend
*           @li The recommended user priority should be within the range of [10, 30]. Do not use the priorities of
*            [0, 5] and [31].CNcomment:用户优先级配置建议使用[10,30]，切记不可使用[0,5]和[31]号的优先级。CNend
*           @li Setting user priorities may affect task scheduling. The user needs to plan tasks in the SDK.
CNcomment:设置用户优先级有可能影响任务调度，用户需要SDK中对各任务统一规划。CNend
*
* @param  taskid      [IN] type #td_u32，task ID. CNcomment:任务ID号。CNend
* @param  priority    [IN] type #td_u32，task priority.CNcomment:任务优先级。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  uapi_task_get_priority。
*/
ext_errno uapi_task_set_priority(td_u32 taskid, td_u32 priority);

/**
* @ingroup  iot_task
* @brief  Obtains the current task ID.CNcomment:获取当前任务ID。CNend
*
* @par 描述:
*         Obtains the current task ID.CNcomment:获取当前任务ID。CNend
*
* @attention None
* @param  None
*
* @retval #td_u32  Task ID. If the task fails, #EXT_INVALID_TASK_ID is returned.
CNcomment:任务ID，失败返回#EXT_INVALID_TASK_ID。CNend
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  None
*/
td_u32 uapi_task_get_current_id(td_void);

/**
* @ingroup  iot_task
* @brief  Lock task switch.CNcomment:禁止系统任务调度。CNend
*
* @par 描述:
*         Lock task switch.CNcomment:禁止系统任务调度。CNend
*
* @attention  Work pair with ext_task_unlock.CNcomment:与ext_task_unlock配对使用。CNend
* @param  None
*
* @retval None
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see None
*/
td_void uapi_task_lock(td_void);

/**
* @ingroup  iot_task
* @brief  Unlock task switch. CNcomment:允许系统任务调度。CNend
*
* @par 描述:
*         Unlock task switch. CNcomment:允许系统任务调度。CNend
*
* @attention  Work pair with uapi_task_lock; Call uapi_task_lock to disable task switch, then call ext_task_unlock
*             reenable it.
CNcomment:与uapi_task_lock配对使用；先调用uapi_task_lock禁止任务调度，然后调用ext_task_unlock打开任务调度。CNend
* @param  None
*
* @retval  None
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  None
*/
td_void uapi_task_unlock(td_void);

/**
* @ingroup  iot_task
* @brief  Check whether task switch is locked. CNcomment:检查是否允许任务调度。CNend
*
* @par 描述:
*         Check whether task switch is locked. CNcomment:检查是否允许任务调度。CNend
*
* @attention  None
* @param  None
*
* @retval #EXT_FALSE    task switch unlock.CNcomment:允许任务调度。CNend
* @retval #EXT_TRUE     task switch lock.CNcomment:不允许任务调度。CNend
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  None
*/
td_bool uapi_task_is_lock(td_void);

/**
* @ingroup  iot_task
* @brief Task sleep.CNcomment:任务睡眠。CNend
*
* @par 描述:
*          Task sleep.CNcomment:任务睡眠。CNend
*
* @attention
*           @li In the interrupt processing function or in the case of a lock task, the td_sleep operation fails.
CNcomment:在中断处理函数中或者在锁任务的情况下，执行td_sleep操作会失败。CNend
*           @li When less than 10 ms, the input parameter value should be replaced by 10 ms. When greater than 10 ms,
*            the input parameter value should be exactly divided and then rounded-down to the nearest integer.
CNcomment:入参小于10ms时，当做10ms处理，Tick=1；大于10ms时，整除向下对齐，Tick = ms/10。CNend
*           @li This function cannot be used for precise timing and will be woken up after Tick system scheduling.
*            The actual sleep time is related to the time consumed by the Tick when the function is called.
CNcomment:本函数不能用于精确计时，将在Tick个系统调度后唤醒，实际睡眠时间与函数被调用时该Tick已消耗的时间相关。CNend
* @param  ms      [IN] type #td_u32，sleep time (unit: ms). The precision is 10 ms.
CNcomment:睡眠时间（单位：ms），精度为10ms。CNend
*
* @retval #EXT_ERR_SUCCESS Success
* @retval #Other Failure. For details, see soc_errno.h.
* @par 依赖:
*           @li soc_task.h：Describes the task APIs.CNcomment:文件用于描述任务相关接口。CNend
* @see  None
*/
ext_errno uapi_sleep(td_u32);

#endif

