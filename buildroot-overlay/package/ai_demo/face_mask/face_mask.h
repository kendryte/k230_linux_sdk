/* Copyright (c) 2024, Canaan Bright Sight Co., Ltd
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
 */
#ifndef _FACE_MASK_H
#define _FACE_MASK_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

/**
 * @brief 人脸是否佩戴口罩结果
 */
typedef struct FaceMaskInfo
{
    string label;                   // 是否佩戴口罩
    float score;                    // 类别得分
} FaceMaskInfo;

/**
 * @brief 人脸是否佩戴口罩
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceMask : public AIBase
{
public:
    /**
     * @brief FaceMask构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file kmodel文件路径
     * @param mask_thresh FaceMask阈值
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceMask(const char *kmodel_file,float mask_thresh, const int debug_mode = 1);

    /**
     * @brief FaceMask构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file kmodel文件路径
     * @param mask_thresh FaceMask阈值
     * @param isp_shape   isp输入大小（chw）
     * @param vaddr       isp对应虚拟地址
     * @param paddr       isp对应物理地址
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceMask(const char *kmodel_file,float mask_thresh, FrameCHWSize isp_shape, const int debug_mode);

    /**
     * @brief FaceMask析构函数
     * @return None
     */
    ~FaceMask();

    /**
     * @brief 图片预处理
     * @param ori_img          原始图片
     * @param sparse_points    原始图片中人脸五官点位置
     * @return None
     */
    void pre_process(cv::Mat ori_img, float* sparse_points);

    /**
     * @brief 视频流预处理
     * @param img_data         当前视频帧数据
     * @param sparse_points 原始图片中人脸五官点位置
     * @return None
     */
    void pre_process(runtime_tensor& img_data, float* sparse_points);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @return None
     */
    void post_process(FaceMaskInfo& result);

    /**
     * @brief 将处理好的分类结果画到原图
     * @param src_img     原图
     * @param bbox        人脸的检测框位置
     * @param result      人脸佩戴口罩分类结果
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_result(cv::Mat& src_img,Bbox& bbox, FaceMaskInfo& result, bool pic_mode=true);

    /**
     * @brief 将处理好的分类结果画到显示器
     * @param src_img     视频帧
     * @param bbox        人脸的检测框位置
     * @param result      人脸佩戴口罩分类结果
     * @return None
     */
    static void draw_result_video(cv::Mat& src_img,Bbox& bbox,FaceMaskInfo& result);
private:
    /** 
     * @brief svd
     * @param a     原始矩阵
     * @param u     左奇异向量
     * @param s     对角阵
     * @param v     右奇异向量
     * @return None
     */
    void svd22(const float a[4], float u[4], float s[2], float v[4]);
    
    /**
    * @brief 使用Umeyama算法计算仿射变换矩阵
    * @param src  原图像点位置
    * @param dst  目标图像（128*128）点位置。
    */
    void image_umeyama_128(float* src, float* dst);

    /**
    * @brief 获取affine变换矩阵
    * @param sparse_points  原图像人脸五官点位置
    */
    void get_affine_matrix(float* sparse_points);

    /**
     * @brief 计算数据softmax之后的值
     * @param input        原始数据
     * @param output       计算softmax之后的数据
     * @return None
     */
    void softmax(vector<float>& input,vector<float>& output);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    float matrix_dst_[10];                       // 人脸affine的变换矩阵
    float mask_thresh_;                          // 人脸佩戴口罩阈值
};
#endif