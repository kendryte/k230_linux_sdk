#include <stdio.h>
//#include "utils.h"
#include <stdlib.h>
#include <string.h>
// #include <sleep.h>
// #include "dwc_csi.h"
// #include "video.h"
// #include <interrupt.h>
// #include <sys/types.h>
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
#include "linux/delay.h"
#include <asm/io.h>
#include "vo_table.h"
#include "common.h"

#define msleep mdelay
uint32_t dwc_dsi_set_lan_num(uint32_t lan_num)
{
    uint32_t reg = 0;

    // dwc dsi phy_stop_wait_time == 0x280
    reg = 0x2800 + lan_num ;
    writel(reg, DSI_BASE_ADDR + PHY_IF_CFG);   ///0x04

    return 0;
}


uint32_t dwc_dsi_set_lpdt_div(uint32_t div)
{
    uint32_t reg = 0;

    reg = 0x100 + div;
    writel(reg, DSI_BASE_ADDR + CLKMGR_CFG);

    return 0;
}

void dwc_dsi_set_vcid(void)
{
    writel(0x303, DSI_BASE_ADDR + GEN_VCID);
}

static uint32_t dw_mipi_dsi_get_hcomponent_lbcc(uint32_t hcomponent, uint64_t mipi_phy_clk, uint64_t mipi_dsi_clk)
{
    uint64_t frac, lbcc;

    lbcc = hcomponent * mipi_phy_clk / 8;

    frac = lbcc % mipi_dsi_clk;
    lbcc = lbcc / mipi_dsi_clk;
    if (frac)
        lbcc++;

    return (uint32_t)lbcc;
}


uint32_t dwc_dsi_set_timing(k_vo_display_resolution *resolution)
{
    uint32_t hsa = resolution->hsync_len;//20;
    uint32_t hbp = resolution->hback_porch;//40;
    //uint32_t hfp = resolution->hfront_porch;//220;
    uint32_t hact = resolution->hdisplay;//1920;
    uint32_t hline = resolution->htotal;

    uint32_t vsa = resolution->vsync_len;//1;
    uint32_t vbp = resolution->vback_porch;//10;//36;
    uint32_t vfp = resolution->vfront_porch;//35;//4;
    uint32_t vact = resolution->vdisplay;//1080;
    //uint32_t vline =  resolution->vtotal;


    writel(hact, DSI_BASE_ADDR + VID_PKT_SIZE);   //0x80
    writel(0x0, DSI_BASE_ADDR + VID_NUM_CHUNKS);
    writel(0x0, DSI_BASE_ADDR + VID_NULL_SIZE);


    writel(dw_mipi_dsi_get_hcomponent_lbcc(hsa, resolution->phyclk, resolution->pclk), DSI_BASE_ADDR + VID_HSA_TIME);   //   hsa = 0x21
    writel(dw_mipi_dsi_get_hcomponent_lbcc(hbp, resolution->phyclk, resolution->pclk), DSI_BASE_ADDR + VID_HBP_TIME);   //   hbp = 0x6f
    writel(dw_mipi_dsi_get_hcomponent_lbcc(hline, resolution->phyclk, resolution->pclk), DSI_BASE_ADDR + VID_HLINE_TIME);  //  hline = 0x671

    writel(vsa, DSI_BASE_ADDR + VID_VSA_LINES);   //vsa
    writel(vbp, DSI_BASE_ADDR + VID_VBP_LINES);  //     vbp
    writel(vfp, DSI_BASE_ADDR + VID_VFP_LINES);  //    vfp
    writel(vact, DSI_BASE_ADDR + VID_VACTIVE_LINES);  //  vact


    // vo_debug_trace("vsa is %x \n", readl(DSI_BASE_ADDR + VID_VSA_LINES));
    // vo_debug_trace("vbp is %x \n", readl(DSI_BASE_ADDR + VID_VBP_LINES));
    // vo_debug_trace("vact is %x \n", readl(DSI_BASE_ADDR + VID_VACTIVE_LINES));

    return 0;
}


void k230_set_pixclk(uint32_t div)
{
    uint32_t reg = 0;

    reg = readl(CLK_BASE_ADDR + 0x78);
    reg = (reg & ~(GENMASK(10, 3))) | (div << 3);         //  8M =    pll1(2376) / 4 / 66
    reg = reg | (1 << 31);
    writel(reg, CLK_BASE_ADDR+ 0x78);
}

uint32_t dwc_dsi_set_lpdt_mode(void)
{

    writel(0x1, DSI_BASE_ADDR + MODE_CFG);              // set lp cmd mode
    writel(0xbf02, DSI_BASE_ADDR + VID_MODE_CFG);
    writel(0x10f7f01, DSI_BASE_ADDR + CMD_MODE_CFG);
    writel(0x1c, DSI_BASE_ADDR + PCKHDL_CFG);
    writel(0x1, DSI_BASE_ADDR + 0x4);

    return 0;
}


