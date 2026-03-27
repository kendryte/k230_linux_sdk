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

#include "nonai_2d.h"
#include "nonai_2d_reg.h"

int nonai_2d_set_src_size(uint32_t width, uint32_t height, void *stream_offset)
{
    writel((width << NONAI_2D_SRC_WIDTH_OFFSET) |
               (height << NONAI_2D_SRC_HEIGHT_OFFSET),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG0_OFFSET);
    return 0;
}

int nonai_2d_set_src_ch0_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG1_OFFSET);
    return 0;
}

int nonai_2d_set_src_ch1_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG2_OFFSET);
    return 0;
}

int nonai_2d_set_src_ch2_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG3_OFFSET);
    return 0;
}

int nonai_2d_set_src_addr(uint32_t addr, uint32_t channel_index,
              void *stream_offset)
{
    switch (channel_index) {
    case 0:
        nonai_2d_set_src_ch0_addr(addr, stream_offset);
        break;
    case 1:
        nonai_2d_set_src_ch1_addr(addr, stream_offset);
        break;
    case 2:
        nonai_2d_set_src_ch2_addr(addr, stream_offset);
        break;
    default:
        return -1;
    }
    return 0;
}

int nonai_2d_set_src_ch0_line_byte_stride(uint32_t stride, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET);
    value &= ~(0x0000ffff);
    value |= (stride << NONAI_2D_SRC_CH0_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET);
    return 0;
}

int nonai_2d_set_src_ch1_line_byte_stride(uint32_t stride, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET);
    value &= ~(0xffff0000);
    value |= (stride << NONAI_2D_SRC_CH1_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET);
    return 0;
}

int nonai_2d_set_src_ch2_line_byte_stride(uint32_t stride, void *stream_offset)
{
    writel(stride, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG5_OFFSET);
    return 0;
}

int nonai_2d_set_src_line_byte_stride(uint32_t stride, uint32_t channel_index,
                      void *stream_offset)
{
    switch (channel_index) {
    case 0:
        nonai_2d_set_src_ch0_line_byte_stride(stride, stream_offset);
        break;
    case 1:
        nonai_2d_set_src_ch1_line_byte_stride(stride, stream_offset);
        break;
    case 2:
        nonai_2d_set_src_ch2_line_byte_stride(stride, stream_offset);
        break;
    default:
        return -1;
    }
    return 0;
}

int nonai_2d_set_fmt(enum nonai_2d_src_dst_fmt src_fmt,
             enum nonai_2d_src_dst_fmt dst_fmt, void *stream_offset)
{
    writel((src_fmt << NONAI_2D_SRC_FORMAT_OFFSET) |
               (dst_fmt << NONAI_2D_DST_FORMAT_OFFSET),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG6_OFFSET);
    return 0;
}

