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
#include <linux/mod_devicetable.h>
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
#include "vvcam_video_register.h"
#include "vvcam_v4l2_common.h"
#include "vvcam_video_event.h"

#ifndef V4L2_PIX_FMT_P010
#define V4L2_PIX_FMT_P010 v4l2_fourcc('P', '0', '1', '0') /* 24 Y/CbCr 4:2:0 10-bit per component */
#endif

static struct vvcam_video_fmt_info vvcam_formats_info[] = {
    {
        .fourcc    = V4L2_PIX_FMT_NV16,
        .mbus      = MEDIA_BUS_FMT_YUYV8_2X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_NV12,
        .mbus      = MEDIA_BUS_FMT_YUYV8_1_5X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_YUYV,
        .mbus      = MEDIA_BUS_FMT_YUYV8_1X16,
    },
    {
        .fourcc    = V4L2_PIX_FMT_BGR24,
        .mbus      = MEDIA_BUS_FMT_BGR888_1X24,
    },
    {
        .fourcc    = v4l2_fourcc('B', 'G', '3', 'P'),
        .mbus      = MEDIA_BUS_FMT_BGR888_3X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR8,
        .mbus      = MEDIA_BUS_FMT_SBGGR8_1X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG8,
        .mbus      = MEDIA_BUS_FMT_SGBRG8_1X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG8,
        .mbus      = MEDIA_BUS_FMT_SGRBG8_1X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB8,
        .mbus      = MEDIA_BUS_FMT_SRGGB8_1X8,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR10,
        .mbus      = MEDIA_BUS_FMT_SBGGR10_1X10,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG10,
        .mbus      = MEDIA_BUS_FMT_SGBRG10_1X10,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG10,
        .mbus      = MEDIA_BUS_FMT_SGRBG10_1X10,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB10,
        .mbus      = MEDIA_BUS_FMT_SRGGB10_1X10,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR12,
        .mbus      = MEDIA_BUS_FMT_SBGGR12_1X12,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG12,
        .mbus      = MEDIA_BUS_FMT_SGBRG12_1X12,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG12,
        .mbus      = MEDIA_BUS_FMT_SGRBG12_1X12,
    },
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB12,
        .mbus      = MEDIA_BUS_FMT_SRGGB12_1X12,
    },
    {
        .fourcc    = V4L2_PIX_FMT_P010,
        .mbus      = MEDIA_BUS_FMT_YUYV10_2X10,
    },
};

static int vvcam_video_mbus_to_fourcc(uint32_t mbus, uint32_t *fourcc)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(vvcam_formats_info); i++) {
        if (vvcam_formats_info[i].mbus == mbus) {
            *fourcc = vvcam_formats_info[i].fourcc;
            return 0;
        }
    }

    return -EINVAL;
}

static int vvcam_video_fourcc_to_mbus(uint32_t fourcc, uint32_t *mbus)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(vvcam_formats_info); i++) {
        if (vvcam_formats_info[i].fourcc == fourcc) {
            *mbus = vvcam_formats_info[i].mbus;
            return 0;
        }
    }

    return -EINVAL;
}

static int vvcam_video_vfmt_to_mfmt(struct v4l2_format *f, struct v4l2_subdev_format *mfmt)
{
    int ret;

    mfmt->format.width        = f->fmt.pix.width;
    mfmt->format.height       = f->fmt.pix.height;
    mfmt->format.field        = f->fmt.pix.field;
    mfmt->format.colorspace   = f->fmt.pix.colorspace;
    mfmt->format.quantization = f->fmt.pix.quantization;

    ret = vvcam_video_fourcc_to_mbus(f->fmt.pix.pixelformat, &mfmt->format.code);

    return ret;

}

