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
#ifndef __K_2D_COMM_H__
#define __K_2D_COMM_H__

#include "k_video_comm.h"


#define K_2D_MAX_DEV_NUMS (1)
#define K_2D_MAX_CHN_NUMS (24)
#define K_MAX_2D_OSD_REGION_NUM (8) /* Max region number of osd */
#define K_MAX_2D_BORDER_NUM (32) /* Max number of border */
#define K_2D_MAX_CHN_NUM (3) /* Max number of 2d channel */
#define K_2D_COEF_NUM (12) /* 2d coefficient */

/**
 * @brief Defines the attributes of a NONAI_2D channel
 *
 */

enum k_nonai_2d_calc_mode {
	K_2D_CALC_MODE_CSC = 0, /* Color space conversion */
	K_2D_CALC_MODE_OSD, /* On Screen Display */
	K_2D_CALC_MODE_BORDER, /* Draw border */
	K_2D_CALC_MODE_OSD_BORDER, /* OSD first, then draw border */
	K_2D_CALC_MODE_BUTT
};

enum k_nonai_2d_color_gamut {
	K_2D_COLOR_GAMUT_BT601 = 0,
	K_2D_COLOR_GAMUT_BT709,
	K_2D_COLOR_GAMUT_BT2020,
	K_2D_COLOR_GAMUT_BUTT
};

struct k_nonai_2d_chn_attr {
	enum k_pixel_format dst_fmt; /* Format of output image */
	enum k_nonai_2d_calc_mode mode;
};

struct k_nonai_2d_coef_attr {
	int16_t coef[12]; /* Pointer of coefficent */
};

enum k_2d_osd_fmt {
	K_2D_OSD_FMT_ARGB8888 = 0, /* OSD format: ARGB8888 */
	K_2D_OSD_FMT_ARGB4444, /* OSD format: ARGB4444 */
	K_2D_OSD_FMT_ARGB1555, /* OSD format: ARGB1555 */
	K_2D_OSD_FMT_XRGB8888, /* OSD format: XRGB8888 */
	K_2D_OSD_FMT_XRGB4444, /* OSD format: XRGB4444 */
	K_2D_OSD_FMT_XRGB1555, /* OSD format: XRGB1555 */
	K_2D_OSD_FMT_BGRA8888, /* OSD format: BGRA8888 */
	K_2D_OSD_FMT_BGRA4444, /* OSD format: BGRA4444 */
	K_2D_OSD_FMT_BGRA5551, /* OSD format: BGRA5551 */
	K_2D_OSD_FMT_BGRX8888, /* OSD format: BGRX8888 */
	K_2D_OSD_FMT_BGRX4444, /* OSD format: BGRX4444 */
	K_2D_OSD_FMT_BGRX5551, /* OSD format: BGRX5551 */
	K_2D_OSD_FMT_RGB888, /* OSD format: RGB888 */
	K_2D_OSD_FMT_BGR888, /* OSD format: BGR888 */
	K_2D_OSD_FMT_RGB565, /* OSD format: RGB565 */
	K_2D_OSD_FMT_BGR565, /* OSD format: BGR565 */
	K_2D_OSD_FMT_SEPERATE_RGB, /* OSD format: SEPERATE_RGB */
	K_2D_OSD_FMT_BUTT
};

