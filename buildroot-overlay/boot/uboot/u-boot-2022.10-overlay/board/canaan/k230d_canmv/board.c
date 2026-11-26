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
#include "../common/k230_board_common.h"
#include <env_internal.h>
#include <linux/delay.h>
#include <mmc.h>
// sysctl_boot_mode_e sysctl_boot_get_boot_mode(void)
// {
// 	return SYSCTL_BOOT_SDIO1;
// }

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    wifi_sdio0_rst();
    return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
/*
 * bootcmd="run bnuttx;run blinuxilp32" loads nuttx at 0x7000000 and runs it
 * alongside linux, so linux must not be told it owns the top 16MB nuttx uses.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
    phys_addr_t base = env_get_bootm_low();
    phys_size_t size = env_get_bootm_size();
    const char *bootcmd = env_get("bootcmd");

    //printf("xwjx ft_board_setup: base=0x%llx size=0x%llx bootcmd=%s\n", (unsigned long long)base, (unsigned long long)size, bootcmd);

    if (bootcmd && strstr(bootcmd, "bnuttx")){
        size -= 0x1000000;
        fdt_status_disabled(blob, fdt_path_offset(blob, "/soc/serial@91402000"));
    }

    fdt_fixup_memory(blob, (u64)base, (u64)size);
    return 0;
}
#endif

// char *fdt_chosen_bootargs(void)
// {
//     char *bootargs = "root=/dev/mmcblk0p2 loglevel=8 rw rootdelay=4 rootfstype=ext4 console=ttyS0,115200  earlycon=sbi";
// 	return bootargs;
// }