static const struct v4l2_format_info *vvcam_video_vfmt_info(u32 format)
{
    /* format info for user define or supported by later versions */
    static const struct v4l2_format_info formats[] = {
        /* YUV planar formats */
        {
            .format      = V4L2_PIX_FMT_P010,
            .pixel_enc   = V4L2_PIXEL_ENC_YUV,
            .mem_planes  = 1,
            .comp_planes = 2,
            .bpp         = {2, 2, 0, 0},
            .hdiv        = 2,
            .vdiv        = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
            .bpp_div     = {1, 1, 1, 1},
#endif
        },
    };

    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(formats); i++) {
        if (formats[i].format == format)
            return &formats[i];
    }

    return NULL;
}

static int vvcam_video_mfmt_to_vfmt( struct v4l2_subdev_format *mfmt, struct v4l2_format *f)
{
    int ret;
    const struct v4l2_format_info *info = NULL;
    uint32_t bytesperline;
    uint32_t sizeimage = 0;
    uint32_t width;
    uint32_t height;
    int i;
    struct v4l2_format_info bg3p_info = {
        .format = v4l2_fourcc('B', 'G', '3', 'P'),
        .pixel_enc = V4L2_PIXEL_ENC_RGB,
        .mem_planes = 3,
        .comp_planes = 3,
        .bpp = {
            1, 1, 1, 0
        },
        .bpp_div = { 1, 1, 1, 1 },
        .hdiv = 1,
        .vdiv = 1
    };

    f->fmt.pix.width       = mfmt->format.width;
    f->fmt.pix.height      = mfmt->format.height;
    f->fmt.pix.field       = mfmt->format.field;
    f->fmt.pix.colorspace  = mfmt->format.colorspace;
    f->fmt.pix.quantization = mfmt->format.quantization;
    ret = vvcam_video_mbus_to_fourcc(mfmt->format.code, &f->fmt.pix.pixelformat);
    if (ret) {
        return ret;
    }

    width  = f->fmt.pix.width;
    height = f->fmt.pix.height;
    if (f->fmt.pix.pixelformat == v4l2_fourcc('B', 'G', '3', 'P')) {
        // special format
        info = &bg3p_info;
    } else {
        info = v4l2_format_info(f->fmt.pix.pixelformat);
    }
    if (info == NULL) {
        info = vvcam_video_vfmt_info(f->fmt.pix.pixelformat);
        if (info == NULL)
            return -EINVAL;
    }

    bytesperline = info->bpp[0] * width;
    sizeimage = bytesperline * height;

    if (info->comp_planes == 1) {
        f->fmt.pix.bytesperline = bytesperline;
        f->fmt.pix.sizeimage = sizeimage;
        return 0;
    }

    f->fmt.pix.bytesperline = bytesperline;
    f->fmt.pix.sizeimage = sizeimage;
    for (i = 1; i < info->comp_planes; i++) {
        bytesperline = info->bpp[i] * DIV_ROUND_UP(width, info->hdiv);
        sizeimage = bytesperline * DIV_ROUND_UP(height, info->vdiv);

        f->fmt.pix.sizeimage += sizeimage;
    }

    return 0;
}

static struct v4l2_subdev *vvcam_video_remote_subdev(struct vvcam_video_dev *vvcam_vdev)
{
    struct media_pad *pad;
    struct v4l2_subdev *subdev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
    pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
    pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
	if (!pad || !is_media_entity_v4l2_subdev(pad->entity))
		return NULL;

    subdev = media_entity_to_v4l2_subdev(pad->entity);

    return subdev;
}

