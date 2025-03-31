/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: CPU usage APIs.
 * Author: CompanyName
 * Create: 2020-12-17
 */
/**
 * @defgroup cpup CPU usage
 * @ingroup  osa
 */

#ifndef __SOC_CPUP_H__
#define __SOC_CPUP_H__

#include <td_base.h>

TD_CPP_START

/**
 * @ingroup hct_cpup
 * CPU usage information structure.
 */
typedef struct {
    td_u8 valid : 1;   /**< Whether the information is valid. TD_TRUE: yes; TD_FALSE: no. */
    td_u8 task : 1;    /**< Whether the message is a task or an interrupt. TD_TRUE: task; TD_FALSE: interrupt. */
    td_u8 pad0 : 6;    /**< Reserved. */
    td_u8 id;          /**< Task ID/Interrupt number. */
    td_u16 pad1;       /**< Reserved. */
    td_u32 permillage; /**< CPU usage (per mil). */
} ext_cpup_item;

/**
 * @ingroup  hct_cpup
 * @brief  Obtains the CPU usage.
 *
 * @par Description:
 *         Obtains the CPU usage between the initialization of the CPU usage module is started or
 *         the CPU usage statistics are reset to each task/interrupt.
 *
 * @attention
 * @li A task/interrupt not scheduled after the CPU usage module is initialized or
 *     the CPU usage statistics are reset is excluded.
 *
 * @param array_count [IN] Type #td_u32 Number of CPU usage records that can be stored
 *                         in the space corresponding to cpup_items.
 * @param cpup_items [OUT] Type #ext_cpup_item* CPU usage information space, applied by the caller.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_cpup.h: Describes CPU usage APIs.
 * @see  None
 */
td_u32 uapi_cpup_get_usage(td_u32 array_count, ext_cpup_item *cpup_items);

/**
 * @ingroup  hct_cpup
 * @brief  Resets CPU usage statistics.
 *
 * @par Description:
 *      Resets CPU usage statistics. After the reset, the CPU usage statistics of all tasks and interrupts are cleared.
 *
 * @attention None
 *
 * @par Dependency:
 *            @li soc_cpup.h: Describes CPU usage APIs.
 * @see  None
 */
td_void uapi_cpup_reset_usage(td_void);


/**
* @ingroup  iot_cpu
* @brief  Resets CPU usage statistics. CNcomment:重置cpu使用情况信息。CNend
*
* @par 描述:
*         Resets CPU usage statistics. After the reset, the CPU usage statistics of all tasks and interrupts are
*         cleared. CNcomment:重置cpu使用情况信息，重置后所有任务和中断的cpu占用率清0。CNend
*
* @attention None
*
* @param task_id [in] the id of current task
* @param ms [in] ms, idle 态任务占用率小于门限后，当前任务需要睡多长时间
* @par Dependency:
*            @li soc_cpup.h: Describes CPU usage APIs.
CNcomment:文件用于描述cpu相关接口。CNend
* @see None
*/
td_void uapi_cpup_load_check_proc(td_u32 task_id, td_u32 ms);

/**
* @ingroup  hct_cpup
* @brief  Initializes the CPU usage statistics module.
*
* @par Description:
*         Initializes the CPU usage statistics module.
*
* @attention
* @li This API needs to be called before other APIs. This API can be called only once.
* @li The user needs to modify the API implementation as required.
* @retval #0   Success
* @retval #Other values Failure. For details, see soc_errno.h.
* @par Dependency:
*            @li soc_cpup.h: Describes CPU usage APIs.
* @see  None
*/
td_void uapi_cpup_init(td_void);

TD_CPP_END
#endif /* __SOC_CPUP_H__ */
