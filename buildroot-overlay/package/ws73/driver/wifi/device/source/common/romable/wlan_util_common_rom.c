/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: oal util api.
 * Create: 2020-05-02
 */

#include "osal_types.h"
#include "wlan_util_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* ****************************************************************************
 函 数 名  : oal_bit_find_first_bit_four_byte
 功能描述  : 找到1字节右数第一个是1的位数
**************************************************************************** */
osal_u8 oal_bit_find_first_bit_four_byte(osal_u32 byte)
{
    osal_u8 ret = 0;

    if (0 == byte) {
        return ret;
    }

    if (!(byte & 0xffff)) {
        byte >>= 16;  /* 低16bit全0 */
        ret += 16;    /* 查找高16bit */
    }

    if (!(byte & 0xff)) {
        byte >>= 8; /* 低8bit全0 */
        ret += 8;   /* 查找高8bit */
    }

    if (!(byte & 0xf)) {
        byte >>= 4;  /* 低4bit全0 */
        ret += 4;    /* 查找高4bit */
    }

    if (!(byte & 3)) { /* 3表示低2bit全0 */
        byte >>= 2;  /* 低2bit全0 */
        ret += 2;    /* 查找高2bit */
    }

    if (!(byte & 1)) { /* 3表示低1bit全0 */
        byte >>= 1;  /* 低1bit全0 */
        ret += 1;    /* 查找高1bit */
    }

    return ret;
}

/* ****************************************************************************
 功能描述  : 找到4字节右数第一个是0的位数
**************************************************************************** */
osal_u8 oal_bit_find_first_zero_four_byte(osal_u32 byte)
{
    osal_u8 ret = 0;

    byte = ~byte;

    if (!(byte & 0xffff)) {
        byte >>= 16; /* 低16bit全1 */
        ret += 16;   /* 查找高16bit */
    }

    if (!(byte & 0xff)) {
        byte >>= 8;  /* 低8bit全1 */
        ret += 8;    /* 查找高8bit */
    }

    if (!(byte & 0xf)) {
        byte >>= 4;  /* 低4bit全1 */
        ret += 4;    /* 查找高4bit */
    }

    if (!(byte & 0x3)) { /* 3表示低2bit全0 */
        byte >>= 2;  /* 低2bit全1 */
        ret += 2;    /* 查找高2bit */
    }

    if (!(byte & 1)) { /* 3表示低1bit全0 */
        ret += 1; /* 查找高1bit */
    }

    return ret;
}

/* ****************************************************************************
 函 数 名  : oal_get_lut_index
 功能描述  : 从LUT index bitmap表中，获取一个没有被使用的索引，没有找到的话，
             返回不可用的索引标识(非关键路径，未进行优化，有时间可以优化)
**************************************************************************** */
osal_u8 oal_get_lut_index(osal_u8 *lut_index_table, osal_u8 bmap_len, osal_u16 max_lut_size,
    osal_u16 start, osal_u16 stop)
{
    osal_u8 byte = 0;
    osal_u8 bit = 0;
    osal_u8 temp = 0;
    osal_u16 index = 0;

    for (byte = 0; byte < bmap_len; byte++) {
        temp = lut_index_table[byte];
        /* 1个byte 8个bit */
        for (bit = 0; bit < 8; bit++) {
            if (0x0 == (temp & (1 << bit))) {
                /* 1个byte 8个bit */
                index = (byte * 8 + bit);
                if ((index < start) || (index >= stop)) {
                    continue;
                }

                if (index < max_lut_size) {
                    lut_index_table[byte] |= (osal_u8)(1 << bit);

                    return (osal_u8)index;
                } else {
                    return (osal_u8)max_lut_size;
                }
            }
        }
    }
    return (osal_u8)max_lut_size;
}

osal_void oal_rssi_smooth(osal_s16 *old_rssi, osal_s8 new_rssi)
{
    osal_s16 s_delta;

    /* 若上报的值超过了合法范围，则不进行平滑操作，函数直接返回 */
    if (new_rssi < OAL_RSSI_SIGNAL_MIN || new_rssi > OAL_RSSI_SIGNAL_MAX) {
        return;
    }

    /* 若上报的值为0，则是描述符未填rssi值,不进行平滑 */
    if (new_rssi == 0) {
        return;
    }

    /* 判断为初始值,芯片上报的rssi作为平滑结果 */
    if (*old_rssi == OAL_RSSI_INIT_MARKER) {
        *old_rssi = (osal_s16)new_rssi << OAL_RSSI_FRACTION_BITS;
    }

    /* old_rssi四舍五入后计算delta */
    s_delta = (osal_s16)new_rssi - oal_get_real_rssi(*old_rssi);
    if (s_delta > OAL_RSSI_MAX_DELTA) {
        s_delta = OAL_RSSI_MAX_DELTA;
    }
    if (s_delta < -OAL_RSSI_MAX_DELTA) {
        s_delta = -OAL_RSSI_MAX_DELTA;
    }
    *old_rssi += s_delta;
}

osal_s8 oal_get_real_rssi(osal_s16 s_scaled_rssi)
{
    /* 四舍五入 */
    return (osal_s8)oal_round(s_scaled_rssi, OAL_RSSI_FRACTION_BITS);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

/* end of wlan_util_common_rom.c */