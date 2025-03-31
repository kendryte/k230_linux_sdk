/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of WLAN product specification macro definition.
 * Create: 2020-10-18
 */

#ifndef __OAL_UTIL_HCM_H__
#define __OAL_UTIL_HCM_H__

#include "securec.h"
#include "osal_types.h"
#include "osal_adapt.h"
#include "oal_types.h"
#include "wlan_util_common.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/etherdevice.h>
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define wifi_printf_always(fmt, arg...) osal_printk(fmt, ##arg)
#ifdef _PRE_WIFI_PRINTK
#ifdef CONTROLLER_CUSTOMIZATION
#define wifi_printf(fmt, ...) printk(KERN_EMERG KBUILD_MODNAME "[%s:%d]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define wifi_printf(fmt, arg...) osal_printk(fmt, ##arg)
#endif
#else
#define wifi_printf(fmt, arg...)
#endif


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define OAL_DECLARE_PACKED    __attribute__((__packed__))
#define WIFI_ROM_TEXT           __attribute__ ((section(".wifi.rom.text")))   /* ROM代码段 */
#define WIFI_ROM_RODATA         __attribute__ ((section(".wifi.rom.rodata"))) /* ROM const全局变量段 可与text段复用 */
#define WIFI_ROM_DATA           __attribute__ ((section(".wifi.rom.data")))   /* ROM 初值非0全局变量段 不能与bss混用 */
#define WIFI_ROM_BSS            __attribute__ ((section(".wifi.rom.bss")))    /* ROM 初值0或未赋值全局变量段
                                                                                 可以data段混用 只会影响RAM段大小 */
#if defined(_PRE_WLAN_FEATURE_WS63)
#define WIFI_TCM_TEXT __attribute__((section(".wifi.tcm.text")))
#define WIFI_TCM_RODATA __attribute__((section(".wifi.tcm.rodata")))
#define WIFI_HMAC_TCM_TEXT
#define WIFI_HMAC_TCM_RODATA
#elif defined(_PRE_WLAN_FEATURE_WS53)
#define WIFI_TCM_TEXT
#define WIFI_TCM_RODATA __attribute__((section(".wifi.tcm.rodata")))
#define WIFI_HMAC_TCM_TEXT __attribute__((section(".wifi.tcm.text")))
#define WIFI_HMAC_TCM_RODATA
#else
#define WIFI_TCM_TEXT
#define WIFI_TCM_RODATA
#define WIFI_HMAC_TCM_TEXT
#define WIFI_HMAC_TCM_RODATA
#endif

/* 拼接为16 bit或者 32bit */
#define OAL_MAKE_WORD16(lsb, msb) ((((osal_u16)(msb) << 8) & 0xFF00) | (lsb))
#define OAL_MAKE_WORD32(lsw, msw) ((((osal_u32)(msw) << 16) & 0xFFFF0000) | (lsw))

/* 比较宏 */
#define OAL_MIN(_A, _B) (((_A) < (_B)) ? (_A) : (_B))

/* 比较宏 */
#define OAL_MAX(_A, _B) (((_A) > (_B)) ? (_A) : (_B))
#define OAL_SUB(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : (0))
#define OAL_ABSOLUTE_SUB(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : ((_B) - (_A)))

#define OAL_ARRAY_SIZE(_ast_array)                  (sizeof(_ast_array) / sizeof((_ast_array)[0]))

/* 修改_PRE_BIG_CPU_ENDIAN保持HMAC&DMAC统一 */
/* BIG_ENDIAN */
#if defined(_PRE_BIG_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) && (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)
#define oal_ntoh_16(_val)                  (_val)
#define oal_ntoh_32(_val)                  (_val)
#define oal_hton_16(_val)                  (_val)
#define oal_hton_32(_val)                  (_val)
#define OAL_NTOH_16(_val)                  (_val)
#define OAL_NTOH_32(_val)                  (_val)
#define OAL_HTON_16(_val)                  (_val)
#define OAL_HTON_32(_val)                  (_val)

#else     /* LITTLE_ENDIAN */
#define oal_ntoh_16(_val)                  oal_swap_byteorder_16(_val)
#define oal_ntoh_32(_val)                  oal_swap_byteorder_32(_val)
#define oal_hton_16(_val)                  oal_swap_byteorder_16(_val)
#define oal_hton_32(_val)                  oal_swap_byteorder_32(_val)
#define OAL_NTOH_16(_val)                  oal_swap_byteorder_16(_val)
#define OAL_NTOH_32(_val)                  oal_swap_byteorder_32(_val)
#define OAL_HTON_16(_val)                  oal_swap_byteorder_16(_val)
#define OAL_HTON_32(_val)                  oal_swap_byteorder_32(_val)
#endif

/* 获取大小 */
#define OAL_SIZEOF                                 sizeof

#define HCC_DATA_LEN_ALIGN 32
/* 获取数组大小 */
#define osal_array_size(_array)                     (sizeof(_array) / sizeof((_array)[0]))

/* 从某个设备读取某个寄存器地址的32-bit寄存器的值。 */
#define OAL_REG_READ32(_addr)    \
        *((OAL_VOLATILE osal_u32 *)(_addr))

#define OAL_REG_READ16(_addr)    \
    *((OAL_VOLATILE osal_u16 *)(_addr))

/* 往某个设备某个32-bit寄存器地址写入某个值 */
#define OAL_REG_WRITE32(_addr, _val)    \
    (*((OAL_VOLATILE osal_u32 *)(_addr)) = (_val))
#define OAL_REG_WRITE16(_addr, _val)    \
    (*((OAL_VOLATILE osal_u16 *)(_addr)) = (_val))

#ifndef round_mask
#define round_mask(x, y) ((__typeof__(x))((y) - 1))
#endif

#ifndef round_up
#define round_up(x, y) ((((x) - 1) | round_mask(x, y)) + 1)
#endif
#define OAL_ROUND_UP        round_up

#ifndef round_down
#define round_down(x, y) ((x) & ~round_mask(x, y))
#endif
#define OAL_ROUND_DOWN      round_down

#define OAL_IPV4_ADDR_LEN 6
#define OAL_IPV6_ADDR_LEN 16

/* 获取CORE ID */
#define OAL_GET_CORE_ID()    (0)

typedef osal_ulong    oal_bitops;

#define OAL_OFFSET_OF           offsetof

typedef ssize_t     oal_ssize_t;
typedef size_t      oal_size_t;

#define OAL_STRLEN                                  osal_strlen
#define OAL_MEMCMP                                  osal_memcmp
#define OAL_STRSTR                                  osal_strstr

#define todigit(c) ((c) - '0')
#define tochar(n)  ((n) + '0')
#define oal_tolower(x) ((osal_u8)(x) | 0x20)
#define oal_isdigit(c) ('0' <= (c) && (c) <= '9')
#define oal_isxdigit(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))

#define oal_is_null_ptr4(ptr1, ptr2, ptr3, ptr4) \
    ((NULL == (ptr1)) || (NULL == (ptr2)) || (NULL == (ptr3)) || (NULL == (ptr4)))

static inline osal_slong oal_strtol(OAL_CONST osal_s8 *pc_nptr, osal_s8 **ppc_endptr, osal_s32 l_base);
/*****************************************************************************
 函 数 名  : oal_bit_find_first_zero_one_byte
 功能描述  : 找到1字节右数第一个是0的位数
*****************************************************************************/
static inline osal_u8 oal_bit_find_first_zero_one_byte(osal_u8 byte)
{
    osal_u8 zero_bit = 0;

    byte = ~byte;
    byte = byte & (-byte);

    while (byte != 1) {
        zero_bit++;
        byte = (byte >> 1);

        if (zero_bit > 7) {  /* 7表示超过字节最大bit数。退出循环 */
            return zero_bit;
        }
    }

    return zero_bit;
}

/*****************************************************************************
 函 数 名  : oal_set_mac_addr_zero
 功能描述  : mac地址置零
*****************************************************************************/
static inline osal_void oal_set_mac_addr_zero(osal_u8 *mac_addr)
{
    const osal_u32 mac_len = 6; /* 6字节mac地址长度 */
    (osal_void)memset_s(mac_addr, mac_len, 0, mac_len);
}

/*****************************************************************************
 函 数 名  : osal_strtohex
 功能描述  : 将一个字符转化成16进制数
*****************************************************************************/
static inline osal_u8 oal_strtohex(const osal_s8 *string)
{
    osal_u8 ret = 0;

    if (*string >= '0' && *string <= '9') {
        ret = (osal_u8)(*string - '0');
    } else if (*string >= 'A' && *string <= 'F') {
        ret = (osal_u8)(*string - 'A' + 10); /* 10进制转换 */
    } else if (*string >= 'a' && *string <= 'f') {
        ret = (osal_u8)(*string - 'a' + 10); /* 10进制转换 */
    } else {
        ret = 0xff;
    }

    return ret;
}

/* ****************************************************************************
 函 数 名  : oal_strtoaddr
 功能描述  : 字符串转MAC地址
**************************************************************************** */
static inline osal_void oal_strtoaddr(const osal_char *pc_param, osal_u8 *mac_addr)
{
    osal_u8 char_index;

    /* 获取mac地址,16进制转换 */
    for (char_index = 0; char_index < 12; char_index++) { /* 12 mac地址字符长度 */
        while ((*pc_param == ':') || (*pc_param == '-')) {
            pc_param++;

            continue;
        }

        /* 处理2字符, 16表字节内高4bit */
        mac_addr[char_index / 2] = (osal_u8)(mac_addr[char_index / 2] * 16 * (char_index % 2) +
            oal_strtohex((const osal_s8 *)pc_param));
        pc_param++;
    }
}

/*****************************************************************************
 函 数 名  : oal_strtoipv6
 功能描述  : 字符串转ipv6地址
 输入参数  : pc_param: ipv6地址字符串, 格式 xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx
 输出参数  : mac_addr: 转换成16进制后的ipv6地址
*****************************************************************************/
static inline osal_void oal_strtoipv6(const osal_s8 *pc_param, osal_u8 *ipv6_addr)
{
    osal_u8   char_index;

    /* 获取ipv6地址,16进制转换,每个byte对应2位十六进制数 */
    for (char_index = 0; char_index < OAL_IPV6_ADDR_LEN * 2; char_index++) {
        if ((*pc_param == ':')) {
            pc_param++;
            if (char_index != 0) {
                char_index--;
            }
            continue;
        }
        /* 将ipv6字符串转换为16进制数组,4 2分别用作位移数及求余 */
        ipv6_addr[char_index >> 1] = (osal_u8)(((ipv6_addr[char_index>>1]) << 4) * (char_index % 2) +
            oal_strtohex(pc_param));
        pc_param++;
    }
}

static inline osal_void oal_strtoipv4(const osal_s8 *ip_str, osal_u8 *ipv4_addr)
{
    osal_char *pc_token;
    osal_char *pc_ctx;
    osal_char *pc_sep = ".";
    osal_s8 param_num = 3; /* IPV4地址有4段, 数组idx为0/1/2/3 */

    pc_token = strtok_s((osal_char *)ip_str, (OAL_CONST osal_char *)pc_sep, &pc_ctx);
    while (pc_token) {
         /* 10 base权值 */
        *(ipv4_addr + param_num) = (osal_u8)oal_strtol((OAL_CONST osal_s8 *)pc_token, OAL_PTR_NULL, 10);
        pc_token = strtok_s(OAL_PTR_NULL, (OAL_CONST osal_char *)pc_sep, &pc_ctx);
        param_num--;
    }
}

/*****************************************************************************
 函 数 名  : oal_gen_random
 功能描述  : 产生随机数
 输入参数  : val:随机种子   rst_flag:0:不更新随机种子，非0:更新随机种子
*****************************************************************************/
static inline osal_u8 oal_gen_random(osal_u32 val, osal_u8 rst_flag)
{
    OAL_STATIC osal_u32 rand = 0;
    if (rst_flag != 0) {
        rand = val;
    }
    rand = rand * 1664525L + 1013904223L;
    /* 32位 - 8位 = 24位，右移24位得到u8 */
    return (osal_u8) (rand >> 24);
}

/*****************************************************************************
 函 数 名  : oal_strim
 功能描述  : 去掉字符串开始与结尾的空格
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
static inline osal_s8 *oal_strim(osal_s8 *s)
{
    osal_s8 *end;

    while (*s == ' ') {
        ++s;
    }

    end = s;
    while ((*end != ' ') && (*end != '\0')) {
        ++end;
    }

    *end = '\0';

    return s;
}

static inline void oal_eth_random_addr(osal_u8 *addr, osal_u8 len)
{
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    struct timeval tv1;
    struct timeval tv2;
    unref_param(len);

    /* 获取随机种子 */
    gettimeofday(&tv1, NULL);

    /* 防止秒级种子为0 */
    tv1.tv_sec += 2; /* 加2 */

    tv2.tv_sec = (osal_slong)((td_u32)((td_u64)tv1.tv_sec * tv1.tv_sec) * (td_u64)tv1.tv_usec);
    tv2.tv_usec = (osal_slong)((td_u32)((td_u64)tv1.tv_sec * tv1.tv_usec) * (td_u64)tv1.tv_usec);

    /* 生成随机的mac地址 */
    addr[0] = ((td_u32)tv2.tv_sec & 0xff) & 0xfe;
    addr[1] = (osal_u8)((td_u32)tv2.tv_usec & 0xff);
    addr[2] = (osal_u8)(((td_u32)tv2.tv_sec & 0xff0) >> 4);   /* mac_addr[2]右移4 bit */
    addr[3] = (osal_u8)(((td_u32)tv2.tv_usec & 0xff0) >> 4);  /* mac_addr[3]右移4 bit */
    addr[4] = (osal_u8)(((td_u32)tv2.tv_sec & 0xff00) >> 8);  /* mac_addr[4]右移8 bit */
    addr[5] = (osal_u8)(((td_u32)tv2.tv_usec & 0xff00) >> 8); /* mac_addr[5]右移8 bit */
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    unref_param(len);
    eth_random_addr(addr);
#endif
}