enum k_2d_video_format {
	K_2D_VIDEO_FMT_YUV420_NV12 =
		0, /* Source/destination format of 2d: NV12 */
	K_2D_VIDEO_FMT_YUV420_NV21, /* Source/destination format of 2d: NV21 */
	K_2D_VIDEO_FMT_YUV420_I420, /* Source/destination format of 2d: I420/420p */
	K_2D_VIDEO_FMT_ARGB8888 =
		4, /* Source/destination format of 2d: ARGB8888 */
	K_2D_VIDEO_FMT_ARGB4444, /* Source/destination format of 2d: ARGB4444 */
	K_2D_VIDEO_FMT_ARGB1555, /* Source/destination format of 2d: ARGB1555 */
	K_2D_VIDEO_FMT_XRGB8888, /* Source/destination format of 2d: XRGB8888 */
	K_2D_VIDEO_FMT_XRGB4444, /* Source/destination format of 2d: XRGB4444 */
	K_2D_VIDEO_FMT_XRGB1555, /* Source/destination format of 2d: XRGB1555 */
	K_2D_VIDEO_FMT_BGRA8888, /* Source/destination format of 2d: BGRA8888 */
	K_2D_VIDEO_FMT_BGRA4444, /* Source/destination format of 2d: BGRA4444 */
	K_2D_VIDEO_FMT_BGRA5551, /* Source/destination format of 2d: BGRA5551 */
	K_2D_VIDEO_FMT_BGRX8888, /* Source/destination format of 2d: BGRX8888 */
	K_2D_VIDEO_FMT_BGRX4444, /* Source/destination format of 2d: BGRX4444 */
	K_2D_VIDEO_FMT_BGRX5551, /* Source/destination format of 2d: BGRX5551 */
	K_2D_VIDEO_FMT_RGB888, /* Source/destination format of 2d: RGB888 */
	K_2D_VIDEO_FMT_BGR888, /* Source/destination format of 2d: BGR888 */
	K_2D_VIDEO_FMT_RGB565, /* Source/destination format of 2d: RGB565 */
	K_2D_VIDEO_FMT_BGR565, /* Source/destination format of 2d: BGR565 */
	K_2D_VIDEO_FMT_SEPERATE_RGB, /* Source/destination format of 2d: SEPERATE_RGB */
	K_2D_VIDEO_FMT_BUTT
};

enum k_2d_add_order {
	/* bottom ------> top */
	K_2D_ADD_ORDER_VIDEO_OSD =
		0, /* Add order of OSD, from bottom to top: VIDEO_OSD */
	K_2D_ADD_ORDER_OSD_VIDEO, /* Add order of OSD, from bottom to top: OSD_VIDEO */
	K_2D_ADD_ORDER_VIDEO_BG, /* Add order of OSD, from bottom to top: VIDEO_BG */
	K_2D_ADD_ORDER_BG_VIDEO, /* Add order of OSD, from bottom to top: BG_VIDEO */
	K_2D_ADD_ORDER_VIDEO_BG_OSD, /* Add order of OSD, from bottom to top: VIDEO_BG_OSD */
	K_2D_ADD_ORDER_VIDEO_OSD_BG, /* Add order of OSD, from bottom to top: VIDEO_OSD_BG */
	K_2D_ADD_ORDER_BG_VIDEO_OSD, /* Add order of OSD, from bottom to top: BG_VIDEO_OSD */
	K_2D_ADD_ORDER_BG_OSD_VIDEO, /* Add order of OSD, from bottom to top: BG_OSD_VIDEO */
	K_2D_ADD_ORDER_OSD_VIDEO_BG, /* Add order of OSD, from bottom to top: OSD_VIDEO_BG */
	K_2D_ADD_ORDER_OSD_BG_VIDEO, /* Add order of OSD, from bottom to top: OSD_BG_VIDEO */
	K_2D_ADD_ORDER_BUTT
};

struct k_2d_osd_attr {
	uint16_t width; /* Width of OSD image */
	uint16_t height; /* Height of OSD image */
	uint16_t startx; /* Start coordinate in horizontal of OSD image */
	uint16_t starty; /* Start coordinate in vertical of OSD image */
	uint32_t phys_addr[3]; /* Physical address of OSD image */
	uint8_t bg_alpha; /* Alpha of background in OSD region */
	uint8_t osd_alpha; /* Alpha of OSD in OSD region */
	uint8_t video_alpha; /* Alpha of input image in OSD region */
	enum k_2d_add_order add_order; /* Add order of OSD region */
	uint32_t bg_color; /* Background color in OSD region */
	enum k_2d_osd_fmt fmt; /* Format of OSD image */
};

struct k_2d_border_attr {
	uint16_t width; /* Width of border */
	uint16_t height; /* Height of border */
	uint16_t line_width; /* Wide of border line */
	uint32_t color; /* Color of border */
	uint16_t startx; /* Start coordinate in horizontal of border */
	uint16_t starty; /* Start coordinate in vertical of border */
};

#endif /* __K_2D_COMM_H__ */
