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

#include <linux/platform_device.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-dma-sg.h>
#include <media/videobuf2-v4l2.h>

#include "k_nonai_2d_comm.h"
#include "nonai_2d.h"
#include "nonai_2d_reg.h"

static struct nonai_2d_dev_t g_nonai_2d;
void *(NONAI_2D_REG) = NULL;
struct nonai_2d_chn_t g_nonai_2d_chn[NONAI_2D_MAX_CHN_NUMS];

const int16_t RGB2YUV_BT601_FULL[12] = { 77, 150, 29, 0, -43, -85, 128, 128, 128, -107, -21, 128 };
const int16_t RGB2YUV_BT601_LIMITED[12] = { 66, 129, 25, 16, -38, -74, 112, 128, 112, -94, -18, 128 };
const int16_t RGB2YUV_BT709_FULL[12] = { 54, 183, 19, 0, -29, -99, 128, 128, 128, -116, -12, 128 };
const int16_t RGB2YUV_BT709_LIMITED[12] = { 47, 157, 16, 16, -26, -86, 112, 128, 112, -102, -10, 128 };
const int16_t RGB2YUV_BT2020[12] = { 67, 174, 15, 0, -29, -99, 128, 128, 128, -116, -12, 128 };

const int16_t YUV2RGB_BT601_FULL[12] = { 256, 0, 359, -180, 256, -88, -183, 135, 256, 453, 0, -227 };
const int16_t YUV2RGB_BT601_LIMITED[12] = { 298, 0, 409, -223, 298, -100, -208, 135, 298, 516, 0, -277 };
const int16_t YUV2RGB_BT709_FULL[12] = { 256, 0, 403, -202, 256, -48, -120, 84, 256, 475, 0, -238 };
const int16_t YUV2RGB_BT709_LIMITED[12] = { 298, 0, 459, -248, 298, -55, -136, 77, 298, 541, 0, -289 };
const int16_t YUV2RGB_BT2020[12] = { 256, 0, 377, -189, 256, -42, -146, 94, 256, 482, 0, -241 };

const int16_t BT601LIMIT2BT709LIMIT[12] = { 256, -30, -53, 41, 0, 261, 29, -17, 0, 19, 262, -13 };
const int16_t BT709LIMIT2BT601LIMIT[12] = { 256, 25, 49, -37, 0, 253, -28, 15, 0, -19, 252, 11 };

const int16_t RGB2YUV_Abnormal1[12] = { 77, 150, 29, 500, -43, -85, 128, 500, 128, -107, -21, 500 };
const int16_t RGB2YUV_Abnormal2[12] = { 77, 150, 29, -500, -43, -85, 128, -500, 128, -107, -21, -500 };
const int16_t YUV2RGB_Abnormal1[12] = { 256, 0, 359, 500, 256, -88, -183, 500, 256, 453, 0, 500 };
const int16_t YUV2RGB_Abnormal2[12] = { 256, 0, 359, -500, 256, -88, -183, -500, 256, 453, 0, -500 };

const int16_t YUV2YUV_FULL[12] = { 256, 0, 0, 0, 0, 256, 0, 0, 0, 0, 256, 0 };


void dump_reg(void *stream_offset)
{
#if NONAI_2D_DEBUG
    int i;

    nonai_2d_dbg("%s>\n", __func__);

    //dump register
    uint32_t *pReg = (uint32_t*)stream_offset;
    for(i = 0; i<32; i++)
    {
        nonai_2d_dbg("%08x: %08x %08x %08x %08x %08x %08x %08x %08x\n", i*32,
            readl(pReg), readl(pReg+1), readl(pReg+2), readl(pReg+3),
            readl(pReg+4), readl(pReg+5), readl(pReg+6), readl(pReg+7));
        pReg += 8;
    }
    nonai_2d_dbg("\n");
#endif
}

void *nonai_2d_get_stream_base(uint8_t stream_index)
{
    return (NONAI_2D_REG + stream_index * 0x400);
}

