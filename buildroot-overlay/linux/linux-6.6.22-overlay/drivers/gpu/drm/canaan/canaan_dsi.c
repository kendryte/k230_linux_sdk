/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/clk.h>
#include <linux/component.h>
#include <linux/crc-ccitt.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy-mipi-dphy.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/slab.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include "canaan_dsi.h"
#include <video/mipi_display.h>

#define DSI_GEN_HDR				0x6c
#define DSI_GEN_PLD_DATA		0x70
#define DSI_CMD_PKT_STATUS		0x74
#define GEN_RD_CMD_BUSY			BIT(6)
#define GEN_PLD_R_FULL			BIT(5)
#define GEN_PLD_R_EMPTY			BIT(4)
#define GEN_PLD_W_FULL			BIT(3)
#define GEN_PLD_W_EMPTY			BIT(2)
#define GEN_CMD_FULL			BIT(1)
#define GEN_CMD_EMPTY			BIT(0)
#define PHY_IF_CFG              (0xa4)
#define CLKMGR_CFG              (0x08)
#define GEN_VCID                (0x30)
#define VID_PKT_SIZE            (0x3c)
#define VID_NUM_CHUNKS          (0x40)
#define VID_NULL_SIZE           (0x44)
#define VID_HSA_TIME            (0x48)
#define VID_HBP_TIME            (0x4c)
#define VID_HLINE_TIME          (0x50)
#define VID_VSA_LINES           (0x54)
#define VID_VBP_LINES           (0x58)
#define VID_VFP_LINES           (0x5c)
#define VID_VACTIVE_LINES       (0x60)
#define DSI_TO_CNT_CFG			0x78
#define HSTX_TO_CNT(p)			(((p) & 0xffff) << 16)
#define LPRX_TO_CNT(p)			((p) & 0xffff)
#define DSI_BTA_TO_CNT			0x8c
#define MODE_CFG                (0x34)
#define VID_MODE_CFG            (0x38)
#define CMD_MODE_CFG            (0x68)
#define DPI_COLOR_CODING        (0x10)
#define LPCLK_CTRL              (0x94)
#define PCKHDL_CFG              (0x2c)

#define CMD_PKT_STATUS_TIMEOUT_US	20000

#define TXPHY_445_5_M           (295)
#define TXPHY_445_5_N           (15)
#define TXPHY_445_5_VOC         (0x17)
#define TXPHY_445_5_HS_FREQ     (0x96)

#define TXPHY_891_M             (165)
#define TXPHY_891_N             (8)
#define TXPHY_891_VOC           (0x09)
#define TXPHY_891_HS_FREQ       (0x96)


static inline void dsi_write(struct canaan_dsi *dsi, u32 reg, u32 val)
{
	writel(val, dsi->base + reg);
}

static inline u32 dsi_read(struct canaan_dsi *dsi, u32 reg)
{
	return readl(dsi->base + reg);
}


static void canaan_dsi_inst_abort(struct canaan_dsi *dsi)
{
	// TODO
}

static int canaan_dsi_inst_wait_for_completion(struct canaan_dsi *dsi)
{
	// TODO
	return 0;
}

static int dw_mipi_dsi_gen_pkt_hdr_write(struct canaan_dsi *dsi, u32 hdr_val)
{
	int ret;
	u32 val;

	ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
				 val, !(val & GEN_CMD_FULL), 1000,
				 CMD_PKT_STATUS_TIMEOUT_US);
	if (ret) {
		dev_err(dsi->dev, "failed to get available command FIFO\n");
		return ret;
	}

	dsi_write(dsi, DSI_GEN_HDR, hdr_val);
	return 0;
}

static int canaan_dsi_dcs_write_short(struct canaan_dsi *dsi,
				     const struct mipi_dsi_msg *msg)
{
	uint32_t hdr_val = 0;
	uint32_t vcid = msg->channel;
	__le32 word = 0;
	const u8 *tx_buf = msg->tx_buf;

	hdr_val = (tx_buf[0] << 8) + (vcid << 6) + 0x5;
	memcpy(&word, &hdr_val, sizeof(hdr_val));
	return dw_mipi_dsi_gen_pkt_hdr_write(dsi, le32_to_cpu(word));
}

