// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>

#include <linux/platform_device.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-dma-sg.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>
#include <linux/platform_device.h>

#include "k_nonai_2d_comm.h"
#include "nonai_2d.h"
#include "nonai_2d_reg.h"
#include "nonai2d-v4l2-controls.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

#define NONAI_2D_NAME "nonai-2d"

struct nonai_2d_format {
    uint32_t fourcc;
    int depth;
    int plane;
    enum k_2d_video_format fmt_2d;
    enum k_2d_osd_fmt fmt_osd;
    enum k_pixel_format pixel_format;
};

struct nonai_2d_frame {
    /* Original dimensions */
    uint32_t width;
    uint32_t height;
    uint32_t colorspace;
    uint32_t num_planes;
    /* Image format */
    struct nonai_2d_format fmt;

    uint32_t stride;
    uint32_t size;
    uint32_t chn;
    dma_addr_t addr[3];
};

struct nonai_2d_drv_ctx {
    struct v4l2_device v4l2_dev;
    struct v4l2_m2m_dev *m2m_dev;
    struct video_device *vfd;
    struct device *dev;
    void __iomem *regs_2d;

    /* vfd lock */
    struct mutex mutex;
    /* ctrl parm lock */
    spinlock_t ctrl_lock;
    struct nonai_2d_v4l2_ctx *v4l2_ctx;
    uint32_t num;
    uint32_t dev_num[NONAI_2D_MAX_CHN_NUMS];
};

struct nonai_2d_v4l2_ctx {
    struct v4l2_fh fh;
    struct v4l2_ctrl_handler ctrl_handler;
    struct nonai_2d_drv_ctx *drv_ctx;
    struct nonai_2d_frame in;
    struct nonai_2d_frame out;
    uint32_t type;
    bool ctrl_initialized;
};

static void nonai_2d_config_job(struct nonai_2d_job *job, struct nonai_2d_v4l2_ctx *v4l2_ctx);

static struct nonai_2d_format formats[] = {
    {
        .fourcc = V4L2_PIX_FMT_ARGB32,
        .depth = 32,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_ARGB8888,
        .fmt_osd = K_2D_OSD_FMT_ARGB8888,
        .pixel_format = PIXEL_FORMAT_ARGB_8888,
    },
    {
        .fourcc = V4L2_PIX_FMT_XRGB32,
        .depth = 32,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_XRGB8888,
        .fmt_osd = K_2D_OSD_FMT_XRGB8888,
        .pixel_format = PIXEL_FORMAT_ARGB_8888,
    },
    {
        .fourcc = V4L2_PIX_FMT_ABGR32,
        .depth = 32,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_ARGB8888,
        .fmt_osd = K_2D_OSD_FMT_ARGB8888,
        .pixel_format = PIXEL_FORMAT_ABGR_8888,
    },
    {
        .fourcc = V4L2_PIX_FMT_XBGR32,
        .depth = 32,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_XRGB8888,
        .fmt_osd = K_2D_OSD_FMT_XRGB8888,
        .pixel_format = PIXEL_FORMAT_ABGR_8888,
    },
    {
        .fourcc = V4L2_PIX_FMT_RGB24,
        .depth = 24,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_RGB888, //K_2D_VIDEO_FMT_RGB888,
        .fmt_osd = K_2D_OSD_FMT_RGB888,
        .pixel_format = PIXEL_FORMAT_RGB_888, //PIXEL_FORMAT_RGB_888,
    },
    {
        .fourcc = V4L2_PIX_FMT_RGB24,
        .depth = 24,
        .plane = 3,
        .fmt_2d = K_2D_VIDEO_FMT_SEPERATE_RGB, //K_2D_VIDEO_FMT_RGB888,
        .fmt_osd = K_2D_OSD_FMT_RGB888,
        .pixel_format =
            PIXEL_FORMAT_RGB_888_PLANAR, //PIXEL_FORMAT_RGB_888,
    },
    {
        .fourcc = V4L2_PIX_FMT_BGR24,
        .depth = 24,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_RGB888,
        .fmt_osd = K_2D_OSD_FMT_RGB888,
        .pixel_format = PIXEL_FORMAT_BGR_888,
    },
    {
        .fourcc = V4L2_PIX_FMT_ARGB444,
        .depth = 16,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_ARGB4444,
        .fmt_osd = K_2D_OSD_FMT_ARGB4444,
        .pixel_format = PIXEL_FORMAT_ARGB_4444,
    },
    {
        .fourcc = V4L2_PIX_FMT_ARGB555,
        .depth = 16,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_ARGB1555,
        .fmt_osd = K_2D_OSD_FMT_ARGB1555,
        .pixel_format = PIXEL_FORMAT_ARGB_1555,
    },
    {
        .fourcc = V4L2_PIX_FMT_RGB565,
        .depth = 16,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_RGB565,
        .fmt_osd = K_2D_OSD_FMT_RGB565,
        .pixel_format = PIXEL_FORMAT_RGB_565,
    },
    {
        .fourcc = V4L2_PIX_FMT_NV21,
        .depth = 12,
        .plane = 2,
        .fmt_2d = K_2D_VIDEO_FMT_YUV420_NV21,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    },
    {
        .fourcc = V4L2_PIX_FMT_NV12,
        .depth = 12,
        .plane = 2,
        .fmt_2d = K_2D_VIDEO_FMT_YUV420_NV12,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    },
    {
        .fourcc = V4L2_PIX_FMT_YUV420,
        .depth = 12,
        .plane = 3,
        .fmt_2d = K_2D_VIDEO_FMT_YUV420_I420,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YVU_PLANAR_420,
    },
    {
        .fourcc = V4L2_PIX_FMT_YVU420,
        .depth = 12,
        .plane = 3,
        .fmt_2d = K_2D_VIDEO_FMT_YUV420_I420,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YVU_PLANAR_420,
    },
    {
        .fourcc = V4L2_PIX_FMT_YUV444,
        .depth = 24,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_SEPERATE_RGB,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_444,
    },
    {
        .fourcc = V4L2_PIX_FMT_YUV444M,
        .depth = 24,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_SEPERATE_RGB,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_444,
    },
    {
        .fourcc = V4L2_PIX_FMT_YUV410,
        .depth = 8,
        .plane = 1,
        .fmt_2d = K_2D_VIDEO_FMT_BUTT,
        .fmt_osd = K_2D_OSD_FMT_BUTT,
        .pixel_format = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    },
};

#define NUM_FORMATS ARRAY_SIZE(formats)

static struct nonai_2d_frame
    def_frame = { .width = DEFAULT_WIDTH,
              .height = DEFAULT_HEIGHT,
              .colorspace = V4L2_COLORSPACE_DEFAULT,
              .fmt = {
                  .fourcc = V4L2_PIX_FMT_ARGB32,
                  .depth = 32,
                  .plane = 1,
                  .fmt_2d = K_2D_VIDEO_FMT_ARGB8888,
                  .fmt_osd = K_2D_OSD_FMT_ARGB8888,
                  .pixel_format = PIXEL_FORMAT_ARGB_8888,
              } };

static void nonai_2d_update_frame_layout(struct nonai_2d_frame *frm)
{
    uint32_t bytes_per_line;

    if (frm->num_planes == 0)
        frm->num_planes = frm->fmt.plane ? frm->fmt.plane : 1;

    bytes_per_line = (frm->width * frm->fmt.depth) >> 3;
    if (frm->stride == 0)
        frm->stride = bytes_per_line;

    if (frm->size == 0)
        frm->size = frm->height * frm->stride;
}

static void nonai_2d_fill_plane_layout(const struct nonai_2d_format *fmt,
                   uint32_t width, uint32_t height,
                   uint32_t bytesperline[3], uint32_t sizeimage[3])
{
    memset(bytesperline, 0, sizeof(uint32_t) * 3);
    memset(sizeimage, 0, sizeof(uint32_t) * 3);

    switch (fmt->fourcc) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
        bytesperline[0] = width;
        sizeimage[0] = width * height;
        bytesperline[1] = width;
        sizeimage[1] = (width * height) / 2;
        break;
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
        bytesperline[0] = width;
        sizeimage[0] = width * height;
        bytesperline[1] = width / 2;
        sizeimage[1] = (width * height) / 4;
        bytesperline[2] = width / 2;
        sizeimage[2] = (width * height) / 4;
        break;
    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_BGR24:
        if (fmt->plane == 3) {
            bytesperline[0] = width;
            sizeimage[0] = width * height;
            bytesperline[1] = width;
            sizeimage[1] = width * height;
            bytesperline[2] = width;
            sizeimage[2] = width * height;
            break;
        }
        bytesperline[0] = width * 3;
        sizeimage[0] = width * height * 3;
        break;
    default:
        bytesperline[0] = (width * fmt->depth) >> 3;
        sizeimage[0] = height * bytesperline[0];
        break;
    }
}

