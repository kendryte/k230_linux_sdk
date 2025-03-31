/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: oam log printing interface
 */

#ifndef __WLAN_UTIL_COMMON_ROM_H__
#define __WLAN_UTIL_COMMON_ROM_H__

#include "securec.h"
#include "td_type.h"

#include "osal_types.h"

#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_RSSI_INIT_MARKER 0x320 /* RSSI平滑值初始值 */
#define OAL_RSSI_MAX_DELTA 24      /* 最大步长 24/8 = 3 */
#define OAL_RSSI_FRACTION_BITS 3
#define OAL_RSSI_SIGNAL_MIN     (-103)   /* 上报RSSI下边界 */
#define OAL_RSSI_SIGNAL_MAX 5      /* 上报RSSI上边界 */
#define OAL_SNR_INIT_VALUE 0x7F    /* SNR上报的初始值 */
#define OAL_RSSI_INIT_VALUE (-128) /* RSSI的初始值 */

/* 位数定义 */
#define NUM_1_BITS 1
#define NUM_2_BITS 2
#define NUM_3_BITS 3
#define NUM_4_BITS 4
#define NUM_5_BITS 5
#define NUM_8_BITS 8
#define NUM_16_BITS 16
#define NUM_20_BITS 20
#define NUM_26_BITS 26


/* 位偏移定义 */
#define BIT_OFFSET_0 0
#define BIT_OFFSET_1 1
#define BIT_OFFSET_2 2
#define BIT_OFFSET_4 4
#define BIT_OFFSET_5 5
#define BIT_OFFSET_6 6
#define BIT_OFFSET_7 7
#define BIT_OFFSET_8 8
#define BIT_OFFSET_9 9
#define BIT_OFFSET_10 10
#define BIT_OFFSET_11 11
#define BIT_OFFSET_12 12
#define BIT_OFFSET_13 13
#define BIT_OFFSET_14 14
#define BIT_OFFSET_15 15
#define BIT_OFFSET_16 16
#define BIT_OFFSET_17 17
#define BIT_OFFSET_18 18
#define BIT_OFFSET_19 19
#define BIT_OFFSET_20 20
#define BIT_OFFSET_21 21
#define BIT_OFFSET_24 24
#define BIT_OFFSET_26 26

#ifdef BUILD_UT
#define OSAL_STATIC
#else
#define OSAL_STATIC static
#endif

#ifndef unref_param
#define unref_param(P)  ((P) = (P))
#endif

#ifndef unref_param_prv
#define unref_param_prv(P)  ((P) = (P))
#endif

/* 获取数组大小 */
#define oal_array_size(_ast_array) (sizeof(_ast_array) / sizeof((_ast_array)[0]))

#define OAL_BITS_PER_BYTE 8 /* 一个字节中包含的bit数目 */

#define SWAP_BYTEORDER_MSAK_8  8
#define SWAP_BYTEORDER_MSAK_24 24
#define SWAP_BYTEORDER_MSAK_40 40
#define SWAP_BYTEORDER_MSAK_56 56

/* 大小端转换 */
#define oal_swap_byteorder_16(_val) ((((_val) & 0x00FF) << 8) + (((_val) & 0xFF00) >> 8))

/* todo 修改_PRE_BIG_CPU_ENDIAN保持HMAC&DMAC统一 */
#if defined(_PRE_BIG_CPU_ENDIAN) && defined(_PRE_BIG_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) \
    && (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN)          /* BIG_ENDIAN */
#define oal_byteorder_to_le32(_val)        oal_swap_byteorder_32(_val)
#define oal_byteorder_to_le16(_val)        oal_swap_byteorder_16(_val)
#define oal_mask_inverse(_len, _offset)    ((osal_u32)(oal_swap_byteorder_32(~(((1 << (_len)) - 1) << (_offset)))))
#define oal_mask(_len, _offset)            ((osal_u32)(oal_swap_byteorder_32(((1 << (_len)) - 1) << (_offset))))
#else     /* LITTLE_ENDIAN */
#define oal_byteorder_to_le32(_val)        (_val)
#define oal_byteorder_to_le16(_val)        (_val)
#define oal_mask_inverse(_len, _offset)    ((osal_u32)(~(((1UL << (_len)) - 1) << (_offset))))
#define oal_mask(_len, _offset)            ((osal_u32)(((1UL << (_len)) - 1) << (_offset)))
#endif

/* 位操作 */
#define oal_set_bit(_val) (1 << (_val))
#define oal_left_shift(_data, _num) (osal_s64)((osal_u64)(_data) << (_num))
#define oal_rght_shift(_data, _num) ((_data) >> (_num))
#define oal_get_bits(_data, _bits, _pos) (((_data) >> (_pos)) & (((osal_u32)1 << (_bits)) - 1))

