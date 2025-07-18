#include <stdio.h>
//#include "utils.h"
#include <stdlib.h>
#include <string.h>
// #include <sleep.h>
// #include "dwc_csi.h"
// #include "video.h"
// #include <interrupt.h>
// #include <uart.h>
// #include "csi_rv64_gcc.h"
// #include "verify_common.h"
// #include "core_rv64.h"
// #include "kendryte_log.h"

// #include "kendryte_log.h"
// #include "gpio.h"
// #include "io.h"
// #include "muxpin.h"

#include "display_logo.h"
#include "sysctl.h"
#include "../k230_board_common.h"
#include <linux/delay.h>
#include "common.h"

#define msleep mdelay
static void st7701_hardware_init(void)
{

	#define GPIO_RST_PIN			24
	#define GPIO_LCD_BACKLIGHT_EN   25
    char high = 1,low = 0;

    // muxpin_config_t config = { //0x388
    //     0x0, //st
    //     0x4, //ds
    //     0x0, //pd
    //     0x0, //pu
    //     0x1, //oe			// 1 output
    //     0x0, //ie			// 1 input
    //     0x1, //msc
    //     0x0, //sl
    //     0x0 //io_sel
    // };

	// bit only didplsy
    // rst
    //muxpin_set_config(GPIO_RST_PIN, config);
    // gpio_set_drive_mode(GPIO_RST_PIN, GPIO_DM_OUTPUT);
    // gpio_set_pin(GPIO_RST_PIN, GPIO_PV_HIGH);  //GPIO_PV_LOW  GPIO_PV_HIGH

    k230_gpio('o', GPIO_RST_PIN , &high);

    msleep(200);
    k230_gpio('s', GPIO_RST_PIN, &low);
    //gpio_set_pin(GPIO_RST_PIN, GPIO_PV_LOW);  //GPIO_PV_LOW  GPIO_PV_HIGH

    msleep(200);
    k230_gpio('s', GPIO_RST_PIN, &high);
   // gpio_set_pin(GPIO_RST_PIN, GPIO_PV_HIGH);  //GPIO_PV_LOW  GPIO_PV_HIGH
    msleep(200);

	// backlight
	//muxpin_set_config(GPIO_LCD_BACKLIGHT_EN, config);
    // gpio_set_drive_mode(GPIO_LCD_BACKLIGHT_EN, GPIO_DM_OUTPUT);
    // gpio_set_pin(GPIO_LCD_BACKLIGHT_EN, GPIO_PV_HIGH);  //GPIO_PV_LOW  GPIO_PV_HIGH
    k230_gpio('o', GPIO_LCD_BACKLIGHT_EN , &high);

}


