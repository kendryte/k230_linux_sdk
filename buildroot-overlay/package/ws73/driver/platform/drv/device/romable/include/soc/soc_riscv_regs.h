/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description : CPU Register Defines Headfile
 * Author: CompanyName
 * Create : 2021-04-20
*/
#ifndef _CPU_REGS_H
#define _CPU_REGS_H

#include <td_base.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define READ_CSR_REG(csrReg)                             \
    ({                                                   \
        td_u32 tmp_;                                     \
        asm volatile ("csrr %0, " #csrReg : "=r"(tmp_)); \
        tmp_;                                            \
    })

#define READ_CSR(csrReg) ({ td_u32 tmp_;             \
    asm volatile ("csrr %0, " #csrReg : "=r"(tmp_)); \
    tmp_;})                                          \

#define WRITE_CSR(csrReg, csrVal) do {                                     \
    if (__builtin_constant_p(csrVal) && (td_u32)(csrVal) < 32) {           \
        asm volatile ("csrw " #csrReg ", %0" :: "i"(csrVal));              \
    } else {                                                               \
        asm volatile ("csrw " #csrReg ", %0" :: "r"(csrVal));              \
    }                                                                      \
} while (0)

#define SET_CSR(csrReg, csrBit) ({ td_u32 tmp_;                                 \
    if (__builtin_constant_p(csrBit) && (td_u32)(csrBit) < 32) {                \
        asm volatile ("csrrs %0, " #csrReg ", %1" : "=r"(tmp_) : "i"(csrBit));  \
    } else {                                                                    \
        asm volatile ("csrrs %0, " #csrReg ", %1" : "=r"(tmp_) : "r"(csrBit));   \
    }                                                                           \
    tmp_;})

#define CLEAR_CSR(csrReg, csrBit) ({ td_u32 tmp_;                               \
    if (__builtin_constant_p(csrBit) && (td_u32)(csrBit) < 32) {                \
        asm volatile ("csrrc %0, " #csrReg ", %1" : "=r"(tmp_) : "i"(csrBit));  \
    } else {                                                                    \
        asm volatile ("csrrc %0, " #csrReg ", %1" : "=r"(tmp_) : "r"(csrBit));  \
    }                                                                           \
    tmp_; })

#define READ_CUSTOM_CSR(csrReg) ({ td_u32 tmp_;              \
    asm volatile ("csrr %0, %1" : "=r"(tmp_) : "i"(csrReg)); \
    tmp_; })

#define WRITE_CUSTOM_CSR_VAL(csrRegAddr, csrVal) do {    \
    if (__builtin_constant_p(csrVal))  {                 \
        asm volatile("li t0," "%0" : : "i"(csrVal));     \
    } else {                                             \
        asm volatile("mv t0," "%0" : : "r"(csrVal));     \
    }                                                    \
    asm volatile("csrw %0, t0" :: "i"(csrRegAddr));      \
} while (0)

#define SET_CUSTOM_CSR(csrRegAddr, csrBit) do {                  \
    if (__builtin_constant_p(csrBit) && (td_u32)(csrBit) < 32) { \
        asm volatile("li t0," "%0" : : "i"(csrBit));             \
    } else {                                                     \
        asm volatile("mv t0," "%0" : : "r"(csrBit));             \
    }                                                            \
    asm volatile("csrs %0, t0" :: "i"(csrRegAddr));              \
} while (0)

#define CLEAR_CUSTOM_CSR(csrRegAddr, csrBit) do {                \
    if (__builtin_constant_p(csrBit) && (td_u32)(csrBit) < 32) { \
        asm volatile("li t0," "%0" : : "i"(csrBit));             \
    } else {                                                     \
        asm volatile("mv t0," "%0" : : "r"(csrBit));             \
    }                                                            \
    asm volatile("csrc %0, t0" :: "i"(csrRegAddr));              \
} while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _CPU_REGS_H */
