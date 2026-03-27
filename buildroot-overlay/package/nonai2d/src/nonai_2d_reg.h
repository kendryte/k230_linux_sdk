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

#ifndef _NONAI_2D_REG_H_
#define _NONAI_2D_REG_H_

#define NONAI_2D_INT_NO (141)
#define NONAI_2D_BASE (0x8080C000)
#define NONAI_2D_REG_SIZE (0x1000)

extern void *(NONAI_2D_REG);
// #define NONAI_2D_STREAM0_REG_BASE     (NONAI_2D_REG + 0x00000000)
// #define NONAI_2D_MAIN_CTRL_REG_BASE   (NONAI_2D_REG + 0x000003a4)
// #define NONAI_2D_STREAM1_REG_BASE     (NONAI_2D_REG + 0x00000400)
// #define NONAI_2D_STREAM2_REG_BASE     (NONAI_2D_REG + 0x00000800)

#define NONAI_2D_STREAM0_REG_BASE (0x00000000)
#define NONAI_2D_MAIN_CTRL_REG_BASE (0x000003a4)
#define NONAI_2D_STREAM1_REG_BASE (0x00000400)
#define NONAI_2D_STREAM2_REG_BASE (0x00000800)

#define NONAI_2D_STREAM_REG_BASE(x) (NONAI_2D_STREAM##x##_REG_BASE)

#define NONAI_2D_CONNECT(x, y) (x##y)
#define STRING(x) #x
// #define CHAR(x)                             #@x

#define NONAI_2D_STREAM_MAIN_CTRL_CFG0_OFFSET \
    (0x000) /* stream src width & height */
#define NONAI_2D_SRC_WIDTH_OFFSET (0)
#define NONAI_2D_SRC_HEIGHT_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG1_OFFSET (0x004) /* stream src ch0 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG2_OFFSET (0x008) /* stream src ch1 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG3_OFFSET (0x00c) /* stream src ch2 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET \
    (0x010) /* stream ch0 & ch1 line stride */
#define NONAI_2D_SRC_CH0_LINE_BYTE_STRIDE_OFFSET (0)
#define NONAI_2D_SRC_CH1_LINE_BYTE_STRIDE_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG5_OFFSET \
    (0x014) /* stream ch2 line stride*/
#define NONAI_2D_STREAM_MAIN_CTRL_CFG6_OFFSET \
    (0x018) /* stream src & dst format */
#define NONAI_2D_SRC_FORMAT_OFFSET (0)
#define NONAI_2D_DST_FORMAT_OFFSET (16)
enum nonai_2d_src_dst_fmt {
    nonai_2d_src_dst_fmt_yuv420_nv12 = 0,
    nonai_2d_src_dst_fmt_yuv420_nv21 = 1,
    nonai_2d_src_dst_fmt_yuv420_i420 = 2,
    nonai_2d_src_dst_fmt_yuv422p = 3,
    nonai_2d_src_dst_fmt_argb8888 = 4,
    nonai_2d_src_dst_fmt_argb4444 = 5,
    nonai_2d_src_dst_fmt_argb1555 = 6,
    nonai_2d_src_dst_fmt_xrgb8888 = 7,
    nonai_2d_src_dst_fmt_xrgb4444 = 8,
    nonai_2d_src_dst_fmt_xrgb1555 = 9,
    nonai_2d_src_dst_fmt_bgra8888 = 10,
    nonai_2d_src_dst_fmt_bgra4444 = 11,
    nonai_2d_src_dst_fmt_bgra5551 = 12,
    nonai_2d_src_dst_fmt_bgrx8888 = 13,
    nonai_2d_src_dst_fmt_bgrx4444 = 14,
    nonai_2d_src_dst_fmt_bgrx5551 = 15,
    nonai_2d_src_dst_fmt_rgb888 = 16,
    nonai_2d_src_dst_fmt_bgr888 = 17,
    nonai_2d_src_dst_fmt_rgb565 = 18,
    nonai_2d_src_dst_fmt_bgr565 = 19,
    nonai_2d_src_dst_fmt_seperate_rgb = 20
};
#define NONAI_2D_STREAM_MAIN_CTRL_CFG7_OFFSET(y) \
    (0x01c + y * 4) /* stream osd width & height */
#define NONAI_2D_OSD_WIDTH_OFFSET (0)
#define NONAI_2D_OSD_HEIGHT_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG8_OFFSET(y) \
    (0x03c + y * 4) /* stream osd start addr */