struct nonai_2d_frame *nonai_2d_get_frame(struct nonai_2d_v4l2_ctx *ctx,
                      enum v4l2_buf_type type)
{
    switch (type) {
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
        return &ctx->in;
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
        return &ctx->in;
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
        return &ctx->out;
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
        return &ctx->out;
    default:
        return ERR_PTR(-EINVAL);
    }
}

static struct nonai_2d_format *nonai_2d_fmt_find(struct v4l2_format *f)
{
    unsigned int i;
    uint32_t pixelformat;
    uint32_t num_planes;

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        pixelformat = f->fmt.pix_mp.pixelformat;
        num_planes = f->fmt.pix_mp.num_planes;

        if (num_planes == 0) {
            for (i = 0; i < NUM_FORMATS; i++) {
                if (formats[i].fourcc == pixelformat)
                    return &formats[i];
            }
            return NULL;
        }

        for (i = 0; i < NUM_FORMATS; i++) {
            if (formats[i].fourcc == pixelformat &&
                formats[i].plane == num_planes)
                return &formats[i];
        }
    } else {
        for (i = 0; i < NUM_FORMATS; i++) {
            if (formats[i].fourcc == f->fmt.pix.pixelformat)
                return &formats[i];
        }
    }

    return NULL;
}

static int nonai_2d_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers,
                unsigned int *nplanes, unsigned int sizes[],
                struct device *alloc_devs[])
{
    int i;
    struct nonai_2d_v4l2_ctx *ctx = vb2_get_drv_priv(vq);
    struct nonai_2d_frame *f = nonai_2d_get_frame(ctx, vq->type);
    uint32_t bytesperline[3];
    uint32_t sizeimage[3];

    if (IS_ERR(f))
        return PTR_ERR(f);

    nonai_2d_update_frame_layout(f);
    nonai_2d_fill_plane_layout(&f->fmt, f->width, f->height, bytesperline, sizeimage);

    nonai_2d_info("nonai2d queue_setup: type=%u req_nbuf=%u req_nplanes=%u fmt=0x%x wxh=%ux%u frm_planes=%u\n",
        vq->type, *nbuffers, *nplanes, f->fmt.fourcc, f->width, f->height, f->num_planes);

    if (*nplanes) {
        if (*nplanes != f->num_planes)
            return -EINVAL;
        for (i = 0; i < f->num_planes; i++) {
            nonai_2d_info("nonai2d queue_setup(check): plane=%d req_size=%u need_size=%u bpl=%u\n",
                i, sizes[i], sizeimage[i], bytesperline[i]);
            if (sizes[i] < sizeimage[i])
                return -EINVAL;
        }
        return 0;
    }

    *nplanes = f->num_planes;

    for (i = 0; i < f->num_planes; i++) {
        sizes[i] = sizeimage[i];
        nonai_2d_info("nonai2d queue_setup(set): plane=%d size=%u bpl=%u\n", i, sizes[i], bytesperline[i]);
    }

    nonai_2d_info("nonai2d queue_setup(done): grant_nbuf=%u grant_nplanes=%u\n", *nbuffers, *nplanes);

    return 0;
}

static int nonai_2d_buf_prepare(struct vb2_buffer *vb)
{
    struct nonai_2d_v4l2_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
    struct nonai_2d_frame *f = nonai_2d_get_frame(ctx, vb->vb2_queue->type);
    uint32_t bytesperline[3];
    uint32_t sizeimage[3];
    uint32_t plane;
    dma_addr_t dst_addr;
    dma_addr_t plane_addr;

    dst_addr = vb2_dma_contig_plane_dma_addr(vb, 0);

    if (IS_ERR(f))
        return PTR_ERR(f);

    nonai_2d_fill_plane_layout(&f->fmt, f->width, f->height, bytesperline, sizeimage);
    for (plane = 0; plane < f->num_planes; plane++) {
        if (vb2_plane_size(vb, plane) < sizeimage[plane])
            return -EINVAL;
        vb2_set_plane_payload(vb, plane, sizeimage[plane]);
        plane_addr = vb2_dma_contig_plane_dma_addr(vb, plane);
        nonai_2d_info("nonai2d buf_prepare: type=%u index=%u plane=%u dma=0x%llx payload=%u size=%u\n",
            vb->vb2_queue->type, vb->index, plane, (unsigned long long)plane_addr,
            sizeimage[plane], (unsigned int)vb2_plane_size(vb, plane));
    }
    return 0;
}

static void nonai_2d_buf_queue(struct vb2_buffer *vb)
{
    struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
    struct nonai_2d_v4l2_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
    struct nonai_2d_drv_ctx *drv_ctx = ctx->drv_ctx;
    struct nonai_2d_chn_t *chn = &g_nonai_2d_chn[ctx->in.chn];
    unsigned long flags;
    int plane;
    int dst_planes;
    struct nonai_2d_job job;

    v4l2_m2m_buf_queue(ctx->fh.m2m_ctx, vbuf);

    /* In OSD mode, trigger hardware processing immediately when capture buffer is queued */
    if (vb->vb2_queue->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        spin_lock_irqsave(&drv_ctx->ctrl_lock, flags);

        /* Check if it's OSD mode */
        if (chn->cfg.nonai_2d_calc_mode != nonai_2d_calc_mode_csc) {
            struct vb2_v4l2_buffer *dst = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);

            if (dst) {
                drv_ctx->v4l2_ctx = ctx;

                dst_planes = dst->vb2_buf.num_planes > 3 ? 3 : dst->vb2_buf.num_planes;

                /* Clear addresses */
                for (plane = 0; plane < 3; plane++) {
                    ctx->in.addr[plane] = 0;
                    ctx->out.addr[plane] = 0;
                }

                /* Set output addresses from capture buffer */
                for (plane = 0; plane < dst_planes; plane++) {
                    ctx->out.addr[plane] = vb2_dma_contig_plane_dma_addr(&dst->vb2_buf, plane);
                }

                /* In OSD mode, input and output use the same buffer (in-place processing) */
                for (plane = 0; plane < dst_planes; plane++) {
                    ctx->in.addr[plane] = ctx->out.addr[plane];
                }

                /* Configure hardware */
                nonai_2d_config_job(&job, ctx);
            }
        }

        spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
    }
}

static int nonai_2d_buf_start_streaming(struct vb2_queue *q, unsigned int count)
{
    return 0;
}

static void nonai_2d_buf_stop_streaming(struct vb2_queue *q)
{
    struct nonai_2d_v4l2_ctx *ctx = vb2_get_drv_priv(q);
    struct vb2_v4l2_buffer *vb;

    if (!ctx)
        return;

    if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
        while ((vb = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx)))
            v4l2_m2m_buf_done(vb, VB2_BUF_STATE_ERROR);
    } else {
        while ((vb = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx)))
            v4l2_m2m_buf_done(vb, VB2_BUF_STATE_ERROR);
    }
}

static int nonai_2d_buf_init(struct vb2_buffer *vb)
{
    return 0;
}

const struct vb2_ops nonai_2d_qops = {
    .buf_init = nonai_2d_buf_init,
    .queue_setup = nonai_2d_queue_setup,
    .buf_prepare = nonai_2d_buf_prepare,
    .buf_queue = nonai_2d_buf_queue,
    .wait_prepare = vb2_ops_wait_prepare,
    .wait_finish = vb2_ops_wait_finish,
    .start_streaming = nonai_2d_buf_start_streaming,
    .stop_streaming = nonai_2d_buf_stop_streaming,
};