static int canaan_dsi_dcs_write_long(struct canaan_dsi *dsi,
				    const struct mipi_dsi_msg *msg)
{
	int ret = 0;
	uint32_t val;
	uint32_t hdr_val = 0;
	uint32_t len = msg->tx_len;
	uint32_t pld_data_bytes = sizeof(u32);
	uint32_t vcid = msg->channel;
	__le32 word;
	const u8 *tx_buf = msg->tx_buf;

	while (len) {
		if (len < pld_data_bytes) {
			word = 0;
			memcpy(&word, tx_buf, len);
			dsi_write(dsi, DSI_GEN_PLD_DATA, le32_to_cpu(word));
			len = 0;
		} else {
			memcpy(&word, tx_buf, pld_data_bytes);
			dsi_write(dsi, DSI_GEN_PLD_DATA, le32_to_cpu(word));
			tx_buf += pld_data_bytes;
			len -= pld_data_bytes;
		}

		ret = readl_poll_timeout(dsi->base + DSI_CMD_PKT_STATUS,
					 val, !(val & GEN_PLD_W_FULL), 1000,
					 CMD_PKT_STATUS_TIMEOUT_US);
		if (ret) {
			dev_err(dsi->dev,
				"failed to get available write payload FIFO\n");
			return ret;
		}
	}
	word = 0;
	// set cmd tpye
	hdr_val = (msg->tx_len << 8) + (vcid << 6) + 0x39;
	memcpy(&word, &hdr_val, sizeof(hdr_val));
	return dw_mipi_dsi_gen_pkt_hdr_write(dsi, le32_to_cpu(word));
}

static int canaan_dsi_dcs_read(struct canaan_dsi *dsi,
			      const struct mipi_dsi_msg *msg)
{
	// TODO
	return 1;
}

static void canaan_dsi_set_lan_num(struct canaan_dsi *dsi, int lan_num)
{
	switch(lan_num)
	{
		case 1:
			dsi_write(dsi, PHY_IF_CFG, 0x2800);
			break;
		case 2:
			dsi_write(dsi, PHY_IF_CFG, 0x2801);
			break;
		case 4:
			dsi_write(dsi, PHY_IF_CFG, 0x2803);
			break;
		default :
			printk("dsi lan num only support 1 , 2, 4 lane \n");
			break;
	}
}


static u32 canaan_dsi_get_hcomponent_lbcc(struct canaan_dsi *dsi,
					   const struct drm_display_mode *mode,
					   u32 hcomponent)
{
	u32 frac, lbcc;

	lbcc = hcomponent * dsi->phy_freq / 8;

	frac = lbcc % mode->clock;
	lbcc = lbcc / mode->clock;
	if (frac)
		lbcc++;

	return lbcc;
}


static void canaan_dsi_set_dpi_timing(struct canaan_dsi *dsi, struct drm_display_mode *mode)
{
	u32 htotal, hsa, hbp, hact, lbcc, vsa, vbp, vfp, vact;

	htotal = mode->htotal;
	hsa = mode->hsync_end - mode->hsync_start;
	hbp = mode->htotal - mode->hsync_end;
	hact = mode->hdisplay;

	vsa = mode->vsync_end - mode->vsync_start;
	vbp = mode->vtotal - mode->vsync_end;
	vact = mode->vdisplay;
	vfp = mode->vtotal - vsa - vbp - vact;

	dsi_write(dsi, VID_PKT_SIZE, hact);
	dsi_write(dsi, VID_NUM_CHUNKS, 0);
	dsi_write(dsi, VID_NULL_SIZE, 0);

	// set hsa
	lbcc = canaan_dsi_get_hcomponent_lbcc(dsi, mode, hsa);
	dsi_write(dsi, VID_HSA_TIME, lbcc);

	// set hbp
	lbcc = canaan_dsi_get_hcomponent_lbcc(dsi, mode, hbp);
	dsi_write(dsi, VID_HBP_TIME, lbcc);

	// set hline
	lbcc = canaan_dsi_get_hcomponent_lbcc(dsi, mode, htotal);
	dsi_write(dsi, VID_HLINE_TIME, lbcc);

	// set vsa
	dsi_write(dsi, VID_VSA_LINES, vsa);
	// set vbp
	dsi_write(dsi, VID_VBP_LINES, vbp);
	// set vfp
	dsi_write(dsi, VID_VFP_LINES, vfp);
	// set vline
	dsi_write(dsi, VID_VACTIVE_LINES, vact);
}