uint32_t calc_video_size(enum k_pixel_format video_fmt, uint16_t width, uint16_t height)
{
    uint32_t size = 0;

    switch (video_fmt) {
    case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
    case PIXEL_FORMAT_YVU_SEMIPLANAR_420:
    case PIXEL_FORMAT_YVU_PLANAR_420:
        size = width * height * 3 / 2;
        break;

    case PIXEL_FORMAT_RGB_888:
    case PIXEL_FORMAT_BGR_888:
    case PIXEL_FORMAT_YUV_PACKAGE_444:
    case PIXEL_FORMAT_BGR_888_PLANAR:
    case PIXEL_FORMAT_RGB_888_PLANAR:
    case PIXEL_FORMAT_YVU_PLANAR_444:
        size = width * height * 3;
        break;

    default:
        pr_err("%s>unknown data type %d\n", __func__, video_fmt);
        return -1;
    }
    return size;
}

int32_t calc_video_line_stride(enum k_2d_video_format video_fmt, uint16_t width, uint16_t *stride)
{
    switch (video_fmt) {
    case K_2D_VIDEO_FMT_YUV420_NV12:
    case K_2D_VIDEO_FMT_YUV420_NV21:
        stride[0] = width;
        stride[1] = width;
        stride[2] = 0;
        break;
    case K_2D_VIDEO_FMT_YUV420_I420:
        stride[0] = width;
        stride[1] = width / 2;
        stride[2] = width / 2;
        break;
    case K_2D_VIDEO_FMT_ARGB8888:
    case K_2D_VIDEO_FMT_XRGB8888:
    case K_2D_VIDEO_FMT_BGRA8888:
    case K_2D_VIDEO_FMT_BGRX8888:
        stride[0] = width * 4;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_VIDEO_FMT_ARGB4444:
    case K_2D_VIDEO_FMT_ARGB1555:
    case K_2D_VIDEO_FMT_XRGB4444:
    case K_2D_VIDEO_FMT_XRGB1555:
    case K_2D_VIDEO_FMT_BGRA4444:
    case K_2D_VIDEO_FMT_BGRA5551:
    case K_2D_VIDEO_FMT_BGRX4444:
    case K_2D_VIDEO_FMT_BGRX5551:
    case K_2D_VIDEO_FMT_RGB565:
    case K_2D_VIDEO_FMT_BGR565:
        stride[0] = width * 2;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_VIDEO_FMT_RGB888:
    case K_2D_VIDEO_FMT_BGR888:
        stride[0] = width * 3;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_VIDEO_FMT_SEPERATE_RGB:
        stride[0] = width;
        stride[1] = width;
        stride[2] = width;
        break;
    default:
        pr_err("%s>unknown data type %d\n", __func__, video_fmt);
        return -1;
    }
    return 0;
}

int32_t calc_osd_line_stride(enum k_2d_osd_fmt osd_fmt, uint16_t width, uint16_t *stride)
{
    switch (osd_fmt) {
    case K_2D_OSD_FMT_ARGB8888:
    case K_2D_OSD_FMT_XRGB8888:
    case K_2D_OSD_FMT_BGRA8888:
    case K_2D_OSD_FMT_BGRX8888:
        stride[0] = width * 4;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_OSD_FMT_ARGB4444:
    case K_2D_OSD_FMT_ARGB1555:
    case K_2D_OSD_FMT_XRGB4444:
    case K_2D_OSD_FMT_XRGB1555:
    case K_2D_OSD_FMT_BGRA4444:
    case K_2D_OSD_FMT_BGRA5551:
    case K_2D_OSD_FMT_BGRX4444:
    case K_2D_OSD_FMT_BGRX5551:
    case K_2D_OSD_FMT_RGB565:
    case K_2D_OSD_FMT_BGR565:
        stride[0] = width * 2;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_OSD_FMT_RGB888:
    case K_2D_OSD_FMT_BGR888:
        stride[0] = width * 3;
        stride[1] = 0;
        stride[2] = 0;
        break;
    case K_2D_OSD_FMT_SEPERATE_RGB:
        stride[0] = width;
        stride[1] = width;
        stride[2] = width;
        break;
    default:
        pr_err("%s>unknown data type\n", __func__);
        return -1;
    }
    return 0;
}