void dwc_dsi_set_color_coding(void)
{
    // set color 24-bit , vo 24 bits passed
    writel(0x105, DSI_BASE_ADDR + DPI_COLOR_CODING);
}

uint32_t dwc_dsi_init(void)
{
    // dwc_mipi_phy_config();

    dwc_dsi_set_color_coding();

    // dwc_dsi_set_vcid();

    writel(0x320068, DSI_BASE_ADDR + 0x9c);
    writel(0x2e0080, DSI_BASE_ADDR + 0x98);
    writel(0xffffffff, DSI_BASE_ADDR + 0xc4);
    writel(0xffffffff, DSI_BASE_ADDR + 0xc8);

    writel(0x0, DSI_BASE_ADDR + MODE_CFG);              // set hs cmd mode
    // set high speed transmiss
    writel(0x0, DSI_BASE_ADDR + CMD_MODE_CFG);
    // set clk continuous bit1 = 1 ;
    writel(0x3, DSI_BASE_ADDR + LPCLK_CTRL);  //0x3

    return 0;
}

uint32_t dwc_dsi_enable(uint32_t enable)
{
    dwc_dsi_init();
    // start
    writel(0x1, DSI_BASE_ADDR + LPCLK_CTRL);  //0x3

    // dwc_dsi_get_reg_val();

    return 0;
}


uint32_t kd_dsi_set_attr(k_vo_dsi_attr *attr)
{
    // set lan num
    dwc_dsi_set_lan_num(attr->lan_num);
    // set lp div
    dwc_dsi_set_lpdt_div(attr->lp_div);
    // set vcid
    dwc_dsi_set_vcid();
    // set dsi timing
    dwc_dsi_set_timing(&attr->resolution);
    // set cmd mode

    dwc_dsi_set_lpdt_mode();

    return 0;
}


uint32_t kd_dwc_lpdt_send_pkg(uint8_t *buf, uint32_t cmd_len)
{
    int pld_data_bytes = sizeof(uint32_t);
    //int ret = 0;
    //int cut = 0;
    int len = cmd_len;
    uint32_t word = 0;
    //uint32_t reg = 0;
    uint32_t hdr_val = 0;

	// printf("cmd_len is %d \n", cmd_len);
    if (cmd_len >= 2)
    {
        while (len)
        {
            if (len < pld_data_bytes)            // 0 1
            {

                memcpy(&word, buf, pld_data_bytes);
                writel(word, DSI_BASE_ADDR + GEN_PLD_DATA);
                len = 0;
            }
            else
            {
                memcpy(&word, buf, pld_data_bytes);
                writel(word, DSI_BASE_ADDR + GEN_PLD_DATA);
                buf += pld_data_bytes;
                len -= pld_data_bytes;
            }
        }

        hdr_val = 0x39;//0x37;//(0x05) + (0x3 << 6) + (cmd_len << 8);  // long package
        hdr_val = (hdr_val & ~(GENMASK(7, 6))) | (0x0 << 6);            //0x3
        hdr_val = (hdr_val & ~(GENMASK(23, 8))) | (cmd_len << 8);
        writel(hdr_val, DSI_BASE_ADDR + GEN_HDR);
		// printf("long hdr_val is %x \n", hdr_val);
    //    msleep(3);
    }
    else
    {
        hdr_val = 0x05;//(0x05) + (0x3 << 6) + (cmd_len << 8);
        hdr_val = (hdr_val & ~(GENMASK(7, 6))) | (0x0 << 6);        //0x3
        hdr_val = (hdr_val & ~(GENMASK(23, 8))) | (buf[0] << 8);
        writel(hdr_val, DSI_BASE_ADDR + GEN_HDR);
        // printf("short hdr_val is %x \n", hdr_val);
	}
    msleep(3);
    return 0;
}


void k230_display_rst(void)
{
    //uint32_t rdata = 0;

    writel(0x0, (volatile void __iomem* )0x91101090);
    writel(0x1, (volatile void __iomem* )0x91101090);

    msleep(10);
}


/*  vo  reg init   */


static void kd_vo_set_hsync(uint32_t start, uint32_t end)
{
    uint32_t reg = 0;

    reg = (reg & ~(GENMASK(12, 0))) | (start << 0);
    reg = (reg & ~(GENMASK(28, 16))) | (end << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_HSYNC_CTL);
}

static void kd_vo_set_hsync1(uint32_t start, uint32_t end)
{
    uint32_t reg = 0;

    reg = (reg & ~(GENMASK(12, 0))) | (start << 0);
    reg = (reg & ~(GENMASK(28, 16))) | (end << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_HSYNC1_CTL);
}

static void kd_vo_set_hsync2(uint32_t start, uint32_t end)
{
    uint32_t reg = 0;

    reg = (reg & ~(GENMASK(12, 0))) | (start << 0);
    reg = (reg & ~(GENMASK(28, 16))) | (end << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_HSYNC2_CTL);
}

