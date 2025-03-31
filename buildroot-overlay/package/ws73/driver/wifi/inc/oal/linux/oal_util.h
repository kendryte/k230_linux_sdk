/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: oal util interface
 * Create: 2020-05-02
 */

#ifndef __OAL_UTIL_H__
#define __OAL_UTIL_H__
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <asm/string.h>
#include <linux/thread_info.h>
#include <asm/byteorder.h>
#include <linux/byteorder/generic.h>
#include <linux/bitops.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/kobject.h>      /*  add sys for 51/02 */
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/etherdevice.h>  /*  for random mac address */
#include <linux/version.h>
#include <linux/math64.h>

#include <linux/ktime.h>
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_mm.h"
#include "securec.h"

#include "oal_util_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* linux arch相关的代码，待合并处理 */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define OAL_CONST                                   const

typedef struct file              oal_file_stru;
#define OAL_FILE_FAIL            OAL_PTR_NULL

/* 内存读屏障 */
#define OAL_RMB()               rmb()

/* 内存写屏障 */
#define OAL_WMB()               wmb()

/* 内存屏障 */
#define OAL_MB()                mb()

#define OAL_OFFSET_OF          offsetof

#define  OAL_PLAT_MEM_BASE_ADDR  0x80000000

#define OAL_DSCR_VIRT_TO_PHY(_virt_addr)    (virt_to_phys(_virt_addr) - OAL_PLAT_MEM_BASE_ADDR)
#define OAL_DSCR_PHY_TO_VIRT(_phy_addr)     phys_to_virt((_phy_addr) + OAL_PLAT_MEM_BASE_ADDR)

/* 物理地址和虚拟地址之间的转换,作为netbuf用 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,38))
#define OAL_VIRT_TO_PHY_ADDR(_virt_addr)  (virt_to_phys((const volatile void *)(_virt_addr)) - OAL_PLAT_MEM_BASE_ADDR)
#else
#define OAL_VIRT_TO_PHY_ADDR(_virt_addr)  (virt_to_phys((void *)(_virt_addr)) - OAL_PLAT_MEM_BASE_ADDR)
#endif
#define OAL_PHY_TO_VIRT_ADDR(_phy_addr)   phys_to_virt((_phy_addr) + OAL_PLAT_MEM_BASE_ADDR)

typedef struct device_attribute     oal_device_attribute_stru;
typedef struct device               oal_device_stru;
#define OAL_DEVICE_ATTR             DEVICE_ATTR
#define OAL_S_IRUGO                 S_IRUGO
#define OAL_S_IWGRP                 S_IWGRP
#define OAL_S_IWUSR                 S_IWUSR
typedef struct kobject              oal_kobject;

#define OAL_STRCMP                                  strcmp
#define OAL_STRNCMP                                 strncmp
#define OAL_STRNCASECMP                             strncasecmp

/* for sys interface  51/02 */

/* 检查size1不大于size2, 其中size1/2可以使用sizeof(a) */
#define     SIZE_OF_SIZE1_NOT_LARGER_THAN_SIZE2_BY_NAME(name, size1, size2) \
static inline char size_of_##name##_size1_not_larger_than_size2(void) \
{ \
    char __dummy1[size2 - size1]; \
    return __dummy1[0]; \
}

/* 检查结构体的大小是否不大于特定值 */
#define    SIZE_OF_TYPE_NOT_LARGER_THAN_DETECT(type, size) \
static inline char size_of_##type##_not_larger_than_##size(void) \
{ \
    char __dummy1[size - sizeof(type)]; \
    return __dummy1[0]; \
}

/* us time cost sub */
#define time_cost_var_start(name)       local_cost_time_start_##name
#define time_cost_var_end(name)            local_cost_time_end_##name
#define time_cost_var_sub(name)         trans_us_##name