/* 32字节序大小端转换 */
static inline osal_u32 oal_swap_byteorder_32(osal_u32 val)
{
    return (((osal_u32)(val & 0xFF) << 24) + ((val & 0xFF00) << 8) +  /* 0xFF,0xFF00掩码，24,8字节移位 */
        ((val & 0xFF0000) >> 8) + ((osal_u32)(val & 0xFF000000) >> 24));  /* 0xFF0000,0xFF000000掩码，24,8字节移位 */
}

/* ****************************************************************************
 函 数 名  : oal_sub
 功能描述  : 求两数之差,限定差值大于0
**************************************************************************** */
static inline osal_u8 oal_sub(osal_u16 a, osal_u16 b)
{
    return (a > b) ? (a - b) : 0;
}

/* ****************************************************************************
 函 数 名  : oal_make_word16
 功能描述  : 将两个8bit数拼接为16bit数
**************************************************************************** */
static inline osal_u16 oal_make_word16(osal_u8 lsb, osal_u8 msb)
{
    return ((((osal_u16)msb << 8) & 0xFF00) | lsb); /* msb 左移 8位 */
}

/* ****************************************************************************
 函 数 名  : oal_make_word32
 功能描述  : 将两个16bit数拼接为32bit数
**************************************************************************** */
static inline osal_u32 oal_make_word32(osal_u16 lsw, osal_u16 msw)
{
    return ((((osal_u32)msw << 16) & 0xFFFF0000) | lsw); /* msw 左移 16位 */
}

/* ****************************************************************************
 函 数 名  : oal_join_word32
 功能描述  : 将四个8bit数拼接为32bit数
**************************************************************************** */
static inline osal_u32 oal_join_word32(osal_u8 lsb, osal_u8 ssb, osal_u8 asb, osal_u8 msb)
{
    return (((osal_u32)msb << 24) | ((osal_u32)asb << 16) | ((osal_u32)ssb << 8) | lsb); /* 分别左移24 16 8 位 */
}

/* ****************************************************************************
 函 数 名  : oal_absolute_sub
 功能描述  : 返回两数之差的绝对值
**************************************************************************** */
static inline osal_u32 oal_absolute_sub(osal_u32 num_a, osal_u32 num_b)
{
    return ((num_a > num_b) ? (num_a - num_b) : (num_b - num_a));
}

/* ****************************************************************************
 函 数 名  : round_pos
 功能描述  : 定点数四舍五入，并取整数部分, fract_bits为小数位数
**************************************************************************** */
static inline osal_s32 round_pos(osal_s32 fix_num, osal_s32 fract_bits)
{
    return ((fix_num + (1 << (fract_bits - 1))) >> fract_bits);
}

/* ****************************************************************************
 函 数 名  : round_neg
 功能描述  : 定点数四舍五入，并取整数部分, fract_bits为小数位数
**************************************************************************** */
static inline osal_s32 round_neg(osal_s32 fix_num, osal_s32 fract_bits)
{
    return (osal_s32)(0 - round_pos(0 - fix_num, fract_bits));
}

/* ****************************************************************************
 函 数 名  : oal_round
 功能描述  : 定点数四舍五入，并取整数部分, fract_bits为小数位数
**************************************************************************** */
static inline osal_s32 oal_round(osal_s32 fix_num, osal_s32 fract_bits)
{
    return (fix_num > 0 ? round_pos(fix_num, fract_bits) : round_neg(fix_num, fract_bits));
}

static inline osal_s32 oal_pow(osal_s32 x, osal_u32 n)
{
    osal_s32 result = 1;
    while (n) {
        if (n & 0x1) {
            result *= x;
        }
        n >>= 1;
        x *= x;
    }
    return result;
}

/* ****************************************************************************
 函 数 名  : oal_bit_get_num_four_byte
 功能描述  : 获取4字节中bit1的个数
**************************************************************************** */
static inline osal_u32 oal_bit_get_num_four_byte(osal_u32 byte)
{
    byte = (byte & 0x55555555) + ((byte >> 1) & 0x55555555); /* 0x55555555掩码, 1移位数 */
    byte = (byte & 0x33333333) + ((byte >> 2) & 0x33333333); /* 0x33333333掩码, 2移位数 */
    byte = (byte & 0x0F0F0F0F) + ((byte >> 4) & 0x0F0F0F0F); /* 0x0F0F0F0F掩码, 4移位数 */
    byte = (byte & 0x00FF00FF) + ((byte >> 8) & 0x00FF00FF); /* 0x00FF00FF掩码, 8移位数 */
    byte = (byte & 0x0000FFFF) + ((byte >> 16) & 0x0000FFFF); /* 0x0000FFFF掩码, 16移位数 */

    return byte;
}