static void kd_vo_set_vsync1(uint32_t start, uint32_t end)
{
    uint32_t reg = 0;

    reg = (reg & ~(GENMASK(12, 0))) | (start << 0);
    reg = (reg & ~(GENMASK(28, 16))) | (end << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_VSYNC1_CTL);
}

static void kd_vo_set_vsync2(uint32_t start, uint32_t end)
{
    uint32_t reg = 0;

    reg = (reg & ~(GENMASK(12, 0))) | (start << 0);
    reg = (reg & ~(GENMASK(28, 16))) | (end << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_VSYNC2_CTL);
}

static void kd_vo_wrap_init(void)
{
    uint8_t i = 0;

    writel(0x11, VO_BASE_ADDR + VO_DMA_SW_CTL);
    writel(0x88, VO_BASE_ADDR + VO_DMA_RD_CTL_OUT);
    writel(0x0, VO_BASE_ADDR + VO_DMA_ARB_MODE);

    for (i = 0; i < 8; i++)
    {
        writel(0x1010101, VO_BASE_ADDR + 0x14 + (4 * i));
    }

    writel(0x76543210, VO_BASE_ADDR + 0x34);
    writel(0x76543210, VO_BASE_ADDR + 0x3c);
    writel(0x76543210, VO_BASE_ADDR + 0x44);
    writel(0x76543210, VO_BASE_ADDR + VO_DMA_ID_RD_0);

    writel(0xfedcba98, VO_BASE_ADDR + 0x38);
    writel(0xfedcba98, VO_BASE_ADDR + 0x40);
    writel(0xfedcba98, VO_BASE_ADDR + 0x48);
    writel(0xfedcba98, VO_BASE_ADDR + 0x50);

    for (i = 0; i < 10; i++)
    {
        writel(0x701, VO_BASE_ADDR + VO_LAYER0_LINE0_BD_CTL + (4 * i));
    }

    writel(0x701, VO_BASE_ADDR + 0x800);
    writel(0x701, VO_BASE_ADDR + 0x804);
    writel(0x701, VO_BASE_ADDR + 0x808);
    writel(0x701, VO_BASE_ADDR + 0x80c);
    writel(0x701, VO_BASE_ADDR + 0x810);

    writel(0x0, VO_BASE_ADDR + VO_DISP_ENABLE);
}


static void kd_vo_table_init(void)
{
    int i = 0;

    for (i = 0; i < 64; i++)
    {
        writel(vo_V_Coef[i * 2], (VO_BASE_ADDR + VO_VSCALE_BASE + ((i * 2) << 2)));
        writel(vo_V_Coef[i * 2 + 1], (VO_BASE_ADDR + VO_VSCALE_BASE + ((i * 2 + 1) << 2)));
    }

    // init HSCALE
    for (i = 0; i < 64; i++)
    {
        writel(vo_H_Coef[i * 3], (VO_BASE_ADDR + VO_HSCALE_BASE + ((i * 4) << 2)));
        writel(vo_H_Coef[i * 3 + 1], (VO_BASE_ADDR + VO_HSCALE_BASE + ((i * 4 + 1) << 2)));
        writel(vo_H_Coef[i * 3 + 2], (VO_BASE_ADDR + VO_HSCALE_BASE + ((i * 4 + 2) << 2)));
    }
}


static void kd_vo_set_config_mix(void)
{
    uint32_t reg = 0;

    writel(0x7f, VO_BASE_ADDR + VO_DISP_MIX_LAYER_GLB_EN);
    writel(0xffffffff, VO_BASE_ADDR + VO_DISP_MIX_LAYER_GLB_ALPHA0);
    writel(0xffffffff, VO_BASE_ADDR + VO_DISP_MIX_LAYER_GLB_ALPHA1);
    writel(0xffffffff, VO_BASE_ADDR + 0x940);

    reg = 0 + (1 << 4) + (2 << 8) + (3 << 12);
    writel(reg, VO_BASE_ADDR + VO_DISP_MIX_SEL);

    reg = 4 + (5 << 4) + (6 << 8) + (7 << 12) + (8 << 16) + (9 << 20) + (10 << 24) + (11 << 28);
    writel(reg, VO_BASE_ADDR + 0x950);

    // config csc
    writel(0x1, VO_BASE_ADDR + VO_DISP_YUV2RGB_CTL);
    // disp gamma enable
    writel(0x0, VO_BASE_ADDR + VO_DISP_CLUT_CTL);
    // disp dith enable
    writel(0x1, VO_BASE_ADDR + VO_DISP_DITH_CTL);

    writel(0x11111111, VO_BASE_ADDR + VO_OSD_RGB2YUV_CTL);

    writel(0xff, VO_BASE_ADDR + VO_DISP_MIX_LAYER_GLB_EN);

    //close vo irq
    writel(0x0, VO_BASE_ADDR + VO_DISP_IRQ0_CTL);
    writel(0x0, VO_BASE_ADDR + VO_DISP_IRQ1_CTL);
    writel(0x0, VO_BASE_ADDR + VO_DISP_IRQ2_CTL);
}


