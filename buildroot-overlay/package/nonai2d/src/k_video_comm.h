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
#ifndef __K_VIDEO_COMM_H__
#define __K_VIDEO_COMM_H__

enum k_mod_id {
	K_ID_CMPI = 0, /**< common module platform interface           */
	K_ID_LOG = 1, /**< mpi device log                             */
	K_ID_MMZ = 2, /**< media memory zone                          */
	K_ID_BUTT, /**< Invalid                                    */
};

enum k_pixel_format {
	PIXEL_FORMAT_RGB_444 = 0,
	PIXEL_FORMAT_RGB_555,
	PIXEL_FORMAT_RGB_565,
	PIXEL_FORMAT_RGB_888,

	PIXEL_FORMAT_BGR_444,
	PIXEL_FORMAT_BGR_555,
	PIXEL_FORMAT_BGR_565,
	PIXEL_FORMAT_BGR_888,

	PIXEL_FORMAT_ARGB_1555,
	PIXEL_FORMAT_ARGB_4444,
	PIXEL_FORMAT_ARGB_8565,
	PIXEL_FORMAT_ARGB_8888,
	PIXEL_FORMAT_ARGB_2BPP,

	PIXEL_FORMAT_ABGR_1555,
	PIXEL_FORMAT_ABGR_4444,
	PIXEL_FORMAT_ABGR_8565,
	PIXEL_FORMAT_ABGR_8888,

	PIXEL_FORMAT_BGRA_8888,

	PIXEL_FORMAT_RGB_MONOCHROME_8BPP,

	PIXEL_FORMAT_RGB_BAYER_8BPP,
	PIXEL_FORMAT_RGB_BAYER_10BPP,
	PIXEL_FORMAT_RGB_BAYER_12BPP,
	PIXEL_FORMAT_RGB_BAYER_14BPP,
	PIXEL_FORMAT_RGB_BAYER_16BPP,

	PIXEL_FORMAT_YVU_PLANAR_422,
	PIXEL_FORMAT_YVU_PLANAR_420,
	PIXEL_FORMAT_YVU_PLANAR_444,

	PIXEL_FORMAT_YVU_SEMIPLANAR_422,
	PIXEL_FORMAT_YVU_SEMIPLANAR_420,
	PIXEL_FORMAT_YVU_SEMIPLANAR_444,

	PIXEL_FORMAT_YUV_SEMIPLANAR_422,
	PIXEL_FORMAT_YUV_SEMIPLANAR_420,
	PIXEL_FORMAT_YUV_SEMIPLANAR_444,

	PIXEL_FORMAT_YUYV_PACKAGE_422,
	PIXEL_FORMAT_YVYU_PACKAGE_422,
	PIXEL_FORMAT_UYVY_PACKAGE_422,
	PIXEL_FORMAT_VYUY_PACKAGE_422,
	PIXEL_FORMAT_YYUV_PACKAGE_422,
	PIXEL_FORMAT_YYVU_PACKAGE_422,
	PIXEL_FORMAT_UVYY_PACKAGE_422,
	PIXEL_FORMAT_VUYY_PACKAGE_422,
	PIXEL_FORMAT_VY1UY0_PACKAGE_422,
	PIXEL_FORMAT_YUV_PACKAGE_444,

	PIXEL_FORMAT_YUV_400,
	PIXEL_FORMAT_UV_420,

	/* SVP data format */
	PIXEL_FORMAT_BGR_888_PLANAR,
	PIXEL_FORMAT_RGB_888_PLANAR,
	PIXEL_FORMAT_HSV_888_PACKAGE,
	PIXEL_FORMAT_HSV_888_PLANAR,
	PIXEL_FORMAT_LAB_888_PACKAGE,
	PIXEL_FORMAT_LAB_888_PLANAR,
	PIXEL_FORMAT_S8C1,
	PIXEL_FORMAT_S8C2_PACKAGE,
	PIXEL_FORMAT_S8C2_PLANAR,
	PIXEL_FORMAT_S8C3_PLANAR,
	PIXEL_FORMAT_S16C1,
	PIXEL_FORMAT_U8C1,
	PIXEL_FORMAT_U16C1,
	PIXEL_FORMAT_S32C1,
	PIXEL_FORMAT_U32C1,
	PIXEL_FORMAT_U64C1,
	PIXEL_FORMAT_S64C1,

	PIXEL_FORMAT_BUTT
};

enum k_video_field {
	VIDEO_FIELD_TOP = 0x1, /* even field */
	VIDEO_FIELD_BOTTOM = 0x2, /* odd field */
	VIDEO_FIELD_INTERLACED = 0x3, /* two interlaced fields */
	VIDEO_FIELD_FRAME = 0x4, /* frame */

	VIDEO_FIELD_BUTT
};

enum k_video_format {
	VIDEO_FORMAT_LINEAR = 0, /* nature video line */
	VIDEO_FORMAT_TILE_64x16, /* tile cell: 64pixel x 16line */
	VIDEO_FORMAT_TILE_16x8, /* tile cell: 16pixel x 8line */
	VIDEO_FORMAT_LINEAR_DISCRETE, /* The data bits are aligned in bytes */
	VIDEO_FORMAT_BUTT
};

/**
 * @brief Describe the data structure of video
 *
 */
struct k_video_frame {
	uint32_t width; /**< Picture width  */
	uint32_t height; /**< Picture height */
	enum k_video_field field; /**< video frame filed*/
	enum k_pixel_format pixel_format; /**< Pixel format of a  picture */
	enum k_video_format video_format;

	uint32_t header_stride[3];
	uint32_t stride[3];

	uint64_t header_phys_addr[3];
	uint64_t header_virt_addr[3];

	uint64_t phys_addr[3];
	uint64_t virt_addr[3];

	int16_t offset_top; /* top offset of show area */
	int16_t offset_bottom; /* bottom offset of show area */
	int16_t offset_left; /* left offset of show area */
	int16_t offset_right; /* right offset of show area */

	uint32_t time_ref;
	uint64_t pts;

	uint64_t priv_data; /* bit 0:7 to fill light ctrl state */
};

/**
 * @brief Defines the video picture information
 *
 */
struct k_video_frame_info {
	struct k_video_frame v_frame; /**< Video picture frame */
	uint32_t pool_id; /**< VB pool ID */
	enum k_mod_id mod_id; /**< Logical unit for generating video frames */
};

#endif /* __K_VIDEO_COMM_H__ */