static int queue_init(void *priv, struct vb2_queue *src_vq, struct vb2_queue *dst_vq)
{
    struct nonai_2d_v4l2_ctx *ctx = priv;
    int ret;

    /* OUTPUT_MPLANE: OSD input buffers (src queue) */
    src_vq->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    src_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    src_vq->drv_priv = ctx;
    src_vq->ops = &nonai_2d_qops;
    src_vq->mem_ops = &vb2_dma_contig_memops; //&vb2_dma_sg_memops;
    src_vq->allow_zero_bytesused = false;
    src_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
    src_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
    src_vq->lock = &ctx->drv_ctx->mutex;
    src_vq->dev = ctx->drv_ctx->v4l2_dev.dev;

    ret = vb2_queue_init(src_vq);
    if (ret)
        return ret;

    /* CAPTURE_MPLANE: video frame output buffers (dst queue) */
    dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    dst_vq->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
    dst_vq->drv_priv = ctx;
    dst_vq->ops = &nonai_2d_qops;
    dst_vq->mem_ops = &vb2_dma_contig_memops; //&vb2_dma_sg_memops;
    dst_vq->allow_zero_bytesused = false;
    dst_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
    dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
    dst_vq->lock = &ctx->drv_ctx->mutex;
    dst_vq->dev = ctx->drv_ctx->v4l2_dev.dev;
    return vb2_queue_init(dst_vq);
}

static void nonai_2d_release_osd_dmabuf(struct nonai_2d_drv_ctx *drv_ctx,
                    struct nonai_2d_chn_t *chn, int index)
{
    if (!chn->osd_dmabuf[index])
        return;

    dma_free_coherent(drv_ctx->dev,
              chn->osd_dmabuf_size[index],
              chn->osd_dmabuf[index],
              chn->osd_dma_handle[index]);

    chn->osd_dmabuf[index] = NULL;
    chn->osd_dmabuf_size[index] = 0;
    chn->osd_dma_handle[index] = 0;
    chn->osd_attr[index].phys_addr[0] = 0;
    chn->osd_attr[index].phys_addr[1] = 0;
    chn->osd_attr[index].phys_addr[2] = 0;
}

static int nonai_2d_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct nonai_2d_v4l2_ctx *ctx = container_of(ctrl->handler, struct nonai_2d_v4l2_ctx, ctrl_handler);
    struct nonai_2d_chn_t *chn;

    chn = &g_nonai_2d_chn[ctx->in.chn];

    if (!ctx->ctrl_initialized) {
        return 0;
    }

    switch (ctrl->id) {
        case V4L2_CID_USER_NONAI2D_MODE:
            if(ctrl->val > K_2D_CALC_MODE_BUTT) {
                v4l2_err(&ctx->drv_ctx->v4l2_dev, "set 2d mode failed\n");
                return -EINVAL;
            }
            chn->cfg.nonai_2d_calc_mode = (enum k_nonai_2d_calc_mode)ctrl->val;
            break;
        case V4L2_CID_USER_NONAI2D_OSD_ATTR:
            if (ctrl->p_new.p_u8) {
                nonai2d_osd_attr *attrs = (nonai2d_osd_attr *)ctrl->p_new.p_u8;
                void *dmabuf;
                size_t osd_size;
                uint32_t bytes_per_pixel;
                int index, j;
                dma_addr_t dma_handle;

                for(index = 0; index < NONAI2D_OSD_REGION_NUM; index++) {

                    chn->osd_valid |= attrs[index].valid << index;

                    if(attrs[index].valid == 0) {
                        nonai_2d_release_osd_dmabuf(ctx->drv_ctx, chn, index);
                        continue;
                    }
                    chn->osd_attr[index].width = attrs[index].width;
                    chn->osd_attr[index].height = attrs[index].height;
                    chn->osd_attr[index].startx = attrs[index].startx;
                    chn->osd_attr[index].starty = attrs[index].starty;
                    chn->osd_attr[index].bg_alpha = attrs[index].bg_alpha;
                    chn->osd_attr[index].osd_alpha = attrs[index].osd_alpha;
                    chn->osd_attr[index].video_alpha = attrs[index].video_alpha;
                    chn->osd_attr[index].add_order = (enum k_2d_add_order)attrs[index].add_order;
                    chn->osd_attr[index].bg_color = attrs[index].bg_color;

                    /* Helper to find format */
                    chn->osd_attr[index].fmt = K_2D_OSD_FMT_ARGB8888; // Default
                    bytes_per_pixel = 4;
                    for (j = 0; j < ARRAY_SIZE(formats); j++) {
                        if (formats[j].fourcc == attrs[index].pixfmt) {
                            chn->osd_attr[index].fmt = formats[j].fmt_osd;
                            bytes_per_pixel = DIV_ROUND_UP(formats[j].depth, 8);
                            break;
                        }
                    }

                    if (!attrs[index].data) {
                        v4l2_err(&ctx->drv_ctx->v4l2_dev, "set 2d osd attr[%d] failed, data is null\n", index);
                        return -EINVAL;
                    }

                    osd_size = (size_t)attrs[index].width * attrs[index].height * bytes_per_pixel;
                    if (osd_size == 0) {
                        v4l2_err(&ctx->drv_ctx->v4l2_dev, "set 2d osd attr[%d] failed, invalid size\n", index);
                        return -EINVAL;
                    }

                    if (!chn->osd_dmabuf[index] || chn->osd_dmabuf_size[index] != osd_size) {
                        nonai_2d_release_osd_dmabuf(ctx->drv_ctx, chn, index);

                        dmabuf = dma_alloc_coherent(ctx->drv_ctx->dev, osd_size,
                                        &dma_handle, GFP_KERNEL);
                        if (!dmabuf) {
                            v4l2_err(&ctx->drv_ctx->v4l2_dev,
                                "set 2d osd attr[%d] failed, dma alloc size=%zu\n",
                                index, osd_size);
                            return -ENOMEM;
                        }

                        chn->osd_dmabuf[index] = dmabuf;
                        chn->osd_dmabuf_size[index] = osd_size;
                        chn->osd_dma_handle[index] = dma_handle;
                    }

                    dmabuf = chn->osd_dmabuf[index];
                    if (copy_from_user(dmabuf, (const void __user *)attrs[index].data,
                            osd_size)) {
                        v4l2_err(&ctx->drv_ctx->v4l2_dev,
                            "set 2d osd attr[%d] failed, copy data size=%zu\n",
                            index, osd_size);
                        return -EFAULT;
                    }

                    chn->osd_attr[index].phys_addr[0] = (uint32_t)chn->osd_dma_handle[index];
                    chn->osd_attr[index].phys_addr[1] = 0;
                    chn->osd_attr[index].phys_addr[2] = 0;

                    nonai_2d_info(
                        "set osd_attr idx=%d w=%u h=%u x=%u y=%u bg_a=%u osd_a=%u vid_a=%u order=%u bg=0x%08x pixfmt=0x%08x fmt=%u dma=0x%llx size=%zu valid=0x%x\n",
                        index,
                        attrs[index].width,
                        attrs[index].height,
                        attrs[index].startx,
                        attrs[index].starty,
                        attrs[index].bg_alpha,
                        attrs[index].osd_alpha,
                        attrs[index].video_alpha,
                        attrs[index].add_order,
                        attrs[index].bg_color,
                        attrs[index].pixfmt,
                        chn->osd_attr[index].fmt,
                        (unsigned long long)chn->osd_dma_handle[index],
                        osd_size,
                        chn->osd_valid);
                }
            }
            break;
        case V4L2_CID_USER_NONAI2D_BORDER_ATTR:
            if (ctrl->p_new.p_u8) {
                nonai2d_border_attr *attrs = (nonai2d_border_attr *)ctrl->p_new.p_u8;
                int index;

                index = attrs->index;
                if(index >= K_MAX_2D_BORDER_NUM) {
                    v4l2_err(&ctx->drv_ctx->v4l2_dev, "set 2d border attr failed, invalid index\n");
                    return -EINVAL;
                }
                chn->border_attr[index].width = attrs->width;
                chn->border_attr[index].height = attrs->height;
                chn->border_attr[index].startx = attrs->startx;
                chn->border_attr[index].starty = attrs->starty;
                chn->border_attr[index].line_width = attrs->line_width;
                chn->border_attr[index].color = attrs->color;
                chn->border_valid |= 1 << index;

                nonai_2d_info(
                    "set border_attr idx=%d w=%u h=%u x=%u y=%u line_w=%u color=0x%08x valid=0x%x\n",
                    index,
                    attrs->width,
                    attrs->height,
                    attrs->startx,
                    attrs->starty,
                    attrs->line_width,
                    attrs->color,
                    chn->border_valid);

            }
            break;

        default:
            v4l2_err(&ctx->drv_ctx->v4l2_dev, "unsupported control id 0x%x\n", ctrl->id);
            break;
    }

    return 0;
}