/*****************************************************************************
 函 数 名  : oal_byteorder_host_to_net_uint16
 功能描述  : 将16位本地字节序转换为网络字节序
 输入参数  : byte: 需要字节序转换的变量
 输出参数  : 无
 返 回 值  : 转换好的变量
*****************************************************************************/
static inline osal_u16 oal_byteorder_net_to_host_uint16(osal_u16 byte)
{
    return oal_ntoh_16(byte);
}

/*****************************************************************************
 函 数 名  : oal_byteorder_host_to_net_uint16
 功能描述  : 将32位本地字节序转换为网络字节序
 输入参数  : byte: 需要字节序转换的变量
 输出参数  : 无
 返 回 值  : 转换好的变量
*****************************************************************************/
static inline osal_u32 oal_byteorder_host_to_net_uint32(osal_u32 byte)
{
    return oal_hton_32(byte);
}

/*****************************************************************************
 函 数 名  : oal_atoi
 功能描述  : 字符串类型转换成整形
 输入参数  : c_string: 字符串地址
*****************************************************************************/
static inline osal_s32 oal_atoi(const osal_s8 *string)
{
    osal_s32 ret = 0;
    osal_s32 flag = 0;

    for (; ; string++) {
        switch (*string) {
            case '0'...'9':
                /* 乘10进位 */
                ret = 10 * ret + (*string - '0');
                break;
            case '-':
                flag = 1;
                break;

            case ' ':
                continue;

            default:
                return ((flag == 0) ? ret : (-ret));
        }
    }
}

