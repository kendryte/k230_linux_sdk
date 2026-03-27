/* SPDX-License-Identifier: GPL-2.0-only
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
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
 */

#ifndef _NONAI_2D_H_
#define _NONAI_2D_H_

#include <linux/types.h>
#include <linux/dma-mapping.h>

#include "nonai_2d_reg.h"
#include "k_nonai_2d_comm.h"
#include "nonai_2d.h"

#define NONAI_2D_DEBUG    0
#define NONAI_2D_INFO     0
#define NONAI_2D_DUMP     0

#if NONAI_2D_DEBUG
#define nonai_2d_dbg(fmt, ...) \
    do { \
        printk(KERN_DEBUG "nonai2d: " fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define nonai_2d_dbg(fmt, ...) do { } while (0)
#endif

#if NONAI_2D_INFO
#define nonai_2d_info(fmt, ...) \
    do { \
        printk(KERN_INFO "nonai2d: " fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define nonai_2d_info(fmt, ...) do { } while (0)
#endif

#define NONAI_2D_MAX_DEV_NUMS 1
#define NONAI_2D_MAX_CHN_NUMS 32

#define OUT_QUEUE_NUMS 30
#define NONAI_2D_QUEUE_DEPTH (100)
#define NONAI_2D_TASK_STACK_SIZE (8 * 1024)
#define NONAI_2D_SCHE_STACK_SIZE (8 * 1024)

enum NONAI_2D_CH_STATE {
    NONAI_2D_CH_STATE_IDLE = 0,
    NONAI_2D_CH_STATE_INIT,
    NONAI_2D_CH_STATE_START,
    NONAI_2D_CH_STATE_STOPPING,
    NONAI_2D_CH_STATE_STOPPED,
    NONAI_2D_CH_STATE_BUTT
};

enum k_nonai_2d_pixel_type {
    PIXEL_TYPE_YUV = 0,
    PIXEL_TYPE_RGB,
    PIXEL_TYPE_BUTT
};

struct nonai_2d_dev_t {
    int32_t init_cnt;
    uint32_t inq_wm;
    uint32_t outq_wm;
    bool running;
    bool exited;
    uint32_t total_in_cnt;
    uint32_t total_out_cnt;
    bool need_input;
};

struct nonai_2d_chn_t {
    uint32_t chn_id;
    uint16_t width;
    uint16_t height;
    uint8_t osd_valid;
    uint32_t border_valid;
    struct k_2d_osd_attr osd_attr[K_MAX_2D_OSD_REGION_NUM];
    struct k_2d_border_attr border_attr[K_MAX_2D_BORDER_NUM];
    struct k_nonai_2d_main_cfg cfg;
    bool attached;
    enum k_nonai_2d_pixel_type src_type;
    enum k_nonai_2d_pixel_type dst_type;
    bool custom_csc_coef_en;
    uint16_t custom_csc_coef[12];
    bool custom_osd_coef_en;
    uint16_t custom_osd_coef[12];
    enum k_nonai_2d_color_gamut color_gamut;
    int32_t osd_pool_id[K_MAX_2D_OSD_REGION_NUM];
    void *osd_dmabuf[K_MAX_2D_OSD_REGION_NUM];
    dma_addr_t osd_dma_handle[K_MAX_2D_OSD_REGION_NUM];
    size_t osd_dmabuf_size[K_MAX_2D_OSD_REGION_NUM];
    void __iomem *stream_offset;
    bool input_waiting;
    bool output_waiting;
    int output_wp;
    int output_alloc;
    int output_rp;
    int output_release;
    enum NONAI_2D_CH_STATE state;
    bool bind_sender;
    enum k_2d_video_format src_fmt;
    enum k_2d_video_format dst_fmt;
    uint32_t total_in_cnt;
    uint32_t total_out_cnt;
    enum k_pixel_format in_pic_format;
    enum k_pixel_format out_pic_format;
};

extern struct nonai_2d_chn_t g_nonai_2d_chn[NONAI_2D_MAX_CHN_NUMS];

struct nonai_2d_job {
    uint32_t chn_id;
    struct k_video_frame_info in_frame;
    struct k_video_frame_info out_frame;
};

int32_t nonai_2d_hw_init(void);
int32_t nonai_2d_hw_exit(void);

int32_t nonai_2d_cfg_hw(struct nonai_2d_job *job);
uint32_t calc_video_size(enum k_pixel_format video_fmt, uint16_t width,
             uint16_t height);
enum k_nonai_2d_pixel_type get_video_type(enum k_pixel_format pixel_format);
void dump_reg(void *stream_offset);

#endif /* _NONAI_2D_H_ */