enum k_2d_video_format get_video_fmt(enum k_pixel_format pixel_format)
{
    switch (pixel_format) {
    case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
    case PIXEL_FORMAT_YVU_SEMIPLANAR_420:
        return K_2D_VIDEO_FMT_YUV420_NV12;

    case PIXEL_FORMAT_YVU_PLANAR_420:
        return K_2D_VIDEO_FMT_YUV420_I420;

    case PIXEL_FORMAT_ARGB_8888:
        return K_2D_VIDEO_FMT_ARGB8888;

    case PIXEL_FORMAT_RGB_565:
        return K_2D_VIDEO_FMT_RGB565;

    case PIXEL_FORMAT_RGB_888:
    case PIXEL_FORMAT_YUV_PACKAGE_444:
    case PIXEL_FORMAT_YUV_SEMIPLANAR_444:
    case PIXEL_FORMAT_YVU_SEMIPLANAR_444:
        return K_2D_VIDEO_FMT_RGB888;

    case PIXEL_FORMAT_BGR_565:
        return K_2D_VIDEO_FMT_BGR565;

    case PIXEL_FORMAT_BGR_888:
        return K_2D_VIDEO_FMT_BGR888;

    case PIXEL_FORMAT_ARGB_1555:
        return K_2D_VIDEO_FMT_ARGB1555;

    case PIXEL_FORMAT_ARGB_4444:
        return K_2D_VIDEO_FMT_ARGB4444;

    case PIXEL_FORMAT_BGR_888_PLANAR:
    case PIXEL_FORMAT_RGB_888_PLANAR:
    case PIXEL_FORMAT_YVU_PLANAR_444:
        return K_2D_VIDEO_FMT_SEPERATE_RGB;
    default:
        pr_err("%s>unknown pixel_format %d\n", __func__, pixel_format);
        return K_2D_VIDEO_FMT_YUV420_NV12;
    }
}

enum k_nonai_2d_pixel_type get_video_type(enum k_pixel_format pixel_format)
{
    enum k_nonai_2d_pixel_type type;

    if (get_video_fmt(pixel_format) <= K_2D_VIDEO_FMT_YUV420_I420)
        type = PIXEL_TYPE_YUV;
    else
        type = PIXEL_TYPE_RGB;

    if (pixel_format == PIXEL_FORMAT_YVU_PLANAR_444 ||
        pixel_format == PIXEL_FORMAT_YUV_PACKAGE_444 ||
        pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_444 ||
        pixel_format == PIXEL_FORMAT_YVU_SEMIPLANAR_444) {
        type = PIXEL_TYPE_YUV;
    }
    return type;
}

int32_t nonai_2d_update_base_attr(uint32_t chn_num, struct k_video_frame_info *frame)
{
    struct nonai_2d_chn_t *chn;
    struct k_2d_osd_attr *osd_attr;
    uint16_t stride[3];
    enum k_2d_video_format video_fmt;

    chn = &g_nonai_2d_chn[chn_num];
    osd_attr = chn->osd_attr;
    video_fmt = get_video_fmt(frame->v_frame.pixel_format);

    nonai_2d_set_fmt((enum nonai_2d_src_dst_fmt)video_fmt, (enum nonai_2d_src_dst_fmt)chn->dst_fmt, chn->stream_offset);
    nonai_2d_set_src_size(frame->v_frame.width, frame->v_frame.height, chn->stream_offset);

    calc_video_line_stride(video_fmt, frame->v_frame.width, stride);
    nonai_2d_set_src_ch0_line_byte_stride(stride[0], chn->stream_offset);
    nonai_2d_set_src_ch1_line_byte_stride(stride[1], chn->stream_offset);
    nonai_2d_set_src_ch2_line_byte_stride(stride[2], chn->stream_offset);

    nonai_2d_set_dst_ch0_line_byte_stride(stride[0], chn->stream_offset);
    nonai_2d_set_dst_ch1_line_byte_stride(stride[1], chn->stream_offset);
    nonai_2d_set_dst_ch2_line_byte_stride(stride[2], chn->stream_offset);

    nonai_2d_set_src_ch0_addr(frame->v_frame.phys_addr[0], chn->stream_offset);
    nonai_2d_set_src_ch1_addr(frame->v_frame.phys_addr[1], chn->stream_offset);
    nonai_2d_set_src_ch2_addr(frame->v_frame.phys_addr[2], chn->stream_offset);

    nonai_2d_set_dst_ch0_addr(frame->v_frame.phys_addr[0], chn->stream_offset);
    nonai_2d_set_dst_ch1_addr(frame->v_frame.phys_addr[1], chn->stream_offset);
    nonai_2d_set_dst_ch2_addr(frame->v_frame.phys_addr[2], chn->stream_offset);
    if(chn->total_out_cnt == 0) {
    nonai_2d_dbg("nonai2d chn=%u calc_mode=%u\n", chn_num, chn->cfg.nonai_2d_calc_mode);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_fmt(src_fmt=%u, dst_fmt=%u)\n", chn_num, video_fmt, chn->dst_fmt);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_size(width=%u, height=%u)\n",chn_num, frame->v_frame.width, frame->v_frame.height);
    nonai_2d_dbg("nonai2d chn=%u calc_video_line_stride(video_fmt=%u, width=%u) => stride=[%u,%u,%u]\n",chn_num, video_fmt, frame->v_frame.width, stride[0], stride[1], stride[2]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch0_line_byte_stride(stride=%u)\n",chn_num, stride[0]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch1_line_byte_stride(stride=%u)\n",chn_num, stride[1]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch2_line_byte_stride(stride=%u)\n",chn_num, stride[2]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch0_line_byte_stride(stride=%u)\n",chn_num, stride[0]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch1_line_byte_stride(stride=%u)\n",chn_num, stride[1]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch2_line_byte_stride(stride=%u)\n",chn_num, stride[2]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch0_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[0]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch1_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[1]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_src_ch2_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[2]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch0_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[0]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch1_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[1]);
    nonai_2d_dbg("nonai2d chn=%u nonai_2d_set_dst_ch2_addr(addr=0x%llx)\n",chn_num, frame->v_frame.phys_addr[2]);
    }
    chn->width = frame->v_frame.width;
    chn->height = frame->v_frame.height;
    chn->src_fmt = video_fmt;

    return 0;
}