static int vvcam_video_try_create_pipeline(struct vvcam_video_dev *vvcam_vdev)
{
    int ret;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct v4l2_subdev_format sd_fmt;
    struct v4l2_subdev_pad_config pad_cfg;
    struct v4l2_subdev_state sd_state = {
        .pads = &pad_cfg,
    };

    if (vvcam_vdev->pipeline) {
        return 0;
    }
    ret = vvcam_video_create_pipeline_event(vvcam_vdev);
    if (ret) {
        return ret;
    }

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (!subdev) {
        return -EINVAL;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
    pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
    pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif

    memset(&sd_fmt, 0, sizeof(sd_fmt));
    sd_fmt.pad = pad->index;
    sd_fmt.which = V4L2_SUBDEV_FORMAT_TRY;

    ret = v4l2_subdev_call(subdev, pad, get_fmt, &sd_state, &sd_fmt);
    if (ret) {
        return ret;
    }

    ret = vvcam_video_mfmt_to_vfmt(&sd_fmt, &vvcam_vdev->format);
    if (ret) {
        return ret;
    }

    vvcam_vdev->pipeline = 1;

    return 0;
}

static int vvcam_video_destroy_pipeline(struct vvcam_video_dev *vvcam_vdev)
{
    vvcam_vdev->pipeline = 0;
    vvcam_video_destroy_pipeline_event(vvcam_vdev);
    return 0;
}

static int vvcam_videoc_querycap(struct file *file, void *priv,
                                struct v4l2_capability *cap)
{
	struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
	strlcpy(cap->driver, vvcam_vdev->video->name, sizeof(cap->driver));
	strlcpy(cap->card, vvcam_vdev->video->name, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info),
            "platform:%s", vvcam_vdev->video->name);

	return 0;
}

static int vvcam_videoc_enum_fmt_vid_cap(struct file *file, void *priv,
                                        struct v4l2_fmtdesc *f)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct v4l2_subdev_mbus_code_enum mbus_code;
    struct v4l2_subdev_pad_config pad_cfg;
    struct v4l2_subdev_state sd_state = {
        .pads = &pad_cfg,
    };
    int ret = -EINVAL;

    ret = vvcam_video_try_create_pipeline(vvcam_vdev);
    if (ret) {
        return ret;
    }

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&mbus_code, 0, sizeof(mbus_code));
        mbus_code.pad = pad->index;
        mbus_code.index = f->index;
        ret = v4l2_subdev_call(subdev, pad, enum_mbus_code, &sd_state, &mbus_code);
        if (ret)
            return ret;

        ret = vvcam_video_mbus_to_fourcc(mbus_code.code, &f->pixelformat);
        if (ret)
            return ret;
    }

    return ret;
}

static int vvcam_videoc_try_fmt_vid_cap(struct file *file, void *priv,
                                        struct v4l2_format *f)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct v4l2_subdev_format sd_fmt;
    struct v4l2_subdev_pad_config pad_cfg;
    struct v4l2_subdev_state sd_state = {
        .pads = &pad_cfg,
    };
    int ret;

    if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

    ret = vvcam_video_try_create_pipeline(vvcam_vdev);
    if (ret) {
        return ret;
    }

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (!subdev)
        return -EINVAL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
    pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
    pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif

    memset(&sd_fmt, 0, sizeof(sd_fmt));
    sd_fmt.pad = pad->index;
    sd_fmt.which = V4L2_SUBDEV_FORMAT_TRY;

    vvcam_video_vfmt_to_mfmt(f, &sd_fmt);
    ret = v4l2_subdev_call(subdev, pad, set_fmt, &sd_state, &sd_fmt);
    if (ret) {
        return ret;
    }

    ret = vvcam_video_mfmt_to_vfmt(&sd_fmt, f);

    return ret;
}

static int vvcam_videoc_s_fmt_vid_cap(struct file *file, void *priv,
                                    struct v4l2_format *f)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct vb2_queue *queue = &vvcam_vdev->queue;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct v4l2_subdev_format sd_fmt;
    struct v4l2_subdev_pad_config pad_cfg;
    struct v4l2_subdev_state sd_state = {
        .pads = &pad_cfg,
    };
    int ret;

    if (vb2_is_busy(queue))
        return -EBUSY;
    ret = vvcam_videoc_try_fmt_vid_cap(file, priv, f);
    if (ret)
        return ret;
    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (!subdev)
        return -EINVAL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
    pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
    pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif

    memset(&sd_fmt, 0, sizeof(sd_fmt));
    sd_fmt.pad = pad->index;
    sd_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

    vvcam_video_vfmt_to_mfmt(f, &sd_fmt);

    ret = v4l2_subdev_call(subdev, pad, set_fmt, &sd_state, &sd_fmt);
    if (ret) {
        return ret;
    }

    vvcam_vdev->format = *f;
    printk("%d x %d size %d fmt %s \n",
    f->fmt.pix.width,f->fmt.pix.height,f->fmt.pix.sizeimage, (char *)&f->fmt.pix.pixelformat);

    return 0;
}