#define NONAI_2D_OSD_START_ADDR_X_OFFSET (0)
#define NONAI_2D_OSD_START_ADDR_Y_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG9_OFFSET(y) \
    (0x05c + y * 4) /* stream osd ch0 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG10_OFFSET(y) \
    (0x07c + y * 4) /* stream osd ch1 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG11_OFFSET(y) \
    (0x09c + y * 4) /* stream osd ch2 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(y) \
    (0x0bc + y * 4) /* stream osd ch0 & ch1 line byte stride */
#define NONAI_2D_OSD_CH0_LINE_BYTE_STRIDE_OFFSET (0)
#define NONAI_2D_OSD_CH1_LINE_BYTE_STRIDE_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG13_OFFSET(y) \
    (0x0dc + y * 4) /* stream osd ch2 line byte stride */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(y) \
    (0x0fc + y * 4) /* stream order & alpha */
#define NONAI_2D_ADD_ORDER_OFFSET (0)
enum nonai_2d_add_order {
    /* bottom ------> top */
    nonai_2d_add_order_video_osd = 0,
    nonai_2d_add_order_osd_video = 1,
    nonai_2d_add_order_video_bg = 2,
    nonai_2d_add_order_bg_video = 3,
    nonai_2d_add_order_video_bg_osd = 4,
    nonai_2d_add_order_video_osd_bg = 5,
    nonai_2d_add_order_bg_video_osd = 6,
    nonai_2d_add_order_bg_osd_video = 7,
    nonai_2d_add_order_osd_video_bg = 8,
    nonai_2d_add_order_osd_bg_video = 9
};
#define NONAI_2D_VIDEO_GLOBAL_ALPHA_OFFSET (8)
#define NONAI_2D_OSD_GLOBAL_ALPHA_OFFSET (16)
#define NONAI_2D_BG_GLOBAL_ALPHA_OFFSET (24)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG15_OFFSET(y) \
    (0x11c + y * 4) /* stream bg color */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG16_OFFSET (0x13c) /* stream dst ch0 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG17_OFFSET (0x140) /* stream dst ch1 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG18_OFFSET (0x144) /* stream dst ch2 addr */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET \
    (0x148) /* stream ch0 & ch1 line byte stride */
#define NONAI_2D_DST_CH0_LINE_BYTE_STRIDE_OFFSET (0)
#define NONAI_2D_DST_CH1_LINE_BYTE_STRIDE_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG20_OFFSET \
    (0x14c) /* stream ch2 line byte stride */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG21_OFFSET(y) \
    (0x150 + y * 4) /* stream border width & height */
#define NONAI_2D_BORDER_WIDTH_OFFSET (0)
#define NONAI_2D_BORDER_HEIGHT_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG22_OFFSET(y) \
    (0x1d0 + y * 4) /* stream border line wide */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG23_OFFSET(y) \
    (0x250 + y * 4) /* stream border color */
#define NONAI_2D_STREAM_MAIN_CTRL_CFG24_OFFSET(y) \
    (0x2d0 + y * 4) /* stream border start addr */
#define NONAI_2D_BORDER_START_ADDR_X_OFFSET (0)
#define NONAI_2D_BORDER_START_ADDR_Y_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG25_OFFSET \
    (0x350) /* stream csc coef 0 & 1 */
#define NONAI_2D_CSC_COEF_0_OFFSET (0)
#define NONAI_2D_CSC_COEF_1_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG26_OFFSET \
    (0x354) /* stream csc coef 2 & 3 */
#define NONAI_2D_CSC_COEF_2_OFFSET (0)
#define NONAI_2D_CSC_COEF_3_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG27_OFFSET \
    (0x358) /* stream csc coef 4 & 5 */
#define NONAI_2D_CSC_COEF_4_OFFSET (0)
#define NONAI_2D_CSC_COEF_5_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG28_OFFSET \
    (0x35c) /* stream csc coef 6 & 7 */
#define NONAI_2D_CSC_COEF_6_OFFSET (0)
#define NONAI_2D_CSC_COEF_7_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG29_OFFSET \
    (0x360) /* stream csc coef 8 & 9 */
#define NONAI_2D_CSC_COEF_8_OFFSET (0)
#define NONAI_2D_CSC_COEF_9_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG30_OFFSET \
    (0x364) /* stream csc coef 10 & 11 */
#define NONAI_2D_CSC_COEF_10_OFFSET (0)
#define NONAI_2D_CSC_COEF_11_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG31_OFFSET \
    (0x368) /* stream osd coef 0 & 1 */