static const struct v4l2_ctrl_ops nonai_2d_ctrl_ops = {
    .s_ctrl = nonai_2d_s_ctrl,
};

static const struct v4l2_ctrl_config nonai_2d_ctrl_mode_cfg = {
    .ops = &nonai_2d_ctrl_ops,
    .id = V4L2_CID_USER_NONAI2D_MODE,
    .name = "nonai2d mode",
    .type = V4L2_CTRL_TYPE_INTEGER,
    .min = 0,
    .max = K_2D_CALC_MODE_BUTT - 1,
    .step = 1,
    .def = K_2D_CALC_MODE_CSC,
};

static const struct v4l2_ctrl_config nonai_2d_ctrl_osd_attr_cfg = {
    .ops = &nonai_2d_ctrl_ops,
    .id = V4L2_CID_USER_NONAI2D_OSD_ATTR,
    .name = "nonai2d osd attr",
    .type = V4L2_CTRL_TYPE_U8,
    .min = 0,
    .max = 0xff,
    .step = 1,
    .def = 0,
    .elem_size = sizeof(u8),
    .dims = { sizeof(nonai2d_osd_attr)*NONAI2D_OSD_REGION_NUM, 0, 0, 0 },
};

static const struct v4l2_ctrl_config nonai_2d_ctrl_border_attr_cfg = {
    .ops = &nonai_2d_ctrl_ops,
    .id = V4L2_CID_USER_NONAI2D_BORDER_ATTR,
    .name = "nonai2d border attr",
    .type = V4L2_CTRL_TYPE_U8,
    .min = 0,
    .max = 0xff,
    .step = 1,
    .def = 0,
    .elem_size = sizeof(u8),
    .dims = { sizeof(nonai2d_border_attr), 0, 0, 0 },
};

static const struct v4l2_ctrl_config nonai_2d_ctrl_coeff_cfg = {
    .ops = &nonai_2d_ctrl_ops,
    .id = V4L2_CID_USER_NONAI2D_COEFF,
    .name = "nonai2d coeff",
    .type = V4L2_CTRL_TYPE_U8,
    .min = 0,
    .max = 0xff,
    .step = 1,
    .def = 0,
    .elem_size = sizeof(u8),
    .dims = { sizeof(nonai2d_coef_attr), 0, 0, 0 },
};

static int nonai_2d_setup_ctrls(struct nonai_2d_v4l2_ctx *ctx)
{
    v4l2_ctrl_handler_init(&ctx->ctrl_handler, 4);

    v4l2_ctrl_new_custom(&ctx->ctrl_handler, &nonai_2d_ctrl_mode_cfg, NULL);
    v4l2_ctrl_new_custom(&ctx->ctrl_handler, &nonai_2d_ctrl_osd_attr_cfg, NULL);
    v4l2_ctrl_new_custom(&ctx->ctrl_handler, &nonai_2d_ctrl_border_attr_cfg, NULL);
    v4l2_ctrl_new_custom(&ctx->ctrl_handler, &nonai_2d_ctrl_coeff_cfg, NULL);

    if (ctx->ctrl_handler.error) {
        int err = ctx->ctrl_handler.error;

        v4l2_err(&ctx->drv_ctx->v4l2_dev, "%s failed, err=%d\n", __func__, err);

        v4l2_ctrl_handler_free(&ctx->ctrl_handler);
        return err;
    }

    return 0;
}

static uint32_t nonai_2d_create_channel(struct nonai_2d_v4l2_ctx *v4l2_ctx,
                    void __iomem *regs_2d)
{
    struct nonai_2d_chn_t *chn;

    chn = &g_nonai_2d_chn[v4l2_ctx->in.chn];

    memset(chn, 0, sizeof(struct nonai_2d_chn_t));

    chn->cfg.nonai_2d_calc_mode = K_2D_CALC_MODE_CSC;
    chn->chn_id = v4l2_ctx->in.chn;
    chn->stream_offset = regs_2d;

    chn->state = NONAI_2D_CH_STATE_START; //NONAI_2D_CH_STATE_INIT;
    return 0;
}

static uint32_t nonai_2d_destory_channel(struct nonai_2d_v4l2_ctx *v4l2_ctx)
{
    struct nonai_2d_chn_t *chn;
    int i;

    chn = &g_nonai_2d_chn[v4l2_ctx->in.chn];

    for (i = 0; i < K_MAX_2D_OSD_REGION_NUM; i++)
        nonai_2d_release_osd_dmabuf(v4l2_ctx->drv_ctx, chn, i);

    chn->state = NONAI_2D_CH_STATE_IDLE;

    return 0;
}

static int find_dev_nun(struct nonai_2d_drv_ctx *drv_ctx)
{
    for (int i = 0; i < NONAI_2D_MAX_CHN_NUMS; i++) {
        if (drv_ctx->dev_num[i] == 0)
            return i;
    }
    return -1;
}

static int nonai_2d_open(struct file *file)
{
    struct nonai_2d_drv_ctx *drv_ctx = video_drvdata(file);
    struct nonai_2d_v4l2_ctx *v4l2_ctx = NULL;
    int ret = 0;

    drv_ctx->num = find_dev_nun(drv_ctx);
    if (drv_ctx->num < 0)
        return -ENOMEM;

    ret = pm_runtime_get_sync(drv_ctx->dev);
    if (ret < 0) {
        v4l2_err(&drv_ctx->v4l2_dev, "2d pm_runtime_get_sync failed\n");
        return ret;
    }

    v4l2_ctx = kzalloc(sizeof(*v4l2_ctx), GFP_KERNEL);
    if (!v4l2_ctx)
        return -ENOMEM;
    v4l2_ctx->drv_ctx = drv_ctx;
    /* Set default formats */
    v4l2_ctx->in = def_frame;
    v4l2_ctx->out = def_frame;
    nonai_2d_update_frame_layout(&v4l2_ctx->in);
    nonai_2d_update_frame_layout(&v4l2_ctx->out);

    if (mutex_lock_interruptible(&drv_ctx->mutex)) {
        kfree(v4l2_ctx);
        return -ERESTARTSYS;
    }

    v4l2_ctx->in.chn = drv_ctx->num;
    v4l2_ctx->out.chn = drv_ctx->num;

    if (drv_ctx->num > NONAI_2D_MAX_CHN_NUMS) {
        v4l2_err(&drv_ctx->v4l2_dev, "2d drv_ctx->num = %d failed ,only support 10 chn\n", drv_ctx->num);
        mutex_unlock(&drv_ctx->mutex);
        kfree(v4l2_ctx);
        return -EBUSY;
    }

    // creat channel
    nonai_2d_create_channel(v4l2_ctx, drv_ctx->regs_2d);

    v4l2_ctx->fh.m2m_ctx =
        v4l2_m2m_ctx_init(drv_ctx->m2m_dev, v4l2_ctx, &queue_init);
    if (IS_ERR(v4l2_ctx->fh.m2m_ctx)) {
        ret = PTR_ERR(v4l2_ctx->fh.m2m_ctx);
        mutex_unlock(&drv_ctx->mutex);
        kfree(v4l2_ctx);
        return ret;
    }

    /* Non-CSC mode can run without queued src buffers. */
    v4l2_m2m_set_src_buffered(v4l2_ctx->fh.m2m_ctx, true);

    v4l2_fh_init(&v4l2_ctx->fh, video_devdata(file));
    file->private_data = &v4l2_ctx->fh;
    v4l2_fh_add(&v4l2_ctx->fh);

    nonai_2d_setup_ctrls(v4l2_ctx);

    /* Write the default values to the ctx struct */
    v4l2_ctrl_handler_setup(&v4l2_ctx->ctrl_handler);
    v4l2_ctx->ctrl_initialized = true;

    v4l2_ctx->fh.ctrl_handler = &v4l2_ctx->ctrl_handler;

    // drv_ctx->num = drv_ctx->num + 1 ;
    mutex_unlock(&drv_ctx->mutex);

    return 0;
}