static int vvcam_videoc_g_fmt_vid_cap(struct file *file, void *fh,
                                struct v4l2_format *f)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);

    int ret;

    if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

    ret = vvcam_video_try_create_pipeline(vvcam_vdev);
    if (ret) {
        return ret;
    }

    *f = vvcam_vdev->format;

    return 0;
}

static int vvcam_videoc_reqbufs(struct file *file, void *priv,
			            struct v4l2_requestbuffers *p)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_reqbufs pad_requbufs;

    int ret;

    ret = vvcam_video_try_create_pipeline(vvcam_vdev);
    if (ret)
        return ret;

    ret = vb2_ioctl_reqbufs(file, priv, p);
    if (ret)
        return ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_requbufs, 0, sizeof(pad_requbufs));
        pad_requbufs.pad = pad->index;
        pad_requbufs.num_buffers = p->count;
        v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_REQUBUFS, &pad_requbufs);
    }

    return ret;
}

static int vvcam_videoc_enum_input(struct file *file, void *fh,
                                struct v4l2_input *input)
{
    if (input->index > 0)
        return -EINVAL;

    strscpy(input->name, "camera", sizeof(input->name));
    input->type = V4L2_INPUT_TYPE_CAMERA;

	return 0;
}

static int vvcam_videoc_g_input(struct file *file, void *fh, unsigned int *input)
{
    *input = 0;
	return 0;
}

static int vvcam_videoc_s_input(struct file *file, void *fh, unsigned int input)
{
	return input == 0 ? 0 : -EINVAL;
}

static int vvcam_videoc_queryctrl(struct file *file, void *fh,
				struct v4l2_queryctrl *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_queryctrl pad_query_ctrl;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_query_ctrl, 0, sizeof(pad_query_ctrl));
        pad_query_ctrl.pad = pad->index;
        pad_query_ctrl.query_ctrl = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_QUERYCTRL, &pad_query_ctrl);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_videoc_query_ext_ctrl(struct file *file, void *fh,
				     struct v4l2_query_ext_ctrl *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_query_ext_ctrl pad_query_ext_ctrl;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_query_ext_ctrl, 0, sizeof(pad_query_ext_ctrl));
        pad_query_ext_ctrl.pad = pad->index;
        pad_query_ext_ctrl.query_ext_ctrl = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_QUERY_EXT_CTRL, &pad_query_ext_ctrl);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_g_ctrl(struct file *file, void *fh,
			     struct v4l2_control *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_control pad_control;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_control, 0, sizeof(pad_control));
        pad_control.pad = pad->index;
        pad_control.control = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_G_CTRL, &pad_control);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_s_ctrl(struct file *file, void *fh,
			     struct v4l2_control *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_control pad_control;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_control, 0, sizeof(pad_control));
        pad_control.pad = pad->index;
        pad_control.control = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_S_CTRL, &pad_control);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_g_ext_ctrls(struct file *file, void *fh,
				  struct v4l2_ext_controls *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_ext_controls pad_ext_controls;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_ext_controls, 0, sizeof(pad_ext_controls));
        pad_ext_controls.pad = pad->index;
        pad_ext_controls.ext_controls = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_G_EXT_CTRLS, &pad_ext_controls);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_s_ext_ctrls(struct file *file, void *fh,
				  struct v4l2_ext_controls *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_ext_controls pad_ext_controls;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_ext_controls, 0, sizeof(pad_ext_controls));
        pad_ext_controls.pad = pad->index;
        pad_ext_controls.ext_controls = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_S_EXT_CTRLS, &pad_ext_controls);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_try_ext_ctrls(struct file *file, void *fh,
				  struct v4l2_ext_controls *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_ext_controls pad_ext_controls;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_ext_controls, 0, sizeof(pad_ext_controls));
        pad_ext_controls.pad = pad->index;
        pad_ext_controls.ext_controls = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_TRY_EXT_CTRLS, &pad_ext_controls);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_vidioc_querymenu(struct file *file, void *fh,
				struct v4l2_querymenu *a)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_querymenu pad_querymenu;
    int ret;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_querymenu, 0, sizeof(pad_querymenu));
        pad_querymenu.pad = pad->index;
        pad_querymenu.querymenu = a;
        ret = v4l2_subdev_call(subdev, core, ioctl,
                        VVCAM_PAD_QUERYMENU, &pad_querymenu);

    } else {
        return -ENOTTY;
    }

    return ret;
}