/* ****************************************************************************
 函 数 名  : oal_bit_set_bit_one_byte
 功能描述  : 对1字节的指定位置一
**************************************************************************** */
static inline osal_void oal_bit_set_bit_one_byte(osal_u8 *byte, osal_u32 nr)
{
    *byte |= ((osal_u8)(1 << nr));
}

/* ****************************************************************************
 函 数 名  : oal_bit_clear_bit_one_byte
 功能描述  : 对1字节的指定位置零
**************************************************************************** */
static inline osal_void oal_bit_clear_bit_one_byte(osal_u8 *byte, osal_u32 nr)
{
    *byte &= (~((osal_u8)(1 << nr)));
}

static inline osal_u8 oal_bit_get_bit_one_byte(osal_u8 byte, osal_u32 nr)
{
    return ((byte >> nr) & 0x1);
}

/* ****************************************************************************
 函 数 名  : oal_bit_set_bit_eight_byte
 功能描述  : 对8字节的指定位置一
**************************************************************************** */
static inline osal_void oal_bit_set_bit_eight_byte(osal_u64 *pull_byte, osal_u32 nr)
{
    *pull_byte |= ((osal_u64)1 << nr);
}

/* ****************************************************************************
 函 数 名  : oal_bit_find_first_bit_one_byte
 功能描述  : 找到1字节右数第一个是1的位数
**************************************************************************** */
static inline osal_u8 oal_bit_find_first_bit_one_byte(osal_u8 byte)
{
    osal_u8 ret = 0;

    byte = byte & (-byte);

    while (byte != 1) {
        ret++;
        byte = (byte >> 1);

        if (ret > 7) { /* 7表示超过字节最大bit数。退出循环 */
            return ret;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : oal_set_mac_addr
 功能描述  : 将第二个mac地址 赋值给第一个mac地址
**************************************************************************** */
static inline osal_void oal_set_mac_addr(osal_u8 *addr1, const osal_u8 *addr2)
{
    const osal_u32 mac_len = 6; /* 6字节mac地址长度 */
    (osal_void)memcpy_s(addr1, mac_len, addr2, mac_len);
}

/*****************************************************************************
 函 数 名  : oal_compare_mac_addr
 功能描述  : 比较两个mac地址是否相等
**************************************************************************** */
static inline osal_u32 oal_compare_mac_addr(const osal_u8 *mac_addr1, const osal_u8 *mac_addr2)
{
    const osal_u32 mac_len = 6; /* 6字节mac地址长度 */
    return (osal_u32)(memcmp(mac_addr1, mac_addr2, mac_len));
}

/* ****************************************************************************
 函 数 名  : oal_byteorder_host_to_net_uint16
 功能描述  : 将16位本地字节序转换为网络字节序
**************************************************************************** */
static inline osal_u16 oal_byteorder_host_to_net_uint16(osal_u16 byte)
{
    byte = ((byte & 0x00FF) << 8) + ((byte & 0xFF00) >> 8); /* 0x00FF,0xFF00高低字节掩码，8高低字节移位 */
    return byte;
}

/* ****************************************************************************
 函 数 名  : oal_del_lut_index
 功能描述  : 在LUT index bitmap表中，删除一个LUT index (注:%可以作为优化项)
**************************************************************************** */
static inline osal_void oal_del_lut_index(osal_u8 *lut_index_table, osal_u8 idx)
{
    osal_u8 byte = idx / NUM_8_BITS;
    osal_u8 bit = idx % NUM_8_BITS;

    lut_index_table[byte] &= ~(osal_u8)(1 << bit); /* 1bit移位 */
}

static inline osal_s32 oal_memcmp(const osal_void *buf1, const osal_void *buf2, osal_u32 count)
{
    return memcmp(buf1, buf2, count);
}


osal_u8 oal_bit_find_first_bit_four_byte(osal_u32 byte);
osal_u8 oal_bit_find_first_zero_four_byte(osal_u32 byte);
osal_u8 oal_get_lut_index(osal_u8 *lut_index_table, osal_u8 bmap_len, osal_u16 max_lut_size,
    osal_u16 start, osal_u16 stop);

/* rssi平滑函数改为非内联函数,内联版本仅保证rom编译通过 */
osal_s8 oal_get_real_rssi(osal_s16 scaled_rssi);
osal_void oal_rssi_smooth(osal_s16 *old_rssi, osal_s8 new_rssi);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_util_common.h */
