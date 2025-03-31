/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal util header
 * Author: Huanghe
 * Create: 2023-01-06
 */

#ifndef __OAL_UTIL_H__
#define __OAL_UTIL_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <asm/string.h>
#include <asm/byteorder.h>
#include <asm/delay.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/thread_info.h>
#include <linux/byteorder/generic.h>
#include <linux/bitops.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/math64.h>
#include <linux/ktime.h>
#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_CONST                                   const

typedef struct kobject              oal_kobject;
#define OAL_WARN_ON(condition) ({ \
    int __ret_warn_on = !!(condition); \
    if (unlikely(__ret_warn_on)) \
        dump_stack(); \
    unlikely(__ret_warn_on); \
})

#define STRTOK_RIGHT_SHIFT       3
#define STRTOK_BITWISE_AND       7

OAL_STATIC OAL_INLINE oal_int32  oal_atoi(const oal_int8 *c_string)
{
    oal_int32 l_ret = 0;
    oal_int32 flag = 0;

    for (; ; c_string++) {
        switch (*c_string) {
            case '0'...'9':
                l_ret = 10 * l_ret + (*c_string - '0');

                break;
            case '-':
                flag=1;
                break;

            case ' ':
                continue;

            default:

                return ((0 == flag) ? l_ret:(-l_ret));
        }
    }
}

OAL_STATIC OAL_INLINE oal_int8 *oal_strtok(oal_int8 *pc_token, OAL_CONST oal_int8 *pc_delemit, oal_int8 **ppc_context)
{
    oal_int8 *pc_str;
    OAL_CONST oal_int8 *pc_ctrl = pc_delemit;

    oal_uint8 uc_map[32];
    oal_int32 l_count;

    static oal_int8 *pc_nextoken;

    /* Clear control map */
    for (l_count = 0; l_count < 32; l_count++) { // uc_maps数组长度为32
        uc_map[l_count] = 0;
    }

    /* Set bits in delimiter table */
    do {
        uc_map[*(td_u8 *)pc_ctrl >> STRTOK_RIGHT_SHIFT] |= (1 << (*(td_u8 *)pc_ctrl & STRTOK_BITWISE_AND));
    } while (*pc_ctrl++);

    /* Initialize str. If string is NULL, set str to the saved
    * pointer (i.e., continue breaking tokens out of the string
    * from the last strtok call) */
    if (pc_token) {
        pc_str = pc_token;
    } else {
        pc_str = pc_nextoken;
    }

    /* Find beginning of token (skip over leading delimiters). Note that
    * there is no token iff this loop sets str to point to the terminal
    * null (*str == '\0') */
    while ((uc_map[*(td_u8 *)pc_str >> STRTOK_RIGHT_SHIFT] & (1 << (*(td_u8 *)pc_str & STRTOK_BITWISE_AND))) && \
        *pc_str) {
        pc_str++;
    }

    pc_token = pc_str;

    /* Find the end of the token. If it is not the end of the string,
    * put a null there. */
    for (; *pc_str ; pc_str++) {
        if (uc_map[*(td_u8 *)pc_str >> STRTOK_RIGHT_SHIFT] & (1 << (*(td_u8 *)pc_str & STRTOK_BITWISE_AND))) {
            *pc_str++ = '\0';
            break;
        }
    }

    /* Update nextoken (or the corresponding field in the per-thread data
    * structure */
    pc_nextoken = pc_str;

    /* Determine if a token has been found. */
    if (pc_token == pc_str) {
        return NULL;
    } else {
        return pc_token;
    }
}

/* Works only for digits and letters, but small and fast */
OAL_STATIC OAL_INLINE char to_lower(const char x)
{
    return ((x) | 0x20);
}

OAL_STATIC OAL_INLINE td_bool is_digit(oal_int8 c)
{
    return ('0' <= (c) && (c) <= '9');
}

OAL_STATIC OAL_INLINE td_bool is_xdigit(const char c)
{
    return (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'));
}

#define OAL_ROUND_DOWN      round_down

OAL_STATIC OAL_INLINE unsigned int simple_guess_base(const char *cp)
{
    if (cp[0] == '0') {
        if (to_lower(cp[1]) == 'x' && is_xdigit(cp[2])) // 判断ox后的cp[2]范围是否符合16进制的范围
            return 16; // 16进制
        else
            return 8; // 8进制
    } else {
        return 10; // 10进制
    }
}

OAL_STATIC OAL_INLINE unsigned long long oal_simple_strtoull(const oal_int8 *cp, oal_int8 **endp, unsigned int base)
{
    unsigned long long result = 0;

    if (!base)
        base = simple_guess_base(cp);

    if (base == 16 && cp[0] == '0' && to_lower(cp[1]) == 'x') // 16进制
        cp += 2;

    while (is_xdigit(*cp)) {
        unsigned int value;

        value = is_digit(*cp) ? *cp - '0' : to_lower(*(oal_uint8 *)cp) - 'a' + 10; // 10进制
        if (value >= base)
            break;
        result = result * base + value;
        cp++;
    }
    if (endp)
        *endp = (oal_int8 *)cp;

    return result;
}

OAL_STATIC OAL_INLINE oal_int  oal_strtol(OAL_CONST oal_int8 *pc_nptr, oal_int8 **ppc_endptr, oal_int32 l_base)
{
    /* 跳过空格 */
    while (' ' == (*pc_nptr)) {
        pc_nptr++;
    }

    if (*pc_nptr == '-') {
        return -oal_simple_strtoull(pc_nptr + 1, ppc_endptr, l_base);
    }

    return oal_simple_strtoull(pc_nptr, ppc_endptr, l_base);
}

OAL_STATIC OAL_INLINE oal_void  oal_udelay(oal_uint u_loops)
{
    udelay(u_loops);
}

OAL_STATIC OAL_INLINE oal_void  oal_msleep(oal_uint32 ul_usecs)
{
    msleep(ul_usecs);
}

OAL_STATIC OAL_INLINE oal_void  oal_usleep_range(oal_ulong min_us, oal_ulong max_us)
{
    usleep_range(min_us, max_us); /* 微妙级睡眠，可能让出CPU */
}

OAL_STATIC OAL_INLINE oal_void oal_print_hex_dump(oal_uint8*addr, oal_int32 len, oal_int32 groupsize,oal_int8* pre_str)
{
#ifdef CONFIG_PRINTK
    OAL_REFERENCE(groupsize);
    printk("buf %p,len:%d\n",
           addr,
           len);
    print_hex_dump(KERN_ERR, pre_str, DUMP_PREFIX_ADDRESS, 16, 1,
		       addr, len, true);
    printk("\n");
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_util.h */
