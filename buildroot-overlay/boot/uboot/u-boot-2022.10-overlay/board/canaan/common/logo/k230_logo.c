#include <stdio.h>
#include <common.h>
#include <command.h>
#include <sysctl.h>
#include "linux/delay.h"
#include "display_logo.h"
#include "image.h"
#include "cpu_func.h"
typedef struct
{
    volatile uint32_t cpu0_pwr_tim;         /* 0x00 */
    volatile uint32_t cpu0_lpi_tim;         /* 0x04 */
    volatile uint32_t cpu0_pwr_lpi_ctl;     /* 0x08 */
    volatile uint32_t cpu0_pwr_lpi_state;   /* 0x0c */

    volatile uint32_t cpu1_pwr_tim;         /* 0x10 */
    volatile uint32_t cpu1_lpi_tim;         /* 0x14 */
    volatile uint32_t cpu1_pwr_lpi_ctl;     /* 0x18 */
    volatile uint32_t cpu1_pwr_lpi_state;   /* 0x1c */

    volatile uint32_t ai_pwr_tim;           /* 0x20 */
    volatile uint32_t ai_lpi_tim;           /* 0x24 */
    volatile uint32_t ai_pwr_lpi_ctl;       /* 0x28 */
    volatile uint32_t ai_pwr_lpi_state;     /* 0x2c */

    volatile uint32_t disp_pwr_tim;         /* 0x30 */
    volatile uint32_t disp_lpi_tim;         /* 0x34 */
    volatile uint32_t disp_gpu_tim;         /* 0x38 */
    volatile uint32_t disp_lpi_ctl;         /* 0x3c */
    volatile uint32_t disp_lpi_state;       /* 0x40 */
    volatile uint32_t disp_reserved[7];     /* 0x44-0x4c, 0x50-0x5c */

    volatile uint32_t shrm_pwr_tim;         /* 0x60 */
    volatile uint32_t shrm_lpi_tim;         /* 0x64 */
    volatile uint32_t shrm_pwr_lpi_ctl;     /* 0x68 */
    volatile uint32_t shrm_pwr_lpi_state;   /* 0x6c */

    volatile uint32_t vpu_pwr_tim;          /* 0x70 */
    volatile uint32_t vpu_lpi_tim;          /* 0x74 */
    volatile uint32_t vpu_qch_tim;          /* 0x78 */
    volatile uint32_t vpu_pwr_lpi_ctl;      /* 0x7c */
    volatile uint32_t vpu_lpi_state;        /* 0x80 */
    volatile uint32_t vpu_reserved[3];      /* 0x84-0x8c */

    volatile uint32_t mctl_pwr_tim0;        /* 0x90 */
    volatile uint32_t mctl_noc_lpi_tim;     /* 0x94 */
    volatile uint32_t mctl_axi_lpi_tim;     /* 0x98 */
    volatile uint32_t mctl_pwr_lpi_ctl;     /* 0x9c */
    volatile uint32_t mctl_clock_switch;    /* 0xa0 */
    volatile uint32_t mctl_lpi_state;       /* 0xa4 */
    volatile uint32_t mctl_reserved[22];    /* 0xa8-0xac, 0xb0-0xbc, 0xc0-0xcc, 0xd0-0xdc, 0xe0-0xec, 0xf0-0xfc */

    volatile uint32_t dpu_pwr_tim;          /* 0x100 */
    volatile uint32_t dpu_lpi_tim;          /* 0x104 */
    volatile uint32_t dpu_pwr_lpi_ctl;      /* 0x108 */
    volatile uint32_t dpu_pwr_lpi_state;    /* 0x10c */

    volatile uint32_t hi_pwr_tim;           /* 0x110 */
    volatile uint32_t hi_lpi_tim;           /* 0x114 */
    volatile uint32_t hi_pwr_lpi_ctl;       /* 0x118 */
    volatile uint32_t hi_lpi_state;         /* 0x11c */

    volatile uint32_t ls_pwr_tim;           /* 0x120 */
    volatile uint32_t ls_lpi_tim;           /* 0x124 */
    volatile uint32_t ls_pwr_lpi_ctl;       /* 0x128 */
    volatile uint32_t ls_lpi_state;         /* 0x12c */

    volatile uint32_t sec_pwr_tim;          /* 0x130 */
    volatile uint32_t sec_lpi_tim;          /* 0x134 */
    volatile uint32_t sec_pwr_lpi_ctl;      /* 0x138 */
    volatile uint32_t sec_pwr_lpi_state;    /* 0x13c */

    volatile uint32_t isp_pwr_tim;          /* 0x140 */
    volatile uint32_t isp_lpi_tim;          /* 0x144 */
    volatile uint32_t isp_pwr_lpi_ctl;      /* 0x148 */
    volatile uint32_t isp_pwr_lpi_state;    /* 0x14c */

    volatile uint32_t pmu_pwr_tim;          /* 0x150 */
    volatile uint32_t pmu_lpi_tim;          /* 0x154 */
    volatile uint32_t pmu_pwr_lpi_ctl;      /* 0x158 */
    volatile uint32_t pmu_pwr_lpi_state;    /* 0x15c */

    volatile uint32_t repair_status;        /* 0x160 */
    volatile uint32_t sram0_repair_tim;     /* 0x164 */
    volatile uint32_t ssys_ctl_gpio_ctl;    /* 0x168 */
    volatile uint32_t ssys_reserved;        /* 0x16c */
    volatile uint32_t ssys_ctl_gpio_en0;    /* 0x170 */
    volatile uint32_t ssys_ctl_gpio_en1;    /* 0x174 */

    volatile uint32_t cpu_repair_tim;       /* 0x178 */
} sysctl_pwr_s;
typedef enum
{
    SYSCTL_PD_CPU1,
    SYSCTL_PD_AI,
    SYSCTL_PD_DISP,
    SYSCTL_PD_VPU,
    SYSCTL_PD_DPU,
    SYSCTL_PD_MAX,
} sysctl_pwr_domain_e;
bool sysctl_pwr_set_power(sysctl_pwr_domain_e powerdomain, bool enable);


