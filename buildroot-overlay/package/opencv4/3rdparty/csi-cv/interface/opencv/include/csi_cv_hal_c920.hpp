/*
 * Copyright (C) 2021-2022 T-HEAD Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef _csi_cv_hal_H_INCLUDED_
#define _csi_cv_hal_H_INCLUDED_

#include <cstddef>
#include <cstring>
#include <vector>
#include <opencv2/core/base.hpp>

#ifdef __cplusplus
extern "C"{
#endif

struct cvhalFilter2D;
int morph_u8_init(cvhalFilter2D **ctx, int operation, int src_type, int dst_type, int max_width, int max_height, int kernel_type, unsigned char *kernel_data,
                        size_t kernel_step, int kernel_width, int kernel_height, int anchor_x, int anchor_y,
                        int borderType, const double borderValue[4], int iterations, int allowSubmatrix, int allowInplace);

int morph_u8_deinit(cvhalFilter2D *handle);

int morph_u8(cvhalFilter2D *ctx, unsigned char *src_data, size_t src_step, unsigned char *dst_data, size_t dst_step, int width, int height,
                int src_full_width, int src_full_height, int src_roi_x, int src_roi_y, int dst_full_width, int dst_full_height, int dst_roi_x, int dst_roi_y);

int integral_wrap(int depth, int sdepth, int sqdepth, const unsigned char *src_data, size_t src_step, unsigned char *sum_data, size_t sum_step,
                    unsigned char *sqsum_data, size_t sqsum_step, unsigned char *tilted_data, size_t tilted_step, int width, int height, int cn);

int thresh_wrap(const unsigned char* src_data, size_t src_step, unsigned char* dst_data, size_t dst_step, int width, int height,
                int depth, int cn, double thresh, double maxValue, int thresholdType);

int scharr_u8(const unsigned char* src_data, size_t src_step, unsigned char* dst_data, size_t dst_step, int width, int height,
                int src_depth, int dst_depth, int cn, int margin_left, int margin_top, int margin_right, int margin_bottom,
                int dx, int dy, double scale, double delta, int border_type);

int median_blur_u8(const unsigned char* src_data, size_t src_step, unsigned char* dst_data,
                    size_t dst_step, int width, int height, int depth, int cn, int ksize);

int blur_u8(const unsigned char* src_data, size_t src_step, unsigned char* dst_data, size_t dst_step, int width, int height,
            int src_depth, int dst_depth, int cn, int margin_left, int margin_top, int margin_right, int margin_bottom,
            size_t ksize_width, size_t ksize_height, int anchor_x, int anchor_y, int normalize, int border_type);

int sobel_u8(const unsigned char* src_data, size_t src_step, unsigned char* dst_data, size_t dst_step, int width, int height,
                int src_depth, int dst_depth, int cn, int margin_left, int margin_top, int margin_right, int margin_bottom,
                int dx, int dy, int ksize, double scale, double delta, int border_type);

int gaussian_u8(const unsigned char* src_data, size_t src_step, unsigned char* dst_data, size_t dst_step, int width, int height, 
                int depth, int cn, size_t margin_left, size_t margin_top, size_t margin_right, size_t margin_bottom, 
                size_t ksize_width, size_t ksize_height, double sigmaX, double sigmaY, int border_type);

int filter_init(cvhalFilter2D **context, unsigned char *kernel_data, size_t kernel_step, int kernel_type,
                        int kernel_width, int kernel_height, int max_width, int max_height, int src_type,
                        int dst_type, int borderType, double delta, int anchor_x, int anchor_y,
                        int allowSubmatrix, int allowInplace);

int filter(cvhalFilter2D *context, unsigned char *src_data, size_t src_step, unsigned char *dst_data,
            size_t dst_step, int width, int height, int full_width, int full_height, int offset_x, int offset_y);

int filter_free(cvhalFilter2D *context);

int resize_u8(int src_type, const unsigned char *src_data, size_t src_step, int src_width, int src_height, unsigned char *dst_data, 
                    size_t dst_step, int dst_width, int dst_height, double inv_scale_x, double inv_scale_y, int interpolation);

int cvt_bgr_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data,
                    size_t dst_step, int width, int height, int depth, int scn, int dcn, int swap_blue);

int cvt_bgr_bgr5x5_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data,
                        size_t dst_step, int width, int height, int scn, int swapBlue, int greenBits);

int cvt_bgr5x5_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data,
                        size_t dst_step, int width, int height, int dcn, int swap_blue, int green_bits);

int cvt_bgr_gray_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data,
                        size_t dst_step, int width, int height, int depth, int scn, bool swapBlue);

int cvt_gray_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data,
                        size_t dst_step, int width, int height, int depth, int dcn);

int cvt_bgr_yuv_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                        int width, int height, int depth, int scn, int swap_blue, int is_cbcr);

int cvt_yuv_bgr_u8(const unsigned char *src_data, size_t src_step, unsigned char *dst_data, size_t dst_step,
                    int width, int height, int depth, int dcn, int swap_blue, int is_cbcr);

int cvt_bgr_xyz_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                    int width, int height, int depth, int scn, int swap_blue);

int cvt_xyz_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                    int width, int height, int depth, int dcn, int swap_blue);

int cvt_yuv420sp_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                            int dst_width, int dst_height, int dcn, int swap_blue, int uidx);

int cvt_yuv420sp_bgr_ex_u8(const unsigned char *y_data, size_t y_step, const unsigned char *uv_data,
                                    size_t uv_step, unsigned char *dst_data, size_t dst_step, int dst_width,
                                    int dst_height, int dcn, int swap_blue, int uidx);

int cvt_yuv420p_bgr_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                        int dst_width, int dst_height, int dcn, int swap_blue, int uidx);

int cvt_bgr_yuv420p_u8(const unsigned char * src_data, size_t src_step, unsigned char * dst_data, size_t dst_step,
                        int width, int height, int scn, int swap_blue, int uidx);

int cvt_bgr_yuv420sp_u8(const unsigned char* src_data, size_t src_step, unsigned char *y_data, size_t y_step,
                        unsigned char *uv_data, size_t uv_step, int width, int height, int scn, bool swap_blue, int uidx);

int cvt_yuv422_bgr_u8(const unsigned char *src_data, size_t src_step, unsigned char *dst_data, size_t dst_step,
                        int width, int height, int dcn, int swap_blue, int uidx, int ycn);

int cvt_bgra_mbgra_u8(const unsigned char *src_data, size_t src_step,
                        unsigned char *dst_data, size_t dst_step, int width, int height);

int cvt_mbgra_bgra_u8(const unsigned char * src_data, size_t src_step,
                        unsigned char * dst_data, size_t dst_step, int width, int height);

int add_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height);
int add_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height);
int add_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height);
int add_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height);
int add_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height);
int add_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height);
int add_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height);

int sub_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height);
int sub_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height);
int sub_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height);
int sub_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height);
int sub_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height);
int sub_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height);
int sub_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height);

int max_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height);
int max_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height);
int max_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height);
int max_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height);
int max_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height);
int max_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height);
int max_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height);

int min_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height);
int min_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height);
int min_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height);
int min_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height);
int min_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height);
int min_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height);
int min_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height);

int absdiff_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height);
int absdiff_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height);
int absdiff_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height);
int absdiff_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height);
int absdiff_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height);
int absdiff_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height);
int absdiff_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height);

int mul_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step,  unsigned char *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step,  unsigned short *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step,  short *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step,  int *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step,  float *dst_data, size_t dst_step, int width, int height, double scalar);
int mul_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step,  double *dst_data, size_t dst_step, int width, int height, double scalar);

int div_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, double scale);
int div_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height, double scale);
int div_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step, unsigned short *dst_data, size_t dst_step, int width, int height, double scale);
int div_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step, short *dst_data, size_t dst_step, int width, int height, double scale);
int div_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step, int *dst_data, size_t dst_step, int width, int height, double scale);
int div_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step, float *dst_data, size_t dst_step, int width, int height, double scale);
int div_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step, double *dst_data, size_t dst_step, int width, int height, double scale);

int recip_u8(const unsigned char *src_data, size_t src_step, unsigned char *dst_data, size_t dst_step, int width, int height, double scale);
int recip_s8(const signed char *src_data, size_t src_step, signed char *dst_data, size_t dst_step, int width, int height, double scale);
int recip_u16(const unsigned short *src_data, size_t src_step, unsigned short *dst_data, size_t dst_step, int width, int height, double scale);
int recip_s16(const short *src_data, size_t src_step, short *dst_data, size_t dst_step, int width, int height, double scale);
int recip_s32(const int *src_data, size_t src_step, int *dst_data, size_t dst_step, int width, int height, double scale);
int recip_f32(const float *src_data, size_t src_step, float *dst_data, size_t dst_step, int width, int height, double scale);
int recip_f64(const double *src_data, size_t src_step, double *dst_data, size_t dst_step, int width, int height, double scale);

int add_weighted_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, signed char *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step, unsigned short *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step, short *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step, int *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step, float *dst_data, size_t dst_step, int width, int height, const double *scale);
int add_weighted_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step, double *dst_data, size_t dst_step, int width, int height, const double *scale);

int and_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height);
int or_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height);
int xor_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height);
int not_u8(const unsigned char *src_data, size_t src_step, unsigned char *dst_data, size_t dst_step, int width, int height);

int cmp_u8(const unsigned char *src1_data, size_t src1_step, const unsigned char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_s8(const signed char *src1_data, size_t src1_step, const signed char *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_u16(const unsigned short *src1_data, size_t src1_step, const unsigned short *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_s16(const short *src1_data, size_t src1_step, const short *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_s32(const int *src1_data, size_t src1_step, const int *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_f32(const float *src1_data, size_t src1_step, const float *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);
int cmp_f64(const double *src1_data, size_t src1_step, const double *src2_data, size_t src2_step, unsigned char *dst_data, size_t dst_step, int width, int height, int operation);

int split_u8(const unsigned char *src_data, unsigned char **dst_data, int len, int cn);
int split_u16(const unsigned short *src_data, unsigned short **dst_data, int len, int cn);
int split_s32(const int *src_data, int **dst_data, int len, int cn);
int split_s64(const int64_t *src_data, int64_t **dst_data, int len, int cn);

int merge_u8(const unsigned char **src_data, unsigned char *dst_data, int len, int cn);
int merge_u16(const unsigned short **src_data, unsigned short *dst_data, int len, int cn);
int merge_s32(const int **src_data, int *dst_data, int len, int cn);
int merge_s64(const int64_t **src_data, int64_t *dst_data, int len, int cn);

int atan_f32(const float *src1_data, const float *src2_data, float *dst_data, int length, bool in_degree);
int atan_f64(const double *src1_data, const double *src2_data, double *dst_data, int length, bool in_degree);

int warp_affine(int src_type, const unsigned char *src_data, size_t src_step, int src_width, int src_height, unsigned char *dst_data, size_t dst_step, int dst_width, int dst_height, const double M[6], int interpolation, int borderType, const double borderValue[4]);

int warp_perspective(int src_type, const unsigned char *src_data, size_t src_step, int src_width, int src_height, unsigned char *dst_data, size_t dst_step, int dst_width, int dst_height, const double M[9], int interpolation, int borderType, const double borderValue[4]);

int pyrdown(const unsigned char *src_data, size_t src_step, int src_width, int src_height, unsigned char *dst_data, size_t dst_step, int dst_width, int dst_height, int depth, int cn, int border_type);

int optflowpyrLK(const unsigned char* src1_data, const unsigned char* src2_data, size_t src1_step, size_t src2_step, int width, int height, int cn, int depth, float* pre_pt,
                 float* next_pt, int num_pt, const unsigned char* status, float *err,int winx, int winy, int maxl, int flags, double min_threshold, int criteria_type, int max_count, double epsilon);

int wrap_countNonZero(const unsigned char *src_data, size_t step, int width, int height, int cn, int depth, int *num);

int wrap_convert(const unsigned char *src_data, size_t src_step, int src_depth, const unsigned char *dst_data,
                        size_t dst_step, int dst_depth, int width, int height, int cn, double alpha, double beta);

int wrap_norm1(const unsigned char *src_data, size_t src_step, int src_depth, const unsigned char *mask_data,
                    size_t mask_step, int mask_depth, int width, int height, int cn, int norm_type, double *scale);

#ifdef __cplusplus
}
#endif

//#undef cv_hal_gaussianBlur
//#define cv_hal_gaussianBlur gaussian_u8
#undef cv_hal_add8u
#define cv_hal_add8u add_u8
#undef cv_hal_add8s
#define cv_hal_add8s add_s8
#undef cv_hal_add16u
#define cv_hal_add16u add_u16
#undef cv_hal_add16s
#define cv_hal_add16s add_s16
#undef cv_hal_add32s
#define cv_hal_add32s add_s32
#undef cv_hal_add32f
#define cv_hal_add32f add_f32
#undef cv_hal_add64f
#define cv_hal_add64f add_f64

#undef cv_hal_sub8u
#define cv_hal_sub8u sub_u8
#undef cv_hal_sub8s
#define cv_hal_sub8s sub_s8
#undef cv_hal_sub16u
#define cv_hal_sub16u sub_u16
#undef cv_hal_sub16s
#define cv_hal_sub16s sub_s16
#undef cv_hal_sub32s
#define cv_hal_sub32s sub_s32
#undef cv_hal_sub32f
#define cv_hal_sub32f sub_f32
#undef cv_hal_sub64f
#define cv_hal_sub64f sub_f64

#undef cv_hal_max8u
#define cv_hal_max8u max_u8
#undef cv_hal_max8s
#define cv_hal_max8s max_s8
#undef cv_hal_max16u
#define cv_hal_max16u max_u16
#undef cv_hal_max16s
#define cv_hal_max16s max_s16
#undef cv_hal_max32s
#define cv_hal_max32s max_s32
#undef cv_hal_max32f
#define cv_hal_max32f max_f32
#undef cv_hal_max64f
#define cv_hal_max64f max_f64

#undef cv_hal_min8u
#define cv_hal_min8u min_u8
#undef cv_hal_min8s
#define cv_hal_min8s min_s8
#undef cv_hal_min16u
#define cv_hal_min16u min_u16
#undef cv_hal_min16s
#define cv_hal_min16s min_s16
#undef cv_hal_min32s
#define cv_hal_min32s min_s32
#undef cv_hal_min32f
#define cv_hal_min32f min_f32
#undef cv_hal_min64f
#define cv_hal_min64f min_f64

#undef cv_hal_mul8u
#define cv_hal_mul8u mul_u8
#undef cv_hal_mul8s
#define cv_hal_mul8s mul_s8
#undef cv_hal_mul16u
#define cv_hal_mul16u mul_u16
#undef cv_hal_mul16s
#define cv_hal_mul16s mul_s16
#undef cv_hal_mul32s
#define cv_hal_mul32s mul_s32
#undef cv_hal_mul32f
#define cv_hal_mul32f mul_f32
#undef cv_hal_mul64f
#define cv_hal_mul64f mul_f64

#undef cv_hal_absdiff8u
#define cv_hal_absdiff8u absdiff_u8
#undef cv_hal_absdiff8s
#define cv_hal_absdiff8s absdiff_s8
#undef cv_hal_absdiff16u
#define cv_hal_absdiff16u absdiff_u16
#undef cv_hal_absdiff16s
#define cv_hal_absdiff16s absdiff_s16
#undef cv_hal_absdiff32s
#define cv_hal_absdiff32s absdiff_s32
#undef cv_hal_absdiff32f
#define cv_hal_absdiff32f absdiff_f32
#undef cv_hal_absdiff64f
#define cv_hal_absdiff64f absdiff_f64

#undef cv_hal_div8u
#define cv_hal_div8u div_u8
#undef cv_hal_div8s
#define cv_hal_div8s div_s8
#undef cv_hal_div16u
#define cv_hal_div16u div_u16
#undef cv_hal_div16s
#define cv_hal_div16s div_s16
#undef cv_hal_div32s
#define cv_hal_div32s div_s32
#undef cv_hal_div32f
#define cv_hal_div32f div_f32
#undef cv_hal_div64f
#define cv_hal_div64f div_f64

#undef cv_hal_recip8u
#define cv_hal_recip8u recip_u8
#undef cv_hal_recip8s
#define cv_hal_recip8s recip_s8
#undef cv_hal_recip16u
#define cv_hal_recip16u recip_u16
#undef cv_hal_recip16s
#define cv_hal_recip16s recip_s16
#undef cv_hal_recip32s
#define cv_hal_recip32s recip_s32
#undef cv_hal_recip32f
#define cv_hal_recip32f recip_f32
#undef cv_hal_recip64f
#define cv_hal_recip64f recip_f64

#undef cv_hal_addWeighted8u
#define cv_hal_addWeighted8u add_weighted_u8
#undef cv_hal_addWeighted8s
#define cv_hal_addWeighted8s add_weighted_s8
#undef cv_hal_addWeighted16u
#define cv_hal_addWeighted16u add_weighted_u16
#undef cv_hal_addWeighted16s
#define cv_hal_addWeighted16s add_weighted_s16
#undef cv_hal_addWeighted32s
#define cv_hal_addWeighted32s add_weighted_s32
#undef cv_hal_addWeighted32f
#define cv_hal_addWeighted32f add_weighted_f32
#undef cv_hal_addWeighted64f
#define cv_hal_addWeighted64f add_weighted_f64

#undef cv_hal_and8u
#define cv_hal_and8u and_u8
#undef cv_hal_or8u
#define cv_hal_or8u or_u8
#undef cv_hal_xor8u
#define cv_hal_xor8u xor_u8
#undef cv_hal_not8u
#define cv_hal_not8u not_u8

#undef cv_hal_cmp8u
#define cv_hal_cmp8u cmp_u8
#undef cv_hal_cmp8s
#define cv_hal_cmp8s cmp_s8
#undef cv_hal_cmp16u
#define cv_hal_cmp16u cmp_u16
#undef cv_hal_cmp16s
#define cv_hal_cmp16s cmp_s16
#undef cv_hal_cmp32s
#define cv_hal_cmp32s cmp_s32
#undef cv_hal_cmp32f
#define cv_hal_cmp32f cmp_f32
#undef cv_hal_cmp64f
#define cv_hal_cmp64f cmp_f64

#undef cv_hal_merge8u
#define cv_hal_merge8u merge_u8

#undef cv_hal_fastAtan32f
#define cv_hal_fastAtan32f atan_f32
#undef cv_hal_fastAtan64f
#define cv_hal_fastAtan64f atan_f64

#undef cv_hal_cvtBGRtoBGR5x5
#define cv_hal_cvtBGRtoBGR5x5 cvt_bgr_bgr5x5_u8

#undef cv_hal_cvtXYZtoBGR
#define cv_hal_cvtXYZtoBGR cvt_xyz_bgr_u8

#undef cv_hal_cvtBGRtoYUV
#define cv_hal_cvtBGRtoYUV cvt_bgr_yuv_u8

#undef cv_hal_cvtYUVtoBGR
#define cv_hal_cvtYUVtoBGR cvt_yuv_bgr_u8

#undef cv_hal_boxFilter
#define cv_hal_boxFilter blur_u8

#undef cv_hal_resize
#define cv_hal_resize resize_u8

#undef cv_hal_threshold 
#define cv_hal_threshold thresh_wrap
#undef cv_hal_medianBlur
#define cv_hal_medianBlur median_blur_u8

#undef cv_hal_morphInit
#define cv_hal_morphInit morph_u8_init

#undef cv_hal_morph
#define cv_hal_morph morph_u8

#undef cv_hal_morphFree
#define cv_hal_morphFree morph_u8_deinit

#undef cv_hal_scharr
#define cv_hal_scharr scharr_u8

#undef cv_hal_warpPerspective
#define cv_hal_warpPerspective warp_perspective

#undef cv_hal_countNonZero
#define cv_hal_countNonZero wrap_countNonZero

#undef cv_hal_convert
#define cv_hal_convert wrap_convert

#endif