void st7701_480x640_init(void)
{

	uint8_t param1[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13};
	uint8_t param2[] = { 0xef, 0x08};
	uint8_t param3[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x10};
	uint8_t param4[] = { 0xb0, 0x00, 0x0b, 0x13, 0x0d, 0x10, 0x07, 0x02, 0x08, 0x07, 0x1f, 0x04, 0x11, 0x0f, 0x28, 0x2f, 0x1f};
	uint8_t param5[] = { 0xb1, 0x00, 0x0c, 0x13, 0x0c, 0x10, 0x05, 0x02, 0x08, 0x08, 0x1e, 0x05, 0x13, 0x11, 0x27, 0x30, 0x1f};
	uint8_t param6[] = { 0xc0, 0x4F, 0x00};
	uint8_t param7[] = { 0xc1, 0x0E, 0x02};
	uint8_t param8[] = { 0xc2, 0x01, 0x14};
	uint8_t param9[] = { 0xcc, 0x10};
	uint8_t param10[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x11};
	uint8_t param11[] = { 0xb0, 0x45};
	uint8_t param12[] = { 0xb1, 0x13};
	uint8_t param13[] = { 0xb2, 0x07};
	uint8_t param14[] = { 0xb3, 0x80};
	uint8_t param15[] = { 0xb5, 0x47};
	uint8_t param16[] = { 0xb7, 0x85};
	uint8_t param17[] = { 0xb8, 0x10};
	uint8_t param18[] = { 0xc1, 0x78};
	uint8_t param19[] = { 0xc2, 0x78};
	uint8_t param20[] = { 0xd0, 0x88};
	uint8_t param21[] = { 0xee, 0x42};
	uint8_t param22[] = { 0xe0, 0x00, 0x00, 0x02};
	uint8_t param23[] = { 0xe1, 0x04, 0xb0, 0x06, 0xb0, 0x05, 0xb0, 0x07, 0xb0, 0x00, 0x44, 0x44};
	uint8_t param24[] = { 0xe2, 0x20, 0x20, 0x44, 0x44, 0x96, 0xa0, 0x00, 0x00, 0x96, 0xa0, 0x00, 0x00};
	uint8_t param25[] = { 0xe3, 0x00, 0x00, 0x22, 0x22};
	uint8_t param26[] = { 0xe4, 0x44, 0x44};
	uint8_t param27[] = { 0xe5, 0x0c, 0x90, 0xb0, 0xa0, 0x0e, 0x92, 0xb0, 0xa0, 0x08, 0x8c, 0xb0, 0xa0, 0x0a, 0x8e, 0xb0, 0xa0};
	uint8_t param28[] = { 0xe6, 0x00, 0x00, 0x22, 0x22};
	uint8_t param29[] = { 0xe7, 0x44, 0x44};
	uint8_t param30[] = { 0xe8, 0x0d, 0x91, 0xb0, 0xa0, 0x0f, 0x93, 0xb0, 0xa0, 0x09, 0x8d, 0xb0, 0xa0, 0x0b, 0x8f, 0xb0, 0xa0};
	uint8_t param31[] = { 0xe9, 0x36, 0x00};
	uint8_t param32[] = { 0xeb, 0x00, 0x00, 0xe4, 0xe4, 0x44, 0x88, 0x40};
	uint8_t param33[] = { 0xed, 0xc1, 0xa2, 0xbf, 0x0f, 0x67, 0x45, 0xff, 0xff, 0xff, 0xff, 0x54, 0x76, 0xf0, 0xfb, 0x2a, 0x1c};
	uint8_t param34[] = { 0xef, 0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f};
	uint8_t param35[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13};
	uint8_t param36[] = { 0xe8, 0x00, 0x0e};
	uint8_t param37[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x00};
	uint8_t param38[] = { 0x11};
	uint8_t param39[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13};
	uint8_t param40[] = { 0xe8, 0x00, 0x0c};
	uint8_t param41[] = { 0xe8, 0x00, 0x00,};
	uint8_t param42[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x00};
	uint8_t param43[] = { 0x11};
	uint8_t param44[] = { 0x29};

	//uint8_t val = 0x04;
//	dwc_lpdt_read_pkg(&val, 1);

    kd_dwc_lpdt_send_pkg(param1, sizeof(param1));

	kd_dwc_lpdt_send_pkg(param2, sizeof(param2));

	kd_dwc_lpdt_send_pkg(param3, sizeof(param3));

	kd_dwc_lpdt_send_pkg(param4, sizeof(param4));

	kd_dwc_lpdt_send_pkg(param5, sizeof(param5));

	kd_dwc_lpdt_send_pkg(param6, sizeof(param6));

	kd_dwc_lpdt_send_pkg(param7, sizeof(param7));

	kd_dwc_lpdt_send_pkg(param8, sizeof(param8));

	kd_dwc_lpdt_send_pkg(param9, sizeof(param9));

	kd_dwc_lpdt_send_pkg(param10, sizeof(param10));

	kd_dwc_lpdt_send_pkg(param11, sizeof(param11));

	kd_dwc_lpdt_send_pkg(param12, sizeof(param12));

	kd_dwc_lpdt_send_pkg(param13, sizeof(param13));

	kd_dwc_lpdt_send_pkg(param14, sizeof(param14));

	kd_dwc_lpdt_send_pkg(param15, sizeof(param15));

	kd_dwc_lpdt_send_pkg(param16, sizeof(param16));

	kd_dwc_lpdt_send_pkg(param17, sizeof(param17));

	kd_dwc_lpdt_send_pkg(param18, sizeof(param18));

	kd_dwc_lpdt_send_pkg(param19, sizeof(param19));

	kd_dwc_lpdt_send_pkg(param20, sizeof(param20));

	kd_dwc_lpdt_send_pkg(param21, sizeof(param21));

	kd_dwc_lpdt_send_pkg(param22, sizeof(param22));

	kd_dwc_lpdt_send_pkg(param23, sizeof(param23));

	kd_dwc_lpdt_send_pkg(param24, sizeof(param24));

	kd_dwc_lpdt_send_pkg(param25, sizeof(param25));

	kd_dwc_lpdt_send_pkg(param26, sizeof(param26));

	kd_dwc_lpdt_send_pkg(param27, sizeof(param27));

	kd_dwc_lpdt_send_pkg(param28, sizeof(param28));

	kd_dwc_lpdt_send_pkg(param29, sizeof(param29));

	kd_dwc_lpdt_send_pkg(param30, sizeof(param30));

	kd_dwc_lpdt_send_pkg(param31, sizeof(param31));

	kd_dwc_lpdt_send_pkg(param32, sizeof(param32));

	kd_dwc_lpdt_send_pkg(param33, sizeof(param33));

	kd_dwc_lpdt_send_pkg(param34, sizeof(param34));

	kd_dwc_lpdt_send_pkg(param35, sizeof(param35));

	kd_dwc_lpdt_send_pkg(param36, sizeof(param36));

	kd_dwc_lpdt_send_pkg(param37, sizeof(param37));

	kd_dwc_lpdt_send_pkg(param38, sizeof(param38));
	msleep(100);
	kd_dwc_lpdt_send_pkg(param39, sizeof(param39));

	kd_dwc_lpdt_send_pkg(param40, sizeof(param40));

	kd_dwc_lpdt_send_pkg(param41, sizeof(param41));
	msleep(100);
	kd_dwc_lpdt_send_pkg(param42, sizeof(param42));

	kd_dwc_lpdt_send_pkg(param43, sizeof(param43));

	kd_dwc_lpdt_send_pkg(param44, sizeof(param44));

}