static void kd_vo_osd_set_addr_select_mode(k_vo_osd osd, uint32_t mode)
{

    writel(mode, VO_BASE_ADDR + VO_OSD_ADDR_SEL_MODE(osd));
}

static void kd_vo_osd_set_dma_request(k_vo_osd osd)
{
    uint32_t reg = 0;


    reg = readl(VO_BASE_ADDR + VO_OSD_DMA_CTRL(osd));
    reg = (reg & ~(GENMASK(3, 0))) | (0xf << 0);
    writel(reg, VO_BASE_ADDR + VO_OSD_DMA_CTRL(osd));
}


static void kd_vo_osd_set_dma_map(k_vo_osd osd, k_vo_osd_dma_map map)
{
    uint32_t reg = 0;

    reg = readl(VO_BASE_ADDR + VO_OSD_DMA_CTRL(osd));
    reg = (reg & ~(GENMASK(5, 4))) | (map << 4);
    writel(reg, VO_BASE_ADDR + VO_OSD_DMA_CTRL(osd));

}


void kd_vo_layer_set_uv_endian_mode(k_vo_layer layer, k_vo_layer_uv_endian_mode mode)
{
    uint32_t reg = 0;

    reg = readl(VO_BASE_ADDR + VO_DISP_LAYER_CFG(layer));

    reg = (reg & ~(GENMASK(18, 16))) | (mode << 16);

    writel(reg, VO_BASE_ADDR + VO_DISP_LAYER_CFG(layer));
}

void kd_vo_layer_set_y_endian_mode(k_vo_layer layer, k_vo_layer_y_endian_mode mode)
{
    uint32_t reg = 0;

    reg = readl(VO_BASE_ADDR + VO_DISP_LAYER_CFG(layer));

    reg = (reg & ~(GENMASK(13, 12))) | (mode << 12);

    writel(reg, VO_BASE_ADDR + VO_DISP_LAYER_CFG(layer));
}


void kd_vo_layer_set_img_blenth(k_vo_layer layer, uint32_t blenth)
{
    writel(blenth, VO_BASE_ADDR + VO_DISP_LAYER_IMG_BLENTH(layer));
}

void kd_vo_layer_addr_select_mode(k_vo_layer layer, uint32_t mode)
{
    writel(mode, VO_BASE_ADDR + VO_DISP_LAYER_ADDR_SEL_MODE(layer));
}

void kd_vo_set_layer_outstanding(k_vo_layer layer)
{
    writel(0x7, VO_BASE_ADDR + VO_DISP_LAYER_CFG(layer));
}


void kd_vo_set_vtth_intr(bool status, uint32_t vpos)
{
    uint32_t reg = 0;

    reg = (reg & ~(BIT_MASK(20))) | (status << 20);
    if (status != 0)
        reg = (reg & ~(GENMASK(12, 0))) | (vpos << 0);

    writel(reg, VO_BASE_ADDR + VO_DISP_IRQ1_CTL);
}

uint32_t vo_init(void)
{
    uint8_t i = 0;
    // kd_vo_software_reset();
    kd_vo_wrap_init();
    kd_vo_set_config_mix();
    kd_vo_table_init();

    for (i = 0; i < 4; i ++)
    {
        // set osd addr select
        kd_vo_osd_set_addr_select_mode(K_VO_OSD0 + i, 0x100); //0x100
        // set osd dma map
        kd_vo_osd_set_dma_map(K_VO_OSD0 + i, K_VO_OSD_MAP_1234_TO_4321);
        //
        kd_vo_osd_set_dma_request(K_VO_OSD0 + i);
    }

    for (i = 0; i < 3; i++)
    {
        kd_vo_layer_set_uv_endian_mode(K_VO_LAYER0 + i, K_VO_LAYER_UV_ENDIAN_DODE2);
        kd_vo_layer_set_y_endian_mode(K_VO_LAYER0 + i, K_VO_LAYER_Y_ENDIAN_DODE1);
        kd_vo_layer_set_img_blenth(K_VO_LAYER0 + i, 0xf);
        kd_vo_layer_addr_select_mode(K_VO_LAYER0 + i, 0x100);
    }

    // set layer0 layer1 blenth and outstanding
    for (i = 0; i < 2; i++)
    {
        kd_vo_set_layer_outstanding(K_VO_LAYER0 + i);

    }

    // default open frame intr
    // kd_vo_set_frame_intr(K_TRUE);

    kd_vo_set_vtth_intr(1, 10);

    return 0;
}


