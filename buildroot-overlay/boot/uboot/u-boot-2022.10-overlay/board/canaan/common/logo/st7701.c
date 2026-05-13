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

void ili9881_800x1280_init(void)
{
    printf("f=%s xxx\n",__func__);
	// ILI9881 800x1280 initialization sequence from DTS
	// Page 0x03
	uint8_t param1[] = {0xff, 0x98, 0x81, 0x03};
	uint8_t param2[] = {0x01, 0x00};
	uint8_t param3[] = {0x02, 0x00};
	uint8_t param4[] = {0x03, 0x53};
	uint8_t param5[] = {0x04, 0x13};
	uint8_t param6[] = {0x05, 0x00};
	uint8_t param7[] = {0x06, 0x04};
	uint8_t param8[] = {0x07, 0x00};
	uint8_t param9[] = {0x08, 0x00};
	uint8_t param10[] = {0x09, 0x22};
	uint8_t param11[] = {0x0a, 0x22};
	uint8_t param12[] = {0x0b, 0x00};
	uint8_t param13[] = {0x0c, 0x01};
	uint8_t param14[] = {0x0d, 0x00};
	uint8_t param15[] = {0x0e, 0x00};
	uint8_t param16[] = {0x0f, 0x25};
	uint8_t param17[] = {0x10, 0x25};
	uint8_t param18[] = {0x11, 0x00};
	uint8_t param19[] = {0x12, 0x00};
	uint8_t param20[] = {0x13, 0x00};
	uint8_t param21[] = {0x14, 0x00};
	uint8_t param22[] = {0x15, 0x00};
	uint8_t param23[] = {0x16, 0x00};
	uint8_t param24[] = {0x17, 0x00};
	uint8_t param25[] = {0x18, 0x00};
	uint8_t param26[] = {0x19, 0x00};
	uint8_t param27[] = {0x1a, 0x00};
	uint8_t param28[] = {0x1b, 0x00};
	uint8_t param29[] = {0x1c, 0x00};
	uint8_t param30[] = {0x1d, 0x00};
	uint8_t param31[] = {0x1e, 0x44};
	uint8_t param32[] = {0x1f, 0x80};
	uint8_t param33[] = {0x20, 0x02};
	uint8_t param34[] = {0x21, 0x03};
	uint8_t param35[] = {0x22, 0x00};
	uint8_t param36[] = {0x23, 0x00};
	uint8_t param37[] = {0x24, 0x00};
	uint8_t param38[] = {0x25, 0x00};
	uint8_t param39[] = {0x26, 0x00};
	uint8_t param40[] = {0x27, 0x00};
	uint8_t param41[] = {0x28, 0x33};
	uint8_t param42[] = {0x29, 0x03};
	uint8_t param43[] = {0x2a, 0x00};
	uint8_t param44[] = {0x2b, 0x00};
	uint8_t param45[] = {0x2c, 0x00};
	uint8_t param46[] = {0x2d, 0x00};
	uint8_t param47[] = {0x2e, 0x00};
	uint8_t param48[] = {0x2f, 0x00};
	uint8_t param49[] = {0x30, 0x00};
	uint8_t param50[] = {0x31, 0x00};
	uint8_t param51[] = {0x32, 0x00};
	uint8_t param52[] = {0x33, 0x00};
	uint8_t param53[] = {0x34, 0x04};
	uint8_t param54[] = {0x35, 0x00};
	uint8_t param55[] = {0x36, 0x00};
	uint8_t param56[] = {0x37, 0x00};
	uint8_t param57[] = {0x38, 0x3c};
	uint8_t param58[] = {0x39, 0x00};
	uint8_t param59[] = {0x3a, 0x40};
	uint8_t param60[] = {0x3b, 0x40};
	uint8_t param61[] = {0x3c, 0x00};
	uint8_t param62[] = {0x3d, 0x00};
	uint8_t param63[] = {0x3e, 0x00};
	uint8_t param64[] = {0x3f, 0x00};
	uint8_t param65[] = {0x40, 0x00};
	uint8_t param66[] = {0x41, 0x00};
	uint8_t param67[] = {0x42, 0x00};
	uint8_t param68[] = {0x43, 0x00};
	uint8_t param69[] = {0x44, 0x00};
	uint8_t param70[] = {0x50, 0x01};
	uint8_t param71[] = {0x51, 0x23};
	uint8_t param72[] = {0x52, 0x45};
	uint8_t param73[] = {0x53, 0x67};
	uint8_t param74[] = {0x54, 0x89};
	uint8_t param75[] = {0x55, 0xab};
	uint8_t param76[] = {0x56, 0x01};
	uint8_t param77[] = {0x57, 0x23};
	uint8_t param78[] = {0x58, 0x45};
	uint8_t param79[] = {0x59, 0x67};
	uint8_t param80[] = {0x5a, 0x89};
	uint8_t param81[] = {0x5b, 0xab};
	uint8_t param82[] = {0x5c, 0xcd};
	uint8_t param83[] = {0x5d, 0xef};
	uint8_t param84[] = {0x5e, 0x11};
	uint8_t param85[] = {0x5f, 0x01};
	uint8_t param86[] = {0x60, 0x00};
	uint8_t param87[] = {0x61, 0x15};
	uint8_t param88[] = {0x62, 0x14};
	uint8_t param89[] = {0x63, 0x0c};
	uint8_t param90[] = {0x64, 0x0d};
	uint8_t param91[] = {0x65, 0x0e};
	uint8_t param92[] = {0x66, 0x0f};
	uint8_t param93[] = {0x67, 0x06};
	uint8_t param94[] = {0x68, 0x02};
	uint8_t param95[] = {0x69, 0x02};
	uint8_t param96[] = {0x6a, 0x02};
	uint8_t param97[] = {0x6b, 0x02};
	uint8_t param98[] = {0x6c, 0x02};
	uint8_t param99[] = {0x6d, 0x02};
	uint8_t param100[] = {0x6e, 0x08};
	uint8_t param101[] = {0x6f, 0x02};
	uint8_t param102[] = {0x70, 0x02};
	uint8_t param103[] = {0x71, 0x02};
	uint8_t param104[] = {0x72, 0x02};
	uint8_t param105[] = {0x73, 0x02};
	uint8_t param106[] = {0x74, 0x02};
	uint8_t param107[] = {0x75, 0x01};
	uint8_t param108[] = {0x76, 0x00};
	uint8_t param109[] = {0x77, 0x15};
	uint8_t param110[] = {0x78, 0x14};
	uint8_t param111[] = {0x79, 0x0c};
	uint8_t param112[] = {0x7a, 0x0d};
	uint8_t param113[] = {0x7b, 0x0e};
	uint8_t param114[] = {0x7c, 0x0f};
	uint8_t param115[] = {0x7d, 0x08};
	uint8_t param116[] = {0x7e, 0x02};
	uint8_t param117[] = {0x7f, 0x02};
	uint8_t param118[] = {0x80, 0x02};
	uint8_t param119[] = {0x81, 0x02};
	uint8_t param120[] = {0x82, 0x02};
	uint8_t param121[] = {0x83, 0x02};
	uint8_t param122[] = {0x84, 0x06};
	uint8_t param123[] = {0x85, 0x02};
	uint8_t param124[] = {0x86, 0x02};
	uint8_t param125[] = {0x87, 0x02};
	uint8_t param126[] = {0x88, 0x02};
	uint8_t param127[] = {0x89, 0x02};
	uint8_t param128[] = {0x8a, 0x02};
	// Page 0x04
	uint8_t param129[] = {0xff, 0x98, 0x81, 0x04};
	uint8_t param130[] = {0x6c, 0x15};
	uint8_t param131[] = {0x6e, 0x3b};
	uint8_t param132[] = {0x6f, 0x55};
	uint8_t param133[] = {0x8d, 0x14};
	uint8_t param134[] = {0x87, 0xba};
	uint8_t param135[] = {0x26, 0x76};
	uint8_t param136[] = {0xb2, 0xd1};
	uint8_t param137[] = {0x3b, 0x98};
	uint8_t param138[] = {0x35, 0x1f};
	uint8_t param139[] = {0x3a, 0x24};
	uint8_t param140[] = {0xb5, 0x27};
	uint8_t param141[] = {0x31, 0x75};
	uint8_t param142[] = {0x30, 0x03};
	uint8_t param143[] = {0x33, 0x14};
	uint8_t param144[] = {0x38, 0x02};
	uint8_t param145[] = {0x39, 0x00};
	uint8_t param146[] = {0x7a, 0x10};
	// Page 0x01
	uint8_t param147[] = {0xff, 0x98, 0x81, 0x01};
	uint8_t param148[] = {0x22, 0x0a};
	uint8_t param149[] = {0x31, 0x0a};
	uint8_t param150[] = {0x50, 0xae};
	uint8_t param151[] = {0x51, 0xa9};
	uint8_t param152[] = {0x60, 0x1f};
	uint8_t param153[] = {0x62, 0x07};
	uint8_t param154[] = {0x63, 0x00};
	uint8_t param155[] = {0x52, 0x00};
	uint8_t param156[] = {0x53, 0x56};
	uint8_t param157[] = {0x54, 0x00};
	uint8_t param158[] = {0x55, 0x59};
	uint8_t param159[] = {0xa0, 0x08};
	uint8_t param160[] = {0xa1, 0x20};
	uint8_t param161[] = {0xa2, 0x2d};
	uint8_t param162[] = {0xa3, 0x13};
	uint8_t param163[] = {0xa4, 0x16};
	uint8_t param164[] = {0xa5, 0x29};
	uint8_t param165[] = {0xa6, 0x1d};
	uint8_t param166[] = {0xa7, 0x1e};
	uint8_t param167[] = {0xa8, 0x77};
	uint8_t param168[] = {0xa9, 0x17};
	uint8_t param169[] = {0xaa, 0x24};
	uint8_t param170[] = {0xab, 0x6a};
	uint8_t param171[] = {0xac, 0x22};
	uint8_t param172[] = {0xad, 0x24};
	uint8_t param173[] = {0xae, 0x5a};
	uint8_t param174[] = {0xaf, 0x2b};
	uint8_t param175[] = {0xb0, 0x2c};
	uint8_t param176[] = {0xb1, 0x4d};
	uint8_t param177[] = {0xb2, 0x6f};
	uint8_t param178[] = {0xb3, 0x3f};
	uint8_t param179[] = {0xc0, 0x08};
	uint8_t param180[] = {0xc1, 0x1e};
	uint8_t param181[] = {0xc2, 0x2b};
	uint8_t param182[] = {0xc3, 0x13};
	uint8_t param183[] = {0xc4, 0x16};
	uint8_t param184[] = {0xc5, 0x28};
	uint8_t param185[] = {0xc6, 0x1a};
	uint8_t param186[] = {0xc7, 0x1d};
	uint8_t param187[] = {0xc8, 0x75};
	uint8_t param188[] = {0xc9, 0x18};
	uint8_t param189[] = {0xca, 0x25};
	uint8_t param190[] = {0xcb, 0x71};
	uint8_t param191[] = {0xcc, 0x23};
	uint8_t param192[] = {0xcd, 0x28};
	uint8_t param193[] = {0xce, 0x59};
	uint8_t param194[] = {0xcf, 0x2c};
	uint8_t param195[] = {0xd0, 0x30};
	uint8_t param196[] = {0xd1, 0x55};
	uint8_t param197[] = {0xd2, 0x6b};
	uint8_t param198[] = {0xd3, 0x3f};
	// Page 0x00
	uint8_t param199[] = { 0xff, 0x98, 0x81, 0x00};
	uint8_t param200[] = {0x35};
	uint8_t param201[] = {0x11};
	uint8_t param202[] = {0x29};

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
	kd_dwc_lpdt_send_pkg(param39, sizeof(param39));
	kd_dwc_lpdt_send_pkg(param40, sizeof(param40));
	kd_dwc_lpdt_send_pkg(param41, sizeof(param41));
	kd_dwc_lpdt_send_pkg(param42, sizeof(param42));
	kd_dwc_lpdt_send_pkg(param43, sizeof(param43));
	kd_dwc_lpdt_send_pkg(param44, sizeof(param44));
	kd_dwc_lpdt_send_pkg(param45, sizeof(param45));
	kd_dwc_lpdt_send_pkg(param46, sizeof(param46));
	kd_dwc_lpdt_send_pkg(param47, sizeof(param47));
	kd_dwc_lpdt_send_pkg(param48, sizeof(param48));
	kd_dwc_lpdt_send_pkg(param49, sizeof(param49));
	kd_dwc_lpdt_send_pkg(param50, sizeof(param50));
	kd_dwc_lpdt_send_pkg(param51, sizeof(param51));
	kd_dwc_lpdt_send_pkg(param52, sizeof(param52));
	kd_dwc_lpdt_send_pkg(param53, sizeof(param53));
	kd_dwc_lpdt_send_pkg(param54, sizeof(param54));
	kd_dwc_lpdt_send_pkg(param55, sizeof(param55));
	kd_dwc_lpdt_send_pkg(param56, sizeof(param56));
	kd_dwc_lpdt_send_pkg(param57, sizeof(param57));
	kd_dwc_lpdt_send_pkg(param58, sizeof(param58));
	kd_dwc_lpdt_send_pkg(param59, sizeof(param59));
	kd_dwc_lpdt_send_pkg(param60, sizeof(param60));
	kd_dwc_lpdt_send_pkg(param61, sizeof(param61));
	kd_dwc_lpdt_send_pkg(param62, sizeof(param62));
	kd_dwc_lpdt_send_pkg(param63, sizeof(param63));
	kd_dwc_lpdt_send_pkg(param64, sizeof(param64));
	kd_dwc_lpdt_send_pkg(param65, sizeof(param65));
	kd_dwc_lpdt_send_pkg(param66, sizeof(param66));
	kd_dwc_lpdt_send_pkg(param67, sizeof(param67));
	kd_dwc_lpdt_send_pkg(param68, sizeof(param68));
	kd_dwc_lpdt_send_pkg(param69, sizeof(param69));
	kd_dwc_lpdt_send_pkg(param70, sizeof(param70));
	kd_dwc_lpdt_send_pkg(param71, sizeof(param71));
	kd_dwc_lpdt_send_pkg(param72, sizeof(param72));
	kd_dwc_lpdt_send_pkg(param73, sizeof(param73));
	kd_dwc_lpdt_send_pkg(param74, sizeof(param74));
	kd_dwc_lpdt_send_pkg(param75, sizeof(param75));
	kd_dwc_lpdt_send_pkg(param76, sizeof(param76));
	kd_dwc_lpdt_send_pkg(param77, sizeof(param77));
	kd_dwc_lpdt_send_pkg(param78, sizeof(param78));
	kd_dwc_lpdt_send_pkg(param79, sizeof(param79));
	kd_dwc_lpdt_send_pkg(param80, sizeof(param80));
	kd_dwc_lpdt_send_pkg(param81, sizeof(param81));
	kd_dwc_lpdt_send_pkg(param82, sizeof(param82));
	kd_dwc_lpdt_send_pkg(param83, sizeof(param83));
	kd_dwc_lpdt_send_pkg(param84, sizeof(param84));
	kd_dwc_lpdt_send_pkg(param85, sizeof(param85));
	kd_dwc_lpdt_send_pkg(param86, sizeof(param86));
	kd_dwc_lpdt_send_pkg(param87, sizeof(param87));
	kd_dwc_lpdt_send_pkg(param88, sizeof(param88));
	kd_dwc_lpdt_send_pkg(param89, sizeof(param89));
	kd_dwc_lpdt_send_pkg(param90, sizeof(param90));
	kd_dwc_lpdt_send_pkg(param91, sizeof(param91));
	kd_dwc_lpdt_send_pkg(param92, sizeof(param92));
	kd_dwc_lpdt_send_pkg(param93, sizeof(param93));
	kd_dwc_lpdt_send_pkg(param94, sizeof(param94));
	kd_dwc_lpdt_send_pkg(param95, sizeof(param95));
	kd_dwc_lpdt_send_pkg(param96, sizeof(param96));
	kd_dwc_lpdt_send_pkg(param97, sizeof(param97));
	kd_dwc_lpdt_send_pkg(param98, sizeof(param98));
	kd_dwc_lpdt_send_pkg(param99, sizeof(param99));
	kd_dwc_lpdt_send_pkg(param100, sizeof(param100));
	kd_dwc_lpdt_send_pkg(param101, sizeof(param101));
	kd_dwc_lpdt_send_pkg(param102, sizeof(param102));
	kd_dwc_lpdt_send_pkg(param103, sizeof(param103));
	kd_dwc_lpdt_send_pkg(param104, sizeof(param104));
	kd_dwc_lpdt_send_pkg(param105, sizeof(param105));
	kd_dwc_lpdt_send_pkg(param106, sizeof(param106));
	kd_dwc_lpdt_send_pkg(param107, sizeof(param107));
	kd_dwc_lpdt_send_pkg(param108, sizeof(param108));
	kd_dwc_lpdt_send_pkg(param109, sizeof(param109));
	kd_dwc_lpdt_send_pkg(param110, sizeof(param110));
	kd_dwc_lpdt_send_pkg(param111, sizeof(param111));
	kd_dwc_lpdt_send_pkg(param112, sizeof(param112));
	kd_dwc_lpdt_send_pkg(param113, sizeof(param113));
	kd_dwc_lpdt_send_pkg(param114, sizeof(param114));
	kd_dwc_lpdt_send_pkg(param115, sizeof(param115));
	kd_dwc_lpdt_send_pkg(param116, sizeof(param116));
	kd_dwc_lpdt_send_pkg(param117, sizeof(param117));
	kd_dwc_lpdt_send_pkg(param118, sizeof(param118));
	kd_dwc_lpdt_send_pkg(param119, sizeof(param119));
	kd_dwc_lpdt_send_pkg(param120, sizeof(param120));
	kd_dwc_lpdt_send_pkg(param121, sizeof(param121));
	kd_dwc_lpdt_send_pkg(param122, sizeof(param122));
	kd_dwc_lpdt_send_pkg(param123, sizeof(param123));
	kd_dwc_lpdt_send_pkg(param124, sizeof(param124));
	kd_dwc_lpdt_send_pkg(param125, sizeof(param125));
	kd_dwc_lpdt_send_pkg(param126, sizeof(param126));
	kd_dwc_lpdt_send_pkg(param127, sizeof(param127));
	kd_dwc_lpdt_send_pkg(param128, sizeof(param128));
	kd_dwc_lpdt_send_pkg(param129, sizeof(param129));
	kd_dwc_lpdt_send_pkg(param130, sizeof(param130));
	kd_dwc_lpdt_send_pkg(param131, sizeof(param131));
	kd_dwc_lpdt_send_pkg(param132, sizeof(param132));
	kd_dwc_lpdt_send_pkg(param133, sizeof(param133));
	kd_dwc_lpdt_send_pkg(param134, sizeof(param134));
	kd_dwc_lpdt_send_pkg(param135, sizeof(param135));
	kd_dwc_lpdt_send_pkg(param136, sizeof(param136));
	kd_dwc_lpdt_send_pkg(param137, sizeof(param137));
	kd_dwc_lpdt_send_pkg(param138, sizeof(param138));
	kd_dwc_lpdt_send_pkg(param139, sizeof(param139));
	kd_dwc_lpdt_send_pkg(param140, sizeof(param140));
	kd_dwc_lpdt_send_pkg(param141, sizeof(param141));
	kd_dwc_lpdt_send_pkg(param142, sizeof(param142));
	kd_dwc_lpdt_send_pkg(param143, sizeof(param143));
	kd_dwc_lpdt_send_pkg(param144, sizeof(param144));
	kd_dwc_lpdt_send_pkg(param145, sizeof(param145));
	kd_dwc_lpdt_send_pkg(param146, sizeof(param146));
	kd_dwc_lpdt_send_pkg(param147, sizeof(param147));
	kd_dwc_lpdt_send_pkg(param148, sizeof(param148));
	kd_dwc_lpdt_send_pkg(param149, sizeof(param149));
	kd_dwc_lpdt_send_pkg(param150, sizeof(param150));
	kd_dwc_lpdt_send_pkg(param151, sizeof(param151));
	kd_dwc_lpdt_send_pkg(param152, sizeof(param152));
	kd_dwc_lpdt_send_pkg(param153, sizeof(param153));
	kd_dwc_lpdt_send_pkg(param154, sizeof(param154));
	kd_dwc_lpdt_send_pkg(param155, sizeof(param155));
	kd_dwc_lpdt_send_pkg(param156, sizeof(param156));
	kd_dwc_lpdt_send_pkg(param157, sizeof(param157));
	kd_dwc_lpdt_send_pkg(param158, sizeof(param158));
	kd_dwc_lpdt_send_pkg(param159, sizeof(param159));
	kd_dwc_lpdt_send_pkg(param160, sizeof(param160));
	kd_dwc_lpdt_send_pkg(param161, sizeof(param161));
	kd_dwc_lpdt_send_pkg(param162, sizeof(param162));
	kd_dwc_lpdt_send_pkg(param163, sizeof(param163));
	kd_dwc_lpdt_send_pkg(param164, sizeof(param164));
	kd_dwc_lpdt_send_pkg(param165, sizeof(param165));
	kd_dwc_lpdt_send_pkg(param166, sizeof(param166));
	kd_dwc_lpdt_send_pkg(param167, sizeof(param167));
	kd_dwc_lpdt_send_pkg(param168, sizeof(param168));
	kd_dwc_lpdt_send_pkg(param169, sizeof(param169));
	kd_dwc_lpdt_send_pkg(param170, sizeof(param170));
	kd_dwc_lpdt_send_pkg(param171, sizeof(param171));
	kd_dwc_lpdt_send_pkg(param172, sizeof(param172));
	kd_dwc_lpdt_send_pkg(param173, sizeof(param173));
	kd_dwc_lpdt_send_pkg(param174, sizeof(param174));
	kd_dwc_lpdt_send_pkg(param175, sizeof(param175));
	kd_dwc_lpdt_send_pkg(param176, sizeof(param176));
	kd_dwc_lpdt_send_pkg(param177, sizeof(param177));
	kd_dwc_lpdt_send_pkg(param178, sizeof(param178));
	kd_dwc_lpdt_send_pkg(param179, sizeof(param179));
	kd_dwc_lpdt_send_pkg(param180, sizeof(param180));
	kd_dwc_lpdt_send_pkg(param181, sizeof(param181));
	kd_dwc_lpdt_send_pkg(param182, sizeof(param182));
	kd_dwc_lpdt_send_pkg(param183, sizeof(param183));
	kd_dwc_lpdt_send_pkg(param184, sizeof(param184));
	kd_dwc_lpdt_send_pkg(param185, sizeof(param185));
	kd_dwc_lpdt_send_pkg(param186, sizeof(param186));
	kd_dwc_lpdt_send_pkg(param187, sizeof(param187));
	kd_dwc_lpdt_send_pkg(param188, sizeof(param188));
	kd_dwc_lpdt_send_pkg(param189, sizeof(param189));
	kd_dwc_lpdt_send_pkg(param190, sizeof(param190));
	kd_dwc_lpdt_send_pkg(param191, sizeof(param191));
	kd_dwc_lpdt_send_pkg(param192, sizeof(param192));
	kd_dwc_lpdt_send_pkg(param193, sizeof(param193));
	kd_dwc_lpdt_send_pkg(param194, sizeof(param194));
	kd_dwc_lpdt_send_pkg(param195, sizeof(param195));
	kd_dwc_lpdt_send_pkg(param196, sizeof(param196));
	kd_dwc_lpdt_send_pkg(param197, sizeof(param197));
	kd_dwc_lpdt_send_pkg(param198, sizeof(param198));
	kd_dwc_lpdt_send_pkg(param199, sizeof(param199));
	kd_dwc_lpdt_send_pkg(param200, sizeof(param200));
	kd_dwc_lpdt_send_pkg(param201, sizeof(param201));
	kd_dwc_lpdt_send_pkg(param202, sizeof(param202));

	msleep(100);
	kd_dwc_lpdt_send_pkg(param201, sizeof(param201));
	msleep(120);
	kd_dwc_lpdt_send_pkg(param202, sizeof(param202));

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
    #elif defined(CONFIG_K230_BARE_DISP_LOGO_ILI9881)
    ili9881_800x1280_init();
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