volatile sysctl_pwr_s* sysctl_pwr = (volatile sysctl_pwr_s*)SYSCTL_PWR_BASE_ADDR;
/*****************************************************************************************
*                                POWER DOMAIN REPAIR
*   powerdomain: power domain
*****************************************************************************************/

/* powerup的时候，设置电源域repair */
bool sysctl_pwr_set_repair_enable(sysctl_pwr_domain_e powerdomain)
{
    switch(powerdomain)
    {
        case SYSCTL_PD_AI:
            sysctl_pwr->ai_pwr_lpi_ctl |= (1 << 4) | (1 << 20);
            return true;
        default:
            return false;
    }
}


/*****************************************************************************************
*                                POWER DOMAIN ON OR OFF
*   powerdomain: power domain
*   enable: true for powerup, false for poweroff.
*****************************************************************************************/

bool sysctl_pwr_set_pwr_reg(volatile uint32_t *regctl, volatile uint32_t *regsta, bool enable)
{
    /* enable==true, power on; enable==false, power off */
    *regctl |= (true == enable) ? ((1 << 1) | (1 << 17)) : ((1 << 0) | (1 << 16));

    udelay(500);

    /* power enable state */
    if(true == enable)
        return (*regsta & (1 << 1)) ? true:false;
    else
        return (*regsta & (1 << 0)) ? true:false;
}

