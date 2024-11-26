/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_graph.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>

#include "linux/dev_printk.h"
#include "media/v4l2-async.h"
#include "vvcam_v4l2_common.h"
#include "vvcam_isp_driver.h"
#include "vvcam_isp_event.h"
#include "vvcam_isp_ctrl.h"
#include "vvcam_isp_procfs.h"
#ifdef VVCAM_PLATFORM_REGISTER
#include "vvcam_isp_platform.h"
#endif

#ifdef VVCAM_PLATFORM_REGISTER

#if 1
#define VVCAM_ISP_DEFAULT_SENSOR        "ov5647"
#define VVCAM_ISP_DEFAULT_SENSOR_MODE   0
#define VVCAM_ISP_DEFAULT_SENSOR_XML    "/etc/vvcam/ov5647.xml"
#define VVCAM_ISP_DEFAULT_SENSOR_MANU_JSON    "/etc/vvcam/ov5647.manual.json"
#define VVCAM_ISP_DEFAULT_SENSOR_AUTO_JSON    "/etc/vvcam/ov5647.auto.json"

#else
#define VVCAM_ISP_DEFAULT_SENSOR        "gc2093"
#define VVCAM_ISP_DEFAULT_SENSOR_MODE   0
#define VVCAM_ISP_DEFAULT_SENSOR_XML    "/etc/vvcam/gc2093.xml"
#define VVCAM_ISP_DEFAULT_SENSOR_MANU_JSON    "/etc/vvcam/gc2093.manual.json"
#define VVCAM_ISP_DEFAULT_SENSOR_AUTO_JSON    "/etc/vvcam/gc2093.auto.json"

#endif

#endif

struct vvcam_isp_mbus_fmt vvcam_isp_mp_fmts[] = {
    {
        .code = MEDIA_BUS_FMT_YUYV8_2X8,   /*NV16*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1_5X8, /*NV12*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1X16,  /*YUYV*/
    },
    {
        .code = MEDIA_BUS_FMT_BGR888_1X24, /*BGR888*/
    },
    {
        .code = MEDIA_BUS_FMT_BGR888_3X8, /*BGR888P*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV10_2X10, /*P010*/
    },
};

struct vvcam_isp_mbus_fmt vvcam_isp_sp_fmts[] = {
    {
        .code = MEDIA_BUS_FMT_YUYV8_2X8,   /*NV16*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1_5X8, /*NV12*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1X16,  /*YUYV*/
    },
    {
        .code = MEDIA_BUS_FMT_BGR888_1X24, /*RGB888*/
    },
    {
        .code = MEDIA_BUS_FMT_BGR888_3X8, /*RGB888P*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV10_2X10, /*P010*/
    },
};

static int vvcam_isp_querycap(struct v4l2_subdev *sd, void *arg)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)arg;

    strlcpy(cap->driver, sd->name, sizeof(cap->driver));
	strlcpy(cap->card, sd->name, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info),
            "platform:%s", sd->name);

	return 0;
}

static int vvcam_isp_pad_requbufs(struct v4l2_subdev *sd, void *arg)
{
    struct vvcam_pad_reqbufs *pad_requbufs = (struct vvcam_pad_reqbufs *)arg;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

    return vvcam_isp_requebus_event(isp_dev, pad_requbufs->pad, pad_requbufs->num_buffers);
}

static int vvcam_isp_pad_buf_queue(struct v4l2_subdev *sd, void *arg)
{
    struct vvcam_pad_buf *pad_buf = (struct vvcam_pad_buf *)arg;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    int ret;
    unsigned long flags;
    struct vvcam_isp_pad_data *cur_pad;

    cur_pad = &isp_dev->pad_data[pad_buf->pad];

    spin_lock_irqsave(&cur_pad->qlock, flags);

    list_add_tail(&pad_buf->buf->list, &cur_pad->queue);

	spin_unlock_irqrestore(&cur_pad->qlock, flags);

    ret = vvcam_isp_qbuf_event(isp_dev, pad_buf->pad, pad_buf->buf);

    return ret;
}