static void nonai_2d_config_job(struct nonai_2d_job *job, struct nonai_2d_v4l2_ctx *v4l2_ctx)
{
    uint32_t chn_id = 0;
    struct k_video_frame_info in_frame;
    struct k_video_frame_info out_frame;
    struct nonai_2d_chn_t *chn;

    memset(&in_frame, 0, sizeof(in_frame));
    memset(&out_frame, 0, sizeof(out_frame));

    chn_id = v4l2_ctx->in.chn;
    chn = &g_nonai_2d_chn[chn_id];

    job->chn_id = chn_id;

    //config job in frame
    if (chn->cfg.nonai_2d_calc_mode == nonai_2d_calc_mode_csc) {
        // set in frame
        in_frame.v_frame.pixel_format = v4l2_ctx->in.fmt.pixel_format;
        in_frame.v_frame.width = v4l2_ctx->in.width;
        in_frame.v_frame.height = v4l2_ctx->in.height;
        in_frame.v_frame.phys_addr[0] = v4l2_ctx->in.addr[0];
        in_frame.v_frame.phys_addr[1] = v4l2_ctx->in.addr[1];
        in_frame.v_frame.phys_addr[2] = v4l2_ctx->in.addr[2];

        // set out frame
        out_frame.v_frame.pixel_format = v4l2_ctx->out.fmt.pixel_format;
        out_frame.v_frame.width = v4l2_ctx->out.width;
        out_frame.v_frame.height = v4l2_ctx->out.height;
        out_frame.v_frame.phys_addr[0] = v4l2_ctx->out.addr[0];
        out_frame.v_frame.phys_addr[1] = v4l2_ctx->out.addr[1];
        out_frame.v_frame.phys_addr[2] = v4l2_ctx->out.addr[2];

        memcpy(&job->in_frame, &in_frame, sizeof(struct k_video_frame_info));
        memcpy(&job->out_frame, &out_frame, sizeof(struct k_video_frame_info));
    } else {
        // OSD/BORDER works in-place on dst(video) buffer
        in_frame.v_frame.pixel_format = v4l2_ctx->out.fmt.pixel_format;
        in_frame.v_frame.width = v4l2_ctx->out.width;
        in_frame.v_frame.height = v4l2_ctx->out.height;
        in_frame.v_frame.phys_addr[0] = v4l2_ctx->out.addr[0];
        in_frame.v_frame.phys_addr[1] = v4l2_ctx->out.addr[1];
        in_frame.v_frame.phys_addr[2] = v4l2_ctx->out.addr[2];

        chn->src_type = get_video_type(in_frame.v_frame.pixel_format);
        chn->dst_type = chn->src_type;

        memcpy(&job->in_frame, &in_frame, sizeof(struct k_video_frame_info));
    }

    nonai_2d_cfg_hw(job);
}

static void device_run(void *prv)
{
    struct nonai_2d_v4l2_ctx *v4l2_ctx = prv;
    struct nonai_2d_drv_ctx *drv_ctx = v4l2_ctx->drv_ctx;
    struct nonai_2d_chn_t *chn = &g_nonai_2d_chn[v4l2_ctx->in.chn];
    struct vb2_v4l2_buffer *src, *dst;
    unsigned long flags;
    int plane;
    int src_planes;
    int dst_planes;
    struct nonai_2d_job job;

    spin_lock_irqsave(&drv_ctx->ctrl_lock, flags);

    drv_ctx->v4l2_ctx = v4l2_ctx;

    if(chn->cfg.nonai_2d_calc_mode == nonai_2d_calc_mode_csc) {
         src = v4l2_m2m_next_src_buf(v4l2_ctx->fh.m2m_ctx);
        dst = v4l2_m2m_next_dst_buf(v4l2_ctx->fh.m2m_ctx);

        if (src == NULL || dst == NULL) {
            v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d src or dst is NULL\n");
            spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
            return;
        }

        src_planes = src->vb2_buf.num_planes > 3 ? 3 : src->vb2_buf.num_planes;
        dst_planes = dst->vb2_buf.num_planes > 3 ? 3 : dst->vb2_buf.num_planes;

        for (plane = 0; plane < 3; plane++) {
            v4l2_ctx->in.addr[plane] = 0;
            v4l2_ctx->out.addr[plane] = 0;
        }

        for (plane = 0; plane < src_planes; plane++)
            v4l2_ctx->in.addr[plane] = vb2_dma_contig_plane_dma_addr(&src->vb2_buf, plane);

        for (plane = 0; plane < dst_planes; plane++)
            v4l2_ctx->out.addr[plane] = vb2_dma_contig_plane_dma_addr(&dst->vb2_buf, plane);

        if (!v4l2_ctx->in.addr[0] || !v4l2_ctx->out.addr[0]) {
            v4l2_err(&drv_ctx->v4l2_dev,
                "nonai 2d invalid dma addr src=[0x%llx,0x%llx,0x%llx] dst=[0x%llx,0x%llx,0x%llx]\n",
                (unsigned long long)v4l2_ctx->in.addr[0],
                (unsigned long long)v4l2_ctx->in.addr[1],
                (unsigned long long)v4l2_ctx->in.addr[2],
                (unsigned long long)v4l2_ctx->out.addr[0],
                (unsigned long long)v4l2_ctx->out.addr[1],
                (unsigned long long)v4l2_ctx->out.addr[2]);
            spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
            return;
        }
    } else {
        dst = v4l2_m2m_next_dst_buf(v4l2_ctx->fh.m2m_ctx);

        if (!dst) {
            v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d dst is NULL\n");
            spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
            return;
        }

        dst_planes = dst->vb2_buf.num_planes > 3 ? 3 : dst->vb2_buf.num_planes;

        for (plane = 0; plane < 3; plane++) {
            v4l2_ctx->in.addr[plane] = 0;
            v4l2_ctx->out.addr[plane] = 0;
        }

        for (plane = 0; plane < dst_planes; plane++)
            v4l2_ctx->out.addr[plane] = vb2_dma_contig_plane_dma_addr(&dst->vb2_buf, plane);

        /* OSD/BORDER mode works in-place on capture buffer only. */
        for (plane = 0; plane < dst_planes; plane++)
            v4l2_ctx->in.addr[plane] = v4l2_ctx->out.addr[plane];

        if (!v4l2_ctx->out.addr[0]) {
            v4l2_err(&drv_ctx->v4l2_dev,
                "nonai 2d invalid dma addr src=[0x%llx,0x%llx,0x%llx] dst=[0x%llx,0x%llx,0x%llx]\n",
                (unsigned long long)v4l2_ctx->in.addr[0],
                (unsigned long long)v4l2_ctx->in.addr[1],
                (unsigned long long)v4l2_ctx->in.addr[2],
                (unsigned long long)v4l2_ctx->out.addr[0],
                (unsigned long long)v4l2_ctx->out.addr[1],
                (unsigned long long)v4l2_ctx->out.addr[2]);
            spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
            return;
        }
    }

    nonai_2d_info("nonai 2d device_run in.addr=[0x%llx,0x%llx,0x%llx] out.addr=[0x%llx,0x%llx,0x%llx]\n",
            (unsigned long long)v4l2_ctx->in.addr[0],
            (unsigned long long)v4l2_ctx->in.addr[1],
            (unsigned long long)v4l2_ctx->in.addr[2],
            (unsigned long long)v4l2_ctx->out.addr[0],
            (unsigned long long)v4l2_ctx->out.addr[1],
            (unsigned long long)v4l2_ctx->out.addr[2]);

    nonai_2d_config_job(&job, v4l2_ctx);

    spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);
}