void canaan_dsi_lpdt_init(struct canaan_dsi *dsi, struct drm_display_mode *mode)
{
	// set lpdt div 
	dsi_write(dsi, CLKMGR_CFG, 0x108);
	// set vcid
	dsi_write(dsi, GEN_VCID, 0x303);
	// stt dpi tinging
	canaan_dsi_set_dpi_timing(dsi, mode);
	/*
	 * TODO dw drv improvements
	 * compute high speed transmission counter timeout according
	 * to the timeout clock division (TO_CLK_DIVISION) and byte lane...
	 */
	// dsi_write(dsi, DSI_TO_CNT_CFG, HSTX_TO_CNT(1000) | LPRX_TO_CNT(1000));
	// /*
	//  * TODO dw drv improvements
	//  * the Bus-Turn-Around Timeout Counter should be computed
	//  * according to byte lane...
	//  */
	// dsi_write(dsi, DSI_BTA_TO_CNT, 0xd00);

	dsi_write(dsi, MODE_CFG, 0x1);
	dsi_write(dsi, VID_MODE_CFG, 0xbf02);
	dsi_write(dsi, CMD_MODE_CFG, 0x10f7f01);
	dsi_write(dsi, PCKHDL_CFG, 0x1c);
	dsi_write(dsi, 0x4, 0x1);
}


static void canaan_mipi_dsi_set_dsi_enable(struct canaan_dsi *dsi)
{
	dsi_write(dsi, DPI_COLOR_CODING, 0x105);
	dsi_write(dsi, 0x9c, 0x320068);
	dsi_write(dsi, 0x98, 0x2e0080);
	dsi_write(dsi, 0xc4, 0xffffffff);
	dsi_write(dsi, 0xc8, 0xffffffff);
	dsi_write(dsi, MODE_CFG, 0x0);
	dsi_write(dsi, CMD_MODE_CFG, 0x0);
	dsi_write(dsi, LPCLK_CTRL, 0x3);
	dsi_write(dsi, LPCLK_CTRL, 0x1);

}

static void canaan_mipi_dsi_set_test_mode(struct canaan_dsi *dsi)
{
	uint32_t reg = 0;
    //1. set MODE_CFG register to enable Video mode

    //2. Configure the DPI_COLOR_CODING register.

    //3. Configure the frame using the registers

    //4. Configure the pattern generation mode
    reg = dsi_read(dsi, VID_MODE_CFG);
    reg = (reg & ~(BIT_MASK(20))) | (1 << 20);
    reg = (reg & ~(BIT_MASK(24))) | (1 << 24);
    reg = (reg & ~(BIT_MASK(16))) | (1 << 16);
	dsi_write(dsi, VID_MODE_CFG, reg);
}