int nonai_2d_set_osd_size(uint16_t width, uint16_t height, uint8_t osd_index,
              void *stream_offset)
{
    writel((width << NONAI_2D_OSD_WIDTH_OFFSET) |
               (height << NONAI_2D_OSD_HEIGHT_OFFSET),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG7_OFFSET(0) +
               osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_start_position(uint16_t addrx, uint16_t addry,
                    uint8_t osd_index, void *stream_offset)
{
    writel((addrx << NONAI_2D_OSD_START_ADDR_X_OFFSET) |
               (addry << NONAI_2D_OSD_START_ADDR_Y_OFFSET),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG8_OFFSET(0) +
               osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_ch0_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG9_OFFSET(0) +
                 osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_ch1_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG10_OFFSET(0) +
                 osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_ch2_addr(uint32_t addr, uint8_t osd_index,
                  void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG11_OFFSET(0) +
                 osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_addr(uint32_t addr, uint32_t channel_index,
              uint8_t osd_index, void *stream_offset)
{
    switch (channel_index) {
    case 0:
        nonai_2d_set_osd_ch0_addr(addr, osd_index, stream_offset);
        break;
    case 1:
        nonai_2d_set_osd_ch1_addr(addr, osd_index, stream_offset);
        break;
    case 2:
        nonai_2d_set_osd_ch2_addr(addr, osd_index, stream_offset);
        break;
    default:
        return -1;
    }
    return 0;
}

int nonai_2d_set_osd_ch0_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
              osd_index * 4);
    value &= ~(0x0000ffff);
    value |= (stride << NONAI_2D_OSD_CH0_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_ch1_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
              osd_index * 4);
    value &= ~(0xffff0000);
    value |= (stride << NONAI_2D_OSD_CH1_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_ch2_line_byte_stride(uint32_t stride, uint8_t osd_index,
                      void *stream_offset)
{
    writel(stride, stream_offset +
                   NONAI_2D_STREAM_MAIN_CTRL_CFG13_OFFSET(0) +
                   osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_line_byte_stride(uint32_t stride, uint32_t channel_index,
                      uint8_t osd_index, void *stream_offset)
{
    switch (channel_index) {
    case 0:
        nonai_2d_set_osd_ch0_line_byte_stride(stride, osd_index,
                              stream_offset);
        break;
    case 1:
        nonai_2d_set_osd_ch1_line_byte_stride(stride, osd_index,
                              stream_offset);
        break;
    case 2:
        nonai_2d_set_osd_ch2_line_byte_stride(stride, osd_index,
                              stream_offset);
        break;
    default:
        return -1;
    }
    return 0;
}

int nonai_2d_set_add_order(enum nonai_2d_add_order add_order, uint8_t osd_index,
               void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    value &= (0xfffffff0);
    value |= (add_order << NONAI_2D_ADD_ORDER_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_video_global_alpha(uint32_t video_global_alpha,
                    uint8_t osd_index, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    value &= (0xffff00ff);
    value |= (video_global_alpha << NONAI_2D_VIDEO_GLOBAL_ALPHA_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_osd_global_alpha(uint32_t osd_global_alpha, uint8_t osd_index,
                  void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    value &= (0xff00ffff);
    value |= (osd_global_alpha << NONAI_2D_OSD_GLOBAL_ALPHA_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_bg_global_alpha(uint32_t bg_global_alpha, uint8_t osd_index,
                 void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    value &= (0x00ffffff);
    value |= (bg_global_alpha << NONAI_2D_BG_GLOBAL_ALPHA_OFFSET);
    writel(value, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_bg_color(uint32_t color, uint8_t osd_index,
              void *stream_offset)
{
    writel(color, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG15_OFFSET(0) +
                  osd_index * 4);
    return 0;
}

int nonai_2d_set_dst_ch0_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG16_OFFSET);
    return 0;
}

int nonai_2d_set_dst_ch1_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG17_OFFSET);
    return 0;
}

int nonai_2d_set_dst_ch2_addr(uint32_t addr, void *stream_offset)
{
    writel(addr, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG18_OFFSET);
    return 0;
}

int nonai_2d_set_dst_ch0_line_byte_stride(uint32_t stride, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET);
    value &= ~(0x0000ffff);
    value |= (stride << NONAI_2D_DST_CH0_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET);
    return 0;
}

int nonai_2d_set_dst_ch1_line_byte_stride(uint32_t stride, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET);
    value &= ~(0xffff0000);
    value |= (stride << NONAI_2D_DST_CH1_LINE_BYTE_STRIDE_OFFSET);
    writel(value, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET);
    return 0;
}

int nonai_2d_set_dst_ch2_line_byte_stride(uint32_t stride, void *stream_offset)
{
    writel(stride, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG20_OFFSET);
    return 0;
}

int nonai_2d_set_dst_line_byte_stride(uint32_t stride, uint32_t channel_index,
                      void *stream_offset)
{
    switch (channel_index) {
    case 0:
        nonai_2d_set_dst_ch0_line_byte_stride(stride, stream_offset);
        break;
    case 1:
        nonai_2d_set_dst_ch1_line_byte_stride(stride, stream_offset);
        break;
    case 2:
        nonai_2d_set_dst_ch2_line_byte_stride(stride, stream_offset);
        break;
    default:
        return -1;
    }
    return 0;
}

int nonai_2d_set_border_size(uint16_t width, uint16_t height,
                 uint8_t border_index, void *stream_offset)
{
    writel(((width << NONAI_2D_BORDER_WIDTH_OFFSET) |
        (height << NONAI_2D_BORDER_HEIGHT_OFFSET)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG21_OFFSET(0) +
               border_index * 4);
    return 0;
}

int nonai_2d_set_border_wide(uint16_t wide, uint8_t border_index,
                 void *stream_offset)
{
    writel(wide, stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG22_OFFSET(0) +
                 border_index * 4);
    return 0;
}

int nonai_2d_set_border_color(uint32_t color, uint8_t border_index,
                  void *stream_offset)
{
    writel(color, stream_offset +
                  NONAI_2D_STREAM_MAIN_CTRL_CFG23_OFFSET(0) +
                  border_index * 4);
    return 0;
}

int nonai_2d_set_border_start_position(uint32_t addrx, uint32_t addry,
                       uint8_t border_index,
                       void *stream_offset)
{
    writel(((addrx << NONAI_2D_BORDER_START_ADDR_X_OFFSET) |
        (addry << NONAI_2D_BORDER_START_ADDR_Y_OFFSET)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG24_OFFSET(0) +
               border_index * 4);
    return 0;
}

int nonai_2d_set_csc_coef(uint16_t *coef, void *stream_offset)
{
    writel(((coef[0] & 0xffff) << 0) | ((coef[1] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG25_OFFSET);
    writel(((coef[2] & 0xffff) << 0) | ((coef[3] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG26_OFFSET);
    writel(((coef[4] & 0xffff) << 0) | ((coef[5] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG27_OFFSET);
    writel(((coef[6] & 0xffff) << 0) | ((coef[7] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG28_OFFSET);
    writel(((coef[8] & 0xffff) << 0) | ((coef[9] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG29_OFFSET);
    writel(((coef[10] & 0xffff) << 0) | ((coef[11] & 0xffff) << 16),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG30_OFFSET);
    return 0;
}

int nonai_2d_set_osd_coef(uint16_t *coef, void *stream_offset)
{
    writel(((*(coef + 0) << 0) | (*(coef + 1) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG31_OFFSET);
    writel(((*(coef + 2) << 0) | (*(coef + 3) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG32_OFFSET);
    writel(((*(coef + 4) << 0) | (*(coef + 5) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG33_OFFSET);
    writel(((*(coef + 6) << 0) | (*(coef + 7) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG34_OFFSET);
    writel(((*(coef + 8) << 0) | (*(coef + 9) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG35_OFFSET);
    writel(((*(coef + 10) << 0) | (*(coef + 11) << 16)),
           stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG36_OFFSET);
    return 0;
}

int nonai_2d_set_osd_fmt(enum nonai_2d_osd_fmt osd_fmt, uint8_t osd_index,
             void *stream_offset)
{
    writel(osd_fmt, stream_offset +
                NONAI_2D_STREAM_MAIN_CTRL_CFG37_OFFSET(0) +
                osd_index * 4);
    return 0;
}

struct pic_size nonai_2d_get_src_size(void *stream_offset)
{
    uint32_t value;
    struct pic_size pic_size = { 0 };

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG0_OFFSET);
    pic_size.width = (value & 0x0000ffff) >> NONAI_2D_SRC_WIDTH_OFFSET;
    pic_size.height = (value & 0xffff0000) >> NONAI_2D_SRC_HEIGHT_OFFSET;
    return pic_size;
}

int nonai_2d_get_src_ch0_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG1_OFFSET);
}

int nonai_2d_get_src_ch1_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG2_OFFSET);
}

int nonai_2d_get_src_ch2_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG3_OFFSET);
}

uint16_t nonai_2d_get_src_ch0_line_byte_stride(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET) &
        0x0000ffff) >>
           NONAI_2D_SRC_CH0_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_src_ch1_line_byte_stride(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG4_OFFSET) &
        0xffff0000) >>
           NONAI_2D_SRC_CH1_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_src_ch2_line_byte_stride(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG5_OFFSET) &
           0x0000ffff;
}

enum nonai_2d_src_dst_fmt nonai_2d_get_src_fmt(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG6_OFFSET) &
        0x0000ffff) >>
           NONAI_2D_SRC_FORMAT_OFFSET;
}

enum nonai_2d_src_dst_fmt nonai_2d_get_dst_fmt(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG6_OFFSET) &
        0xffff0000) >>
           NONAI_2D_DST_FORMAT_OFFSET;
}

struct pic_size nonai_2d_get_osd_size(uint8_t osd_index, void *stream_offset)
{
    struct pic_size osd_size;
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG7_OFFSET(0) +
              osd_index * 4);
    osd_size.width = value & 0x0000ffff;
    osd_size.height = (value & 0xffff0000) >> NONAI_2D_OSD_HEIGHT_OFFSET;

    return osd_size;
}

uint16_t nonai_2d_get_osd_startx_position(uint8_t osd_index,
                      void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG8_OFFSET(0) +
              osd_index * 4) &
        0x0000ffff) >>
           NONAI_2D_BORDER_START_ADDR_X_OFFSET;
}

uint16_t nonai_2d_get_osd_starty_position(uint8_t osd_index,
                      void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG8_OFFSET(0) +
              osd_index * 4) &
        0xffff0000) >>
           NONAI_2D_BORDER_START_ADDR_Y_OFFSET;
}

uint32_t nonai_2d_get_osd_ch0_addr(uint8_t osd_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG9_OFFSET(0) +
             osd_index * 4);
}

uint32_t nonai_2d_get_osd_ch1_addr(uint8_t osd_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG10_OFFSET(0) +
             osd_index * 4);
}

uint32_t nonai_2d_get_osd_ch2_addr(uint8_t osd_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG11_OFFSET(0) +
             osd_index * 4);
}

uint16_t nonai_2d_get_osd_ch0_line_byte_stride(uint8_t osd_index,
                           void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
              osd_index * 4);
    return (value & 0x0000ffff) >> NONAI_2D_OSD_CH0_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_osd_ch1_line_byte_stride(uint8_t osd_index,
                           void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG12_OFFSET(0) +
              osd_index * 4);
    return (value & 0xffff0000) >> NONAI_2D_OSD_CH1_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_osd_ch2_line_byte_stride(uint8_t osd_index,
                           void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG13_OFFSET(0) +
              osd_index * 4);
    return value & 0x0000ffff;
}

enum nonai_2d_add_order nonai_2d_get_add_order(uint8_t osd_index,
                           void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    return (enum nonai_2d_add_order)(value & (0xf));
}

uint8_t nonai_2d_get_video_global_alpha(uint8_t osd_index, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    return (value & 0x0000ff00) >> 8;
}

uint8_t nonai_2d_get_osd_global_alpha(uint8_t osd_index, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    return (value & (0x00ff0000)) >> 16;
}

uint8_t nonai_2d_get_bg_global_alpha(uint8_t osd_index, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG14_OFFSET(0) +
              osd_index * 4);
    return (value & (0xff000000)) >> 24;
}

uint32_t nonai_2d_get_bg_color(uint8_t osd_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG15_OFFSET(0) +
             osd_index * 4);
}

uint32_t nonai_2d_get_dst_ch0_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG16_OFFSET);
}

uint32_t nonai_2d_get_dst_ch1_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG17_OFFSET);
}

uint32_t nonai_2d_get_dst_ch2_addr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG18_OFFSET);
}

