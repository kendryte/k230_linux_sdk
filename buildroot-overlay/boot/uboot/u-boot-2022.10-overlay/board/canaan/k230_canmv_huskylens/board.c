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
#include <platform.h>
#include "../common/k230_board_common.h"


#define K230_GET_BIT(val, bit) (((val) >> (bit)) & 1)
#define GPIO_EXT_PORTA 0x50

char k230_gpio0(){
        u32 reg_org;
        volatile u32 * gpio_value_in = (volatile int *)(GPIO_BASE_ADDR0+GPIO_EXT_PORTA);
        reg_org = *gpio_value_in;
    return K230_GET_BIT(reg_org, 0);
}


sysctl_boot_mode_e sysctl_boot_get_boot_mode(void)
{

	char value;
    value = k230_gpio0();
    if (value == 0)
          return SYSCTL_BOOT_SDIO0;

	return SYSCTL_BOOT_SDIO1;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    #define USB_IDPULLUP0 		(1<<4)
    #define USB_DMPULLDOWN0 	(1<<8)
    #define USB_DPPULLDOWN0 	(1<<9)


    char value;
    uint32_t SDIO_BASE_ADDR = SDIO1_BASE_ADDR;
    value = k230_gpio0();
    if (value == 0)
          SDIO_BASE_ADDR = SDIO0_BASE_ADDR;


    u32 usb_ctl3 = readl((const volatile void __iomem *)(SDIO_BASE_ADDR + 0x7c));

	usb_ctl3 |= USB_IDPULLUP0;
	usb_ctl3 &= ~(USB_DMPULLDOWN0 | USB_DPPULLDOWN0);
	writel(usb_ctl3, ( volatile void __iomem *)(SDIO_BASE_ADDR + 0x7c));


	usb_ctl3 = readl((const volatile void __iomem *)(SDIO_BASE_ADDR + 0x9c));
	usb_ctl3 |= USB_IDPULLUP0;
	usb_ctl3 |= (USB_DMPULLDOWN0 | USB_DPPULLDOWN0);
	writel(usb_ctl3, ( volatile void __iomem *)(SDIO_BASE_ADDR + 0x9c));


	env_set_ulong("mmc_boot_dev_num", g_bootmod - SYSCTL_BOOT_SDIO0);


    return 0;
}
#endif