static void nonai_2d_cfg_coef(uint32_t chn_num)
{
    struct nonai_2d_chn_t *chn;
    int16_t yuv2rgb_coef[12];
    int16_t rgb2yuv_coef[12];

    chn = &g_nonai_2d_chn[chn_num];

    switch (chn->color_gamut) {
    case 0:
    default:
        memcpy(yuv2rgb_coef, YUV2RGB_BT601_FULL, sizeof(int16_t) * 12);
        memcpy(rgb2yuv_coef, RGB2YUV_BT601_FULL, sizeof(int16_t) * 12);
        break;
    case 1:
        memcpy(yuv2rgb_coef, YUV2RGB_BT709_FULL, sizeof(int16_t) * 12);
        memcpy(rgb2yuv_coef, RGB2YUV_BT709_FULL, sizeof(int16_t) * 12);
        break;
    case 2:
        memcpy(yuv2rgb_coef, YUV2RGB_BT2020, sizeof(int16_t) * 12);
        memcpy(rgb2yuv_coef, RGB2YUV_BT2020, sizeof(int16_t) * 12);
        break;
    }

    if (chn->cfg.nonai_2d_calc_mode == nonai_2d_calc_mode_csc) {
        if (chn->custom_csc_coef_en) {
            nonai_2d_set_csc_coef(chn->custom_csc_coef, chn->stream_offset);
        } else {
            if ((chn->src_type == PIXEL_TYPE_YUV) &&
                (chn->dst_type == PIXEL_TYPE_RGB)) {
                nonai_2d_set_csc_coef((uint16_t *)yuv2rgb_coef, chn->stream_offset);
            } else if ((chn->src_type == PIXEL_TYPE_RGB) &&
                   (chn->dst_type == PIXEL_TYPE_YUV)) {
                nonai_2d_set_csc_coef((uint16_t *)rgb2yuv_coef, chn->stream_offset);
            } else {
                nonai_2d_set_csc_coef((uint16_t *)YUV2YUV_FULL, chn->stream_offset);
            }
        }
    } else {
        if (chn->custom_osd_coef_en) {
            nonai_2d_set_osd_coef(chn->custom_osd_coef, chn->stream_offset);
        } else {
            if (chn->src_type == PIXEL_TYPE_YUV) {
                nonai_2d_set_osd_coef((uint16_t *)rgb2yuv_coef, chn->stream_offset);
            }
        }
    }
}