static int nonai_2d_release(struct file *file)
{
    struct nonai_2d_v4l2_ctx *v4l2_ctx = container_of(file->private_data, struct nonai_2d_v4l2_ctx, fh);
    struct nonai_2d_drv_ctx *drv_ctx = v4l2_ctx->drv_ctx;

    mutex_lock(&drv_ctx->mutex);

    //destroy channel
    nonai_2d_destory_channel(v4l2_ctx);

    v4l2_m2m_ctx_release(v4l2_ctx->fh.m2m_ctx);

    v4l2_ctrl_handler_free(&v4l2_ctx->ctrl_handler);
    v4l2_fh_del(&v4l2_ctx->fh);
    v4l2_fh_exit(&v4l2_ctx->fh);
    kfree(v4l2_ctx);

    mutex_unlock(&drv_ctx->mutex);
    return 0;
}

static int nonai_2d_job_ready(void *priv)
{
    struct nonai_2d_v4l2_ctx *ctx = priv;
    struct nonai_2d_chn_t *chn;
    unsigned int src_ready;
    unsigned int dst_ready;

    if (!ctx || !ctx->fh.m2m_ctx)
        return 0;

    chn = &g_nonai_2d_chn[ctx->in.chn];

    src_ready = v4l2_m2m_num_src_bufs_ready(ctx->fh.m2m_ctx);
    dst_ready = v4l2_m2m_num_dst_bufs_ready(ctx->fh.m2m_ctx);

    if (chn->cfg.nonai_2d_calc_mode == nonai_2d_calc_mode_csc)
        return src_ready > 0 && dst_ready > 0;

    return dst_ready > 0;
}

static const struct v4l2_m2m_ops nonai_2d_m2m_ops = {
    .device_run = device_run,
    .job_ready = nonai_2d_job_ready,
};

static int nonai_2d_vidioc_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
    struct video_device *vdev = video_devdata(file);

    strscpy(cap->driver, NONAI_2D_NAME, sizeof(cap->driver));
    strscpy(cap->card, "Canaan nonai 2D", sizeof(cap->card));
    strscpy(cap->bus_info, "platform:nonai-2d", sizeof(cap->bus_info));

    cap->device_caps = vdev ? vdev->device_caps :
                              (V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING);
    cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

    return 0;
}

static int nonai_2d_vidioc_enum_fmt(struct file *file, void *prv,
                    struct v4l2_fmtdesc *f)
{
    struct nonai_2d_format *fmt;

    if (f->index >= NUM_FORMATS)
        return -EINVAL;

    fmt = &formats[f->index];
    f->pixelformat = fmt->fourcc;

    return 0;
}

static int nonai_2d_vidioc_g_fmt(struct file *file, void *prv, struct v4l2_format *f)
{
    struct nonai_2d_v4l2_ctx *ctx = prv;
    struct vb2_queue *vq;
    struct nonai_2d_frame *frm;
    uint32_t bytesperline[3];
    uint32_t sizeimage[3];
    uint32_t plane;

    vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, f->type);
    if (!vq)
        return -EINVAL;
    frm = nonai_2d_get_frame(ctx, f->type);
    if (IS_ERR(frm))
        return PTR_ERR(frm);

    nonai_2d_fill_plane_layout(&frm->fmt, frm->width, frm->height, bytesperline, sizeimage);

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        f->fmt.pix_mp.width = frm->width;
        f->fmt.pix_mp.height = frm->height;
        f->fmt.pix_mp.field = V4L2_FIELD_NONE;
        f->fmt.pix_mp.pixelformat = frm->fmt.fourcc;
        f->fmt.pix_mp.num_planes = frm->num_planes;
        for (plane = 0; plane < frm->num_planes; plane++) {
            f->fmt.pix_mp.plane_fmt[plane].bytesperline = bytesperline[plane];
            f->fmt.pix_mp.plane_fmt[plane].sizeimage = sizeimage[plane];
        }
        f->fmt.pix_mp.colorspace = frm->colorspace;
    } else {
        f->fmt.pix.width = frm->width;
        f->fmt.pix.height = frm->height;
        f->fmt.pix.field = V4L2_FIELD_NONE;
        f->fmt.pix.pixelformat = frm->fmt.fourcc;
        f->fmt.pix.bytesperline = frm->stride;
        f->fmt.pix.sizeimage = frm->size;
        f->fmt.pix.colorspace = frm->colorspace;
    }
    return 0;
}

static int nonai_2d_vidioc_try_fmt(struct file *file, void *prv, struct v4l2_format *f)
{
    struct nonai_2d_format *fmt;
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t bytesperline[3];
    uint32_t sizeimage[3];
    uint32_t plane;

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        width = f->fmt.pix_mp.width;
        height = f->fmt.pix_mp.height;
        pixelformat = f->fmt.pix_mp.pixelformat;
    } else {
        width = f->fmt.pix.width;
        height = f->fmt.pix.height;
        pixelformat = f->fmt.pix.pixelformat;
    }

    fmt = nonai_2d_fmt_find(f);
    if (!fmt) {
        fmt = &formats[0];
        pixelformat = fmt->fourcc;
    }

    if (width > MAX_WIDTH)
        width = MAX_WIDTH;
    if (height > MAX_HEIGHT)
        height = MAX_HEIGHT;

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        f->fmt.pix_mp.width = width;
        f->fmt.pix_mp.height = height;
        f->fmt.pix_mp.field = V4L2_FIELD_NONE;
        f->fmt.pix_mp.pixelformat = pixelformat;
        f->fmt.pix_mp.num_planes = fmt->plane ? fmt->plane : 1;
        nonai_2d_fill_plane_layout(fmt, width, height, bytesperline, sizeimage);
        for (plane = 0; plane < f->fmt.pix_mp.num_planes; plane++) {
            f->fmt.pix_mp.plane_fmt[plane].bytesperline = bytesperline[plane];
            f->fmt.pix_mp.plane_fmt[plane].sizeimage = sizeimage[plane];
        }
    } else {
        f->fmt.pix.width = width;
        f->fmt.pix.height = height;
        f->fmt.pix.field = V4L2_FIELD_NONE;
        f->fmt.pix.pixelformat = pixelformat;
        f->fmt.pix.bytesperline = (width * fmt->depth) >> 3;
        f->fmt.pix.sizeimage = (height * (width * fmt->depth)) >> 3;
    }

    return 0;
}

static int nonai_2d_vidioc_s_fmt(struct file *file, void *prv, struct v4l2_format *f)
{
    struct nonai_2d_v4l2_ctx *ctx = prv;
    struct nonai_2d_drv_ctx *drv_ctx = ctx->drv_ctx;
    struct vb2_queue *vq;
    struct nonai_2d_frame *frm;
    struct nonai_2d_format *fmt;
    int ret = 0;
    struct nonai_2d_chn_t *chn;

    chn = &g_nonai_2d_chn[ctx->in.chn];

    /* Adjust all values accordingly to the hardware capabilities
     * and chosen format.
     */
    ret = nonai_2d_vidioc_try_fmt(file, prv, f);
    if (ret)
    {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d failed, line %d\n", __LINE__);
        return ret;
    }

    vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, f->type);
    if (vb2_is_busy(vq)) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d failed, line %d\n", __LINE__);
        return -EBUSY;
    }

    frm = nonai_2d_get_frame(ctx, f->type);
    if (IS_ERR(frm)) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d failed, line %d\n", __LINE__);
        return PTR_ERR(frm);
    }

    fmt = nonai_2d_fmt_find(f);
    if (!fmt) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d failed, line %d\n", __LINE__);
        return -EINVAL;
    }

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ||
        f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        frm->width = f->fmt.pix_mp.width;
        frm->height = f->fmt.pix_mp.height;
        frm->size = f->fmt.pix_mp.plane_fmt[0].sizeimage;
        frm->stride = f->fmt.pix_mp.plane_fmt[0].bytesperline;
        frm->colorspace = f->fmt.pix_mp.colorspace;
        frm->num_planes = f->fmt.pix_mp.num_planes;
    } else {
        frm->width = f->fmt.pix.width;
        frm->height = f->fmt.pix.height;
        frm->size = f->fmt.pix.sizeimage;
        frm->stride = f->fmt.pix.bytesperline;
        frm->colorspace = f->fmt.pix.colorspace;
        frm->num_planes = 1;
    }

    memcpy(&frm->fmt, fmt, sizeof(*fmt));
    nonai_2d_update_frame_layout(frm);

    if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
        f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        chn->src_fmt = fmt->fmt_2d;
        chn->src_type = get_video_type(fmt->pixel_format);
        chn->in_pic_format = fmt->pixel_format;
    } else {
        chn->dst_fmt = fmt->fmt_2d;
        chn->dst_type = get_video_type(fmt->pixel_format);
        chn->out_pic_format = fmt->pixel_format;
    }
    return 0;
}