void kd_vo_set_timing(k_vo_display_resolution *resolution)
{
    uint32_t hbp = resolution->hback_porch;//40;
    uint32_t hact = resolution->hdisplay;//1920;
    uint32_t hline = resolution->htotal;
    uint32_t vbp = resolution->vback_porch;//10;//36;
    uint32_t vact = resolution->vdisplay;//1080;
    uint32_t vline =  resolution->vtotal;
    uint32_t reg;

    kd_vo_set_hsync(0x2, 0x5);
    kd_vo_set_hsync1(0x2, 0x5);
    kd_vo_set_hsync2(0x2, 0x5);
    kd_vo_set_vsync1(0x0, 0x0);
    kd_vo_set_vsync2(0x0, 0x0);

    // display_remap->w_start = hbp;
    // display_remap->h_start = vbp;
    // set xzone
    reg = (hbp) + ((hact + hbp - 1) << 16);
    writel(reg, VO_BASE_ADDR + VO_DISP_XZONE_CTL);
    // set yzone
    reg = (vbp) + ((vact + vbp - 1) << 16);
    writel(reg, VO_BASE_ADDR + VO_DISP_YZONE_CTL);
    // set total size
    reg = (hline) + ((vline) << 16);
    writel(reg, VO_BASE_ADDR + VO_DISP_TOTAL_SIZE);
    // set draw
    reg = (hact - 1) + ((vact - 1) << 16) + (0x1 << 15);
    writel(reg, VO_BASE_ADDR + 0x780); // enalbe remap  0x77f8437
}


void kd_vo_set_background(uint32_t yuv)
{
    writel(yuv, VO_BASE_ADDR + VO_DISP_BACKGROUND);
}


// 设置公共属性  视频输出设备公共属性结构体指针。
int kd_vo_set_dev_param(k_vo_pub_attr *attr)
{
    // set timing
    kd_vo_set_timing(attr->sync_info);

    //set backlight
    kd_vo_set_background(attr->bg_color);

    return 0;
}


void kd_vo_enable(void)
{
    writel(0x11, VO_BASE_ADDR + VO_REG_LOAD_CTL);

}

