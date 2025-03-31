/**
* @file soc_isr.h
*
* Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.  \n
* Description: Interruption APIs.   \n
* Author: CompanyName   \n
* Create: 2019-05-29
*/

/**
 * @defgroup iot_isr Interruption
 * @ingroup osa
 */

#ifndef __SOC_ISR_H__
#define __SOC_ISR_H__
#include <td_base.h>

#define EXT_EXC_FLAG_NO_FLOAT                0x10000000
#define EXT_EXC_FLAG_FAULTADDR_VALID         0x01
#define EXT_EXC_FLAG_IN_HWI                  0x02

typedef struct {
    /* handler save */
    td_u32 r4;
    td_u32 r5;
    td_u32 r6;
    td_u32 r7;
    td_u32 r8;
    td_u32 r9;
    td_u32 r10;
    td_u32 r11;
    td_u32 pri_mask;
    /* auto save */
    td_u32 sp;
    td_u32 r0;
    td_u32 r1;
    td_u32 r2;
    td_u32 r3;
    td_u32 r12;
    td_u32 lr;
    td_u32 pc;
    td_u32 xpsr;
} ext_exc_context;

/**
* @ingroup  iot_isr
* @brief  HISR callback function type.CNcomment:HISR中断回调函数的类型。CNend
*
* @par 描述：
*           HISR callback function type.CNcomment:HISR中断回调函数的类型。CNend
*
* @attention None
* @param  param [IN] type #td_u32，Callback input parameter.CNcomment:回调入参。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see None
*/
typedef td_s32 (*irq_routine)(td_s32 irq, td_void *param);

/**
* @ingroup  iot_isr
* @brief  Interrupt off.CNcomment:关中断。CNend
*
* @par 描述:
*           Interrupt off.CNcomment:关中断。CNend
*
* @attention
*           @li A function that causes scheduling cannot be executed in an interrupt off context, for example,
*               td_sleep and other blocked APIs.
*               CNcomment:关中断后不能执行引起调度的函数，如td_sleep或其他阻塞接口。CNend
*           @li Interrupt off only protects short-time operations that can be expected. Otherwise, the interrupt
*               response and the performance may be affected.
*               CNcomment:关中断仅保护可预期的短时间的操作，否则影响中断响应，可能引起性能问题。CNend
*           @li After the protection operation is complete, the uapi_int_restore interface must be called to
*               restore the status before the interruption.
*               CNcomment:关中断保护的操作结束之后，必须调用接口uapi_int_restore恢复中断前的状态。CNend
*
* @param  None
*
* @retval #td_u32 Interruption status value  Interrupt status before interrupt off.
CNcomment:中断状态值  关中断前的中断状态。CNend
*
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_int_restore。
*/
td_u32 uapi_int_lock(td_void);

td_void uapi_int_unlock(td_void);

/**
* @ingroup  iot_isr
* @brief  Restores the status before interrupt off.CNcomment:恢复关中断前的状态。CNend
*
* @par 描述:
*           Restores the status before interrupt off.CNcomment:恢复关中断前的状态。CNend
*
* @attention The input argument must be the value of CPSR that is saved before interrupt off.
CNcomment:入参必须是与之对应的关中断时保存的关中断之前的CPSR的值。CNend
*
* @param  int_value [IN] type #td_u32，Interrupt status.CNcomment:中断状态。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_int_lock。
*/
td_void uapi_int_restore(td_u32 int_value);

/**
* @ingroup  iot_isr
* @brief  Check whether int is locked. CNcomment:检查是否关中断。CNend
*
* @par 描述:
*         Check whether int is locked. CNcomment:检查是否关中断。CNend
*
* @attention  None
* @param  None
*
* @retval #EXT_FALSE    int unlocked.CNcomment:未关中断。CNend
* @retval #EXT_TRUE     int locked.CNcomment:已关中断。CNend
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  None
*/
td_bool uapi_int_is_lock(td_void);

/**
* @ingroup  iot_isr
* @brief  Enables a specified interrupt.CNcomment:使能指定中断。CNend
*
* @par 描述:
*           Enables a specified interrupt.CNcomment:使能指定中断。CNend
*
* @attention None
*
* @param  vector [IN] type #td_u32，Interrupt ID, 0-25 are system interrupts, please don't use.
CNcomment:中断号, 其中0-25为系统中断，请不要使用。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure. For details, see soc_errno.h
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_irq_disable。
*/
ext_errno uapi_irq_enable(td_u32 vector);

/**
* @ingroup  iot_isr
* @brief  Disables a specified interrupt.CNcomment:去使能指定中断。CNend
*
* @par 描述:
*           Disables a specified interrupt.CNcomment:去使能指定中断。CNend
*
* @attention None
* @param  vector [IN] type #td_u32，Interrupt ID, 0-25 are system interrupts, please don't use.
CNcomment:中断号, 其中0-25为系统中断，请不要使用。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_irq_enable。
*/
td_void uapi_irq_disable(td_u32 vector);

/* 由于不支持中断嵌套，仅当中断同时触发的场景下有效。 */
#define EXT_IRQ_FLAG_PRI0    0
#define EXT_IRQ_FLAG_PRI1    1
#define EXT_IRQ_FLAG_PRI2    2
#define EXT_IRQ_FLAG_PRI3    3
#define EXT_IRQ_FLAG_PRI4    4
#define EXT_IRQ_FLAG_PRI5    5
#define EXT_IRQ_FLAG_PRI6    6
#define EXT_IRQ_FLAG_PRI_MASK  0x7
#define EXT_IRQ_FLAG_NOT_IN_FLASH 0x10