#define NONAI_2D_CSC_OSD_0_OFFSET (0)
#define NONAI_2D_CSC_OSD_1_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG32_OFFSET \
    (0x36c) /* stream osd coef 2 & 3 */
#define NONAI_2D_CSC_OSD_2_OFFSET (0)
#define NONAI_2D_CSC_OSD_3_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG33_OFFSET \
    (0x370) /* stream osd coef 4 & 5 */
#define NONAI_2D_CSC_OSD_4_OFFSET (0)
#define NONAI_2D_CSC_OSD_5_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG34_OFFSET \
    (0x374) /* stream osd coef 6 & 7 */
#define NONAI_2D_CSC_OSD_6_OFFSET (0)
#define NONAI_2D_CSC_OSD_7_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG35_OFFSET \
    (0x378) /* stream osd coef 8 & 9 */
#define NONAI_2D_CSC_OSD_8_OFFSET (0)
#define NONAI_2D_CSC_OSD_9_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG36_OFFSET \
    (0x37c) /* stream osd coef 10 & 11 */
#define NONAI_2D_CSC_OSD_10_OFFSET (0)
#define NONAI_2D_CSC_OSD_11_OFFSET (16)
#define NONAI_2D_STREAM_MAIN_CTRL_CFG37_OFFSET(y) \
    (0x380 + y * 4) /* stream osd format */
enum nonai_2d_osd_fmt {
    nonai_2d_osd_fmt_argb8888 = 0,
    nonai_2d_osd_fmt_argb4444 = 1,
    nonai_2d_osd_fmt_argb1555 = 2,
    nonai_2d_osd_fmt_xrgb8888 = 3,
    nonai_2d_osd_fmt_xrgb4444 = 4,
    nonai_2d_osd_fmt_xrgb1555 = 5,
    nonai_2d_osd_fmt_bgra8888 = 6,
    nonai_2d_osd_fmt_bgra4444 = 7,
    nonai_2d_osd_fmt_bgra5551 = 8,
    nonai_2d_osd_fmt_bgrx8888 = 9,
    nonai_2d_osd_fmt_bgrx4444 = 10,
    nonai_2d_osd_fmt_bgrx5551 = 11,
    nonai_2d_osd_fmt_rgb888 = 12,
    nonai_2d_osd_fmt_bgr888 = 13,
    nonai_2d_osd_fmt_rgb565 = 14,
    nonai_2d_osd_fmt_bgr565 = 15,
    nonai_2d_osd_fmt_seperate_rgb = 16
};

#define NONAI_2D_MAIN_CTRL_CFG31 (NONAI_2D_STREAM0_REG_BASE + 0x3a0)
#define NONAI_2D_CALC_EN_OFFSET (0)
#define NONAI_2D_CSC_MATRIX_EN_OFFSET (1)
#define NONAI_2D_CALC_STREAM_ID_OFFSET (2)
#define NONAI_2D_CALC_MODE_OFFSET (4)
enum nonai_2d_calc_mode {
    nonai_2d_calc_mode_csc = 0,
    nonai_2d_calc_mode_osd = 1,
    nonai_2d_calc_mode_border = 2,
    nonai_2d_calc_mode_osd_border = 3

};
#define NONAI_2D_OSD_REGION_NUM_OFFSET (6)
#define NONAI_2D_DRAW_BORDER_NUM_OFFSET (10)
#define NONAI_2D_INTR_MASK_OFFSET (16)
#define NONAI_2D_MAIN_CTRL_CFG32 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3a4) /* watchdog num */
#define NONAI_2D_MAIN_CTRL_CFG33 (NONAI_2D_STREAM0_REG_BASE + 0x3a8)
#define NONAI_2D_INTR_TYPE_OFFSET (0)
enum nonai_2d_intr_type { intr_normal = 0, intr_timeout = 1, intr_error = 2 };
#define NONAI_2D_OS_CNT_OFFSET (16)
#define NONAI_2D_MAIN_CTRL_CFG34 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3ac) /* intr clear */
#define NONAI_2D_MAIN_CTRL_CFG35 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3b0) /* stop busy */
#define NONAI_2D_MAIN_CTRL_CFG36 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3b4) /* debug0 */
#define NONAI_2D_MAIN_CTRL_CFG37 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3b8) /* debug1 */
#define NONAI_2D_MAIN_CTRL_CFG38 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3bc) /* debug2 */
#define NONAI_2D_MAIN_CTRL_CFG39 \
    (NONAI_2D_STREAM0_REG_BASE + 0x3c0) /* disable clock gating */

