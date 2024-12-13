#include <asm/asm.h>
#include <asm/io.h>
#include <asm/types.h>
#include <lmb.h>
#include <cpu_func.h>
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <image.h>
#include <gzip.h>
#include <asm/spl.h>
#include "sysctl.h"

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
// #include "sdk_autoconf.h"
// #include "k230_board_common.h"
#include <env_internal.h>
#include <linux/delay.h>
#include <dm.h>
#include "../common/k230_board_common.h"
int ddr_init_training(void)
{
	if( 0x00 != (readl((const volatile void __iomem *)0x980001bcULL) & 0x1 )) {
		//have init ,not need reinit;
		return 0;
	}

	board_ddr_init();

	return 0;
}

int board_late_init(void)
{
    env_set_ulong("mmc_boot_dev_num", g_bootmod - SYSCTL_BOOT_SDIO0);
    return 0;
}