int32_t nonai_2d_cfg_csc_attr(uint32_t chn_num, struct nonai_2d_job *job)
{
    struct nonai_2d_chn_t *chn;
    uint16_t src_stride[3];
    uint16_t dst_stride[3];
    enum k_2d_video_format video_fmt;
    struct k_video_frame_info *frame;

    chn = &g_nonai_2d_chn[chn_num];
    frame = &job->in_frame;

    video_fmt = get_video_fmt(frame->v_frame.pixel_format);
    chn->src_type = get_video_type(frame->v_frame.pixel_format);
    chn->dst_type = get_video_type(job->out_frame.v_frame.pixel_format);

    nonai_2d_set_fmt((enum nonai_2d_src_dst_fmt)video_fmt, (enum nonai_2d_src_dst_fmt)chn->dst_fmt, chn->stream_offset);
    nonai_2d_set_src_size(frame->v_frame.width, frame->v_frame.height, chn->stream_offset);

    calc_video_line_stride(video_fmt, frame->v_frame.width, src_stride);
    nonai_2d_set_src_ch0_line_byte_stride(src_stride[0], chn->stream_offset);
    nonai_2d_set_src_ch1_line_byte_stride(src_stride[1], chn->stream_offset);
    nonai_2d_set_src_ch2_line_byte_stride(src_stride[2], chn->stream_offset);

    nonai_2d_set_src_ch0_addr(frame->v_frame.phys_addr[0], chn->stream_offset);
    nonai_2d_set_src_ch1_addr(frame->v_frame.phys_addr[1], chn->stream_offset);
    nonai_2d_set_src_ch2_addr(frame->v_frame.phys_addr[2], chn->stream_offset);

    calc_video_line_stride(chn->dst_fmt, frame->v_frame.width, dst_stride);
    nonai_2d_set_dst_ch0_line_byte_stride(dst_stride[0], chn->stream_offset);
    nonai_2d_set_dst_ch1_line_byte_stride(dst_stride[1], chn->stream_offset);
    nonai_2d_set_dst_ch2_line_byte_stride(dst_stride[2], chn->stream_offset);

    nonai_2d_set_dst_ch0_addr(job->out_frame.v_frame.phys_addr[0], chn->stream_offset);
    nonai_2d_set_dst_ch1_addr(job->out_frame.v_frame.phys_addr[1], chn->stream_offset);
    nonai_2d_set_dst_ch2_addr(job->out_frame.v_frame.phys_addr[2], chn->stream_offset);

    if(chn->total_out_cnt == 0) {
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_fmt(src_fmt=%u, dst_fmt=%u) src_type=%u dst_type=%u\n",
            chn_num, video_fmt, chn->dst_fmt, chn->src_type, chn->dst_type);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_size(width=%u, height=%u)\n",
            chn_num, frame->v_frame.width, frame->v_frame.height);
        nonai_2d_dbg("nonai2d csc chn=%u calc_video_line_stride(src_fmt=%u, width=%u) => stride=[%u,%u,%u]\n",
            chn_num, video_fmt, frame->v_frame.width, src_stride[0], src_stride[1], src_stride[2]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch0_line_byte_stride(stride=%u)\n", chn_num, src_stride[0]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch1_line_byte_stride(stride=%u)\n", chn_num, src_stride[1]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch2_line_byte_stride(stride=%u)\n", chn_num, src_stride[2]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch0_addr(addr=0x%llx)\n", chn_num, frame->v_frame.phys_addr[0]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch1_addr(addr=0x%llx)\n", chn_num, frame->v_frame.phys_addr[1]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_src_ch2_addr(addr=0x%llx)\n", chn_num, frame->v_frame.phys_addr[2]);
        nonai_2d_dbg("nonai2d csc chn=%u calc_video_line_stride(dst_fmt=%u, width=%u) => stride=[%u,%u,%u]\n",
            chn_num, chn->dst_fmt, frame->v_frame.width, dst_stride[0], dst_stride[1], dst_stride[2]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch0_line_byte_stride(stride=%u)\n", chn_num, dst_stride[0]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch1_line_byte_stride(stride=%u)\n", chn_num, dst_stride[1]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch2_line_byte_stride(stride=%u)\n", chn_num, dst_stride[2]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch0_addr(addr=0x%llx)\n", chn_num, job->out_frame.v_frame.phys_addr[0]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch1_addr(addr=0x%llx)\n", chn_num, job->out_frame.v_frame.phys_addr[1]);
        nonai_2d_dbg("nonai2d csc chn=%u nonai_2d_set_dst_ch2_addr(addr=0x%llx)\n", chn_num, job->out_frame.v_frame.phys_addr[2]);
    }

    nonai_2d_cfg_coef(chn_num);

    chn->src_fmt = video_fmt;
    chn->width = frame->v_frame.width;
    chn->height = frame->v_frame.height;
    return 0;
}