static void st7701_480x800_init(void)
{
    // uint8_t param1[] = {0x11, 0x00};
    uint8_t param2[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    uint8_t param3[] = {0xEF, 0x08};
    uint8_t param4[] = {0xFF, 0x77,0x01,0x00,0x00,0x10};
    uint8_t param5[] = {0xC0, 0x63,0x00};      // (99 + 1) * 8  = 800
    uint8_t param6[] = {0xC1, 0x10, 0x02};     // vbp  = 0x10 = 16   vfp = 0x02
    uint8_t param7[] = {0xC2, 0x31, 0x02};     // pclk = 512 + 12 = 614
    uint8_t param8[] = {0xCC, 0x10};
    uint8_t param9[] = {0xB0, 0xC0, 0x0C, 0x92, 0x0C, 0x10, 0x05, 0x02, 0x0D, 0x07, 0x21, 0x04, 0x53, 0x11, 0x6A, 0x32, 0x1F};
    uint8_t param10[] = {0xB1, 0xC0, 0x87, 0xCF, 0x0C, 0x10, 0x06, 0x00, 0x03, 0x08, 0x1D, 0x06, 0x54, 0x12, 0xE6, 0xEC, 0x0F};
    uint8_t param11[] = {0xFF, 0x77,0x01,0x00,0x00,0x11};
    uint8_t param12[] = {0xB0, 0x5D};
    uint8_t param13[] = {0xB1, 0x62};
    uint8_t param14[] = {0xB2, 0x82};
    uint8_t param15[] = {0xB3, 0x80}; // 0x05
    uint8_t param16[] = {0xB5, 0x42};
    uint8_t param17[] = {0xB7, 0x85};
    uint8_t param18[] = {0xB8, 0x20};
    uint8_t param19[] = {0xC0, 0x09};
    uint8_t param20[] = {0xC1, 0x78};
    uint8_t param21[] = {0xC2, 0x78};
    uint8_t param22[] = {0xD0, 0x88};
    uint8_t param23[] = {0xEE, 0x42};
    uint8_t param24[] = {0xE0, 0x00, 0x00, 0x02};
    uint8_t param25[] = {0xE1, 0x04,0xA0,0x06,0xA0,0x05,0xA0,0x07,0xA0,0x00,0x44,0x44};
    uint8_t param26[] = {0xE2, 0x00,0x00,0x33,0x33,0x01,0xA0,0x00,0x00,0x01,0xA0,0x00,0x00};
    uint8_t param27[] = {0xE3, 0x00,0x00,0x33,0x33};
    uint8_t param28[] = {0xE4, 0x44,0x44}; // 0x05
    uint8_t param29[] = {0xE5, 0x0C,0x30,0xA0,0xA0,0x0E,0x32,0xA0,0xA0,0x08,0x2C,0xA0,0xA0,0x0A,0x2E,0xA0,0xA0};
    uint8_t param30[] = {0xE6, 0x00,0x00,0x33,0x33};
    uint8_t param31[] = {0xE7, 0x44,0x44};
    uint8_t param32[] = {0xE8, 0x0D,0x31,0xA0,0xA0,0x0F,0x33,0xA0,0xA0,0x09,0x2D,0xA0,0xA0,0x0B,0x2F,0xA0,0xA0};
    uint8_t param33[] = {0xEB, 0x00,0x01,0xE4,0xE4,0x44,0x88,0x00};
    uint8_t param34[] = {0xED, 0xFF,0xF5,0x47,0x6F,0x0B,0xA1,0xA2,0xBF,0xFB,0x2A,0x1A,0xB0,0xF6,0x74,0x5F,0xFF};
    uint8_t param35[] = {0xEF, 0x08,0x08,0x08,0x40,0x3F, 0x64};
    uint8_t param36[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    uint8_t param37[] = {0xE8, 0x00, 0x0E};
    uint8_t param38[] = {0xFF, 0x77,0x01,0x00,0x00,0x00};
    uint8_t param39[] = {0x11};

    uint8_t param40[] = {0xFF, 0x77,0x01,0x00,0x00,0x13};
    uint8_t param41[] = {0xE8, 0x00, 0x0C};

    uint8_t param42[] = {0xE8, 0x00, 0x00};
    uint8_t param43[] = {0xFF, 0x77,0x01,0x00,0x00,0x00};
    uint8_t param44[] = {0x3A, 0x50};
    uint8_t param45[] = {0x29};

    // k_u32 val = connecter_dsi_read_pkg(0xB9);

    // rt_kprintf("0xB9 val is %d \n", val);

    // kd_dwc_lpdt_send_pkg(param1, sizeof(param1));
    // usleep(120000);
    kd_dwc_lpdt_send_pkg(param2, sizeof(param2));
    kd_dwc_lpdt_send_pkg(param3, sizeof(param3));
    kd_dwc_lpdt_send_pkg(param4, sizeof(param4));
    kd_dwc_lpdt_send_pkg(param5, sizeof(param5));
    kd_dwc_lpdt_send_pkg(param6, sizeof(param6));
    kd_dwc_lpdt_send_pkg(param7, sizeof(param7));
    kd_dwc_lpdt_send_pkg(param8, sizeof(param8));
    kd_dwc_lpdt_send_pkg(param9, sizeof(param9));
    kd_dwc_lpdt_send_pkg(param10, sizeof(param10));
    kd_dwc_lpdt_send_pkg(param11, sizeof(param11));
    kd_dwc_lpdt_send_pkg(param12, sizeof(param12));
    kd_dwc_lpdt_send_pkg(param13, sizeof(param13));
    kd_dwc_lpdt_send_pkg(param14, sizeof(param14));
    kd_dwc_lpdt_send_pkg(param15, sizeof(param15));
    kd_dwc_lpdt_send_pkg(param16, sizeof(param16));
    kd_dwc_lpdt_send_pkg(param17, sizeof(param17));
    kd_dwc_lpdt_send_pkg(param18, sizeof(param18));
    kd_dwc_lpdt_send_pkg(param19, sizeof(param19));
    kd_dwc_lpdt_send_pkg(param20, sizeof(param20));
    kd_dwc_lpdt_send_pkg(param21, sizeof(param21));
    kd_dwc_lpdt_send_pkg(param22, sizeof(param22));
    kd_dwc_lpdt_send_pkg(param23, sizeof(param23));
    msleep(100);
    kd_dwc_lpdt_send_pkg(param24, sizeof(param24));
    kd_dwc_lpdt_send_pkg(param25, sizeof(param25));
    kd_dwc_lpdt_send_pkg(param26, sizeof(param26));
    kd_dwc_lpdt_send_pkg(param27, sizeof(param27));
    kd_dwc_lpdt_send_pkg(param28, sizeof(param28));
    kd_dwc_lpdt_send_pkg(param29, sizeof(param29));
    kd_dwc_lpdt_send_pkg(param30, sizeof(param30));
    kd_dwc_lpdt_send_pkg(param31, sizeof(param31));
    kd_dwc_lpdt_send_pkg(param32, sizeof(param32));
    kd_dwc_lpdt_send_pkg(param33, sizeof(param33));
    kd_dwc_lpdt_send_pkg(param34, sizeof(param34));
    kd_dwc_lpdt_send_pkg(param35, sizeof(param35));
    kd_dwc_lpdt_send_pkg(param36, sizeof(param36));
    kd_dwc_lpdt_send_pkg(param37, sizeof(param37));
    kd_dwc_lpdt_send_pkg(param38, sizeof(param38));
    kd_dwc_lpdt_send_pkg(param39, sizeof(param39));
    msleep(200);
    kd_dwc_lpdt_send_pkg(param40, sizeof(param40));
    kd_dwc_lpdt_send_pkg(param41, sizeof(param41));
    msleep(100);
    kd_dwc_lpdt_send_pkg(param42, sizeof(param42));
    kd_dwc_lpdt_send_pkg(param43, sizeof(param43));
    kd_dwc_lpdt_send_pkg(param44, sizeof(param44));
    kd_dwc_lpdt_send_pkg(param45, sizeof(param45));
    msleep(500);
}


static int st7701_dsi_resolution_init(k_connector_info *info)
{
    k_vo_dsi_attr attr;
    k_vo_display_resolution resolution;

    memset(&attr, 0, sizeof(k_vo_dsi_attr));
    attr.lan_num = info->lan_num;
    attr.cmd_mode = info->cmd_mode;
    attr.lp_div = 8;
    attr.work_mode = info->work_mode;
    memcpy(&resolution, &info->resolution, sizeof(k_vo_display_resolution));
    memcpy(&attr.resolution, &resolution, sizeof(k_vo_display_resolution));
    kd_dsi_set_attr(&attr);
    #ifdef CONFIG_K230_BARE_DISP_LOGO_DF
	st7701_480x640_init();
    #else
	st7701_480x800_init();
    #endif
    dwc_dsi_enable(1);

    return 0;
}



static int st7701_vo_resolution_init(k_vo_display_resolution *resolution, uint32_t bg_color, uint32_t intr_line)
{
    //k_vo_display_resolution vo_resolution;
    k_vo_pub_attr attr;

    memset(&attr, 0, sizeof(k_vo_pub_attr));
    attr.bg_color = bg_color;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    vo_init();
    kd_vo_set_vtth_intr(1, intr_line);
    kd_vo_set_dev_param(&attr);
    kd_vo_enable();

    return 0;
}

static int st7701_set_phy_freq(k_connectori_phy_attr *phy_attr)
{
    k_vo_mipi_phy_attr mipi_phy_attr;

    memset(&mipi_phy_attr, 0, sizeof(k_vo_mipi_phy_attr));

    mipi_phy_attr.m = phy_attr->m;
    mipi_phy_attr.n = phy_attr->n;
    mipi_phy_attr.hs_freq = phy_attr->hs_freq;
    mipi_phy_attr.voc = phy_attr->voc;
    mipi_phy_attr.phy_lan_num = K_DSI_4LAN;

    dwc_mipi_phy_config(&mipi_phy_attr);

    return 0;
}


int st7701_init(k_connector_info *info)
{
    int ret = 0;

	info->h_start = info->resolution.vback_porch;
    info->w_start = info->resolution.hback_porch;

	st7701_hardware_init();

	k230_display_rst();

    if(info->pixclk_div != 0)
        k230_set_pixclk(info->pixclk_div);

    ret |= st7701_set_phy_freq(&info->phy_attr);

    ret |= st7701_dsi_resolution_init(info);

    ret |= st7701_vo_resolution_init(&info->resolution, info->bg_color, info->intr_line);

	// kd_vo_set_layer(info, 320, 240, 0 , 0 , K_ROTATION_90, 0x13000000);
	vo_layer1_test(info);

    return ret;
}