#if NONAI_2D_DUMP
static const char *nonai_2d_buf_type_name(enum v4l2_buf_type type)
{
    switch (type) {
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
        return "output";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
        return "capture";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
        return "output_mplane";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
        return "capture_mplane";
    default:
        return "unknown";
    }
}

static void nonai_2d_dump_buf_to_file(struct nonai_2d_drv_ctx *drv_ctx,
                    struct vb2_buffer *vb,
                    enum v4l2_buf_type type,
                    uint32_t index,
                    int is_qbuf)
{
    char file_name[160];
    const char *type_name = nonai_2d_buf_type_name(type);
    struct file *fp;
    loff_t pos = 0;
    uint32_t plane;

    if(type == V4L2_BUF_TYPE_VIDEO_OUTPUT || type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        snprintf(file_name, sizeof(file_name), "/sharefs/app/nonai2d_%s_%s_idx%u.rgb", is_qbuf ? "qbuf" : "dqbuf", type_name, index);
    else
        snprintf(file_name, sizeof(file_name), "/sharefs/app/nonai2d_%s_%s_idx%u.yuv", is_qbuf ? "qbuf" : "dqbuf", type_name, index);

    fp = filp_open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (IS_ERR(fp)) {
        v4l2_err(&drv_ctx->v4l2_dev,
             "dump open failed %s err=%ld\n",
             file_name, PTR_ERR(fp));
        return;
    }

    for (plane = 0; plane < vb->num_planes; plane++) {
        void *vaddr;
        size_t dump_size;
        ssize_t write_size;
        struct dma_buf *dbuf = NULL;
        struct iosys_map map = { 0 };
        bool dmabuf_mapped = false;
        bool dmabuf_cpu_access = false;
        int ret;

        vaddr = vb2_plane_vaddr(vb, plane);
        dump_size = vb2_get_plane_payload(vb, plane);
        if (dump_size == 0)
            dump_size = vb2_plane_size(vb, plane);

        if (!vaddr && vb->vb2_queue->memory == VB2_MEMORY_DMABUF) {
            dbuf = vb->planes[plane].dbuf;
            if (!dbuf) {
                v4l2_err(&drv_ctx->v4l2_dev,
                     "dump skip type=%u idx=%u plane=%u no dmabuf\n",
                     type, index, plane);
                continue;
            }

            ret = dma_buf_begin_cpu_access(dbuf, DMA_BIDIRECTIONAL);
            if (ret) {
                v4l2_err(&drv_ctx->v4l2_dev,
                     "dump cpu_access failed type=%u idx=%u plane=%u err=%d\n",
                     type, index, plane, ret);
                continue;
            }
            dmabuf_cpu_access = true;

            ret = dma_buf_vmap(dbuf, &map);
            if (ret) {
                v4l2_err(&drv_ctx->v4l2_dev,
                     "dump vmap failed type=%u idx=%u plane=%u err=%d\n",
                     type, index, plane, ret);
                dma_buf_end_cpu_access(dbuf, DMA_BIDIRECTIONAL);
                continue;
            }

            dmabuf_mapped = true;
            vaddr = map.vaddr;
        }

        if (!vaddr || dump_size == 0) {
            v4l2_err(&drv_ctx->v4l2_dev,
                 "dump skip type=%u idx=%u plane=%u vaddr=%px size=%zu\n",
                 type, index, plane, vaddr, dump_size);
            if (dmabuf_mapped)
                dma_buf_vunmap(dbuf, &map);
            if (dmabuf_cpu_access)
                dma_buf_end_cpu_access(dbuf, DMA_BIDIRECTIONAL);
            continue;
        }

        write_size = kernel_write(fp, vaddr, dump_size, &pos);
        if (write_size < 0) {
            v4l2_err(&drv_ctx->v4l2_dev,
                 "dump write failed %s err=%zd\n",
                 file_name, write_size);
        } else {
            nonai_2d_dbg("nonai2d dump file=%s type=%u idx=%u plane=%u size=%zd\n",
                file_name, type, index, plane, write_size);
        }

        if (dmabuf_mapped)
            dma_buf_vunmap(dbuf, &map);
        if (dmabuf_cpu_access)
            dma_buf_end_cpu_access(dbuf, DMA_BIDIRECTIONAL);
    }

    filp_close(fp, NULL);
    v4l2_err(&drv_ctx->v4l2_dev, "nonai2d dump %s\n", file_name);
}
#endif
static int nonai_2d_v4l2_m2m_ioctl_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct nonai_2d_v4l2_ctx *ctx = fh;
    struct nonai_2d_drv_ctx *drv_ctx = ctx->drv_ctx;
    int ret = 0;

    ret = v4l2_m2m_ioctl_qbuf(file, fh, b);
    if (ret) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d qbuf failed, line %d\n", __LINE__);
        return ret;
    }

#if NONAI_2D_DUMP
    uint32_t index;
    struct vb2_queue *vq;
    struct vb2_buffer *vb;
    struct nonai_2d_chn_t *chn;

    chn = &g_nonai_2d_chn[ctx->in.chn];

    index = b->index;
    vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, b->type);
    if (!vq) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d qbuf get_vq failed, type=%u\n", b->type);
        return ret;
    }

    vb = vb2_get_buffer(vq, index);
    if (!vb) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d qbuf get_buffer failed, type=%u index=%u\n", b->type, index);
        return ret;
    }

    if(chn->total_out_cnt == 10) {
        nonai_2d_dump_buf_to_file(drv_ctx, vb, b->type, index, 1);
    }
#endif

    return ret;
}

static int nonai_2d_v4l2_m2m_ioctl_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct nonai_2d_v4l2_ctx *ctx = fh;
    struct nonai_2d_drv_ctx *drv_ctx = ctx->drv_ctx;
    int ret = 0;

    ret = v4l2_m2m_ioctl_dqbuf(file, fh, b);

    if (ret) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d dqbuf failed, line %d, type=%u, index=%u, err=%d\n",
                __LINE__, b->type, b->index, ret);
        return ret;
    }

#if NONAI_2D_DUMP
    struct nonai_2d_chn_t *chn;
    uint32_t index;
    struct vb2_queue *vq;
    struct vb2_buffer *vb;

    chn = &g_nonai_2d_chn[ctx->in.chn];

    index = b->index;
    vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx, b->type);
    if (!vq) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d dqbuf get_vq failed, type=%u\n", b->type);
        return ret;
    }

    vb = vb2_get_buffer(vq, index);
    if (!vb) {
        v4l2_err(&drv_ctx->v4l2_dev, "nonai 2d dqbuf get_buffer failed, type=%u index=%u\n", b->type, index);
        return ret;
    }

    if(chn->total_out_cnt == 11) {
        nonai_2d_dump_buf_to_file(drv_ctx, vb, b->type, index, 0);
    }
#endif

    return ret;
}