static int vvcam_videoc_subscribe_event(struct v4l2_fh *fh,
                                const struct v4l2_event_subscription *sub)
{
    int ret;
    switch (sub->type) {
	case V4L2_EVENT_CTRL:
		ret = v4l2_ctrl_subscribe_event(fh, sub);
        break;
    case VVCAM_VIDEO_DEAMON_EVENT:
        ret = v4l2_event_subscribe(fh, sub, 2, NULL);
        break;
	default:
		ret = -EINVAL;
        break;
	}

    return ret;
}

static const struct v4l2_ioctl_ops vvcam_video_ioctl_ops = {
    .vidioc_querycap            = vvcam_videoc_querycap,
    .vidioc_enum_fmt_vid_cap    = vvcam_videoc_enum_fmt_vid_cap,
    .vidioc_try_fmt_vid_cap     = vvcam_videoc_try_fmt_vid_cap,
    .vidioc_g_fmt_vid_cap       = vvcam_videoc_g_fmt_vid_cap,
    .vidioc_s_fmt_vid_cap       = vvcam_videoc_s_fmt_vid_cap,

    .vidioc_reqbufs             = vvcam_videoc_reqbufs,
    .vidioc_querybuf            = vb2_ioctl_querybuf,
    .vidioc_create_bufs         = vb2_ioctl_create_bufs,
    .vidioc_qbuf                = vb2_ioctl_qbuf,
    .vidioc_expbuf              = vb2_ioctl_expbuf,
    .vidioc_dqbuf               = vb2_ioctl_dqbuf,
    .vidioc_prepare_buf         = vb2_ioctl_prepare_buf,
    .vidioc_streamon            = vb2_ioctl_streamon,
    .vidioc_streamoff           = vb2_ioctl_streamoff,

    .vidioc_enum_input          = vvcam_videoc_enum_input,
    .vidioc_g_input             = vvcam_videoc_g_input,
    .vidioc_s_input             = vvcam_videoc_s_input,
    /*.vidioc_g_selection         = vvcam_videoc_g_selection,
    .vidioc_s_selection         = vvcam_videoc_s_selection,
    .vidioc_g_parm              = vvcam_videoc_g_parm,
    .vidioc_s_parm              = vvcam_videoc_s_parm,
    .vidioc_enum_framesizes     = vvcam_videoc_enum_framesizes,
    .vidioc_enum_frameintervals = vvcam_videoc_enum_frmaeintervals,*/
    .vidioc_queryctrl           = vvcam_videoc_queryctrl,
    .vidioc_query_ext_ctrl      = vvcam_videoc_query_ext_ctrl,
    .vidioc_g_ctrl              = vvcam_vidioc_g_ctrl,
    .vidioc_s_ctrl              = vvcam_vidioc_s_ctrl,
    .vidioc_g_ext_ctrls         = vvcam_vidioc_g_ext_ctrls,
    .vidioc_s_ext_ctrls         = vvcam_vidioc_s_ext_ctrls,
    .vidioc_try_ext_ctrls       = vvcam_vidioc_try_ext_ctrls,
    .vidioc_querymenu           = vvcam_vidioc_querymenu,
    .vidioc_subscribe_event     = vvcam_videoc_subscribe_event,
    .vidioc_unsubscribe_event   = v4l2_event_unsubscribe,
};