static void canaan_dsi_encoder_enable(struct drm_encoder *encoder)
{
	struct canaan_dsi *dsi = encoder_to_canaan_dsi(encoder);
	struct drm_display_mode *adjusted_mode = &encoder->crtc->state->adjusted_mode;
	struct mipi_dsi_device *device = dsi->device;	
	int dsi_test_en = 0;

	DRM_DEBUG_DRIVER("Enabling DSI output\n");

	printk("canaan_dsi_encoder_enable -------------------  \n");
	switch(adjusted_mode->clock)
	{
		case 74250:
			// 74.25M
			k230_dsi_config_4lan_phy(dsi, TXPHY_445_5_M, TXPHY_445_5_N, TXPHY_445_5_VOC, TXPHY_445_5_HS_FREQ);
			// set clk todo
			dsi->phy_freq = 445500;
			dsi->clk_freq = 74250;
			break;
		case 148500:
			// 144.5M
			k230_dsi_config_4lan_phy(dsi, TXPHY_891_M, TXPHY_891_N, TXPHY_891_VOC, TXPHY_891_HS_FREQ);
			// set clk todo

			dsi->phy_freq = 890666;
			dsi->clk_freq = 148500;
			break;
		default:
			printk("mipi clk not support \n");
			break;
	}
	// set dsi lan num
	canaan_dsi_set_lan_num(dsi, device->lanes);
	// set lpdt 
	canaan_dsi_lpdt_init(dsi, adjusted_mode);

	/*
	 * Enable the DSI block.
	 */

	if (dsi->panel)
		drm_panel_prepare(dsi->panel);

	/*
	 * FIXME: This should be moved after the switch to HS mode.
	 *
	 * Unfortunately, once in HS mode, it seems like we're not
	 * able to send DCS commands anymore, which would prevent any
	 * panel to send any DCS command as part as their enable
	 * method, which is quite common.
	 *
	 * I haven't seen any artifact due to that sub-optimal
	 * ordering on the panels I've tested it with, so I guess this
	 * will do for now, until that IP is better understood.
	 */
	if (dsi->panel)
		drm_panel_enable(dsi->panel);

	//dsi enable 
	canaan_mipi_dsi_set_dsi_enable(dsi);

	if(dsi_test_en == 1)
		canaan_mipi_dsi_set_test_mode(dsi);

}

static void canaan_dsi_encoder_disable(struct drm_encoder *encoder)
{
	struct canaan_dsi *dsi = encoder_to_canaan_dsi(encoder);

	DRM_DEBUG_DRIVER("Disabling DSI output\n");

	if (dsi->panel) {
		drm_panel_disable(dsi->panel);
		drm_panel_unprepare(dsi->panel);
	}
}

static int canaan_dsi_get_modes(struct drm_connector *connector)
{
	struct canaan_dsi *dsi = connector_to_canaan_dsi(connector);

	return drm_panel_get_modes(dsi->panel, connector);
}

static const struct drm_connector_helper_funcs canaan_dsi_connector_helper_funcs = {
	.get_modes	= canaan_dsi_get_modes,
};

static enum drm_connector_status
canaan_dsi_connector_detect(struct drm_connector *connector, bool force)
{
	struct canaan_dsi *dsi = connector_to_canaan_dsi(connector);

	return dsi->panel ? connector_status_connected :
			    connector_status_disconnected;
}

static const struct drm_connector_funcs canaan_dsi_connector_funcs = {
	.detect			= canaan_dsi_connector_detect,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= drm_connector_cleanup,
	.reset			= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
};

static const struct drm_encoder_helper_funcs canaan_dsi_enc_helper_funcs = {
	.disable	= canaan_dsi_encoder_disable,
	.enable		= canaan_dsi_encoder_enable,
};

static int canaan_dsi_attach(struct mipi_dsi_host *host,
			    struct mipi_dsi_device *device)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);
	struct drm_panel *panel = of_drm_find_panel(device->dev.of_node);

	if (IS_ERR(panel))
		return PTR_ERR(panel);
	else
		dsi->connector.status = connector_status_connected;

	dsi->panel = panel;
	dsi->device = device;

	dev_info(host->dev, "Attached device %s\n", device->name);

	return 0;
}

static int canaan_dsi_detach(struct mipi_dsi_host *host,
			    struct mipi_dsi_device *device)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);

	dsi->panel = NULL;
	dsi->device = NULL;

	return 0;
}

static ssize_t canaan_dsi_transfer(struct mipi_dsi_host *host,
				  const struct mipi_dsi_msg *msg)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);
	int ret;

	ret = canaan_dsi_inst_wait_for_completion(dsi);
	if (ret < 0)
		canaan_dsi_inst_abort(dsi);

	switch (msg->type) {
		case MIPI_DSI_DCS_SHORT_WRITE:
			ret = canaan_dsi_dcs_write_short(dsi, msg);
			break;
		case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
		case MIPI_DSI_DCS_LONG_WRITE:
		case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
			ret = canaan_dsi_dcs_write_long(dsi, msg);
			break;

		case MIPI_DSI_DCS_READ:
			if (msg->rx_len == 1) {
				ret = canaan_dsi_dcs_read(dsi, msg);
				break;
			}
			fallthrough;
		default:
			ret = -EINVAL;
	}
	return ret;
}