int32_t nonai_2d_cfg_osd_attr(uint32_t chn_num, struct k_video_frame_info *frame)
{
    struct nonai_2d_chn_t *chn;
    struct k_2d_osd_attr *osd_attr;
    uint16_t stride[3];
    int i;
    int cnt = 0;

    chn = &g_nonai_2d_chn[chn_num];

    for (i = 0; i < K_MAX_2D_OSD_REGION_NUM; i++) {
        if (chn->osd_valid >> i) {
            osd_attr = &chn->osd_attr[i];
            nonai_2d_set_osd_size(osd_attr->width, osd_attr->height, i, chn->stream_offset);
            nonai_2d_set_osd_start_position(osd_attr->startx, osd_attr->starty, i, chn->stream_offset);
            nonai_2d_set_osd_ch0_addr(osd_attr->phys_addr[0], i, chn->stream_offset);
            nonai_2d_set_osd_ch1_addr(osd_attr->phys_addr[1], i, chn->stream_offset);
            nonai_2d_set_osd_ch2_addr(osd_attr->phys_addr[2], i, chn->stream_offset);
            calc_osd_line_stride(osd_attr->fmt, osd_attr->width, stride);
            nonai_2d_set_osd_ch0_line_byte_stride(stride[0], i, chn->stream_offset);
            nonai_2d_set_osd_ch1_line_byte_stride(stride[1], i, chn->stream_offset);
            nonai_2d_set_osd_ch2_line_byte_stride(stride[2], i, chn->stream_offset);
            nonai_2d_set_bg_global_alpha(osd_attr->bg_alpha, i, chn->stream_offset);
            nonai_2d_set_osd_global_alpha(osd_attr->osd_alpha, i, chn->stream_offset);
            nonai_2d_set_video_global_alpha(osd_attr->video_alpha, i, chn->stream_offset);
            nonai_2d_set_add_order((enum nonai_2d_add_order)osd_attr->add_order, i, chn->stream_offset);
            nonai_2d_set_bg_color(osd_attr->bg_color, i, chn->stream_offset);
            nonai_2d_set_osd_fmt((enum nonai_2d_osd_fmt)osd_attr->fmt, i, chn->stream_offset);
            cnt++;

            if(chn->total_out_cnt == 0) {
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_size(width=%u, height=%u, region=%d)\n",i, osd_attr->width, osd_attr->height, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_start_position(startx=%u, starty=%u, region=%d)\n",i, osd_attr->startx, osd_attr->starty, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch0_addr(addr=0x%llx, region=%d)\n",
                    i, (unsigned long long)osd_attr->phys_addr[0], i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch1_addr(addr=0x%llx, region=%d)\n",
                    i, (unsigned long long)osd_attr->phys_addr[1], i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch2_addr(addr=0x%llx, region=%d)\n",
                    i, (unsigned long long)osd_attr->phys_addr[2], i);
                nonai_2d_dbg("nonai2d osd[%d] calc_osd_line_stride(fmt=%u, width=%u) => stride=[%u,%u,%u]\n",
                    i, osd_attr->fmt, osd_attr->width, stride[0], stride[1], stride[2]);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch0_line_byte_stride(stride=%u, region=%d)\n",
                    i, stride[0], i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch1_line_byte_stride(stride=%u, region=%d)\n",
                    i, stride[1], i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_ch2_line_byte_stride(stride=%u, region=%d)\n",
                    i, stride[2], i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_bg_global_alpha(bg_alpha=%u, region=%d)\n",
                    i, osd_attr->bg_alpha, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_global_alpha(osd_alpha=%u, region=%d)\n",
                    i, osd_attr->osd_alpha, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_video_global_alpha(video_alpha=%u, region=%d)\n",
                    i, osd_attr->video_alpha, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_add_order(add_order=%u, region=%d)\n",
                    i, osd_attr->add_order, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_bg_color(bg_color=0x%x, region=%d)\n",
                    i, osd_attr->bg_color, i);
                nonai_2d_dbg("nonai2d osd[%d] nonai_2d_set_osd_fmt(fmt=%u, region=%d)\n",
                    i, osd_attr->fmt, i);
            }
        }
    }

    chn->cfg.osd_region_num = cnt;

    nonai_2d_cfg_coef(chn_num);

    return 0;
}

