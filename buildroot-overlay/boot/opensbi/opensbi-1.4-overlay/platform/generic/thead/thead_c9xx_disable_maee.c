/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Authors:
 *   Yangyu Chen <cyy@cyyself.name>
 *
 */

#include <sbi/riscv_asm.h>
#include <thead/c9xx_encoding.h>

void thead_disable_maee(void)
{
	csr_clear(THEAD_C9XX_CSR_MXSTATUS, THEAED_C9XX_MXSTATUS_MAEE);
}