static const struct mipi_dsi_host_ops canaan_dsi_host_ops = {
	.attach		= canaan_dsi_attach,
	.detach		= canaan_dsi_detach,
	.transfer	= canaan_dsi_transfer,
};


static int canaan_dsi_bind(struct device *dev, struct device *master,
			 void *data)
{
	struct drm_device *drm = data;
	struct canaan_dsi *dsi = dev_get_drvdata(dev);
	int ret;

	drm_encoder_helper_add(&dsi->encoder,
			       &canaan_dsi_enc_helper_funcs);
	ret = drm_simple_encoder_init(drm, &dsi->encoder,
				      DRM_MODE_ENCODER_DSI);
	if (ret) {
		dev_err(dsi->dev, "Couldn't initialise the DSI encoder\n");
		return ret;
	}
	dsi->encoder.possible_crtcs = BIT(0);

	drm_connector_helper_add(&dsi->connector,
				 &canaan_dsi_connector_helper_funcs);
	ret = drm_connector_init(drm, &dsi->connector,
				 &canaan_dsi_connector_funcs,
				 DRM_MODE_CONNECTOR_DSI);
	if (ret) {
		dev_err(dsi->dev,
			"Couldn't initialise the DSI connector\n");
		goto err_cleanup_connector;
	}

	drm_connector_attach_encoder(&dsi->connector, &dsi->encoder);

	dsi->drm = drm;

	return 0;

err_cleanup_connector:
	drm_encoder_cleanup(&dsi->encoder);
	return ret;
}

static void canaan_dsi_unbind(struct device *dev, struct device *master,
			    void *data)
{
	struct canaan_dsi *dsi = dev_get_drvdata(dev);

	dsi->drm = NULL;
}

static const struct component_ops canaan_dsi_ops = {
	.bind	= canaan_dsi_bind,
	.unbind	= canaan_dsi_unbind,
};

static int canaan_dsi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct canaan_dsi *dsi;
	struct resource *res;
	int ret;

	dsi = devm_kzalloc(dev, sizeof(*dsi), GFP_KERNEL);
	if (!dsi)
		return -ENOMEM;
	dev_set_drvdata(dev, dsi);
	dsi->dev = dev;
	dsi->host.ops = &canaan_dsi_host_ops;
	dsi->host.dev = dev;

	// DSI Device Tree Read..
	// get dsi base addr 
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    dsi->base = devm_ioremap_resource(dev, res);
    if (IS_ERR(dsi->base)) {
        dev_err(dev, "Couldn't map the DSI encoder registers\n");
        return PTR_ERR(dsi->base);
    }

	ret = mipi_dsi_host_register(&dsi->host);
	if (ret) {
		dev_err(dev, "Couldn't register MIPI-DSI host\n");
		goto err_unprotect_clk;
	}

	ret = component_add(&pdev->dev, &canaan_dsi_ops);
	if (ret) {
		dev_err(dev, "Couldn't register our component\n");
		goto err_remove_dsi_host;
	}

	return 0;

err_remove_dsi_host:
	mipi_dsi_host_unregister(&dsi->host);
err_unprotect_clk:
	clk_rate_exclusive_put(dsi->mod_clk);
// err_attach_clk:
// 	if (!IS_ERR(dsi->bus_clk))
// 		regmap_mmio_detach_clk(dsi->regs);
	return ret;
}

static int canaan_dsi_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id canaan_dsi_of_table[] = {
	{ .compatible = "canaan,k230-mipi-dsi" },
	{ }
};
MODULE_DEVICE_TABLE(of, canaan_dsi_of_table);

struct platform_driver canaan_dsi_driver = {
	.probe		= canaan_dsi_probe,
	.remove		= canaan_dsi_remove,
	.driver		= {
		.name		= "canaan-mipi-dsi",
		.of_match_table	= canaan_dsi_of_table,
	},
};


MODULE_AUTHOR("");
MODULE_DESCRIPTION("Canaan K230 DSI Driver");
MODULE_LICENSE("GPL");