static int vvcam_isp_pad_s_stream(struct v4l2_subdev *sd, void *arg)
{
    struct vvcam_pad_stream_status *pad_stream = (struct vvcam_pad_stream_status *)arg;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    int ret;

    isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

    if (pad_stream->status == 0 ) {
        INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
    }
    ret = vvcam_isp_s_stream_event(isp_dev, pad_stream->pad, pad_stream->status);

    return ret;
}

static int vvcam_isp_buf_done(struct v4l2_subdev *sd, void *arg)
{
    struct vvcam_isp_buf ubuf;
    struct vvcam_isp_pad_data *cur_pad;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    unsigned long flags;
    struct vvcam_vb2_buffer *pos, *next;
    struct vvcam_vb2_buffer *buf = NULL;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct video_device *video;
    struct vvcam_pad_buf pad_buf;
    int ret;

    memcpy(&ubuf, arg, sizeof(struct vvcam_isp_buf));

    cur_pad = &isp_dev->pad_data[ubuf.pad];

    if (list_empty(&cur_pad->queue) || (cur_pad->stream == 0))
        return -EINVAL;

    spin_lock_irqsave(&cur_pad->qlock, flags);
    list_for_each_entry_safe(pos, next, &cur_pad->queue, list) {
        if (pos && (pos->sequence == ubuf.index)) {
           buf = pos;
           list_del(&pos->list);
           break;
        }
    }
    spin_unlock_irqrestore(&cur_pad->qlock, flags);

    if (buf) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&isp_dev->pads[ubuf.pad]);
#else
        pad = media_entity_remote_pad(&isp_dev->pads[ubuf.pad]);
#endif
        if (!pad)
            return -EINVAL;
        if (is_media_entity_v4l2_subdev(pad->entity)) {

            subdev = media_entity_to_v4l2_subdev(pad->entity);
            memset(&pad_buf, 0, sizeof(pad_buf));
            pad_buf.pad = pad->index;
            pad_buf.buf = buf;
            ret = v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_BUF_DONE, &pad_buf);
            if (ret)
                return ret;

        } else if (is_media_entity_v4l2_video_device(pad->entity)){
            video = media_entity_to_video_device(pad->entity);
            if (buf->sequence < video->queue->num_buffers) {
                if (buf->vb.vb2_buf.state == VB2_BUF_STATE_ACTIVE) {
                    vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);
                }
            }
        }
    }

    return 0;
}

static int vvcam_isp_queryctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_queryctrl *pad_querctrl =
                                    (struct vvcam_pad_queryctrl *)arg;
    ret = v4l2_queryctrl(&isp_dev->ctrl_handler, pad_querctrl->query_ctrl);

    return ret;
}

static int vvcam_isp_query_ext_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_query_ext_ctrl *pad_quer_ext_ctrl =
                                    (struct vvcam_pad_query_ext_ctrl *)arg;
    ret = v4l2_query_ext_ctrl(&isp_dev->ctrl_handler,
                        pad_quer_ext_ctrl->query_ext_ctrl);

    return ret;
}

static int vvcam_isp_querymenu(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_querymenu *pad_quermenu =
                                    (struct vvcam_pad_querymenu *)arg;
    ret = v4l2_querymenu(&isp_dev->ctrl_handler,
                        pad_quermenu->querymenu);

    return ret;
}