void kd_vo_set_layer(k_connector_info *info, uint32_t width, uint32_t height, uint32_t pos_x , uint32_t pos_y , k_vo_rotation rotation, uint32_t addr)
{
	uint32_t reg = 0;
	uint32_t uv_addr  = 0;
	//uv swap
	reg = readl(VO_BASE_ADDR + VO_DISP_LAYER_CFG(K_VO_LAYER1));
	reg = (reg & ~(BIT_MASK(9))) | (0 << 9);
	writel(reg, VO_BASE_ADDR + VO_DISP_LAYER_CFG(K_VO_LAYER1));

	//stride
	reg = (height << 16) + (width / 8 - 1);
	writel(reg, VO_BASE_ADDR + VO_LAYER1_IMG_IN_STRIDE);

	// in pix_size
	reg = (height << 16) + (width - 1);
	writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_ACT_SIZE);

	// set layer addr
	writel(addr, VO_BASE_ADDR + VO_DISP_LAYER_Y_ADDR0(K_VO_LAYER1));
    writel(addr, VO_BASE_ADDR + VO_DISP_LAYER_Y_ADDR1(K_VO_LAYER1));

	uv_addr = width * height + addr;
    writel(uv_addr, VO_BASE_ADDR + VO_DISP_LAYER_UV_ADDR0(K_VO_LAYER1));
    writel(uv_addr, VO_BASE_ADDR + VO_DISP_LAYER_UV_ADDR1(K_VO_LAYER1));

	// set postion
	// rotation
	if ((rotation & K_ROTATION_90) || (rotation & K_ROTATION_270))
	{
		// set rotation
		reg = readl(VO_BASE_ADDR + VO_LAYER1_CONFIG);
		reg = (reg & ~(GENMASK(5, 4))) | (1 << 4);
		writel(reg, VO_BASE_ADDR + VO_LAYER1_CONFIG);

		// LAYER1 size
		reg = (width << 16) + (height);
		writel(reg, VO_BASE_ADDR + VO_LAYER1_SIZE);

		// LAYER1 stride
		reg = (height << 16) + (height);
		writel(reg, VO_BASE_ADDR + VO_LAYER1_STRIDE);

		reg = ((width + info->w_start - 1 + pos_x) << 16) + (info->w_start + pos_x);
		writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_XCTL);

		reg = ((height + info->h_start - 1 + pos_y) << 16) + (info->h_start + pos_y);
		writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_YCTL);
	}
	else
	{
		// LAYER1 size
		reg = (height << 16) + (width);
		writel(reg, VO_BASE_ADDR + VO_LAYER1_SIZE);

		// LAYER1 stride
		reg = (width << 16) + (width);
		writel(reg, VO_BASE_ADDR + VO_LAYER1_STRIDE);

		reg = ((width + info->w_start - 1 + pos_x) << 16) + (info->w_start + pos_x);

		writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_XCTL);

		reg = ((height + info->h_start - 1 +pos_y) << 16) + (info->h_start +pos_y);
		writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_YCTL);
	}

	///enable layer1
	reg = readl(VO_BASE_ADDR + VO_LAYER1_CTL);
	reg = (reg & ~(BIT_MASK(0))) | (1 << 0);
	writel(1, VO_BASE_ADDR + VO_LAYER1_CTL);

	reg = readl(VO_BASE_ADDR + VO_LAYER1_ADDR_SEL);
	reg = (reg & ~(BIT_MASK(16))) | (1 << 16);
	writel(reg, VO_BASE_ADDR + VO_LAYER1_ADDR_SEL);

	// enable layer crtl
	reg = readl(VO_BASE_ADDR + VO_DISP_ENABLE);
	reg = (reg & ~(BIT_MASK(1))) | (1 << 1);
	writel(reg, VO_BASE_ADDR + VO_DISP_ENABLE);
}
#define YUV_PIC_ADD k230_display_logo_get_pic_mem_addr()
void vo_layer1_test(k_connector_info *info)
{
    uint32_t reg = 0;
    #ifdef CONFIG_K230_BARE_DISP_LOGO_DF
    uint32_t act_w = 480;
    uint32_t act_h = 640;
    #else
    uint32_t act_w = 480;
    uint32_t act_h = 800;
    #endif

    //uint32_t offset_w = 0;
    //uint32_t offset_h = 0;

    // set layer2- format
    reg = 1 + (0 << 9) + (1 << 8) + (1 << 12) + (3 << 16) + (1 << 28);
    writel(reg, VO_BASE_ADDR + VO_LAYER1_CTL);

    // addr
    writel(YUV_PIC_ADD, VO_BASE_ADDR + VO_DISP_LAYER1_Y_ADDR0);
    writel(YUV_PIC_ADD, VO_BASE_ADDR + VO_DISP_LAYER1_Y_ADDR1);

    writel(YUV_PIC_ADD + (act_w * act_h), VO_BASE_ADDR + VO_DISP_LAYER1_UV_ADDR0);
    writel(YUV_PIC_ADD + (act_w * act_h), VO_BASE_ADDR + VO_DISP_LAYER1_UV_ADDR1);

    // in offset
    writel(0x0, VO_BASE_ADDR + VO_LAYER1_IMG_IN_OFFSET);

    //blenth
    writel(0xf, VO_BASE_ADDR + VO_LAYER1_IMG_IN_BLENTH);

    //stride
    reg = (act_h << 16) + (act_w / 8 - 1);
    writel(reg, VO_BASE_ADDR + VO_LAYER1_IMG_IN_STRIDE);

    // in pix_size
    reg = (act_h << 16) + (act_w - 1);
    writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_ACT_SIZE);

    // LAYER0 Cfg rot 90
    writel(0x17, VO_BASE_ADDR + VO_LAYER1_CONFIG);

    // LAYER1 size
    reg = (act_w << 16) + (act_h);
    writel(reg, VO_BASE_ADDR + VO_LAYER1_SIZE);

    // LAYER1 stride
    reg = (act_h << 16) + (act_h);
    writel(reg, VO_BASE_ADDR + VO_LAYER1_STRIDE);

    // LAYER1 addr sel
    writel(0x10000, VO_BASE_ADDR + VO_LAYER1_ADDR_SEL);

    //set layer pos
   // reg = ((act_w + w_start - 1) << 16) + (w_start);
    reg = ((act_w + info->w_start - 1) << 16) + (info->w_start);
    writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_XCTL);

    reg = ((act_h + info->h_start - 1 ) << 16) + (info->h_start);
    writel(reg, VO_BASE_ADDR + VO_DISP_LAYER1_YCTL);

	reg  = readl(VO_BASE_ADDR + VO_LAYER1_CONFIG);
	reg = (reg & ~(BIT_MASK(9))) | (0 << 9);
	writel(reg, VO_BASE_ADDR + VO_LAYER1_CONFIG);   // disable all plane

	writel(0x2, VO_BASE_ADDR + VO_DISP_ENABLE);   // disable all plane

}

/* phy */

void k230_dsi_write_phy_reg(uint8_t addr, uint8_t val)
{
    uint32_t reg = 0;

    reg = 0x10000 + addr;
    writel(reg, PHY_TST_CTRL1 + DSI_BASE_ADDR);
    writel(0x2, PHY_TST_CTRL0 + DSI_BASE_ADDR);
    writel(0x0, PHY_TST_CTRL0 + DSI_BASE_ADDR);

    reg = val;
    writel(reg, PHY_TST_CTRL1 + DSI_BASE_ADDR);
    writel(0x2, PHY_TST_CTRL0 + DSI_BASE_ADDR);
    writel(0x0, PHY_TST_CTRL0 + DSI_BASE_ADDR);
}