static inline unsigned int simple_guess_base(const char *cp)
{
    if (cp[0] == '0') {
        if (oal_tolower(cp[1]) == 'x' && oal_isxdigit(cp[2])) {
            /* 对应16进制 */
            return 16;
        } else {
            /* 对应8进制 */
            return 8;
        }
    } else {
        /* 对应10进制 */
        return 10;
    }
}

static inline unsigned long long oal_simple_strtoull(const osal_s8 *cp, osal_s8 **endp, unsigned int base)
{
    unsigned long long result = 0;

    if (base == 0) {
        base = simple_guess_base((const char *)cp);
    }
    /* 对应16进制 */
    if (base == 16 && cp[0] == '0' && oal_tolower(cp[1]) == 'x') {
        /* 指针后移2位 */
        cp += 2;
    }
    while (oal_isxdigit(*cp)) {
        unsigned int value;
        /* 16进制下，a对应10 */
        value = oal_isdigit(*cp) ? *cp - '0' : oal_tolower(*cp) - 'a' + 10;
        if (value >= base) {
            break;
        }
        result = result * base + value;
        cp++;
    }
    if (endp) {
        *endp = (osal_s8 *)cp;
    }
    return result;
}

static inline osal_slong oal_strtol(OAL_CONST osal_s8 *pc_nptr, osal_s8 **ppc_endptr, osal_s32 l_base)
{
    /* 跳过空格 */
    while ((*pc_nptr) == ' ') {
        pc_nptr++;
    }

    if (*pc_nptr == '-') {
        return (osal_slong)(-oal_simple_strtoull(pc_nptr + 1, ppc_endptr, (osal_u32)l_base));
    }
    return (osal_slong)oal_simple_strtoull(pc_nptr, ppc_endptr, (osal_u32)l_base);
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
static inline osal_void oal_print_hex_dump(const osal_u8* addr, osal_s32 len, osal_s32 groupsize,
    const osal_char* pre_str)
{
    unref_param(groupsize);
    wifi_printf("%s: len = %d", pre_str, len);
    for (td_s32 i = 0; i < len; i++) {
        if (((td_u32)i & 0x000f) == 0) {
            wifi_printf("\r\n%04d: ", i);
        }
        wifi_printf(" %02x", addr[i]);
    }
    wifi_printf("\r\n");
}

static inline osal_void oal_dump_stack(osal_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ArchBackTrace();
#endif
    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

