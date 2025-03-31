/**
* @file td_sem.h
*
* Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.  \n
* Description: Semaphore APIs.CNcomment:接口。CNend
*
* @li Wait semaphore. In the interrupt function, disable interrupt context, and lock task context, this API must not be
*     called. Otherwise, uncontrollable exception scheduling may result.CNcomment:等待信号量:在中断、关中断、
锁任务上下文禁止调用等待信号量接口，进而产生不可控的异常调度。CNend
* @li Release semaphore.In the disable interrupt context, this API must not be called. Otherwise, uncontrollable
*     exception scheduling may result.CNcomment:释放信号量:在关中断上下文禁止调用释放信号量接口，
进而产生不可控的异常调度。CNend   \n
* Author: CompanyName   \n
* Create: 2019-05-29
*/

/**
 * @defgroup iot_sem Semaphore
 * @ingroup osa
 */
#ifndef __SOC_SEM_H__
#define __SOC_SEM_H__
#include <td_base.h>

#define EXT_SEM_ONE  ((td_u8)1) /* ucInit Obtained value of the input: Critical resource protection.
                                 CNcomment:输入的取值: 临界资源保护 CNend */
#define EXT_SEM_ZERO ((td_u8)0) /* ucInit Obtained value of the input: Synchronization
                                 CNcomment:输入的取值: 同步 CNend */

/**
* @ingroup  iot_sem
* @brief  Creates a semaphore.CNcomment:创建信号量。CNend
*
* @par 描述:
*           Creates a semaphore.CNcomment:创建信号量。CNend
*
* @attention The blocking mode (permanent blocking or timing blocking) of the semaphore application operation cannot
*            be used in the interrupt, and the interrupt cannot be blocked.CNcomment:信号量申请操作的阻塞模式
（永久阻塞和定时阻塞）不能在中断中使用，中断不能被阻塞。CNend
*
* @param  sem_id          [OUT] type #td_u32*，semaphore ID.CNcomment:信号量ID号。CNend
* @param  init_value      [IN]  type #td_u16，Number of initialized valid signals. The value range is [0, 0xFFFF].
CNcomment:有效信号的初始化个数，范围为:[0, 0xFFFF]CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure, for details, see soc_errno.h
* @par 依赖:
*            @li td_sem.h：Describes the semaphore APIs.CNcomment:文件用于描述信号量相关接口。CNend
* @see  td_sem_delete。
*/
ext_errno uapi_sem_create(td_u32 *sem_id, td_u16 init_value);


/**
* @ingroup  iot_sem
* @brief  Creates a two-value semaphore(0/1).CNcomment:创建二值信号量（0/1）。CNend
*
* @par 描述:
*           Creates a two-value semaphore(0/1).CNcomment:创建二值信号量（0/1）。CNend
*
* @attention The blocking mode (permanent blocking or timing blocking) of the semaphore application operation cannot
*            be used in the interrupt, and the interrupt cannot be blocked.CNcomment:信号量申请操作的阻塞模式
（永久阻塞和定时阻塞）不能在中断中使用，中断不能被阻塞。CNend
*
* @param  sem_id      [OUT] type #td_u32*，semaphore ID.CNcomment:信号量ID号。CNend
* @param  init_value  [IN]  type #td_u8，initial value. Generally, when the value is EXT_SEM_ONE, the API is used for
*                     critical resource protection. When the value is EXT_SEM_ZERO, the API is used for synchronization.
CNcomment:初始值。一般情况下，当值为EXT_SEM_ONE时，用作临界资源保护；当值为EXT_SEM_ZERO时，用作同步。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure, for details, see soc_errno.h
* @par 依赖:
*            @li td_sem.h：Describes the semaphore APIs.CNcomment:文件用于描述信号量相关接口。CNend
* @see  td_sem_delete。
*/
ext_errno uapi_sem_bcreate(td_u32 *sem_id, td_u8 init_value);

/**
* @ingroup  iot_sem
* @brief  Deletes the semaphore.CNcomment:删除信号量。CNend
*
* @par 描述:
*           Deletes the semaphore.CNcomment:删除信号量。CNend
*
* @attention The blocking mode (permanent blocking or timing blocking) of the semaphore application operation cannot
*            be used in the interrupt, and the interrupt cannot be blocked.CNcomment:信号量申请操作的阻塞模式
（永久阻塞和定时阻塞）不能在中断中使用，中断不能被阻塞。CNend
*
* @param  sem_id    [IN] type #td_u32，semaphore ID.CNcomment:信号量ID号。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure, for details, see soc_errno.h
* @par 依赖:
*            @li td_sem.h：Describes the semaphore APIs.CNcomment:文件用于描述信号量相关接口。CNend
* @see  td_sem_bcreate。
*/
ext_errno uapi_sem_delete(td_u32 sem_id);

/**
* @ingroup  iot_sem
* @brief  Obtains the semaphore.CNcomment:获取信号量。CNend
*
* @par 描述:
*           Obtains the semaphore.CNcomment:获取信号量。CNend
*
*
* @attention The blocking mode (permanent blocking or timing blocking) of the semaphore application operation cannot
*            be used in the interrupt, and the interrupt cannot be blocked.CNcomment:信号量申请操作的阻塞模式
（永久阻塞和定时阻塞）不能在中断中使用，中断不能被阻塞。CNend
*
* @param  sem_id     [IN] type #td_u32，semaphore ID.CNcomment:信号量ID号。CNend
* @param  timeout_ms [IN] type #td_u32，timeout period (unit: ms). EXT_SYS_WAIT_FOREVER indicates permanent wait.
CNcomment:超时时间（单位：ms），EXT_SYS_WAIT_FOREVER为永久等待。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure, for details, see soc_errno.h
* @par 依赖:
*            @li td_sem.h：Describes the semaphore APIs.CNcomment:文件用于描述信号量相关接口。CNend
* @see  td_sem_signal。
*/
ext_errno uapi_sem_wait(td_u32 sem_id, td_u32 timeout_ms);

/**
* @ingroup  iot_sem
* @brief  Releases the semaphore.CNcomment:释放信号量。CNend
*
* @par 描述:
*           Releases the semaphore.CNcomment:释放信号量。CNend
*
* @attention The blocking mode (permanent blocking or timing blocking) of the semaphore application operation cannot
*            be used in the interrupt, and the interrupt cannot be blocked.CNcomment:信号量申请操作的阻塞模式
（永久阻塞和定时阻塞）不能在中断中使用，中断不能被阻塞。CNend
*
* @param  sem_id    [IN] type #td_u32，semaphore ID.CNcomment:信号量ID号。CNend
*
* @retval #EXT_ERR_SUCCESS Success.
* @retval #Other Failure, for details, see soc_errno.h
* @par 依赖:
*            @li td_sem.h：Describes the semaphore APIs.CNcomment:文件用于描述信号量相关接口。CNend
* @see  td_sem_wait。
*/
ext_errno uapi_sem_signal(td_u32 sem_id);

#endif