static __poll_t vvcam_video_poll(struct file *file,
                        struct poll_table_struct *wait)
{
    struct v4l2_fh *fh = file->private_data;
    if (!list_empty(&fh->subscribed)) {
        return v4l2_ctrl_poll(file, wait);
    } else {
        return vb2_fop_poll(file, wait);
    }
}

static int vvcam_video_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    struct v4l2_fh *fh = file->private_data;
    int ret;

    if (vvcam_vdev->video->queue->owner &&
        (vvcam_vdev->video->queue->owner == fh)) {
       return vb2_fop_mmap(file, vma);
    } else {
        ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
                            vma->vm_end - vma->vm_start,
                            vma->vm_page_prot);
        return ret;
    }
}

static int vvcam_video_open(struct file *file)
{
    return v4l2_fh_open(file);
}

static int vvcam_video_release(struct file *file)
{
    struct vvcam_video_dev *vvcam_vdev = video_drvdata(file);
    int ret;

    ret = vb2_fop_release(file);
    if (vvcam_vdev->video->queue->owner == NULL) {
        if (vvcam_vdev->pipeline) {
            vvcam_video_destroy_pipeline(vvcam_vdev);
        }
    }

    return ret;
}

static const struct v4l2_file_operations vvcam_video_fops = {
	.owner          = THIS_MODULE,
	.open           = vvcam_video_open,
	.release        = vvcam_video_release,
	.poll           = vvcam_video_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap           = vvcam_video_mmap,
};

static int vvcam_video_vb2_queue_setup(struct vb2_queue *queue,
                                    unsigned int *num_buffers,
                                    unsigned int *num_planes,
                                    unsigned int sizes[],
                                    struct device *alloc_devs[])
{
    struct vvcam_video_dev *vvcam_vdev = queue->drv_priv;
    struct v4l2_format *format = &vvcam_vdev->format;
    unsigned int i;

    if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        if (*num_planes) {
            if (*num_planes != 1)
                return -EINVAL;
            if (sizes[0] < format->fmt.pix.sizeimage)
                return -EINVAL;
        } else {
            *num_planes = 1;
            sizes[0] = format->fmt.pix.sizeimage;
        }
    } else if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        if (*num_planes) {
            if (*num_planes != format->fmt.pix_mp.num_planes)
                return -EINVAL;
            for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
                if (sizes[i] < format->fmt.pix_mp.plane_fmt[i].sizeimage)
                    return -EINVAL;
            }
        } else {
            *num_planes = format->fmt.pix_mp.num_planes;
            for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
                sizes[i] = format->fmt.pix_mp.plane_fmt[i].sizeimage;
            }
        }
    } else {
        return -EINVAL;
    }

    return 0;
}

static int vvcam_video_vb2_buf_prepare(struct vb2_buffer *vb)
{
    struct vvcam_video_dev *vvcam_vdev = vb->vb2_queue->drv_priv;
    struct v4l2_format *format = &vvcam_vdev->format;
    struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct vvcam_vb2_buffer *buf = container_of(vbuf,
						  struct vvcam_vb2_buffer, vb);
    int i;

    if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        if (vb2_plane_size(vb, 0) < format->fmt.pix.sizeimage)
            return -EINVAL;

        buf->num_planes = 1;
        buf->planes[0].dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
        buf->planes[0].size     = format->fmt.pix.sizeimage;
        buf->sequence           = vb->index;

        vb2_set_plane_payload(vb, 0, buf->planes[0].size);

    } else if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        buf->num_planes = format->fmt.pix_mp.num_planes;
        buf->sequence   = vb->index;
        for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
            if (vb2_plane_size(vb, i) < format->fmt.pix_mp.plane_fmt[i].sizeimage)
                return -EINVAL;

            buf->planes[i].dma_addr = vb2_dma_contig_plane_dma_addr(vb, i);
            buf->planes[i].size     = format->fmt.pix_mp.plane_fmt[i].sizeimage;

            vb2_set_plane_payload(vb, i, buf->planes[i].size);
        }
    } else {
        return -EINVAL;
    }

    return 0;
}

