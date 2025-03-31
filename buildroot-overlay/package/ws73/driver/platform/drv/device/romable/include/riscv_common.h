/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: RISC-V common header file
 */
#ifndef __RISCV_COMMON_H__
#define __RISCV_COMMON_H__

#define dsb() __asm__ __volatile__("fence":::"memory")

#endif