void k230_dsi_phy_pll_config(uint32_t m, uint32_t n, uint8_t vco)
{
    uint32_t reg = 0;

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(9))) | (1 << 9);             // shadow_clear field
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(9))) | (0 << 9);            // shadow_clear field
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(1, 0))) | (0x1 << 0);            // clksel
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);         // TODO case by case
    reg = (reg & ~(GENMASK(26, 17))) | (m << 17);           // M=m+2   M=125
    reg = (reg & ~(GENMASK(30, 27))) | (n << 27);           // N=n+1   N=6
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);         // expect fout = 125 = (1M/4N)*24;  m=125,n=6

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(16, 11))) | (vco << 11);         // vco_cntrl
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(6, 0))) | (0x10 << 0);             // cpbias_cntrl
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(9, 8))) | (0x00 << 8);             // gmp_cntrl
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(16, 11))) | (0x00 << 11);             // int_cntrl
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG0);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(8, 3))) | (0x8 << 3);         // prop_cntrl
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

    k230_dsi_write_phy_reg(0x14, 0x2);    // pll1_th1
    k230_dsi_write_phy_reg(0x15, 0x60);    // pll1_th2
    k230_dsi_write_phy_reg(0x16, 0x3);    // pll1_th3
    k230_dsi_write_phy_reg(0x1D, 0x1);    // pll_lock_sel

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(10))) | (1 << 10);
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

    msleep(1);

    reg = (reg & ~(BIT_MASK(10))) | (0 << 10);
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_PLL_CFG1);

}

void k230_dsi_phy0_config(uint32_t m, uint32_t n, uint8_t vco, uint8_t hsfreq)
{
    uint32_t reg = 0;

    // select phy0
    writel(0x0, TXPHY_BASE_ADDR + TXDPHY_CFG1);
    // set rstz
    writel(0xc, DSI_BASE_ADDR + PHY_RSTZ);
    // Set test clr to 1
    writel(0x1, DSI_BASE_ADDR + PHY_TST_CTRL0);
    // Set test clr to 0
    writel(0x0, DSI_BASE_ADDR + PHY_TST_CTRL0);
    // set mastermacro=1, prototyping_env =1 ??
    k230_dsi_write_phy_reg(0x0c, 0x03);
    //set hsfreqrange
    // k230_dsi_write_phy_reg(0x44, 0x96);             // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    k230_dsi_write_phy_reg(0x44, hsfreq);             // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    // for < 1.5Gbps data rate set Slew
    k230_dsi_write_phy_reg(0xa0, 0x40);           // set Slew rate vs DDL sr_range      *TODO case by case
    k230_dsi_write_phy_reg(0xa4, 0x11);           // set Slew sr_osc_freq_target[6:0]   *TODO case by case
    k230_dsi_write_phy_reg(0xa4, 0x85);           // set Slew sr_osc_freq_target[11:7]  *TODO case by case
    k230_dsi_write_phy_reg(0xa3, 0x1);           // enable Slew rate calibration  (sr_sel_tester)

    k230_dsi_write_phy_reg(0x1f, 0x1);               // mpll_prog[0] = 1'b1
    k230_dsi_write_phy_reg(0x4a, 0x40);               // prg_on_lane0 = 1'b1

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_CFG0);
    reg = (reg & ~(GENMASK(7, 2))) | (0x28 << 2);            //phy0 cfgclkfreqrange     6'b101000;
    reg = (reg & ~(GENMASK(13, 8))) | (0x28 << 8);          //phy1 cfgclkfreqrange      6'b101000;
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_CFG0);

    // config pll
    // k230_dsi_phy_pll_config(295, 15, 0x17);                  // 222.75 M   phy clk = pll x 2
    k230_dsi_phy_pll_config(m, n, vco);                  // 222.75 M   phy clk = pll x 2

    writel(0x28a0, TXPHY_BASE_ADDR + TXDPHY_CFG0);
    writel(0xc, DSI_BASE_ADDR + PHY_RSTZ);
}