uint16_t nonai_2d_get_dst_ch0_line_byte_stride(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET) &
        0x0000ffff) >>
           NONAI_2D_DST_CH0_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_dst_ch1_line_byte_stride(void *stream_offset)
{
    return (readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG19_OFFSET) &
        0xffff0000) >>
           NONAI_2D_DST_CH1_LINE_BYTE_STRIDE_OFFSET;
}

uint16_t nonai_2d_get_dst_ch2_line_byte_stride(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG20_OFFSET);
}

struct pic_size nonai_2d_get_border_size(uint8_t border_index,
                     void *stream_offset)
{
    uint32_t value;
    struct pic_size size;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG21_OFFSET(0) +
              border_index * 4);
    size.width = value & 0x0000ffff;
    size.height = (value & 0xffff0000) >> 16;
    return size;
}

uint16_t nonai_2d_get_border_wide(uint8_t border_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG22_OFFSET(0) +
             border_index * 4);
}

uint32_t nonai_2d_get_border_color(uint8_t border_index, void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG23_OFFSET(0) +
             border_index * 4);
}

uint16_t nonai_2d_get_border_startx_position(uint8_t border_index,
                         void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG24_OFFSET(0) +
              border_index * 4);
    return (value & 0x0000ffff) >> NONAI_2D_BORDER_START_ADDR_X_OFFSET;
}

