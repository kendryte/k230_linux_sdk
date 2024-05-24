// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022, Canaan Bright Sight Co., Ltd
 *
 * All enquiries to https://www.canaan-creative.com/
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>


struct canaan_panel_instr {
	char cmd;
	char data;
};

struct canaan_panel_desc {
	const struct canaan_panel_instr *init;
	const size_t init_length;
	//const struct drm_display_mode *mode;
};

struct panel_cmd_header {
	u8 data_type;
	u8 delay;
	u8 payload_length;
} __packed;

struct panel_cmd_desc {
	struct panel_cmd_header header;
	u8 *payload;
};

struct panel_cmd_seq {
	struct panel_cmd_desc *cmds;
	unsigned int cmd_cnt;
};


struct canaan_panel {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	const struct canaan_panel_desc	*desc;

	struct regulator *power;
	struct gpio_desc *reset;
	struct gpio_desc *power_on;
	struct videomode vm;
	u32 width_mm;
	u32 height_mm;
	u32 lan_num;
	u32 init_set_v1_flag;

	struct panel_cmd_seq *init_seq_v1;
};


// static const struct canaan_panel_instr hx8399_init[] = {
	
// };


static inline struct canaan_panel *panel_to_canaan_panel(struct drm_panel *panel)
{
	return container_of(panel, struct canaan_panel, panel);
}

static void panel_simple_sleep(unsigned int msec)
{
	if (msec > 20)
		msleep(msec);
	else
		usleep_range(msec * 1000, (msec + 1) * 1000);

}

static int panel_simple_xfer_dsi_cmd_seq(struct canaan_panel *panel,
				     struct panel_cmd_seq *seq)
{
	
	struct device *dev = panel->panel.dev;
	struct mipi_dsi_device *dsi = panel->dsi;
	unsigned int i;
	int err;

	if (!seq)
		return -EINVAL;

	for (i = 0; i < seq->cmd_cnt; i++) {
		struct panel_cmd_desc *cmd = &seq->cmds[i];

		switch (cmd->header.data_type) {
			case MIPI_DSI_DCS_SHORT_WRITE:
			case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
				// printk("panel_simple_xfer_dsi_cmd_seq short package \n");
				err = mipi_dsi_dcs_write_buffer(dsi, cmd->payload,
								cmd->header.payload_length);
				break;
			case MIPI_DSI_DCS_LONG_WRITE:
				// printk("panel_simple_xfer_dsi_cmd_seq  long package \n");
				err = mipi_dsi_dcs_write_buffer(dsi, cmd->payload,
								cmd->header.payload_length);
				break;
			default:
				return -EINVAL;
		}

		if (err < 0)
			dev_err(dev, "failed to write dcs cmd: %d\n", err);

		if (cmd->header.delay)
			panel_simple_sleep(cmd->header.delay);
	}
//	//printk("panel_simple_xfer_dsi_cmd_seq  end \n");
	return 0;
}

static int canaan_panel_prepare(struct drm_panel *panel)
{
	struct canaan_panel *p = panel_to_canaan_panel(panel);
	// int err;

	// set power on 
	if(!IS_ERR(p->power_on))
	{
		gpiod_direction_output(p->power_on, 1);
		gpiod_set_value_cansleep(p->power_on, 1);
	}
	// set rst
	if(!IS_ERR(p->reset))
	{
		gpiod_direction_output(p->reset, 1);
		
		gpiod_set_value_cansleep(p->reset, 1);
		panel_simple_sleep(200);
		gpiod_set_value_cansleep(p->reset, 0);
		panel_simple_sleep(200);
		gpiod_set_value_cansleep(p->reset, 1);
		panel_simple_sleep(200);
	}

	if(p->init_set_v1_flag)
	{
		// config screen
		panel_simple_xfer_dsi_cmd_seq(p, p->init_seq_v1);
	}
	printk("%s done\n", __func__);
	return 0;
}

static int canaan_panel_enable(struct drm_panel *panel)
{
	
	return 0;
}

static int canaan_panel_disable(struct drm_panel *panel)
{
	return 0;
}

static int canaan_panel_unprepare(struct drm_panel *panel)
{
	struct canaan_panel *p = panel_to_canaan_panel(panel);
	// int err = 0;

	if(p->power_on)
	{
		printk("power off is ok \n");
		gpiod_set_value_cansleep(p->power_on, 0);
	}
	if(p->reset)
	{
		printk("power off reset is ok \n");
		gpiod_set_value_cansleep(p->reset, 0);
	}

	panel_simple_sleep(500);

	return 0;
}

static int canaan_panel_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct canaan_panel *ctx = panel_to_canaan_panel(panel);
	struct drm_display_mode *mode;

	mode = drm_mode_create(connector->dev);
	if (!mode) {
		dev_err(panel->dev, "failed to create a new display mode\n");
		return 0;
	}

	drm_display_mode_from_videomode(&ctx->vm, mode);
	mode->width_mm = ctx->width_mm;
	mode->height_mm = ctx->height_mm;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs canaan_panel_funcs = {
	.prepare	= canaan_panel_prepare,
	.unprepare	= canaan_panel_unprepare,
	.enable		= canaan_panel_enable,
	.disable	= canaan_panel_disable,
	.get_modes	= canaan_panel_get_modes,
};