static int vvcam_isp_g_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_control *pad_ctrl = (struct vvcam_pad_control *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ctrl->pad;
    ret = v4l2_g_ctrl(&isp_dev->ctrl_handler, pad_ctrl->control);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_s_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_control *pad_ctrl = (struct vvcam_pad_control *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ctrl->pad;
    ret = v4l2_s_ctrl(NULL, &isp_dev->ctrl_handler, pad_ctrl->control);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_g_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ext_ctrls->pad;
    ret = v4l2_g_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_s_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ext_ctrls->pad;
    ret = v4l2_s_ext_ctrls(NULL, &isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_try_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;
    ret = v4l2_try_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);

    return ret;
}

static long vvcam_isp_priv_ioctl(struct v4l2_subdev *sd,
                                unsigned int cmd, void *arg)
{
    int ret = -EINVAL;
    switch (cmd) {
        case VIDIOC_QUERYCAP:
            ret = vvcam_isp_querycap(sd, arg);
            break;
        case VVCAM_PAD_REQUBUFS:
            ret = vvcam_isp_pad_requbufs(sd, arg);
            break;
        case VVCAM_PAD_BUF_QUEUE:
            ret = vvcam_isp_pad_buf_queue(sd, arg);
            break;
        case VVCAM_PAD_S_STREAM:
            ret = vvcam_isp_pad_s_stream(sd, arg);
            break;
        case VVCAM_ISP_IOC_BUFDONE:
            ret = vvcam_isp_buf_done(sd, arg);
            break;
        case VVCAM_PAD_QUERYCTRL:
            ret = vvcam_isp_queryctrl(sd, arg);
            break;
        case VVCAM_PAD_QUERY_EXT_CTRL:
            ret = vvcam_isp_query_ext_ctrl(sd, arg);
            break;
        case VVCAM_PAD_G_CTRL:
            ret = vvcam_isp_g_ctrl(sd, arg);
            break;
        case VVCAM_PAD_S_CTRL:
            ret = vvcam_isp_s_ctrl(sd, arg);
            break;
        case VVCAM_PAD_G_EXT_CTRLS:
            ret = vvcam_isp_g_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_S_EXT_CTRLS:
            ret = vvcam_isp_s_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_TRY_EXT_CTRLS:
            ret = vvcam_isp_try_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_QUERYMENU:
            ret = vvcam_isp_querymenu(sd, arg);
            break;
        default:
            break;
    }

    return ret;
}

int vvcam_isp_subscribe_event(struct v4l2_subdev *sd,
			                struct v4l2_fh *fh,
			                struct v4l2_event_subscription *sub)
{
    switch (sub->type) {
        case V4L2_EVENT_CTRL:
            return v4l2_ctrl_subdev_subscribe_event(sd, fh, sub);
        case VVCAM_ISP_DEAMON_EVENT:
            return v4l2_event_subscribe(fh, sub, 2, NULL);
        default:
            return -EINVAL;
    }

}

static struct v4l2_subdev_core_ops vvcam_isp_core_ops = {
	.ioctl             = vvcam_isp_priv_ioctl,
	.subscribe_event   = vvcam_isp_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static struct v4l2_subdev_video_ops vvcam_isp_video_ops = {
	/*.s_stream = vvcam_isp_s_stream,*/
};

static int vvcam_isp_set_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    uint32_t w, h;
    uint32_t sink_pad_index;
    struct vvcam_isp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
    struct vvcam_isp_pad_data *sink_pad;
    struct vvcam_isp_pad_data *source_pad;
    int i;
    int ret;

    sink_pad_index = format->pad - (format->pad % VVCAM_ISP_PORT_PAD_NR);
    sink_pad = &isp_dev->pad_data[sink_pad_index];

    if (sink_pad == cur_pad) {
        cur_pad->sink_detected = 1;
        cur_pad->format = format->format;
        for (i = 1; i < VVCAM_ISP_PORT_PAD_NR; i++) {
            source_pad = &isp_dev->pad_data[sink_pad_index + i];
            source_pad->sink_detected = 1;

            switch (i) {
                case VVCAM_ISP_PORT_PAD_SOURCE_MP:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP1:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP2:
                    source_pad->format = format->format;
                    source_pad->format.code = source_pad->mbus_fmt[0].code;
                    source_pad->format.field = V4L2_FIELD_NONE;
                    source_pad->format.quantization = V4L2_QUANTIZATION_DEFAULT;
                    source_pad->format.colorspace = V4L2_COLORSPACE_DEFAULT;
                    break;
                case VVCAM_ISP_PORT_PAD_SOURCE_RAW:
                    source_pad->format = format->format;
                    source_pad->mbus_fmt[0].code = format->format.code;
                    break;
                default:
                    break;
            }
        }
        return 0;
    }

    w = ALIGN(format->format.width, VVCAM_ISP_WIDTH_ALIGN);
    h = ALIGN(format->format.height, VVCAM_ISP_HEIGHT_ALIGN);
    w = clamp_t(uint32_t, w, VVCAM_ISP_WIDTH_MIN, sink_pad->format.width);
    h = clamp_t(uint32_t, h, VVCAM_ISP_HEIGHT_MIN, sink_pad->format.height);

    format->format.width = w;
    format->format.height = h;

    for (i = 0; i < cur_pad->num_formats; i++) {
        if (format->format.code == cur_pad->mbus_fmt[i].code)
            break;
    }

    if (i >= cur_pad->num_formats) {
        format->format.code = cur_pad->mbus_fmt[0].code;
    }

    ret = vvcam_isp_set_fmt_event(isp_dev, format->pad, &format->format);
    if (ret)
        return ret;

    cur_pad->format = format->format;

    return 0;
}

static int vvcam_isp_get_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_isp_pad_data *pad_data = &isp_dev->pad_data[format->pad];

    if (pad_data->sink_detected) {
        format->format = pad_data->format;
    } else {
        return -EINVAL;
    }

    return 0;
}

