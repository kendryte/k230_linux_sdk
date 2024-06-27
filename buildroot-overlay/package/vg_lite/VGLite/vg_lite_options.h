/****************************************************************************
*
*    Copyright 2012 - 2022 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#ifndef _vg_lite_options_h_
#define _vg_lite_options_h_

#define CHIPID          0x265
#define REVISION        0x1003
#define CID             0x417

#define gcFEATURE_VG_IM_INDEX_FORMAT    1
#define gcFEATURE_VG_SCISSOR    1
#define gcFEATURE_VG_BORDER_CULLING    1
#define gcFEATURE_VG_RGBA2_FORMAT    1
#define gcFEATURE_VG_QUALITY_8X    0
#define gcFEATURE_VG_IM_FASTCLEAR    0
#define gcFEATURE_VG_RADIAL_GRADIENT    0
#define gcFEATURE_VG_GLOBAL_ALPHA    1
#define gcFEATURE_VG_RGBA8_ETC2_EAC    1
#define gcFEATURE_VG_COLOR_KEY    0
#define gcFEATURE_VG_DOUBLE_IMAGE    0
#define gcFEATURE_VG_YUV_OUTPUT    0
#define gcFEATURE_VG_FLEXA    0
#define gcFEATURE_VG_24BIT    0
#define gcFEATURE_VG_DITHER    0
#define gcFEATURE_VG_USE_DST    0
#define gcFEATURE_VG_PE_CLEAR    1
#define gcFEATURE_VG_IM_INPUT    1
#define gcFEATURE_VG_DEC_COMPRESS    0
#define gcFEATURE_VG_LINEAR_GRADIENT_EXT    0
#define gcFEATURE_VG_MASK    0
#define gcFEATURE_VG_MIRROR    0
#define gcFEATURE_VG_GAMMA    0
#define gcFEATURE_VG_NEW_BLEND_MODE    0
#define gcFEATURE_VG_STENCIL    0
#define gcFEATURE_VG_SRC_PREMULTIPLIED    1
#define gcFEATURE_VG_HW_PREMULTIPLY    0
#define gcFEATURE_VG_COLOR_TRANSFORMATION    0
#define gcFEATURE_VG_LVGL_SUPPORT    0
#define gcFEATURE_VG_INDEX_ENDIAN    0
#define gcFEATURE_VG_24BIT_PLANAR    0
#define gcFEATURE_VG_PIXEL_MATRIX    0
#define gcFEATURE_VG_NEW_IMAGE_INDEX    0
#define gcFEATURE_VG_PARALLEL_PATHS    0
#define gcFEATURE_VG_STRIPE_MODE    0
#define gcFEATURE_VG_IM_DEC_INPUT    0
#define gcFEATURE_VG_GAUSSIAN_BLUR    0
#define gcFEATURE_VG_RECTANGLE_TILED_OUT    1
#define gcFEATURE_VG_TESSELLATION_TILED_OUT    1
#define gcFEATURE_VG_IM_REPEAT_REFLECT    0
#define gcFEATURE_VG_YUY2_INPUT    1
#define gcFEATURE_VG_YUV_INPUT    0
#define gcFEATURE_VG_YUV_TILED_INPUT    0
#define gcFEATURE_VG_AYUV_INPUT    0
#define gcFEATURE_VG_16PIXELS_ALIGNED    1

/* SW Features */
#define gcFEATURE_VG_STROKE_PATH    1
#define gcFEATURE_VG_ARC_PATH    1
#define gcFEATURE_VG_ERROR_CHECK    1
#define gcFEATURE_VG_TRACE_API    0
#endif
