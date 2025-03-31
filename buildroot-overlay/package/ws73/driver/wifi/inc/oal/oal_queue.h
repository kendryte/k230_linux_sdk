/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: oal_queue 的头文件
 * Create: 2012年11月29日
 */

#ifndef __OAL_QUEUE_H__
#define __OAL_QUEUE_H__

#include "oal_types.h"
#include "oal_mem_hcm.h"

#include "common_dft_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_QUEUE_H

#define OAL_QUEUE_DESTROY

typedef struct {
    osal_u8           element_cnt;    /* 本队列中已经存放的元素个数 */
    osal_u8           max_elements;   /* 本队列中所能存放的最大元素个数 */
    osal_u8           tail_index;     /* 指向下一个元素入队位置的索引 */
    osal_u8           head_index;     /* 指向当前元素出队位置的索引 */
    osal_ulong        *pul_buf;           /* 队列缓存 */
} oal_queue_stru;

typedef struct {
    osal_u16          element_cnt;    /* 本队列中已经存放的元素个数 */
    osal_u16          max_elements;   /* 本队列中所能存放的最大元素个数 */
    osal_u16          tail_index;     /* 指向下一个元素入队位置的索引 */
    osal_u16          head_index;     /* 指向当前元素出队位置的索引 */
    osal_ulong        *pul_buf;           /* 队列缓存 */
} oal_queue_stru_16;

/*****************************************************************************
 函 数 名  : oal_queue_set
 功能描述  : 设置队列参数
 输入参数  : queue      : 队列指针
             pul_buf        : 指向队列缓冲区的指针
             max_elements: 最大元素个数
 输出参数  : 无
 返 回 值: 无

 修改历史    :
  1.日    期   : 2012年10月23日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void oal_queue_set(oal_queue_stru *queue, osal_ulong *pul_buf, osal_u8 max_elements)
{
    queue->pul_buf         = pul_buf;

    queue->tail_index   = 0;
    queue->head_index   = 0;
    queue->element_cnt  = 0;
    queue->max_elements = max_elements;
}

OAL_STATIC OAL_INLINE osal_void oal_queue_set_16(oal_queue_stru_16 *queue, osal_ulong *pul_buf, osal_u16 max_elements)
{
    queue->pul_buf         = pul_buf;

    queue->tail_index   = 0;
    queue->head_index   = 0;
    queue->element_cnt  = 0;
    queue->max_elements = max_elements;
}

/*****************************************************************************
 函 数 名  : oal_queue_enqueue
 功能描述  : 元素入队
 输入参数  : queue: 队列指针
             p_element: 元素指针
 输出参数  : 无
 返 回 值  : OAL_SUCC 或其它错误码

 修改历史:
  1.日    期   : 2012年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32  oal_queue_enqueue(oal_queue_stru *queue, osal_void *p_element)
{
    osal_u8   tail_index;

    /* 异常: 队列已满 */
    if (queue->element_cnt == queue->max_elements) {
        return OAL_FAIL;
    }

    tail_index = queue->tail_index;

    /* 将元素的地址保存在队列中 */
    queue->pul_buf[tail_index] = (uintptr_t)p_element;

    tail_index++;

    queue->tail_index = ((tail_index >= queue->max_elements) ? 0 : tail_index);

    queue->element_cnt++;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE osal_u32  oal_queue_enqueue_16(oal_queue_stru_16 *queue, osal_void *p_element)
{
    osal_u16   tail_index;

    /* 异常: 队列已满 */
    if (queue->element_cnt == queue->max_elements) {
        return OAL_FAIL;
    }

    tail_index = queue->tail_index;

    /* 将元素的地址保存在队列中 */
    queue->pul_buf[tail_index] = (uintptr_t)p_element;

    tail_index++;

    queue->tail_index = ((tail_index >= queue->max_elements) ? 0 : tail_index);

    queue->element_cnt++;

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE osal_u32  oal_queue_enqueue_8user(oal_queue_stru_16 *queue, osal_void *p_element)
{
    osal_u16   tail_index;
    osal_u16 tail_index_next;

    /* 异常: 队列已满 */
    if (queue->element_cnt == queue->max_elements) {
        return OAL_FAIL;
    }

    tail_index = (queue->tail_index > 0) ? (queue->tail_index - 1) : (queue->max_elements - 1);

    /* 将元素的地址保存在队列中 */
    queue->pul_buf[tail_index] = (uintptr_t)p_element;

    queue->tail_index = tail_index;

    tail_index_next = (tail_index + 1 < queue->max_elements) ? (tail_index + 1) : 0;
    /* 保证element 1(user idx 0) 一直处于tail index */
    if ((queue->element_cnt > 0) && (queue->pul_buf[tail_index_next] == 1)) {
        queue->pul_buf[tail_index_next] = (uintptr_t)p_element;
        queue->pul_buf[tail_index] = 1;
    }

    queue->element_cnt++;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : oal_queue_dequeue
 功能描述  : 元素出队
 输入参数  : queue: 队列指针
 输出参数  : 无
 返 回 值  : 成功: 事件指针
             失败: OAL_PTR_NULL

 修改历史      :
  1.日    期   : 2012年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void* oal_queue_dequeue(oal_queue_stru *queue)
{
    osal_u8    head_index;
    osal_void     *p_element;

    /* 异常: 队列为空 */
    if (queue->element_cnt == 0) {
        return OAL_PTR_NULL;
    }

    head_index = queue->head_index;

    p_element = (osal_void *)(uintptr_t)queue->pul_buf[head_index];

    head_index++;

    queue->head_index = ((head_index >= queue->max_elements) ? 0 : head_index);
    queue->element_cnt--;

    return p_element;
}

OAL_STATIC OAL_INLINE osal_void *oal_queue_dequeue_8user(oal_queue_stru_16 *queue)
{
    osal_u16    tail_index;
    osal_void     *p_element;

    /* 异常: 队列为空 */
    if (queue->element_cnt == 0) {
        return OAL_PTR_NULL;
    }

    tail_index = queue->tail_index;

    p_element = (osal_void *)(uintptr_t)queue->pul_buf[tail_index];

    tail_index++;

    queue->tail_index = (tail_index >= queue->max_elements) ? 0 : tail_index;
    queue->element_cnt--;

    return p_element;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_data_stru.h */