struct pic_size {
    uint16_t width;
    uint16_t height;
};

struct k_nonai_2d_main_cfg {
    uint32_t nonai_2d_calc_en : 1;
    uint32_t csc_matrix_en : 1;
    uint32_t nonai_2d_calc_stream_id : 2;
    enum nonai_2d_calc_mode nonai_2d_calc_mode : 2;
    uint32_t osd_region_num : 4;
    uint32_t draw_border_num : 6;
    uint32_t nonai_2d_intr_mask : 1;
};

void *nonai_2d_get_stream_base(uint8_t stream_index);
int nonai_2d_enable(void);
// int nonai_2d_disable(void);
int nonai_2d_set_src_size(uint32_t width, uint32_t height, void *stream_offset);
int nonai_2d_set_src_ch0_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_src_ch1_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_src_ch2_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_src_addr(uint32_t addr, uint32_t channel_index,
              void *stream_offset);
int nonai_2d_set_src_ch0_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_src_ch1_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_src_ch2_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_src_line_byte_stride(uint32_t stride, uint32_t channel_index,
                      void *stream_offset);
int nonai_2d_set_fmt(enum nonai_2d_src_dst_fmt src_fmt,
             enum nonai_2d_src_dst_fmt dst_fmt, void *stream_offset);
int nonai_2d_set_osd_size(uint16_t width, uint16_t height, uint8_t osd_index,
              void *stream_offset);
int nonai_2d_set_osd_start_position(uint16_t addrx, uint16_t addry,
                    uint8_t osd_index, void *stream_offset);
int nonai_2d_set_osd_ch0_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset);
int nonai_2d_set_osd_ch1_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset);
int nonai_2d_set_osd_ch2_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset);
int nonai_2d_set_osd_addr(uint32_t addr, uint32_t channel_index,
              uint8_t osd_index, void *stream_offset);
int nonai_2d_set_osd_ch0_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset);
int nonai_2d_set_osd_ch1_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset);
int nonai_2d_set_osd_ch2_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset);
int nonai_2d_set_osd_line_byte_stride(uint32_t stride, uint32_t channel_index,
                      uint8_t osd_index, void *stream_offset);
int nonai_2d_set_add_order(enum nonai_2d_add_order add_order, uint8_t osd_index,
               void *stream_offset);
int nonai_2d_set_video_global_alpha(uint32_t video_global_alpha,
                    uint8_t osd_index, void *stream_offset);
int nonai_2d_set_osd_global_alpha(uint32_t osd_global_alpha, uint8_t osd_index,
                  void *stream_offset);
int nonai_2d_set_bg_global_alpha(uint32_t bg_global_alpha, uint8_t osd_index,
                 void *stream_offset);
int nonai_2d_set_bg_color(uint32_t color, uint8_t osd_index,
              void *stream_offset);
int nonai_2d_set_dst_ch0_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_dst_ch1_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_dst_ch2_addr(uint32_t addr, void *stream_offset);
int nonai_2d_set_dst_ch0_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_dst_ch1_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_dst_ch2_line_byte_stride(uint32_t stride, void *stream_offset);
int nonai_2d_set_border_size(uint16_t width, uint16_t height,
                 uint8_t border_index, void *stream_offset);
int nonai_2d_set_border_wide(uint16_t wide, uint8_t border_index,
                 void *stream_offset);
int nonai_2d_set_border_color(uint32_t color, uint8_t border_index,
                  void *stream_offset);
int nonai_2d_set_border_start_position(uint32_t addrx, uint32_t addry,
                       uint8_t border_index,
                       void *stream_offset);
int nonai_2d_set_csc_coef(uint16_t *coef, void *stream_offset);
int nonai_2d_set_osd_coef(uint16_t *coef, void *stream_offset);
int nonai_2d_set_osd_fmt(enum nonai_2d_osd_fmt osd_fmt, uint8_t osd_index,
             void *stream_offset);

struct pic_size nonai_2d_get_src_size(void *stream_offset);
int nonai_2d_get_src_ch0_addr(void *stream_offset);
int nonai_2d_get_src_ch1_addr(void *stream_offset);
int nonai_2d_get_src_ch2_addr(void *stream_offset);
uint16_t nonai_2d_get_src_ch0_line_byte_stride(void *stream_offset);
uint16_t nonai_2d_get_src_ch1_line_byte_stride(void *stream_offset);
uint16_t nonai_2d_get_src_ch2_line_byte_stride(void *stream_offset);
enum nonai_2d_src_dst_fmt nonai_2d_get_src_fmt(void *stream_offset);
enum nonai_2d_src_dst_fmt nonai_2d_get_dst_fmt(void *stream_offset);
struct pic_size nonai_2d_get_osd_size(uint8_t osd_index, void *stream_offset);
uint16_t nonai_2d_get_osd_startx_position(uint8_t osd_index,
                      void *stream_offset);