uint16_t nonai_2d_get_border_starty_position(uint8_t border_index,
                         void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset +
              NONAI_2D_STREAM_MAIN_CTRL_CFG24_OFFSET(0) +
              border_index * 4);
    return (value & 0xffff0000) >> NONAI_2D_BORDER_START_ADDR_Y_OFFSET;
}

int nonai_2d_get_csc_coef(uint16_t *coef, void *stream_offset)
{
    uint32_t value = 0;
    int i;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG25_OFFSET);
    coef[0] = value & 0xffff;
    coef[1] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG26_OFFSET);
    coef[2] = value & 0xffff;
    coef[3] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG27_OFFSET);
    coef[4] = value & 0xffff;
    coef[5] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG28_OFFSET);
    coef[6] = value & 0xffff;
    coef[7] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG29_OFFSET);
    coef[8] = value & 0xffff;
    coef[9] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG30_OFFSET);
    coef[10] = value & 0xffff;
    coef[11] = (value >> 16) & 0xffff;

    for (i = 0; i < 12; i++) {
        if (coef[i] & 0x0800)
            coef[i] |= 0xf000;
    }
    return 0;
}

int nonai_2d_get_osd_coef(uint16_t *coef, void *stream_offset)
{
    uint32_t value;
    int i;

    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG31_OFFSET);
    coef[0] = value & 0xffff;
    coef[1] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG32_OFFSET);
    coef[2] = value & 0xffff;
    coef[3] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG33_OFFSET);
    coef[4] = value & 0xffff;
    coef[5] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG34_OFFSET);
    coef[6] = value & 0xffff;
    coef[7] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG35_OFFSET);
    coef[8] = value & 0xffff;
    coef[9] = (value >> 16) & 0xffff;
    value = readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG36_OFFSET);
    coef[10] = value & 0xffff;
    coef[11] = (value >> 16) & 0xffff;

    for (i = 0; i < 12; i++) {
        if (coef[i] & 0x0800)
            coef[i] |= 0xf000;
    }
    return 0;
}