int32_t nonai_2d_cfg_border_attr(uint32_t chn_num, struct k_video_frame_info *frame)
{
    struct nonai_2d_chn_t *chn;
    struct k_2d_border_attr *border_attr;
    int i;
    int cnt = 0;

    chn = &g_nonai_2d_chn[chn_num];

    for (i = 0; i < K_MAX_2D_BORDER_NUM; i++) {
        if (chn->border_valid >> i) {
            border_attr = &chn->border_attr[i];
            nonai_2d_set_border_size(border_attr->width, border_attr->height, i, chn->stream_offset);
            nonai_2d_set_border_wide(border_attr->line_width, i, chn->stream_offset);
            nonai_2d_set_border_color(border_attr->color, i, chn->stream_offset);
            nonai_2d_set_border_start_position(border_attr->startx, border_attr->starty, i, chn->stream_offset);
            cnt++;
            if(chn->total_out_cnt == 0) {
                nonai_2d_dbg("nonai2d border[%d] nonai_2d_set_border_size(width=%u, height=%u)\n",i, border_attr->width, border_attr->height);
                nonai_2d_dbg("nonai2d border[%d] nonai_2d_set_border_wide(line_width=%u)\n",i, border_attr->line_width);
                nonai_2d_dbg("nonai2d border[%d] nonai_2d_set_border_color(color=0x%x)\n",i, border_attr->color);
                nonai_2d_dbg("nonai2d border[%d] nonai_2d_set_border_start_position(startx=%u, starty=%u)\n",i, border_attr->startx, border_attr->starty);
            }
        }
    }

    chn->cfg.draw_border_num = cnt;

    return 0;
}

int32_t nonai_2d_cfg_hw(struct nonai_2d_job *job)
{
    struct nonai_2d_chn_t *chn;
    struct k_nonai_2d_main_cfg *cfg;
    struct k_video_frame_info *frame;

    chn = &g_nonai_2d_chn[job->chn_id];
    cfg = &chn->cfg;

    frame = &job->in_frame;

    switch (chn->cfg.nonai_2d_calc_mode) {
    case nonai_2d_calc_mode_csc:
        nonai_2d_cfg_csc_attr(job->chn_id, job);

        nonai_2d_main_cfg(chn->stream_offset, 1, 1, 0, nonai_2d_calc_mode_csc, 0, 0, 0);
        break;
    case nonai_2d_calc_mode_osd:
        nonai_2d_cfg_osd_attr(job->chn_id, frame);
        nonai_2d_update_base_attr(job->chn_id, frame);
        nonai_2d_main_cfg(chn->stream_offset, 1, 0, 0, nonai_2d_calc_mode_osd, cfg->osd_region_num, 0, 0);
        break;
    case nonai_2d_calc_mode_border:
        nonai_2d_cfg_border_attr(job->chn_id, frame);
        nonai_2d_update_base_attr(job->chn_id, frame);
        nonai_2d_main_cfg(chn->stream_offset, 1, 0, 0, nonai_2d_calc_mode_border, 0, cfg->draw_border_num, 0);
        break;
    case nonai_2d_calc_mode_osd_border:
        nonai_2d_cfg_osd_attr(job->chn_id, frame);
        nonai_2d_cfg_border_attr(job->chn_id, frame);
        nonai_2d_update_base_attr(job->chn_id, frame);
        nonai_2d_main_cfg(chn->stream_offset, 1, 0, 0, nonai_2d_calc_mode_osd_border, cfg->osd_region_num, cfg->draw_border_num, 0);
        break;
    default:
        return -1;
    }

    if(chn->total_out_cnt == 0) {
        dump_reg(chn->stream_offset);
    }
    chn->total_out_cnt++;

    return 0;
}

void nonai_2d_irq_callback(int irq, void *param)
{
    // nonai_2d_main_clear_intr();
}

int32_t nonai_2d_hw_init(void)
{
    memset(&g_nonai_2d, 0, sizeof(struct nonai_2d_dev_t));
    memset(&g_nonai_2d_chn, 0, sizeof(struct nonai_2d_chn_t) * NONAI_2D_MAX_CHN_NUMS);

    return 0;
}

int32_t nonai_2d_hw_exit(void)
{
    g_nonai_2d.init_cnt--;
    if (NONAI_2D_REG && g_nonai_2d.init_cnt <= 0)
        NONAI_2D_REG = NULL;

    return 0;
}