static const struct v4l2_ioctl_ops nonai_2d_ioctl_ops = {
    .vidioc_querycap = nonai_2d_vidioc_querycap,

    .vidioc_enum_fmt_vid_cap = nonai_2d_vidioc_enum_fmt,
    .vidioc_enum_fmt_vid_out = nonai_2d_vidioc_enum_fmt,

    .vidioc_g_fmt_vid_cap = nonai_2d_vidioc_g_fmt,
    .vidioc_g_fmt_vid_out = nonai_2d_vidioc_g_fmt,

    .vidioc_try_fmt_vid_cap = nonai_2d_vidioc_try_fmt,
    .vidioc_try_fmt_vid_out = nonai_2d_vidioc_try_fmt,

    .vidioc_s_fmt_vid_cap = nonai_2d_vidioc_s_fmt,
    .vidioc_s_fmt_vid_out = nonai_2d_vidioc_s_fmt,

    .vidioc_enum_fmt_vid_cap = nonai_2d_vidioc_enum_fmt,
    .vidioc_enum_fmt_vid_out = nonai_2d_vidioc_enum_fmt,

    .vidioc_g_fmt_vid_cap_mplane = nonai_2d_vidioc_g_fmt,
    .vidioc_g_fmt_vid_out_mplane = nonai_2d_vidioc_g_fmt,

    .vidioc_try_fmt_vid_cap_mplane = nonai_2d_vidioc_try_fmt,
    .vidioc_try_fmt_vid_out_mplane = nonai_2d_vidioc_try_fmt,

    .vidioc_s_fmt_vid_cap_mplane = nonai_2d_vidioc_s_fmt,
    .vidioc_s_fmt_vid_out_mplane = nonai_2d_vidioc_s_fmt,

    .vidioc_reqbufs = v4l2_m2m_ioctl_reqbufs,
    .vidioc_querybuf = v4l2_m2m_ioctl_querybuf,
    .vidioc_qbuf = nonai_2d_v4l2_m2m_ioctl_qbuf,
    .vidioc_dqbuf = nonai_2d_v4l2_m2m_ioctl_dqbuf,
    .vidioc_prepare_buf = v4l2_m2m_ioctl_prepare_buf,
    .vidioc_create_bufs = v4l2_m2m_ioctl_create_bufs,
    .vidioc_expbuf = v4l2_m2m_ioctl_expbuf,
    .vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
    .vidioc_unsubscribe_event = v4l2_event_unsubscribe,
    .vidioc_streamon = v4l2_m2m_ioctl_streamon,
    .vidioc_streamoff = v4l2_m2m_ioctl_streamoff,
};

static const struct v4l2_file_operations nonai_2d_fops = {
    .owner = THIS_MODULE,
    .open = nonai_2d_open,
    .release = nonai_2d_release,
    .poll = v4l2_m2m_fop_poll,
    .unlocked_ioctl = video_ioctl2,
    .mmap = v4l2_m2m_fop_mmap,
};

static const struct video_device nonai_2d_videodev = {
    .name = "canaan-non-ai-2d",
    .fops = &nonai_2d_fops,
    .ioctl_ops = &nonai_2d_ioctl_ops,
    .minor = -1,
    .release = video_device_release,
    .vfl_dir = VFL_DIR_M2M,
    .device_caps = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING,
};

static irqreturn_t isr_2d(int irq, void *prv)
{
    struct nonai_2d_drv_ctx *drv_ctx = prv;
    struct nonai_2d_v4l2_ctx *curr_ctx = drv_ctx->v4l2_ctx;
    struct vb2_v4l2_buffer *src_buf, *dst_buf;
    unsigned long flags;
    uint32_t chn_id = 0;
    struct nonai_2d_chn_t *chn;
    uint32_t reg = 0;

    chn_id = curr_ctx->in.chn;
    chn = &g_nonai_2d_chn[chn_id];

    reg = readl(drv_ctx->regs_2d + 0x3a8);

    nonai_2d_main_clear_intr(chn->stream_offset);

    spin_lock_irqsave(&drv_ctx->ctrl_lock, flags);

    if (chn->cfg.nonai_2d_calc_mode == nonai_2d_calc_mode_csc) {
        src_buf = v4l2_m2m_src_buf_remove(curr_ctx->fh.m2m_ctx);
        dst_buf = v4l2_m2m_dst_buf_remove(curr_ctx->fh.m2m_ctx);

        if (src_buf && dst_buf) {
            v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_DONE);
            v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_DONE);
            v4l2_m2m_job_finish(drv_ctx->m2m_dev, curr_ctx->fh.m2m_ctx);
        }
    } else {
        dst_buf = v4l2_m2m_dst_buf_remove(curr_ctx->fh.m2m_ctx);
        while ((src_buf = v4l2_m2m_src_buf_remove(curr_ctx->fh.m2m_ctx))) {
            v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_DONE);
        }

        if (dst_buf)
            v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_DONE);

        if (dst_buf)
            v4l2_m2m_job_finish(drv_ctx->m2m_dev, curr_ctx->fh.m2m_ctx);
    }

    spin_unlock_irqrestore(&drv_ctx->ctrl_lock, flags);

    return IRQ_HANDLED;
}

static int nonai_2d_probe(struct platform_device *pdev)
{
    int ret;
    unsigned int irq_2d;
    struct nonai_2d_drv_ctx *drv_ctx;
    struct video_device *vfd;

    dev_info(&pdev->dev, "%s>start\n", __func__);

    drv_ctx = devm_kzalloc(&pdev->dev, sizeof(struct nonai_2d_drv_ctx), GFP_KERNEL);
    if (!drv_ctx) {
        dev_err(&pdev->dev, "failed to allocate memory for 2d_ctx\n");
        return -ENOMEM;
    }

    memset(drv_ctx, 0, sizeof(struct nonai_2d_drv_ctx));

    memset(drv_ctx->dev_num, 0, sizeof(uint32_t) * NONAI_2D_MAX_CHN_NUMS);

    drv_ctx->dev = &pdev->dev;
    spin_lock_init(&drv_ctx->ctrl_lock);
    mutex_init(&drv_ctx->mutex);

    pm_runtime_enable(drv_ctx->dev);


    drv_ctx->regs_2d = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(drv_ctx->regs_2d)) {
        ret = PTR_ERR(drv_ctx->regs_2d);
        dev_err(&pdev->dev, "failed to ioremap regs_2d\n");
        return ret;
    }

    irq_2d = platform_get_irq(pdev, 0);
    if (irq_2d < 0) {
        ret = irq_2d;
        dev_err(&pdev->dev, "failed to get irq for 2d\n");
        return ret;
    }

    ret = devm_request_irq(drv_ctx->dev, irq_2d, isr_2d, 0, dev_name(drv_ctx->dev), drv_ctx);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to request irq for 2d\n");
        return ret;
    }

    ret = v4l2_device_register(&pdev->dev, &drv_ctx->v4l2_dev);
    if (ret) {
        dev_err(&pdev->dev, "failed to register v4l2\n");
        return ret;
    }

    vfd = video_device_alloc();
    if (!vfd) {
        dev_err(&pdev->dev, "failed to alloc device for 2d\n");
        return -ENOMEM;
    }

    *vfd = nonai_2d_videodev;
    vfd->lock = &drv_ctx->mutex;
    vfd->v4l2_dev = &drv_ctx->v4l2_dev;

    video_set_drvdata(vfd, drv_ctx);
    drv_ctx->vfd = vfd;

    platform_set_drvdata(pdev, drv_ctx);
    drv_ctx->m2m_dev = v4l2_m2m_init(&nonai_2d_m2m_ops);
    if (IS_ERR(drv_ctx->m2m_dev)) {
        v4l2_err(&drv_ctx->v4l2_dev, "failed to init mem2mem device for 2d\n");
        ret = PTR_ERR(drv_ctx->m2m_dev);
        return ret;
    }

    ret = video_register_device(vfd, VFL_TYPE_VIDEO, -1);
    if (ret) {
        v4l2_err(&drv_ctx->v4l2_dev, "Failed to register video device for 2d\n");
        return ret;
    }

    dev_info(&pdev->dev, "nonai_2d probe success\n");

    return 0;
}

static int nonai_2d_remove(struct platform_device *pdev)
{
    struct nonai_2d_drv_ctx *drv_ctx = platform_get_drvdata(pdev);


    v4l2_m2m_release(drv_ctx->m2m_dev);
    video_unregister_device(drv_ctx->vfd);
    v4l2_device_unregister(&drv_ctx->v4l2_dev);

    return 0;
}

static const struct of_device_id canaan_nonai_2d_match[] = {
    { .compatible = "canaan,nonai-2d" },
    { { 0 } }
};

MODULE_DEVICE_TABLE(of, canaan_nonai_2d_match);

static struct platform_driver nonai_2d_drv = {
    .probe = nonai_2d_probe,
    .remove = nonai_2d_remove,
    .driver = {
        .name = NONAI_2D_NAME,
        .of_match_table = canaan_nonai_2d_match,
    },
};

module_platform_driver(nonai_2d_drv);

MODULE_AUTHOR("Canaan");
MODULE_DESCRIPTION("Canaan 2d Graphic Acceleration Unit");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(DMA_BUF);