enum nonai_2d_osd_fmt nonai_2d_get_osd_fmt(uint8_t osd_index,
                       void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_STREAM_MAIN_CTRL_CFG37_OFFSET(0) +
             osd_index * 4);
}

int nonai_2d_main_set_calc_en(uint32_t enable, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x00000001);
    value |= (enable << NONAI_2D_CALC_EN_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_csc_matrix_en(uint32_t enable, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x00000002);
    value |= (enable << NONAI_2D_CSC_MATRIX_EN_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_calc_stream_id(uint32_t stream_index, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x0000000c);
    value |= (stream_index << NONAI_2D_CALC_STREAM_ID_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_calc_mode(enum nonai_2d_calc_mode mode, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x00000030);
    value |= (mode << NONAI_2D_CALC_MODE_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_osd_region_num(uint32_t region_num, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x000003c0);
    value |= (region_num << NONAI_2D_OSD_REGION_NUM_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_border_num(uint32_t border_num, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x0000fc00);
    value |= (border_num << NONAI_2D_DRAW_BORDER_NUM_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_intr_mask(uint32_t mask, void *stream_offset)
{
    uint32_t value;

    value = readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    value &= ~(0x00010000);
    value |= (mask << NONAI_2D_INTR_MASK_OFFSET);
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_cfg(void *stream_offset, uint8_t calc_en, uint8_t matrix_en,
              uint8_t stream_index, enum nonai_2d_calc_mode calc_mode,
              uint8_t osd_num, uint8_t border_num, uint8_t intr_mask)
{
    uint32_t value;

    value = (((calc_en & 0x1) << NONAI_2D_CALC_EN_OFFSET) |
         ((matrix_en & 0x1) << NONAI_2D_CSC_MATRIX_EN_OFFSET) |
         ((stream_index & 0x3) << NONAI_2D_CALC_STREAM_ID_OFFSET) |
         ((calc_mode & 0x3) << NONAI_2D_CALC_MODE_OFFSET) |
         ((osd_num & 0xf) << NONAI_2D_OSD_REGION_NUM_OFFSET) |
         ((border_num & 0x3f) << NONAI_2D_DRAW_BORDER_NUM_OFFSET) |
         ((intr_mask & 0x1) << NONAI_2D_INTR_MASK_OFFSET));
    writel(value, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_disable(void *stream_offset)
{
    writel(0, stream_offset + NONAI_2D_MAIN_CTRL_CFG31);
    return 0;
}

int nonai_2d_main_set_wcd_num(uint32_t num, void *stream_offset)
{
    writel(num, stream_offset + NONAI_2D_MAIN_CTRL_CFG32);
    return 0;
}

enum nonai_2d_intr_type nonai_2d_main_get_intr_type(void *stream_offset)
{
    enum nonai_2d_intr_type intr_type;

    intr_type = (readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG33) & 0x0000ffff);
    return intr_type;
}

uint32_t nonai_2d_main_get_outstanding_cnt(void *stream_offset)
{
    uint32_t cnt;

    cnt = (readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG33) & 0xffff0000) >>
          NONAI_2D_OS_CNT_OFFSET;
    return cnt;
}

uint32_t nonai_2d_main_get_intr(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG33) & 0xffff;
}

int nonai_2d_main_clear_intr(void *offset)
{
    writel(0x1, offset + NONAI_2D_MAIN_CTRL_CFG34);
    return 0;
}

uint32_t nonai_2d_main_get_stop_busy(void *stream_offset)
{
    return readl(stream_offset + NONAI_2D_MAIN_CTRL_CFG35);
}
