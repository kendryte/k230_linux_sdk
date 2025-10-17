# /* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
#  *
#  * Redistribution and use in source and binary forms, with or without
#  * modification, are permitted provided that the following conditions are met:
#  * 1. Redistributions of source code must retain the above copyright
#  * notice, this list of conditions and the following disclaimer.
#  * 2. Redistributions in binary form must reproduce the above copyright
#  * notice, this list of conditions and the following disclaimer in the
#  * documentation and/or other materials provided with the distribution.
#  *
#  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#  * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#  * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
#  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  */

obj-y += board.o



obj-$(CONFIG_CANMV_GT6700_LPDDR4_2667) += gen_canmvv3_lpddr4_2667.o
obj-$(CONFIG_CANMV_GT6700_LPDDR4_3200) += gen_canmvv3_lpddr4_3200.o
obj-$(CONFIG_CANMV_GT6700_LPDDR4_1600_WITHOUT_ODT) += gen_lpddr4_init_32_swap_1600_without_odt.o

$(obj)/gen_canmvv3_lpddr4_2667.c: $(src)/../k230_canmv_01studio/lpddr4_init_32_swap_2667.c    $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  || exit 1)

$(obj)/gen_canmvv3_lpddr4_3200.c: $(src)/../k230_canmv_01studio/lpddr4_init_32_swap_3200.c  $(srctree)/arch/riscv/cpu/k230/ddr_2d.sh
	($(srctree)/arch/riscv/cpu/k230/ddr_2d.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  0x542bd || exit 1)

$(obj)/gen_lpddr4_init_32_swap_1600_without_odt.c: $(src)/../k230_canmv_01studio/lpddr4_init_32_swap_1600_without_odt.c    $(srctree)/arch/riscv/cpu/k230/ddr.sh
	($(srctree)/arch/riscv/cpu/k230/ddr.sh $< $@ 0x50000 0x53fff 0x54000 0x5433d  || exit 1)


EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_common
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_crypto
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_dma
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ka
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_rt
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecc
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_hmac
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_ecp
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sm2
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sp38a
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/cpu/k230/puf_fw/pufs_sp38d