static void vvcam_video_vb2_buf_queue(struct vb2_buffer *vb)
{
    struct vvcam_video_dev *vvcam_vdev = vb->vb2_queue->drv_priv;
    struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct vvcam_vb2_buffer *buf = container_of(vbuf,
						  struct vvcam_vb2_buffer, vb);
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_buf pad_buf;


    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&pad_buf, 0, sizeof(pad_buf));
        pad_buf.pad = pad->index;
        pad_buf.buf = buf;
        v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_BUF_QUEUE, &pad_buf);
    }

    return;
}

static int vvcam_video_vb2_start_streaming(struct vb2_queue *queue,
                                        unsigned int count)
{
   struct vvcam_video_dev *vvcam_vdev = queue->drv_priv;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_stream_status stream_status;
    int ret = -EINVAL;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&stream_status, 0, sizeof(stream_status));
        stream_status.pad = pad->index;
        stream_status.status = 1;
        ret = v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_S_STREAM, &stream_status);
    }

    return ret;
}

static void vvcam_video_vb2_stop_streaming(struct vb2_queue *queue)
{
    struct vvcam_video_dev *vvcam_vdev = queue->drv_priv;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct vvcam_pad_stream_status stream_status;
    int i;

    subdev = vvcam_video_remote_subdev(vvcam_vdev);
    if (subdev) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&vvcam_vdev->pad);
#else
        pad = media_entity_remote_pad(&vvcam_vdev->pad);
#endif
        memset(&stream_status, 0, sizeof(stream_status));
        stream_status.pad = pad->index;
        stream_status.status = 0;
        v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_S_STREAM, &stream_status);
    }

    for(i = 0; i < queue->num_buffers; i++) {
		if(queue->bufs[i]->state == VB2_BUF_STATE_ACTIVE)
			vb2_buffer_done(queue->bufs[i], VB2_BUF_STATE_ERROR);
	}

    return;
}

static const struct vb2_ops vvcam_video_queue_ops = {
	.queue_setup     = vvcam_video_vb2_queue_setup,
	.buf_prepare     = vvcam_video_vb2_buf_prepare,
	.buf_queue       = vvcam_video_vb2_buf_queue,
    .wait_prepare    = vb2_ops_wait_prepare,
    .wait_finish     = vb2_ops_wait_finish,
	.start_streaming = vvcam_video_vb2_start_streaming,
	.stop_streaming  = vvcam_video_vb2_stop_streaming,
};

static int vvcam_video_queue_init(struct vvcam_video_dev *vvcam_vdev)
{
    int ret = 0;
    struct vb2_queue *queue;

    queue = &vvcam_vdev->queue;
    queue->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queue->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    queue->drv_priv = vvcam_vdev;
    queue->ops = &vvcam_video_queue_ops;
    queue->mem_ops = &vb2_dma_contig_memops;
    queue->buf_struct_size = sizeof(struct vvcam_vb2_buffer);
	queue->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
    queue->lock = &vvcam_vdev->video_lock;
    queue->dev = vvcam_vdev->vvcam_mdev->dev;

    ret = vb2_queue_init(queue);
    if (ret) {
        dev_err(vvcam_vdev->vvcam_mdev->dev, "vb2 queue init failed\n");
        return ret;
    }
    vvcam_vdev->video->queue = queue;

    return 0;
}

static int vvcam_video_link_setup(struct media_entity *entity,
                const struct media_pad *local,
				const struct media_pad *remote, u32 flags)
{
	return 0;
}


static const struct media_entity_operations vvcam_video_entity_ops = {
	.link_setup     = vvcam_video_link_setup,
	.link_validate  = v4l2_subdev_link_validate,
};

