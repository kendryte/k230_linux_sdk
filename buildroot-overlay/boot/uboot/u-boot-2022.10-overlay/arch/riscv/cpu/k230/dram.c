/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;




void ddr_init_board(void);
__weak void ddr_init_board(void)
{
  printf("skip ddr init \n");
}

__weak int ddr_init_training(void)
{
	#if defined(CONFIG_TARGET_K230_FPGA)
		return 0; //fpga not need init;
	#endif

	if( (readl((const volatile void __iomem *)0x980001bcULL) & 0x1 ) != 0 ){
		return 0; //have init ,not need reinit;
	}

  ddr_init_board();
	return 0;
}

u32 detect_ddr_size(void)
{
    u64 ddr_size = 0;
    u64 readv0,readv1 = 0;
    ulong ddr_detect_pattern[]={0x1234567887654321, 0x1122334455667788};


    gd->ram_base = 0;
    gd->ram_size  = 0x8000000;

    writeq(ddr_detect_pattern[0], (void __iomem *)0);
    writeq(ddr_detect_pattern[0], (void __iomem *)0+8);
    flush_dcache_range(0, 0+CONFIG_SYS_CACHELINE_SIZE);

    for(ddr_size = 128*1024*1024; ddr_size <= (u64)(1*1024*1024*1024); ddr_size=ddr_size<<1 ){
        invalidate_dcache_range(ddr_size, ddr_size + CONFIG_SYS_CACHELINE_SIZE);
        readv0 = readq((void __iomem *)ddr_size);

        if(readv0 == ddr_detect_pattern[0]){//再次确认下
            writeq(ddr_detect_pattern[1], (void __iomem *)ddr_size+8);
            flush_dcache_range(ddr_size, ddr_size+CONFIG_SYS_CACHELINE_SIZE);
            invalidate_dcache_range(0, 0 + CONFIG_SYS_CACHELINE_SIZE);

            readv1 = readq((void __iomem *)0+8);
            if(readv1 == ddr_detect_pattern[1]){
                //printf("get ddr size=%lx\n", ddr_size);
                break;
            }
        }
        //printf("ddr size=%lx %lx,%lx \n", ddr_size,readv0,readv1);
    }
    gd->ram_size = ddr_size;
    //printf("ddr detect error %x\n\n", ddr_size);
    return ddr_size;
}


int dram_init(void)
{
    ddr_init_training();
    detect_ddr_size();
    return 0;
}


ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_64BIT
	/*
	 * Ensure that we run from first 4GB so that all
	 * addresses used by U-Boot are 32bit addresses.
	 *
	 * This in-turn ensures that 32bit DMA capable
	 * devices work fine because DMA mapping APIs will
	 * provide 32bit DMA addresses only.
	 */
	if (gd->ram_top > SZ_4G)
		return SZ_4G;
#endif
	return gd->ram_top;
}
