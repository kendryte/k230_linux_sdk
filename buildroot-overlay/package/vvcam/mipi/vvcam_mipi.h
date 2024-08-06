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

#ifndef __VVCAM_MIPI_H__
#define __VVCAM_MIPI_H__

#define MIPI_LANE_NUM_MAX 8
#define MIPI_OUTPUT_INTERFACES_MAX 8

typedef enum {
    VVCAM_INPUT_MODE_MIPI    = 0x00,
    VVCAM_INPUT_MODE_LVDS    = 0x01,
    VVCAM_INPUT_MODE_BT1120  = 0x02,
    VVCAM_INPUT_MODE_DVP     = 0x03
} vvcam_input_mode_t;

typedef struct {
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
} vvcam_image_rect_t;

typedef enum {
    VVCAM_RAW_8BIT = 0,
    VVCAM_RAW_10BIT,
    VVCAM_RAW_12BIT,
    VVCAM_RAW_14BIT,
    VVCAM_RAW_16BIT,
    VVCAM_RAW_20BIT,
    VVCAM_RAW_24BIT,
    VVCAM_RGB_444,
    VVCAM_RGB_555,
    VVCAM_RGB_565,
    VVCAM_RGB_666,
    VVCAM_RGB_888,
    VVCAM_YUV420_8BIT,
    VVCAM_YUV420_8BIT_LEG,
    VVCAM_YUV420_8BIT_CSPS,
    VVCAM_YUV420_10BIT,
    VVCAM_YUV420_10BIT_CSPS,
    VVCAM_YUV422_8BIT,
    VVCAM_YUV422_10BIT,
} vvcam_data_type_t;

typedef enum {
    VVCAM_ITF_VCID_0 = 0,
    VVCAM_ITF_VCID_1,
    VVCAM_ITF_VCID_2,
    VVCAM_ITF_VCID_3,
    VVCAM_ITF_VCID_4,
    VVCAM_ITF_VCID_5,
    VVCAM_ITF_VCID_6,
    VVCAM_ITF_VCID_7,
    VVCAM_ITF_VCID_MAX,
} vvcam_itf_vcid_t;

typedef enum {
    VVCAM_ITF_BIT_MODE_48BIT = 0,
    VVCAM_ITF_BIT_MODE_16BIT,
} vvcam_itf_bit_mode_t;

typedef enum {
    VVCAM_SYNC_POL_HIGH_ACIVE = 0,
    VVCAM_SYNC_POL_LOW_ACIVE
}vvcam_sync_pol_t;

typedef struct {
    uint32_t hsa;   /* horizontal synchronism activate (pixel cycle) */
    uint32_t hbp;   /* horizontal back porch (pixel cycle) */
    uint32_t hfp;   /* horizontal front porch (pixel cycle) */
    uint32_t hline;   /* overall time for each line (pixel cycle) */
} vvcam_itf_horizontal_time_t;

typedef struct {
    uint32_t vsa;   /* vertical synchronism activate (hline) */
    uint32_t vbp;   /* vertical back porch (hline) */
    uint32_t vfp;   /* vertical front porch (hline) */
    uint32_t vactivate_lines;   /* vertical resolution of video */
} vvcam_itf_vertical_time_t;

typedef struct {

	vvcam_sync_pol_t pwdn;
	vvcam_sync_pol_t rst;
	vvcam_sync_pol_t fsin;
}vvcam_power_attr_t;

typedef struct {
	vvcam_power_attr_t powerPin;
	bool isPower;
}vvcam_dvp_dev_attr_t;

typedef struct {
    bool enable;
    bool embedded_data_en;
    vvcam_itf_vcid_t vcid;
    vvcam_itf_bit_mode_t bit_mode;
    vvcam_data_type_t data_type;
    vvcam_itf_horizontal_time_t h_time;
    vvcam_itf_vertical_time_t v_time;
} vvcam_mipi_output_itf_t;

typedef struct {
    vvcam_mipi_output_itf_t itf_cfg[MIPI_OUTPUT_INTERFACES_MAX];
    short lane_id[MIPI_LANE_NUM_MAX];  /* lane id: -1 - disable */
    uint32_t phy_freq;
} vvcam_mipi_dev_attr_t;

typedef struct {
    vvcam_input_mode_t mode;       /* input mode: MIPI/LVDS/BT1120 */
    vvcam_image_rect_t image_rect; /* Rx device area */

    union {
        vvcam_mipi_dev_attr_t mipi_attr;
		vvcam_dvp_dev_attr_t  dvp_attr;
        //vvcam_lvds_dev_attr_t lvds_attr;
    };
} vvcam_input_dev_attr_t;

#define VVCAM_MIPI_IOC_MAGIC 'v'
#define VVCAM_MIPI_RESET        _IOW(VVCAM_MIPI_IOC_MAGIC, 0x01, unsigned int) /* 1:reset 0:unreset */
#define VVCAM_MIPI_SET_DEV_ATTR _IOW(VVCAM_MIPI_IOC_MAGIC, 0x02, vvcam_input_dev_attr_t)
#define VVCAM_MIPI_SET_STREAM   _IOW(VVCAM_MIPI_IOC_MAGIC, 0x03, unsigned int)

#endif