bool sysctl_pwr_set_power(sysctl_pwr_domain_e powerdomain, bool enable)
{
    volatile uint32_t *pwr_ctl_reg = NULL;
    volatile uint32_t *pwr_sta_reg = NULL;

    switch(powerdomain)
    {
        case SYSCTL_PD_CPU1:
        {
            pwr_ctl_reg = (volatile uint32_t *)&sysctl_pwr->cpu1_pwr_lpi_ctl;
            pwr_sta_reg = (volatile uint32_t *)&sysctl_pwr->cpu1_pwr_lpi_state;
            break;
        }
        case SYSCTL_PD_AI:
        {
            pwr_ctl_reg = (volatile uint32_t *)&sysctl_pwr->ai_pwr_lpi_ctl;
            pwr_sta_reg = (volatile uint32_t *)&sysctl_pwr->ai_pwr_lpi_state;
            break;
        }
        case SYSCTL_PD_DISP:
        {
            pwr_ctl_reg = (volatile uint32_t *)&sysctl_pwr->disp_lpi_ctl;
            pwr_sta_reg = (volatile uint32_t *)&sysctl_pwr->disp_lpi_state;
            break;
        }
        case SYSCTL_PD_VPU:
        {
            pwr_ctl_reg = (volatile uint32_t *)&sysctl_pwr->vpu_pwr_lpi_ctl;
            pwr_sta_reg = (volatile uint32_t *)&sysctl_pwr->vpu_lpi_state;
            break;
        }
        case SYSCTL_PD_DPU:
        {
            pwr_ctl_reg = (volatile uint32_t *)&sysctl_pwr->dpu_pwr_lpi_ctl;
            pwr_sta_reg = (volatile uint32_t *)&sysctl_pwr->dpu_pwr_lpi_state;
            break;
        }
        default:
            return false;
    }

    /* repair powerdomain */
    /* only powerup need repair */
    if(true == enable)
    {
        (void)sysctl_pwr_set_repair_enable(powerdomain);
    }

    return sysctl_pwr_set_pwr_reg(pwr_ctl_reg, pwr_sta_reg, enable);
}
#define BACKGROUND_BLACK_COLOR                            (0x808000)
#define BACKGROUND_PINK_COLOR                             (0xffffff)
unsigned long k230_display_logo_get_pic_mem_addr(void)
{
    return env_get_bootm_size(); //预留1M 给logo；
}
int _k230_display_logo_load_pic()
{
    char cmd[128];
    unsigned long add = k230_display_logo_get_pic_mem_addr();
    unsigned long size = 0x100000;

    sprintf(cmd, "ext4load mmc ${mmc_boot_dev_num}:1 0x1000000 /logo.yuv ");
    printf("cmd=%s\n", cmd);
    run_command(cmd, 0);

    size = env_get_ulong("filesize", 16,0x100000);
    memcpy((void*)add, (void*)0x1000000, size);
    flush_cache(add, ROUND(size, CONFIG_SYS_CACHELINE_SIZE));//
    return 0;
}
int k230_display_logo(void)
{
    #ifdef CONFIG_K230_BARE_DISP_LOGO_DF
     k_connector_info info = {
        0,
        BACKGROUND_PINK_COLOR,
        10,
        23,
        1,
        K_DSI_2LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 3, 97, 0x27, 0xa5 }, // 0x96
        { 24750, 297000, 550, 480,
                    10, 10, 50, 1500,
                     640, 10, 250, 600 },
    };
    #else
    k_connector_info info = {
        0,
        BACKGROUND_PINK_COLOR,
        10,
        14,
        1,
        K_DSI_2LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 9, 196, 0x17, 0xa3 }, // 0x96
        { 39600, 475200, 600, 480, 20, 20, 80, 1100, 800, 10, 70, 220 },
    };
    #endif

    _k230_display_logo_load_pic();

    sysctl_pwr_set_power(SYSCTL_PD_DISP, true);
    st7701_init(&info);
    return 0;
}

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
    k230_display_logo();
    return 0;
}
#endif
static int do_k230_logo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    k230_display_logo();
    return 0;
}

U_BOOT_CMD(
	k230_logo, CONFIG_SYS_MAXARGS, 0, do_k230_logo,
	"k230 logo",
	"k230 logo"
);

/*
usb start;dhcp;
tftp 0x1000000 10.10.1.94:wjx/disney_800x480_nv12.yuv ;
tftp 0x100000 10.10.1.94:wjx/display_logo.bin;cp.b 0x100000 0x80200000 0x100000;
boot_baremetal 0 0x80200000 0x100000



usb start;dhcp; tftp 0x100000 10.10.1.94:wjx/u-boot.bin;cp.b 0x100000 0 0x100000;
go 0;
usb start;dhcp;  tftp 0x30000000 10.10.1.94:wjx/disney_800x480_nv12.yuv ; cp.b 0x30000000   0x3ff00000 0x100000;k230_logo;
k230_logo;
uboot下更新vmlinux;更新设备树：

boot_baremetal 0 0x80200000 0x100000
*/