static int vvcam_isp_enum_mbus_code(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
            struct v4l2_subdev_mbus_code_enum *code)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_isp_pad_data *pad_data = &isp_dev->pad_data[code->pad];

    if (code->index >= pad_data->num_formats) {
        return -EINVAL;
    }

    code->code = pad_data->mbus_fmt[code->index].code;

    return 0;
}

static const struct v4l2_subdev_pad_ops vvcam_isp_pad_ops = {
	.set_fmt        = vvcam_isp_set_fmt,
    .get_fmt        = vvcam_isp_get_fmt,
    .enum_mbus_code = vvcam_isp_enum_mbus_code,
};

struct v4l2_subdev_ops vvcam_isp_subdev_ops = {
	.core  = &vvcam_isp_core_ops,
	.video = &vvcam_isp_video_ops,
	.pad   = &vvcam_isp_pad_ops,
};

static int vvcam_isp_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt++;
	pm_runtime_get_sync(sd->dev);

	mutex_unlock(&isp_dev->mlock);
	return 0;
}

static int vvcam_isp_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt--;
	pm_runtime_put_sync(sd->dev);

	mutex_unlock(&isp_dev->mlock);

	return 0;
}


static struct v4l2_subdev_internal_ops vvcam_isp_internal_ops = {
	.open  = vvcam_isp_open,
	.close = vvcam_isp_close,
};

static int vvcam_isp_link_setup(struct media_entity *entity,
		const struct media_pad *local,
		const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations vvcam_isp_entity_ops = {
	.link_setup     = vvcam_isp_link_setup,
	.link_validate  = v4l2_subdev_link_validate,
	.get_fwnode_pad = v4l2_subdev_get_fwnode_pad_1_to_1,

};

static int vvcam_isp_notifier_bound(struct v4l2_async_notifier *notifier,
		                            struct v4l2_subdev *sd,
		                            struct v4l2_async_connection *asc)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev = container_of(notifier,
			struct vvcam_isp_dev, notifier);
    struct device *dev =  isp_dev->dev;

    struct fwnode_handle *ep = NULL;
	struct v4l2_fwnode_link link;
	struct media_entity *source, *sink;
	unsigned int source_pad, sink_pad;

    while(1) {
        ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep);
        if (!ep)
			break;

        ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "failed to parse link for %pOF: %d\n",
                    to_of_node(ep), ret);
			continue;
		}

        if (sd->entity.pads[link.local_port].flags == MEDIA_PAD_FL_SINK)
			continue;

        source     = &sd->entity;
		source_pad = link.remote_port;
		sink       = &isp_dev->sd.entity;
		sink_pad   = link.local_port;
		v4l2_fwnode_put_link(&link);
		ret = media_create_pad_link(source, source_pad,
				sink, sink_pad, MEDIA_LNK_FL_ENABLED);
        if (ret) {
			dev_err(dev, "failed to create %s:%u -> %s:%u link\n",
				source->name, source_pad,
				sink->name, sink_pad);
			break;
		}

    }

    fwnode_handle_put(ep);

	return ret;
}

