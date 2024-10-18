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
#ifndef _FACE_GENDER_H
#define _FACE_GENDER_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

/**
 * @brief 人脸性别分类结果
 */
typedef struct FaceGenderInfo
{
    float age;                        // 人脸年龄大小
    string gender;                    // 人脸性别
} FaceGenderInfo;

/**
 * @brief 人脸性别分类
 */
class FaceGender : public AIBase
{
public:
    /**
     * @brief FaceGender构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceGender(const char *kmodel_file, const int debug_mode = 1);

    /**
     * @brief FaceGender构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file kmodel文件路径
     * @param isp_shape   isp输入大小（chw）
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceGender(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode);

    /**
     * @brief FaceGender析构函数
     * @return None
     */
    ~FaceGender();

    /**
     * @brief 图片预处理
     * @param ori_img          原始图片
     * @param bbox    原始图片中人脸框位置
     * @return None
     */
    void pre_process(cv::Mat ori_img, Bbox& bbox);

    /**
     * @brief 视频流预处理
     * @param img_data         当前视频帧数据
     * @param bbox             原始图片中人脸框位置
     * @return None
     */
    void pre_process(runtime_tensor& img_data, Bbox& bbox);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @return None
     */
    void post_process(FaceGenderInfo& result);

    /**
     * @brief 将处理好的分类结果画到原图
     * @param src_img     原图
     * @param bbox        人脸的检测框位置
     * @param result      人脸年龄推理结果
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_result(cv::Mat& src_img,Bbox& bbox, FaceGenderInfo& result, bool pic_mode=true);

    /**
     * @brief 将处理好的分类结果画到显示器
     * @param src_img     视频帧
     * @param bbox        人脸的检测框位置
     * @param result      人脸年龄推理结果
     * @return None
     */
    static void draw_result_video(cv::Mat& src_img,Bbox& bbox,FaceGenderInfo& result);
private:
    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    float margin_;                                
};
#endif