#define EXT_IRQ_FLAG_DEFAULT    EXT_IRQ_FLAG_NOT_IN_FLASH

/**
* @ingroup  iot_isr
* @brief  Registers an interrupt.CNcomment:注册中断。CNend
*
* @par 描述:
*           Registers an interrupt.CNcomment:注册中断。CNend
*
* @attention The interruption handling program cannot take too long a time, which affects the timely response of the
*            CPU to the interrupt.CNcomment:中断处理程序耗时不能过长，影响CPU对中断的及时响应。CNend
*
* @param  vector [IN] type #td_u32，Interrupt ID, 0-25 are system interrupts, please don't use.
CNcomment:中断号, 其中0-25为系统中断，请不要使用。CNend
* @param  reserved [IN] type #td_u32, reserved.CNcomment:保留。CNend
* @param  routine  [IN] type #irq_routine，Interrupt callback function.CNcomment:中断回调函数。CNend
* @param  param    [IN] type #td_u32，Parameter transferred to the callback function.
CNcomment:中断回调函数的入参。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure. For details, see soc_errno.h
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_irq_free。
*/
ext_errno uapi_irq_request(td_u32 vector, td_u32 reserved, irq_routine routine, td_u32 param);

/**
* @ingroup  iot_isr
* @brief  Set interrupts priority.CNcomment:设置中断优先级。CNend
*
* @par 描述:
*           Set interrupts priority.CNcomment:设置中断优先级。CNend
*
* @attention None
* @param  vector [IN] type #td_u32，Interrupt ID, 0-25 are system interrupts, please don't use.
CNcomment:中断号, 其中0-25为系统中断，请不要使用。CNend
* @param  priority [IN] type #td_u32, interrupt priority to be set.CNcomment:要设置的中断优先级。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure. For details, see soc_errno.h
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_irq_free。
*/
ext_errno uapi_irq_set_priority(td_u32 vector, td_u32 priority);

/**
* @ingroup  iot_isr
* @brief  Clears a registered interrupt.CNcomment:清除注册中断。CNend
*
* @par 描述:
*           Clears a registered interrupt.CNcomment:清除注册中断。CNend
*
* @attention None
* @param  vector [IN] type #td_u32，Interrupt ID, 0-25 are system interrupts, please don't use.
CNcomment:中断号, 其中0-25为系统中断，请不要使用。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure. For details, see soc_errno.h
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  uapi_irq_request。
*/
ext_errno uapi_irq_free(td_u32 vector);

/**
* @ingroup  iot_isr
* @brief  Checks whether it is in the interrupt context.CNcomment:检查是否在中断上下文中。CNend
*
* @par 描述:
*           Checks whether it is in the interrupt context.CNcomment:检查是否在中断上下文中。CNend
*
* @attention None
* @param  None
*
* @retval #EXT_FALSE     Not in the interrupt context.CNcomment:不在中断上下文中。CNend
* @retval #EXT_TRUE     In the interrupt context.CNcomment:在中断上下文中。CNend
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  无。
*/
td_bool uapi_is_int_context(td_void);

/**
* @ingroup  iot_isr
* @brief  Pending status of interrupt ID to be cleared.CNcomment:清除指定中断的pending状态。CNend
*
* @par 描述:
*           Clears a registered interrupt.CNcomment:清除指定中断的pending状态。CNend
*
* @attention The interrupt ID smaller than 26 is not supported.CNcomment:不支持小于26的中断号。
* @param  vector [IN] type #td_u32，Interrupt ID.CNcomment:中断号。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  无。
*/
td_void uapi_irq_clear_pending(td_u32 vector);

/**
* @ingroup  iot_isr
* @brief  Get pending status of interrupt ID.CNcomment:获取指定中断的pending状态。CNend
*
* @par 描述:
*           Get interrupt pending status.CNcomment:获取指定中断的pending状态。CNend
*
* @attention The interrupt ID smaller than 26 is not supported.CNcomment:不支持小于26的中断号。
* @param  vector [IN] type #td_u32，Interrupt ID.CNcomment:中断号。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  无。
*/
td_bool uapi_irq_is_pending(td_u32 vector);

/**
* @ingroup  iot_isr
* @brief  Set the interrupt context state.CNcomment:设置中断上下文状态。CNend
*
* @par 描述:
*         Set the interrupt context state.CNcomment:设置中断上下文状态。CNend
*
* @attention Applies only to products that cannot call @uapi_int_lock and @uapi_int_restore functions in interrupts.
*            CNcomment:只应用于不能在中断中调用@uapi_int_lock和@uapi_int_restore的产品中(Uranus cgra)。
* @param  state [IN] type #td_bool，context state : 中断上下文状态。CNend
*               #EXT_FALSE    Not in the interrupt context.CNcomment:不在中断上下文中。CNend
*               #EXT_TRUE     In the interrupt context.CNcomment:在中断上下文中。CNend
*
* @retval None
* @par 依赖:
*            @li soc_isr.h：Describes ISR APIs.CNcomment:文件用于描述ISR相关接口。CNend
* @see  无。
*/
td_void uapi_irq_set_int_context(td_bool state);

#endif