uint16_t nonai_2d_get_osd_starty_position(uint8_t osd_index,
                      void *stream_offset);
uint32_t nonai_2d_get_osd_ch0_addr(uint8_t osd_index, void *stream_offset);
uint32_t nonai_2d_get_osd_ch1_addr(uint8_t osd_index, void *stream_offset);
uint32_t nonai_2d_get_osd_ch2_addr(uint8_t osd_index, void *stream_offset);
uint16_t nonai_2d_get_osd_ch0_line_byte_stride(uint8_t osd_index,
                           void *stream_offset);
uint16_t nonai_2d_get_osd_ch1_line_byte_stride(uint8_t osd_index,
                           void *stream_offset);
uint16_t nonai_2d_get_osd_ch2_line_byte_stride(uint8_t osd_index,
                           void *stream_offset);
enum nonai_2d_add_order nonai_2d_get_add_order(uint8_t osd_index,
                           void *stream_offset);
uint8_t nonai_2d_get_video_global_alpha(uint8_t osd_index, void *stream_offset);
uint8_t nonai_2d_get_osd_global_alpha(uint8_t osd_index, void *stream_offset);
uint8_t nonai_2d_get_bg_global_alpha(uint8_t osd_index, void *stream_offset);
uint32_t nonai_2d_get_bg_color(uint8_t osd_index, void *stream_offset);
uint32_t nonai_2d_get_dst_ch0_addr(void *stream_offset);
uint32_t nonai_2d_get_dst_ch1_addr(void *stream_offset);
uint32_t nonai_2d_get_dst_ch2_addr(void *stream_offset);
uint16_t nonai_2d_get_dst_ch0_line_byte_stride(void *stream_offset);
uint16_t nonai_2d_get_dst_ch1_line_byte_stride(void *stream_offset);
uint16_t nonai_2d_get_dst_ch2_line_byte_stride(void *stream_offset);
struct pic_size nonai_2d_get_border_size(uint8_t border_index, void *stream_offset);
uint16_t nonai_2d_get_border_wide(uint8_t border_index, void *stream_offset);
uint32_t nonai_2d_get_border_color(uint8_t border_index, void *stream_offset);
uint16_t nonai_2d_get_border_startx_position(uint8_t border_index,
                         void *stream_offset);
uint16_t nonai_2d_get_border_starty_position(uint8_t border_index,
                         void *stream_offset);
int nonai_2d_get_csc_coef(uint16_t *coef, void *stream_offset);
int nonai_2d_get_osd_coef(uint16_t *coef, void *stream_offset);
enum nonai_2d_osd_fmt nonai_2d_get_osd_fmt(uint8_t osd_index,
                       void *stream_offset);
int nonai_2d_main_set_calc_en(uint32_t enable, void *stream_offset);
int nonai_2d_main_set_csc_matrix_en(uint32_t enable, void *stream_offset);
int nonai_2d_main_set_calc_stream_id(uint32_t stream_index, void *stream_offset);
int nonai_2d_main_set_calc_mode(enum nonai_2d_calc_mode mode, void *stream_offset);
int nonai_2d_main_set_osd_region_num(uint32_t region_num, void *stream_offset);
int nonai_2d_main_set_border_num(uint32_t border_num, void *stream_offset);
int nonai_2d_main_set_intr_mask(uint32_t mask, void *stream_offset);
int nonai_2d_main_cfg(void *stream_offset, uint8_t calc_en, uint8_t matrix_en,
              uint8_t stream_index, enum nonai_2d_calc_mode calc_mode,
              uint8_t osd_num, uint8_t border_num, uint8_t intr_mask);
int nonai_2d_disable(void *stream_offset);
int nonai_2d_main_set_wcd_num(uint32_t num, void *stream_offset);
enum nonai_2d_intr_type nonai_2d_main_get_intr_type(void *stream_offset);
uint32_t nonai_2d_main_get_outstanding_cnt(void *stream_offset);
uint32_t nonai_2d_main_get_intr(void *stream_offset);
int nonai_2d_main_clear_intr(void *offset);
uint32_t nonai_2d_main_get_stop_busy(void *stream_offset);

#endif /* _NONAI_2D_REG_H_ */
