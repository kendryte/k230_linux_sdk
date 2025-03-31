/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准复数运算相关
 * Create: 2022-09-15
 */

#ifndef __CALI_COMPLEX_NUM_H__
#define __CALI_COMPLEX_NUM_H__

#include "cali_outer_os_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define CALI_CORDIC_ANGLE_ERROR_CODE 0xFFFF
typedef struct {
    osal_s32 real;
    osal_s32 imag;
} cali_complex_stru;

cali_complex_stru cali_complex_minus(cali_complex_stru a, cali_complex_stru b);
osal_s32 cali_div_round_closest(osal_s32 a, osal_s32 b);
cali_complex_stru cali_complex_negate(cali_complex_stru a);
cali_complex_stru cali_complex_add(cali_complex_stru a, cali_complex_stru b);
cali_complex_stru cali_complex_conj(cali_complex_stru a);
cali_complex_stru cali_complex_multiply(cali_complex_stru a, cali_complex_stru b);
cali_complex_stru cali_complex_divide(cali_complex_stru complex_num, cali_complex_stru complex_denom);
cali_complex_stru cali_complex_div_with_fix_bit(cali_complex_stru complex_num,
    cali_complex_stru complex_denom, osal_u8 fix_bit);
cali_complex_stru cali_complex_multiply_minus(cali_complex_stru a, cali_complex_stru b,
    cali_complex_stru c, cali_complex_stru d);
osal_void cali_print_complex_data(char *name, cali_complex_stru value);
osal_s32 cali_right_shift_round_closest(osal_s32 num, osal_u8 bits);
cali_complex_stru cali_complex_shift_left(cali_complex_stru complex_num, osal_u8 bits);
cali_complex_stru cali_complex_shift_right(cali_complex_stru complex_num, osal_u8 bits);
osal_s32 cali_div_floor(osal_s32 a, osal_s32 b);
osal_s32 cali_cordic_angle(osal_s32 data_i_in, osal_s32 data_q_in, osal_s32 width);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