static int panel_simple_parse_cmd_seq(struct device *dev,
				      const u8 *data, int length,
				      struct panel_cmd_seq *seq)
{
	struct panel_cmd_header *header;
	struct panel_cmd_desc *desc;
	char *buf, *d;
	unsigned int i, cnt, len;

	if (!seq)
		return -EINVAL;

	buf = devm_kmemdup(dev, data, length, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	d = buf;
	len = length;
	cnt = 0;
	while (len > sizeof(*header)) {
		header = (struct panel_cmd_header *)d;

		d += sizeof(*header);
		len -= sizeof(*header);

		if (header->payload_length > len)
			return -EINVAL;

		d += header->payload_length;
		len -= header->payload_length;
		cnt++;
	}

	if (len)
		return -EINVAL;

	seq->cmd_cnt = cnt;
	seq->cmds = devm_kcalloc(dev, cnt, sizeof(*desc), GFP_KERNEL);
	if (!seq->cmds)
		return -ENOMEM;

	d = buf;
	len = length;
	for (i = 0; i < cnt; i++) {
		header = (struct panel_cmd_header *)d;
		len -= sizeof(*header);
		d += sizeof(*header);

		desc = &seq->cmds[i];
		desc->header = *header;
		desc->payload = d;

		d += header->payload_length;
		len -= header->payload_length;
	}

	return 0;
}

static int canaan_panel_parse_dt(struct canaan_panel *ctx)
{
	struct device *dev = &ctx->dsi->dev;
	struct device_node *np = dev->of_node;
	const void *data;
	int ret, err;
	int len;

	// get diplay timing
	ret = of_get_videomode(np, &ctx->vm, 0);
	if (ret < 0)
		return ret;

	// get width && heidth
	of_property_read_u32(np, "panel-width-mm", &ctx->width_mm);
	of_property_read_u32(np, "panel-height-mm", &ctx->height_mm);
	of_property_read_u32(np, "panel-dsi-lane", &ctx->lan_num);
	
	// get screen data
	data = of_get_property(np, "panel-init-sequence", &len);
	if (data) {
		ctx->init_seq_v1 = devm_kzalloc(dev, sizeof(*ctx->init_seq_v1), GFP_KERNEL);
		if (!ctx->init_seq_v1)
			return -ENOMEM;

		err = panel_simple_parse_cmd_seq(dev, data, len, ctx->init_seq_v1);
		if (err) {
			dev_err(dev, "failed to parse init sequence v1\n");
			return err;
		}
		ctx->init_set_v1_flag = 1;
	}

	printk("panel-dsi-lane is %d \n", ctx->lan_num);
	printk("panel-height-mm is %d \n", ctx->height_mm);
	printk("ctx->init_seq_v1 cmd_cnt is %d  cmd is %d \n", ctx->init_seq_v1->cmd_cnt, ctx->init_seq_v1->cmds->header.data_type);

	return 0;
}

static int canaan_panel_dsi_probe(struct mipi_dsi_device *dsi)
{
	struct canaan_panel *ctx;
	int ret;

	ctx = devm_kzalloc(&dsi->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->init_set_v1_flag = 0;

	mipi_dsi_set_drvdata(dsi, ctx);
	
	ctx->reset = devm_gpiod_get(&dsi->dev, "dsi_reset", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->reset)) {
		dev_err(&dsi->dev, "Couldn't get our reset GPIO, error: %ld\n", PTR_ERR(ctx->reset));
		// return PTR_ERR(ctx->reset);
	}

	ctx->power_on = devm_gpiod_get(&dsi->dev, "backlight_gpio", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->power_on)) {
		dev_err(&dsi->dev, "Couldn't get our backlight_gpio GPIO, error: %ld\n", PTR_ERR(ctx->power_on));
		// return PTR_ERR(ctx->power_on);
	}

	ctx->dsi = dsi;
	ctx->desc = of_device_get_match_data(&dsi->dev);

	ret = canaan_panel_parse_dt(ctx);
	if (ret < 0)
		return ret;

	// Panel Device Tree Read..

	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_SYNC_PULSE;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->lanes = ctx->lan_num;

	drm_panel_init(&ctx->panel, &dsi->dev, &canaan_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret) {
		drm_panel_remove(&ctx->panel);
	}

	return ret;
}

static void canaan_panel_dsi_remove(struct mipi_dsi_device *dsi)
{
	struct canaan_panel *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);

	return;
}

// static const struct canaan_panel_desc hx8399_desc = {
// 	// .init = hx8399_init,
// 	// .init_length = ARRAY_SIZE(hx8399_init),
// 	// .mode = &hx8399_default_mode,
// };


static const struct of_device_id canaan_panel_of_match[] = {
	{ .compatible = "canaan,hx8399",},
	{ }
};
MODULE_DEVICE_TABLE(of, canaan_panel_of_match);

static struct mipi_dsi_driver canaan_panel_driver = {
	.probe		= canaan_panel_dsi_probe,
	.remove		= canaan_panel_dsi_remove,
	.driver = {
		.name		= "canaan-panel-dsi",
		.of_match_table	= canaan_panel_of_match,
	},
};
module_mipi_dsi_driver(canaan_panel_driver);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Canaan K230 Panel Driver");
MODULE_LICENSE("GPL");