static void vvcam_isp_notifier_unbound(struct v4l2_async_notifier *notifier,
		                            struct v4l2_subdev *sd,
		                            struct v4l2_async_connection *asc)
{
	return;
}

static const struct v4l2_async_notifier_operations vvcam_isp_notify_ops = {
	.bound    = vvcam_isp_notifier_bound,
	.unbind   = vvcam_isp_notifier_unbound,
};

static int vvcam_isp_async_notifier(struct vvcam_isp_dev *isp_dev)
{
    struct fwnode_handle *ep;
	struct fwnode_handle *remote_ep;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    struct v4l2_async_connection *asc;
#else
	struct v4l2_async_subdev asd;
#endif
    struct device *dev = isp_dev->dev;
    int ret = 0;
    int pad = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
    v4l2_async_subdev_nf_init(&isp_dev->notifier, &isp_dev->sd);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    v4l2_async_nf_init(&isp_dev->notifier);
#else
    v4l2_async_notifier_init(&isp_dev->notifier);
#endif

    isp_dev->notifier.ops = &vvcam_isp_notify_ops;

    if (dev_fwnode(isp_dev->dev) == NULL)
        return 0;

    for (pad = 0; pad < VVCAM_ISP_PAD_NR; pad++) {

        if (isp_dev->pads[pad].flags != MEDIA_PAD_FL_SINK)
            continue;

        ep = fwnode_graph_get_endpoint_by_id(dev_fwnode(dev),
                                        pad, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
        if (!ep)
            continue;
        remote_ep = fwnode_graph_get_remote_endpoint(ep);
        if (!remote_ep) {
            fwnode_handle_put(ep);
            continue;
        }
        fwnode_handle_put(remote_ep);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
        asc = v4l2_async_nf_add_fwnode_remote(&isp_dev->notifier,
                                            ep, struct v4l2_async_connection);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        asd = v4l2_async_nf_add_fwnode_remote(&isp_dev->notifier,
                                            ep, struct v4l2_async_subdev);
#else
        asd = v4l2_async_notifier_add_fwnode_remote_subdev(&isp_dev->notifier,
                                                ep, struct v4l2_async_subdev);
#endif

        fwnode_handle_put(ep);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
        if (IS_ERR(asc)) {
            ret = PTR_ERR(asc);
#else
        if (IS_ERR(asd)) {
            ret = PTR_ERR(asd);
#endif
			if (ret != -EEXIST) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
                v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
				return ret;
			}
        }
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
    ret = v4l2_async_nf_register(&isp_dev->notifier);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    ret = v4l2_async_subdev_nf_register(&isp_dev->sd,
						  &isp_dev->notifier);
#else
    ret = v4l2_async_subdev_notifier_register(&isp_dev->sd,
						  &isp_dev->notifier);
#endif
    if (ret) {
        dev_err(isp_dev->dev, "Async notifier register error\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
		v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
    }

    return ret;
}

static int vvcam_isp_pads_init(struct vvcam_isp_dev *isp_dev)
{
    int pad = 0;

    for (pad = 0; pad < VVCAM_ISP_PAD_NR; pad++) {
        if ((pad % VVCAM_ISP_PORT_PAD_NR) == VVCAM_ISP_PORT_PAD_SINK) {
            isp_dev->pads[pad].flags = MEDIA_PAD_FL_SINK;
        } else {
            isp_dev->pads[pad].flags = MEDIA_PAD_FL_SOURCE;
        }

        switch (pad % VVCAM_ISP_PORT_PAD_NR) {
            case VVCAM_ISP_PORT_PAD_SINK:
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_MP:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_mp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_mp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_SP1:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_sp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_sp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_SP2:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_sp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_sp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_RAW:
                isp_dev->pad_data[pad].num_formats = 1;
                isp_dev->pad_data[pad].mbus_fmt = devm_kzalloc(isp_dev->dev,
		                sizeof(struct vvcam_isp_mbus_fmt), GFP_KERNEL);;
                break;
            default:
                break;
        }

        INIT_LIST_HEAD(&isp_dev->pad_data[pad].queue);
        spin_lock_init(&isp_dev->pad_data[pad].qlock);
    }

    return 0;
}

static int vvcam_isp_parse_params(struct vvcam_isp_dev *isp_dev,
                        struct platform_device *pdev)
{

#ifdef VVCAM_PLATFORM_REGISTER
    int port = 0;
    isp_dev->id  = pdev->id;
    for (port = 0; port < VVCAM_ISP_PORT_NR; port++) {
        strncpy(isp_dev->sensor_info[port].sensor, VVCAM_ISP_DEFAULT_SENSOR,
            strlen(VVCAM_ISP_DEFAULT_SENSOR));
        strncpy(isp_dev->sensor_info[port].xml, VVCAM_ISP_DEFAULT_SENSOR_XML,
            strlen(VVCAM_ISP_DEFAULT_SENSOR_XML));
        isp_dev->sensor_info[port].mode = VVCAM_ISP_DEFAULT_SENSOR_MODE;
        strncpy(isp_dev->sensor_info[port].manu_json, VVCAM_ISP_DEFAULT_SENSOR_MANU_JSON,
            strlen(VVCAM_ISP_DEFAULT_SENSOR_MANU_JSON));
        strncpy(isp_dev->sensor_info[port].auto_json, VVCAM_ISP_DEFAULT_SENSOR_AUTO_JSON,
            strlen(VVCAM_ISP_DEFAULT_SENSOR_AUTO_JSON));
    }
#else
    fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &isp_dev->id);
#endif
    return 0;
}

#ifdef VVCAM_PLATFORM_REGISTER
struct v4l2_subdev *g_vvcam_isp_subdev[VVCAM_ISP_DEV_MAX] = {NULL};
EXPORT_SYMBOL_GPL(g_vvcam_isp_subdev);
#endif

static int vvcam_isp_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct vvcam_isp_dev *isp_dev;
    int ret;

    dev_info(dev, "%s\n", __func__);

    isp_dev = devm_kzalloc(&pdev->dev,
		        sizeof(struct vvcam_isp_dev), GFP_KERNEL);
    if (!isp_dev)
        return -ENOMEM;

    mutex_init(&isp_dev->mlock);
    mutex_init(&isp_dev->ctrl_lock);
    isp_dev->dev = &pdev->dev;
    platform_set_drvdata(pdev, isp_dev);

    ret = vvcam_isp_parse_params(isp_dev, pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to parse params\n");
        return -EINVAL;
    }

    v4l2_subdev_init(&isp_dev->sd, &vvcam_isp_subdev_ops);
	snprintf(isp_dev->sd.name, V4L2_SUBDEV_NAME_SIZE,
		"%s.%d",VVCAM_ISP_NAME, isp_dev->id);

    isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
    isp_dev->sd.dev =  &pdev->dev;
    isp_dev->sd.owner = THIS_MODULE;
    isp_dev->sd.internal_ops = &vvcam_isp_internal_ops;
    isp_dev->sd.entity.ops = &vvcam_isp_entity_ops;
    isp_dev->sd.entity.function = MEDIA_ENT_F_IO_V4L;
    isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
    isp_dev->sd.entity.name = isp_dev->sd.name;
    v4l2_set_subdevdata(&isp_dev->sd, isp_dev);

    vvcam_isp_pads_init(isp_dev);
    ret = media_entity_pads_init(&isp_dev->sd.entity,
                                VVCAM_ISP_PAD_NR, isp_dev->pads);
    if (ret)
       return ret;

    ret = vvcam_isp_async_notifier(isp_dev);
    if (ret)
        goto err_async_notifier;
#ifdef VVCAM_PLATFORM_REGISTER
	isp_dev->sd.fwnode = &isp_dev->fwnode;
    g_vvcam_isp_subdev[isp_dev->id] = &isp_dev->sd;
#endif
    ret = v4l2_async_register_subdev(&isp_dev->sd);
    if (ret) {
		dev_err(dev, "register subdev error\n");
		goto error_regiter_subdev;
	}

    ret = vvcam_isp_procfs_register(isp_dev, &isp_dev->pde);
    if (ret) {
        dev_err(dev, "register procfs failed.\n");
        goto err_register_procfs;
    }

    isp_dev->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 3);
    isp_dev->event_shm.size = PAGE_SIZE * 8;
    memset(isp_dev->event_shm.virt_addr, 0, isp_dev->event_shm.size);
    isp_dev->event_shm.phy_addr = virt_to_phys(isp_dev->event_shm.virt_addr);
    mutex_init(&isp_dev->event_shm.event_lock);

    pm_runtime_enable(&pdev->dev);
    vvcam_isp_ctrl_init(isp_dev);

    dev_info(&pdev->dev, "vvcam isp driver probe success\n");

    return 0;

err_register_procfs:
    v4l2_async_unregister_subdev(&isp_dev->sd);

error_regiter_subdev:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
    v4l2_async_nf_cleanup(&isp_dev->notifier);
    v4l2_async_nf_unregister(&isp_dev->notifier);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    v4l2_async_nf_unregister(&isp_dev->notifier);
    v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
    v4l2_async_notifier_unregister(&isp_dev->notifier);
    v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
err_async_notifier:
    media_entity_cleanup(&isp_dev->sd.entity);

    return ret;
}