int vvcam_video_register(struct vvcam_media_dev *vvcam_mdev, int port)
{
    int ret = 0;
    struct vvcam_video_dev *vvcam_vdev;

    vvcam_vdev = devm_kzalloc(vvcam_mdev->dev,
                sizeof(struct vvcam_video_dev), GFP_KERNEL);
    if (!vvcam_vdev)
        return -ENOMEM;

    mutex_init(&vvcam_vdev->video_lock);
    vvcam_vdev->vvcam_mdev = vvcam_mdev;
    vvcam_vdev->video_params = vvcam_mdev->video_params[port];

    vvcam_vdev->video = video_device_alloc();
    if (!vvcam_vdev->video) {
        dev_err(vvcam_mdev->dev, "could not alloc video device\n");
        ret = -ENOMEM;
        goto error_free_vvcam_vdev;
    }

    snprintf(vvcam_vdev->video->name, sizeof(vvcam_vdev->video->name),
                "%s.%d.%d", VVCAM_VIDEO_NAME, vvcam_mdev->id, port);

    vvcam_vdev->video->fops          = &vvcam_video_fops;
    vvcam_vdev->video->ioctl_ops     = &vvcam_video_ioctl_ops;
    vvcam_vdev->video->release       = video_device_release_empty;
    vvcam_vdev->video->v4l2_dev      = &vvcam_mdev->v4l2_dev;
    //vvcam_vdev->video->lock          = &vvcam_vdev->video_lock;
    vvcam_vdev->video->device_caps   = V4L2_CAP_VIDEO_CAPTURE |
                                       V4L2_CAP_STREAMING;
    vvcam_vdev->video->minor         = -1;

    video_set_drvdata(vvcam_vdev->video, vvcam_vdev);

    vvcam_vdev->video->entity.name     = vvcam_vdev->video->name;
    vvcam_vdev->video->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
	vvcam_vdev->video->entity.function = MEDIA_ENT_F_IO_V4L;
	vvcam_vdev->video->entity.ops      = &vvcam_video_entity_ops;
	vvcam_vdev->pad.flags              = MEDIA_PAD_FL_SINK;

    ret = media_entity_pads_init(&vvcam_vdev->video->entity, 1, &vvcam_vdev->pad);
	if (ret) {
		dev_err(vvcam_mdev->dev, "entity pad init error\n");
		goto error_video_device_release;
	}

    ret = vvcam_video_queue_init(vvcam_vdev);
    if (ret) {
        dev_err(vvcam_mdev->dev, "queue init error\n");
		goto err_media_entity_cleanup;
    }

	ret = video_register_device(vvcam_vdev->video, VFL_TYPE_VIDEO, -1);
    if (ret) {
		dev_err(vvcam_mdev->dev, "video register device error\n");
		goto err_media_entity_cleanup;
    }

    vvcam_vdev->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 0);
    vvcam_vdev->event_shm.size = PAGE_SIZE;
    memset(vvcam_vdev->event_shm.virt_addr, 0, vvcam_vdev->event_shm.size);
    vvcam_vdev->event_shm.phy_addr = virt_to_phys(vvcam_vdev->event_shm.virt_addr);
    mutex_init(&vvcam_vdev->event_shm.event_lock);

    vvcam_mdev->video_devs[port] = vvcam_vdev;

    return 0;

err_media_entity_cleanup:
    media_entity_cleanup(&vvcam_vdev->video->entity);

error_video_device_release:
    video_device_release(vvcam_vdev->video);
    vvcam_vdev->video = NULL;

error_free_vvcam_vdev:
    devm_kfree(vvcam_mdev->dev,vvcam_vdev);

    return ret;
}

int vvcam_video_unregister(struct vvcam_media_dev *vvcam_mdev, int port)
{
    struct vvcam_video_dev *vvcam_vdev = vvcam_mdev->video_devs[port];

    if (vvcam_vdev == NULL)
        return 0;

    video_unregister_device(vvcam_vdev->video);
    media_entity_cleanup(&vvcam_vdev->video->entity);
    video_device_release(vvcam_vdev->video);
    devm_kfree(vvcam_mdev->dev,vvcam_vdev);
    vvcam_mdev->video_devs[port] = NULL;

    free_pages((unsigned long)vvcam_vdev->event_shm.virt_addr, 0);

    return 0;
}

MODULE_DESCRIPTION("Verisilicon video driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