void k230_dsi_phy1_config(uint8_t hsfreq)
{
    uint32_t reg = 0;

    // select1
    writel(0x400000, TXPHY_BASE_ADDR + TXDPHY_CFG1);

    // printf("0x400000 is %x \n", readl(TXPHY_BASE_ADDR + TXDPHY_CFG1));

    writel(0x1, DSI_BASE_ADDR + PHY_TST_CTRL0);
    msleep(1);
    writel(0x0, DSI_BASE_ADDR + PHY_TST_CTRL0);

    // SET TEST CLR TO LOW
    k230_dsi_write_phy_reg(0xc, 0x0);

    // select1
    writel(0x400000, TXPHY_BASE_ADDR + TXDPHY_CFG1);


    //set hsfreqrange 445.5Mbps
    // k230_dsi_write_phy_reg(0x44, 0x96);         // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    k230_dsi_write_phy_reg(0x44, hsfreq);         // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）

    // try slave phy hs clk lane
    k230_dsi_write_phy_reg(0x30, 0xff);

    // for < 1.5Gbps data rate set Slew
    k230_dsi_write_phy_reg(0xa0, 0x40);           // set Slew rate vs DDL sr_range      *TODO case by cas

    k230_dsi_write_phy_reg(0xa4, 0x11);           // set Slew sr_osc_freq_target[6:0]   *TODO case by case

    k230_dsi_write_phy_reg(0xa4, 0x85);           // set Slew sr_osc_freq_target[11:7]  *TODO case by case
    k230_dsi_write_phy_reg(0xa3, 0x1);           // enable Slew rate calibration  (sr_sel_tester)
    k230_dsi_write_phy_reg(0x1f, 0x1);               // mpll_prog[0] = 1'b1

    k230_dsi_write_phy_reg(0x4a, 0x40);               // prg_on_lane0 = 1'b1


    reg = readl(TXPHY_BASE_ADDR + TXDPHY_CFG0);
    reg = (reg & ~(GENMASK(7, 2))) | (0x28 << 2);            //phy0 cfgclkfreqrange     6'b101000;
    reg = (reg & ~(GENMASK(13, 8))) | (0x28 << 8);          //phy1 cfgclkfreqrange      6'b101000;
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_CFG0);

    reg = readl(TXPHY_BASE_ADDR + TXDPHY_CFG0);
    reg = (reg & ~(BIT_MASK(1))) | (0 << 1);
    writel(reg, TXPHY_BASE_ADDR + TXDPHY_CFG0);            //phy1 basedir

    writel(0x4, DSI_BASE_ADDR + PHY_RSTZ);               //phy enableclk
    writel(0xd, DSI_BASE_ADDR + PHY_RSTZ);               // redundant
    writel(0xf, DSI_BASE_ADDR + PHY_RSTZ);

    k230_dsi_write_phy_reg(0x3, 0x80);              // monitor phy fsm

    while (readl(DSI_BASE_ADDR + PHY_TST_CTRL1) != 0x580)  //0x580
    {
        k230_dsi_write_phy_reg(0x03, 0x80);
        msleep(1);
    }

    // printf("phy1 config done \n");
}


void k230_dsi_config_4lan_phy(uint32_t m, uint32_t n, uint8_t vco, uint8_t hsfreq)
{
    uint32_t reg = 0;

    // vo_debug_trace("m is %d n is %d vco is %x hsfreq is %x \n", m, n, vco, hsfreq);

    k230_dsi_phy0_config(m, n, vco, hsfreq);
    k230_dsi_phy1_config(hsfreq);


    // printf("=== wait_for_PHY0_PWRUP() begin=== \n");
    reg = readl(TXPHY_BASE_ADDR + TXDPHY_CFG1); // rdata = 0x0
    writel(0x0, TXPHY_BASE_ADDR + TXDPHY_CFG1);
    writel(0xd, DSI_BASE_ADDR + PHY_RSTZ);
    writel(0xf, DSI_BASE_ADDR + PHY_RSTZ);

    while (readl(DSI_BASE_ADDR + PHY_STATUS) != 0x1fbd);

    msleep(1);
    /* To postpone HS request,  for MIPI spec: First STOP_STATE time should be greater than T_INIT time at least 100us */
    writel(0x1, DSI_BASE_ADDR + LPCLK_CTRL);

    //-------- display_dut_cfg_seq_dsi::wait_for_PHY1_PWRUP() -------//
    // printf("=== wait_for_PHY1_PWRUP() begin=== \n");
    reg = readl(TXPHY_BASE_ADDR + TXDPHY_CFG1); // rdata = 0x0
    writel(0x400000, TXPHY_BASE_ADDR + TXDPHY_CFG1);

    while (readl(DSI_BASE_ADDR + PHY_STATUS) != 0x1fbd);

    reg = readl(DSI_BASE_ADDR + PHY_STATUS); // rdata = 0x1fbd

}



uint32_t dwc_mipi_phy_config(k_vo_mipi_phy_attr *phy)
{
#if 0
    // 下边的是和 phy 有关的，以后在研究
    // writel(0x320068, DSI_BASE_ADDR + 0x9c);
    // writel(0x2e0080, DSI_BASE_ADDR + 0x98);
    // writel(0xffffffff, DSI_BASE_ADDR + 0xc4);
    // writel(0xffffffff, DSI_BASE_ADDR + 0xc8);
    writel(0xf, DSI_BASE_ADDR + 0xa0);   ///0x04
    // writel(0x0, DSI_BASE_ADDR + 0x04);
#else

    writel(0x0, DSI_BASE_ADDR + 0xc);

    k230_dsi_config_4lan_phy(phy->m, phy->n, phy->voc, phy->hs_freq);
#endif
    return 0;
}
