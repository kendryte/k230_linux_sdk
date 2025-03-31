/**
* @file soc_men.h
*
* Copyright (c) CompanyNameMagicTag 2019-2019. All rights reserved.  \n
* Description: Memory management.   \n
* Author: CompanyName   \n
* Create: 2019-12-18
*/

/**
 * @defgroup iot_mem  Memory
 * @ingroup osa
 */

#ifndef __SOC_MEM_H__
#define __SOC_MEM_H__
#include <td_base.h>

/**
 * @ingroup iot_mem
 * Overall memory information.CNcomment:整体内存信息。CNend
 */
typedef struct {
    td_u32 total;                /* Total space of the memory pool (unit: byte).
                                    CNcomment:内存池总大小（单位：byte）CNend */
    td_u32 used;                 /* Used space of the memory pool (unit: byte).
                                    CNcomment:内存池已经使用大小（单位：byte）CNend */
    td_u32 free;                 /* Free space of the memory pool (unit: byte).
                                    CNcomment:内存池剩余空间（单位：byte）CNend */
    td_u32 free_node_num;        /* Number of free nodes in the memory pool.
                                    CNcomment:内存池剩余空间节点个数 CNend */
    td_u32 used_node_num;        /* Number of used nodes in the memory pool.
                                    CNcomment:内存池已经使用的节点个数 CNend */
    td_u32 max_free_node_size;   /* Maximum size of the node in the free space of the memory pool (unit: byte).
                                    CNcomment:内存池剩余空间节点中最大节点的大小（单位：byte）CNend */
    td_u32 malloc_fail_count;    /* Number of memory application failures.CNcomment:内存申请失败计数 CNend */
    td_u32 peek_size;            /* Peak memory usage of the memory pool.CNcomment:内存池使用峰值CNend */
} ext_mdm_mem_info;

typedef struct {
    td_u32 pool_addr;       /* 内存池地址 */
    td_u32 pool_size;       /* 内存池大小 */
    td_u32 fail_count;      /* 内存申请失败计数 */
    td_u32 cur_use_size;    /* 内存池已经使用大小（单位：byte） */
    td_u32 peek_size;       /* 内存池使用峰值 */
} ext_mem_pool_crash_info;

/**
* @ingroup  iot_mem
* @brief  Dynamically applies for memory.CNcomment:动态申请内存。CNend
*
* @par 描述:
*           Dynamically applies for memory.CNcomment:动态申请内存。CNend
*
* @attention None
* @param  size    [IN] type #td_u32，Requested memory size (unit: byte)
CNcomment:申请内存大小（单位：byte）。CNend
*
* @retval #>0 Success
* @retval #EXT_NULL   Failure. The memory is insufficient.
* @par 依赖:
*            @li soc_mem.h：Describes memory APIs.CNcomment:文件用于描述内存相关接口。CNend
* @see  uapi_free。
*/
td_pvoid uapi_malloc(td_u32 size);

/**
* @ingroup  iot_mem
* @brief  Releases the memory that is dynamically applied for.CNcomment:释放动态申请的内存。CNend
*
* @par 描述:
*          Releases the memory that is dynamically applied for.CNcomment:释放动态申请的内存。CNend
*
* @attention None
* @param  addr    [IN] type #td_pvoid，Start address of the requested memory. The validity of the address is ensured
*                 by the caller.CNcomment:所申请内存的首地址，地址合法性由调用者保证。CNend
*
* @retval None
* @par 依赖:
*            @li soc_mem.h：Describes memory APIs.CNcomment:文件用于描述内存相关接口。CNend
* @see  uapi_malloc。
*/
td_void uapi_free(const td_pvoid addr);

/**
* @ingroup  iot_mem
* @brief  Obtains memory information, used in a crash process.
CNcomment:获取内存信息，死机流程中使用。CNend
*
* @par 描述:
*           Obtains memory information, used in a crash process. When the board is reset due to a memory exception,
*           if ext_os_get_mem_sys_info is used to obtain memory information, another exception may occur. In this case,
*           use ext_mem_get_sys_info_crash instead.CNcomment:获取内存信息，死机流程中使用。当内存异常导致单板复位时，
如果通过ext_mem_get_sys_info获取内存信息可能再次产生异常，此时应该使用ext_mem_get_sys_info_crash。CNend
*
* @attention None
*
* @retval #ext_mem_pool_crash_info   Memory information.CNcomment:内存信息。CNend
*
* @par 依赖:
*            @li soc_mem.h：Describes memory APIs.CNcomment:文件用于描述内存相关接口。CNend
* @see  None
*/
TD_CONST ext_mem_pool_crash_info *uapi_mem_get_sys_info_crash(td_void);

/*
 * Description: This API is used to init mem pool.
 */
ext_errno uapi_pool_mem_init(td_void *pool, td_u32 size);

/*
 * Description: This API is used to alloc a memory block from the @pool.
 */
td_void *uapi_pool_mem_alloc(td_void *pool, td_ulong size);

/*
 * Description: This API is used to free @addr to the @pool.
 */
td_void uapi_pool_mem_free(td_void *pool, TD_CONST td_void *addr);

/*
 * Description: This API is used to deinit mem pool.
 */
ext_errno uapi_pool_mem_deinit(td_void *pool);
#endif