static int vvcam_isp_remove(struct platform_device *pdev)
{
    struct vvcam_isp_dev *isp_dev;

    isp_dev = platform_get_drvdata(pdev);

    vvcam_isp_procfs_unregister(isp_dev->pde);
    v4l2_async_unregister_subdev(&isp_dev->sd);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
    v4l2_async_nf_cleanup(&isp_dev->notifier);
    v4l2_async_nf_unregister(&isp_dev->notifier);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    v4l2_async_nf_unregister(&isp_dev->notifier);
    v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
    v4l2_async_notifier_unregister(&isp_dev->notifier);
    v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
    media_entity_cleanup(&isp_dev->sd.entity);
    pm_runtime_disable(&pdev->dev);
    free_pages((unsigned long)isp_dev->event_shm.virt_addr, 3);
    vvcam_isp_ctrl_destroy(isp_dev);
    dev_info(&pdev->dev, "vvcam isp driver remove\n");

    return 0;
}

static int vvcam_isp_system_suspend(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_suspend(dev);
    if (ret) {
        dev_err(dev, "force suspend %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_system_resume(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_resume(dev);
    if (ret) {
        dev_err(dev, "force resume %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_runtime_suspend(struct device *dev)
{
    return 0;
}

static int vvcam_isp_runtime_resume(struct device *dev)
{
    return 0;
}

static const struct dev_pm_ops vvcam_isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(vvcam_isp_system_suspend, vvcam_isp_system_resume)
	SET_RUNTIME_PM_OPS(vvcam_isp_runtime_suspend, vvcam_isp_runtime_resume, NULL)
};

static const struct of_device_id vvcam_isp_of_match[] = {
	{.compatible = "verisilicon,isp-v4l2",},
	{ /* sentinel */ },
};

static struct platform_driver vvcam_isp_driver = {
	.probe  = vvcam_isp_probe,
	.remove = vvcam_isp_remove,
	.driver = {
		.name           = VVCAM_ISP_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = vvcam_isp_of_match,
        .pm             = &vvcam_isp_pm_ops,
	}
};

static int __init vvcam_isp_init_module(void)
{
    int ret;
    ret = platform_driver_register(&vvcam_isp_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register isp driver\n");
        return ret;
    }

#ifdef VVCAM_PLATFORM_REGISTER
    ret = vvcam_isp_platform_device_register();
    if (ret) {
		platform_driver_unregister(&vvcam_isp_driver);
		printk(KERN_ERR "Failed to register vvcam isp platform devices\n");
		return ret;
	}
#endif

    return ret;
}

static void __exit vvcam_isp_exit_module(void)
{
    platform_driver_unregister(&vvcam_isp_driver);
#ifdef VVCAM_PLATFORM_REGISTER
    vvcam_isp_platform_device_unregister();
#endif
}

module_init(vvcam_isp_init_module);
module_exit(vvcam_isp_exit_module);

MODULE_DESCRIPTION("Verisilicon isp v4l2 driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