#define oal_get_time_cost_start(name)   time_cost_var_start(name) = ktime_get()
#define oal_get_time_cost_end(name)     time_cost_var_end(name)   = ktime_get()
#define oal_calc_time_cost_sub(name)    time_cost_var_sub(name)   = \
                                    (osal_u64)ktime_to_us(ktime_sub(time_cost_var_end(name), time_cost_var_start(name)))

/*****************************************************************************
 功能描述  : 打印函数调用栈
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void oal_dump_stack(osal_void)
{
    dump_stack();
}

OAL_STATIC OAL_INLINE osal_void oal_print_hex_dump(osal_u8 *addr, osal_s32 len, osal_s32 groupsize, osal_s8 *pre_str)
{
#ifdef CONFIG_PRINTK
    oal_reference(groupsize);
    printk("buf %p,len:%d\n",
           addr,
           len);
    print_hex_dump(KERN_ERR, pre_str, DUMP_PREFIX_ADDRESS, 16, 1,
                   addr, len, true);
    printk("\n");
#endif
}

/*****************************************************************************
 功能描述  : linux osal_ulong 除法
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u64 oal_div_u64(osal_u64 dividend, osal_u32 divisor)
{
    osal_u64 ull_tmp;

    ull_tmp = div_u64(dividend, divisor);

    return ull_tmp;
}
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define OAL_VA_START            va_start
#define OAL_VA_END              va_end

#define OAL_VA_LIST             va_list

#define HCC_DATA_LEN_ALIGN 32

/* 拼接为16 bit或者 32bit */

/* increment with wrap-around */
#define OAL_INCR(_l, _sz)   (_l)++; (_l) &= ((_sz) - 1)
#define OAL_DECR(_l, _sz)   (_l)--; (_l) &= ((_sz) - 1)

/* 获取大小 */
#define OAL_SIZEOF                                  sizeof

/* 获取数组大小 */
#define OAL_ARRAY_SIZE(_ast_array)                  (sizeof(_ast_array) / sizeof((_ast_array)[0]))

/* 四字节对齐 */
#define OAL_GET_4BYTE_ALIGN_VALUE(_ul_size)         (((_ul_size) + 0x03) & (~0x03))

/* 获取当前线程信息 */
#define OAL_CURRENT_TASK     (current_thread_info()->task)

/* BIG_ENDIAN */
#if defined(_PRE_BIG_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) && (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)
#define oal_byteorder_to_le32(_val)        oal_swap_byteorder_32(_val)
#define oal_byteorder_to_le16(_val)        oal_swap_byteorder_16(_val)

/* LITTLE_ENDIAN */
#elif defined(_PRE_LITTLE_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) && (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)
#define oal_byteorder_to_le32(_val)        (_val)
#define oal_byteorder_to_le16(_val)        (_val)
#endif

/* 比较宏 */
#define OAL_MIN(_A, _B) (((_A) < (_B)) ? (_A) : (_B))

/* 比较宏 */
#define OAL_MAX(_A, _B) (((_A) > (_B)) ? (_A) : (_B))

#define OAL_SUB(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : (0))

#define OAL_ABSOLUTE_SUB(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : ((_B) - (_A)))

#define ALL                     0xFFFF
#ifndef BIT
#define BIT(nr)                 (1UL << (nr))
#endif

#define OAL_BITS_PER_BYTE       8   /* 一个字节中包含的bit数目 */

/* 位操作 */
#define OAL_SET_BIT(_val)                           (1 << (_val))
#define OAL_LEFT_SHIFT(_data, _num)                 ((_data) << (_num))
#define OAL_RGHT_SHIFT(_data, _num)                 ((_data) >> (_num))
#define OAL_WRITE_BITS(_data, _val, _bits, _pos)    do { \
        (_data) &= ~((((osal_u32)1 << (_bits)) - 1) << (_pos)); \
        (_data) |= (((_val) & (((osal_u32)1 << (_bits)) - 1)) << (_pos)); \
    } while (0)

#define OAL_IPV6_ADDR_LEN 16

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_util.h */
